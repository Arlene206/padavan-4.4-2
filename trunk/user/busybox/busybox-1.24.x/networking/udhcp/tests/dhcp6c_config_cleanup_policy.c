#include <assert.h>
#include <stdio.h>

#include "dhcp6c_config_cleanup_policy.h"

#define IA_NA 3
#define IA_TA 4
#define IA_PD 25

struct cleanup_record {
	void *expected;
	void *actual;
	unsigned int calls;
};

static void
record_cleanup(void *context)
{
	struct cleanup_record *record = (struct cleanup_record *)context;

	record->actual = context;
	record->calls++;
}

static void
test_ia_na_prefix_list_is_released(void)
{
	struct cleanup_record record = { NULL, NULL, 0 };

	record.expected = &record;
	dhcp6c_config_cleanup_prefix_list(IA_NA, IA_NA, IA_PD,
	                                  &record, record_cleanup);
	assert(record.calls == 1);
	assert(record.actual == record.expected);
}

static void
test_ia_pd_prefix_list_is_released(void)
{
	struct cleanup_record record = { NULL, NULL, 0 };

	record.expected = &record;
	dhcp6c_config_cleanup_prefix_list(IA_PD, IA_NA, IA_PD,
	                                  &record, record_cleanup);
	assert(record.calls == 1);
	assert(record.actual == record.expected);
}

static void
test_unrelated_ia_type_does_not_release_unowned_list(void)
{
	struct cleanup_record record = { NULL, NULL, 0 };

	dhcp6c_config_cleanup_prefix_list(IA_TA, IA_NA, IA_PD,
	                                  &record, record_cleanup);
	assert(record.calls == 0);
	assert(record.actual == NULL);
}

static void
test_invalid_cleanup_inputs_are_noops(void)
{
	struct cleanup_record record = { NULL, NULL, 0 };

	dhcp6c_config_cleanup_prefix_list(IA_NA, IA_NA, IA_PD,
	                                  NULL, record_cleanup);
	dhcp6c_config_cleanup_prefix_list(IA_NA, IA_NA, IA_PD,
	                                  &record, NULL);
	assert(record.calls == 0);
}

int
main(void)
{
	test_ia_na_prefix_list_is_released();
	test_ia_pd_prefix_list_is_released();
	test_unrelated_ia_type_does_not_release_unowned_list();
	test_invalid_cleanup_inputs_are_noops();
	puts("DHCPv6 IA config cleanup policy: PASS");
	return 0;
}
