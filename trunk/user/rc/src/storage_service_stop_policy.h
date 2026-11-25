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


/* A fresh failed launch is still considered active unless absence is proven. */
static inline int
storage_service_failed_start_may_leave_daemon(int process_state)
{
	return process_state != 0; /* PROCESS_PRESENCE_ABSENT */
}

/* UNKNOWN before/after requires conservative firewall reconciliation. */
static inline int
storage_service_process_presence_changed(int before, int after)
{
	return before < 0 || after < 0 || before != after;
}

/* Preserve the first failure while still attempting every daemon in a group. */
static inline int
storage_service_stop_status_merge(int current_status, int next_status)
{
	return current_status != 0 ? current_status : next_status;
}

/* A successful unmount call is insufficient if mounts remain or the final
 * /proc/mounts inventory is unknown. */
static inline int
storage_safe_remove_complete(int path_status, int fallback_status, int remaining_mounts)
{
	if (path_status != 0 || fallback_status != 0 || remaining_mounts != 0)
		return -1;
	return 0;
}

#endif
