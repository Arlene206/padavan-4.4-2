#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "lanauth_socket_io_policy.h"

struct read_fixture {
    const unsigned char *source;
    size_t source_size;
    size_t source_offset;
    ssize_t read_results[8];
    size_t read_result_count;
    size_t read_calls;
    int wait_results[8];
    int wait_errors[8];
    size_t wait_result_count;
    size_t wait_calls;
    int wait_timeouts[8];
};

struct write_fixture {
    unsigned char output[64];
    size_t output_size;
    ssize_t write_results[8];
    size_t write_result_count;
    size_t write_calls;
};

static int
fake_wait_readable(void *opaque, int fd, int timeout_seconds)
{
    struct read_fixture *fixture = (struct read_fixture *)opaque;
    size_t index = fixture->wait_calls++;
    int result;

    (void)fd;
    if (index < sizeof(fixture->wait_timeouts) / sizeof(fixture->wait_timeouts[0]))
        fixture->wait_timeouts[index] = timeout_seconds;
    if (index >= fixture->wait_result_count)
        return 1;
    result = fixture->wait_results[index];
    if (result < 0)
        errno = fixture->wait_errors[index] ? fixture->wait_errors[index] : EIO;
    return result;
}

static ssize_t
fake_read(void *opaque, int fd, void *buffer, size_t size)
{
    struct read_fixture *fixture = (struct read_fixture *)opaque;
    size_t index = fixture->read_calls++;
    ssize_t result;
    size_t count;

    (void)fd;
    if (index < fixture->read_result_count)
        result = fixture->read_results[index];
    else
        result = (ssize_t)size;
    if (result < 0) {
        errno = (int)-result;
        return -1;
    }
    if (result == 0)
        return 0;
    count = (size_t)result;
    if (count > size)
        count = size;
    if (count > fixture->source_size - fixture->source_offset)
        count = fixture->source_size - fixture->source_offset;
    if (count == 0)
        return 0;
    memcpy(buffer, fixture->source + fixture->source_offset, count);
    fixture->source_offset += count;
    return (ssize_t)count;
}

static ssize_t
fake_write(void *opaque, int fd, const void *buffer, size_t size)
{
    struct write_fixture *fixture = (struct write_fixture *)opaque;
    size_t index = fixture->write_calls++;
    ssize_t result;
    size_t count;

    (void)fd;
    if (index < fixture->write_result_count)
        result = fixture->write_results[index];
    else
        result = (ssize_t)size;
    if (result < 0) {
        errno = (int)-result;
        return -1;
    }
    if (result == 0)
        return 0;
    count = (size_t)result;
    if (count > size)
        count = size;
    if (count > sizeof(fixture->output) - fixture->output_size)
        count = sizeof(fixture->output) - fixture->output_size;
    if (count == 0)
        return 0;
    memcpy(fixture->output + fixture->output_size, buffer, count);
    fixture->output_size += count;
    return (ssize_t)count;
}

static int
test_read_exact_reassembles_fragments(void)
{
    static const unsigned char input[] = "challenge-response-frame";
    struct read_fixture fixture;
    unsigned char output[sizeof(input) - 1];
    size_t completed;
    int stage, status;

    memset(&fixture, 0, sizeof(fixture));
    fixture.source = input;
    fixture.source_size = sizeof(input) - 1;
    fixture.read_results[0] = 2;
    fixture.read_results[1] = 1;
    fixture.read_results[2] = 3;
    fixture.read_result_count = 3;
    memset(output, 0, sizeof(output));
    status = lanauth_read_exact_with_ops(4, output, sizeof(output), 12, 3,
        fake_wait_readable, fake_read, &fixture, &stage, &completed);
    if (status != 0 || stage != LANAUTH_SOCKET_IO_STAGE_NONE ||
        completed != sizeof(output) || memcmp(output, input, sizeof(output)) != 0 ||
        fixture.wait_calls != 4 || fixture.wait_timeouts[0] != 12 ||
        fixture.wait_timeouts[1] != 3 || fixture.wait_timeouts[2] != 3 ||
        fixture.wait_timeouts[3] != 3)
        return 1;
    return 0;
}

static int
test_partial_read_times_out_closed_and_reports_progress(void)
{
    static const unsigned char input[] = "partial";
    struct read_fixture fixture;
    unsigned char output[sizeof(input) - 1];
    size_t completed;
    int stage, status;

    memset(&fixture, 0, sizeof(fixture));
    fixture.source = input;
    fixture.source_size = sizeof(input) - 1;
    fixture.read_results[0] = 2;
    fixture.read_result_count = 1;
    fixture.wait_results[0] = 1;
    fixture.wait_results[1] = 0;
    fixture.wait_result_count = 2;
    status = lanauth_read_exact_with_ops(4, output, sizeof(output), 12, 3,
        fake_wait_readable, fake_read, &fixture, &stage, &completed);
    if (status != ETIMEDOUT || stage != LANAUTH_SOCKET_IO_STAGE_TIMEOUT ||
        completed != 2 || fixture.wait_calls != 2)
        return 1;
    return 0;
}

static int
test_read_eof_and_interrupt_are_not_success(void)
{
    static const unsigned char input[] = "partial";
    struct read_fixture fixture;
    unsigned char output[sizeof(input) - 1];
    size_t completed;
    int stage, status;

    memset(&fixture, 0, sizeof(fixture));
    fixture.source = input;
    fixture.source_size = sizeof(input) - 1;
    fixture.read_results[0] = 2;
    fixture.read_results[1] = 0;
    fixture.read_result_count = 2;
    fixture.wait_results[0] = 1;
    fixture.wait_results[1] = 1;
    fixture.wait_result_count = 2;
    status = lanauth_read_exact_with_ops(4, output, sizeof(output), 12, 3,
        fake_wait_readable, fake_read, &fixture, &stage, &completed);
    if (status != ECONNRESET || stage != LANAUTH_SOCKET_IO_STAGE_READ ||
        completed != 2)
        return 1;

    memset(&fixture, 0, sizeof(fixture));
    fixture.source = input;
    fixture.source_size = sizeof(input) - 1;
    fixture.wait_results[0] = -1;
    fixture.wait_errors[0] = EINTR;
    fixture.wait_result_count = 1;
    status = lanauth_read_exact_with_ops(4, output, sizeof(output), 12, 3,
        fake_wait_readable, fake_read, &fixture, &stage, &completed);
    if (status != EINTR || stage != LANAUTH_SOCKET_IO_STAGE_WAIT || completed != 0)
        return 1;
    return 0;
}

static int
test_write_all_reassembles_short_writes(void)
{
    static const unsigned char input[] = "complete-digest";
    struct write_fixture fixture;
    size_t completed;
    int status;

    memset(&fixture, 0, sizeof(fixture));
    fixture.write_results[0] = 2;
    fixture.write_results[1] = 3;
    fixture.write_result_count = 2;
    status = lanauth_write_all_with_ops(5, input, sizeof(input) - 1,
        fake_write, &fixture, &completed);
    if (status != 0 || completed != sizeof(input) - 1 ||
        fixture.output_size != sizeof(input) - 1 ||
        memcmp(fixture.output, input, sizeof(input) - 1) != 0 ||
        fixture.write_calls != 3)
        return 1;
    return 0;
}

static int
test_partial_write_failure_requires_reconnect(void)
{
    static const unsigned char input[] = "complete-digest";
    struct write_fixture fixture;
    size_t completed;
    int status;

    memset(&fixture, 0, sizeof(fixture));
    fixture.write_results[0] = 3;
    fixture.write_results[1] = -EINTR;
    fixture.write_result_count = 2;
    status = lanauth_write_all_with_ops(5, input, sizeof(input) - 1,
        fake_write, &fixture, &completed);
    if (status != EINTR || completed != 3 || fixture.output_size != 3 ||
        fixture.write_calls != 2)
        return 1;

    memset(&fixture, 0, sizeof(fixture));
    fixture.write_results[0] = 0;
    fixture.write_result_count = 1;
    status = lanauth_write_all_with_ops(5, input, sizeof(input) - 1,
        fake_write, &fixture, &completed);
    if (status != EIO || completed != 0 || fixture.output_size != 0)
        return 1;
    return 0;
}

static int
test_invalid_callbacks_and_arguments_fail_closed(void)
{
    static const unsigned char input[] = "x";
    struct read_fixture read_fixture;
    struct write_fixture write_fixture;
    size_t completed;
    int status;

    memset(&read_fixture, 0, sizeof(read_fixture));
    memset(&write_fixture, 0, sizeof(write_fixture));
    read_fixture.source = input;
    read_fixture.source_size = sizeof(input) - 1;
    status = lanauth_read_exact_with_ops(4, (void *)input, 1, 1, 1,
        NULL, fake_read, &read_fixture, NULL, &completed);
    if (status != EINVAL)
        return 1;
    status = lanauth_write_all_with_ops(5, input, 1, NULL,
        &write_fixture, &completed);
    if (status != EINVAL)
        return 1;
    status = lanauth_read_exact_with_ops(-1, (void *)input, 1, 1, 1,
        fake_wait_readable, fake_read, &read_fixture, NULL, &completed);
    if (status != EINVAL)
        return 1;
    return 0;
}

int
main(void)
{
    if (test_read_exact_reassembles_fragments() != 0 ||
        test_partial_read_times_out_closed_and_reports_progress() != 0 ||
        test_read_eof_and_interrupt_are_not_success() != 0 ||
        test_write_all_reassembles_short_writes() != 0 ||
        test_partial_write_failure_requires_reconnect() != 0 ||
        test_invalid_callbacks_and_arguments_fail_closed() != 0) {
        fprintf(stderr, "LAN authentication framed socket I/O regression failed\n");
        return 1;
    }
    puts("LAN authentication exact socket I/O policy: PASS");
    return 0;
}
