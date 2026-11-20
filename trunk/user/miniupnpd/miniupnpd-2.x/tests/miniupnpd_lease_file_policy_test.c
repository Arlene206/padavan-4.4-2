#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "miniupnpd_lease_file_policy.h"

static void
test_single_line_description(void)
{
	char record[128];
	const char description[] = "camera\nTCP:9999:192.168.1.2:22:0:injected\rdisplay";
	const char *newline;
	int length;

	length = miniupnpd_lease_record_format(record, sizeof(record), "TCP",
	                                       1234, "192.168.1.10", 8080,
	                                       1700000000U, description);
	assert(length > 0 && (size_t)length == strlen(record));
	newline = strchr(record, '\n');
	assert(newline != NULL && newline == record + length - 1);
	assert(strchr(newline + 1, '\n') == NULL);
	assert(strchr(record, '\r') == NULL);
	assert(strstr(record, ":camera TCP:9999:192.168.1.2:22:0:injected display\n") != NULL);
}

static void
test_description_is_bounded(void)
{
	char record[128];
	char description[256];
	const char *last_colon;
	size_t i;
	int length;

	for (i = 0; i < sizeof(description) - 1; ++i)
		description[i] = 'x';
	description[sizeof(description) - 1] = '\0';

	length = miniupnpd_lease_record_format(record, sizeof(record), "UDPLITE",
	                                       65535, "255.255.255.255", 65535,
	                                       4294967295U, description);
	assert(length > 0 && length < (int)sizeof(record));
	last_colon = strrchr(record, ':');
	assert(last_colon != NULL);
	assert(strlen(last_colon + 1) == MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX + 1);
	assert(record[length - 1] == '\n');
}

static void
test_invalid_output_buffer(void)
{
	char buffer[8] = "stale";

	errno = 0;
	assert(miniupnpd_lease_record_format(buffer, sizeof(buffer), "TCP", 1,
	                                     "192.168.1.1", 1, 0, "x") == -1);
	assert(errno == ENOSPC);
	assert(buffer[0] == '\0');
	errno = 0;
	assert(miniupnpd_lease_record_format(buffer, 0, "TCP", 1,
	                                     "192.168.1.1", 1, 0, "x") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_lease_record_format(NULL, sizeof(buffer), "TCP", 1,
	                                     "192.168.1.1", 1, 0, "x") == -1);
	assert(errno == EINVAL);
}

static void
test_exact_record_key_matching(void)
{
	const char *key = "TCP:1234";
	const char *matching = "TCP:1234:192.168.1.2:80:0:camera\n";
	const char *longer_port = "TCP:12345:192.168.1.2:80:0:camera\n";
	const char *different_protocol = "UDP:1234:192.168.1.2:80:0:camera\n";
	const char *missing_separator = "TCP:1234x:192.168.1.2:80:0:camera\n";
	const char *short_record = "TCP:123";

	assert(miniupnpd_lease_record_key_matches(
		matching, strlen(matching), key, strlen(key)) == 1);
	assert(miniupnpd_lease_record_key_matches(
		longer_port, strlen(longer_port), key, strlen(key)) == 0);
	assert(miniupnpd_lease_record_key_matches(
		different_protocol, strlen(different_protocol), key, strlen(key)) == 0);
	assert(miniupnpd_lease_record_key_matches(
		missing_separator, strlen(missing_separator), key, strlen(key)) == 0);
	assert(miniupnpd_lease_record_key_matches(
		short_record, strlen(short_record), key, strlen(key)) == 0);
	assert(miniupnpd_lease_record_key_matches(NULL, 0, key, strlen(key)) == 0);
}

static int simulated_write_calls;

static ssize_t
partial_then_enospc_write(int fd, const void *buffer, size_t length)
{
	if (simulated_write_calls++ == 0 && length > 3)
		return write(fd, buffer, 3);
	errno = ENOSPC;
	return -1;
}

static void
test_lease_append_rolls_back_partial_write(void)
{
	char path[] = "/tmp/miniupnpd-lease-append-XXXXXX";
	const char original[] = "old-record\n";
	const char appended[] = "new-record\n";
	char actual[64];
	struct miniupnpd_lease_append_ops failing_ops = {
		lseek, partial_then_enospc_write, ftruncate, fsync
	};
	struct miniupnpd_lease_append_ops working_ops = {
		lseek, write, ftruncate, fsync
	};
	int fd;
	int rollback_failed = 0;
	ssize_t length;

	fd = mkstemp(path);
	assert(fd >= 0);
	assert(write(fd, original, sizeof(original) - 1) ==
	       (ssize_t)(sizeof(original) - 1));

	simulated_write_calls = 0;
	errno = 0;
	assert(miniupnpd_lease_append_transaction(
		fd, appended, sizeof(appended) - 1, &failing_ops,
		&rollback_failed) == -1);
	assert(errno == ENOSPC);
	assert(rollback_failed == 0);

	assert(lseek(fd, 0, SEEK_SET) == 0);
	length = read(fd, actual, sizeof(actual));
	assert(length == (ssize_t)(sizeof(original) - 1));
	assert(memcmp(actual, original, sizeof(original) - 1) == 0);

	rollback_failed = 0;
	assert(miniupnpd_lease_append_transaction(
		fd, appended, sizeof(appended) - 1, &working_ops,
		&rollback_failed) == 0);
	assert(rollback_failed == 0);
	assert(lseek(fd, 0, SEEK_SET) == 0);
	length = read(fd, actual, sizeof(actual));
	assert(length == (ssize_t)((sizeof(original) - 1) +
	                            (sizeof(appended) - 1)));
	assert(memcmp(actual, original, sizeof(original) - 1) == 0);
	assert(memcmp(actual + sizeof(original) - 1, appended,
	              sizeof(appended) - 1) == 0);

	assert(close(fd) == 0);
	assert(unlink(path) == 0);
}

int
main(void)
{
	test_single_line_description();
	test_description_is_bounded();
	test_invalid_output_buffer();
	test_exact_record_key_matching();
	test_lease_append_rolls_back_partial_write();
	puts("MiniUPnP lease-file record serialization policy: PASS");
	return 0;
}
