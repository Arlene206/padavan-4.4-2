#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "firefly_config_file.h"

static void
write_file(const char *path, const char *text)
{
	FILE *stream = fopen(path, "w");

	assert(stream != NULL);
	assert(fputs(text, stream) >= 0);
	assert(fclose(stream) == 0);
}

static void
assert_contains(const char *path, const char *expected)
{
	char buffer[4096];
	size_t length;
	FILE *stream = fopen(path, "r");

	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	buffer[length] = '\0';
	assert(fclose(stream) == 0);
	assert(strstr(buffer, expected) != NULL);
}

static void
assert_not_contains(const char *path, const char *unexpected)
{
	char buffer[4096];
	size_t length;
	FILE *stream = fopen(path, "r");

	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	buffer[length] = '\0';
	assert(fclose(stream) == 0);
	assert(strstr(buffer, unexpected) == NULL);
}

static void
assert_no_temporary_files(const char *directory)
{
	DIR *dir = opendir(directory);
	struct dirent *entry;

	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL)
		assert(strstr(entry->d_name, ".tmp.") == NULL);
	assert(closedir(dir) == 0);
}

int
main(void)
{
	char directory[] = "/tmp/firefly-config-test.XXXXXX";
	char path[256];
	char default_path[256];
	char directory_target[256];
	struct stat st;
	int status;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/mt-daapd.conf", directory) > 0);
	assert(snprintf(default_path, sizeof(default_path), "%s/default.conf", directory) > 0);
	assert(snprintf(directory_target, sizeof(directory_target), "%s/directory-target", directory) > 0);

	write_file(path,
		   "[general]\n"
		   "web_root = /old/web\n"
		   "port = 1234\n"
		   "runas = old-user\n"
		   "admin_pw = keep-this-password\n"
		   "db_type = old\n"
		   "db_parms = /old/database\n"
		   "plugin_dir = /old/plugins\n"
		   "custom_setting = preserve-me\n");
	status = firefly_config_publish(path, path, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status == 0);
	assert_contains(path, "web_root = /usr/share/mt-daapd\n");
	assert_contains(path, "port = 3689\n");
	assert_contains(path, "runas = media-user\n");
	assert_contains(path, "admin_pw = keep-this-password\n");
	assert_contains(path, "db_type = sqlite3\n");
	assert_contains(path, "db_parms = /mnt/firefly\n");
	assert_contains(path, "plugin_dir = /usr/lib/mt-daapd\n");
	assert_contains(path, "custom_setting = preserve-me\n");
	assert_not_contains(path, "old-user");
	assert_no_temporary_files(directory);

	status = firefly_config_publish(default_path, default_path, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status == 0);
	assert_contains(default_path, "admin_pw = media-pass\n");
	assert_contains(default_path, "logfile = /mnt/firefly/mt-daapd.log\n");
	assert_contains(default_path, "mp3_tag_codepage = WINDOWS-1251\n");
	assert(stat(default_path, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert_no_temporary_files(directory);

	write_file(path, "last-good-configuration\n");
	status = firefly_config_publish(path, directory, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status != 0);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	assert(mkdir(directory_target, 0700) == 0);
	status = firefly_config_publish(directory_target, NULL, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status != 0);
	assert(stat(directory_target, &st) == 0 && S_ISDIR(st.st_mode));
	assert_no_temporary_files(directory);

	assert(firefly_config_publish(NULL, NULL, "/mnt/firefly",
				       "media-user", "media-pass") == EINVAL);
	assert(firefly_config_publish(path, NULL, NULL,
				       "media-user", "media-pass") == EINVAL);

	assert(unlink(path) == 0);
	assert(unlink(default_path) == 0);
	assert(rmdir(directory_target) == 0);
	assert(rmdir(directory) == 0);
	puts("Firefly atomic configuration publication: PASS");
	return 0;
}
