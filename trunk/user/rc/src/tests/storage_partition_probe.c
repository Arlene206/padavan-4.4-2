#include <assert.h>
#include <stdio.h>
#include "storage_partition_probe.h"

static int
probe(const char *contents, const char *disk_name, int is_mmc)
{
	FILE *stream = tmpfile();
	int result;

	assert(stream != NULL);
	assert(fputs(contents, stream) >= 0);
	rewind(stream);
	result = storage_disk_has_partitions(stream, disk_name, is_mmc);
	assert(fclose(stream) == 0);
	return result;
}

static void
test_malformed_inventory_fails_closed(void)
{
	const char *header = "major minor #blocks name\n";

	assert(probe("8 0 100 sda\n", "sda", 0) == -1);
	assert(probe("wrong header\n8 1 20 sda1\n", "sda", 0) == -1);
	assert(probe("major minor #blocks name extra\n8 1 20 sda1\n",
		     "sda", 0) == -1);
	assert(probe("major minor #blocks name\n8 -1 20 sda1\n",
		     "sda", 0) == -1);
	assert(probe("major minor #blocks name\n8 1 +20 sda1\n",
		     "sda", 0) == -1);
	assert(probe("major minor #blocks name\n8 1 20 sda1 extra\n",
		     "sda", 0) == -1);
	assert(probe("major minor #blocks name\n8 1 20 sda1\n"
		     "bad trailing row\n", "sda", 0) == -1);
	assert(probe("major minor #blocks name\n"
		     "major minor #blocks name\n", "sda", 0) == -1);
	assert(probe(header, "sda", 0) == 0);
}

static void
test_truncated_inventory_fails_closed(void)
{
	char overlong[256];
	size_t length, index;

	assert(probe("major minor #blocks name\n8 1 20 sda1",
		     "sda", 0) == -1);
	assert(probe("major minor #blocks name\n8 1 20 sda1\n"
		     "8 2 30 sda2", "sda", 0) == -1);

	length = (size_t)snprintf(overlong, sizeof(overlong),
				  "major minor #blocks name\n8 1 20 ");
	for (index = length; index < 180; index++)
		overlong[index] = 'x';
	overlong[180] = '\n';
	overlong[181] = '\0';
	assert(probe(overlong, "sda", 0) == -1);
}

static void
test_scans_past_first_matching_partition(void)
{
	assert(probe("major minor #blocks name\n"
		     "8 0 100 sda\n"
		     "8 1 20 sda1\n"
		     "not-a-proc-partitions-row\n", "sda", 0) == -1);
	assert(probe("major minor #blocks name\n"
		     "8 0 100 sda\n"
		     "8 1 20 sda1\n"
		     "8 2 20 sda2\n", "sda", 0) == 1);
}


int
main(void)
{
	const char *partitions =
		"major minor  #blocks  name\n"
		"   8        0  1953525168 sda\n"
		"   8        1   976762584 sda1\n"
		"   8       16   976762584 sdb1\n"
		"   8       32   976762584 sdaa1\n"
		" 179        0    62333952 mmcblk0\n"
		" 179        1    31165440 mmcblk0p1\n"
		" 179       16    31165440 mmcblk1p1\n";

	assert(storage_disk_has_partitions(NULL, "sda", 0) == -1);
	test_malformed_inventory_fails_closed();
	test_truncated_inventory_fails_closed();
	test_scans_past_first_matching_partition();
	assert(probe(partitions, "sda", 0) == 1);
	assert(probe(partitions, "sdb", 0) == 1);
	assert(probe(partitions, "sdc", 0) == 0);
	assert(probe(partitions, "mmcblk0", 1) == 1);
	assert(probe(partitions, "mmcblk1", 1) == 1);
	assert(probe(partitions, "mmcblk2", 1) == 0);

	assert(probe("major minor #blocks name\n8 0 100 sda\n8 1 20 sdaX\n", "sda", 0) == 0);
	assert(probe("major minor #blocks name\n179 0 100 mmcblk0\n179 1 20 mmcblk0pX\n",
		     "mmcblk0", 1) == 0);
	assert(probe(partitions, "sda1", 0) == -1);
	assert(probe(partitions, "sd/", 0) == -1);
	assert(probe(partitions, "mmcblk0p1", 1) == -1);

	puts("Block-storage partition detection and fail-closed semantics: PASS");
	return 0;
}
