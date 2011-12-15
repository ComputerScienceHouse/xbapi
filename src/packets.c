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
		case XBAPI_OP_STATUS_OK:
		case XBAPI_OP_STATUS_ERROR:
		case XBAPI_OP_STATUS_INVALID_CMD:
		case XBAPI_OP_STATUS_INVALID_PARAM:
		case XBAPI_OP_STATUS_TX_FAILURE:
			return (xbapi_op_status_e)packet[4];
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
		case XBAPI_MODEM_HARDWARE_RESET:
		case XBAPI_MODEM_WDT_RESET:
		case XBAPI_MODEM_JOINED_NETWORK:
		case XBAPI_MODEM_DISASSOCIATED:
		case XBAPI_MODEM_COORDINATOR_STARTED:
		case XBAPI_MODEM_SECURITY_KEY_UPDATED:
		case XBAPI_MODEM_OVERVOLTAGE:
		case XBAPI_MODEM_CONFIG_CHANGED_WHILE_JOINING:
		case XBAPI_MODEM_STACK_ERROR:
			return (xbapi_modem_status_e)packet[1];
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
		case XBAPI_RX_OPT_BROADCAST:
			return (xbapi_rx_opt_e)packet[11];
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
		case XBAPI_DEVICE_TYPE_COORDINATOR:
		case XBAPI_DEVICE_TYPE_ROUTER:
		case XBAPI_DEVICE_TYPE_END_DEVICE:
			return (xbapi_device_type_e)b;
		default:
			return XBAPI_DEVICE_TYPE_INVALID;
	}
}

xbapi_source_event_e source_event_from_node_id(uint8_t *packet) {
	uint8_t b = packet[ni_string_len_from_node_id(packet) + 26];
	switch (b) {
		case XBAPI_SOURCE_EVENT_PUSHBUTTON:
		case XBAPI_SOURCE_EVENT_JOINED:
		case XBAPI_SOURCE_EVENT_POWER_CYCLE:
			return (xbapi_source_event_e)b;
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

