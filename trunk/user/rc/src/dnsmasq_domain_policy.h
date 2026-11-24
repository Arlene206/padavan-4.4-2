#ifndef PADAVAN_DNSMASQ_DOMAIN_POLICY_H
#define PADAVAN_DNSMASQ_DOMAIN_POLICY_H

#include <stddef.h>
#include <string.h>

/* Keep DHCP-derived domain names inside a single resolver/hosts field.
 * The option is network-controlled; control bytes, slashes and whitespace
 * must not be able to append records or directives. */
static int
dnsmasq_valid_server_domain(const char *domain)
{
	size_t length, label_len, i;
	unsigned char c;

	if (!domain)
		return 0;
	length = strlen(domain);
	if (length == 0 || length > 254)
		return 0;
	/* A single final dot is the conventional absolute DNS-name form. */
	if (domain[length - 1] == '.')
		length--;
	if (length == 0 || length > 253)
		return 0;

	label_len = 0;
	for (i = 0; i < length; i++) {
		c = (unsigned char)domain[i];
		if (c == '.') {
			if (label_len == 0 || domain[i - 1] == '-')
				return 0;
			label_len = 0;
			continue;
		}
		if (!((c >= 'a' && c <= 'z') ||
		      (c >= 'A' && c <= 'Z') ||
		      (c >= '0' && c <= '9') || c == '-'))
			return 0;
		if (label_len == 0 && c == '-')
			return 0;
		label_len++;
		if (label_len > 63)
			return 0;
	}

	if (label_len == 0 || domain[length - 1] == '-')
		return 0;

	return 1;
}

#endif /* PADAVAN_DNSMASQ_DOMAIN_POLICY_H */
