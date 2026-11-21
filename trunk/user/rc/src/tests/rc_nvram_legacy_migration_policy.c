#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../rc_nvram_legacy_migration_policy.h"

#define CHECK(c) do { if (!(c)) { \
	fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); return 1; \
} } while (0)

struct fake_nvram {
	int set_calls, unset_calls, fail_set_at, unset_status;
	char events[8][80];
};
static int fake_set(void *context, const char *name, const char *value)
{
	struct fake_nvram *f = (struct fake_nvram *)context;
	int call = ++f->set_calls;
	if (call <= 8)
		snprintf(f->events[call - 1], sizeof(f->events[0]),
			 "set:%s=%s", name, value);
	if (f->fail_set_at == call)
		return -ENOSPC;
	return 0;
}
static int fake_unset(void *context, const char *name)
{
	struct fake_nvram *f = (struct fake_nvram *)context;
	int slot = f->set_calls + f->unset_calls;
	++f->unset_calls;
	if (slot < 8)
		snprintf(f->events[slot], sizeof(f->events[0]), "unset:%s", name);
	return f->unset_status;
}
static const struct rc_nvram_legacy_migration_ops fake_ops = {
	fake_set, fake_unset
};

static int test_success_publishes_every_target_before_unset(void)
{
	struct fake_nvram f;
	struct rc_nvram_legacy_migration_write writes[] = {
		{ "front_led_all", "0" }, { "front_led_pwr", "0" }
	};
	int changed = 0;
	memset(&f, 0, sizeof(f));
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "front_leds", writes, 2, &changed) == 0);
	CHECK(changed == 1 && f.set_calls == 2 && f.unset_calls == 1);
	CHECK(strcmp(f.events[0], "set:front_led_all=0") == 0);
	CHECK(strcmp(f.events[1], "set:front_led_pwr=0") == 0);
	CHECK(strcmp(f.events[2], "unset:front_leds") == 0);
	return 0;
}
static int test_first_write_failure_keeps_source(void)
{
	struct fake_nvram f;
	struct rc_nvram_legacy_migration_write write = {
		"wan_ppp_peer", "peer.example"
	};
	int changed = 0;
	memset(&f, 0, sizeof(f)); f.fail_set_at = 1;
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "wan_heartbeat_x", &write, 1, &changed) == -ENOSPC);
	CHECK(f.set_calls == 1 && f.unset_calls == 0 && changed == 0);
	return 0;
}
static int test_later_write_failure_keeps_source(void)
{
	struct fake_nvram f;
	struct rc_nvram_legacy_migration_write writes[] = {
		{ "front_led_all", "0" }, { "front_led_pwr", "0" }
	};
	int changed = 0;
	memset(&f, 0, sizeof(f)); f.fail_set_at = 2;
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "front_leds", writes, 2, &changed) == -ENOSPC);
	CHECK(f.set_calls == 2 && f.unset_calls == 0 && changed == 0);
	return 0;
}
static int test_unset_failure_is_reported(void)
{
	struct fake_nvram f;
	struct rc_nvram_legacy_migration_write write = { "modem_pin", "1234" };
	int changed = 0;
	memset(&f, 0, sizeof(f)); f.unset_status = -EIO;
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "wan_3g_pin", &write, 1, &changed) == -EIO);
	CHECK(f.set_calls == 1 && f.unset_calls == 1 && changed == 0);
	return 0;
}
static int test_zero_target_cleanup_and_invalid_inputs(void)
{
	struct fake_nvram f;
	struct rc_nvram_legacy_migration_write bad = { NULL, "0" };
	int changed = 0;
	memset(&f, 0, sizeof(f));
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "wan0_heartbeat_x", NULL, 0, &changed) == 0);
	CHECK(f.set_calls == 0 && f.unset_calls == 1 && changed == 1);
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "legacy", &bad, 1, &changed) == EINVAL);
	CHECK(rc_nvram_legacy_publish_then_unset(
		&fake_ops, &f, "", NULL, 0, &changed) == EINVAL);
	return 0;
}
int main(void)
{
	CHECK(test_success_publishes_every_target_before_unset() == 0);
	CHECK(test_first_write_failure_keeps_source() == 0);
	CHECK(test_later_write_failure_keeps_source() == 0);
	CHECK(test_unset_failure_is_reported() == 0);
	CHECK(test_zero_target_cleanup_and_invalid_inputs() == 0);
	puts("Legacy NVRAM migration preserves source keys on failed publication: PASS");
	return 0;
}
