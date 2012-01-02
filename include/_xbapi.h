#ifndef ___XBAPI_H__
#define ___XBAPI_H__

#include <stdint.h>
#include <sys/types.h>
#include "xbapi.h"

static const int INITIAL_OP_SET_SIZE = 5;

typedef enum {
	XBAPI_OP_TYPE_CMD,
	XBAPI_OP_TYPE_TX
} xbapi_op_type_e;

struct _xbapi_op_t {
	uint8_t frame_id;
	xbapi_op_status_e status;
	xbapi_op_type_e type;
	union {
		xbapi_delivery_status_e delivery_status;
		xbapi_at_cmd_status_e at_cmd_status;
	};
	uint8_t *response_data;
	void *user_data;
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
