#ifndef DETECT_LINK_MODEM_RECONNECT_POLICY_H
#define DETECT_LINK_MODEM_RECONNECT_POLICY_H

#define DETECT_LINK_MODEM_RECONNECT_RETRY_TICKS 2UL
#define DETECT_LINK_MODEM_RECONNECT_MAX_RETRIES 2U

/*
 * Return 0 when the reconnect notification is accepted or not needed, 1 when
 * a bounded retry was scheduled, and -1 after the initial attempt plus all
 * retries have failed.
 */
static inline int
detect_link_modem_reconnect_retry_status(int status,
                                         unsigned int *retry_count,
                                         unsigned long current_tick,
                                         unsigned long *next_tick)
{
	if (!retry_count || !next_tick)
		return -1;

	if (status == 0) {
		*retry_count = 0;
		*next_tick = 0;
		return 0;
	}

	if (*retry_count < DETECT_LINK_MODEM_RECONNECT_MAX_RETRIES) {
		++*retry_count;
		*next_tick = current_tick + DETECT_LINK_MODEM_RECONNECT_RETRY_TICKS;
		return 1;
	}

	*retry_count = 0;
	*next_tick = 0;
	return -1;
}

#endif /* DETECT_LINK_MODEM_RECONNECT_POLICY_H */
