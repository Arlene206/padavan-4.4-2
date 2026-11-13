#ifndef PADAVAN_STORAGE_SWAP_POLICY_H
#define PADAVAN_STORAGE_SWAP_POLICY_H

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define STORAGE_SWAP_INACTIVE 0
#define STORAGE_SWAP_ACTIVE 1
#define STORAGE_SWAP_UNKNOWN (-1)

static int
storage_swap_build_device_path(const char *dev_name, char *path, size_t path_size)
{
	const unsigned char *p;
	int length;

	if (!dev_name || !*dev_name || !path || path_size == 0)
		return -1;

	for (p = (const unsigned char *)dev_name; *p; ++p) {
		if (*p == '/' || *p < 32 || *p == 127 || isspace(*p))
			return -1;
	}

	length = snprintf(path, path_size, "/dev/%s", dev_name);
	if (length < 0 || (size_t)length >= path_size) {
		path[0] = '\0';
		return -1;
	}

	return 0;
}

/* Return 1 for a field, 0 for a blank line, or -1 for invalid/truncated data. */
static int
storage_swap_first_field(const char *line, char *field, size_t field_size)
{
	const unsigned char *start, *p;
	size_t length;

	if (!line || !field || field_size == 0)
		return -1;

	p = (const unsigned char *)line;
	while (*p && isspace(*p))
		++p;
	if (!*p) {
		field[0] = '\0';
		return 0;
	}

	start = p;
	while (*p && !isspace(*p))
		++p;
	length = (size_t)(p - start);
	if (length >= field_size) {
		field[0] = '\0';
		return -1;
	}

	memcpy(field, start, length);
	field[length] = '\0';
	return 1;
}

static int
storage_swap_header_valid(const char *line)
{
	char field[16];

	return storage_swap_first_field(line, field, sizeof(field)) == 1 &&
		strcmp(field, "Filename") == 0;
}

/* Exact first-field comparison prevents /dev/sda matching /dev/sda1. */
static int
storage_swap_line_matches_device(const char *line, const char *device_path)
{
	char field[256];
	int parsed;

	if (!device_path || !*device_path)
		return -1;

	parsed = storage_swap_first_field(line, field, sizeof(field));
	if (parsed != 1)
		return -1;

	return strcmp(field, device_path) == 0;
}

static int
storage_swap_inventory_result(int found, int io_failed)
{
	if (io_failed)
		return STORAGE_SWAP_UNKNOWN;
	return found ? STORAGE_SWAP_ACTIVE : STORAGE_SWAP_INACTIVE;
}


/* Validate complete procfs records before global cleanup performs side effects. */
static int
storage_inventory_line_is_complete(const char *line)
{
	size_t length;

	if (!line)
		return 0;
	length = strlen(line);
	return length > 0 && line[length - 1] == '\n';
}

/* Return 1 for a field, 0 at end of record, and -1 for invalid/long data. */
static int
storage_inventory_next_field(const char **cursor, char *field, size_t field_size)
{
	const unsigned char *start, *p;
	size_t length;

	if (!cursor || !*cursor || !field || field_size == 0)
		return -1;

	p = (const unsigned char *)*cursor;
	while (*p && isspace(*p))
		++p;
	if (!*p) {
		*cursor = (const char *)p;
		return 0;
	}

	start = p;
	while (*p && !isspace(*p))
		++p;
	length = (size_t)(p - start);
	if (length == 0 || length >= field_size)
		return -1;

	memcpy(field, start, length);
	field[length] = '\0';
	*cursor = (const char *)p;
	return 1;
}

/* /proc/mounts has exactly six fields. */
static int
storage_inventory_parse_mount_line(const char *line, char *device,
				   size_t device_size, char *mountpoint,
				   size_t mountpoint_size)
{
	const char *cursor;
	char ignored[512];
	int status, i;

	if (!storage_inventory_line_is_complete(line))
		return -1;

	cursor = line;
	if (storage_inventory_next_field(&cursor, device, device_size) != 1 ||
	    storage_inventory_next_field(&cursor, mountpoint, mountpoint_size) != 1)
		return -1;

	for (i = 0; i < 4; i++) {
		status = storage_inventory_next_field(&cursor, ignored, sizeof(ignored));
		if (status != 1)
			return -1;
	}
	return storage_inventory_next_field(&cursor, ignored, sizeof(ignored)) == 0 ? 0 : -1;
}

/* Validate the full five-column /proc/swaps header. */
static int
storage_inventory_parse_swaps_header(const char *line)
{
	static const char *const expected[] = {
		"Filename", "Type", "Size", "Used", "Priority"
	};
	const char *cursor;
	char field[32];
	int i;

	if (!storage_inventory_line_is_complete(line))
		return -1;
	cursor = line;
	for (i = 0; i < 5; i++) {
		if (storage_inventory_next_field(&cursor, field, sizeof(field)) != 1 ||
		    strcmp(field, expected[i]) != 0)
			return -1;
	}
	return storage_inventory_next_field(&cursor, field, sizeof(field)) == 0 ? 0 : -1;
}

/* /proc/swaps data rows have exactly five fields. */
static int
storage_inventory_parse_swap_line(const char *line, char *swap_path,
				  size_t swap_path_size)
{
	const char *cursor;
	char ignored[512];
	int i;

	if (!storage_inventory_line_is_complete(line))
		return -1;
	cursor = line;
	if (storage_inventory_next_field(&cursor, swap_path, swap_path_size) != 1)
		return -1;

	for (i = 0; i < 4; i++) {
		if (storage_inventory_next_field(&cursor, ignored, sizeof(ignored)) != 1)
			return -1;
	}
	return storage_inventory_next_field(&cursor, ignored, sizeof(ignored)) == 0 ? 0 : -1;
}

static int
storage_inventory_complete(int read_error, int close_error, int parse_error)
{
	return !read_error && !close_error && !parse_error;
}

/* Do not unmount a backing device while any active swap could remain. */
static int
storage_inventory_may_unmount(int swapoff_status)
{
	return swapoff_status == 0;
}

#endif /* PADAVAN_STORAGE_SWAP_POLICY_H */
