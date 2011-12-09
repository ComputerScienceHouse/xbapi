#ifdef ___XBAPI_H__
#error Double include of private header _xbapi.h
#else /* ___XBAPI_H__ */
#define ___XBAPI_H__

static const int INITIAL_OP_SET_SIZE = 5;

typedef enum {
	XBAPI_CONTROL_FRAME_DELIM = 0x7E,
	XBAPI_CONTROL_ESCAPE      = 0x7D,
	XBAPI_CONTROL_XON         = 0x11,
	XBAPI_CONTROL_XOFF        = 0x13
} xbapi_control_chars_e;

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
} xbapi_frame_types_e;

struct _xbapi_op_t {
	uint8_t frame_id;
	xbapi_op_status_e status;
	uint8_t *data;
};

struct _xbapi_op_set_t {
	size_t success_count;
	size_t failure_count;
	size_t pending_count;
	xbapi_op_t **ops_success;
	xbapi_op_t **ops_failure;
	xbapi_op_t **ops_pending;
};

struct _xbapi_conn_t{
	int fd;
	uint8_t frame_id;
	uint8_t *buffer;
	char rollover_escape;
	xbapi_modem_status_e latest_modem_status;
};

static xbapi_rc_t xbapi_escape( uint8_t **buf );
static xbapi_rc_t xbapi_unescape( uint8_t **buf );

xbapi_rc_t create_operation(xbapi_op_set_t *set, xbapi_op_t **op);
xbapi_rc_t move_operation(xbapi_op_set_t *set, xbapi_op_t *op);

const char *at_cmd_str(xbapi_at_e);
xbapi_modem_status_e modem_status_to_enum(uint8_t);

#endif /* ___XBAPI_H__ */
