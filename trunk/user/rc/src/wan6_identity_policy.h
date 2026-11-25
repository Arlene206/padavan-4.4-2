#ifndef WAN6_IDENTITY_POLICY_H
#define WAN6_IDENTITY_POLICY_H

#include <errno.h>
#include <stddef.h>
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

#endif /* WAN6_IDENTITY_POLICY_H */
