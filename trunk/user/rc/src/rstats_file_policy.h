#ifndef RSTATS_FILE_POLICY_H
#define RSTATS_FILE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct rstats_file_ops {
  int (*mkstemp_fn)(char *template_path);
  int (*fchmod_fn)(int fd, mode_t mode);
  ssize_t (*write_fn)(int fd, const void *buffer, size_t length);
  int (*fsync_fn)(int fd);
  int (*close_fn)(int fd);
  int (*rename_fn)(const char *old_path, const char *new_path);
  int (*unlink_fn)(const char *path);
};

static const struct rstats_file_ops rstats_file_system_ops = {
  mkstemp, fchmod, write, fsync, close, rename, unlink
};

static inline int
rstats_file_fsync_retry(int fd, int (*fsync_fn)(int))
{
  int result;
  do {
    result = fsync_fn(fd);
  } while (result < 0 && errno == EINTR);
  return result;
}

/*
 * Publish a complete replacement in the target directory. A failed write,
 * sync, close or rename leaves the previous target untouched.
 */
static inline int
rstats_file_write_atomic_with_ops(const char *path, const void *buffer,
                                  size_t length,
                                  const struct rstats_file_ops *ops)
{
  char temporary[PATH_MAX];
  struct stat existing;
  mode_t mode = 0666;
  size_t written = 0;
  int fd = -1;
  int created = 0;
  int saved_errno;
  int count;

  if (!path || !*path || (!buffer && length != 0)) {
    errno = EINVAL;
    return -1;
  }
  if (!ops)
    ops = &rstats_file_system_ops;
  if (!ops->mkstemp_fn || !ops->fchmod_fn || !ops->write_fn ||
      !ops->fsync_fn || !ops->close_fn || !ops->rename_fn ||
      !ops->unlink_fn) {
    errno = EINVAL;
    return -1;
  }

  if (lstat(path, &existing) == 0) {
    if (!S_ISREG(existing.st_mode)) {
      errno = EINVAL;
      return -1;
    }
    mode = existing.st_mode & 0777;
  } else if (errno != ENOENT) {
    return -1;
  }

  count = snprintf(temporary, sizeof(temporary), "%s.tmp.XXXXXX", path);
  if (count < 0 || (size_t) count >= sizeof(temporary)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  fd = ops->mkstemp_fn(temporary);
  if (fd < 0)
    return -1;
  created = 1;

  if (ops->fchmod_fn(fd, mode) != 0)
    goto fail;

  while (written < length) {
    ssize_t result = ops->write_fn(fd,
        (const char *) buffer + written, length - written);
    if (result < 0) {
      if (errno == EINTR)
        continue;
      goto fail;
    }
    if (result == 0) {
      errno = EIO;
      goto fail;
    }
    written += (size_t) result;
  }

  if (rstats_file_fsync_retry(fd, ops->fsync_fn) != 0)
    goto fail;

  if (ops->close_fn(fd) != 0) {
    fd = -1; /* Linux closes the descriptor even when close reports EINTR. */
    goto fail;
  }
  fd = -1;

  if (ops->rename_fn(temporary, path) != 0)
    goto fail;

  return 0;

fail:
  saved_errno = errno ? errno : EIO;
  if (fd >= 0)
    (void) ops->close_fn(fd);
  if (created)
    (void) ops->unlink_fn(temporary);
  errno = saved_errno;
  return -1;
}

static inline int
rstats_file_write_atomic(const char *path, const void *buffer, size_t length)
{
  return rstats_file_write_atomic_with_ops(path, buffer, length,
                                           &rstats_file_system_ops);
}

/*
 * Open a unique, same-directory stream staging file without following a
 * predictable temporary pathname. Preserve the published file mode when it
 * already exists; new files retain the daemon's historical 0666 mode.
 */
static inline FILE *
rstats_file_open_stream(const char *target, char *temporary,
                        size_t temporary_size)
{
  struct stat existing;
  mode_t mode = 0666;
  int fd;
  int saved_errno;
  int count;
  FILE *stream;

  if (!target || !*target || !temporary || temporary_size == 0) {
    errno = EINVAL;
    return NULL;
  }

  if (lstat(target, &existing) == 0) {
    if (!S_ISREG(existing.st_mode)) {
      errno = EINVAL;
      return NULL;
    }
    mode = existing.st_mode & 0777;
  } else if (errno != ENOENT) {
    return NULL;
  }

  count = snprintf(temporary, temporary_size, "%s.tmp.XXXXXX", target);
  if (count < 0 || (size_t) count >= temporary_size) {
    errno = ENAMETOOLONG;
    return NULL;
  }

  fd = mkstemp(temporary);
  if (fd < 0)
    return NULL;
  if (fchmod(fd, mode) != 0)
    goto fail;

  stream = fdopen(fd, "w");
  if (stream)
    return stream;

fail:
  saved_errno = errno ? errno : EIO;
  (void) close(fd);
  (void) unlink(temporary);
  errno = saved_errno;
  return NULL;
}

/*
 * Finish a stdio-generated staging file only after buffered output, fsync and
 * close have succeeded. The caller must use a same-directory staging path.
 */
static inline int
rstats_file_publish_stream(FILE *stream, const char *temporary,
                           const char *target)
{
  int fd;
  int saved_errno = 0;

  if (!stream || !temporary || !*temporary || !target || !*target) {
    errno = EINVAL;
    return -1;
  }

  fd = fileno(stream);
  if (fd < 0)
    saved_errno = errno ? errno : EIO;
  if (ferror(stream) && saved_errno == 0)
    saved_errno = EIO;
  if (fflush(stream) != 0 && saved_errno == 0)
    saved_errno = errno ? errno : EIO;
  if (saved_errno == 0 && rstats_file_fsync_retry(fd, fsync) != 0)
    saved_errno = errno ? errno : EIO;
  if (fclose(stream) != 0 && saved_errno == 0)
    saved_errno = errno ? errno : EIO;

  if (saved_errno == 0) {
    if (rename(temporary, target) == 0)
      return 0;
    saved_errno = errno ? errno : EIO;
  }

  (void) unlink(temporary);
  errno = saved_errno;
  return -1;
}

#endif /* RSTATS_FILE_POLICY_H */
