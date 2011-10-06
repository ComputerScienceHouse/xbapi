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

#include "xbapi.h"

#include "test_xbapi.h"

__attribute__((noreturn, nonnull))
void xbapi_die( const char *ctx, xbapi_rc_t rc ) {
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

int main() {
	//talloc_enable_leak_report_full();
	if( CU_initialize_registry() != CUE_SUCCESS ) CU_die("CU_initialize_registry");
	CU_pSuite suite;
	if( (suite = CU_add_suite("xbapi", NULL, NULL)) == NULL ) CU_die("CU_add_suite");
	if( CU_ADD_TEST(suite, test_xbapi_escape) == NULL ) CU_die("CU_ADD_TEST");
	if( CU_basic_run_tests() != CUE_SUCCESS ) CU_die("CU_basic_run_tests");
	CU_cleanup_registry();
	return EXIT_SUCCESS;
}
