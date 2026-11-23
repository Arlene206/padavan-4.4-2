/*
 * GNU89 regression for bounded read_whole_file() error handling.
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "disk_io_tools.h"

#define TEST_MAX_BYTES (1024U * 1024U)

static char test_dir[128];

static void make_path(char *buffer, size_t size, const char *name)
{
	int length = snprintf(buffer, size, "%s/%s", test_dir, name);
	assert(length >= 0 && (size_t)length < size);
}

static void write_file(const char *path, const char *data, size_t length)
{
	FILE *fp = fopen(path, "w");

	assert(fp != NULL);
	assert(fwrite(data, 1, length, fp) == length);
	assert(fclose(fp) == 0);
}

static void test_reads_complete_text_file(void)
{
	char path[256];
	const char *sample = "router metadata\nsecond line\n";
	char *contents;

	make_path(path, sizeof(path), "sample.txt");
	write_file(path, sample, strlen(sample));
	contents = read_whole_file(path);
	assert(contents != NULL);
	assert(strcmp(contents, sample) == 0);
	free(contents);
	assert(unlink(path) == 0);
}

static void test_reads_empty_file(void)
{
	char path[256];
	char *contents;

	make_path(path, sizeof(path), "empty.txt");
	write_file(path, "", 0);
	contents = read_whole_file(path);
	assert(contents != NULL);
	assert(contents[0] == '\0');
	free(contents);
	assert(unlink(path) == 0);
}

static void test_rejects_invalid_and_missing_paths(void)
{
	char path[256];

	errno = 0;
	assert(read_whole_file(NULL) == NULL);
	assert(errno == EINVAL);
	errno = 0;
	assert(read_whole_file("") == NULL);
	assert(errno == EINVAL);

	make_path(path, sizeof(path), "missing.txt");
	errno = 0;
	assert(read_whole_file(path) == NULL);
	assert(errno == ENOENT);
}

static void test_rejects_oversized_file_with_efbig(void)
{
	char path[256];
	int fd;

	make_path(path, sizeof(path), "oversized.bin");
	fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);
	assert(fd >= 0);
	assert(ftruncate(fd, (off_t)TEST_MAX_BYTES + 1) == 0);
	assert(close(fd) == 0);

	errno = 0;
	assert(read_whole_file(path) == NULL);
	assert(errno == EFBIG);
	assert(unlink(path) == 0);
}

static void test_reports_directory_read_error(void)
{
	char *contents;

	errno = 0;
	contents = read_whole_file(test_dir);
	assert(contents == NULL);
	assert(errno != 0);
}


static void
test_read_regular_file_nofollow_accepts_regular_file(void)
{
	char path[256];
	const char *sample = "safe folder-list cache\n";
	char *contents;

	make_path(path, sizeof(path), "regular-cache.txt");
	write_file(path, sample, strlen(sample));
	contents = read_regular_file_nofollow(path);
	assert(contents != NULL);
	assert(strcmp(contents, sample) == 0);
	free(contents);
	assert(unlink(path) == 0);
}

static void
test_read_regular_file_nofollow_rejects_symlink(void)
{
	char path[256], target[256];
	char *contents;

	make_path(path, sizeof(path), "linked-cache.txt");
	make_path(target, sizeof(target), "cache-target.txt");
	write_file(target, "must not be read\n", strlen("must not be read\n"));
	assert(symlink(target, path) == 0);

	errno = 0;
	contents = read_regular_file_nofollow(path);
	assert(contents == NULL);
	assert(errno == ELOOP);

	assert(unlink(path) == 0);
	assert(unlink(target) == 0);
}

static void
test_read_regular_file_nofollow_rejects_fifo_without_blocking(void)
{
	char path[256];
	char *contents;

	make_path(path, sizeof(path), "fifo-cache");
	assert(mkfifo(path, 0600) == 0);
	errno = 0;
	contents = read_regular_file_nofollow(path);
	assert(contents == NULL);
	assert(errno == EINVAL);
	assert(unlink(path) == 0);
}

static void
test_read_regular_file_nofollow_rejects_directory(void)
{
	char *contents;

	errno = 0;
	contents = read_regular_file_nofollow(test_dir);
	assert(contents == NULL);
	assert(errno == EINVAL);
}

static void test_sanity_name_replaces_high_bit_reserved_byte(void)
{
	char name[] = { 'A', (char)0x84, 'B', '/', '\0' };

	sanity_name(name);
	assert(name[0] == 'A');
	assert(name[1] == ' ');
	assert(name[2] == 'B');
	assert(name[3] == ' ');
}

int main(void)
{
	char directory_template[] = "/tmp/disk-io-tools-read-XXXXXX";
	char *directory = mkdtemp(directory_template);

	assert(directory != NULL);
	assert(strlen(directory) < sizeof(test_dir));
	strcpy(test_dir, directory);

	test_reads_complete_text_file();
	test_reads_empty_file();
	test_rejects_invalid_and_missing_paths();
	test_rejects_oversized_file_with_efbig();
	test_reports_directory_read_error();
	test_read_regular_file_nofollow_accepts_regular_file();
	test_read_regular_file_nofollow_rejects_symlink();
	test_read_regular_file_nofollow_rejects_fifo_without_blocking();
	test_read_regular_file_nofollow_rejects_directory();
	test_sanity_name_replaces_high_bit_reserved_byte();

	assert(rmdir(test_dir) == 0);
	return 0;
}
