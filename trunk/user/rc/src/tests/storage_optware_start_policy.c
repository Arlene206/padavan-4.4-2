#include <assert.h>
#include <stdio.h>

#include "storage_optware_start_policy.h"

int
main(void)
{
	assert(storage_optware_start_decode_result(-1, 0) ==
	       STORAGE_OPTWARE_START_CHILD_RUNNING);
	assert(storage_optware_start_decode_result(1, 0) ==
	       STORAGE_OPTWARE_START_CHILD_SUCCESS);
	assert(storage_optware_start_decode_result(1, 1) ==
	       STORAGE_OPTWARE_START_CHILD_FAILURE);
	assert(storage_optware_start_decode_result(0, 0) ==
	       STORAGE_OPTWARE_START_CHILD_FAILURE);
	assert(storage_optware_start_decode_result(2, 0) ==
	       STORAGE_OPTWARE_START_CHILD_FAILURE);

	puts("deferred Optware child-result policy: PASS");
	return 0;
}
