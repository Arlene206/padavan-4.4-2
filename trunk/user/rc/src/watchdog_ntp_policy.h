#ifndef WATCHDOG_NTP_POLICY_H
#define WATCHDOG_NTP_POLICY_H

#include <errno.h>

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

/* Preserve the legacy one-server mirror and pool.ntp.org fallback behavior. */
static inline int
watchdog_ntp_resolve_server_pair(char *server0, size_t server0_size,
                                 char *server1, size_t server1_size)
{
    const char *fallback = "pool.ntp.org";
    size_t length;

    if (!server0 || server0_size == 0 || !server1 || server1_size == 0)
        return EINVAL;

    if (strlen(server0) < 3) {
        length = strlen(server1);
        if (length >= server0_size)
            return ENAMETOOLONG;
        memmove(server0, server1, length + 1);
    } else if (strlen(server1) < 3) {
        length = strlen(server0);
        if (length >= server1_size)
            return ENAMETOOLONG;
        memmove(server1, server0, length + 1);
    }

    if (strlen(server0) < 3) {
        length = strlen(fallback);
        if (length >= server0_size || length >= server1_size)
            return ENAMETOOLONG;
        memcpy(server0, fallback, length + 1);
        memcpy(server1, fallback, length + 1);
    }

    return 0;
}

#endif /* WATCHDOG_NTP_POLICY_H */
