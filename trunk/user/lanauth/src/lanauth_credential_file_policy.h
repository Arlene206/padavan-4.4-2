#ifndef LANAUTH_CREDENTIAL_FILE_POLICY_H
#define LANAUTH_CREDENTIAL_FILE_POLICY_H
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define LANAUTH_PASSWORD_MAX_LENGTH 20
static int
lanauth_read_private_password_file(const char *path, char *password,
                                   size_t password_size)
{
    struct stat opened_stat, named_stat;
    size_t expected, used;
    ssize_t count;
    int fd = -1, have_opened_stat = 0, status = 0;

    if (!password) {
        errno = EINVAL;
        return EINVAL;
    }
    memset(password, 0, password_size);
    if (!path || !path[0] ||
        password_size <= LANAUTH_PASSWORD_MAX_LENGTH) {
        errno = EINVAL;
        return EINVAL;
    }
    fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) {
        status = errno ? errno : EIO;
        goto done;
    }
    if (fstat(fd, &opened_stat) != 0) {
        status = errno ? errno : EIO;
        goto done;
    }
    have_opened_stat = 1;
    if (!S_ISREG(opened_stat.st_mode) ||
        opened_stat.st_uid != geteuid() ||
        opened_stat.st_nlink != 1 ||
        (opened_stat.st_mode & 07777) != 0600) {
        status = EACCES;
        goto consume;
    }
    if (opened_stat.st_size <= 0) {
        status = EINVAL;
        goto consume;
    }
    if (opened_stat.st_size > LANAUTH_PASSWORD_MAX_LENGTH ||
        (size_t)opened_stat.st_size >= password_size) {
        status = ENAMETOOLONG;
        goto consume;
    }
    expected = (size_t)opened_stat.st_size;
    used = 0;
    while (used < expected) {
        count = read(fd, password + used, expected - used);
        if (count < 0 && errno == EINTR)
            continue;
        if (count < 0) {
            status = errno ? errno : EIO;
            goto consume;
        }
        if (count == 0) {
            status = EIO;
            goto consume;
        }
        used += (size_t)count;
    }
    if (memchr(password, '\0', used) != NULL) {
        status = EINVAL;
        goto consume;
    }
    password[used] = '\0';

consume:
    /* Do not unlink a replaced pathname: it must still name our opened inode. */
    if (have_opened_stat) {
        if (lstat(path, &named_stat) != 0) {
            if (status == 0)
                status = errno ? errno : EIO;
        } else if (S_ISREG(named_stat.st_mode) &&
                   named_stat.st_uid == geteuid() &&
                   named_stat.st_dev == opened_stat.st_dev &&
                   named_stat.st_ino == opened_stat.st_ino) {
            if (unlink(path) != 0 && status == 0)
                status = errno ? errno : EIO;
        } else if (status == 0) {
            status = EIO;
        }
    }
done:
    if (fd >= 0 && close(fd) != 0 && status == 0)
        status = errno ? errno : EIO;
    if (status != 0)
        memset(password, 0, password_size);
    errno = status;
    return status;
}
#endif
