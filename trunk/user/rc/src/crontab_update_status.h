#ifndef RC_CRONTAB_UPDATE_STATUS_H
#define RC_CRONTAB_UPDATE_STATUS_H

#include <errno.h>
#include <sys/wait.h>

/* Decode system()'s wait status into the script's 0/1/2+ result contract. */
static inline int
rc_crontab_update_status_from_wait(int wait_status, int wait_errno)
{
	if (wait_status == -1)
		/* Negative errors cannot collide with exit 1 (successful append). */
		return wait_errno > 0 ? -wait_errno : -1;
	if (wait_status < 0 || !WIFEXITED(wait_status))
		return EIO;
	return WEXITSTATUS(wait_status);
}

/* Preserve the first error; otherwise remember whether any entry was added. */
static inline int
rc_crontab_update_status_merge(int first_status, int next_status)
{
	if (first_status != 0 && first_status != 1)
		return first_status;
	if (next_status != 0 && next_status != 1)
		return next_status;
	return first_status == 1 || next_status == 1 ? 1 : 0;
}

static inline int
rc_crontab_update_requires_persistence(int status)
{
	return status == 1;
}

#endif
