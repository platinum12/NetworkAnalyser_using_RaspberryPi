#########################################################################
# Network Capture using Raspberry Pi
# ----------------------------------
#
# This folder contains two software components
# which are to be run on Raspberry Pi
# (1) C Local Server application (2) Local Standalone Java application(client)
# Remote Java Server Webapp is part of bigger EurekaBits project
# RESTFul APIs can be found in files (exported from Postman):
# (i)EurekaBits-NA.json.postman_collection
# (ii) EurekaBits-Users.json.postman_collection
#########################################################################

## Project has been divided into following components : 

1) Software on Raspberry Pi
  ===========================

    (i) C Local Server application (folder = rpi-local-server) :
        ------------------------------------------------------- 

- On start accepts connection from local clients. Accepts commands from local clients.

- When it get ‘capture’ command from local clients, it will capture Beacon packets and 

convert them into JSON string. This JSON will be sent back as response to client


    (ii) Local Standalone Java Application (folder = rpi-local-client) :
         ---------------------------------------------------------

- On start will try to connect to remote server and get capture interval configuration from it.

- Based on received capture interval, schedule a task to regularly contact local C Server 

to capture packets and return the JSON data to Remote Server



2) Software on Remote Server
   ========================

     (i) Java Webapp (part of bigger Eurekabits Repository): 

- This is a Spring based typical MVC Java Webapp

- It is deployed on Tomcat7 Servlet container

- It exposes RESTful APIs which are consumed by rpi-java-app and UI

- It uses MYSQL database for storing captured packets

- The main work done by app is to provide configuration to Pi , store captured data in 

mysql database and return captured data as per request to UI.

- All RESTful API’s are protected by BASIC Authentication. The final product will have all 

communication over HTTPs and hence totally secure.

     (ii) Static JavaScript based UI Pages (folder = UI) : 

- Currently we have a single UI page index.html which shows the captured data in form of 

LineChart graph

- We use JQUERY and ChartJS.org for making AJAX calls and graphing functionality.

- UI pulls all the desired captured data from Remote Server using RESTful API call. 

Data is returned as JSON.
