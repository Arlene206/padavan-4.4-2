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
	char ignored[512];
	int status, i;

	if (!storage_ejected_line_is_complete(line))
		return -1;

	cursor = line;
	status = storage_ejected_next_token(&cursor, device, device_size);
	if (status != 1)
		return -1;

	status = storage_ejected_next_token(&cursor, mountpoint, mountpoint_size);
	if (status != 1)
		return -1;

	/* /proc/mounts has six fields; require the whole record before using it. */
	for (i = 0; i < 4; i++) {
		status = storage_ejected_next_token(&cursor, ignored, sizeof(ignored));
		if (status != 1)
			return -1;
	}

	/* Reject extra fields as well as missing fields. */
	return storage_ejected_next_token(&cursor, ignored, sizeof(ignored)) == 0 ? 0 : -1;
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

/* Parse one complete /proc/partitions row: 1=data, 0=header/blank, -1=invalid. */
static int
storage_ejected_parse_partition_line(const char *line, char *device,
				     size_t device_size)
{
	static const char *const header[] = { "minor", "#blocks", "name" };
	const char *cursor;
	char major[16], minor[16], blocks[32], ignored[32];
	int status, i;

	if (!device || device_size == 0 ||
	    !storage_ejected_line_is_complete(line))
		return -1;

	device[0] = '\0';
	cursor = line;
	status = storage_ejected_next_token(&cursor, major, sizeof(major));
	if (status == 0)
		return 0;
	if (status != 1)
		return -1;

	if (strcmp(major, "major") == 0) {
		for (i = 0; i < 3; i++) {
			if (storage_ejected_next_token(&cursor, ignored,
						       sizeof(ignored)) != 1 ||
			    strcmp(ignored, header[i]) != 0)
				return -1;
		}
		return storage_ejected_next_token(&cursor, ignored,
						  sizeof(ignored)) == 0 ? 0 : -1;
	}

	if (!storage_ejected_decimal_token(major))
		return -1;
	status = storage_ejected_next_token(&cursor, minor, sizeof(minor));
	if (status != 1 || !storage_ejected_decimal_token(minor))
		return -1;
	status = storage_ejected_next_token(&cursor, blocks, sizeof(blocks));
	if (status != 1 || !storage_ejected_decimal_token(blocks))
		return -1;
	status = storage_ejected_next_token(&cursor, device, device_size);
	if (status != 1)
		return -1;

	if (storage_ejected_next_token(&cursor, ignored, sizeof(ignored)) != 0) {
		device[0] = '\0';
		return -1;
	}

	return 1;
}

static int
storage_ejected_partition_matches(const char *line, const char *expected_device,
				  int *matches)
{
	char device[32];
	int status;

	if (!matches || !expected_device || !*expected_device)
		return -1;
	*matches = 0;
	status = storage_ejected_parse_partition_line(line, device, sizeof(device));
	if (status < 0)
		return -1;
	if (status == 1)
		*matches = (strcmp(device, expected_device) == 0);
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


/* Match one physical disk and its partition naming scheme without prefix aliases. */
static int
storage_ejected_disk_or_partition(const char *candidate, const char *disk,
				  int is_mmc)
{
	size_t disk_len;

	if (!candidate || !disk || !*disk)
		return 0;

	disk_len = strlen(disk);
	if (strncmp(candidate, disk, disk_len) != 0)
		return 0;

	candidate += disk_len;
	if (!*candidate)
		return 1;

	if (is_mmc) {
		if (*candidate != 'p')
			return 0;
		candidate++;
	}

	if (!*candidate)
		return 0;
	for (; *candidate; candidate++) {
		if (*candidate < '0' || *candidate > '9')
			return 0;
	}

	return 1;
}

static int
storage_ejected_mount_matches_disk(const char *device, const char *disk)
{
	if (!device || !disk)
		return 0;

	if (strncmp(device, "/dev/sd", 7) == 0)
		return storage_ejected_disk_or_partition(device + 5, disk, 0);
	if (strncmp(device, "/dev/mmcblk", 11) == 0)
		return storage_ejected_disk_or_partition(device + 5, disk, 1);

	return 0;
}

/* Do not unmount unless the mount inventory is complete and all swapoffs pass. */
static int
storage_ejected_may_unmount_device(int mount_inventory_complete,
				   int swapoff_status)
{
	return mount_inventory_complete && swapoff_status == 0;
}

#endif /* PADAVAN_STORAGE_EJECTED_INVENTORY_POLICY_H */
