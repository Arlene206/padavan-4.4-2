#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "miniupnpd_config_file.h"

struct writer_context {
	const char *text;
	int result;
};

static int
write_config(FILE *stream, void *opaque)
{
	struct writer_context *context = (struct writer_context *)opaque;

	if (fputs(context->text, stream) == EOF)
		return errno ? errno : EIO;
	return context->result;
}

static void
write_file(const char *path, const char *text)
{
	FILE *stream = fopen(path, "w");
	assert(stream != NULL);
	assert(fputs(text, stream) >= 0);
	assert(fclose(stream) == 0);
}

static void
assert_file_equals(const char *path, const char *expected)
{
	char buffer[64];
	FILE *stream = fopen(path, "r");
	assert(stream != NULL);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, expected) == 0);
	assert(fclose(stream) == 0);
}

static void
assert_only_target_remains(const char *directory, const char *target_name)
{
	DIR *dir = opendir(directory);
	struct dirent *entry;
	int count = 0;
	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0)
			continue;
		assert(strcmp(entry->d_name, target_name) == 0);
		count++;
	}
	assert(closedir(dir) == 0);
	assert(count == 1);
}

static void
assert_two_targets_remain(const char *directory, const char *first,
			  const char *second)
{
	DIR *dir = opendir(directory);
	struct dirent *entry;
	int first_seen = 0, second_seen = 0;
	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0)
			continue;
		if (strcmp(entry->d_name, first) == 0)
			first_seen++;
		else if (strcmp(entry->d_name, second) == 0)
			second_seen++;
		else
			assert(0 && "unexpected temporary file remains");
	}
	assert(closedir(dir) == 0);
	assert(first_seen == 1 && second_seen == 1);
}

static void
test_checked_integer_read_preserves_defaults_and_rejects_errors(void)
{
	int result;

	result = 77;
	assert(miniupnpd_config_file_read_integer_status(0, 0, NULL, 0, 0, 2, &result) == 0);
	assert(result == 0);
	result = 77;
	assert(miniupnpd_config_file_read_integer_status(0, 0, NULL, 1, 0, 2, &result) == 0);
	assert(result == 1);
	result = 77;
	assert(miniupnpd_config_file_read_integer_status(1, 0, "2", 0, 0, 2, &result) == 0);
	assert(result == 2);
	result = 77;
	assert(miniupnpd_config_file_read_integer_status(1, 0, " \t1 \r\n", 0, 0, 1, &result) == 0);
	assert(result == 1);
	result = 77;
	assert(miniupnpd_config_file_read_integer_status(1, 0, "", 0, 0, 1, &result) == 0);
	assert(result == 0);
	result = 77;
	assert(miniupnpd_config_file_read_integer_status(1, 0, " \t ", 1, 0, 2, &result) == 0);
	assert(result == 1);

	result = 77;
	assert(miniupnpd_config_file_read_integer_status(-1, EACCES, "1", 0, 0, 1, &result) == EACCES);
	assert(result == 77);
	assert(miniupnpd_config_file_read_integer_status(-1, 0, "1", 0, 0, 1, &result) == EIO);
	assert(miniupnpd_config_file_read_integer_status(2, 0, "1", 0, 0, 1, &result) == EIO);
	assert(miniupnpd_config_file_read_integer_status(1, 0, NULL, 0, 0, 1, &result) == EIO);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "-1", 0, 0, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "+1", 0, 0, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "1x", 0, 0, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "1.0", 0, 0, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "2", 0, 0, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "999999999999999999999999999999999999", 0, 0, 1, &result) == ERANGE);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "1", 2, 0, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "1", 0, 2, 1, &result) == EINVAL);
	assert(miniupnpd_config_file_read_integer_status(1, 0, "1", 0, 0, 1, NULL) == EINVAL);
	assert(result == 77);
}


int
main(void)
{
	char directory[] = "/tmp/miniupnpd-config-test.XXXXXX";
	char path[256];
	char directory_target[256];
	char missing_parent[256];
	char linked_parent[256];
	char linked_path[256];
	struct writer_context context;
	struct stat st;
	int status;

	test_checked_integer_read_preserves_defaults_and_rejects_errors();
	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/miniupnpd.conf", directory) > 0);
	assert(snprintf(directory_target, sizeof(directory_target), "%s/directory-target", directory) > 0);
	write_file(path, "last-good\n");

	context.text = "partial-new-config\n";
	context.result = EIO;
	status = miniupnpd_config_publish(path, write_config, &context);
	assert(status == EIO);
	assert_file_equals(path, "last-good\n");
	assert_only_target_remains(directory, "miniupnpd.conf");

	context.text = "complete-new-config\n";
	context.result = 0;
	status = miniupnpd_config_publish(path, write_config, &context);
	assert(status == 0);
	assert_file_equals(path, "complete-new-config\n");

	assert(config_file_sync_parent(path) == 0);
	assert(snprintf(missing_parent, sizeof(missing_parent),
			"%s/missing/miniupnpd.conf", directory) > 0);
	errno = 0;
	assert(config_file_sync_parent(missing_parent) != 0);
	assert(errno == ENOENT);

	assert(snprintf(linked_parent, sizeof(linked_parent),
			"%s/linked-parent", directory) > 0);
	assert(symlink(directory, linked_parent) == 0);
	assert(snprintf(linked_path, sizeof(linked_path),
			"%s/miniupnpd.conf", linked_parent) > 0);
	errno = 0;
	assert(config_file_sync_parent(linked_path) != 0);
	assert(unlink(linked_parent) == 0);
	assert(stat(path, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert_only_target_remains(directory, "miniupnpd.conf");

	assert(miniupnpd_config_publish(path, NULL, &context) == EINVAL);
	assert(miniupnpd_config_publish(NULL, write_config, &context) == EINVAL);
	assert_file_equals(path, "complete-new-config\n");

	/* A failed final rename must preserve its destination and remove the stage. */
	assert(mkdir(directory_target, 0700) == 0);
	status = miniupnpd_config_publish(directory_target, write_config, &context);
	assert(status == EISDIR);
	assert(stat(directory_target, &st) == 0 && S_ISDIR(st.st_mode));
	assert_two_targets_remain(directory, "miniupnpd.conf", "directory-target");

	assert(unlink(path) == 0);
	assert(rmdir(directory_target) == 0);
	assert(rmdir(directory) == 0);
	puts("miniupnpd atomic config publication: PASS");
	return 0;
}
