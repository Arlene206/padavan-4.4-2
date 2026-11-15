#include <assert.h>
#include <stdio.h>
#include "dnsmasq_static_host_policy.h"

static void
test_valid_unicast_mac_text(void)
{
	assert(dnsmasq_static_mac_is_valid("001122334455"));
	assert(dnsmasq_static_mac_is_valid("aAbBcCdDeEfF"));
	assert(dnsmasq_static_mac_is_valid("000000000000"));
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

int
main(void)
{
	test_valid_unicast_mac_text();
	test_rejects_injection_characters();
	test_rejects_wrong_lengths();
	puts("DNSMasq static-host MAC validation policy: PASS");
	return 0;
}
