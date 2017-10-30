package com.eurekabits;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.Generated;

import com.fasterxml.jackson.annotation.JsonAnyGetter;
import com.fasterxml.jackson.annotation.JsonAnySetter;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonPropertyOrder;

@JsonInclude(JsonInclude.Include.NON_NULL)
@Generated("org.jsonschema2pojo")
@JsonPropertyOrder({ "packets" })
public class NetworkCaptureData {

	@JsonProperty("devId")
	private String devId;
	
	@JsonProperty("packets")
	private List<Packet> packets = new ArrayList<Packet>();
	@JsonIgnore
	private Map<String, Object> additionalProperties = new HashMap<String, Object>();
	
	@JsonProperty("devId")
	public String getDevId() {
		return devId;
	}
	@JsonProperty("devId")
	public void setDevId(String devId) {
		this.devId = devId;
	}

	/**
	 * 
	 * @return The packets
	 */
	@JsonProperty("packets")
	public List<Packet> getPackets() {
		return packets;
	}

	/**
	 * 
	 * @param packets
	 *            The packets
	 */
	@JsonProperty("packets")
	public void setPackets(List<Packet> packets) {
		this.packets = packets;
	}

	@JsonAnyGetter
	public Map<String, Object> getAdditionalProperties() {
		return this.additionalProperties;
	}

	@JsonAnySetter
	public void setAdditionalProperty(String name, Object value) {
		this.additionalProperties.put(name, value);
	}
	@Override
	public String toString() {
		return "NetworkCaptureData [devId=" + devId + ", packets=" + packets
				+ "]";
	}
}
