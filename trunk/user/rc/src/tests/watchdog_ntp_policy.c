#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "watchdog_ntp_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_successful_stop_allows_refresh(void)
{
	CHECK(watchdog_ntp_refresh_should_launch(0));
	return 0;
}

static int
test_incomplete_stop_blocks_refresh(void)
{
	CHECK(!watchdog_ntp_refresh_should_launch(-1));
	CHECK(!watchdog_ntp_refresh_should_launch(1));
	CHECK(!watchdog_ntp_refresh_should_launch(EBUSY));
	CHECK(!watchdog_ntp_refresh_should_launch(EIO));
	return 0;
}

static int
test_time_notification_result_precedence(void)
{
	CHECK(watchdog_time_notification_error(0, EIO, -1, EIO) == 0);
	CHECK(watchdog_time_notification_error(-1, ENOENT, 0, 0) == 0);
	CHECK(watchdog_time_notification_error(-1, ESRCH, 0, 0) == 0);
	CHECK(watchdog_time_notification_error(-1, ENOENT, -1, EBUSY) == EBUSY);
	CHECK(watchdog_time_notification_error(-1, ESRCH, 7, 0) == 7);
	CHECK(watchdog_time_notification_error(-1, ENOENT, -1, 0) == EIO);
	CHECK(watchdog_time_notification_error(-1, EPERM, 0, 0) == EPERM);
	CHECK(watchdog_time_notification_error(-1, 0, 0, 0) == EIO);
	CHECK(watchdog_time_notification_error(3, 0, 0, 0) == 3);
	return 0;
}


static void
test_ntp_step_requires_nonempty_offset(void)
{
	assert(!watchdog_ntp_step_has_offset((const char *)0));
	assert(!watchdog_ntp_step_has_offset(""));
	assert(watchdog_ntp_step_has_offset("0.125"));
	assert(watchdog_ntp_step_has_offset("-3.5"));
}

static void
test_ntp_ready_decision_requires_both_checked_reads(void)
{
    int retry_required = 1;

    assert(watchdog_ntp_ready_decision(0, 1, 0, 0,
                                       &retry_required) == 0);
    assert(retry_required == 1);
    assert(watchdog_ntp_ready_decision(0, 1, 0, 1,
                                       &retry_required) == 0);
    assert(retry_required == 0);

    retry_required = 1;
    assert(watchdog_ntp_ready_decision(EIO, 1, 0, 0,
                                       &retry_required) == EIO);
    assert(retry_required == 0);
    retry_required = 1;
    assert(watchdog_ntp_ready_decision(0, 1, ENODEV, 0,
                                       &retry_required) == ENODEV);
    assert(retry_required == 0);
    assert(watchdog_ntp_ready_decision(0, 1, 0, 0, NULL) == EINVAL);
}


static void
test_ntp_ready_publication_retries_only_after_a_real_step(void)
{
	assert(!watchdog_ntp_ready_retry_required(0, 0));
	assert(!watchdog_ntp_ready_retry_required(-1, 0));
	assert(watchdog_ntp_ready_retry_required(1, 0));
	assert(!watchdog_ntp_ready_retry_required(1, 1));
	assert(watchdog_ntp_ready_retry_required(4, 2));
}

static void
test_deferred_wanup_requires_checked_deadline(void)
{
	int due = 1;

	assert(watchdog_deferred_wanup_due(0, 0, 100, &due) == 0);
	assert(due == 0);
	assert(watchdog_deferred_wanup_due(0, 50, 49, &due) == 0);
	assert(due == 0);
	assert(watchdog_deferred_wanup_due(0, 50, 50, &due) == 0);
	assert(due == 1);
	assert(watchdog_deferred_wanup_due(0, -1, 100, &due) == 0);
	assert(due == 0);
	assert(watchdog_deferred_wanup_due(EIO, 50, 100, &due) == EIO);
	assert(due == 0);
	assert(watchdog_deferred_wanup_due(0, 50, 100, NULL) == EINVAL);
}

static void
test_ntp_counter_advance_rejects_corruption_and_saturates(void)
{
	int next = -1;

	assert(watchdog_ntp_counter_advance(0, &next) == 0);
	assert(next == 1);
	assert(watchdog_ntp_counter_advance(41, &next) == 0);
	assert(next == 42);
	assert(watchdog_ntp_counter_advance(INT_MAX, &next) == 0);
	assert(next == INT_MAX);
	assert(watchdog_ntp_counter_advance(-1, &next) == EINVAL);
	assert(watchdog_ntp_counter_advance(1, NULL) == EINVAL);
}

struct fake_ntp_nvram {
    const char *server0;
    const char *server1;
    int server0_status;
    int server1_status;
    char cache[WATCHDOG_NTP_SERVER_MAX + 32];
    int calls;
};

static int
fake_ntp_nvram_get(void *context, const char *name, char **value)
{
    struct fake_ntp_nvram *fake = (struct fake_ntp_nvram *)context;
    const char *source;
    int status;

    if (!fake || !name || !value)
        return -1;
    if (strcmp(name, "ntp_server0") == 0) {
        source = fake->server0;
        status = fake->server0_status;
    } else if (strcmp(name, "ntp_server1") == 0) {
        source = fake->server1;
        status = fake->server1_status;
    } else {
        return -1;
    }

    fake->calls++;
    if (status <= 0)
        return status;
    if (!source)
        return -1;
    if (snprintf(fake->cache, sizeof(fake->cache), "%s", source) >=
        (int)sizeof(fake->cache))
        return -1;
    *value = fake->cache;
    return 1;
}

static int
test_ntp_server_pair_copies_cache_before_next_read(void)
{
    struct fake_ntp_nvram fake;
    char server0[WATCHDOG_NTP_SERVER_MAX];
    char server1[WATCHDOG_NTP_SERVER_MAX];

    memset(&fake, 0, sizeof(fake));
    fake.server0 = "time-a.example";
    fake.server1 = "time-b.example";
    fake.server0_status = 1;
    fake.server1_status = 1;

    CHECK(watchdog_ntp_read_server_pair(fake_ntp_nvram_get, &fake,
                                        server0, sizeof(server0),
                                        server1, sizeof(server1)) == 0);
    CHECK(strcmp(server0, "time-a.example") == 0);
    CHECK(strcmp(server1, "time-b.example") == 0);
    CHECK(fake.calls == 2);
    return 0;
}

static int
test_ntp_server_pair_failure_clears_partial_snapshot(void)
{
    struct fake_ntp_nvram fake;
    char server0[WATCHDOG_NTP_SERVER_MAX];
    char server1[WATCHDOG_NTP_SERVER_MAX];

    memset(&fake, 0, sizeof(fake));
    fake.server0 = "time-a.example";
    fake.server0_status = 1;
    fake.server1_status = -1;
    strcpy(server0, "stale-server");
    strcpy(server1, "stale-server");

    CHECK(watchdog_ntp_read_server_pair(fake_ntp_nvram_get, &fake,
                                        server0, sizeof(server0),
                                        server1, sizeof(server1)) == EIO);
    CHECK(server0[0] == '\0' && server1[0] == '\0');
    CHECK(fake.calls == 2);
    return 0;
}

static int
test_ntp_missing_keys_preserve_legacy_fallbacks(void)
{
    struct fake_ntp_nvram fake;
    char server0[WATCHDOG_NTP_SERVER_MAX];
    char server1[WATCHDOG_NTP_SERVER_MAX];

    memset(&fake, 0, sizeof(fake));
    fake.server0_status = 0;
    fake.server1 = "time-b.example";
    fake.server1_status = 1;
    CHECK(watchdog_ntp_read_server_pair(fake_ntp_nvram_get, &fake,
                                        server0, sizeof(server0),
                                        server1, sizeof(server1)) == 0);
    CHECK(watchdog_ntp_resolve_server_pair(server0, sizeof(server0),
                                           server1, sizeof(server1)) == 0);
    CHECK(strcmp(server0, "time-b.example") == 0);
    CHECK(strcmp(server1, "time-b.example") == 0);

    fake.server1_status = 0;
    CHECK(watchdog_ntp_read_server_pair(fake_ntp_nvram_get, &fake,
                                        server0, sizeof(server0),
                                        server1, sizeof(server1)) == 0);
    CHECK(watchdog_ntp_resolve_server_pair(server0, sizeof(server0),
                                           server1, sizeof(server1)) == 0);
    CHECK(strcmp(server0, "pool.ntp.org") == 0);
    CHECK(strcmp(server1, "pool.ntp.org") == 0);
    return 0;
}

static int
test_ntp_overlong_server_is_rejected_without_truncation(void)
{
    struct fake_ntp_nvram fake;
    char long_server[WATCHDOG_NTP_SERVER_MAX + 16];
    char server0[WATCHDOG_NTP_SERVER_MAX];
    char server1[WATCHDOG_NTP_SERVER_MAX];

    memset(&fake, 0, sizeof(fake));
    memset(long_server, 'a', sizeof(long_server) - 1);
    long_server[sizeof(long_server) - 1] = '\0';
    fake.server0 = long_server;
    fake.server0_status = 1;
    fake.server1 = "time-b.example";
    fake.server1_status = 1;

    CHECK(watchdog_ntp_read_server_pair(fake_ntp_nvram_get, &fake,
                                        server0, sizeof(server0),
                                        server1, sizeof(server1)) == ENAMETOOLONG);
    CHECK(server0[0] == '\0' && server1[0] == '\0');
    return 0;
}



#define WATCHDOG_SCHEDULE_KEY_COUNT 5
static const char *const schedule_keys[WATCHDOG_SCHEDULE_KEY_COUNT] = {
	"reboot_schedule_enable", "ss_schedule_enable", "ntp_ready",
	"reboot_schedule", "ss_schedule"
};
struct fake_schedule_nvram {
	const char *values[WATCHDOG_SCHEDULE_KEY_COUNT];
	int statuses[WATCHDOG_SCHEDULE_KEY_COUNT];
	char cache[WATCHDOG_NTP_SERVER_MAX + 32];
	int calls;
};
static int
fake_schedule_nvram_get(void *context, const char *name, char **value)
{
	struct fake_schedule_nvram *fake = (struct fake_schedule_nvram *)context;
	int i, status;
	if (!fake || !name || !value) { errno = EINVAL; return -1; }
	for (i = 0; i < WATCHDOG_SCHEDULE_KEY_COUNT; i++)
		if (strcmp(name, schedule_keys[i]) == 0) break;
	if (i == WATCHDOG_SCHEDULE_KEY_COUNT) { errno = ENOENT; return -1; }
	fake->calls++;
	status = fake->statuses[i];
	if (status == 0) return 0;
	if (status < 0) { errno = EIO; return -1; }
	if (!fake->values[i] ||
	    snprintf(fake->cache, sizeof(fake->cache), "%s", fake->values[i]) >=
	    (int)sizeof(fake->cache)) { errno = EOVERFLOW; return -1; }
	*value = fake->cache;
	return 1;
}
static void
fake_schedule_enable_all(struct fake_schedule_nvram *fake)
{
	int i;
	memset(fake, 0, sizeof(*fake));
	for (i = 0; i < WATCHDOG_SCHEDULE_KEY_COUNT; i++) fake->statuses[i] = 1;
}
static int
test_schedule_snapshot_copies_all_values_before_next_read(void)
{
	struct fake_schedule_nvram fake;
	struct watchdog_ntp_schedule_snapshot snapshot;
	fake_schedule_enable_all(&fake);
	fake.values[0] = "1"; fake.values[1] = "1"; fake.values[2] = "1";
	fake.values[3] = "11111112345"; fake.values[4] = "00000001234";
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == 0);
	CHECK(snapshot.reboot_enabled == 1 && snapshot.ss_enabled == 1 && snapshot.ntp_ready == 1);
	CHECK(strcmp(snapshot.reboot_schedule, "11111112345") == 0);
	CHECK(strcmp(snapshot.ss_schedule, "00000001234") == 0);
	CHECK(fake.calls == WATCHDOG_SCHEDULE_KEY_COUNT);
	return 0;
}
static int
test_schedule_snapshot_missing_keys_keep_disabled_defaults(void)
{
	struct fake_schedule_nvram fake;
	struct watchdog_ntp_schedule_snapshot snapshot;
	memset(&fake, 0, sizeof(fake));
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == 0);
	CHECK(snapshot.reboot_enabled == 0 && snapshot.ss_enabled == 0 && snapshot.ntp_ready == 0);
	CHECK(snapshot.reboot_schedule[0] == '\0' && snapshot.ss_schedule[0] == '\0');
	CHECK(fake.calls == WATCHDOG_SCHEDULE_KEY_COUNT);
	return 0;
}
static int
test_schedule_snapshot_read_failure_clears_partial_state(void)
{
	struct fake_schedule_nvram fake;
	struct watchdog_ntp_schedule_snapshot snapshot;
	fake_schedule_enable_all(&fake);
	fake.values[0] = "1"; fake.values[1] = "0"; fake.values[2] = "1";
	fake.values[3] = "11111112345"; fake.values[4] = "00000001234";
	fake.statuses[3] = -1;
	memset(&snapshot, 0x5a, sizeof(snapshot));
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == EIO);
	CHECK(snapshot.reboot_enabled == 0 && snapshot.ss_enabled == 0 && snapshot.ntp_ready == 0);
	CHECK(snapshot.reboot_schedule[0] == '\0' && snapshot.ss_schedule[0] == '\0');
	CHECK(fake.calls == 4);
	return 0;
}
static int
test_schedule_snapshot_rejects_overlong_schedule_without_truncation(void)
{
	struct fake_schedule_nvram fake;
	struct watchdog_ntp_schedule_snapshot snapshot;
	fake_schedule_enable_all(&fake);
	fake.values[0] = "1"; fake.values[1] = "0"; fake.values[2] = "1";
	fake.values[3] = "111111112345"; fake.values[4] = "00000001234";
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == ENAMETOOLONG);
	CHECK(snapshot.reboot_enabled == 0 && snapshot.ss_enabled == 0 && snapshot.ntp_ready == 0);
	CHECK(snapshot.reboot_schedule[0] == '\0' && snapshot.ss_schedule[0] == '\0');
	return 0;
}
static int
test_schedule_snapshot_requires_exact_enable_tokens(void)
{
	struct fake_schedule_nvram fake;
	struct watchdog_ntp_schedule_snapshot snapshot;
	fake_schedule_enable_all(&fake);
	fake.values[0] = "01"; fake.values[1] = "true"; fake.values[2] = "1 ";
	fake.values[3] = "11111112345"; fake.values[4] = "00000001234";
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == 0);
	CHECK(snapshot.reboot_enabled == 0 && snapshot.ss_enabled == 0 && snapshot.ntp_ready == 0);
	CHECK(fake.calls == 3);
	return 0;
}


static int
test_schedule_snapshot_skips_disabled_and_unready_schedule_reads(void)
{
	struct fake_schedule_nvram fake;
	struct watchdog_ntp_schedule_snapshot snapshot;

	fake_schedule_enable_all(&fake);
	fake.values[0] = "0"; fake.values[1] = "0"; fake.values[2] = "1";
	fake.statuses[3] = -1; fake.statuses[4] = -1;
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == 0);
	CHECK(!snapshot.reboot_enabled && !snapshot.ss_enabled && snapshot.ntp_ready);
	CHECK(snapshot.reboot_schedule[0] == '\0' && snapshot.ss_schedule[0] == '\0');
	CHECK(fake.calls == 3);

	fake_schedule_enable_all(&fake);
	fake.values[0] = "1"; fake.values[1] = "0"; fake.values[2] = "1";
	fake.values[3] = "11111112345"; fake.statuses[4] = -1;
	CHECK(watchdog_ntp_schedule_snapshot_read(fake_schedule_nvram_get, &fake, &snapshot) == 0);
	CHECK(snapshot.reboot_enabled && !snapshot.ss_enabled && snapshot.ntp_ready);
	CHECK(strcmp(snapshot.reboot_schedule, "11111112345") == 0);
	CHECK(snapshot.ss_schedule[0] == '\0' && fake.calls == 4);
	return 0;
}



int
main(void)
{
	test_ntp_step_requires_nonempty_offset();
	test_ntp_ready_publication_retries_only_after_a_real_step();
	test_ntp_counter_advance_rejects_corruption_and_saturates();
	test_deferred_wanup_requires_checked_deadline();
	test_ntp_ready_decision_requires_both_checked_reads();
	CHECK(test_ntp_server_pair_copies_cache_before_next_read() == 0);
	CHECK(test_ntp_server_pair_failure_clears_partial_snapshot() == 0);
	CHECK(test_ntp_missing_keys_preserve_legacy_fallbacks() == 0);
	CHECK(test_ntp_overlong_server_is_rejected_without_truncation() == 0);
	CHECK(test_schedule_snapshot_copies_all_values_before_next_read() == 0);
	CHECK(test_schedule_snapshot_missing_keys_keep_disabled_defaults() == 0);
	CHECK(test_schedule_snapshot_read_failure_clears_partial_state() == 0);
	CHECK(test_schedule_snapshot_rejects_overlong_schedule_without_truncation() == 0);
	CHECK(test_schedule_snapshot_requires_exact_enable_tokens() == 0);
	CHECK(test_schedule_snapshot_skips_disabled_and_unready_schedule_reads() == 0);

	CHECK(test_successful_stop_allows_refresh() == 0);
	CHECK(test_incomplete_stop_blocks_refresh() == 0);
	CHECK(test_time_notification_result_precedence() == 0);
	puts("watchdog NTP stop-before-start policy: PASS");
	return 0;
}
