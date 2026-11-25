#ifndef WIRELESS_RESTART_POLICY_H
#define WIRELESS_RESTART_POLICY_H

#include <errno.h>

/*
 * Prepare a replacement radio configuration before any running radio
 * services are stopped. Preserve the generator's status convention:
 * negative values are failures, while non-negative values retain their
 * existing caller semantics.
 */
typedef int (*wireless_restart_config_generator)(int disable_autoscan);

static inline int
wireless_restart_preflight(int need_reload_conf,
                           wireless_restart_config_generator generate_config)
{
    if (!need_reload_conf)
        return 0;

    if (!generate_config)
        return -EINVAL;

    return generate_config(0);
}

#endif /* WIRELESS_RESTART_POLICY_H */
