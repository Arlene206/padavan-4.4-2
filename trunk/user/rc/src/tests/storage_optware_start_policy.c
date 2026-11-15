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

	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_ABSENT,
						 STORAGE_OPTWARE_STOP_ABSENT, 0) == 0);
	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_RUNNING,
						 STORAGE_OPTWARE_STOP_ABSENT, 0) == -1);
	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_ABSENT,
						 STORAGE_OPTWARE_STOP_RUNNING, 0) == -1);
	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_UNKNOWN,
						 STORAGE_OPTWARE_STOP_ABSENT, 0) == -1);
	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_ABSENT,
						 STORAGE_OPTWARE_STOP_UNKNOWN, 0) == -1);
	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_ABSENT,
						 STORAGE_OPTWARE_STOP_ABSENT, 5) == 5);
	assert(storage_optware_stop_policy_result(STORAGE_OPTWARE_STOP_RUNNING,
						 STORAGE_OPTWARE_STOP_RUNNING, 13) == 13);

	assert(storage_optware_ubi_detach_complete(STORAGE_OPTWARE_STOP_ABSENT,
						 STORAGE_OPTWARE_STOP_ABSENT));
	assert(storage_optware_ubi_detach_complete(STORAGE_OPTWARE_STOP_RUNNING,
						 STORAGE_OPTWARE_STOP_ABSENT));
	assert(!storage_optware_ubi_detach_complete(STORAGE_OPTWARE_STOP_RUNNING,
						  STORAGE_OPTWARE_STOP_RUNNING));
	assert(!storage_optware_ubi_detach_complete(STORAGE_OPTWARE_STOP_RUNNING,
						  STORAGE_OPTWARE_STOP_UNKNOWN));
	assert(!storage_optware_ubi_detach_complete(STORAGE_OPTWARE_STOP_UNKNOWN,
						  STORAGE_OPTWARE_STOP_ABSENT));

	puts("deferred Optware child-result, retry, fail-closed stop, and UBI detach policy: PASS");
	return 0;
}
