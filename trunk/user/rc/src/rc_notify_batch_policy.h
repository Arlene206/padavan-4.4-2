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

#endif /* RC_NOTIFY_BATCH_POLICY_H */
