###### High level information about applications running on Network capture device (RPi) ###########
###### And what is the role of each application #########################
This project 'network-capture' is a maven, Java8 based
java client which runs on network capture device, i.e.
in this case Raspberry Pi.

Requirements for this application : 
Java 8, was tested on Raspberry Pi

From this point on capture device will be referred directly
by Raspberry Pi (RPi), but in-theory it can be any network
capture hardware device.
 
There are two software components that run on RPi both working in local
client-server architecture.
(i) C based local server : 
    --------------------
Local server which is entirely written in C, should be the application
that is started first on RPi. When started it will start listening for
requests on port 8765 from local clients. 
Once it receives 'capture' command from any client, it will
- capture network packets(currently Beacon packets containing RadioTap headers)
- convert them into JSON and then
- send that JSON as response to the client's request.


(ii) Java based local client :
    ------------------------
NOTE: This java client should be started only after C Server has been started 
and is listening for requests on pre-determined port (8765).

Java based local client serves two purpose 
(1) It connects to the remote Java Webapp (currently 'http://na.eurekabits.com')
    and requests latest configuration for itself from it.
    The configuration primarily contains two things
    (I) How frequently client should collect network packets from local(C) server
        and send it to remote server
    (II) How many packets should it ask local server to capture.
(2) Connects with local C server and sends capture commands to it and
    sends JSON returned by it to remote Server.
    
When this Java App is run : 
- It starts a local Quartz based task(ConfigSyncJob.java) (as configured in QuartzConfiguration.java)
  and requests configuration from remote Server every N seconds(currently 15secs).
- It then compares its local cache(in-memory) to see if it already has the configuration and
  if the new configuration is same as currently fetched one. If the configuration
  hasn't changed then it simply doesn't do anything.
  Else if configuration has changed then it will use the capture frequency specified
  in configuration to start a local Timer task PacketCaptureTimerTask.java configured
  to execute as per capture frequency specified by Remote Server.
  
- PacketCaptureTimerTask.java during its every run :
  (I) Sends 'capture' command to local C Server (which indicates to server it has to capture
      network packets) and receives back JSON from C Server which is the captured data.
  (II) Immediately sends this JSON data (POST request) to remote server (makes RESTful API call).
       Remote server saves this data in it's local MySQL database as statistics, which are eventually
       displayed as Statistics Graphs on UI in browser.
- Note: This Java Client and remote server currently communicate over HTTP using BASIC Authentication.
        Currently username and password in hard-coded in the application.
        In future Java client and remote Server can easily communicate over HTTPs making it secure,
        without need of changing any software component.
- Note: Remote Server exposes RESTful Service and all communication (whether fetch config or sending 
        captured data) happens using REST APIs over HTTP(s).
