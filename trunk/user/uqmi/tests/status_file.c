#define _GNU_SOURCE
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../uqmi-status-file.h"

static void
make_path(char *path, size_t size, const char *directory, const char *name)
{
	int length = snprintf(path, size, "%s/%s", directory, name);
	assert(length > 0 && (size_t)length < size);
}

static void
read_exact(const char *path, const char *expected)
{
	char buffer[64];
	FILE *fp = fopen(path, "r");
	size_t length;

	assert(fp != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, fp);
	assert(!ferror(fp));
	assert(fclose(fp) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, expected) == 0);
}

static void
test_atomic_replace_and_restricted_mode(const char *directory)
{
	char path[512];
	struct stat before, after;

	make_path(path, sizeof(path), directory, "client-id");
	assert(uqmi_status_file_write(path, 7) == 0);
	assert(stat(path, &before) == 0);
	assert(S_ISREG(before.st_mode));
	assert((before.st_mode & 07777) == 0600);
	assert(before.st_uid == geteuid());
	read_exact(path, "7");

	assert(uqmi_status_file_write(path, ULONG_MAX) == 0);
	assert(stat(path, &after) == 0);
	assert(after.st_ino != before.st_ino);
	assert((after.st_mode & 07777) == 0600);
	{
		char value[3 * sizeof(unsigned long) + 1];
		snprintf(value, sizeof(value), "%lu", ULONG_MAX);
		read_exact(path, value);
	}
	assert(unlink(path) == 0);
}

static void
test_destination_symlink_is_replaced_not_followed(const char *directory)
{
	char path[512], target[512];
	struct stat st;
	FILE *fp;

	make_path(path, sizeof(path), directory, "client-id-link");
	make_path(target, sizeof(target), directory, "target");
	fp = fopen(target, "w");
	assert(fp != NULL);
	assert(fputs("original", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(symlink(target, path) == 0);

	assert(uqmi_status_file_write(path, 23) == 0);
	assert(lstat(path, &st) == 0 && S_ISREG(st.st_mode));
	assert((st.st_mode & 07777) == 0600);
	read_exact(path, "23");
	read_exact(target, "original");
	assert(unlink(path) == 0);
	assert(unlink(target) == 0);
}

static void
test_failed_rename_preserves_destination_and_cleans_temp(const char *directory)
{
	char path[512], prefix[512];
	DIR *dir;
	struct dirent *entry;
	int staging_files = 0;

	make_path(path, sizeof(path), directory, "destination-directory");
	assert(mkdir(path, 0700) == 0);
	assert(uqmi_status_file_write(path, 99) == -1);
	assert(errno == EISDIR || errno == ENOTEMPTY || errno == EEXIST);
	dir = opendir(directory);
	assert(dir != NULL);
	snprintf(prefix, sizeof(prefix), "destination-directory.tmp.");
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0)
			staging_files++;
	}
	assert(closedir(dir) == 0);
	assert(staging_files == 0);
	assert(rmdir(path) == 0);
}

static void
test_invalid_paths_are_rejected(void)
{
	assert(uqmi_status_file_write(NULL, 1) == -1 && errno == EINVAL);
	assert(uqmi_status_file_write("relative/path", 1) == -1 && errno == EINVAL);
}

int
main(void)
{
	char template[] = "/tmp/uqmi-status-file.XXXXXX";
	char *directory = mkdtemp(template);

	assert(directory != NULL);
	assert(chmod(directory, 0700) == 0);
	test_atomic_replace_and_restricted_mode(directory);
	test_destination_symlink_is_replaced_not_followed(directory);
	test_failed_rename_preserves_destination_and_cleans_temp(directory);
	test_invalid_paths_are_rejected();
	assert(rmdir(directory) == 0);
	puts("uqmi atomic QMI status-file publication: PASS");
	return 0;
}
