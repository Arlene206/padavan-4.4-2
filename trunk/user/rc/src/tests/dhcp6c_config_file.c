#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int injected_file_fsync(int fd);
static int injected_parent_sync(const char *target_path);
#define DHCP6C_CONFIG_FILE_FSYNC(fd) injected_file_fsync(fd)
#define DHCP6C_CONFIG_FILE_SYNC_PARENT(path) injected_parent_sync(path)
#include "dhcp6c_config_file.h"

static int file_fsync_calls;
static int file_fsync_interruptions_remaining;
static int file_fsync_error;
static int parent_sync_calls;
static int parent_sync_error;

static int
injected_file_fsync(int fd)
{
	file_fsync_calls++;
	if (file_fsync_interruptions_remaining > 0) {
		file_fsync_interruptions_remaining--;
		errno = EINTR;
		return -1;
	}
	if (file_fsync_error != 0) {
		errno = file_fsync_error;
		return -1;
	}
	return fsync(fd);
}

static int
injected_parent_sync(const char *target_path)
{
	parent_sync_calls++;
	if (parent_sync_error != 0) {
		errno = parent_sync_error;
		return -1;
	}
	return config_file_sync_parent(target_path);
}

int
main(void)
{
	char directory[] = "/tmp/dhcp6c-config-test.XXXXXX";
	char target[256], staged[256], failed_stage[256], bad_stage[256];
	char failed_sync_stage[256], directory_sync_stage[256];
	char missing_parent_target[256], symlink_parent[256], symlink_target[256];
	char buffer[128];
	FILE *stream = NULL;
	struct stat st;
	mode_t saved_umask;
	int fd;
	size_t length;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(target, sizeof(target), "%s/dhcp6c.conf", directory) > 0);
	assert(snprintf(staged, sizeof(staged), "%s/.dhcp6c.conf.XXXXXX", directory) > 0);
	assert(snprintf(failed_stage, sizeof(failed_stage),
			"%s/.dhcp6c-failed.XXXXXX", directory) > 0);
	assert(snprintf(bad_stage, sizeof(bad_stage),
			"%s/missing/.dhcp6c.conf.XXXXXX", directory) > 0);
	assert(snprintf(failed_sync_stage, sizeof(failed_sync_stage),
			"%s/.dhcp6c-fsync.XXXXXX", directory) > 0);
	assert(snprintf(directory_sync_stage, sizeof(directory_sync_stage),
			"%s/.dhcp6c-dirsync.XXXXXX", directory) > 0);
	assert(snprintf(missing_parent_target, sizeof(missing_parent_target),
			"%s/missing/dhcp6c.conf", directory) > 0);
	assert(snprintf(symlink_parent, sizeof(symlink_parent),
			"%s/parent-link", directory) > 0);
	assert(snprintf(symlink_target, sizeof(symlink_target),
			"%s/dhcp6c.conf", symlink_parent) > 0);

	/* Even a permissive PPP callback umask must not expose the new config. */
	saved_umask = umask(0000);
	fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "old configuration\n", 18) == 18);
	assert(close(fd) == 0);

	assert(dhcp6c_config_file_open_staged(staged, &stream) == 0);
	assert(stream != NULL);
	assert(lstat(staged, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0600);
	assert(fputs("new configuration\n", stream) >= 0);
	file_fsync_interruptions_remaining = 2;
	{
		int calls_before = file_fsync_calls;
		assert(dhcp6c_config_file_publish(stream, staged, target) == 0);
		assert(file_fsync_calls - calls_before == 3);
	}
	stream = NULL;
	assert(file_fsync_interruptions_remaining == 0);
	assert(parent_sync_calls == 1);

	assert(lstat(target, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0600);
	assert(access(staged, F_OK) != 0 && errno == ENOENT);
	stream = fopen(target, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, "new configuration\n") == 0);

	/* A file fsync error must keep the old active configuration. */
	assert(dhcp6c_config_file_open_staged(failed_sync_stage, &stream) == 0);
	assert(fputs("must-not-publish\n", stream) >= 0);
	file_fsync_error = EIO;
	errno = 0;
	assert(dhcp6c_config_file_publish(stream, failed_sync_stage, target) == -1);
	assert(errno == EIO);
	file_fsync_error = 0;
	stream = NULL;
	assert(parent_sync_calls == 1);
	assert(access(failed_sync_stage, F_OK) != 0 && errno == ENOENT);
	stream = fopen(target, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, "new configuration\n") == 0);

	/* A directory fsync error is reported after rename; do not start on it. */
	assert(dhcp6c_config_file_open_staged(directory_sync_stage, &stream) == 0);
	assert(fputs("directory-sync-error\n", stream) >= 0);
	parent_sync_error = EIO;
	errno = 0;
	assert(dhcp6c_config_file_publish(stream, directory_sync_stage, target) == -1);
	assert(errno == EIO);
	parent_sync_error = 0;
	stream = NULL;
	assert(parent_sync_calls == 2);
	assert(access(directory_sync_stage, F_OK) != 0 && errno == ENOENT);
	stream = fopen(target, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, "directory-sync-error\n") == 0);

	/* The shared barrier rejects missing and symlinked parent directories. */
	assert(config_file_sync_parent(missing_parent_target) == -1);
	assert(symlink(directory, symlink_parent) == 0);
	assert(config_file_sync_parent(symlink_target) == -1);
	assert(unlink(symlink_parent) == 0);

	/* A failed rename must discard the staged file, not the active config. */
	assert(dhcp6c_config_file_open_staged(failed_stage, &stream) == 0);
	assert(fputs("unpublished configuration\n", stream) >= 0);
	assert(dhcp6c_config_file_publish(stream, failed_stage, directory) == -1);
	stream = NULL;
	assert(access(failed_stage, F_OK) != 0 && errno == ENOENT);
	stream = fopen(target, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, "new configuration\n") == 0);

	/* Invalid staged locations fail before publishing anything. */
	assert(dhcp6c_config_file_open_staged(bad_stage, &stream) == -1);
	assert(stream == NULL);

	assert(unlink(target) == 0);
	assert(rmdir(directory) == 0);
	umask(saved_umask);

	assert(parent_sync_calls == 2);
	puts("DHCPv6 config staging, fsync retries, atomic replace and no-follow directory barrier: PASS");
	return 0;
}
