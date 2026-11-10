## 2026-11-10 11:34 CST (UTC+8) — Correct a formatting-sensitive MDTM workflow assertion

- **Base main SHA:** `07ba33d7ffd92dbaee037ca35d4f20f051070153`. The first VSFTPD workflow run [42749731924] compiled and executed the GNU89 host regression successfully, including the new pinned-inode metadata replacement case. The Python source-contract step then failed at its assertion requiring the literal substring `vsf_sysutil_setmodtime_fd(opened_file`; the production call intentionally placed the descriptor argument on the next line.
- **Correction:** make the source contract assert the function name and the following `opened_file, modtime, tunable_use_localtime` argument independently. This validates the same descriptor relationship without coupling to line wrapping. No production code changed in this follow-up.
- **Validation:** the prior host binary result is PASS on `07ba33d7ffd92dbaee037ca35d4f20f051070153`; the corrected Python semantic contract and updated AI handoff check are pending on THIS resulting SHA.
- **Next:** reconcile exact-head VSFTPD and AI docs Actions, then return to the still-open upload-cleanup check/unlink race. Do not claim the race is closed.

---

## 2026-11-10 11:20 CST (UTC+8) — Move FTP MDTM and SITE CHMOD metadata to pinned descriptors

- **Base main SHA:** `c5ec0e4c0d33e53f784a06763d479cddbc35dcbd`. AI handoff docs [42747120963] and HTTPD authentication/disclosure regression [42747121115] completed SUCCESS on this exact base SHA.
- **Finding:** FTP `MDTM` and `SITE CHMOD` checked the configured path filter and share ACL by pathname, then re-resolved the pathname for `utime()` / `chmod()`. A rename/substitution between authorization and mutation could direct metadata changes at a different inode.
- **Fix:** retain `vsf_access_check_file()` filtering and route MDTM/CHMOD through `asus_open_existing_path()` so share authorization and final regular-file identity are pinned by a descriptor. MDTM reads the opened inode via `fstat()` and sets both timestamps via new `vsf_sysutil_setmodtime_fd()` (`futimes`) while preserving the existing timezone conversion; SITE CHMOD uses new recoverable `vsf_sysutil_fchmod_ret()` instead of path-based `str_chmod()`. Sysutil wrappers return syscall failures for FTP error responses rather than invoking the fatal legacy wrapper. The original MDTM timestamp-write syntax discriminator and disabled-write failure response are retained.
- **Regression:** extend the GNU89 path-policy host binary: pin an existing file through `ftp_acl_path_open_existing_file()`, rename it away, install a replacement at its former leaf, then apply `fchmod()` and `futimes()` through the pinned descriptor. Verify the original inode/mode/mtime changed and the replacement stayed unchanged. Extend VSFTPD workflow source contracts for both handlers and the sysutil APIs.
- **Changed paths:** `trunk/user/vsftpd/src/postlogin.c`; `trunk/user/vsftpd/src/sysutil.c`; `trunk/user/vsftpd/src/sysutil.h`; `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`; `.github/workflows/vsftpd-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 source/control-flow review PASS. Host regression and semantic checks are wired into the VSFTPD workflow but not yet executed on THIS resulting SHA. No local full source checkout, target MIPS/uClibc build, firmware/TRX build or device test is claimed.
- **Known limitation intentionally not closed here:** `ftp_acl_path_unlink_opened_entry()` still performs a leaf identity check followed by `unlinkat()`, leaving a narrow replacement race between those operations. The current regression covers replacement before the check only. Audit Linux 4.4 syscall availability and design a separate fail-closed/quarantine change; do not claim unlink-by-inode atomicity.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head VSFTPD + AI docs Actions; after green, investigate and safely bound the upload-cleanup race. Keep FULLCONENAT target MIPS, NFS compatibility, bridge idempotency, PPP/IPv6 and L3/L4 validation open.

---

## 2026-11-10 11:12 CST (UTC+8) — Remove cross-step Python helper dependency from HTTPD DHCP contract

- **Base main SHA:** `fc0d7b6694a58cd5b396ea2219765803c257913d`; remote branches: `main` only.
- **Exact-head Actions:** AI handoff docs [42746896717] completed SUCCESS. HTTPD [42746896790] failed in `Verify DHCP lease input and renewal reconciliation`, Python line 159, with `NameError: name 'body' is not defined`.
- **Failure analysis:** the prior contract-only correction checked the shared `manv_route_deconfig()` helper implementation successfully, but called `body()` to extract the dispatcher. That helper is defined in a different `run: |` Python process and is not visible in this block. This is test-harness scope, not production C.
- **Fix:** replace the cross-step helper call with direct bounded slicing from `udhcpc_viptv_main(int argc, char **argv)` to the next `zcip_main()` definition, then verify the shared helper dispatch call. No production code changes.
- **Changed paths:** `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** source structure and exact replacement reviewed. Corrected HTTPD contract and AI handoff docs are pending on THIS commit; no local workflow runner or production target build was available.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head HTTPD + AI docs Actions. Once green, continue FTP MDTM/SITE CHMOD descriptor-relative authorization and upload-cleanup race analysis; retain FULLCONENAT MIPS, NFS, bridge and L3/L4 targets.

---

## 2026-11-10 11:05 CST (UTC+8) — Correct HTTPD DHCP contract for shared IPTV route cleanup

- **Base main SHA:** `e79e88c011dca8c1a046b1df945cd57f300ff3f4`; remote branches: `main` only.
- **Exact-head Actions:** WAN callback [42746555691], WAN teardown [42746555514], WAN route [42746555502], and AI handoff docs [42746555591] completed SUCCESS. HTTPD [42746555780] failed only at `Verify DHCP lease input and renewal reconciliation`, Python line 155. Its preceding production/policy compile and test plus all earlier checks passed.
- **Failure analysis:** the stale contract required `del_dhcp_routes_by_prefix("manv_", ...)` to appear after `udhcpc_viptv_main()`. The ZeroConf work intentionally factored DHCP/IPTV route inventory cleanup into the earlier shared `manv_route_deconfig()` helper, and the dispatcher delegates to it. This is a test-extractor assumption mismatch, not a demonstrated production defect.
- **Fix:** update `.github/workflows/httpd-security.yml` to check the shared helper implementation contains the DHCP route deletion and that `udhcpc_viptv_main()` calls the helper. No production source changes in this follow-up.
- **Changed paths:** `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** assertion replacement reviewed against the exact helper/caller layout. The corrected HTTPD workflow and AI handoff docs are pending Actions on THIS resulting SHA; no new host binary or production target compile was run locally.
- **Resulting commit:** THIS COMMIT.
- **Next:** inspect exact-head HTTPD + AI docs Actions. Once green, begin FTP MDTM/SITE CHMOD descriptor-relative authorization and upload-cleanup race audit; retain FULLCONENAT MIPS, NFS, bridge membership and L3/L4 limits.

---

## 2026-11-10 10:47 CST (UTC+8) — Make ZeroConf WAN/IPTV callbacks transactional and teardown retry-safe

- **Base main SHA:** `ea9c91531e9611a8fc4bf227137c3ea6aa3f5ed5`; remote branches: `main` only, no checkpoint branch.
- **Recovered exact-head Actions:** the preceding static-IP IPTV startup fix [42744955204], WAN interface teardown [42744955205], route status [42744955153], HTTPD security [42744955208] and AI handoff docs [42744955469] all completed SUCCESS on this exact SHA. The WAN callback job compiled and ran both interface rollback and static-IPTV GNU89 binaries and passed the expanded source contract.
- **Findings:** `zcip_bound()` and `zcip_viptv_bound()` ignored clear/set `ifconfig()` failures and published the ZeroConf lease before confirming the address was installed; MAN route setup was hidden behind the public void `man_up()` wrapper. `man_down_internal()` cleared gateway/DNS/route inventory even after route deletion failures, and `udhcpc_deconfig()` marked the old MAN address clearable even if its interface-clear ioctl failed. IPTV ZeroConf had a no-op `deconfig` implementation.
- **BusyBox lifecycle boundary:** inspected this fork's BusyBox 1.24 `networking/zcip.c`. The IPTV launcher uses `-q`; after the `config` callback the applet returns before entering MONITOR, so successful normal IPTV ZeroConf setup does not later invoke `deconfig`. Kept `-q` unchanged (no unmanaged persistent worker was introduced) but implemented safe defensive deconfig if the event is dispatched by another path.
- **Fix:** validate input before mutation; apply WAN/IPTV link-local addresses through `dhcp_wan_interface_apply()` with checked clear/set and previous-address restoration; only then publish `wanx_*`/`manv_*` state. WAN ZeroConf now calls `man_up_internal()` and returns/logs route setup errors. MAN teardown only clears lease/route source state after every route deletion succeeds; incomplete teardown preserves it. The MAN deconfig caller marks the old address clearable only after successful interface clear. Share IPTV route cleanup through `manv_route_deconfig()`; defensive ZeroConf IPTV deconfig checks address clear, retains `manv_ipaddr` unless both address and route cleanup succeed, stops multicast, and returns the first error.
- **Regression:** add an RFC3927 first-lease case to the GNU89 interface transaction suite. Extend the WAN callback Actions source contract for both transactional ZeroConf handlers, MAN teardown preservation, ifconfig-gated address clearing, shared IPTV route cleanup, defensive IPTV deconfig, and BusyBox `-q` config/exit ordering. Add BusyBox source to push/PR path filters.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; `trunk/user/rc/src/tests/dhcp_wan_interface_policy.c`; `.github/workflows/wan-dhcp-callback-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 source/control-flow and lifecycle review PASS. This commit's GNU89 host regression, extended semantic workflow, and AI handoff validation are pending on the exact resulting SHA. No local checkout/compiler or docs-checker run was available. Production RC/MIPS/uClibc object, firmware/TRX and L4 router validation are NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile WAN callback, HTTPD and AI handoff Actions on the exact new SHA. If green, leave WAN callbacks for now and continue bounded FTP MDTM/SITE CHMOD descriptor-relative metadata and upload-cleanup race review; keep FULLCONENAT target compilation, NFS compatibility, wireless bridge idempotency and L3/L4 validation open.

---

## 2026-11-10 10:25 CST (UTC+8) — Gate static-IP IPTV startup on interface and route status

- **Base main SHA:** `875f85edd72362c54b9aa373b6e451adba27e345`. The preceding HTTPD restart-preflight contract correction [42742135752] and AI handoff documentation check [42742135820] both completed SUCCESS on that exact SHA. Wireless [42741673513], USB storage [42741673496], WAN route status [42741673402], vsftpd [42741673364] and Samba [42741673346] completed SUCCESS on its parent code SHA `4cb9d3dc4d2644e02dd03e717eb5f019c7a28c42`.
- **Finding:** `launch_viptv_wan()` published static `manv_ipaddr`/`manv_netmask` before checking `ifconfig()`, discarded the address ioctl result, and ignored the metric-10 default-route result. It then started IGMP proxy and returned success even when the interface or route setup failed. An invalid static mode/address combination also fell through to the DHCP IPTV client.
- **Fix:** add a small, host-testable static-IPTV startup policy. Address configuration is now the first operation; failed address setup prevents NVRAM publication, route creation and multicast startup. On success, publish the IP/netmask and either the valid gateway or an empty gateway before route installation so teardown retains source state; propagate route errors while still attempting multicast startup. Static mode with invalid address/netmask now fails closed instead of starting DHCP. Propagate the result through `launch_viptv_wan()` and make the caller's diagnostic cover VLAN and service setup failures.
- **Regression:** new GNU89 `-Wall -Wextra -Werror` host policy tests cover address failure/no publication, successful address-to-state-to-route-to-multicast order, route failure with recovery state retained, no-gateway behavior, and invalid policy arguments. Extend the WAN DHCP callback workflow with the new helper/test path filters and source-order checks for the production wrappers and static-mode dispatch.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; new `trunk/user/rc/src/wan_viptv_static_policy.h`; new `trunk/user/rc/src/tests/wan_viptv_static_policy.c`; `.github/workflows/wan-dhcp-callback-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 control-flow review PASS. Exact-host GNU89 policy binary and static IPTV semantic source contract are configured but pending on THIS resulting SHA. The dependent HTTPD workflow and AI docs validation also need exact-head reconciliation. No local checkout/compiler was available; no MIPS/uClibc production object, firmware/TRX or L4 router test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** after exact-head Actions pass, audit ZeroConf MAN/IPTV callback lifecycle in `zcip_bound()`, `zcip_viptv_bound()`, `zcip_main()` and `zcip_viptv_main()`; specifically fix interface failure propagation and the IPTV no-op `deconfig` cleanup with appropriate multicast and `manv_*` retention. Keep bridge idempotency, FULLCONENAT target compilation, FTP metadata/upload cleanup, NFS compatibility and L3/L4 validation open.

---

## 2026-11-10 10:19 CST (UTC+8) — Correct HTTPD wireless preflight contract for status-gated startup

- **Base main SHA:** `4cb9d3dc4d2644e02dd03e717eb5f019c7a28c42`, the AP/WDS/APCLI startup-failure propagation commit. Exact-head Wireless [42741673513], AI handoff docs [42741673482], USB storage [42741673496], WAN route status [42741673402], vsftpd [42741673364] and Samba [42741673346] completed SUCCESS.
- **Failure found:** HTTPD security [42741673534] passed its GNU89 policy tests and the wireless configuration preflight source check, then failed in its subsequent legacy Python extractor. The extractor called `restart.index("start_wifi_ap_wl(radio_on);")`; the updated restart code correctly places the helper within `if (start_wifi_ap_wl(radio_on) < 0 || ...)`, so the literal no longer exists. The observed exception was `ValueError: substring not found`, not a production-code or preflight-order failure.
- **Correction:** update the HTTPD workflow's ordering contract to locate the status-gated AP helper expression (`if (... < 0 ||`) for each radio. The assertion still proves configuration generation precedes stop, and that the reload marker remains after shutdown and before startup.
- **Changed paths:** `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** exact base SHA and all seven run outcomes above were inspected; the wireless GNU89 policy/semantic test and AI handoff docs both passed on exact SHA `4cb9d3dc4d2644e02dd03e717eb5f019c7a28c42`. THIS correction's HTTPD security and AI documentation Actions are pending on its resulting SHA. No MIPS/uClibc, firmware/TRX or router-device validation is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify all Actions on the exact resulting SHA. If green, rotate to static-IP IPTV/ZeroConf callback lifecycle in `net_wan.c`; keep the bridge idempotency caveat, FULLCONENAT target compile, FTP metadata/upload cleanup, NFS and L3/L4 validation open.

---
## 2026-11-10 09:48 CST (UTC+8) — Propagate wireless AP/WDS/APCLI startup failures

- **Base main SHA:** `1de8fe9466a5df0273d42551e1d54312ed087f46`. The exact-head Samba [42736815814], vsftpd [42736815815], HTTPD [42736815819], USB storage [42736815820], wireless shutdown [42736815829], WAN route [42736815917] and AI handoff [42736816088] workflows all completed SUCCESS on that base SHA.
- **Finding:** the six `start_wifi_ap_*`, `start_wifi_wds_*` and `start_wifi_apcli_*` helpers returned `void`. AP/APCLI helpers silently returned after failed `ifconfig up`; WDS helpers skipped only the affected bridge-add command and discarded the interface-up failure. Both restart paths then started 802.1X and reported success even when an interface did not come up. The RT3352 MII helper similarly hid failed `rai0` startup from the 2.4GHz caller.
- **Fix:** change the six helper APIs to return status. AP/APCLI interface-up and APCLI AutoConnect command failures now return `-EIO`; WDS helpers preserve a sticky aggregate error while continuing to attempt every configured interface. RT3352 MII startup returns a failure on unsuccessful `rai0` bring-up, and the intentionally disabled iNIC path skips guest startup rather than being misclassified as a device fault. The restart callers gate 802.1X startup and return failure after best-effort interface rollback; a DBDC peer failure also stops the already-started local 802.1X process and rolls back both radios. A failed `inicd` health-monitor launch is logged without taking otherwise functioning 2.4GHz service offline.
- **Bridge attachment audit:** do not propagate raw add ioctl errors yet. `br_add_del_if()` returns the raw `SIOCBRADDIF` result, while this repository's Linux 4.4 `br_add_if()` returns `-EBUSY` whenever `netdev_master_upper_dev_get(dev)` is non-NULL, even if the interface is already attached to the same bridge. Wireless stop only lowers interfaces and leaves existing AP/WDS/APCLI bridge membership in place; direct status checking would falsely fail ordinary repeated starts. A later bridge-focused change needs an idempotent membership/master query and dedicated regression before these results can be trusted.
- **Regression:** extend the GNU89 warnings-as-errors policy test for sticky startup aggregation and success/error conversion. Extend the wireless source contract for all AP/APCLI error returns, AutoConnect failures, WDS full-attempt order, iNIC propagation, DBDC rollback and authentication gates; include Linux 4.4 bridge source in path filters and assert the `-EBUSY` behavior.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `trunk/user/rc/src/rc.h`; `trunk/user/rc/src/wireless_radio_policy.h`; `trunk/user/rc/src/tests/wireless_radio_policy.c`; `.github/workflows/wireless-radio-state.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 source/control-flow review PASS. The policy binary and Python source/kernel contract are configured in the exact-head GitHub Actions workflow but are pending on THIS resulting SHA. The AI handoff documentation check is also pending. No local checkout was available for independent execution; no RC/MIPS/uClibc object, firmware/TRX or L4 device validation is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile all workflows against the exact resulting main SHA. If green, rotate to static-IP IPTV/ZeroConf callback lifecycle in `net_wan.c`. Keep FULLCONENAT target compilation, FTP MDTM/SITE CHMOD and upload-cleanup race, NFS compatibility, idempotent bridge membership and L3/L4 validation open.

---
## 2026-11-10 09:16 CST (UTC+8) — Propagate incomplete wireless shutdown into restart policy

- **Base main SHA:** `e1f27e5a5fb607a6ca8177c6cf1f220dc3515e7e`. HTTPD/DNSMasq generated-file regression [42735569321], AI handoff documentation [42735569423] and Samba config regression [42735569085] all completed SUCCESS on that exact SHA.
- **Finding:** `stop_wifi_all_wl()` and `stop_wifi_all_rt()` already aggregated per-interface down-command failures to retain the radio LED, but returned `void`. Both `restart_wifi_wl()` and `restart_wifi_rt()` discarded that aggregate and continued stopping 802.1X services and launching replacement interfaces. In DBDC builds the peer-radio shutdown result was also ignored. An interface that resisted shutdown could therefore coexist with a newly published wireless configuration while the restart API returned success.
- **Fix:** both shutdown functions now return an RC-style negative error for an incomplete stop. The single-radio and DBDC restart paths gate 802.1X teardown and replacement-interface startup on successful shutdown of the current and, where applicable, peer radio. The shutdown helper still attempts every interface and preserves the LED state on any failure. Public declarations were updated.
- **Regression:** GNU89 policy regression covers success-to-zero and incomplete-shutdown-to-`-EIO`. The wireless source contract checks both radio shutdown gates and DBDC peer ordering; `rc.h` was added to push/PR path filters.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `trunk/user/rc/src/rc.h`; `trunk/user/rc/src/wireless_radio_policy.h`; `trunk/user/rc/src/tests/wireless_radio_policy.c`; `.github/workflows/wireless-radio-state.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 source review PASS. Exact-head GNU89 policy test, wireless restart semantic contract and AI handoff validation are pending on THIS resulting SHA. No local checkout was available for independent execution; production RC/MIPS/uClibc build and L4 radio/device behavior are NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile wireless radio-state and AI handoff Actions against the exact resulting SHA. If green, continue AP/WDS/APCLI startup-helper status propagation and bridge attachment result auditing, then rotate to another subsystem. Static-IP IPTV/ZeroConf, FTP metadata/upload-cleanup race, FULLCONENAT target validation, NFS compatibility and L3/L4 remain open.

---

## 2026-11-10 09:15 CST (UTC+8) — Gate DNSMasq lease-file validation on actual DHCP lease use

- **Base main SHA:** `186d3e6a2a392c21045aced3a3754073853c4c2e`. WAN DHCP callback [42733942876], WAN route status [42733942923], WAN interface teardown [42733942837], HTTPD security [42733942826] and AI handoff validation [42733942873] all completed SUCCESS on that exact parent SHA.
- **Finding:** `prepare_dns_dhcpd()` unconditionally validated/created `/tmp/dnsmasq.leases` before the DHCP feature flags were known. DNS-only router mode and AP-mode RA-only mode can generate a valid dnsmasq configuration without a `dhcp-leasefile` directive, yet an unrelated stale symlink, hardlink, FIFO or non-regular lease-path object made preparation fail. The restart policy then preserved a running daemon (or aborted first start) even though the service did not consume that file.
- **Fix:** define the DHCP lease/RA feature bits and gate `dnsmasq_lease_file_ensure()` on the finalized lease-using feature bit, immediately before writing the matching `dhcp-leasefile` directive. On rejection, close and discard the staged configuration and return the saved error. DNS-only and RA-only paths bypass an unused lease file; configurations that actually use DHCP leases retain the existing fail-closed symlink/hardlink/FIFO checks.
- **Regression:** extend the existing GNU89 DNSMasq file test for no-feature, RA-only, lease-using and combined flags. Extend the HTTPD security semantic contract to verify that the lease-file check is guarded, precedes the config directive, and discards the staged main configuration on failure.
- **Changed paths:** `trunk/user/rc/src/services_ex.c`; `trunk/user/rc/src/dnsmasq_file.h`; `trunk/user/rc/src/tests/dnsmasq_file.c`; `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 control-flow review PASS. GNU89 host execution, HTTPD/DNSMasq semantic source contract and AI handoff validation are pending on THIS resulting SHA. No local clone/compiler was available for an independent host run; production RC/MIPS/uClibc and firmware builds and L4 router tests are NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile the DNSMasq and AI handoff Actions on the exact resulting SHA. If green, rotate to the wireless driver/interface start-stop and bridge-state lifecycle audit; inspect `net_wifi.c` and `ralink.c` for a concrete failure-propagation or stale-state defect before editing. Keep static-IP IPTV/ZeroConf, FTP MDTM/SITE CHMOD and upload-cleanup race, FULLCONENAT target validation, NFS compatibility and L3/L4 open.

---

## 2026-11-10 09:03 CST (UTC+8) — Propagate IPTV DHCP callback interface and route failures

- **Base main SHA:** `a86e2ceab5f6a448acfe82b8f388529a9941c3d3`; VSFTPD [42731280222] and AI handoff docs [42731280294] both completed SUCCESS on that exact SHA.
- **Finding:** `udhcpc_viptv_bound()` discarded `ifconfig()`, `route_del()`, `del_dhcp_routes_by_prefix()`, `add_dhcp_routes()` and default `route_add()` results. It could publish a replacement `manv_*` lease/route inventory while old routes remained, or report a bound lease despite interface/route setup failure. The IPTV `deconfig` branch also erased gateway/route metadata regardless of cleanup result.
- **Fix:** reuse the existing DHCP WAN address apply/rollback policy for IPTV; snapshot old address/netmask/gateway; verify the candidate address before route mutation; aggregate old default/DHCP route deletion before publishing new state; restore the old address and retain its route inventory if teardown fails; aggregate/log new route-add failures. Deconfig now clears route inventory only when all requested deletions succeed and returns the first error.
- **Regression:** extend the existing GNU89 WAN interface-policy workflow with IPTV source-order checks for address rollback, delete-before-publish, route-add propagation and deconfig inventory retention. The existing route-status policy test verifies first-error preservation while continuing subsequent operations.
- **BusyBox boundary:** the prior bounded applet audit established that `udhcp_run_script()` deliberately ignores the script's exit status. This change does not alter BusyBox state transitions; callback logs and retained NVRAM route inventory are the observable failure/recovery signals.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; `.github/workflows/wan-dhcp-callback-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 source review PASS. The current parent SHA's VSFTPD and documentation Actions are green. THIS host-policy and IPTV semantic source-contract Actions are pending on the resulting SHA; the workflow does not compile production `net_wan.c`. No MIPS/uClibc object, firmware/TRX or router hardware result is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile every Actions workflow triggered by the exact new SHA. If green, rotate to a different subsystem and trace a bounded DNSMasq/resolver lifecycle stability candidate; keep static-IP IPTV/ZeroConf, FTP metadata/upload cleanup, FULLCONENAT target build, NFS and L3/L4 work open.

---

## 2026-11-10 08:49 CST (UTC+8) — Report incomplete FTP STAT directory listings accurately

- **Base main SHA:** `edd8688641cd5bb6fcefea325880f7c4c56aa511`; VSFTPD [42730772799] and AI handoff docs [42730772420] both completed SUCCESS on this exact SHA. The FIFO cleanup correction passed all three GNU89 host policy binaries and the full Python semantic source contract.
- **Finding:** `handle_dir_common()` always sent `213 End of status` for STAT after starting a multiline reply, regardless of the return from `vsf_ftpdataio_transfer_dir()`. A recursive LIST/STAT depth or entry-cap abort (and other transfer errors) was thus presented as a completed listing. A missing/unopenable directory or an anonymous directory rejected by `anon_world_readable_only` also left `retval == 0` and produced the same success wording.
- **Fix:** retain the 213 completion code required to terminate the multiline response, but select a clearly incomplete/unavailable final text when listing transfer failed or the directory could not be listed. Successful STAT retains the existing final text. No LIST/NLST data-channel response behavior is changed.
- **Regression:** extend the GNU89 ACL/listing policy fixture to verify complete/incomplete response text constants; extend the VSFTPD semantic workflow to assert failure propagation, unavailable-path handling and conditional final response selection.
- **Changed paths:** `trunk/user/vsftpd/src/postlogin.c`; `trunk/user/vsftpd/src/ftp_listing_policy.h`; `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`; `.github/workflows/vsftpd-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this log.
- **Validation:** L1 control-flow review PASS. The base VSFTPD and docs Actions cited above are green on the exact parent SHA. THIS GNU89 host test, Python semantic contract and AI docs checks are pending on the resulting SHA. The workflow does not compile production `postlogin.c`; no MIPS/uClibc object, firmware/TRX or router hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions. If green, shift the next batch to IPTV DHCP callback route/interface status propagation in `trunk/user/rc/src/net_wan.c`; keep remaining FTP metadata and upload-cleanup work queued.

---

## 2026-11-10 08:42 CST (UTC+8) — Remove the FTP ACL FIFO fixture during teardown

- **Base main SHA:** `cd3d66c4687d8e207ce293c088f2920b1ccc7a6d`; VSFTPD run [42728646505] FAILED and AI handoff docs [42728646431] completed SUCCESS on this exact SHA.
- **Failure analysis:** the new ACL-path fixture created `acl-config-fifo` and correctly rejected it without blocking, but the final cleanup chain omitted that FIFO before removing `shareA`. The Actions log reports `remove fixture: Directory not empty`; the preceding FTP config-file and daemon-policy binaries passed. The workflow stopped before the Python semantic source-contract step.
- **Fix:** add `unlink(fifo_file)` to the fixture teardown before `rmdir(a)`. This is a test-fixture lifecycle correction; no production behavior is changed.
- **Changed paths:** `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 review confirms the FIFO is removed before its parent directory. Exact-head VSFTPD regression and AI handoff documentation Actions are triggered by THIS commit and remain pending at commit creation. No local checkout/compiler or local docs checker was available. Production `asus_ext.c` is not compiled by the workflow; L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile both Actions on the exact new SHA. If green, audit FTP STAT/LIST/NLST output compatibility under the configured bounds, then continue MDTM/SITE CHMOD and the upload cleanup race.

---

## 2026-11-10 08:52 CST (UTC+8) — Reject blocking and substituted FTP ACL config files

- **Base main SHA:** `633aa2a4ed39a436ecd820adf1ac1e86ab272c03`; VSFTPD run [42728161936] and AI handoff docs [42728162030] both completed SUCCESS on this exact SHA. The prior non-void listing return contract passed.
- **Finding:** `read_file_to_buffer()` performed `stat(target_file)` but ignored its result and type, then called blocking `fopen()` before validating the size. If the reserved `.__<account>_var.txt` ACL config path is a FIFO, opening it can block the FTP process indefinitely; stat/open pathname races also mean the size check does not describe the opened object.
- **Fix:** add `ftp_acl_path_open_bounded_regular_file()` in the shared path-policy header. It opens with `O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC`, `fstat()`s the returned descriptor, and accepts only nonempty regular files within the supplied bound. Rework `read_file_to_buffer()` to read at most the opened size, retry EINTR, reject short/inconsistent reads, and close/free on every error path.
- **Regression:** extend GNU89 host policy tests for an accepted regular file, nonblocking FIFO rejection, symlink rejection and an exceeded size bound. Add workflow source assertions for the open flags, fd-based type/size checks, bounded read/EINTR handling and all four fixtures.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `asus_ext.c`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** source review PASS. Base exact-head VSFTPD and docs Actions above passed. THIS GNU89 standalone policy regression/source contract and AI-doc validation are pending; workflow does not compile production `asus_ext.c`. L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions. Then audit LIST/NLST/STAT output compatibility and finish MDTM/SITE CHMOD descriptor-relative metadata plus upload cleanup race.

---

## 2026-11-10 08:41 CST (UTC+8) — Propagate recursive listing policy errors with a valid return

- **Base main SHA:** `b8ac678cf872dc36c7ace1d424562c575c0ae365`; VSFTPD regression [42727752390] and AI handoff docs [42727752549] both completed SUCCESS on this exact SHA.
- **Review finding:** the bounded-entry change converted `vsf_ls_populate_dir_list()` from `void` to `int`, but the early error path for `dirfd()`/listing-base resolution still used a bare `return;`. The current workflow does not compile production `ls.c`, so the exact-host Actions could not catch this C return-type defect.
- **Correction:** return `-1` after releasing the local listing strings on directory-fd/base-resolution failure; extend the workflow semantic contract to require a value return and reject bare `return;` in the now-non-void listing function.
- **Validation boundary:** base exact-head Actions passed the standalone GNU89 policy fixtures and semantic source contract, but no production `ls.c` compilation was performed. THIS corrected exact-head Actions are pending. L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- **Changed paths:** `trunk/user/vsftpd/src/ls.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions and then audit LIST/NLST/STAT output/error/sort/filter compatibility at the resource limits; continue MDTM/SITE CHMOD and upload cleanup hardening.

---

## 2026-11-10 08:35 CST (UTC+8) — Bound FTP recursive listing depth and retained entry memory

- **Base main SHA:** `3a998e5de9c0b5b127a92d24f6453bed581fd92d`; VSFTPD regression [42727139083] and AI handoff docs [42727139346] both completed SUCCESS on that exact SHA; remote branch inventory was `main` only.
- **Finding:** `transfer_dir_internal()` recursively retained every ancestor `subdir_list` and also kept each fully formatted `dir_list` alive while descending. Client-controlled `LIST -R` had no explicit depth/total-entry bound. Generic `str_list.c` allows up to ten million nodes before `die()`, far above a useful router FTP listing safety boundary.
- **Fix:** introduce `ftp_listing_policy.h` with 32 recursion levels, 4096 accepted entries per directory and 32768 total accepted entries per transfer. `vsf_ls_populate_dir_list()` now counts accepted (post-filter/visibility/stat) entries and returns failure at either entry limit; `transfer_dir_internal()` propagates the failure, rejects further recursion when the depth bound is reached and child directories remain, and frees each formatted parent `dir_list` before descending. This caps the retained ancestor subdirectory lists and avoids silently returning an incomplete listing.
- **Regression:** the GNU89 host policy test checks just-below/at per-directory and total entry limits, just-below/at recursion depth, and no false depth failure for a leaf. Workflow semantic checks bind the constants and policy functions to the actual `ls.c` accounting and `ftpdataio.c` recursion/freeing paths.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_listing_policy.h` (new), `ftpdataio.c`, `ls.c`, `ls.h`, `tests/ftp_acl_path_policy.c`, `ftp_acl_path_policy.h` (comment placement correction), `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/control-flow review PASS. Prior listing-base visibility VSFTPD run [42727139083] and AI docs [42727139346] passed exact base SHA `3a998e5de9c0b5b127a92d24f6453bed581fd92d`. THIS GNU89 host policy regression/source contract and AI-doc Actions are pending on the exact resulting SHA. No local checkout/compiler was available; L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions; then review FTP STAT/LIST/NLST output/error/sort/filter compatibility under the new caps and continue MDTM/SITE CHMOD and upload cleanup hardening. Keep DHCP/FULLCONENAT/NFS and target/device validation open.

---

## 2026-11-10 08:26 CST (UTC+8) — Bind FTP listing visibility to the actual listing base

- **Base main SHA:** `9ec4f037172c6aaa4af7dd32860705f152e9c9a8`; only remote branch is `main`, no checkpoint branch.
- **Recovered exact-head evidence:** VSFTPD stability regression [42726427585] and AI handoff docs [42726427801] both completed SUCCESS on this exact SHA. GNU89 host policy suites and the semantic source contract passed, including descriptor-relative LIST metadata.
- **Finding:** `vsf_ls_populate_dir_list()` passed only each entry leaf to `asus_check_file_visible()`. That helper rebuilt the full path from process CWD. Thus `LIST /mnt` while CWD was elsewhere, and recursive listing of a child directory, could classify the name against the wrong path; hidden-share/system-folder and share ACL boundary filtering could be inconsistent with the directory whose entries were actually read.
- **Fix:** add `ftp_acl_path_build_child_path()` to join a validated leaf to an already-canonical base. `asus_resolve_listing_base()` canonicalizes the explicit `p_base_dir_str` once per opened directory and verifies it matches the `DIR*` fd. Listing passes that canonical base and fd into `asus_check_file_visible()`; the helper no longer calls `getcwd()`, retains hidden-share/system-folder/share-right checks, and uses `fstatat(dirfd, leaf, 0)` for the mount/share directory-entry type checks. This also avoids repeating `realpath()` for every entry.
- **Regression:** host policy fixture switches CWD to share B while building a child path under share A, verifies the resulting path still refers to A's entry, and rejects slash/traversal leaves. Semantic source contract asserts one base resolution per listing, fd identity verification, no CWD dependency in the visibility helper, and the resolved-base/dirfd call chain.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `asus_ext.c`, `asus_ext.h`, `ls.c`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/control-flow review PASS. Base per-entry metadata VSFTPD run [42726427585] and AI docs [42726427801] passed exact SHA `9ec4f037172c6aaa4af7dd32860705f152e9c9a8`. THIS change's GNU89 host regression/source contract and AI-doc Actions are pending on its resulting exact SHA. No local checkout/compiler was available; L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions; then audit recursive LIST resource/descriptor/memory bounds and option/filter compatibility. Keep MDTM/SITE CHMOD, upload cleanup, DHCP/FULLCONENAT/NFS and target/device validation open.

---

## 2026-11-10 08:19 CST (UTC+8) — Resolve LIST entry metadata relative to the enumerated directory

- **Base main SHA:** `4cf4f33894658e8f23495f6fff65066143f767dd`.
- **Recovered exact-head evidence:** recursive listing VSFTPD regression [42725894055] and AI handoff docs [42725893729] both completed SUCCESS on exact SHA `4cf4f33894658e8f23495f6fff65066143f767dd`. The GNU89 workflow compiled and executed the FTP ACL policy suite and passed the semantic source contract after the fixture/assertion correction.
- **Finding:** `vsf_ls_populate_dir_list()` enumerated names from an already-open `DIR*`, but performed subsequent per-entry metadata operations with `str_lstat(base/name)` and `str_readlink(base/name)`. If the base pathname were renamed or replaced after opening, LIST could report type/metadata/link targets from a different directory than the one whose entries were enumerated.
- **Fix:** add `vsf_sysutil_lstatat()` (uses `fstatat(..., AT_SYMLINK_NOFOLLOW)`), `vsf_sysutil_readlinkat()`, and matching `str_lstatat()` / `str_readlinkat()` wrappers. `ls.c` obtains the listing directory fd once and uses the entry leaf with those wrappers for both type metadata and symlink-target formatting. NLST's display path remains unchanged; only metadata lookup becomes descriptor-relative.
- **Regression:** extend the host policy fixture to verify that descriptor-relative `fstatat()` retains symlink-entry identity and `readlinkat()` returns the link target. Extend exact-source semantic checks and push/PR path filters for `ls.c`, `sysstr.c` and `sysstr.h`.
- **Changed paths:** `trunk/user/vsftpd/src/sysutil.c`, `sysutil.h`, `sysstr.c`, `sysstr.h`, `ls.c`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/control-flow review PASS. Prior recursive-listing VSFTPD host/source-contract run [42725894055] and AI docs [42725893729] passed exact base SHA `4cf4f33894658e8f23495f6fff65066143f767dd`. THIS per-entry metadata GNU89 host regression/source contract and AI-doc Actions are pending on the exact resulting SHA. No local checkout/compiler was available; L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions, then fix listing-base visibility resolution and define host-testable recursion resource bounds. Keep MDTM/SITE CHMOD, upload cleanup, DHCP/FULLCONENAT/NFS and target/device validation open.

---

## 2026-11-10 08:11 CST (UTC+8) — Repair recursive-listing regression fixture and workflow contract

- **Base main SHA:** `2db271ad5e46fc3128f1599d8102dab7fabb582e`.
- **Exact-head evidence:** VSFTPD run [42725673947] targeted this SHA and failed in the first GNU89 compile/run step. Compiler output identified `-Werror=comment` at the new fixture block: the insertion retained the first two lines of an existing comment, then re-emitted its opener. Consequently none of the host binaries ran and the Python semantic contract was skipped. AI handoff docs [42725673900] passed on the same exact SHA.
- **Correction:** remove the duplicated comment opener from `ftp_acl_path_policy.c`. Static review also caught a second stale workflow assertion comparing the LIST authorization point against the old `asus_check_permission_directory()`; update it to the new descriptor-bound `asus_check_permission_open_directory()`.
- **Changed paths:** `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** source diff/L1 review PASS. The previous VSFTPD host regression is FAIL before executing tests; AI docs passed. THIS corrected exact-head host regression, semantic source contract and AI-doc Actions are pending. No claim of host-test success is made for the previous SHA. L3 MIPS/uClibc and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** inspect exact-head Actions for THIS correction. On success, resume the remaining `ls.c` per-entry metadata and recursion resource audit; leave MDTM/SITE CHMOD, upload cleanup, DHCP/FULLCONENAT/NFS and target/device validation open.

---

## 2026-11-10 08:05 CST (UTC+8) — Bind recursive FTP listing to ACL-checked directory descriptors

- **Base main SHA:** `5ca27e7c420284f1ac21362e28566cfb56a693f1`; remote branches: `main` only, no checkpoint branch.
- **Recovered exact-head evidence:** RNFR/RNTO VSFTPD regression [42724420056] and AI handoff docs [42724420034] both completed SUCCESS on this exact SHA. The two-parent `renameat()` implementation and corrected source-order contract are green.
- **Finding:** `handle_dir_common()` opened the requested directory with path-based `opendir()` and then checked READ ACL against a separately resolved pathname without proving both names identified the same directory. More critically, `transfer_dir_internal()` recursively reopened every discovered subdirectory with path-based `opendir(base/leaf)` and performed no READ ACL check for that child. A writable directory entry replaced between enumeration and recursion could redirect the recursive listing through a symlink into another share; the opened object was not bound to the parent directory originally enumerated.
- **Fix:** top-level LIST/NLST/STAT now verifies the opened `DIR*` descriptor identity against its canonical ACL path before and after the READ check. Add `ftp_acl_path_open_child_directory()` using `openat(parent_fd, leaf, O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC)`, then compare the opened child with `fstatat(parent_fd, leaf, AT_SYMLINK_NOFOLLOW)`. `asus_open_subdirectory_path()` canonicalizes/verifies the parent path, checks the child READ ACL, then revalidates both parent and child identities. Recursive transfer uses `fdopendir()` on this authorized descriptor rather than `str_opendir(path)`.
- **Regression:** GNU89 path-policy tests now verify direct-child directory open/inode identity and reject an ordinary directory symlink as a recursive child. The semantic workflow checks top-level directory-fd ACL binding, no-follow child open, post-open child identity, per-child ACL, fdopendir wiring, and both push/PR path filters for the added production files.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `asus_ext.c`, `asus_ext.h`, `postlogin.c`, `ftpdataio.c`, `sysutil.c`, `sysutil.h`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/control-flow review PASS. Base RNFR/RNTO workflows above passed exact SHA `5ca27e7c420284f1ac21362e28566cfb56a693f1`. THIS commit's GNU89 host regression/source contract and AI-doc Actions are pending on its exact resulting SHA; no local checkout/compiler/docs checker was available. L3 MIPS/uClibc target build and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head VSFTPD and AI-doc Actions; then continue the residual `ls.c` per-entry `lstat()`/`readlink()`, listing-base/session-CWD visibility and recursion-resource audit. Keep MDTM/SITE CHMOD, upload cleanup, DHCP/FULLCONENAT/NFS and L3/L4 open.

---

## 2026-11-10 07:51 CST (UTC+8) — Correct RNFR/RNTO state-clear source contract

- **Base main SHA:** `2953898fb09e4d5535fd0628bb3c74bb8b21a831`; remote branches: `main` only.
- **Recovered exact-head results:** RNFR/RNTO VSFTPD run [42724086864] and AI handoff docs [42724086704] both targeted this exact SHA. The VSFTPD GNU89 compile/run step completed successfully, including the descriptor-relative rename policy tests; its following Python source-contract step failed at the assertion requiring `asus_rename_path()` to precede the first `str_free(&rnfr_filename_str)`.
- **Failure analysis:** `handle_rnto()` deliberately clears the pending RNFR source in the earlier destination `vsf_access_check_file()` denial branch, before replying and returning; after an allowed rename attempt it clears the same state again after `asus_rename_path()`. The code ordering is correct for both outcomes; the workflow incorrectly treated the first cleanup as the only cleanup.
- **Correction:** change the workflow contract to inspect the denial block and assert clear-before-rejection, then use `rindex()` to assert the post-rename cleanup follows the actual `asus_rename_path()` call. Production C and host policy test sources are unchanged.
- **Changed paths:** `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 workflow/source review PASS. Base exact-head GNU89 host compile/run PASS; the complete base VSFTPD workflow was FAIL only at the stale semantic source assertion. AI docs passed exact SHA `2953898fb09e4d5535fd0628bb3c74bb8b21a831`. Corrected VSFTPD and docs Actions are pending for THIS resulting commit. No local checkout/compiler/docs checker was available; L3 MIPS/uClibc and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile the corrected VSFTPD and AI docs runs on the exact resulting SHA. If both pass, begin the LIST/NLST/STAT directory traversal/recursion audit; retain MDTM/SITE CHMOD, upload cleanup race, DHCP/FULLCONENAT/NFS and target/device validation as open work.

---

## 2026-11-10 07:29 CST (UTC+8) — Anchor FTP RNFR/RNTO to two ACL-authorized directory descriptors

- **Base main SHA:** `9b6c65686ad95295fd456e074292638a6c9ccf92`; remote branches: `main` only, no checkpoint branch.
- **Recovered exact-head evidence:** MKD VSFTPD regression [42722114919] and AI handoff docs [42722114837] both completed SUCCESS on exact SHA `9b6c65686ad95295fd456e074292638a6c9ccf92`. GNU89 host suites and the semantic source contract passed.
- **Finding:** RNFR pins an absolute source path, but RNTO previously checked source DELETE/destination WRITE ACLs through pathnames and then used `str_rename()`. Either parent pathname could be replaced between authorization and the rename syscall.
- **Fix:** `ftp_acl_path_resolve_rename_entry()` preserves the final leaf for ordinary operands and follows directory-path semantics for trailing-slash operands. `asus_rename_path()` pins both canonical parent dirfds, applies source DELETE and destination WRITE ACL checks, revalidates both parent device/inode identities, then performs `renameat()`. Ordinary final symlinks are renamed as entries; overwrite semantics remain those of `rename(2)`. RNTO consumes pending RNFR state on any attempted result.
- **Regression:** GNU89 policy tests cover final-link behavior, trailing-slash directory symlink resolution, metadata/dot leaf rejection, cross-directory overwrite and use of both pinned parent descriptors after both corresponding pathnames are substituted with symlinks. The workflow source contract checks the ordered ACL calls, both identity rechecks, renameat use and RNFR-state clearing.
- **Changed paths:** FTP policy/header/ACL/postlogin/test/workflow plus `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/control-flow review PASS. Exact-head VSFTPD GNU89 regression/source contract and AI-doc Actions are pending for THIS resulting SHA; no local checkout/compiler was available. L3 MIPS/uClibc target build and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions. If green, audit LIST/NLST/STAT traversal/recursion; keep MDTM/SITE CHMOD, non-atomic upload cleanup, and DHCP/FULLCONENAT/NFS queue open.

---
## 2026-11-10 07:10 CST (UTC+8) — Anchor FTP MKD creation to the authorized parent descriptor

- **Base main SHA:** `05c7a13f44e64b4ba81f4af34b1000bb8eb4792e`.
- **Recovered exact-head CI:** DELE/RMD VSFTPD regression [42721783706] and AI handoff docs [42721783725] both completed SUCCESS on this exact SHA. GNU89 path-policy tests confirmed DELE unlinks a symlink entry without deleting its target, RMD refuses directory symlinks, the empty-directory operation works, metadata leaves are denied, and parent identity replacement is detected.
- **Finding:** MKD evaluated WRITE permission against a canonical parent path and then invoked path-based `str_mkdir()`, leaving the same parent-component TOCTOU window as DELE/RMD.
- **Fix:** add `asus_make_directory_path()`, which resolves and verifies the canonical parent descriptor, evaluates the existing WRITE ACL against that parent while holding the dirfd, rechecks device/inode identity, and invokes `mkdirat()` on the final leaf. The existing mode `0777`, process umask, log behavior and FTP error classes are preserved.
- **Regression:** add GNU89 helper coverage for descriptor-relative directory creation, mode-independent directory type verification, and reserved metadata rejection. Extend the semantic workflow to require MKD's descriptor wrapper and forbid `str_mkdir()`.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `asus_ext.c`, `asus_ext.h`, `postlogin.c`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source review PASS. Exact-head GNU89 test/source-contract and AI-doc Actions are pending for THIS commit. No local checkout/compiler or docs checker is available. L3 target compilation and L4 physical-router testing remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions; then map and test RNFR/RNTO state and two-parent semantics before replacing `str_rename()` with `renameat()`. Preserve LIST/NLST/STAT, MDTM/SITE CHMOD and upload-cleanup race as open work.

---

## 2026-11-10 07:00 CST (UTC+8) — Anchor FTP DELE and RMD to pinned parent descriptors

- **Base main SHA:** `bfd883ad75d5579c54922432a597efcade5a7509`; branch inventory at start of slice: `main` only.
- **Recovered exact-head CI:** RETR/SIZE VSFTPD regression [42721150755] and AI handoff docs [42721150532] both completed SUCCESS on the exact base SHA. The descriptor-based SIZE source-order and reserved-metadata contracts are green.
- **Finding:** `handle_dele()` and `handle_rmd()` checked DELETE permission against a canonicalized parent path, then called `str_unlink(path)` / `str_rmdir(path)`. A parent component swapped after ACL approval could redirect the mutation.
- **Fix:** add `asus_remove_path()`: split the final leaf without resolving it, resolve and pin the canonical parent directory, perform the existing per-share DELETE ACL check while holding the dirfd, compare the canonical path's current dev/inode against the pinned descriptor, then call `unlinkat()` relative to that descriptor.
- **Compatibility:** DELE uses `unlinkat(..., 0)`, which removes a final symlink entry itself; RMD uses `unlinkat(..., AT_REMOVEDIR)` and refuses final symlinks. Metadata leaves are rejected. DELE trailing slashes fail with `ENOTDIR`, preserving the legacy path syscall behavior.
- **Regression:** GNU89 path-policy coverage adds symlink-target preservation for DELE, RMD refusal of a directory symlink, empty-directory removal, reserved-metadata rejection and deterministic parent-path identity mismatch after symlink substitution. Workflow semantic checks require both handlers to use the wrapper and prohibit the prior pathname syscalls.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `asus_ext.c`, `asus_ext.h`, `postlogin.c`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source and control-flow review PASS. Exact-head GNU89 path-policy regression, VSFTPD source-contract and AI-doc Actions are PENDING for THIS COMMIT. No local checkout/compiler or local docs checker was available. L3 target build and L4 device testing remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions; if green, convert MKD to `mkdirat()`, then make RNFR/RNTO descriptor-relative with explicit source-delete and destination-write ACL checks. LIST/NLST/STAT recursion and MDTM/SITE CHMOD remain open.

---

## 2026-11-10 06:59 CST (UTC+8) — Refresh the FTP semantic contract for descriptor-based SIZE

- **Base main SHA:** `4c0d2d8acd9f16c175037a64e4f05494ec7e565f`.
- **CI review:** VSFTPD run [42720948938] completed GNU89 host compilation and all three test executions successfully, including the updated ACL path regression. The workflow then failed in its Python source contract: the SIZE ordering check still searched for the removed `asus_check_permission_existing_path()` and `str_stat()` sequence. The new implementation correctly uses `asus_open_existing_path()` followed by `vsf_sysutil_fstat()`. AI handoff validation [42720949030] completed SUCCESS on the exact base SHA.
- **Correction:** update the SIZE ordering contract to compare descriptor open before `fstat`, and update the internal-metadata canonical-target count from three to four because the new existing-entry resolver performs the additional fail-closed canonical-target check.
- **Changed paths:** `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** host compile/test execution PASS on the preceding exact SHA; semantic source contract failed on stale assertions, which are corrected in THIS COMMIT. Exact-head Actions are pending. L3 target build and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile the exact-head regression and docs runs. If both pass, continue with descriptor-relative DELE/RMD/MKD/RNTO while keeping LIST/NLST, MDTM/SITE CHMOD and upload-cleanup race items open.

---

## 2026-11-10 06:58 CST (UTC+8) — Correct the FTP final-symlink regression expectation

- **Base main SHA:** `6f15ad9a2580f9d0bba16447d61d315d5e8eb4e3`.
- **CI review:** VSFTPD regression [42720752045] compiled all GNU89 host tests and passed the config-file and daemon-policy executions. The ACL-path test then failed because it required `ELOOP` from the descriptor helper. The helper first executes `fstatat(AT_SYMLINK_NOFOLLOW)`, identifies that the substituted entry is a symlink rather than a regular file, and correctly rejects it with `EINVAL` before `openat()`; the separate direct `openat(O_NOFOLLOW)` assertion still specifically expects `ELOOP`. AI handoff validation [42720752091] completed SUCCESS on the exact base SHA.
- **Correction:** expect `EINVAL` from the helper's earlier no-follow metadata gate and clarify the test failure message. Update the workflow source contract to match the corrected test.
- **Changed paths:** `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 review confirms fail-closed helper behavior; host regression and semantic source-contract Actions are pending on THIS resulting SHA. No successful ACL-path host run is claimed yet. L3 target build and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head VSFTPD and AI-doc Actions. If green, proceed to descriptor-relative DELE/RMD/MKD/RNTO while retaining LIST/NLST, MDTM/SITE CHMOD, the upload cleanup race, and the cross-module priority queue.

---

## 2026-11-10 06:57 CST (UTC+8) — Anchor FTP RETR and SIZE to the ACL-authorized file descriptor

- **Base main SHA:** `f03f31fb33875fcf104f444cd6c87b77258126eb`; active branch: `main`; remote branches: `main` only.
- **Recovered exact-head CI:** VSFTPD regression [42714563434] and AI handoff docs [42714560664] both completed SUCCESS on exact base SHA `f03f31fb33875fcf104f444cd6c87b77258126eb`; the previous upload dirfd anchoring task and multiline source-contract correction are complete.
- **Finding:** `handle_retr()` checked share ACLs through `realpath()` and then reopened the original pathname with `str_open()`; `handle_size()` similarly checked the effective target and later called `str_stat()`. A final or parent symlink could be replaced after authorization, so the final file operation could address a different entry.
- **Fix:** add `ftp_acl_path_resolve_existing_entry()` to canonicalize an existing target (including final symlinks) into parent/leaf; pin and verify the canonical parent directory; recheck its identity after the share ACL decision; open only a regular leaf using `openat(O_NOFOLLOW|O_CLOEXEC)`; and compare the pre-open `fstatat(AT_SYMLINK_NOFOLLOW)` identity with the opened descriptor's `fstat()`. RETR transfers from that descriptor and SIZE obtains metadata from an `O_PATH` descriptor, retaining stat-only access without requiring file read permission. Dangling/internal-metadata targets and substituted symlink leaves fail closed.
- **Regression:** extend the GNU89 path-policy test for canonical final-symlink targets, descriptor-relative existing-file open, `O_PATH` stat-only semantics, verified parent pinning, substituted final-symlink rejection and dangling/metadata-link denial. Extend the VSFTPD semantic workflow to require descriptor-based RETR/SIZE and prohibit their old path-based open/stat operations.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `asus_ext.c`, `asus_ext.h`, `postlogin.c`, `tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/control-flow review PASS; host regression and semantic source-contract Actions are **PENDING** on THIS resulting SHA. No local checkout/compiler was available for a separate host run. L3 MIPS/uClibc object/firmware and L4 hardware validation remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** inspect exact-head VSFTPD and AI-doc runs; if green, implement descriptor-relative DELE/RMD/MKD/RNTO while preserving existing ACL semantics. Keep LIST/NLST recursion, path-based MDTM/SITE CHMOD, and the non-atomic upload cleanup race open.

---

## 2026-11-10 06:46 CST (UTC+8) — Make FTP upload source contract robust to multiline calls

- **Base main SHA:** `9d538991d0644d3393aed62cef09fce89ca86774`; active branch: `main`.
- **CI review:** VSFTPD regression [42714075313] compiled and executed the GNU89 host policy test successfully (step 3), then failed in the source-contract step at Python line 76. The contract searched for a one-line `asus_open_upload_path(p_sess, p_filename` string, while the production call intentionally breaks after the opening parenthesis. This is a brittle source assertion, not a host C regression failure. AI handoff documentation [42714075402] completed SUCCESS on the exact base SHA.
- **Correction:** make the workflow assert the helper call and both exclusive/non-exclusive arguments independently across lines. Also repair missing line breaks in the current handoff/roadmap checkpoint paragraphs discovered during this reconciliation, and clarify exact prior CI evidence.
- **Changed paths:** `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation:** L1 source/contract review PASS. The prior host regression step on [42714075313] PASS; its overall workflow FAILED in the brittle source-contract assertion. Actions for THIS commit are PENDING exact SHA. No target MIPS/uClibc build or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile the exact-head VSFTPD and docs Actions. If the regression is green, continue the active FTP TOCTOU audit with RETR/SIZE and file mutation/listing paths; preserve the known non-atomic cleanup limitation.

---

## 2026-11-10 06:39 CST (UTC+8) — Anchor FTP uploads to an ACL-authorized directory descriptor

- **Base main SHA:** `6b4b62e61ee13c7882579648c395fee8acc6ebbf`; active branch: `main`; no remote checkpoint branch.
- **Recovered state:** the preceding main SHA had Samba share configuration stability and AI handoff documentation Actions [42709723952] / [42709723572] SUCCESS. The FTP realpath-to-filesystem-syscall audit remained the unfinished handoff task.
- **Finding:** STOR/APPE/STOU performed canonical-parent ACL validation, then reopened the original pathname through `str_create()` / `str_create_exclusive()`. A final or parent symlink could be substituted between the ACL decision and the actual write.
- **Fix:** first code commit `ac75a8ff43fca3672b44605a2974a0497ecd1c0d` added a split canonical parent/leaf resolver, component-by-component `O_PATH|O_DIRECTORY|O_NOFOLLOW` parent walk, `openat(O_NOFOLLOW|O_CLOEXEC)` upload open, and descriptor-relative failed-upload cleanup guarded by device/inode identity. Existing final symlink writes still resolve to their target; dangling links and internal metadata remain rejected. Cleanup through a final symlink alias is intentionally disabled.
- **Race refinement in THIS COMMIT:** compare `stat(resolved_parent)` with `fstat(parent_fd)` after the no-follow component walk; reject a changed/non-directory identity with `ESTALE`. Correct the workflow's previous hard-coded expectation of two canonical metadata-target checks to three after the new resolver added the third.
- **Cleanup limit:** `fstatat(AT_SYMLINK_NOFOLLOW)` followed by `unlinkat()` catches a replacement already present at check time, but Linux 4.4 has no unlink-by-fd primitive; the final name-check/unlink pair is not atomic against a concurrent writer in the same directory. This remains best-effort and is explicitly open.
- **Regression:** the GNU89 host policy test covers canonical write-leaf resolution, cross-share parent-path symlink replacement while a dirfd is pinned, final-leaf `O_NOFOLLOW`, and failed-upload cleanup when the leaf inode was replaced. The compile/run step 3 of vsftpd Actions [42713615415] completed SUCCESS on exact SHA `ac75a8ff43fca3672b44605a2974a0497ecd1c0d`; the later static-contract step failed on the stale count assertion described above. AI docs [42713615421] completed SUCCESS on that SHA. This commit's exact-head FTP and docs Actions are pending.
- **Changed paths:** `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `trunk/user/vsftpd/src/asus_ext.c`, `trunk/user/vsftpd/src/asus_ext.h`, `trunk/user/vsftpd/src/postlogin.c`, `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- **Validation boundary:** L1 focused source review PASS. L2 regression compile/run step PASS but the full workflow on `ac75a8f...` FAILED in its semantic contract; final exact-head regression is pending. L3 MIPS/uClibc production object and L4 hardware validation NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** continue the FTP TOCTOU audit with RETR/SIZE, then file-entry mutation and recursive LIST/NLST; MDTM and SITE CHMOD remain path-based. Keep IPTV DHCP callback, FULLCONENAT target build and NFS compatibility tasks queued.

---

## 2026-11-10 06:08 CST (UTC+8) — Recover Samba passdb/config publication after same-boot process interruption

- **Base main SHA:** `258532a78922549698bc6d9b869c7777f8f4ac05`; active branch: `main`; remote checkpoint branches: none.
- **Recovered unfinished work:** the previous handoff's DHCP callback source-contract correction completed on exact SHA `6b44e023c2f669865fc8b5cb5425d824b8335d8b`; WAN DHCP callback [42703609156] and AI handoff docs [42703609087] both completed SUCCESS. The bounded BusyBox event review also completed without a justified applet behavior change.
- **Finding:** `run_samba()` had ordinary-error passdb rollback, but a process exit after replacing `/etc/samba/smbpasswd` and before the matching `/etc/smb.conf` rename could leave a mismatched pair with no same-boot recovery state.
- **Fix:** add a PREPARED/COMMITTED paired publication journal and hardlink snapshots under `/etc/samba/.config-transaction`; recover uncommitted pairs idempotently, preserve the committed pair after COMMITTED, and retry cleanup on later writers. Serialize `run_samba()`, public `write_smb_conf()`, and WINS `start_wins()` with `/var/run/samba-config.lock`; all writers recover before config changes. The journal is intentionally volatile under `/etc` and makes no power-loss durability claim.
- **Regression:** add GNU89 host tests for interruption after passdb rename, interruption after both file renames, committed cleanup, first-run missing passdb, partial rollback retry and symlink refusal. Update Samba and HTTPD semantic contracts to inspect the unlocked config generator and new transaction path.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`; `trunk/user/rc/src/services_ex.c`; new `trunk/user/rc/src/samba_config_transaction.h`; new `trunk/user/rc/src/tests/samba_config_transaction.c`; `trunk/user/rc/src/samba_passdb_policy.h`; `.github/workflows/samba-share-stability.yml`; `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** Samba transaction host compile/run and source-order contract [42708213921] completed SUCCESS on exact SHA `e9a93eb84ac9f95447150869fab7e0ae1c2da9ff`. Dependent HTTPD/Samba trust-boundary regression [42708503436] and AI docs validation [42708503300] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`. Initial stale semantic assertions were corrected; their earlier failures are not counted as PASS. No local target build, MIPS/uClibc object, firmware/TRX or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** audit FTP `realpath()`-to-filesystem-syscall TOCTOU end-to-end, preserving symlink/ACL semantics and the embedded libc/kernel constraints. Keep IPTV DHCP callback status loss, FULLCONENAT target build and NFS compatibility questions queued.

---

## 2026-11-10 05:25 CST (UTC+8) — Align DHCP callback regression with status-returning WAN/MAN up helpers

- **Base main SHA:** `38a86273a44bc481666674606979321701703f71`; active branch: `main`.
- **Exact-head Actions inspected:** WAN route [42703179674], AI handoff docs [42703179667], WAN teardown [42703179436], and HTTPD security [42703179416] SUCCESS. WAN DHCP callback [42703179710] failed in its source-contract step after its GNU89 interface transaction compile/run step passed.
- **Failure analysis:** the DHCP contract still required `udhcpc_bound()` to call the public `wan_up()` wrapper. The new route-add change deliberately calls `wan_up_internal()` / `man_up_internal()` instead, so that the callback can consume and return the route setup status while the public void APIs remain compatible. The failure was a stale ordering assertion, not a production source compile error.
- **Correction:** update the callback contract to assert that the stored DHCP gateway is published before the internal up call, that both WAN/MAN private results are captured, and that `udhcpc_bound()` returns `route_setup_status`. The production C source is unchanged in THIS follow-up.
- **Changed paths:** `.github/workflows/wan-dhcp-callback-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** L1 review of the contract change complete. The new DHCP callback workflow and docs check are pending exact resulting SHA; do not treat the prior failed run as green. No production object, MIPS/uClibc build, full firmware or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** inspect all exact-head Actions after this test-only correction; then finish the bounded BusyBox DHCP callback-state review and broaden to Samba/other userland modules.

---

## 2026-11-10 05:11 CST (UTC+8) — Propagate MAN/WAN route-add failures through up paths

- **Base main SHA:** `639ebbdedca75d1938e7d280108d146d05e2e479`; active branch: `main`.
- **Recovered exact-head Actions:** WAN interface teardown [42698031149], AI handoff docs [42698030922], WAN route status [42698030798], HTTPD security [42698030765], and WAN DHCP callback [42698030655] all completed SUCCESS on exact SHA `639ebbdedca75d1938e7d280108d146d05e2e479`.
- **Finding:** `add_man_gateway_routes()` discarded the MAN gateway host/default/DNS `route_add()` results. `wan_up()` discarded its direct host/default route results and ignored the int status already returned by static WAN, static MAN and DHCP route helpers. `man_up()` likewise ignored the static MAN, DHCP MAN and gateway helper statuses. Thus an accepted lease or an "up" callback could silently report successful route setup after one or more route operations failed.
- **Fix:** convert MAN gateway route installation to a private status-returning helper; add private `man_up_internal()` and `wan_up_internal()` aggregators; preserve the existing public void APIs as logging wrappers. Every direct and helper route-add status is accumulated without short-circuiting later route attempts or service startup. `udhcpc_bound()` now calls the private helper, logs an accepted lease with incomplete route setup, and returns the route status.
- **Rollback decision:** do not delete earlier successful additions after a later error. The route ioctl does not identify whether an identical route pre-existed or was installed by this invocation, so blind rollback could remove another path's valid route. Keep the accepted lease/interface state, expose the first error, and leave cleanup to the matching teardown/retry lifecycle.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; `.github/workflows/wan-route-status.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** L1 source/control-flow review and local structural checks of transformed functions PASS. The exact-head route workflow's GNU89 ASan/UBSan policy regression and extended semantic source contract are PENDING for THIS commit; dependent workflow results must also be checked on its exact SHA. No production RC object, MIPS/uClibc, complete firmware/TRX or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions; then finish the bounded BusyBox leasefail/NAK/expiry/renew/SIGUSR2 callback-state review before rotating to Samba and other userland modules.

---

## 2026-11-10 04:59 CST (UTC+8) — Brace MAN DNS route teardown status handling under its predicate

- **Base main SHA:** `ca94b95812ba8eace9162279b268a54184e9f7bc`; active branch: `main`.
- **Prior exact-head validation:** `ca94b95812ba8eace9162279b268a54184e9f7bc` WAN route [42697380249], WAN interface teardown [42697380232], HTTPD [42697380274], WAN DHCP callback [42697380012] and AI docs [42697379978] all completed SUCCESS.
- **Review correction:** source review found that the MAN DNS conditional retained its original single-statement form while the new status aggregation/logging statements were inserted below it without braces. Those statements could execute for non-matching DNS entries, reusing a previous `route_status` despite not performing a route delete.
- **Fix:** add braces around the complete DNS host-route delete, status accumulation and per-operation failure log. Extend the WAN route-status source contract to require this multi-condition block to own all three operations.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; `.github/workflows/wan-route-status.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** L1 control-flow review complete. All five parent exact-head Actions above are green. THIS corrective route/DHCP/teardown/HTTPD/docs Actions are pending exact-head SHA. No production RC object, target build or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** inspect all applicable exact-head Actions; once green, review MAN/WAN route additions and then broaden whole-userland governance coverage.

---
## 2026-11-10 04:55 CST (UTC+8) — Propagate MAN/WAN route teardown failures without short-circuiting cleanup

- **Base main SHA:** `19b3ce5dc958a3a1594cf18baa9aaba96e55b806`; active branch: `main`.
- **Recovered exact-head evidence:** WAN route status [42695351469], HTTPD Authentication/Security [42695351402] and AI handoff docs [42695351591] all completed SUCCESS on this exact SHA. The route-status sanitizer host regression and semantic checks passed; the HTTPD suite completed its 39 steps; docs validation reports 74/119 root records and valid archive inventory. The preceding MAN deconfig implementation passed WAN DHCP callback [42694649038] and WAN interface teardown [42694649527] on exact parent SHA `5718821e4b566fea1856945c322899b5145c995c`.
- **Finding:** `route_del()`, `del_dhcp_routes_by_prefix()` and `del_static_*_routes()` return useful error status, but `man_down()` / `wan_down()` discarded those results before teardown returned. The DHCP callback therefore could report an interface-clear failure but not route cleanup failures.
- **Fix:** add private status-returning `man_down_internal()` and `wan_down_internal()`; accumulate each MAN gateway host/default/DNS, DHCP and static route delete with `route_status_accumulate()`, and aggregate WAN DHCP/default/static route deletion plus static WAN address-clear results. Continue all cleanup after the first error and log failed operations. Preserve public void APIs through wrappers that log aggregate failures. `udhcpc_deconfig()` consumes the private status and returns the first interface-clear or route-cleanup error after all cleanup.
- **Regression coverage:** update WAN route-status source contract for both internal helpers, each status-returning operation, continued cleanup ordering and public wrapper logging. Update DHCP callback contract for combined status and HTTPD extractor for the internal signature.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; `.github/workflows/wan-route-status.yml`; `.github/workflows/wan-dhcp-callback-stability.yml`; `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** L1 source review and transformed-source semantic assertions passed. Exact-head route/HTTPD/docs checks on base SHA passed; WAN DHCP/teardown checks passed on the prior production SHA. THIS changed-head WAN DHCP, WAN route, WAN teardown, HTTPD and AI docs Actions are **PENDING**. No production RC object, MIPS/uClibc, firmware/TRX or router test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions; then audit route additions in MAN/WAN up paths for status handling and partial-install rollback.

---

## 2026-11-10 04:42 CST (UTC+8) — Repair WAN route and HTTPD source contracts after MAN teardown refactor

- **Base main SHA:** `5718821e4b566fea1856945c322899b5145c995c`; active branch: `main`; remote branches: `main` only.
- **Exact-head Action outcomes:** WAN DHCP callback [42694649038], WAN interface teardown [42694649527] and AI handoff docs [42694649648] completed SUCCESS on this SHA. WAN route status [42694649350] and HTTPD security [42694649091] failed at Python semantic checks after their host compilation / earlier source checks had passed.
- **Failure diagnosis:** the MAN route cleanup implementation had been moved from the public `man_down()` wrapper into `man_down_internal()` to retain the old address through host-route deletion and invalidate it before resolver/firewall refresh. The two dependent workflows still extracted/asserted the public wrapper body; their logs showed a stale test-range assumption, not a failure in the MAN deconfig implementation.
- **Correction:** update `wan-route-status.yml` to validate route deletion/cleanup order in `man_down_internal()` and the unchanged public wrapper delegation; update the HTTPD DHCP lease contract to inspect the internal implementation and assert the old gateway host route is removed before `wanx_ipaddr` is invalidated. No production source changes in THIS correction.
- **Changed paths:** `.github/workflows/wan-route-status.yml`; `.github/workflows/httpd-security.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** exact-head WAN DHCP/teardown/docs jobs were green. WAN route sanitizer host compilation/execution passed but its semantic step failed; HTTPD earlier policy and source checks passed but the final DHCP assertion failed. Both exact logs were inspected. THIS corrected workflow runs are **PENDING** on the resulting SHA. No production RC/MIPS/firmware or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** require WAN DHCP, teardown, route status, HTTPD and AI docs workflows to pass together on the exact resulting SHA; then investigate route-delete error propagation.

---

## 2026-11-10 04:30 CST (UTC+8) — Keep MAN DHCP lease identity through gateway route cleanup

- **Base main SHA:** `bf80204bf59e26f74636e9c75ce52c6d5d652bee`; active branch: `main`.
- **Recovered exact-head Actions:** the preceding first-lease rollback-failure regression [42690696672] and AI handoff docs [42690696645] both completed SUCCESS on the exact same full SHA `bf80204bf59e26f74636e9c75ce52c6d5d652bee`. The WAN job compiled/ran the GNU89 interface rollback test and passed the callback ordering/rollback/path-filter source contract. The docs job passed handoff, roadmap, log-window and archive-inventory validation.
- **Finding:** `add_man_gateway_routes()` adds a host route for an off-subnet MAN gateway based on the current `wanx_ipaddr`/`wanx_netmask`. On lease loss, `udhcpc_deconfig()` set `wanx_ipaddr=0.0.0.0` before `man_down()`; the latter then failed its old-lease subnet check and skipped deletion of that host route.
- **Fix:** split the existing teardown into a private `man_down_internal()` with an explicit address-reset flag. The DHCP deconfig path preserves the old address through gateway, DHCP and static MAN route removal, invalidates it afterward, and does so before resolver/firewall refresh. The public `man_down()` entry point retains its prior semantics. Deconfig also logs and returns a failed interface-clear status while continuing best-effort route cleanup.
- **Callback audit:** BusyBox's `udhcp_run_script()` discards `spawn_and_wait()` status. `leasefail` is the initial discovery retry-exhaustion callback; NAK is followed by deconfig for a previously bound state; SIGUSR2/release-on-quit use deconfig directly, not a separate release script. No BusyBox behavior change is made.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; `.github/workflows/wan-dhcp-callback-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** L1 source/caller review confirms the off-subnet gateway host-route is created and deleted from the same lease address/netmask identity, and the revised order retains that identity through teardown. The workflow now asserts MAN host-route deletion before address invalidation and checks interface-clear error propagation. Exact-head WAN callback and docs Actions for THIS commit are **PENDING**. No local full RC compile, MIPS/uClibc object, firmware/TRX or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile the exact-head WAN DHCP callback and AI docs Actions; then propagate and reconcile route-delete statuses through MAN/WAN teardown without skipping remaining cleanup.

---

## 2026-11-10 04:11 CST (UTC+8) — Cover first-lease WAN DHCP rollback failure and repair handoff checkpoint headings

- **Base main SHA:** `b6c1cdea285b39925c1e9254c7fb325739129491`; active branch: `main`.
- **Exact-head evidence on the base:** WAN DHCP callback [42690381038], AI handoff docs [42690381155], WAN route status [42690381079], WAN interface teardown [42690380777] and HTTPD security/DHCP regression [42690380735] all completed SUCCESS on the same exact SHA `b6c1cdea285b39925c1e9254c7fb325739129491`.
- **Test extension:** added a GNU89 fake-interface case where no previous lease is valid, the first address-clear ioctl fails, and the best-effort rollback clear fails too. The test asserts that the original failure remains the returned status, the rollback status is exposed, and both clear attempts occur.
- **Handoff repair:** removed duplicate `Verification boundary` / `Exact first action` sections left by the previous content replacement. The actionable handoff now has one state boundary, one verification matrix and one first-action section. Updated roadmap evidence to distinguish the green base implementation SHA from THIS pending edge-case head.
- **Changed paths:** `trunk/user/rc/src/tests/dhcp_wan_interface_policy.c`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** source-level test path reviewed. Exact-head WAN DHCP compile/run and AI docs workflows are **PENDING** for THIS commit. No local compiler invocation or target build is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile the exact-head edge-case and AI handoff Actions; then continue `udhcpc_deconfig()`, leasefail/NAK/release callback semantics and WAN route result propagation.

---

## 2026-11-10 03:42 CST (UTC+8) — Preserve WAN lease state when DHCP address activation fails

- **Base main SHA:** `bc99b6801c44f55e13dc0214336408f712e0abf8`; active branch: `main`.
- **Recovered verification:** FULLCONENAT lifecycle [42678875089] and AI handoff documentation [42678875391] both completed SUCCESS on the exact base SHA `bc99b6801c44f55e13dc0214336408f712e0abf8`; the previous callback-contract task is complete at L2.
- **NFS provenance audit:** traced `/etc/storage/exports` through the RC writer, NFS lifecycle, tmpfs/MTD storage provisioning, the web UI and backup restore path. `/etc/storage` is root-managed persistent storage; the visible restore route requires authenticated privileged access. The writer accepts a regular file or symlink to a regular file, treats only ENOENT as missing, and fails closed for invalid paths. `nfsd.sh` propagates `exportfs -r` failures. No untrusted writer or demonstrated override vulnerability was found; no NFS code/default change was justified. Async durability, AUTH_SYS/no_root_squash and external mounts remain separate decisions.
- **Finding:** `udhcpc_bound()` removed the previous gateway/DHCP routes and published new IP/netmask/gateway NVRAM state before checking whether `ifconfig()` had actually configured the interface. Those ioctl results were discarded. A failed address operation could therefore leave the router reporting the new lease while its interface was not configured and the old route inventory had already been removed.
- **Fix:** added `dhcp_wan_interface_policy.h` and a GNU89 fake-interface regression. The policy validates candidate input, checks clear/set results, attempts to restore the previous valid address (or clear an interface with no valid previous lease), preserves the original operation error and reports rollback failure. `udhcpc_bound()` now applies the candidate address before removing old gateway/DHCP routes or publishing primary lease NVRAM fields. On interface failure it returns early with old route/lease state retained. The existing LAN-subnet conflict behavior remains, but initial conflict-address clearing is now checked.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; new `trunk/user/rc/src/dhcp_wan_interface_policy.h`; new `trunk/user/rc/src/tests/dhcp_wan_interface_policy.c`; new `.github/workflows/wan-dhcp-callback-stability.yml`; `SESSION_HANDOFF.md`; `REFACTOR_ROADMAP.md`; this root log record.
- **Validation:** L1 source review PASS for the new control-flow ordering and rollback path. The added workflow compiles/runs the GNU89 regression and checks source ordering plus complete push/pull_request path filters. Its exact-head Action is **PENDING** at commit creation. Prior FULLCONENAT and docs Actions above are green on the base SHA only; they do not validate THIS commit. No local host binary, RC object, MIPS/uClibc build, firmware/TRX or hardware test is claimed.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile WAN DHCP callback and AI handoff Actions against the exact resulting main SHA. If green, continue with WAN DHCP deconfig/leasefail/NAK/release callback state and remaining route-delete/add error handling.

---

## 2026-11-10 02:48 CST (UTC+8) — Make FULLCONENAT callback source contract preprocessor-aware

- **Base main SHA:** 32ade08379b4bad18b8137801a57bb109815cf2a; resulting commit: THIS COMMIT.
- **Exact-head Actions:** FULLCONENAT [42678293858] passed notifier ownership/cleanup, GNU89 namespace/zone policy compile/run, mapping struct and lookup checks, check_mapping helpers, corrected IPv4 calls and the IPv6 inbound/outbound lookup assertions. It then failed while extracting ct_event_cb(): CONFIG_NF_CONNTRACK_CHAIN_EVENTS and its #else branch declare two mutually exclusive signatures, each including an opening brace. The generic source scanner started at the first signature and counted both branches as one function.
- **Confirmation:** this conditional is intentional API compatibility: the chain-events callback accepts (notifier_block *, events, void *), while the legacy callback accepts (events, nf_ct_event *). The kernel preprocessor compiles only one branch. Do not collapse or alter these distinct production signatures.
- **Fix:** scope the semantic contract's callback source region from the stable callback marker through the next function declaration, then assert that the selected region saves the conntrack namespace/zone context. AI handoff docs [42678293955] passed on the exact base SHA.
- **Changed paths:** .github/workflows/fullconenat-lifecycle.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md.
- **Validation:** source review confirms the dual signatures are mutually exclusive and preserve the 4.4 notifier APIs; prior exact-head GNU89 policy and all preceding semantic checks PASS. THIS workflow/docs exact-head reruns pending. No production kernel object, MIPS/uClibc build, firmware/TRX or hardware test is claimed.
- **Next:** verify all Actions on THIS resulting SHA; then proceed to NFS custom exports writer provenance/ownership/mode/content audit if green.

---

## 2026-11-10 02:43 CST (UTC+8) — Align FULLCONENAT IPv6 inbound validation contract

- **Base main SHA:** 736fab062b66468a654f73f6c67ed42bd9babb69; resulting commit: THIS COMMIT.
- **Exact-head Actions:** FULLCONENAT [42677876443] passed notifier ownership/cleanup, GNU89 namespace/zone policy compile/run, struct layout, namespace/zone lookup, check_mapping helpers, corrected IPv4 calls and five mapping-function source checks. It then failed at the IPv6 target assertion. The test incorrectly required a direct check_mapping6(mapping, &context) call inside fullconenat_tg6(); in this implementation get_mapping6_by_ext_port(port, ip, &context) validates each candidate via check_mapping6(p_current, context) before returning it. Outbound reuse separately calls check_mapping6(src_mapping, &context). AI handoff docs [42677876166] passed on exact base SHA.
- **Correction:** assert the inbound target calls the context-scoped external-port lookup and that the lookup itself performs check_mapping6() before returning a candidate. Retain the direct outbound-reuse assertion. No production behavior change.
- **Changed paths:** .github/workflows/fullconenat-lifecycle.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md.
- **Validation:** source review confirms the inbound validation resides in get_mapping6_by_ext_port() and the IPv6 target invokes that helper with &context; prior exact-head GNU89 policy binary PASS. Exact-head FULLCONENAT and docs runs for THIS contract correction are pending. No kernel translation-unit build, MIPS/uClibc kernel build, firmware/TRX or device test is claimed.
- **Next:** verify every exact-head Action on THIS resulting SHA. If green, start the NFS custom exports writer/provenance/permissions/content-policy audit; keep DHCP callback state and FTP pathname TOCTOU queued.

---

## 2026-11-10 02:38 CST (UTC+8) — Simplify FULLCONENAT context-based function signatures

- **Base main SHA:** 3f3368948e481d90db14e7e00868062dce7ef02b; resulting commit: THIS COMMIT.
- **Exact-head Actions:** FULLCONENAT [42677259891] passed notifier ownership/cleanup and the GNU89 namespace/zone policy binary. Its semantic source contract passed the corrected struct regex and the new IPv4 helper call-site assertions, then failed at function_body() for get_mapping6_by_ext_port(): both #if/#else branches contained the same signature line with an opening brace, and the simple brace scanner counted both alternatives. AI handoff docs [42677259833] completed SUCCESS on the exact base SHA.
- **Finding:** after the namespace-context refactor, check_mapping6(), get_mapping6_by_ext_port(), find_appropriate_port6(), check_mapping() and find_appropriate_port() retained version-conditional declaration wrappers, but both branches now have byte-identical signatures. Their duplicate opening braces confuse the workflow's source-function extractor; the actual preprocessor selects only one branch.
- **Fix:** collapse those five redundant #if/#else declaration wrappers to one function signature each. Function parameters, bodies, version-specific logic within the bodies and runtime behavior are unchanged. Existing semantic function-body assertions now have a single declaration boundary.
- **Changed paths:** trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md.
- **Validation:** local structural scan of all five affected source functions now finds balanced bodies after removing the duplicate conditional opening braces. Prior exact-head GNU89 namespace/zone policy binary PASS; new FULLCONENAT and docs Actions pending. No full kernel object, MIPS/uClibc build, firmware/TRX or device test is claimed.
- **Next:** verify all Actions on THIS exact resulting SHA; if green, begin NFS custom exports writer/provenance/ownership/mode/content-policy tracing. Keep DHCP callback state and FTP pathname TOCTOU queued.

---

## 2026-11-10 02:31 CST (UTC+8) — Fix FULLCONENAT namespace call sites and regression contracts

- **Base main SHA:** e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d; resulting commit: THIS COMMIT.
- **Recovered exact-head Actions:** FULLCONENAT [42674430844] ran the existing notifier lifecycle contract and GNU89 namespace/zone identity regression successfully, then failed the semantic contract at struct_body(): the workflow used over-escaped regex tokens and could not match the real struct nat_mapping. AI handoff docs [42674431213] also reported failure on the exact base SHA, but the Actions API exposed no jobs/check-run detail; its cause is unclassified.
- **Production source review:** the namespace-context refactor changed check_mapping() to accept const struct fullconenat_namespace_context *, but three IPv4 target call sites retained the old API: two passed context by value and one passed obsolete (net, zone) arguments. Correct all three to pass &context; IPv6 call sites already use the pointer form.
- **Workflow correction:** use normal Python regex escapes for mapping struct declarations; resolve the purge helper by its stable function-name/signature substring rather than a literal escaped newline; assert the IPv4 target uses only the new pointer API and that the IPv6 target remains aligned.
- **Changed paths:** trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c, .github/workflows/fullconenat-lifecycle.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md.
- **Validation:** L1 inspection confirmed the saved zone matches the Linux 4.4 nf_conntrack_find_get(net, zone, tuple) prototype and nf_ct_zone_init(zone,id,dir,flags) helper. The base exact-head workflow's GNU89 policy binary compiled and ran PASS; the semantic step then failed as above. THIS correction's exact-head FULLCONENAT and docs runs are pending. No full xt_FULLCONENAT.c kernel object, MIPS/uClibc kernel build, firmware/TRX or hardware validation is claimed.
- **Next:** inspect each exact-head Actions run on THIS resulting SHA, including failed job logs; then begin NFS custom exports writer provenance/ownership/mode/content trust-boundary audit if green. Keep DHCP callback state and FTP pathname TOCTOU on the wider queue.

---

## 2026-11-10 01:46 CST (UTC+8) — Correct vsftpd semantic workflow assertion ordering

- **Base main SHA:** `30d64c73546db236c12679d24eda74c1ea9f2916`; resulting commit: THIS COMMIT.
- **Previous exact-head results:** vsftpd [42667318607] ran GNU89 compilation and all host regression binaries successfully, then failed the semantic Python contract with `NameError: acl_path is not defined`; AI handoff docs [42667318504] passed. Both run `head_sha` values exactly matched the base.
- **Root cause:** new assertions for permission helper bodies were inserted before the workflow's later `function_body()` assignments. This was a workflow test defect, not a C compile/test failure.
- **Fix:** move extraction of `acl_path`, `acl_existing`, `acl_upload`, `acl_directory` and `acl_visible` before their assertions, and remove the later duplicate `acl_path` assignment. No production C or host-test source changed in this correction.
- **Validation:** correction is awaiting exact-head Actions; previous host binaries already passed. Do not count semantic contract validation as green until rerun succeeds.
- **Next whole-repo audit:** begin with NFS custom exports provenance/permissions/content policy and generated `async,secure,no_root_squash,no_subtree_check` exports. Initial review found custom override content is linked through without local ownership/mode/content checks; trace the trusted writer/UI and `/etc/storage` setup before classifying it as a vulnerability. NFS daemon startup/stop state gates and generated export ACL validation are already present.
- Keep DHCP callback leasefail/accepted-NAK/release state, FULLCONENAT tuple namespace, and FTP path TOCTOU in the broader queue.

---

## 2026-11-10 01:41 CST (UTC+8) — Close remaining FTP metadata command ACL and sidecar disclosure gaps

- **Base main SHA:** `071739a00401b1b229bb79613d30bdd4c452faa4`; resulting commit: THIS COMMIT.
- **Parent exact-head Actions:** vsftpd [42665789660] SUCCESS and AI handoff docs [42665789545] SUCCESS; both full `head_sha` values matched the base. The prior batch's GNU89 path regression binaries and semantic source contracts passed.
- **Finding:** SIZE/MDTM/SITE CHMOD still bypassed per-share path ACL checks. The share subsystem stores `.__folder_list.txt` and `.__<account>_var.txt` under each USB mount root; that is MOUNT_LAYER, where the FTP read ACL did not apply. Direct file commands could disclose those ACL/account metadata files.
- **Fix:** SIZE uses target PERM_READ; MDTM checks PERM_WRITE for an enabled timestamp mutation and PERM_READ for read-only behavior; SITE CHMOD uses target PERM_WRITE. The shared path policy recognizes only the exact folder-list and per-account sidecar filename shapes, denies them before the chroot-ACL bypass, and hides them from directory results at all layers. Existing-file and upload resolvers also check the canonical target, preventing an ordinary symlink alias from exposing or overwriting a reserved sidecar.
- **Regression coverage:** add GNU89 positive/negative metadata classifier cases, direct and symlink-alias denial cases, and semantic source contracts for SIZE, MDTM, SITE CHMOD and metadata filtering.
- **Validation:** L1 source review PASS. New exact-head host/semantic Actions pending. No production vsftpd object, MIPS/uClibc build, firmware/TRX or device test claimed.
- **Residual:** pathname canonicalization and later filesystem syscalls remain vulnerable to concurrent symlink replacement; dirfd/openat-style binding is still open. FTP auth/anonymous compatibility and target validation also remain.
- **Next:** verify exact-head Actions, then shift next development batch to the highest-impact non-FTP ledger item (NFS override/ACL/durability, DHCP callback state, or FULLCONENAT tuple namespace).

---

## 2026-11-10 01:30 CST (UTC+8) — Enforce target-path share ACLs for FTP file and directory commands

- **Base main SHA:** `d784677ab88890bf1551c0a03fec85219285933f`; resulting commit: THIS COMMIT.
- **Parent exact-head Actions:** vsftpd [42662555873] SUCCESS and AI handoff docs [42662555510] SUCCESS; both full `head_sha` values matched the base SHA.
- **Finding:** STOR/APPE/STOU, RETR, MKD/RMD/DELE and LIST/NLST/STAT were checking Padavan share rights against the session CWD rather than the path argument's effective target. This allowed a request naming another share to be evaluated against the wrong ACL context.
- **Fix:** apply operation-specific path ACL checks. Uploads resolve existing final symlinks to their actual target and reject dangling/unresolvable final entries; new leaves use their canonical parent. RETR resolves its existing target. MKD/RMD/DELE check canonical entry parents, while LIST/NLST/STAT check the actual opened directory before data-channel setup.
- **Regression coverage:** extend the GNU89 host path-policy test for cross-share final symlinks, dangling upload symlinks, new upload leaves, symlinked parents and directory symlinks. Add source contracts for each command family and ACL-before-transfer ordering.
- **Validation:** L1 source review PASS. New host regression and semantic Actions are pending exact-head CI. No production vsftpd object, MIPS/uClibc build, firmware/TRX or physical-router test is claimed.
- **Remaining FTP audit:** SIZE, MDTM, SITE CHMOD and direct access to per-account `.__*` ACL metadata/folder inventory are not covered by THIS patch; inspect deployed mount/file modes and the remaining pathname TOCTOU risk.
- **Next:** inspect all Actions triggered by THIS commit at the exact new full SHA, then continue the remaining FTP metadata/command audit. Keep NFS custom override/durability, DHCP callback semantics and FULLCONENAT mapping-hash/dying-tuple namespace ownership on the wider governance queue.

---

## 2026-11-10 01:09 CST (UTC+8) — Relax FTP ACL diagnostic source-contract assertion

- **Base main SHA:** `9133aba76e4de64b8e1fa19e743ce3d95716aad6`; resulting commit: THIS COMMIT.
- **Exact-head parent Actions:** AI handoff docs [42661398097] SUCCESS. vsftpd [42661398024] compiled and ran all three GNU89 host regression binaries successfully, including the ACL path-parent test, but failed in the following Python source-contract step.
- **Root cause:** the contract required adjacent double quotes around a diagnostic substring; the C diagnostic contains an escaped newline before the closing quote, so the quoted exact substring did not exist in source text.
- **Correction:** assert the diagnostic phrase without coupling the check to its surrounding C quotes or escaped newline. No production code or runtime regression behavior changes.
- **Next:** verify exact-head vsftpd and AI handoff Actions for THIS source-contract correction, then continue the remaining FTP path-command ACL audit in `SESSION_HANDOFF.md`.

---

## 2026-11-10 01:05 CST (UTC+8) — Avoid libc header shadowing in FTP path regression build

- **Base main SHA:** `a36c2aa14804e9a8eb45f367672d52cf1f306912`; resulting commit: THIS COMMIT.
- **Exact-head parent Actions:** AI handoff docs [42660622526] SUCCESS; vsftpd [42660622353] FAILURE at the newly added host-test compile. The two existing FTP config/daemon-policy GNU89 regression binaries had passed earlier in the same step.
- **Root cause:** `-I trunk/user/vsftpd/src` put the vsftpd source directory ahead of system include paths. Its local `features.h` shadowed glibc's `<features.h>`, causing system-header macro parse errors before the new test compiled.
- **Correction:** use GCC's quote-only `-iquote trunk/user/vsftpd/src` for the helper's quoted include. This does not affect angle-bracket system-header lookup. Reproduced the collision using a competing `features.h` fixture; the same regression compiled with GNU89, `-Wall -Wextra -Werror` and passed under `-iquote`.
- **Scope:** workflow compile flag only; no production behavior changed. The RNFR/RNTO path-scoped ACL fix and its test remain unchanged.
- **Next:** verify exact-head vsftpd and AI handoff Actions for THIS correction. Then continue the remaining FTP path-command ACL audit recorded in `SESSION_HANDOFF.md`.

---

## 2026-11-10 00:47 CST (UTC+8) — Enforce path-scoped ACLs for FTP rename source and destination

- **Base main SHA:** `f35d0a8b97db4b4ef283cbd367cd4198d63f6b73`; resulting commit: THIS COMMIT.
- **Recovered exact-head parent Actions:** vsftpd [42654362095], AI handoff docs [42654362075] and HTTPD/security [42654362050] completed SUCCESS with matching full `head_sha=f35d0a8b97db4b4ef283cbd367cd4198d63f6b73`; inspected relevant job steps.
- **Finding:** Padavan NVRAM FTP accounts all map to UID/GID 99, so per-share metadata ACLs are the meaningful account boundary. `RNFR` checked delete permission for the current working directory rather than the source parent and stored a relative source path that could change after a later CWD. `RNTO` checked only the generic deny-file filter and called `str_rename()` without requiring destination-share write permission. A path-based cross-share rename could bypass the destination ACL.
- **Fix:** pin the accepted RNFR source to an absolute path. Resolve source/destination parent directories with `realpath()` and apply the existing mount/share ACL policy to the canonical paths. Check source `PERM_DELETE` at RNFR, revalidate it at RNTO, and require destination `PERM_WRITE` before rename. Invalid or missing parents fail closed.
- **Regression:** add a GNU89-tested path-parent helper covering relative/absolute names, `..` traversal, symlinked parent directories, root/trailing slash, empty/undersized buffers and missing parents. Extend vsftpd workflow compile/run and source-contract coverage plus both push/PR filters.
- **Changed paths:** `trunk/user/vsftpd/src/asus_ext.c`, `trunk/user/vsftpd/src/asus_ext.h`, `trunk/user/vsftpd/src/postlogin.c`, `trunk/user/vsftpd/src/ftp_acl_path_policy.h`, `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source/path review PASS. The standalone path-parent helper/test compiled with `cc -std=gnu89 -Wall -Wextra -Werror` and passed locally. Exact-head Actions for THIS commit are pending. No bundled vsftpd target object, MIPS/uClibc, firmware/TRX or router test is claimed.
- **Next:** verify exact-head Actions; continue path-aware review of STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT and per-account metadata access. This patch is deliberately limited to RNFR/RNTO.

---

## 2026-11-10 00:46 CST (UTC+8) — Repair stale FTP source contracts after lifecycle API change

- **Base main SHA:** `af76d3b551a9161d1b10ecb1d33dc3cb27d49839`; resulting commit: THIS COMMIT.
- **Exact-head parent Actions:** Samba [42653764405], USB storage [42653764334], NFS [42653764330], WAN route [42653764278] and AI docs [42653764298] completed SUCCESS with matching full `head_sha=af76d3b551a9161d1b10ecb1d33dc3cb27d49839`.
- **Finding — vsftpd CI:** both GNU89 host tests (atomic config publication and bounded daemon presence) compiled and passed. The semantic workflow still searched for the old void-function `return;` after config failure; `run_ftp()` now correctly returns `-1`, causing a stale-contract false negative.
- **Finding — HTTPD CI:** the FTP trust-boundary contract sliced source from `write_vsftpd_conf()` up to the former adjacent `void run_ftp` signature. The inserted daemon wait adapter changed that adjacency, so the source range lookup failed even though the banner guard remained in place.
- **Fix:** update the FTP assertion to require `return -1;`; terminate the HTTPD FTP-config slice at the new runtime adapter; extend vsftpd filter coverage to all changed RC, policy and test files. No production service logic changed in this corrective commit.
- **Changed paths:** `.github/workflows/vsftpd-stability.yml`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** parent exact-head logs confirm both FTP GNU89 tests PASS and the remaining failures were Python source-contract lookups. New exact-head Actions are pending. No MIPS/production RC build, firmware/TRX or router test is claimed.
- **Next:** verify all exact-head Actions. If green, continue source review of the bundled vsftpd authentication/account, share-mode ACL, anonymous write/delete and chroot/path boundaries.

---

## 2026-11-10 00:35 CST (UTC+8) — Propagate vsftpd lifecycle failures into storage safety gates

- **Base main SHA:** `78508d0de29e64952dafe8d5ae2ca30ab6f20f60`; resulting commit: THIS COMMIT.
- **Recovered exact-head parent Actions:** Samba [42652191209], USB storage [42652191230], NFS [42652191208], HTTPD/security [42652191190], vsftpd [42652191151] and AI handoff docs [42652191260] all completed SUCCESS with exact matching full `head_sha=78508d0de29e64952dafe8d5ae2ca30ab6f20f60`; inspected every job and step.
- **Finding — storage safety:** `stop_ftp()` discarded `kill_services()` status, and `stop_stor_apps()` ignored FTP stop outcome. Its safe-removal/eject callers already abort unmount on a nonzero `stop_stor_apps()`, but only NFS and Samba failures were previously included. A vsftpd session continuing to serve uploads while its volume was detached could therefore be hidden.
- **Finding — daemon lifecycle:** `run_ftp()` ignored `eval("/sbin/vsftpd")` and reported no failure when the daemon immediately exited; `restart_ftpd()` ignored stop status and treated negative `storage_has_mounted_devices()` inventory errors as a truthy “mounted” state.
- **Fix:** return and propagate stop/start/restart status; require a bounded stable process-presence postcondition after startup; treat unknown storage inventory as a hard restart failure; include FTP stop failure in the existing storage removal gate; preserve best-effort stop ordering while preventing unmount after incomplete stop. Propagate results through notification markers, direct RC commands and combined FTP/Samba commands.
- **Regression:** add a GNU89 callback-driven daemon-presence test for delayed startup, exit during settling, timeout and invalid arguments; extend vsftpd workflow path filters, compile/run and source contracts to cover stop/restart status, daemon postconditions, RC propagation and storage removal gating.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/ftp_daemon_policy.h`, `trunk/user/rc/src/tests/ftp_daemon_policy.c`, `.github/workflows/vsftpd-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** local GNU89 compile/run of the new FTP daemon-presence policy test PASS. Exact-head Actions for THIS commit are pending. No local full repository production build, MIPS/uClibc object, firmware/TRX or router test is claimed.
- **Next:** verify every exact-head Action; then audit the shipped vsftpd version's authentication, account provisioning, share-mode ACL/anonymous write policy and chroot/path confinement. Do not assume directives are supported without reading the bundled daemon source.

---

## 2026-11-10 00:12 CST (UTC+8) — Align Samba regression contract with extracted daemon policy

- **Base main SHA:** `ef9217b4af66b81e76b53c21bbb0ac559756fa39`; resulting commit: THIS COMMIT.
- **Exact-head parent Actions:** USB storage [42651732010], WAN route [42651731613], HTTPD/security [42651731589], vsftpd [42651731452], AI handoff docs [42651731340] and NFS [42651731312] completed SUCCESS with matching full `head_sha=ef9217b4af66b81e76b53c21bbb0ac559756fa39`. Samba [42651731824] compiled and ran its GNU89 config, passdb and new daemon-presence regressions successfully, but the semantic source-contract step failed at the old assertion requiring the wait loop to remain inline in `services_stor.c`.
- **Finding:** daemon wait logic is now deliberately extracted into `samba_daemon_policy.h`; the previous regression still required `usleep(500000)`, `pids(daemon)` and `ETIMEDOUT` directly inside the wrapper. This was a stale test contract, not a production-code or host-test failure.
- **Fix:** assert the wrapper delegates to the extracted policy and that its runtime callbacks call `pids()` and `usleep()`; timeout interval and `ETIMEDOUT` behavior remain covered by the host policy test. Align `stop_stor_apps()` local variable indentation.
- **Changed paths:** `.github/workflows/samba-share-stability.yml`, `trunk/user/rc/src/services_stor.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** exact parent Samba log confirms compile/run success for all three GNU89 regressions; the source-contract false negative is fixed here. New exact-head Actions are pending. No production RC object/MIPS build, firmware/TRX or router test is claimed.
- **Next:** verify all Actions for THIS exact SHA. If green, resume the whole-repository queue at vsftpd authentication/credential, ACL/path-confinement and stop/restart lifecycle.

---

## 2026-11-09 23:35 CST (UTC+8) — Gate Samba stop and WINS lifecycle on verified outcomes

- **Base main SHA:** `af3a388fd889238789f9eb0d19efe6b89c2b24e9`; resulting commit: THIS COMMIT.
- **Recovered exact-head parent Actions:** HTTPD/security [42647880398] and AI handoff docs [42647880152] both completed SUCCESS with exact matching full `head_sha=af3a388fd889238789f9eb0d19efe6b89c2b24e9`; inspected all job steps.
- **Finding — storage stop:** `stop_samba()` discarded `kill_services()` status, and `stop_stor_apps()` ignored Samba's stop result. Its safe-removal/eject callers already abort unmount when `stop_stor_apps()` fails, but the failure was only collected for NFS; a still-running Samba daemon could therefore be hidden while storage removal continued.
- **Finding — restart inventory:** `storage_has_mounted_devices()` is tri-state (negative on incomplete `/proc/mounts` inventory, zero for known-empty, positive for mounted storage). `restart_smbd()` treated any nonzero result as true, so an inventory error could incorrectly enter the service-start path instead of failing closed.
- **Finding — WINS-only lifecycle:** `start_wins()` ignored the nmbd `eval()` result and did not verify that nmbd remained present. The nmbd stop/restart and smbd reload paths also discarded stop/start/reload status.
- **Audit correction:** `dev_init.sh` mounts `/etc` as tmpfs before creating `/etc/samba`. Samba config/passdb and any journal under `/etc` are volatile and disappear on reboot. A persistent power-loss journal in this location cannot provide durability; same-boot abrupt process termination is a separate, narrower concern.
- **Fix:** make Samba/nmbd stop and start/reload helpers return status; propagate `stop_samba()` through `restart_smbd()`, RC stop commands and `stop_stor_apps()`; abort storage-dependent Samba startup on unknown mount inventory; enforce bounded process-presence postconditions for WINS-only starts/reloads. Preserve best-effort service stop ordering but return failure so the existing safe-removal gate blocks unmount.
- **Regression:** add a GNU89 callback-driven daemon-presence policy test for delayed startup, exit during settling, timeout and invalid arguments; extend Samba workflow path filters, compile/run coverage and source contracts for stop propagation, storage gating and WINS result handling.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/samba_daemon_policy.h`, `trunk/user/rc/src/tests/samba_daemon_policy.c`, `.github/workflows/samba-share-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 caller/source review PASS. Exact-head GNU89/Actions validation for THIS commit is pending. No local full repository build, MIPS/uClibc production object, firmware/TRX or router test is claimed.
- **Next:** verify exact-head Samba, HTTPD, USB/storage, NFS, vsftpd and AI docs Actions; then rotate to vsftpd authentication, ACL/path confinement and stop lifecycle. Keep same-boot process-abort recovery as a separate lower-priority item; do not add an /etc tmpfs “durable journal”.

---

## 2026-11-09 22:55 CST (UTC+8) — Align HTTPD and handoff CI contracts with daemon status API

- **Base main SHA:** `30432e4636c5b00c96cb8918caaebe37b133a28c`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** Samba [42647280352], NFS [42647280434], USB storage [42647280185], vsftpd [42647280155] and WAN route [42647280379] passed. HTTPD [42647280187] failed because its source contract still searched for `void run_samba(void)` after the function became status-returning. AI handoff docs [42647282663] failed because its validator requires `## Active task` and `## Exact first action for the next GPT`.
- **Fix:** update the HTTPD regression signature lookup to `int run_samba(void)`; restore the two required handoff headings and record the exact next action. No production C change in this corrective patch.
- **Verification:** failure causes confirmed from the GitHub Actions logs; corrective exact-head Actions pending. No production RC object/firmware/device build claimed.
- **Next:** verify every current exact-head workflow; if green, continue durable PREPARED/COMMITTED passdb/config recovery and then the WINS-only daemon status path.

---

## 2026-11-09 22:45 CST (UTC+8) — Check Samba daemon reload/start status and postconditions

- **Base main SHA:** `cf75016ebf1f1150cd4db39308ead1e09bb0e66e`; resulting commit: THIS COMMIT.
- **Recovered exact-head parent CI:** all six workflows completed SUCCESS with exact matching `head_sha=cf75016ebf1f1150cd4db39308ead1e09bb0e66e`: Samba [42638624538], NFS [42638624367], HTTPD/security [42638624464], USB storage [42638624595], vsftpd [42638624417], and AI handoff docs [42638624607].
- **Finding:** `run_samba()` ignored command status for `killall -SIGHUP` and daemon `eval()`; a failed reload/start could be hidden from RC callers and the notification queue.
- **Fix:** check each required nmbd/smbd command result and a bounded stable-PID postcondition; treat a nonzero command result or missing/stale process as failure. Check optional WSDD2 too, but keep its failure as a warning rather than masking SMB availability. Make `run_samba()`/`restart_smbd()` return int; propagate through RC notification and direct command entry points. Storage startup logs failures.
- **Regression contract:** Samba workflow checks command return handling, bounded PID postconditions, function return types and RC propagation; add `rc.c` to workflow path filters.
- **Verification:** source/caller review PASS. Parent exact-head Actions above PASS. This commit's Samba/HTTPD Actions pending exact-head. No local full checkout, production RC object, MIPS/uClibc build, firmware/TRX or physical router test is claimed.
- **Next:** implement durable PREPARED/COMMITTED recovery for the passdb/config pair; then fix WINS-only nmbd start/stop status propagation before rotating to vsftpd lifecycle.

---

## 2026-11-09 22:20 CST (UTC+8) — Defer Samba share config until passdb provisioning succeeds

- **Base main SHA:** `90e6c14d835216f06163f1347c0d5d8a3a465484`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding NFS custom override fail-closed commit passed all six workflows on matching exact `head_sha=90e6c14d835216f06163f1347c0d5d8a3a465484`: Samba config [42635610400](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610400), vsftpd [42635610401](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610401), USB storage [42635610419](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610419), AI handoff docs [42635610461](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610461), NFS [42635610314](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610314), and HTTPD/security [42635610731](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610731).
- **Finding:** `run_samba()` atomically published the new `/etc/smb.conf` before validating/provisioning all NVRAM credentials. On Samba 3.6.25 the passdb is separately staged and the previous database is preserved when a later credential fails, so the new share ACL/config could be left on disk with the old passdb while daemon reload is skipped.
- **Fix:** finish all account writes to the private staged passdb first. Only then snapshot the previous `/etc/samba/smbpasswd` with a same-directory hard link, atomically replace it, and publish the newly generated smb.conf through the existing staged writer. If config generation/publication fails, restore the old passdb (or remove a new first-run DB); preserve and log the recovery snapshot if rollback itself fails. A passdb rename failure leaves old smb.conf untouched.
- **Regression:** extend `samba_passdb_policy.h` and its GNU89 test for hard-link snapshot/restore, first-run cleanup, and non-regular/symlink target rejection. Extend `samba-share-stability.yml` to compile/run those cases and assert source order: credential provisioning -> passdb snapshot/replace -> config publication -> daemon reload.
- **Limit:** this is rollback on normal error returns, not a power-loss-atomic two-file transaction. A reset between the passdb replacement and smb.conf publication may leave the hidden rollback snapshot; interrupted-transaction startup recovery remains a follow-up. Live daemon reload/signal command status propagation also remains open.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/samba_passdb_policy.h`, `trunk/user/rc/src/tests/samba_passdb_policy.c`, `.github/workflows/samba-share-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 caller-order and rollback-flow review PASS. GNU89 host regression and exact-head Samba/HTTPD Actions are pending for THIS commit. No full local repository compile, MIPS/uClibc production object, firmware/TRX or router test is claimed.
- **Next:** reconcile exact-head Samba/HTTPD workflows; then consider startup recovery for interrupted passdb/config updates and check daemon SIGHUP/start status/postconditions before rotating to vsftpd.

---

## 2026-11-09 22:18 CST (UTC+8) — Fail closed on invalid custom NFS exports paths

- **Base main SHA:** `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding generated NFS reserved-source-port commit was green on exact `head_sha=a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`: storage [42631443686](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443686), NFS [42631443757](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443757), vsftpd [42631443784](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443784), Samba [42631443514](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443514), AI docs [42631443719](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443719) and HTTPD [42631443747](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443747); all six were completed SUCCESS and their full `head_sha` values matched.
- **Finding:** `write_nfsd_exports()` used `check_if_file_exist()`, which follows symlinks with `stat()` and returns false for directories/non-regular targets and unresolved symlinks. A dangling `/etc/storage/exports` symlink or an existing non-regular/inaccessible path was therefore conflated with an absent override; the writer could publish generated default media exports instead of preserving the operator's intended custom-export boundary.
- **Fix:** use `lstat()` to distinguish an existing entry, follow only symlinks with `stat()`, and permit generated fallback only for an `ENOENT` lstat result. Accept a direct regular file or a symlink to a regular file; fail closed on other lstat errors, dangling/looping links, non-regular targets, and direct directories/FIFOs/devices. Reject before reading `/proc/mounts` or staging `/etc/exports`, so invalid override state cannot replace the existing exports file. Preserve existing atomic symlink publication for valid overrides.
- **Regression:** add GNU89 state-matrix assertions for true absence, EACCES/ENOTDIR, direct regular file, valid symlink, dangling/error symlink, non-regular symlink target and direct non-regular entry. Extend NFS workflow's source contract to require the lstat/stat classifier before generated exports and forbid the old `check_if_file_exist(exports_link)` branch.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 source and error-path review PASS. Focused standalone classification test compiled with `cc -std=gnu89 -Wall -Wextra -Werror` and passed; this scratch test exercises the policy logic, not the complete production source. Exact-head NFS/HTTPD/storage/docs Actions are pending on THIS commit. No full local checkout, MIPS/uClibc production object, firmware/TRX or router test is claimed.
- **Compatibility/open risks:** `no_root_squash` remains for root-client compatibility; FAT/exFAT/NTFS/HFS automount uses broad `umask=0`, while ext filesystems retain POSIX ownership/mode behavior, so root-squash needs a separate compatibility path. `async` durability versus throughput is separate. A valid custom export still bypasses generated ACL/path restrictions; malformed regular-file content and `exportfs` recovery remain open; external/network mounts require an explicit trusted override.
- **Next:** reconcile all exact-head workflows. If green, rotate to Samba passdb/account mutation consistency, credential failure/recovery and live reload. Keep NFS `async`, root-squash, custom override authority/content handling and external-mount behavior as explicit audit items.

---

## 2026-11-09 22:00 CST (UTC+8) — Require reserved source ports for generated NFS exports

- **Base main SHA:** `010d5ebec0926cb52686b5c24f145d33a06514d7`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding Samba write-only ACL correction was green on exact `head_sha=010d5ebec0926cb52686b5c24f145d33a06514d7`: Samba [42628532465](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532465), HTTPD [42628532589](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532589), NFS [42628532555](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532555), storage [42628532417](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532417), vsftpd [42628532559](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532559), and AI docs [42628532588](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532588); all six full `head_sha` values exactly matched.
- **Finding:** the generated `/media/*` NFSv2/v3 AUTH_SYS export policy used `async,insecure,no_root_squash,no_subtree_check`. In Linux 4.4 `fs/nfsd/auth.c::nfsd_setuser()`, the request UID/GID is applied directly to filesystem credentials and UID/GID 0 is mapped only when `NFSEXP_ROOTSQUASH` is enabled. In `fs/nfsd/nfsfh.c::nfsd_setuser_and_check_port()`, a non-reserved source port is rejected unless `NFSEXP_INSECURE_PORT` is set. Together, `insecure,no_root_squash` let an unprivileged process on an ACL-authorized LAN/VPN network claim UID 0 from a high source port and operate as root on writable exported media.
- **Compatibility decision:** keep `no_root_squash` and `async` unchanged in this bounded fix. Root-squash changes access to existing root-owned files and needs a separate root-client/migration plan. `async` durability versus USB throughput also needs a separate decision. This commit removes only the unnecessary `insecure` exception by requiring `secure` source ports; standard clients that use reserved ports retain root UID mapping. Root clients can still act as server root, so this is a targeted mitigation, not full AUTH_SYS authentication or root-squash protection.
- **Fix:** define the shared default `async,secure,no_root_squash,no_subtree_check` and make `write_nfsd_exports()` consume it. Preserve the operator `/etc/storage/exports` override, which remains a complete authoritative configuration that bypasses generated LAN/VPN ACL and local-media path restrictions; an administrator may explicitly opt into `insecure` there, but must treat the file as trusted root configuration.
- **Regression:** extend the GNU89 policy test to assert the exact option tuple, absence of `insecure` and intentional retention of `no_root_squash`. Extend the NFS workflow source contract to require use of the shared option constant, forbid the former default, and preserve custom override precedence/staged symlink publication.
- **Changed paths:** `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 kernel/source trust-boundary review PASS. The GNU89 regression and exact-head NFS/HTTPD/docs Actions are pending on THIS commit. No local complete checkout, host compiler execution or documentation validator invocation is claimed. L3 MIPS/uClibc/production RC/firmware and L4 client behavior/physical router remain NOT VERIFIED.
- **Next:** reconcile exact-head Actions. If green, decide a compatibility-backed root-squash migration, then audit `async` durability, custom exports absent/dangling/unreadable/malformed-path behavior, export failure recovery and external-mount support before resuming Samba passdb/account/credential audits.

---

## 2026-11-09 21:31 CST (UTC+8) — Preserve Samba write-only permissions in generated share ACLs

- **Base main SHA:** `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`; resulting commit: THIS COMMIT. Active branch `main`; no other checkpoint branch exists.
- **Recovered exact-head CI:** after the NFS IPv4 policy-header guard fix, AI handoff docs [42625376685](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376685), NFS lifecycle [42625376591](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376591), and HTTPD security [42625376585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376585) completed SUCCESS; each full `head_sha` exactly matched `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`.
- **Finding:** `libdisk/src/disk_share.c::get_permission()` returns CIFS rights 0=deny, 1=read, 2=write, 3=read/write, and returns -1 for malformed/error state. Samba mode-4's `invalid users` builder used “cannot read” as “deny”. That incorrectly put right=2 users into the deny list despite adding them to `write list`; Samba denies users listed in `invalid users`, including users also allowed by `valid users`. The same read-only guest gate omitted write-only guest sessions from `guest ok` and `valid users`.
- **Fix:** add `samba_acl_right_is_denied()`, which denies only when neither read nor write is granted (including -1 and out-of-range values). Use it for the guest login/valid-users gates and the account deny list. Keep read-list and write-list output tied to read and write rights respectively. This restores write-only behavior without treating lookup errors as grants.
- **Exact-head Actions:** Samba [42628532465](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532465), HTTPD [42628532589](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532589), NFS [42628532555](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532555), storage [42628532417](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532417), vsftpd [42628532559](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532559) and AI docs [42628532588](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532588) all SUCCESS on exact `head_sha=010d5ebec0926cb52686b5c24f145d33a06514d7`.
- **Regression:** `tests/samba_share_policy.c` covers -1/0/1/2/3/4; HTTPD production-source contract requires the helper in both guest gates and the actual `invalid users` block.
- **NFS audit evidence:** nfs-utils 2.6.1 disables NFSv4, GSS and Kerberos and installs no GSS auth daemons; generated exports retain `async,insecure,no_root_squash,no_subtree_check`. Kernel nfsd maps request UID/GID directly unless root-squash is enabled. With AUTH_SYS and `no_root_squash`, a client in an admitted LAN/VPN network can assert UID/GID 0 against exported removable-media data. This is a P2 data trust-boundary risk, not evidence of direct router-root compromise. `async` durability, `insecure` source-port behavior, custom `/etc/storage/exports` authority and external mounts remain compatibility items; no historic NFS option was changed.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/samba_share_policy.h`, `trunk/user/rc/src/tests/samba_share_policy.c`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source and call-site review completed. L2 exact-head HTTPD/Samba/NFS/storage/docs workflows PENDING; no local complete checkout or docs validator invocation is claimed. L3 production RC/MIPS/full firmware and L4 physical router remain NOT VERIFIED.
- **Next:** inspect all Actions for THIS resulting SHA. If green, make an explicit, compatibility-backed NFS AUTH_SYS/root-squash policy decision before altering export defaults; then continue Samba passdb/account mutation and vsftpd authentication/path lifecycle audits.

---

## 2026-11-09 21:27 CST (UTC+8) — Correct NFS policy header conditional guard after exact-head compile failure

- **Base main SHA:** `ef1aec7b10389eaf240c20b5e999d80ef48996d1`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** storage [42624781463](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781463), Samba [42624781449](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781449), vsftpd [42624781407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781407) and AI docs [42624781292](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781292) all SUCCESS on exact SHA. NFS [42624781415](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781415) and HTTPD [42624781560](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781560) failed during compile of `storage_nfs_export_policy.h`; each run's full `head_sha` matched `ef1aec7b10389eaf240c20b5e999d80ef48996d1`.
- **Finding:** the new unconditional IPv4 includes and old IPv6-only block left two consecutive `#if defined (USE_IPV6)` openings but only one corresponding close, so the outer include guard was unterminated. NFS shell syntax and lifecycle fixture passed; the HTTPD NFS-policy step was reached after all preceding checks succeeded.
- **Fix:** remove the duplicate opening conditional only; retain one IPv6 helper guard and the final header include guard. No production ACL algorithm, caller behavior or historic exports options changed.
- **Changed paths:** `trunk/user/rc/src/storage_nfs_export_policy.h`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** preprocessor directive balance checked from the fetched exact source (outer include guard and one IPv6 block balanced); NFS/HTTPD Actions compile reruns pending on THIS commit's exact head. No local complete checkout or target build claimed.
- **Validation boundary:** L3 MIPS/uClibc/production RC object, complete firmware/TRX and L4 physical-router NFS behavior remain NOT VERIFIED.
- **Next:** reconcile all Actions for THIS exact SHA; only then continue the NFS AUTH_SYS identity, `no_root_squash`, `async`, `insecure`, custom-exports and external-mount audit.

---

## 2026-11-09 21:20 CST (UTC+8) — Validate NFS LAN and VPN IPv4 export networks

- **Base main SHA:** `3c879e06efa256ffbf376848ad69359a9098330e`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding storage regression correction passed USB storage [42623404967](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42623404967) and AI handoff docs [42623405148](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42623405148), both SUCCESS on exact `head_sha=3c879e06efa256ffbf376848ad69359a9098330e`. The exact NFS lifecycle workflow [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) passed on the unchanged NFS source SHA `eb21523750bc3218121f5adf3457c587d8e9a6e0`.
- **Finding:** `write_nfsd_exports()` built IPv4 LAN and VPN ACLs with legacy `ip2class()`. That helper feeds unchecked `inet_addr()` results into the network calculation and counts every set netmask bit instead of verifying a contiguous prefix. The LAN path's `is_valid_ipv4()` check validates only address syntax, not mask contiguity; the enabled VPN path did not validate `vpns_vnet` at all. A non-contiguous mask could be represented as a different CIDR, resulting in an unintended export client range.
- **Fix:** add a strict IPv4 ACL formatter that validates dotted-quad address/netmask syntax, rejects non-contiguous masks, unspecified/all-ones addresses and unrestricted /0, computes the canonical network and checks output bounds. Use it for LAN and enabled VPN ACLs; on invalid state discard the staged exports candidate and prevent NFS start/reload. Preserve the legacy `async,insecure,no_root_squash,no_subtree_check` options pending their separate compatibility review.
- **Regression:** add GNU89 host cases for /24, /32 and /1, non-contiguous masks, /0, invalid/broadcast addresses, injection-like tokens and undersized output buffers. Extend the NFS workflow source contract to require the formatter in both LAN/VPN paths and forbid the unchecked legacy call in the export writer.
- **Changed paths:** `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `trunk/user/rc/src/services_stor.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 source review; the affected GNU89 test and exact-head Actions are pending. No local full checkout/target build or hardware validation is claimed.
- **Validation boundary:** L3 MIPS/uClibc and production RC object, full firmware/TRX, live NFS AUTH_SYS/permission behavior and L4 hardware remain NOT VERIFIED.
- **Next:** reconcile all affected Actions against the resulting full SHA; then continue the NFS root UID/GID, export-option and custom-exports compatibility audit.

---

## 2026-11-09 21:13 CST (UTC+8) — Align storage caller-graph contract with NFS mount-state semantics

- **Base main SHA:** `eb21523750bc3218121f5adf3457c587d8e9a6e0`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** NFS [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) and AI handoff docs [42622521280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521280) both SUCCESS on exact `head_sha=eb21523750bc3218121f5adf3457c587d8e9a6e0`. USB storage [42622521225](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521225) FAILED on the same exact SHA at its final whole-caller-graph assertion; all preceding shell syntax, Optware lifecycle and GNU89 host regression steps passed.
- **Finding:** the storage workflow still expected `func_unmount_if_mounted()` to use both numeric `$mount_status -eq 0/1` tests. The NFS mount-identity fix replaced that logic with an explicit `case` that preserves the three states (expected mount / absent / conflicting or unreadable) and refuses unsafe teardown. This is a stale cross-workflow contract, not a production NFS test failure.
- **Correction:** require the `case`-based pre-unmount conflict guard, exact tuple inventory invocation, refusal log, checked `umount`, guard-before-unmount ordering and post-unmount absent-state check. Production NFS/storage code is unchanged.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** exact job-log/source review identified the obsolete assertion in the final caller-graph step. No local full checkout or local execution of the workflow Python contract/docs validator is available; the updated storage and docs Actions are pending on THIS COMMIT's exact head.
- **Validation boundary:** L1 contract/source review PASS. NFS runtime workflow and docs/archive workflow PASS on the base exact SHA; the full storage workflow is not green yet. L3 MIPS/uClibc, complete firmware/TRX and L4 hardware remain NOT VERIFIED.
- **Next:** reconcile storage/docs Actions on the resulting exact SHA; then continue NFS export UID/GID/permission and option compatibility review before rotating to Samba passdb/account/ACL/credential security.

---

## 2026-11-09 21:01 CST (UTC+8) — Verify exact NFS pseudo-filesystem mounts before startup and teardown

- **Base main SHA:** `363f2e4101bd8087e25710e875a4719023af6637`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding IPv6 export ACL commit passed NFS [42620760496](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760496), USB storage [42620760576](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760576), HTTPD [42620760658](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760658), vsftpd [42620760715](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760715), AI docs [42620760726](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760726), and Samba [42620760733](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760733); all six exact `head_sha` fields matched the full SHA.
- **Finding:** `func_mount_if_needed()` and `func_unmount_if_mounted()` in `nfsd.sh` searched only for the mountpoint text. They could accept an unrelated source/filesystem as the nfsd or rpc_pipefs pseudo-filesystem and could unmount that unrelated filesystem during rollback or stop. Multiple stacked mount entries were also treated as a simple present mount.
- **Fix:** add an exact `/proc/mounts` tuple check for source, mountpoint and filesystem type; distinguish absent from exactly one expected row and reject mismatched/duplicate/error states. Refuse to mount over a conflict, verify the expected entry after mount, only unmount an exact single expected tuple, and require the mountpoint to be empty afterwards.
- **Regression:** update the NFS lifecycle fixture's mount mock to record the real `mount -t filesystem source mountpoint` tuple. Add wrong-source, wrong-filesystem and duplicate/stacked mount cases, assert conflicts are preserved and never unmounted, and extend the NFS workflow source contract to cover exact field equality and guarded teardown.
- **Changed paths:** `trunk/user/nfs-utils/nfsd.sh`, `trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** local shell regressions PASS for absent, expected, wrong-source, wrong-filesystem, prefix-neighbor and duplicate mount entries; guarded mount/unmount simulation PASS. The full repository NFS workflow and AI docs/archive validator are PENDING on THIS exact commit.
- **Validation boundary:** L1 focused source review PASS. The test harness does not execute on the actual router mount namespace; MIPS/uClibc production RC build, complete firmware/TRX, real mount-stack behavior and L4 hardware validation are NOT VERIFIED.
- **Remaining risks:** check-to-mount/unmount races are not made atomic by this shell-level check. Only the reserved fixed NFS pseudo-filesystem mountpoints are covered. Historic NFS export options and root identity semantics remain unchanged and need a separate compatibility/security audit.
- **Next:** verify NFS lifecycle and AI documentation Actions on the exact resulting SHA. If green, continue the NFS root UID/GID/export permission review (`no_root_squash`, AUTH_SYS, `async`, `insecure`), custom exports override and external-mount behavior.

---

## 2026-11-09 20:51 CST (UTC+8) — Validate IPv6 LAN ACL before publishing NFS exports

- **Base main SHA:** `5faded68510e91fb526dea6ab3ebd34b5d9dc252`; resulting commit: THIS COMMIT.
- **Recovered exact-head state:** preceding MTD retry/shutdown checkpoint passed USB storage [42618640557](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640557), AI docs [42618640385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640385), Samba [42618640482](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640482), WAN [42618640381](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640381), and HTTPD [42618640379](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640379); all run SHAs exactly matched `5faded68510e91fb526dea6ab3ebd34b5d9dc252`.
- **Finding:** `write_nfsd_exports()` emitted raw `ip6_lan_addr` and `ip6_lan_size` NVRAM values into `/etc/exports`. Unlike the separate IPv6 network configuration path, this writer did not validate either field before interpreting them as NFS export syntax.
- **Fix:** validate the IPv6 address with `inet_pton(AF_INET6)`, accept only the existing optional CIDR lexical form, canonicalize with `inet_ntop(AF_INET6)`, and accept only decimal LAN prefix lengths 48–64. The separately stored prefix size remains authoritative, matching `reset_lan6_vars()`. Emit only the canonical combined value. Invalid non-empty configuration aborts and removes the staged file before `/etc/exports` is renamed, preserving the previous file.
- **Regression:** extend `storage_nfs_export_policy.c` with canonicalization, optional CIDR, empty/unconfigured, /48–/64 bounds, invalid addresses and NFS syntax injection cases. The NFS workflow now compiles/runs the policy in GNU89 with `USE_IPV6`, watches helper/test paths on push and pull requests, and asserts the production writer cannot emit the raw address/prefix interpolation.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source review PASS. L2 GNU89 IPv6 ACL policy compile/run, NFS source/start/reload regression, storage/HTTPD caller contracts and docs/archive validator pending for THIS COMMIT's exact head. L3 MIPS/uClibc production object and firmware/TRX NOT VERIFIED. L4 live NFS/IPv6 client/permissions, storage-eject and hardware NOT VERIFIED.
- **Open security compatibility item:** generated exports still use the historic `async,insecure,no_root_squash,no_subtree_check` options. Root squash, AUTH_SYS identity mapping and durability/client-port policy require a separate backward-compatibility and permissions review; this commit does not silently change access semantics.
- **Next:** verify exact-head NFS/storage/HTTPD/docs Actions, then continue NFS root UID/GID policy and external-mount lifecycle/mountpoint matching audit.

---

## 2026-11-09 20:30 CST (UTC+8) — Bound MTD Optware retries and cancel its worker during shutdown

- **Base main SHA:** `652581beb0b71787096a1a74a90a3f6c32ce7cd6`; resulting commit: THIS COMMIT.
- **Recovered exact-head state:** the previous MTD caller-contract checkpoint passed USB storage [42615347517](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347517) and AI handoff docs [42615347722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347722), both with exact `head_sha=652581beb0b71787096a1a74a90a3f6c32ce7cd6`. The remote branch list contained only `main`.
- **Finding:** boot-time MTD Optware retries were relaunched every five seconds indefinitely after persistent hook failures. The worker inherited PID1's blocked signal mask and signal dispositions: most importantly, PID1's SIGCHLD reaper could reap the shell child used by `system()`, while inherited shutdown handlers did not terminate the worker. No explicit worker-group stop ran before ordinary services were stopped, allowing late startup hooks to race shutdown.
- **Fix:** retain five-second nonblocking polling but exponentially back off actual relaunch attempts from 5 seconds to a 300-second ceiling using monotonic `uptime()`; reset after successful startup. Give the worker a private process group, reset inherited signal dispositions and unblock its signal mask before `system()`, track the worker PID, and terminate its process group with TERM/KILL from `shutdown_router()` before ordinary service-stop work. Keep external hooks asynchronous and outside `storage.lock`; preserve fail-closed RWFS/UBI teardown.
- **Regression:** extend the existing GNU89 retry-policy test with the complete backoff sequence and saturation checks. Extend the storage workflow's whole-caller-graph contract to assert worker group/signal isolation, monotonic backoff/reset, PID tracking, SIGCHLD-safe cancellation and shutdown ordering.
- **Changed paths:** `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/storage_optware_start_policy.h`, `trunk/user/rc/src/tests/storage_optware_start_policy.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source/caller review PASS. L2 GNU89 policy compile/run, whole storage caller-graph contract and docs/archive validator are pending exact-head Actions for THIS COMMIT. No local full checkout or production RC build. L3 MIPS/uClibc/firmware and L4 actual UBI hooks, shutdown race injection and router hardware are NOT VERIFIED.
- **Risk note:** a third-party daemon that deliberately changes process group/session can escape group cancellation; the existing RWFS teardown lock/result gates remain fail-closed if Optware teardown is still busy.
- **Next:** verify exact-head storage/docs Actions. If green, audit NFS export ACL/permission mapping and external mount lifecycle across `nfsd.sh`, `services_stor.c` and RC safe-removal callers.

---

## 2026-11-09 20:15 CST (UTC+8) — Remove obsolete MTD fire-and-forget workflow assertion

- **Base main SHA:** `93406e8618deae765a7c9cf6033ce978e58e230a`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** USB storage [42615049407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049407) passed the Optware/NFS shell checks, lifecycle lock/teardown/startup regressions, GNU89 inventory test, deferred child-result test, and all new MTD worker/result/retry/PID1 source checks. It then failed at the final legacy assertion that still required `doSystem("/usr/bin/opt-mount.sh ... &")`. Samba [42615049566](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049566), WAN route [42615049422](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049422), HTTPD [42615049699](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049699), and docs [42615049390](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049390) all SUCCESS on the same full SHA.
- **Correction:** remove the superseded fire-and-forget expectation and retain explicit presence checks for the new tracked `start_rwfs_optware()` and `retry_rwfs_optware()` entrypoints. Production C is unchanged.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** base workflow's earlier shell/host/GNU89 tests and new MTD assertions passed; only stale final assertion failed. Exact-head storage/docs reruns pending. No full local checkout, production RC object, MIPS/uClibc, firmware/TRX, or hardware test claimed.
- **Next:** verify exact-head storage/docs Actions, then assess persistent-failure retry/backoff and shutdown races.


---

## 2026-11-09 20:08 CST (UTC+8) — Track and retry internal MTD/UBI Optware boot startup

- **Base main SHA:** `19c1627490ec550b74b2deb3e19e8965d4374406`; resulting commit: THIS COMMIT.
- **Recovered exact-head verification:** prior removable USB deferred startup fix is complete: storage [42613569550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569550) and AI handoff docs [42613569474](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569474) both SUCCESS on full SHA `19c1627490ec550b74b2deb3e19e8965d4374406`; all 11 storage steps and the handoff/archive validator passed.
- **Finding:** boot-time `start_rwfs_optware()` still used `doSystem("... opt-mount.sh ... &")`, which only reports background launch and discards the actual MTD Optware startup exit status. There was no retry mechanism if the script's later mount/lock/hook path failed.
- **Fix:** retain asynchronous startup but track a worker using a nonblocking pipe; the worker waits for the real `opt-mount.sh` status, writes a one-byte result and signals PID1. Add a retry/poll function and call it from PID1's deferred SIGALRM event, rearming five-second polling while running or after failure. Share the existing Optware child-result decoder, use close-on-exec on the writer, and provide a no-NAND stub. PID1 never waits synchronously for external hooks.
- **Regression:** extend the storage workflow's symmetric push/PR filters and whole-caller-graph source contract to check MTD result reader, worker descriptor flags, fork/exit decoding, result publication, pending-state transitions, retry, and PID1 event-loop caller.
- **Changed paths:** `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/init.c`, `trunk/user/rc/src/rc.h`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source review PASS. Prior USB storage+docs Actions are green on the exact base. THIS commit's exact-head storage/docs Actions pending. No full local checkout, production RC object, MIPS/uClibc, firmware/TRX, real NAND/UBI or physical-router test claimed.
- **Next:** reconcile exact-head storage/docs workflows; then scrutinize persistent-failure retry cadence and shutdown while the lifecycle lock is held.


---

## 2026-11-09 19:52 CST (UTC+8) — Correct deferred Optware caller-graph assertion anchors

- **Base main SHA:** `a154f30da5d0eac2d3923d221c39eeb8295965f2`; resulting commit: THIS COMMIT.
- **Exact-head evidence:** USB storage run [42612494953](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494953) compiled and passed the new GNU89 child-result decoder test, Optware shell regressions, and storage inventory/lock tests, then failed the final source contract at stdin line 129 (the deferred inventory/startup ordering assertion). AI docs [42612494818](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494818), Samba [42612494997](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494997), vsftpd [42612494935](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494935), NFS [42612494823](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494823), and HTTPD [42612494815](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494815) SUCCESS on the same exact SHA.
- **Correction:** explicitly anchor the inventory assignment, scope the mounted-branch `usb_hotplug_ms` clear search after `start_stor_apps();` (the complete-empty branch legitimately clears it earlier), and find `storage_optware_start_async()` without requiring a standalone semicolon. This is a workflow contract correction only; production code and test coverage are unchanged.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** base GNU89 child-result test and all earlier shell/storage tests PASS; final source contract failed at the identified assertion. Exact-head storage/docs reruns pending. No full local checkout, full production object, MIPS/uClibc, firmware/TRX, or target-router validation claimed.
- **Next:** reconcile exact-head storage/docs Actions; then audit the separate async MTD boot-time Optware caller result/retry path.


---

## 2026-11-09 19:45 CST (UTC+8) — Preserve deferred USB Optware startup result and retry state

- **Base main SHA:** `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the previous MTD/UBI and RWFS source-contract task is complete: storage [42609876845](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876845) and AI handoff docs [42609876722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876722) both SUCCESS on the exact full SHA `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`. All storage workflow steps and the documentation/archive validator passed.
- **Finding:** `storage_has_mounted_devices()` conflated incomplete `/proc/mounts` reads with a complete empty inventory. The deferred handler also treated successful background launch as successful Optware startup, cleared both pending NVRAM flags without observing hook exit status, and repeated integrated service startup on retries.
- **Fix:** preserve the inventory's three-state result; keep both pending flags on incomplete inventory; clear the one-shot `usb_hotplug_ms` flag only after integrated services have been started against a complete mounted inventory; run `opt-start.sh` in a single asynchronous worker only after releasing `storage.lock`; deliver its actual exit result through a nonblocking pipe and SIGALRM wakeup; clear `usb_opt_start` only after child success, retain it and retry after child failure/EOF, and suppress duplicate worker launches while the result channel is open.
- **Regression:** add a GNU89 `-Wall -Wextra -Werror` unit test for child-result decoding and source contracts for inventory tri-state, lock release before fork, close-on-exec pipe writer, real wait-status decoding, child result publication, retry flag ordering, and symmetric push/PR path filters. The policy unit test compiled and passed locally in a scratch directory.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, new `trunk/user/rc/src/storage_optware_start_policy.h`, new `trunk/user/rc/src/tests/storage_optware_start_policy.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Verification boundary:** L1 source/lock-order review PASS; local GNU89 child-result policy test PASS. Exact-head storage and AI-doc Actions for THIS commit are pending at creation. No full local checkout/full-source build, production RC object, MIPS/uClibc object, firmware/TRX, external hook failure/hang injection, removable-media stress or physical-router test is claimed.
- **Next:** reconcile the new exact-head storage/docs workflows. If green, audit the separate asynchronous boot-time `start_rwfs_optware()` result/retry path in `common_ex.c`.


---

## 2026-11-09 19:03 CST — Correct RWFS failure-gate source-contract search offset

- **Base main SHA:** `d225e9e1e09e56b79c6b00b46e61d2e0a08b6166`; resulting workflow-validation checkpoint: THIS COMMIT.
- **Exact-head Actions:** storage [42609330908](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609330908) passed NFS/Optware shell syntax, Optware lock/teardown/startup host regressions and GNU89 storage tests, then failed only in the final caller-graph Python contract at line 309. The lookup searched for `if (doSystem(` starting after the matched `doSystem` call, even though the `if` token precedes it. Docs [42609330808](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609330808) SUCCESS on the same SHA.
- **Correction:** start the `if (doSystem(` lookup at the RWFS function body. The checks still require its failure return to precede RWFS unmount and UBI detach. No production source or tested behavior changed.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Verification boundary:** base shell/runtime/GNU89 checks passed; final source contract failed only at this offset. Exact-head Actions pending for THIS commit. No local test/build or target runtime claimed.
- **Next:** reconcile exact-head storage/docs Actions. When green, resume the documented USB deferred Optware child result/retry audit; inspect alarm callback/signal delivery and child ownership before changing NVRAM flag semantics.

---

## 2026-11-09 19:00 CST — Correct MTD branch anchor in storage source contract

- **Base main SHA:** `b13ca6b46e0bef9474ee6744cf4f7f1734aadc97`; resulting validation checkpoint: THIS COMMIT.
- **Exact-head Actions:** storage regression [42608853044](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42608853044) passed `opt-mount.sh` syntax, Optware lock, teardown-status, startup/rollback and GNU89 storage tests, then failed only in the final caller-graph Python contract. The traceback was `ValueError: substring not found` at line 294: the assertion looked for an explicit `if [ -n "$mtd_device" ] ; then`, while the production source represents its MTD branch as the `else` of the removable-device check. AI docs [42608852967](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42608852967) SUCCESS on the same full SHA.
- **Correction:** anchor the MTD-specific source contract to the existing explanatory branch comment. Preserve all checks that the shared lock is sourced/acquired before /opt mount and bind, common supervised hooks run, and the lock is released. No production assertion or lifecycle behavior is weakened.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Verification boundary:** this is a test-contract-only correction. Base Actions establish that shell syntax, existing Optware regressions and storage GNU89 tests pass; the new static contract did not reach its new RWFS/MTD assertions. Exact-head storage/docs Actions are pending for THIS commit. No local build/test, production MIPS object, firmware/TRX, UBI runtime or device test is claimed.
- **Next:** reconcile exact-head storage/docs results. If green, continue the deferred USB `opt-start.sh` result/retry redesign in `services_stor.c`; audit async result/retry for the boot-time `start_rwfs_optware()` caller separately.

---

## 2026-11-09 18:57 CST — Serialize internal MTD Optware startup and fail closed on RWFS teardown

- **Base main SHA:** `0ae74382cdeb593770ef609f7685f36d28ee46f8`; resulting implementation/test/docs checkpoint: THIS COMMIT.
- **Exact-head recovery:** before editing, USB storage hotplug regression [42602101323](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42602101323) and AI handoff documentation contract [42602101300](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42602101300) were both SUCCESS on the exact full base SHA. The remote branch list contained only `main`.
- **Module / priority:** USB storage / internal NAND RWFS Optware lifecycle, P2 stability.
- **Caller trace:** `init_router()` calls `start_rwfs_optware()` in `common_ex.c`, which launches `opt-mount.sh /dev/ubiN_0 /media/mtd_rwfs` asynchronously and does not hold `storage.lock`. The normal removable-storage caller reaches `opt-mount.sh` from `automount.sh` while mdev owns `storage.lock`, but that non-MTD branch only records `usb_opt_start` and returns. The internal MTD/UBI branch formerly bound `/opt` and ran hooks through an unquoted `ls` command-substitution loop, bypassing the shared lifecycle lock and rollback helper. On shutdown, `umount_rwfs_partition()` ignored `opt-umount.sh` failure and continued with RWFS unmount/UBI detach even though the teardown script deliberately returns failure when startup owns the lifecycle lock.
- **Fix:** MTD/UBI `opt-mount.sh` now acquires the shared lifecycle lock before the `/opt` mount check and holds it across bind setup and supervised service startup. It calls `optware_lifecycle_start_services()`, propagates hook/rollback failure, and releases the lock on every explicit early/success/failure exit. The removable-storage branch retains its deferred-start behavior and does not take this lock while the caller owns `storage.lock`. `umount_rwfs_partition()` now aborts before unmount/detach when the Optware stop hook fails; a real RWFS unmount error also keeps UBI attached, and detach failures are logged.
- **Changed paths:** `trunk/user/optware/opt-mount.sh`, `trunk/user/rc/src/common_ex.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Regression:** extend the storage workflow to run `sh -n opt-mount.sh` and assert the MTD caller takes the lifecycle lock before binding, uses the common rollback supervisor instead of the direct loop, and that shutdown gates both RWFS unmount and UBI detach on teardown/unmount status. Exact-head Actions are pending at commit creation; no local test/build is claimed.
- **Verification boundary:** L1 source/caller and lock-order review PASS. L2 storage/AI-doc Actions PENDING on the resulting exact SHA. No MIPS/uClibc production object, firmware/TRX, live UBI filesystem, external init-hook failure injection or physical-router test. The C boot caller still launches MTD startup asynchronously and does not consume the script exit status; the deferred USB child result/retry contract is also open.
- **Next:** reconcile exact-head storage/docs Actions. Then fix `try_start_stor_apps()` / `on_deferred_hotplug_dev()` so background `opt-start.sh` failure does not lose `usb_opt_start`/hotplug retry state, without executing external hooks under `storage.lock` or starting duplicate children while the lifecycle lock is busy. Preserve pending work when mount inventory is incomplete, and do not repeat integrated service startup on every retry. Separately assess retry/error reporting for asynchronous `start_rwfs_optware()`.

---

## 2026-11-09 18:36 CST (UTC+8) — Align storage source contract with shared Optware startup helper

- **Base main SHA:** `57aee4fd406d9e4c06f3ee113742ba5b6999fc0d`; resulting commit: THIS COMMIT.
- **Exact-head evidence:** startup regression, teardown propagation, lifecycle-lock regression, NFS shell syntax and GNU89 storage inventory/operation-lock tests all SUCCESS on `57aee4fd406d9e4c06f3ee113742ba5b6999fc0d`. AI handoff docs [42601392329](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42601392329) SUCCESS on that exact SHA. Storage [42601392256](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42601392256) failed only in its final whole-caller-graph Python contract.
- **Finding:** the source contract still searched for the old inline `start 9>&-` command inside `opt-start.sh`. The production loop had intentionally moved into `optware_lifecycle_start_services()` in the shared helper, so the assertion raised `ValueError: substring not found` before the remaining caller graph assertions ran.
- **Correction:** assert lock acquisition → /opt mount recheck → shared startup supervisor → lifecycle lock release in `opt-start.sh`; separately require hook invocation with fd 9 closed inside the shared helper. No production code behavior changed.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** exact-head job logs inspected. Fresh storage and docs Actions are pending on THIS COMMIT; no claim of green current-head workflow yet.
- **Validation boundary:** L1 production logic unchanged from prior source review; L2 prior host regressions pass but the storage caller-graph contract has just been corrected and awaits exact-head rerun. No MIPS/uClibc production object, firmware/TRX or hardware validation.
- **Next:** reconcile the corrected storage/docs Actions on the resulting exact SHA. Continue analysis of the Optware retry contract: `try_start_stor_apps()` launches `opt-start.sh` in the background and clears `usb_opt_start` on successful launch without observing the child's exit status. Determine a non-blocking, lock-order-safe way to surface startup completion/failure and schedule retries before taking the separate `opt-mount.sh` MTD/UBI startup bypass.

---

## 2026-11-09 18:30 CST (UTC+8) — Correct Optware startup regression and handoff contract

- **Base main SHA:** `7005e6c48835b928d596f4a52f79ca9168a52994`; resulting commit: THIS COMMIT.
- **Recovered exact-head failures:** storage-hotplug [42600941876](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941876) passed the existing lock and teardown regressions, then failed the new startup fixture. Source review found the fixture's printf formats were double-escaped, leaving literal backslash-n text and a trailing event marker; normalize them to actual newline escapes. AI handoff docs [42600941917](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941917) failed because the live handoff used `## Active checkpoint` instead of the validator-required `## Active task`.
- **Correction:** correct all startup fixture printf escapes and retain the exact mandatory handoff heading. No production implementation change; the startup supervisor remains the behavior introduced by base commit `7005e6c`.
- **Validation:** corrected startup fixture and shared lifecycle helper `sh -n` and focused host run PASS in the scratch directory. The prior storage job's first three shell regressions were green; its final whole-caller-graph step was skipped after the startup fixture failed. The docs failure occurred at the heading contract before archive checks. New exact-head storage/docs Actions pending.
- **Changed paths:** `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `trunk/user/scripts/tests/optware_lifecycle_startup.sh`.
- **Validation boundary:** L1 source review unchanged; L2 focused host regression PASS, corrected exact-head Actions pending. No production MIPS/uClibc object, firmware/TRX or router validation.
- **Next:** reconcile corrected storage/docs Actions on the resulting exact SHA. If green, investigate the separate `opt-mount.sh` internal MTD/UBI direct service-start loop and its caller/lock context; preserve NFS ACL/permissions, Samba and vsftpd audits, DHCP callback semantics and FULLCONENAT namespace ownership as open work.

---

## 2026-11-09 18:26 CST (UTC+8) — Supervise Optware startup hooks and roll back partial starts

- **Base main SHA:** `8e818c475d3aa26fb4d92e255e25405c11f2a08f`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** NFS daemon start/reload [42598774070](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774070), USB storage hotplug [42598773937](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598773937), and AI handoff docs [42598774002](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774002) all completed SUCCESS on exact `8e818c475d3aa26fb4d92e255e25405c11f2a08f`. The NFS follow-up is therefore complete at source/CI level and is not repeated.
- **Finding:** `opt-start.sh` enumerated third-party `S??*` hooks through `ls` plus command substitution, then executed unquoted path words. Paths containing whitespace were split, hook exit statuses were discarded, startup proceeded after failure, and the final lock close could leave the script reporting success. A partially-started service set had no rollback.
- **Fix:** add `optware_lifecycle_start_services()` to the shared Optware lifecycle helper. It uses quoted pathname expansion, skips non-regular/non-executable entries, starts hooks in order, and on the first failure calls `stop` on that possibly partially-started hook and all prior successful hooks in reverse order. Rollback continues after individual stop failures and the helper remains failed. Each child closes fd 9 so external hooks cannot inherit the lifecycle lock. `opt-start.sh` now aborts before package-manager bootstrap when service startup fails, releases the lock and exits nonzero; normal completion explicitly exits zero after release.
- **Regression:** new `optware_lifecycle_startup.sh` tests ordered startup, filenames with spaces, non-executable filtering, closed lock-fd inheritance, empty/missing init directories, failure of a start hook, rollback order, and continued cleanup after a rollback stop failure. The storage-hotplug workflow runs this test and checks the source/caller contract on push and pull_request.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `trunk/user/optware/opt-start.sh`, `trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh`, new `trunk/user/scripts/tests/optware_lifecycle_startup.sh`.
- **Local validation:** `sh -n` passed for the changed helper, startup script and regression; the focused shell regression passed in a host scratch directory. The regression is not a target-filesystem or real third-party init-script integration test.
- **Exact-head Actions:** storage-hotplug and AI handoff documentation runs for THIS COMMIT are pending at commit creation; fetch their run IDs, conclusions and complete `head_sha` before treating L2 CI as green.
- **Validation boundary:** L1 source/caller review PASS; L2 focused host shell regression PASS, exact-head workflows pending. No MIPS/uClibc production object, firmware/TRX, live Optware package/service integration, storage contention stress or router hardware test.
- **Next:** reconcile storage/docs Actions against the new full main SHA. If green, audit the duplicate `opt-mount.sh` internal MTD/UBI startup loop (which still bypasses this supervisor), including lock ordering and failure propagation, without introducing a storage-lock/lifecycle-lock inversion. Keep NFS export ACL/permission mapping, Samba account/ACL/credentials, vsftpd authentication/path confinement, DHCP release/NAK semantics and FULLCONENAT namespace ownership open.

---

## 2026-11-09 18:06 CST (UTC+8) — Correct NFS start contract and roll back pre-daemon failures

- **Base main SHA:** 'a816159737241f398baa7f827d753f7b99e8131b'; resulting commit: THIS COMMIT.
- **Exact-head CI:** storage [42597871309](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871309), HTTPD [42597871334](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871334), Samba [42597871360](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871360), vsftpd [42597871344](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871344) and AI docs [42597871472](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871472) all SUCCESS on a816159. NFS [42597871349](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871349) passed shell syntax and every runtime mock; only its Python source contract failed because it expected numeric comparisons while the helper uses a case statement.
- **Correction:** align source contract with the explicit 0 / 1 / unknown case labels; do not change the production implementation to satisfy a stale assertion.
- **Implementation:** failed mounting either NFS control pseudo-filesystem and failed export publication now invoke checked func_stop cleanup. Tests verify successful rollback removes previously mounted control filesystems and etab; if export withdrawal fails, etab and pseudo-mounts remain as recovery evidence.
- **Changed paths:** .github/workflows/nfsd-start-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md, trunk/user/nfs-utils/nfsd.sh, trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh.
- **Validation:** the previous exact-head shell syntax and runtime mocks PASS; this follow-up's source-contract correction and added rollback assertions require fresh exact-head Actions. No local full checkout, docs validator, production MIPS/uClibc object, firmware/TRX, live NFS or router testing is claimed.
- **Validation boundary:** L1 source review PASS; L2 this follow-up pending exact-head NFS/HTTPD/storage/docs Actions. L3/L4 NOT VERIFIED.
- **Next:** reconcile exact-head Actions. If green, rotate to Optware opt-start.sh startup-hook enumeration, status aggregation, partial-start rollback, lock/fd ownership and retry semantics; keep NFS ACL/permissions and external mount lifecycle open.

---

## 2026-11-09 17:56 CST (UTC+8) — Fail closed on NFS start and reload errors

- **Base main SHA:** `d143581bf5f351cb2cf0d388f2542c3710802c7b`; resulting commit: THIS COMMIT.
- **Recovered CI:** HTTPD [42596369094](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369094) SUCCESS on exact base SHA d143581. The predecessor HTTPD run [42596117899](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117899) had failed on exact SHA `9dd2f4a6eed24103617023fc93ea058f51466a0a` because its NFS restart assertion still expected the old void signature; d143581 corrected the workflow contract to the int status/unknown-inventory guard. Storage [42596118456](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596118456), Samba [42596117948](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117948), vsftpd [42596117585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117585), WAN route [42596117615](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117615) and AI docs [42596117462](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117462) all SUCCESS on exact source SHA 9dd2f4a6. AI docs [42596369073](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369073) remained queued on d143581 at base inspection.
- **Finding:** `func_start()` ignored rpcbind, state-file/directory, module/mount and `exportfs -r` failures. The final `$?` check tested only `rpc.mountd`, so a failing `rpc.nfsd` could be masked by a later successful mountd command. A mountd failure could leave a partially started kernel server. The already-running and `func_reload()` branches could also report success when reload prerequisites were absent or export reload failed. C `run_nfsd()` and `reload_nfsd()` discarded `eval()` results.
- **Implementation:** check rpcbind start and process presence; refuse to reset export recovery files if only one NFS daemon is alive; check state creation, `/proc/filesystems`, NFS control and rpc_pipefs mounts, exports publication and each RPC start. A tri-state mount helper treats an unreadable `/proc/mounts` as an error instead of an absent mount. Verify both daemons exist after successful commands; roll back a partial kernel-server start with the existing checked `func_stop()` path and retain export evidence if cleanup fails. Keep max-block-size tuning best-effort. Reload now requires rpcbind, both daemons and `/etc/exports`, and returns the real `exportfs` failure. C start/reload wrappers log nonzero command results.
- **Regression:** add a mock-based shell test for rpcbind/module/mount/exportfs/RPC failures, process false-success, partial daemon state preservation, startup rollback, start ordering and reload prerequisites/results. Add a path-filtered NFS start/reload workflow covering both push and pull_request, shell syntax, runtime mocks and C caller source contracts.
- **Changed paths:** `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, new `trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh`, `trunk/user/nfs-utils/nfsd.sh`, `trunk/user/rc/src/services_stor.c`.
- **Validation:** source/caller order and failure gates reviewed against the exact base blobs. No full local repository checkout is available; no local docs-validator, full production object or MIPS build is claimed. Exact-head NFS, HTTPD, storage and docs Actions are pending for THIS COMMIT.
- **Validation boundary:** L1 focused source/caller review PASS; L2 exact-head regression and Actions pending. No production RC object/MIPS-uClibc, firmware/TRX, live NFS export/reload/rollback or physical-router test.
- **Next:** reconcile all exact-head NFS/HTTPD/storage/docs Actions. If green, rotate to Optware `opt-start.sh` service-hook enumeration and start-result aggregation; preserve NFS export ACL/permissions and external mount lifecycle as open audits.

---

## 2026-11-09 — Propagate NFS stop failures before storage detach

- **Base main SHA:** `656e9819f1de186767738a6d49aedbd171f26ff0`; resulting commit: THIS COMMIT.
- **Recovered baseline:** remote branches contained only `main`. HTTPD/security [42594767916](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767916) and AI handoff docs [42594767915](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767915) passed on exact SHA `656e9819f1de186767738a6d49aedbd171f26ff0`. Prior NFS mount-inventory/restart-preflight work is complete and not repeated.
- **Finding:** NFS stop previously ignored daemon termination, `exportfs -ua` and pseudo-filesystem unmount results. `etab` removal could erase recovery evidence after an incomplete stop. C `stop_nfsd()` dropped the shell result and storage eject continued to unmount the device.
- **Implementation:** wait for both `rpc.mountd` and `nfsd`, escalate only after a bounded graceful window, verify exit, check export withdrawal, and use tri-state `/proc/mounts` handling (absent / present / unreadable) for `rpc_pipefs` and `nfsd`. Retain `etab` on failure. Gate shell restart on successful stop; propagate return status through C stop/restart APIs and RC applets/notifications; abort per-device and eject-all removal before unmount while freeing staged data and releasing the storage lock.
- **Regression coverage:** storage-hotplug workflow now watches `nfsd.sh` and `rc.h`, runs `sh -n`, and checks shell/C/RC propagation plus both safe-eject abort paths and lock cleanup.
- **Changed paths:** `trunk/user/nfs-utils/nfsd.sh`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/rc.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** source transformations and regression-contract invariants checked against the exact base blobs before creating Git objects. No local checkout is available; no local shell syntax, compiler, Python workflow test or docs-validator run is claimed. Exact-head storage/docs Actions are pending.
- **Validation boundary:** L1 bounded source review PASS; L2 for this commit PENDING exact-head Actions. No production MIPS/uClibc object, firmware/TRX, live NFS export/stop, USB detach or physical router test.
- **Next:** reconcile exact-head storage/docs Actions. If green, audit NFS start/reload partial failure and cleanup, then rotate to arbitrary Optware/third-party service hooks and init ordering. Keep Samba account/ACL, vsftpd confinement, DHCP callback state and the separate FULLCONENAT kernel namespace audit open.

---
## 2026-11-09 17:35 CST (UTC+8) — Preserve NFS service when mount inventory is incomplete

- **Base main SHA:** `d5cd264e5fda1568d8c1872bac09c0a4ec0da233`; resulting commit: THIS COMMIT.
- **Module / priority:** NFS storage-service restart lifecycle, P2 stability. NFS and third-party add-ons remain PARTIAL / AUDITING.
- **Finding:** `restart_nfsd()` stopped the NFS daemon before calling `count_stor_mountpoint()`. If `/proc/mounts` open/read/close failed, the function returned on the unknown (`-1`) inventory, leaving the previously running NFS service stopped.
- **Fix:** add `storage_mount_inventory_is_complete()` to the tri-state mount-inventory policy and use it in the shared storage-service preflight. Move `stop_nfsd()` after the NFS restart's completeness guard. A failed inventory now preserves current daemon/module state; complete inventories continue through the existing restart/unload decision.
- **Regression:** extend the GNU89 policy test for incomplete, empty and non-empty inventory states. Strengthen the HTTPD workflow source contract to assert inventory read → incomplete guard/return → daemon stop ordering. Existing path filters already cover the source, helper and regression paths.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/storage_mount_inventory_policy.h`, `trunk/user/rc/src/tests/storage_mount_inventory_policy.c`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Baseline exact-head evidence:** HTTPD [42591187385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42591187385) and AI handoff docs [42591187335](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42591187335) SUCCESS on `d5cd264e5fda1568d8c1872bac09c0a4ec0da233`. This commit's host policy test, source contract and docs validation await exact-head Actions.
- **Local validation:** no local checkout is available; no local GNU89 test, workflow Python contract or `scripts/check_ai_handoff_docs.py` run is claimed.
- **Validation boundary:** L1 focused source/caller review PASS; L2 pending exact-head Actions. No production RC object, MIPS/uClibc, firmware/TRX or live NFS mount/restart/router validation. The separate `nfsd.sh` command-level start/stop/reload status audit and Optware startup-hook aggregation remain open.
- **Next:** reconcile HTTPD/docs Actions against the resulting full SHA. Then audit `trunk/user/nfs-utils/nfsd.sh` start/stop/reload error propagation as one bounded lifecycle task, followed by third-party `opt-start.sh` start-hook result handling.

---

## 2026-11-09 17:09 CST (UTC+8) — Bound Samba folder contract to the complete writer body

- **Base main SHA:** `301de504e474f51f4f04c08ef7f36975f19df9fa`; resulting commit: THIS COMMIT.
- **Exact-head results:** HTTPD [42590546450](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42590546450) FAILED at step 26, line 62 of its Samba trust-boundary Python contract, with `ValueError: substring not found`. Investigation showed the slice stopped at the first free_2_dimension_list(&sh_num, &folder_list); inside an early close-error branch, before the share output. The old check therefore looked for output outside its own truncated slice. Docs [42590546420](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42590546420) PASS. The C test remains unchanged and passed on exact SHA `088a87e61ca0da3ab040fc230f2d9e875b3920a1`.
- **Correction:** find the complete write_smb_conf() range, then bound the folder-loop contract from its marker through the final folder-list cleanup in that writer. This keeps the share-name/path guard-before-output assertions over the whole relevant block.
- **Changed paths:** `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** exact source and job-log inspection; the corrected HTTPD/docs workflows are pending on THIS COMMIT. No local checkout, local Python workflow run or docs-validator run is available/claimed.
- **Validation boundary:** no production C implementation changed. Samba GNU89 host test PASS on SHA 088a87e; HTTPD trust-boundary contract not green yet. No MIPS/uClibc, firmware/TRX, live SMB share/auth/ACL or hardware test.
- **Next:** reconcile HTTPD and docs Actions against the resulting full main SHA. If green, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; keep Samba account/ACL and FTP authentication/path-confinement audits open.

---

## 2026-11-09 17:04 CST (UTC+8) — Remove brittle Samba contract slice from HTTPD regression

- **Base main SHA:** `088a87e61ca0da3ab040fc230f2d9e875b3920a1`; resulting commit: THIS COMMIT.
- **Exact-head results:** Samba share stability [42589984483](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42589984483) and AI handoff docs [42589984462](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42589984462) both SUCCESS on the exact base SHA. HTTPD [42589984431](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42589984431) still FAILED at the Samba trust-boundary step: its `config_header` slice raised `ValueError: substring not found` after the function signature migration. Earlier HTTPD policy/regression steps passed.
- **Correction:** remove the unnecessary end-marker slice. Anchor at `write_smb_conf_header(FILE *fp)`, then compare the workgroup and hostname sanitization guards directly with their respective `fprintf()` calls. Preserve both checks and their ordering requirement.
- **Changed paths:** `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** root cause confirmed from exact-head job 129069003316 logs and current `services_ex.c`. No local repository checkout; no local execution of the Python contract or docs validator. Updated HTTPD and docs Actions are pending on THIS COMMIT.
- **Validation boundary:** no production C implementation changes. Samba GNU89 host regression PASS on exact SHA `088a87e61ca0da3ab040fc230f2d9e875b3920a1`; exact-head HTTPD contract is not yet green. No MIPS/uClibc, firmware/TRX, live SMB share or hardware validation.
- **Next:** reconcile HTTPD and AI handoff documentation Actions against the resulting full main SHA. If green, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; keep Samba account/ACL and FTP authentication/path-confinement audits open.

---

## 2026-11-09 16:53 CST (UTC+8) — Repair Samba exact-head regression and dependent HTTPD contract

- **Base main SHA:** `724b4c9982cba13c3c7274ba4744f88fb5799835`; resulting commit: THIS COMMIT.
- **Recovered failures:** exact-head Samba workflow [42587965407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965407) failed at GNU89 test compilation because `tests/samba_config_file.c:95` contained a double-escaped character constant (`'\\0'`). The exact-head HTTPD workflow [42587965159](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965159) failed in its Samba trust-boundary source contract: it still required the old `write_smb_conf_header(void)` signature and direct live-file append/close/return patterns after the implementation moved to a stream-based header and atomic staging.
- **Correction:** normalize the discard test's newline/NUL literals to valid single C escapes. Update HTTPD's Samba contract to require `write_smb_conf_header(FILE *fp)`, same-directory staging, staged-file appends/duplicate checks, checked close, discard on generation failure and publication only after complete generation; explicitly reject direct append to `SAMBA_CONF`. Preserve the existing permission/account/security assertions.
- **Other exact-head evidence:** vsftpd [42587965266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965266), storage [42587965191](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965191), WAN route [42587965174](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965174) and AI handoff docs [42587965152](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965152) all passed on the exact base SHA.
- **Changed paths:** `.github/workflows/httpd-security.yml`, `trunk/user/rc/src/tests/samba_config_file.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** root-cause review is based on the exact Actions job logs and current source blobs. No local repository checkout is available; local GNU89 test, workflow Python contracts and docs validator were NOT RUN in this round. Corrected Samba/HTTPD exact-head Actions are pending after this commit.
- **Validation boundary:** no new production C implementation was changed. The atomic Samba publisher remains L1-reviewed but its corrected regression still requires L2 confirmation. No production RC/Samba object, MIPS/uClibc build, firmware/TRX, live SMB share/auth/ACL or hardware test.
- **Next:** fetch live main/all branches and verify the corrected Samba and HTTPD Actions against the resulting exact SHA. If both pass, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; keep Samba account/ACL and FTP authentication/path-confinement audits open.

---

## 2026-11-09 — Stage and atomically publish generated Samba configuration

- **Base main SHA:** `c51c0f73fa2dccdd191515347acf80fd2a0132d2`; resulting source/test checkpoint: THIS COMMIT.
- **Module / priority:** Samba share configuration and service lifecycle, P2 stability with security implications. Samba remains PARTIAL / AUDITING.
- **Findings:** `write_smb_conf_header()` unlinked `SAMBA_CONF` before opening it and wrote the global section directly to the live file. `write_smb_conf()` subsequently wrote share sections to the same live file, closed/reopened it for duplicate-share checks, and could leave a partial/truncated config on later write or read errors. The folder-list reader's failure was treated as `continue`, silently omitting shares. `start_wins()` started nmbd even when header write/close failed; `restart_nmbd()` ignored `write_smb_conf()` failure and stopped nmbd before checking whether a replacement could be generated.
- **Fix:** add `samba_config_file.h` to stage a same-directory candidate with `mkstemp()`, mode 0644, checked stream/flush/close and atomic `rename()`. The active `smb.conf` is left intact on all pre-publication failures. Refactor `write_smb_conf_header(FILE *)` into a stream-only emitter; write all share definitions and duplicate checks against the staged file; match exact section names; treat folder inventory/read/append failures as fatal and clean up the account/folder lists; publish only after complete generation. `start_wins()` now publishes before nmbd launch. `restart_nmbd()` generates the new config before stopping nmbd, and skips both nmbd restart and smbd HUP if generation fails.
- **Regression:** add `tests/samba_config_file.c` for old-file visibility before rename, successful replacement/mode 0644, buffered `/dev/full` ENOSPC handling, rename failure cleanup, explicit discard, missing-parent failure and short-path-buffer failure. New `.github/workflows/samba-share-stability.yml` compiles with `-std=gnu89 -Wall -Wextra -Werror` and checks the generation, share scan, WINS and restart failure gates.
- **Changed paths:** `.github/workflows/samba-share-stability.yml`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/rc.h`, new `trunk/user/rc/src/samba_config_file.h`, new `trunk/user/rc/src/tests/samba_config_file.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Base exact-head Actions:** vsftpd [42586171489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171489), storage [42586171713](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171713), HTTPD [42586171537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171537) and AI handoff docs [42586171552](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171552) all completed SUCCESS with exact `head_sha=c51c0f73fa2dccdd191515347acf80fd2a0132d2`.
- **Local validation:** the new Samba config helper and host test were compiled using `cc -std=gnu89 -Wall -Wextra -Werror` and executed in a scratch directory: **PASS**. This exercised successful publication, prior-file preservation before rename, ENOSPC, rename failure, explicit discard and invalid-path cases. Source-contract assertions were evaluated against the staged source blobs. No complete local repository checkout was available, so the GitHub Python source contract and `scripts/check_ai_handoff_docs.py` were not run locally.
- **Exact-head Actions:** the new Samba share workflow and FTP/storage/HTTPD/docs workflow runs for THIS COMMIT are PENDING at commit creation; reconcile every run against the resulting full `head_sha` and inspect failed steps.
- **Validation boundary:** L1 targeted source/caller review PASS; L2 standalone GNU89 host helper test PASS, exact-head Actions pending. No production RC/Samba object build, MIPS/uClibc, firmware/TRX, real SMB mount/auth/ACL test, external add-on integration or physical-router validation.
- **Next:** verify all exact-head workflows, then rotate to NFS mount/permission lifecycle and third-party add-on service supervision. Keep Samba passdb/account and ACL audits plus broader FTP authentication/path-confinement work open; preserve DHCP callback semantics and FULLCONENAT namespace work as separate tasks.

---
## 2026-11-09 16:27 CST (UTC+8) — Correct vsftpd configuration error return

- **Base main SHA:** `2e27361f396adfe79017b1d815962524af47d097`; resulting commit: THIS COMMIT.
- **Review correction:** after the atomic-publication commit passed its exact-head vsftpd, storage, HTTPD and documentation workflows, a final source reread found that the new int-valued write_vsftpd_conf() used a bare return after a failed ftp_config_file_publish(). That is an invalid return contract and may trigger a compiler diagnostic.
- **Fix:** return -1 on publication failure; preserve return 0 only for a completed publication. Extend the Python source contract to require the publication failure return to precede the success return.
- **Prior exact-head evidence:** vsftpd 42585929021, storage 42585929025, HTTPD 42585929042 and AI handoff docs 42585929052 all completed SUCCESS on 2e27361f396adfe79017b1d815962524af47d097. THIS COMMIT's vsftpd and docs workflows are PENDING.
- **Validation boundary:** the previous commit's GNU89 test passed; this follow-up is a small RC source-return correction and CI contract addition. No full production object/MIPS/uClibc build or hardware testing.
- **Next:** reconcile THIS COMMIT's exact-head checks, then continue the Samba/accounts/NFS/add-on reliability rotation. The wider FTP credential/ACL/path-confinement audit remains open.

## 2026-11-09 16:25 CST (UTC+8) — Atomically publish generated vsftpd configuration

- **Base main SHA:** `85591fa3ea404728bc47a24c4d018148fa974295`; resulting commit: THIS COMMIT.
- **Module / priority:** vsftpd generated configuration and service startup, P2 stability with security implications; broader FTP audit remains AUDITING.
- **Finding:** write_vsftpd_conf() opened /etc/vsftpd.conf directly with "w", truncating the last working configuration before generation completed. It ignored all fprintf()/flush/close failures and returned no status; run_ftp() launched vsftpd regardless. A full/read-only filesystem or late write/close failure could leave the daemon reading a partial or empty configuration.
- **Fix:** add ftp_config_file.h to create a same-directory mkstemp() staging file, set its mode, and publish only after stream-error, flush and close checks succeed. A single rename() atomically replaces the destination; failure discards the staged file and retains the previous config. write_vsftpd_conf() now returns status and run_ftp() logs and skips daemon startup when publication fails.
- **Changed paths:** .github/workflows/vsftpd-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md, trunk/user/rc/src/services_stor.c, new trunk/user/rc/src/ftp_config_file.h, and new trunk/user/rc/src/tests/ftp_config_file.c.
- **Regression:** add GNU89 -Wall -Wextra -Werror host coverage for successful atomic replacement/file mode, buffered write failure injected with /dev/full, failed staging open, pathname truncation and preservation of last-known-good config. Workflow source contracts check publication ordering, required output/close checks, daemon-start gating and complete push/PR path filters. No local checkout was available, so the C test and Python source contract were not run locally; exact-head Actions are PENDING.
- **Validation boundary:** L1 selected caller/config control-flow review PASS. No production RC/vsftpd object build, MIPS/uClibc, firmware/TRX, real FTP login/transfer or hardware validation.
- **Next:** reconcile the vsftpd stability and AI handoff Actions on THIS COMMIT's exact full head_sha; then rotate to Samba/accounts/NFS/add-on service reliability/security. Keep the broader FTP credential/ACL/path audit open.

## 2026-11-09 — Extend USB storage hotplug caller-graph regression

- **Base main SHA:** `2b3a2d19d76399199d8d813fe96260b0a0910368`; resulting source/test checkpoint: THIS COMMIT.
- **Scope:** P2 storage stability audit follow-through. This is a CI/source-contract change; no production C or shell code was changed.
- **Reviewed graph:** SD/MMC mdev dispatch and per-device/shared operation locks; synchronous automount; automount mountpoint ownership lock and fd-8 child-close discipline; filesystem swap activation; Optware bind and swap setup; exact swap state and failure recheck; complete `/proc/swaps` + `/proc/mounts` snapshots before global cleanup side effects; swapoff-before-unmount; safe-removal and eject-all error gates; disk spindown only after successful unmount and a second mounted-source guard; deferred startup retry; RC mdev/notification/eject/firmware/shutdown entry points.
- **Regression:** expanded `.github/workflows/storage-hotplug-stability.yml` source contracts, added production/helper path filters for push and pull_request, and renamed the step to reflect whole caller-graph coverage. Existing Optware lock/teardown and storage C GNU89 host tests remain in the same workflow.
- **Review result:** source/caller and lock-order review completed. The new inline Python assertion block could not be run locally because this session has no repository checkout; its execution is delegated to the exact-head Actions workflow. Existing shell and C regressions had passed on the immediately preceding checkpoint `2b3a2d19d76399199d8d813fe96260b0a0910368`.
- **Actions rule:** query the storage-hotplug and AI handoff-documentation runs generated by THIS COMMIT and match their complete `head_sha`; never reuse results from the preceding checkpoint. Do not add a docs-only follow-up just to restate a green result.
- **Validation boundary:** no production RC/libdisk object build, MIPS/uClibc build, firmware/TRX, real removable-media stress, external Optware script integration, filesystem failure injection or physical-router testing.
- **Next:** rotate to `vsftpd` service/config/auth/path-confinement lifecycle audit and choose one bounded reliability/security defect. Keep DHCP leasefail/accepted-NAK/release semantics and FULLCONENAT namespace work separate.

---

## 2026-11-09 15:18 CST (UTC+8) — Propagate Optware stop-hook and /opt unmount failures

- **Base main SHA:** `70415484275e9c542be3d45b8bad807fb6a09591`; resulting source/test checkpoint: THIS COMMIT.
- **Module / priority:** USB storage / Optware safe teardown, P2 stability. Storage remains PARTIAL / AUDITING.
- **Finding:** `opt-umount.sh` ignored every executable `S??*` stop-hook result, ignored the second `umount /opt` result, and then released fd 9, potentially returning success despite incomplete teardown. The storage C callers already checked `eval()` status, but the shared macro's `_eval()` status contract was not enforced in the storage workflow.
- **Fix:** add shared helper functions that attempt all executable init scripts in reverse order and aggregate failures; empty init.d is valid, while failed script enumeration fails closed. Any stop failure releases the lifecycle lock and exits nonzero before swapoff/unmount. The `/opt` helper retries once and returns nonzero if both attempts fail; `opt-umount.sh` explicitly propagates failure after lock release. The nonblocking teardown try-lock and blocking startup lock remain unchanged.
- **Caller proof:** `eval()` in `trunk/user/shared/src/shutils.h` returns `_eval()`'s command status/errno. Both `services_stor.c` call sites check `hook_status` and skip the matching mount unmount on failure; no unnecessary C production edit was made.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `trunk/user/optware/opt-umount.sh`, `trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh`, and new `trunk/user/scripts/tests/optware_lifecycle_teardown.sh`.
- **Regression:** mock-based shell test covers reverse-order full traversal after a stop failure, all-success, missing and empty service directories, fail-closed inventory enumeration, immediate unmount success, retry success and two-attempt failure. Local `sh` regression: **PASS**. `sh -n` on the changed shell scripts: **PASS**. Storage workflow source contracts now verify both C callers gate their unmount on `eval()` status and the new production helper wiring.
- **Actions:** exact-head USB storage regression and AI handoff documentation contract are **PENDING** at commit creation; reconcile both run IDs, conclusions and full `head_sha` against the resulting `main` commit.
- **Validation boundary:** L1 selected source/caller review PASS; L2 focused host shell regression PASS locally, Actions pending. No production RC object, MIPS/uClibc, firmware/TRX, real Optware scripts, filesystem failure injection or physical-router validation.
- **Next:** reconcile exact-head storage/docs Actions, then finish the drafted whole in-tree storage caller-graph regression. After that bounded audit, rotate to another userland service area. Keep DHCP leasefail/NAK/release semantics and FULLCONENAT namespace work separate.

---
## 2026-11-09 15:04 CST (UTC+8) — Make AP-mode LAN DHCP bound updates transactional

- **Base main SHA:** `2f4942be4a1068e72982475166b178add129981c`; resulting commit: THIS COMMIT.
- **Module / priority:** LAN DHCP client and resolver lifecycle, P1/P2 stability; module remains PARTIAL / AUDITING.
- **Finding:** the bound/renew callback published temporary NVRAM lease identity before checking interface address changes, ignored default-route addition errors, and rewrote `/etc/resolv.conf` in place without the resolver lock or checked atomic publication. BusyBox's `udhcp_run_script()` synchronously waits for the callback but discards its exit status, so an error return alone cannot keep RC state and the kernel on the previous lease.
- **Fix:** add a host-testable DHCP bound transaction policy. It deletes a replaced old route, applies the address and new route, then atomically publishes the resolver before committing temporary lease fields. Any failure attempts all compensating operations, restores the previous address/route where they were actually changed, leaves temporary lease state untouched, and records the first rollback failure separately. Route-add `EEXIST` is treated as already-present without claiming ownership for rollback. Resolver generation now uses the shared `resolv` lock and `dnsmasq_file_open/publish` staging path.
- **Changed paths:** `trunk/user/rc/src/net_lan.c`, `trunk/user/rc/src/dhcp_lease_policy.h`, `trunk/user/rc/src/tests/dhcp_lease_policy.c`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Regression:** expanded GNU89 policy tests for successful transition, old-route deletion failure, interface clear/set failures, route-add failure, resolver publication failure, `EEXIST` ownership behavior, withdrawn gateway, and incomplete rollback. The HTTPD workflow compiles/runs the host regression and verifies the transaction/source wiring. Exact-head Actions are PENDING at commit creation.
- **Validation boundary:** this commit's host regression and source contracts require exact-head Actions confirmation. L1 source/caller review is complete for the bounded bound callback; L3 production RC object/MIPS-uClibc/firmware and L4 real DHCP renew/failure injection/router validation remain NOT VERIFIED. The BusyBox callback exit-status limitation remains intrinsic; this change protects RC/kernel state with rollback instead of relying on the callback status.
- **Next:** reconcile HTTPD/DHCP and AI handoff Actions against the exact implementation SHA; then resume Optware `opt-umount.sh` service-stop/second-`umount /opt` error propagation. Keep the storage caller-graph contract queued, then rotate to an unaudited user-space module.

---

## 2026-11-09 14:37 CST (UTC+8) — Simplify cross-session records and batch log rotation

- **Base main SHA:** `ae0ddfe000ca0ed92154f3b14b125d667a76832a`.
- **Governance correction:** replace per-entry 20-record rotation with a 120/100/20 batch policy. The root `dev_log.md` accumulates newest-first records from 20 through 119; when a new entry would create record 120, archive the oldest 100 complete entries verbatim in one batch and retain the newest 20. A 384 KiB target is an earlier size-based rotation trigger; 512 KiB is the hard limit.
- **Live-state files:** `SESSION_HANDOFF.md` remains one actionable checkpoint and `REFACTOR_ROADMAP.md` remains one current dashboard. Update each in place only when its own facts materially change. Git history is the version archive; routine Handoff/Roadmap snapshots are no longer created.
- **No mechanical triple-update:** update the log for a coherent milestone, Handoff only when the resumable checkpoint changes or before stopping, and Roadmap only when module status, evidence, inventory or priorities change. Do not create a docs-only follow-up commit just to write a completed CI result back into the files.
- **Historical preservation:** all existing dev-log, handoff and roadmap archive files/indexes remain unchanged and immutable. The validator continues checking their indexed paths, byte counts, heading counts and Git blob SHAs.
- **Changed paths:** `AI_BOOTSTRAP_PROMPT.md`, `AI_DEVELOPMENT_PROTOCOL.md`, `scripts/check_ai_handoff_docs.py`, `.github/workflows/ai-handoff-docs.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** reviewed the new count/size policy and the checker/workflow wiring. The local checker was not run because this session edited repository files through the GitHub API without a local checkout. Exact-head `AI handoff documentation contract` is triggered by this commit; inspect its run against the resulting SHA.
- **Resulting commit:** THIS COMMIT.
- **Next:** resume the existing DHCP bound-path transaction in `udhcpc_lan_bound()` / `lan_up_auto()` as recorded in the current handoff. Reconcile this documentation workflow from live Actions; do not make another docs-only commit solely to record its conclusion.

---

## 2026-11-09 14:27 CST — Atomically publish LAN DHCP deconfig resolver cleanup

- Repository: `Arlene206/padavan-4.4-2`. Base source/test head: `011b6fcf27129efcde3b948384c5b4d1907d0599`.
- Implementation: `d900e566cb990891551f4dfad8dffd2fd897548e` changes `lan_down_auto()` to stage an empty resolver file with `dnsmasq_file_open("/etc/resolv.conf.XXXXXX")` and publish it with `dnsmasq_file_publish()` while holding the existing `resolv` lock. This replaces truncate-in-place `fopen(DNS_RESOLV_CONF, "w+")`: failed staging, flush/write, close or rename does not expose a partial empty resolver file, and the DHCP last-known-good temporary lease identity is still reset only after route deletion, static interface configuration and resolver publication all succeed.
- GNU89 follow-up: `c42ce06cfc784e64bb71e44088423abc144e2cd6` moves the temporary pathname declaration to the start of the function's declaration block; no declaration-after-statement dependency is introduced.
- Regression: the existing DHCP GNU89 host policy test and HTTPD workflow source contract now verify the deconfig stage/publish order and reject direct resolver truncation. Exact-head HTTPD Authentication and Security Regression [42572938754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938754) SUCCESS and AI handoff documentation contract [42572938846](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938846) SUCCESS, both on `c42ce06cfc784e64bb71e44088423abc144e2cd6`.
- Verification boundary: L1 source and contract review PASS; L2 the existing DHCP pure-policy regression compiled/run under GNU89 with `-Wall -Wextra -Werror` and exact-head Actions PASS. The workflow does not compile the complete production `net_lan.c` translation unit. No production RC object, MIPS/uClibc, firmware/TRX, live DHCP failure injection or router hardware test.
- Next: finish the still-open bound transition in `udhcpc_lan_bound()` / `lan_up_auto()`: check address reset/configuration, default-route add and resolver publication before state/side-effect publication, and design retry-safe rollback while BusyBox still ignores DHCP callback exit status. Then audit Optware stop-hook/second-umount status propagation and add the drafted storage caller-graph regression.

---

## 2026-11-09 14:00 CST — Map BusyBox LAN DHCP callbacks and avoid Optware eject lock inversion

- Repository: Arlene206/padavan-4.4-2. Source/test HEAD: c2706ac972970bd325b3b9ebb9bd5311f49125f8. Current source commits include AP-mode DHCP lease validation (b96fa1e), fail-closed changed-gateway replacement (2332158), Optware lifecycle safe-eject integration (2c8b5ff), storage test correction (2a5911b), and the lock-order correction below.
- Optware lock-order correction: c2706ac972970bd325b3b9ebb9bd5311f49125f8 adds optware_lifecycle_lock_try_acquire() using flock -n. opt-umount.sh uses this nonblocking acquire because it runs below storage.lock; if opt-start.sh currently owns the lifecycle lock, teardown returns failure promptly and leaves storage attached for retry rather than waiting in reverse lock order. opt-start.sh retains the blocking lifecycle acquisition. Child service commands close fd 9 so they cannot inherit the lock.
- Regression: storage Actions now verify that a competing teardown immediately fails to acquire while startup owns the lifecycle lock, while a normal blocking acquisition waits until release. Exact-head USB storage run 42570160275 SUCCESS and AI handoff documentation contract 42570160219 SUCCESS, both on c2706ac972970bd325b3b9ebb9bd5311f49125f8.
- Remaining Optware issue: opt-umount.sh currently does not check each S??* service stop status and does not check the status of the second umount /opt attempt; a final lock-release command can mask those earlier failures. Inspect and correct hook status propagation before considering safe eject complete. External init scripts and their behavior remain user-supplied and outside repository review.
- BusyBox DHCP callback contract: trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c udhcp_run_script() is void; it calls spawn_and_wait(argv) synchronously but discards child exit status. Therefore callback return values do not steer BusyBox's lease state machine. leasefail runs after discovery/request retry exhaustion; accepted NAK invokes nak, then deconfig only if previous state != REQUESTING; deconfig also follows lease expiry, ARP-conflicting offer (outside REQUESTING), repeated SIGUSR1 during RENEW_REQUESTED, and SIGUSR2/release or SIGTERM with release-on-quit.
- DHCP failure state: route_del() in rc/src/net.c returns 0 or positive errno; ifconfig() in shared/src/netutils.c returns 0, positive errno on ioctl error, and -1 on socket/name-copy failure. udhcpc_lan_deconfig() ignores static-address ifconfig and lan_down_auto results. lan_down_auto() ignores route_del and resolver fopen/fclose, then calls update_lan_status(0), which can erase the only temp gateway identity after a failed route delete. Bound path publishes new temp lease fields before ignoring ifconfig / route_add / resolver write-close failures; treat bound/deconfig as one transaction review.
- No new DHCP production code change in this audit. Existing 23321588 change only protects changed-gateway deletion before bound/renew publication; L3/L4 remain unverified.
- Next: design DHCP bound/deconfig internal state handling around BusyBox ignoring script exit status. Preserve old route identity across failed deletion; define interface fallback semantics and atomic/checked resolver publication; add GNU89 helper regression and exact workflow assertions. Then fix opt-umount service-stop/second-umount error propagation, finish the drafted storage caller-graph CI contract, and rotate to another userland module.

---

## 2026-11-09 13:42 CST — Serialize Optware startup and teardown with safe storage removal

- Repository: Arlene206/padavan-4.4-2. DHCP work: b96fa1ee123d45bd15d2e90ff6e4a183ecda0fd0 hardened AP-mode bound/renew lease validation; 23321588f38c3ea9a28355ce7ccc50feb06ef1d8 moved changed-gateway deletion ahead of lease publication, accepts an already-absent ESRCH route and fails closed on other route-delete errors.
- DHCP exact-head regressions: HTTPD Authentication and Security Regression [42568560276](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568560276) SUCCESS and AI handoff docs [42568560354](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568560354) SUCCESS on 23321588f38c3ea9a28355ce7ccc50feb06ef1d8. The former b96fa1e HTTPD failure was a stale validator locator after extraction into dnsmasq_domain_policy.h; fixed in 23321588.
- Optware/safe-eject implementation: 2c8b5ff9797cbb6a0abf99c5fa18210d09df6671 adds /sbin/optware_lifecycle_lock.sh and takes its process-shared flock in opt-start.sh and opt-umount.sh. Storage unmount/ejected cleanup now treats a failing Optware stop hook as a reason not to detach the affected mount; eject-all avoids its generic fallback after a failed per-device pass. Deferred storage-app startup acquires storage.lock and preserves pending hotplug flags / schedules retry if the operation lock cannot be acquired.
- Regression correction: first exact-head storage Actions [42568701858](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701858) failed because the new shell test called the lock function before sourcing its helper. 2a5911b24548ac2a1ef06442e0b61771a2714733 adds the missing source statement; storage hotplug Actions [42568851009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568851009) then completed SUCCESS on exact HEAD. HTTPD/security [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) passed on 2c8b5ff; AI docs [42568850779](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568850779) passed on 2a5911b before this documentation refresh.
- Storage caller review: traced mdev SD/MMC rules, per-device/shared locks, synchronous _eval/automount, swap/mount/unmount, safe-eject aliases, USB wrapper, PID 1 cleanup and libdisk range/eject-all flow. No direct bypass found in the inspected in-tree call graph. The separate mountpoint flock is not the operation lock. A drafted whole-callgraph Python contract remains uncommitted; current storage Actions cover the Optware lifecycle lock and existing storage regressions, not that uncommitted draft.
- Limits / residuals: Optware init scripts are user-supplied and absent from this repository. Review lock-order interactions if a custom init script invokes storage eject/mount operations while holding the Optware lifecycle lock. Check whether opt-umount stop/unmount failures are surfaced sufficiently; the caller's physical umount still serves as the final busy-device gate. No package-wide external-hook audit or runtime claim.
- Validation boundary: exact-head storage Actions and current pre-refresh docs Actions PASS as described. No MIPS/uClibc production object build, firmware/TRX, live DHCP lease churn, removable-disk churn or physical hardware test was run.
- Next: continue LAN DHCP deconfig / leasefail / NAK semantics, trace BusyBox callback producers and route/interface/resolver failure propagation while preserving last-known-good lease state. Then review Optware hook status/lock-order edge cases and complete the drafted storage caller-graph CI contract; after these bounded items rotate to a different userland module.

---

## 2026-11-09 13:04 CST — Serialize storage hotplug and safe-removal operations

- **Base main SHA:** `b0eecff6e22d056ea144595fa69ceeac1bf1aa83`.
- **Implementation commit:** `5d5b61cfc9d5dd3bca237ff4be31eb0118da1dfb` — introduce a process-shared `storage` fcntl lock around SD/MMC hotplug add/remove, manual safe removal, eject-all and PID 1 ejected-device cleanup. Preserve per-device locks and acquire the shared lock after them; release in reverse order.
- **Regression correction commit:** `cb70977f05b3d7d64dcabf5c02247bda05969f35` — correct the storage policy include terminator and refine the source contract to distinguish the early per-device-lock error release from the normal reverse-order cleanup.
- **Changed paths:** `trunk/user/rc/src/hotplug_stor.c`, `trunk/user/rc/src/services_stor.c`, new `trunk/user/rc/src/storage_operation_lock.h` and `trunk/user/rc/src/tests/storage_operation_lock.c`, `.github/workflows/storage-hotplug-stability.yml`, `.github/workflows/httpd-security.yml`.
- **Finding:** `mdev_sd_main()` / `mdev_mmc_main()` used different `/var/lock/<device>.lock` identities for a disk and its partitions (for example `sda` versus `sda1`); `safe_remove_stor_device()`, eject-all and PID 1 ejected cleanup had no common serialization. Concurrent add/remove and eject flows could inspect and mutate storage topology concurrently. The shared lock uses the existing `file_lock()` ABI. Its same-process re-entry guard makes a nested public `umount_ejected()` unsafe, so the operation body is now `umount_ejected_locked()` behind a locking public wrapper; safe removal invokes the locked helper while already holding the lock.
- **Validation:** exact-head USB storage Actions [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868), HTTPD/security Actions [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905), and AI handoff documentation contract [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843) all **SUCCESS** on `cb70977`. The storage job compiled and ran the new GNU89 `-Wall -Wextra -Werror` fork-based lock regression against the repository's `bin_sem_asus.c`; the competing process was blocked for 2,000 ms. HTTPD/security completed all 39 validation steps. The first implementation attempt [42564591289](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564591289) and [42564591242](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564591242) failed on a source-contract ordering assumption and a malformed include; both were corrected before the successful exact-head reruns. The documentation-only snapshot `c2020331b891ed07dd01f729136a5b728fff99f2` was also checked by AI handoff documentation contract [42565709754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42565709754), which passed on that exact SHA.
- **Validation boundary / residuals:** no full RC production object, MIPS/uClibc build, firmware/TRX build, live filesystem test or physical router test. The existing `automount.sh` mountpoint flock remains a separate label/mountpoint-ownership guard; direct shell-side mount/unmount/swap entrypoints and all callers still need auditing to prove they cannot bypass the operation lock.
- **Next:** inventory every direct caller of `/sbin/automount.sh`, mount/unmount and swapon/swapoff paths; verify which callers already execute beneath the new `storage` lock and identify any bypass without creating nested-lock deadlocks. Then continue disk/partition notification ordering and per-device lock scope. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 security/stability task.

---

## 2026-11-09 12:31 CST — Record green storage eject and repository security regressions

- **Source change:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — stage a complete partition inventory before ejected-device unmounts and propagate cleanup errors into PID 1 notification completion.
- **Contract synchronization:** `82096fbab795598bc6eb4384003e8f66998d1997` — update both legacy HTTPD safe-eject assertion blocks to the single-snapshot design and correct archive-index byte counts for the handoff/roadmap snapshots.
- **Exact-head Actions:** USB storage hotplug regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) **SUCCESS** on source head `7fee5c1`; HTTPD Authentication and Security Regression [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008) **SUCCESS** on `82096fb`; AI handoff documentation contract [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077) **SUCCESS** on `82096fb`. The HTTPD job completed all 39 validation steps successfully. The earlier failed attempts [42561802353](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802353) and [42561802333](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802333) were corrected and superseded by these green exact-head runs.
- **Validation boundary:** L1 source/control-flow review and L2 focused GNU89 helper regression plus whole-repository HTTPD/source/documentation contracts PASS. This does not constitute a production MIPS/uClibc build, firmware/TRX build, filesystem integration test or physical router test.
- **Next:** verify the documentation contract triggered by the final handoff snapshot commit, then continue the storage caller/concurrency audit (disk add/remove ordering, safe-remove callers and lock identity). USB/storage remains PARTIAL / AUDITING.

---

## 2026-11-09 12:27 CST — Correct staged eject CI contracts and archive index byte counts

- **Base main SHA:** `fdd00a4893d40225105bb7521691dbd2621351fd`.
- **Workflow correction:** THIS COMMIT — update the second legacy safe-eject contract block to check the operation-wide partition snapshot, instead of the removed per-mount `procpt2` scan.
- **Documentation correction:** the prior archive-index update recorded the superseded handoff and roadmap lengths as 15 bytes because the index generator measured the wrapper object rather than its `content`. The original Git blob references and archived bytes were preserved; the indexed sizes are corrected here to 6,377 and 8,180 bytes.
- **Exact-head CI at the base:** HTTPD regression [42561802353](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802353) failed at the second, still-stale `ejected_body` assertion block; AI handoff contract [42561802333](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802333) failed on the two incorrect archive byte counts. The focused USB hotplug workflow [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) passed on code head `7fee5c1fc589dc98c3333dd4e052d28881b8756a`.
- **Validation boundary:** implementation regression remains L2 PASS on its exact source SHA; this follow-up corrects CI/index metadata and awaits exact-head reruns. No MIPS/uClibc firmware or physical USB validation is claimed.
- **Next:** verify HTTPD security and AI handoff contracts on the exact resulting main SHA. Once green, resume the storage add/remove concurrency and per-device lock audit.

---

## 2026-11-09 12:20 CST — Align the storage CI contract with staged eject inventory

- **Base main SHA:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a`.
- **Workflow correction:** THIS COMMIT — migrate the legacy `httpd-security.yml` safe-eject contract from the removed per-mount `procpt2` scan to the operation-wide staged partition snapshot.
- **Exact-head CI:** storage hotplug regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) and AI handoff documentation contract [42561240402](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240402) both passed on `7fee5c1fc589dc98c3333dd4e052d28881b8756a`. HTTPD security regression [42561240397](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240397) failed only at its legacy source assertion requiring `procpt2`/per-mount `/proc/partitions` reads; preceding compile/regression and unrelated source checks passed. The assertion is updated here to require one staged inventory before unmount and no per-mount rescan.
- **Validation boundary:** implementation and focused GNU89 policy test are L2 PASS on the prior exact code head; the updated HTTPD workflow and refreshed documentation require exact-head Actions verification. L3 MIPS/uClibc and L4 physical-device validation remain NOT VERIFIED.
- **Next:** check all Actions for the exact resulting `main` SHA; then continue storage hotplug concurrency audit from add/remove event ordering and per-device lock scope. Do not treat the contract correction as module completion.

---

## 2026-11-09 12:10 CST — Stage storage eject inventories and propagate cleanup failures

- **Base main SHA:** `692b7b2372dd4ccabab879a53ddcd78aad69e751`.
- **Implementation:** THIS COMMIT — stage one complete `/proc/partitions` snapshot before any ejected-device unmount; propagate `umount_ejected()` failure into PID 1 notification completion.
- **Finding:** `umount_ejected()` already staged all of `/proc/mounts`, but reopened and reparsed `/proc/partitions` for each mount. A later partition-inventory read/parse/close failure could therefore be discovered only after earlier mountpoints had already been unmounted. The `on_unplug_mass_storage` handler also discarded the cleanup return value, causing the queue to mark a failed operation as completed.
- **Change:** parse and validate every partition row once, including the procfs heading, exact field count, numeric major/minor/block fields, newline termination, read/close status and allocation outcomes. On any preflight failure, leave all staged mountpoints untouched. Compare the staged names during cleanup instead of reopening procfs. PID 1 now retains the incomplete notification marker when safe-eject cleanup fails; the failure log is generic for all notification types.
- **Regression:** extend `storage_ejected_inventory_policy.c` for valid/header/blank/malformed/truncated/extra-field partition records; add `storage-hotplug-stability.yml` to compile/run the GNU89 host test and assert preflight ordering, single partition scan and notification error propagation. Exact-head execution is pending.
- **Validation boundary:** L1 source/caller review PASS. L2 host regression / Actions PENDING on THIS COMMIT. No production MIPS/uClibc RC build, full firmware image or physical USB/filesystem test is claimed.
- **Next:** verify the storage-hotplug workflow and AI documentation contract on the exact resulting `main` SHA, then continue the caller/concurrency audit across `mdev_sd_main()`, `mdev_mmc_main()`, `safe_remove_usb_device()`, eject commands and per-device lock identity. USB/storage remains PARTIAL / AUDITING.

---

## 2026-11-09 12:05 CST (UTC+8) — Reconcile withdrawn DHCP router options during renewal

- **Base main SHA:** `d96f8df72fe36afcd7875d30ca70228151085a83`; **implementation commit:** `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`.
- **Repository:** maintained fork `Arlene206/padavan-4.4-2`.
- **Finding:** BusyBox udhcpc exports only DHCP options present in the ACK. A server that withdraws option 3 (Router) on renewal leaves `getenv("router") == NULL`. The old WAN and IPTV renew callbacks guarded router comparison with `if (value)`, so neither reconciled a withdrawn gateway and the old default route could survive.
- **Fix:** add `dhcp_lease_router_is_changed()` to `dhcp_lease_policy.h`, normalizing missing/empty router options to an empty gateway; invoke it unconditionally from WAN and IPTV renew callbacks in `net_wan.c`. A changed/withdrawn router enters the existing bound path, which removes old lease routes and clears the gateway. Add host cases for absent, empty, unchanged, changed and invalid options; extend workflow source contracts for both callers.
- **Exact-head verification:** [AI handoff contract 42559613261](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613261), [WAN teardown 42559613131](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613131), [HTTPD/DHCP regression 42559613119](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613119) and [WAN route status 42559612983](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559612983) all SUCCESS on exact implementation SHA `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`. The HTTPD job's DHCP policy test and normalization/renew source-contract step both reported OK.
- **Verification boundary:** L1/L2 PASS for this scoped path. MIPS/uClibc RC/full firmware (L3) and router DHCP withdrawal/route behavior (L4) NOT VERIFIED.
- **Next:** verify the AI handoff contract on the new governance/archive commit's exact SHA, then rotate to USB/storage hotplug caller/concurrency audit. Keep DHCP LAN and deconfig/leasefail/NAK/release coverage open.

---

## 2026-11-09 12:04 CST (UTC+8) — Restore canonical roadmap archive shard and repair archive inventory

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Recovery:** the prior handoff referenced `docs/roadmap-history/2026/11/part-008.md`, but that canonical path was absent. Recovered the exact Git blob `622163bfb030d1d7fbdf7e4bc844a14ace7df1d0` from its misplaced nested copy and restored the standard indexed path in commit `8bcde43b62bd8c2116a2023ee48c3cdc04ee61a3`.
- **Correction:** the misplaced nested duplicate was not indexed and caused the follow-up archive inventory check to fail. Removed only `docs/roadmap-history/archive/2026/11/part-008.md` in commit `d96f8df72fe36afcd7875d30ca70228151085a83`; the canonical standard-path archive remained intact and byte-identical.
- **Verification:** [AI handoff contract 42559250321](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559250321) SUCCESS on exact correction SHA `d96f8df72fe36afcd7875d30ca70228151085a83`; log confirms `root log=20 entries; archive shards: dev-log=5, handoff=9, roadmap=8`.
- **Next:** preserve the archive/index invariant during this checkpoint rotation; always compare archive inventory with index rows, not only blob contents.

---

## 2026-11-09 11:40 CST (UTC+8) — Restore required handoff verification heading after contract failure

- **Base main SHA:** `0810b15526a7c6c7b7ba5980679c62fd522f481a`; resulting documentation-only commit: THIS COMMIT.
- **Wireless code regression:** [42557455460](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455460) SUCCESS on exact code SHA `0810b15526a7c6c7b7ba5980679c62fd522f481a`; the workflow compiled and ran the GNU89 policy test and passed the wireless source contracts for iNIC recovery, interface-down aggregation, MII teardown and LED publication. Dependent HTTPD regression [42557455207](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455207) also SUCCESS on that exact SHA.
- **Documentation contract finding:** [42557455144](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455144) FAILED on `0810b15526a7c6c7b7ba5980679c62fd522f481a` because `SESSION_HANDOFF.md` used `## Validation boundary`; `scripts/check_ai_handoff_docs.py` requires the literal heading `## Verification boundary`. No archive/hash/count error was reached in that run.
- **Correction:** restore the required heading without changing the substantive checkpoint; preserve the superseded handoff and roadmap byte-for-byte, update indexes, and rotate the oldest complete root log record to the active shard.
- **Validation:** wireless L2 Actions PASS on exact code SHA; docs L2 correction is PENDING until a fresh exact-head contract run. No target RC/MIPS firmware or hardware check.
- **Remaining:** current docs run must pass on the resulting main SHA. Wireless driver/switch consumers and board-specific runtime remain open; FULLCONENAT mapping namespace scope remains a distinct P2 audit.
- **Next:** verify the fresh AI handoff documentation contract on exact live main. Then resume DNS/DHCP client lifecycle error handling.
- **Result:** THIS COMMIT.

---

## 2026-11-09 11:21 CST (UTC+8) — Fix wireless interface-down result handling and LED state publication

- **Base main SHA:** `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6`; resulting source/documentation commit: THIS COMMIT.
- **Module / priority:** wireless management and RT3352 iNIC lifecycle, P2 stability. Whole-module audit remains open.
- **Findings:** `check_inic_mii_rebooted()` discarded the return from the periodic iNIC guest-interface-up command, so a failed recovery was indistinguishable from success. `inicd.c` invokes the check from its SIGALRM timer handler, making this an opportunity for an explicit retry on the next health-check tick. `stop_wifi_all_wl()` and `stop_wifi_all_rt()` ignored every interface-down result and switched the radio LED off even if a netdevice remained up. The RT3352 MII startup path brings up `IFNAME_INIC_MAIN` and optionally `IFNAME_INIC_GUEST`, but the old stop path attempted the non-MII 2.4GHz APCLI/WDS/AP names instead of tearing down those iNIC interfaces.
- **Fix:** route the health-check guest-up through `wif_control_success()` and explicitly defer recovery to the next periodic check on failure. Add a sticky shutdown-result policy; every supported interface-down is attempted and checked, a later success cannot erase an earlier failure, and the LED is switched off only if all required shutdown operations succeeded. The MII path now stops the actual iNIC guest/main interfaces, removes guest VLAN bridge membership, and reports isolate/RGMII-disable failures without short-circuiting cleanup.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `trunk/user/rc/src/wireless_radio_policy.h`; `trunk/user/rc/src/tests/wireless_radio_policy.c`; `.github/workflows/wireless-radio-state.yml`; compact root handoff/roadmap/log plus lossless archives/indexes.
- **Regression:** extended GNU89 host policy test with sticky shutdown-failure aggregation; added source contracts for periodic iNIC recovery status, every stop attempt, MII teardown and LED-off ordering. Exact-head wireless and documentation Actions are PENDING at commit creation.
- **Validation boundary:** L1 source/caller review PASS for the scoped health-check and shutdown paths. L2 host compile/test and Actions PENDING until the new exact-head run completes. No MIPS/uClibc RC build, complete firmware/TRX, live radio restart or hardware test claimed.
- **Remaining scope:** driver/switch consumers, other board-conditional lifecycle paths and concurrent restart/stop behavior remain open. Firewall/NAT remains AUDITING; FULLCONENAT mapping hashes and the dying-tuple queue are still module-global and not namespace-keyed.
- **Next:** reconcile wireless and AI handoff documentation workflows against exact live main SHA. Once green, rotate to DNS/DHCP client lifecycle error handling; retain FULLCONENAT mapping namespace isolation as a separate P2 audit.
- **Result:** THIS COMMIT.

---

## 2026-11-09 11:17 CST (UTC+8) — Confirm FULLCONENAT per-net notifier regression and handoff archive CI

- **Base main SHA:** `91cae3d19bb72a9e0cebcad509db256aded52467`; this documentation-only follow-up records the exact-head results for the code commit.
- **FULLCONENAT lifecycle regression:** [run 42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213), SUCCESS on exact source commit `91cae3d19bb72a9e0cebcad509db256aded52467`. Its updated step, “Verify FULLCONENAT per-net notifier ownership and cleanup”, passed the per-net registry/refcount, both notifier API layouts, registration/unwind, owning-net unregister, RCU grace period, IPv4/IPv6 rule destructor/module reference order and two-namespace lifecycle assertions.
- **AI handoff documentation contract:** [run 42555491302](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555491302), SUCCESS on exact source commit `91cae3d19bb72a9e0cebcad509db256aded52467`. The executed `scripts/check_ai_handoff_docs.py` reported: `PASS (root log=20 entries; archive shards: dev-log=4, handoff=6, roadmap=5)`.
- **Code commit:** `91cae3d19bb72a9e0cebcad509db256aded52467` — `fix(netfilter): scope FULLCONENAT notifiers per netns`.
- **Validation boundary:** L1 source review PASS; L2 focused source-contract and documentation-archive Actions PASS on the exact code commit. These workflows do not compile the kernel module or execute conntrack/NAT in a live namespace. No local kernel object, MIPS/uClibc, full firmware/TRX or hardware test is claimed.
- **Open risks:** Firewall/NAT stays AUDITING. FULLCONENAT IPv4/IPv6 mapping tables and its dying-tuple queue remain module-global and are not netns-keyed; IPv4/IPv6 writer parity, rollback/runtime semantics and complete module closure remain open.
- **Actions on this follow-up:** the documentation-only commit will trigger a fresh AI handoff contract run. The next handoff must reconcile that run against the resulting live `main` SHA; the FULLCONENAT code regression above remains tied to the unchanged source commit `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **Next:** after the new documentation contract passes on exact live main, rotate to Wireless: inspect `check_inic_mii_rebooted()` guest-interface-up handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication gates. Keep FULLCONENAT mapping namespace scope as an explicit later audit.

---

## 2026-11-09 11:11 CST (UTC+8) — Scope FULLCONENAT conntrack notifiers to their owning network namespace

- **Base main SHA:** `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.
- **Module / priority:** Linux Netfilter `xt_FULLCONENAT`; P2 conntrack event lifecycle and namespace-cleanup reliability. Firewall/NAT remains **AUDITING**.
- **Recovered task:** resumed the exact handoff's remaining audit of the module-global notifier/reference count against Linux 4.4's per-`struct net` conntrack notifier API. Previous FULLCONENAT GC workqueue/registration-error handling was not repeated.
- **Confirmed defect:** `include/net/netns/conntrack.h` stores `nf_conntrack_event_cb` / `nf_conntrack_chain` inside each namespace. A single global `tg_refer_count` registered the callback only in the first namespace; a last-rule teardown in a second namespace then unregistered against the wrong `par->net`. In the legacy event API this can trigger `BUG_ON(notify != new)`; in chain-event builds it targets the wrong notifier chain. Destroy events for other namespaces were also not delivered to FULLCONENAT's GC.
- **Fix:** `xt_FULLCONENAT.c` now maintains one notifier object and reference count per namespace. Each namespace registers independently; same-net rules share the node; the last destructor unregisters from the exact stored owner. Allocation, registration and overflow failures release the `nf_ct_netns_get()` reference; legacy callback storage is freed only after `synchronize_rcu()`. Added explicit slab/RCU headers and kept declarations GNU89-compatible.
- **Teardown review:** IPv4/IPv6 iptables cleanup passes its owning `struct net` into the target destructor and performs `module_put()` only after the destructor returns. The regression contract was extended to cover this ordering, both notifier API variants, registration/unwind, per-net refcounts and independent namespace lifecycles.
- **Changed paths:** `trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c`; `.github/workflows/fullconenat-lifecycle.yml`; the workflow also reads `include/net/netns/conntrack.h`, IPv4 `ip_tables.c`, IPv6 `ip6_tables.c`, conntrack event provider and existing IPv6 restore rollback sources.
- **Validation:** L1 source review PASS for notifier API ownership, error unwind, target teardown and RCU lifetime. The expanded workflow is a source-contract/model regression only; exact-head Actions are **PENDING**. No local kernel object/module build or standalone host executable was run.
- **Actions:** FULLCONENAT lifecycle regression and AI handoff documentation contract are PENDING until their run IDs, conclusions and `head_sha` are reconciled against the resulting main commit.
- **Remaining risks:** FULLCONENAT mapping hashes and the dying-tuple queue remain module-global and are not keyed by namespace; this round fixes notifier ownership only. IPv4/IPv6 writer parity, complete NAT rollback/runtime semantics, MIPS/uClibc, full firmware/TRX and hardware are not verified.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify both Actions workflows on exact live main. If green, rotate to Wireless residuals at `check_inic_mii_rebooted()` guest-interface-up handling and `stop_wifi_all_wl/rt()` interface-down/LED publication; retain FULLCONENAT mapping namespace scope as an explicit open audit.

---

## 2026-11-09 11:08 CST (UTC+8) — Confirm exact-head route, teardown, HTTPD and handoff CI

- **Base main SHA:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`.
- **Route code CI:** WAN route operation status regression [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS on exact implementation SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer helper regression and route source/caller contract passed.
- **WAN teardown CI:** [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on that same implementation SHA.
- **Documentation at route implementation SHA:** [42552840438](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840438) SUCCESS.
- **Cross-workflow stale assertion:** HTTPD run 42552840406 first failed because an existing DHCP lease source contract still expected a void route-delete wrapper and an `if (is_add)` branch. Its DHCP lease C regression passed. The contract was updated in `.github/workflows/httpd-security.yml`; corrected HTTPD run [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) and documentation run [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) both SUCCESS on exact SHA `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; all HTTPD workflow steps passed.
- **Handoff:** root status records both the route implementation and the cross-workflow contract correction; the prior handoff/dashboard are archived byte-identically. New root log is still the newest 20 records.
- **Validation boundary:** L1 source review PASS; L2 helper sanitizer, route workflow, WAN teardown, corrected HTTPD and pre-final docs workflow PASS. The final documentation-only commit's handoff-contract run must be reconciled against its exact HEAD. Production RC object, MIPS/uClibc, complete firmware/TRX, live route failure injection and hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify the final handoff-contract Actions run against exact main, then start the bounded FULLCONENAT namespace/notifier ownership audit. Do not repeat the route status fix.

---

## 2026-11-09 10:47 CST (UTC+8) — Reconcile WAN route Actions and correct dependent HTTPD contract

- **Base main SHA:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Route implementation CI:** [WAN route operation status regression 42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) completed SUCCESS on exact `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer regression and source/caller contract both passed.
- **Additional exact-head checks:** WAN interface teardown run 42552840447 and AI handoff documentation run 42552840438 both completed SUCCESS on exact `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Recovered cross-workflow failure:** HTTPD run 42552840406 failed in its existing DHCP lease source-contract Python step after the route API changed from `void` to `int` and from a literal `if (is_add)` branch to the equivalent checked ternary. The DHCP lease C policy regression itself passed. This was a stale workflow expectation, not a production compile or runtime finding.
- **Correction:** `.github/workflows/httpd-security.yml` now checks the int-returning DHCP delete wrapper/prototype and ternary add/delete selection. The corrected workflow is included in this follow-up commit; rerun on its exact HEAD is PENDING.
- **Documentation:** handoff/roadmap now record the exact run outcomes and next Firewall/NAT task. Previous root checkpoint/dashboard are preserved byte-identically in indexed archives; root log remains a 20-entry window.
- **Validation boundary:** route host sanitizer regression and exact-head route Actions PASS; exact-head WAN teardown and documentation-contract Actions PASS. Corrected HTTPD Actions PENDING. No production RC object, MIPS/uClibc, firmware/TRX or hardware test.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify corrected HTTPD and AI handoff workflows against exact main HEAD, then begin FULLCONENAT per-net notifier/reference-count lifecycle audit.

---

## 2026-11-09 10:34 CST (UTC+8) — Propagate static and DHCP route operation failures

- **Base main SHA:** \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Finding:** \`control_static_routes()\` discarded every \`route_add()\` / \`route_del()\` status and returned success; DHCP route setup/removal used a void helper and discarded legacy and RFC3442/Microsoft route failures. DHCP-list allocation failures were also hidden.
- **Fix:** static and DHCP helpers now return the first nonzero operation status while continuing to attempt every remaining valid route. Both DHCP option lists are handled independently, so one allocation failure does not suppress processing of the other. Public DHCP wrappers now return status; every failed route and allocation is logged. Route deletion remains available when dynamic-route addition is disabled, and WAN/MAN cleanup ordering is preserved.
- **Changed paths:** \`trunk/user/rc/src/net.c\`, \`net_wan.c\`, \`rc.h\`; new \`route_status_policy.h\`, \`tests/route_status_policy.c\`, and \`.github/workflows/wan-route-status.yml\`.
- **Validation:** local GNU89 \`-Wall -Wextra -Werror\` regression with ASan/UBSan and leak detection: **PASS**. The regression verifies first-error preservation and that later route operations are still attempted. Workflow source contracts cover static/DHCP wiring, allocation handling, public return types and WAN/MAN delete ordering.
- **Actions:** exact-head WAN route workflow is **PENDING** at commit creation; reconcile its run ID, conclusion and head SHA during the next recovery. The existing documentation-contract run [42550725545](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42550725545) passed on the pre-change main SHA \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Validation boundary:** L1 selected helper/caller review PASS; L2 focused host policy regression PASS. Production RC translation-unit/object, MIPS/uClibc target build, complete firmware/TRX, live route failure injection and physical-router validation are **NOT VERIFIED**.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions, then rotate away from WAN to Firewall/NAT lifecycle ownership; do not mark WAN/PPP audited or target-verified.

---

## 2026-11-09 10:23 CST — Establish durable cross-conversation handoff and bounded project records

- **Base main SHA:** `71de75dd39b2ebc87fa56b1c74ff798cc0e57fdf`.
- **First priority:** make GitHub sufficient for a fresh GPT session to recover the exact active task, whole-repository progress and verification boundary without relying on chat memory.
- **Governance:** bootstrap and protocol now enforce one recovery sequence, compact current-state root files, size limits, lossless archives and resumption of the open task before selecting new work.
- **History preservation:** the old `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and all 663 original `dev_log.md` entries are retained verbatim in indexed archives. This root log contains this entry plus the 19 newest prior entries.
- **Automation:** add `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow. Its exact-HEAD result is PENDING until retrieved.
- **Scope:** documentation/governance only; no firmware source, MIPS build, firmware image or hardware test is changed or claimed.
- **Resulting commit:** THIS COMMIT; fetch live `main` during the next recovery to obtain the exact SHA.


---

## 2026-11-09 09:58 CST — Include the active USB NDIS interface in WAN teardown verification

- **Base main SHA:** `d324ce8e26f6f730aa36d311c8d51c701879edcc`.
- **Follow-up finding:** a final caller-path review found that `stop_wan_usbnet()` can operate on a distinct NDIS interface stored in `wan0_ifname_t`. The first teardown-policy version checked only the management interface and `IFNAME_WAN`; because the USB helper itself returns void, a distinct `wwan0` / `weth0` down failure could still escape the checked result.
- **Fix:** code commit `8a02953fb60cced248254df86feca85e8bc42d10` passes the runtime `ifname_t` as an optional third interface only when a USB modem is active and `modem_type == 3`. The shared policy now attempts all non-empty distinct management, physical-WAN and USB-NDIS names, deduplicates equal names, and aggregates failures without skipping later cleanup. The adapter's existing `ENODEV` handling still treats a disappeared interface as unavailable, while other errors keep the final state-clear gate closed.
- **Regression expansion:** `tests/wan_teardown_policy.c` now covers optional NDIS shutdown, first/second/third-interface failures, continued attempts after a failure, duplicates across the three names, wired-only operation and invalid required inputs. The workflow source contract verifies the active NDIS selection and that `ifname_t` reaches the common checked teardown.
- **Workflow compatibility:** exact-head WAN regression [run 42548284186](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284186) completed **SUCCESS** on `8a02953fb60cced248254df86feca85e8bc42d10`. The broader HTTPD/service lifecycle regression [run 42548284184](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284184) also completed **SUCCESS** on the same head. This followed two corrected source-contract mismatches: the initial WAN workflow's global return search was narrowed in `395af4db6d5f10e9356b9709d9072aa4673be0f8`, and the existing HTTPD workflow's expected `stop_wan()` failure-return count was updated from three to four, with an explicit interface-state gate, in `d63ddae44e104d3fd781a2d19b30a86d2bd7d083`. Both workflows subsequently passed on the final code head.
- **Additional diagnostic precision:** `d324ce8e26f6f730aa36d311c8d51c701879edcc` changes the failure log to say that the *final* WAN state clear is skipped; this avoids implying that protocol-specific `wan_down()` bookkeeping performed earlier has been rolled back.
- **Validation:** L1 control-flow review PASS. Local GNU89 host regression compiled with `-Wall -Wextra -Werror -fsanitize=address,undefined` and ran with ASan/UBSan plus leak detection: **PASS**. Exact-head Actions above passed both the focused policy regression and the existing service-stop/WAN contracts.
- **Validation boundary:** no production RC translation-unit/object build, MIPS/uClibc, firmware/TRX, runtime NDIS hot-unplug/failure injection or physical-router testing. The static/DHCP route result-propagation finding remains separate and open; WAN/PPP, Wireless and Firewall/NAT are not closed.

---

## 2026-11-09 09:49 CST — Preserve WAN runtime state when interface shutdown fails

- **Base main SHA:** `54689c55e57094f95fda7566afd2933ecf40fbf9`.
- **Implementation commit:** `26c3c67d27e1b18fba4da14bd607f80fa70afe3b` — add a small host-testable interface teardown policy and make `stop_wan()` check the final management and physical-WAN `ifconfig(..., down, 0.0.0.0)` operations after USB-modem shutdown. Both distinct interface names are attempted even if the first fails; duplicate names are attempted once. `ENODEV` is treated as an interface already removed/unavailable; other failures are logged and cause `stop_wan()` to return `-1` after callback-link cleanup but before `clear_wan_state()` and final WAN LED-down publication.
- **Finding:** `ifconfig()` returns `-1` for socket/name-copy failures and positive errno values for ioctl failures. The previous `stop_wan()` discarded the return from both interface-down calls and still cleared WAN runtime state and returned success. A failed interface-down could therefore be reported as a completed disconnect/reconnect precondition while an interface remained active.
- **SIGUSR2 investigation:** the suspected ignored `killall -SIGUSR2 udhcpc` status was traced rather than patched speculatively. It is a best-effort DHCP RELEASE signal; the subsequent `kill_services(svcs_wan, 3, 1)` uses the process inventory, TERM/KILL escalation and final survivor check, and `stop_wan()` returns before interface teardown if a WAN worker remains. No unsafe teardown ordering was demonstrated for the ignored RELEASE result or fixed 300ms grace delay. Keep its status/retry policy as an explicit review item only if new evidence shows a defect.
- **Route audit evidence:** `route_add()` / `route_del()` return `route_manip()` errors, but `net.c::control_static_routes()` discards per-route results and returns success, while `net_wan.c::control_dhcp_routes()` is void and discards dynamic-route add/delete failures. This is a separate unimplemented follow-up, not included in this bounded interface teardown change.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; new `trunk/user/rc/src/wan_teardown_policy.h`; new `trunk/user/rc/src/tests/wan_teardown_policy.c`; new `.github/workflows/wan-teardown-stability.yml`.
- **Regression / workflow correction:** the initial workflow commit's global `return -1` ordering assertion matched an earlier PPP/service failure return and failed on run [42547441349](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42547441349). Workflow-only correction commit `395af4db6d5f10e9356b9709d9072aa4673be0f8` scopes the assertion to the interface-failure block; no production-code change was needed for this test correction.
- **Validation:** L1 source review confirmed the `ifconfig()` errno contract, the preceding PPP/WAN service-exit fences, the dual-interface shutdown ordering and the state-clear gate. L2 local host regression compiled with `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` and ran with ASan/UBSan plus leak detection: **PASS**. Exact workflow-head Actions [run 42547527340](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42547527340) completed **SUCCESS** on `395af4db6d5f10e9356b9709d9072aa4673be0f8`; compile/run and Python source-contract steps passed.
- **Validation boundary / remaining risks:** no production RC translation-unit/object build, MIPS/uClibc build, complete firmware/TRX, live DHCP client/interface failure injection, USB modem runtime or physical-router validation. Static and DHCP route add/delete result propagation remains open; Wireless still has the separate iNIC reboot/stop residuals; Firewall/NAT and the wider WAN/PPP audit remain open.

---
