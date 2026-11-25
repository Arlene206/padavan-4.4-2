#ifndef AP_MODE_READ_POLICY_H
#define AP_MODE_READ_POLICY_H

#include <errno.h>
#include <string.h>

/* Missing wan_route_x preserves the legacy router-mode default. */
static inline int
rc_ap_mode_read_status(int api_result, int saved_errno, const char *value,
                       int *is_ap_mode)
{
    int mode;

    if (!is_ap_mode)
        return EINVAL;
    if (api_result < 0)
        return saved_errno > 0 ? saved_errno : EIO;

    if (api_result == 0) {
        if (value)
            return EIO;
        mode = 0;
    } else if (api_result == 1) {
        if (!value)
            return EIO;
        mode = strcmp(value, "IP_Bridged") == 0;
    } else {
        return EIO;
    }

    *is_ap_mode = mode;
    return 0;
}

/*
 * Legacy Boolean callers cannot propagate a read error. Choose the
 * conservative AP value so unknown NVRAM state cannot enable router-only
 * WAN/firewall actions.
 */
static inline int
rc_ap_mode_read_legacy_fallback(int read_status, int is_ap_mode)
{
    return read_status == 0 ? is_ap_mode : 1;
}

#endif /* AP_MODE_READ_POLICY_H */
