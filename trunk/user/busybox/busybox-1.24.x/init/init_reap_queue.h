#ifndef PADAVAN_BUSYBOX_INIT_REAP_QUEUE_H
#define PADAVAN_BUSYBOX_INIT_REAP_QUEUE_H

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>

/*
 * The SIGTSTP handler may reap children while PID 1 is paused. It must not
 * update utmp or traverse the mutable inittab action list in signal context.
 * Store only PIDs here and drain the queue in the normal main-loop context.
 *
 * A full queue intentionally stops further handler-side reaping. Remaining
 * exited children stay waitable and are collected by the main loop on SIGCONT.
 */
#define INIT_REAP_QUEUE_CAPACITY 64

struct init_reap_queue {
	volatile sig_atomic_t count;
	volatile sig_atomic_t pids[INIT_REAP_QUEUE_CAPACITY];
};

/*
 * Wait for one direct child without consuming a different child. ECHILD is a
 * normal completion state here: the SIGTSTP handler may already have reaped
 * this PID into pending_reaped_children, which the caller drains afterward.
 */
typedef pid_t (*init_waitpid_fn)(pid_t, int *, int);

static inline int
init_wait_for_child(pid_t target_pid, init_waitpid_fn wait_child,
		    int *child_reaped)
{
	pid_t result;

	if (child_reaped)
		*child_reaped = 0;
	if (target_pid <= 0 || !wait_child || !child_reaped)
		return EINVAL;

	for (;;) {
		errno = 0;
		result = wait_child(target_pid, NULL, 0);
		if (result == target_pid) {
			*child_reaped = 1;
			return 0;
		}
		if (result < 0 && errno == EINTR)
			return EINTR;
		if (result < 0 && errno == ECHILD)
			return 0;
		if (result < 0)
			return errno > 0 ? errno : EIO;
		return EIO;
	}
}

static inline int
init_reap_queue_has_capacity(const struct init_reap_queue *queue)
{
	return queue != NULL
		&& queue->count >= 0
		&& queue->count < INIT_REAP_QUEUE_CAPACITY;
}

/* Return 1 when queued, 0 when full, and -1 for invalid input. */
static inline int
init_reap_queue_push(struct init_reap_queue *queue, pid_t pid)
{
	sig_atomic_t count;

	if (!queue || pid <= 0)
		return -1;
	if ((pid_t)(sig_atomic_t)pid != pid)
		return -1;

	count = queue->count;
	if (count < 0 || count >= INIT_REAP_QUEUE_CAPACITY)
		return 0;

	queue->pids[count] = (sig_atomic_t)pid;
	queue->count = count + 1;
	return 1;
}

/*
 * Drain only while SIGTSTP is blocked by the caller. If the destination is
 * too small or queue state is invalid, leave all entries untouched.
 */
static inline sig_atomic_t
init_reap_queue_drain(struct init_reap_queue *queue, pid_t *pids,
		      size_t capacity)
{
	sig_atomic_t count, i;

	if (!queue)
		return -1;
	count = queue->count;
	if (count < 0 || count > INIT_REAP_QUEUE_CAPACITY)
		return -1;
	if (capacity < (size_t)count || (!pids && count != 0))
		return -1;

	for (i = 0; i < count; i++)
		pids[i] = (pid_t)queue->pids[i];
	queue->count = 0;
	return count;
}

#endif /* PADAVAN_BUSYBOX_INIT_REAP_QUEUE_H */
