/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include <dev_info.h>
#include <disk_initial.h>

#include "rc.h"
#include "storage_operation_lock.h"
#include "storage_partition_probe.h"
#include "storage_hotplug_name_policy.h"

static int
report_device_swapoff(const char *device_name)
{
	int status, saved_errno;

	status = try_device_swapoff(device_name);
	if (status != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage("Storage", "could not verify or disable swap for removed device %s (status %d, errno %d)",
			   device_name ? device_name : "(unknown)", status, saved_errno);
		errno = saved_errno;
	}

	return status;
}


static int
check_root_partition(const char *devname, int is_mmc)
{
	FILE *procpt;
	int result;

	if (!storage_disk_name_is_valid(devname, is_mmc))
		return -1;

	procpt = fopen(PROC_PARTITIONS_FILE, "r");
	if (!procpt)
		return -1;

	result = storage_disk_has_partitions(procpt, devname, is_mmc);
	if (fclose(procpt) != 0 && result == 0)
		return -1;

	return result;
}
#if defined (USE_BLK_DEV_SD)
int
mdev_sd_main(int argc, char **argv)
{
	int isLock, storage_lock;
	int swapoff_status = 0;
	int mount_status = 0;
	int root_partition_status = 0;
	int topology_status = 0;
	int share_name_status;
	int tune_status = 0;
	size_t len, disk_len;
	char share_name[32], disk_name[32];
	char *device_name, *action, *partno;

	if (argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 1;
	}

	device_name = argv[1];
	action = argv[2];

	if (get_device_type_by_device(device_name) != DEVICE_TYPE_SCSI_DISK)
		return 1;

	// Check Lock.
	if ((isLock = file_lock(device_name)) == -1)
		return 1;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock == -1) {
		file_unlock(isLock);
		return 1;
	}

	// If remove the device?
	if (!get_hotplug_action(action)) {
		swapoff_status = report_device_swapoff(device_name);
		len = strlen(device_name);
		if (len > 2 && device_name[0] == 's' && device_name[1] == 'd' &&
		    (device_name[len - 1] < '0' || device_name[len - 1] > '9'))
			notify_rc("on_unplug_mass_storage");
		
		goto out_unlock;
	}

	len = strlen(device_name);
	if (len >= sizeof(disk_name) || strncmp(device_name, "sd", 2) != 0)
		goto out_unlock;

	disk_len = len;
	while (disk_len > 2 &&
	       device_name[disk_len - 1] >= '0' &&
	       device_name[disk_len - 1] <= '9')
		disk_len--;
	if (disk_len <= 2)
		goto out_unlock;

	memcpy(disk_name, device_name, disk_len);
	disk_name[disk_len] = '\0';
	if (disk_len == len) {
		char *tune_argv[] = { "/sbin/hddtune.sh", disk_name, NULL };

		/* Whole disk: tune it and wait for partition-specific hotplug events. */
		tune_status = _eval(tune_argv, NULL, 0, NULL);
		if (tune_status != 0)
			logmessage("Storage", "disk tuning failed for %s (status %d); continuing storage mount",
				   disk_name, tune_status);
		root_partition_status = check_root_partition(disk_name, 0);
		if (root_partition_status < 0) {
			logmessage("Storage", "partition inventory is unknown for %s; refusing whole-device mount",
				   disk_name);
			topology_status = 1;
			goto out_unlock;
		}
		if (root_partition_status > 0)
			goto out_unlock;
		partno = "1";
	} else {
		partno = device_name + disk_len;
	}

	share_name_status = snprintf(share_name, sizeof(share_name),
				 "AiDisk_%s%s", disk_name + 2, partno);
	if (storage_hotplug_name_format_status(share_name_status,
					       sizeof(share_name)) != 0) {
		logmessage("Storage", "share name cannot be represented for %s (status %d); refusing mount",
			   device_name, share_name_status);
		topology_status = 1;
		goto out_unlock;
	}

	umask(0000);
	{
		char *mount_argv[] = { "/sbin/automount.sh", device_name, share_name, NULL };
		mount_status = _eval(mount_argv, NULL, 0, NULL);
		if (mount_status == 0)
			notify_rc("on_hotplug_mass_storage");
		else
			logmessage("Storage", "automount helper failed for %s (status %d)",
				   device_name, mount_status);
	}

out_unlock:
	file_unlock(storage_lock);
	file_unlock(isLock);

	return (swapoff_status == 0 && tune_status == 0 && mount_status == 0 && topology_status == 0) ? 0 : 1;
}
#endif

#if defined (USE_MMC_SUPPORT)
int
mdev_mmc_main(int argc, char **argv)
{
	int isLock, storage_lock;
	int swapoff_status = 0;
	int mount_status = 0;
	int root_partition_status = 0;
	int topology_status = 0;
	int share_name_status;
	size_t len, disk_len, i;
	const char *p;
	char share_name[32], disk_name[32];
	char *device_name, *action, *partno;

	if (argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 1;
	}

	device_name = argv[1];
	action = argv[2];

	if (get_device_type_by_device(device_name) != DEVICE_TYPE_MMC)
		return 1;

	// Check Lock.
	if ((isLock = file_lock(device_name)) == -1)
		return 1;

	storage_lock = storage_operation_lock_acquire(file_lock);
	if (storage_lock == -1) {
		file_unlock(isLock);
		return 1;
	}

	len = strlen(device_name);

	// If remove the device?
	if (!get_hotplug_action(action)) {
		swapoff_status = report_device_swapoff(device_name);
		if (strncmp(device_name, "mmcblk", 6) == 0 &&
		    !strchr(device_name + 6, 'p'))
			notify_rc("on_unplug_mass_storage");
		
		goto out_unlock;
	}

	if (len >= sizeof(disk_name) || strncmp(device_name, "mmcblk", 6) != 0)
		goto out_unlock;

	{
		char *partition = strrchr(device_name, 'p');
		if (partition) {
			disk_len = (size_t)(partition - device_name);
			partno = partition + 1;
			if (disk_len <= 6 || !*partno)
				goto out_unlock;
		} else {
			disk_len = len;
			partno = "1";
			root_partition_status = check_root_partition(device_name, 1);
			if (root_partition_status < 0) {
				logmessage("Storage", "partition inventory is unknown for %s; refusing whole-device mount",
					   device_name);
				topology_status = 1;
				goto out_unlock;
			}
			if (root_partition_status > 0)
				goto out_unlock;
		}
	}

	for (i = 6; i < disk_len; i++) {
		if (device_name[i] < '0' || device_name[i] > '9')
			goto out_unlock;
	}
	for (p = partno; *p; p++) {
		if (*p < '0' || *p > '9')
			goto out_unlock;
	}

	memcpy(disk_name, device_name, disk_len);
	disk_name[disk_len] = '\0';
	share_name_status = snprintf(share_name, sizeof(share_name),
				 "AiCard_%s%s", disk_name + 6, partno);
	if (storage_hotplug_name_format_status(share_name_status,
					       sizeof(share_name)) != 0) {
		logmessage("Storage", "share name cannot be represented for %s (status %d); refusing mount",
			   device_name, share_name_status);
		topology_status = 1;
		goto out_unlock;
	}

	umask(0000);
	{
		char *mount_argv[] = { "/sbin/automount.sh", device_name, share_name, NULL };
		mount_status = _eval(mount_argv, NULL, 0, NULL);
		if (mount_status == 0)
			notify_rc("on_hotplug_mass_storage");
		else
			logmessage("Storage", "automount helper failed for %s (status %d)",
				   device_name, mount_status);
	}

out_unlock:
	file_unlock(storage_lock);
	file_unlock(isLock);

	return (swapoff_status == 0 && mount_status == 0 && topology_status == 0) ? 0 : 1;
}
#endif
