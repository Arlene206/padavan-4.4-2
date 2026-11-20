#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "auth_startup_handshake.h"

struct fake_startup_io {
	unsigned char value;
	unsigned int read_calls;
	unsigned int write_calls;
	unsigned int read_interrupts;
	unsigned int write_interrupts;
	int read_error;
	int write_error;
	int read_eof;
};

static ssize_t
fake_read(void *context, int fd, void *buffer, size_t length)
{
	struct fake_startup_io *fake = (struct fake_startup_io *)context;

	(void)fd;
	fake->read_calls++;
	if (fake->read_interrupts != 0) {
		--fake->read_interrupts;
		errno = EINTR;
		return -1;
	}
	if (fake->read_error != 0) {
		errno = fake->read_error;
		return -1;
	}
	if (fake->read_eof)
		return 0;
	if (length != sizeof(fake->value)) {
		errno = EINVAL;
		return -1;
	}
	*(unsigned char *)buffer = fake->value;
	return (ssize_t)sizeof(fake->value);
}

static ssize_t
fake_write(void *context, int fd, const void *buffer, size_t length)
{
	struct fake_startup_io *fake = (struct fake_startup_io *)context;

	(void)fd;
	fake->write_calls++;
	if (fake->write_interrupts != 0) {
		--fake->write_interrupts;
		errno = EINTR;
		return -1;
	}
	if (fake->write_error != 0) {
		errno = fake->write_error;
		return -1;
	}
	if (length != sizeof(fake->value)) {
		errno = EINVAL;
		return -1;
	}
	fake->value = *(const unsigned char *)buffer;
	return (ssize_t)sizeof(fake->value);
}

static const struct auth_startup_io_ops fake_ops = {
	fake_read,
	fake_write
};

static void
test_interface_name_formatting_is_bounded(void)
{
	char name[16];
	const char max_prefix[] = "01234567890123";
	const char too_long_prefix[] = "012345678901234";

	assert(auth_startup_format_interface_name(name, sizeof(name),
						  "ra", 0) == 0);
	assert(strcmp(name, "ra0") == 0);
	assert(auth_startup_format_interface_name(name, sizeof(name),
						  max_prefix, 0) == 0);
	assert(strlen(name) == sizeof(name) - 1);

	assert(auth_startup_format_interface_name(name, sizeof(name),
						  max_prefix, 10) == -1);
	assert(errno == ENAMETOOLONG);
	assert(name[0] == '\0');
	assert(auth_startup_format_interface_name(name, sizeof(name),
						  too_long_prefix, 0) == -1);
	assert(errno == ENAMETOOLONG);
	assert(name[0] == '\0');
	assert(auth_startup_format_interface_name(name, 2, "ra", 0) == -1);
	assert(errno == ENAMETOOLONG);
	assert(auth_startup_format_interface_name(name, 0, "ra", 0) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_format_interface_name(name, sizeof(name), "", 0) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_format_interface_name(NULL, sizeof(name), "ra", 0) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_format_interface_name(name, sizeof(name), NULL, 0) == -1);
	assert(errno == EINVAL);
}

static void
test_driver_interface_name_copy_is_bounded(void)
{
	char name[16];
	const char ordinary[] = { 'r', 'a', '0' };
	const char terminated[] = { 'r', 'a', '0', '\0' };
	const char maximum[] = "012345678901234";
	const char too_long[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '0', '1', '2', '3', '4', '5'
	};
	const char embedded_nul[] = { 'r', '\0', 'a' };

	assert(auth_startup_copy_interface_name(name, sizeof(name),
						ordinary, sizeof(ordinary),
						sizeof(ordinary)) == 0);
	assert(strcmp(name, "ra0") == 0);
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						terminated, sizeof(terminated),
						sizeof(terminated)) == 0);
	assert(strcmp(name, "ra0") == 0);
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						maximum, sizeof(maximum) - 1,
						sizeof(maximum) - 1) == 0);
	assert(strlen(name) == sizeof(name) - 1);
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						too_long, sizeof(too_long),
						sizeof(too_long)) == -1);
	assert(errno == ENAMETOOLONG);
	assert(name[0] == '\0');
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						ordinary, 2, sizeof(ordinary)) == -1);
	assert(errno == EINVAL);
	assert(name[0] == '\0');
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						embedded_nul, sizeof(embedded_nul),
						sizeof(embedded_nul)) == -1);
	assert(errno == EINVAL);
	assert(name[0] == '\0');
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						ordinary, sizeof(ordinary), 0) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_copy_interface_name(name, 2, ordinary,
						sizeof(ordinary), sizeof(ordinary)) == -1);
	assert(errno == ENAMETOOLONG);
	assert(auth_startup_copy_interface_name(NULL, sizeof(name),
						ordinary, sizeof(ordinary),
						sizeof(ordinary)) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_copy_interface_name(name, sizeof(name),
						NULL, 0, 0) == -1);
	assert(errno == EINVAL);
}

static void
test_driver_radius_fields_are_bounded(void)
{
	unsigned char key[64];
	unsigned char *copy;
	const unsigned char short_key[] = { 's', 'e', 'c', 'r', 'e', 't' };

	assert(auth_startup_validate_driver_count(0, 2) == 0);
	assert(auth_startup_validate_driver_count(2, 2) == 0);
	assert(auth_startup_validate_driver_count(3, 2) == -1);
	assert(errno == EINVAL);

	memset(key, 0x5a, sizeof(key));
	copy = auth_startup_copy_radius_secret(short_key,
					       sizeof(short_key),
					       sizeof(short_key));
	assert(copy != NULL);
	assert(memcmp(copy, short_key, sizeof(short_key)) == 0);
	assert(copy[sizeof(short_key)] == '\0');
	free(copy);

	copy = auth_startup_copy_radius_secret(key, sizeof(key), sizeof(key));
	assert(copy != NULL);
	assert(memcmp(copy, key, sizeof(key)) == 0);
	assert(copy[sizeof(key)] == '\0');
	free(copy);

	copy = auth_startup_copy_radius_secret(key, sizeof(key), sizeof(key) + 1);
	assert(copy == NULL);
	assert(errno == EINVAL);
	copy = auth_startup_copy_radius_secret(key, 2, sizeof(short_key));
	assert(copy == NULL);
	assert(errno == EINVAL);
	copy = auth_startup_copy_radius_secret(key, sizeof(key), 0);
	assert(copy == NULL);
	assert(errno == EINVAL);
	copy = auth_startup_copy_radius_secret(NULL, sizeof(key), sizeof(key));
	assert(copy == NULL);
	assert(errno == EINVAL);
}

static void
test_real_pipe_round_trip(void)
{
	int pipe_fds[2];
	unsigned char result = 0xff;

	assert(pipe(pipe_fds) == 0);
	assert(auth_startup_write_result(pipe_fds[1], 0) == 0);
	assert(auth_startup_read_result(pipe_fds[0], &result) == 0);
	assert(result == 0);
	assert(close(pipe_fds[0]) == 0);
	assert(close(pipe_fds[1]) == 0);
}

static void
test_child_failure_byte_is_not_success(void)
{
	struct fake_startup_io fake;
	unsigned char result = 0;

	memset(&fake, 0, sizeof(fake));
	fake.value = 1;
	assert(auth_startup_read_result_with(3, &result, &fake_ops, &fake) == 0);
	assert(result == 1);
	assert(fake.read_calls == 1);
}

static void
test_interrupted_io_retries_are_bounded_and_complete(void)
{
	struct fake_startup_io fake;
	unsigned char result = 0;

	memset(&fake, 0, sizeof(fake));
	fake.read_interrupts = 2;
	fake.write_interrupts = 3;
	assert(auth_startup_write_result_with(4, 0, &fake_ops, &fake) == 0);
	assert(fake.write_calls == 4);
	assert(fake.value == 0);
	fake.value = 1;
	assert(auth_startup_read_result_with(4, &result, &fake_ops, &fake) == 0);
	assert(fake.read_calls == 3);
	assert(result == 1);
}

static void
test_eof_is_not_a_ready_notification(void)
{
	struct fake_startup_io fake;
	unsigned char result = 0;

	memset(&fake, 0, sizeof(fake));
	fake.read_eof = 1;
	assert(auth_startup_read_result_with(5, &result, &fake_ops, &fake) == -1);
	assert(errno == EPIPE);
	assert(fake.read_calls == 1);
}

static void
test_io_errors_are_preserved(void)
{
	struct fake_startup_io fake;
	unsigned char result = 0;

	memset(&fake, 0, sizeof(fake));
	fake.write_error = ENOSPC;
	assert(auth_startup_write_result_with(6, 1, &fake_ops, &fake) == -1);
	assert(errno == ENOSPC);
	memset(&fake, 0, sizeof(fake));
	fake.read_error = EACCES;
	assert(auth_startup_read_result_with(6, &result, &fake_ops, &fake) == -1);
	assert(errno == EACCES);
}

static void
test_interrupt_storm_is_bounded(void)
{
	struct fake_startup_io fake;

	memset(&fake, 0, sizeof(fake));
	fake.write_interrupts = AUTH_STARTUP_IO_MAX_INTERRUPTS + 1;
	assert(auth_startup_write_result_with(7, 0, &fake_ops, &fake) == -1);
	assert(errno == EINTR);
	assert(fake.write_calls == AUTH_STARTUP_IO_MAX_INTERRUPTS + 1);
}

static void
test_invalid_arguments_are_rejected(void)
{
	struct fake_startup_io fake;
	unsigned char result = 0;

	memset(&fake, 0, sizeof(fake));
	assert(auth_startup_write_result_with(-1, 0, &fake_ops, &fake) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_read_result_with(3, NULL, &fake_ops, &fake) == -1);
	assert(errno == EINVAL);
	assert(auth_startup_read_result_with(3, &result, NULL, &fake) == -1);
	assert(errno == EINVAL);
	assert(fake.read_calls == 0);
	assert(fake.write_calls == 0);
}

int
main(void)
{
	test_driver_radius_fields_are_bounded();
	test_driver_interface_name_copy_is_bounded();
	test_interface_name_formatting_is_bounded();
	test_real_pipe_round_trip();
	test_child_failure_byte_is_not_success();
	test_interrupted_io_retries_are_bounded_and_complete();
	test_eof_is_not_a_ready_notification();
	test_io_errors_are_preserved();
	test_interrupt_storm_is_bounded();
	test_invalid_arguments_are_rejected();
	puts("Ralink 802.1X startup handshake regression: PASS");
	return 0;
}
