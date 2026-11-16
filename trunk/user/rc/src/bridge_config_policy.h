#ifndef RC_BRIDGE_CONFIG_POLICY_H
#define RC_BRIDGE_CONFIG_POLICY_H
#include <errno.h>

typedef struct {
    int (*set_bridge_param)(const char *, const char *, int);
    int (*set_bridge_port_m2u)(const char *, int);
    int (*set_phy_igmp_static_port)(int);
    int (*set_phy_igmp_snooping)(unsigned int);
} rc_bridge_config_ops_t;

static inline void
rc_bridge_config_record_failure(int operation_status, const char *operation,
                                int *first_status, int *failure_count,
                                const char **first_failure)
{
    if (operation_status == 0)
        return;
    if (*first_status == 0) {
        *first_status = operation_status;
        if (first_failure)
            *first_failure = operation;
    }
    (*failure_count)++;
}

/* Attempt every independent bridge/switch setting and preserve the first error. */
static inline int
rc_bridge_config_apply(const rc_bridge_config_ops_t *ops,
                       const char *bridge_ifname, const char *wired_ifname,
                       int multicast_router, int multicast_querier,
                       int multicast_query_use_ifaddr, int multicast_snooping,
                       int multicast_to_unicast, int static_port,
                       unsigned int phy_snooping, int *failure_count,
                       const char **first_failure)
{
    int first_status = 0;
    int failures = 0;
    int status;

    if (failure_count)
        *failure_count = 0;
    if (first_failure)
        *first_failure = NULL;
    if (!ops || !ops->set_bridge_param || !ops->set_bridge_port_m2u ||
        !ops->set_phy_igmp_static_port || !ops->set_phy_igmp_snooping ||
        !bridge_ifname || !*bridge_ifname || !wired_ifname || !*wired_ifname)
        return EINVAL;

    status = ops->set_bridge_param(bridge_ifname, "multicast_router", multicast_router);
    rc_bridge_config_record_failure(status, "multicast_router", &first_status, &failures, first_failure);
    status = ops->set_bridge_param(bridge_ifname, "multicast_querier", multicast_querier);
    rc_bridge_config_record_failure(status, "multicast_querier", &first_status, &failures, first_failure);
    status = ops->set_bridge_param(bridge_ifname, "multicast_query_use_ifaddr", multicast_query_use_ifaddr);
    rc_bridge_config_record_failure(status, "multicast_query_use_ifaddr", &first_status, &failures, first_failure);
    status = ops->set_bridge_param(bridge_ifname, "multicast_snooping", multicast_snooping);
    rc_bridge_config_record_failure(status, "multicast_snooping", &first_status, &failures, first_failure);
    status = ops->set_bridge_port_m2u(wired_ifname, multicast_to_unicast);
    rc_bridge_config_record_failure(status, "multicast_to_unicast", &first_status, &failures, first_failure);
    status = ops->set_phy_igmp_static_port(static_port);
    rc_bridge_config_record_failure(status, "phy_igmp_static_port", &first_status, &failures, first_failure);
    status = ops->set_phy_igmp_snooping(phy_snooping);
    rc_bridge_config_record_failure(status, "phy_igmp_snooping", &first_status, &failures, first_failure);

    if (failure_count)
        *failure_count = failures;
    return first_status;
}

#endif /* RC_BRIDGE_CONFIG_POLICY_H */
