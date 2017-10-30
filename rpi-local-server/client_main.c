/**********************************************************************
* Author: Richa Lakhe
*
* Description:
*
* Compile with:
* gcc -Wall -pedantic client_main.c -lpcap (-o output_file_name)
*
* Usage:
* a.out (# of packets) "filter string"
*
**********************************************************************/

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include "radiotap-parser.h"

#define MAX_PACKET_INFO_COUNT 50 // Currently let's just assume we support max 50 packet storage
#define MAX_PACKET_CAPTURE_COUNT 100 // This is the # of packets we capture and filter it down to max MAX_PACKET_INFO_COUNT
#define MAX_SSID_CHAR_COUNT 15 // Currently let's just assume we support 15 chars for SSID.
static int cur_packet_count = 0; // Holds the current number of packet being processed.
static int dup_packets_discarded = 0; // Holds the number of duplicate (same SSID, same BSSID) packets discarded.
static int packets_same_ssid_diff_bssid = 0; // Holds the number of packets with same SSID, but different BSSID.
#define MAX_OUTPUT_BUFFER_SIZE 8192
char output_buff[MAX_OUTPUT_BUFFER_SIZE];
char *buff_ptr;

/**
 * Custom structure which Maps to Ethernet Beacon frame.
 */
struct ethernet_beacon_frame {
	u16 frame_control_field;
	u16 time_in_micros;
	u_int8_t  ether_dhost[ETH_ALEN];	/* destination eth addr	*/
	u_int8_t  ether_shost[ETH_ALEN];	/* source ether addr	*/
	u_int8_t  bssid[ETH_ALEN];	        /* BSS Id	*/
	u16 fragment_seq_number;
};

/**
 * Custom structure which Maps to a Tag of Ethernet Beacon frame.
 */
struct ethernet_mgmt_frame_tag {
	u8 tag_number;  /*1byte Tag number*/
	u8 tag_length; /*1byte Tag length*/
	char ssid_start;
};

/**
 * Custom structure which Maps to Ethernet Management frame.
 */
struct ethernet_mgmt_frame {
	unsigned char fixed_params[12];  /*Fixed parameters of 12 bytes*/
	struct ethernet_mgmt_frame_tag mgmt_frame_tag;
};

/**
 * Custom structure which holds a single packet's information.
 */
struct packet_info {
	unsigned int ethernet_type;
	int data_rate_kbps, antenna;
	unsigned short channel;
	char signal_pwr_dbm_at_antenna;
	char ssid[MAX_SSID_CHAR_COUNT];
	char ethernet_src_addr[18]; // 48-bit (12 chars + 5 colons + 1 NULL char) Source Ethernet Addr
	char ethernet_dst_addr[18]; // 48-bit Destination Ethernet Addr
	char ethernet_bss_id[18]; // 48-bits BSS ID.
};

/**
 * Custom structure to carry all the captured data.
 * This will hold an array of packet capture info.
 * Field 'packet_info_count' will indicate how many elements are valid from array.
 */
struct capture_info {
	int packet_info_count; // how many packets we really captured.
	struct packet_info packet_info_array[50]; // buffer to hold all the captured packed info.
};

// Global variable to hold all captured info, let's keep it simple for now.
struct capture_info cap_info;

#if 0
/*
 * Structs exported from netinet/in.h (for easy reference)
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr;
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

/**
 * This function parses the RadioTap header from provided packet.
 *
 * @param packet_ptr Pointer to the captured Packet's buffer
 * @param buflen Max length of Packet buffer.
 * @return Zero on success. Negative on failure.
 */
int parseRadioTap(struct ieee80211_radiotap_header * packet_ptr, int packet_buf_len)
{
	int data_rate_kbps = 0, antenna = 0;
	unsigned short channel = 0;
	unsigned short *channel_ptr;
	char signal_pwr_dbm_at_antenna = 0;
	char *signal_pwr_dbm_ptr;
	struct ieee80211_radiotap_iterator iterator;
	// Initialize the Iterator first.
	// Return value is -1 if unsuccessful initialization.
	int ret = ieee80211_radiotap_iterator_init(&iterator, packet_ptr, packet_buf_len);

    // Proceed only if initialization was successful.
	while (ret != -1) {
		// Get next Radio Tap field.
		ret = ieee80211_radiotap_iterator_next(&iterator);

		// -1 return value imples no more fields available.
		if (ret == -1) {
			return 0;
		}

		//fprintf(stdout, "\n");
		switch (iterator.this_arg_index) {
		/*
		 * You must take care when dereferencing iterator.this_arg
		 * for multibyte types... the pointer is not aligned.  Use
		 * get_unaligned((type *)iterator.this_arg) to dereference
		 * iterator.this_arg for type "type" safely on all arches.
		 */
		case IEEE80211_RADIOTAP_RATE:
			// radiotap "rate" u8 is in
			// 500kbps units, eg, 0x02=1Mbps
			data_rate_kbps = ((*iterator.this_arg) * 500);
			cap_info.packet_info_array[cur_packet_count].data_rate_kbps = data_rate_kbps;
			//printf(">>>RadioTap Info : Packet Rate : %i Mbps\n", data_rate_kbps/1000);
			break;

		case IEEE80211_RADIOTAP_ANTENNA:
			/* radiotap uses 0 for 1st ant */
			antenna = (*iterator.this_arg);
			cap_info.packet_info_array[cur_packet_count].antenna = antenna;
			//printf(">>>RadioTap Info : Antenna : %i\n", antenna);
			break;
		case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
			//RF signal power at the antenna, decibel difference from one milliwatt.
			signal_pwr_dbm_ptr = iterator.this_arg;
			signal_pwr_dbm_at_antenna = *signal_pwr_dbm_ptr;
			cap_info.packet_info_array[cur_packet_count].signal_pwr_dbm_at_antenna = signal_pwr_dbm_at_antenna;
			//printf(">>>RadioTap Info : Power@Antenna : %i dBm\n", signal_pwr_dbm_at_antenna);
			break;
		case IEEE80211_RADIOTAP_CHANNEL:
			channel_ptr = iterator.this_arg;
			channel = *channel_ptr; // Channel Freq in MHz.
			cap_info.packet_info_array[cur_packet_count].channel = channel;
			//printf(">>>RadioTap Info : Channel=%f GHz\n", (float)((float)channel/1000));
			break;
		default:
			//printf(">>>RadioTap Info : Found non-standard index : %i\n", iterator.this_arg_index);
			break;
		}
	}  /* while more rt headers */

	if (ret != -ENOENT)
		return -1;

	/* discard the radiotap header part */
	packet_ptr += iterator.max_length;
	packet_buf_len -= iterator.max_length;
	return 0;
}

/**
 * Find if any Packet with given ssid+bssid has already been captured.
 *
 * @return 1 if exists, else 0.
 */
int pkt_with_ssid_bssid_exists(char *new_ssid, char *new_bssid) {
	// search linearly from start to latest packet captured
	short count=0;
	for(count = 0; count < cur_packet_count; count++) {
		char *existing_ssid = cap_info.packet_info_array[count].ssid;
		char *existing_bssid = cap_info.packet_info_array[count].ethernet_bss_id;

		// Calculate another stat, Packets with same SSID, but different BSSID
		if((strcmp(existing_ssid, new_ssid) == 0) &&
				(strcmp(existing_bssid, new_bssid) != 0)) {
			//fprintf(stdout, "\nFound packet with same SSID: '%s' but different BSSID: '%s'", new_ssid, new_bssid);
			packets_same_ssid_diff_bssid++;
		}

		//fprintf(stdout,"\n Comparing OldSSID '%s', New '%s' ; OldBSSID '%s', New '%s'", existing_ssid, new_ssid, existing_bssid, new_bssid);
		// check if any previous packet matches both SSID and BSSID. Then discard.
		if((strcmp(existing_ssid, new_ssid) == 0) &&
				(strcmp(existing_bssid, new_bssid) == 0)) {
			//fprintf(stdout, "\nFound <SSID,BSSID> match!");
			dup_packets_discarded++;
			return 1;
		}
	}
	return 0; // no match.
}

/**
 * Try to parse Ethernet information from provided packet.
 *
 * @param args Arguments
 * @param pkthdr Generic per packet information provided by libpcap
 * @param packet Packet buffer.
 * @return Integer represending Ethernet Frame type. Zero if packet wasn't parsed(mostly since SSID already exists).
 */
unsigned int parse_ethernet(struct capture_info* cap_info2,
		const struct pcap_pkthdr* pkthdr,
		const u_char* packet) {

	u_char *ptr = packet;

	ptr += 18; // goto start of Beacon frame (First 18-bytes is RadioTap header).
	struct ethernet_beacon_frame *beacon_frame = (struct ethernet_beacon_frame *) ptr;
	unsigned int ether_frame_type = ntohs (beacon_frame->frame_control_field);
	//fprintf(stdout,"\nETHER_TYPE:(%x)\n", ether_frame_type, ether_frame_type);
	cap_info.packet_info_array[cur_packet_count].ethernet_type = ether_frame_type;

	switch(ether_frame_type) {
	case 0x8000:
		//fprintf(stdout, "\n-----------ETHER IPv4--------------");
		ptr += 24; // goto start of WLAN Mgmt Frame
		struct ethernet_mgmt_frame *mgmt_frame = (struct ethernet_mgmt_frame *) ptr;
		char ssid_buff[MAX_SSID_CHAR_COUNT];
		int ssid_length = mgmt_frame->mgmt_frame_tag.tag_length;
		int char_count = 0; // count of SSID string position.
		if(ssid_length > 0) {
			char *ssid_ptr = & (mgmt_frame->mgmt_frame_tag.ssid_start); // get address of start of SSID string.
			// Start filling SSID in buffer.
			for(char_count = 0; char_count < ssid_length; char_count++) {
				ssid_buff[char_count] = *ssid_ptr;
				cap_info.packet_info_array[cur_packet_count].ssid[char_count] = *ssid_ptr;
				ssid_ptr++; // goto next char of SSID.
			}
		}
		//Null terminate last char
		if(ssid_length < MAX_SSID_CHAR_COUNT) {
			ssid_buff[char_count] = '\0';
			cap_info.packet_info_array[cur_packet_count].ssid[char_count] = '\0';
		}else {
			// (Note: In this case we might truncate the SSID, fine for now).
			ssid_buff[MAX_SSID_CHAR_COUNT] = '\0';
			cap_info.packet_info_array[cur_packet_count].ssid[MAX_SSID_CHAR_COUNT] = '\0';
		}
		//fprintf(stdout, "\nSSID = %s", ssid_buff);

		// **** Process BSSID now ******
		//fprintf(stdout,"\nETHER_BSS_ID: %s",ether_ntoa(beacon_frame->bssid));
		char * addr_ptr = ether_ntoa(beacon_frame->bssid);
		for(char_count=0; char_count < 17/*48bit+colons(17bytes) Ether Addr*/; char_count++) {
			cap_info.packet_info_array[cur_packet_count].ethernet_bss_id[char_count] = *addr_ptr;
			addr_ptr++; // goto next character
		}
		cap_info.packet_info_array[cur_packet_count].ethernet_bss_id[char_count] = '\0';

		// If packet with this SSID,BSSID is previously captured, move-on to next packet processing.
		int found_dup_pkt = pkt_with_ssid_bssid_exists(ssid_buff, cap_info.packet_info_array[cur_packet_count].ethernet_bss_id);
		if(found_dup_pkt == 1) {
			return 0;
		}


		//fprintf(stdout,"\nETHER_DST_ADDR: %s",ether_ntoa(beacon_frame->ether_dhost));
		addr_ptr = ether_ntoa(beacon_frame->ether_dhost);
		for(char_count=0; char_count < 17/*48bit+colons(17bytes) Ether Addr*/; char_count++) {
			cap_info.packet_info_array[cur_packet_count].ethernet_dst_addr[char_count] = *addr_ptr;
			addr_ptr++; // goto next character
		}
		cap_info.packet_info_array[cur_packet_count].ethernet_dst_addr[char_count] = '\0';

		//fprintf(stdout,"\nETHER_SRC_ADDR: %s",ether_ntoa(beacon_frame->ether_shost));
		addr_ptr = ether_ntoa(beacon_frame->ether_shost);
		for(char_count=0; char_count < 17/*48bit+colons(17bytes) Ether Addr*/; char_count++) {
			cap_info.packet_info_array[cur_packet_count].ethernet_src_addr[char_count] = *addr_ptr;
			addr_ptr++; // goto next character
		}
		cap_info.packet_info_array[cur_packet_count].ethernet_src_addr[char_count] = '\0';

		//fprintf(stdout, "\n-----------ETHER IPv4--------------");
		break;
	default:
		//fprintf(stdout, "NOT ETHER IPv4 Frame. Not Analyzing!");
		break;
	}

	return ether_frame_type;
}

/**
 * Callback function invoked by pcaplib for each packet captured.
 * We need to handle all packet processing in this method.
 *
 * @param args Arguments
 * @param pkthdr Generic per packet information provided by libpcap
 * @param packet Packet buffer.
 */
void packet_capture_callback(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char*
        packet)
{
	// Convert the args pointer to expected struct pointer.
	// cap_info will carry all the captured data.
	struct capture_info *cap_info2 = (struct capture_info *) args;

	// Skip further packet processing, if we have processed Max allowed packets.
	if(cur_packet_count >= MAX_PACKET_INFO_COUNT) {
		return; // skip further processing.
	}

	//fprintf(stdout, "\n#########################################");
	// First try to parse the capture packet for Ethernet frames.
	// Based on Ethernet Type returned by this method, we can decide
	// if we can go ahead and parse the RadioTap header information.
	int ethernet_type = parse_ethernet(cap_info2,pkthdr,packet);
	// if return value is zero => this packet wasn't processed
	//                           (mostly because packet of same SSID was already processed before).
	//                           In this case we don't increment the count and skip further processing of current packet.
	if(ethernet_type == 0) {
		return; // skip further processing.
	}

    unsigned int ether_frame_type = cap_info.packet_info_array[cur_packet_count].ethernet_type;
    // Parse RadioTap info only if this is ETHER IPv4 type packet.
    if(cap_info.packet_info_array[cur_packet_count].ethernet_type == 0x8000/*ETHER IPv4*/) {
    	int retVal = parseRadioTap(packet, BUFSIZ);
    } else {
    	fprintf(stdout, "NOT ETHER IPv4 Frame. Not analyzing RadioTap Info!");
    }
    // Increment count after we finish processing current packet
    cur_packet_count++;
    cap_info.packet_info_count++;
}



int capture_packets(char *dev, int num_of_packets, char *filter_string
		, struct capture_info *cap_info2) {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* descr;
	struct bpf_program fp;      /* hold compiled program     */
	bpf_u_int32 maskp;          /* subnet mask               */
	bpf_u_int32 netp;           /* ip                        */
	//u_char* args = NULL;

	// Uncomment following if we want to capture from some default interface.
	//dev = pcap_lookupdev(errbuf);

	if(dev == NULL)
	{ printf("%s\n",errbuf); exit(1); }

	// take network address and mask of the device from pcap.
	pcap_lookupnet(dev,&netp,&maskp,errbuf);

	// open device 'dev' for reading.
	// NOTE: we are going to operate in promiscuous mode to capture
	//       Beacon / RadioTap / Ethernet Management Frame information.
	descr = pcap_open_live(dev,BUFSIZ,1,-1,errbuf);

	if(descr == NULL)
	{ printf("pcap_open_live(): %s\n",errbuf); exit(1); }

	// If user has provided filter string, use that string
	// to filter specified types of packets to be captured by pcap.
	if(filter_string != NULL)
	{
		// Compile the user provided filter program(string)
		// If any issue with filter program, we exit.
		if(pcap_compile(descr,&fp,filter_string,0,netp) == -1)
		{ fprintf(stderr,"Error calling pcap_compile\n"); exit(1); }

		// Since compilation was success, use compiled program as the filter
		if(pcap_setfilter(descr,&fp) == -1)
		{ fprintf(stderr,"Error setting filter\n"); exit(1); }
	}

	// START capturing the user provided number of packets.
	// For each captured packet provided callback function will be invoked,
	// wherein we can process the captured packet.
	printf("\nStarting to capture %i number of packets\n", num_of_packets);
	cur_packet_count = 0; //reset count to zero, before we start capturing new set of packets.
	dup_packets_discarded = 0; // reset count to zero.
	packets_same_ssid_diff_bssid = 0; // reset count to zero.
	cap_info.packet_info_count = 0; // reset packet count to zero.
	pcap_loop(descr,num_of_packets,packet_capture_callback,cap_info2);

	// Above loop will exit once pcap has completed capturing required
	// number of packets. At this time we are done.
	fprintf(stdout,"\n**Finished**. Saved %i packets. Duplicate Packets discarded %i . Packets with SAME SSID DIFF BSSID %i \n",
			cur_packet_count, dup_packets_discarded, packets_same_ssid_diff_bssid);
	return 0;
}

void writePacket(int cur_packet_num) {
	int cnt = sprintf(buff_ptr, "{\"packet\":{\"datarate\":\"%i\",\"antenna\":\"%i\",\"channel\":\"%i\",\"power\":\"%i\",\"ssid\":\"%s\",\"src\":\"%s\",\"dst\":\"%s\",\"bssid\":\"%s\"}}",
			cap_info.packet_info_array[cur_packet_num].data_rate_kbps,
			cap_info.packet_info_array[cur_packet_num].antenna,
			cap_info.packet_info_array[cur_packet_num].channel,
			cap_info.packet_info_array[cur_packet_num].signal_pwr_dbm_at_antenna,
			cap_info.packet_info_array[cur_packet_num].ssid,
			cap_info.packet_info_array[cur_packet_num].ethernet_src_addr,
			cap_info.packet_info_array[cur_packet_num].ethernet_dst_addr,
			cap_info.packet_info_array[cur_packet_num].ethernet_bss_id);
	buff_ptr += cnt;
}

void generateJson() {
	int pkt_count = 0;
	buff_ptr = output_buff;

    int cnt = sprintf(buff_ptr, "{\"packets\":[");
    buff_ptr += cnt;

	// Iterate through each captured packet and generate JSON for it.
	for(pkt_count = 0; pkt_count < cap_info.packet_info_count; pkt_count++) {
		writePacket(pkt_count);
		// comma after all but last packet object
		if(pkt_count < (cap_info.packet_info_count-1)) {
			// comma after all but last packet object
			cnt = sprintf(buff_ptr, ",");
			buff_ptr += cnt;
		}
	}

	cnt = sprintf(buff_ptr, "]}");
	buff_ptr += cnt;
}

void process_client_command(char *client_cmd_buf, struct capture_info *cap_info) {
	if(strcmp(client_cmd_buf, "exit\r\n") == 0) {
		exit(0);
	}else if(strcmp(client_cmd_buf, "exit") == 0) {
		exit(0);
	}else if(strcmp(client_cmd_buf, "capture\r\n") == 0) {
		char filter_string[] = "subtype beacon";
		capture_packets("mon1", MAX_PACKET_CAPTURE_COUNT, filter_string, cap_info);
		generateJson();
	}else if(strcmp(client_cmd_buf, "capture") == 0) {
		char filter_string[] = "subtype beacon";
		capture_packets("mon1", MAX_PACKET_CAPTURE_COUNT, filter_string, cap_info);
		generateJson();
	}else {
		printf("\n UNKNOWN command '%s' received from Client.", client_cmd_buf);
	}
}

#define BUFSIZE 1024
/**
 * Program entry point main.
 *
 */
int main(int argc,char **argv)
{
	int listenfd; /* listening socket */
	int connfd; /* connection socket */
	int portno; /* port to listen on */
	int clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char client_cmd_buf[BUFSIZE]; /* client command buffer */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */
	struct capture_info cap_info2[MAX_PACKET_INFO_COUNT]; // holds the captured data.

	/* check command line args */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

	/* socket: create a socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets
	 * us rerun the server immediately after we kill it;
	 * otherwise we have to wait about 20 secs.
	 * Eliminates "ERROR on binding: Address already in use" error.
	 */
	optval = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval , sizeof(int));

	/* build the server's internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; /* we are using the Internet */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
	serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

	/* bind: associate the listening socket with a port */
	if (bind(listenfd, (struct sockaddr *) &serveraddr,
			sizeof(serveraddr)) < 0)
		error("ERROR on binding");

	/* listen: make it a listening socket ready to accept connection requests */
	if (listen(listenfd, 5) < 0) /* allow 5 requests to queue up */
		error("ERROR on listen");

	/* main loop: wait for a connection request, echo input line,
	     then close connection. */
	clientlen = sizeof(clientaddr);
	printf("*****SERVER started, waiting for client connections.....\n");
	while (1) {

		/* accept: wait for a connection request */
		connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
		if (connfd < 0)
			error("ERROR on accept");

		/* gethostbyaddr: determine who sent the message */
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
				sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL)
			error("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
			error("ERROR on inet_ntoa\n");
		printf("\nserver established connection with %s (%s)\n",
				hostp->h_name, hostaddrp);


		/* read: read input string from the client */
		bzero(client_cmd_buf, BUFSIZE);
		n = read(connfd, client_cmd_buf, BUFSIZE);
		if (n < 0)
			error("ERROR reading from socket");
		printf("server received %d bytes command: '%s'", n, client_cmd_buf);
		process_client_command(client_cmd_buf, cap_info2);

		/* write: echo the input string back to the client */

		/*char buf[1024];
		int chars = sprintf(buf, "{\"count\":\"%i\"}", 88);*/

		n = write(connfd, output_buff, strlen(output_buff));
		if (n < 0)
			error("ERROR writing to socket");

		close(connfd);
	}
    return 0;
}
