package com.eurekabits;

import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.charset.Charset;
import java.util.Base64;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;
import org.springframework.web.client.RestTemplate;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * Service which interacts with Remote Network capture client.
 */
@Service
public class RpiServiceImpl implements RpiService {
    
    private static final String RpiId = "4062a43b-7e77-42c7-a694-f35a264a18db";
    private static final Logger logger = LoggerFactory.getLogger(RpiServiceImpl.class);
    private final RestTemplate restTemplate; // for making RESTful calls to WebServer.
    static final int MAX_NETWORK_INPUT_BUFFER_SIZE = 8192; // Size of buffer to store data from Remote client.
    static final String CLIENT_CAPTURE_COMMAND = "capture";
    
    Socket socket;
    OutputStream out = null;
    InputStream in = null;
    InetAddress host = null;
    
    public RpiServiceImpl() {
        // Initialize RestTemplate
        restTemplate = new RestTemplate();
        // Init Networking info for remote calls to RpiClient
        try {
			host = InetAddress.getLocalHost();
		} catch (UnknownHostException e) {
			logger.error("ERROR getting localhost address in RpiService. Ex = {}", e.getMessage());
			e.printStackTrace();
		}
    }

    @Override
    public NetworkCaptureData getNetworkInfoFromClient() {
    	byte[] captureCmdBytes = CLIENT_CAPTURE_COMMAND.getBytes();
    	byte[] captureDataBytes = new byte[MAX_NETWORK_INPUT_BUFFER_SIZE];
    	NetworkCaptureData data = null;
    	try {
    		// establish socket connection to remote client
            socket = new Socket(host.getHostName(), 8765);
            // write to socket using OutputStream
            out = socket.getOutputStream();
            logger.debug("Sending request to Socket Server");
            out.write(captureCmdBytes);
            // read the server response message
            in = socket.getInputStream();
            int readCount = in.read(captureDataBytes);
            logger.debug("Network Capture data: " + new String(captureDataBytes));
            if(readCount > 0) {
            	try {
            		ObjectMapper mapper = new ObjectMapper();
            		data = mapper.readValue(captureDataBytes, NetworkCaptureData.class);
            		logger.debug("De-Serialized data : {}", data);
            	} catch(JsonGenerationException jsonex) {
            		logger.error("ERROR while deserializing JSON data to Java Object");
            		jsonex.printStackTrace();
            	}
            }
            // close resources
            in.close();
            out.close();
            Thread.sleep(100);
			
		} catch (IOException e) {
			logger.error("IOException while sending capture command to RpiClient: {}", e.getMessage());
			e.printStackTrace();
		} catch (InterruptedException ie) {
			logger.error("InterruptedException while sending capture command to RpiClient: {}", ie.getMessage());
			ie.printStackTrace();
		}
    	
    	// Add missing RpiId
    	data.setDevId(RpiId);

    	return data;
    }
    
    /**
     *
     * @return
     */
    @Override
    public RpiConfig getRpiConfigFromServer() {
        ResponseEntity<RpiConfig> response;
        HttpHeaders httpHeaders = createHeaders("richa123", "password");

        //String url = "http://localhost:8080/api/v1/na/config?id=" + RpiId;
        String url = "http://na.eurekabits.com/api/v1/na/config?id=" + RpiId;
        response = restTemplate.exchange(url, HttpMethod.GET, new HttpEntity<>(httpHeaders), RpiConfig.class);
        RpiConfig rpiConfig = response.getBody();
        return rpiConfig;
        
    }
    
    /**
     * 
     * @param networkCapture
     * @return 
     */
    @Override
    public ResponseEntity<String> postCapturedDataToServer(NetworkCaptureData networkCaptureData) {
        ResponseEntity<String> response;
        HttpHeaders httpHeaders = createHeaders("richa123", "password");
        HttpEntity<NetworkCaptureData> httpEntity = new HttpEntity<>(networkCaptureData, httpHeaders);

        //String url = "http://localhost:8080/api/v1/na/network-capture";
        String url = "http://na.eurekabits.com/api/v1/na/network-capture";
        response = restTemplate.exchange(url, HttpMethod.POST, httpEntity, String.class);
        
        return response;
    }
    
    /**
     * Create Standard required headers (eg: BASIC Auth etc) for RESTful calls.
     *
     * @param username
     * @param password
     * @return 
     */
    private HttpHeaders createHeaders(final String username, final String password) {
        HttpHeaders headers = new HttpHeaders() {
            {
                String auth = username + ":" + password;
                
                byte[] encodedAuth = Base64.getEncoder().encode(
                        auth.getBytes(Charset.forName("US-ASCII")));
                String authHeader = "Basic " + new String(encodedAuth);
                set("Authorization", authHeader);
            }
        };
        // Webserver currently only supports application/json
        headers.add("Content-Type", "application/json");
        headers.add("Accept", "application/json");

        return headers;
    }
    
}
