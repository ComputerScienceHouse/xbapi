#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <arpa/inet.h>

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

	uint8_t *ret = talloc_realloc_size(NULL, b, retlen);
	// Since we are shrinking, this should never fail.
	// If it does though, there is no better recourse.
	if( ret == NULL ) abort();

	*buf = ret;
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
	if( b[0] != XBAPI_FRAME_DELIM ) return xbapi_rc(XBAPI_ERR_BADPACKET);

	size_t blen = talloc_array_length(b);
	assert(blen >= 5);

	uint16_t dlen = ntohs(*((uint16_t *) (b + 1)));
	uint8_t checksum = b[blen - 1];

	size_t dlen_esc = blen - 4;

	memmove(b, b + 3, dlen_esc);

	uint8_t *ret = talloc_realloc_size(NULL, b, dlen_esc);
	// There's no easy recourse if this fails.
	// It shouldn't though since we're shrinking.
	if( ret == NULL ) abort();

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_unescape(buf)) != XBAPI_ERR_NOERR ) return rc;
	b = *buf;
	assert(talloc_array_length(b) == dlen);

	uint8_t tmpsum = 0;
	for( size_t i = 0; i < dlen; i++ ) tmpsum += b[i];
	tmpsum += checksum;
	if( tmpsum != 0xFF ) {
		// TODO: Need to restore lost state
		return xbapi_rc(XBAPI_ERR_BADPACKET);
	}

	*buf = ret;
	return xbapi_rc(XBAPI_ERR_NOERR);
}

// buf must be talloc'ed
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

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_escape(buf)) != XBAPI_ERR_NOERR ) return rc;
	b = *buf;
	blen = talloc_array_length(b);

	uint8_t *ret = talloc_realloc_size(NULL, b, blen + 4);
	if( ret == NULL ) return xbapi_rc_sys();

	memmove(ret + 3, ret, blen);
	ret[0] = XBAPI_FRAME_DELIM;
	*((uint16_t *) (ret + 1)) = htons(packetlen);
	ret[blen + 3] = checksum;

	*buf = ret;
	return xbapi_rc(XBAPI_ERR_NOERR);
}
