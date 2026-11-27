#ifndef IGMPPROXY_CONFIG_FILE_H
#define IGMPPROXY_CONFIG_FILE_H

#include "config_file_sync.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iptv_proxy_altnet_policy.h"

struct igmpproxy_config_values {
	const char *upstream_interface;
	const char *altnet;
	const char *downstream_interface;
	int quickleave;
};

/* Configuration values are emitted as single parser tokens. */
static int
igmpproxy_config_token(const char *value)
{
	const unsigned char *cursor = (const unsigned char *)value;

	if (cursor == NULL || *cursor == '\0')
		return 0;
	while (*cursor != '\0') {
		/* Whitespace splits directives; '#' starts a config comment. */
		if (*cursor <= 0x20 || *cursor >= 0x7f || *cursor == '#')
			return 0;
		++cursor;
	}
	return 1;
}

static int
igmpproxy_config_altnet_token(const char *value)
{
	return igmpproxy_config_token(value) &&
		iptv_proxy_altnet_is_valid(value);
}
static const char *
igmpproxy_config_altnet_or_default(const char *value)
{
	return (value && value[0] != '\0') ? value : "0.0.0.0/0";
}

static int
igmpproxy_config_printf(FILE *stream, const char *format, ...)
{
	va_list arguments;
	int result;

	errno = 0;
	va_start(arguments, format);
	result = vfprintf(stream, format, arguments);
	va_end(arguments);
	if (result < 0)
		return errno ? errno : EIO;
	return 0;
}

static int
igmpproxy_config_write(FILE *stream,
		       const struct igmpproxy_config_values *values)
{
	int status;

	if (stream == NULL || values == NULL ||
	    !igmpproxy_config_token(values->upstream_interface) ||
	    !igmpproxy_config_altnet_token(values->altnet) ||
	    !igmpproxy_config_token(values->downstream_interface) ||
	    (values->quickleave != 0 && values->quickleave != 1))
		return EINVAL;

	status = igmpproxy_config_printf(stream, "# automatically generated\n");
	if (status == 0 && values->quickleave)
		status = igmpproxy_config_printf(stream, "quickleave\n");
	if (status == 0)
		status = igmpproxy_config_printf(
			stream,
			"\nphyint %s upstream ratelimit 0  threshold 1\n"
			"\taltnet %s\n"
			"\nphyint %s downstream ratelimit 0  threshold 1\n"
			"\n",
			values->upstream_interface, values->altnet,
			values->downstream_interface);
	return status;
}

/* Publish beside the live file; any failure preserves the last-good config. */
static int
igmpproxy_config_publish(const char *path,
			 const struct igmpproxy_config_values *values)
{
	static const char suffix[] = ".tmp.XXXXXX";
	char *temporary_path = NULL;
	size_t path_length;
	FILE *output = NULL;
	int fd = -1;
	int status = 0;
	int sync_status;

	if (path == NULL || path[0] == '\0' || values == NULL)
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
		goto out;
	}
	output = fdopen(fd, "w");
	if (output == NULL) {
		status = errno ? errno : EIO;
		close(fd);
		fd = -1;
		goto out;
	}
	fd = -1;

	status = igmpproxy_config_write(output, values);
	if (status == 0) {
		errno = 0;
		if (fflush(output) != 0)
			status = errno ? errno : EIO;
	}
	if (status == 0) {
		do {
			errno = 0;
			sync_status = fsync(fileno(output));
		} while (sync_status != 0 && errno == EINTR);
		if (sync_status != 0)
			status = errno ? errno : EIO;
	}
	if (fclose(output) != 0 && status == 0)
		status = errno ? errno : EIO;
	output = NULL;

	if (status == 0) {
		errno = 0;
		if (rename(temporary_path, path) != 0)
			status = errno ? errno : EIO;
		else if (config_file_sync_parent(path) != 0)
			status = errno ? errno : EIO;
	}
	if (status != 0)
		unlink(temporary_path);

out:
	if (output != NULL)
		fclose(output);
	if (fd >= 0)
		close(fd);
	if (temporary_path != NULL) {
		if (status != 0)
			unlink(temporary_path);
		free(temporary_path);
	}
	return status;
}

#endif /* IGMPPROXY_CONFIG_FILE_H */
