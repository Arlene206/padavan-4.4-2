#ifndef _HTTPD_NETDEV_POLICY_H_
#define _HTTPD_NETDEV_POLICY_H_

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#define HTTPD_NETDEV_MAX_ENTRIES 64

typedef struct {
	char ifdesc[64];
	int ifindex;
	uint64_t rx;
	uint64_t tx;
} httpd_netdev_entry_t;

static inline int
httpd_netdev_status_from_errno(int saved_errno)
{
	return saved_errno > 0 ? saved_errno : EIO;
}

static inline int
httpd_netdev_is_space(unsigned char ch)
{
	return ch == ' ' || ch == '\t' || ch == '\n' ||
	       ch == '\r' || ch == '\f' || ch == '\v';
}

/*
 * /proc/net/dev has sixteen decimal counters per interface. Parse all sixteen
 * before publishing RX/TX so a truncated or malformed row cannot masquerade
 * as a valid zero or a partial traffic sample.
 */
static inline int
httpd_netdev_parse_counters(const char *text, uint64_t *rx, uint64_t *tx)
{
	unsigned long long fields[16];
	const char *cursor;
	char *end;
	unsigned int i;

	if (!text || !rx || !tx)
		return EINVAL;

	cursor = text;
	for (i = 0; i < 16; ++i) {
		while (httpd_netdev_is_space((unsigned char)*cursor))
			++cursor;
		if (*cursor < '0' || *cursor > '9')
			return EPROTO;

		errno = 0;
		fields[i] = strtoull(cursor, &end, 10);
		if (end == cursor)
			return EPROTO;
		if (errno == ERANGE)
			return ERANGE;
		if (*end && !httpd_netdev_is_space((unsigned char)*end))
			return EPROTO;
		cursor = end;
	}
	while (httpd_netdev_is_space((unsigned char)*cursor))
		++cursor;
	if (*cursor != '\0')
		return EPROTO;

	*rx = (uint64_t)fields[0];
	*tx = (uint64_t)fields[8];
	return 0;
}

#endif
