#ifndef PADAVAN_DHCP_LEASE_POLICY_H
#define PADAVAN_DHCP_LEASE_POLICY_H

#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Check canonical dotted-quad syntax, including the all-zero route prefix. */
static inline int
dhcp_lease_ipv4_text_is_valid(const char *value)
{
	struct in_addr address;

	return value && inet_pton(AF_INET, value, &address) == 1;
}

/* Match the existing RC is_valid_ipv4() contract: 0.0.0.0 is not a host. */
static inline int
dhcp_lease_ipv4_is_valid(const char *value)
{
	return dhcp_lease_ipv4_text_is_valid(value) &&
	       strcmp(value, "0.0.0.0") != 0;
}

/* Reject truncation of corrupt prior lease values into other IPv4 addresses. */
static inline int
dhcp_lease_text_fits_buffer(const char *value, size_t buffer_size)
{
	return value != NULL && buffer_size > 0 && strlen(value) < buffer_size;
}

/* RFC classless routes permit 0.0.0.0 only for the /0 default route. */
static inline int
dhcp_lease_route_destination_is_valid(const char *address,
                                      unsigned int prefix_length)
{
	if (prefix_length > 32U)
		return 0;
	if (prefix_length == 0U)
		return address && strcmp(address, "0.0.0.0") == 0;

	return dhcp_lease_ipv4_is_valid(address);
}

/* A netmask must be non-zero and contain one contiguous run of leading ones. */
static inline int
dhcp_lease_netmask_is_valid(const char *value)
{
	struct in_addr address;
	unsigned int mask, inverse;

	if (!value || inet_pton(AF_INET, value, &address) != 1)
		return 0;

	mask = ntohl(address.s_addr);
	if (mask == 0)
		return 0;

	inverse = ~mask;
	return (inverse & (inverse + 1U)) == 0;
}

/* Validate a whitespace-separated list of IPv4 addresses with explicit bounds. */
static inline int
dhcp_lease_ipv4_list_is_valid(const char *value, size_t max_length,
                              unsigned int max_items, int allow_empty)
{
	const unsigned char *p;
	unsigned int items = 0;
	size_t length;

	if (!value)
		return allow_empty ? 1 : 0;

	length = strlen(value);
	if (length > max_length)
		return 0;

	p = (const unsigned char *)value;
	while (*p) {
		char address[INET_ADDRSTRLEN];
		const unsigned char *start;
		size_t token_length;

		while (*p == ' ' || *p == '\t')
			++p;
		if (!*p)
			break;

		start = p;
		while (*p && *p != ' ' && *p != '\t')
			++p;
		token_length = (size_t)(p - start);
		if (token_length == 0 || token_length >= sizeof(address))
			return 0;
		if (++items > max_items)
			return 0;

		memcpy(address, start, token_length);
		address[token_length] = '\0';
		if (!dhcp_lease_ipv4_is_valid(address))
			return 0;
	}

	return items > 0 || allow_empty;
}

/* Return 1 for a first address, 0 for an empty list and -1 for invalid input. */
static inline int
dhcp_lease_ipv4_list_first(const char *value, char *out, size_t out_size)
{
	const unsigned char *p;
	const unsigned char *start;
	size_t token_length;

	if (!out || out_size == 0)
		return -1;
	out[0] = '\0';

	if (!dhcp_lease_ipv4_list_is_valid(value, 255, 16, 1))
		return -1;
	if (!value)
		return 0;

	p = (const unsigned char *)value;
	while (*p == ' ' || *p == '\t')
		++p;
	if (!*p)
		return 0;

	start = p;
	while (*p && *p != ' ' && *p != '\t')
		++p;
	token_length = (size_t)(p - start);
	if (token_length >= out_size)
		return -1;

	memcpy(out, start, token_length);
	out[token_length] = '\0';
	return 1;
}

/* A missing or withdrawn DHCP router option is equivalent to an empty
 * gateway. Compare the normalized first router against the stored lease so
 * renewal callbacks do not preserve a gateway that the server removed. */
static inline int
dhcp_lease_router_is_changed(const char *stored_gateway,
                             const char *router_option,
                             char *next_gateway, size_t next_gateway_size)
{
	if (dhcp_lease_ipv4_list_first(router_option,
	                               next_gateway, next_gateway_size) < 0)
		next_gateway[0] = '\0';

	return strcmp(stored_gateway ? stored_gateway : "", next_gateway) != 0;
}

/* A missing old default route is harmless during lease reconciliation;
 * other delete errors must stop publication of the replacement lease. */
static inline int
dhcp_lease_route_delete_is_ok(int status)
{
	return status == 0 || status == ESRCH;
}

/* Static host files may be restored once the old route is absent and the
 * interface accepted its persistent static address. Resolver publication is
 * checked independently so a resolver retry does not leave stale host files. */
static inline int
dhcp_lease_deconfig_address_ready(int route_status, int ifconfig_status)
{
	return dhcp_lease_route_delete_is_ok(route_status) && ifconfig_status == 0;
}

/* Deconfig may publish static LAN state only after the old DHCP route is
 * absent and both interface and resolver cleanup have completed. */
static inline int
dhcp_lease_deconfig_status(int route_status, int ifconfig_status,
			   int resolver_status)
{
	if (!dhcp_lease_route_delete_is_ok(route_status))
		return route_status;
	if (ifconfig_status != 0)
		return ifconfig_status;
	return resolver_status;
}


/* Route creation can report EEXIST when the desired default route is already
 * installed. Treat that as an idempotent success, but do not later delete a
 * route that this transaction did not create. */
static inline int
dhcp_lease_route_add_is_ok(int status)
{
	return status == 0 || status == EEXIST;
}

typedef int (*dhcp_lease_route_operation)(void *context, const char *gateway);
typedef int (*dhcp_lease_clear_address_operation)(void *context);
typedef int (*dhcp_lease_set_address_operation)(void *context,
						const char *ipaddr,
						const char *netmask);
typedef int (*dhcp_lease_publish_resolver_operation)(void *context);

struct dhcp_lease_bound_ops {
	dhcp_lease_route_operation delete_route;
	dhcp_lease_clear_address_operation clear_address;
	dhcp_lease_set_address_operation set_address;
	dhcp_lease_route_operation add_route;
	dhcp_lease_publish_resolver_operation publish_resolver;
};

/* Apply address, gateway and resolver changes as one recoverable LAN DHCP
 * transition. The resolver callback must publish atomically. Network changes
 * are rolled back on any failed operation because BusyBox's script runner
 * discards this callback's exit status and will not roll its lease state back. */
static inline int
dhcp_lease_bound_apply(const char *old_ipaddr, const char *old_netmask,
		       const char *old_gateway, const char *new_ipaddr,
		       const char *new_netmask, const char *new_gateway,
		       int apply_address, int clear_address, int apply_gateway,
		       const struct dhcp_lease_bound_ops *ops, void *context,
		       int *rollback_status)
{
	int status = 0, rollback = 0;
	int gateway_changed, old_route_deleted = 0;
	int address_touched = 0, new_route_added = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (!old_ipaddr || !old_netmask || !old_gateway ||
	    !new_ipaddr || !new_netmask || !new_gateway || !ops ||
	    !ops->delete_route || !ops->clear_address || !ops->set_address ||
	    !ops->add_route || !ops->publish_resolver ||
	    (clear_address && !apply_address))
		return EINVAL;

	if (apply_address &&
	    (!dhcp_lease_ipv4_is_valid(old_ipaddr) ||
	     !dhcp_lease_netmask_is_valid(old_netmask) ||
	     !dhcp_lease_ipv4_is_valid(new_ipaddr) ||
	     !dhcp_lease_netmask_is_valid(new_netmask)))
		return EINVAL;

	gateway_changed = strcmp(old_gateway, new_gateway) != 0;
	if (gateway_changed && dhcp_lease_ipv4_is_valid(old_gateway)) {
		status = ops->delete_route(context, old_gateway);
		if (!dhcp_lease_route_delete_is_ok(status))
			return status;
		old_route_deleted = (status == 0);
	}

	if (apply_address) {
		address_touched = 1;
		if (clear_address) {
			status = ops->clear_address(context);
			if (status != 0)
				goto rollback;
		}
		status = ops->set_address(context, new_ipaddr, new_netmask);
		if (status != 0)
			goto rollback;
	}

	if ((gateway_changed || apply_gateway) &&
	    dhcp_lease_ipv4_is_valid(new_gateway)) {
		status = ops->add_route(context, new_gateway);
		if (!dhcp_lease_route_add_is_ok(status))
			goto rollback;
		new_route_added = (status == 0);
	}

	status = ops->publish_resolver(context);
	if (status != 0)
		goto rollback;

	return 0;

rollback:
	/* Attempt every compensating action even if an earlier rollback fails.
	 * Keep the original operation error as the function result and expose the
	 * first rollback error separately for diagnostics and the next handoff. */
	if (new_route_added) {
		int cleanup_status = ops->delete_route(context, new_gateway);
		if (!dhcp_lease_route_delete_is_ok(cleanup_status) && !rollback)
			rollback = cleanup_status;
	}
	if (address_touched) {
		int cleanup_status = ops->set_address(context, old_ipaddr,
						       old_netmask);
		if (cleanup_status != 0 && !rollback)
			rollback = cleanup_status;
	}
	if (old_route_deleted) {
		int cleanup_status = ops->add_route(context, old_gateway);
		if (!dhcp_lease_route_add_is_ok(cleanup_status) && !rollback)
			rollback = cleanup_status;
	}
	if (rollback_status)
		*rollback_status = rollback;
	return status;
}

/*
 * Publish related temporary NVRAM lease keys as one compensatable group.
 * The caller owns the NVRAM transaction lock for the complete operation.
 * Restore the failing key too: its ioctl may have succeeded even if close
 * subsequently reported an error.
 */
struct dhcp_lease_nvram_value {
	const char *name;
	const char *value;
	const char *previous_value;
};

typedef int (*dhcp_lease_nvram_set_operation)(void *context,
					      const char *name,
					      const char *value);

static inline int
dhcp_lease_nvram_publish(const struct dhcp_lease_nvram_value *values,
			 size_t count,
			 dhcp_lease_nvram_set_operation set_value,
			 void *context, int *rollback_status)
{
	size_t i, j;
	int status, first_status = 0, rollback = 0;

	if (rollback_status)
		*rollback_status = 0;
	if (!values || count == 0 || !set_value)
		return EINVAL;
	/* Validate the complete vector before changing the first value. */
	for (i = 0; i < count; ++i) {
		if (!values[i].name || !*values[i].name ||
		    !values[i].value || !values[i].previous_value)
			return EINVAL;
	}

	/* A failed close/ioctl may still have applied; attempt every candidate. */
	for (i = 0; i < count; ++i) {
		status = set_value(context, values[i].name, values[i].value);
		if (status != 0 && first_status == 0)
			first_status = status;
	}
	if (first_status == 0)
		return 0;

	/* Restore the whole last-known-good tuple, including later fields. */
	for (j = count; j > 0; --j) {
		status = set_value(context, values[j - 1].name,
				   values[j - 1].previous_value);
		if (status != 0 && rollback == 0)
			rollback = status;
	}
	if (rollback_status)
		*rollback_status = rollback;
	return first_status;
}


/*
 * Guard a delayed lease publication against a newer callback that committed
 * after the caller captured its rollback tuple. The caller must hold the same
 * exclusive NVRAM transaction for this comparison and the subsequent writes.
 * A NULL current value represents a missing key, whose legacy value is "".
 */
typedef int (*dhcp_lease_nvram_get_operation)(void *context,
                                               const char *name,
                                               const char **value);

static inline int
dhcp_lease_nvram_snapshot_matches(
	const struct dhcp_lease_nvram_value *values, size_t count,
	dhcp_lease_nvram_get_operation get_value, void *context)
{
	const char *current_value;
	size_t i;
	int status;

	if (!values || count == 0 || !get_value)
		return EINVAL;
	for (i = 0; i < count; ++i) {
		if (!values[i].name || !values[i].name[0] ||
		    !values[i].value || !values[i].previous_value)
			return EINVAL;
	}

	for (i = 0; i < count; ++i) {
		current_value = NULL;
		status = get_value(context, values[i].name, &current_value);
		if (status != 0)
			return status;
		if (!current_value)
			current_value = "";
		if (strcmp(current_value, values[i].previous_value) != 0)
			return EAGAIN;
	}
	return 0;
}

/*
 * Compare-and-publish a lease tuple while a transaction lock is held. The
 * attempted flag distinguishes stale/read-failed preflight from a write or
 * commit failure, so callers never roll a concurrent writer back to stale
 * values when no publication was attempted.
 */
static inline int
dhcp_lease_nvram_publish_if_current(
	const struct dhcp_lease_nvram_value *values, size_t count,
	dhcp_lease_nvram_get_operation get_value,
	dhcp_lease_nvram_set_operation set_value, void *context,
	int *publish_attempted, int *rollback_status)
{
	int status;

	if (!publish_attempted)
		return EINVAL;
	*publish_attempted = 0;
	if (rollback_status)
		*rollback_status = 0;

	status = dhcp_lease_nvram_snapshot_matches(
		values, count, get_value, context);
	if (status != 0)
		return status;

	*publish_attempted = 1;
	return dhcp_lease_nvram_publish(
		values, count, set_value, context, rollback_status);
}

/*
 * Best-effort restoration is intentionally different from publish rollback:
 * it attempts every old tuple even if one setter reports an error. It must
 * never compensate a failed restore by re-applying the candidate lease.
 */
static inline int
dhcp_lease_nvram_restore(const struct dhcp_lease_nvram_value *values,
			 size_t count,
			 dhcp_lease_nvram_set_operation set_value,
			 void *context, int *restore_status)
{
	size_t i;
	int status, first_status = 0;

	if (restore_status)
		*restore_status = 0;
	if (!values || count == 0 || !set_value)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		if (!values[i].name || !*values[i].name ||
		    !values[i].previous_value)
			return EINVAL;
	}

	for (i = count; i > 0; --i) {
		status = set_value(context, values[i - 1].name,
				   values[i - 1].previous_value);
		if (status != 0 && first_status == 0)
			first_status = status;
	}

	if (restore_status)
		*restore_status = first_status;
	return first_status;
}

/* Strict decimal parser: no signs, whitespace, suffixes or overflow. */
static inline int
dhcp_lease_decimal_is_valid(const char *value, unsigned int minimum,
                            unsigned int maximum, unsigned int *result)
{
	const unsigned char *p = (const unsigned char *)value;
	unsigned int number = 0;

	if (!p || !*p || minimum > maximum)
		return 0;

	for (; *p; ++p) {
		unsigned int digit;

		if (*p < '0' || *p > '9')
			return 0;
		digit = (unsigned int)(*p - '0');
		if (number > maximum / 10U ||
		    (number == maximum / 10U && digit > maximum % 10U))
			return 0;
		number = number * 10U + digit;
	}

	if (number < minimum)
		return 0;
	if (result)
		*result = number;
	return 1;
}

/* Route options are parsed as data, never as shell commands. Bound their size
 * and alphabet before storing them for the route-table parser. */
static inline int
dhcp_lease_route_text_is_safe(const char *value)
{
	const unsigned char *p = (const unsigned char *)value;
	size_t length;

	if (!p)
		return 0;
	length = strlen(value);
	if (length > 2048)
		return 0;

	for (; *p; ++p) {
		if (!((*p >= '0' && *p <= '9') || *p == '.' ||
		      *p == '/' || *p == ' ' || *p == '\t'))
			return 0;
	}

	return 1;
}


/* Clear a withdrawn IPTV lease address and delete its old routes even if
 * either operation fails. Keep lease identity until both are complete so
 * BusyBox can retry DECONFIG_PENDING without resurrecting a revoked address. */
typedef int (*dhcp_lease_viptv_deconfig_operation)(void *context);
typedef int (*dhcp_lease_viptv_deconfig_publish_operation)(void *context);

struct dhcp_lease_viptv_deconfig_ops {
	dhcp_lease_viptv_deconfig_operation clear_address;
	dhcp_lease_viptv_deconfig_operation delete_routes;
	dhcp_lease_viptv_deconfig_publish_operation publish_empty_address;
};

static inline int
dhcp_lease_viptv_deconfig_apply(
	const struct dhcp_lease_viptv_deconfig_ops *ops, void *context)
{
	int interface_status, route_status, status;

	if (!ops || !ops->clear_address || !ops->delete_routes ||
	    !ops->publish_empty_address)
		return EINVAL;

	/* A failed interface clear must not suppress independent route cleanup. */
	interface_status = ops->clear_address(context);
	route_status = ops->delete_routes(context);
	status = dhcp_lease_deconfig_status(route_status, interface_status, 0);
	if (status != 0)
		return status;

	return ops->publish_empty_address(context);
}

#endif /* PADAVAN_DHCP_LEASE_POLICY_H */
