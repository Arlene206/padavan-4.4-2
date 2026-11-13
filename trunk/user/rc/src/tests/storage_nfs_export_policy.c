#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "storage_nfs_export_policy.h"

int
main(void)
{
	char device[32], mountpoint[256], fstype[32], options[256];
	char ipv4_acl[32], ipv4_small[8];
	char long_line[700];
#if defined (USE_IPV6)
	char ipv6_acl[INET6_ADDRSTRLEN + 4];
#endif

	/* Keep client source ports privileged without changing legacy root mapping. */
	assert(strcmp(STORAGE_NFS_EXPORT_DEFAULT_OPTIONS,
		      "async,secure,no_root_squash,no_subtree_check") == 0);
	assert(strstr(STORAGE_NFS_EXPORT_DEFAULT_OPTIONS, "insecure") == NULL);
	assert(strstr(STORAGE_NFS_EXPORT_DEFAULT_OPTIONS, "no_root_squash") != NULL);

	/* Only a missing path permits generated exports; invalid overrides fail closed. */
	assert(storage_nfs_export_override_state(-1, ENOENT, 0, 0, -1, 0) == 0);
	assert(storage_nfs_export_override_state(-1, EACCES, 0, 0, -1, 0) == -1);
	assert(storage_nfs_export_override_state(-1, ENOTDIR, 0, 0, -1, 0) == -1);
	assert(storage_nfs_export_override_state(0, 0, 1, 0, -1, 0) == 1);
	assert(storage_nfs_export_override_state(0, 0, 0, 1, 0, 1) == 1);
	assert(storage_nfs_export_override_state(0, 0, 0, 1, -1, 0) == -1);
	assert(storage_nfs_export_override_state(0, 0, 0, 1, 0, 0) == -1);
	assert(storage_nfs_export_override_state(0, 0, 0, 0, -1, 0) == -1);

	/* Canonical IPv4 ACLs must preserve valid CIDR scope exactly. */
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42", "255.255.255.0",
		ipv4_acl, sizeof(ipv4_acl)) == 1);
	assert(strcmp(ipv4_acl, "192.168.1.0/24") == 0);
	assert(storage_nfs_export_format_ipv4_acl(
		"10.8.7.9", "255.255.255.255",
		ipv4_acl, sizeof(ipv4_acl)) == 1);
	assert(strcmp(ipv4_acl, "10.8.7.9/32") == 0);
	assert(storage_nfs_export_format_ipv4_acl(
		"128.0.0.9", "128.0.0.0",
		ipv4_acl, sizeof(ipv4_acl)) == 1);
	assert(strcmp(ipv4_acl, "128.0.0.0/1") == 0);

	/* Invalid masks/addresses must not widen or corrupt an export ACL. */
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42", "255.0.255.0",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(ipv4_acl[0] == '\0');
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42", "255.255.0.255",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42", "0.0.0.0",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		"0.0.0.0", "255.255.255.0",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		"255.255.255.255", "255.255.255.0",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42/24", "255.255.255.0",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42;0.0.0.0/0", "255.255.255.0",
		ipv4_acl, sizeof(ipv4_acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		"192.168.1.42", "255.255.255.0",
		ipv4_small, sizeof(ipv4_small)) == -1);
	assert(ipv4_small[0] == '\0');

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
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw 0\n", device, sizeof(device),
		mountpoint, sizeof(mountpoint), fstype, sizeof(fstype),
		options, sizeof(options)) != 0);
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw 0 0\n", device, sizeof(device),
		mountpoint, sizeof(mountpoint), fstype, sizeof(fstype),
		options, sizeof(options)) == 0);
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw invalid 0\n", device, sizeof(device),
		mountpoint, sizeof(mountpoint), fstype, sizeof(fstype),
		options, sizeof(options)) != 0);
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw 0 0 extra\n", device, sizeof(device),
		mountpoint, sizeof(mountpoint), fstype, sizeof(fstype),
		options, sizeof(options)) != 0);
	assert(storage_nfs_export_parse_mount_line(
		"/dev/sda1 /media/usb ext4 rw 12345678901234567 0\n",
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

#if defined (USE_IPV6)
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:0db8:0000:0000:0000:0000:0000:0001", "64",
		ipv6_acl, sizeof(ipv6_acl)) == 1);
	assert(strcmp(ipv6_acl, "2001:db8::1/64") == 0);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1/56", "64", ipv6_acl, sizeof(ipv6_acl)) == 1);
	assert(strcmp(ipv6_acl, "2001:db8::1/64") == 0);
	assert(storage_nfs_export_format_ipv6_acl(
		"", "64", ipv6_acl, sizeof(ipv6_acl)) == 0);
	assert(ipv6_acl[0] == '\0');
	assert(storage_nfs_export_format_ipv6_acl(
		NULL, NULL, ipv6_acl, sizeof(ipv6_acl)) == 0);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1", "48", ipv6_acl, sizeof(ipv6_acl)) == 1);
	assert(strcmp(ipv6_acl, "2001:db8::1/48") == 0);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1", "47", ipv6_acl, sizeof(ipv6_acl)) == -1);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1", "65", ipv6_acl, sizeof(ipv6_acl)) == -1);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1", "64;rw,no_root_squash", ipv6_acl,
		sizeof(ipv6_acl)) == -1);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1) 0.0.0.0/0(rw)", "64", ipv6_acl,
		sizeof(ipv6_acl)) == -1);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1/64;rw,no_root_squash", "64", ipv6_acl,
		sizeof(ipv6_acl)) == -1);
	assert(storage_nfs_export_format_ipv6_acl(
		"not-an-ipv6-address", "64", ipv6_acl, sizeof(ipv6_acl)) == -1);
	assert(storage_nfs_export_format_ipv6_acl(
		"2001:db8::1", "128", ipv6_acl, sizeof(ipv6_acl)) == -1);
#endif /* USE_IPV6 */

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

	/* RC callers must observe export-generation and daemon-start failures. */
	assert(storage_nfs_start_status(0, 0) == 0);
	assert(storage_nfs_start_status(-1, 0) == -1);
	assert(storage_nfs_start_status(0, 17) == -1);
	assert(storage_nfs_start_status(-1, 17) == -1);

	puts("Storage NFS override, source-port policy, inventory, IPv6 ACL and start gate: PASS");
	return 0;
}
