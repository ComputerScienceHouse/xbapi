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

static void test_xbapi_set_at_param1() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	xbapi_conn_t conn = { .fd = fds[1], .frame_id = 0x05 };
	xbapi_op_t op;
	xbapi_at_arg_u args = { .u8 = 0xFF };

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_set_at_param(&conn, &op, XBAPI_AT_NJ, &args)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_set_at_param", rc);

	uint8_t buf[10];
	int buflen = read(fds[0], &buf, 10);
	uint8_t expected[] = { 0x7E, 0x00, 0x05, 0x08, 0x06, 0x4E, 0x4A, 0xFF, 0x5A };

	CU_ASSERT_EQUAL(buflen, 9);
	for( int i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
}

static void test_xbapi_set_at_param2() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	xbapi_conn_t conn = { .fd = fds[1], .frame_id = 0x38 };
	xbapi_op_t op;
	xbapi_at_arg_u args = { .text = talloc_array(NULL, uint8_t, 0) };

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_set_at_param(&conn, &op, XBAPI_AT_ND, &args)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_set_at_param", rc);

	uint8_t buf[9];
	int buflen = read(fds[0], &buf, 9);
	uint8_t expected[] = { 0x7E, 0x00, 0x04, 0x08, 0x39, 0x4E, 0x44, 0x2C };

	CU_ASSERT_EQUAL(buflen, 8);
	for( int i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
}

static void test_xbapi_set_at_param3() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	xbapi_conn_t conn = { .fd = fds[1], .frame_id = 0x1D };
	xbapi_op_t op;
	uint8_t *te = talloc_array(NULL, uint8_t, 3);
	memcpy(te, "ABC", 3);
	xbapi_at_arg_u args = { .text = te };

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_set_at_param(&conn, &op, XBAPI_AT_ND, &args)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_set_at_param", rc);

	uint8_t buf[12];
	int buflen = read(fds[0], &buf, 12);
	uint8_t expected[] = { 0x7E, 0x00, 0x07, 0x08, 0x1E, 0x4E, 0x44, 0x41, 0x42, 0x43, 0x81 };

	CU_ASSERT_EQUAL(buflen, 11);
	for( int i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
}

static void test_xbapi_set_at_param4() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	xbapi_conn_t conn = { .fd = fds[1], .frame_id = 0xFF };
	xbapi_op_t op;

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_set_at_param(&conn, &op, XBAPI_AT_IS, NULL)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_set_at_param", rc);

	uint8_t buf[9];
	int buflen = read(fds[0], &buf, 9);
	uint8_t expected[] = { 0x7E, 0x00, 0x04, 0x08, 0x01, 0x49, 0x53, 0x5A };

	CU_ASSERT_EQUAL(buflen, 8);
	for( int i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
}

static void test_xbapi_query_at_param1() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	xbapi_conn_t conn = { .fd = fds[1], .frame_id = 0x18 };
	xbapi_op_t op;

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_query_at_param(&conn, &op, XBAPI_AT_BH)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_query_at_param", rc);

	uint8_t buf[9];
	int buflen = read(fds[0], &buf, 8);
	uint8_t expected[] = { 0x7E, 0x00, 0x04, 0x08, 0x19, 0x42, 0x48, 0x54 };

	CU_ASSERT_EQUAL(buflen, 8);
	for( int i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
}

static void test_xbapi_query_at_param2() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	xbapi_conn_t conn = { .fd = fds[1], .frame_id = 0xD6 };
	xbapi_op_t op;

	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_query_at_param(&conn, &op, XBAPI_AT_EE)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_query_at_param", rc);

	uint8_t buf[9];
	int buflen = read(fds[0], &buf, 8);
	uint8_t expected[] = { 0x7E, 0x00, 0x04, 0x08, 0xD7, 0x45, 0x45, 0x96 };

	CU_ASSERT_EQUAL(buflen, 8);
	for( int i = 0; i < buflen; i++ ) CU_ASSERT_EQUAL(buf[i], expected[i]);
}

static void test_xbapi_process_data1() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	uint8_t buf[350];
	buf[0] = 0x7E;
	buf[1] = 0x0F;
	buf[2] = 0xFF;
	for (int i = 3; i < 350; i++) buf[i] = 0x01;
	write(fds[1], buf, 350);

	xbapi_conn_t conn = xbapi_init_conn(fds[0]);
	xbapi_rc_t rc;
	if( xbapi_errno(rc = xbapi_process_data(&conn, NULL)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_xbapi_process_data", rc);

	CU_ASSERT_EQUAL(talloc_array_length(conn.buffer), 350);

	xbapi_free_conn(&conn);
}

static void test_xbapi_process_data2() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	uint8_t buf[] = { 0x7E, 0x00, 0x05, 0x88, 0x01, 0x42, 0x44, 0x02, 0xEE };
	write(fds[1], buf, 9);

	xbapi_conn_t conn = xbapi_init_conn(fds[0]);
	xbapi_rc_t rc;
	xbapi_op_t *ops = talloc_array(NULL, xbapi_op_t, 3);
	for (int i = 0; i < 3; i++) ops[i].status = XBAPI_OP_STATUS_PENDING;
	ops[0].frame_id = 0x07;
	ops[1].frame_id = 0x01;
	ops[2].frame_id = 0x19;

	if( xbapi_errno(rc = xbapi_process_data(&conn, ops)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_xbapi_process_data", rc);

	CU_ASSERT_EQUAL(ops[0].status, XBAPI_OP_STATUS_PENDING);
	CU_ASSERT_EQUAL(ops[1].status, XBAPI_OP_STATUS_INVALID_CMD);
	CU_ASSERT_EQUAL(ops[2].status, XBAPI_OP_STATUS_PENDING);
	xbapi_free_conn(&conn);
}

static void test_xbapi_process_data3() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	uint8_t buf[] = { 0x7E, 0x00, 0x07, 0x88, 0x47, 0x42, 0x44, 0x00, 0x76, 0xA4, 0x90 };
	write(fds[1], buf, 11);

	xbapi_conn_t conn = xbapi_init_conn(fds[0]);
	xbapi_rc_t rc;
	xbapi_op_t *ops = talloc_array(NULL, xbapi_op_t, 3);
	for (int i = 0; i < 3; i++) ops[i].status = XBAPI_OP_STATUS_PENDING;
	ops[0].frame_id = 0x07;
	ops[1].frame_id = 0x01;
	ops[2].frame_id = 0x47;

	if( xbapi_errno(rc = xbapi_process_data(&conn, ops)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_xbapi_process_data", rc);

	CU_ASSERT_EQUAL(ops[0].status, XBAPI_OP_STATUS_PENDING);
	CU_ASSERT_EQUAL(ops[1].status, XBAPI_OP_STATUS_PENDING);
	CU_ASSERT_EQUAL(ops[2].status, XBAPI_OP_STATUS_OK);

	size_t data_len = talloc_array_length(ops[2].data);
	CU_ASSERT_EQUAL(data_len, 2);

	uint8_t expected[] = { 0x76, 0xA4 };
	for (size_t i = 0; i < data_len; i++) CU_ASSERT_EQUAL(ops[2].data[i], expected[i]);

	xbapi_free_conn(&conn);
}

static void test_xbapi_process_data4() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	uint8_t buf[] = { 0x7E, 0x00, 0x09, 0x88, 0x52, 0x53, 0x48, 0x00, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0xD5 };

	xbapi_conn_t conn = xbapi_init_conn(fds[0]);
	xbapi_rc_t rc;
	xbapi_op_t *ops = talloc_array(NULL, xbapi_op_t, 3);
	for (int i = 0; i < 3; i++) ops[i].status = XBAPI_OP_STATUS_PENDING;
	ops[0].frame_id = 0x52;
	ops[1].frame_id = 0x01;
	ops[2].frame_id = 0x11;

	//Write the data in two chucks seperated by an escape
	write(fds[1], buf, 10);
	if( xbapi_errno(rc = xbapi_process_data(&conn, ops)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_xbapi_process_data", rc);

	write(fds[1], buf + 10, 4);
	if( xbapi_errno(rc = xbapi_process_data(&conn, ops)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_xbapi_process_data", rc);

	// Check the statuses
	CU_ASSERT_EQUAL(ops[0].status, XBAPI_OP_STATUS_OK);
	CU_ASSERT_EQUAL(ops[1].status, XBAPI_OP_STATUS_PENDING);
	CU_ASSERT_EQUAL(ops[2].status, XBAPI_OP_STATUS_PENDING);

	size_t data_len = talloc_array_length(ops[0].data);
	CU_ASSERT_EQUAL(data_len, 4);

	uint8_t expected[] = { 0x00, 0x13, 0xA2, 0x00 };
	for (size_t i = 0; i < data_len; i++) CU_ASSERT_EQUAL(ops[0].data[i], expected[i]);

	xbapi_free_conn(&conn);
}

static void test_xbapi_process_data5() {
	int fds[2];
	if (pipe(fds) == -1) xbapi_die("pipe", xbapi_rc_sys());

	uint8_t buf[] = { 0x7E, 0x00, 0x07, 0x88, 0x47, 0x42, 0x44, 0x00, 0x76, 0xA4, 0x90, 0x7E, 0x00, 0x05, 0x88, 0x01, 0x00, 0x00, 0x01, 0x75 };
	write(fds[1], buf, 20);

	xbapi_conn_t conn = xbapi_init_conn(fds[0]);
	xbapi_rc_t rc;
	xbapi_op_t *ops = talloc_array(NULL, xbapi_op_t, 3);
	for (int i = 0; i < 3; i++) ops[i].status = XBAPI_OP_STATUS_PENDING;
	ops[0].frame_id = 0x07;
	ops[1].frame_id = 0x01;
	ops[2].frame_id = 0x47;

	if( xbapi_errno(rc = xbapi_process_data(&conn, ops)) != XBAPI_ERR_NOERR )
		xbapi_die("xbapi_xbapi_process_data", rc);

	CU_ASSERT_EQUAL(ops[0].status, XBAPI_OP_STATUS_PENDING);
	CU_ASSERT_EQUAL(ops[1].status, XBAPI_OP_STATUS_ERROR);
	CU_ASSERT_EQUAL(ops[2].status, XBAPI_OP_STATUS_OK);

	size_t data_len = talloc_array_length(ops[2].data);
	CU_ASSERT_EQUAL(data_len, 2);

	uint8_t expected[] = { 0x76, 0xA4 };
	for (size_t i = 0; i < data_len; i++) CU_ASSERT_EQUAL(ops[2].data[i], expected[i]);

	xbapi_free_conn(&conn);
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
	if( CU_ADD_TEST(suite, test_xbapi_set_at_param1) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_set_at_param2) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_set_at_param3) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_set_at_param4) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_query_at_param1) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_query_at_param2) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_process_data1) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_process_data2) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_process_data3) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_process_data4) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_ADD_TEST(suite, test_xbapi_process_data5) == NULL ) CU_die("CU_ADD_TEST");
}

