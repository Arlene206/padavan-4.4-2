#ifndef PADAVAN_STORAGE_OPTWARE_START_POLICY_H
#define PADAVAN_STORAGE_OPTWARE_START_POLICY_H

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

#endif /* PADAVAN_STORAGE_OPTWARE_START_POLICY_H */
