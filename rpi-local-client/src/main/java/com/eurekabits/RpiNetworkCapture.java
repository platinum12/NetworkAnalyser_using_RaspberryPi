package com.eurekabits;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

/**
 * DTO to hold network data captured by Rpi.
 * This DTO will be used to send data to remote Server.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class RpiNetworkCapture {
    
    String data;

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    @Override
    public String toString() {
        return "RpiNetworkCapture{" + "data=" + data + '}';
    }
}
