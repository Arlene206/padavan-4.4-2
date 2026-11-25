#ifndef RC_DDNS_RESTART_POLICY_H
#define RC_DDNS_RESTART_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef int (*rc_ddns_start_fn)(int clear_cache);
typedef int (*rc_ddns_stop_fn)(void);

/*
 * start_ddns() already owns the stop-before-start transaction. Avoid a
 * redundant stop here, and treat AP mode / a disabled DDNS client as a stop
 * request rather than interpreting start_ddns()'s intentional -1 as failure.
 */
static inline int
rc_ddns_restart_run(int ap_mode, int ddns_enabled,
                    rc_ddns_start_fn start, rc_ddns_stop_fn stop)
{
    if (start == NULL || stop == NULL ||
        (ap_mode != 0 && ap_mode != 1) ||
        (ddns_enabled != 0 && ddns_enabled != 1))
        return EINVAL;

    if (ap_mode || !ddns_enabled)
        return stop();

    return start(1);
}

#endif /* RC_DDNS_RESTART_POLICY_H */
