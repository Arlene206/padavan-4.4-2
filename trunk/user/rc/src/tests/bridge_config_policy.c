#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "bridge_config_policy.h"

static const char *calls[7];
static int values[7];
static int results[7];
static int call_count;

static int record_call(const char *name, int value)
{
    assert(call_count < 7);
    calls[call_count] = name;
    values[call_count] = value;
    return results[call_count++];
}
static int mock_bridge_param(const char *bridge, const char *param, int value)
{
    assert(strcmp(bridge, "br0") == 0);
    return record_call(param, value);
}
static int mock_bridge_port_m2u(const char *ifname, int enabled)
{
    assert(strcmp(ifname, "eth0") == 0);
    return record_call("multicast_to_unicast", enabled);
}
static int mock_phy_static_port(int static_port)
{
    return record_call("phy_igmp_static_port", static_port);
}
static int mock_phy_snooping(unsigned int enabled)
{
    return record_call("phy_igmp_snooping", (int)enabled);
}
static const rc_bridge_config_ops_t mock_ops = {
    mock_bridge_param, mock_bridge_port_m2u, mock_phy_static_port, mock_phy_snooping
};
static void reset_mocks(void)
{
    int i;
    call_count = 0;
    for (i = 0; i < 7; i++) {
        calls[i] = NULL;
        values[i] = -1;
        results[i] = 0;
    }
}
static void check_call(int index, const char *name, int value)
{
    assert(index >= 0 && index < call_count);
    assert(strcmp(calls[index], name) == 0);
    assert(values[index] == value);
}
static void test_all_bridge_operations_succeed(void)
{
    const char *first_failure = "not-cleared";
    int failures = -1;
    int status;
    reset_mocks();
    status = rc_bridge_config_apply(&mock_ops, "br0", "eth0", 2, 0, 1, 1, 1, 3, 1,
                                    &failures, &first_failure);
    assert(status == 0 && call_count == 7 && failures == 0 && first_failure == NULL);
    check_call(0, "multicast_router", 2);
    check_call(1, "multicast_querier", 0);
    check_call(2, "multicast_query_use_ifaddr", 1);
    check_call(3, "multicast_snooping", 1);
    check_call(4, "multicast_to_unicast", 1);
    check_call(5, "phy_igmp_static_port", 3);
    check_call(6, "phy_igmp_snooping", 1);
}
static void test_first_failure_is_returned_after_all_operations(void)
{
    const char *first_failure = NULL;
    int failures = 0;
    int status;
    reset_mocks();
    results[0] = EIO;
    results[2] = ENODEV;
    results[4] = -EACCES;
    results[6] = EBUSY;
    status = rc_bridge_config_apply(&mock_ops, "br0", "eth0", 2, 0, 1, 1, 1, 3, 1,
                                    &failures, &first_failure);
    assert(status == EIO && call_count == 7 && failures == 4);
    assert(first_failure != NULL && strcmp(first_failure, "multicast_router") == 0);
}
static void test_configuration_rejects_incomplete_operations_before_side_effects(void)
{
    rc_bridge_config_ops_t incomplete = mock_ops;
    int failures = -1;
    const char *first_failure = "not-cleared";
    reset_mocks();
    assert(rc_bridge_config_apply(NULL, "br0", "eth0", 0, 0, 1, 0, 0, -1, 0,
                                  &failures, &first_failure) == EINVAL);
    assert(call_count == 0 && failures == 0 && first_failure == NULL);
    incomplete.set_phy_igmp_snooping = NULL;
    assert(rc_bridge_config_apply(&incomplete, "br0", "eth0", 0, 0, 1, 0, 0, -1, 0,
                                  NULL, NULL) == EINVAL);
    assert(call_count == 0);
    assert(rc_bridge_config_apply(&mock_ops, "br0", "", 0, 0, 1, 0, 0, -1, 0,
                                  NULL, NULL) == EINVAL);
    assert(call_count == 0);
}
int main(void)
{
    test_all_bridge_operations_succeed();
    test_first_failure_is_returned_after_all_operations();
    test_configuration_rejects_incomplete_operations_before_side_effects();
    puts("Bridge multicast setup status aggregation: PASS");
    return 0;
}
