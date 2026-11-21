#ifndef PADAVAN_VPN_SERVER_CONFIG_POLICY_H
#define PADAVAN_VPN_SERVER_CONFIG_POLICY_H

#include <ctype.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * The VPN server is started unconditionally by PID 1. A missing enable key
 * means disabled; a failed NVRAM read must not be interpreted as permission
 * to start a remotely reachable service.
 */
static inline int
vpn_server_config_enable_status(int read_status, int read_errno,
                                const char *value, int *enabled)
{
	if (!enabled)
		return EINVAL;

	*enabled = 0;
	if (read_status < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_status == 0)
		return 0;
	if (read_status != 1 || !value)
		return EIO;

	*enabled = strcmp(value, "1") == 0;
	return 0;
}

/*
 * Preserve the historical PPTP default for an absent type key, but reject
 * backend errors, malformed text and unsupported modes instead of silently
 * turning them into type 0.
 */
static inline int
vpn_server_config_type_status(int read_status, int read_errno,
                              const char *value, int *type)
{
	const char *cursor;
	char *end;
	long parsed;

	if (!type)
		return EINVAL;
	if (read_status < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_status == 0) {
		*type = 0;
		return 0;
	}
	if (read_status != 1 || !value)
		return EIO;

	cursor = value;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (!isdigit((unsigned char)*cursor))
		return EINVAL;
	while (isdigit((unsigned char)*cursor))
		++cursor;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (*cursor)
		return EINVAL;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno == ERANGE)
		return ERANGE;
	if (end == value || parsed < 0 || parsed > 2)
		return EINVAL;

	*type = (int)parsed;
	return 0;
}

/*
 * Checked bounded integer read for configuration values whose backend
 * failure must not silently become the legacy integer zero.
 */
static inline int
vpn_server_config_int_status(int read_status, int read_errno,
                             const char *value, int default_value,
                             int minimum, int maximum, int *result)
{
	const char *cursor;
	char *end;
	long parsed;

	if (!result || minimum > maximum ||
	    default_value < minimum || default_value > maximum)
		return EINVAL;
	if (read_status < 0)
		return read_errno > 0 ? read_errno : EIO;
	if (read_status == 0) {
		*result = default_value;
		return 0;
	}
	if (read_status != 1 || !value)
		return EIO;

	cursor = value;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (!*cursor) {
		*result = default_value;
		return 0;
	}
	if (!isdigit((unsigned char)*cursor))
		return EINVAL;
	while (isdigit((unsigned char)*cursor))
		++cursor;
	while (*cursor && isspace((unsigned char)*cursor))
		++cursor;
	if (*cursor)
		return EINVAL;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno == ERANGE)
		return ERANGE;
	if (end == value || parsed < minimum || parsed > maximum)
		return EINVAL;

	*result = (int)parsed;
	return 0;
}

static inline int
vpn_server_pool_parse_ipv4(const char *value, uint32_t *address)
{
	struct in_addr parsed;
	if (!value || !address || inet_pton(AF_INET, value, &parsed) != 1)
		return EINVAL;
	*address = ntohl(parsed.s_addr);
	return 0;
}

static inline int
vpn_server_pool_validate_lan(uint32_t lan_address, uint32_t lan_mask,
			     uint32_t *network, uint32_t *broadcast)
{
	uint32_t host_mask, host_address;
	if (!network || !broadcast || lan_mask == 0)
		return EINVAL;
	host_mask = ~lan_mask;
	if (host_mask < 3 || (host_mask & (host_mask + 1)) != 0)
		return EINVAL;
	*network = lan_address & lan_mask;
	*broadcast = *network | host_mask;
	host_address = lan_address & host_mask;
	if (host_address == 0 || host_address == host_mask)
		return EINVAL;
	return 0;
}

static inline int
vpn_server_pool_resolve_lan(uint32_t lan_address, uint32_t lan_mask,
			    int configured_start, int configured_end,
			    int maximum_clients, uint32_t *local_address,
			    uint32_t *result_mask, uint32_t *client_start,
			    uint32_t *client_end)
{
	uint32_t network, broadcast, host_mask, gateway_offset;
	unsigned int start, end, last_host;
	int status;
	if (!local_address || !result_mask || !client_start || !client_end ||
	    maximum_clients < 1 || configured_start < 1 ||
	    configured_start > 254 || configured_end < 2 ||
	    configured_end > 254 || configured_start >= configured_end ||
	    configured_end - configured_start > maximum_clients)
		return EINVAL;
	status = vpn_server_pool_validate_lan(lan_address, lan_mask,
					      &network, &broadcast);
	if (status != 0)
		return status;
	host_mask = ~lan_mask;
	last_host = host_mask - 1;
	start = (unsigned int)configured_start;
	end = (unsigned int)configured_end;
	if (start >= last_host)
		start = last_host - 1;
	if (end > last_host)
		end = last_host;
	if (end <= start)
		end = start + 1;
	if (start < 1 || end > last_host || start >= end ||
	    end - start > (unsigned int)maximum_clients)
		return EINVAL;
	gateway_offset = lan_address & host_mask;
	if (gateway_offset >= start && gateway_offset <= end)
		return EADDRINUSE;
	*local_address = network | start;
	*result_mask = lan_mask;
	*client_start = start + 1;
	*client_end = end;
	return 0;
}

static inline int
vpn_server_pool_resolve_virtual(uint32_t virtual_address,
				uint32_t lan_address, uint32_t lan_mask,
				int maximum_clients, uint32_t *local_address,
				uint32_t *result_mask, uint32_t *client_start,
				uint32_t *client_end)
{
	uint32_t lan_network, lan_broadcast, virtual_network, virtual_broadcast;
	unsigned int first, second;
	int status;
	if (!local_address || !result_mask || !client_start || !client_end ||
	    maximum_clients < 1 || maximum_clients > 50)
		return EINVAL;
	status = vpn_server_pool_validate_lan(lan_address, lan_mask,
					      &lan_network, &lan_broadcast);
	if (status != 0)
		return status;
	first = (virtual_address >> 24) & 0xff;
	second = (virtual_address >> 16) & 0xff;
	if (!(first == 10 ||
	      (first == 172 && second >= 16 && second <= 31) ||
	      (first == 192 && second == 168)))
		return EINVAL;
	virtual_network = virtual_address & 0xffffff00U;
	virtual_broadcast = virtual_network | 0xffU;
	if (virtual_network <= lan_broadcast &&
	    virtual_broadcast >= lan_network)
		return EADDRINUSE;
	*local_address = virtual_network | 1U;
	*result_mask = 0xffffff00U;
	*client_start = 2U;
	*client_end = (uint32_t)maximum_clients + 1U;
	return 0;
}

#endif /* PADAVAN_VPN_SERVER_CONFIG_POLICY_H */
