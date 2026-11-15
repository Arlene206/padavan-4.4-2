#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "disk_inventory_policy.h"
int
main(void)
{
	unsigned int major, minor;
	unsigned long long blocks;
	char device[32];
	assert(disk_inventory_parse_header_line("major minor  #blocks  name\n") == 0);
	assert(disk_inventory_parse_header_line("major minor #blocks name extra\n") != 0);
	assert(disk_inventory_parse_header_line("major minor #block name\n") != 0);
	assert(disk_inventory_parse_header_line("major minor #blocks name") != 0);
	assert(disk_inventory_line_is_blank(" \t\r\n"));
	assert(!disk_inventory_line_is_blank("unexpected\n"));
	assert(!disk_inventory_line_is_blank("\ntrailing"));
	assert(disk_inventory_parse_partition_line("   8        0  976762584 sda\n", &major, &minor, &blocks, device, sizeof(device)) == 1);
	assert(major == 8 && minor == 0 && blocks == 976762584ULL && strcmp(device, "sda") == 0);
	assert(disk_inventory_parse_partition_line(" 179 1 123456 mmcblk0p1\n", &major, &minor, &blocks, device, sizeof(device)) == 1);
	assert(major == 179 && minor == 1 && strcmp(device, "mmcblk0p1") == 0);
	assert(disk_inventory_parse_partition_line(" 1 0 4096 ram0\n", &major, &minor, &blocks, device, sizeof(device)) == 1);
	assert(disk_inventory_parse_partition_line("\n", &major, &minor, &blocks, device, sizeof(device)) == 0);
	assert(disk_inventory_parse_partition_line(" -8 0 10 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line(" +8 0 10 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 -1 10 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 +10 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("4294967296 0 10 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 4294967296 10 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 18446744073709551616 sda\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 10 sda extra\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 10\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 10 abcdefghijklmnopqrstuvwxyz123456\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 10 sda", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("8 1 10 sda\nextra\n", &major, &minor, &blocks, device, sizeof(device)) != 0);
	assert(disk_inventory_parse_partition_line("4294967295 4294967295 18446744073709551615 sda\n", &major, &minor, &blocks, device, sizeof(device)) == 1);
	assert(major == UINT_MAX && minor == UINT_MAX && blocks == ULLONG_MAX);
	puts("Strict /proc/partitions parser policy: PASS");
	return 0;
}
