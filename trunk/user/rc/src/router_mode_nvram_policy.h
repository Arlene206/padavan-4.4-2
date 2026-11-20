#ifndef ROUTER_MODE_NVRAM_POLICY_H
#define ROUTER_MODE_NVRAM_POLICY_H

#include <errno.h>

/* Keep the boot-critical router-mode writes injectable for host regressions. */
typedef int (*rc_router_mode_nvram_set_fn)(void *context,
                                           const char *name,
                                           const char *value);

/* Distinguish a missing key from an NVRAM backend read failure. */
static inline int
rc_router_mode_nvram_read_status(int api_result, int saved_errno,
                                 int value_present)
{
    if ((api_result == 0 && !value_present) ||
        (api_result == 1 && value_present))
        return 0;

    if (api_result < 0)
        return saved_errno > 0 ? saved_errno : EIO;

    return EIO;
}

/*
 * Normalize an unknown router mode to gateway mode, then publish every
 * NVRAM value that defines the effective mode before dependent boot setup.
 * Stop at the first failed write: continuing would create a mixed policy.
 */
static inline int
rc_router_mode_nvram_configure(int configured_mode,
                               rc_router_mode_nvram_set_fn set_value,
                               void *context, int *effective_mode)
{
    int mode = configured_mode;
    int status;

    if (!set_value || !effective_mode)
        return EINVAL;

    if (mode != 1 && mode != 3 && mode != 4)
        mode = 1;
    *effective_mode = mode;

    if (configured_mode != mode) {
        status = set_value(context, "sw_mode", "1");
        if (status != 0)
            return status;
    }

    if (mode != 3) {
        status = set_value(context, "wan_nat_x", mode == 1 ? "1" : "0");
        if (status != 0)
            return status;
    }

    return set_value(context, "wan_route_x",
                     mode == 3 ? "IP_Bridged" : "IP_Routed");
}

#endif /* ROUTER_MODE_NVRAM_POLICY_H */
