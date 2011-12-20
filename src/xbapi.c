#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <talloc.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "xbapi.h"
#include "_xbapi.h"
#include "_packets.h"

xbapi_err_e xbapi_errno( xbapi_rc_t rc ) {
	return rc.code;
}

int xbapi_sys_errno( xbapi_rc_t rc ) {
	assert(rc.code == XBAPI_ERR_SYS);
	return rc.sys_errno;
}

xbapi_rc_t xbapi_rc( xbapi_err_e rc ) {
	xbapi_rc_t ret = {
		.code = rc
	};
	return ret;
}

xbapi_rc_t xbapi_rc_sys() {
	xbapi_rc_t ret = xbapi_rc(XBAPI_ERR_SYS);
	ret.sys_errno = errno;
	return ret;
}

const char *xbapi_strerror( xbapi_rc_t rc ) {
	static char * const errors[] = {
		[XBAPI_ERR_NOERR] = "No error",
		[XBAPI_ERR_SYS] = "System error",
		[XBAPI_ERR_OVERFLOW] = "Overflow",
		[XBAPI_ERR_BADPACKET] = "Bad packet",
		[XBAPI_ERR_BUFBIG] = "Buffer too big"
	};
	if( rc.code == XBAPI_ERR_SYS ) {
		return strerror(rc.sys_errno);
	} else {
		return errors[rc.code];
	}
}

__attribute__((const))
static bool is_control( uint8_t byte ) {
	switch( byte ) {
		case _XBAPI_CONTROL_FRAME_DELIM:
		case _XBAPI_CONTROL_ESCAPE:
		case _XBAPI_CONTROL_XON:
		case _XBAPI_CONTROL_XOFF:
			return true;
		default:
			return false;
	}
}

// buf must be talloc'ed
// if an error occurs, the data in buf is indeterminate
xbapi_rc_t xbapi_unescape( uint8_t **buf ) {
	assert(buf != NULL);
	assert(*buf != NULL);

	uint8_t *b = *buf;
	size_t blen = talloc_array_length(b), retlen = blen;
	if( blen < 2 ) return xbapi_rc(XBAPI_ERR_NOERR);

	for( size_t i = 0; i < blen; i++ ) if( b[i] == _XBAPI_CONTROL_ESCAPE ) retlen--;

	size_t retidx = 0, bidx = 0;
	do {
		if( b[bidx] == _XBAPI_CONTROL_ESCAPE ) {
			bidx++;
			if( bidx >= blen ) return xbapi_rc(XBAPI_ERR_BADPACKET);
			b[retidx] = b[bidx] ^ 0x20;
		} else if( retidx != bidx ) {
			b[retidx] = b[bidx];
		}
	} while( ++retidx, ++bidx < blen );

	uint8_t *ret;
	if( (ret = talloc_realloc_size(NULL, b, retlen)) == NULL ) return xbapi_rc_sys();
	b = ret;
	*buf = b;

	return xbapi_rc(XBAPI_ERR_NOERR);
}

// buf must be talloc'ed
xbapi_rc_t xbapi_escape( uint8_t **buf ) {
	assert(buf != NULL);
	assert(*buf != NULL);

	uint8_t *b = *buf;
	size_t blen = talloc_array_length(b), retlen = 0;

	for( size_t i = 0; i < blen; i++ ) if( is_control(b[i]) ) retlen++;

	// Check for overflow
	if( SIZE_MAX - retlen < blen ) return xbapi_rc(XBAPI_ERR_OVERFLOW);
	retlen += blen;

	uint8_t *ret;
	if( (ret = talloc_realloc_size(NULL, b, retlen)) == NULL ) return xbapi_rc_sys();
	b = ret;
	*buf = b;

	size_t retidx = retlen - 1, bidx = blen - 1;

	do {
		if( is_control(b[bidx]) ) {
			b[retidx--] = b[bidx] ^ 0x20;
			b[retidx] = 0x7D;
		} else if( retidx != bidx ) {
			b[retidx] = b[bidx];
		}
		if( bidx != 0 ) assert(retidx != 0);
	} while( retidx--, bidx --> 0 );

	return xbapi_rc(XBAPI_ERR_NOERR);
}

// buf must be talloc'ed
// if an error occurs, the data in buf is indeterminate
xbapi_rc_t xbapi_unwrap( uint8_t **buf ) {
	assert(buf != NULL);
	assert(*buf != NULL);

	uint8_t *b = *buf;
	if( b[0] != _XBAPI_CONTROL_FRAME_DELIM ) return xbapi_rc(XBAPI_ERR_BADPACKET);

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_unescape(buf)) != XBAPI_ERR_NOERR ) return rc;
	b = *buf;

	size_t blen = talloc_array_length(b);
	assert(blen >= 5);


	uint16_t dlen = ntohs(*((uint16_t *) (b + 1)));
	assert(blen == (dlen + 4u));
	uint8_t checksum = b[blen - 1];
	size_t dlen_esc = blen - 4;

	memmove(b, b + 3, dlen_esc);

	uint8_t *ret;
	if( (ret = talloc_realloc_size(NULL, b, dlen_esc)) == NULL ) return xbapi_rc_sys();
	b = ret;
	*buf = b;

	uint8_t tmpsum = 0;
	for( size_t i = 0; i < dlen; i++ ) tmpsum += b[i];
	tmpsum += checksum;
	if( tmpsum != 0xFF ) {
		return xbapi_rc(XBAPI_ERR_BADPACKET);
	}

	return xbapi_rc(XBAPI_ERR_NOERR);
}

// buf must be talloc'ed
// if an error occurs, the data in buf is indeterminate
xbapi_rc_t xbapi_wrap( uint8_t **buf ) {
	assert(buf != NULL);
	assert(*buf != NULL);

	uint8_t *b = *buf;
	size_t blen = talloc_array_length(b);
	assert(blen >= 1);

	if( blen > 65535 ) return xbapi_rc(XBAPI_ERR_BUFBIG);

	// Calculate checksum _before_ escaping
	uint8_t checksum = 0;
	for( size_t i = 0; i < blen; i++ ) {
		checksum += b[i];
	}
	checksum = 0xFF - checksum;

	// Save the len before escaping so we can put it in the packet.
	uint16_t packetlen = blen;

	uint8_t *ret;
	if( (ret = talloc_realloc_size(NULL, b, blen + 4)) == NULL ) return xbapi_rc_sys();
	b = ret;

	// Prepend the frame header leaving out the delimiter
	// (we don't want to escape it)
	memmove(b + 3, b, blen);
	b[0] = 0;
	*((uint16_t *) (b + 1)) = htons(packetlen);
	b[blen + 3] = checksum;

	*buf = b;
	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_escape(buf)) != XBAPI_ERR_NOERR ) return rc;

	// Restore the frame delimiter after we've escaped
	b[0] = _XBAPI_CONTROL_FRAME_DELIM;

	return xbapi_rc(XBAPI_ERR_NOERR);
}

const char *at_cmd_str(xbapi_at_e command) {
	static const char AT_COMMAND_STRINGS[][2] = {
		[XBAPI_AT_DH] = "DH",
		[XBAPI_AT_DL] = "DL",
		[XBAPI_AT_MY] = "MY",
		[XBAPI_AT_MP] = "MP",
		[XBAPI_AT_NC] = "NC",
		[XBAPI_AT_SH] = "SH",
		[XBAPI_AT_SL] = "SL",
		[XBAPI_AT_NI] = "HI",
		[XBAPI_AT_SE] = "SE",
		[XBAPI_AT_DE] = "DE",
		[XBAPI_AT_CI] = "CI",
		[XBAPI_AT_NP] = "NP",
		[XBAPI_AT_DD] = "DD",
		[XBAPI_AT_CH] = "CH",
		[XBAPI_AT_ID] = "ID",
		[XBAPI_AT_OP] = "OP",
		[XBAPI_AT_NH] = "NH",
		[XBAPI_AT_BH] = "BH",
		[XBAPI_AT_OI] = "OI",
		[XBAPI_AT_NT] = "NT",
		[XBAPI_AT_NO] = "NO",
		[XBAPI_AT_SC] = "SC",
		[XBAPI_AT_SD] = "SD",
		[XBAPI_AT_ZS] = "ZS",
		[XBAPI_AT_NJ] = "NJ",
		[XBAPI_AT_JV] = "JV",
		[XBAPI_AT_NW] = "NW",
		[XBAPI_AT_JN] = "JN",
		[XBAPI_AT_AR] = "AR",
		[XBAPI_AT_EE] = "EE",
		[XBAPI_AT_EO] = "EO",
		[XBAPI_AT_NK] = "NK",
		[XBAPI_AT_KY] = "KY",
		[XBAPI_AT_PL] = "PL",
		[XBAPI_AT_PM] = "PM",
		[XBAPI_AT_DB] = "DB",
		[XBAPI_AT_PP] = "PP",
		[XBAPI_AT_AP] = "AP",
		[XBAPI_AT_AO] = "AO",
		[XBAPI_AT_BD] = "BD",
		[XBAPI_AT_NB] = "NB",
		[XBAPI_AT_SB] = "SB",
		[XBAPI_AT_RO] = "RO",
		[XBAPI_AT_D7] = "D7",
		[XBAPI_AT_D6] = "D6",
		[XBAPI_AT_IR] = "IR",
		[XBAPI_AT_IC] = "IC",
		[XBAPI_AT_P0] = "P0",
		[XBAPI_AT_P1] = "P1",
		[XBAPI_AT_P2] = "P2",
		[XBAPI_AT_P3] = "P3",
		[XBAPI_AT_D0] = "D0",
		[XBAPI_AT_D1] = "D1",
		[XBAPI_AT_D2] = "D2",
		[XBAPI_AT_D3] = "D3",
		[XBAPI_AT_D4] = "D4",
		[XBAPI_AT_D5] = "D5",
		[XBAPI_AT_D8] = "D8",
		[XBAPI_AT_LT] = "LT",
		[XBAPI_AT_PR] = "PR",
		[XBAPI_AT_RP] = "RP",
		[XBAPI_AT_SV] = "SV",
		[XBAPI_AT_VS] = "VS",
		[XBAPI_AT_TP] = "TP",
		[XBAPI_AT_VR] = "VR",
		[XBAPI_AT_HV] = "HV",
		[XBAPI_AT_AI] = "AI",
		[XBAPI_AT_CT] = "CT",
		[XBAPI_AT_CN] = "CN",
		[XBAPI_AT_GT] = "GT",
		[XBAPI_AT_CC] = "CC",
		[XBAPI_AT_SM] = "CM",
		[XBAPI_AT_SN] = "SN",
		[XBAPI_AT_SP] = "SP",
		[XBAPI_AT_ST] = "ST",
		[XBAPI_AT_SO] = "SO",
		[XBAPI_AT_WH] = "WH",
		[XBAPI_AT_PO] = "PO",
		[XBAPI_AT_AC] = "AC",
		[XBAPI_AT_WR] = "WR",
		[XBAPI_AT_RE] = "RE",
		[XBAPI_AT_FR] = "FR",
		[XBAPI_AT_NR] = "NR",
		[XBAPI_AT_SI] = "SI",
		[XBAPI_AT_CB] = "CB",
		[XBAPI_AT_ND] = "ND",
		[XBAPI_AT_DN] = "DN",
		[XBAPI_AT_IS] = "IS",
		[XBAPI_AT_1S] = "1S"
	};

	return AT_COMMAND_STRINGS[command];
}



xbapi_rc_t xbapi_send(xbapi_conn_t *conn, uint8_t *packet) {
	errno = 0;
	// Casting a size_t to an int
	int packet_len = talloc_array_length(packet);
	if (write(conn->fd, packet, packet_len) != packet_len) {
		if (errno)
			return xbapi_rc_sys();
		return xbapi_rc(XBAPI_ERR_INCWRITE);
	}
	return xbapi_rc(XBAPI_ERR_NOERR);
}

xbapi_rc_t xbapi_set_at_param(xbapi_conn_t *conn, xbapi_op_set_t *ops, xbapi_at_e command, xbapi_at_arg_u *args, xbapi_op_t **out_op) {
	assert(conn != NULL);
	assert(ops != NULL);
	assert(out_op != NULL);

	conn->frame_id++;
	if (conn->frame_id == 0) conn->frame_id++;

	const char *cmdstr = at_cmd_str(command);
	static const int PACKET_HEAD_LEN = 4;
	uint8_t packet_head[] = { _XBAPI_FRAME_AT_CMD, conn->frame_id, cmdstr[0], cmdstr[1] };
	uint8_t *packet = NULL;

	// Set up the operation structure (frame id, clear result)
	xbapi_op_t *op;
	xbapi_rc_t rc = create_operation(ops, &op);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;
	op->frame_id = conn->frame_id;
	*out_op = op;

	// Allocate space for the packet and copy the args into it
	switch (command) {
		case XBAPI_AT_DH:
		case XBAPI_AT_DL:
		case XBAPI_AT_SH:
		case XBAPI_AT_SL:
		case XBAPI_AT_DD:
		case XBAPI_AT_ID:
		case XBAPI_AT_OP:
			assert(args != NULL);
			packet = talloc_array_size(NULL, 1, sizeof(args->u32) + PACKET_HEAD_LEN);
			if (packet == NULL && args != NULL)
				return xbapi_rc_sys();
			*((uint32_t *) (packet + PACKET_HEAD_LEN)) = args->u32;
			break;

		case XBAPI_AT_MY:
		case XBAPI_AT_MP:
		case XBAPI_AT_CI:
		case XBAPI_AT_NP:
		case XBAPI_AT_OI:
		case XBAPI_AT_SC:
		case XBAPI_AT_NW:
		case XBAPI_AT_IR:
		case XBAPI_AT_IC:
		case XBAPI_AT_PR:
		case XBAPI_AT_SV:
		case XBAPI_AT_VS:
		case XBAPI_AT_TP:
		case XBAPI_AT_VR:
		case XBAPI_AT_HV:
		case XBAPI_AT_CT:
		case XBAPI_AT_GT:
		case XBAPI_AT_SN:
		case XBAPI_AT_SP:
		case XBAPI_AT_ST:
		case XBAPI_AT_WH:
		case XBAPI_AT_PO:
			assert(args != NULL);
			packet = talloc_array_size(NULL, 1, sizeof(args->u16) + PACKET_HEAD_LEN);
			if (packet == NULL && args != NULL)
				return xbapi_rc_sys();
			*((uint16_t *) (packet + PACKET_HEAD_LEN)) = args->u16;
			break;

		case XBAPI_AT_CN:
		case XBAPI_AT_AC:
		case XBAPI_AT_WR:
		case XBAPI_AT_RE:
		case XBAPI_AT_FR:
		case XBAPI_AT_SI:
		case XBAPI_AT_IS:
		case XBAPI_AT_1S:
		case XBAPI_AT_CB:
			assert(args == NULL);
			packet = talloc_array_size(NULL, 1, PACKET_HEAD_LEN);
			if (packet == NULL && args != NULL)
				return xbapi_rc_sys();
			break;

		case XBAPI_AT_NI:
		case XBAPI_AT_ND:
		case XBAPI_AT_DN:
			assert(args != NULL);
			size_t textlen = talloc_array_length(args->text);
			packet = talloc_array_size(NULL, 1, textlen + PACKET_HEAD_LEN);
			if (packet == NULL && args != NULL)
				return xbapi_rc_sys();
			memcpy(packet + PACKET_HEAD_LEN, args->text, textlen);
			break;

		case XBAPI_AT_NC:
		case XBAPI_AT_SE:
		case XBAPI_AT_DE:
		case XBAPI_AT_CH:
		case XBAPI_AT_NH:
		case XBAPI_AT_BH:
		case XBAPI_AT_NT:
		case XBAPI_AT_NO:
		case XBAPI_AT_SD:
		case XBAPI_AT_ZS:
		case XBAPI_AT_NJ:
		case XBAPI_AT_JV:
		case XBAPI_AT_JN:
		case XBAPI_AT_AR:
		case XBAPI_AT_EE:
		case XBAPI_AT_EO:
		case XBAPI_AT_PL:
		case XBAPI_AT_PM:
		case XBAPI_AT_DB:
		case XBAPI_AT_PP:
		case XBAPI_AT_AP:
		case XBAPI_AT_AO:
		case XBAPI_AT_BD:
		case XBAPI_AT_NB:
		case XBAPI_AT_SB:
		case XBAPI_AT_RO:
		case XBAPI_AT_D7:
		case XBAPI_AT_D6:
		case XBAPI_AT_P0:
		case XBAPI_AT_P1:
		case XBAPI_AT_P2:
		case XBAPI_AT_P3:
		case XBAPI_AT_D0:
		case XBAPI_AT_D1:
		case XBAPI_AT_D2:
		case XBAPI_AT_D3:
		case XBAPI_AT_D4:
		case XBAPI_AT_D5:
		case XBAPI_AT_D8:
		case XBAPI_AT_LT:
		case XBAPI_AT_RP:
		case XBAPI_AT_AI:
		case XBAPI_AT_CC:
		case XBAPI_AT_SM:
		case XBAPI_AT_SO:
		case XBAPI_AT_NR:
			assert(args != NULL);
			packet = talloc_array_size(NULL, 1, sizeof(args->u8) + PACKET_HEAD_LEN);
			if (packet == NULL && args != NULL)
				return xbapi_rc_sys();
			packet[PACKET_HEAD_LEN] = args->u8;
			break;

		case XBAPI_AT_NK:
		case XBAPI_AT_KY:
			assert(args != NULL);
			packet = talloc_array_size(NULL, 1, sizeof(args->u128) + PACKET_HEAD_LEN);
			if (packet == NULL && args != NULL)
				return xbapi_rc_sys();
			memcpy(packet + PACKET_HEAD_LEN, args->u128, sizeof(args->u128));
			break;
	}

	// Copy the packet head into the packet and wrap it
	memcpy(packet, packet_head, PACKET_HEAD_LEN);
	rc = xbapi_wrap(&packet);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;

	return xbapi_send(conn, packet);
}

xbapi_rc_t xbapi_query_at_param(xbapi_conn_t *conn, xbapi_op_set_t *ops, xbapi_at_e command, xbapi_op_t **out_op) {
	assert(conn != NULL);
	assert(ops != NULL);
	assert(out_op != NULL);

	conn->frame_id++;
	if (conn->frame_id == 0) conn->frame_id++;

	const char *cmdstr = at_cmd_str(command);
	static const int PACKET_LEN = 4;
	uint8_t packet_data[] = { _XBAPI_FRAME_AT_CMD, conn->frame_id, cmdstr[0], cmdstr[1] };
	uint8_t *packet = talloc_array_size(NULL, 1, PACKET_LEN);
	memcpy(packet, packet_data, PACKET_LEN);
	xbapi_rc_t rc = xbapi_wrap(&packet);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;

	// Set up the operation structure (frame id, clear result)
	xbapi_op_t *op;
	rc = create_operation(ops, &op);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;
	op->frame_id = conn->frame_id;
	*out_op = op;

	return xbapi_send(conn, packet);
}

xbapi_rc_t xbapi_process_data(xbapi_conn_t *conn, xbapi_op_set_t *ops, xbapi_callbacks_t *callbacks) {
	assert(conn != NULL);
	assert(ops != NULL);
	assert(callbacks != NULL);

	static const int BUF_SIZE = 100;
	int buf_len = 0;
	uint8_t buf[BUF_SIZE];
	size_t buffer_len = 0;

	// If the last message ended in a trailing escape character (and we removed it),
	// set the message to the escape character
	if (conn->rollover_escape) {
		buffer_len = talloc_array_length(conn->buffer);
		conn->rollover_escape = false;
		conn->buffer = talloc_realloc_size(conn, conn->buffer, buffer_len + 1);
		conn->buffer[buffer_len] = _XBAPI_CONTROL_ESCAPE;
	}

	// Read the serial device and append the data to the existing buffer
	do {
		buf_len = read(conn->fd, buf, BUF_SIZE);
		if (buf_len == -1) return xbapi_rc_sys();

		// Casting a size_t to an int
		buffer_len = talloc_array_length(conn->buffer);
		conn->buffer = talloc_realloc_size(conn, conn->buffer, buffer_len + buf_len);
		if(conn->buffer == NULL) return xbapi_rc_sys();
		memcpy(conn->buffer + buffer_len, buf, buf_len);
	} while (buf_len == BUF_SIZE);

	// Figure out if there is an unmatched trailing escape character
	buffer_len = talloc_array_length(conn->buffer);
	bool has_trailing = false;
	for (uint8_t *ptr = conn->buffer + buffer_len - 1; ptr >= conn->buffer; ptr--) {
		if (*ptr == _XBAPI_CONTROL_ESCAPE) {
			has_trailing = !has_trailing;
		} else
			break;
	}

	// Remove the trailing escape and save it for the next batch
	buffer_len = talloc_array_length(conn->buffer);
	if (has_trailing) {
		conn->rollover_escape = true;
		conn->buffer = talloc_realloc_size(conn, conn->buffer, buffer_len - 1);
		buffer_len--;
	}

	// Unescape the buffered message
	xbapi_rc_t rc = xbapi_unescape(&conn->buffer);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;
	buffer_len = talloc_array_length(conn->buffer);

	uint8_t *ptr = conn->buffer;
	while (buffer_len > 0) {
		// Toss out data until we find a frame start delimiter
		while (*ptr != _XBAPI_CONTROL_FRAME_DELIM && buffer_len > 0) {
			ptr++;
			buffer_len--;
		}

		// Check the length of the frame
		int frm_len = -1;
		if (buffer_len >= 2) {
			// Casting a uint16_t to an int
			frm_len = ntohs(*((uint16_t *) (ptr + 1)));
		}
		// If we don't have the entire frame, save what we have in the connection buffer and return
		size_t packet_len = frm_len + 4;
		size_t orig_packet_len = packet_len;
		if ((frm_len == -1) || (buffer_len < packet_len)) break;

		// Unwrap the frame
		uint8_t *packet = talloc_array_size(NULL, 1, packet_len);
		if(packet == NULL) return xbapi_rc_sys();
		memcpy(packet, ptr, packet_len);
		xbapi_rc_t rc = xbapi_unwrap(&packet);
		if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;
		packet_len = talloc_array_length(packet);

		// Handle the message
		uint8_t *data;
		switch (frame_type_from_packet(packet)) {
			case XBAPI_FRAME_AT_CMD_RES:
				assert(packet_len >= AT_CMD_RES_MIN_LEN);

				if (command_data_len_from_at_cmd_res(packet)) {
					data = talloc_array(NULL, uint8_t, command_data_len_from_at_cmd_res(packet));
					if(conn->buffer == NULL) return xbapi_rc_sys();
					memcpy(data, command_data_from_at_cmd_res(packet), command_data_len_from_at_cmd_res(packet));
				} else {
					data = NULL;
				}

				uint8_t frame_id = frame_id_from_at_cmd_res(packet);
				// Find the corresponding operation structure
				for (size_t i = 0; i < ops->pending_count; i++) {
					xbapi_op_t *op = ops->ops_pending[i];
					if (op->frame_id == frame_id) {
						op->status = command_status_from_at_cmd_res(packet);
						op->data = data;
						move_operation(ops, op);
						break;
					}
				}
				break;

			case XBAPI_FRAME_MODEM_STAT:
				assert(packet_len == MODEM_STAT_LEN);
				conn->latest_modem_status = status_from_modem_stat(packet);
				break;

			case XBAPI_FRAME_TX_STAT:
				assert(packet_len == TX_STAT_LEN);

				// USE THIS frame_id_from_tx_stat(packet);
				if (callbacks->transmit_completed == NULL) break;

				xbapi_tx_status_t *status = talloc(NULL, xbapi_tx_status_t);

				status->delivery_network_address = delivery_network_address_from_tx_stat(packet);
				status->retry_count = retry_count_from_tx_stat(packet);
				status->delivery_status = delivery_status_from_tx_stat(packet);
				status->discovery_status = discovery_status_from_tx_stat(packet);

				callbacks->transmit_completed(status);

				talloc_free(status);
				break;

			case XBAPI_FRAME_RX_PACKET:
				assert(packet_len >= RX_PACKET_MIN_LEN);

				if (callbacks->node_connected == NULL) break;

				xbapi_rx_packet_t *received = talloc(NULL, xbapi_rx_packet_t);

				received->source_address = source_address_from_rx_packet(packet);
				received->source_network_address = source_network_address_from_rx_packet(packet);
				received->options = options_from_rx_packet(packet);

				size_t data_len;
				data = data_from_rx_packet(packet, &data_len);
				received->data = talloc_array(received, uint8_t, data_len);
				memcpy(received->data, data, data_len);

				callbacks->received_packet(received);

				talloc_free(received);
				break;

			case XBAPI_FRAME_NODE_ID:
				assert(packet_len >= NODE_ID_MIN_LEN);

				if (callbacks->node_connected == NULL) break;

				xbapi_node_identification_t *node = talloc(NULL, xbapi_node_identification_t);

				node->source_address         = source_address_from_node_id(packet);
				node->source_network_address = source_network_address_from_node_id(packet);
				node->remote_address         = remote_address_from_node_id(packet);
				node->remote_network_address = remote_network_address_from_node_id(packet);
				node->receive_options        = receive_options_from_node_id(packet);
				node->node_identifier        = ni_string_from_node_id(packet);
				node->parent_network_address = parent_network_address_from_node_id(packet);
				node->device_type            = device_type_from_node_id(packet);
				node->source_event           = source_event_from_node_id(packet);
				node->profile_id             = profile_id_from_node_id(packet);
				node->manufacturer_id        = manufacturer_id_from_node_id(packet);

				callbacks->node_connected(node);

				talloc_free(node);
				break;

			// We don't support these messages, so just ignore them
			case XBAPI_FRAME_IO_DATA_RX:
			case XBAPI_FRAME_SENSOR_READ:
			case XBAPI_FRAME_RT_RC_INDIC:
			case XBAPI_FRAME_M_1_RT_REQ:
			case XBAPI_FRAME_XRX_INDIC:
			case XBAPI_FRAME_RMT_CMD_RES:
			case XBAPI_FRAME_UPDATE_STAT:
				break;

			// We shouldn't receive these messages (these are host to xbee only)
			case XBAPI_FRAME_AT_CMD:
			case XBAPI_FRAME_AT_QUEUED:
			case XBAPI_FRAME_TX_REQ:
			case XBAPI_FRAME_XADDR_CMD:
			case XBAPI_FRAME_RMT_CMD_REQ:
			case XBAPI_FRAME_CRT_SRC_RT:
			case XBAPI_FRAME_INVALID:
				assert(false);
		}

		ptr += orig_packet_len;
		buffer_len -= orig_packet_len;
		talloc_free(packet);
	}
	if (buffer_len > 0) {
		memmove(conn->buffer, ptr, buffer_len);
		conn->buffer = talloc_realloc_size(conn, conn->buffer, buffer_len);
		if (conn->buffer == NULL && buffer_len > 0) return xbapi_rc_sys();
	} else {
		talloc_free(conn->buffer);
		conn->buffer = NULL;
	}

	return xbapi_rc(XBAPI_ERR_NOERR);
}

xbapi_conn_t *xbapi_init_conn(int fd) {
	xbapi_conn_t *rt = talloc(NULL, xbapi_conn_t);
	rt->fd = fd;
	rt->frame_id = 0;
	rt->buffer = NULL;
	rt->rollover_escape = false;
	return rt;
}

void xbapi_free_conn(xbapi_conn_t *conn) {
	talloc_free(conn);
	conn = NULL;
}

xbapi_op_set_t *xbapi_init_op_set() {
	xbapi_op_set_t *set = talloc(NULL, xbapi_op_set_t);
	set->ops_success = talloc_array(set, xbapi_op_t*, INITIAL_OP_SET_SIZE);
	set->ops_failure = talloc_array(set, xbapi_op_t*, INITIAL_OP_SET_SIZE);
	set->ops_pending = talloc_array(set, xbapi_op_t*, INITIAL_OP_SET_SIZE);
	set->success_count = 0;
	set->failure_count = 0;
	set->pending_count = 0;
	return set;
}

void xbapi_free_op_set(xbapi_op_set_t *set) {
	talloc_free(set);
	set = NULL;
}

xbapi_rc_t create_operation(xbapi_op_set_t *set, xbapi_op_t **op) {
	assert(op != NULL);

	*op = talloc(set, xbapi_op_t);
	(*op)->status = XBAPI_OP_STATUS_PENDING;

	xbapi_rc_t rc = move_operation(set, *op);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR ) return rc;

	return xbapi_rc(XBAPI_ERR_NOERR);
}

xbapi_rc_t move_operation(xbapi_op_set_t *set, xbapi_op_t *op) {
	// Remove the operation from the pending queue (if it exists)
	for (size_t i = 0; i < set->pending_count; i++) {
		if ((set->ops_pending[i]) == op) {
			for (size_t j = i; j < set->pending_count - 1; j++) set->ops_pending[j] = set->ops_pending[j + 1];
			break;
		}
	}

	// Copy the operation into the appropriate queue
	switch(op->status) {
		case XBAPI_OP_STATUS_OK:
			if (set->success_count == talloc_array_length(set->ops_success)) {
				set->ops_success = talloc_realloc_size(set, set->ops_success, talloc_array_length(set->ops_success) + INITIAL_OP_SET_SIZE);
				if (set->ops_success == NULL) return xbapi_rc_sys();
			}
			set->ops_success[set->success_count] = op;
			set->success_count++;
			break;
		case XBAPI_OP_STATUS_ERROR:
		case XBAPI_OP_STATUS_INVALID_CMD:
		case XBAPI_OP_STATUS_INVALID_PARAM:
		case XBAPI_OP_STATUS_TX_FAILURE:
			if (set->failure_count == talloc_array_length(set->ops_failure)) {
				set->ops_failure = talloc_realloc_size(set, set->ops_failure, talloc_array_length(set->ops_failure) + INITIAL_OP_SET_SIZE);
				if (set->ops_failure == NULL) return xbapi_rc_sys();
			}
			set->ops_failure[set->failure_count] = op;
			set->failure_count++;
			break;
		case XBAPI_OP_STATUS_PENDING:
			if (set->pending_count == talloc_array_length(set->ops_pending)) {
				set->ops_pending = talloc_realloc_size(set, set->ops_pending, talloc_array_length(set->ops_pending) + INITIAL_OP_SET_SIZE);
				if (set->ops_pending == NULL) return xbapi_rc_sys();
			}
			set->ops_pending[set->pending_count] = op;
			set->pending_count++;
			break;
	}

	return xbapi_rc(XBAPI_ERR_NOERR);
}

// Get the list of successes and failures

xbapi_op_status_e status_from_operation(xbapi_op_t *op) {
	assert(op != NULL);
	return op->status;
}

uint8_t *data_from_operation(xbapi_op_t *op) {
	assert(op != NULL);
	return op->data;
}

xbapi_rc_t xbapi_transmit_data(xbapi_conn_t *conn, xbapi_op_set_t *ops, uint8_t *data, uint64_t destination, xbapi_op_t **out_op) {
	size_t data_len = talloc_array_length(data);
	uint8_t *packet = talloc_array(NULL, uint8_t, 14 + data_len);

	conn->frame_id++;
	if (conn->frame_id == 0) conn->frame_id++;

	// Specify the frame type and id
	packet[0] = _XBAPI_FRAME_TX_REQ;
	packet[1] = conn->frame_id;
	// Set the destination address
	*((uint64_t *)(packet + 2)) = htonll(destination);
	// Set the destination network address to unknown
	packet[10] = 0xFF;
	packet[11] = 0xFE;
	// Set broadcast radius to the maximum
	packet[12] = 0x00;
	// No additional options
	packet[13] = 0x00;
	// Append the data
	memcpy(packet + 14, data, data_len);

	xbapi_rc_t rc = xbapi_wrap(&packet);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;

	// Set up the operation structure (frame id, clear result)
	xbapi_op_t *op;
	rc = create_operation(ops, &op);
	if (xbapi_errno(rc) != XBAPI_ERR_NOERR) return rc;
	op->frame_id = conn->frame_id;
	*out_op = op;

	return xbapi_send(conn, packet);
}

