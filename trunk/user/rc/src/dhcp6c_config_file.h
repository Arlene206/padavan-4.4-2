#ifndef PADAVAN_DHCP6C_CONFIG_FILE_H
#define PADAVAN_DHCP6C_CONFIG_FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config_file_sync.h"

#ifndef DHCP6C_CONFIG_FILE_FSYNC
#define DHCP6C_CONFIG_FILE_FSYNC(fd) fsync(fd)
#endif

#ifndef DHCP6C_CONFIG_FILE_SYNC_PARENT
#define DHCP6C_CONFIG_FILE_SYNC_PARENT(path) config_file_sync_parent(path)
#endif

/*
 * Open a same-directory staged configuration with mkstemp's 0600 mode.
 * This remains private even when the caller has temporarily set umask(0000).
 */
static int
dhcp6c_config_file_open_staged(char *staged_path, FILE **stream_out)
{
	int fd, saved_errno;

	if (!staged_path || !stream_out) {
		errno = EINVAL;
		return -1;
	}
	*stream_out = NULL;

	fd = mkstemp(staged_path);
	if (fd < 0)
		return -1;

	*stream_out = fdopen(fd, "w");
	if (!*stream_out) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}

	return 0;
}

/*
 * Flush and fsync the staged file before closing and atomically renaming it.
 * Then fsync the parent directory so the new name survives a crash. A parent
 * sync failure is reported after rename; the new file may be visible even
 * though durable publication could not be confirmed.
 */
static int
dhcp6c_config_file_publish(FILE *stream, const char *staged_path,
			   const char *target_path)
{
	int output_error = 0, saved_errno = 0, sync_result;

	if (!stream || !staged_path || !target_path) {
		errno = EINVAL;
		return -1;
	}

	if (ferror(stream) || fflush(stream) != 0) {
		output_error = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (!output_error) {
		do {
			sync_result = DHCP6C_CONFIG_FILE_FSYNC(fileno(stream));
		} while (sync_result < 0 && errno == EINTR);
		if (sync_result < 0) {
			output_error = 1;
			saved_errno = errno ? errno : EIO;
		}
	}
	if (fclose(stream) != 0) {
		if (!output_error)
			saved_errno = errno ? errno : EIO;
		output_error = 1;
	}

	if (output_error) {
		unlink(staged_path);
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}

	if (rename(staged_path, target_path) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_path);
		errno = saved_errno;
		return -1;
	}
	if (DHCP6C_CONFIG_FILE_SYNC_PARENT(target_path) != 0) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		return -1;
	}

	return 0;
}

#endif /* PADAVAN_DHCP6C_CONFIG_FILE_H */
