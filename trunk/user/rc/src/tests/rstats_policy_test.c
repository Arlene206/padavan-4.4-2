#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rstats_file_policy.h"
#include "rstats_signal_policy.h"

static char last_temp[PATH_MAX];
static int write_mode, write_calls;

static int test_mkstemp(char *p) {
  int fd = mkstemp(p);
  if (fd >= 0) snprintf(last_temp, sizeof(last_temp), "%s", p);
  return fd;
}
static ssize_t test_write(int fd, const void *buf, size_t len) {
  ++write_calls;
  if (write_mode == 1 && write_calls == 1)
    return write(fd, buf, len > 3 ? 3 : len);
  if (write_mode == 1 && write_calls == 2) { errno = EINTR; return -1; }
  if (write_mode == 2 && write_calls == 1)
    return write(fd, buf, len > 2 ? 2 : len);
  if (write_mode == 2) { errno = EIO; return -1; }
  return write(fd, buf, len);
}
static int fail_fsync(int fd) { (void)fd; errno = EIO; return -1; }
static int fail_rename(const char *a, const char *b) {
  (void)a; (void)b; errno = EIO; return -1;
}
static int fail_close(int fd) {
  int rc = close(fd); if (rc != 0) return rc; errno = EIO; return -1;
}
static void ops_init(struct rstats_file_ops *o) {
  o->mkstemp_fn = test_mkstemp; o->fchmod_fn = fchmod; o->write_fn = test_write;
  o->fsync_fn = fsync; o->close_fn = close; o->rename_fn = rename; o->unlink_fn = unlink;
}
static void put(const char *p, const char *s) {
  int fd = open(p, O_WRONLY|O_CREAT|O_TRUNC, 0600);
  assert(fd >= 0); assert(write(fd, s, strlen(s)) == (ssize_t)strlen(s)); assert(close(fd) == 0);
}
static void expect(const char *p, const char *s) {
  char b[128]; int fd = open(p, O_RDONLY); ssize_t n;
  assert(fd >= 0); n = read(fd, b, sizeof(b)-1); assert(n >= 0); assert(close(fd) == 0);
  b[n] = '\0'; assert(strcmp(b, s) == 0);
}
static void temp_gone(void) { assert(last_temp[0] && access(last_temp, F_OK) < 0 && errno == ENOENT); }

int main(void) {
  char dir[] = "/tmp/rstats-policy.XXXXXX";
  char target[128], victim[128], stage[128];
  struct rstats_file_ops o;
  struct stat st;
  volatile sig_atomic_t pending = 0;
  sig_atomic_t events;
  FILE *fp;
  assert(mkdtemp(dir) != NULL);
  snprintf(target, sizeof(target), "%s/history", dir);
  snprintf(victim, sizeof(victim), "%s/victim", dir);
  snprintf(stage, sizeof(stage), "%s/view.tmp", dir);

  /* Short writes and EINTR are retried, and the old mode is retained. */
  put(target, "old-good"); assert(chmod(target, 0640) == 0); ops_init(&o);
  write_mode = 1; write_calls = 0; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "complete-record", 15, &o) == 0);
  expect(target, "complete-record"); assert(stat(target, &st) == 0 && (st.st_mode & 0777) == 0640);
  assert(write_calls >= 3); temp_gone();

  /* A partial write or later fsync/close/rename failure keeps the good file. */
  ops_init(&o); write_mode = 2; write_calls = 0; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "partial-data", 12, &o) < 0);
  expect(target, "complete-record"); temp_gone();
  ops_init(&o); o.write_fn = write; o.fsync_fn = fail_fsync; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "new", 3, &o) < 0);
  expect(target, "complete-record"); temp_gone();
  ops_init(&o); o.write_fn = write; o.close_fn = fail_close; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "new", 3, &o) < 0);
  expect(target, "complete-record"); temp_gone();
  ops_init(&o); o.write_fn = write; o.rename_fn = fail_rename; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "new", 3, &o) < 0);
  expect(target, "complete-record"); temp_gone();

  /* Never replace or follow a symlink target. */
  put(victim, "protected"); assert(unlink(target) == 0); assert(symlink(victim, target) == 0);
  errno = 0; assert(rstats_file_write_atomic(target, "bad", 3) < 0 && errno == EINVAL);
  errno = 0; assert(rstats_file_open_stream(target, stage, sizeof(stage)) == NULL && errno == EINVAL);
  expect(victim, "protected"); assert(unlink(target) == 0);

  /* JSON stream publication rejects buffered I/O errors and preserves old data. */
  put(target, "json-old"); fp = rstats_file_open_stream(target, stage, sizeof(stage));
  assert(fp != NULL && strstr(stage, ".tmp.") != NULL);
  assert(fputs("json-new", fp) >= 0); assert(rstats_file_publish_stream(fp, stage, target) == 0);
  expect(target, "json-new");
  assert(symlink("/dev/full", stage) == 0); fp = fopen(stage, "w"); assert(fp != NULL);
  (void)fputs("cannot-fit", fp); assert(rstats_file_publish_stream(fp, stage, target) < 0);
  expect(target, "json-new"); assert(access(stage, F_OK) < 0 && errno == ENOENT);

  /* Signal handlers only accumulate coalesced bits for normal-context work. */
  rstats_record_signal(&pending, SIGHUP); rstats_record_signal(&pending, SIGHUP);
  rstats_record_signal(&pending, SIGUSR1); rstats_record_signal(&pending, SIGUSR2);
  rstats_record_signal(&pending, SIGTERM); rstats_record_signal(&pending, SIGTERM);
  rstats_record_signal(&pending, SIGINT);
  assert(pending == (RSTATS_PENDING_TIME_REFRESH | RSTATS_PENDING_SPEED_EXPORT |
                      RSTATS_PENDING_HISTORY_EXPORT | RSTATS_PENDING_TERMINATE));
  events = rstats_take_pending_events(&pending);
  assert(events == (RSTATS_PENDING_TIME_REFRESH | RSTATS_PENDING_SPEED_EXPORT |
                      RSTATS_PENDING_HISTORY_EXPORT | RSTATS_PENDING_TERMINATE));
  assert(pending == 0 && rstats_take_pending_events(&pending) == 0);

  unlink(target); unlink(victim); unlink(stage); assert(rmdir(dir) == 0);
  puts("rstats persistence and signal policy: PASS");
  return 0;
}
