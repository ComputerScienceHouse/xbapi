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
		[XBAPI_ERR_OVERFLOW] = "Overflow"
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
	size_t blen = talloc_array_length(b), retlen;
	for( size_t i = 0; i < blen; i++ ) if( is_control(b[i]) ) retlen++;
	// Check for overflow
	if( SIZE_MAX - retlen < blen ) return xbapi_rc(XBAPI_ERR_OVERFLOW);
	retlen += blen;
	void *ret = talloc_realloc_size(NULL, b, retlen);
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

	return xbapi_rc(XBAPI_ERR_NOERR);
}

static void test_xbapi_escape() {
	sleep(10);
	xbapi_rc_t rc;
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 2)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x23;
	buf[1] = 0x11;
	if( xbapi_errno(rc = xbapi_escape(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_escape", rc);
	size_t datalen = talloc_array_length(buf);
	if( (buf = talloc_realloc(NULL, buf, uint8_t, datalen + 4)) == NULL ) xbapi_die("talloc_realloc", xbapi_rc_sys());
	memmove(buf + 3, buf, datalen);
	buf[0] = 0x7E;
	buf[1] = 0x00;
	buf[2] = 0x02;
	buf[5] = 0xCB;
	for( size_t i = 0; i < talloc_array_length(buf); i++ ) {
		printf("0x%02X ", buf[i]);
	}
	printf("\n");
	TALLOC_FREE(buf);
}
