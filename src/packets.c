#include <arpa/inet.h>
#include <talloc.h>
#include "_packets.h"

static inline uint64_t ntohll(uint64_t data) {
	return ((uint64_t)ntohl((uint32_t)data) << 32 | ntohl((uint32_t)(data >> 32)));
}

static inline uint64_t htonll(uint64_t data) {
	return ntohll(data);
}

xbapi_frame_type_e frame_type_from_packet(uint8_t *packet) {
	switch (packet[0]) {
		case (uint8_t)_XBAPI_FRAME_AT_CMD:
			return XBAPI_FRAME_AT_CMD;

		case (uint8_t)_XBAPI_FRAME_AT_QUEUED:
			return XBAPI_FRAME_AT_QUEUED;

		case (uint8_t)_XBAPI_FRAME_TX_REQ:
			return XBAPI_FRAME_TX_REQ;

		case (uint8_t)_XBAPI_FRAME_XADDR_CMD:
			return XBAPI_FRAME_XADDR_CMD;

		case (uint8_t)_XBAPI_FRAME_RMT_CMD_REQ:
			return XBAPI_FRAME_RMT_CMD_REQ;

		case (uint8_t)_XBAPI_FRAME_CRT_SRC_RT:
			return XBAPI_FRAME_CRT_SRC_RT;

		case (uint8_t)_XBAPI_FRAME_AT_CMD_RES:
			return XBAPI_FRAME_AT_CMD_RES;

		case (uint8_t)_XBAPI_FRAME_MODEM_STAT:
			return XBAPI_FRAME_MODEM_STAT;

		case (uint8_t)_XBAPI_FRAME_TX_STAT:
			return XBAPI_FRAME_TX_STAT;

		case (uint8_t)_XBAPI_FRAME_RX_PACKET:
			return XBAPI_FRAME_RX_PACKET;

		case (uint8_t)_XBAPI_FRAME_XRX_INDIC:
			return XBAPI_FRAME_XRX_INDIC;

		case (uint8_t)_XBAPI_FRAME_IO_DATA_RX:
			return XBAPI_FRAME_IO_DATA_RX;

		case (uint8_t)_XBAPI_FRAME_SENSOR_READ:
			return XBAPI_FRAME_SENSOR_READ;

		case (uint8_t)_XBAPI_FRAME_NODE_ID:
			return XBAPI_FRAME_NODE_ID;

		case (uint8_t)_XBAPI_FRAME_RMT_CMD_RES:
			return XBAPI_FRAME_RMT_CMD_RES;

		case (uint8_t)_XBAPI_FRAME_UPDATE_STAT:
			return XBAPI_FRAME_UPDATE_STAT;

		case (uint8_t)_XBAPI_FRAME_RT_RC_INDIC:
			return XBAPI_FRAME_RT_RC_INDIC;

		case (uint8_t)_XBAPI_FRAME_M_1_RT_REQ:
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

xbapi_op_status_e command_status_from_at_cmd_res(uint8_t *packet) {
	switch (packet[4]) {
		case (uint8_t)_XBAPI_OP_STATUS_OK:
			return XBAPI_OP_STATUS_OK;

		case (uint8_t)_XBAPI_OP_STATUS_ERROR:
			return XBAPI_OP_STATUS_ERROR;

		case (uint8_t)_XBAPI_OP_STATUS_INVALID_CMD:
			return XBAPI_OP_STATUS_INVALID_CMD;

		case (uint8_t)_XBAPI_OP_STATUS_INVALID_PARAM:
			return XBAPI_OP_STATUS_INVALID_PARAM;

		case (uint8_t)_XBAPI_OP_STATUS_TX_FAILURE:
			return XBAPI_OP_STATUS_TX_FAILURE;

		default:
			return XBAPI_OP_STATUS_PENDING;
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
		case (uint8_t)_XBAPI_MODEM_HARDWARE_RESET:
			return XBAPI_MODEM_HARDWARE_RESET;

		case (uint8_t)_XBAPI_MODEM_WDT_RESET:
			return XBAPI_MODEM_WDT_RESET;

		case (uint8_t)_XBAPI_MODEM_JOINED_NETWORK:
			return XBAPI_MODEM_JOINED_NETWORK;

		case (uint8_t)_XBAPI_MODEM_DISASSOCIATED:
			return XBAPI_MODEM_DISASSOCIATED;

		case (uint8_t)_XBAPI_MODEM_COORDINATOR_STARTED:
			return XBAPI_MODEM_COORDINATOR_STARTED;

		case (uint8_t)_XBAPI_MODEM_SECURITY_KEY_UPDATED:
			return XBAPI_MODEM_SECURITY_KEY_UPDATED;

		case (uint8_t)_XBAPI_MODEM_OVERVOLTAGE:
			return XBAPI_MODEM_OVERVOLTAGE;

		case (uint8_t)_XBAPI_MODEM_CONFIG_CHANGED_WHILE_JOINING:
			return XBAPI_MODEM_CONFIG_CHANGED_WHILE_JOINING;

		case (uint8_t)_XBAPI_MODEM_STACK_ERROR:
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
		case XBAPI_RX_OPT_ACKNOWLEDGE:
			return (uint8_t)_XBAPI_RX_OPT_ACKNOWLEDGE;

		case XBAPI_RX_OPT_BROADCAST:
			return (uint8_t)_XBAPI_RX_OPT_BROADCAST;

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
		case (uint8_t)_XBAPI_DEVICE_TYPE_COORDINATOR:
			return XBAPI_DEVICE_TYPE_COORDINATOR;

		case (uint8_t)_XBAPI_DEVICE_TYPE_ROUTER:
			return XBAPI_DEVICE_TYPE_ROUTER;

		case (uint8_t)_XBAPI_DEVICE_TYPE_END_DEVICE:
			return XBAPI_DEVICE_TYPE_END_DEVICE;

		default:
			return XBAPI_DEVICE_TYPE_INVALID;
	}
}

xbapi_source_event_e source_event_from_node_id(uint8_t *packet) {
	uint8_t b = packet[ni_string_len_from_node_id(packet) + 26];
	switch (b) {
		case (uint8_t)_XBAPI_SOURCE_EVENT_PUSHBUTTON:
			return XBAPI_SOURCE_EVENT_PUSHBUTTON;

		case (uint8_t)_XBAPI_SOURCE_EVENT_JOINED:
			return XBAPI_SOURCE_EVENT_JOINED;

		case (uint8_t)_XBAPI_SOURCE_EVENT_POWER_CYCLE:
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

