#ifndef PADAVAN_IPTV_PROXY_NVRAM_POLICY_H
#define PADAVAN_IPTV_PROXY_NVRAM_POLICY_H

#include <errno.h>
#include <net/if.h>
#include <stddef.h>
#include <string.h>

#include "iptv_proxy_altnet_policy.h"

#define IPTV_PROXY_ALTN_MAX 64

struct iptv_proxy_nvram_settings {
	char viptv_ifname[IFNAMSIZ];
	char altnet[IPTV_PROXY_ALTN_MAX];
	int enabled;
	int quickleave;
};

typedef int (*iptv_proxy_nvram_copy_fn)(void *context, const char *name,
					char *destination, size_t size);
typedef int (*iptv_proxy_nvram_match_fn)(void *context, const char *name,
					 const char *expected,
					 const char *missing_value,
					 int *matches);

/* Read a strict 0/1 NVRAM selector; only an absent key gets a default. */
static inline int
iptv_proxy_nvram_boolean_read(iptv_proxy_nvram_match_fn match_value,
                              void *context, const char *name,
                              const char *missing_value, int *value_out)
{
	int matches_zero = 0, matches_one = 0;
	int status;

	if (!match_value || !name || !name[0] || !missing_value || !value_out) {
		errno = EINVAL;
		return EINVAL;
	}
	*value_out = 0;
	status = match_value(context, name, "0", missing_value, &matches_zero);
	if (status != 0)
		goto failed;
	status = match_value(context, name, "1", missing_value, &matches_one);
	if (status != 0)
		goto failed;
	if (matches_zero == matches_one) {
		status = EINVAL;
		goto failed;
	}
	*value_out = matches_one ? 1 : 0;
	return 0;
failed:
	if (status < 0)
		status = errno > 0 ? errno : EIO;
	if (status == 0)
		status = EIO;
	errno = status;
	return status;
}

/* Caller holds one NVRAM transaction and checks transaction close. */
static inline int
iptv_proxy_nvram_settings_read(struct iptv_proxy_nvram_settings *settings,
			       iptv_proxy_nvram_copy_fn copy_value,
			       iptv_proxy_nvram_match_fn match_value,
			       void *context)
{
	int status, enabled = 0, quickleave = 0;

	if (!settings || !copy_value || !match_value) {
		errno = EINVAL;
		return EINVAL;
	}
	memset(settings, 0, sizeof(*settings));

	status = copy_value(context, "viptv_ifname", settings->viptv_ifname,
			    sizeof(settings->viptv_ifname));
	if (status != 0)
		goto failed;

	/* Missing mr_enable_x historically enables the multicast proxy. */
	status = iptv_proxy_nvram_boolean_read(match_value, context,
					       "mr_enable_x", "1", &enabled);
	if (status != 0)
		goto failed;
	settings->enabled = enabled != 0;

	if (settings->enabled) {
		status = copy_value(context, "mr_altnet_x", settings->altnet,
				    sizeof(settings->altnet));
		if (status != 0)
			goto failed;
		status = iptv_proxy_altnet_normalize(settings->altnet,
						      sizeof(settings->altnet));
		if (status != 0)
			goto failed;

		/* Missing mr_qleave_x historically disables quickleave. */
		status = iptv_proxy_nvram_boolean_read(match_value, context,
						       "mr_qleave_x", "0",
						       &quickleave);
		if (status != 0)
			goto failed;
		settings->quickleave = quickleave;
	}
	return 0;

failed:
	memset(settings, 0, sizeof(*settings));
	if (status < 0)
		status = errno > 0 ? errno : EIO;
	if (status == 0)
		status = EIO;
	errno = status;
	return status;
}

#endif /* PADAVAN_IPTV_PROXY_NVRAM_POLICY_H */
