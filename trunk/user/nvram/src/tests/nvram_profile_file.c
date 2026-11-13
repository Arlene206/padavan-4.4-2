#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "nvram_profile_file.h"

static char last_temp[512];
static int write_mode, write_calls, fsync_mode, fsync_calls;

static int test_mkstemp(char *path)
{
    int fd = mkstemp(path);
    if (fd >= 0)
        snprintf(last_temp, sizeof(last_temp), "%s", path);
    return fd;
}

static ssize_t test_write(int fd, const void *buffer, size_t length)
{
    ++write_calls;
    if (write_mode == 1 && write_calls == 1)
        return write(fd, buffer, length > 3 ? 3 : length);
    if (write_mode == 1 && write_calls == 2) {
        errno = EINTR;
        return -1;
    }
    if (write_mode == 2 && write_calls == 1)
        return write(fd, buffer, length > 2 ? 2 : length);
    if (write_mode == 2) {
        errno = EIO;
        return -1;
    }
    return write(fd, buffer, length);
}

static int test_fsync(int fd)
{
    ++fsync_calls;
    if (fsync_mode == 1 && fsync_calls == 1) {
        errno = EINTR;
        return -1;
    }
    if (fsync_mode == 2) {
        errno = EIO;
        return -1;
    }
    return fsync(fd);
}

static int fail_fchmod(int fd, mode_t mode)
{
    (void)fd;
    (void)mode;
    errno = EIO;
    return -1;
}

static int fail_rename(const char *old_path, const char *new_path)
{
    (void)old_path;
    (void)new_path;
    errno = EIO;
    return -1;
}

static int fail_close(int fd)
{
    int result = close(fd);
    if (result != 0)
        return result;
    errno = EIO;
    return -1;
}

static void ops_init(struct nvram_profile_file_ops *ops)
{
    ops->mkstemp_fn = test_mkstemp;
    ops->fchmod_fn = fchmod;
    ops->write_fn = test_write;
    ops->fsync_fn = test_fsync;
    ops->close_fn = close;
    ops->rename_fn = rename;
    ops->unlink_fn = unlink;
}

static void put_file(const char *path, const char *value, mode_t mode)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
    size_t length = strlen(value);
    assert(fd >= 0);
    assert(write(fd, value, length) == (ssize_t)length);
    assert(close(fd) == 0);
    assert(chmod(path, mode) == 0);
}

static void expect_file(const char *path, const char *value)
{
    char buffer[128];
    ssize_t count;
    int fd = open(path, O_RDONLY);
    assert(fd >= 0);
    count = read(fd, buffer, sizeof(buffer) - 1);
    assert(count >= 0);
    assert(close(fd) == 0);
    buffer[count] = '\0';
    assert(strcmp(buffer, value) == 0);
}

static void expect_published_profile(const char *path)
{
    static const unsigned char expected[] = {
        'H', 'D', 'R', '2', 0x04, 0x00, 0x00, 0x2a,
        'k', '=', 'v', 0, 0, 0xfd, 0xfe, 0xff
    };
    unsigned char actual[32];
    ssize_t count;
    int fd = open(path, O_RDONLY);

    assert(fd >= 0);
    count = read(fd, actual, sizeof(actual));
    assert(count == (ssize_t)sizeof(expected));
    assert(close(fd) == 0);
    assert(memcmp(actual, expected, sizeof(expected)) == 0);
}

static void expect_temp_removed(void)
{
    assert(last_temp[0] != '\0');
    assert(access(last_temp, F_OK) < 0 && errno == ENOENT);
}

static int save(const char *path, const struct nvram_profile_file_ops *ops)
{
    static const unsigned char header[] = {
        'H', 'D', 'R', '2', 0x04, 0x00, 0x00, 0x2a
    };
    static const unsigned char payload[] = { 'k', '=', 'v', 0, 0 };
    static const unsigned char padding[] = { 0xfd, 0xfe, 0xff };

    return nvram_profile_write_atomic_with_ops(
        path, header, sizeof(header), payload, sizeof(payload),
        padding, sizeof(padding), ops);
}

int main(void)
{
    char dir[] = "/tmp/nvram-profile-file.XXXXXX";
    char target[256], new_target[256], victim[256];
    struct nvram_profile_file_ops ops;
    struct stat st;

    assert(mkdtemp(dir) != NULL);
    snprintf(target, sizeof(target), "%s/profile.prf", dir);
    snprintf(new_target, sizeof(new_target), "%s/new-profile.prf", dir);
    snprintf(victim, sizeof(victim), "%s/victim", dir);

    /* The new file is private, complete and atomically replaces the old file. */
    put_file(target, "old-good-profile", 0640);
    ops_init(&ops);
    write_mode = 1;
    write_calls = 0;
    fsync_mode = 1;
    fsync_calls = 0;
    last_temp[0] = '\0';
    assert(save(target, &ops) == 0);
    assert(write_calls >= 3 && fsync_calls == 2);
    assert(stat(target, &st) == 0 && (st.st_mode & 0777) == 0600);
    expect_published_profile(target);
    expect_temp_removed();

    /* A new target is also created privately. */
    ops_init(&ops);
    write_mode = 0;
    write_calls = 0;
    fsync_mode = 0;
    fsync_calls = 0;
    last_temp[0] = '\0';
    assert(save(new_target, &ops) == 0);
    assert(stat(new_target, &st) == 0 && (st.st_mode & 0777) == 0600);
    expect_temp_removed();

    /* Short write followed by a hard write error keeps the old profile. */
    put_file(target, "last-good", 0640);
    ops_init(&ops);
    write_mode = 2;
    write_calls = 0;
    last_temp[0] = '\0';
    assert(save(target, &ops) < 0);
    expect_file(target, "last-good");
    expect_temp_removed();

    /* fchmod, fsync, close and rename errors all retain old content. */
    ops_init(&ops);
    ops.fchmod_fn = fail_fchmod;
    last_temp[0] = '\0';
    assert(save(target, &ops) < 0);
    expect_file(target, "last-good");
    expect_temp_removed();

    ops_init(&ops);
    write_mode = 0;
    write_calls = 0;
    fsync_mode = 2;
    fsync_calls = 0;
    last_temp[0] = '\0';
    assert(save(target, &ops) < 0);
    expect_file(target, "last-good");
    expect_temp_removed();

    ops_init(&ops);
    write_mode = 0;
    write_calls = 0;
    fsync_mode = 0;
    fsync_calls = 0;
    ops.close_fn = fail_close;
    last_temp[0] = '\0';
    assert(save(target, &ops) < 0);
    expect_file(target, "last-good");
    expect_temp_removed();

    ops_init(&ops);
    write_mode = 0;
    write_calls = 0;
    fsync_mode = 0;
    fsync_calls = 0;
    ops.rename_fn = fail_rename;
    last_temp[0] = '\0';
    assert(save(target, &ops) < 0);
    expect_file(target, "last-good");
    expect_temp_removed();

    /* Never follow a pre-existing symlink or replace a non-regular object. */
    put_file(victim, "protected", 0600);
    assert(unlink(target) == 0);
    assert(symlink(victim, target) == 0);
    errno = 0;
    assert(save(target, NULL) < 0 && errno == EINVAL);
    expect_file(victim, "protected");
    assert(unlink(target) == 0);
    assert(mkdir(target, 0700) == 0);
    errno = 0;
    assert(save(target, NULL) < 0 && errno == EINVAL);
    assert(rmdir(target) == 0);

    assert(unlink(new_target) == 0);
    assert(unlink(victim) == 0);
    assert(rmdir(dir) == 0);
    puts("NVRAM profile atomic publication: PASS");
    return 0;
}
