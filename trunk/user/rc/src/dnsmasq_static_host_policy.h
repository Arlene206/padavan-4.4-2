#ifndef PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H

#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

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
