#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "storage_swap_policy.h"

int
main(void)
{
	char path[64];
	char field[32];
	char long_name[96];

	assert(storage_swap_build_device_path("sda1", path, sizeof(path)) == 0);
	assert(strcmp(path, "/dev/sda1") == 0);
	assert(storage_swap_build_device_path("mmcblk0p2", path, sizeof(path)) == 0);
	assert(strcmp(path, "/dev/mmcblk0p2") == 0);
	assert(storage_swap_build_device_path("../sda1", path, sizeof(path)) == -1);
	assert(storage_swap_build_device_path("sda 1", path, sizeof(path)) == -1);
	assert(storage_swap_build_device_path(NULL, path, sizeof(path)) == -1);
	memset(long_name, 'x', sizeof(long_name) - 1);
	long_name[sizeof(long_name) - 1] = '\0';
	assert(storage_swap_build_device_path(long_name, path, sizeof(path)) == -1);

	assert(storage_swap_header_valid("Filename\t\tType\tSize\tUsed\tPriority\n"));
	assert(!storage_swap_header_valid("Device\t\tType\tSize\n"));
	assert(!storage_swap_header_valid("FilenameX Type Size\n"));

	assert(storage_swap_first_field("  /dev/sda1  partition  1024  0  -2\n",
					field, sizeof(field)) == 1);
	assert(strcmp(field, "/dev/sda1") == 0);
	assert(storage_swap_first_field("\t\n", field, sizeof(field)) == 0);
	assert(storage_swap_first_field("/this/field/is/too/long\n", field, 4) == -1);

	assert(storage_swap_line_matches_device("/dev/sda1 partition 1024 0 -2\n",
						"/dev/sda1") == 1);
	assert(storage_swap_line_matches_device("/dev/sda10 partition 1024 0 -2\n",
						"/dev/sda1") == 0);
	assert(storage_swap_line_matches_device("/swapfile file 1024 0 -2\n",
						"/dev/sda1") == 0);
	assert(storage_swap_line_matches_device("  \n", "/dev/sda1") == -1);

	assert(storage_swap_inventory_result(0, 0) == STORAGE_SWAP_INACTIVE);
	assert(storage_swap_inventory_result(1, 0) == STORAGE_SWAP_ACTIVE);
	assert(storage_swap_inventory_result(0, 1) == STORAGE_SWAP_UNKNOWN);
	assert(storage_swap_inventory_result(1, 1) == STORAGE_SWAP_UNKNOWN);

	{
		char device[32], mountpoint[256], swap_path[256];

		assert(storage_inventory_parse_mount_line(
			"/dev/sda1 /media/usb ext4 rw,relatime 0 0\n",
			device, sizeof(device), mountpoint, sizeof(mountpoint)) == 0);
		assert(strcmp(device, "/dev/sda1") == 0);
		assert(strcmp(mountpoint, "/media/usb") == 0);
		assert(storage_inventory_parse_mount_line(
			"/dev/sda1 /media/usb ext4 rw 0\n",
			device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);
		assert(storage_inventory_parse_mount_line(
			"/dev/sda1 /media/usb ext4 rw 0 0 extra\n",
			device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);
		assert(storage_inventory_parse_mount_line(
			"/dev/sda1 /media/usb ext4 rw 0 0",
			device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);

		assert(storage_inventory_parse_swaps_header(
			"Filename Type Size Used Priority\n") == 0);
		assert(storage_inventory_parse_swaps_header(
			"Filename Type Size Used\n") != 0);
		assert(storage_inventory_parse_swaps_header(
			"Filename Type Size Used Priority Extra\n") != 0);
		assert(storage_inventory_parse_swap_line(
			"/dev/sda2 partition 1024 0 -2\n",
			swap_path, sizeof(swap_path)) == 0);
		assert(strcmp(swap_path, "/dev/sda2") == 0);
		assert(storage_inventory_parse_swap_line(
			"/dev/sda2 partition 1024 0\n",
			swap_path, sizeof(swap_path)) != 0);
		assert(storage_inventory_parse_swap_line(
			"/dev/sda2 partition 1024 0 -2 extra\n",
			swap_path, sizeof(swap_path)) != 0);
		assert(storage_inventory_parse_swap_line(
			"/dev/sda2 partition 1024 0 -2",
			swap_path, sizeof(swap_path)) != 0);
		assert(!storage_inventory_line_is_complete("/dev/sda2 partition"));
		assert(storage_inventory_complete(0, 0, 0));
		assert(!storage_inventory_complete(1, 0, 0));
		assert(!storage_inventory_complete(0, 1, 0));
		assert(!storage_inventory_complete(0, 0, 1));
		assert(storage_inventory_may_unmount(0));
		assert(!storage_inventory_may_unmount(-1));
	}

	puts("Storage swap and global cleanup inventory policy: PASS");
	return 0;
}
