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
		case XBAPI_FRAME_DELIM:
		case XBAPI_ESCAPE:
		case XBAPI_XON:
		case XBAPI_XOFF:
			return true;
		default:
			return false;
	}
}

// buf must be talloc'ed
// if an error occurs, the data in buf is indeterminate
static xbapi_rc_t xbapi_unescape( uint8_t **buf ) {
	assert(buf != NULL);
	assert(*buf != NULL);

	uint8_t *b = *buf;
	size_t blen = talloc_array_length(b), retlen = blen;
	if( blen < 2 ) return xbapi_rc(XBAPI_ERR_NOERR);

	for( size_t i = 0; i < blen; i++ ) if( b[i] == XBAPI_ESCAPE ) retlen--;

	size_t retidx = 0, bidx = 0;
	do {
		if( b[bidx] == XBAPI_ESCAPE ) {
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
static xbapi_rc_t xbapi_escape( uint8_t **buf ) {
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
	if( b[0] != XBAPI_FRAME_DELIM ) return xbapi_rc(XBAPI_ERR_BADPACKET);

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
	b[0] = XBAPI_FRAME_DELIM;

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
	int packet_len = talloc_array_length(packet);
	if (write(conn->fd, packet, packet_len) != packet_len) {
		if (errno)
			return xbapi_rc_sys();
		return xbapi_rc(XBAPI_ERR_INCWRITE);
	}
	return xbapi_rc(XBAPI_ERR_NOERR);
}


xbapi_rc_t xbapi_set_at_param(xbapi_conn_t *conn, xbapi_op_t *op, xbapi_at_e command, xbapi_at_arg_u *args) {
	assert(conn != NULL);
	assert(op != NULL);

	conn->frame_id++;
	if (conn->frame_id == 0) conn->frame_id++;

	const char *cmdstr = at_cmd_str(command);
	static const int PACKET_HEAD_LEN = 4;
	uint8_t packet_head[] = { XBAPI_FRAME_AT, conn->frame_id, cmdstr[0], cmdstr[1] };
	uint8_t *packet = NULL;

	// Set up the operation structure (frame id, clear result)
	op->frame_id = conn->frame_id;
	op->status = XBAPI_OP_STATUS_PENDING;

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
			int textlen = talloc_array_length(args->text);
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
	xbapi_wrap(&packet);

	return xbapi_send(conn, packet);
}

xbapi_rc_t xbapi_query_at_param(xbapi_conn_t *conn, xbapi_op_t *op, xbapi_at_e command) {
	assert(conn != NULL);
	assert(op != NULL);

	conn->frame_id++;
	if (conn->frame_id == 0) conn->frame_id++;

	const char *cmdstr = at_cmd_str(command);
	static const int PACKET_LEN = 4;
	uint8_t packet_data[] = { XBAPI_FRAME_AT, conn->frame_id, cmdstr[0], cmdstr[1] };
	uint8_t *packet = talloc_array_size(NULL, 1, PACKET_LEN);
	memcpy(packet, packet_data, PACKET_LEN);
	xbapi_wrap(&packet);

	// Set up the operation structure (frame id, clear result)
	op->frame_id = conn->frame_id;
	op->status = XBAPI_OP_STATUS_PENDING;

	return xbapi_send(conn, packet);
}
