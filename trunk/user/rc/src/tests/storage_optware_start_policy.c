#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "storage_optware_start_policy.h"

static void
test_result_pipe_descriptors_are_nonblocking_and_cloexec(void)
{
	int descriptors[2], flags;

	assert(pipe(descriptors) == 0);
	assert(storage_optware_configure_result_pipe(descriptors[0],
						     descriptors[1]) == 0);

	flags = fcntl(descriptors[0], F_GETFL, 0);
	assert(flags >= 0 && (flags & O_NONBLOCK) != 0);
	flags = fcntl(descriptors[0], F_GETFD);
	assert(flags >= 0 && (flags & FD_CLOEXEC) != 0);
	flags = fcntl(descriptors[1], F_GETFD);
	assert(flags >= 0 && (flags & FD_CLOEXEC) != 0);

	assert(close(descriptors[0]) == 0);
	assert(close(descriptors[1]) == 0);
}

static void
test_result_pipe_configuration_rejects_invalid_descriptors(void)
{
	int descriptors[2];

	assert(pipe(descriptors) == 0);
	assert(storage_optware_configure_result_pipe(-1, descriptors[1]) == -1);
	assert(close(descriptors[0]) == 0);
	assert(close(descriptors[1]) == 0);
}

int
main(void)
{
	test_result_pipe_descriptors_are_nonblocking_and_cloexec();
	test_result_pipe_configuration_rejects_invalid_descriptors();
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

{
		int ack_pending = 0;

		assert(storage_optware_ack_state(1, 0, &ack_pending) == 0);
		assert(ack_pending == 0);
		assert(storage_optware_ack_state(1, EACCES, &ack_pending) == EACCES);
		assert(ack_pending == 1);
		assert(storage_optware_ack_state(1, 0, &ack_pending) == 0);
		assert(ack_pending == 0);
		assert(storage_optware_ack_state(1, -1, &ack_pending) == EIO);
		assert(ack_pending == 1);
		assert(storage_optware_ack_state(0, EIO, &ack_pending) == 0);
		assert(ack_pending == 0);
		assert(storage_optware_ack_state(1, EIO, NULL) == EINVAL);
	}

	puts("deferred Optware child-result, acknowledgement retry, fail-closed stop, and UBI detach policy: PASS");
	return 0;
}
