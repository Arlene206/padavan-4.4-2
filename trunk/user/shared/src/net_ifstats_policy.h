#ifndef NET_IFSTATS_POLICY_H
#define NET_IFSTATS_POLICY_H

#include <errno.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Build a sysfs counter path only from one bounded interface component and a
 * counter name selected by the two public byte-counter wrappers. */
static inline int
net_ifstats_build_path(char *path, size_t path_size,
                       const char *ifname, const char *counter_name)
{
	size_t length;
	int written;

	if (path == NULL || path_size == 0)
		return EINVAL;
	path[0] = '\0';
	if (ifname == NULL || counter_name == NULL)
		return EINVAL;

	for (length = 0; length < IFNAMSIZ && ifname[length] != '\0'; ++length) {
		unsigned char c = (unsigned char)ifname[length];
		if (!((c >= 'a' && c <= 'z') ||
		      (c >= 'A' && c <= 'Z') ||
		      (c >= '0' && c <= '9') ||
		      c == '_' || c == '-' || c == '.'))
			return EINVAL;
	}
	if (length == 0)
		return EINVAL;
	if (length == IFNAMSIZ)
		return ENAMETOOLONG;
	if ((length == 1 && ifname[0] == '.') ||
	    (length == 2 && ifname[0] == '.' && ifname[1] == '.'))
		return EINVAL;

	if (strcmp(counter_name, "rx_bytes") != 0 &&
	    strcmp(counter_name, "tx_bytes") != 0)
		return EINVAL;

	written = snprintf(path, path_size,
			  "/sys/class/net/%s/statistics/%s",
			  ifname, counter_name);
	if (written < 0) {
		path[0] = '\0';
		return EIO;
	}
	if ((size_t)written >= path_size) {
		path[0] = '\0';
		return ENAMETOOLONG;
	}
	return 0;
}

/* Publish a counter pair only when both sysfs reads succeeded. */
static inline int
net_ifstats_pair_result(int rx_status, int tx_status,
                        uint64_t *rx_bytes, uint64_t *tx_bytes)
{
	if (rx_bytes == NULL || tx_bytes == NULL || rx_bytes == tx_bytes) {
		if (rx_bytes != NULL)
			*rx_bytes = 0;
		if (tx_bytes != NULL)
			*tx_bytes = 0;
		return EINVAL;
	}
	if (rx_status != 0 || tx_status != 0) {
		*rx_bytes = 0;
		*tx_bytes = 0;
		return rx_status != 0 ? rx_status : tx_status;
	}
	return 0;
}

/* Parse the single unsigned-decimal line emitted by a sysfs byte counter.
 * Reject signs, leading whitespace, partial numbers, junk and overflow. */
static inline int
net_ifstats_parse_counter(const char *text, uint64_t *value)
{
	const unsigned char *cursor;
	const unsigned char *digits_end;
	char *end;
	unsigned long long parsed;

	if (text == NULL || value == NULL || text[0] == '\0')
		return EINVAL;

	cursor = (const unsigned char *)text;
	while (*cursor >= '0' && *cursor <= '9')
		++cursor;
	if (cursor == (const unsigned char *)text)
		return EINVAL;
	digits_end = cursor;

	while (*cursor == ' ' || *cursor == '\t' ||
	       *cursor == '\r' || *cursor == '\n')
		++cursor;
	if (*cursor != '\0')
		return EINVAL;

	errno = 0;
	parsed = strtoull(text, &end, 10);
	if (errno == ERANGE || parsed > UINT64_MAX)
		return ERANGE;
	if (errno != 0 || end == text ||
	    (const unsigned char *)end != digits_end)
		return EINVAL;

	*value = (uint64_t)parsed;
	return 0;
}

/* Subtract a persisted baseline without accepting strtoull's valid-prefix
 * behavior. Empty values retain the legacy zero baseline for first boot. */
static inline int
net_ifstats_counter_delta(uint64_t current, const char *baseline_text,
                          uint64_t *delta)
{
	uint64_t baseline = 0;
	int status;

	if (delta == NULL)
		return EINVAL;
	*delta = 0;
	if (baseline_text == NULL)
		return EINVAL;
	if (baseline_text[0] != '\0') {
		status = net_ifstats_parse_counter(baseline_text, &baseline);
		if (status != 0)
			return status;
	}
	if (current >= baseline)
		*delta = current - baseline;
	return 0;
}

#endif /* NET_IFSTATS_POLICY_H */

