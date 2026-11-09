#ifndef WIRELESS_RADIO_POLICY_H
#define WIRELESS_RADIO_POLICY_H

/*
 * A radio operation is considered successful only when the driver command or
 * the enclosing restart transaction returns the conventional zero status.
 */
static inline int
wireless_radio_status_success(int operation_status)
{
	return operation_status == 0;
}

#endif /* WIRELESS_RADIO_POLICY_H */
