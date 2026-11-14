#ifndef SERVICE_LIFECYCLE_POLICY_H
#define SERVICE_LIFECYCLE_POLICY_H

#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <process_inventory.h>

#define SERVICE_LIFECYCLE_FORCE_WAIT_SECONDS 3

enum service_lifecycle_result {
	SERVICE_LIFECYCLE_STOPPED = 0,
	SERVICE_LIFECYCLE_FAILED = -1,
	SERVICE_LIFECYCLE_UNKNOWN = -2,
	SERVICE_LIFECYCLE_TIMEOUT = -3,
	SERVICE_LIFECYCLE_CHANGED = -4
};

struct service_lifecycle_ops {
	enum process_presence (*scan)(void *context, const char *service_name,
				      struct process_snapshot *snapshot);
	enum process_presence (*validate)(void *context,
					   const char *service_name,
					   const struct process_identity *identity);
	int (*send_signal)(void *context,
			   const struct process_identity *identity,
			   int signal_number);
	void (*sleep_seconds)(void *context, unsigned int seconds);
};

struct service_lifecycle_target {
	const char *name;
	struct process_snapshot initial;
};

static inline int
service_lifecycle_identity_equal(const struct process_identity *left,
				 const struct process_identity *right)
{
	return left->pid == right->pid &&
		left->start_time == right->start_time &&
		left->argv0_hash == right->argv0_hash &&
		left->argv1_hash == right->argv1_hash &&
		left->comm_truncated == right->comm_truncated &&
		strcmp(left->comm, right->comm) == 0;
}

static inline int
service_lifecycle_target_contains(const struct service_lifecycle_target *target,
				  const struct process_identity *identity)
{
	size_t i;

	for (i = 0; i < target->initial.count; i++) {
		if (service_lifecycle_identity_equal(&target->initial.items[i],
						     identity))
			return 1;
	}
	return 0;
}

static inline void
service_lifecycle_release_targets(struct service_lifecycle_target *targets,
				  size_t count)
{
	size_t i;

	if (!targets)
		return;
	for (i = 0; i < count; i++)
		process_snapshot_release(&targets[i].initial);
	free(targets);
}

static inline enum service_lifecycle_result
service_lifecycle_preflight(struct service_lifecycle_target *targets,
			     size_t count,
			     const struct service_lifecycle_ops *ops,
			     void *context, int capture_snapshots)
{
	size_t i, j;
	enum process_presence state;

	/*
	 * Capture the original inventory only once. Before KILL, retain those
	 * identities and compare a fresh inventory to the original plan; never
	 * overwrite the original snapshots with the second pass.
	 */
	if (capture_snapshots) {
		for (i = 0; i < count; i++) {
			state = ops->scan(context, targets[i].name, &targets[i].initial);
			if (state == PROCESS_PRESENCE_UNKNOWN)
				return SERVICE_LIFECYCLE_UNKNOWN;
			if ((state != PROCESS_PRESENCE_RUNNING &&
			     state != PROCESS_PRESENCE_ABSENT) ||
			    targets[i].initial.state != state ||
			    (state == PROCESS_PRESENCE_ABSENT &&
			     targets[i].initial.count != 0) ||
			    (state == PROCESS_PRESENCE_RUNNING &&
			     targets[i].initial.count == 0))
				return SERVICE_LIFECYCLE_UNKNOWN;
		}
	} else {
		for (i = 0; i < count; i++) {
			state = targets[i].initial.state;
			if ((state != PROCESS_PRESENCE_RUNNING &&
			     state != PROCESS_PRESENCE_ABSENT) ||
			    (state == PROCESS_PRESENCE_ABSENT &&
			     targets[i].initial.count != 0) ||
			    (state == PROCESS_PRESENCE_RUNNING &&
			     targets[i].initial.count == 0))
				return SERVICE_LIFECYCLE_UNKNOWN;
		}
	}

	/*
	 * Rescan every service before the first TERM and again before KILL.
	 * Newly appeared/replaced PIDs invalidate the original plan.
	 */
	for (i = 0; i < count; i++) {
		struct process_snapshot current;

		memset(&current, 0, sizeof(current));
		state = ops->scan(context, targets[i].name, &current);
		if (state == PROCESS_PRESENCE_UNKNOWN ||
		    current.state != state) {
			process_snapshot_release(&current);
			return SERVICE_LIFECYCLE_UNKNOWN;
		}
		if (state != PROCESS_PRESENCE_RUNNING &&
		    state != PROCESS_PRESENCE_ABSENT) {
			process_snapshot_release(&current);
			return SERVICE_LIFECYCLE_UNKNOWN;
		}
		for (j = 0; j < current.count; j++) {
			if (!service_lifecycle_target_contains(&targets[i],
							       &current.items[j])) {
				process_snapshot_release(&current);
				return SERVICE_LIFECYCLE_CHANGED;
			}
		}
		process_snapshot_release(&current);
	}

	/* Validate all captured identities before the transaction can signal. */
	for (i = 0; i < count; i++) {
		for (j = 0; j < targets[i].initial.count; j++) {
			state = ops->validate(context, targets[i].name,
					      &targets[i].initial.items[j]);
			if (state == PROCESS_PRESENCE_UNKNOWN)
				return SERVICE_LIFECYCLE_UNKNOWN;
			if (state != PROCESS_PRESENCE_RUNNING &&
			    state != PROCESS_PRESENCE_ABSENT)
				return SERVICE_LIFECYCLE_UNKNOWN;
		}
	}
	return SERVICE_LIFECYCLE_STOPPED;
}

static inline enum service_lifecycle_result
service_lifecycle_signal_targets(struct service_lifecycle_target *targets,
				 size_t count,
				 const struct service_lifecycle_ops *ops,
				 void *context, int signal_number)
{
	size_t i, j;
	enum process_presence state;

	for (i = 0; i < count; i++) {
		for (j = 0; j < targets[i].initial.count; j++) {
			/* Revalidate the exact PID/start-time identity before each signal. */
			state = ops->validate(context, targets[i].name,
					      &targets[i].initial.items[j]);
			if (state == PROCESS_PRESENCE_UNKNOWN)
				return SERVICE_LIFECYCLE_UNKNOWN;
			if (state == PROCESS_PRESENCE_ABSENT)
				continue;
			if (state != PROCESS_PRESENCE_RUNNING)
				return SERVICE_LIFECYCLE_UNKNOWN;
			if (ops->send_signal(context, &targets[i].initial.items[j],
					     signal_number) != 0)
				return SERVICE_LIFECYCLE_FAILED;
		}
	}
	return SERVICE_LIFECYCLE_STOPPED;
}

static inline enum service_lifecycle_result
service_lifecycle_probe_targets(struct service_lifecycle_target *targets,
				 size_t count,
				 const struct service_lifecycle_ops *ops,
				 void *context, int *any_running)
{
	size_t i, j;
	enum process_presence state;

	*any_running = 0;
	for (i = 0; i < count; i++) {
		struct process_snapshot current;

		memset(&current, 0, sizeof(current));
		state = ops->scan(context, targets[i].name, &current);
		if (state == PROCESS_PRESENCE_UNKNOWN ||
		    current.state != state) {
			process_snapshot_release(&current);
			return SERVICE_LIFECYCLE_UNKNOWN;
		}
		if (state != PROCESS_PRESENCE_RUNNING &&
		    state != PROCESS_PRESENCE_ABSENT) {
			process_snapshot_release(&current);
			return SERVICE_LIFECYCLE_UNKNOWN;
		}
		for (j = 0; j < current.count; j++) {
			if (!service_lifecycle_target_contains(&targets[i],
							       &current.items[j])) {
				process_snapshot_release(&current);
				return SERVICE_LIFECYCLE_CHANGED;
			}
			*any_running = 1;
		}
		process_snapshot_release(&current);
	}
	return SERVICE_LIFECYCLE_STOPPED;
}

static inline enum service_lifecycle_result
service_lifecycle_wait_for_exit(struct service_lifecycle_target *targets,
				 size_t count,
				 const struct service_lifecycle_ops *ops,
				 void *context, unsigned int timeout_seconds)
{
	unsigned int waited;
	int any_running;
	enum service_lifecycle_result result;

	for (waited = 0; ; waited++) {
		result = service_lifecycle_probe_targets(targets, count, ops,
							 context, &any_running);
		if (result != SERVICE_LIFECYCLE_STOPPED)
			return result;
		if (!any_running)
			return SERVICE_LIFECYCLE_STOPPED;
		if (waited >= timeout_seconds)
			return SERVICE_LIFECYCLE_TIMEOUT;
		ops->sleep_seconds(context, 1);
	}
}

static inline int
service_lifecycle_allows_followup(enum service_lifecycle_result result)
{
	return result == SERVICE_LIFECYCLE_STOPPED;
}

/*
 * UNKNOWN/CHANGED/FAILED/TIMEOUT are all stop failures. Only STOPPED may
 * authorize restart, destructive cleanup, stopped-state publication,
 * storage release or firmware-update progression.
 */
static inline enum service_lifecycle_result
service_lifecycle_stop(char *service_names[], int wait_timeout, int forcekill,
		       const struct service_lifecycle_ops *ops, void *context)
{
	struct service_lifecycle_target *targets;
	size_t count = 0, i;
	enum service_lifecycle_result result;

	if (!service_names || !ops || !ops->scan || !ops->validate ||
	    !ops->send_signal || !ops->sleep_seconds)
		return SERVICE_LIFECYCLE_FAILED;
	while (service_names[count] && *service_names[count])
		count++;
	if (count == 0)
		return SERVICE_LIFECYCLE_STOPPED;
	if (count > (size_t)-1 / sizeof(*targets))
		return SERVICE_LIFECYCLE_FAILED;

	targets = (struct service_lifecycle_target *)
		calloc(count, sizeof(*targets));
	if (!targets)
		return SERVICE_LIFECYCLE_UNKNOWN;
	for (i = 0; i < count; i++)
		targets[i].name = service_names[i];

	/* No TERM may occur until every service passed this complete preflight. */
	result = service_lifecycle_preflight(targets, count, ops, context, 1);
	if (result != SERVICE_LIFECYCLE_STOPPED)
		goto out;
	result = service_lifecycle_signal_targets(targets, count, ops, context,
						  SIGTERM);
	if (result != SERVICE_LIFECYCLE_STOPPED)
		goto out;

	if (wait_timeout < 1)
		wait_timeout = 1;
	result = service_lifecycle_wait_for_exit(targets, count, ops, context,
						 (unsigned int)wait_timeout);
	if (result == SERVICE_LIFECYCLE_STOPPED)
		goto out;
	if (!forcekill)
		goto out;
	if (result != SERVICE_LIFECYCLE_TIMEOUT)
		goto out;

	/* Re-inventory the entire plan before escalation, then recheck each PID. */
	result = service_lifecycle_preflight(targets, count, ops, context, 0);
	if (result != SERVICE_LIFECYCLE_STOPPED)
		goto out;
	result = service_lifecycle_signal_targets(targets, count, ops, context,
						  SIGKILL);
	if (result != SERVICE_LIFECYCLE_STOPPED)
		goto out;
	result = service_lifecycle_wait_for_exit(targets, count, ops, context,
						 SERVICE_LIFECYCLE_FORCE_WAIT_SECONDS);

out:
	service_lifecycle_release_targets(targets, count);
	return result;
}

#endif
