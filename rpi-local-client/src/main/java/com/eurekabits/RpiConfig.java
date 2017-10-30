package com.eurekabits;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

/**
 * DTO to hold Rpi configuration received from Server.
 *
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class RpiConfig {
    long captureTaskStartDelay; // in milliseconds.
    long captureTaskRepeatInterval; // in milliseconds.
    long numberOfPacketsToCapture;

    public long getCaptureTaskStartDelay() {
        return captureTaskStartDelay;
    }

    public void setCaptureTaskStartDelay(long captureTaskStartDelay) {
        this.captureTaskStartDelay = captureTaskStartDelay;
    }

    public long getCaptureTaskRepeatInterval() {
        return captureTaskRepeatInterval;
    }

    public void setCaptureTaskRepeatInterval(long captureTaskRepeatInterval) {
        this.captureTaskRepeatInterval = captureTaskRepeatInterval;
    }

    public long getNumberOfPacketsToCapture() {
		return numberOfPacketsToCapture;
	}

	public void setNumberOfPacketsToCapture(long numberOfPacketsToCapture) {
		this.numberOfPacketsToCapture = numberOfPacketsToCapture;
	}
	
	

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime
				* result
				+ (int) (captureTaskRepeatInterval ^ (captureTaskRepeatInterval >>> 32));
		result = prime
				* result
				+ (int) (captureTaskStartDelay ^ (captureTaskStartDelay >>> 32));
		result = prime
				* result
				+ (int) (numberOfPacketsToCapture ^ (numberOfPacketsToCapture >>> 32));
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof RpiConfig)) {
			return false;
		}
		RpiConfig other = (RpiConfig) obj;
		if (captureTaskRepeatInterval != other.captureTaskRepeatInterval) {
			return false;
		}
		if (captureTaskStartDelay != other.captureTaskStartDelay) {
			return false;
		}
		if (numberOfPacketsToCapture != other.numberOfPacketsToCapture) {
			return false;
		}
		return true;
	}

	@Override
	public String toString() {
		return "RpiConfig [captureTaskStartDelay=" + captureTaskStartDelay
				+ ", captureTaskRepeatInterval=" + captureTaskRepeatInterval
				+ ", numberOfPacketsToCapture=" + numberOfPacketsToCapture
				+ "]";
	}
}
