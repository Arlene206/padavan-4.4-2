#ifndef WAN6_IDENTITY_POLICY_H
#define WAN6_IDENTITY_POLICY_H

#include <errno.h>
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

#endif /* WAN6_IDENTITY_POLICY_H */
