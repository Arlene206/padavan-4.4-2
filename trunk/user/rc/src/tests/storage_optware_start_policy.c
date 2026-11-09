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

	assert(storage_optware_retry_next_delay(0) == 5U);
	assert(storage_optware_retry_next_delay(5U) == 10U);
	assert(storage_optware_retry_next_delay(10U) == 20U);
	assert(storage_optware_retry_next_delay(20U) == 40U);
	assert(storage_optware_retry_next_delay(40U) == 80U);
	assert(storage_optware_retry_next_delay(80U) == 160U);
	assert(storage_optware_retry_next_delay(160U) == 300U);
	assert(storage_optware_retry_next_delay(300U) == 300U);
	assert(storage_optware_retry_next_delay(1000U) == 300U);

	puts("deferred Optware child-result and retry policy: PASS");
	return 0;
}
