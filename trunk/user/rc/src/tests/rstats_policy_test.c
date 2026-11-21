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
#include "rstats_time_policy.h"

static char last_temp[PATH_MAX];
static int write_mode, write_calls;
static int fsync_mode, fsync_calls;

struct scripted_read_fixture {
  const char *data;
  size_t data_length;
  size_t offset;
  size_t max_chunk;
  size_t fail_after;
  unsigned int calls;
  unsigned int interruptions_remaining;
  int interrupt_every_other_call;
  int interrupt_once;
  int fail_errno;
};

static ssize_t scripted_read(int fd, void *buffer, size_t length, void *opaque) {
  struct scripted_read_fixture *fixture =
      (struct scripted_read_fixture *)opaque;
  size_t count;

  assert(fd == 42);
  ++fixture->calls;
  if (fixture->interruptions_remaining > 0) {
    --fixture->interruptions_remaining;
    errno = EINTR;
    return -1;
  }
  if (fixture->interrupt_every_other_call && (fixture->calls & 1U)) {
    errno = EINTR;
    return -1;
  }
  if (fixture->interrupt_once) {
    fixture->interrupt_once = 0;
    errno = EINTR;
    return -1;
  }
  if (fixture->fail_errno && fixture->offset >= fixture->fail_after) {
    errno = fixture->fail_errno;
    return -1;
  }
  if (fixture->offset >= fixture->data_length)
    return 0;

  count = fixture->data_length - fixture->offset;
  if (count > length)
    count = length;
  if (fixture->max_chunk && count > fixture->max_chunk)
    count = fixture->max_chunk;
  if (fixture->fail_errno &&
      count > fixture->fail_after - fixture->offset)
    count = fixture->fail_after - fixture->offset;
  if (count == 0) {
    errno = EIO;
    return -1;
  }
  memcpy(buffer, fixture->data + fixture->offset, count);
  fixture->offset += count;
  return (ssize_t)count;
}

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
  if (write_mode == 3 && write_calls <= (int)RSTATS_FILE_MAX_INTERRUPTS) {
    errno = EINTR; return -1;
  }
  if (write_mode == 4) { errno = EINTR; return -1; }
  if (write_mode == 5) {
    if (write_calls & 1) { errno = EINTR; return -1; }
    return write(fd, buf, len > 1 ? 1 : len);
  }
  return write(fd, buf, len);
}
static int fail_fsync(int fd) { (void)fd; errno = EIO; return -1; }
static int test_fsync(int fd) {
  ++fsync_calls;
  if (fsync_mode == 2 ||
      (fsync_mode == 1 && fsync_calls <= (int)RSTATS_FILE_MAX_INTERRUPTS)) {
    errno = EINTR;
    return -1;
  }
  return fsync(fd);
}
static int fail_rename(const char *a, const char *b) {
  (void)a; (void)b; errno = EIO; return -1;
}
static int fail_close(int fd) {
  int rc = close(fd); if (rc != 0) return rc; errno = EIO; return -1;
}
static void ops_init(struct rstats_file_ops *o) {
  fsync_mode = 0; fsync_calls = 0;
  o->mkstemp_fn = test_mkstemp; o->fchmod_fn = fchmod; o->write_fn = test_write;
  o->fsync_fn = test_fsync; o->close_fn = close; o->rename_fn = rename; o->unlink_fn = unlink;
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

static void test_new_file_modes_follow_umask(const char *directory)
{
  char raw_path[128], view_path[128], temporary[128];
  struct rstats_file_ops ops;
  struct stat st;
  FILE *stream;
  mode_t previous_mask, observed_mask;

  snprintf(raw_path, sizeof(raw_path), "%s/new-history", directory);
  snprintf(view_path, sizeof(view_path), "%s/new-view.js", directory);

  previous_mask = umask(0027);
  assert(rstats_file_creation_mode() == 0640);

  ops_init(&ops);
  last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(raw_path, "raw", 3, &ops) == 0);
  assert(stat(raw_path, &st) == 0 && (st.st_mode & 0777) == 0640);

  stream = rstats_file_open_stream(view_path, temporary, sizeof(temporary));
  assert(stream != NULL);
  assert(fputs("view", stream) >= 0);
  assert(rstats_file_publish_stream(stream, temporary, view_path) == 0);
  assert(stat(view_path, &st) == 0 && (st.st_mode & 0777) == 0640);

  /* The helper must not leave the process umask changed. */
  observed_mask = umask(0027);
  assert(observed_mask == 0027);
  (void)umask(previous_mask);

  assert(unlink(raw_path) == 0);
  assert(unlink(view_path) == 0);
}

struct scripted_open_fixture {
  unsigned int calls;
  unsigned int interruptions;
  int failure;
};

static int scripted_open_readonly(const char *path, int flags, void *opaque) {
  struct scripted_open_fixture *fixture =
      (struct scripted_open_fixture *)opaque;
  assert(strcmp(path, "/etc/storage/rstats-history") == 0);
  assert(flags == O_RDONLY);
  ++fixture->calls;
  if (fixture->calls <= fixture->interruptions) {
    errno = EINTR;
    return -1;
  }
  if (fixture->failure) {
    errno = fixture->failure;
    return -1;
  }
  return 42;
}

static void test_bounded_open_eintr(void) {
  struct scripted_open_fixture fixture;

  memset(&fixture, 0, sizeof(fixture));
  fixture.interruptions = RSTATS_FILE_MAX_INTERRUPTS;
  assert(rstats_file_open_readonly_with("/etc/storage/rstats-history",
      scripted_open_readonly, &fixture) == 42);
  assert(fixture.calls == RSTATS_FILE_MAX_INTERRUPTS + 1U);

  memset(&fixture, 0, sizeof(fixture));
  fixture.interruptions = RSTATS_FILE_MAX_INTERRUPTS + 1U;
  errno = 0;
  assert(rstats_file_open_readonly_with("/etc/storage/rstats-history",
      scripted_open_readonly, &fixture) == -1);
  assert(errno == EINTR && fixture.calls == RSTATS_FILE_MAX_INTERRUPTS + 1U);

  memset(&fixture, 0, sizeof(fixture));
  fixture.failure = EACCES;
  errno = 0;
  assert(rstats_file_open_readonly_with("/etc/storage/rstats-history",
      scripted_open_readonly, &fixture) == -1);
  assert(errno == EACCES && fixture.calls == 1);
}

static void test_exact_read_retries_eintr_and_short_reads(void)
{
  static const char payload[] = "complete-history-record";
  char output[sizeof(payload)];
  struct scripted_read_fixture fixture;
  size_t payload_length = sizeof(payload) - 1;
  int result;

  memset(&fixture, 0, sizeof(fixture));
  fixture.data = payload;
  fixture.data_length = payload_length;
  fixture.max_chunk = 3;
  fixture.interrupt_once = 1;
  memset(output, 0, sizeof(output));
  result = rstats_file_read_exact_with(42, output, payload_length,
                                       scripted_read, &fixture);
  assert(result == (int)payload_length);
  assert(memcmp(output, payload, payload_length) == 0);
  assert(fixture.offset == payload_length);
  assert(fixture.calls == 1 + (payload_length + 2) / 3);

  /* Eight interruptions are recoverable when the next read makes progress. */
  memset(&fixture, 0, sizeof(fixture));
  fixture.data = payload;
  fixture.data_length = payload_length;
  fixture.interruptions_remaining = RSTATS_FILE_MAX_INTERRUPTS;
  result = rstats_file_read_exact_with(42, output, payload_length,
                                       scripted_read, &fixture);
  assert(result == (int)payload_length);
  assert(fixture.calls == RSTATS_FILE_MAX_INTERRUPTS + 1U);

  /* Positive progress resets the consecutive-interruption budget. */
  memset(&fixture, 0, sizeof(fixture));
  fixture.data = payload;
  fixture.data_length = payload_length;
  fixture.max_chunk = 1;
  fixture.interrupt_every_other_call = 1;
  result = rstats_file_read_exact_with(42, output, payload_length,
                                       scripted_read, &fixture);
  assert(result == (int)payload_length);
  assert(fixture.calls == payload_length * 2);

  /* A ninth consecutive interruption fails closed and preserves errno. */
  memset(&fixture, 0, sizeof(fixture));
  fixture.data = payload;
  fixture.data_length = payload_length;
  fixture.interruptions_remaining = RSTATS_FILE_MAX_INTERRUPTS + 1U;
  errno = 0;
  assert(rstats_file_read_exact_with(42, output, 1,
                                     scripted_read, &fixture) == -1);
  assert(errno == EINTR);
  assert(fixture.calls == RSTATS_FILE_MAX_INTERRUPTS + 1U);

  /* EOF after a partial record remains a short count. */
  memset(&fixture, 0, sizeof(fixture));
  fixture.data = payload;
  fixture.data_length = 5;
  fixture.max_chunk = 2;
  result = rstats_file_read_exact_with(42, output, payload_length,
                                       scripted_read, &fixture);
  assert(result == 5);
  assert(memcmp(output, payload, 5) == 0);
  assert(fixture.calls == 4);

  /* A non-EINTR error after partial progress stays a hard failure. */
  memset(&fixture, 0, sizeof(fixture));
  fixture.data = payload;
  fixture.data_length = payload_length;
  fixture.max_chunk = 2;
  fixture.fail_after = 4;
  fixture.fail_errno = EIO;
  errno = 0;
  assert(rstats_file_read_exact_with(42, output, payload_length,
                                     scripted_read, &fixture) == -1);
  assert(errno == EIO && fixture.offset == 4 && fixture.calls == 3);

  /* Empty input and invalid arguments have deterministic outcomes. */
  memset(&fixture, 0, sizeof(fixture));
  assert(rstats_file_read_exact_with(42, NULL, 0, scripted_read,
                                     &fixture) == 0);
  assert(fixture.calls == 0);
  errno = 0;
  assert(rstats_file_read_exact_with(-1, output, 1, scripted_read,
                                     &fixture) == -1);
  assert(errno == EINVAL);
  errno = 0;
  assert(rstats_file_read_exact_with(42, NULL, 1, scripted_read,
                                     &fixture) == -1);
  assert(errno == EINVAL);
  errno = 0;
  assert(rstats_file_read_exact_with(42, output, 1, NULL, &fixture) == -1);
  assert(errno == EINVAL);
  errno = 0;
  assert(rstats_file_read_exact_with(42, output, (size_t)INT_MAX + 1u,
                                     scripted_read, &fixture) == -1);
  assert(errno == EOVERFLOW);
}

static void test_null_safe_system_time_policy(void) {
  struct tm value;

  assert(!rstats_time_has_valid_year(NULL, 2020));
  memset(&value, 0, sizeof(value));
  value.tm_year = 119; /* 2019 */
  assert(!rstats_time_has_valid_year(&value, 2020));
  value.tm_year = 120; /* 2020 is the configured boot-time boundary */
  assert(!rstats_time_has_valid_year(&value, 2020));
  value.tm_year = 121; /* a year beyond the boundary */
  assert(rstats_time_has_valid_year(&value, 2020));
}

static void test_atomic_write_and_fsync_retries(const char *target) {
  struct rstats_file_ops ops;

  ops_init(&ops);
  write_mode = 3; write_calls = 0; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "retry-write", 11, &ops) == 0);
  assert(write_calls == RSTATS_FILE_MAX_INTERRUPTS + 1);
  expect(target, "retry-write");
  temp_gone();

  ops_init(&ops);
  write_mode = 4; write_calls = 0; last_temp[0] = '\0';
  errno = 0;
  assert(rstats_file_write_atomic_with_ops(target, "must-not-publish", 16, &ops) < 0);
  assert(errno == EINTR);
  expect(target, "retry-write");
  temp_gone();

  ops_init(&ops);
  write_mode = 5; write_calls = 0; last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "interleave", 10, &ops) == 0);
  assert(write_calls == 20);
  expect(target, "interleave");
  temp_gone();

  ops_init(&ops);
  write_mode = 0; write_calls = 0; fsync_mode = 1; fsync_calls = 0;
  last_temp[0] = '\0';
  assert(rstats_file_write_atomic_with_ops(target, "fsync-retry", 11, &ops) == 0);
  assert(fsync_calls == RSTATS_FILE_MAX_INTERRUPTS + 1);
  expect(target, "fsync-retry");
  temp_gone();

  ops_init(&ops);
  write_mode = 0; write_calls = 0; fsync_mode = 2; fsync_calls = 0;
  last_temp[0] = '\0';
  errno = 0;
  assert(rstats_file_write_atomic_with_ops(target, "must-not-publish", 16, &ops) < 0);
  assert(errno == EINTR && fsync_calls == RSTATS_FILE_MAX_INTERRUPTS + 1);
  expect(target, "fsync-retry");
  temp_gone();
}

int main(void) {
  test_null_safe_system_time_policy();
  test_bounded_open_eintr();
  test_exact_read_retries_eintr_and_short_reads();
  char dir[] = "/tmp/rstats-policy.XXXXXX";
  char target[128], victim[128], stage[128];
  struct rstats_file_ops o;
  struct stat st;
  volatile sig_atomic_t pending = 0;
  sig_atomic_t events;
  FILE *fp;
  assert(mkdtemp(dir) != NULL);
  test_new_file_modes_follow_umask(dir);
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

  test_atomic_write_and_fsync_retries(target);

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
