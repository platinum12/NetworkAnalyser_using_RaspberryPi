package com.eurekabits;

import java.time.LocalDateTime;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicInteger;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * Job which synchronizes configuration of RaspberryPi client with Server.
 */
@Service("configSyncJob")
public class ConfigSyncJob {
    private static final Logger logger = LoggerFactory.getLogger(ConfigSyncJob.class);
    private Timer timer;
    private TimerTask captureTimerTask;
    private final AtomicInteger jobRunCount;
    private RpiConfig curRpiConfig; 
    @Autowired
    RpiService rpiService;
    
    public ConfigSyncJob() {
        logger.debug("In ConfigSyncJob Constructor");
        this.jobRunCount = new AtomicInteger();                
        logger.debug("Exiting ConfigSyncJob Constructor");
    }
    
    /**
     * Job entry method.
     */
    protected void start() {
        long startTimeInMs = System.currentTimeMillis();
    	logger.debug("Started ConfigSyncJob(run={}) @{}", jobRunCount.incrementAndGet(), LocalDateTime.now());
        try {

            // Get Config from server.
            RpiConfig rpiConfig = rpiService.getRpiConfigFromServer();
            logger.debug(rpiConfig.toString());
            
            // Check if existing config is same as one received from Server.
            // If configs are same, don't cancel existing TimerTask and don't start new one.
            if(curRpiConfig != null/*skip first run*/ && curRpiConfig.equals(rpiConfig)) {
            	logger.debug("No change in RpiConfig. Not scheduling new TimerTask");
            	return;
            }

            // Save the Config received from Server as current configuration.
            curRpiConfig = rpiConfig; 
            
            // Cancel existing TimerTask if it exists.
            if(timer != null) {
                timer.cancel();
            }
            
            // Give some time for TimerTask to clean-up previous threads.
            try {
                logger.debug("Waiting for TimerTask cleanup ....");
                Thread.sleep(5000); // 5 secs for cleanup.
                logger.debug("Done TimerTask cleanup.");
            } catch (InterruptedException e) {
                logger.error("Error while waiting for CaptureTask to cancel itself {}. Details: {}", e.getLocalizedMessage(),
                        Arrays.toString(e.getStackTrace()));
            }

            // Schedule New PacketCaptureTimerTask based on configs received from Server..
            timer = null; // does explicitly nullifying ref here helps in GC?
            timer = new Timer();
            // Create PacketCaptureTimerTask first.
            // Here we pass Spring Bean RpiService reference to Non-wired Capture Task.
            captureTimerTask = new PacketCaptureTimerTask(this.rpiService);
            timer.scheduleAtFixedRate(captureTimerTask, rpiConfig.getCaptureTaskStartDelay(),
                    rpiConfig.getCaptureTaskRepeatInterval());
            logger.debug("Started Capture Task with delay={} and repeatInterval={}",
                    rpiConfig.getCaptureTaskStartDelay(), rpiConfig.getCaptureTaskRepeatInterval());
        } catch (Exception ex) {
            logger.error("Error while running Sync Job. Ex: {}", ex.getLocalizedMessage());
            logger.debug(Arrays.toString(ex.getStackTrace()));
        } finally {
            logger.debug("ConfigSyncJob(run={}) took time {} ms.", jobRunCount.get(), (System.currentTimeMillis()-startTimeInMs));
        }
        
    }
}


