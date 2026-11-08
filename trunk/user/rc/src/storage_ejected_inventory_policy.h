#ifndef PADAVAN_STORAGE_EJECTED_INVENTORY_POLICY_H
#define PADAVAN_STORAGE_EJECTED_INVENTORY_POLICY_H

#include <ctype.h>
#include <stddef.h>
#include <string.h>

/* procfs inventory records are line-oriented; a truncated record is unknown. */
static int
storage_ejected_line_is_complete(const char *line)
{
	size_t length;

	if (!line)
		return 0;

	length = strlen(line);
	return (length > 0 && line[length - 1] == '\n');
}

/* Return 1 for a token, 0 at end of line, and -1 for an invalid/long token. */
static int
storage_ejected_next_token(const char **cursor, char *token, size_t token_size)
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

static int
storage_ejected_parse_mount_line(const char *line, char *device,
				 size_t device_size, char *mountpoint,
				 size_t mountpoint_size)
{
	const char *cursor;
	int status;

	if (!storage_ejected_line_is_complete(line))
		return -1;

	cursor = line;
	status = storage_ejected_next_token(&cursor, device, device_size);
	if (status != 1)
		return -1;

	status = storage_ejected_next_token(&cursor, mountpoint, mountpoint_size);
	if (status != 1)
		return -1;

	return 0;
}

static int
storage_ejected_decimal_token(const char *token)
{
	const unsigned char *p;

	if (!token || !*token)
		return 0;

	for (p = (const unsigned char *)token; *p; p++) {
		if (!isdigit(*p))
			return 0;
	}

	return 1;
}

/* Parse /proc/partitions and report an exact device-name match. */
static int
storage_ejected_partition_matches(const char *line, const char *expected_device,
				  int *matches)
{
	char major[16], minor[16], blocks[32], name[32];
	const char *cursor;
	int status;

	if (!matches || !expected_device || !*expected_device ||
	    !storage_ejected_line_is_complete(line))
		return -1;

	*matches = 0;
	cursor = line;

	status = storage_ejected_next_token(&cursor, major, sizeof(major));
	if (status == 0)
		return 0;
	if (status != 1)
		return -1;

	/* The first line of /proc/partitions is a textual column header. */
	if (strcmp(major, "major") == 0)
		return 0;
	if (!storage_ejected_decimal_token(major))
		return -1;

	status = storage_ejected_next_token(&cursor, minor, sizeof(minor));
	if (status != 1 || !storage_ejected_decimal_token(minor))
		return -1;

	status = storage_ejected_next_token(&cursor, blocks, sizeof(blocks));
	if (status != 1 || !storage_ejected_decimal_token(blocks))
		return -1;

	status = storage_ejected_next_token(&cursor, name, sizeof(name));
	if (status != 1)
		return -1;

	*matches = (strcmp(name, expected_device) == 0);
	return 0;
}

static int
storage_ejected_inventory_complete(int read_error, int close_error,
				   int parse_error)
{
	return !read_error && !close_error && !parse_error;
}

static int
storage_ejected_may_unmount(int active, int read_error, int close_error,
			    int parse_error)
{
	return !active && storage_ejected_inventory_complete(read_error,
							     close_error,
							     parse_error);
}

#endif /* PADAVAN_STORAGE_EJECTED_INVENTORY_POLICY_H */
