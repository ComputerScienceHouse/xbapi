#ifndef ___XBAPI_H__
#define ___XBAPI_H__

#include <stdint.h>
#include <sys/types.h>
#include "xbapi.h"

static const int INITIAL_OP_SET_SIZE = 5;

typedef enum {
	XBAPI_CONTROL_FRAME_DELIM = 0x7E,
	XBAPI_CONTROL_ESCAPE      = 0x7D,
	XBAPI_CONTROL_XON         = 0x11,
	XBAPI_CONTROL_XOFF        = 0x13
} xbapi_control_chars_e;

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

xbapi_rc_t xbapi_escape( uint8_t **buf );
xbapi_rc_t xbapi_unescape( uint8_t **buf );

xbapi_rc_t create_operation(xbapi_op_set_t *set, xbapi_op_t **op);
xbapi_rc_t move_operation(xbapi_op_set_t *set, xbapi_op_t *op);

const char *at_cmd_str(xbapi_at_e);
xbapi_modem_status_e modem_status_to_enum(uint8_t);

#endif /* ___XBAPI_H__ */
