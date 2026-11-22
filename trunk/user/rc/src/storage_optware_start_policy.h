#ifndef PADAVAN_STORAGE_OPTWARE_START_POLICY_H
#define PADAVAN_STORAGE_OPTWARE_START_POLICY_H

#include <fcntl.h>

/*
 * PID 1 keeps the read end until the worker reports completion. Mark both
 * descriptors close-on-exec so subsequent daemon launches cannot leak the
 * result channel into unrelated long-lived processes.
 */
static inline int
storage_optware_configure_result_pipe(int read_fd, int write_fd)
{
	int flags;

	flags = fcntl(read_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(read_fd, F_SETFL, flags | O_NONBLOCK) != 0)
		return -1;

	flags = fcntl(read_fd, F_GETFD);
	if (flags < 0 || fcntl(read_fd, F_SETFD, flags | FD_CLOEXEC) != 0)
		return -1;

	flags = fcntl(write_fd, F_GETFD);
	if (flags < 0 || fcntl(write_fd, F_SETFD, flags | FD_CLOEXEC) != 0)
		return -1;

	return 0;
}

enum storage_optware_start_child_result {
	STORAGE_OPTWARE_START_CHILD_FAILURE = -1,
	STORAGE_OPTWARE_START_CHILD_RUNNING = 0,
	STORAGE_OPTWARE_START_CHILD_SUCCESS = 1,
	STORAGE_OPTWARE_START_CHILD_NONE = 2
};

/* Decode the one-byte result channel used by the asynchronous Optware worker. */
static int
storage_optware_start_decode_result(int bytes_read, int child_status)
{
	if (bytes_read < 0)
		return STORAGE_OPTWARE_START_CHILD_RUNNING;
	if (bytes_read != 1)
		return STORAGE_OPTWARE_START_CHILD_FAILURE;

	return child_status == 0 ? STORAGE_OPTWARE_START_CHILD_SUCCESS :
		STORAGE_OPTWARE_START_CHILD_FAILURE;
}

#define STORAGE_OPTWARE_RETRY_INITIAL_SECONDS 5U
#define STORAGE_OPTWARE_RETRY_MAX_SECONDS 300U

/* Double the retry interval up to five minutes; zero/invalid starts at 5s. */
static inline unsigned int
storage_optware_retry_next_delay(unsigned int current_delay)
{
	if (current_delay < STORAGE_OPTWARE_RETRY_INITIAL_SECONDS)
		return STORAGE_OPTWARE_RETRY_INITIAL_SECONDS;
	if (current_delay >= STORAGE_OPTWARE_RETRY_MAX_SECONDS / 2U)
		return STORAGE_OPTWARE_RETRY_MAX_SECONDS;
	return current_delay * 2U;
}

enum storage_optware_stop_presence {
	STORAGE_OPTWARE_STOP_UNKNOWN = -1,
	STORAGE_OPTWARE_STOP_ABSENT = 0,
	STORAGE_OPTWARE_STOP_RUNNING = 1
};

static inline int
storage_optware_stop_policy_result(int worker_presence, int group_presence,
				   int operation_status)
{
	if (operation_status != 0)
		return operation_status;
	if (worker_presence != STORAGE_OPTWARE_STOP_ABSENT ||
	    group_presence != STORAGE_OPTWARE_STOP_ABSENT)
		return -1;
	return 0;
}

/*
 * UBI detach is complete only when the pre-state is known and the post-state
 * confirms that the RWFS UBI device is absent.
 */
static inline int
storage_optware_ubi_detach_complete(int before, int after)
{
	return (before == STORAGE_OPTWARE_STOP_ABSENT ||
		before == STORAGE_OPTWARE_STOP_RUNNING) &&
		after == STORAGE_OPTWARE_STOP_ABSENT;
}

/*
 * Keep a successful Optware child completion latched until its request
 * marker is confirmed clear. A failed acknowledgement must not cause the
 * next deferred retry to launch the same worker again in this boot.
 */
static inline int
storage_optware_ack_state(int marker_pending, int clear_status,
			 int *ack_pending)
{
	if (!ack_pending)
		return EINVAL;
	if (!marker_pending || clear_status == 0) {
		*ack_pending = 0;
		return 0;
	}
	*ack_pending = 1;
	return clear_status > 0 ? clear_status : EIO;
}

#endif /* PADAVAN_STORAGE_OPTWARE_START_POLICY_H */
