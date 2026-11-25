#ifndef PADAVAN_TCP_TWEAKS_POLICY_H
#define PADAVAN_TCP_TWEAKS_POLICY_H

#include <errno.h>
#include <stddef.h>

/* Keep the boot-time TCP tuning set and its legacy values in one testable list. */
struct rc_tcp_tweak_setting {
	const char *name;
	int value;
};

static const struct rc_tcp_tweak_setting rc_tcp_tweak_settings[] = {
	{ "tcp_fin_timeout", 30 },
	{ "tcp_keepalive_intvl", 30 },
	{ "tcp_keepalive_probes", 5 },
	{ "tcp_keepalive_time", 1800 },
	{ "tcp_retries2", 5 },
	{ "tcp_syn_retries", 3 },
	{ "tcp_synack_retries", 3 },
	{ "tcp_tw_reuse", 1 },
	{ "tcp_rfc1337", 1 }
};

#define RC_TCP_TWEAK_COUNT \
	(sizeof(rc_tcp_tweak_settings) / sizeof(rc_tcp_tweak_settings[0]))

typedef int (*rc_tcp_tweak_write_fn)(void *context, const char *name, int value);

/* Attempt every independent sysctl and preserve the first failure. */
static int
rc_tcp_tweaks_apply(rc_tcp_tweak_write_fn write_operation, void *context)
{
	size_t i;
	int status;
	int first_status = 0;

	if (!write_operation)
		return EINVAL;

	for (i = 0; i < RC_TCP_TWEAK_COUNT; i++) {
		status = write_operation(context, rc_tcp_tweak_settings[i].name,
					 rc_tcp_tweak_settings[i].value);
		if (status != 0 && first_status == 0)
			first_status = status;
	}

	return first_status;
}

#endif /* PADAVAN_TCP_TWEAKS_POLICY_H */
