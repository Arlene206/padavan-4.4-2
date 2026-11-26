/* GNU89 regression tests for atomic DHCP lease-file replacement. */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "udhcp_lease_file_atomic.h"

enum failure_mode {
    FAIL_NONE,
    FAIL_MKSTEMP,
    FAIL_PARTIAL_WRITE,
    FAIL_FSYNC,
    FAIL_CLOSE,
    FAIL_RENAME,
    FAIL_OPEN_DIR,
    FAIL_DIR_FSYNC
};

static enum failure_mode active_failure;
static int write_calls;
static int mkstemp_calls;
static int directory_fsync_calls;

static int
test_mkstemp(char *path_template)
{
    mkstemp_calls++;
    if (active_failure == FAIL_MKSTEMP) {
        errno = EACCES;
        return -1;
    }
    return mkstemp(path_template);
}

static ssize_t
test_write(int fd, const void *buffer, size_t length)
{
    if (active_failure == FAIL_PARTIAL_WRITE) {
        if (write_calls++ != 0) {
            errno = EIO;
            return -1;
        }
        if (length > 1)
            length /= 2;
    }
    return write(fd, buffer, length);
}

static int
test_fsync(int fd)
{
    if (active_failure == FAIL_FSYNC) {
        errno = EIO;
        return -1;
    }
    return fsync(fd);
}

static int
test_close(int fd)
{
    int result = close(fd);

    if (active_failure == FAIL_CLOSE) {
        errno = EIO;
        return -1;
    }
    return result;
}

static int
test_rename(const char *old_path, const char *new_path)
{
    if (active_failure == FAIL_RENAME) {
        errno = EIO;
        return -1;
    }
    return rename(old_path, new_path);
}

static int
test_unlink(const char *path)
{
    return unlink(path);
}

static int
test_open_dir(const char *path)
{
    if (active_failure == FAIL_OPEN_DIR) {
        errno = EACCES;
        return -1;
    }
    return open(path, O_RDONLY | O_DIRECTORY);
}

static int
test_dir_fsync(int fd)
{
    directory_fsync_calls++;
    if (active_failure == FAIL_DIR_FSYNC) {
        errno = EIO;
        return -1;
    }
    return fsync(fd);
}

static int
test_close_dir(int fd)
{
    return close(fd);
}

static const struct udhcp_lease_file_io_ops test_ops = {
    test_mkstemp,
    test_write,
    test_fsync,
    test_close,
    test_rename,
    test_unlink,
    test_open_dir,
    test_dir_fsync,
    test_close_dir
};

static void
write_original(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);

    assert(fd >= 0);
    assert(write(fd, "old-valid", 9) == 9);
    assert(close(fd) == 0);
}

static void
assert_file_contents(const char *path, const char *expected)
{
    char buffer[64];
    ssize_t length;
    int fd = open(path, O_RDONLY);

    assert(fd >= 0);
    length = read(fd, buffer, sizeof(buffer) - 1);
    assert(length >= 0);
    buffer[length] = '\0';
    assert(close(fd) == 0);
    assert(strcmp(buffer, expected) == 0);
}

static void
test_success_replaces_file_atomically(const char *path)
{
    active_failure = FAIL_NONE;
    write_calls = 0;
    directory_fsync_calls = 0;
    write_original(path);
    assert(udhcp_lease_file_write_atomic(path, "new-valid-data", 14,
                                         &test_ops) == 0);
    assert_file_contents(path, "new-valid-data");
    assert(directory_fsync_calls == 1);
}

static void
test_pre_rename_failures_preserve_old_file(const char *path)
{
    enum failure_mode mode;

    for (mode = FAIL_MKSTEMP; mode <= FAIL_RENAME; mode++) {
        char template_path[300];

        active_failure = mode;
        write_calls = 0;
        write_original(path);
        if (mode == FAIL_MKSTEMP) {
            assert(snprintf(template_path, sizeof(template_path),
                            "%s.tmp.XXXXXX", path) > 0);
            write_original(template_path);
        }
        assert(udhcp_lease_file_write_atomic(path, "replacement", 11,
                                             &test_ops) == -1);
        assert_file_contents(path, "old-valid");
        if (mode == FAIL_MKSTEMP) {
            assert_file_contents(template_path, "old-valid");
            assert(unlink(template_path) == 0);
        }
    }
    active_failure = FAIL_NONE;
}


static void
test_directory_open_failure_preserves_old_file(const char *path)
{
    active_failure = FAIL_OPEN_DIR;
    mkstemp_calls = 0;
    write_original(path);
    assert(udhcp_lease_file_write_atomic(path, "replacement", 11,
                                         &test_ops) == -1);
    assert(mkstemp_calls == 0);
    assert_file_contents(path, "old-valid");
}

static void
test_directory_fsync_failure_reports_published_file(const char *path)
{
    active_failure = FAIL_DIR_FSYNC;
    directory_fsync_calls = 0;
    write_original(path);
    assert(udhcp_lease_file_write_atomic(path, "published", 9,
                 &test_ops) == UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED);
    assert(directory_fsync_calls == 1);
    assert_file_contents(path, "published");
}

int
main(void)
{
    char directory[] = "/tmp/udhcp-lease-atomic.XXXXXX";
    char path[256];

    assert(mkdtemp(directory) != NULL);
    assert(snprintf(path, sizeof(path), "%s/leases", directory) > 0);
    test_success_replaces_file_atomically(path);
    test_pre_rename_failures_preserve_old_file(path);
    test_directory_open_failure_preserves_old_file(path);
    test_directory_fsync_failure_reports_published_file(path);
    active_failure = FAIL_NONE;
    assert(unlink(path) == 0);
    assert(rmdir(directory) == 0);
    puts("DHCP lease file atomic replacement regression: PASS");
    return 0;
}
