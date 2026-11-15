#!/usr/bin/env python3
"""Supplemental integration checks for process-inventory production wiring."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
shared_makefile = (root / "trunk/user/shared/src/Makefile").read_text()
shared_header = (root / "trunk/user/shared/src/shutils.h").read_text()
pids_source = (root / "trunk/user/shared/src/pids.c").read_text()
common_ex = (root / "trunk/user/rc/src/common_ex.c").read_text()
policy = (root / "trunk/user/rc/src/service_lifecycle_policy.h").read_text()
usb_services = (root / "trunk/user/rc/src/services_usb.c").read_text()
usb_modem = (root / "trunk/user/rc/src/usb_modem.c").read_text()
rc_header = (root / "trunk/user/rc/src/rc.h").read_text()
lifecycle_test = (root / "trunk/user/rc/src/tests/service_lifecycle_policy.c").read_text()
net_lan_notification = (root / "trunk/user/rc/src/net_lan.c").read_text()
assert "service_lifecycle_notification_allowed" in policy
assert "service_lifecycle_notify(" in policy
assert "service_lifecycle_preflight(targets, count, ops, context, 1)" in policy
assert "service_lifecycle_signal_targets(targets, count, ops" in policy
notify_body = policy.split("service_lifecycle_notify(char *service_names[]", 1)[1].split("service_lifecycle_probe_targets", 1)[0]
pidfile_stop_body = common_ex.split("kill_services_with_pidfiles(char *svc_name[]", 1)[1].split("kill_process_pidfile(", 1)[0]
assert "service_lifecycle_release_targets(targets, count);" in notify_body
assert "free(targets)" not in notify_body
assert "service_lifecycle_release_targets(targets, count);" in pidfile_stop_body
assert "free(targets)" not in pidfile_stop_body
assert "signal_services_by_identity" in rc_header
assert "service_lifecycle_notify(svc_name, signal_number, &ops, NULL)" in common_ex
assert "service_lifecycle_notification_allowed(signal_number)" in common_ex
assert "signal_services_by_identity(httpd_services, SIGUSR1)" in net_lan_notification
assert 'doSystem("killall %s %s", "-SIGUSR1", "httpd")' not in net_lan_notification
assert "test_identity_bound_notification_preflights_all_targets" in lifecycle_test
assert "test_unknown_notification_inventory_sends_zero_signals" in lifecycle_test
net_source = (root / "trunk/user/rc/src/net.c").read_text()
net_lan_notification = (root / "trunk/user/rc/src/net_lan.c").read_text()
pppoe_policy = (root / "trunk/user/rc/src/pppoe_relay_policy.h").read_text()
pppoe_test = (root / "trunk/user/rc/src/tests/pppoe_relay_policy.c").read_text()

assert "process_inventory.o" in shared_makefile
assert '#include "process_inventory.h"' in shared_header
assert "process_inventory_scan(appname, &snapshot)" in pids_source
assert "PROCESS_PRESENCE_UNKNOWN" in pids_source
assert "return EXIT_FAILURE;" in pids_source
assert "return EXIT_SUCCESS;" in pids_source
assert "return count;" not in pids_source
assert '#include <process_inventory.h>' in policy
assert "process_inventory_scan(service_name, snapshot)" in common_ex
assert "process_identity_validate(service_name, identity)" in common_ex
assert "kill(identity->pid, signal_number)" in common_ex
assert 'doSystem("killall %s %s", signal_option, service_name)' not in common_ex
assert "service_lifecycle_preflight" in policy
pppoe_start = net_source.split("set_passthrough_pppoe(int is_on)", 1)[1].split("set_igmp_mld_version", 1)[0]
assert '#include "process_inventory.h"' in net_source
assert '#include "pppoe_relay_policy.h"' in net_source
assert 'process_inventory_scan("pppoe-relay", &snapshot)' in pppoe_start
assert "scan_error = snapshot.error;" in pppoe_start
assert pppoe_start.index("scan_error = snapshot.error;") < pppoe_start.index("process_snapshot_release(&snapshot);")
assert pppoe_start.index("process_snapshot_release(&snapshot);") < pppoe_start.index("pppoe_relay_should_start(presence)")
assert "if (pppoe_relay_should_start(presence))" in pppoe_start
assert "else if (presence == PROCESS_PRESENCE_UNKNOWN)" in pppoe_start
assert "strerror(scan_error)" in pppoe_start
assert "if (!pids(" not in pppoe_start and 'pids("pppoe-relay")' not in pppoe_start
assert pppoe_start.index("if (pppoe_relay_should_start(presence))") < pppoe_start.index('eval("/usr/sbin/pppoe-relay"')
assert "presence == PROCESS_PRESENCE_ABSENT" in pppoe_policy
assert "PROCESS_PRESENCE_UNKNOWN" in pppoe_test and "PROCESS_PRESENCE_RUNNING" in pppoe_test
assert "process inventory failed" in pppoe_start
assert "service_lifecycle_signal_targets" in policy
assert "SERVICE_LIFECYCLE_UNKNOWN" in policy

stop_lpd = usb_services.split("stop_lpd(void)", 1)[1].split("#endif", 1)[0]
assert "kill_services(svcs, 3, 1)" in stop_lpd
assert 'unlink("/var/run/lpdparent.pid")' not in stop_lpd
print("process inventory and lifecycle integration contract: PASS")

lpd_source = (root / "trunk/user/LPRng/src/src/common/lpd.c").read_text()
pid_writer = lpd_source.split("static int update_pidfile(void)", 1)[1].split("void logmessage", 1)[0]
assert "O_CREAT | O_EXCL" in pid_writer
assert "write(fd, pid_text + offset, len - offset)" in pid_writer
assert "errno == EINTR" in pid_writer
assert 'rename(temp_path, "/var/run/lpdparent.pid")' in pid_writer
assert "fopen(\"/var/run/lpdparent.pid\", \"w\")" not in pid_writer
assert "if (update_pidfile() < 0)" in lpd_source

assert '#include <process_inventory.h>' in lpd_source
pid_reader = lpd_source.split("static int read_lpd_pidfile(pid_t *pid)", 1)[1].split("static int lpd_pid_is_running", 1)[0]
assert "errno == ENOENT ? 0 : -1" in pid_reader
pid_policy = (root / "trunk/user/LPRng/src/src/include/lpd_pidfile_policy.h").read_text()
assert "text[i] < '0' || text[i] > '9'" in pid_policy
assert "value <= 1 || value > (unsigned long)INT_MAX" in pid_policy
assert "static int lpd_pid_is_running(pid_t pid)" in lpd_source
assert "process_inventory_scan(\"lpd\", &snapshot)" in lpd_source
assert "process_identity_validate(\"lpd\", &snapshot.items[i])" in lpd_source
assert "fscanf(fp, \"%d\", &pid)" not in lpd_source

assert "if (pid == getpid())" in lpd_source

assert lpd_source.index("bind(sockfd") < lpd_source.index("update_pidfile() < 0")
assert lpd_source.index("update_pidfile() < 0") < lpd_source.index("listen(sockfd")

assert '#include "lpd_pidfile_policy.h"' in lpd_source
assert "lpd_pidfile_parse(buffer, used, pid)" in lpd_source
assert "value <= 1 || value > (unsigned long)INT_MAX" not in lpd_source

pidfile_source = (root / "trunk/user/shared/src/process_pidfile.c").read_text()
pidfile_header = (root / "trunk/user/shared/src/process_pidfile.h").read_text()
assert "process_pidfile.o" in shared_makefile
assert "process_pidfile_parse_record" in pidfile_source
assert "process_identity_validate(process_name, &snapshot.items[i])" in pidfile_source
assert "process_inventory_scan(process_name, &snapshot)" in pidfile_source
assert "process_pidfile_write" in pidfile_header
assert "process_pidfile_signal" in pidfile_header

detect_internet_source = (root / "trunk/user/rc/src/detect_internet.c").read_text()
httpd_source = (root / "trunk/user/httpd/src/web_ex.c").read_text()
assert "process_pidfile_write(DI_PID_FILE, \"detect_internet\")" in detect_internet_source
assert "process_pidfile_signal(DI_PID_FILE, \"detect_internet\", SIGUSR1)" in detect_internet_source
assert "process_pidfile_signal(DI_PID_FILE, \"detect_internet\", SIGUSR2)" in detect_internet_source
assert "kill_pidfile_s(DI_PID_FILE" not in detect_internet_source
assert 'process_pidfile_signal("/var/run/detect_internet.pid", "detect_internet", SIGHUP)' in httpd_source
assert 'kill_pidfile_s("/var/run/detect_internet.pid"' not in httpd_source

detect_link_source = (root / "trunk/user/rc/src/detect_link.c").read_text()
assert "process_pidfile_write(DL_PID_FILE, \"detect_link\")" in detect_link_source
assert "process_pidfile_signal(DL_PID_FILE, \"detect_link\", SIGUSR1)" in detect_link_source
assert "process_pidfile_signal(DL_PID_FILE, \"detect_link\", SIGHUP)" in detect_link_source
assert "kill_pidfile_s(DL_PID_FILE" not in detect_link_source

rstats_source = (root / "trunk/user/rc/src/rstats.c").read_text()
assert "process_pidfile_write(RSTATS_PID_FILE, \"rstats\")" in rstats_source
assert "process_pidfile_signal(RSTATS_PID_FILE, \"rstats\", SIGHUP)" in rstats_source
assert "kill_pidfile_s(RSTATS_PID_FILE" not in rstats_source
assert 'process_pidfile_signal(RSTATS_PID_FILE, "rstats", sig)' in httpd_source
assert "kill_pidfile_s(RSTATS_PID_FILE" not in httpd_source

pppd_policy = (root / "trunk/user/pppd/ppp-2.4.7/pppd/pppd_pidfile_policy.h").read_text()
pppd_main = (root / "trunk/user/pppd/ppp-2.4.7/pppd/main.c").read_text()
pppd_writer = pppd_main.split("static void\ncreate_pidfile(pid)", 1)[1].split("void\ncreate_linkpidfile(pid)", 1)[0]
process_pidfile = (root / "trunk/user/shared/src/process_pidfile.c").read_text()
vpn_server = (root / "trunk/user/rc/src/vpn_server.c").read_text()
assert "pppd_pidfile_parse_start_time" in pppd_policy
assert "pppd_pidfile_format_record" in pppd_policy
assert "read_process_start_time(pid" in pppd_main
assert "write_pidfile_record(pidfilename, (pid_t)pid)" in pppd_writer
assert 'fprintf(pidfile, "%d\\n", pid)' not in pppd_writer
assert "process_pidfile_read_identity" in process_pidfile
assert "O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK" in process_pidfile
assert "process_identity_validate(process_name, &snapshot.items[i])" in process_pidfile
assert 'process_pidfile_read_identity(pidfile, "pppd", &identity)' in vpn_server
assert "pid_text_file" not in vpn_server
print("PPPD PID-file start-time identity integration: PASS")

lifecycle_policy = (root / "trunk/user/rc/src/service_lifecycle_policy.h").read_text()
lifecycle_test = (root / "trunk/user/rc/src/tests/service_lifecycle_policy.c").read_text()
common_ex = (root / "trunk/user/rc/src/common_ex.c").read_text()
vpn_server = (root / "trunk/user/rc/src/vpn_server.c").read_text()
vpn_stop = vpn_server.split("static int\nstop_vpn_server_locked(void)", 1)[1].split("\n}\n", 1)[0]
assert "service_lifecycle_scan_target" in lifecycle_policy
assert "read_pidfile" in lifecycle_policy and "force_only" in lifecycle_policy
assert "service_lifecycle_stop_targets" in lifecycle_policy
assert "process_pidfile_read_identity" in common_ex
assert "kill_services_with_pidfiles" in common_ex
assert "test_pidfile_unknown_blocks_entire_preflight" in lifecycle_test
assert "test_pidfile_identity_change_blocks_escalation" in lifecycle_test
assert "test_force_only_pidfile_target_skips_term_and_joins_kill" in lifecycle_test
assert "kill_services_with_pidfiles(svcs, pidfile_paths, \"pppd\", 1, 5, 1)" in vpn_stop
assert "kill_pidfile_s" not in vpn_stop
assert "stop_status != 0" in vpn_server
print("PPP PID-file all-target stop transaction integration: PASS")

vpn_client = (root / "trunk/user/rc/src/vpn_client.c").read_text()
vpn_client_stop = vpn_client.split("static int\nstop_vpn_client_locked(void)", 1)[1].split("\n}\n", 1)[0]
vpn_client_stop_public = vpn_client.split("int\nstop_vpn_client(void)", 1)[1].split("\n}\n", 1)[0]
vpn_client_force = vpn_client.split("static int\nstop_vpn_client_force(void)", 1)[1].split("\n}\n", 1)[0]
assert "stop_vpn_client_ppp(0)" in vpn_client_stop
assert "if (stop_status != 0)" in vpn_client_stop
assert "vpn_server_lifecycle_lock_acquire()" in vpn_client_stop_public
assert "stop_vpn_client_locked()" in vpn_client_stop_public
assert "vpn_server_lifecycle_lock_release(lock_fd)" in vpn_client_stop_public
assert 'process_inventory_scan("xl2tpd", &snapshot)' in vpn_client_stop
assert "PROCESS_PRESENCE_START_ABORT" in vpn_client_stop
assert 'control_xl2tpd("d", "VPNC")' in vpn_client_stop
assert "stop_vpn_client_ppp(1)" in vpn_client_force
assert "process_pidfile_parse_link_record" in process_pidfile
assert "write_link_pidfile_record" in pppd_main
assert "pppd_pidfile_format_link_record" in pppd_policy
assert "stop_vpn_client() != 0" in (root / "trunk/user/rc/src/net_wan.c").read_text()
print("VPN client PPP link PID-file stop integration: PASS")

udhcpc_source = (root / "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c").read_text()
udhcpc_writer = udhcpc_source.split("static int\nwrite_udhcpc_pidfile", 1)[1].split("\n#if BB_MMU", 1)[0]
assert '#include "process_pidfile_record.h"' in udhcpc_source
assert "process_pidfile_record_read_start_time(getpid(), &start_time)" in udhcpc_writer
assert "process_pidfile_record_format(getpid(), start_time" in udhcpc_writer
assert "mkstemp(temp_path)" in udhcpc_writer and "fsync(fd)" in udhcpc_writer
assert "rename(temp_path, path)" in udhcpc_writer
assert "write_pidfile(client_config.pidfile)" not in udhcpc_source
assert "remove_pidfile(client_config.pidfile)" not in udhcpc_source
assert "unlink-by-path can race a new publisher" in udhcpc_source
print("BusyBox udhcpc identity-bound PID-file producer integration: PASS")

net_wan = (root / "trunk/user/rc/src/net_wan.c").read_text()
net_wan_dhcp = net_wan
net_lan_dhcp = (root / "trunk/user/rc/src/net_lan.c").read_text()
wan_dhcp_stop = net_wan_dhcp.split("static int\nstop_udhcpc_pidfile", 1)[1].split("\nint renew_udhcpc_wan", 1)[0]
lan_stop = net_lan_dhcp.split("int\nstop_lan(int is_ap_mode)", 1)[1].split("\nint\nfull_restart_lan", 1)[0]
lan_restart = net_lan_dhcp.split("int\nfull_restart_lan(void)", 1)[1]
assert "kill_services_with_pidfiles" in wan_dhcp_stop
assert 'process_pidfile_signal(pidfile, "udhcpc", SIGUSR1)' in net_wan_dhcp
assert 'process_pidfile_signal(pidfile, "udhcpc", SIGUSR2)' in net_wan_dhcp
assert "kill_pidfile(" not in net_wan_dhcp[net_wan_dhcp.index("static int\nstop_udhcpc_pidfile"):]
assert "kill_services_with_pidfiles(svcs, pidfiles, \"udhcpc\", 0, 3, 1)" in lan_stop
assert '"/var/run/udhcpc_lan.pid"' in lan_stop
assert "kill_services(svcs, 3, 1)" not in lan_stop
assert "if (lan_status != 0)" in lan_restart
assert "LAN restart aborted" in lan_restart
print("WAN/LAN udhcpc identity-bound stop integration: PASS")

wan_start = net_wan.split("void\nstart_wan(void)", 1)[1].split(
    "\nstatic int\nstop_wan_ppp_locked", 1
)[0]
wan_up = net_wan.split("wan_up_internal(char *wan_ifname, int unit, int is_static)", 1)[1].split(
    "\nint\nwan_up(", 1
)[0]
wan_down = net_wan.split("wan_down_internal(char *wan_ifname, int unit, int is_static)", 1)[1].split(
    "\nint\nwan_down(", 1
)[0]
assert "auth_status = start_auth_eapol(wan_ifname, unit, wan_auth_mode - 2);" in wan_start
wan_eapol_failure = wan_start.split("if (auth_status != 0)", 1)[1].split("/* We are done configuration */", 1)[0]
assert "set_wan_unit_value_int(unit, \"err\", 1)" in wan_eapol_failure
assert 'ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL)' in wan_eapol_failure
assert "continue;" in wan_eapol_failure
assert wan_start.index("if (auth_status != 0)") < wan_start.index("start_udhcpc_wan(wan_ifname, unit, 0)")
assert "route_status = start_auth_kabinet();" in wan_up
assert "status = route_status_accumulate(status, route_status);" in wan_up
assert "WAN kabinet authentication startup incomplete" in wan_up
assert "route_status = stop_auth_kabinet();" in wan_down
assert "status = route_status_accumulate(status, route_status);" in wan_down
assert "WAN kabinet authentication teardown incomplete" in wan_down
assert "eapol_stop_status = stop_auth_eapol();" in wan_stop
assert "kabinet_stop_status = stop_auth_kabinet();" in wan_stop
assert "if (eapol_stop_status != 0 || kabinet_stop_status != 0)" in wan_stop
assert wan_stop.index("if (eapol_stop_status != 0 || kabinet_stop_status != 0)") < wan_stop.index(
    "set_passthrough_pppoe(0);"
)

wan_stop = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\n}\n", 1)[0]
usb_modem = (root / "trunk/user/rc/src/usb_modem.c").read_text()
modem_remove = usb_modem.split("int\nsafe_remove_usb_modem(void)", 1)[1].split("\n}\n", 1)[0]
assert '"udhcpc"' not in wan_stop.split("char* svcs_wan[]", 1)[1].split("};", 1)[0]
assert "process_pidfile_signal(udhcpc_pidfiles[0], \"udhcpc\", SIGUSR2)" in wan_stop
assert "process_pidfile_signal(udhcpc_pidfiles[1], \"udhcpc\", SIGUSR2)" in wan_stop
assert "kill_services_with_pidfiles(svcs_wan, udhcpc_pidfiles, \"udhcpc\", 0, 3, 1)" in wan_stop
assert "killall %s %s\", \"-SIGUSR2\", \"udhcpc\"" not in wan_stop
assert "stop_udhcpc_wan(0)" in modem_remove
assert '"/var/run/ppp-wan0.pid"' in modem_remove
assert "kill_services_with_pidfiles(NULL, pidfiles, \"pppd\", 0, 10, 1)" in modem_remove
assert "set_usb_modem_dev_wan(0, 0)" in modem_remove
assert modem_remove.index("if (stop_status != 0)") < modem_remove.index("set_usb_modem_dev_wan(0, 0)")
print("WAN and USB modem DHCP/PPP identity-bound teardown integration: PASS")

usbnet_stop = usb_modem.split("int\nstop_wan_usbnet(void)", 1)[1].split("\n}\n", 1)[0]
assert "ndis_control_network(ndis_ifname, modem_devnum, 0)" in usbnet_stop
assert "if (control_status != 0)" in usbnet_stop
assert 'ifdown_status = ifconfig(ndis_ifname, 0, "0.0.0.0", NULL)' in usbnet_stop
assert "ifdown_status != ENODEV" in usbnet_stop
assert 'is_usbnet_has_module(ndis_ifname, "cdc_mbim")' in usbnet_stop
assert "int  safe_remove_usb_modem(void);" in rc_header
assert "int  stop_wan_usbnet(void);" in rc_header

usb_modem_guard = usb_modem.split(
    "static int\nusb_modem_require_no_active_process(const char *process_name)", 1
)[1].split("\n}\n", 1)[0]
assert "process_inventory_scan(process_name, &snapshot)" in usb_modem_guard
assert "scan_error = snapshot.error;" in usb_modem_guard
assert usb_modem_guard.index("scan_error = snapshot.error;") < usb_modem_guard.index(
    "process_snapshot_release(&snapshot);"
)
assert usb_modem_guard.index("process_snapshot_release(&snapshot);") < usb_modem_guard.index(
    "if (presence == PROCESS_PRESENCE_ABSENT)"
)
assert "PROCESS_PRESENCE_RUNNING" in usb_modem_guard
assert "EBUSY" in usb_modem_guard and "scan_error ? scan_error : EIO" in usb_modem_guard

modem_remove = usb_modem.split("int\nsafe_remove_usb_modem(void)", 1)[1].split("\n}\n", 1)[0]
assert 'usb_modem_require_no_active_process("usb_modeswitch")' in modem_remove
assert 'usb_modem_require_no_active_process("eject")' in modem_remove
assert modem_remove.index('usb_modem_require_no_active_process("usb_modeswitch")') < modem_remove.index(
    "stop_udhcpc_wan(0)"
)
assert "killall" not in modem_remove
assert "stop_status = stop_wan_usbnet();" in modem_remove
assert "if (stop_status != 0)" in modem_remove
assert modem_remove.index("return stop_status;") < modem_remove.index("set_usb_modem_dev_wan(0, 0)")
assert "return 0;" in modem_remove

usb_remove = usb_services.split("void\nsafe_remove_usb_device(int port, const char *dev_name)", 1)[1].split("\n}\n", 1)[0]
assert "modem_remove_status = safe_remove_usb_modem();" in usb_remove
assert "if (modem_remove_status == 0)" in usb_remove
assert usb_remove.index("if (modem_remove_status == 0)") < usb_remove.index("try_wan_reconnect(0, 0)")
assert "skipping WAN reconnect after incomplete USB modem removal" in usb_remove

usbnet_stop_pos = wan_stop.index("stop_status = stop_wan_usbnet();")
wan_ifdown_pos = wan_stop.index("ifdown_status = wan_teardown_ifdown_interfaces(")
wan_failure_pos = wan_stop.index("if (stop_status != 0 || ifdown_status != 0)")
wan_clear_pos = wan_stop.index("clear_wan_state();")
assert usbnet_stop_pos < wan_ifdown_pos < wan_failure_pos < wan_clear_pos
assert "goto abort_resume_detect;" in wan_stop[wan_failure_pos:]
print("USB modem stop-result propagation and WAN restart gate: PASS")


lan_stop_transaction = net_lan_dhcp.split("int\nstop_lan(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
lan_pause = lan_stop_transaction.index("notify_pause_detect_internet()")
lan_preflight = lan_stop_transaction.index('kill_services_with_pidfiles(svcs, pidfiles, "udhcpc", 0, 3, 1)')
lan_preflight_failure = lan_stop_transaction.index("if (status != 0)", lan_preflight)
lan_preflight_resume = lan_stop_transaction.index("notify_run_detect_internet(2)", lan_preflight_failure)
lan_ifdown = lan_stop_transaction.index("ifconfig(IFNAME_BR, 0, NULL, NULL)")
lan_ifdown_resume = lan_stop_transaction.index("notify_run_detect_internet(2)", lan_ifdown)
assert lan_pause < lan_preflight < lan_preflight_failure < lan_preflight_resume < lan_ifdown
assert lan_ifdown < lan_ifdown_resume
assert "if (status != 0 && is_ap_mode)" in lan_stop_transaction
print("AP LAN detect_internet pause rollback on failed teardown: PASS")

lan_restart_transaction = net_lan_dhcp.split("int\nfull_restart_lan(void)", 1)[1].split("\n}\n", 1)[0]
lan_vpn_stop = lan_restart_transaction.index("vpn_status = stop_vpn_server()")
lan_vpn_failure = lan_restart_transaction.index("if (vpn_status != 0)", lan_vpn_stop)
lan_vpn_abort = lan_restart_transaction.index("return vpn_status;", lan_vpn_failure)
lan_first_other_stop = lan_restart_transaction.index("stop_lltd();")
assert lan_vpn_stop < lan_vpn_failure < lan_vpn_abort < lan_first_other_stop
assert lan_restart_transaction.count("stop_vpn_server();") == 1
lan_dhcp_failure = lan_restart_transaction.index("if (lan_status != 0)")
lan_dns_unlock = lan_restart_transaction.index("dnsmasq_lifecycle_lock_release(dnsmasq_lock)", lan_dhcp_failure)
lan_vpn_restore = lan_restart_transaction.index("restart_vpn_server();", lan_dns_unlock)
assert lan_dhcp_failure < lan_dns_unlock < lan_vpn_restore
print("LAN restart VPN fail-closed stop and rollback integration: PASS")

wan_stop_transaction = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\n}\n", 1)[0]
wan_ppp_stop_transaction = net_wan.split("static int\nstop_wan_ppp_locked(void)", 1)[1].split("\n}\n", 1)[0]
assert "abort_resume_detect:" in wan_stop_transaction
assert "notify_run_detect_internet(2)" in wan_stop_transaction
assert "goto abort_resume_detect;" in wan_stop_transaction
assert "abort_resume_detect:" in wan_ppp_stop_transaction
assert "notify_run_detect_internet(2)" in wan_ppp_stop_transaction
assert "goto abort_resume_detect;" in wan_ppp_stop_transaction
print("WAN PPP/WAN teardown detect_internet pause rollback: PASS")

xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
xy_target_sources = (
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c",
    "trunk/user/pppd/ppp-2.4.7/pppd/main.c",
    "trunk/user/pppd/ppp-2.4.7/pppd/pppd_pidfile_policy.h",
    "trunk/user/rc/src/common_ex.c",
    "trunk/user/rc/src/net.c",
    "trunk/user/rc/src/pppoe_relay_policy.h",
    "trunk/user/rc/src/net_lan.c",
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/service_lifecycle_policy.h",
    "trunk/user/rc/src/services_ex.c",
    "trunk/user/rc/src/dnsmasq_reload_policy.h",
    "trunk/user/rc/src/usb_modem.c",
    "trunk/user/rc/src/vpn_client.c",
    "trunk/user/rc/src/vpn_server.c",
    "trunk/user/rc/src/inicd.c",
    "trunk/user/rc/src/net_wifi.c",
    "trunk/user/rc/src/detect_wan.c",
    "trunk/user/rc/src/detect_wan_udhcpc_policy.h",
    "trunk/user/shared/src/process_pidfile.c",
    "trunk/user/shared/src/process_pidfile.h",
    "trunk/user/shared/src/process_pidfile_record.h",
)
for path in xy_target_sources:
    assert '"{}"'.format(path) in xy_workflow, (
        "XY-C1 target-build workflow does not watch {}".format(path)
    )
print("XY-C1 MIPS/uClibc source-trigger coverage: PASS")

wan_source = (root / "trunk/user/rc/src/detect_wan.c").read_text()
wan_policy = (root / "trunk/user/rc/src/detect_wan_udhcpc_policy.h").read_text()
wan_test = (root / "trunk/user/rc/src/tests/detect_wan_udhcpc_policy.c").read_text()
process_workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()

wan_renew = wan_source.split("static int\nchk_udhcpc(int ap_mode)", 1)[1].split("\n}\n", 1)[0]
wan_poll = wan_source.split("static int\npoll_gateway(void)", 1)[1].split("\n}\n", 1)[0]
assert "process_inventory_scan(\"udhcpc\", &snapshot)" in wan_renew
assert "detect_wan_udhcpc_should_renew(presence)" in wan_renew
assert "scan_error = snapshot.error;" in wan_renew
assert wan_renew.index("scan_error = snapshot.error;") < wan_renew.index("process_snapshot_release(&snapshot);")
assert wan_renew.index("process_snapshot_release(&snapshot);") < wan_renew.index('process_pidfile_signal(detect_wan_udhcpc_pidfile(ap_mode),')
assert "if (renew_decision < 0)" in wan_renew
assert "errno = scan_error ? scan_error : EIO;" in wan_renew
assert "if (renew_decision == 0)" in wan_renew
assert 'process_pidfile_signal(detect_wan_udhcpc_pidfile(ap_mode), "udhcpc", SIGUSR1)' in " ".join(wan_renew.split())
assert 'doSystem("killall %s %s", "-SIGUSR1", "udhcpc")' not in wan_renew
assert 'return ap_mode ? "/var/run/udhcpc_lan.pid" : "/var/run/udhcpc0.pid";' in wan_policy
assert 'detect_wan_udhcpc_pidfile(0), "/var/run/udhcpc0.pid"' in wan_test
assert 'detect_wan_udhcpc_pidfile(1), "/var/run/udhcpc_lan.pid"' in wan_test
assert 'pids("udhcpc")' not in wan_renew
assert "return -1;" in wan_renew
assert "renew_status = chk_udhcpc(ap_mode);" in wan_poll
assert "if (renew_status != 0)" in wan_poll
assert wan_poll.index("if (renew_status != 0)") < wan_poll.index("return -1;")
assert all(state in wan_policy for state in ("PROCESS_PRESENCE_RUNNING", "PROCESS_PRESENCE_ABSENT", "PROCESS_PRESENCE_UNKNOWN"))
assert "errno = EIO;" in wan_policy and "return -1;" in wan_policy
assert "PROCESS_PRESENCE_UNKNOWN) == -1" in wan_test
assert "PROCESS_PRESENCE_ABSENT) == 0" in wan_test
assert "PROCESS_PRESENCE_RUNNING) == 1" in wan_test
assert "WAN DHCP renewal tri-state policy: PASS" in wan_test
for path in (
    "trunk/user/rc/src/detect_wan.c",
    "trunk/user/rc/src/net.c",
    "trunk/user/rc/src/pppoe_relay_policy.h",
    "trunk/user/rc/src/tests/pppoe_relay_policy.c",
    "trunk/user/rc/src/detect_wan_udhcpc_policy.h",
    "trunk/user/rc/src/tests/detect_wan_udhcpc_policy.c",
):
    assert '"{}"'.format(path) in process_workflow, "process workflow missing {}".format(path)
assert "WAN DHCP renewal presence policy regression" in process_workflow

presence_policy = (root / "trunk/user/rc/src/process_presence_start_policy.h").read_text()
process_inventory_header = (root / "trunk/user/shared/src/process_inventory.h").read_text()
presence_test = (root / "trunk/user/rc/src/tests/process_presence_start_policy.c").read_text()
detect_internet_source = (root / "trunk/user/rc/src/detect_internet.c").read_text()
detect_link_source = (root / "trunk/user/rc/src/detect_link.c").read_text()
di_notify = detect_internet_source.split(
    "void\nnotify_run_detect_internet(int delay_time)", 1
)[1].split("\n}\n", 1)[0]
dl_notify = detect_link_source.split(
    "void\nnotify_reset_detect_link(void)", 1
)[1].split("\n}\n", 1)[0]
wan_gateway_start = net_wan.split(
    "/* Start the gateway ARP checker only after a complete ABSENT scan. */", 1
)[1].split("if (wan_gate)", 1)[0]
for source in (detect_internet_source, detect_link_source, net_wan):
    assert '#include "process_presence_start_policy.h"' in source
    assert '#include <process_inventory.h>' in source
for body, process_name in ((di_notify, "detect_internet"), (dl_notify, "detect_link")):
    assert 'process_inventory_scan("{}"'.format(process_name) in body
    assert "scan_error = snapshot.error;" in body
    assert body.index("scan_error = snapshot.error;") < body.index("process_snapshot_release(&snapshot);")
    assert body.index("process_snapshot_release(&snapshot);") < body.index("process_presence_start_action_for_state(presence)")
    assert "PROCESS_PRESENCE_START_NEW" in body
    assert "PROCESS_PRESENCE_START_NOTIFY" in body
    assert "skipping start/notification because process inventory is unknown" in body
    assert "pids(" not in body
assert 'process_inventory_scan("detect_wan", &snapshot)' in wan_gateway_start
assert "scan_error = snapshot.error;" in wan_gateway_start
assert wan_gateway_start.index("scan_error = snapshot.error;") < wan_gateway_start.index("process_snapshot_release(&snapshot);")
assert wan_gateway_start.index("process_snapshot_release(&snapshot);") < wan_gateway_start.index("process_presence_start_action_for_state(presence)")
assert "PROCESS_PRESENCE_START_NEW" in wan_gateway_start
assert "PROCESS_PRESENCE_START_ABORT" in wan_gateway_start
assert 'pids("detect_wan")' not in wan_gateway_start
assert "PROCESS_PRESENCE_START_ABORT" in presence_policy
assert "PROCESS_PRESENCE_UNKNOWN" in process_inventory_header
for token in ("PROCESS_PRESENCE_ABSENT", "PROCESS_PRESENCE_RUNNING",
              "PROCESS_PRESENCE_UNKNOWN", "(enum process_presence)7"):
    assert token in presence_test
assert "Process presence start/notify tri-state policy: PASS" in presence_test
for path in (
    "trunk/user/rc/src/detect_internet.c",
    "trunk/user/rc/src/detect_link.c",
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/process_presence_start_policy.h",
    "trunk/user/rc/src/tests/process_presence_start_policy.c",
):
    assert '"{}"'.format(path) in process_workflow, "process workflow missing {}".format(path)
    assert '"{}"'.format(path) in xy_workflow, "XY-C1 workflow missing {}".format(path)
print("detect_internet/detect_link/WAN gateway tri-state start-gate integration: PASS")

wifi_source = (root / "trunk/user/rc/src/net_wifi.c").read_text()
rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
stop_8021x_wl = wifi_source.split("int\nstop_8021x_wl(void)", 1)[1].split("\n}", 1)[0]
stop_8021x_rt = wifi_source.split("int\nstop_8021x_rt(void)", 1)[1].split("\n}", 1)[0]
stop_8021x_all = wifi_source.split("int\nstop_8021x_all(void)", 1)[1].split("\n}", 1)[0]
assert "return kill_services(svcs, 3, 1);" in stop_8021x_wl
assert "return 0;" in stop_8021x_wl
assert "return kill_services(svcs, 3, 1);" in stop_8021x_rt
assert "return kill_services(svcs, 3, 1);" in stop_8021x_all
restart_wifi_wl = wifi_source.split("restart_wifi_wl(int radio_on, int need_reload_conf)", 1)[1].split(
    "\nint\nrestart_wifi_rt", 1
)[0]
restart_wifi_rt = wifi_source.split("restart_wifi_rt(int radio_on, int need_reload_conf)", 1)[1].split(
    "\nint\n", 1
)[0]
assert "if (stop_8021x_wl() != 0)" in restart_wifi_wl
assert "if (need_reload_conf && stop_8021x_rt() != 0)" in restart_wifi_wl
assert restart_wifi_wl.index("if (stop_8021x_wl() != 0)") < restart_wifi_wl.index(
    'nvram_set_int_temp("reload_svc_wl", 1)'
)
assert "if (stop_8021x_rt() != 0)" in restart_wifi_rt
assert "if (need_reload_conf && stop_8021x_wl() != 0)" in restart_wifi_rt
assert restart_wifi_rt.index("if (stop_8021x_rt() != 0)") < restart_wifi_rt.index(
    'nvram_set_int_temp("reload_svc_rt", 1)'
)
assert "DBDC rollback could not stop the 5GHz 802.1X daemon" in restart_wifi_wl
assert "DBDC rollback could not stop the 2.4GHz 802.1X daemon" in restart_wifi_rt
shutdown_router = rc_source.split("shutdown_router(int level)", 1)[1].split("\n}\n", 1)[0]
assert "auth_stop_status = stop_8021x_all();" in shutdown_router
assert "auth_stop_status == 0" in shutdown_router
assert shutdown_router.index("auth_stop_status = stop_8021x_all();") < shutdown_router.index(
    "auth_stop_status == 0"
)
assert '"trunk/user/rc/src/net_wifi.c"' in process_workflow
assert '"trunk/user/rc/src/rc.c"' in process_workflow

auth_source = (root / "trunk/user/rc/src/auth.c").read_text()
auth_stop_kabinet = auth_source.split("int stop_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_stop_eapol = auth_source.split("int stop_auth_eapol(void)", 1)[1].split("\n}", 1)[0]
auth_start_kabinet = auth_source.split("int start_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_start_eapol = auth_source.split("int start_auth_eapol(", 1)[1].split("\n}", 1)[0]
assert "return kill_services(svcs, 3, 1);" in auth_stop_kabinet
assert "return kill_services(svcs, 3, 1);" in auth_stop_eapol
assert "ret = stop_auth_kabinet();" in auth_start_kabinet
assert "if (ret != 0)" in auth_start_kabinet
assert auth_start_kabinet.index("ret = stop_auth_kabinet();") < auth_start_kabinet.index("if (ret != 0)") < auth_start_kabinet.index('eval("/usr/sbin/lanauth"')
assert "ret = stop_auth_eapol();" in auth_start_eapol
assert "if (ret != 0)" in auth_start_eapol
assert auth_start_eapol.index("ret = stop_auth_eapol();") < auth_start_eapol.index("if (ret != 0)") < auth_start_eapol.index('fopen(wpa_conf, "w")')
assert '"trunk/user/rc/src/auth.c"' in process_workflow
assert '"trunk/user/rc/src/auth.c"' in xy_workflow

watchdog_source = (root / "trunk/user/rc/src/watchdog.c").read_text()
httpd_probe = watchdog_source.split("static int\nhttpd_check_v2()", 1)[1].split(
    "\nstatic int\nrefresh_ntp(void)", 1
)[0]
busybox_config = (root / "trunk/configs/boards/busybox.config").read_text()
assert 'wget -q -T 1 http://127.0.0.1:%d/httpd_check.htm' in httpd_probe
assert 'pids("wget")' not in httpd_probe
assert 'killall wget' not in httpd_probe
assert "CONFIG_FEATURE_WGET_TIMEOUT=y" in busybox_config
watchdog_httpd = watchdog_source.split("static void httpd_process_check(void)", 1)[1].split(
    "/* Sometimes, dnsmasq crashed, try to re-run it */", 1
)[0]
watchdog_dnsmasq = watchdog_source.split("static void\ndnsmasq_process_check(void)", 1)[1].split(
    "\n}\n", 1
)[0]
watchdog_start = watchdog_source.split("int\nstart_watchdog(void)", 1)[1].split(
    "\n}\n", 1
)[0]
watchdog_notify = watchdog_source.split("void\nnotify_watchdog_time(void)", 1)[1].split(
    "\n}\n", 1
)[0]
for body, service in ((watchdog_httpd, "httpd"), (watchdog_dnsmasq, "dnsmasq"),
                      (watchdog_start, "watchdog")):
    assert 'process_inventory_scan("{}", &snapshot)'.format(service) in body
    assert "scan_error = snapshot.error;" in body
    assert body.index("scan_error = snapshot.error;") < body.index("process_snapshot_release(&snapshot);")
    assert body.index("process_snapshot_release(&snapshot);") < body.index("process_presence_start_action_for_state(presence)")
    assert "pids(" not in body
assert "is_dns_dhcpd_run()" not in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_httpd
assert "stop_status = stop_httpd();" in watchdog_httpd
assert watchdog_httpd.count("stop_httpd();") == 1
assert "if (stop_status != 0)" in watchdog_httpd
httpd_stop_failure = watchdog_httpd.split("if (stop_status != 0)", 1)[1].split(
    "start_status = start_httpd(0);", 1
)[0]
assert watchdog_httpd.index("stop_status = stop_httpd();") < watchdog_httpd.index(
    "if (stop_status != 0)"
) < watchdog_httpd.index("start_status = start_httpd(0);")
assert "return;" in httpd_stop_failure
assert "killall -9 httpd" not in watchdog_httpd
assert "system(" not in watchdog_httpd
assert "start_httpd(0)" in watchdog_httpd
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_NEW" in watchdog_dnsmasq
assert "dnsmasq_missing = 0;" in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_NOTIFY" in watchdog_start
assert "PROCESS_PRESENCE_START_NEW" in watchdog_start
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_start
assert "if (action == PROCESS_PRESENCE_START_NEW)" in watchdog_start
assert watchdog_start.index("if (action == PROCESS_PRESENCE_START_NEW)") < watchdog_start.index('return eval("/sbin/watchdog")')
assert "refusing watchdog start because process-presence policy is invalid" in watchdog_start
assert "start_watchdog()" in watchdog_notify
assert "pids(" not in watchdog_notify
assert "process_pidfile_signal(WD_PID_FILE, \"watchdog\", SIGHUP)" in watchdog_notify
assert '"trunk/user/rc/src/watchdog.c"' in process_workflow
assert '"trunk/user/rc/src/watchdog.c"' in xy_workflow
assert '"trunk/user/rc/src/process_presence_start_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/process_presence_start_policy.c"' in process_workflow
print("watchdog process inventory tri-state recovery/start integration: PASS")
