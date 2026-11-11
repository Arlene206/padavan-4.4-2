#ifndef WATCHDOG_NTP_POLICY_H
#define WATCHDOG_NTP_POLICY_H

/* Do not overlap one-shot clock synchronization with an ntpd that survived
 * the bounded TERM/KILL lifecycle stop. */
static inline int
watchdog_ntp_refresh_should_launch(int previous_stop_status)
{
	return previous_stop_status == 0;
}

#endif /* WATCHDOG_NTP_POLICY_H */
