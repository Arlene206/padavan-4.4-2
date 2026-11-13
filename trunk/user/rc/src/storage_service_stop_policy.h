#ifndef STORAGE_SERVICE_STOP_POLICY_H
#define STORAGE_SERVICE_STOP_POLICY_H

/* A storage-backed daemon is stopped only when its stop command succeeded
 * and the daemon is no longer observable. Either uncertainty blocks eject. */
static inline int
storage_service_stop_complete(int command_status, int daemon_running)
{
	if (command_status != 0 || daemon_running != 0)
		return -1;

	return 0;
}


/* Preserve the first failure while still attempting every daemon in a group. */
static inline int
storage_service_stop_status_merge(int current_status, int next_status)
{
	return current_status != 0 ? current_status : next_status;
}

#endif
