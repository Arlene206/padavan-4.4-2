#ifndef MINIUPNPD_CONFIG_FILE_H
#define MINIUPNPD_CONFIG_FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* A writer returns zero or an errno value. The target is replaced only after
 * the complete temporary file has been flushed, synced and closed. */
typedef int (*miniupnpd_config_writer_t)(FILE *stream, void *context);

static int
miniupnpd_config_publish(const char *path,
                         miniupnpd_config_writer_t writer,
                         void *context)
{
	static const char suffix[] = ".tmp.XXXXXX";
	char *temporary_path;
	size_t path_length;
	int fd, status, sync_status;
	FILE *stream;

	if (path == NULL || path[0] == '\0' || writer == NULL)
		return EINVAL;

	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(suffix))
		return ENAMETOOLONG;
	temporary_path = (char *)malloc(path_length + sizeof(suffix));
	if (temporary_path == NULL)
		return ENOMEM;
	memcpy(temporary_path, path, path_length);
	memcpy(temporary_path + path_length, suffix, sizeof(suffix));

	fd = mkstemp(temporary_path);
	if (fd < 0) {
		status = errno ? errno : EIO;
		free(temporary_path);
		return status;
	}

	stream = fdopen(fd, "w");
	if (stream == NULL) {
		status = errno ? errno : EIO;
		close(fd);
		unlink(temporary_path);
		free(temporary_path);
		return status;
	}

	status = writer(stream, context);
	if (status < 0)
		status = errno ? errno : EIO;
	if (status == 0 && fflush(stream) != 0)
		status = errno ? errno : EIO;
	if (status == 0) {
		do {
			sync_status = fsync(fileno(stream));
		} while (sync_status != 0 && errno == EINTR);
		if (sync_status != 0)
			status = errno ? errno : EIO;
	}
	if (fclose(stream) != 0 && status == 0)
		status = errno ? errno : EIO;

	if (status == 0 && rename(temporary_path, path) != 0)
		status = errno ? errno : EIO;
	if (status != 0)
		unlink(temporary_path);

	free(temporary_path);
	return status;
}

#endif /* MINIUPNPD_CONFIG_FILE_H */
