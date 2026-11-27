#ifndef RC_BRIDGE_INIT_POLICY_H
#define RC_BRIDGE_INIT_POLICY_H

#include <errno.h>

typedef struct {
	int first_status;
	unsigned int failure_count;
	const char *first_operation;
} rc_bridge_init_status_t;

typedef struct {
	int rt_radio_on;
	int rt_mode_x;
	int wl_radio_on;
	int wl_mode_x;
} rc_bridge_wireless_state_t;

typedef int (*rc_bridge_wireless_int_reader_fn)(void *context,
						 const char *name,
						 int *value);

/* Resolve every bridge radio selector before changing hardware or interfaces. */
static inline int
rc_bridge_wireless_state_read(rc_bridge_wireless_int_reader_fn read_int,
			      void *context, int read_rt_mode,
			      int has_wl, rc_bridge_wireless_state_t *state)
{
	int status;

	if (!read_int || !state ||
	    (read_rt_mode != 0 && read_rt_mode != 1) ||
	    (has_wl != 0 && has_wl != 1))
		return EINVAL;

	state->rt_radio_on = 0;
	state->rt_mode_x = 0;
	state->wl_radio_on = 0;
	state->wl_mode_x = 0;

	status = read_int(context, "rt_radio_x", &state->rt_radio_on);
	if (status != 0)
		goto fail;
	if (read_rt_mode) {
		status = read_int(context, "rt_mode_x", &state->rt_mode_x);
		if (status != 0)
			goto fail;
	}
	if (has_wl) {
		status = read_int(context, "wl_radio_x", &state->wl_radio_on);
		if (status != 0)
			goto fail;
		status = read_int(context, "wl_mode_x", &state->wl_mode_x);
		if (status != 0)
			goto fail;
	}
	return 0;

fail:
	state->rt_radio_on = 0;
	state->rt_mode_x = 0;
	state->wl_radio_on = 0;
	state->wl_mode_x = 0;
	return status;
}

static inline void
rc_bridge_init_status_reset(rc_bridge_init_status_t *status)
{
	if (!status)
		return;
	status->first_status = 0;
	status->failure_count = 0;
	status->first_operation = NULL;
}

static inline void
rc_bridge_init_record(rc_bridge_init_status_t *status, const char *operation,
		      int operation_status)
{
	if (!status || operation_status == 0)
		return;
	if (status->first_status == 0) {
		status->first_status = operation_status;
		status->first_operation = operation;
	}
	status->failure_count++;
}

static inline int
rc_bridge_init_result(const rc_bridge_init_status_t *status)
{
	return status ? status->first_status : EINVAL;
}

/*
 * A recorded bridge-start failure means the directly started guest APs cannot
 * be assumed to have a complete LAN/isolation topology. A missing status is
 * also unsafe; the caller's rollback helper preserves pre-existing APs.
 */
static inline int
rc_bridge_init_guest_ap_rollback_required(const rc_bridge_init_status_t *status)
{
	return !status || status->first_status != 0;
}

/* Never select a physical WAN/LAN bridge member from an error-collapsed mode. */
static inline int
rc_bridge_init_wan_port_uses_vlan(int mode_status, int bridge_mode,
                                  int disabled_mode, int *uses_vlan)
{
	if (!uses_vlan)
		return EINVAL;
	*uses_vlan = 0;
	if (mode_status != 0)
		return mode_status;
	*uses_vlan = (bridge_mode != disabled_mode);
	return 0;
}

/* An existing bridge is success only when a kernel bridge-port query confirms it. */
static inline int
rc_bridge_init_add_bridge_result(int add_status, int add_errno,
				 int bridge_probe_status)
{
	if (add_status == 0)
		return 0;
	if (add_status < 0 && add_errno == EEXIST && bridge_probe_status >= 0)
		return 0;
	return add_status;
}

#endif /* RC_BRIDGE_INIT_POLICY_H */
