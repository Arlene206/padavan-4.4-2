#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "storage_nfs_export_policy.h"

int
main(void)
{
	char device[32], mountpoint[256], fstype[32], options[256];
	char long_line[700];

	assert(storage_nfs_export_line_is_complete(
		"/dev/sda1 /media/AiDisk_1 ext4 rw,relatime 0 0\n"));
	assert(!storage_nfs_export_line_is_complete(
		"/dev/sda1 /media/AiDisk_1 ext4 rw,relatime"));

	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/AiDisk_1 ext4 rw,relatime 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint),
		fstype, sizeof(fstype), options, sizeof(options)) == 0);
	assert(strcmp(device, "/dev/sda1") == 0);
	assert(strcmp(mountpoint, "/media/AiDisk_1") == 0);
	assert(strcmp(fstype, "ext4") == 0);
	assert(strcmp(options, "rw,relatime") == 0);

	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/disk\\040one ntfs ro 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint),
		fstype, sizeof(fstype), options, sizeof(options)) == 0);
	assert(strcmp(mountpoint, "/media/disk\\040one") == 0);
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4\n", device, sizeof(device),
		mountpoint, sizeof(mountpoint), fstype, sizeof(fstype),
		options, sizeof(options)) != 0);
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw,relatime 0 0",
		device, sizeof(device), mountpoint, sizeof(mountpoint),
		fstype, sizeof(fstype), options, sizeof(options)) != 0);

	{
		size_t prefix_length = strlen("/dev/sda1 /");

		strcpy(long_line, "/dev/sda1 /");
		memset(long_line + prefix_length, 'm', 300);
		strcpy(long_line + prefix_length + 300, " ext4 rw 0 0\n");
	}
	assert(storage_nfs_export_parse_mount_line(
		long_line, device, sizeof(device), mountpoint, sizeof(mountpoint),
		fstype, sizeof(fstype), options, sizeof(options)) != 0);

	assert(storage_nfs_export_inventory_complete(0, 0, 0));
	assert(!storage_nfs_export_inventory_complete(1, 0, 0));
	assert(!storage_nfs_export_inventory_complete(0, 1, 0));
	assert(!storage_nfs_export_inventory_complete(0, 0, 1));
	assert(storage_nfs_export_may_publish(1, 0, 0, 0));
	assert(!storage_nfs_export_may_publish(0, 0, 0, 0));
	assert(!storage_nfs_export_may_publish(1, 1, 0, 0));
	assert(!storage_nfs_export_may_publish(1, 0, 1, 0));
	assert(!storage_nfs_export_may_publish(1, 0, 0, 1));
	assert(storage_nfs_export_may_start(1, 0));
	assert(!storage_nfs_export_may_start(0, 0));
	assert(!storage_nfs_export_may_start(1, -1));

	puts("Storage NFS export inventory and start gate: PASS");
	return 0;
}
