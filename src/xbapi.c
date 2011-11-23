#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

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
		[XBAPI_ERR_BADPACKET] = "Bad packet"
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
		case 0x7E:
		case 0x7D:
		case 0x11:
		case 0x13:
			return true;
		default:
			return false;
	}
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
	uint8_t *ret = talloc_realloc_size(NULL, b, retlen);
	if( ret == NULL ) return xbapi_rc_sys();
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

	*buf = ret;
	return xbapi_rc(XBAPI_ERR_NOERR);
}

// buf must be talloc'ed
xbapi_rc_t xbapi_unwrap( uint8_t **buf ) {
	assert(buf != NULL);
	assert(*buf != NULL);
	uint8_t *b = *buf;
	if( b[0] != 0x7E ) return xbapi_rc(XBAPI_ERR_BADPACKET);
	size_t blen = talloc_array_length(b);
	assert(blen >= 5);
	// TODO: This is endian specific
	uint16_t dlen = (b[1] << 8) | b[2];
	uint8_t checksum = b[blen - 1], runningChecksum = 0;
	for( size_t i = 3; i < blen - 1; i++ ) runningChecksum += b[i];
	runningChecksum = 0xFF - runningChecksum;
	if( runningChecksum != checksum ) return xbapi_rc(XBAPI_ERR_BADPACKET);
	memmove(b, b + 3, dlen);
	uint8_t *ret = talloc_realloc_size(NULL, b, dlen);
	if( ret == NULL ) {
		int eno = errno;
		// Shit. TODO: Undo our destructive work to b.
		errno = eno;
		return xbapi_rc_sys();
	}
	*buf = ret;
	return xbapi_rc(XBAPI_ERR_NOERR);
}
