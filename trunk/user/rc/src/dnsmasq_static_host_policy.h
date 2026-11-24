#ifndef PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H

#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>


/* /proc/net/arp exposes six whitespace-delimited columns. Validate every
 * column rather than allowing a truncated row to disappear from cleanup. */
static inline int
dnsmasq_static_arp_ipv4_is_valid(const char *address)
{
	unsigned int octet = 0;
	unsigned int digits = 0;
	unsigned int separators = 0;
	const unsigned char *p = (const unsigned char *)address;

	if (!address || !*address)
		return 0;
	for (; *p; p++) {
		if (*p >= '0' && *p <= '9') {
			if (++digits > 3)
				return 0;
			octet = octet * 10U + (*p - '0');
			if (octet > 255U)
				return 0;
		} else if (*p == '.' && digits > 0 && separators < 3) {
			separators++;
			digits = 0;
			octet = 0;
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
	unsigned int ignored_hardware_type, parsed_flags;
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
					      &ignored_hardware_type);
	if (status != 0)
		return status;
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
		if (!ips[i] || !ips[i][0] || !macs[i] || !macs[i][0])
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

#endif /* PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H */
