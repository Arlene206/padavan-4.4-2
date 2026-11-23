#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "modem_wan_start_policy.h"
#include "service_start_status.h"
#include "dhcp_lease_policy.h"

static void
test_link_change_reconnect_requires_checked_modem_state(void)
{
	int reconnect = -1;

	/* Reconnect on both legacy equal-state transitions. */
	assert(modem_wan_link_change_reconnect_enabled(2, 7, 1, 1,
						      &reconnect) == 0);
	assert(reconnect == 1);
	assert(modem_wan_link_change_reconnect_enabled(2, 7, 0, 0,
						      &reconnect) == 0);
	assert(reconnect == 1);

	/* Priority, device readiness and identity/link mismatch all gate it. */
	assert(modem_wan_link_change_reconnect_enabled(1, 7, 1, 1,
						      &reconnect) == 0);
	assert(reconnect == 0);
	assert(modem_wan_link_change_reconnect_enabled(2, 0, 1, 1,
						      &reconnect) == 0);
	assert(reconnect == 0);
	assert(modem_wan_link_change_reconnect_enabled(2, 7, 1, 0,
						      &reconnect) == 0);
	assert(reconnect == 0);

	/* The legacy has_link argument is truthy/falsy, not restricted to 0/1. */
	assert(modem_wan_link_change_reconnect_enabled(2, 7, 1, 9,
						      &reconnect) == 0);
	assert(reconnect == 1);

	assert(modem_wan_link_change_reconnect_enabled(2, -1, 0, 0,
						      &reconnect) == EINVAL);
	assert(reconnect == 0);
	assert(modem_wan_link_change_reconnect_enabled(2, 7, 2, 1,
						      &reconnect) == EINVAL);
	assert(reconnect == 0);
	assert(modem_wan_link_change_reconnect_enabled(2, 7, 0, 1,
						      NULL) == EINVAL);
}


static void
test_hotplug_notification_preserves_ndis_and_ras_legacy_gates(void)
{
	int notify = -1;
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "1", 1, "3", 1, &notify) == 0);
	assert(notify == 1);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "1", 1, "3", 0, &notify) == 0);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "2", 1, "1", 1, "4", 0, &notify) == 0);
	assert(notify == 1);
	assert(modem_wan_hotplug_notification_enabled(1, "0", 1, "1", 1, "3", 1, &notify) == 0);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "0", 1, "3", 1, &notify) == 0);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "01", 1, "3", 1, &notify) == 0);
	assert(notify == 0);
	/* A present empty type remains an inverted match for the RAS gate. */
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "1", 1, "", 0, &notify) == 0);
	assert(notify == 1);
}

static void
test_hotplug_notification_missing_any_legacy_match_key_stays_disabled(void)
{
	int notify = -1;
	assert(modem_wan_hotplug_notification_enabled(0, NULL, 1, "1", 1, "3", 1, &notify) == 0);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 0, NULL, 1, "3", 1, &notify) == 0);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "1", 0, NULL, 0, &notify) == 0);
	assert(notify == 0);
}

static void
test_hotplug_notification_rejects_invalid_and_unreadable_inputs(void)
{
	int notify = 1;
	assert(modem_wan_hotplug_notification_enabled(2, "1", 1, "1", 1, "3", 1, &notify) == EINVAL);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, NULL, 1, "1", 1, "3", 1, &notify) == EIO);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "1", 1, "3", 2, &notify) == EINVAL);
	assert(notify == 0);
	assert(modem_wan_hotplug_notification_enabled(1, "1", 1, "1", 1, "3", 1, NULL) == EINVAL);
}

static void
test_miniupnp_migration_failure_blocks_dhcp(void)
{
	assert(!modem_wan_start_migration_succeeded(EBUSY));
	assert(modem_wan_start_status(EBUSY, 0) == EBUSY);
	assert(modem_wan_start_status(EBUSY, EIO) == EBUSY);
	assert(!modem_wan_start_migration_succeeded(-1));
	assert(modem_wan_start_status(-1, 0) == -1);
}

static void
test_dhcp_launch_result_is_returned_after_migration(void)
{
	assert(modem_wan_start_migration_succeeded(0));
	assert(modem_wan_start_status(0, 0) == 0);
	assert(modem_wan_start_status(0, EIO) == EIO);
	assert(modem_wan_start_status(0, 23) == 23);
	assert(modem_wan_start_status(0, -1) == -1);
}

static void
test_ras_peer_file_cleanup_only_after_failed_launch(void)
{
	assert(!modem_wan_ras_peer_cleanup_required(0));
	assert(modem_wan_ras_peer_cleanup_required(1));
	assert(modem_wan_ras_peer_cleanup_required(-1));
}

static void
test_modem_selection_preserves_missing_priority_default(void)
{
	int selected = -1;

	/* A missing modem_prio key historically resolves to zero: no modem. */
	assert(modem_wan_select_device_status(7, 0, 0, 0, 0, 0,
					      &selected) == 0);
	assert(selected == 0);
}

static void
test_modem_selection_fails_closed_on_priority_read_error(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(7, EIO, 2, 0, 0, 0,
					      &selected) == EIO);
	assert(selected == 7);
}

static void
test_modem_selection_fails_closed_on_unknown_ethernet_link(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(7, 0, 2, 0, EACCES, 0,
					      &selected) == EACCES);
	assert(selected == 7);
}

static void
test_modem_link_probe_error_fails_closed(void)
{
	assert(modem_wan_link_probe_error(0, 0) == 0);
	assert(modem_wan_link_probe_error(1, 0) == 0);
	assert(modem_wan_link_probe_error(-1, EACCES) == EACCES);
	assert(modem_wan_link_probe_error(-1, 0) == EIO);
	assert(modem_wan_link_probe_error(2, 0) == EIO);
}

static void
test_modem_selection_prefers_wired_link_when_present(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(7, 0, 2, 0, 0, 1,
					      &selected) == 0);
	assert(selected == 0);
}

static void
test_modem_selection_uses_modem_only_after_confirmed_link_absence(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(7, 0, 2, 0, 0, 0,
					      &selected) == 0);
	assert(selected == 7);
}

static void
test_modem_selection_keeps_wisp_compatibility_without_link_probe(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(7, 0, 2, 1, EIO, 0,
					      &selected) == 0);
	assert(selected == 0);
}

static void
test_modem_selection_skips_priority_read_when_no_modem_is_present(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(0, EIO, 2, 0, EIO, 0,
					      &selected) == 0);
	assert(selected == 0);
}

static void
test_modem_selection_rejects_invalid_probe_and_output(void)
{
	int selected = -1;

	assert(modem_wan_select_device_status(7, 0, 2, 0, 0, 2,
					      &selected) == EIO);
	assert(selected == 7);
	assert(modem_wan_select_device_status(7, 0, 1, 0, 0, 0,
					      NULL) == EINVAL);
	assert(modem_wan_select_device_status(-1, 0, 1, 0, 0, 0,
					      &selected) == EINVAL);
}

static void
test_ndis_interface_bringup_failure_skips_network_start(void)
{
	int interface_status = EIO;
	int status = 0;
	int firewall_calls = 0;
	int control_calls = 0;
	int dhcp_calls = 0;

	/* launch_wan_usbnet() must return before starting any dependent service. */
	if (interface_status != 0) {
		status = modem_wan_start_status(0, interface_status);
	} else {
		firewall_calls++;
		control_calls++;
		dhcp_calls++;
	}

	assert(status == EIO);
	assert(firewall_calls == 0);
	assert(control_calls == 0);
	assert(dhcp_calls == 0);
}

static void
test_ndis_firewall_failure_reaches_startup_result(void)
{
	int startup_status = modem_wan_start_status(0, EIO);

	/* Later successful control and DHCP setup must not hide firewall failure. */
	assert(startup_status == EIO);
	startup_status = modem_wan_start_status(startup_status, 0);
	assert(startup_status == EIO);

	/* A later DHCP failure must not overwrite the earlier firewall error. */
	startup_status = modem_wan_start_status(startup_status, ETIMEDOUT);
	assert(startup_status == EIO);

	/* Without a firewall failure, DHCP's own failure is still returned. */
	assert(modem_wan_start_status(0, ETIMEDOUT) == ETIMEDOUT);
}

static void
test_ndis_optional_control_path_does_not_block_dhcp(void)
{
	int control_required = 0;
	int control_status = 1; /* No user-space control operation was performed. */
	int dhcp_calls = 0;

	if (!modem_ndis_control_failed(control_required, control_status))
		dhcp_calls++;

	assert(dhcp_calls == 1);
	assert(!modem_ndis_control_failed(control_required, control_status));
}

static void
test_ndis_required_control_failure_blocks_dhcp(void)
{
	int control_required = 1;
	int control_status = EIO;
	int startup_status = 0;
	int dhcp_calls = 0;

	if (modem_ndis_control_failed(control_required, control_status))
		startup_status = modem_wan_start_status(0, control_status);
	else
		dhcp_calls++;

	assert(startup_status == EIO);
	assert(dhcp_calls == 0);
	assert(modem_ndis_control_failed(1, -1));
}

static void
test_ndis_required_control_success_allows_dhcp(void)
{
	assert(!modem_ndis_control_failed(1, 0));
}

static void
test_first_modem_error_survives_best_effort_followup(void)
{
	int status = 0;
	int dhcp_attempts = 0;
	int passthrough_attempts = 0;
	int migration_status = ENOENT;

	if (modem_wan_start_migration_succeeded(migration_status))
		dhcp_attempts++;
	status = rc_first_service_start_status(
		status, modem_wan_start_status(migration_status, 0));
	passthrough_attempts++;
	status = rc_first_service_start_status(status, 0);

	assert(dhcp_attempts == 0);
	assert(passthrough_attempts == 1);
	assert(status == ENOENT);
}

static void
test_first_wan_error_survives_usb_modem_and_later_wan_failures(void)
{
	int status = 0;

	/* A preflight/NAT error must survive a later USB modem and passthrough failure. */
	status = rc_first_service_start_status(status, ENOMEM);
	status = rc_first_service_start_status(status, EBUSY);  /* NDIS/RAS startup */
	status = rc_first_service_start_status(status, EIO);    /* later modem action */
	status = rc_first_service_start_status(status, EACCES); /* final WAN adjunct */
	assert(status == ENOMEM);

	/* Without an earlier WAN error, the first modem error remains authoritative. */
	status = 0;
	status = rc_first_service_start_status(status, EBUSY);  /* RAS firewall */
	status = rc_first_service_start_status(status, EIO);    /* RAS client launch */
	status = rc_first_service_start_status(status, EINVAL); /* passthrough */
	assert(status == EBUSY);
}

static void
test_early_wan_abort_preserves_first_error(void)
{
	int status;

	/* Callback storage failure must not overwrite an earlier WAN sysctl error. */
	status = rc_first_service_start_status(0, ENOMEM);
	status = rc_first_service_start_status(status, EACCES);
	assert(status == ENOMEM);
	assert(rc_first_service_start_status(0, EACCES) == EACCES);

	/* VLAN configuration abort follows the same chronological rule. */
	status = rc_first_service_start_status(0, ENOMEM);
	status = rc_first_service_start_status(status, EIO);
	assert(status == ENOMEM);
	assert(rc_first_service_start_status(0, EIO) == EIO);

	/* AP-client migration failure must retain the earliest startup error too. */
	status = rc_first_service_start_status(0, ENOMEM);
	status = rc_first_service_start_status(status, ENETDOWN);
	assert(status == ENOMEM);
	assert(rc_first_service_start_status(0, ENETDOWN) == ENETDOWN);
}

static void
test_ras_physical_setup_error_survives_ras_launch(void)
{
	int status = 0;
	int wanx_status = EBUSY;
	int ras_launch_status = EIO;
	int ras_launch_attempts = 0;

	status = rc_first_service_start_status(status, wanx_status);
	/* RAS startup remains best-effort after physical setup reports failure. */
	ras_launch_attempts++;
	status = rc_first_service_start_status(status, ras_launch_status);

	assert(ras_launch_attempts == 1);
	assert(status == EBUSY);
}

static void
test_manual_pppoe_physical_setup_failure_is_reported_without_blocking_pppd(void)
{
	int status = 0;
	int is_pppoe = 1;
	int wanx_status = EINVAL;
	int pppd_status = 0;
	int pppd_attempts = 0;

	/* A configured MAN failure is reportable, but PPPoE does not need its
	 * physical IPv4 lease and must retain the existing best-effort PPP attempt. */
	if (wanx_status != 0)
		status = rc_first_service_start_status(status, wanx_status);
	if (!is_pppoe && wanx_status != 0)
		return;

	pppd_attempts++;
	status = rc_first_service_start_status(status, pppd_status);

	assert(pppd_attempts == 1);
	assert(status == EINVAL);
	assert(rc_first_service_start_status(EBUSY, wanx_status) == EBUSY);
}

static void
test_physical_wan_ifup_failure_survives_cleanup(void)
{
	int status = 0;
	int interface_status = ENODEV;
	int cleanup_status = EIO;

	status = rc_first_service_start_status(status, interface_status);
	/* Cleanup is best-effort and must not overwrite the original IFUP error. */
	status = rc_first_service_start_status(status, cleanup_status);

	assert(status == ENODEV);
}

static void
test_required_ppp_failures_reach_wan_start_status(void)
{
	int status = 0;

	status = rc_first_service_start_status(status, EBUSY); /* UPnP migration */
	status = rc_first_service_start_status(status, ENODEV); /* PPTP/L2TP physical WAN */
	status = rc_first_service_start_status(status, EIO); /* pppd launch */
	status = rc_first_service_start_status(status, ETIMEDOUT); /* demand readiness */
	status = rc_first_service_start_status(status, EACCES); /* best-effort restore */

	assert(status == EBUSY);
}

static void
test_required_ipoe_failures_reach_wan_start_status(void)
{
	int status = 0;

	status = rc_first_service_start_status(status, EBUSY); /* UPnP migration */
	status = rc_first_service_start_status(status, EADDRNOTAVAIL); /* static IPv4 address */
	status = rc_first_service_start_status(status, EACCES); /* EAPOL */
	status = rc_first_service_start_status(status, EIO); /* DHCP client */

	assert(status == EBUSY);
}

static void
test_wan_adjunct_service_failures_reach_start_status(void)
{
	int status = 0;
	int iptv_status = ENODEV;
	int passthrough_status = EIO;

	status = rc_first_service_start_status(status, iptv_status);
	status = rc_first_service_start_status(status, passthrough_status);

	assert(status == ENODEV);
	assert(rc_first_service_start_status(0, passthrough_status) == EIO);
}

static void
test_static_ipoe_upnp_reload_failure_reaches_start_status(void)
{
	int status = 0;
	int wan_up_status = 0;
	int wan_upnp_status = EIO;

	status = rc_first_service_start_status(status, wan_up_status);
	status = rc_first_service_start_status(status, wan_upnp_status);

	assert(status == EIO);
	assert(rc_first_service_start_status(ENETUNREACH, wan_upnp_status) == ENETUNREACH);
}

static void
test_unplug_ifdown_failure_is_distinguished_from_removed_interface(void)
{
	assert(modem_wan_unplug_interface_down_result(0, -1, ENODEV, ENODEV) == 0);
	assert(modem_wan_unplug_interface_down_result(EBUSY, 0, 0, ENODEV) == EBUSY);
	assert(modem_wan_unplug_interface_down_result(-1, 1, 0, ENODEV) == -1);
	assert(modem_wan_unplug_interface_down_result(EIO, -1, ENODEV, ENODEV) == 0);
	assert(modem_wan_unplug_interface_down_result(EIO, -1, EACCES, ENODEV) == EACCES);
	assert(modem_wan_unplug_interface_down_result(EIO, -1, 0, ENODEV) == EIO);
}

static void
test_hotplug_notification_queue_failure_reaches_handler_result(void)
{
	assert(modem_wan_hotplug_notification_result(0, 0) == 0);
	assert(modem_wan_hotplug_notification_result(0, -1) == 1);
	assert(modem_wan_hotplug_notification_result(0, EIO) == 1);
	assert(modem_wan_hotplug_notification_result(1, 0) == 1);
	assert(modem_wan_hotplug_notification_result(1, -1) == 1);
	assert(modem_wan_hotplug_notification_result(7, EIO) == 7);
}

static void
test_modem_hotplug_retry_mask_preserves_failed_and_coalesced_events(void)
{
	unsigned int retry_mask;

	retry_mask = modem_wan_hotplug_retry_mask(0, 0, 0, 0);
	assert(retry_mask == 0);

	retry_mask = modem_wan_hotplug_retry_mask(1, 0, EIO, 0);
	assert(retry_mask == MODEM_WAN_HOTPLUG_RETRY_UNPLUG);

	retry_mask = modem_wan_hotplug_retry_mask(0, 1, 0, EIO);
	assert(retry_mask == MODEM_WAN_HOTPLUG_RETRY_PLUG);

	/* A simultaneous replug waits even when unplug's reconnect succeeds. */
	retry_mask = modem_wan_hotplug_retry_mask(1, 1, 0, 0);
	assert(retry_mask == MODEM_WAN_HOTPLUG_RETRY_PLUG);

	/* If unplug also fails, keep both independent events pending. */
	retry_mask = modem_wan_hotplug_retry_mask(1, 1, EBUSY, EIO);
	assert(retry_mask ==
	       (MODEM_WAN_HOTPLUG_RETRY_UNPLUG |
		MODEM_WAN_HOTPLUG_RETRY_PLUG));
}

static void
test_static_wan_address_and_netmask_validation(void)
{
	assert(dhcp_lease_ipv4_is_valid("192.0.2.10"));
	assert(!dhcp_lease_ipv4_is_valid("not-an-ip"));
	assert(!dhcp_lease_ipv4_is_valid("192.0.2.999"));
	assert(!dhcp_lease_ipv4_is_valid("0.0.0.0"));
	assert(dhcp_lease_netmask_is_valid("255.255.255.0"));
	assert(dhcp_lease_netmask_is_valid("255.255.255.255"));
	assert(!dhcp_lease_netmask_is_valid("255.0.255.0"));
	assert(!dhcp_lease_netmask_is_valid("255.255.255.1"));
	assert(!dhcp_lease_netmask_is_valid("255.255.255.256"));
	assert(!dhcp_lease_netmask_is_valid("0.0.0.0"));
}


struct fake_ndis_identity_writer {
	int fail_call;
	int failure_status;
	int cleanup_fail_call;
	int cleanup_failure_status;
	unsigned int calls;
	char names[4][16];
	char values[4][32];
};

static int
fake_ndis_identity_set(void *opaque, const char *param_name, const char *value)
{
	struct fake_ndis_identity_writer *fake =
		(struct fake_ndis_identity_writer *)opaque;
	unsigned int index = fake->calls++;

	assert(index < 4);
	snprintf(fake->names[index], sizeof(fake->names[index]), "%s", param_name);
	snprintf(fake->values[index], sizeof(fake->values[index]), "%s", value);
	if ((int)index == fake->fail_call)
		return fake->failure_status;
	if ((int)index == fake->cleanup_fail_call)
		return fake->cleanup_failure_status;
	return 0;
}

static void
test_ndis_identity_clear_removes_both_fields(void)
{
	struct fake_ndis_identity_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = -1;
	fake.cleanup_fail_call = -1;
	assert(modem_wan_clear_ndis_identity(fake_ndis_identity_set, &fake) == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.names[0], "ifname_t") == 0);
	assert(strcmp(fake.values[0], "") == 0);
	assert(strcmp(fake.names[1], "proto_t") == 0);
	assert(strcmp(fake.values[1], "") == 0);
}

static void
test_ndis_identity_clear_attempts_protocol_after_ifname_failure(void)
{
	struct fake_ndis_identity_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = 0;
	fake.failure_status = EIO;
	fake.cleanup_fail_call = 1;
	fake.cleanup_failure_status = EPERM;
	assert(modem_wan_clear_ndis_identity(fake_ndis_identity_set, &fake) == EIO);
	assert(fake.calls == 2);
	assert(strcmp(fake.names[0], "ifname_t") == 0);
	assert(strcmp(fake.names[1], "proto_t") == 0);
}

static void
test_ndis_identity_clear_reports_second_field_failure(void)
{
	struct fake_ndis_identity_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = 1;
	fake.failure_status = ENOSPC;
	fake.cleanup_fail_call = -1;
	assert(modem_wan_clear_ndis_identity(fake_ndis_identity_set, &fake) == ENOSPC);
	assert(fake.calls == 2);
}

static void
test_ndis_identity_clear_rejects_missing_writer(void)
{
	assert(modem_wan_clear_ndis_identity(NULL, NULL) == EINVAL);
}

static void
test_ndis_identity_is_published_before_interface_activation(void)
{
	struct fake_ndis_identity_writer fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = -1;
	fake.cleanup_fail_call = -1;
	assert(modem_wan_publish_ndis_identity(
		fake_ndis_identity_set, &fake, "wwan0", &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.names[0], "proto_t") == 0);
	assert(strcmp(fake.values[0], "NDIS Modem") == 0);
	assert(strcmp(fake.names[1], "ifname_t") == 0);
	assert(strcmp(fake.values[1], "wwan0") == 0);
}

static void
test_ndis_identity_failure_clears_both_fields_and_preserves_primary_error(void)
{
	struct fake_ndis_identity_writer fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = 1; /* ifname_t publication */
	fake.failure_status = EIO;
	fake.cleanup_fail_call = 2; /* ifname_t clear */
	fake.cleanup_failure_status = EPERM;

	assert(modem_wan_publish_ndis_identity(
		fake_ndis_identity_set, &fake, "wwan0", &rollback_status) == EIO);
	assert(rollback_status == EPERM);
	assert(fake.calls == 4);
	assert(strcmp(fake.names[0], "proto_t") == 0);
	assert(strcmp(fake.names[1], "ifname_t") == 0);
	assert(strcmp(fake.names[2], "ifname_t") == 0);
	assert(strcmp(fake.values[2], "") == 0);
	assert(strcmp(fake.names[3], "proto_t") == 0);
	assert(strcmp(fake.values[3], "") == 0);
}

static void
test_ndis_identity_first_write_failure_still_clears_stale_fields(void)
{
	struct fake_ndis_identity_writer fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = 0;
	fake.failure_status = ENOSPC;
	fake.cleanup_fail_call = -1;

	assert(modem_wan_publish_ndis_identity(
		fake_ndis_identity_set, &fake, "wwan0", &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(fake.calls == 3);
	assert(strcmp(fake.names[1], "ifname_t") == 0);
	assert(strcmp(fake.values[1], "") == 0);
	assert(strcmp(fake.names[2], "proto_t") == 0);
	assert(strcmp(fake.values[2], "") == 0);
}

static void
test_ndis_identity_rejects_invalid_arguments_before_writes(void)
{
	struct fake_ndis_identity_writer fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	fake.fail_call = -1;
	assert(modem_wan_publish_ndis_identity(
		NULL, &fake, "wwan0", &rollback_status) == EINVAL);
	assert(modem_wan_publish_ndis_identity(
		fake_ndis_identity_set, &fake, NULL, &rollback_status) == EINVAL);
	assert(modem_wan_publish_ndis_identity(
		fake_ndis_identity_set, &fake, "", &rollback_status) == EINVAL);
	assert(rollback_status == 0);
	assert(fake.calls == 0);
}

static void
test_modem_unplug_rule_read_fails_closed(void)
{
	int unplug_required = -1;

	/* A missing key retains the historical modem_rule == 0 behavior. */
	assert(modem_wan_unplug_event_status(0, 0, &unplug_required) == 0);
	assert(unplug_required == 0);
	assert(modem_wan_unplug_event_status(0, 1, &unplug_required) == 0);
	assert(unplug_required == 1);
	assert(modem_wan_unplug_event_status(0, -1, &unplug_required) == 0);
	assert(unplug_required == 0);

	/* Failed reads never masquerade as "no configured modem". */
	assert(modem_wan_unplug_event_status(EIO, 1, &unplug_required) == EIO);
	assert(unplug_required == 0);
	assert(modem_wan_unplug_event_status(-1, 1, &unplug_required) == EIO);
	assert(unplug_required == 0);
	assert(modem_wan_unplug_event_status(0, 1, NULL) == EINVAL);
}


static void
test_unplug_notification_is_conservative_on_unknown_identity(void)
{
	assert(!modem_wan_unplug_notification_required(0, 0));
	assert(modem_wan_unplug_notification_required(0, 1));
	assert(modem_wan_unplug_notification_required(EIO, 0));
	assert(modem_wan_unplug_notification_required(ENOSPC, 0));
	assert(modem_wan_unplug_notification_required(ENOSPC, 1));
}

static void
test_modem_auto_start_requires_known_non_modem_identity(void)
{
	assert(modem_wan_auto_start_allowed(0, 0));
	assert(!modem_wan_auto_start_allowed(0, 1));
	assert(!modem_wan_auto_start_allowed(EIO, 0));
	assert(!modem_wan_auto_start_allowed(ENOSPC, 0));
	assert(!modem_wan_auto_start_allowed(ENOSPC, 1));
}


static void
test_modem_restart_requires_known_identity(void)
{
	assert(modem_wan_restart_allowed(0));
	assert(!modem_wan_restart_allowed(EIO));
	assert(!modem_wan_restart_allowed(ENOSPC));
}

int
main(void)
{
	test_link_change_reconnect_requires_checked_modem_state();
	test_hotplug_notification_preserves_ndis_and_ras_legacy_gates();
	test_hotplug_notification_missing_any_legacy_match_key_stays_disabled();
	test_hotplug_notification_rejects_invalid_and_unreadable_inputs();
	test_miniupnp_migration_failure_blocks_dhcp();
	test_modem_selection_preserves_missing_priority_default();
	test_modem_selection_fails_closed_on_priority_read_error();
	test_modem_selection_fails_closed_on_unknown_ethernet_link();
	test_modem_link_probe_error_fails_closed();
	test_modem_selection_prefers_wired_link_when_present();
	test_modem_selection_uses_modem_only_after_confirmed_link_absence();
	test_modem_selection_keeps_wisp_compatibility_without_link_probe();
	test_modem_selection_skips_priority_read_when_no_modem_is_present();
	test_modem_selection_rejects_invalid_probe_and_output();
	test_dhcp_launch_result_is_returned_after_migration();
	test_ras_peer_file_cleanup_only_after_failed_launch();
	test_ndis_interface_bringup_failure_skips_network_start();
	test_ndis_identity_clear_removes_both_fields();
	test_ndis_identity_clear_attempts_protocol_after_ifname_failure();
	test_ndis_identity_clear_reports_second_field_failure();
	test_ndis_identity_clear_rejects_missing_writer();
	test_ndis_identity_is_published_before_interface_activation();
	test_ndis_identity_failure_clears_both_fields_and_preserves_primary_error();
	test_ndis_identity_first_write_failure_still_clears_stale_fields();
	test_ndis_identity_rejects_invalid_arguments_before_writes();
	test_ndis_firewall_failure_reaches_startup_result();
	test_ndis_optional_control_path_does_not_block_dhcp();
	test_ndis_required_control_failure_blocks_dhcp();
	test_ndis_required_control_success_allows_dhcp();
	test_first_modem_error_survives_best_effort_followup();
	test_first_wan_error_survives_usb_modem_and_later_wan_failures();
	test_early_wan_abort_preserves_first_error();
	test_ras_physical_setup_error_survives_ras_launch();
	test_manual_pppoe_physical_setup_failure_is_reported_without_blocking_pppd();
	test_physical_wan_ifup_failure_survives_cleanup();
	test_required_ppp_failures_reach_wan_start_status();
	test_required_ipoe_failures_reach_wan_start_status();
	test_wan_adjunct_service_failures_reach_start_status();
	test_static_ipoe_upnp_reload_failure_reaches_start_status();
	test_hotplug_notification_queue_failure_reaches_handler_result();
	test_unplug_notification_is_conservative_on_unknown_identity();
	test_modem_auto_start_requires_known_non_modem_identity();
	test_modem_restart_requires_known_identity();
	test_modem_hotplug_retry_mask_preserves_failed_and_coalesced_events();
	test_modem_unplug_rule_read_fails_closed();
	test_unplug_ifdown_failure_is_distinguished_from_removed_interface();
	test_static_wan_address_and_netmask_validation();

	puts("USB modem WAN startup status policy: PASS");
	return 0;
}
