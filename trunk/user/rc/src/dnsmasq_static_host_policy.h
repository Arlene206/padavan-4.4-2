#ifndef PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H
#define PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H

#include <stddef.h>
#include <errno.h>
#include <sys/wait.h>

/* A dnsmasq --dhcp-host MAC field must contain exactly twelve hex digits. */
static int
dnsmasq_static_mac_is_valid(const char *mac)
{
	size_t i;
	unsigned int first_octet = 0;
	unsigned int digit;
	int nonzero = 0;

	if (!mac)
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
	if (mac[12] != '\0')
		return 0;

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

#endif /* PADAVAN_DNSMASQ_STATIC_HOST_POLICY_H */
