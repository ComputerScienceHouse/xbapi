#include "xbapi.c"

#include "test_xbapi.h"
#include "cunit.h"

__attribute__((noreturn, nonnull))
static void xbapi_die( const char *ctx, xbapi_rc_t rc ) {
	assert(xbapi_errno(rc) != XBAPI_ERR_NOERR);
	fprintf(stderr, "%s: %s\n", ctx, xbapi_strerror(rc));
	exit(EXIT_FAILURE);
}

static void test_xbapi_unescape() {
	xbapi_rc_t rc;
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 3)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x23;
	buf[1] = 0x7D;
	buf[2] = 0x31;
	if( xbapi_errno(rc = xbapi_unescape(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_escape", rc);
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(buflen, 2);
	uint8_t expected[] = { 0x23, 0x11 };
	for( size_t i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
	TALLOC_FREE(buf);
}

static void test_xbapi_escape() {
	xbapi_rc_t rc;
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 2)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x23;
	buf[1] = 0x11;
	if( xbapi_errno(rc = xbapi_escape(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_escape", rc);
	uint8_t expected[] = { 0x23, 0x7D, 0x31 };
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(buflen, 3);
	for( size_t i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
	TALLOC_FREE(buf);
}

static void test_xbapi_unwrap1() {
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 10)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x7E;
	buf[1] = 0x00;
	buf[2] = 0x05;
	buf[3] = 0x08;
	buf[4] = 0x01;
	buf[5] = 0x4E;
	buf[6] = 0x7D;
	buf[7] = 0x31;
	buf[8] = 0xFF;
	buf[9] = 0x98;
	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_unwrap(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_unwrap", rc);
	uint8_t expected[] = { 0x08, 0x01, 0x4E, 0x11, 0xFF };
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(buflen, 5);
	for( size_t i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
	TALLOC_FREE(buf);
}

static void test_xbapi_unwrap2() {
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 24)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x7E;
	buf[1] = 0x00;
	buf[2] = 0x7D;
	buf[3] = 0x31;
	buf[4] = 0x08;
	buf[5] = 0x01;
	buf[6] = 0x4E;
	buf[7] = 0x7D;
	buf[8] = 0x31;
	buf[9] = 0xFF;
	buf[10] = 0x00;
	buf[11] = 0x00;
	buf[12] = 0x00;
	buf[13] = 0x00;
	buf[14] = 0x00;
	buf[15] = 0x00;
	buf[16] = 0x00;
	buf[17] = 0x00;
	buf[18] = 0x00;
	buf[19] = 0x00;
	buf[20] = 0x00;
	buf[21] = 0x1B;
	buf[22] = 0x7D;
	buf[23] = 0x5D;
	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_unwrap(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_unwrap", rc);
	uint8_t expected[] = { 0x08, 0x01, 0x4E, 0x11, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B };
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(buflen, 17);
	for( size_t i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
	TALLOC_FREE(buf);
}

static void test_xbapi_wrap1() {
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 5)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x08;
	buf[1] = 0x01;
	buf[2] = 0x4E;
	buf[3] = 0x11;
	buf[4] = 0xFF;
	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_wrap(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_unwrap", rc);
	uint8_t expected[] = { 0x7E, 0x00, 0x05, 0x08, 0x01, 0x4E, 0x7D, 0x31, 0xFF, 0x98 };
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(buflen, 10);
	for( size_t i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
	TALLOC_FREE(buf);
}

static void test_xbapi_wrap2() {
	uint8_t *buf;
	if( (buf = talloc_array(NULL, uint8_t, 17)) == NULL ) xbapi_die("talloc_array", xbapi_rc_sys());
	buf[0] = 0x08;
	buf[1] = 0x01;
	buf[2] = 0x4E;
	buf[3] = 0x11;
	buf[4] = 0xFF;
	buf[5] = 0x00;
	buf[6] = 0x00;
	buf[7] = 0x00;
	buf[8] = 0x00;
	buf[9] = 0x00;
	buf[10] = 0x00;
	buf[11] = 0x00;
	buf[12] = 0x00;
	buf[13] = 0x00;
	buf[14] = 0x00;
	buf[15] = 0x00;
	buf[16] = 0x1A;
	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_wrap(&buf)) != XBAPI_ERR_NOERR ) xbapi_die("xbapi_unwrap", rc);
	uint8_t expected[] = { 0x7E, 0x00, 0x7D, 0x31, 0x08, 0x01, 0x4E, 0x7D, 0x31, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x7D, 0x5E };
	size_t buflen = talloc_array_length(buf);
	CU_ASSERT_EQUAL(buflen, 24);
	for( size_t i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
	TALLOC_FREE(buf);
}

void xbapi_add_suite() {
	CU_pSuite suite;
	if( (suite = CU_add_suite("xbapi", NULL, NULL)) == NULL ) CU_die("CU_add_suite");
	if( CU_ADD_TEST(suite, test_xbapi_escape) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_unescape) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_wrap1) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_wrap2) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_unwrap1) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_unwrap2) == NULL ) CU_die("CU_ADD_TEST");
}
