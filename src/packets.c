#include <arpa/inet.h>
#include <talloc.h>
#include "_packets.h"

xbapi_frame_type_e frame_type_from_packet(uint8_t *packet) {
	switch (packet[0]) {
		case  _XBAPI_FRAME_AT_CMD:
		return XBAPI_FRAME_AT_CMD;
		case  _XBAPI_FRAME_AT_QUEUED:
		return XBAPI_FRAME_AT_QUEUED;
		case  _XBAPI_FRAME_TX_REQ:
		return XBAPI_FRAME_TX_REQ;
		case  _XBAPI_FRAME_XADDR_CMD:
		return XBAPI_FRAME_XADDR_CMD;
		case  _XBAPI_FRAME_RMT_CMD_REQ:
		return XBAPI_FRAME_RMT_CMD_REQ;
		case  _XBAPI_FRAME_CRT_SRC_RT:
		return XBAPI_FRAME_CRT_SRC_RT;
		case  _XBAPI_FRAME_AT_CMD_RES:
		return XBAPI_FRAME_AT_CMD_RES;
		case  _XBAPI_FRAME_MODEM_STAT:
		return XBAPI_FRAME_MODEM_STAT;
		case  _XBAPI_FRAME_TX_STAT:
		return XBAPI_FRAME_TX_STAT;
		case  _XBAPI_FRAME_RX_PACKET:
		return XBAPI_FRAME_RX_PACKET;
		case  _XBAPI_FRAME_XRX_INDIC:
		return XBAPI_FRAME_XRX_INDIC;
		case  _XBAPI_FRAME_IO_DATA_RX:
		return XBAPI_FRAME_IO_DATA_RX;
		case  _XBAPI_FRAME_SENSOR_READ:
		return XBAPI_FRAME_SENSOR_READ;
		case  _XBAPI_FRAME_NODE_ID:
		return XBAPI_FRAME_NODE_ID;
		case  _XBAPI_FRAME_RMT_CMD_RES:
		return XBAPI_FRAME_RMT_CMD_RES;
		case  _XBAPI_FRAME_UPDATE_STAT:
		return XBAPI_FRAME_UPDATE_STAT;
		case  _XBAPI_FRAME_RT_RC_INDIC:
		return XBAPI_FRAME_RT_RC_INDIC;
		case  _XBAPI_FRAME_M_1_RT_REQ:
		return XBAPI_FRAME_M_1_RT_REQ;
		default:
		return XBAPI_FRAME_INVALID;
	}


	return (xbapi_frame_type_e)packet[0];
}

// AT Command Response
uint8_t frame_id_from_at_cmd_res(uint8_t *packet) {
	return packet[1];
}

char *at_command_from_at_cmd_res(uint8_t *packet) {
	return (char *)(packet + 2);
}

xbapi_at_cmd_status_e command_status_from_at_cmd_res(uint8_t *packet) {
	switch (packet[4]) {
		case  _XBAPI_AT_CMD_STATUS_OK:
		return XBAPI_AT_CMD_STATUS_OK;
		case  _XBAPI_AT_CMD_STATUS_ERROR:
		return XBAPI_AT_CMD_STATUS_ERROR;
		case  _XBAPI_AT_CMD_STATUS_INVALID_CMD:
		return XBAPI_AT_CMD_STATUS_INVALID_CMD;
		case  _XBAPI_AT_CMD_STATUS_INVALID_PARAM:
		return XBAPI_AT_CMD_STATUS_INVALID_PARAM;
		case  _XBAPI_AT_CMD_STATUS_TX_FAILURE:
		return XBAPI_AT_CMD_STATUS_TX_FAILURE;
		default:
		return XBAPI_AT_CMD_STATUS_INVALID;
	}
}

size_t command_data_len_from_at_cmd_res(uint8_t *packet) {
	size_t packet_len = talloc_array_length(packet);
	if (packet_len > AT_CMD_RES_MIN_LEN)
		return packet_len - AT_CMD_RES_MIN_LEN;
	else
		return 0;
}

uint8_t *command_data_from_at_cmd_res(uint8_t *packet) {
	return packet + 5;
}


// Modem Status
xbapi_modem_status_e status_from_modem_stat(uint8_t *packet) {
	switch (packet[1]) {
		case  _XBAPI_MODEM_HARDWARE_RESET:
		return XBAPI_MODEM_HARDWARE_RESET;
		case  _XBAPI_MODEM_WDT_RESET:
		return XBAPI_MODEM_WDT_RESET;
		case  _XBAPI_MODEM_JOINED_NETWORK:
		return XBAPI_MODEM_JOINED_NETWORK;
		case  _XBAPI_MODEM_DISASSOCIATED:
		return XBAPI_MODEM_DISASSOCIATED;
		case  _XBAPI_MODEM_COORDINATOR_STARTED:
		return XBAPI_MODEM_COORDINATOR_STARTED;
		case  _XBAPI_MODEM_SECURITY_KEY_UPDATED:
		return XBAPI_MODEM_SECURITY_KEY_UPDATED;
		case  _XBAPI_MODEM_OVERVOLTAGE:
		return XBAPI_MODEM_OVERVOLTAGE;
		case  _XBAPI_MODEM_CONFIG_CHANGED_WHILE_JOINING:
		return XBAPI_MODEM_CONFIG_CHANGED_WHILE_JOINING;
		case  _XBAPI_MODEM_STACK_ERROR:
		return XBAPI_MODEM_STACK_ERROR;
		default:
		return XBAPI_MODEM_STATUS_UNKNOWN;
	}
}


// Node Identification
uint64_t source_address_from_node_id(uint8_t *packet) {
	return ntohll(*(uint64_t *)(packet + 1));
}

uint16_t source_network_address_from_node_id(uint8_t *packet) {
	return ntohs(*(uint16_t *)(packet + 9));
}

uint64_t remote_address_from_node_id(uint8_t *packet) {
	return ntohll(*(uint64_t *)(packet + 14));
}

uint16_t remote_network_address_from_node_id(uint8_t *packet) {
	return ntohs(*(uint16_t *)(packet + 12));
}

xbapi_rx_opt_e receive_options_from_node_id(uint8_t *packet) {
	switch(packet[11]) {
		case  _XBAPI_RX_OPT_ACKNOWLEDGE:
		return XBAPI_RX_OPT_ACKNOWLEDGE;
		case  _XBAPI_RX_OPT_BROADCAST:
		return XBAPI_RX_OPT_BROADCAST;
		default:
		return XBAPI_RX_OPT_INVALID;
	}
}

size_t ni_string_len_from_node_id(uint8_t *packet) {
	size_t packet_len = talloc_array_length(packet);
	if (packet_len > NODE_ID_MIN_LEN)
		return packet_len - NODE_ID_MIN_LEN;
	else
		return 0;
}

char *ni_string_from_node_id(uint8_t *packet) {
	return (char *)(packet + 22);
}

uint16_t parent_network_address_from_node_id(uint8_t *packet) {
	return ntohs(*(uint16_t *)(packet + ni_string_len_from_node_id(packet) + 23));
}

xbapi_device_type_e device_type_from_node_id(uint8_t *packet) {
	uint8_t b = packet[ni_string_len_from_node_id(packet) + 25];
	switch (b) {
		case  _XBAPI_DEVICE_TYPE_COORDINATOR:
		return XBAPI_DEVICE_TYPE_COORDINATOR;
		case  _XBAPI_DEVICE_TYPE_ROUTER:
		return XBAPI_DEVICE_TYPE_ROUTER;
		case  _XBAPI_DEVICE_TYPE_END_DEVICE:
		return XBAPI_DEVICE_TYPE_END_DEVICE;
		default:
		return XBAPI_DEVICE_TYPE_INVALID;
	}
}

xbapi_source_event_e source_event_from_node_id(uint8_t *packet) {
	uint8_t b = packet[ni_string_len_from_node_id(packet) + 26];
	switch (b) {
		case  _XBAPI_SOURCE_EVENT_PUSHBUTTON:
		return XBAPI_SOURCE_EVENT_PUSHBUTTON;
		case  _XBAPI_SOURCE_EVENT_JOINED:
		return XBAPI_SOURCE_EVENT_JOINED;
		case  _XBAPI_SOURCE_EVENT_POWER_CYCLE:
		return XBAPI_SOURCE_EVENT_POWER_CYCLE;
		default:
		return XBAPI_SOURCE_EVENT_INVALID;
	}
}

uint16_t profile_id_from_node_id(uint8_t *packet) {
	return ntohs(*(uint16_t *)(packet + ni_string_len_from_node_id(packet) + 27));
}

uint16_t manufacturer_id_from_node_id(uint8_t *packet) {
	return ntohs(*(uint16_t *)(packet + ni_string_len_from_node_id(packet) + 29));
}


// TX Status
uint8_t frame_id_from_tx_stat(uint8_t *packet) {
	return packet[1];
}

uint16_t delivery_network_address_from_tx_stat(uint8_t *packet) {
	return ntohs(*(uint16_t *)(packet + 2));
}

uint8_t retry_count_from_tx_stat(uint8_t *packet) {
	return packet[4];
}

xbapi_delivery_status_e delivery_status_from_tx_stat(uint8_t *packet) {
	switch (packet[5]) {
		case  _XBAPI_DELIVERY_STATUS_SUCCESS:
		return XBAPI_DELIVERY_STATUS_SUCCESS;
		case  _XBAPI_DELIVERY_STATUS_MAC_ACK_FAIL:
		return XBAPI_DELIVERY_STATUS_MAC_ACK_FAIL;
		case  _XBAPI_DELIVERY_STATUS_CCA_FAIL:
		return XBAPI_DELIVERY_STATUS_CCA_FAIL;
		case  _XBAPI_DELIVERY_STATUS_INVALID_DEST:
		return XBAPI_DELIVERY_STATUS_INVALID_DEST;
		case  _XBAPI_DELIVERY_STATUS_NET_ACK_FAIL:
		return XBAPI_DELIVERY_STATUS_NET_ACK_FAIL;
		case  _XBAPI_DELIVERY_STATUS_NOT_JOINED:
		return XBAPI_DELIVERY_STATUS_NOT_JOINED;
		case  _XBAPI_DELIVERY_STATUS_SELF_ADDRESSED:
		return XBAPI_DELIVERY_STATUS_SELF_ADDRESSED;
		case  _XBAPI_DELIVERY_STATUS_ADDRESS_NOT_FOUND:
		return XBAPI_DELIVERY_STATUS_ADDRESS_NOT_FOUND;
		case  _XBAPI_DELIVERY_STATUS_ROUTE_NOT_FOUND:
		return XBAPI_DELIVERY_STATUS_ROUTE_NOT_FOUND;
		case  _XBAPI_DELIVERY_STATUS_NO_RELAY:
		return XBAPI_DELIVERY_STATUS_NO_RELAY;
		case  _XBAPI_DELIVERY_STATUS_INVALID_BIND:
		return XBAPI_DELIVERY_STATUS_INVALID_BIND;
		case  _XBAPI_DELIVERY_STATUS_RESOURCE_1:
		return XBAPI_DELIVERY_STATUS_RESOURCE_1;
		case  _XBAPI_DELIVERY_STATUS_BROADCAST_APS:
		return XBAPI_DELIVERY_STATUS_BROADCAST_APS;
		case  _XBAPI_DELIVERY_STATUS_UNICAST_APS:
		return XBAPI_DELIVERY_STATUS_UNICAST_APS;
		case  _XBAPI_DELIVERY_STATUS_RESOURCE_2:
		return XBAPI_DELIVERY_STATUS_RESOURCE_2;
		case  _XBAPI_DELIVERY_STATUS_TOO_LARGE:
		return XBAPI_DELIVERY_STATUS_TOO_LARGE;
		case  _XBAPI_DELIVERY_STATUS_INDIRECT:
		return XBAPI_DELIVERY_STATUS_INDIRECT;
		default:
		return XBAPI_DELIVERY_STATUS_INVALID;
	}
}

xbapi_discovery_status_e discovery_status_from_tx_stat(uint8_t *packet) {
	switch (packet[6]) {
		case  _XBAPI_DISCOVERY_STATUS_NONE:
		return XBAPI_DISCOVERY_STATUS_NONE;
		case  _XBAPI_DISCOVERY_STATUS_ADDRESS:
		return XBAPI_DISCOVERY_STATUS_ADDRESS;
		case  _XBAPI_DISCOVERY_STATUS_ROUTE:
		return XBAPI_DISCOVERY_STATUS_ROUTE;
		case  _XBAPI_DISCOVERY_STATUS_BOTH:
		return XBAPI_DISCOVERY_STATUS_BOTH;
		case  _XBAPI_DISCOVERY_STATUS_TIMEOUT:
		return XBAPI_DISCOVERY_STATUS_TIMEOUT;
		default:
		return XBAPI_DISCOVERY_STATUS_INVALID;
	}
}


// RX Packet
uint64_t source_address_from_rx_packet(uint8_t *packet) {
	return ntohll(*(uint64_t *)(packet + 1));
}

uint16_t source_network_address_from_rx_packet(uint8_t *packet) {
	return ntohl(*(uint16_t *)(packet + 9));
}

xbapi_rx_options_e options_from_rx_packet(uint8_t *packet) {
	switch (packet[11]) {
		case  _XBAPI_RX_OPTIONS_ACK:
		return XBAPI_RX_OPTIONS_ACK;
		case  _XBAPI_RX_OPTIONS_BROADCAST:
		return XBAPI_RX_OPTIONS_BROADCAST;
		case  _XBAPI_RX_OPTIONS_ENCRYPTED:
		return XBAPI_RX_OPTIONS_ENCRYPTED;
		case  _XBAPI_RX_OPTIONS_END_DEVICE:
		return XBAPI_RX_OPTIONS_END_DEVICE;
		default:
		return XBAPI_RX_OPTIONS_INVALID;
	}
}

uint8_t *data_from_rx_packet(uint8_t *packet, size_t *len) {
	*len = talloc_array_length(packet) - RX_PACKET_MIN_LEN + 1;
	return packet + 12;
}

