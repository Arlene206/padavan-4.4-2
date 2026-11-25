#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../netfilter/iptcrdr_allocation_policy.h"

static void
test_entry_allocation_failure(void)
{
	errno = 0;
	assert(miniupnpd_iptcrdr_allocation_failure(
		NULL, NULL, NULL, "test", "rule entry") == -1);
	assert(errno == ENOMEM);
}

static void
test_match_allocation_failure_releases_entry(void)
{
	void *entry = malloc(32);

	assert(entry != NULL);
	errno = 0;
	assert(miniupnpd_iptcrdr_allocation_failure(
		entry, NULL, NULL, "test", "protocol match") == -1);
	assert(errno == ENOMEM);
}

static void
test_target_allocation_failure_releases_prior_parts(void)
{
	void *entry = malloc(32);
	void *match = malloc(32);

	assert(entry != NULL);
	assert(match != NULL);
	errno = 0;
	assert(miniupnpd_iptcrdr_allocation_failure(
		entry, match, NULL, "test", "DNAT target") == -1);
	assert(errno == ENOMEM);
}

static void
test_rule_buffer_failure_releases_all_components(void)
{
	void *entry = malloc(32);
	void *match = malloc(32);
	void *target = malloc(32);

	assert(entry != NULL);
	assert(match != NULL);
	assert(target != NULL);
	errno = 0;
	assert(miniupnpd_iptcrdr_allocation_failure(
		entry, match, target, "test", "rule buffer") == -1);
	assert(errno == ENOMEM);
}

static void
test_original_allocation_errno_is_preserved(void)
{
	void *entry = malloc(32);

	assert(entry != NULL);
	errno = EAGAIN;
	assert(miniupnpd_iptcrdr_allocation_failure(
		entry, NULL, NULL, "test", "protocol match") == -1);
	assert(errno == EAGAIN);
}

int
main(void)
{
	test_entry_allocation_failure();
	test_match_allocation_failure_releases_entry();
	test_target_allocation_failure_releases_prior_parts();
	test_rule_buffer_failure_releases_all_components();
	test_original_allocation_errno_is_preserved();
	puts("MiniUPnP iptables allocation failure policy: PASS");
	return 0;
}
