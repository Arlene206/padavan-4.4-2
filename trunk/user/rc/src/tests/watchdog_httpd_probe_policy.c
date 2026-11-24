#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "watchdog_httpd_probe_policy.h"

static void
test_login_timestamp_requires_complete_decimal_value(void)
{
    unsigned long timestamp = 0;

    assert(watchdog_httpd_login_timestamp_parse("12345", &timestamp) == 0);
    assert(timestamp == 12345UL);
    assert(watchdog_httpd_login_timestamp_parse("0", &timestamp) == 0);
    assert(watchdog_httpd_login_timestamp_parse(NULL, &timestamp) == EINVAL);
    assert(watchdog_httpd_login_timestamp_parse("", &timestamp) == EINVAL);
    assert(watchdog_httpd_login_timestamp_parse("12x", &timestamp) == EINVAL);
    assert(watchdog_httpd_login_timestamp_parse("-1", &timestamp) == EINVAL);
    assert(watchdog_httpd_login_timestamp_parse(
               "999999999999999999999999999999999999", &timestamp) == ERANGE);
}

static void
test_probe_requires_trustworthy_http_configuration(void)
{
    assert(watchdog_httpd_probe_config_usable(0, 1, 80, 0));
    assert(!watchdog_httpd_probe_config_usable(EIO, 1, 80, 0));
    assert(!watchdog_httpd_probe_config_usable(0, 0, 80, 0));
    assert(!watchdog_httpd_probe_config_usable(0, 1, 0, 0));
    assert(!watchdog_httpd_probe_config_usable(0, 1, 65536, 0));
    assert(!watchdog_httpd_probe_config_usable(0, 1, 80, 1));
    assert(!watchdog_httpd_probe_config_usable(0, 1, 80, 2));
}

static void
test_stale_probe_result_must_be_removed_before_probe(void)
{
    assert(watchdog_httpd_probe_stale_file_remove_status(0, 0) == 0);
    assert(watchdog_httpd_probe_stale_file_remove_status(-1, ENOENT) == 0);
    assert(watchdog_httpd_probe_stale_file_remove_status(-1, EACCES) == EACCES);
    assert(watchdog_httpd_probe_stale_file_remove_status(-1, EIO) == EIO);
    assert(watchdog_httpd_probe_stale_file_remove_status(-1, 0) == EIO);
}

static void
test_port_recheck_does_not_hide_read_errors_or_changes(void)
{
    assert(watchdog_httpd_probe_port_unchanged(0, 8080, 8080));
    assert(!watchdog_httpd_probe_port_unchanged(0, 8081, 8080));
    assert(!watchdog_httpd_probe_port_unchanged(EIO, 8080, 8080));
    assert(!watchdog_httpd_probe_port_unchanged(0, 0, 8080));
    assert(!watchdog_httpd_probe_port_unchanged(0, 8080, 0));
}

static void
test_probe_file_io_errors_skip_restart_decision(void)
{
    assert(watchdog_httpd_probe_file_io_status(0, 0, 0, 0, 0) == 0);
    assert(watchdog_httpd_probe_file_io_status(ENOENT, 0, 0, 0, 0) == 0);
    assert(watchdog_httpd_probe_file_io_status(EACCES, 0, 0, 0, 0) == EACCES);
    assert(watchdog_httpd_probe_file_io_status(EMFILE, 0, 0, 0, 0) == EMFILE);
    assert(watchdog_httpd_probe_file_io_status(EIO, 0, 0, 0, 0) == EIO);
    assert(watchdog_httpd_probe_file_io_status(0, 1, EIO, 0, 0) == EIO);
    assert(watchdog_httpd_probe_file_io_status(0, 1, 0, 0, 0) == EIO);
    assert(watchdog_httpd_probe_file_io_status(0, 0, 0, -1, EIO) == EIO);
    assert(watchdog_httpd_probe_file_io_status(0, 0, 0, -1, 0) == EIO);
}

int
main(void)
{
    test_login_timestamp_requires_complete_decimal_value();
    test_probe_requires_trustworthy_http_configuration();
    test_port_recheck_does_not_hide_read_errors_or_changes();
    test_stale_probe_result_must_be_removed_before_probe();
    test_probe_file_io_errors_skip_restart_decision();
    puts("watchdog HTTPD probe safety policy: PASS");
    return 0;
}
