#ifndef INIT_MDEV_RETRY_POLICY_H
#define INIT_MDEV_RETRY_POLICY_H
#define INIT_MDEV_RETRY_DELAY_SECONDS 5
typedef struct { int pending; } init_mdev_retry_state_t;
static inline void
init_mdev_retry_state_begin(init_mdev_retry_state_t *state,
			    int dev_init_status, int mdev_init_status)
{
	if (state)
		state->pending = (dev_init_status == 0 && mdev_init_status != 0);
}
static inline int
init_mdev_retry_state_is_pending(const init_mdev_retry_state_t *state)
{
	return state && state->pending != 0;
}
static inline int
init_mdev_retry_state_record_result(init_mdev_retry_state_t *state,
				    int mdev_init_status)
{
	if (!state || !state->pending)
		return 0;
	if (mdev_init_status == 0)
		state->pending = 0;
	return state->pending;
}
#endif /* INIT_MDEV_RETRY_POLICY_H */
