#ifndef RC_NOTIFY_BATCH_POLICY_H
#define RC_NOTIFY_BATCH_POLICY_H

#define RC_NOTIFY_BATCH_LIMIT 10

/* Requeue only after a full bounded batch when safe work remains. */
static inline int
rc_notify_batch_should_requeue(unsigned int entries_seen, int queue_pending,
			       int stop_handle)
{
	return !stop_handle && queue_pending &&
	       entries_seen >= RC_NOTIFY_BATCH_LIMIT;
}

/* Notification handlers retain the incomplete marker only for negative
 * action results. Normalize errno-style positive service failures as well. */
static inline int
rc_notify_merge_action_result(int current_result, int operation_status)
{
	return (current_result < 0 || operation_status != 0) ? -1 : 0;
}

#endif /* RC_NOTIFY_BATCH_POLICY_H */
