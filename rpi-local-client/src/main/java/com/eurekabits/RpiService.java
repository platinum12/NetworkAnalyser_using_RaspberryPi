package com.eurekabits;

import org.springframework.http.ResponseEntity;

/**
 * Service which implements core functionality for RPi.
 */
public interface RpiService {
    
    public RpiConfig getRpiConfigFromServer();
    public ResponseEntity<String> postCapturedDataToServer(NetworkCaptureData networkCaptureData);
    public NetworkCaptureData getNetworkInfoFromClient();
}
