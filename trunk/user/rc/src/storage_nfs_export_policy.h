#ifndef PADAVAN_STORAGE_NFS_EXPORT_POLICY_H
#define PADAVAN_STORAGE_NFS_EXPORT_POLICY_H

#include <stddef.h>
#include <string.h>

static int
storage_nfs_export_line_is_complete(const char *line)
{
	size_t length;

	if (!line)
		return 0;

	length = strlen(line);
	return (length > 0 && line[length - 1] == '\n');
}

/* Return 1 for a token, 0 at end of line, and -1 for an invalid token. */
static int
storage_nfs_export_next_token(const char **cursor, char *token,
			      size_t token_size)
{
	const char *start;
	const char *p;
	size_t length;

	if (!cursor || !*cursor || !token || token_size == 0)
		return -1;

	p = *cursor;
	while (*p == ' ' || *p == '\t' || *p == '\r')
		p++;

	if (*p == '\0' || *p == '\n') {
		*cursor = p;
		return 0;
	}

	start = p;
	while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
		p++;

	length = (size_t)(p - start);
	if (length == 0 || length >= token_size)
		return -1;

	memcpy(token, start, length);
	token[length] = '\0';
	*cursor = p;
	return 1;
}

/* Parse the four fields needed to generate an export from /proc/mounts. */
static int
storage_nfs_export_parse_mount_line(const char *line,
				    char *device, size_t device_size,
				    char *mountpoint, size_t mountpoint_size,
				    char *fstype, size_t fstype_size,
				    char *mount_options, size_t mount_options_size)
{
	const char *cursor;

	if (!storage_nfs_export_line_is_complete(line))
		return -1;

	cursor = line;
	if (storage_nfs_export_next_token(&cursor, device, device_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, mountpoint, mountpoint_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, fstype, fstype_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, mount_options,
					  mount_options_size) != 1)
		return -1;

	return 0;
}

static int
storage_nfs_export_inventory_complete(int read_error, int close_error,
				      int parse_error)
{
	return !read_error && !close_error && !parse_error;
}

static int
storage_nfs_export_may_publish(int enabled, int read_error, int close_error,
			       int parse_error)
{
	return enabled && storage_nfs_export_inventory_complete(read_error,
								close_error,
								parse_error);
}

static int
storage_nfs_export_may_start(int enabled, int export_status)
{
	return enabled && export_status == 0;
}

#endif /* PADAVAN_STORAGE_NFS_EXPORT_POLICY_H */
