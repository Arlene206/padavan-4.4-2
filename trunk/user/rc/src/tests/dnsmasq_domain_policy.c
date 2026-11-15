#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dnsmasq_domain_policy.h"

static void
test_valid_dns_domains(void)
{
	assert(dnsmasq_valid_server_domain("router"));
	assert(dnsmasq_valid_server_domain("router.lan"));
	assert(dnsmasq_valid_server_domain("home.example."));
	assert(dnsmasq_valid_server_domain("a-1.sub-domain.example"));
}

static void
test_rejects_directive_injection(void)
{
	assert(!dnsmasq_valid_server_domain(NULL));
	assert(!dnsmasq_valid_server_domain(""));
	assert(!dnsmasq_valid_server_domain("bad name"));
	assert(!dnsmasq_valid_server_domain("bad\nserver=8.8.8.8"));
	assert(!dnsmasq_valid_server_domain("bad\rserver=8.8.8.8"));
	assert(!dnsmasq_valid_server_domain("bad,server=8.8.8.8"));
	assert(!dnsmasq_valid_server_domain("bad/server"));
	assert(!dnsmasq_valid_server_domain("bad\\server"));
	assert(!dnsmasq_valid_server_domain("bad;server=8.8.8.8"));
}

static void
test_rejects_invalid_label_lengths(void)
{
	char label[65];
	char domain[256];

	memset(label, 'a', sizeof(label) - 1);
	label[sizeof(label) - 1] = '\0';
	assert(!dnsmasq_valid_server_domain(label));
	assert(!dnsmasq_valid_server_domain("-leading.example"));
	assert(!dnsmasq_valid_server_domain("trailing-.example"));
	assert(!dnsmasq_valid_server_domain("empty..label"));
	assert(!dnsmasq_valid_server_domain(".leading.example"));
	assert(!dnsmasq_valid_server_domain("trailing.example.."));

	memset(domain, 'a', sizeof(domain) - 1);
	domain[sizeof(domain) - 1] = '\0';
	assert(!dnsmasq_valid_server_domain(domain));
}

int
main(void)
{
	test_valid_dns_domains();
	test_rejects_directive_injection();
	test_rejects_invalid_label_lengths();
	puts("DNSMasq LAN domain validation policy: PASS");
	return 0;
}
