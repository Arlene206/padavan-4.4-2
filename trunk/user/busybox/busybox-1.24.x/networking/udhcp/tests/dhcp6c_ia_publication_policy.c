#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include "dhcp6c_ia_policy.h"

struct fake_target {
	int id;
	int installed;
	int failures_left;
	int attempts;
};

static int
reconcile(struct fake_target *targets, size_t count, int server_interface)
{
	size_t i;

	for (i = 0; i < count; ++i) {
		if (!dhcp6c_ia_pd_target_needs_apply(
		    targets[i].id == server_interface, targets[i].installed))
			continue;
		++targets[i].attempts;
		if (targets[i].failures_left > 0) {
			--targets[i].failures_left;
			return EIO;
		}
		targets[i].installed = 1;
	}
	return 0;
}

static void
test_partial_pd_failure_is_retried_without_reapplying_successes(void)
{
	struct fake_target targets[] = {
		{ 1, 0, 0, 0 }, /* DHCP-facing interface: never assign here */
		{ 2, 0, 0, 0 },
		{ 3, 0, 1, 0 }, /* transient kernel failure on first attempt */
		{ 4, 0, 0, 0 }
	};
	int status;

	status = reconcile(targets, sizeof(targets) / sizeof(targets[0]), 1);
	assert(status == EIO);
	assert(!targets[0].installed && targets[0].attempts == 0);
	assert(targets[1].installed && targets[1].attempts == 1);
	assert(!targets[2].installed && targets[2].attempts == 1);
	assert(!targets[3].installed && targets[3].attempts == 0);

	status = reconcile(targets, sizeof(targets) / sizeof(targets[0]), 1);
	assert(status == 0);
	assert(targets[0].attempts == 0);
	assert(targets[1].installed && targets[1].attempts == 1);
	assert(targets[2].installed && targets[2].attempts == 2);
	assert(targets[3].installed && targets[3].attempts == 1);

	assert(reconcile(targets, sizeof(targets) / sizeof(targets[0]), 1) == 0);
	assert(targets[1].attempts == 1);
	assert(targets[2].attempts == 2);
	assert(targets[3].attempts == 1);
}

static void
test_operation_error_is_retryable(void)
{
	assert(dhcp6c_ia_operation_error(0, EIO) == 0);
	assert(dhcp6c_ia_operation_error(-1, EAGAIN) == EAGAIN);
	assert(dhcp6c_ia_operation_error(-1, 0) == EIO);
	assert(dhcp6c_ia_retry_required(
		dhcp6c_ia_operation_error(-1, ENOMEM)));
	assert(!dhcp6c_ia_retry_required(
		dhcp6c_ia_operation_error(0, 0)));
}

static void
test_first_error_and_retry_gate(void)
{
	int status = 0;
	status = dhcp6c_ia_first_error(status, EIO);
	status = dhcp6c_ia_first_error(status, EINVAL);
	assert(status == EIO);
	assert(dhcp6c_ia_retry_required(status));
	assert(!dhcp6c_ia_retry_required(0));
	assert(dhcp6c_ia_first_error(0, 0) == 0);
}

struct fake_remove_target {
	int pending;
	int error;
	int attempts;
};

static int
remove_targets(struct fake_remove_target *targets, size_t count)
{
	size_t i;
	int first_error = 0;

	for (i = 0; i < count; ++i) {
		if (!targets[i].pending)
			continue;
		++targets[i].attempts;
		if (!dhcp6c_ia_ifaddr_change_complete(0, targets[i].error, 1)) {
			if (first_error == 0)
				first_error = targets[i].error ? targets[i].error : EIO;
			continue;
		}
		targets[i].pending = 0;
	}
	return first_error;
}

static void
test_failed_address_removal_is_retained_and_retried(void)
{
	struct fake_remove_target targets[] = {
		{ 1, 0, 0 },
		{ 1, EIO, 0 },
		{ 1, ENOENT, 0 }
	};

	assert(remove_targets(targets, sizeof(targets) / sizeof(targets[0])) == EIO);
	assert(!targets[0].pending && targets[0].attempts == 1);
	assert(targets[1].pending && targets[1].attempts == 1);
	assert(!targets[2].pending && targets[2].attempts == 1);

	targets[1].error = 0;
	assert(remove_targets(targets, sizeof(targets) / sizeof(targets[0])) == 0);
	assert(!targets[1].pending && targets[1].attempts == 2);
	assert(remove_targets(targets, sizeof(targets) / sizeof(targets[0])) == 0);
	assert(targets[0].attempts == 1 && targets[1].attempts == 2);
	assert(targets[2].attempts == 1);
}

static void
test_address_change_absent_and_duplicate_errors(void)
{
	assert(DHCP6C_IA_CLEANUP_RETRY_DELAY_MS == 5000ULL);
	assert(dhcp6c_ia_ifaddr_change_complete(1, 0, 0));
	assert(!dhcp6c_ia_ifaddr_change_complete(1, EEXIST, 0));
	assert(dhcp6c_ia_ifaddr_change_complete(1, EEXIST, 1));
	assert(!dhcp6c_ia_ifaddr_change_complete(1, ENOENT, 1));
	assert(dhcp6c_ia_ifaddr_change_complete(0, 0, 0));
	assert(dhcp6c_ia_ifaddr_change_complete(0, ENOENT, 0));
	assert(dhcp6c_ia_ifaddr_change_complete(0, EADDRNOTAVAIL, 0));
	assert(!dhcp6c_ia_ifaddr_change_complete(0, EEXIST, 1));
	assert(!dhcp6c_ia_ifaddr_change_complete(0, EIO, 1));
}

static void
test_default_ia_timer_ratios_do_not_overflow(void)
{
	assert(dhcp6c_ia_t2_from_duration(0U) == 0U);
	assert(dhcp6c_ia_t2_from_duration(1U) == 0U);
	assert(dhcp6c_ia_t2_from_duration(5U) == 4U);
	assert(dhcp6c_ia_t2_from_duration(1800U) == 1440U);
	assert(dhcp6c_ia_t2_from_duration(2000000000U) == 1600000000U);
	assert(dhcp6c_ia_t2_from_duration(0xfffffffeU) == 3435973835U);

	assert(dhcp6c_ia_t1_from_t2(0U) == 0U);
	assert(dhcp6c_ia_t1_from_t2(8U) == 5U);
	assert(dhcp6c_ia_t1_from_t2(2000000000U) == 1250000000U);
	assert(dhcp6c_ia_t1_from_t2(0xfffffffeU) == 2684354558U);
}

static void
test_prefix_expiry_is_required_for_finite_lifetimes(void)
{
	assert(!dhcp6c_ia_prefix_expiry_timer_required(0UL, 0xffffffffUL));
	assert(dhcp6c_ia_prefix_expiry_timer_required(1UL, 0xffffffffUL));
	assert(dhcp6c_ia_prefix_expiry_timer_required(3600UL, 0xffffffffUL));
	assert(dhcp6c_ia_prefix_expiry_timer_required(0xfffffffeUL, 0xffffffffUL));
	assert(!dhcp6c_ia_prefix_expiry_timer_required(0xffffffffUL, 0xffffffffUL));
}

static void
test_unowned_ia_na_collision_is_never_claimed_or_removed(void)
{
	int address_owned = 0;

	if (dhcp6c_ia_ifaddr_change_complete(1, EEXIST, address_owned))
		address_owned = 1;
	assert(address_owned == 0);
	assert(!dhcp6c_ia_na_address_needs_remove(address_owned));
}

static void
test_owned_ia_na_address_can_be_reconciled_and_removed(void)
{
	int address_owned = 0;

	if (dhcp6c_ia_ifaddr_change_complete(1, 0, address_owned))
		address_owned = 1;
	assert(address_owned == 1);
	assert(dhcp6c_ia_ifaddr_change_complete(1, EEXIST, address_owned));
	assert(dhcp6c_ia_na_address_needs_remove(address_owned));
}

int
main(void)
{
	test_partial_pd_failure_is_retried_without_reapplying_successes();
	test_operation_error_is_retryable();
	test_first_error_and_retry_gate();
	test_default_ia_timer_ratios_do_not_overflow();
	test_prefix_expiry_is_required_for_finite_lifetimes();
	test_failed_address_removal_is_retained_and_retried();
	test_address_change_absent_and_duplicate_errors();
	test_unowned_ia_na_collision_is_never_claimed_or_removed();
	test_owned_ia_na_address_can_be_reconciled_and_removed();
	puts("DHCPv6 IA delegated-prefix retry and removal policy: PASS");
	return 0;
}
