#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "storage_ejected_inventory_policy.h"

int
main(void)
{
	char device[32], mountpoint[256], long_mount[512], partition_name[32];
	char swap_path[256], swap_device[32];
	int matches;

	assert(storage_ejected_line_is_complete("/dev/sda1 /media/usb ext4 rw 0 0\n"));
	assert(!storage_ejected_line_is_complete("/dev/sda1 /media/usb ext4 rw"));

	assert(storage_ejected_parse_mount_line("/dev/sda1 /media/usb ext4 rw 0 0\n",
						 device, sizeof(device), mountpoint,
						 sizeof(mountpoint)) == 0);
	assert(strcmp(device, "/dev/sda1") == 0);
	assert(strcmp(mountpoint, "/media/usb") == 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/USB\\040Disk ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) == 0);
	assert(strcmp(mountpoint, "/media/USB Disk") == 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/USB\\134Disk ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) == 0);
	assert(strcmp(mountpoint, "/media/USB\\Disk") == 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/USB\\011Disk ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) == 0);
	assert(strcmp(mountpoint, "/media/USB\tDisk") == 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/USB\\04Disk ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/USB\\000Disk ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, sizeof(mountpoint)) != 0);
	assert(storage_ejected_parse_mount_line(
		"/dev/sda1 /media/USB\\040Disk ext4 rw 0 0\n",
		device, sizeof(device), mountpoint, 8) != 0);
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

	assert(storage_ejected_disk_matches_request(NULL, "sda"));
	assert(storage_ejected_disk_matches_request("sda", "sda"));
	assert(!storage_ejected_disk_matches_request("sda", "sdb"));
	assert(!storage_ejected_disk_matches_request("sda", "sda1"));
	assert(!storage_ejected_disk_matches_request("sda", NULL));
	assert(storage_ejected_disk_matches_request(NULL, NULL));

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

	assert(storage_ejected_storage_start_retry_required(3, 1));
	assert(storage_ejected_storage_start_retry_required(3, -1));
	assert(!storage_ejected_storage_start_retry_required(3, 0));
	assert(storage_ejected_storage_start_retry_required(-1, 0));
	assert(storage_ejected_storage_start_retry_required(-1, -1));
	assert(!storage_ejected_storage_start_retry_required(0, 1));
	assert(!storage_ejected_storage_start_retry_required(0, 0));

	assert(storage_ejected_may_restart_apps(0, 0, 3));
	assert(!storage_ejected_may_restart_apps(-1, 0, 3));
	assert(!storage_ejected_may_restart_apps(0, -1, 3));
	assert(!storage_ejected_may_restart_apps(0, 0, 0));
	assert(!storage_ejected_may_restart_apps(0, 0, -1));

	assert(storage_ejected_target_remove_complete(0, 0, 0, 0));
	assert(storage_ejected_target_remove_complete(0, 0, 3, 0));
	assert(!storage_ejected_target_remove_complete(-1, 0, 3, 0));
	assert(!storage_ejected_target_remove_complete(0, -1, 3, 0));
	assert(!storage_ejected_target_remove_complete(0, 0, -1, 0));
	assert(!storage_ejected_target_remove_complete(0, 0, 3, -1));

	assert(storage_ejected_inventory_complete(0, 0, 0));
	assert(!storage_ejected_inventory_complete(1, 0, 0));
	assert(!storage_ejected_inventory_complete(0, 1, 0));
	assert(!storage_ejected_inventory_complete(0, 0, 1));
	assert(storage_ejected_may_unmount(0, 0, 0, 0));
	assert(!storage_ejected_may_unmount(1, 0, 0, 0));
	assert(!storage_ejected_may_unmount(0, 1, 0, 0));
	assert(!storage_ejected_may_unmount(0, 0, 1, 0));
	assert(!storage_ejected_may_unmount(0, 0, 0, 1));

	assert(storage_ejected_parse_swaps_header(
		"Filename Type Size Used Priority\n") == 0);
	assert(storage_ejected_parse_swaps_header(
		"Filename Type Size Used\n") != 0);
	assert(storage_ejected_parse_swaps_header(
		"Filename Type Size Used Priority extra\n") != 0);
	assert(storage_ejected_parse_swaps_header(
		"Filename Type Size Used Priority") != 0);
	assert(storage_ejected_parse_swap_line(
		"/dev/sda1 partition 12582908 0 -2\n", swap_path,
		sizeof(swap_path)) == 0);
	assert(strcmp(swap_path, "/dev/sda1") == 0);
	assert(storage_ejected_parse_swap_line(
		"/dev/mmcblk0p2 partition 8388604 0 -3\n", swap_path,
		sizeof(swap_path)) == 0);
	assert(storage_ejected_parse_swap_line(
		"/dev/sda1 partition 12582908 0\n", swap_path,
		sizeof(swap_path)) != 0);
	assert(storage_ejected_parse_swap_line(
		"/dev/sda1 partition 12582908 0 -2", swap_path,
		sizeof(swap_path)) != 0);
	assert(storage_ejected_parse_swap_line(
		"/dev/sda1 partition 12582908 0 -2 extra\n", swap_path,
		sizeof(swap_path)) != 0);
	assert(storage_ejected_swap_device_name("/dev/sda", swap_device,
		sizeof(swap_device)) == 1);
	assert(strcmp(swap_device, "sda") == 0);
	assert(storage_ejected_swap_device_name("/dev/sdaa12", swap_device,
		sizeof(swap_device)) == 1);
	assert(strcmp(swap_device, "sdaa12") == 0);
	assert(storage_ejected_swap_device_name("/dev/mmcblk0p2", swap_device,
		sizeof(swap_device)) == 1);
	assert(strcmp(swap_device, "mmcblk0p2") == 0);
	assert(storage_ejected_swap_device_name("/mnt/opt/swapfile", swap_device,
		sizeof(swap_device)) == 0);
	assert(storage_ejected_swap_path_is_on_mount(
		"/media/usb/swapfile", "/media/usb"));
	assert(storage_ejected_swap_path_is_on_mount(
		"/media/usb/deep/dir/swapfile", "/media/usb/"));
	assert(storage_ejected_swap_path_is_on_mount("/swapfile", "/"));
	assert(!storage_ejected_swap_path_is_on_mount(
		"/media/usb2/swapfile", "/media/usb"));
	assert(!storage_ejected_swap_path_is_on_mount(
		"/media/usb", "/media/usb"));
	assert(!storage_ejected_swap_path_is_on_mount(
		"/media/usb/../internal/swapfile", "/media/usb"));
	assert(!storage_ejected_swap_path_is_on_mount(
		"media/usb/swapfile", "/media/usb"));
	assert(!storage_ejected_swap_path_is_on_mount(NULL, "/media/usb"));
	assert(!storage_ejected_swap_path_is_on_mount("/media/usb/swapfile", NULL));
	assert(storage_ejected_swap_device_name("/dev/sda/evil", swap_device,
		sizeof(swap_device)) == -1);
	assert(storage_ejected_swap_device_name("/dev/sd1", swap_device,
		sizeof(swap_device)) == -1);
	assert(storage_ejected_swap_device_name("/dev/mmcblk0px", swap_device,
		sizeof(swap_device)) == -1);
	assert(storage_ejected_swap_device_name("/dev/mmcblkp1", swap_device,
		sizeof(swap_device)) == -1);
	puts("Storage ejected-device inventory policy: PASS");
	return 0;
}
