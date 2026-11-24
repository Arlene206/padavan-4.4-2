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
/*
 * /proc/mounts and /proc/swaps encode whitespace and backslashes as
 * three-digit octal escapes (for example, a space is \\040). Decode these
 * fields before using their paths with umount/swapoff or Optware hooks.
 */
static int
storage_ejected_next_token(const char **cursor, char *token, size_t token_size)
{
	const unsigned char *p, *end;
	size_t output_length;

	if (!cursor || !*cursor || !token || token_size == 0)
		return -1;

	p = (const unsigned char *)*cursor;
	while (*p == ' ' || *p == '\t' || *p == '\r')
		p++;

	if (*p == '\0' || *p == '\n') {
		*cursor = (const char *)p;
		return 0;
	}

	end = p;
	while (*end && *end != ' ' && *end != '\t' &&
	       *end != '\r' && *end != '\n')
		end++;

	output_length = 0;
	token[0] = '\0';
	while (p < end) {
		unsigned int decoded;

		if (*p == '\\') {
			if ((size_t)(end - p) < 4 ||
			    p[1] < '0' || p[1] > '7' ||
			    p[2] < '0' || p[2] > '7' ||
			    p[3] < '0' || p[3] > '7')
				return -1;
			decoded = (unsigned int)(p[1] - '0') * 64U +
				  (unsigned int)(p[2] - '0') * 8U +
				  (unsigned int)(p[3] - '0');
			if (decoded == 0 || decoded > 255U)
				return -1;
			p += 4;
		} else {
			decoded = *p++;
		}

		if (output_length + 1 >= token_size) {
			token[0] = '\0';
			return -1;
		}
		token[output_length++] = (char)decoded;
	}

	token[output_length] = '\0';
	*cursor = (const char *)end;
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

static int
storage_ejected_signed_decimal_token(const char *token)
{
	if (!token || !*token)
		return 0;
	if (*token == '-' || *token == '+')
		token++;
	return storage_ejected_decimal_token(token);
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


/* NULL selects all disks; a supplied name must match one inventory row exactly. */
static int
storage_ejected_disk_matches_request(const char *requested, const char *candidate)
{
	if (!requested)
		return 1;
	return candidate && *candidate && strcmp(requested, candidate) == 0;
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

/* Retry service startup only while remaining storage may need the services. */
static int
storage_ejected_storage_start_retry_required(int remaining_mounts, int start_status)
{
	return remaining_mounts < 0 ||
		(remaining_mounts > 0 && start_status != 0);
}

/* A targeted safe-remove may restart daemons only after BOTH cleanup stages
 * succeeded and a fresh inventory confirms other storage remains mounted. */
static int
storage_ejected_may_restart_apps(int unmount_status, int ejected_status,
				 int remaining_mounts)
{
	return unmount_status == 0 && ejected_status == 0 &&
		remaining_mounts > 0;
}

/* A targeted eject succeeds only after cleanup and any required app restart. */
static int
storage_ejected_target_remove_complete(int unmount_status, int ejected_status,
				       int remaining_mounts, int start_status)
{
	return unmount_status == 0 && ejected_status == 0 &&
		remaining_mounts >= 0 &&
		(remaining_mounts == 0 || start_status == 0);
}


/* /proc/swaps header must be the complete five-column kernel format. */
static int
storage_ejected_parse_swaps_header(const char *line)
{
	static const char *const expected[] = {
		"Filename", "Type", "Size", "Used", "Priority"
	};
	const char *cursor;
	char field[32];
	int i;

	if (!storage_ejected_line_is_complete(line))
		return -1;

	cursor = line;
	for (i = 0; i < 5; i++) {
		if (storage_ejected_next_token(&cursor, field, sizeof(field)) != 1 ||
		    strcmp(field, expected[i]) != 0)
			return -1;
	}
	return storage_ejected_next_token(&cursor, field, sizeof(field)) == 0 ? 0 : -1;
}

/* /proc/swaps rows contain a filename followed by exactly four fields. */
static int
storage_ejected_parse_swap_line(const char *line, char *swap_path,
				 size_t swap_path_size)
{
	const char *cursor;
	char type[16], size[32], used[32], priority[32], extra[2];

	if (!storage_ejected_line_is_complete(line))
		return -1;
	cursor = line;
	if (storage_ejected_next_token(&cursor, swap_path, swap_path_size) != 1 ||
	    storage_ejected_next_token(&cursor, type, sizeof(type)) != 1 ||
	    (strcmp(type, "partition") != 0 && strcmp(type, "file") != 0) ||
	    storage_ejected_next_token(&cursor, size, sizeof(size)) != 1 ||
	    !storage_ejected_decimal_token(size) ||
	    storage_ejected_next_token(&cursor, used, sizeof(used)) != 1 ||
	    !storage_ejected_decimal_token(used) ||
	    storage_ejected_next_token(&cursor, priority, sizeof(priority)) != 1 ||
	    !storage_ejected_signed_decimal_token(priority))
		return -1;

	return storage_ejected_next_token(&cursor, extra, sizeof(extra)) == 0 ? 0 : -1;
}

/*
 * Return 1 and copy a removable block-device name, 0 for a non-removable
 * swap path (for example a swap file), or -1 for a malformed removable path.
 */
static int
storage_ejected_swap_device_name(const char *swap_path, char *device,
				size_t device_size)
{
	const char *name, *p;
	size_t length;
	int digits;

	if (!swap_path || !device || device_size == 0)
		return -1;
	device[0] = '\0';

	if (strncmp(swap_path, "/dev/sd", 7) == 0 ||
	    strncmp(swap_path, "/dev/mmcblk", 11) == 0)
		name = swap_path + 5;
	else
		return 0;

	length = strlen(name);
	if (length == 0 || length >= device_size)
		return -1;

	if (strncmp(name, "sd", 2) == 0) {
		p = name + 2;
		while (*p >= 'a' && *p <= 'z')
			p++;
		if (p == name + 2)
			return -1;
		while (*p >= '0' && *p <= '9')
			p++;
		if (*p)
			return -1;
	} else if (strncmp(name, "mmcblk", 6) == 0) {
		p = name + 6;
		digits = 0;
		while (*p >= '0' && *p <= '9') {
			digits++;
			p++;
		}
		if (digits == 0)
			return -1;
		if (*p == 'p') {
			p++;
			digits = 0;
			while (*p >= '0' && *p <= '9') {
				digits++;
				p++;
			}
			if (digits == 0)
				return -1;
		}
		if (*p)
			return -1;
	} else {
		return -1;
	}

	memcpy(device, name, length + 1);
	return 1;
}

/*
 * Match an absolute swap-file path to a mountpoint by component boundary
 * rather than byte prefix. Reject dot components instead of normalizing so
 * lexical matching cannot associate a path that escapes the removable mount.
 */
static int
storage_ejected_path_has_normal_components(const char *path)
{
	const char *component, *end;

	if (!path || path[0] != '/')
		return 0;

	component = path + 1;
	while (*component) {
		while (*component == '/')
			component++;
		if (!*component)
			break;
		end = component;
		while (*end && *end != '/')
			end++;
		if ((end - component == 1 && component[0] == '.') ||
		    (end - component == 2 && component[0] == '.' &&
		     component[1] == '.'))
			return 0;
		component = end;
	}
	return 1;
}

static int
storage_ejected_swap_path_is_on_mount(const char *swap_path,
				      const char *mountpoint)
{
	size_t mount_length;

	if (!storage_ejected_path_has_normal_components(swap_path) ||
	    !storage_ejected_path_has_normal_components(mountpoint))
		return 0;

	mount_length = strlen(mountpoint);
	while (mount_length > 1 && mountpoint[mount_length - 1] == '/')
		mount_length--;

	if (mount_length == 1 && mountpoint[0] == '/')
		return swap_path[1] != '\0';

	return strlen(swap_path) > mount_length &&
	       strncmp(swap_path, mountpoint, mount_length) == 0 &&
	       swap_path[mount_length] == '/' &&
	       swap_path[mount_length + 1] != '\0';
}

#endif /* PADAVAN_STORAGE_EJECTED_INVENTORY_POLICY_H */
