#ifndef WAN6_IDENTITY_POLICY_H
#define WAN6_IDENTITY_POLICY_H

#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef int (*wan6_identity_set_value_fn)(void *context, int unit,
					  const char *param_name,
					  const char *value);
typedef int (*wan6_identity_stop_sit_fn)(void *context);

/* Do not leave a newly-created SIT device without a committed teardown identity. */
static inline int
wan6_publish_sit_identity(wan6_identity_set_value_fn set_value,
			  wan6_identity_stop_sit_fn stop_sit,
			  void *context, int unit, const char *ifname,
			  int *rollback_status)
{
	int status, stop_status, clear_status;

	if (rollback_status)
		*rollback_status = 0;
	if (!set_value || !stop_sit || unit < 0 || !ifname || !ifname[0])
		return EINVAL;

	status = set_value(context, unit, "ifname6", ifname);
	if (status == 0)
		return 0;

	stop_status = stop_sit(context);
	if (stop_status != 0) {
		if (rollback_status)
			*rollback_status = stop_status;
		return status;
	}

	clear_status = set_value(context, unit, "ifname6", "");
	if (rollback_status)
		*rollback_status = clear_status;
	return status;
}

/*
 * Preserve ifname6 as a recovery handle unless every native kernel rollback
 * operation completed successfully.
 */
static inline int
wan6_clear_native_identity_after_rollback(
	wan6_identity_set_value_fn set_value, void *context, int unit,
	int rollback_status)
{
	if (!set_value || unit < 0)
		return EINVAL;
	if (rollback_status != 0)
		return rollback_status;
	return set_value(context, unit, "ifname6", "");
}

/* Attempt both metadata clears and preserve the first failure. */
static inline int
wan6_clear_runtime_metadata(wan6_identity_set_value_fn set_value,
			    void *context, int unit, int clear_dns)
{
	int status = 0;
	int operation_status;

	if (!set_value || unit < 0)
		return EINVAL;

	if (clear_dns) {
		operation_status = set_value(context, unit, "dns6", "");
		if (status == 0)
			status = operation_status;
	}
	operation_status = set_value(context, unit, "ifname6", "");
	if (status == 0)
		status = operation_status;

	return status;
}


typedef int (*wan6_identity_get_addr6_fn)(void *context, const char *ifname,
					  char *address, int *found);

/*
 * A clean miss may use the LAN endpoint fallback. An NVRAM or address
 * enumeration error must remain distinguishable and must not select LAN.
 */
static inline int
wan6_identity_resolve_addr6_host(int ifname_status, const char *ifname,
				 char *address, const char **host,
				 wan6_identity_get_addr6_fn get_addr6,
				 void *context)
{
	char *prefix;
	int found = 0;
	int status;

	if (!ifname || !address || !host || !get_addr6)
		return EINVAL;

	*host = NULL;
	address[0] = '\0';
	if (ifname_status != 0)
		return ifname_status;
	if (!ifname[0])
		return 0;

	status = get_addr6(context, ifname, address, &found);
	if (status != 0) {
		address[0] = '\0';
		return status;
	}
	if (!found)
		return 0;

	prefix = strchr(address, '/');
	if (prefix)
		*prefix = '\0';
	if (!address[0])
		return EIO;

	*host = address;
	return 0;
}


typedef int (*wan6_lan_addr6_lookup_fn)(void *context, char *address,
					size_t address_size, int *found);
typedef int (*wan6_lan_addr6_publish_fn)(void *context, const char *address);

/*
 * Keep the historical clean-miss behavior (do not overwrite the last
 * published address), but never collapse an address-enumeration error into
 * a successful no-op.
 */
static inline int
wan6_identity_publish_lan_addr6(wan6_lan_addr6_lookup_fn lookup,
				wan6_lan_addr6_publish_fn publish,
				void *context, char *address,
				size_t address_size)
{
	int found = 0;
	int status;

	if (!lookup || !publish || !address || address_size == 0)
		return EINVAL;
	address[0] = '\0';

	status = lookup(context, address, address_size, &found);
	if (status != 0) {
		address[0] = '\0';
		return status;
	}
	if (!found)
		return 0;
	if (!address[0])
		return EIO;

	return publish(context, address);
}


typedef int (*wan6_sit_lan_addr6_clear_fn)(void *context);
typedef int (*wan6_sit_lan_addr6_add_fn)(void *context, const char *address);

/*
 * start_sit_tunnel() replaces the derived LAN address only after the SIT
 * endpoint and tunnel are configured. If a later operation fails, remove any
 * partial replacement and put the previous NVRAM-configured address back.
 * Both compensation operations are attempted; the first rollback error is
 * returned separately from the primary startup failure.
 */
static inline int
wan6_sit_restore_lan_addr6(int address_touched, const char *previous_address,
			   wan6_sit_lan_addr6_clear_fn clear_address,
			   wan6_sit_lan_addr6_add_fn add_address,
			   void *context, int *rollback_status)
{
	int status;
	int first_status = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (!address_touched)
		return 0;
	if (!previous_address || !clear_address || !add_address)
		return EINVAL;

	status = clear_address(context);
	if (status != 0)
		first_status = status;
	if (previous_address[0]) {
		status = add_address(context, previous_address);
		if (status != 0 && first_status == 0)
			first_status = status;
	}
	if (rollback_status)
		*rollback_status = first_status;
	return first_status;
}

/* A duplicate endpoint host route is already usable, but was not created here. */
static inline int
wan6_sit_endpoint_route_add_status(int add_status, int *route_added)
{
	if (!route_added)
		return EINVAL;

	*route_added = 0;
	if (add_status == 0) {
		*route_added = 1;
		return 0;
	}
	if (add_status == EEXIST)
		return 0;

	return add_status;
}


/* route_manip() treats an empty non-NULL gateway as invalid input. */
static inline const char *
wan6_sit_route4_gateway_argument(const char *gateway)
{
	return (gateway && gateway[0]) ? gateway : NULL;
}

/*
 * Normalize the optional WAN endpoint gateway before route_add() can pass it
 * to inet_aton(). Empty and canonical 0.0.0.0 retain the legacy direct-route
 * meaning; other values must be unambiguous dotted-decimal IPv4 literals.
 */
static inline int
wan6_sit_route4_gateway_normalize(const char *gateway, char *normalized,
				  size_t normalized_size, int *has_gateway)
{
	struct in_addr address;

	if (!normalized || normalized_size == 0 || !has_gateway)
		return EINVAL;
	normalized[0] = '\0';
	*has_gateway = 0;
	if (normalized_size < INET_ADDRSTRLEN)
		return ENAMETOOLONG;
	if (!gateway || !gateway[0] || strcmp(gateway, "0.0.0.0") == 0)
		return 0;
	if (inet_pton(AF_INET, gateway, &address) != 1)
		return EINVAL;
	if (!inet_ntop(AF_INET, &address, normalized, normalized_size))
		return errno ? errno : EIO;
	*has_gateway = 1;
	return 0;
}

#define WAN6_SIT_ROUTE4_RECORD_SIZE 64
#define WAN6_SIT_ROUTE4_IFNAME_SIZE 16
#define WAN6_SIT_ROUTE4_IPV4_SIZE 16

/*
 * route_manip() uses inet_aton(), which accepts legacy abbreviated and
 * hexadecimal IPv4 forms. Persisted ownership records must use strict
 * dotted-quad literals so corrupted state cannot alias a different route.
 */
static inline int
wan6_sit_route4_ipv4_literal_is_valid(const char *text)
{
	struct in_addr address;

	return text && text[0] && inet_pton(AF_INET, text, &address) == 1;
}

static inline int
wan6_sit_route4_record_format(const char *ifname, const char *endpoint,
			      const char *gateway, char *record,
			      size_t record_size)
{
	const char *gateway_value = gateway ? gateway : "";
	int written;

	if (!ifname || !ifname[0] || !endpoint || !endpoint[0] ||
	    !record || record_size == 0 ||
	    strchr(ifname, '|') || strchr(endpoint, '|') ||
	    strchr(gateway_value, '|') ||
	    !wan6_sit_route4_ipv4_literal_is_valid(endpoint) ||
	    (gateway_value[0] &&
	     !wan6_sit_route4_ipv4_literal_is_valid(gateway_value)))
		return EINVAL;
	/* Keep the persisted identity parseable by WAN6_SIT_ROUTE4_IFNAME_SIZE. */
	if (strlen(ifname) >= WAN6_SIT_ROUTE4_IFNAME_SIZE)
		return ENAMETOOLONG;

	record[0] = '\0';
	written = snprintf(record, record_size, "%s|%s|%s",
			   ifname, endpoint, gateway_value);
	if (written < 0)
		return EIO;
	if ((size_t)written >= record_size) {
		record[0] = '\0';
		return ENAMETOOLONG;
	}
	return 0;
}

static inline int
wan6_sit_route4_record_parse(const char *record, char *ifname,
			     size_t ifname_size, char *endpoint,
			     size_t endpoint_size, char *gateway,
			     size_t gateway_size)
{
	const char *first, *second;
	size_t ifname_length, endpoint_length, gateway_length;

	if (!record || !ifname || ifname_size == 0 ||
	    !endpoint || endpoint_size == 0 || !gateway || gateway_size == 0)
		return EINVAL;

	ifname[0] = '\0';
	endpoint[0] = '\0';
	gateway[0] = '\0';
	first = strchr(record, '|');
	if (!first || first == record)
		return EINVAL;
	second = strchr(first + 1, '|');
	if (!second || second == first + 1 || strchr(second + 1, '|'))
		return EINVAL;

	ifname_length = (size_t)(first - record);
	endpoint_length = (size_t)(second - first - 1);
	gateway_length = strlen(second + 1);
	if (ifname_length >= ifname_size ||
	    endpoint_length >= endpoint_size ||
	    gateway_length >= gateway_size)
		return ENAMETOOLONG;

	memcpy(ifname, record, ifname_length);
	ifname[ifname_length] = '\0';
	memcpy(endpoint, first + 1, endpoint_length);
	endpoint[endpoint_length] = '\0';
	memcpy(gateway, second + 1, gateway_length + 1);
	if (!wan6_sit_route4_ipv4_literal_is_valid(endpoint) ||
	    (gateway[0] &&
	     !wan6_sit_route4_ipv4_literal_is_valid(gateway))) {
		ifname[0] = '\0';
		endpoint[0] = '\0';
		gateway[0] = '\0';
		return EINVAL;
	}
	return 0;
}

typedef int (*wan6_sit_route4_delete_fn)(void *context,
					 const char *ifname,
					 const char *endpoint,
					 const char *gateway);
typedef int (*wan6_sit_route4_clear_fn)(void *context, int unit);

/*
 * Remove only the endpoint route recorded as locally owned. Keep the record
 * when deletion fails; ESRCH/ENODEV mean the route/interface is already gone.
 */
static inline int
wan6_sit_route4_cleanup_record(int unit, const char *record,
			       wan6_sit_route4_delete_fn delete_route,
			       wan6_sit_route4_clear_fn clear_record,
			       void *context)
{
	char ifname[WAN6_SIT_ROUTE4_IFNAME_SIZE];
	char endpoint[WAN6_SIT_ROUTE4_IPV4_SIZE];
	char gateway[WAN6_SIT_ROUTE4_IPV4_SIZE];
	int status;

	if (unit < 0 || !record || !delete_route || !clear_record)
		return EINVAL;
	if (!record[0])
		return 0;

	status = wan6_sit_route4_record_parse(
		record, ifname, sizeof(ifname), endpoint, sizeof(endpoint),
		gateway, sizeof(gateway));
	if (status != 0)
		return status;

	status = delete_route(context, ifname, endpoint, gateway);
	if (status != 0 && status != ESRCH && status != ENODEV)
		return status;

	return clear_record(context, unit);
}

#endif /* WAN6_IDENTITY_POLICY_H */
