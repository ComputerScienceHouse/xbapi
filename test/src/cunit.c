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

#include "cunit.h"
#include <CUnit/Basic.h>

#include "test_xbapi.h"

__attribute__((noreturn, nonnull))
void CU_die( const char *ctx ) {
	assert(CU_get_error() != CUE_SUCCESS);
	fprintf(stderr, "%s: %s\n", ctx, CU_get_error_msg());
	exit(EXIT_FAILURE);
}

int main() {
	//talloc_enable_leak_report_full();
	if( CU_initialize_registry() != CUE_SUCCESS ) CU_die("CU_initialize_registry");
	xbapi_add_suite();
	if( CU_basic_run_tests() != CUE_SUCCESS ) CU_die("CU_basic_run_tests");
	int ret = CU_get_number_of_failure_records() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
	CU_cleanup_registry();
	return ret;
}
