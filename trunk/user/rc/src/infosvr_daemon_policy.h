#ifndef INFOSVR_DAEMON_POLICY_H
#define INFOSVR_DAEMON_POLICY_H

/* Do not launch a replacement until the previous infosvr is confirmed gone. */
static inline int
infosvr_restart_can_start(int stop_status, int process_present)
{
	return stop_status == 0 && process_present == 0;
}

#endif
