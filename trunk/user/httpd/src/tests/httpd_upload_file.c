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
	char stage[320], missing[320], alias[320], fifo[320], hardlink[320];
	FILE *stream;
	struct stat stage_info;
	int equal, n, read_fd;

	assert(httpd_upload_target_is_regular_or_missing(target));
	read_fd = httpd_upload_open_regular_readonly(target);
	assert(read_fd >= 0);
	assert(close(read_fd) == 0);
	n = snprintf(missing, sizeof(missing), "%s/not-created", dir);
	assert(n > 0 && (size_t)n < sizeof(missing));
	assert(httpd_upload_target_is_regular_or_missing(missing));
	n = snprintf(alias, sizeof(alias), "%s/target-link", dir);
	assert(n > 0 && (size_t)n < sizeof(alias));
	assert(symlink(victim, alias) == 0);
	assert(!httpd_upload_target_is_regular_or_missing(alias));
	errno = 0;
	assert(httpd_upload_open_regular_readonly(alias) < 0);
	assert(errno == ELOOP);
	assert(unlink(alias) == 0);
	n = snprintf(fifo, sizeof(fifo), "%s/target-fifo", dir);
	assert(n > 0 && (size_t)n < sizeof(fifo));
	assert(mkfifo(fifo, 0600) == 0);
	assert(!httpd_upload_target_is_regular_or_missing(fifo));
	errno = 0;
	assert(httpd_upload_open_regular_readonly(fifo) < 0);
	assert(errno == EINVAL);
	assert(unlink(fifo) == 0);
	n = snprintf(hardlink, sizeof(hardlink), "%s/target-hardlink", dir);
	assert(n > 0 && (size_t)n < sizeof(hardlink));
	assert(link(target, hardlink) == 0);
	errno = 0;
	assert(httpd_upload_open_regular_readonly(hardlink) < 0);
	assert(errno == EINVAL);
	assert(unlink(hardlink) == 0);
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


static void
test_multipart_boundary_requires_a_complete_delimiter(void)
{
    char false_prefix[] = "archive\r\n--router-boundaryXpayload";
    char bare_prefix[] = "archive\r\n--router-boundary";
    char regular[] = "archive\r\n--router-boundary\r\nnext";
    char closing[] = "archive\n--router-boundary--\r\n";
    char padded[] = "archive\r\n--router-boundary \t\r\nnext";
    char invalid_closing[] = "archive\r\n--router-boundary--Xpayload";
    const char *marker = "\r\n--router-boundary";
    char *found;

    assert(httpd_upload_find_multipart_boundary(
               false_prefix, strlen(false_prefix), marker) == NULL);
    assert(httpd_upload_find_multipart_boundary(
               bare_prefix, strlen(bare_prefix), marker) == NULL);
    assert(httpd_upload_find_multipart_boundary(
               invalid_closing, strlen(invalid_closing), marker) == NULL);

    found = httpd_upload_find_multipart_boundary(
        regular, strlen(regular), marker);
    assert(found == regular + strlen("archive"));

    found = httpd_upload_find_multipart_boundary(
        closing, strlen(closing), "\n--router-boundary");
    assert(found == closing + strlen("archive"));

    found = httpd_upload_find_multipart_boundary(
        padded, strlen(padded), marker);
    assert(found == padded + strlen("archive"));
}

static void
test_multipart_boundary_carry_covers_maximum_marker_and_padding(void)
{
    char marker[128];
    char padded[HTTPD_UPLOAD_BOUNDARY_LOOKBEHIND + 1];
    char body[400], carry[HTTPD_UPLOAD_BOUNDARY_LOOKBEHIND];
    char combined[HTTPD_UPLOAD_BOUNDARY_LOOKBEHIND + 200];
    const size_t marker_length = 127U;
    const size_t body_marker_offset = 100U;
    const size_t split = body_marker_offset + 80U;
    size_t previous_length, remaining;
    char *found;

    memcpy(marker, "\r\n--", 4U);
    memset(marker + 4U, 'B', marker_length - 4U);
    marker[marker_length] = '\0';

    memcpy(padded, marker, marker_length);
    memset(padded + marker_length, ' ', HTTPD_UPLOAD_BOUNDARY_MAX_PADDING);
    memcpy(padded + marker_length + HTTPD_UPLOAD_BOUNDARY_MAX_PADDING,
           "\r\n", 2U);
    found = httpd_upload_find_multipart_boundary(
        padded, sizeof(padded) - 1U, marker);
    assert(found == padded);

    memset(body, 'x', sizeof(body));
    memcpy(body + body_marker_offset, marker, marker_length);
    memcpy(body + body_marker_offset + marker_length, "\r\n", 2U);

    previous_length = split;
    memset(carry, 0, sizeof(carry));
    memcpy(carry + sizeof(carry) - previous_length, body, previous_length);
    memcpy(combined, carry, sizeof(carry));
    remaining = body_marker_offset + marker_length + 2U - split;
    memcpy(combined + sizeof(carry), body + split, remaining);
    found = httpd_upload_find_multipart_boundary(
        combined, sizeof(carry) + remaining, marker);
    assert(found == combined + sizeof(carry) -
                    previous_length + body_marker_offset);
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
	test_multipart_boundary_requires_a_complete_delimiter();
	test_multipart_boundary_carry_covers_maximum_marker_and_padding();
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
