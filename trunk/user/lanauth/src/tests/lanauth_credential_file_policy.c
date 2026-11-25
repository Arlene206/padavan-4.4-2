#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lanauth_credential_file_policy.h"

static int
make_file(const char *directory, const char *name, const char *contents,
          mode_t mode, char *path, size_t path_size)
{
    int fd;
    size_t length;
    ssize_t count;
    if (snprintf(path, path_size, "%s/%s", directory, name) < 0 ||
        strlen(path) >= path_size)
        return -1;
    fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0)
        return -1;
    if (fchmod(fd, mode) != 0) {
        close(fd);
        unlink(path);
        return -1;
    }
    length = strlen(contents);
    while (length > 0) {
        count = write(fd, contents, length);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0) {
            close(fd);
            unlink(path);
            return -1;
        }
        contents += count;
        length -= (size_t)count;
    }
    if (close(fd) != 0) {
        unlink(path);
        return -1;
    }
    return 0;
}

static int
is_absent(const char *path)
{
    struct stat st;
    errno = 0;
    return lstat(path, &st) != 0 && errno == ENOENT;
}

static int
buffer_is_clear(const char *password, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
        if (password[i] != '\0')
            return 0;
    return 1;
}

static int
test_valid_file_is_read_and_consumed(const char *directory)
{
    char path[256], password[64];
    memset(password, 'x', sizeof(password));
    if (make_file(directory, "valid", "secret", 0600,
                  path, sizeof(path)) != 0)
        return 1;
    if (lanauth_read_private_password_file(path, password,
                                            sizeof(password)) != 0 ||
        strcmp(password, "secret") != 0 || !is_absent(path))
        return 1;
    return 0;
}

static int
test_empty_file_fails_and_is_consumed(const char *directory)
{
    char path[256], password[64];
    if (make_file(directory, "empty", "", 0600, path, sizeof(path)) != 0)
        return 1;
    memset(password, 'x', sizeof(password));
    if (lanauth_read_private_password_file(path, password,
                                            sizeof(password)) != EINVAL ||
        !buffer_is_clear(password, sizeof(password)) || !is_absent(path))
        return 1;
    return 0;
}

static int
test_oversized_file_fails_and_is_consumed(const char *directory)
{
    char path[256], password[64];
    if (make_file(directory, "long", "123456789012345678901", 0600,
                  path, sizeof(path)) != 0)
        return 1;
    memset(password, 'x', sizeof(password));
    if (lanauth_read_private_password_file(path, password,
                                            sizeof(password)) != ENAMETOOLONG ||
        !buffer_is_clear(password, sizeof(password)) || !is_absent(path))
        return 1;
    return 0;
}

static int
test_insecure_mode_fails_and_is_consumed(const char *directory)
{
    char path[256], password[64];
    if (make_file(directory, "insecure", "secret", 0644,
                  path, sizeof(path)) != 0)
        return 1;
    memset(password, 'x', sizeof(password));
    if (lanauth_read_private_password_file(path, password,
                                            sizeof(password)) != EACCES ||
        !buffer_is_clear(password, sizeof(password)) || !is_absent(path))
        return 1;
    return 0;
}

static int
test_symlink_is_rejected_without_touching_target(const char *directory)
{
    char target[256], linkpath[256], password[64];
    if (make_file(directory, "target", "secret", 0600,
                  target, sizeof(target)) != 0)
        return 1;
    if (snprintf(linkpath, sizeof(linkpath), "%s/link", directory) < 0 ||
        symlink(target, linkpath) != 0)
        return 1;
    memset(password, 'x', sizeof(password));
    if (lanauth_read_private_password_file(linkpath, password,
                                            sizeof(password)) != ELOOP ||
        !buffer_is_clear(password, sizeof(password)) ||
        access(linkpath, F_OK) != 0 || access(target, F_OK) != 0)
        return 1;
    unlink(linkpath);
    unlink(target);
    return 0;
}

static int
test_missing_file_fails_closed(const char *directory)
{
    char path[256], password[64];
    if (snprintf(path, sizeof(path), "%s/missing", directory) < 0)
        return 1;
    memset(password, 'x', sizeof(password));
    if (lanauth_read_private_password_file(path, password,
                                            sizeof(password)) != ENOENT ||
        !buffer_is_clear(password, sizeof(password)))
        return 1;
    return 0;
}

int
main(void)
{
    char directory[] = "/tmp/lanauth-credential-test.XXXXXX";
    if (!mkdtemp(directory))
        return 1;
    if (test_valid_file_is_read_and_consumed(directory) != 0 ||
        test_empty_file_fails_and_is_consumed(directory) != 0 ||
        test_oversized_file_fails_and_is_consumed(directory) != 0 ||
        test_insecure_mode_fails_and_is_consumed(directory) != 0 ||
        test_symlink_is_rejected_without_touching_target(directory) != 0 ||
        test_missing_file_fails_closed(directory) != 0) {
        fprintf(stderr, "LAN authentication credential-file policy regression failed\n");
        return 1;
    }
    if (rmdir(directory) != 0)
        return 1;
    puts("LAN authentication private credential-file policy: PASS");
    return 0;
}
