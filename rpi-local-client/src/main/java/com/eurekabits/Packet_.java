package com.eurekabits;

import java.util.HashMap;
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
@JsonPropertyOrder({ "datarate", "antenna", "channel", "power", "ssid", "src",
		"dst", "bssid" })
public class Packet_ {

	@JsonProperty("datarate")
	private String datarate;
	@JsonProperty("antenna")
	private String antenna;
	@JsonProperty("channel")
	private String channel;
	@JsonProperty("power")
	private String power;
	@JsonProperty("ssid")
	private String ssid;
	@JsonProperty("src")
	private String src;
	@JsonProperty("dst")
	private String dst;
	@JsonProperty("bssid")
	private String bssid;
	@JsonIgnore
	private Map<String, Object> additionalProperties = new HashMap<String, Object>();

	/**
	 * 
	 * @return The datarate
	 */
	@JsonProperty("datarate")
	public String getDatarate() {
		return datarate;
	}

	/**
	 * 
	 * @param datarate
	 *            The datarate
	 */
	@JsonProperty("datarate")
	public void setDatarate(String datarate) {
		this.datarate = datarate;
	}

	/**
	 * 
	 * @return The antenna
	 */
	@JsonProperty("antenna")
	public String getAntenna() {
		return antenna;
	}

	/**
	 * 
	 * @param antenna
	 *            The antenna
	 */
	@JsonProperty("antenna")
	public void setAntenna(String antenna) {
		this.antenna = antenna;
	}

	/**
	 * 
	 * @return The channel
	 */
	@JsonProperty("channel")
	public String getChannel() {
		return channel;
	}

	/**
	 * 
	 * @param channel
	 *            The channel
	 */
	@JsonProperty("channel")
	public void setChannel(String channel) {
		this.channel = channel;
	}

	/**
	 * 
	 * @return The power
	 */
	@JsonProperty("power")
	public String getPower() {
		return power;
	}

	/**
	 * 
	 * @param power
	 *            The power
	 */
	@JsonProperty("power")
	public void setPower(String power) {
		this.power = power;
	}

	/**
	 * 
	 * @return The ssid
	 */
	@JsonProperty("ssid")
	public String getSsid() {
		return ssid;
	}

	/**
	 * 
	 * @param ssid
	 *            The ssid
	 */
	@JsonProperty("ssid")
	public void setSsid(String ssid) {
		this.ssid = ssid;
	}

	/**
	 * 
	 * @return The src
	 */
	@JsonProperty("src")
	public String getSrc() {
		return src;
	}

	/**
	 * 
	 * @param src
	 *            The src
	 */
	@JsonProperty("src")
	public void setSrc(String src) {
		this.src = src;
	}

	/**
	 * 
	 * @return The dst
	 */
	@JsonProperty("dst")
	public String getDst() {
		return dst;
	}

	/**
	 * 
	 * @param dst
	 *            The dst
	 */
	@JsonProperty("dst")
	public void setDst(String dst) {
		this.dst = dst;
	}

	/**
	 * 
	 * @return The bssid
	 */
	@JsonProperty("bssid")
	public String getBssid() {
		return bssid;
	}

	/**
	 * 
	 * @param bssid
	 *            The bssid
	 */
	@JsonProperty("bssid")
	public void setBssid(String bssid) {
		this.bssid = bssid;
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
		return "Packet_ [datarate=" + datarate + ", antenna=" + antenna
				+ ", channel=" + channel + ", power=" + power + ", ssid="
				+ ssid + ", src=" + src + ", dst=" + dst + ", bssid=" + bssid
				+ ", additionalProperties=" + additionalProperties + "]";
	}
}