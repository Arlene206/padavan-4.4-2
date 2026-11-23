#ifndef PADAVAN_IPTV_PROXY_NVRAM_POLICY_H
#define PADAVAN_IPTV_PROXY_NVRAM_POLICY_H

#include <errno.h>
#include <net/if.h>
#include <stddef.h>
#include <string.h>

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

/* Caller holds one NVRAM transaction and checks transaction close. */
static inline int
iptv_proxy_nvram_settings_read(struct iptv_proxy_nvram_settings *settings,
			       iptv_proxy_nvram_copy_fn copy_value,
			       iptv_proxy_nvram_match_fn match_value,
			       void *context)
{
	int status, enabled = 0, quickleave_is_zero = 1;

	if (!settings || !copy_value || !match_value) {
		errno = EINVAL;
		return EINVAL;
	}
	memset(settings, 0, sizeof(*settings));

	status = copy_value(context, "viptv_ifname", settings->viptv_ifname,
			    sizeof(settings->viptv_ifname));
	if (status != 0)
		goto failed;

	/* Missing mr_enable_x historically leaves the inverse-match test false. */
	status = match_value(context, "mr_enable_x", "1", "1", &enabled);
	if (status != 0)
		goto failed;
	settings->enabled = enabled != 0;

	if (settings->enabled) {
		status = copy_value(context, "mr_altnet_x", settings->altnet,
				    sizeof(settings->altnet));
		if (status != 0)
			goto failed;

		/* Missing mr_qleave_x historically disables quickleave. */
		status = match_value(context, "mr_qleave_x", "0", "0",
				     &quickleave_is_zero);
		if (status != 0)
			goto failed;
		settings->quickleave = !quickleave_is_zero;
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
