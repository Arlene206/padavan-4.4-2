#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include "dnsmasq_static_host_policy.h"

static void
test_valid_unicast_mac_text(void)
{
	assert(dnsmasq_static_mac_is_valid("001122334455"));
	assert(dnsmasq_static_mac_is_valid("021122334455"));
	assert(dnsmasq_static_mac_is_valid("aAbBcCdDeEfF"));
	assert(dnsmasq_static_mac_is_valid("fe1122334455"));
}

static void
test_rejects_non_unicast_addresses(void)
{
	assert(!dnsmasq_static_mac_is_valid("000000000000"));
	assert(!dnsmasq_static_mac_is_valid("011122334455"));
	assert(!dnsmasq_static_mac_is_valid("ff1122334455"));
}

static void
test_rejects_injection_characters(void)
{
	assert(!dnsmasq_static_mac_is_valid("00112233,455"));
	assert(!dnsmasq_static_mac_is_valid("00112233\n455"));
	assert(!dnsmasq_static_mac_is_valid("00112233 455"));
	assert(!dnsmasq_static_mac_is_valid("00112233/455"));
	assert(!dnsmasq_static_mac_is_valid("00112233:455"));
	assert(!dnsmasq_static_mac_is_valid("00112233;455"));
	assert(!dnsmasq_static_mac_is_valid("00112233$455"));
}

static void
test_rejects_wrong_lengths(void)
{
	assert(!dnsmasq_static_mac_is_valid(NULL));
	assert(!dnsmasq_static_mac_is_valid(""));
	assert(!dnsmasq_static_mac_is_valid("00112233445"));
	assert(!dnsmasq_static_mac_is_valid("0011223344556"));
	assert(!dnsmasq_static_mac_is_valid("00112233445g"));
}


static void
test_static_arp_command_status(void)
{
	assert(dnsmasq_static_arp_command_errno(0, 0) == 0);
	assert(dnsmasq_static_arp_command_errno(9 << 8, 0) == EIO);
	assert(dnsmasq_static_arp_command_errno(-1, ENOENT) == ENOENT);
	assert(dnsmasq_static_arp_command_errno(-1, 0) == EIO);
	assert(dnsmasq_static_arp_command_errno(SIGTERM, 0) == EIO);
}

int
main(void)
{
	test_valid_unicast_mac_text();
	test_rejects_non_unicast_addresses();
	test_rejects_injection_characters();
	test_rejects_wrong_lengths();
	test_static_arp_command_status();
	puts("DNSMasq static-host MAC and ARP command policies: PASS");
	return 0;
}
