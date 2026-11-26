#ifndef PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H

#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define DNSMASQ_STATIC_ARP_MAX 64

struct dnsmasq_static_arp_binding {
	char ip[16];
	char mac[18];
};

struct dnsmasq_static_arp_snapshot {
	size_t count;
	struct dnsmasq_static_arp_binding entries[DNSMASQ_STATIC_ARP_MAX];
};


/* /proc/net/arp exposes six whitespace-delimited columns. Validate every
 * column rather than allowing a truncated row to disappear from cleanup. */
static inline int
dnsmasq_static_arp_ipv4_is_valid(const char *address)
{
	unsigned int octet = 0;
	unsigned int digits = 0;
	unsigned int separators = 0;
	int octet_leading_zero = 0;
	const unsigned char *p = (const unsigned char *)address;

	if (!address || !*address)
		return 0;
	for (; *p; p++) {
		if (*p >= '0' && *p <= '9') {
			if (digits == 0)
				octet_leading_zero = (*p == '0');
			else if (octet_leading_zero)
				return 0;
			if (++digits > 3)
				return 0;
			octet = octet * 10U + (*p - '0');
			if (octet > 255U)
				return 0;
		} else if (*p == '.' && digits > 0 && separators < 3) {
			separators++;
			digits = 0;
			octet = 0;
			octet_leading_zero = 0;
		} else {
			return 0;
		}
	}
	return separators == 3 && digits > 0;
}

static inline int
dnsmasq_static_arp_mac_is_valid(const char *address)
{
	size_t i;

	if (!address || strlen(address) != 17)
		return 0;
	for (i = 0; i < 17; i++) {
		unsigned char value = (unsigned char)address[i];

		if (i % 3 == 2) {
			if (value != ':')
				return 0;
		} else if (!((value >= '0' && value <= '9') ||
			    (value >= 'a' && value <= 'f') ||
			    (value >= 'A' && value <= 'F'))) {
			return 0;
		}
	}
	return 1;
}

/*
 * Parse one generated /etc/ethers record and convert its colon-delimited MAC
 * into the compact 12-hex-digit form accepted by the firmware arp applet.
 */
static inline int
dnsmasq_static_ethers_record_parse(const char *line,
                                   char *ip, size_t ip_size,
                                   char *mac, size_t mac_size)
{
	char parsed_mac[18], parsed_ip[16], compact_mac[13], extra;
	unsigned int first_octet = 0, nonzero = 0;
	size_t i, out = 0, ip_length, mac_length;
	int fields;

	if (!line || !ip || !mac)
		return EINVAL;
	fields = sscanf(line, "%17s %15s %c",
			parsed_mac, parsed_ip, &extra);
	if (fields != 2 ||
	    !dnsmasq_static_arp_ipv4_is_valid(parsed_ip) ||
	    !dnsmasq_static_arp_mac_is_valid(parsed_mac))
		return EIO;

	for (i = 0; i < 17; i++) {
		unsigned char value = (unsigned char)parsed_mac[i];
		unsigned int digit;

		if (i % 3 == 2)
			continue;
		if (value >= '0' && value <= '9')
			digit = value - '0';
		else if (value >= 'a' && value <= 'f')
			digit = value - 'a' + 10U;
		else
			digit = value - 'A' + 10U;
		if (out < 2)
			first_octet = (first_octet << 4) | digit;
		if (digit)
			nonzero = 1;
		if (out >= sizeof(compact_mac) - 1)
			return EIO;
		compact_mac[out++] = parsed_mac[i];
	}
	if (!nonzero || (first_octet & 1U))
		return EIO;
	compact_mac[out] = '\0';

	ip_length = strlen(parsed_ip);
	mac_length = out;
	if (ip_length + 1 > ip_size || mac_length + 1 > mac_size)
		return ENOSPC;
	memcpy(ip, parsed_ip, ip_length + 1);
	memcpy(mac, compact_mac, mac_length + 1);
	return 0;
}

/* Compare validated colon-delimited Ethernet addresses without case bias. */
static inline int
dnsmasq_static_arp_mac_text_equal(const char *left, const char *right)
{
	size_t i;

	if (!dnsmasq_static_arp_mac_is_valid(left) ||
	    !dnsmasq_static_arp_mac_is_valid(right))
		return 0;
	for (i = 0; i < 17; i++) {
		unsigned char a = (unsigned char)left[i];
		unsigned char b = (unsigned char)right[i];

		if (a >= 'A' && a <= 'F')
			a = (unsigned char)(a - 'A' + 'a');
		if (b >= 'A' && b <= 'F')
			b = (unsigned char)(b - 'A' + 'a');
		if (a != b)
			return 0;
	}
	return 1;
}

/* Add one validated /etc/ethers entry to the immutable pre-reconfigure set. */
static inline int
dnsmasq_static_arp_snapshot_add_ethers_record(
	struct dnsmasq_static_arp_snapshot *snapshot, const char *line)
{
	char ip[16], compact_mac[13];
	char *mac;
	size_t i, out = 0;
	int status;

	if (!snapshot)
		return EINVAL;
	if (snapshot->count > DNSMASQ_STATIC_ARP_MAX)
		return EINVAL;
	status = dnsmasq_static_ethers_record_parse(
		line, ip, sizeof(ip), compact_mac, sizeof(compact_mac));
	if (status != 0)
		return status;

	for (i = 0; i < snapshot->count; i++) {
		if (strcmp(snapshot->entries[i].ip, ip) == 0)
			return EEXIST;
	}
	if (snapshot->count >= DNSMASQ_STATIC_ARP_MAX)
		return EOVERFLOW;

	mac = snapshot->entries[snapshot->count].mac;
	for (i = 0; i < 12; i++) {
		if (i != 0 && i % 2 == 0)
			mac[out++] = ':';
		mac[out++] = compact_mac[i];
	}
	mac[out] = '\0';
	memcpy(snapshot->entries[snapshot->count].ip, ip, strlen(ip) + 1);
	snapshot->count++;
	return 0;
}

/* A kernel row is owned only when both its address and Ethernet MAC match. */
static inline int
dnsmasq_static_arp_snapshot_contains(
	const struct dnsmasq_static_arp_snapshot *snapshot,
	const char *ip, const char *mac)
{
	size_t i;

	if (!snapshot || !ip || !mac ||
	    snapshot->count > DNSMASQ_STATIC_ARP_MAX ||
	    !dnsmasq_static_arp_ipv4_is_valid(ip) ||
	    !dnsmasq_static_arp_mac_is_valid(mac))
		return 0;
	for (i = 0; i < snapshot->count; i++) {
		if (strcmp(snapshot->entries[i].ip, ip) == 0 &&
		    dnsmasq_static_arp_mac_text_equal(
			snapshot->entries[i].mac, mac))
			return 1;
	}
	return 0;
}

/* Called after the complete /proc/net/arp row has passed strict parsing. */
static inline int
dnsmasq_static_arp_snapshot_contains_record(
	const struct dnsmasq_static_arp_snapshot *snapshot, const char *line)
{
	char ip[16], hardware_type[8], flags[16], mac[18];

	if (!snapshot || !line ||
	    sscanf(line, "%15s %7s %15s %17s",
		   ip, hardware_type, flags, mac) != 4)
		return 0;
	return dnsmasq_static_arp_snapshot_contains(snapshot, ip, mac);
}

static inline int
dnsmasq_static_arp_hex_value(const char *text, unsigned int *value)
{
	const unsigned char *p;
	char *end;
	unsigned long parsed;

	if (!text || !value || text[0] != '0' ||
	    (text[1] != 'x' && text[1] != 'X') || text[2] == '\0')
		return EIO;
	for (p = (const unsigned char *)text + 2; *p; p++) {
		if (!((*p >= '0' && *p <= '9') ||
		      (*p >= 'a' && *p <= 'f') ||
		      (*p >= 'A' && *p <= 'F')))
			return EIO;
	}
	errno = 0;
	parsed = strtoul(text + 2, &end, 16);
	if (errno == ERANGE || end == text + 2 || *end != '\0' ||
	    parsed > UINT_MAX)
		return EIO;
	*value = (unsigned int)parsed;
	return 0;
}

/*
 * Parse one complete /proc/net/arp record. The caller removes its trailing
 * newline first. A seventh token is rejected; trailing whitespace is fine.
 */

static inline int
dnsmasq_static_arp_header_is_valid(const char *line)
{
	char ip[16], address[16], hw[16], type[16], flags[16];
	char hw_label[16], hw_address[16], mask[16], device[16];
	int consumed = -1;

	if (!line)
		return 0;
	if (sscanf(line, "%15s %15s %15s %15s %15s %15s %15s %15s %15s %n",
		   ip, address, hw, type, flags, hw_label, hw_address,
		   mask, device, &consumed) != 9 || consumed < 0 ||
	    line[consumed] != '\0')
		return 0;
	return strcmp(ip, "IP") == 0 &&
	       strcmp(address, "address") == 0 &&
	       strcmp(hw, "HW") == 0 &&
	       strcmp(type, "type") == 0 &&
	       strcmp(flags, "Flags") == 0 &&
	       strcmp(hw_label, "HW") == 0 &&
	       strcmp(hw_address, "address") == 0 &&
	       strcmp(mask, "Mask") == 0 &&
	       strcmp(device, "Device") == 0;
}

static inline int
dnsmasq_static_arp_record_parse(const char *line,
				char *ip, size_t ip_size,
				unsigned int *flags,
				char *ifname, size_t ifname_size)
{
	char parsed_ip[16], hardware_type[8], flag_text[16];
	char hardware_address[18], mask[16], parsed_ifname[32], extra;
	unsigned int parsed_hardware_type, parsed_flags;
	int fields;
	size_t ip_length, ifname_length;
	int status;

	if (!line || !ip || !flags || !ifname)
		return EINVAL;
	fields = sscanf(line, "%15s %7s %15s %17s %15s %31s %c",
			parsed_ip, hardware_type, flag_text, hardware_address,
			mask, parsed_ifname, &extra);
	if (fields != 6)
		return EIO;
	if (!dnsmasq_static_arp_ipv4_is_valid(parsed_ip) ||
	    !dnsmasq_static_arp_mac_is_valid(hardware_address) ||
	    (strcmp(mask, "*") != 0 &&
	     !dnsmasq_static_arp_ipv4_is_valid(mask)))
		return EIO;
	status = dnsmasq_static_arp_hex_value(hardware_type,
					      &parsed_hardware_type);
	if (status != 0)
		return status;
	/* arpbind_clear() only manages Ethernet entries on the LAN bridge.
	 * Do not interpret an unsupported hardware address format as a MAC. */
	if (parsed_hardware_type != 0x1U)
		return EIO;
	status = dnsmasq_static_arp_hex_value(flag_text, &parsed_flags);
	if (status != 0)
		return status;

	ip_length = strlen(parsed_ip);
	ifname_length = strlen(parsed_ifname);
	if (ip_length + 1 > ip_size || ifname_length + 1 > ifname_size)
		return ENOSPC;
	memcpy(ip, parsed_ip, ip_length + 1);
	memcpy(ifname, parsed_ifname, ifname_length + 1);
	*flags = parsed_flags;
	return 0;
}

/* A dnsmasq --dhcp-host MAC field must contain exactly twelve hex digits. */
static int
dnsmasq_static_mac_is_valid(const char *mac)
{
	size_t i;
	unsigned int first_octet = 0;
	unsigned int digit;
	int nonzero = 0;

	/* Check length before indexing: short NVRAM strings are valid input
	 * to this policy and must not be read past their terminator. */
	if (!mac || strlen(mac) != 12)
		return 0;
	for (i = 0; i < 12; i++) {
		unsigned char value = (unsigned char)mac[i];

		if (value >= '0' && value <= '9')
			digit = value - '0';
		else if (value >= 'a' && value <= 'f')
			digit = value - 'a' + 10U;
		else if (value >= 'A' && value <= 'F')
			digit = value - 'A' + 10U;
		else
			return 0;

		if (i == 0)
			first_octet = digit << 4;
		else if (i == 1)
			first_octet |= digit;
		if (digit != 0)
			nonzero = 1;
	}
	/* DHCP client hardware addresses must be nonzero unicast Ethernet MACs. */
	return nonzero && !(first_octet & 1U);
}


/* system() returns a wait status, not the command's plain exit code. Treat
 * signal termination and nonzero exit as a failed static-ARP configuration. */
static int
dnsmasq_static_arp_command_errno(int status, int command_errno)
{
	if (status == -1)
		return command_errno ? command_errno : EIO;
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 0;
	return EIO;
}


/* Return zero or a positive errno-style error from each command callback. */
typedef int (*dnsmasq_static_arp_command_fn)(const char *ip, const char *mac,
					     int add, void *context);

/*
 * Validate the entire set before side effects. The BusyBox arp applet reports
 * success only after SIOCSARP succeeds; only successful additions are rolled
 * back, so a failed target cannot cause deletion of a pre-existing ARP entry.
 */
static inline int
dnsmasq_static_arp_apply_transaction(const char *const *ips,
				      const char *const *macs, size_t count,
				      dnsmasq_static_arp_command_fn command,
				      void *context)
{
	size_t i, applied = 0;
	int failure = 0;
	int rollback_failure = 0;

	if (count == 0)
		return 0;
	if (!ips || !macs || !command)
		return EINVAL;

	for (i = 0; i < count; i++) {
		/*
		 * Keep this transaction safe for every caller, not only the current
		 * /etc/ethers parser: the backend interpolates values into a command.
		 */
		if (!ips[i] || !ips[i][0] || !macs[i] || !macs[i][0] ||
		    !dnsmasq_static_arp_ipv4_is_valid(ips[i]) ||
		    !dnsmasq_static_mac_is_valid(macs[i]))
			return EINVAL;
	}

	for (i = 0; i < count; i++) {
		failure = command(ips[i], macs[i], 1, context);
		if (failure)
			break;
		applied++;
	}
	if (!failure)
		return 0;
	if (failure < 0)
		failure = EIO;

	while (applied > 0) {
		int status;

		applied--;
		status = command(ips[applied], macs[applied], 0, context);
		if (status && !rollback_failure)
			rollback_failure = status < 0 ? EIO : status;
	}
	return rollback_failure ? rollback_failure : failure;
}

/*
 * Replay a validated /etc/ethers snapshot after a failed DNSMasq replacement.
 * Saved MACs use colon notation; the arp applet requires twelve hex digits.
 */
static inline int
dnsmasq_static_arp_snapshot_apply(
	const struct dnsmasq_static_arp_snapshot *snapshot,
	dnsmasq_static_arp_command_fn command, void *context)
{
	char compact_macs[DNSMASQ_STATIC_ARP_MAX][13];
	const char *ips[DNSMASQ_STATIC_ARP_MAX];
	const char *macs[DNSMASQ_STATIC_ARP_MAX];
	size_t i, j, out;

	if (!snapshot || snapshot->count > DNSMASQ_STATIC_ARP_MAX)
		return EINVAL;
	if (snapshot->count == 0)
		return 0;

	for (i = 0; i < snapshot->count; i++) {
		const char *saved_mac = snapshot->entries[i].mac;

		if (!dnsmasq_static_arp_ipv4_is_valid(snapshot->entries[i].ip) ||
		    !dnsmasq_static_arp_mac_is_valid(saved_mac))
			return EINVAL;
		out = 0;
		for (j = 0; j < 17; j++) {
			if (saved_mac[j] == ':')
				continue;
			if (out >= sizeof(compact_macs[i]) - 1)
				return EINVAL;
			compact_macs[i][out++] = saved_mac[j];
		}
		compact_macs[i][out] = '\0';
		if (!dnsmasq_static_mac_is_valid(compact_macs[i]))
			return EINVAL;
		ips[i] = snapshot->entries[i].ip;
		macs[i] = compact_macs[i];
	}
	return dnsmasq_static_arp_apply_transaction(
		ips, macs, snapshot->count, command, context);
}


#endif /* PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H */
