#ifdef ___XBAPI_H__
#error Double include of private header _xbapi.h
#else /* ___XBAPI_H__ */
#define ___XBAPI_H__


#define XBAPI_FRAME_DELIM   0x7E
#define XBAPI_ESCAPE        0x7D
#define XBAPI_XON           0x11
#define XBAPI_XOFF          0x13

#define XBAPI_FRAME_AT_CMD      0x08
#define XBAPI_FRAME_AT_QUEUED   0x09
#define XBAPI_FRAME_TX_REQ      0x10
#define XBAPI_FRAME_XADDR_CMD   0x11
#define XBAPI_FRAME_RMT_CMD_REQ 0x17
#define XBAPI_FRAME_CRT_SRC_RT  0x21
#define XBAPI_FRAME_AT_CMD_RES  0x88
#define XBAPI_FRAME_MODEM_STAT  0x8A
#define XBAPI_FRAME_TX_STAT     0x8B
#define XBAPI_FRAME_RX_PACKET   0x90
#define XBAPI_FRAME_XRX_INDIC   0x91
#define XBAPI_FRAME_IO_DATA_RX  0x92
#define XBAPI_FRAME_SENSOR_READ 0x94
#define XBAPI_FRAME_NODE_ID     0x95
#define XBAPI_FRAME_RMT_CMD_RES 0x97
#define XBAPI_FRAME_UPDATE_STAT 0xA0
#define XBAPI_FRAME_RT_RC_INDIC 0xA1
#define XBAPI_FRAME_M_1_RT_REQ  0xA3

static xbapi_rc_t xbapi_escape( uint8_t **buf );
static xbapi_rc_t xbapi_unescape( uint8_t **buf );

const char *at_cmd_str(xbapi_at_e);

typedef struct {
	int fd;
	uint8_t frame_id;
	uint8_t *buffer;
	bool rollover_escape;
} xbapi_conn_t;


#endif /* ___XBAPI_H__ */
