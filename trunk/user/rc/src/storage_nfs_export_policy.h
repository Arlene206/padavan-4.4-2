#ifndef PADAVAN_STORAGE_NFS_EXPORT_POLICY_H
#define PADAVAN_STORAGE_NFS_EXPORT_POLICY_H

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "wan_nvram_status_policy.h"

struct storage_nfs_export_acl_snapshot {
	char lan_address[32];
	char lan_netmask[32];
	char fallback_address[32];
	char fallback_netmask[32];
	char ipv6_address[INET6_ADDRSTRLEN + 5];
	char ipv6_prefix[8];
	char router_mode[16];
	char vpn_enabled[16];
	char vpn_use[16];
	char vpn_type[16];
	char vpn_openvpn_mode[16];
	char vpn_address[32];
};

/*
 * The ACL writer consumes related NVRAM values as one authorization snapshot.
 * LAN address/netmask, IPv6 address/prefix and VPN enable/type/address must
 * never be combined across concurrent configuration generations.
 */
static inline int
storage_nfs_export_read_acl_snapshot(
	struct storage_nfs_export_acl_snapshot *snapshot,
	rc_wan_nvram_transaction_fn begin_transaction,
	rc_wan_nvram_get_fn get_value,
	rc_wan_nvram_transaction_fn end_transaction, void *context)
{
	struct rc_wan_nvram_snapshot_field fields[12];

	if (!snapshot)
		return EINVAL;
	memset(snapshot, 0, sizeof(*snapshot));

	fields[0].param_name = "lan_ipaddr_t";
	fields[0].destination = snapshot->lan_address;
	fields[0].destination_size = sizeof(snapshot->lan_address);
	fields[1].param_name = "lan_netmask_t";
	fields[1].destination = snapshot->lan_netmask;
	fields[1].destination_size = sizeof(snapshot->lan_netmask);
	fields[2].param_name = "lan_ipaddr";
	fields[2].destination = snapshot->fallback_address;
	fields[2].destination_size = sizeof(snapshot->fallback_address);
	fields[3].param_name = "lan_netmask";
	fields[3].destination = snapshot->fallback_netmask;
	fields[3].destination_size = sizeof(snapshot->fallback_netmask);
	fields[4].param_name = "ip6_lan_addr";
	fields[4].destination = snapshot->ipv6_address;
	fields[4].destination_size = sizeof(snapshot->ipv6_address);
	fields[5].param_name = "ip6_lan_size";
	fields[5].destination = snapshot->ipv6_prefix;
	fields[5].destination_size = sizeof(snapshot->ipv6_prefix);
	fields[6].param_name = "sw_mode";
	fields[6].destination = snapshot->router_mode;
	fields[6].destination_size = sizeof(snapshot->router_mode);
	fields[7].param_name = "vpns_enable";
	fields[7].destination = snapshot->vpn_enabled;
	fields[7].destination_size = sizeof(snapshot->vpn_enabled);
	fields[8].param_name = "vpns_vuse";
	fields[8].destination = snapshot->vpn_use;
	fields[8].destination_size = sizeof(snapshot->vpn_use);
	fields[9].param_name = "vpns_vnet";
	fields[9].destination = snapshot->vpn_address;
	fields[9].destination_size = sizeof(snapshot->vpn_address);
	fields[10].param_name = "vpns_type";
	fields[10].destination = snapshot->vpn_type;
	fields[10].destination_size = sizeof(snapshot->vpn_type);
	fields[11].param_name = "vpns_ov_mode";
	fields[11].destination = snapshot->vpn_openvpn_mode;
	fields[11].destination_size = sizeof(snapshot->vpn_openvpn_mode);

	return rc_wan_nvram_copy_snapshot(fields,
		(unsigned int)(sizeof(fields) / sizeof(fields[0])),
		begin_transaction, get_value, end_transaction, context);
}

/*
 * AUTH_SYS takes numeric UID/GID values from each client request. Keep the
 * historical no_root_squash mapping for compatibility, but require the
 * reserved client source-port check: insecure disables it and lets an
 * unprivileged process on an admitted network claim UID 0. Root-squash is a
 * separate policy decision because it changes existing root-owned file access.
 */
#define STORAGE_NFS_EXPORT_DEFAULT_OPTIONS \
	"async,secure,no_root_squash,no_subtree_check"

/*
 * Classify the operator's custom exports path.
 * Return 0 only when lstat reports ENOENT, 1 for a regular file (direct or
 * symlink target), and -1 for every present-but-invalid or uncertain state.
 */
static int
storage_nfs_export_override_state(int lstat_status, int lstat_errno,
				  int entry_is_regular, int entry_is_symlink,
				  int stat_status, int target_is_regular)
{
	if (lstat_status != 0)
		return lstat_errno == ENOENT ? 0 : -1;
	if (entry_is_regular)
		return 1;
	if (entry_is_symlink && stat_status == 0 && target_is_regular)
		return 1;
	return -1;
}

/*
 * Build a canonical IPv4 client network for an NFS export. The legacy
 * ip2class() helper accepts unchecked inet_addr() results and counts every
 * set bit in the netmask, so a non-contiguous mask can become a different,
 * unintended CIDR prefix. Reject malformed/unspecified/broadcast addresses,
 * non-contiguous masks and /0 (an unrestricted export).
 *
 * Return 1 for a valid ACL and -1 for invalid or truncated input.
 */
static int
storage_nfs_export_format_ipv4_acl(const char *address, const char *netmask,
				   char *acl, size_t acl_size)
{
	struct in_addr parsed_address, parsed_mask, network;
	unsigned int address_value, mask_value, host_mask, prefix_length = 0;
	unsigned int bit = 0x80000000U;
	char address_text[INET_ADDRSTRLEN];
	int length;

	if (!acl || acl_size == 0)
		return -1;
	acl[0] = '\0';

	if (!address || !address[0] || !netmask || !netmask[0])
		return -1;
	if (inet_pton(AF_INET, address, &parsed_address) != 1 ||
	    inet_pton(AF_INET, netmask, &parsed_mask) != 1)
		return -1;

	address_value = (unsigned int)ntohl(parsed_address.s_addr);
	if (address_value == 0U || address_value == 0xffffffffU)
		return -1;

	mask_value = (unsigned int)ntohl(parsed_mask.s_addr);
	host_mask = ~mask_value;
	if ((host_mask & (host_mask + 1U)) != 0U)
		return -1;

	while (bit && (mask_value & bit)) {
		prefix_length++;
		bit >>= 1;
	}
	if (prefix_length == 0U)
		return -1;

	network.s_addr = parsed_address.s_addr & parsed_mask.s_addr;
	if (!inet_ntop(AF_INET, &network, address_text, sizeof(address_text)))
		return -1;

	length = snprintf(acl, acl_size, "%s/%u", address_text, prefix_length);
	if (length < 0 || (size_t)length >= acl_size) {
		acl[0] = '\0';
		return -1;
	}

	return 1;
}

#if defined (USE_IPV6)

/* Parse a decimal value without accepting signs, whitespace or overflow. */
static int
storage_nfs_export_parse_decimal_range(const char *value,
					unsigned int maximum,
					unsigned int *parsed)
{
	const unsigned char *p;
	unsigned int number = 0;

	if (!value || !value[0] || !parsed)
		return -1;

	for (p = (const unsigned char *)value; *p; ++p) {
		unsigned int digit;

		if (*p < '0' || *p > '9')
			return -1;
		digit = (unsigned int)(*p - '0');
		if (digit > maximum || number > (maximum - digit) / 10U)
			return -1;
		number = number * 10U + digit;
	}
	if (number > maximum)
		return -1;

	*parsed = number;
	return 0;
}

/*
 * Build a canonical NFS IPv6 client prefix from NVRAM. The configured
 * address may carry an optional CIDR suffix (accepted by ipv6_from_string),
 * but the separately stored LAN prefix length is authoritative. Only the
 * existing LAN policy range /48 through /64 is exported. Empty address means
 * that no IPv6 ACL is configured; malformed non-empty state fails closed.
 *
 * Return 1 for an ACL, 0 when IPv6 is unconfigured, and -1 on invalid input.
 */
static int
storage_nfs_export_format_ipv6_acl(const char *address, const char *prefix,
				   char *acl, size_t acl_size)
{
	struct in6_addr parsed_address;
	char address_text[INET6_ADDRSTRLEN];
	const char *slash;
	size_t address_length, acl_length;
	unsigned int embedded_prefix = 0, prefix_length;

	if (!acl || acl_size < INET6_ADDRSTRLEN + 4U)
		return -1;
	acl[0] = '\0';

	if (!address || !address[0])
		return 0;

	slash = strchr(address, '/');
	address_length = slash ? (size_t)(slash - address) : strlen(address);
	if (address_length == 0 || address_length >= sizeof(address_text))
		return -1;
	memcpy(address_text, address, address_length);
	address_text[address_length] = '\0';

	/* Match ipv6_from_string()'s optional prefix syntax without trusting it. */
	if (slash && storage_nfs_export_parse_decimal_range(
		    slash + 1, 128U, &embedded_prefix) != 0)
		return -1;

	if (inet_pton(AF_INET6, address_text, &parsed_address) != 1)
		return -1;
	if (storage_nfs_export_parse_decimal_range(prefix, 64U,
						   &prefix_length) != 0 ||
	    prefix_length < 48U)
		return -1;

	if (!inet_ntop(AF_INET6, &parsed_address, acl, (socklen_t)acl_size))
		return -1;
	acl_length = strlen(acl);
	if (acl_length + 4U > acl_size) {
		acl[0] = '\0';
		return -1;
	}

	acl[acl_length++] = '/';
	acl[acl_length++] = (char)('0' + prefix_length / 10U);
	acl[acl_length++] = (char)('0' + prefix_length % 10U);
	acl[acl_length] = '\0';

	return 1;
}

#endif /* USE_IPV6 */

static int
storage_nfs_export_line_is_complete(const char *line)
{
	size_t length;

	if (!line)
		return 0;

	length = strlen(line);
	return (length > 0 && line[length - 1] == '\n');
}

/* Return 1 for a token, 0 at end of line, and -1 for an invalid token. */
static int
storage_nfs_export_next_token(const char **cursor, char *token,
			      size_t token_size)
{
	const char *start;
	const char *p;
	size_t length;

	if (!cursor || !*cursor || !token || token_size == 0)
		return -1;

	p = *cursor;
	while (*p == ' ' || *p == '\t' || *p == '\r')
		p++;

	if (*p == '\0' || *p == '\n') {
		*cursor = p;
		return 0;
	}

	start = p;
	while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
		p++;

	length = (size_t)(p - start);
	if (length == 0 || length >= token_size)
		return -1;

	memcpy(token, start, length);
	token[length] = '\0';
	*cursor = p;
	return 1;
}

static int
storage_nfs_export_field_is_decimal(const char *field)
{
	const unsigned char *p;

	if (!field || !field[0])
		return 0;

	for (p = (const unsigned char *)field; *p; ++p) {
		if (*p < '0' || *p > '9')
			return 0;
	}

	return 1;
}

/*
 * Parse a complete six-field /proc/mounts record. Only the first four values
 * are returned to the export writer, but the trailing dump/pass fields and
 * end-of-record must also be present so a partial inventory is never treated
 * as a valid mount entry.
 */
static int
storage_nfs_export_parse_mount_line(const char *line,
				    char *device, size_t device_size,
				    char *mountpoint, size_t mountpoint_size,
				    char *fstype, size_t fstype_size,
				    char *mount_options, size_t mount_options_size)
{
	const char *cursor;
	char dump_frequency[16], fsck_order[16], trailing[2];

	if (!storage_nfs_export_line_is_complete(line))
		return -1;

	cursor = line;
	if (storage_nfs_export_next_token(&cursor, device, device_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, mountpoint, mountpoint_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, fstype, fstype_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, mount_options,
					  mount_options_size) != 1 ||
	    storage_nfs_export_next_token(&cursor, dump_frequency,
					  sizeof(dump_frequency)) != 1 ||
	    storage_nfs_export_next_token(&cursor, fsck_order,
					  sizeof(fsck_order)) != 1)
		return -1;

	if (!storage_nfs_export_field_is_decimal(dump_frequency) ||
	    !storage_nfs_export_field_is_decimal(fsck_order))
		return -1;

	/* No seventh field: accepting one would hide a malformed procfs row. */
	return storage_nfs_export_next_token(&cursor, trailing,
					    sizeof(trailing)) == 0 ? 0 : -1;
}

static int
storage_nfs_export_inventory_complete(int read_error, int close_error,
				      int parse_error)
{
	return !read_error && !close_error && !parse_error;
}

static int
storage_nfs_export_may_publish(int enabled, int read_error, int close_error,
			       int parse_error)
{
	return enabled && storage_nfs_export_inventory_complete(read_error,
								close_error,
								parse_error);
}

typedef int (*storage_nfs_export_nvram_get_fn)(void *context,
						 const char *param_name,
						 char **value);

/* nvram_get_checked() exposes cache-backed storage; always retain an owned copy. */
static inline int
storage_nfs_export_copy_nvram_value(
	storage_nfs_export_nvram_get_fn get_value, void *context,
	const char *param_name, char *destination, size_t destination_size)
{
	char *source = NULL;
	size_t length;
	int api_result, saved_errno;

	if (!get_value || !param_name || !param_name[0] ||
	    !destination || destination_size == 0)
		return EINVAL;
	destination[0] = '\0';
	errno = 0;
	api_result = get_value(context, param_name, &source);
	saved_errno = errno;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0)
		return source ? EIO : 0;
	if (api_result != 1 || !source)
		return EIO;
	length = strlen(source);
	if (length >= destination_size)
		return ENAMETOOLONG;
	memcpy(destination, source, length + 1);
	return 0;
}


/*
 * VPN export selectors are small enumerations from the web UI, not arbitrary
 * integers. Reject signs, whitespace, prefixes and out-of-range values so a
 * corrupt "enabled" selector cannot grant a wider NFS client ACL. An absent
 * NVRAM key retains the legacy zero default.
 */
static int
storage_nfs_export_parse_selector(const char *text, unsigned int maximum,
                                  int *value)
{
	if (!text || !value || maximum > 9U)
		return EINVAL;
	*value = 0;
	if (!text[0])
		return 0;
	if (text[1] != '\0' || text[0] < '0' || text[0] > '9' ||
	    (unsigned int)(text[0] - '0') > maximum)
		return EINVAL;
	*value = text[0] - '0';
	return 0;
}


static int
storage_nfs_enabled_status(int api_result, int saved_errno,
			   const char *value, int *enabled)
{
	if (!enabled)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*enabled = 0;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*enabled = strcmp(value, "1") == 0;
	return 0;
}

static int
storage_nfs_export_may_start(int enabled, int export_status)
{
	return enabled && export_status == 0;
}

static int
storage_nfs_start_status(int export_status, int command_status)
{
	return (export_status == 0 && command_status == 0) ? 0 : -1;
}

/*
 * Keep the first actionable filesystem/inventory error. ENOENT is a common
 * candidate-probe result while scanning mounts, so a later useful error
 * takes precedence over an earlier missing-path result.
 */
static int
storage_nfs_export_first_error(int first_error, int next_error)
{
	if (first_error > 0 && first_error != ENOENT)
		return first_error;
	if (next_error > 0 && next_error != ENOENT)
		return next_error;
	if (first_error > 0)
		return first_error;
	if (next_error > 0)
		return next_error;
	return EIO;
}

static int
storage_nfs_export_errno_or_eio(int error)
{
	return error > 0 ? error : EIO;
}

#endif /* PADAVAN_STORAGE_NFS_EXPORT_POLICY_H */
