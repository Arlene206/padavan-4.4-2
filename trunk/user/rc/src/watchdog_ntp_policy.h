#ifndef WATCHDOG_NTP_POLICY_H
#define WATCHDOG_NTP_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

/* Do not overlap one-shot clock synchronization with an ntpd that survived
 * the bounded TERM/KILL lifecycle stop. */
static inline int
watchdog_ntp_refresh_should_launch(int previous_stop_status)
{
	return previous_stop_status == 0;
}

static inline int
watchdog_time_notification_error(int signal_status, int signal_errno,
				 int start_status, int start_errno)
{
	if (signal_status == 0)
		return 0;
	if (signal_errno == ENOENT || signal_errno == ESRCH) {
		if (start_status == 0)
			return 0;
		if (start_errno > 0)
			return start_errno;
		if (start_status > 0)
			return start_status;
		return EIO;
	}
	if (signal_errno > 0)
		return signal_errno;
	if (signal_status > 0)
		return signal_status;
	return EIO;
}


/* Do not treat an empty/missing offset as proof of a completed time step. */
static inline int
watchdog_ntp_step_has_offset(const char *offset)
{
	return offset && offset[0] != '\0';
}

/* A recorded step with ntp_ready still clear requires another persistence try. */
static inline int
watchdog_ntp_ready_retry_required(int ntpc_counter, int ntp_ready)
{
	return ntpc_counter > 0 && ntp_ready != 1;
}

/* NTP readiness writes require both state reads to have completed cleanly. */
static inline int
watchdog_ntp_ready_decision(int counter_status, int ntpc_counter,
			    int ready_status, int ntp_ready,
			    int *retry_required)
{
	if (!retry_required)
		return EINVAL;
	*retry_required = 0;
	if (counter_status != 0)
		return counter_status;
	if (ready_status != 0)
		return ready_status;
	*retry_required = watchdog_ntp_ready_retry_required(
		ntpc_counter, ntp_ready);
	return 0;
}


/*
 * The step counter is a durable retry marker, not an unbounded arithmetic
 * accumulator. Reject corrupt negative state and saturate instead of
 * overflowing signed int after long uptimes or repeated clock corrections.
 */
static inline int
watchdog_ntp_counter_advance(int current, int *next)
{
	if (!next || current < 0)
		return EINVAL;
	*next = (current == INT_MAX) ? INT_MAX : current + 1;
	return 0;
}

/*
 * A transient NVRAM read failure must not be converted to deadline zero:
 * defer this one check and retry on the next watchdog timer cycle.
 */
static inline int
watchdog_deferred_wanup_due(int read_status, long deadline,
			    long current_uptime, int *due)
{
	if (!due)
		return EINVAL;
	*due = 0;
	if (read_status != 0)
		return read_status;
	*due = (deadline > 0 && current_uptime >= deadline);
	return 0;
}

#define WATCHDOG_NTP_SERVER_MAX 256

typedef int (*watchdog_ntp_nvram_get_fn)(void *context,
                                         const char *name, char **value);

/*
 * Read and copy one NVRAM string while the caller holds a transaction lease.
 * nvram_transaction_get() returns 1 for present, 0 for absent and -1 for I/O
 * failure; its returned cache pointer is invalidated by the next read.
 */
static inline int
watchdog_ntp_copy_server_value(watchdog_ntp_nvram_get_fn get_value,
                               void *context, const char *name,
                               char *destination, size_t destination_size)
{
    char *value = NULL;
    size_t length;
    int result;

    if (!get_value || !name || !destination || destination_size == 0)
        return EINVAL;

    destination[0] = '\0';
    errno = 0;
    result = get_value(context, name, &value);
    if (result == 0)
        return 0;
    if (result < 0)
        return errno > 0 ? errno : EIO;
    if (result != 1 || !value)
        return EIO;

    length = strlen(value);
    if (length >= destination_size)
        return ENAMETOOLONG;
    memcpy(destination, value, length + 1);
    return 0;
}

/* Read both server keys; never expose a partially read pair on failure. */
static inline int
watchdog_ntp_read_server_pair(watchdog_ntp_nvram_get_fn get_value,
                              void *context,
                              char *server0, size_t server0_size,
                              char *server1, size_t server1_size)
{
    int status;

    if (!server0 || server0_size == 0 || !server1 || server1_size == 0)
        return EINVAL;
    server0[0] = '\0';
    server1[0] = '\0';

    status = watchdog_ntp_copy_server_value(get_value, context,
                                            "ntp_server0",
                                            server0, server0_size);
    if (status == 0)
        status = watchdog_ntp_copy_server_value(get_value, context,
                                                "ntp_server1",
                                                server1, server1_size);
    if (status != 0) {
        server0[0] = '\0';
        server1[0] = '\0';
    }
    return status;
}

/* Find a terminator without reading beyond the caller's buffer. */
static inline int
watchdog_ntp_bounded_string_length(const char *value, size_t value_size,
                                   size_t *length)
{
    const char *terminator;

    if (!value || value_size == 0 || !length)
        return EINVAL;
    terminator = (const char *)memchr(value, '\\0', value_size);
    if (!terminator)
        return ENAMETOOLONG;
    *length = (size_t)(terminator - value);
    return 0;
}

/* Preserve the legacy one-server mirror and pool.ntp.org fallback behavior. */
static inline int
watchdog_ntp_resolve_server_pair(char *server0, size_t server0_size,
                                 char *server1, size_t server1_size)
{
    const char *fallback = "pool.ntp.org";
    size_t server0_length, server1_length, fallback_length;
    int status;

    status = watchdog_ntp_bounded_string_length(
        server0, server0_size, &server0_length);
    if (status == 0)
        status = watchdog_ntp_bounded_string_length(
            server1, server1_size, &server1_length);
    if (status != 0)
        return status;

    if (server0_length < 3 && server1_length >= 3) {
        if (server1_length >= server0_size)
            return ENAMETOOLONG;
        memmove(server0, server1, server1_length + 1);
        return 0;
    }

    if (server1_length < 3 && server0_length >= 3) {
        if (server0_length >= server1_size)
            return ENAMETOOLONG;
        memmove(server1, server0, server0_length + 1);
        return 0;
    }

    if (server0_length < 3 && server1_length < 3) {
        fallback_length = sizeof("pool.ntp.org") - 1;
        if (fallback_length >= server0_size ||
            fallback_length >= server1_size)
            return ENAMETOOLONG;
        memcpy(server0, fallback, fallback_length + 1);
        memcpy(server1, fallback, fallback_length + 1);
    }

    return 0;
}


#define WATCHDOG_SCHEDULE_SELECTOR_MAX 16
#define WATCHDOG_SCHEDULE_LENGTH 11

struct watchdog_ntp_schedule_snapshot {
	int reboot_enabled;
	int ss_enabled;
	int ntp_ready;
	char reboot_schedule[WATCHDOG_SCHEDULE_LENGTH + 1];
	char ss_schedule[WATCHDOG_SCHEDULE_LENGTH + 1];
};

/* Copy each transaction value before the next read invalidates its cache pointer. */
static inline int
watchdog_ntp_schedule_snapshot_read(watchdog_ntp_nvram_get_fn get_value,
                                    void *context,
                                    struct watchdog_ntp_schedule_snapshot *snapshot)
{
	char reboot_enable[WATCHDOG_SCHEDULE_SELECTOR_MAX];
	char ss_enable[WATCHDOG_SCHEDULE_SELECTOR_MAX];
	char ntp_ready[WATCHDOG_SCHEDULE_SELECTOR_MAX];
	int status;

	if (!snapshot)
		return EINVAL;
	memset(snapshot, 0, sizeof(*snapshot));
	status = watchdog_ntp_copy_server_value(
		get_value, context, "reboot_schedule_enable",
		reboot_enable, sizeof(reboot_enable));
	if (status == 0)
		status = watchdog_ntp_copy_server_value(
			get_value, context, "ss_schedule_enable",
			ss_enable, sizeof(ss_enable));
	if (status == 0)
		status = watchdog_ntp_copy_server_value(
			get_value, context, "ntp_ready",
			ntp_ready, sizeof(ntp_ready));
	if (status == 0) {
		snapshot->reboot_enabled = strcmp(reboot_enable, "1") == 0;
		snapshot->ss_enabled = strcmp(ss_enable, "1") == 0;
		snapshot->ntp_ready = strcmp(ntp_ready, "1") == 0;
	}
	/* Avoid fetching schedule strings for disabled actions or before NTP readiness. */
	if (status == 0 && snapshot->ntp_ready && snapshot->reboot_enabled)
		status = watchdog_ntp_copy_server_value(
			get_value, context, "reboot_schedule",
			snapshot->reboot_schedule, sizeof(snapshot->reboot_schedule));
	if (status == 0 && snapshot->ntp_ready && snapshot->ss_enabled)
		status = watchdog_ntp_copy_server_value(
			get_value, context, "ss_schedule",
			snapshot->ss_schedule, sizeof(snapshot->ss_schedule));
	if (status != 0) {
		memset(snapshot, 0, sizeof(*snapshot));
		return status;
	}
	return 0;
}

#endif /* WATCHDOG_NTP_POLICY_H */
