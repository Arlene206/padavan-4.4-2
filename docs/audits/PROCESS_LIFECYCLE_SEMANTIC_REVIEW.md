# Process lifecycle semantic caller review

## Evidence and scope

- Repository: Arlene206/padavan-4.4-2
- Review baseline: main before this documentation commit, 736259cde2507d0997a240f01814c4ab4ff13527
- Source inventory baseline: d0404d5e33d4239e19c92db9b9ab4fb7529cf26c
- The compare from the source inventory baseline to the review baseline contains two documentation-only commits. Changed paths: REFACTOR_ROADMAP.md, SESSION_HANDOFF.md, dev_log.md and docs/audits/PROCESS_LIFECYCLE_INVENTORY.md. No production source, build file, workflow or test changed between those SHAs.
- Lexical source evidence: Actions run 43581078509, artifact 14108820679, pinned to source SHA d0404d5e33d4239e19c92db9b9ab4fb7529cf26c.
- Reviewed exact API call candidates: 77 pids() calls; one pids_main() caller; 46 kill_services() calls; five kill_pidfile() calls; and 15 kill_pidfile_s() call expressions (14 external callers plus the internal kill_pidfile() wrapper). Definitions and declarations are excluded from caller counts.
- This document completes the requested direct-call-site semantic classification for those APIs. It does not claim that every one of the 5,664 broad lifecycle lexical candidates has been semantically traced. Supplemental direct-signal, process-command, PID producer, child wait/reap and package-boundary findings below are targeted review results.

## Executive findings

The P1 is not just an incomplete process scan. Callers currently use process presence as authorization for notifications, start gates, readiness postconditions, teardown and resource cleanup. Returning -1 for UNKNOWN from the current pids() interface without migrating its callers would make many Boolean branches true and several pids(...) > 0 branches false.

Highest-risk chains found:

1. VPN Server stop ignores kill_services() failure, then sends SIGKILL from multiple PPP PID files, clears server state, unlinks callback scripts, stops optional OpenVPN and clears the lease inventory. restart_vpn_server() starts the service again regardless of whether stop_vpn_server_locked() succeeded.
2. USB modem removal sends SIGUSR2 to udhcpc based on pids(), ignores kill_services() for udhcpc or pppd, then continues WAN USB teardown and clears modem WAN state.
3. AP-mode stop_lan() ignores its udhcpc/detect_wan stop result and continues clearing IPv6 state and bringing the bridge down.
4. flash_firmware() checks its first kill_services() and stop_misc() calls, but later calls the void stop_services(0) orchestrator and continues with shutdown scripts, storage removal, printer stop, IPTV stop, persistence and firmware write. stop_services() itself discards numerous subordinate stop results.
5. HTTPD duplicate-launch gates are conservative for UNKNOWN, but HTTPD and crond startup postconditions use truthiness, so UNKNOWN may be reported as a successful daemon launch. GPIO startup postconditions have the same issue.
6. Samba predicates using pids(...) > 0 map UNKNOWN to ABSENT and can start a duplicate daemon. Raw tri-state values stored in has_smbd/has_nmbd can instead be truthy and authorize SIGHUP. The current mix is unsafe in both directions.
7. kill_pidfile_s() signals only a parsed numeric PID. It does not validate strict input syntax, process name/executable identity or process start time. PID reuse can signal an unrelated process.

No production C code was changed in this semantic-review milestone.

## Shared scanner and current API semantics

Sources: trunk/user/shared/src/pids.c, trunk/user/shared/src/shutils.h and trunk/user/shared/src/shutils.c.

### Failure channels currently collapsed

- alloc_procps_scan() allocates its context and opens /proc. Allocation failure and opendir("/proc") failure both return NULL.
- procps_scan() calls readdir() and treats every NULL as end-of-directory. It does not clear/check errno to distinguish normal EOF from a directory-read error.
- A numeric PID whose /proc/PID/stat open/read fails, whose stat record cannot be parsed, or whose optional /proc/PID/cmdline read fails does not immediately abort the entire outer scan: the per-entry loop breaks and the next procps_scan() call advances to another directory entry. The defect is that these skipped entries are not recorded as an incomplete/UNKNOWN inventory. The final partial process list is treated as authoritative.
- argv0 allocation is unchecked. With a 15-character truncated comm name, loss of argv0 can prevent the argv[1] disambiguation required by comm_match(), producing a false negative. A failed argv0 allocation does not set a scan error.
- find_pid_by_name() first allocates an empty PID vector. Failure returns NULL. A later xrealloc_vector() failure frees the accumulated list and scan context, then also returns NULL. The caller cannot distinguish this from an empty list.
- pids() maps a NULL result from find_pid_by_name() to 0. pids_main() also maps it to a zero process count and does not identify an incomplete scan; this is the command-line ambiguity at rc.c:2005.
- pids() returns a count of matching non-zombie PIDs, not merely a Boolean. Existing callers mix raw count, Boolean conversion and > 0 comparisons.

### Consequence for the proposed tri-state contract

The public result must distinguish RUNNING, ABSENT and UNKNOWN without relying on C truthiness. A negative UNKNOWN must never be used as a Boolean. Callers must compare explicitly against the state or use APIs whose return contract makes UNKNOWN impossible to mistake for presence.

The scan implementation must separately preserve scan status and collected PIDs. Normal EOF is not a scan error. A process disappearing during enumeration can be skipped only when the observed error is a known disappearance (for example ENOENT/ESRCH); permission, I/O, malformed stat data, failed mandatory allocation, vector growth failure and readdir errors must mark the inventory UNKNOWN. A partial list must never be returned as complete.

pids_main() must not print a partial PID list and exit as if it were complete. The RC applet path needs a distinct nonzero error result for UNKNOWN.

## pids() call-site classification

All 77 direct pids() call candidates are grouped below by owning function and intent. “UNKNOWN action” describes the required migration outcome, not current safe behavior.

| File and lines | Owning function / purpose | Current UNKNOWN interpretation or side effect | Required UNKNOWN behavior / propagation |
|---|---|---|---|
| httpd/src/aidisk.c:661,682 | AiDisk Samba/FTP “off” actions; checks smbd/vsftpd before dispatching stop script | NVRAM enable flag is committed to 0 before the process check. A negative UNKNOWN makes !pids() false and dispatches the stop script anyway. | Do not publish a successful stopped state from UNKNOWN. The stop helper must reject UNKNOWN without TERM/KILL; return failure to the UI and retain/reconcile state. |
| httpd/src/web_ex.c:335 | DDNS client action; pids(inadyn) authorizes killall SIGUSR1 | UNKNOWN is truthy and authorizes SIGUSR1. | Only RUNNING may authorize a signal; target the verified process identity. Propagate or log UNKNOWN without signaling. |
| httpd/src/web_ex.c:1980,1982 | WAN/PPP status display for l2tpd/xl2tpd and pppd | UNKNOWN becomes PPP_WAIT because it is truthy; the UI cannot distinguish unobservable from running. | Preserve an explicit unavailable/unknown state in the HTTPD-to-JavaScript status contract; do not label UNKNOWN as RUNNING or ABSENT. |
| httpd/src/web_ex.c:2249,2294 | scutclient and mentohust status hooks | Raw negative state is written to JavaScript as a status integer. | Add an explicit UI UNKNOWN mapping and verify consumers of these status functions before changing numeric values. |
| httpd/src/web_ex.c:2514,2516,2519,2522,2526,2529,2532 | Shadowsocks-family aggregate status and ss-local tunnel status | Aggregate tries the next executable only when the previous pids() result is exactly 0. UNKNOWN from the first candidate stops fallback and is reported as a raw status. | Aggregate returns RUNNING if any candidate is positively observed, UNKNOWN if none is RUNNING but at least one is UNKNOWN, and ABSENT only when all candidates are confirmed absent. Tunnel status exposes UNKNOWN separately. |
| httpd/src/web_ex.c:2580,2600,2608,2617,2624,2633,2641,2684,2693,2702,2708 | dns-forwarder, adbyby, pdnsd, dns2tcp, dnsproxy, zerotier-one, ddnsto, aliyundrive-webdav, smartdns, frpc and frps status hooks | Raw status integer is serialized to JavaScript; consumers may treat any nonzero value, including -1, as active. | Keep UNKNOWN separate in the status protocol and audit the corresponding WebUI consumers before migration. Feature-gated hooks remain conditional on their APP_* build flags. |
| rc/src/common_ex.c:1127 | service_lifecycle_is_running() adapter for kill_services() | Returns the pids() count directly into a Boolean-style lifecycle callback. | Adapter and policy must transport explicit UNKNOWN; policy must not use if(value) or > 0 to authorize signals. |
| rc/src/common_ex.c:1163 | Incomplete-stop diagnostic after kill_services() | if(pids()) logs UNKNOWN as “remains active”. | Log scan failure/unknown distinctly from confirmed RUNNING. Diagnostic only; it must not send a signal. |
| rc/src/detect_internet.c:372 | notify_run_detect_internet() start-or-notify gate | UNKNOWN prevents duplicate start, then attempts PID-file SIGUSR1. | UNKNOWN returns error and must not signal a numeric PID file. Notification helpers should return status instead of swallowing UNKNOWN. |
| rc/src/detect_link.c:357 | notify_reset_detect_link() start-or-notify gate | UNKNOWN prevents duplicate start, then uses PID-file SIGUSR1. | UNKNOWN blocks both duplicate start and PID-file signal, with explicit error. |
| rc/src/detect_wan.c:78 | Gateway reachability check decides whether to renew udhcpc via killall SIGUSR1 | UNKNOWN is truthy, so the test can authorize a signal to every udhcpc process. | Only confirmed RUNNING may enter renew path. No signal on UNKNOWN; propagate failed/unknown probe. |
| rc/src/gpio_btn.c:282 | start_gpio_btn() duplicate-launch guard | UNKNOWN returns success from if(pids()) return 0, falsely claiming the daemon is already started. | UNKNOWN blocks launch but returns EIO/EBUSY, never success. |
| rc/src/gpio_btn.c:287,295 | GPIO detached-child startup postcondition | gpio_btn_start_policy_status() treats any nonzero process result as running; UNKNOWN can satisfy readiness. | Require confirmed RUNNING; UNKNOWN is a failed postcondition and must not be retried as ABSENT. |
| rc/src/net.c:942 | set_passthrough_pppoe() avoids duplicate pppoe-relay launch | UNKNOWN prevents duplicate launch but the void function loses the reason/status. | Do not launch on UNKNOWN; return status so dependent WAN transition can abort. |
| rc/src/net_ppp.c:444 | safe_start_xl2tpd path checks before launch | UNKNOWN prevents another xl2tpd start but returns the same positive “already running” code. | Return distinct failure on UNKNOWN; do not report existing daemon as confirmed. |
| rc/src/net_wan.c:1874 | stop_wan_locked() sends udhcpc SIGUSR2 before kill_services() | UNKNOWN is truthy and directly authorizes SIGUSR2 to udhcpc. | UNKNOWN aborts WAN teardown before the first signal; full preflight includes this notification and the later service group. |
| rc/src/net_wan.c:2324 | Gateway ARP detector start gate | UNKNOWN prevents duplicate launch, but silently drops requested start. | Return/log UNKNOWN explicitly; do not convert outcome to successful no-op. |
| rc/src/services.c:104 | restart_infosvr() checks whether infosvr remains after stop | infosvr_restart_can_start() requires process_present == 0, so UNKNOWN currently blocks restart conservatively. | Preserve fail-closed behavior but transport UNKNOWN distinctly, rather than encoding it as “running”. |
| rc/src/services.c:159 | crond detached-child startup postcondition | crond_start_postcondition() uses truthiness; UNKNOWN can satisfy readiness. | Require confirmed RUNNING; UNKNOWN is failed startup verification. |
| rc/src/services.c:187 | start_networkmap(first_call) avoids duplicate launch | UNKNOWN is truthy and returns 0, reporting a no-op as successful. | Return explicit unknown/error status; do not report readiness. |
| rc/src/services.c:203 | restart_networkmap() sends killall SIGUSR1 or starts process | UNKNOWN is truthy and authorizes SIGUSR1. | UNKNOWN blocks signal and restart; error propagates to RC notification result. |
| rc/src/services.c:235,240,275,300,389,415 | is_sshd_run(), is_scutclient_run(), is_mentohust_run(), is_vlmcsd_run(), is_iperf3_run() status checks | Any nonzero result is converted to Boolean “running”; callers cannot distinguish UNKNOWN. | Replace Boolean wrappers with tri-state or status+error. Firewall/state decisions must not claim running on UNKNOWN. |
| rc/src/services.c:689 | start_httpd() duplicate launch gate | httpd_start_can_launch() rejects any nonzero process result, so UNKNOWN blocks duplicate launch conservatively. | Preserve no-launch-on-UNKNOWN, but return/log UNKNOWN distinctly rather than misreporting confirmed existing HTTPD. |
| rc/src/services.c:710 | start_httpd() detached-child readiness postcondition | httpd_start_postcondition() treats any nonzero as RUNNING; UNKNOWN can publish httpd_started=1 and open firewall paths. | Require confirmed RUNNING before publishing state/opening firewall. UNKNOWN fails startup postcondition. |
| rc/src/services_ex.c:333 | is_dns_dhcpd_run() and DNSMasq gates/restart helpers | Start gates block duplicate launch conservatively on any nonzero; restart_dns() treats UNKNOWN as RUNNING and sends SIGHUP. dnsmasq_confirm_started() may accept UNKNOWN as started. | Every DNSMasq start, readiness, reload and restart policy distinguishes UNKNOWN; UNKNOWN permits no launch or signal and returns error. |
| rc/src/services_ex.c:832 | samba_ex_daemon_is_running() callback | pids(...) > 0 maps UNKNOWN to false/ABSENT. Wait/reload logic may start a duplicate. | Callback returns tri-state; wait helpers fail on UNKNOWN. |
| rc/src/services_ex.c:967 | write_smb_conf() tracks whether nmbd was active | pids("nmbd") > 0 maps UNKNOWN to not running and can start nmbd rather than reload. | Do not choose start/reload until state known; propagate UNKNOWN before config publication. |
| rc/src/services_ex.c:1010 | reload_nmbd() no-op when service appears absent | UNKNOWN is truthy so reload is skipped and function returns 0 as though reload were unnecessary. | Return UNKNOWN as error; no signal and no successful reload claim. |
| rc/src/services_ex.c:1023 | restart_nmbd() checks smbd before config publication and stop | UNKNOWN is truthy, causing config write and subsequent stop/restart path. Even if kill_services later blocks signals, config publication already happened. | Full service preflight before config publication; UNKNOWN blocks publication and restart. |
| rc/src/services_ex.c:1062 | is_upnp_run() UPnP status and update paths | UNKNOWN is truthy, so update_upnp() may send SIGUSR1; start decisions can also be suppressed. | UNKNOWN blocks reload and start, with explicit status returned. stop_upnp() postcondition must preserve UNKNOWN. |
| rc/src/services_ex.c:1621 | notify_ddns_update() writes config then sends inadyn SIGHUP | UNKNOWN authorizes config replacement and SIGHUP. | UNKNOWN aborts before config publication and signal. |
| rc/src/services_stor.c:114 | is_ftp_run() user-visible FTP status | UNKNOWN becomes Boolean running. | Storage/UI status exposes UNKNOWN explicitly. |
| rc/src/services_stor.c:230 | ftp_daemon_is_running() stable-presence callback | pids(...) > 0 maps UNKNOWN to ABSENT, so stable-presence policy may make unsafe start/stop decision. | Callback preserves UNKNOWN; no daemon start or storage cleanup on unknown. |
| rc/src/services_stor.c:889 | config_smb_fastpath(check_pid) | UNKNOWN is truthy and enables fastpath configuration as though smbd were running. | Only confirmed RUNNING enables it; UNKNOWN returns/logs error. |
| rc/src/services_stor.c:948 | samba_daemon_is_running() callback | pids(...) > 0 maps UNKNOWN to ABSENT and can lead to duplicate launch. | Tri-state callback; UNKNOWN aborts reload/start. |
| rc/src/services_stor.c:1046,1047 | run_samba() initial has_smbd/has_nmbd snapshot | Raw -1 is truthy and can be passed as was_running, authorizing SIGHUP. The “both absent” cleanup guard does not run, which is conservatively safe, but does not identify UNKNOWN. | Abort Samba transaction before cleanup, config re-publication, signal or launch if either inventory is UNKNOWN. |
| rc/src/services_stor.c:1170 | wsdd2 reload/start via samba_ensure_daemon() | pids("wsdd2") > 0 maps UNKNOWN to false and can start a duplicate. | UNKNOWN aborts optional daemon transition too; do not treat it as absent. |
| rc/src/services_stor.c:1239 | start_wins() nmbd start gate | UNKNOWN is truthy and blocks duplicate launch, but no error is reported. | Return UNKNOWN as failure; do not report a successful “already running” path. |
| rc/src/services_stor.c:1736,1927,2114,2218 | MiniDLNA, Firefly, Transmission and aria2 status wrappers | UNKNOWN becomes Boolean running. Stop helpers may enter shell/script stop path. Post-stop storage_service_stop_complete() prevents a successful stopped claim while wrapper remains truthy, but shell stop command itself must not signal on UNKNOWN. | Preserve UNKNOWN through status and stop policy; no script signal, restart, mount cleanup or eject continuation until confirmed state. |
| rc/src/usb_modem.c:801 | safe_remove_usb_modem() checks udhcpc before SIGUSR2 | UNKNOWN is truthy and authorizes SIGUSR2. A later kill_services result is ignored. | UNKNOWN aborts modem removal before any signal or interface teardown. |
| rc/src/vpn_client.c:241 | stop_vpn_client() sends control delete for VPNC through xl2tpd | UNKNOWN authorizes xl2tpd control operation. PID-file termination that follows is not identity checked. | UNKNOWN aborts VPN client stop/restart; no control operation, PID-file signal or cleanup. |
| rc/src/vpn_server.c:974,1052 | PPP up/down callbacks gate bcrelay stop | UNKNOWN is truthy and enters kill_services; its result is ignored. | UNKNOWN returns callback failure and blocks dependent route/lease/session cleanup. |
| rc/src/watchdog.c:146,156 | HTTPD probe path kills stale wget clients | UNKNOWN is truthy and runs killall wget. | Do not signal by name on UNKNOWN; target only the verified probe child identity. |
| rc/src/watchdog.c:477 | httpd_process_check() detects a missing daemon | UNKNOWN is truthy and resets httpd_missing rather than confirming absence. If HTTPD_CHECK is enabled and port probe fails, later code still calls stop_httpd(), killall -9 httpd, removes DETECT_HTTPD_FILE and starts HTTPD. | UNKNOWN suppresses restart/escalation/marker cleanup; no SIGKILL on unverified HTTPD state. |
| rc/src/watchdog.c:624 | start_watchdog() duplicate launch guard | UNKNOWN returns 0 as though watchdog is already running. | UNKNOWN blocks duplicate start but returns error, not success. |
| rc/src/watchdog.c:633 | notify_watchdog_time() SIGHUP notification | UNKNOWN is truthy and authorizes PID-file SIGHUP. | UNKNOWN must not reach PID-file signal helper. |
| rc/src/rc.c:2005 via pids_main() | /sbin/pids applet invocation | pids_main() emits zero and exits as if no process matched when inventory setup fails. | Report UNKNOWN with nonzero exit status and no partial/false empty listing. |

### HTTPD status protocol follow-up

C hooks serialize integer status values directly. The complete WebUI-side consumers were not exhaustively resolved by the repository code-search endpoint during this pass (search responses were empty/incomplete), so the numeric UNKNOWN mapping must not be guessed. Phase-2 migration must trace each generated JavaScript function to its ASP/JS consumer and retain existing RUNNING/ABSENT representations while adding an explicit unknown/unavailable state.

## kill_services() call-site classification

There are 46 direct calls. common_ex.c delegates to service_lifecycle_stop(); the policy currently sends TERM before a whole-list authoritative preflight, and its SIGKILL path treats any nonzero process callback as running. The matrix records whether each current result reaches the next lifecycle stage.

| File and lines | Targets / caller role | Current result handling and subsequent action | Required UNKNOWN / failure behavior |
|---|---|---|---|
| rc/src/auth.c:29,59 | stop_auth_kabinet() and stop_auth_eapol() | Both discard stop status. Paired start routines can continue into a new authentication daemon launch. | Convert stop/start pairs to return status; do not launch a replacement if any target is UNKNOWN or stop failed. |
| rc/src/net.c:322 | stop_udpxy(): udpxy, msd_lite | Void wrapper discards result. | Return status to WAN/IPTV transition; no dependent restart after failed stop. |
| rc/src/net.c:384 | stop_xupnpd(): xupnpd | Void wrapper discards result; code exists under APP_XUPNPD. | Return status; no xupnpd replacement/config transition while UNKNOWN. |
| rc/src/net.c:510 | stop_igmpproxy(): xupnpd, udpxy, msd_lite, igmpproxy | Void wrapper discards result. start_igmpproxy() calls it, then starts udpxy/xupnpd and may start igmpproxy. | Stop returns failure; start_igmpproxy() aborts before replacement daemon launch. |
| rc/src/net.c:946 | set_passthrough_pppoe() disable path: pppoe-relay | Result discarded in a void setter. | Propagate failure to WAN transition; preserve service state if stop is unknown/incomplete. |
| rc/src/net_lan.c:677 | stop_lan() AP mode: udhcpc, detect_wan | Result discarded; function then clears LAN IPv6 state and brings IFNAME_BR down. | Make stop_lan() return status; abort interface/IPv6 teardown on UNKNOWN or failed stop. |
| rc/src/net_ppp.c:648,663 | PPP launch preflight: xl2tpd/l2tpd or all pppd during multi-WAN startup | Both check nonzero stop result and return before PPP options publication/launch. | Existing propagation is directionally correct; preserve and add exact UNKNOWN regression. |
| rc/src/net_wan.c:1798,1859,1878 | stop_wan_ppp_locked()/stop_wan_locked(): callback scripts, l2tpd/xl2tpd, pppd and WAN daemons | Stop failures abort before clearing WAN state or bringing interface down. Earlier direct udhcpc SIGUSR2 at line 1875 is still an unsafe pids()-guarded signal. | Preserve failure gate; fold udhcpc and PID-file notifications into preflight before the first signal. |
| rc/src/net_wan6.c:1146 | DHCPv6 stop within callback-generation fence | Returns stop status after releasing callback lock; lock-release failure is also propagated. | Existing propagation is good; UNKNOWN keeps callback fence and prevents downstream IPv6 teardown. |
| rc/src/net_wifi.c:1086,1094,1101 | stop_8021x_wl(), stop_8021x_rt(), stop_8021x_all() | Void wrappers discard stop status; WL branch only compiled with BOARD_HAS_5G_RADIO. | Return status to wireless teardown/reconfiguration; do not continue radio shutdown if authenticator state is UNKNOWN/running. |
| rc/src/rc.c:524 | flash_firmware(): l2tpd, xl2tpd, pppd, wpa_cli, wpa_supplicant | First stop is checked under VPN lifecycle lock; failure aborts firmware update. stop_misc() is also checked. Later stop_services(0) is void and ignores numerous subordinate failures; flash_firmware() proceeds to shutdown script, storage removal, printer stop, IPTV stop, storage persistence and firmware write. | Keep early gates. Make stop_services/stop_services_lan_wan and subordinate stops return aggregated failure; no storage removal, persistence or flash write after UNKNOWN/incomplete service stop. |
| rc/src/services.c:36,43 | stop_syslogd(), stop_klogd() | Void wrappers discard result. | Return status and aggregate in broad shutdown. |
| rc/src/services.c:88,124 | stop_infosvr(), stop_crond() | Return kill_services status. Their restart paths already check stop status. | Preserve; ensure no legacy Boolean postcondition interprets UNKNOWN as RUNNING. |
| rc/src/services.c:197,213 | stop_networkmap(), stop_telnetd() | Void wrappers discard result; run_telnetd() calls stop then launches telnetd. | Convert to status-returning functions; run_telnetd() must not launch after failed stop. |
| rc/src/services.c:735 | stop_httpd() | Checks kill_services and only publishes httpd_started=0 after stop-policy success; returns failure. | Preserve; tri-state postcondition must distinguish UNKNOWN and keep state unmodified. |
| rc/src/services.c:765,797,834 | stop_rstats(), stop_lltd(), stop_logger() | Void wrappers discard result; restart wrappers may continue to launch/reconfigure. | Return status and stop dependent restart/flash progression on failure. |
| rc/src/services.c:1008 | stop_misc(): ntpd, detect_wan, watchdog | Returns kill_services status; flash_firmware() checks it. | Preserve; UNKNOWN blocks firmware path. |
| rc/src/services_ex.c:730 | stop_dns_dhcpd_locked(): dnsmasq | Returns status and only clears static ARP bindings when policy confirms stop. | Existing fail-closed structure is good; policy explicitly treats UNKNOWN as not stopped. |
| rc/src/services_ex.c:935 | stop_nmbd() | Returns kill_services status after postcondition path. | Preserve and ensure config publication/reload occurs only after full known preflight. |
| rc/src/services_ex.c:1265 | stop_upnp(): miniupnpd | Checks kill_services plus miniupnpd_stop_policy_status() and returns status; restart_upnp() aborts firewall restart on failure. | Preserve; UNKNOWN fails postcondition. |
| rc/src/services_ex.c:1348 | stop_ddns(): inadyn | Void wrapper discards stop status. | Return status; do not restart/replace DDNS while stop unknown/incomplete. |
| rc/src/services_stor.c:123 | stop_ftp_locked(): vsftpd | Returns command status after stop/postcondition checks. | Preserve; storage eject remains blocked if UNKNOWN. |
| rc/src/services_stor.c:914 | stop_samba_locked(): smbd, optional wsdd2, nmbd | Returns stop status and skips clean_smbd_trash() if stop failed. It currently writes the SMB fastpath proc value to zero before checking stop result. | Preserve cleanup gate; explicitly report UNKNOWN and keep TDB/trash cleanup blocked. |
| rc/src/services_stor.c:1748,1939 | stop_dms_locked()/stop_itunes_locked(): minidlnad/mt-daapd | Return command status and validate post-stop state under storage operation lock. | Preserve; UNKNOWN blocks storage release and follow-on rescan/start. |
| rc/src/services_usb.c:105 | stop_u2ec() | Void wrapper discards result. | Return and propagate before USB printer subsystem teardown. |
| rc/src/services_usb.c:124 | stop_lpd() | Discards result and unlinks /var/run/lpdparent.pid unconditionally. | Only remove PID file after confirmed stop and confirmed PID ownership; retain it on UNKNOWN. |
| rc/src/services_usb.c:146 | stop_p910nd() | Void wrapper discards result. | Return and propagate before printer/storage release. |
| rc/src/usb_modem.c:806,816 | safe_remove_usb_modem(): udhcpc or pppd | Both results discarded. DHCP path may already have sent SIGUSR2; both continue to stop WAN USB and clear modem WAN state. Compiled only for USB support. | Preflight the complete affected DHCP/PPP group before the first signal; abort modem detach and state clearing on UNKNOWN/failure. |
| rc/src/vpn_client.c:263 | stop_vpn_client_force(): xl2tpd | Result discarded, then xl2tpd_killed_vpnc is set. restart_vpn_client_locked() continues through stop, DNS restoration, wait, new start and firewall restart. | Make force-stop and stop_vpn_client return status; abort DNS/script/state cleanup and restart on UNKNOWN/incomplete stop. |
| rc/src/vpn_server.c:452 | stop_vpn_server_locked(): bcrelay, pptpd, optional xl2tpd | Result discarded before SIGKILL loop over client PPP PID files; then clears server flag, removes callback scripts, stops optional OpenVPN and clears lease inventory. restart_vpn_server() starts replacement regardless of stop status. | Highest-priority caller fix: return status; preflight service list and every client PPP PID identity before first signal; no SIGKILL, NVRAM/script/lease cleanup or restart unless all required stops are confirmed. |
| rc/src/vpn_server.c:976,1053 | PPP callback bcrelay stop on peer-up/peer-down | Result discarded inside callback; route/lease/callback work continues. | Propagate callback stop failure and do not publish dependent peer/session state. |
| rc/src/watchdog.c:176 | refresh_ntp(): ntpd | Checks kill_services and watchdog_ntp_refresh_should_launch(); aborts refresh on stop failure. | Preserve; UNKNOWN blocks replacement ntpd launch. |

### Broad shutdown orchestrators that bypass the direct-call result

- services.c stop_services(int stopall) is void. It calls multiple void stop wrappers, including HTTPD, VPN Server, USB printer processes, optional plugin daemons, networkmap, LLTD, detect_internet, rstats, infosvr, crond and IPTV services, without aggregating return state.
- services.c stop_services_lan_wan() is void and discards stop_dns_dhcpd(), stop_upnp(), stop_detect_link() and optional stop_nmbd() outcomes.
- rc.c flash_firmware() continues after stop_services(0), then removes storage, stops printer spools and persists/rewrites firmware state. This cross-function propagation defect exists even though its earlier kill_services() and stop_misc() calls are checked.

## PID-file caller and producer classification

### Generic PID-file consumers

| File and lines | PID file / signal | Current intent and risk | Required behavior |
|---|---|---|---|
| rc/src/common_ex.c:1179,1186,1187 | Caller-supplied path; repeated TERM then optional KILL in kill_process_pidfile() | Used by VPN client for /var/run/ppp-VPNC.pid. Return from final SIGKILL is ignored. Same numeric file is read repeatedly over a wait interval. | Parse and verify identity/start time on every attempt and escalation; mismatch/UNKNOWN blocks signals and propagates failure. |
| rc/src/net_lan.c:1343 | /var/run/udhcpc_lan.pid, SIGTERM | Stop LAN DHCP client; result returned by stop_udhcpc_lan(). | Verify expected udhcpc identity plus start time; missing, malformed or legacy-unverifiable file is not permission to signal. |
| rc/src/net_wan.c:4321 | /var/run/udhcpcN.pid, SIGTERM | Stop per-unit WAN DHCP client; result returned. | Same identity contract; include all unit PID files in WAN preflight. |
| rc/src/net_wan.c:4329 | /var/run/udhcpc_viptv.pid, SIGTERM | Stop IPTV DHCP client; result returned. | Same identity contract; include IPTV stop before bridge/VLAN/interface teardown. |
| httpd/src/web_ex.c:278 | /var/run/lpdparent.pid, SIGUSR2 | Printer notification from HTTPD. Expected owner/writer of this exact filename was not found in the BusyBox 1.24.x printutils/lpd.c source reviewed; ownership requires further confirmation. | Treat as UNKNOWN until exact writer and identity contract are identified; do not infer identity from filename or PID alone. |
| httpd/src/web_ex.c:2727; rc/src/detect_internet.c:375,381 | /var/run/detect_internet.pid, SIGHUP/SIGUSR1/SIGUSR2 | HTTPD/RC notifications to in-tree detect_internet daemon. | Migrate writer and all readers atomically to PID+start-time format; no fallback signal on legacy numeric data. |
| rc/src/detect_link.c:360,366 | /var/run/detect_link.pid, SIGUSR1/SIGHUP | In-tree detect_link event notifications. | Same PID+start-time migration and fail-closed rule. |
| httpd/src/web_ex.c:5019; rc/src/rstats.c:643 | RSTATS_PID_FILE, signal selected by HTTPD or SIGHUP | Statistics daemon refresh/notification. | Migrate producer and both consumer paths together; status/error reaches caller. |
| rc/src/vpn_server.c:457 | /var/run/pppN.pid, SIGKILL | Force terminates server client sessions after stopping bcrelay/pptpd/xl2tpd. | All client PID files are parsed and identity-checked in the same preflight as server daemons; one UNKNOWN blocks the whole service group before first signal. |
| rc/src/watchdog.c:634,645 | /var/run/watchdog.pid, SIGHUP/SIGUSR1 | Watchdog time/Wi-Fi notification. notify_watchdog_time() first checks pids(); notify_watchdog_wifi() signals the PID file directly. | Both notification routes use the same verified identity lookup. UNKNOWN never falls back to a direct PID signal. |

There are 15 kill_pidfile_s() call expressions in the inventory. Fourteen are external to shared/shutils.c; the fifteenth is the internal kill_pidfile() wrapper calling kill_pidfile_s(..., SIGTERM). The five kill_pidfile() call candidates are the two common_ex.c calls above plus the three udhcpc stop calls.

### PID-file producers located

- detect_internet.c:458-466 writes DI_PID_FILE as decimal PID only.
- detect_link.c:453-461 writes DL_PID_FILE as decimal PID only.
- watchdog.c:717-725 writes WD_PID_FILE as decimal PID only.
- rstats.c:715-723 writes RSTATS_PID_FILE as decimal PID only.
- BusyBox 1.24.x udhcpc: net_lan.c and net_wan.c pass -p PIDFILE. networking/udhcp/dhcpc.c calls write_pidfile() at startup and rewrites the file after daemonization. This is a BusyBox-owned writer; keep its procps scanner separate and check all BusyBox DHCP consumers before changing it.
- PPPD 2.4.7: pppd/main.c create_pidfile() writes numeric PID to /var/run/pppN.pid. create_linkpidfile() writes numeric PID followed by interface name to /var/run/ppp-LINKNAME.pid. VPNC uses /var/run/ppp-VPNC.pid, which the generic helper parses as only its first numeric line today. PPPD producer needs a coordinated change to emit start time.
- INICD writes /var/run/inicd.pid as decimal PID in inicd.c:130-133 but does not use kill_pidfile()/kill_pidfile_s(); stop_inicd() uses killall -q. It is a separate direct process-control route.
- A matching producer for /var/run/lpdparent.pid was not established in the inspected BusyBox lpd.c or call-site evidence. Do not assume that PID file is trustworthy until writer identity is resolved.

### Current shared PID-file parser defect

shutils.c:569-597 reads one line into a fixed 64-byte buffer, calls strtoul(buf, NULL, 0), and signals any result between 1 and ULONG_MAX. It does not validate errno, end pointer, exact numeric syntax, extra/truncated bytes, file read error, file type/ownership, process command identity or /proc/PID/stat start_time. The comment says it returns errno on failure, but the implementation actually returns kill()'s 0/-1 for a parseable PID and leaves fopen/parse failure as -1 with incidental errno.

A PID+start-time record reduces stale-PID reuse risk but does not make check-then-kill atomic. This product's 4.4 kernel does not provide pidfd_send_signal(); document the residual TOCTOU between re-reading /proc/PID/stat and kill(pid, sig), and do not claim race-free identity binding. Every signal escalation still requires an immediate identity/start-time recheck.

## Direct signal, process-command and wait/reap routes

Targeted non-BusyBox process-control paths reviewed in addition to the public APIs:

| File and lines | Route | Semantic classification |
|---|---|---|
| httpd/src/web_ex.c:336 | killall inadyn SIGUSR1 after pids() | UNKNOWN currently authorizes notification; migrate to verified target signal. |
| httpd/src/web_ex.c:4718 | killall watchdog before NVRAM restore | Unconditional daemon termination followed by restore; watchdog is restarted on restore failure. Check/propagate stop and restore result, and ensure it is safe to leave watchdog down during restore. |
| rc/src/auth.c:166 | killall udhcpc SIGUSR1 | Located in disabled #if 0 path; not active at this baseline. Keep documented, do not treat as active caller. |
| rc/src/common_ex.c:1141 | Generic killall adapter for service_lifecycle_stop() | Central service signal implementation; name-based killall is not bound to pre-scanned PID identities and can race process replacement. Replace with snapshot/identity-bound signaling where kernel permits. |
| rc/src/detect_internet.c:346; detect_link.c:345 | killall -q stop helpers | Stop helpers do not return or propagate killall outcome. Callers must not assume confirmed stop. |
| rc/src/detect_wan.c:84 | killall udhcpc SIGUSR1 | pids()-guarded and unsafe for UNKNOWN. |
| rc/src/inicd.c:57 | killall -q inicd | stop_inicd() returns command status, but does not bind signal to owned PID file/start time. |
| rc/src/net_lan.c:811 | killall httpd SIGUSR1 after LAN restart | Best-effort management logout/reload notification after network change; direct name-wide signal has no identity snapshot. |
| rc/src/net_wan.c:1875 | killall udhcpc SIGUSR2 | Precedes WAN kill_services(); UNKNOWN can authorize signal before later stop gate. |
| rc/src/services.c:204 | killall networkmap SIGUSR1 | pids()-guarded; UNKNOWN authorizes signal. |
| rc/src/services_ex.c:819,981,1012,1038,1301,1624; services_stor.c:984 | killall DNSMasq/Samba/miniupnpd/inadyn reload signals | Several paths have pids() guard/snapshot. UNKNOWN can authorize reload, or > 0 can incorrectly suppress it as ABSENT. Revalidate before each signal. |
| rc/src/usb_modem.c:794,795 | killall usb_modeswitch/eject | Best-effort cleanup of short-lived helper processes; not covered by pids() guard. Track only owned child/helper identities if concurrent overlap is possible. |
| rc/src/usb_modem.c:803 | killall udhcpc SIGUSR2 | pids()-guarded and unsafe for UNKNOWN. |
| rc/src/watchdog.c:147,157 | killall wget | pids()-guarded cleanup of HTTPD probe jobs; target only the probe child launched by this watchdog path. |
| rc/src/watchdog.c:497 | killall -9 httpd | SIGKILL after port probe failure is not safely gated by authoritative inventory and can kill unrelated/replaced HTTPD. UNKNOWN blocks escalation and DETECT_HTTPD_FILE removal/restart. |

Raw kill() calls in init/notification/RC control paths were kept separate from service process discovery: signals to PID 1 are control-plane notifications, and common_ex.c waitpid() calls supervise children created by that helper. init.c reaps its own adopted children. These are not substitutes for a complete daemon inventory and must not be swept into the shared pids() API mechanically.

## Build and conditional-compilation boundaries

- trunk/user/shared/src/Makefile compiles pids.c into libshared.so and installs the shared headers. This produces the shared pids()/PID-file API.
- RC links against the staged shared library. trunk/user/rc/src/Makefile includes common_ex.c, services.c, services_ex.c, network services and watchdog in the base RC source list; vpn_client.c and vpn_server.c are also in the RC source list.
- HTTPD is a separate consumer package and links against the staged shared library. aidisk.c and web_ex.c callsites are HTTPD package code, not RC applets.
- services_stor.c is conditional on STORAGE_ENABLED; storage callsites must be compiled/tested under that condition.
- services_usb.c and usb_modem.c are conditional on USB support; printer paths additionally use SRV_LPRD/SRV_U2EC feature guards.
- net_wan6.c is under IPv6 configuration. Wireless authenticator stops have board/radio feature guards, including BOARD_HAS_5G_RADIO. Plugin/status hooks in services.c/services_ex.c/web_ex.c are guarded by APP_* options.
- BusyBox's implementation at trunk/user/busybox/busybox-1.24.x/libbb/procps.c is not the Padavan shared scanner. BusyBox udhcpc and lpd are PID-file producer/consumer contexts, but that does not make them clients of shared pids(). Any BusyBox change must be narrowly justified by PID-file migration and checked against BusyBox applet consumers/configuration.

## Regression status and phase-2 requirements

Existing service_lifecycle_policy.c tests cover six Boolean-policy cases: graceful stop, force kill, delayed exit, stubborn service, non-forced timeout and empty service list. They do not inject UNKNOWN and do not assert all-target preflight or zero signals on unknown.

The new suite must use GNU89, -Wall, -Wextra and -Werror, with ASan/UBSan where available. At minimum:

1. Scanner context allocation failure.
2. /proc open failure, normal EOF and readdir error distinguished.
3. A PID disappearing with ENOENT/ESRCH versus permission/I/O/stat parse error.
4. argv0 allocation failure and PID vector growth failure.
5. Empty, one-match and multi-match inventories; no partial result on UNKNOWN.
6. UNKNOWN in the first, middle and last member of a multi-service set blocks every signal before first TERM.
7. UNKNOWN blocks SIGKILL, stopped-success, restart and destructive cleanup.
8. Each TERM-to-KILL escalation revalidates exact PID/start-time identity; changed identity blocks escalation.
9. Strict PID-file parsing: empty, whitespace, signs, overflow, trailing garbage, truncated line, extra records, read failure, missing file and invalid PID.
10. PID-file identity: current PID/start time, mismatched command, mismatched start time, stale/reused PID, PID disappearance between read and signal, and legacy numeric files.
11. Failure propagation from VPN Server, VPN Client, USB modem removal, AP LAN teardown, flash_firmware()/stop_services(), Samba, HTTPD/crond/GPIO readiness and storage eject.

### Phase-2 design constraints

- Do not silently preserve ambiguous Boolean wrappers for migrated callers. All 77 direct callers must use explicit state comparisons or a helper with a documented UNKNOWN outcome.
- Keep count/snapshot capability separately from RUNNING/ABSENT/UNKNOWN state. A count cannot double as a tri-state enum.
- A service stop enumerates/preflights every configured name before first signal. It sends no TERM if any target is UNKNOWN.
- Before each TERM or KILL, revalidate exact process identity from the preflight snapshot. Do not substitute killall after taking an authoritative snapshot because killall can race and signal a replacement.
- A PID-file record binds PID to process start time and expected process identity. In-tree monitor writers can be migrated together. BusyBox udhcpc and PPPD writers are separate owners and must be explicitly included or old numeric records remain UNKNOWN/no-signal.
- Linux 4.4 lacks pidfd-based atomic signaling. Document residual check/kill race; do not overstate start-time validation.
- Migrate upper-layer stop functions to return status and block dependent cleanup/restart. A safe lower-level policy is insufficient while callers ignore errors.

## Verification boundary

- L1 lexical inventory: PASS on source SHA d0404d5e33d4239e19c92db9b9ab4fb7529cf26c; 6,193 readable text files, zero missing/read errors. Requested direct API semantic caller classification in this document is complete for the exact direct call candidates listed above.
- L2 production scanner/lifecycle regressions: NOT RUN. No scanner, policy, caller or PID-file source changed in this milestone.
- L3 MIPS/uClibc target build: historical evidence only at source SHA 691e4d68a27580d0f053a8b4c9ae7a71bff03e6f; no new build in this milestone.
- L4 XY-C1 hardware flash/boot/runtime: NOT VERIFIED.
