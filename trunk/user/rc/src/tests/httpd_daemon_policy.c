#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "firewall_nvram_match_policy.h"
#include "httpd_daemon_policy.h"

static void test_launcher_failure_is_preserved(void)
{
	assert(httpd_start_postcondition(-1, 1) == -1);
	assert(httpd_start_postcondition(ENOENT, 1) == ENOENT);
	assert(httpd_start_postcondition(127, 0) == 127);
}

static void test_duplicate_launch_gate_requires_authoritative_absence(void)
{
	assert(httpd_start_can_launch(PROCESS_PRESENCE_ABSENT));
	assert(!httpd_start_can_launch(PROCESS_PRESENCE_RUNNING));
	assert(!httpd_start_can_launch(PROCESS_PRESENCE_UNKNOWN));
	assert(httpd_start_precondition_status(PROCESS_PRESENCE_ABSENT) == 0);
	assert(httpd_start_precondition_status(PROCESS_PRESENCE_RUNNING) == EBUSY);
	assert(httpd_start_precondition_status(PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(httpd_start_precondition_status((enum process_presence)99) == EIO);
}

static void test_detached_child_must_be_confirmed_running(void)
{
	assert(httpd_start_postcondition(0, PROCESS_PRESENCE_ABSENT) == ESRCH);
	assert(httpd_start_postcondition(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(httpd_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(httpd_start_postcondition(0, (enum process_presence)99) == EIO);
	assert(httpd_start_postcondition(EPERM, PROCESS_PRESENCE_RUNNING) == EPERM);
	assert(httpd_start_postcondition(127, PROCESS_PRESENCE_UNKNOWN) == 127);
}

static void test_stop_state_requires_confirmed_exit(void)
{
	assert(httpd_stop_policy_publish_stopped(0));
	assert(!httpd_stop_policy_publish_stopped(-1));
	assert(!httpd_stop_policy_publish_stopped(EBUSY));
}

static void test_failed_start_rollback_keeps_possible_listener_published(void)
{
	assert(!httpd_failed_start_preserve_running_state(PROCESS_PRESENCE_ABSENT));
	assert(httpd_failed_start_preserve_running_state(PROCESS_PRESENCE_RUNNING));
	assert(httpd_failed_start_preserve_running_state(PROCESS_PRESENCE_UNKNOWN));
	assert(httpd_failed_start_preserve_running_state((enum process_presence)99));
}

static void
test_httpd_firewall_enable_read_preserves_exact_match_and_backend_errors(void)
{
	int matches = 99;

	assert(firewall_nvram_match_status(1, 0, "1", "1", 0,
					   &matches) == 0);
	assert(matches == 1);
	assert(firewall_nvram_match_status(1, 0, "01", "1", 0,
					   &matches) == 0);
	assert(matches == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0,
					   &matches) == 0);
	assert(matches == 0);
	matches = 7;
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0,
					   &matches) == EACCES);
	assert(matches == 7);
}

static void
test_httpd_firewall_refresh_gate_preserves_short_circuit_semantics(void)
{
	assert(httpd_firewall_refresh_required(1, 1, 1));
	assert(!httpd_firewall_refresh_required(0, 1, 1));
	assert(!httpd_firewall_refresh_required(1, 0, 1));
	assert(!httpd_firewall_refresh_required(1, 1, 0));
}

static void test_firewall_failure_requires_listener_shutdown(void)
{
	assert(!httpd_firewall_failure_requires_stop(0, PROCESS_PRESENCE_RUNNING));
	assert(!httpd_firewall_failure_requires_stop(0, PROCESS_PRESENCE_UNKNOWN));
	assert(!httpd_firewall_failure_requires_stop(1, PROCESS_PRESENCE_ABSENT));
	assert(httpd_firewall_failure_requires_stop(1, PROCESS_PRESENCE_RUNNING));
	assert(httpd_firewall_failure_requires_stop(-1, PROCESS_PRESENCE_UNKNOWN));
	assert(httpd_firewall_failure_requires_stop(1, (enum process_presence)99));
}

static void test_nvram_write_status_preserves_backend_failure(void)
{
	assert(httpd_nvram_write_status(0, 0) == 0);
	assert(httpd_nvram_write_status(0, ENOSPC) == 0);
	assert(httpd_nvram_write_status(-1, ENOSPC) == ENOSPC);
	assert(httpd_nvram_write_status(-1, 0) == EIO);
	assert(httpd_nvram_write_status(1, EACCES) == EACCES);
}

static void test_restart_requires_confirmed_exit(void)
{
	assert(httpd_restart_can_start(0));
	assert(!httpd_restart_can_start(-1));
	assert(!httpd_restart_can_start(EBUSY));
	assert(!httpd_restart_can_start(EPERM));
}

enum httpd_test_nvram_key {
	HTTPD_TEST_HTTP_PROTO = 0,
	HTTPD_TEST_HTTP_PORT,
	HTTPD_TEST_MISC_HTTP,
	HTTPD_TEST_HTTPS_PORT,
	HTTPD_TEST_HTTPS_WOPEN,
	HTTPD_TEST_KEY_COUNT
};

struct fake_httpd_nvram_reader {
	int values[HTTPD_TEST_KEY_COUNT];
	int present[HTTPD_TEST_KEY_COUNT];
	int fail_key;
	int fail_status;
	int calls;
	int read_keys[8];
	int missing_defaults[8];
};

static const char *const httpd_test_nvram_names[HTTPD_TEST_KEY_COUNT] = {
	"http_proto", "http_lanport", "misc_http_x", "https_lport", "https_wopen"
};

static void
fake_httpd_nvram_init(struct fake_httpd_nvram_reader *fake)
{
	int i;

	memset(fake, 0, sizeof(*fake));
	fake->fail_key = -1;
	for (i = 0; i < HTTPD_TEST_KEY_COUNT; i++)
		fake->missing_defaults[i] = -1;
}

static int
fake_httpd_nvram_read(void *context, const char *param_name,
		      int missing_value, int *value)
{
	struct fake_httpd_nvram_reader *fake =
		(struct fake_httpd_nvram_reader *)context;
	int key;

	if (!fake || !param_name || !value)
		return EINVAL;
	for (key = 0; key < HTTPD_TEST_KEY_COUNT; key++)
		if (strcmp(param_name, httpd_test_nvram_names[key]) == 0)
			break;
	if (key == HTTPD_TEST_KEY_COUNT)
		return ENOENT;
	if (fake->calls >= 8)
		return EOVERFLOW;
	fake->read_keys[fake->calls] = key;
	fake->missing_defaults[fake->calls] = missing_value;
	fake->calls++;
	if (fake->fail_key == key)
		return fake->fail_status;
	*value = fake->present[key] ? fake->values[key] : missing_value;
	return 0;
}

static void
test_start_config_reads_only_selected_listener_settings(void)
{
	struct fake_httpd_nvram_reader fake;
	struct httpd_start_config config;

	fake_httpd_nvram_init(&fake);
	fake.present[HTTPD_TEST_HTTP_PROTO] = 1;
	fake.values[HTTPD_TEST_HTTP_PROTO] = 2;
	fake.present[HTTPD_TEST_HTTP_PORT] = 1;
	fake.values[HTTPD_TEST_HTTP_PORT] = 8080;
	fake.present[HTTPD_TEST_MISC_HTTP] = 1;
	fake.values[HTTPD_TEST_MISC_HTTP] = 1;
	fake.present[HTTPD_TEST_HTTPS_PORT] = 1;
	fake.values[HTTPD_TEST_HTTPS_PORT] = 8443;
	fake.present[HTTPD_TEST_HTTPS_WOPEN] = 1;
	fake.values[HTTPD_TEST_HTTPS_WOPEN] = 1;

	assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 1,
				       &config) == 0);
	assert(config.http_proto == 2);
	assert(config.http_port == 8080 && config.misc_http_x == 1);
	assert(config.https_port == 8443 && config.https_wopen == 1);
	assert(fake.calls == 5);
	assert(fake.read_keys[0] == HTTPD_TEST_HTTP_PROTO);
	assert(fake.read_keys[1] == HTTPD_TEST_HTTP_PORT);
	assert(fake.read_keys[2] == HTTPD_TEST_MISC_HTTP);
	assert(fake.read_keys[3] == HTTPD_TEST_HTTPS_PORT);
	assert(fake.read_keys[4] == HTTPD_TEST_HTTPS_WOPEN);
	assert(fake.missing_defaults[0] == 0);
	assert(fake.missing_defaults[1] == 0);
	assert(fake.missing_defaults[2] == 0);
	assert(fake.missing_defaults[3] == 0);
	assert(fake.missing_defaults[4] == 0);
}

static void
test_start_config_preserves_missing_key_defaults_and_http_only_builds(void)
{
	struct fake_httpd_nvram_reader fake;
	struct httpd_start_config config;

	fake_httpd_nvram_init(&fake);
	assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 1,
				       &config) == 0);
	assert(config.http_proto == 0);
	assert(config.http_port == 0 && config.misc_http_x == 0);
	assert(config.https_port == 0 && config.https_wopen == 0);
	assert(fake.calls == 3);
	assert(fake.read_keys[0] == HTTPD_TEST_HTTP_PROTO);
	assert(fake.read_keys[1] == HTTPD_TEST_HTTP_PORT);
	assert(fake.read_keys[2] == HTTPD_TEST_MISC_HTTP);

	fake_httpd_nvram_init(&fake);
	assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 0,
				       &config) == 0);
	assert(config.http_proto == 0);
	assert(fake.calls == 2);
	assert(fake.read_keys[0] == HTTPD_TEST_HTTP_PORT);
	assert(fake.read_keys[1] == HTTPD_TEST_MISC_HTTP);
}

static void
test_start_config_reads_https_only_settings_when_selected(void)
{
	struct fake_httpd_nvram_reader fake;
	struct httpd_start_config config;

	fake_httpd_nvram_init(&fake);
	fake.present[HTTPD_TEST_HTTP_PROTO] = 1;
	fake.values[HTTPD_TEST_HTTP_PROTO] = 1;
	fake.present[HTTPD_TEST_HTTPS_PORT] = 1;
	fake.values[HTTPD_TEST_HTTPS_PORT] = 443;
	fake.present[HTTPD_TEST_HTTPS_WOPEN] = 1;
	fake.values[HTTPD_TEST_HTTPS_WOPEN] = 1;
	assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 1,
				       &config) == 0);
	assert(config.http_proto == 1);
	assert(config.https_port == 443 && config.https_wopen == 1);
	assert(fake.calls == 3);
	assert(fake.read_keys[0] == HTTPD_TEST_HTTP_PROTO);
	assert(fake.read_keys[1] == HTTPD_TEST_HTTPS_PORT);
	assert(fake.read_keys[2] == HTTPD_TEST_HTTPS_WOPEN);
}

static void
test_start_config_fails_closed_without_partial_snapshot(void)
{
	struct fake_httpd_nvram_reader fake;
	struct httpd_start_config config;
	int key;

	for (key = 0; key < HTTPD_TEST_KEY_COUNT; key++) {
		fake_httpd_nvram_init(&fake);
		fake.present[HTTPD_TEST_HTTP_PROTO] = 1;
		fake.values[HTTPD_TEST_HTTP_PROTO] = 2;
		fake.present[HTTPD_TEST_HTTP_PORT] = 1;
		fake.present[HTTPD_TEST_MISC_HTTP] = 1;
		fake.present[HTTPD_TEST_HTTPS_PORT] = 1;
		fake.present[HTTPD_TEST_HTTPS_WOPEN] = 1;
		fake.fail_key = key;
		fake.fail_status = EIO + key;
		config.http_proto = 11;
		config.http_port = 22;
		config.misc_http_x = 33;
		config.https_port = 44;
		config.https_wopen = 55;

		assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 1,
					       &config) == EIO + key);
		assert(config.http_proto == 11);
		assert(config.http_port == 22);
		assert(config.misc_http_x == 33);
		assert(config.https_port == 44);
		assert(config.https_wopen == 55);
		assert(fake.calls == key + 1);
	}
}

static void
test_start_config_rejects_invalid_arguments(void)
{
	struct fake_httpd_nvram_reader fake;
	struct httpd_start_config config;

	fake_httpd_nvram_init(&fake);
	assert(httpd_start_config_read(NULL, &fake, 1, &config) == EINVAL);
	assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 2,
				       &config) == EINVAL);
	assert(httpd_start_config_read(fake_httpd_nvram_read, &fake, 1,
				       NULL) == EINVAL);
}

int main(void)
{
	test_start_config_reads_only_selected_listener_settings();
	test_start_config_preserves_missing_key_defaults_and_http_only_builds();
	test_start_config_reads_https_only_settings_when_selected();
	test_start_config_fails_closed_without_partial_snapshot();
	test_start_config_rejects_invalid_arguments();
	test_launcher_failure_is_preserved();
	test_duplicate_launch_gate_requires_authoritative_absence();
	test_detached_child_must_be_confirmed_running();
	test_stop_state_requires_confirmed_exit();
	test_failed_start_rollback_keeps_possible_listener_published();
	test_restart_requires_confirmed_exit();
	test_nvram_write_status_preserves_backend_failure();
	test_httpd_firewall_enable_read_preserves_exact_match_and_backend_errors();
	test_httpd_firewall_refresh_gate_preserves_short_circuit_semantics();
	test_firewall_failure_requires_listener_shutdown();
	puts("HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS");
	return 0;
}
