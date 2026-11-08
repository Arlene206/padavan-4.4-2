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

#endif /* PADAVAN_STORAGE_SWAP_POLICY_H */
