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

#endif /* PADAVAN_STORAGE_OPTWARE_START_POLICY_H */
