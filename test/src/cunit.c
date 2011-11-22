#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>

#include <talloc.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "xbapi.h"
#include "test_xbapi.h"

__attribute__((noreturn, nonnull))
static void xbapi_die( const char *ctx, xbapi_rc_t rc ) {
	assert(xbapi_errno(rc) != XBAPI_ERR_NOERR);
	fprintf(stderr, "%s: %s\n", ctx, xbapi_strerror(rc));
	exit(EXIT_FAILURE);
}

__attribute__((noreturn, nonnull))
static void CU_die( const char *ctx ) {
	assert(CU_get_error() != CUE_SUCCESS);
	fprintf(stderr, "%s: %s\n", ctx, CU_get_error_msg());
	exit(EXIT_FAILURE);
}

static void test_xbapi_escape() {
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
	buf[datalen + 3] = 0xCB;
	uint8_t expected[] = { 0x7E, 0x00, 0x02, 0x23, 0x7D, 0x31, 0xCB };
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(datalen, 3);
	CU_ASSERT_EQUAL(buflen, 7);
	CU_ASSERT_NSTRING_EQUAL(buf, expected, buflen);
	TALLOC_FREE(buf);
}

int main() {
	//talloc_enable_leak_report_full();
	if( CU_initialize_registry() != CUE_SUCCESS ) CU_die("CU_initialize_registry");
	CU_pSuite suite;
	if( (suite = CU_add_suite("xbapi", NULL, NULL)) == NULL ) CU_die("CU_add_suite");
	if( CU_ADD_TEST(suite, test_xbapi_escape) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_basic_run_tests() != CUE_SUCCESS ) CU_die("CU_basic_run_tests");
	int ret = CU_get_number_of_failure_records() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
	CU_cleanup_registry();
	return ret;
}
