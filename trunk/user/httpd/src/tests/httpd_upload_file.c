#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "httpd_upload_file.h"

static void
write_text(const char *path, const char *text)
{
	FILE *stream = fopen(path, "w");
	assert(stream != NULL);
	assert(fputs(text, stream) >= 0);
	assert(fclose(stream) == 0);
}

static void
assert_text(const char *path, const char *expected)
{
	char buffer[128];
	FILE *stream = fopen(path, "r");
	assert(stream != NULL);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, expected) == 0);
	assert(fclose(stream) == 0);
}

static FILE *
new_staging(const char *target, char *path, size_t path_size)
{
	FILE *stream = httpd_upload_create_staging(target, path, path_size);
	assert(stream != NULL);
	assert(fputs("replacement archive\n", stream) >= 0);
	return stream;
}

static void
assert_stage_removed(const char *path)
{
	struct stat info;
	assert(lstat(path, &info) != 0 && errno == ENOENT);
}

static size_t
fail_write(FILE *stream, const void *buffer, size_t length)
{
	(void)stream;
	(void)buffer;
	(void)length;
	errno = ENOSPC;
	return 0;
}

static int
fail_flush(FILE *stream)
{
	(void)stream;
	errno = EIO;
	return -1;
}

static int
fail_sync(int fd)
{
	(void)fd;
	errno = EIO;
	return -1;
}

static int
fail_close(FILE *stream)
{
	(void)fclose(stream);
	errno = EIO;
	return -1;
}

static int
fail_rename(const char *source, const char *destination)
{
	(void)source;
	(void)destination;
	errno = EXDEV;
	return -1;
}

static void
test_dos2unix_preserves_long_records(void)
{
	char input[8198], expected[8196], actual[8198];
	size_t expected_size = 8195U, actual_size;
	FILE *stream;

	memset(input, 'x', 8192U);
	memcpy(input + 8192U, "\r\nB\r\n", 6U);
	memset(expected, 'x', 8192U);
	expected[8192] = '\n';
	expected[8193] = 'B';
	expected[8194] = '\n';
	expected[8195] = '\0';

	stream = tmpfile();
	assert(stream != NULL);
	assert(httpd_upload_write_dos2unix(stream, input) == 0);
	assert(fflush(stream) == 0);
	rewind(stream);
	actual_size = fread(actual, 1, sizeof(actual) - 1U, stream);
	assert(ferror(stream) == 0);
	actual[actual_size] = '\0';
	assert(actual_size == expected_size);
	assert(memcmp(actual, expected, expected_size) == 0);
	assert(fclose(stream) == 0);

	stream = tmpfile();
	assert(stream != NULL);
	assert(httpd_upload_write_dos2unix(stream, "alpha\r\nbeta") == 0);
	assert(fflush(stream) == 0);
	rewind(stream);
	actual_size = fread(actual, 1, sizeof(actual) - 1U, stream);
	assert(actual_size == sizeof("alpha\nbeta\n") - 1U);
	assert(memcmp(actual, "alpha\nbeta\n", actual_size) == 0);
	assert(fclose(stream) == 0);
}

static void
test_upload_target_policy_and_secure_comparison(const char *dir,
						const char *target,
						const char *victim)
{
	char stage[320], missing[320], alias[320], fifo[320];
	FILE *stream;
	struct stat stage_info;
	int equal, n;

	assert(httpd_upload_target_is_regular_or_missing(target));
	n = snprintf(missing, sizeof(missing), "%s/not-created", dir);
	assert(n > 0 && (size_t)n < sizeof(missing));
	assert(httpd_upload_target_is_regular_or_missing(missing));
	n = snprintf(alias, sizeof(alias), "%s/target-link", dir);
	assert(n > 0 && (size_t)n < sizeof(alias));
	assert(symlink(victim, alias) == 0);
	assert(!httpd_upload_target_is_regular_or_missing(alias));
	assert(unlink(alias) == 0);
	n = snprintf(fifo, sizeof(fifo), "%s/target-fifo", dir);
	assert(n > 0 && (size_t)n < sizeof(fifo));
	assert(mkfifo(fifo, 0600) == 0);
	assert(!httpd_upload_target_is_regular_or_missing(fifo));
	assert(unlink(fifo) == 0);
	assert(!httpd_upload_target_is_regular_or_missing(dir));

	stream = httpd_upload_create_staging(target, stage, sizeof(stage));
	assert(stream != NULL);
	assert(strncmp(stage, target, strlen(target)) == 0);
	assert(strstr(stage + strlen(target), ".upload.") == stage + strlen(target));
	assert(lstat(stage, &stage_info) == 0);
	assert((stage_info.st_mode & 0777) == 0600);
	assert(httpd_upload_write_dos2unix(stream, "uploaded safely\r\n") == 0);
	assert(fchmod(fileno(stream), 0600) == 0);
	assert(fflush(stream) == 0);
	assert(httpd_upload_files_equal(target, stage, &equal) == 0);
	assert(equal == 1);
	httpd_upload_discard_staged(&stream, stage, NULL);
	assert(stream == NULL);
	assert_stage_removed(stage);

	stream = httpd_upload_create_staging(target, stage, sizeof(stage));
	assert(stream != NULL);
	assert(httpd_upload_write_dos2unix(stream, "different content\n") == 0);
	assert(fflush(stream) == 0);
	assert(httpd_upload_files_equal(target, stage, &equal) == 0);
	assert(equal == 0);
	httpd_upload_discard_staged(&stream, stage, NULL);
	assert_stage_removed(stage);

	stream = httpd_upload_create_staging(target, stage, sizeof(stage));
	assert(stream != NULL);
	assert(httpd_upload_files_equal(missing, stage, &equal) == 0);
	assert(equal == 0);
	httpd_upload_discard_staged(&stream, stage, NULL);
	assert_stage_removed(stage);

	assert(symlink(victim, alias) == 0);
	errno = 0;
	assert(httpd_upload_files_equal(alias, target, &equal) != 0);
	assert(errno == ELOOP || errno == EINVAL);
	assert(unlink(alias) == 0);
}

int
main(void)
{
	char dir[] = "/tmp/httpd-upload-staging.XXXXXX";
	char target[256], victim[256], stage[320], missing[320];
	FILE *stream;
	struct stat info;
	httpd_upload_file_ops_t ops;
	int n;

	test_dos2unix_preserves_long_records();
	assert(mkdtemp(dir) != NULL);
	n = snprintf(target, sizeof(target), "%s/upload.bin", dir);
	assert(n > 0 && (size_t)n < sizeof(target));
	n = snprintf(victim, sizeof(victim), "%s/victim.txt", dir);
	assert(n > 0 && (size_t)n < sizeof(victim));

	write_text(victim, "do not overwrite\n");
	assert(symlink(victim, target) == 0);

	errno = 0;
	stream = httpd_upload_open_exclusive(target);
	assert(stream == NULL);
	assert(errno == EEXIST || errno == ELOOP);
	assert_text(victim, "do not overwrite\n");
	assert(unlink(target) == 0);

	stream = httpd_upload_open_exclusive(target);
	assert(stream != NULL);
	assert(fputs("uploaded safely\n", stream) >= 0);
	assert(fclose(stream) == 0);
	assert(lstat(target, &info) == 0);
	assert(S_ISREG(info.st_mode));
	assert((info.st_mode & 0777) == 0600);
	assert_text(target, "uploaded safely\n");
	test_upload_target_policy_and_secure_comparison(dir, target, victim);
	assert_text(victim, "do not overwrite\n");

	/* An unavailable staging directory must never disturb the retained target. */
	n = snprintf(missing, sizeof(missing), "%s/missing/upload.bin", dir);
	assert(n > 0 && (size_t)n < sizeof(missing));
	errno = 0;
	stream = httpd_upload_create_staging(missing, stage, sizeof(stage));
	assert(stream == NULL);
	assert_text(target, "uploaded safely\n");

	/* A short write closes and discards only the new stage. */
	memset(&ops, 0, sizeof(ops));
	ops.write = fail_write;
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_write(stream, "partial", 7, &ops) != 7);
	httpd_upload_discard_staged(&stream, stage, &ops);
	assert(stream == NULL);
	assert_text(target, "uploaded safely\n");
	assert_stage_removed(stage);

	/* A flush error closes and discards only the new stage. */
	memset(&ops, 0, sizeof(ops));
	ops.flush = fail_flush;
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_publish_staged(&stream, stage, target, &ops) != 0);
	assert(stream == NULL);
	assert_text(target, "uploaded safely\n");
	assert_stage_removed(stage);

	/* An fsync error has the same fail-closed publication behavior. */
	memset(&ops, 0, sizeof(ops));
	ops.sync = fail_sync;
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_publish_staged(&stream, stage, target, &ops) != 0);
	assert(stream == NULL);
	assert_text(target, "uploaded safely\n");
	assert_stage_removed(stage);

	/* fclose reports failure after releasing the stream; the old target remains. */
	memset(&ops, 0, sizeof(ops));
	ops.close = fail_close;
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_publish_staged(&stream, stage, target, &ops) != 0);
	assert(stream == NULL);
	assert_text(target, "uploaded safely\n");
	assert_stage_removed(stage);

	/* Atomic rename failure must preserve the prior retry archive. */
	memset(&ops, 0, sizeof(ops));
	ops.rename = fail_rename;
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_publish_staged(&stream, stage, target, &ops) != 0);
	assert(stream == NULL);
	assert_text(target, "uploaded safely\n");
	assert_stage_removed(stage);

	/* Only successful close+rename replaces the retained file. */
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_publish_staged(&stream, stage, target, NULL) == 0);
	assert(stream == NULL);
	assert_stage_removed(stage);
	assert_text(target, "replacement archive\n");
	assert_text(victim, "do not overwrite\n");

	/* Rename replaces a symlink entry rather than following it to its victim. */
	assert(unlink(target) == 0);
	assert(symlink(victim, target) == 0);
	stream = new_staging(target, stage, sizeof(stage));
	assert(httpd_upload_publish_staged(&stream, stage, target, NULL) == 0);
	assert(lstat(target, &info) == 0 && S_ISREG(info.st_mode));
	assert((info.st_mode & 0777) == 0600);
	assert_text(target, "replacement archive\n");
	assert_text(victim, "do not overwrite\n");

	assert(unlink(target) == 0);
	assert(unlink(victim) == 0);
	assert(rmdir(dir) == 0);
	puts("HTTPD upload staging/atomic publication regression: OK");
	return 0;
}
