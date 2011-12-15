#ifndef ___PACKETS_H__
#define ___PACKETS_H__

#include "_xbapi.h"

typedef enum {
	XBAPI_FRAME_AT_CMD      = 0x08,
	XBAPI_FRAME_AT_QUEUED   = 0x09,
	XBAPI_FRAME_TX_REQ      = 0x10,
	XBAPI_FRAME_XADDR_CMD   = 0x11,
	XBAPI_FRAME_RMT_CMD_REQ = 0x17,
	XBAPI_FRAME_CRT_SRC_RT  = 0x21,
	XBAPI_FRAME_AT_CMD_RES  = 0x88,
	XBAPI_FRAME_MODEM_STAT  = 0x8A,
	XBAPI_FRAME_TX_STAT     = 0x8B,
	XBAPI_FRAME_RX_PACKET   = 0x90,
	XBAPI_FRAME_XRX_INDIC   = 0x91,
	XBAPI_FRAME_IO_DATA_RX  = 0x92,
	XBAPI_FRAME_SENSOR_READ = 0x94,
	XBAPI_FRAME_NODE_ID     = 0x95,
	XBAPI_FRAME_RMT_CMD_RES = 0x97,
	XBAPI_FRAME_UPDATE_STAT = 0xA0,
	XBAPI_FRAME_RT_RC_INDIC = 0xA1,
	XBAPI_FRAME_M_1_RT_REQ  = 0xA3
} xbapi_frame_type_e;


xbapi_frame_type_e frame_type_from_packet(uint8_t *packet);



static const unsigned int AT_CMD_RES_MIN_LEN = 5;

uint8_t frame_id_from_at_cmd_res(uint8_t *packet);
char *at_command_from_at_cmd_res(uint8_t *packet);
xbapi_op_status_e command_status_from_at_cmd_res(uint8_t *packet);
size_t command_data_len_from_at_cmd_res(uint8_t *packet);
uint8_t *command_data_from_at_cmd_res(uint8_t *packet);



static const unsigned int MODEM_STAT_LEN = 2;

xbapi_modem_status_e status_from_modem_stat(uint8_t *packet);



static const unsigned int NODE_ID_MIN_LEN = 31;

uint64_t source_address_from_node_id(uint8_t *packet);
uint16_t source_network_address_from_node_id(uint8_t *packet);
uint64_t remote_address_from_node_id(uint8_t *packet);
uint16_t remote_network_address_from_node_id(uint8_t *packet);
xbapi_rx_opt_e receive_options_from_node_id(uint8_t *packet);
size_t ni_string_len_from_node_id(uint8_t *packet);
char *ni_string_from_node_id(uint8_t *packet);
uint16_t parent_network_address_from_node_id(uint8_t *packet);
xbapi_device_type_e device_type_from_node_id(uint8_t *packet);
xbapi_source_event_e source_event_from_node_id(uint8_t *packet);
uint16_t profile_id_from_node_id(uint8_t *packet);
uint16_t manufacturer_id_from_node_id(uint8_t *packet);

#endif /* ___PACKETS_H__ */
