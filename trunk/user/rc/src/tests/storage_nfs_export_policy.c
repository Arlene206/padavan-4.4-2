#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "storage_nfs_export_policy.h"

struct fake_nfs_nvram_cache {
	const char *names[8];
	const char *values[8];
	unsigned int count;
	unsigned int calls;
	int fail_at;
	int missing_at;
	int failure_errno;
	char cache_slot[128];
};


static int
fake_nfs_nvram_cache_get(void *opaque, const char *param_name, char **value)
{
	struct fake_nfs_nvram_cache *fake =
		(struct fake_nfs_nvram_cache *)opaque;
	unsigned int index = fake->calls++;

	if (index >= fake->count) {
		*value = NULL;
		return -1;
	}
	assert(strcmp(param_name, fake->names[index]) == 0);
	if ((int)index == fake->fail_at) {
		*value = NULL;
		errno = fake->failure_errno;
		return -1;
	}
	if ((int)index == fake->missing_at) {
		*value = NULL;
		return 0;
	}
	snprintf(fake->cache_slot, sizeof(fake->cache_slot), "%s",
		 fake->values[index]);
	*value = fake->cache_slot;
	return 1;
}


static void
test_acl_nvram_copies_survive_shared_cache_reuse(void)
{
	static const char *const names[] = {
		"lan_ipaddr_t", "lan_netmask_t", "lan_ipaddr", "lan_netmask",
		"ip6_lan_addr", "ip6_lan_size", "vpns_vnet"
	};
	static const char *const values[] = {
		"192.168.1.42", "255.0.255.0", "10.9.8.7",
		"255.255.255.0", "2001:db8::42", "64", "10.8.0.1"
	};
	struct fake_nfs_nvram_cache fake;
	char lan_address[32], lan_mask[32], fallback_address[32];
	char fallback_mask[32], ipv6_address[INET6_ADDRSTRLEN + 5];
	char ipv6_length[8], vpn_address[32], acl[INET6_ADDRSTRLEN + 4];

	memset(&fake, 0, sizeof(fake));
	fake.count = (unsigned int)(sizeof(names) / sizeof(names[0]));
	fake.fail_at = -1;
	fake.missing_at = -1;
	memcpy(fake.names, names, sizeof(names));
	memcpy(fake.values, values, sizeof(values));

	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[0], lan_address, sizeof(lan_address)) == 0);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[1], lan_mask, sizeof(lan_mask)) == 0);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[2], fallback_address, sizeof(fallback_address)) == 0);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[3], fallback_mask, sizeof(fallback_mask)) == 0);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[4], ipv6_address, sizeof(ipv6_address)) == 0);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[5], ipv6_length, sizeof(ipv6_length)) == 0);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, names[6], vpn_address, sizeof(vpn_address)) == 0);

	assert(fake.calls == fake.count);
	assert(strcmp(fake.cache_slot, "10.8.0.1") == 0);
	assert(strcmp(lan_address, "192.168.1.42") == 0);
	assert(strcmp(lan_mask, "255.0.255.0") == 0);
	assert(strcmp(fallback_address, "10.9.8.7") == 0);
	assert(strcmp(fallback_mask, "255.255.255.0") == 0);
	assert(strcmp(ipv6_address, "2001:db8::42") == 0);
	assert(strcmp(ipv6_length, "64") == 0);
	assert(strcmp(vpn_address, "10.8.0.1") == 0);

	assert(storage_nfs_export_format_ipv4_acl(
		lan_address, lan_mask, acl, sizeof(acl)) == -1);
	assert(storage_nfs_export_format_ipv4_acl(
		fallback_address, fallback_mask, acl, sizeof(acl)) == 1);
	assert(strcmp(acl, "10.9.8.0/24") == 0);
#if defined (USE_IPV6)
	assert(storage_nfs_export_format_ipv6_acl(
		ipv6_address, ipv6_length, acl, sizeof(acl)) == 1);
	assert(strcmp(acl, "2001:db8::42/64") == 0);
#endif
	assert(storage_nfs_export_format_ipv4_acl(
		vpn_address, "255.255.255.0", acl, sizeof(acl)) == 1);
	assert(strcmp(acl, "10.8.0.0/24") == 0);
}


static void
test_acl_nvram_copy_fails_closed_on_missing_error_and_truncation(void)
{
	struct fake_nfs_nvram_cache fake;
	char destination[16];

	memset(&fake, 0, sizeof(fake));
	fake.count = 1;
	fake.names[0] = "lan_ipaddr_t";
	fake.values[0] = "192.168.1.1";
	fake.fail_at = 0;
	fake.missing_at = -1;
	fake.failure_errno = EACCES;
	strcpy(destination, "stale");
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, "lan_ipaddr_t", destination, sizeof(destination)) == EACCES);
	assert(destination[0] == '\0');

	memset(&fake, 0, sizeof(fake));
	fake.count = 1;
	fake.names[0] = "optional_acl";
	fake.values[0] = "";
	fake.fail_at = -1;
	fake.missing_at = 0;
	strcpy(destination, "stale");
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, "optional_acl", destination, sizeof(destination)) == 0);
	assert(destination[0] == '\0');

	memset(&fake, 0, sizeof(fake));
	fake.count = 1;
	fake.names[0] = "long_acl";
	fake.values[0] = "192.168.100.255";
	fake.fail_at = -1;
	fake.missing_at = -1;
	strcpy(destination, "stale");
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, "long_acl", destination, 8) == ENAMETOOLONG);
	assert(destination[0] == '\0');

	assert(storage_nfs_export_copy_nvram_value(
		NULL, &fake, "long_acl", destination, sizeof(destination)) == EINVAL);
	assert(storage_nfs_export_copy_nvram_value(fake_nfs_nvram_cache_get,
		&fake, "long_acl", destination, 0) == EINVAL);
}


struct fake_nfs_nvram_transaction {
	const char *names[12];
	const char *values[12];
	unsigned int calls;
	unsigned int count;
	int read_fail_at;
	int missing_at;
	int read_error;
	int end_error;
	int begin_calls;
	int end_calls;
	char cache_slot[128];
};


static int
fake_nfs_transaction_begin(void *opaque)
{
	struct fake_nfs_nvram_transaction *fake =
		(struct fake_nfs_nvram_transaction *)opaque;

	fake->begin_calls++;
	return 0;
}


static int
fake_nfs_transaction_get(void *opaque, const char *param_name,
			 char **value)
{
	struct fake_nfs_nvram_transaction *fake =
		(struct fake_nfs_nvram_transaction *)opaque;
	unsigned int index = fake->calls++;

	if (index >= fake->count) {
		*value = NULL;
		return -1;
	}
	assert(strcmp(param_name, fake->names[index]) == 0);
	if ((int)index == fake->read_fail_at) {
		*value = NULL;
		errno = fake->read_error;
		return -1;
	}
	if ((int)index == fake->missing_at) {
		*value = NULL;
		return 0;
	}
	snprintf(fake->cache_slot, sizeof(fake->cache_slot), "%s",
		 fake->values[index]);
	*value = fake->cache_slot;
	return 1;
}


static int
fake_nfs_transaction_end(void *opaque)
{
	struct fake_nfs_nvram_transaction *fake =
		(struct fake_nfs_nvram_transaction *)opaque;

	fake->end_calls++;
	if (fake->end_error != 0) {
		errno = fake->end_error;
		return -1;
	}
	return 0;
}


static void
fake_nfs_transaction_init(struct fake_nfs_nvram_transaction *fake)
{
	static const char *const names[12] = {
		"lan_ipaddr_t", "lan_netmask_t", "lan_ipaddr",
		"lan_netmask", "ip6_lan_addr", "ip6_lan_size",
		"sw_mode", "vpns_enable", "vpns_vuse", "vpns_vnet",
		"vpns_type", "vpns_ov_mode"
	};
	static const char *const values[12] = {
		"192.168.1.1", "255.255.255.0", "10.0.0.1",
		"255.0.0.0", "2001:db8::1", "64", "1",
		"1", "1", "10.8.0.1", "2", "1"
	};

	memset(fake, 0, sizeof(*fake));
	memcpy(fake->names, names, sizeof(names));
	memcpy(fake->values, values, sizeof(values));
	fake->count = 12;
	fake->read_fail_at = -1;
	fake->missing_at = -1;
}


static void
test_nfs_acl_inputs_are_copied_from_one_transaction(void)
{
	struct fake_nfs_nvram_transaction fake;
	struct storage_nfs_export_acl_snapshot snapshot;
	int parsed_value;

	fake_nfs_transaction_init(&fake);
	assert(storage_nfs_export_read_acl_snapshot(
		&snapshot, fake_nfs_transaction_begin, fake_nfs_transaction_get,
		fake_nfs_transaction_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.calls == fake.count &&
	       fake.end_calls == 1);
	assert(strcmp(snapshot.lan_address, "192.168.1.1") == 0);
	assert(strcmp(snapshot.lan_netmask, "255.255.255.0") == 0);
	assert(strcmp(snapshot.fallback_address, "10.0.0.1") == 0);
	assert(strcmp(snapshot.fallback_netmask, "255.0.0.0") == 0);
	assert(strcmp(snapshot.ipv6_address, "2001:db8::1") == 0);
	assert(strcmp(snapshot.ipv6_prefix, "64") == 0);
	assert(strcmp(snapshot.router_mode, "1") == 0);
	assert(strcmp(snapshot.vpn_enabled, "1") == 0);
	assert(strcmp(snapshot.vpn_use, "1") == 0);
	assert(strcmp(snapshot.vpn_address, "10.8.0.1") == 0);
	assert(strcmp(snapshot.vpn_type, "2") == 0);
	assert(strcmp(snapshot.vpn_openvpn_mode, "1") == 0);
	assert(rc_wan_nvram_parse_int_value(
		snapshot.vpn_enabled, &parsed_value) == 0 && parsed_value == 1);
	assert(rc_wan_nvram_parse_int_value(
		snapshot.vpn_use, &parsed_value) == 0 && parsed_value == 1);

	/* A genuinely absent key retains the legacy empty-string/zero default. */
	fake_nfs_transaction_init(&fake);
	fake.missing_at = 0;
	assert(storage_nfs_export_read_acl_snapshot(
		&snapshot, fake_nfs_transaction_begin, fake_nfs_transaction_get,
		fake_nfs_transaction_end, &fake) == 0);
	assert(snapshot.lan_address[0] == '\0' &&
	       strcmp(snapshot.lan_netmask, "255.255.255.0") == 0);
}


static void
test_nfs_acl_snapshot_discards_all_fields_on_read_or_close_error(void)
{
	struct fake_nfs_nvram_transaction fake;
	struct storage_nfs_export_acl_snapshot snapshot;

	fake_nfs_transaction_init(&fake);
	fake.read_fail_at = 5;
	fake.read_error = EACCES;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(storage_nfs_export_read_acl_snapshot(
		&snapshot, fake_nfs_transaction_begin, fake_nfs_transaction_get,
		fake_nfs_transaction_end, &fake) == EACCES);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
	assert(snapshot.lan_address[0] == '\0' &&
	       snapshot.lan_netmask[0] == '\0' &&
	       snapshot.fallback_address[0] == '\0' &&
	       snapshot.fallback_netmask[0] == '\0' &&
	       snapshot.ipv6_address[0] == '\0' &&
	       snapshot.ipv6_prefix[0] == '\0' &&
	       snapshot.router_mode[0] == '\0' &&
	       snapshot.vpn_enabled[0] == '\0' &&
	       snapshot.vpn_use[0] == '\0' &&
	       snapshot.vpn_address[0] == '\0' &&
	       snapshot.vpn_type[0] == '\0' &&
	       snapshot.vpn_openvpn_mode[0] == '\0');

	fake_nfs_transaction_init(&fake);
	fake.end_error = EIO;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(storage_nfs_export_read_acl_snapshot(
		&snapshot, fake_nfs_transaction_begin, fake_nfs_transaction_get,
		fake_nfs_transaction_end, &fake) == EIO);
	assert(fake.begin_calls == 1 && fake.calls == fake.count &&
	       fake.end_calls == 1);
	assert(snapshot.lan_address[0] == '\0' &&
	       snapshot.lan_netmask[0] == '\0' &&
	       snapshot.fallback_address[0] == '\0' &&
	       snapshot.fallback_netmask[0] == '\0' &&
	       snapshot.ipv6_address[0] == '\0' &&
	       snapshot.ipv6_prefix[0] == '\0' &&
	       snapshot.router_mode[0] == '\0' &&
	       snapshot.vpn_enabled[0] == '\0' &&
	       snapshot.vpn_use[0] == '\0' &&
	       snapshot.vpn_address[0] == '\0' &&
	       snapshot.vpn_type[0] == '\0' &&
	       snapshot.vpn_openvpn_mode[0] == '\0');
}


int
main(void)
{
	char device[32], mountpoint[256], fstype[32], options[256];
	char ipv4_acl[32], ipv4_small[8];
	char long_line[700];
#if defined (USE_IPV6)
	char ipv6_acl[INET6_ADDRSTRLEN + 4];
#endif

	test_nfs_acl_inputs_are_copied_from_one_transaction();
	test_nfs_acl_snapshot_discards_all_fields_on_read_or_close_error();

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

	/* Missing-path probes are fallback errors; the first actionable error wins. */
	assert(storage_nfs_export_errno_or_eio(EACCES) == EACCES);
	assert(storage_nfs_export_errno_or_eio(ENOENT) == ENOENT);
	assert(storage_nfs_export_errno_or_eio(0) == EIO);
	assert(storage_nfs_export_first_error(0, EACCES) == EACCES);
	assert(storage_nfs_export_first_error(EACCES, EINVAL) == EACCES);
	assert(storage_nfs_export_first_error(ENOENT, EACCES) == EACCES);
	assert(storage_nfs_export_first_error(EINVAL, ENOENT) == EINVAL);
	assert(storage_nfs_export_first_error(0, ENOENT) == ENOENT);

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

	test_acl_nvram_copies_survive_shared_cache_reuse();
	test_acl_nvram_copy_fails_closed_on_missing_error_and_truncation();

	/* Checked NVRAM keeps a missing key disabled but never masks a backend error. */
	{
		int enabled = 77;

		assert(storage_nfs_enabled_status(1, 0, "1", &enabled) == 0);
		assert(enabled == 1);
		assert(storage_nfs_enabled_status(1, 0, "0", &enabled) == 0);
		assert(enabled == 0);
		assert(storage_nfs_enabled_status(1, 0, "", &enabled) == 0);
		assert(enabled == 0);
		assert(storage_nfs_enabled_status(0, 0, NULL, &enabled) == 0);
		assert(enabled == 0);

		enabled = 77;
		assert(storage_nfs_enabled_status(-1, EACCES, NULL, &enabled) == EACCES);
		assert(enabled == 77);
		assert(storage_nfs_enabled_status(-1, 0, NULL, &enabled) == EIO);
		assert(enabled == 77);
		assert(storage_nfs_enabled_status(0, 0, "1", &enabled) == EIO);
		assert(enabled == 77);
		assert(storage_nfs_enabled_status(1, 0, NULL, &enabled) == EIO);
		assert(enabled == 77);
		assert(storage_nfs_enabled_status(2, 0, "1", &enabled) == EIO);
		assert(enabled == 77);
		assert(storage_nfs_enabled_status(1, 0, "1", NULL) == EINVAL);
	}

	/* RC callers must observe export-generation and daemon-start failures. */
	assert(storage_nfs_start_status(0, 0) == 0);
	assert(storage_nfs_start_status(-1, 0) == -1);
	assert(storage_nfs_start_status(0, 17) == -1);
	assert(storage_nfs_start_status(-1, 17) == -1);

	puts("Storage NFS override, source-port policy, inventory, IPv6 ACL and start gate: PASS");
	return 0;
}
