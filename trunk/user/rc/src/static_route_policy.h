#ifndef PADAVAN_STATIC_ROUTE_POLICY_H
#define PADAVAN_STATIC_ROUTE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * The generic HTTPD NVRAM group accepts up to 128 rows, while the current
 * WebUI prevents adding more than 64. Continue to apply older/manual entries
 * above the UI limit rather than dropping routes on upgrade.
 */
#define STATIC_ROUTE_MAX_ENTRIES 128
#define STATIC_ROUTE_IPV4_TEXT_MAX 15
#define STATIC_ROUTE_METRIC_TEXT_MAX 10
#define STATIC_ROUTE_MAX_ENTRY_TEXT \
	(3 * STATIC_ROUTE_IPV4_TEXT_MAX + 3 + STATIC_ROUTE_METRIC_TEXT_MAX)
#define STATIC_ROUTE_MAX_SNAPSHOT_SIZE \
	(STATIC_ROUTE_MAX_ENTRIES * STATIC_ROUTE_MAX_ENTRY_TEXT + \
	 (STATIC_ROUTE_MAX_ENTRIES - 1) + 1)

static inline int
static_route_entry_count_is_safe(int count)
{
	return count >= 0 && count <= STATIC_ROUTE_MAX_ENTRIES;
}

/*
 * Checked replacement for nvram_invmatch("sr_enable_x", "1"). Missing keys
 * retain the legacy disabled default; backend errors must not skip updates.
 */
static inline int
static_route_nvram_enabled_status(int api_result, int saved_errno,
				  const char *value, int missing_enabled,
				  int *enabled)
{
	if (!enabled)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		if (value)
			return EIO;
		*enabled = missing_enabled != 0;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	*enabled = strcmp(value, "1") == 0;
	return 0;
}

/*
 * Empty metrics retain the old atoi("") == 0 behavior. Non-empty values must
 * be unsigned decimal and leave room for route_add()'s metric + 1 adjustment.
 */
static inline int
static_route_metric_parse(const char *value, int *metric)
{
	unsigned int parsed = 0;
	size_t i;

	if (!value || !metric)
		return EINVAL;
	*metric = 0;
	if (!value[0])
		return 0;

	for (i = 0; value[i]; ++i) {
		unsigned int digit;

		if (value[i] < '0' || value[i] > '9')
			return EINVAL;
		digit = (unsigned int)(value[i] - '0');
		if (parsed > ((unsigned int)INT_MAX - 1U - digit) / 10U)
			return ERANGE;
		parsed = parsed * 10U + digit;
	}

	*metric = (int)parsed;
	return 0;
}

static inline int
static_route_interface_is_safe(const char *value)
{
	if (!value)
		return 0;
	return value[0] == '\0' || strcmp(value, "LAN") == 0 ||
	       strcmp(value, "MAN") == 0 || strcmp(value, "WAN") == 0;
}

/* Strict dotted-quad validation for route destinations, masks and gateways. */
static inline int
static_route_ipv4_address_is_valid(const char *value)
{
	unsigned int octet = 0;
	int digits = 0, separators = 0;
	size_t i;

	if (!value || !value[0])
		return 0;
	for (i = 0; value[i]; ++i) {
		if (value[i] >= '0' && value[i] <= '9') {
			/* inet_aton() treats leading-zero octets as octal. Reject them. */
			if (++digits > 3 || (digits > 1 && octet == 0))
				return 0;
			octet = octet * 10U + (unsigned int)(value[i] - '0');
			if (octet > 255U)
				return 0;
		} else if (value[i] == '.') {
			if (digits == 0 || separators >= 3)
				return 0;
			++separators;
			digits = 0;
			octet = 0;
		} else {
			return 0;
		}
	}
	return separators == 3 && digits > 0;
}

/*
 * An IPv4 netmask is a contiguous run of leading one bits. The all-zero
 * mask is retained for an explicit default route (0.0.0.0/0).
 */
static inline int
static_route_ipv4_netmask_is_valid(const char *value)
{
	uint32_t mask = 0;
	uint32_t inverse;
	unsigned int octet = 0;
	size_t i;

	if (!static_route_ipv4_address_is_valid(value))
		return 0;
	for (i = 0; value[i]; ++i) {
		if (value[i] >= '0' && value[i] <= '9') {
			octet = octet * 10U + (unsigned int)(value[i] - '0');
		} else {
			mask = (mask << 8) | octet;
			octet = 0;
		}
	}
	mask = (mask << 8) | octet;
	inverse = ~mask;
	return (inverse & (inverse + UINT32_C(1))) == 0;
}

#endif /* PADAVAN_STATIC_ROUTE_POLICY_H */
