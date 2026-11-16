## 2026-11-16 — Reject incomplete PPP IPCP addresses before publishing WAN state

- Parent main: `8ab4d6ecb4e6b39078aaf5b944c91085f9bc8668`. `ipup_main()` treated missing `IPLOCAL` as optional, skipping interface-address application but still publishing interface identity and continuing into `wan_up()`. Missing `IPREMOTE` left the previous NVRAM gateway in place. PPPD invokes `ip-up` when IPCP has come up and provides both IPv4 endpoint variables; absent/empty values are an inconsistent callback contract.
- `ppp_wan_ipup_apply_address()` now requires non-empty local and remote IPCP endpoints before calling ifconfig or publishing the lease. `ipup_main()` retrieves both unconditionally and writes the current peer gateway only after validation.
- Added GNU89 policy coverage for NULL/empty local and remote endpoint values plus no side effects; strengthened the WAN DHCP callback workflow to prevent reintroducing the optional branches.
- Parent exact-head WAN DHCP callback `43869140586`, WAN route `43869140499`, Process lifecycle `43869140664`, Process inventory `43869140439`, miniupnpd `43869140553`, AI docs `43869140667`, HTTPD `43869140668`, infosvr `43869140608`, AliDDNS `43869140787`, VPN `43869140953`, DNSMasq `43869140657`, AdGuardHome `43869140393`, AliyunDrive `43869140626`, WAN teardown `43869140587`, WAN ARP `43869140602`, detect_internet `43869140477`, detect_link `43869140381`, USB identity `43869140444`, and NVRAM `43869140525` passed. XY-C1 `43869140714` was still building.
- This commit requires exact-head WAN DHCP and XY-C1 validation. Firmware L3 remains unverified until one SHA passes Build, TRX and artifact upload; hardware L4 remains unverified.
- Next: inspect exact-head Actions, then continue the bounded PPP IPv6/DHCPv6 callback state audit without weakening generation fencing.

---

## 2026-11-16 — Propagate static IPoE WAN/LAN subnet overlap to startup status

- Parent main: `c38c9d288707e98616c796bb0724e64fd3885eab`. Static IPoE WAN/LAN subnet overlap prevented `wan_up()` but only set local `wan_err`; it did not update `wan_start_status`, allowing PID 1 to observe false success. The branch now propagates `EADDRINUSE` with first-error semantics.
- Added a WAN DHCP callback regression asserting that the exact overlap branch records the error through `rc_first_service_start_status()`.
- Next: verify exact-head WAN DHCP, Process and miniupnpd checks; inspect XY-C1 firmware Build/TRX/artifact and NVRAM audit before continuing the WAN IPTV/PPPoE adjunct status audit.

---

## 2026-11-16 — Propagate static IPoE subnet conflict to WAN startup status

- Parent main: `68643ba87132dedc5945679d13e6b0e216861cdc`. Static IPoE WAN/LAN subnet overlap already set `wan_err` and blocked `wan_up()`, but failed to update `wan_start_status`. This could return success to PID 1 for a rejected WAN configuration. The branch now aggregates `EADDRINUSE` using first-error semantics.
- Extended the WAN DHCP callback workflow to assert that the exact overlap branch contains the `EADDRINUSE` aggregation, before static address application proceeds.
- The prior miniupnpd source-contract count was repaired to account for both PPP setup branches capturing migration status. Remaining next tasks are exact-head verification and the WAN IPTV/PPPoE adjunct startup status pass.
- No same-SHA firmware Build/TRX/artifact L3 or physical-router L4 claim is made.

---

## 2026-11-16 — Return configured IPTV and PPPoE passthrough startup failures

- Parent main: 168527e76b4ba78f45d2268ecf8992563d93fbf6. Review of the WAN startup tail found two configured adjunct services whose failures were logged but discarded: launch_viptv_wan() (IPTV VLAN/static/DHCP service setup) and set_passthrough_pppoe(1) (including PPPoE-relay launch/readiness or stale-relay stop failure when passthrough is disabled).
- Both statuses now enter wan_start_status using first-error semantics while allowing the core WAN/other services to continue. No required WAN core startup is aborted solely because the IPTV or passthrough adjunct fails; PID 1 now receives the first configured-service error.
- Added a GNU89 policy regression for preserving the first IPTV/passthrough failure and strengthened the shared WAN checker to bind both statuses to their production call sites.
- Parent d8d59 exact-head WAN route 43868343214, Process inventory/lifecycle 43868343273, HTTPD 43868343265, AI docs 43868343106, infosvr 43868343256, AdGuardHome 43868343268, AliyunDrive 43868343283 and AliDDNS 43868343245 passed. miniupnpd 43868343344 failed only at a stale call-count assertion (fixed in 168527). XY-C1 43868343338 was still building and NVRAM 43868343257 auditing.
- Next: verify the configured adjunct-service change with Process/XY-C1/WAN route and the miniupnpd call-count repair; check same-SHA Build/TRX/artifact before any L3 claim.

---

## 2026-11-16 — Correct miniupnpd migration call-count contract after PPP refactor

- Parent main: d8d59acfb40c8d439682d8764e872283bed4a23c. WAN route `43868343214`, Process inventory/lifecycle `43868343273`, HTTPD security `43868343265`, AI docs `43868343106`, infosvr `43868343256`, AdGuardHome `43868343268`, AliyunDrive `43868343283` and AliDDNS `43868343245` passed.
- The miniupnpd workflow reached its final source contract then failed because it expected one captured PPP interface-migration call in start_wan(). There are intentionally two: one for the USB RAS modem path and one for ordinary PPP. The IPoE migration path is a separate wan_ifname call. All three now use the same captured-status gate; the rollback helper retains its independent stop gate.
- Updated the workflow to require both PPP calls rather than weakening the count to an unbounded presence check. No production source change.
- XY-C1 `43868343338` was still building; TRX verification and artifact upload were pending. NVRAM `43868343257` was still auditing. No firmware L3 claim.
- Next: inspect this miniupnpd workflow repair plus XY-C1 Build/TRX/artifact and NVRAM exact-SHA checks; then continue a bounded WAN adjunct-service status pass.

---

## 2026-11-16 — Align WAN/PPP integration checks with the explicit startup status contract

- Parent main: ccca92b531d44c2476b6041a8adc3cbdec2f3a9f. Modem WAN policy and RC first-error tests compiled and passed under GNU89 + ASan/UBSan. The shared checker then failed because it expected raw interface_status, while the implementation converts negative ifconfig results to captured errno and returns failure_status.
- The WAN route workflow searched for a multi-line first-error C assignment as a single-line string. The miniupnpd and HTTPD workflows still expected pre-refactor inline migration/launch checks and boolean demand readiness.
- These are stale cross-module source assertions; no HTTPD or miniupnpd production source failed or was modified. The validator repair updates them to check captured status aggregation before fallback/cleanup.
- Exact-head AI docs, infosvr, VPN, WAN gateway ARP, detect_link, USB modem identity, WAN DHCP callback, DNSMasq, process inventory, detect_internet, AliDDNS, AdGuardHome, WAN teardown and AliyunDrive passed. NVRAM was in progress at last inspection.
- XY-C1 Build/TRX/artifact, WAN route, miniupnpd and HTTPD did not pass because of these contract mismatches; Process inventory failed at the shared WAN checker. No L3/L4 claim.
- Next: inspect exact-head repair Actions; continue WAN error-return review after the validators are green.

---

## 2026-11-16 — Propagate required PPP and IPoE startup failures to PID 1

- Parent main: `48ff82881c7e83c29ab64a5285ee9d2bf8be6955`. Targeted review of `start_wan()` found required PPP/IPoE transitions that aborted startup or set a per-WAN error flag without entering the status returned to PID 1: PPP/IPoE UPnP migration stop, PPTP/L2TP physical setup, pppd launch, demand PPP readiness, static IPv4 assignment, EAPOL startup and DHCP client launch.
- Each required failure now enters the first-error WAN accumulator before `continue` or best-effort recovery. PPPoE's intentional best-effort physical setup behavior and optional MTU/UPnP notification behavior are unchanged.
- `wait_ppp_up()` now returns zero only for readiness success and retains the underlying interface, socket or route error for the caller.
- Added GNU89 policy regressions for PPP/IPoE first-error retention and source-contract coverage for each mandatory operation; updated WAN route workflow demand-readiness assertions to the explicit status contract.
- Parent `48ff8288` AI docs, NVRAM, AliDDNS, infosvr, AliyunDrive, AdGuardHome, Process inventory and WAN DHCP callback runs were queued at last inspection; no exact-head source tests or firmware L3 are claimed yet.
- Next: inspect exact-head Process, XY-C1, WAN route/DHCP and AI docs; continue checking required callback/lease failure propagation. No physical-router L4 claim.

---

## 2026-11-16 — Align WAN DHCP callback contract with the static MAN address status variable

- Parent main: `3c7a5a125f670335cb1b305674790eece80b1dcc`. WAN route operation regression `43866768822` and AI handoff docs `43866769102` passed. WAN DHCP callback `43866769025` passed all six native GNU89 regression binaries, then failed in its final source-contract step because it still expected `launch_status = ifconfig(...)`. The MAN startup fix intentionally records the direct address result in `address_status` before aggregating route and gateway failures.
- Updated the WAN DHCP workflow to assert the current `address_status` contract and verify its order before MAN route setup. Product source is unchanged.
- Next: re-run exact-head WAN DHCP callback and then continue the required PPP/IPoE start-status propagation slice. No L3 claim.

---

## 2026-11-16 — Align WAN route regressions with first-error MAN status accumulation

- Parent main: `acf73fd0fa217560ed670beb2d92bdd0d0a4a924`. The MAN route source fix correctly replaced `launch_status = gateway_route_status` with first-error `route_status_accumulate()` for both the complete MAN route result and the separate gateway-route result. The WAN route and WAN DHCP callback workflows still asserted the old overwrite assignment, so exact-head runs `43866344779` and `43866344884` failed at those stale expectations; no product source/build failure was indicated.
- Updated both workflow source contracts to require preservation of the first failure from both route results. The source change is retained unchanged.
- On parent acf73: AI docs `43866344868`, infosvr `43866344724`, USB modem identity `43866344763`, VPN `43866344749`, miniupnpd `43866345059`, WAN gateway ARP `43866344761`, detect_internet `43866344886`, WAN teardown `43866344963`, AdGuardHome `43866344879`, Process inventory `43866344891`, DNSMasq `43866344739`, detect_link `43866344715`, AliDDNS `43866344897` passed. Process `43866345006`, XY-C1 `43866344767`, NVRAM `43866344923` and HTTPD `43866344906` remained in progress at last inspection.
- Next: verify both corrected WAN regression workflows on the exact repair SHA; continue the required PPP/IPoE status propagation review without claiming L3 until Build/TRX/artifact pass on one SHA.

---

## 2026-11-16 — Propagate manual WAN route and physical IPv6 startup failures

- Parent main: `06bfa191302f0d53567a1cdb10652913b93cd92a`. `launch_wanx()` previously discarded a failed `man_up_internal()` static MAN route result unless the separate gateway-route out parameter also failed. It now preserves first-error status across static address setup, complete MAN route setup, gateway route and configured physical IPv6 tunnel setup, preventing false-success WAN bring-up.
- Added source-contract assertions in the WAN route workflow for the MAN address, full route, gateway and physical IPv6 status paths.
- Parent AI docs `43866012138`, infosvr `43866012145`, AdGuardHome `43866012153`, AliyunDrive `43866012166` and AliDDNS `43866012103`: PASS. Process `43866012142`, XY-C1 `43866012127` and NVRAM `43866012113` were still running at last inspection; no L3/L4 claim.
- The immediately preceding main commit fixed the IPv6 source-checker gate ranges and rotated the 120-record root log; this entry keeps the root rolling log at 22 complete records.
- Next: check exact-head Process, XY-C1, WAN route and AI docs; XY-C1 L3 still requires Build, TRX verification and artifact upload on the same source SHA.

---

## 2026-11-16 — Correct WAN IPv6 contract scope and rotate the rolling development log

- Parent main: `de6799e468caa60bfcd086b263e82bb149efd362`. Exact-head Process `43864900710` and XY-C1 `43864901114` failed at the new WAN source checker because its slices started at the `control_if_ipv6()` call and excluded the enclosing runtime `get_ipv6_type() != IPV6_DISABLED` guard. Both GNU89 C policy binaries passed before the checker failed; XY-C1 Build, TRX verification and artifact upload were skipped. This was a checker-scope false positive, not a product compile failure.
- The checker now captures each complete IPv6 guard, control call, required error return and interface publication boundary.
- AI docs run `43864900749` failed because root `dev_log.md` reached 120 dated entries. This commit preserves the oldest 100 complete entries byte-for-byte and oldest-first in `docs/dev-log/2026/11/part-012.md`, retaining the newest 20 existing records in root plus this correction record (21 total). The archive index is updated with the exact UTF-8 byte count and Git blob SHA.
- PR #1 was reviewed as a closed, unmerged historical branch. The HMAC helper source/Makefile, HMAC test, CA bootstrap test and transport fixture already match main; the live package, launcher, workflow and semantic checker have evolved but retain the HMAC and synchronous CA trust-store contracts. No stale PR branch commits were cherry-picked.
- Next: verify this correction's exact-head Process, AI docs and XY-C1 Actions; do not claim firmware L3 until same-SHA Build, TRX verification and artifact upload all pass.

---

## 2026-11-16 — Propagate required IPv6 control failures during WAN interface publication

- Parent main: `cf45552d54488f6d64cc3d2c1118ba9385a0ea7b`. A targeted WAN call-site audit found `config_vinet_wan()` ignored the result of disabling IPv6 on the physical VLAN parent and enabling it on the selected WAN interface; `config_apcli_wisp()` likewise published the AP-client interface after discarding its IPv6 enable result.
- These controls are required whenever IPv6 WAN mode is active: an incomplete parent disable can leave IPv6 on the wrong VLAN device, while failed enable can publish an interface whose IPv6 stack is not ready. The code now checks each return, logs the exact interface/status, aborts the migration before publishing `ifname`, and skips unnecessary parent-disable work when IPv6 is globally disabled.
- Strengthened `scripts/check_modem_wan_start_status.py` with semantic source contracts for all three checked operations and publication/ordering gates. The existing Process and XY-C1 workflows already run this checker.
- Parent exact-head Process `43864471819` SUCCESS. XY-C1 `43864471994` remained IN_PROGRESS in `Build XY-C1`; TRX and artifact steps were still pending. The new source change needs its own exact-head Process/XY-C1 validation; no L3/L4 claim.
- Next: inspect the exact-head Actions for both WAN slices, then continue tracing unchecked PPP, static-address and DHCP launch results. Root `dev_log.md` will contain 119 complete records after this entry, below its 120-record rotation threshold.

---

## 2026-11-16 — Propagate physical WAN interface bring-up failure to PID 1

- Parent main: `f16026b0af5d00087e36929b14843cf6b94a8b6a`. `start_wan()` checked and logged a failed physical WAN `ifconfig(..., IFUP, ...)`, attempted cleanup and continued, but never merged that required interface-readiness failure into its return status. PID 1 could therefore observe a successful WAN start when the physical interface was unavailable.
- The failure now enters the first-error WAN startup accumulator before best-effort interface-down cleanup. The original IFUP error is retained even if cleanup also fails.
- Added a GNU89 policy regression for first-error preservation and strengthened `check_modem_wan_start_status.py` to require the production IFUP failure path to aggregate before cleanup/continue. Both Process and XY-C1 workflows already compile/run this test and checker; no workflow expansion was necessary.
- Parent exact-head Process `43863800540`, AI docs `43863800477`, infosvr `43863800722`, NVRAM `43863800497`, AliDDNS `43863800501`, AliyunDrive `43863800566` and AdGuardHome `43863800941` passed. Parent XY-C1 `43863800656` was still running; this source change requires fresh exact-head verification. No L3 or L4 claim.
- Next: verify this commit's Process and XY-C1 Actions; continue the remaining WAN failure-propagation review, especially PPP launch/readiness and ignored IPv6 interface-control results.

---

## 2026-11-16 — Repair WAN checker formatting and canonical handoff headings

- Parent main: `5825e35e0987fab26b4f588f879a885094e1708b`. Process `43862972148` passed compiled host policy binaries then failed at whitespace-sensitive `wan_start_status` assertions and an obsolete expectation of `return modem_start_status;`. Updated the checker to accept formatting across helper-call line breaks and assert the combined modem/route return.
- AI docs `43862972023` and infosvr `43862972198` both failed because `SESSION_HANDOFF.md` renamed canonical `## Current repository state` / `## Active task` headings. Restored both names so existing validators and cross-repository consumers keep one stable handoff schema.
- WAN route status `43862971906` and WAN interface teardown `43862972252` passed on the parent SHA. AdGuardHome, AliyunDrive, AliDDNS, detect_internet, detect_link, DNSMasq, WAN gateway ARP, miniupnpd, VPN server, USB modem identity and HTTPD security also passed.
- NVRAM `43862972028` remained in progress. XY-C1 `43862972095` stopped before firmware Build because the pre-build source-contract step failed; TRX/artifact were skipped.
- Next: check exact-head Process/AI docs/infosvr reruns, then audit ignored MTU/interface/IPv6 configuration return values.

---

## 2026-11-16 — Repair exact-head LAN and handoff CI contract failures

- Parent main: `90ac78ccccb7acbf6ab75956999b4847a3628fd8`; product-source baseline under validation: `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`.
- Exact-head Process regression `43859397503` reached the final LAN startup source-contract step after process inventory, GNU89/Werror/ASan/UBSan and policy regressions passed, then failed at the broad raw LAN member-call assertion. Tightened the contract to require exactly two router/AP LAN member calls and require each to assign `operation_status`; the existing source assertions continue to require both first-error records.
- Exact-head AI handoff docs `43859397894` failed because `SESSION_HANDOFF.md` omitted the required end-of-session accuracy-checkpoint statement. Restored the exact validator-required wording.
- XY-C1 `43859397498` was still building the firmware; TRX verification and artifact upload were pending. NVRAM `43859397571` was still tracing raw ioctl callers. Other exact-head module Actions were passing as listed in the handoff.
- This is a workflow/governance repair only; no firmware C source changed. No L3/L4 claim is made.
- Next: verify exact-head Process/AI docs reruns, inspect XY-C1/NVRAM results, and continue the RC initialization audit for unchecked required startup operations.

---

## 2026-11-16 — Propagate WAN route setup and teardown results

- Parent main: `066697195b12b3bbc8bad1efa4a4c4abea5052fa`. Call-site audit found static IPoE `start_wan()` ignored the int result of `wan_up()`, which reports default/static WAN route and related session setup failures. It now aggregates this result into WAN startup status rather than returning success on an incomplete route setup.
- Configured IPv6 SIT tunnel startup failure is now also included in WAN startup status while the code continues attempting the independent remaining setup.
- `stop_wan_locked()` ignored `wan_down()` route teardown result and then reset `stop_status` to zero. It now preserves the route teardown failure and merges USB modem stop result before physical interface teardown; the existing final fail-closed state-clear gate now sees either error.
- Expanded `scripts/check_modem_wan_start_status.py` to assert the WAN route/IPv6 start results, static route teardown result and modem-stop aggregation.
- Parent exact-head Process `43862663789`, AI docs `43862663478`, XY-C1 `43862663471`, NVRAM `43862663293`, AdGuardHome `43862663396`, infosvr `43862663310`, AliyunDrive `43862663359` and AliDDNS `43862663392` were pending at last query. The previous Process reached a stale raw-return assertion in the modem WAN checker after compiled policy tests; the checker was corrected in `066697195b12b3bbc8bad1efa4a4c4abea5052fa`. This source slice requires new exact-head CI; no L3/L4 claim.
- Next: validate all exact-head checks and continue the remaining WAN MTU/address result audit.

---

## 2026-11-16 — Update modem WAN checker for merged L2TP start status

- Parent main: `e97f95dc097e7faa8cd00aa491de9f1fb4ad519a`. Exact-head Process run `43862009119`, job `133280916393`, completed all compiled inventory/policy regressions and the modem WAN policy binary successfully, then failed in `scripts/check_modem_wan_start_status.py` because its old contract expected `return status;` from full WAN restart, reconnect and deferred reconnect.
- This expectation became stale after `merge_l2tp_server_start_status()` was introduced to preserve negative L2TP server startup results without treating positive already-running as failure.
- Updated the checker to assert the helper's status semantics and require each of the three WAN start callers to return `merge_l2tp_server_start_status(status)`. Product WAN C source unchanged in this checker fix.
- Exact-head rerun is required; no same-SHA XY-C1 firmware/TRX/artifact L3 or physical-router L4 result is claimed.
- Next: verify Process/AI docs and current XY-C1/security Actions; continue the call-site audit of WAN startup/teardown and interface MTU return values.

---

## 2026-11-16 — Correct bridge-member errno assertion close ordering

- Parent main: `73940a656e2c62e7d7d172b1ef4c622e07715ca4`. Process run `43860996821` on `43d0ebe973f06f578282531eed4abc0fdba1bb3b`, job `133277465787`, failed at the bridge-member errno source assertion after all compiled policy tests passed.
- Root cause: `br_add_del_if_internal()` has an earlier `close(sockfd)` on the SIOCGIFINDEX error path; the checker used `.index("close(sockfd);")`, accidentally comparing errno save to that earlier close. The actual post-SIOCBRADDIF/DELIF close is the final close in the helper. Changed the assertion to compare against `.rindex("close(sockfd);")`.
- No product source behavior changed in this contract repair. Exact-head Process/AI documentation results for the combined WAN restart work remain pending; verify this checker correction on its own SHA.

---

## 2026-11-16 — Check WAN restart MAC table and L2TP service results

- Parent main: `01d9147b742391933a22781b42eb1213063cb1c1`. Further `full_restart_wan()` tracing found ignored int-returning operations: `phy_clear_mac_table()` and `safe_start_xl2tpd()`. The former is an ioctl on MTK ESW and RTL8367; the latter returns negative on lock/process/config/start failure, zero on launch success, and positive one when the shared daemon is already running.
- `full_restart_wan()` now fails before WAN startup if the switch MAC table cannot be cleared. New `merge_l2tp_server_start_status()` preserves WAN-start failure, treats positive already-running as success, and surfaces configured L2TP server launch failure when WAN start succeeded.
- Applied L2TP result aggregation in full restart, normal reconnect and deferred reconnect; none of the three connect paths now silently discards a negative L2TP server start status.
- Process source contract checks MAC clear ordering and the normalized L2TP result across all three call paths. Exact-parent Process `43861241910`, AI docs `43861242056`, NVRAM `43861243064` and XY-C1 `43861242089` were pending/queued; this slice requires fresh exact-head checks. No L3/L4 claim.
- Next: validate exact-head Process/WAN route and inspect remaining int-returning WAN/RC callsites.

---

## 2026-11-16 — Restore static LAN routes before restarting WAN

- Parent main: `43d0ebe973f06f578282531eed4abc0fdba1bb3b`. `full_restart_wan()` had already been updated to stop before WAN startup when switch VLAN reconfiguration failed, but it still discarded the int result from `add_static_lan_routes(IFNAME_BR)`.
- The helper delegates to `control_static_routes()` and reports an error if the required static LAN routes are not restored after WAN teardown. The full restart now captures that result, logs the failure and returns before `start_wan()`; an incomplete route table is not treated as a successful WAN restart.
- Process semantic source contract now asserts route restoration is checked and the failure return precedes WAN startup.
- The previous exact-head Process run on `90ac78ccccb7acbf6ab75956999b4847a3628fd8` failed only at its final static source-contract assertion; commit `43d0ebe973f06f578282531eed4abc0fdba1bb3b` corrected the checker to require exactly two assigned LAN-member calls. Exact rerun Process `43860996821` and AI docs `43860997587` were still pending at last query.
- THIS source slice needs fresh exact-head CI. No same-source-SHA firmware L3 or physical-router L4 result is claimed.
- Next: verify exact Process/AI docs and inspect any additional int-returning `full_restart_wan()` callers before proceeding.

---

## 2026-11-16 — Preserve bridge membership ioctl errno across close

- Parent main: `1ce4ffe3b71e87bdc5143ccc802d252f190ab5b0`. The preceding commit preserves ioctl errno for `br_add_del_bridge()`; a sibling review found the same issue in `br_add_del_if_internal()` for bridge port add/delete.
- If SIOCBRADDIF/SIOCBRDELIF failed with an error other than a subsequently-confirmed idempotent race, the helper called `close(sockfd)` and returned `-1` without restoring ioctl errno. Now save errno immediately after ioctl and restore it after close for any still-failed operation. Successful concurrent idempotency re-probes remain success.
- Process source contract now checks the production bridge membership helper as well as bridge creation for save-before-close and restore-on-failure behavior.
- Exact parent Actions Process `43860541389` and AI docs `43860541304` were pending; the preceding SHA `7ce9978...` Process `43860044486` and AI docs `43860044408` passed. This change needs exact-head verification; no L3/L4 claim.
- Next: inspect exact-head Process/AI docs and prior XY-C1 `43860044821`, then continue focused ioctl errno/callsite checks.

---

## 2026-11-16 — Preserve bridge ioctl errno across socket close

- Parent main: `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`. The bridge-add idempotency wrapper distinguishes EEXIST from other bridge-add failures before probing kernel bridge membership. The shared `br_add_del_bridge()` helper did not preserve the ioctl errno across its subsequent `close(sockfd)`.
- Save errno immediately after ioctl and restore it after close on ioctl failure, so bridge-init classification always sees the actual bridge operation result rather than a later close-side errno.
- Extended Process source contract to assert save-before-close and restore-on-failure ordering. Existing bridge initialization policy regression continues to cover EEXIST and fail-closed unknown inventory.
- On the parent SHA, Process `43860044486` and AI docs `43860044408` passed; XY-C1 `43860044821` and NVRAM `43860044318` were still in progress. This source slice requires fresh exact-head Actions. No same-SHA firmware L3 or router L4 claim.
- Next: validate exact-head Process/AI docs and check whether previous XY-C1/NVRAM runs completed; continue focused network utility caller audit.

---

## 2026-11-16 — Fail closed on switch VLAN reconfiguration during WAN restart

- Parent main: `0828b3a8acf119345bc2381f89772f213288862d`. Call-site review found `full_restart_wan()` still ignored the newly status-returning `switch_config_vlan(0)` and continued to `start_wan()` regardless of VLAN ioctl failures.
- `full_restart_wan()` now captures switch VLAN configuration status, logs the failure and returns before WAN startup if VLAN rules or switch bridge mode could not be configured. This avoids publishing a WAN session on unverified switch VLAN state after the old WAN has been stopped.
- Process source checker now loads `net_wan.c` and verifies the failure return occurs before `start_wan()`; workflow path filters include this file.
- Exact-head Process and AI docs results for `0828b3a8acf119345bc2381f89772f213288862d` were pending at last inspection after correcting the previous run's false-positive source assertion and handoff wording. THIS fix requires new exact-head Actions; no firmware L3 or physical L4 claim.
- Next: inspect exact Actions, then continue searching callers of recently converted status-returning switch/bridge APIs.

---

## 2026-11-16 — Correct exact-head Process and handoff contracts

- Parent main: `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`. Exact-head Process run `43858811934`, job `133269937845`, failed only at the new negative bridge-member source assertion after all compiled policy regressions (including bridge, wireless-module post-load and bridge membership) had passed. The check used substring containment and falsely matched the unchecked-call text inside the assignment form; changed it to compare complete stripped source lines.
- Exact-head AI handoff docs run `43858811955`, job `133269937387`, failed because the handoff lacked the validator-required literal `Before context exhaustion`. Restored the full mandatory end-of-session live-state checkpoint and explicit pre-exhaustion instruction.
- These are test/governance contract fixes, not new product-source changes. Exact reruns on THIS commit are required; no build or hardware outcome is claimed.

---

## 2026-11-16 — Verify required and optional LAN bridge modules after load

- Parent main: `59047eb21838e0cf78dd24e0a6eab78bea7fc886`. The existing `rc_module_load_verified()` policy explicitly documents that `module_smart_load()` can return zero after a failed `modprobe`; post-load `is_module_loaded()` state is authoritative. Its regression is already part of Process CI.
- Found ignored module load calls in `init_bridge()`: required `iNIC_mii` on `USE_RT3352_MII` boards and optional `hw_nat` bridge offload. The former could fail silently and leave an incomplete iNIC LAN path; the latter was logged as enabled unconditionally even when it had not loaded.
- Added a LAN wrapper using `rc_module_load_verified()`. Required iNIC load status now joins bridge/PID1 startup error aggregation. Optional hardware NAT offload is verified and reports enabled/unavailable, but never makes bridge startup fail.
- Process source contracts now classify both paths; the existing verified module loader unit test remains in the Process workflow. Exact-head CI is pending after publication; no firmware L3 or router L4 claim.
- Next: inspect current exact-head Process/XY-C1 and prior pending docs/security/NVRAM runs, then continue the remaining RC boot operations audit.

---

## 2026-11-16 — Restore mandatory handoff checkpoint contract

- Parent main: `2c0a0f1be6edf5a604a71ce02e726d0b2393a00c`. Exact-head AI handoff documentation run `43858670960`, job `133269446982`, failed because `SESSION_HANDOFF.md` no longer carried the validator-required explicit end-of-session accuracy-checkpoint wording.
- Restored the mandatory live-state/source-baseline/Actions/first-action comparison wording and the conditional-document-edit anti-churn rule in the canonical handoff. No source behavior changed.
- Re-run the AI handoff documentation contract on this exact commit; the prior failure is a real governance-check blocker, not a source-test failure.

---

## 2026-11-16 — Propagate switch base, storm and link configuration errors

- Parent main: `4322737edbbe77196e4380fc7506da3f0851dc0d`. The previous GMAC2 router conditional and AP-tagged LAN member accounting correction is in `4322737edbbe77196e4380fc7506da3f0851dc0d`; exact-head Actions were pending at last inspection.
- Further source tracing showed `switch_config_base()`, `switch_config_storm()`, and `switch_config_link()` discarded all return-bearing PHY ioctl results. The switch API returns int on MTK ESW and RTL8367; MTK ESW unsupported optional storm/green/EEE capabilities explicitly return success/no-op.
- Converted the three helpers to return first-error status while still attempting each independent setting. PID1 bridge startup and the switch-config notification now aggregate those results. LED indication remains explicitly best-effort and is not part of the forwarding status.
- Process semantic source assertions check all base settings, four storm controls, WAN plus each LAN link setting, and PID1/notification aggregation. Exact-head tests are pending after publication; no firmware L3 or physical router L4 result is claimed.
- Next: inspect exact-head Process, XY-C1, storage, NVRAM, docs and HTTPD/security Actions; then continue evidence-backed switch/PHY or other userland reliability fixes.

---

## 2026-11-16 — Correct conditional LAN bridge member status accounting

- Parent main: `aa5d3c9f2566927a4b17fce5d391f525cccc8095`. Exact-head Process run `43857640159` on that parent passed. Job `133265909696` confirms the bridge-init policy binary, multicast policy binary, bridge-membership regression and LAN production source contract all passed.
- A post-commit review found the router GMAC2 conditional `else` path recorded its status outside the conditional block, which could record the preceding eth2.1 attach twice; AP tagged mode also discarded its LAN member attach result.
- Scoped the router fallback attach and status record inside the preprocessor-controlled else block, and added status capture for the AP tagged LAN port. Strengthened the Process source contract to require both branches and reject the unchecked bare call.
- All unrelated exact-SHA checks listed in the preceding entry were passing; NVRAM `43857640139`, storage `43857640310`, HTTPD authentication `43857640193`, AI docs `43857640173`, VPN `43857640155`, and XY-C1 `43857640160` were still running at that inspection. No L3/L4 claim.
- Exact-head checks for THIS correction are pending. Next: inspect Process and the remaining same-SHA checks, then continue switch base/storm/link coverage.

---

## 2026-11-16 — Propagate essential bridge/VLAN initialization status

- Parent main: `9f71cebf2e323d8dc84bca7cfd312d027ca5aab1`. This slice covers required Ethernet/bridge/VLAN initialization; the existing `config_bridge()` multicast policy remains a distinct optional-feature path.
- Found `init_bridge()` discarded results from switch bridge/VLAN setup, MAC interface setup, switch PHY power, VLAN creation, bridge creation/configuration, member attachment and final bridge IFUP. `switch_config_vlan()` also discarded every VLAN rule and bridge-mode ioctl result.
- Added a separate bridge-init first-error policy; all independent VLAN and bridge operations are attempted. Duplicate `br_add_del_bridge()` EEXIST is treated as idempotent only after a successful kernel bridge inventory probe. Existing bridge-port attachment and VLAN helper operations retain their fail-closed/idempotent semantics.
- `init_bridge()` now returns status to PID 1, `switch_config_vlan()` and its restart notification propagate results, and switch configuration notifications preserve `config_bridge()` status. Optional iNIC isolation errors are diagnostic only; multicast controls remain distinct.
- GNU89/Werror/ASan/UBSan bridge-init policy regression passed locally. Process workflow now compiles this regression and the existing bridge-membership policy regression, and checks production definitions/callers/ordering. Exact-head CI is pending after publication; no firmware L3 or hardware L4 result is claimed.
- Next: inspect exact-head Process, XY-C1, storage and NVRAM runs; then continue independent RC status coverage around switch base/storm/link setup.

---

## 2026-11-16 — Gate loopback route setup on successful IFUP

- Parent main: 1f232209a9aa41d520939c18ab783609e9b791b0. Process run 43852281303 failed only at the new loopback source return-type assertion after preceding lifecycle and bridge regressions passed.
- Review found the first init_loopback() status patch attempted route_add() even when ifconfig("lo", ...) failed. Since route installation depends on a configured loopback address, init_loopback() now returns the IFUP error immediately, and only attempts the route after successful address setup. Route-add failures are logged and returned.
- Corrected the Process source contract to use a newline-safe return-type assertion and verify the early-return ordering ahead of route_add().
- XY-C1 43852281447 was still IN_PROGRESS at ccache setup at the last inspection; storage 43852281602 and NVRAM 43852281469 were also pending then. No L3 or L4 result is claimed in this record.
- Next: inspect exact-head Process, XY-C1 Build/TRX/artifact, storage and NVRAM; then audit init_bridge()/switch_config_vlan() required versus optional status handling.

---

## 2026-11-16 — Propagate loopback interface and route startup failures

- Parent main: `b85754180a56755c97a03bb5ab3013b30561f48c`. USB storage hotplug `43851311951` and NVRAM transaction boundary `43851311965` passed; storage included BusyBox 1.24.2/ash lifecycle, lock/eject and persistence regressions. Process inventory `43851311871`, AI docs `43851312213`, AdGuardHome `43851311939`, infosvr `43851311935`, AliDDNS `43851311972`, AliyunDrive `43851311982` passed.
- Found that PID 1 `init_loopback()` silently discarded both loopback `ifconfig()` and route-add failures because it returned `void`.
- Converted it to return first-error status while still attempting route setup after an interface failure; `init_router()` now merges and logs the result in `boot_services_status`. Added GNU89 first-error regression and a Process source contract for definition, operation order, both status aggregations and caller wiring.
- XY-C1 `43850461476` on ancestor production SHA `b81207777615d31b096c1425ff1fa8110898b0c2` was still at Build XY-C1 last checked; no TRX/artifact L3 or physical-router L4 result is claimed.
- Exact-head Actions for THIS commit pending after publication.
- Next: validate Process loopback source/test contract, then implement capability-aware essential bridge/VLAN initialization status propagation without promoting optional multicast features to fatal boot errors.

---

