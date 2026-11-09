#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "storage_ejected_inventory_policy.h"

int
main(void)
{
	char device[32], mountpoint[256], long_mount[512], partition_name[32];
	int matches;

	assert(storage_ejected_line_is_complete("/dev/sda1 /media/usb ext4 rw 0 0\n"));
	assert(!storage_ejected_line_is_complete("/dev/sda1 /media/usb ext4 rw"));

	assert(storage_ejected_parse_mount_line("/dev/sda1 /media/usb ext4 rw 0 0\n",
						 device, sizeof(device), mountpoint,
						 sizeof(mountpoint)) == 0);
	assert(strcmp(device, "/dev/sda1") == 0);
	assert(strcmp(mountpoint, "/media/usb") == 0);
	assert(storage_ejected_parse_mount_line("/dev/sda1\n", device,
						 sizeof(device), mountpoint,
						 sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line("/dev/sda1 /media/usb ext4 rw",
						 device, sizeof(device), mountpoint,
						 sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line("/dev/sda1 /media/usb ext4 rw 0\n",
						 device, sizeof(device), mountpoint,
						 sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line("/dev/sda1 /media/usb ext4\n",
						 device, sizeof(device), mountpoint,
						 sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw 0 0 extra\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/abcdefghijklmnopqrstuvwxyz123456 /media/usb ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);
	strcpy(long_mount, "/dev/sda1 /");
	memset(long_mount + strlen(long_mount), 'm', 300);
	strcpy(long_mount + strlen(long_mount), " ext4 rw 0 0\n");
	assert(storage_ejected_parse_mount_line(long_mount, device, sizeof(device),
						 mountpoint, sizeof(mountpoint)) != 0);

	assert(storage_ejected_parse_partition_line("major minor  #blocks  name\n",
						     partition_name, sizeof(partition_name)) == 0);
	assert(storage_ejected_parse_partition_line("\n", partition_name,
						     sizeof(partition_name)) == 0);
	assert(storage_ejected_parse_partition_line("   8  1  976762584 sda1\n",
						     partition_name, sizeof(partition_name)) == 1);
	assert(strcmp(partition_name, "sda1") == 0);
	assert(storage_ejected_parse_partition_line("   x  1  976762584 sda1\n",
						     partition_name, sizeof(partition_name)) == -1);
	assert(storage_ejected_parse_partition_line("   8  1  976762584 sda1",
						     partition_name, sizeof(partition_name)) == -1);
	assert(storage_ejected_parse_partition_line(
		"   8  1  976762584 sda1 extra\n", partition_name,
		sizeof(partition_name)) == -1);
	assert(storage_ejected_parse_partition_line(
		"major minor  #blocks  name extra\n", partition_name,
		sizeof(partition_name)) == -1);

	assert(storage_ejected_partition_matches("major minor  #blocks  name\n",
						  "sda1", &matches) == 0);
	assert(!matches);
	assert(storage_ejected_partition_matches("   8  1  976762584 sda1\n",
						  "sda1", &matches) == 0);
	assert(matches);
	assert(storage_ejected_partition_matches("   8  0  976762584 sda\n",
						  "sda1", &matches) == 0);
	assert(!matches);
	assert(storage_ejected_partition_matches("   x  1  976762584 sda1\n",
						  "sda1", &matches) != 0);
	assert(storage_ejected_partition_matches(
		"   8  1  976762584 abcdefghijklmnopqrstuvwxyz123456\n",
		"sda1", &matches) != 0);
	assert(storage_ejected_partition_matches("   8  1  976762584 sda1",
						  "sda1", &matches) != 0);

	assert(storage_ejected_disk_or_partition("sda", "sda", 0));
	assert(storage_ejected_disk_or_partition("sda1", "sda", 0));
	assert(storage_ejected_disk_or_partition("sda12", "sda", 0));
	assert(!storage_ejected_disk_or_partition("sdab1", "sda", 0));
	assert(!storage_ejected_disk_or_partition("sda1-extra", "sda", 0));
	assert(storage_ejected_disk_or_partition("sda1", "sda1", 0));
	assert(storage_ejected_disk_or_partition("mmcblk0", "mmcblk0", 1));
	assert(storage_ejected_disk_or_partition("mmcblk0p1", "mmcblk0", 1));
	assert(storage_ejected_disk_or_partition("mmcblk0p12", "mmcblk0", 1));
	assert(!storage_ejected_disk_or_partition("mmcblk0p", "mmcblk0", 1));
	assert(!storage_ejected_disk_or_partition("mmcblk01p1", "mmcblk0", 1));
	assert(storage_ejected_mount_matches_disk("/dev/sda12", "sda"));
	assert(!storage_ejected_mount_matches_disk("/dev/sdab1", "sda"));
	assert(storage_ejected_mount_matches_disk("/dev/mmcblk0p1", "mmcblk0"));
	assert(!storage_ejected_mount_matches_disk("/dev/mmcblk0p1", "mmcblk1"));
	assert(storage_ejected_may_unmount_device(1, 0));
	assert(!storage_ejected_may_unmount_device(0, 0));
	assert(!storage_ejected_may_unmount_device(1, -1));

	assert(storage_ejected_inventory_complete(0, 0, 0));
	assert(!storage_ejected_inventory_complete(1, 0, 0));
	assert(!storage_ejected_inventory_complete(0, 1, 0));
	assert(!storage_ejected_inventory_complete(0, 0, 1));
	assert(storage_ejected_may_unmount(0, 0, 0, 0));
	assert(!storage_ejected_may_unmount(1, 0, 0, 0));
	assert(!storage_ejected_may_unmount(0, 1, 0, 0));
	assert(!storage_ejected_may_unmount(0, 0, 1, 0));
	assert(!storage_ejected_may_unmount(0, 0, 0, 1));

	puts("Storage ejected-device inventory policy: PASS");
	return 0;
}
