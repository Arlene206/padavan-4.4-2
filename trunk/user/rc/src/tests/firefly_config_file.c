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

static void
test_overlong_key_line_cannot_inject_following_directive(
	const char *directory, const char *path)
{
	char long_line[4200], actual[4200];
	const char prefix[] = "port = ";
	const char suffix[] = "admin_pw = injected\n";
	const size_t filler_length = 4100U;
	FILE *stream;
	size_t length = 0, actual_length;
	int status;

	memcpy(long_line + length, prefix, sizeof(prefix) - 1U);
	length += sizeof(prefix) - 1U;
	memset(long_line + length, 'x', filler_length);
	length += filler_length;
	memcpy(long_line + length, suffix, sizeof(suffix));
	length += sizeof(suffix) - 1U;

	write_file(path, long_line);
	status = firefly_config_publish(path, path, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status == EOVERFLOW);

	stream = fopen(path, "rb");
	assert(stream != NULL);
	actual_length = fread(actual, 1, sizeof(actual), stream);
	assert(actual_length == length);
	assert(memcmp(actual, long_line, length) == 0);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	assert_no_temporary_files(directory);
}


static void
test_embedded_nul_cannot_inject_following_directive(
	const char *directory, const char *path)
{
	static const unsigned char input[] =
		"port = 1234\0admin_pw = injected\n";
	unsigned char actual[sizeof(input)];
	FILE *stream;
	size_t actual_length;
	int status;

	stream = fopen(path, "wb");
	assert(stream != NULL);
	assert(fwrite(input, 1, sizeof(input) - 1U, stream) ==
	       sizeof(input) - 1U);
	assert(fclose(stream) == 0);

	status = firefly_config_publish(path, path, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status == EINVAL);

	stream = fopen(path, "rb");
	assert(stream != NULL);
	actual_length = fread(actual, 1, sizeof(actual), stream);
	assert(actual_length == sizeof(input) - 1U);
	assert(memcmp(actual, input, actual_length) == 0);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	assert_no_temporary_files(directory);
}


int
main(void)
{
	char directory[] = "/tmp/firefly-config-test.XXXXXX";
	char path[256];
	char default_path[256];
	char directory_target[256];
	char symlink_source[256];
	char symlink_output[256];
	char fifo_path[256];
	struct stat st;
	int status;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/mt-daapd.conf", directory) > 0);
	assert(snprintf(default_path, sizeof(default_path), "%s/default.conf", directory) > 0);
	assert(snprintf(directory_target, sizeof(directory_target), "%s/directory-target", directory) > 0);
	assert(snprintf(symlink_source, sizeof(symlink_source), "%s/source-link", directory) > 0);
	assert(snprintf(symlink_output, sizeof(symlink_output), "%s/symlink-output.conf", directory) > 0);
	assert(snprintf(fifo_path, sizeof(fifo_path), "%s/fifo.conf", directory) > 0);

	test_embedded_nul_cannot_inject_following_directive(directory, path);
	test_overlong_key_line_cannot_inject_following_directive(directory, path);

	write_file(path,
		   "[general]\n"
		   "  web_root = /old/web\n"
		   "port = 1234\n"
		   "runas = old-user\n"
		   "admin_pw = keep-this-password\n"
		   "db_type = old\n"
		   "db_parms = /old/database\n"
		   "plugin_dir = /old/plugins\n"
		   "custom_setting = preserve-me\n"
		   "web_root_alias = preserve-web-root-alias\n"
		   "port_forward = preserve-port-forward\n"
		   "runas_role = preserve-runas-role\n"
		   "db_type_backup = preserve-db-type\n"
		   "db_parms_extra = preserve-db-parms\n"
		   "plugin_directory = preserve-plugin-dir\n"
		   "# port = comment-port\n"
		   "  ; runas = comment-runas\n");
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
	assert_contains(path, "web_root_alias = preserve-web-root-alias\n");
	assert_contains(path, "port_forward = preserve-port-forward\n");
	assert_contains(path, "runas_role = preserve-runas-role\n");
	assert_contains(path, "db_type_backup = preserve-db-type\n");
	assert_contains(path, "db_parms_extra = preserve-db-parms\n");
	assert_contains(path, "plugin_directory = preserve-plugin-dir\n");
	assert_contains(path, "# port = comment-port\n");
	assert_contains(path, "  ; runas = comment-runas\n");
	assert_not_contains(path, "old-user");
	assert_no_temporary_files(directory);

	/* Symlinks to regular source files remain supported and are pinned open. */
	assert(symlink(path, symlink_source) == 0);
	status = firefly_config_publish(symlink_output, symlink_source,
					"/mnt/firefly", "media-user", "media-pass");
	assert(status == 0);
	assert_contains(symlink_output, "custom_setting = preserve-me\n");
	assert(stat(symlink_output, &st) == 0 && S_ISREG(st.st_mode));
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
	status = firefly_config_publish(path, NULL, "/mnt/firefly",
					"media-user\nadmin_pw = injected", "media-pass");
	assert(status == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	status = firefly_config_publish(path, NULL, "/mnt/firefly",
					"media-user", "media-pass\r\nrunas = root");
	assert(status == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	status = firefly_config_publish(path, NULL, "/mnt/firefly\n[plugins]",
					"media-user", "media-pass");
	assert(status == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	status = firefly_config_publish(path, directory, "/mnt/firefly",
					"media-user", "media-pass");
	assert(status != 0);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	/*
	 * A FIFO at the persisted source path used to block fopen() indefinitely.
	 * The bounded alarm turns a regression into a fast, deterministic failure.
	 */
	assert(mkfifo(fifo_path, 0600) == 0);
	alarm(5);
	status = firefly_config_publish(fifo_path, fifo_path, "/mnt/firefly",
					"media-user", "media-pass");
	alarm(0);
	assert(status == EINVAL);
	assert(lstat(fifo_path, &st) == 0 && S_ISFIFO(st.st_mode));
	assert_no_temporary_files(directory);
	assert(unlink(fifo_path) == 0);

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
	assert(unlink(symlink_source) == 0);
	assert(unlink(symlink_output) == 0);
	assert(rmdir(directory_target) == 0);
	assert(rmdir(directory) == 0);
	puts("Firefly atomic configuration publication: PASS");
	return 0;
}
