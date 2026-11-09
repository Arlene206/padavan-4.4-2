#ifndef PADAVAN_STORAGE_NFS_EXPORT_POLICY_H
#define PADAVAN_STORAGE_NFS_EXPORT_POLICY_H

#include <stddef.h>
#include <string.h>

#if defined (USE_IPV6)
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

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

static int
storage_nfs_export_may_start(int enabled, int export_status)
{
	return enabled && export_status == 0;
}

#endif /* PADAVAN_STORAGE_NFS_EXPORT_POLICY_H */
