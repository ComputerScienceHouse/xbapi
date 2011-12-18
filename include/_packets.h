#ifndef ___PACKETS_H__
#define ___PACKETS_H__

#include "_xbapi.h"

#define _XBAPI_FRAME_AT_CMD      0x08
#define _XBAPI_FRAME_AT_QUEUED   0x09
#define _XBAPI_FRAME_TX_REQ      0x10
#define _XBAPI_FRAME_XADDR_CMD   0x11
#define _XBAPI_FRAME_RMT_CMD_REQ 0x17
#define _XBAPI_FRAME_CRT_SRC_RT  0x21
#define _XBAPI_FRAME_AT_CMD_RES  0x88
#define _XBAPI_FRAME_MODEM_STAT  0x8A
#define _XBAPI_FRAME_TX_STAT     0x8B
#define _XBAPI_FRAME_RX_PACKET   0x90
#define _XBAPI_FRAME_XRX_INDIC   0x91
#define _XBAPI_FRAME_IO_DATA_RX  0x92
#define _XBAPI_FRAME_SENSOR_READ 0x94
#define _XBAPI_FRAME_NODE_ID     0x95
#define _XBAPI_FRAME_RMT_CMD_RES 0x97
#define _XBAPI_FRAME_UPDATE_STAT 0xA0
#define _XBAPI_FRAME_RT_RC_INDIC 0xA1
#define _XBAPI_FRAME_M_1_RT_REQ  0xA3

#define _XBAPI_OP_STATUS_OK            0x00
#define _XBAPI_OP_STATUS_ERROR         0x01
#define _XBAPI_OP_STATUS_INVALID_CMD   0x02
#define _XBAPI_OP_STATUS_INVALID_PARAM 0x03
#define _XBAPI_OP_STATUS_TX_FAILURE    0x04

#define _XBAPI_MODEM_HARDWARE_RESET               0x00
#define _XBAPI_MODEM_WDT_RESET                    0x01
#define _XBAPI_MODEM_JOINED_NETWORK               0x02
#define _XBAPI_MODEM_DISASSOCIATED                0x03
#define _XBAPI_MODEM_COORDINATOR_STARTED          0x06
#define _XBAPI_MODEM_SECURITY_KEY_UPDATED         0x07
#define _XBAPI_MODEM_OVERVOLTAGE                  0x0D
#define _XBAPI_MODEM_CONFIG_CHANGED_WHILE_JOINING 0x11
#define _XBAPI_MODEM_STACK_ERROR                  0x6F

#define _XBAPI_RX_OPT_ACKNOWLEDGE 0x01
#define _XBAPI_RX_OPT_BROADCAST   0x02

#define _XBAPI_DEVICE_TYPE_COORDINATOR 0x00
#define _XBAPI_DEVICE_TYPE_ROUTER      0x01
#define _XBAPI_DEVICE_TYPE_END_DEVICE  0x02

#define _XBAPI_SOURCE_EVENT_PUSHBUTTON  0x01
#define _XBAPI_SOURCE_EVENT_JOINED      0x02
#define _XBAPI_SOURCE_EVENT_POWER_CYCLE 0x03

#define _XBAPI_CONTROL_FRAME_DELIM 0x7E
#define _XBAPI_CONTROL_ESCAPE      0x7D
#define _XBAPI_CONTROL_XON         0x11
#define _XBAPI_CONTROL_XOFF        0x13

#define _XBAPI_DELIVERY_STATUS_SUCCESS           0x00
#define _XBAPI_DELIVERY_STATUS_MAC_ACK_FAIL      0x01
#define _XBAPI_DELIVERY_STATUS_CCA_FAIL          0x02
#define _XBAPI_DELIVERY_STATUS_INVALID_DEST      0x15
#define _XBAPI_DELIVERY_STATUS_NET_ACK_FAIL      0x21
#define _XBAPI_DELIVERY_STATUS_NOT_JOINED        0x22
#define _XBAPI_DELIVERY_STATUS_SELF_ADDRESSED    0x23
#define _XBAPI_DELIVERY_STATUS_ADDRESS_NOT_FOUND 0x24
#define _XBAPI_DELIVERY_STATUS_ROUTE_NOT_FOUND   0x25
#define _XBAPI_DELIVERY_STATUS_NO_RELAY          0x26
#define _XBAPI_DELIVERY_STATUS_INVALID_BIND      0x2B
#define _XBAPI_DELIVERY_STATUS_RESOURCE_1        0x2C
#define _XBAPI_DELIVERY_STATUS_BROADCAST_APS     0x2D
#define _XBAPI_DELIVERY_STATUS_UNICAST_APS       0x2E
#define _XBAPI_DELIVERY_STATUS_RESOURCE_2        0x32
#define _XBAPI_DELIVERY_STATUS_TOO_LARGE         0x74
#define _XBAPI_DELIVERY_STATUS_INDIRECT          0x75

#define _XBAPI_DISCOVERY_STATUS_NONE    0x00
#define _XBAPI_DISCOVERY_STATUS_ADDRESS 0x01
#define _XBAPI_DISCOVERY_STATUS_ROUTE   0x02
#define _XBAPI_DISCOVERY_STATUS_BOTH    0x03
#define _XBAPI_DISCOVERY_STATUS_TIMEOUT 0x40

#define _XBAPI_RX_OPTIONS_ACK        0x01
#define _XBAPI_RX_OPTIONS_BROADCAST  0x02
#define _XBAPI_RX_OPTIONS_ENCRYPTED  0x20
#define _XBAPI_RX_OPTIONS_END_DEVICE 0x40


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



static const unsigned int TX_STAT_LEN = 7;

uint8_t frame_id_from_tx_stat(uint8_t *packet);
uint16_t delivery_network_address_from_tx_stat(uint8_t *packet);
uint8_t retry_count_from_tx_stat(uint8_t *packet);
xbapi_delivery_status_e delivery_status_from_tx_stat(uint8_t *packet);
xbapi_discovery_status_e discovery_status_from_tx_stat(uint8_t *packet);



static const unsigned int RX_PACKET_MIN_LEN = 13;

uint64_t source_address_from_rx_packet(uint8_t *packet);
uint16_t source_network_address_from_rx_packet(uint8_t *packet);
xbapi_rx_options_e options_from_rx_packet(uint8_t *packet);
uint8_t *data_from_rx_packet(uint8_t *packet, size_t *len);

#endif /* ___PACKETS_H__ */
