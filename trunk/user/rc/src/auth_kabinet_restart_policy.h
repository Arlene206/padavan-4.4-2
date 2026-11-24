#ifndef AUTH_KABINET_RESTART_POLICY_H
#define AUTH_KABINET_RESTART_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

#define AUTH_KABINET_GATEWAY_KEY "wan_auth_host"
#define AUTH_KABINET_PASSWORD_KEY "wan_auth_pass"

enum auth_kabinet_restart_stage {
    AUTH_KABINET_RESTART_STAGE_NONE = 0,
    AUTH_KABINET_RESTART_STAGE_READ_GATEWAY,
    AUTH_KABINET_RESTART_STAGE_READ_PASSWORD,
    AUTH_KABINET_RESTART_STAGE_STOP,
    AUTH_KABINET_RESTART_STAGE_EMPTY_PASSWORD,
    AUTH_KABINET_RESTART_STAGE_START
};

typedef int (*auth_kabinet_read_value_fn)(void *context, const char *name,
                                          char *value, size_t value_size);
typedef int (*auth_kabinet_stop_service_fn)(void *context);
typedef int (*auth_kabinet_start_service_fn)(void *context,
                                             const char *gateway,
                                             const char *password);

/*
 * Read the complete configuration before interrupting a working daemon.
 * A readable empty password retains the legacy stop/no-start behavior, while
 * an NVRAM read error leaves the existing daemon untouched and retryable.
 */
static int
auth_kabinet_restart_with_ops(auth_kabinet_read_value_fn read_value,
                              auth_kabinet_stop_service_fn stop_service,
                              auth_kabinet_start_service_fn start_service,
                              void *context,
                              char *gateway, size_t gateway_size,
                              char *password, size_t password_size,
                              enum auth_kabinet_restart_stage *failure_stage)
{
    int status;

    if (failure_stage)
        *failure_stage = AUTH_KABINET_RESTART_STAGE_NONE;
    if (gateway && gateway_size)
        gateway[0] = '\0';
    if (password && password_size)
        password[0] = '\0';
    if (!read_value || !stop_service || !start_service ||
        !gateway || gateway_size == 0 || !password || password_size == 0)
        return EINVAL;

    status = read_value(context, AUTH_KABINET_GATEWAY_KEY,
                        gateway, gateway_size);
    if (status != 0) {
        if (failure_stage)
            *failure_stage = AUTH_KABINET_RESTART_STAGE_READ_GATEWAY;
        goto out;
    }
    if (!memchr(gateway, '\0', gateway_size)) {
        status = ENAMETOOLONG;
        if (failure_stage)
            *failure_stage = AUTH_KABINET_RESTART_STAGE_READ_GATEWAY;
        goto out;
    }

    status = read_value(context, AUTH_KABINET_PASSWORD_KEY,
                        password, password_size);
    if (status != 0) {
        if (failure_stage)
            *failure_stage = AUTH_KABINET_RESTART_STAGE_READ_PASSWORD;
        goto out;
    }
    if (!memchr(password, '\0', password_size)) {
        status = ENAMETOOLONG;
        if (failure_stage)
            *failure_stage = AUTH_KABINET_RESTART_STAGE_READ_PASSWORD;
        goto out;
    }

    status = stop_service(context);
    if (status != 0) {
        if (failure_stage)
            *failure_stage = AUTH_KABINET_RESTART_STAGE_STOP;
        goto out;
    }

    if (password[0] == '\0') {
        status = -1;
        if (failure_stage)
            *failure_stage = AUTH_KABINET_RESTART_STAGE_EMPTY_PASSWORD;
        goto out;
    }

    status = start_service(context, gateway, password);
    if (status != 0 && failure_stage)
        *failure_stage = AUTH_KABINET_RESTART_STAGE_START;

out:
    memset(password, 0, password_size);
    memset(gateway, 0, gateway_size);
    return status;
}

#endif
