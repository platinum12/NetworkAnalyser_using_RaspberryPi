package com.eurekabits;

import java.util.Arrays;
import java.util.TimerTask;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;

/**
 * TimerTask to capture network packets and send them to Server.
 */
public class PacketCaptureTimerTask extends TimerTask {
    
    private static final Logger logger = LoggerFactory.getLogger(PacketCaptureTimerTask.class);
    
    private final RpiService rpiService;
    
    public PacketCaptureTimerTask(RpiService rpiService) {
        this.rpiService = rpiService;
    }

    @Override
    public void run() {
        long startTimeMs = System.currentTimeMillis();
        logger.debug("Starting packet capture....");
        try {
            captureAndSendNetworkData();
        } catch (RuntimeException rex) {
            logger.error("Error while running Capture Task: {}. Detailed Msg: {}", rex.getLocalizedMessage(), rex.getMessage());
            logger.debug(Arrays.toString(rex.getStackTrace()));
        } finally {
            logger.debug("Packet capture and data export done in {} ms.", (System.currentTimeMillis()- startTimeMs));
        }
    }
    
    private void captureAndSendNetworkData() {
        
        // First Capture Network data.
        NetworkCaptureData networkCaptureData = rpiService.getNetworkInfoFromClient();
        logger.debug("Data captured from client : {}", networkCaptureData);
        
        // Then Send captured network data to server.
        ResponseEntity<String> response = rpiService.postCapturedDataToServer(networkCaptureData);
        logger.debug(response.toString());
        logger.info("Sent data to Server. Server Response: {}", response.getStatusCode());
    }
    
}
