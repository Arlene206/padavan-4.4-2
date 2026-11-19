#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "service_start_status.h"
#include "optional_daemon_policy.h"
#include "wan_mtu_setup_policy.h"
#include "rc_notify_batch_policy.h"


static void
test_optional_daemon_start_preflight_is_idempotent_and_fail_closed(void)
{
	/* Only a complete ABSENT scan allows launch; RUNNING is a no-op and
	 * UNKNOWN must fail closed before a second scheduler can be spawned. */
	assert(optional_daemon_start_allowed(PROCESS_PRESENCE_ABSENT));
	assert(!optional_daemon_start_allowed(PROCESS_PRESENCE_RUNNING));
	assert(!optional_daemon_start_allowed(PROCESS_PRESENCE_UNKNOWN));
	assert(!optional_daemon_start_allowed((enum process_presence)99));
}

static void
test_wan_firewall_failure_blocks_protocol_launch(void)
{
	const int failed_statuses[] = { -1, EIO, EBUSY, EACCES };
	int launch_attempts = 0;
	int i;

	if (rc_wan_firewall_start_allows_protocol(0))
		launch_attempts++;
	assert(launch_attempts == 1);

	for (i = 0; i < (int)(sizeof(failed_statuses) / sizeof(failed_statuses[0])); i++) {
		launch_attempts = 0;
		if (rc_wan_firewall_start_allows_protocol(failed_statuses[i]))
			launch_attempts++;
		assert(launch_attempts == 0);
	}
}

static void
test_static_ipoe_eapol_cleanup_failure_forces_interface_down(void)
{
	const int clear_failures[] = { -1, EIO, EBUSY };
	int status;
	int down_attempts;
	int i;

	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(0, EIO));
	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(1, 0));

	for (i = 0; i < (int)(sizeof(clear_failures) / sizeof(clear_failures[0])); i++) {
		status = EACCES;
		down_attempts = 0;

		/* Preserve authentication as the first error, but still force link down. */
		status = rc_first_service_start_status(status, clear_failures[i]);
		if (rc_wan_static_ipoe_clear_failure_requires_ifdown(1, clear_failures[i])) {
			down_attempts++;
			status = rc_first_service_start_status(status, EBUSY);
		}

		assert(down_attempts == 1);
		assert(status == EACCES);
	}
}

static void
test_static_ipoe_address_cleanup_failure_blocks_eapol(void)
{
	const int clear_failures[] = { -1, EIO, EBUSY };
	int status;
	int cleanup_failed;
	int down_attempts;
	int eapol_attempts;
	int i;

	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(0, EIO));
	assert(!rc_wan_static_ipoe_clear_failure_requires_ifdown(1, 0));

	for (i = 0; i < (int)(sizeof(clear_failures) / sizeof(clear_failures[0])); i++) {
		status = EADDRNOTAVAIL;
		cleanup_failed = 0;
		down_attempts = 0;
		eapol_attempts = 0;

		if (rc_wan_static_ipoe_clear_failure_requires_ifdown(1, clear_failures[i])) {
			cleanup_failed = 1;
			status = rc_first_service_start_status(status, clear_failures[i]);
			down_attempts++;
			/* Even a failed ifdown cannot erase the first address error. */
			status = rc_first_service_start_status(status, EIO);
		}
		if (!cleanup_failed)
			eapol_attempts++;

		assert(down_attempts == 1);
		assert(eapol_attempts == 0);
		assert(status == EADDRNOTAVAIL);
	}

	/* A successful address clear leaves the normal EAPOL path available. */
	cleanup_failed = rc_wan_static_ipoe_clear_failure_requires_ifdown(1, 0);
	eapol_attempts = cleanup_failed ? 0 : 1;
	assert(eapol_attempts == 1);
}

static void
test_wan_default_mtu_respects_static_custom_override(void)
{
	assert(wan_default_mtu_should_be_set(1, 1299));
	assert(!wan_default_mtu_should_be_set(1, 1300));
	assert(!wan_default_mtu_should_be_set(1, 1499));
	assert(wan_default_mtu_should_be_set(1, 1500));
	assert(wan_default_mtu_should_be_set(0, 1400));
	assert(wan_default_mtu_should_be_set(0, 0));
}

static void
test_wan_default_mtu_failure_is_reported_while_startup_continues(void)
{
	int status = 0;
	int physical_ifup_attempts = 0;
	int protocol_start_attempts = 0;

	/* set_interface_mtu() returns -1 for a failed ioctl. */
	status = rc_first_service_start_status(status, -1);
	physical_ifup_attempts++;
	status = rc_first_service_start_status(status, 0);
	protocol_start_attempts++;
	status = rc_first_service_start_status(status, EIO);

	assert(physical_ifup_attempts == 1);
	assert(protocol_start_attempts == 1);
	assert(status == -1);
}

static void
test_man_custom_mtu_failure_is_reported_and_setup_continues(void)
{
	int status = 0;
	int address_attempts = 0;
	int route_attempts = 0;

	/* A failed MAN MTU ioctl is retained while independent setup continues. */
	status = rc_first_service_start_status(status, -1);
	address_attempts++;
	status = rc_first_service_start_status(status, 0);
	route_attempts++;
	status = rc_first_service_start_status(status, EIO);

	assert(address_attempts == 1);
	assert(route_attempts == 1);
	assert(status == -1);
}

static void
test_first_error_survives_lan_restart_recovery(void)
{
	int status = 0;
	int stp_disable_attempts = 0;
	int forward_delay_attempts = 0;
	int power_down_attempts = 0;
	int power_up_attempts = 0;
	int lan_start_attempts = 0;
	int stp_restore_attempts = 0;
	int forward_delay_restore_attempts = 0;
	int wan_restart_attempts = 0;
	int vpn_start_attempts = 0;

	/* A failed STP change must remain visible while every recovery step runs. */
	stp_disable_attempts++;
	status = rc_first_service_start_status(status, EIO);
	forward_delay_attempts++;
	status = rc_first_service_start_status(status, 0);
	power_down_attempts++;
	status = rc_first_service_start_status(status, EBUSY);
	power_up_attempts++;
	status = rc_first_service_start_status(status, 0);
	lan_start_attempts++;
	status = rc_first_service_start_status(status, 0);
	stp_restore_attempts++;
	status = rc_first_service_start_status(status, EACCES);
	forward_delay_restore_attempts++;
	status = rc_first_service_start_status(status, 0);
	wan_restart_attempts++;
	status = rc_first_service_start_status(status, ENETUNREACH);
	vpn_start_attempts++;
	status = rc_first_service_start_status(status, 0);

	assert(stp_disable_attempts == 1);
	assert(forward_delay_attempts == 1);
	assert(power_down_attempts == 1);
	assert(power_up_attempts == 1);
	assert(lan_start_attempts == 1);
	assert(stp_restore_attempts == 1);
	assert(forward_delay_restore_attempts == 1);
	assert(wan_restart_attempts == 1);
	assert(vpn_start_attempts == 1);
	assert(status == EIO);

	/* When earlier setup succeeds, a later PHY recovery error becomes first. */
	status = rc_first_service_start_status(0, 0);
	status = rc_first_service_start_status(status, 0);
	status = rc_first_service_start_status(status, EBUSY);
	status = rc_first_service_start_status(status, 0);
	status = rc_first_service_start_status(status, EIO);
	assert(status == EBUSY);
}

static void
test_negative_and_positive_statuses_are_preserved(void)
{
	int status = rc_first_service_start_status(0, -1);

	assert(status == -1);
	status = rc_first_service_start_status(0, 17);
	status = rc_first_service_start_status(status, EIO);
	assert(status == 17);
}

static void
test_lan_start_preserves_first_required_failure(void)
{
	int status = 0;
	int ifup_attempts = 0;
	int dhcp_attempts = 0;
	int route_attempts = 0;

	/* A later DHCP success must not erase an earlier bridge setup failure. */
	ifup_attempts++;
	status = rc_first_service_start_status(status, ENODEV);
	dhcp_attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(ifup_attempts == 1);
	assert(dhcp_attempts == 1);
	assert(status == ENODEV);

	/* Router-mode static-route failures must be observable too. */
	status = 0;
	route_attempts++;
	status = rc_first_service_start_status(status, EIO);
	assert(route_attempts == 1);
	assert(status == EIO);
}

static void
test_router_lan_teardown_preserves_route_failure_and_continues(void)
{
	int status = 0;
	int conntrack_status = 0;
	int route_status = EIO;
	int bridge_status = 0;
	int route_flush_attempts = 0;
	int bridge_down_attempts = 0;

	status = rc_first_service_start_status(status, conntrack_status);
	route_flush_attempts++;
	status = rc_first_service_start_status(status, route_status);
	bridge_down_attempts++;
	status = rc_first_service_start_status(status, bridge_status);
	assert(route_flush_attempts == 1);
	assert(bridge_down_attempts == 1);
	assert(status == EIO);

	/* Conntrack remains the first failure, but route and bridge cleanup run. */
	status = 0;
	conntrack_status = EBUSY;
	route_status = EIO;
	bridge_status = EACCES;
	route_flush_attempts = 0;
	bridge_down_attempts = 0;
	status = rc_first_service_start_status(status, conntrack_status);
	route_flush_attempts++;
	status = rc_first_service_start_status(status, route_status);
	bridge_down_attempts++;
	status = rc_first_service_start_status(status, bridge_status);
	assert(route_flush_attempts == 1);
	assert(bridge_down_attempts == 1);
	assert(status == EBUSY);

	/* A successful route flush must not hide a later bridge-down failure. */
	status = 0;
	conntrack_status = 0;
	route_status = 0;
	bridge_status = EACCES;
	status = rc_first_service_start_status(status, conntrack_status);
	status = rc_first_service_start_status(status, route_status);
	status = rc_first_service_start_status(status, bridge_status);
	assert(status == EACCES);
}

static void
test_router_lan_teardown_preserves_ipv6_clear_failure_and_continues(void)
{
	int status = 0;
	int conntrack_status = 0;
	int route_status = 0;
	int ipv6_status = EIO;
	int bridge_status = 0;
	int ipv6_clear_attempts = 0;
	int bridge_down_attempts = 0;

	status = rc_first_service_start_status(status, conntrack_status);
	status = rc_first_service_start_status(status, route_status);
	ipv6_clear_attempts++;
	status = rc_first_service_start_status(status, ipv6_status);
	bridge_down_attempts++;
	status = rc_first_service_start_status(status, bridge_status);
	assert(ipv6_clear_attempts == 1);
	assert(bridge_down_attempts == 1);
	assert(status == EIO);

	/* An earlier route error remains first; later cleanup still runs. */
	status = 0;
	conntrack_status = 0;
	route_status = EBUSY;
	ipv6_status = EIO;
	bridge_status = EACCES;
	ipv6_clear_attempts = 0;
	bridge_down_attempts = 0;
	status = rc_first_service_start_status(status, conntrack_status);
	status = rc_first_service_start_status(status, route_status);
	ipv6_clear_attempts++;
	status = rc_first_service_start_status(status, ipv6_status);
	bridge_down_attempts++;
	status = rc_first_service_start_status(status, bridge_status);
	assert(ipv6_clear_attempts == 1);
	assert(bridge_down_attempts == 1);
	assert(status == EBUSY);

	/* A bridge-down-only error remains visible after successful cleanup. */
	status = 0;
	conntrack_status = 0;
	route_status = 0;
	ipv6_status = 0;
	bridge_status = EACCES;
	status = rc_first_service_start_status(status, conntrack_status);
	status = rc_first_service_start_status(status, route_status);
	status = rc_first_service_start_status(status, ipv6_status);
	status = rc_first_service_start_status(status, bridge_status);
	assert(status == EACCES);
}

static void
test_loopback_start_preserves_first_required_failure(void)
{
	int status = 0;
	int interface_attempts = 0;
	int route_attempts = 0;

	/* Route setup must still run after the loopback interface operation fails. */
	interface_attempts++;
	status = rc_first_service_start_status(status, ENETDOWN);
	route_attempts++;
	status = rc_first_service_start_status(status, EIO);
	assert(interface_attempts == 1);
	assert(route_attempts == 1);
	assert(status == ENETDOWN);

	/* A route-only failure must also be returned. */
	status = 0;
	status = rc_first_service_start_status(status, 0);
	status = rc_first_service_start_status(status, EIO);
	assert(status == EIO);
}

static void
test_usb_printer_launchers_preserve_first_failure_and_continue(void)
{
	int status = 0;
	int attempts = 0;

	/* A failed per-device launcher must not prevent shared spoolers starting. */
	attempts++;
	status = rc_first_service_start_status(status, EIO);
	attempts++;
	status = rc_first_service_start_status(status, EACCES);
	attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(attempts == 3);
	assert(status == EIO);

	/* A shared-spooler-only failure must also reach the caller. */
	status = 0;
	attempts = 0;
	attempts++;
	status = rc_first_service_start_status(status, 0);
	attempts++;
	status = rc_first_service_start_status(status, EBUSY);
	assert(attempts == 2);
	assert(status == EBUSY);
}

static void
test_usb_printer_optional_hook_failure_is_reported_and_core_launches_continue(void)
{
	int status = 0;
	int attempts = 0;

	/* An optional hook failure is reported while core spooler launches continue. */
	attempts++;
	status = rc_first_service_start_status(status, EACCES);
	attempts++;
	status = rc_first_service_start_status(status, EIO);
	attempts++;
	status = rc_first_service_start_status(status, 0);
	attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(attempts == 4);
	assert(status == EACCES);
}

static void
test_storage_app_startup_preserves_first_failure_and_continues(void)
{
	int status = 0;
	int attempts = 0;

	/* FTP failure must not suppress Samba/NFS, and remains the first error. */
	attempts++;
	status = rc_first_service_start_status(status, EIO);
	attempts++;
	status = rc_first_service_start_status(status, 0);
	attempts++;
	status = rc_first_service_start_status(status, EBUSY);
	assert(attempts == 3);
	assert(status == EIO);

	/* Samba-only and NFS-only failures must also reach the caller. */
	status = 0;
	attempts = 0;
	attempts++;
	status = rc_first_service_start_status(status, 0);
	attempts++;
	status = rc_first_service_start_status(status, EACCES);
	attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(attempts == 3);
	assert(status == EACCES);

	status = 0;
	attempts = 0;
	attempts++;
	status = rc_first_service_start_status(status, 0);
	attempts++;
	status = rc_first_service_start_status(status, 0);
	attempts++;
	status = rc_first_service_start_status(status, ENODEV);
	assert(attempts == 3);
	assert(status == ENODEV);

	/* Optional media/download daemons also feed the shared startup status. */
	status = 0;
	attempts = 0;
	attempts++;
	status = rc_first_service_start_status(status, 0);       /* FTP */
	attempts++;
	status = rc_first_service_start_status(status, 0);       /* Samba */
	attempts++;
	status = rc_first_service_start_status(status, 0);       /* NFS */
	attempts++;
	status = rc_first_service_start_status(status, EBUSY);   /* MiniDLNA */
	attempts++;
	status = rc_first_service_start_status(status, 0);       /* Firefly */
	attempts++;
	status = rc_first_service_start_status(status, ENODEV);  /* Transmission */
	attempts++;
	status = rc_first_service_start_status(status, EACCES);  /* Aria */
	assert(attempts == 7);
	assert(status == EBUSY);
}

static void
test_storage_event_remains_pending_after_failure(void)
{
	int pending = 1;

	if (rc_service_start_event_is_complete(EIO))
		pending = 0;
	assert(pending == 1);

	if (rc_service_start_event_is_complete(EBUSY))
		pending = 0;
	assert(pending == 1);

	if (rc_service_start_event_is_complete(0))
		pending = 0;
	assert(pending == 0);
	assert(!rc_service_start_event_is_complete(-1));
}

static void
test_bridge_parameters_are_both_attempted_and_first_error_survives(void)
{
	int status = 0;
	int stp_attempts = 0;
	int forward_delay_attempts = 0;

	/* STP failure must not skip the independent forward-delay write. */
	stp_attempts++;
	status = rc_first_service_start_status(status, EIO);
	forward_delay_attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(stp_attempts == 1);
	assert(forward_delay_attempts == 1);
	assert(status == EIO);

	/* A forward-delay-only failure is also returned. */
	status = 0;
	stp_attempts = 0;
	forward_delay_attempts = 0;
	stp_attempts++;
	status = rc_first_service_start_status(status, 0);
	forward_delay_attempts++;
	status = rc_first_service_start_status(status, EINVAL);
	assert(stp_attempts == 1);
	assert(forward_delay_attempts == 1);
	assert(status == EINVAL);
}


static void
test_ap_lan_monitor_resume_failure_preserves_primary_teardown_status(void)
{
	int status;
	int monitor_resume_status = EIO;

	/* A DHCP preflight failure remains primary when monitor resume also fails. */
	status = rc_first_service_start_status(EBUSY, monitor_resume_status);
	assert(status == EBUSY);

	/* A bridge-down failure remains primary when monitor resume also fails. */
	status = rc_first_service_start_status(EACCES, monitor_resume_status);
	assert(status == EACCES);

	/* With no earlier failure, a monitor-resume error is still reportable. */
	status = rc_first_service_start_status(0, monitor_resume_status);
	assert(status == EIO);
}

static void
test_ap_lan_dhcp_start_depends_on_phy_wait_not_monitor(void)
{
	int phy_wait_status = ETIMEDOUT;
	int monitor_status = 0;
	int start_status = 0;
	int launch_attempts = 0;

	/* A successful monitor notification must not hide a failed PHY wait. */
	start_status = rc_first_service_start_status(start_status, phy_wait_status);
	start_status = rc_first_service_start_status(start_status, monitor_status);
	if (rc_lan_ap_dhcp_start_allowed_after_phy_wait(phy_wait_status))
		launch_attempts++;
	assert(start_status == ETIMEDOUT);
	assert(launch_attempts == 0);

	/* A monitor failure is reported, but must not suppress a PHY-ready client. */
	phy_wait_status = 0;
	monitor_status = EIO;
	start_status = 0;
	launch_attempts = 0;
	start_status = rc_first_service_start_status(start_status, phy_wait_status);
	start_status = rc_first_service_start_status(start_status, monitor_status);
	if (rc_lan_ap_dhcp_start_allowed_after_phy_wait(phy_wait_status))
		launch_attempts++;
	assert(start_status == EIO);
	assert(launch_attempts == 1);
}

static void
test_sysctl_pppoe_relay_failure_keeps_notification_pending(void)
{
	int restart_status = 0;
	int action_result = 0;
	int samba_attempts = 0;

	/* A positive errno from PPPoE relay must survive later independent work. */
	restart_status = rc_first_service_start_status(restart_status, EBUSY);
	samba_attempts++;
	restart_status = rc_first_service_start_status(restart_status, 0);
	assert(samba_attempts == 1);
	assert(restart_status == EBUSY);

	/* PID 1 converts any nonzero service status into an unacknowledged event. */
	action_result = rc_notify_merge_action_result(action_result, restart_status);
	assert(action_result < 0);
	assert(rc_notify_merge_action_result(0, 0) == 0);
}

static void
test_crond_positive_restart_error_reaches_boot_and_notification_status(void)
{
	int boot_status = 0;
	int action_result = 0;

	/* crond startup returns errno-style positive failures as well as -1. */
	boot_status = rc_first_service_start_status(boot_status, EIO);
	action_result = rc_notify_merge_action_result(action_result, EIO);
	assert(boot_status == EIO);
	assert(action_result == -1);

	/* The first boot error remains authoritative when crond also fails. */
	boot_status = rc_first_service_start_status(ENETDOWN, EIO);
	assert(boot_status == ENETDOWN);

	/* Disabled/no-op crond restart is normalized to zero by start_crond(). */
	assert(rc_first_service_start_status(0, 0) == 0);
	assert(rc_notify_merge_action_result(0, 0) == 0);
}

int
main(void)
{
	test_optional_daemon_start_preflight_is_idempotent_and_fail_closed();
	test_crond_positive_restart_error_reaches_boot_and_notification_status();
	assert(rc_first_service_start_status(0, 0) == 0);
	assert(rc_first_service_start_status(0, -1) == -1);
	assert(rc_first_service_start_status(0, 17) == 17);
	assert(rc_first_service_start_status(-1, 0) == -1);
	assert(rc_first_service_start_status(17, -1) == 17);
	assert(rc_first_service_start_status(-5, 9) == -5);
	assert(rc_first_service_start_status(4, 0) == 4);
	test_wan_firewall_failure_blocks_protocol_launch();
	test_static_ipoe_eapol_cleanup_failure_forces_interface_down();
	test_static_ipoe_address_cleanup_failure_blocks_eapol();
	test_wan_default_mtu_respects_static_custom_override();
	test_wan_default_mtu_failure_is_reported_while_startup_continues();
	test_man_custom_mtu_failure_is_reported_and_setup_continues();
	test_first_error_survives_lan_restart_recovery();
	test_router_lan_teardown_preserves_route_failure_and_continues();
	test_router_lan_teardown_preserves_ipv6_clear_failure_and_continues();
	test_negative_and_positive_statuses_are_preserved();
	test_usb_printer_launchers_preserve_first_failure_and_continue();
	test_usb_printer_optional_hook_failure_is_reported_and_core_launches_continue();
	test_bridge_parameters_are_both_attempted_and_first_error_survives();
	test_ap_lan_monitor_resume_failure_preserves_primary_teardown_status();
	test_ap_lan_dhcp_start_depends_on_phy_wait_not_monitor();
	test_sysctl_pppoe_relay_failure_keeps_notification_pending();
	test_storage_app_startup_preserves_first_failure_and_continues();
	test_storage_event_remains_pending_after_failure();
	test_loopback_start_preserves_first_required_failure();
	test_lan_start_preserves_first_required_failure();
	puts("RC boot, WAN firewall, static IPoE address/EAPOL cleanup and LAN first-failure policy: PASS");
	return 0;
}
