#ifndef MINIUPNPD_PINHOLE_POLICY_H
#define MINIUPNPD_PINHOLE_POLICY_H

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>

typedef int (*miniupnpd_pinhole_uid_used_fn)(unsigned short uid,
                                            void *context);

/* Find a free non-zero 16-bit UID without an unbounded wraparound loop. */
static inline int
miniupnpd_pinhole_find_free_uid(unsigned int first_uid,
                                miniupnpd_pinhole_uid_used_fn is_used,
                                void *context, unsigned short *uid_out)
{
	unsigned int candidate;
	unsigned int attempts;

	if (!is_used || !uid_out) {
		errno = EINVAL;
		return -1;
	}
	candidate = first_uid >= 1 && first_uid <= 65535 ? first_uid : 1;
	for (attempts = 0; attempts < 65535; ++attempts) {
		if (!is_used((unsigned short)candidate, context)) {
			*uid_out = (unsigned short)candidate;
			return 0;
		}
		++candidate;
		if (candidate > 65535)
			candidate = 1;
	}
	errno = ENOSPC;
	return -1;
}

/* NULL, empty and '*' mean any remote IPv6 address (the ::/0 mask). */
static inline int
miniupnpd_pinhole_parse_remote_ipv6(const char *text,
                                    struct in6_addr *address_out,
                                    int *wildcard_out)
{
	struct in6_addr parsed;
	int wildcard;

	if (!address_out || !wildcard_out) {
		errno = EINVAL;
		return -1;
	}
	if (!text || text[0] == '\0' || strcmp(text, "*") == 0) {
		memset(&parsed, 0, sizeof(parsed));
		wildcard = 1;
	} else {
		if (inet_pton(AF_INET6, text, &parsed) != 1) {
			errno = EINVAL;
			return -1;
		}
		wildcard = 0;
	}
	*address_out = parsed;
	*wildcard_out = wildcard;
	return 0;
}

/* The internal destination must always be a literal IPv6 address. */
static inline int
miniupnpd_pinhole_parse_internal_ipv6(const char *text,
                                      struct in6_addr *address_out)
{
	struct in6_addr parsed;

	if (!text || !address_out || inet_pton(AF_INET6, text, &parsed) != 1) {
		errno = EINVAL;
		return -1;
	}
	*address_out = parsed;
	return 0;
}

/* A registry entry may outlive its firewall rule; absent counters are zero. */
static inline void
miniupnpd_pinhole_reset_counters(u_int64_t *packets, u_int64_t *bytes)
{
	if (packets)
		*packets = 0;
	if (bytes)
		*bytes = 0;
}

#endif /* MINIUPNPD_PINHOLE_POLICY_H */
