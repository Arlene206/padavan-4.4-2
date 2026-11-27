#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "minidlna_config_file.h"

static void
assert_contains(const char *path, const char *expected)
{
	char buffer[8192];
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
	char buffer[8192];
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
	char directory[] = "/tmp/minidlna-config-test.XXXXXX";
	char path[256];
	char directory_target[256];
	char missing_parent[256];
	struct minidlna_config_values values;
	struct stat st;
	int status;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/minidlna.conf", directory) > 0);
	assert(snprintf(directory_target, sizeof(directory_target),
			"%s/directory-target", directory) > 0);

	values.interface_name = "br0";
	values.notify_interval = 300;
	values.media_source1 = "V,/media/video";
	values.media_source2 = "";
	values.media_source3 = "A,/media/audio";
	values.root_container = 3;
	values.force_sort = 1;
	values.friendly_name = "Padavan media server";
	values.database_dir = "/mnt/minidlna";

	status = minidlna_config_publish(path, &values);
	assert(status == 0);
	assert(config_file_sync_parent(path) == 0);
	assert_contains(path, "port=8200\n");
	assert_contains(path, "network_interface=br0\n");
	assert_contains(path, "notify_interval=300\n");
	assert_contains(path, "media_dir=V,/media/video\n");
	assert_contains(path, "media_dir=A,/media/audio\n");
	assert_not_contains(path, "media_dir=\n");
	assert_contains(path, "merge_media_dirs=no\n");
	assert_contains(path, "root_container=V\n");
	assert_contains(path, "force_sort_criteria=+upnp:class,+upnp:originalTrackNumber,+dc:title\n");
	assert_contains(path, "friendly_name=Padavan media server\n");
	assert_contains(path, "db_dir=/mnt/minidlna\n");
	assert_contains(path, "log_dir=/mnt/minidlna\n");
	assert_contains(path, "model_number=1\n");
	assert(stat(path, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert_no_temporary_files(directory);

	/* Re-publishing replaces the complete old config, not a truncated prefix. */
	{
		FILE *old = fopen(path, "w");
		assert(old != NULL);
		assert(fputs("old-incomplete-config\n", old) >= 0);
		assert(fclose(old) == 0);
	}
	values.root_container = 0;
	values.force_sort = 0;
	values.media_source2 = "P,/media/photos";
	status = minidlna_config_publish(path, &values);
	assert(status == 0);
	assert_not_contains(path, "old-incomplete-config");
	assert_contains(path, "media_dir=P,/media/photos\n");
	assert_not_contains(path, "root_container=");
	assert_not_contains(path, "force_sort_criteria=");
	assert_no_temporary_files(directory);

	{
		FILE *last_good = fopen(path, "w");
		assert(last_good != NULL);
		assert(fputs("last-good-configuration\n", last_good) >= 0);
		assert(fclose(last_good) == 0);
	}

	values.friendly_name = "Padavan\nmodel_number=999";
	assert(minidlna_config_publish(path, &values) == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	values.friendly_name = "Padavan media server";
	values.media_source1 = "V,/media/video\nmedia_dir=V,/etc";
	assert(minidlna_config_publish(path, &values) == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	values.media_source1 = "V,/media/video";
	values.interface_name = "br0\nnotify_interval=1";
	assert(minidlna_config_publish(path, &values) == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	values.interface_name = "br0";
	values.database_dir = "/mnt/minidlna\nlog_dir=/";
	assert(minidlna_config_publish(path, &values) == EINVAL);
	assert_contains(path, "last-good-configuration\n");
	assert_no_temporary_files(directory);

	values.database_dir = "/mnt/minidlna";
	assert(mkdir(directory_target, 0700) == 0);
	status = minidlna_config_publish(directory_target, &values);
	assert(status != 0);
	assert(stat(directory_target, &st) == 0 && S_ISDIR(st.st_mode));
	assert_no_temporary_files(directory);

	assert(minidlna_config_publish(NULL, &values) == EINVAL);
	assert(minidlna_config_publish(path, NULL) == EINVAL);

	assert(snprintf(missing_parent, sizeof(missing_parent),
			"%s/missing/minidlna.conf", directory) > 0);
	errno = 0;
	assert(config_file_sync_parent(missing_parent) != 0);
	assert(errno == ENOENT);

	assert(unlink(path) == 0);
	assert(rmdir(directory_target) == 0);
	assert(rmdir(directory) == 0);
	puts("MiniDLNA atomic configuration publication: PASS");
	return 0;
}
