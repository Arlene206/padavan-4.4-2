/* GNU89 regression tests for bounded DHCP relay transaction state. */
#include <assert.h>
#include <stdio.h>

#include "udhcp_relay_xid_policy.h"

static void
test_reservation_limit(void)
{
    struct udhcp_relay_xid_budget budget = { 0U };
    unsigned int i;

    for (i = 0; i < UDHCP_RELAY_MAX_PENDING_XIDS; ++i)
        assert(udhcp_relay_xid_reserve(&budget));
    assert(budget.pending == UDHCP_RELAY_MAX_PENDING_XIDS);
    assert(!udhcp_relay_xid_reserve(&budget));
    assert(budget.pending == UDHCP_RELAY_MAX_PENDING_XIDS);
}

static void
test_release_allows_next_transaction(void)
{
    struct udhcp_relay_xid_budget budget = { 0U };
    unsigned int i;

    for (i = 0; i < UDHCP_RELAY_MAX_PENDING_XIDS; ++i)
        assert(udhcp_relay_xid_reserve(&budget));
    assert(!udhcp_relay_xid_reserve(&budget));

    udhcp_relay_xid_release(&budget);
    assert(budget.pending == UDHCP_RELAY_MAX_PENDING_XIDS - 1U);
    assert(udhcp_relay_xid_reserve(&budget));
    assert(budget.pending == UDHCP_RELAY_MAX_PENDING_XIDS);

    for (i = 0; i < UDHCP_RELAY_MAX_PENDING_XIDS; ++i)
        udhcp_relay_xid_release(&budget);
    assert(budget.pending == 0U);
    udhcp_relay_xid_release(&budget); /* underflow is prevented */
    assert(budget.pending == 0U);
}

static void
test_send_result_status(void)
{
    assert(udhcp_relay_send_result_ok(0));
    assert(udhcp_relay_send_result_ok(1));
    assert(udhcp_relay_send_result_ok(548));
    assert(!udhcp_relay_send_result_ok(-1));
    assert(!udhcp_relay_send_result_ok(-11));
}

int
main(void)
{
    test_reservation_limit();
    test_release_allows_next_transaction();
    test_send_result_status();
    puts("DHCP relay bounded XID transaction regression: PASS");
    return 0;
}
