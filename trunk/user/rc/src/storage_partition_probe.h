#ifndef PADAVAN_STORAGE_PARTITION_PROBE_H
#define PADAVAN_STORAGE_PARTITION_PROBE_H

#include <stdio.h>
#include <string.h>
#include "disk_inventory_policy.h"

/*
 * Return 1 when a disk has at least one child partition, 0 when no child
 * partition is present, and -1 when the source or disk name is invalid.
 * A negative result must be treated as "do not mount the whole disk".
 */
static int
storage_disk_name_is_valid(const char *disk_name, int is_mmc)
{
	const char *p;
	size_t length;

	if (!disk_name)
		return 0;

	length = strlen(disk_name);
	if (is_mmc) {
		if (length < 7 || strncmp(disk_name, "mmcblk", 6) != 0)
			return 0;
		for (p = disk_name + 6; *p; ++p) {
			if (*p < '0' || *p > '9')
				return 0;
		}
		return 1;
	}

	if (length < 3 || strncmp(disk_name, "sd", 2) != 0)
		return 0;
	for (p = disk_name + 2; *p; ++p) {
		if (*p < 'a' || *p > 'z')
			return 0;
	}

	return 1;
}

static int
storage_partition_name_is_child(const char *partition_name,
                                const char *disk_name, int is_mmc)
{
	const char *suffix;
	size_t disk_name_len;

	disk_name_len = strlen(disk_name);
	if (strncmp(partition_name, disk_name, disk_name_len) != 0)
		return 0;

	suffix = partition_name + disk_name_len;
	if (is_mmc) {
		if (*suffix++ != 'p')
			return 0;
	}
	if (!*suffix)
		return 0;

	while (*suffix >= '0' && *suffix <= '9')
		suffix++;
	return *suffix == '\0';
}

static int
storage_disk_has_partitions(FILE *proc_partitions, const char *disk_name,
                            int is_mmc)
{
	char line[128], partition_name[64];
	unsigned int major_number, minor_number;
	unsigned long long disk_size;
	int found = 0, header_seen = 0, parse_status;

	if (!proc_partitions || !storage_disk_name_is_valid(disk_name, is_mmc))
		return -1;

	/*
	 * /proc/partitions is a safety boundary: malformed, partial or truncated
	 * inventory must never be interpreted as an empty disk. Read to EOF even
	 * after finding a child so a bad trailing record cannot authorize a
	 * whole-disk mount.
	 */
	while (fgets(line, sizeof(line), proc_partitions)) {
		if (!disk_inventory_line_is_complete(line))
			return -1;
		if (disk_inventory_line_is_blank(line))
			continue;

		if (!header_seen) {
			if (disk_inventory_parse_header_line(line) != 0)
				return -1;
			header_seen = 1;
			continue;
		}

		parse_status = disk_inventory_parse_partition_line(
			line, &major_number, &minor_number, &disk_size,
			partition_name, sizeof(partition_name));
		if (parse_status != 1)
			return -1;

		if (storage_partition_name_is_child(partition_name, disk_name,
						    is_mmc))
			found = 1;
	}

	if (ferror(proc_partitions) || !header_seen)
		return -1;

	return found;
}

#endif /* PADAVAN_STORAGE_PARTITION_PROBE_H */
