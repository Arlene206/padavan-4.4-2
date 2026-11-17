#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include "../firewall_module_table_policy.h"

static void test_loaded(void)
{
	int restore = 0;
	assert(rc_firewall_module_table_restore_decide(1, 0, &restore) == 0);
	assert(restore == 1);
}
static void test_absent_optional(void)
{
	int restore = 1;
	assert(rc_firewall_module_table_restore_decide(0, 0, &restore) == 0);
	assert(restore == 0);
}
static void test_absent_required(void)
{
	int restore = 0;
	assert(rc_firewall_module_table_restore_decide(0, 1, &restore) == 0);
	assert(restore == 1);
}
static void test_unknown_aborts(void)
{
	int restore = 1;
	assert(rc_firewall_module_table_restore_decide(-EACCES, 0, &restore) == EACCES);
	assert(restore == 0);
	assert(rc_firewall_module_table_restore_decide(-EIO, 1, &restore) == EIO);
	assert(restore == 0);
}
static void test_invalid_probe_fails_closed(void)
{
	int restore = 1;
	assert(rc_firewall_module_table_restore_decide(2, 0, &restore) == EIO);
	assert(restore == 0);
	assert(rc_firewall_module_table_restore_decide(INT_MIN, 0, &restore) == EIO);
	assert(restore == 0);
}
static void test_invalid_output(void)
{
	assert(rc_firewall_module_table_restore_decide(1, 0, NULL) == EINVAL);
}
int main(void)
{
	test_loaded();
	test_absent_optional();
	test_absent_required();
	test_unknown_aborts();
	test_invalid_probe_fails_closed();
	test_invalid_output();
	puts("Firewall module-table probe policy: PASS");
	return 0;
}
