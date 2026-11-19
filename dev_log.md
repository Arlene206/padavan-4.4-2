## 2026-11-19 — Fail closed when detect_wan cannot publish its PID identity

- Parent main SHA: `9e7882b2354e57a4408f099b73d054b3a6635b2c` (includes concurrent storage mount decoding and regression fixture corrections).
- L1 found `detect_wan_main()` ignored `fopen(DW_PID_FILE)`, `fprintf()` and `fclose()` failures, then entered `poll_gateway()`. A permissions, space or I/O error could leave a missing/partial PID file while the long-running monitor continued; the adjacent `detect_link` monitor already uses the shared strict PID/start-time writer.
- Replace the ad-hoc PID-only write with `process_pidfile_write(DW_PID_FILE, "detect_wan")`. Any inventory or atomic publication failure now logs errno (EIO fallback) and aborts before gateway polling. Normal monitor completion still removes the PID file.
- Extend the existing Detect WAN ARP workflow contract to require the shared PID helper, guarded publish-before-poll ordering, errno fallback, controlled error log/return, and absence of unchecked `fopen`. Existing Process lifecycle path filters already include `detect_wan.c`; no workflow or trigger expansion is needed.
- Parent source-storage run `44432765082` had failed only a narrow literal check; correction `16e42d02368b485781c1ce1541dc513f413a7647` is followed by mount decoding/test-fixture changes `66a0a5d` and `9e7882b`. Latest exact-head Storage hotplug `44434391034`, HTTPD security `44434391030`, Process inventory `44434391084`, and NVRAM `44434391143` were still in progress/pending when this slice began; poll their results.
- No local shell test or MIPS build was run in this connector-only session. Exact-head Detect WAN, Process, AI docs, and XY-C1 Build/TRX/artifact checks are required; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Match storage mount checker to explicit errno fallback

- Parent main SHA: `35235da0cb4af66a8dedd96b7698948ec6b15687`.
- Exact storage hotplug run `44432765082` passed the preceding full caller-graph/lock-order checks and then failed its new mount-inventory source contract. The implementation correctly preserves the original parser/read/close errno, substitutes EIO when no errno was set, and returns failure; the checker incorrectly required the narrower literal `errno = saved_errno;`.
- Tighten the test to require the actual stronger implementation expression `errno = saved_errno ? saved_errno : EIO;`. No production storage behavior or failure gate is changed.
- Parent exact checks: AI handoff `44432765070`, Process inventory `44432765047`, NVRAM transaction `44432765261`, and HTTPD auth/security `44432765061` PASS. XY-C1 `44432765098` remained IN_PROGRESS at this checkpoint; Build/TRX/artifact must be checked on the exact source SHA.
- No local test or MIPS build was run. Exact-head storage hotplug and XY-C1 verification remain required.

---

## 2026-11-19 — Preserve VPN server up/down hook failures

- Parent main SHA: `6e7e90efb393af3380f0b8607a53ecb8921ba306` (concurrent NFS daemon wait-status fix).
- The VPN server PPP callbacks already preserve route, lease, relay and lock failures, but silently ignored the return value from the configured `VPN_SERVER_UPDOWN_SCRIPT` on both connect and disconnect. A custom hook could fail to install user routing/filtering or fail teardown while the callback still returned success.
- Capture both `_eval(hook_argv, ...)` statuses, log the peer and exact status, and mark the callback failed without skipping subsequent independent cleanup. Add a route-policy regression for custom hook failure propagation and source contracts for both callbacks.
- Exact parent VPN-server lifecycle `44431781964`, WAN route-status `44431782135`, Process regression `44431782106`, Process inventory `44431782022`, and AI handoff docs `44431782169` all PASS on `521c25a387bc45041f4db1e7ca9b27318e7b0e5e`. Exact XY-C1 `44431782312` was still building: Build in progress; TRX and artifact pending. This new hook change requires fresh exact-head L2 and L3.
- No local tests or MIPS build were run. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Propagate VPN server per-peer route failures

- Parent main SHA: `aa3691d7db9efbdbf0020a5b8c58d5734a579e9f`; includes concurrent Crond/shared-daemon probe-wait normalization and its checker/test corrections.
- L1 found `vpns_route_to_remote_lan()` returned void and discarded `route_add()`/`route_del()` results. PPP up/down callbacks therefore returned success even when a configured per-peer remote-LAN route could not be installed or removed.
- Convert the helper to return route status, reject invalid peer/interface inputs, treat `EEXIST` (route already present) and `ESRCH` (route already absent) as idempotent success, and preserve/log other route errors. Both callbacks mark failure while continuing independent lease/firewall/relay work.
- Extend the existing route-status regression with first-error preservation and the VPN-server lifecycle checker with source contracts. Add route policy header/test to VPN-server Actions push/PR filters and the header to XY-C1 rebuild triggers; no new workflow added.
- On parent, VPN-server lifecycle `44430527476` PASS; Process integration/inventory `44430527367`/`44430527397`, WAN route status `44430527641`, XY-C1 `44430527521`, and NVRAM `44430527357` were queued/in progress at last poll. Inspect final conclusions.
- IAPP `007a38b34c5c7b8127f31a2aeb427e08833cdf67`, iNIC signal `82ac332f84dbe8f454ad63247d4db53acac30c23`, and RT3352 iNIC monitor `41c2382d45e0496fdd336fc09bc2873d9216d553` are L3 VERIFIED by exact-source XY Build/TRX/artifact runs `44427295060`, `44427595460`, `44428277197`.
- No local tests or MIPS build were run. Exact-head VPN-server, WAN route-status, AI docs, Process and XY-C1 Build/TRX/artifact checks are required; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Propagate VPN-client callback route failures

- Parent main SHA: 79422b3cc31fea82dd5905720e1f31b9ea037bec.
- L1 found both VPN-client PPP callback route helpers were void and discarded every route_add()/route_del() result. ip-up therefore returned success and set the connected state even when configured split-default or remote-LAN routes failed; with vpnc_dgw enabled, a missing half of the two /1 routes can cause traffic to use the ordinary WAN route instead of the VPN. ip-down also hid failures to remove stale callback routes.
- Convert both helpers to return status, validate required callback interface/peer values only when the corresponding route is configured, normalize route-add EEXIST and route-delete ESRCH (already absent) as idempotent success, and preserve other errors. Both callback handlers attempt all independent route operations and return the first failure; optional up/down scripts also have their execution result preserved.
- Extend the existing route-status policy tests for idempotent delete and errors; extend the existing WAN route-status workflow with VPN-client source assertions and push/PR path coverage. No new workflow was added.
- Parent wireless iNIC monitor status source SHA 41c2382d45e0496fdd336fc09bc2873d9216d553 is in exact XY-C1 Build; it requires the result to be polled. Current head before this change is test-only correction 79422b3cc31fea82dd5905720e1f31b9ea037bec.
- No local shell/C test or MIPS build was run. Exact-head WAN route status, AI docs, Process and XY-C1 checks required. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Update wireless iNIC source contract for captured startup status

- Parent main SHA: 41c2382d45e0496fdd336fc09bc2873d9216d553.
- The new iNIC launch-result aggregation passed all preceding policy unit binaries in Wireless radio run 44428277432, but its inline source contract stopped at the legacy literal start_inicd() != 0. This assertion became stale after production code correctly captures the launcher result in inicd_start_status; it was the first failure reported by the checker.
- Update the static contract to require the exact captured start_inicd() result and its propagation through wireless_radio_status_success(). Do not weaken the contract or change production code.
- Exact Process inventory run 44428277260 passed on parent SHA. Wireless radio 44428277432 failed at the stale assertion. XY-C1 44428277197 was still running when polled and its host checks/build progression must be inspected separately. Parent iNIC signal SHA 82ac332 host/iNIC/Process/NVRAM/AI checks all passed; XY-C1 44427595460 was still building at last poll.
- No local tests/build run. This is a test contract correction; exact-head Wireless radio/AI/Process checks are pending. It cannot itself establish XY-C1 L3.

---

## 2026-11-19 — Propagate RT3352 iNIC health-monitor startup failures

- Parent main SHA: `82ac332f84dbe8f454ad63247d4db53acac30c23`.
- `trunk/user/rc/src/net_wifi.c::start_wifi_ap_rt()` under `USE_RT3352_MII` logged a failed `start_inicd()` but never merged the failure into `all_started`, so the radio restart could return success with its enabled health monitor missing. The corresponding stop path already treats stopping iNICD as a prerequisite before PHY isolation.
- Capture and report the monitor launch status and merge it into the existing sticky radio startup result. Preserve the deliberate no-op when `inic_disable=1`. Added a status-policy regression and a source contract for the NVRAM gate, launch, log, aggregation and final return order.
- The parent iNIC signal workflow `44427595739`, Process inventory `44427595469`, AI docs `44427595502`, and NVRAM audit `44427595602` passed exact parent SHA. Parent XY-C1 run `44427595460` was IN_PROGRESS at last poll; the IAPP production-source XY run `44427295060` was also IN_PROGRESS. No exact L3 evidence is claimed until final Build/TRX/artifact steps pass.
- Local tests and MIPS build NOT RUN (connector-only environment). Exact-head Wireless radio/Process/AI and XY-C1 Build/TRX/artifact checks are required; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Fail closed when iNIC auxiliary signal dispositions cannot be installed

- Parent main SHA: `007a38b34c5c7b8127f31a2aeb427e08833cdf67`.
- L1 found `inicd_main()` checked installation of SIGALRM/SIGTERM handlers but ignored all four `sigaction()` results when setting SIGPIPE, SIGHUP, SIGUSR1 and SIGUSR2 to SIG_IGN. On a disposition-install failure, the daemon still launched with the affected signal's default behavior; e.g. an unexpected SIGHUP could terminate the monitor while the code believed it was ignored.
- The daemon now checks the complete auxiliary disposition setup before `daemon()`; any failure captures errno (or EIO fallback), restores the caller's signal mask and returns without publishing the PID file or launching the monitor. The pure policy helper has GNU89 regression coverage, and the existing iNIC source contract asserts all four checked calls precede daemonization and that failure restores the mask.
- Existing iNIC signal workflow already watches the production source, policy header, test and checker for both push and pull_request; no workflow was disabled or weakened.
- Parent IAPP SHA `007a38b34c5c7b8127f31a2aeb427e08833cdf67`: standalone IAPP `44427295051`, Process inventory `44427294988`, AI docs `44427294975` PASS; Process lifecycle `44427295029` and XY-C1 `44427295060` were in progress at this checkpoint. Build/TRX/artifact for this parent are not yet confirmed.
- No local C test or MIPS build was run in this connector-only session. Exact iNIC signal, Process, AI docs and XY-C1 Build/TRX/artifact checks for this source SHA are required. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Keep IAPP procfs diagnostics in the controlled service log

- Parent main SHA: `d06c6a31212a42c5ec5afec99124010b80fcc2a4`.
- The initial exact IAPP run `44426426321` emitted raw grep errors to stderr for the two intentional missing-procfs regression fixtures; the Process integration checker requires the lifecycle invocation to return no uncontrolled stderr. The production path already records each inventory error through `log_iapp()` and returns grep's exact status, so the raw grep diagnostic duplicated the controlled service log.
- Redirect only the two `grep -q` stderr streams to `/dev/null`; preserve exit status tri-state and fail-closed returns. The structured `log_iapp()` error remains authoritative, while expected inventory faults no longer leak raw command diagnostics to the caller.
- The fixture-only correction parent `d06c6a3` has IAPP lifecycle `44426901415`, Process regression `44426901495`, Process inventory `44426901507`, and AI docs `44426901440` PASS. Its XY-C1 workflow `44426901490` completed successfully but deliberately SKIPPED Build/TRX/artifact because it contained test/docs only; it is not L3 evidence for the radio inventory production source.
- Exact-head IAPP, Process and XY-C1 production Build/TRX/artifact runs for this source change are pending. No local shell test or local MIPS build was run; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Restore IAPP radio fixtures after fallback regression

- Parent main SHA: `0c7648e03312f2923b343339209ce57cdb58f600`.
- Exact IAPP workflow `44426426321` passed shell syntax and the new module/interrupt inventory-error branches, then failed because the added valid-fallback case mutated the shared fixture used by subsequent pre-existing lifecycle checks.
- Restore the original `mt76x3_ap` module and `rai0` interrupt fixtures immediately after the fallback assertion. This is a test-only correction; no production behavior changed.
- Exact-head IAPP, Process, AI docs and XY-C1 checks for the corrected regression remain pending. The preceding production-source SHA retains its independent Actions results; do not treat the failed harness run as L2 completion.

---

## 2026-11-19 — Fail closed on unreadable Ralink IAPP radio inventory

- Parent main SHA: `d7c6c57e47876b320bd42eb2f95f0542d6cb4cc6`.
- `trunk/user/ralinkiappd/iappd.sh::start()` previously treated every non-match from `grep -q` as proof that a radio marker was absent. A procfs read error (grep status 2) therefore selected the legacy `rax0` fallback and could launch the daemon against the wrong interface.
- The startup path now distinguishes grep status 0 (match), 1 (complete inventory with no match), and >1 (inventory error). An unreadable `/proc/modules` or required `/proc/interrupts` now aborts before launching `ralinkiappd` or publishing firewall rules; the verified no-match fallback is unchanged.
- Extended the existing IAPP lifecycle regression with both procfs read-error paths and a valid no-match fallback check. Existing IAPP lifecycle, Process lifecycle, and XY-C1 workflow path filters already include the changed launcher and regression file; no workflow trigger was broadened or disabled.
- The immediately preceding WAN monitor SIGTERM fix at `d7c6c57e47876b320bd42eb2f95f0542d6cb4cc6` passed exact-head WAN ARP (`44424808719`), Process regression (`44424808712`), Process inventory (`44424808772`), NVRAM (`44424808887`) and AI docs (`44424808821`). XY-C1 run `44424808889` completed Build, TRX verification and artifact upload on that exact preceding SHA.
- Exact IAPP lifecycle run `44426426321` on this source SHA failed at the later pre-existing `wrong radio arguments` assertion. The two new unreadable-procfs cases and valid fallback assertion had passed, but that fallback test left the shared module/interrupt fixtures changed for subsequent tests. This is a test-harness fixture leak, not a production-source failure; this follow-up restores the original fixture before remaining lifecycle cases. Process (`44426426254`, `44426426375`), AI docs (`44426426366`) and XY-C1 (`44426426264`) were still in progress at the correction point.
- Local shell tests were not run in this connector-only session. The follow-up fixture correction must pass exact-head IAPP lifecycle; Process, AI docs and exact-source XY-C1 Build/TRX/artifact results must also be inspected. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 07:10 CST (UTC+8) — Defer detect_wan SIGTERM cleanup to normal context

- Parent main SHA: `d98ef418844b40bac6c6e31df6480a24645dc47b`.
- L1 found `catch_sig_detect_wan()` directly called `remove(DW_PID_FILE)` and `exit(0)` from SIGTERM handler context. Neither function is async-signal-safe; if SIGTERM interrupts libc stdio/allocator state, shutdown can deadlock or corrupt process state.
- THIS COMMIT records SIGTERM in a `volatile sig_atomic_t` flag, checks it around bounded ARP receive/retry and DHCP renewal, replaces long waits with one-second shutdown-aware slices, and removes the PID file after the monitor loop returns in normal process context. Signal-action installation errors are no longer ignored.
- Added `detect_wan_signal_policy.h` and GNU89/Werror/ASan/UBSan coverage for SIGTERM-only sticky flagging; extended WAN ARP source-contract checks and added the new production header to XY-C1 push filters.
- Parent AP-LAN Process checker/inventory and WAN route runs `44422350218`, `44422350427`, `44422787189` PASS; AP-LAN XY-C1 source SHA `747bb165` is L3 PASS on `44421896203`. AI docs archive rotation PASS on `44424090829`. USB storage `44420519515` PASS; MentoHUST L3 `44415470536` PASS.
- Local tests NOT RUN (GitHub connector-only environment); exact-head WAN ARP, XY-C1 Build/TRX/artifact rerun pending. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Update WAN route workflow for the AP LAN PHY gate

- Parent main SHA: `094d488122768b104c54ce371fb92943ceca0689`. WAN route run `44421896213` on product-source SHA `747bb165c61051000dfe6426c0de808cfd96f8b1` failed after every preceding route/IPv6/C regression passed. The failing inline source check still expected the old PHY wait result to be held in `operation_status` and used `if (operation_status == 0)` as the DHCP gate. The production change deliberately moved the PHY result to `phy_wait_status` and uses `rc_lan_ap_dhcp_start_allowed_after_phy_wait()`.
- Update the workflow's source contract to require separate PHY result capture, first-error aggregation, independent monitor notification, and the PHY-only DHCP gate. Preserve ordering assertions and both push/pull_request path coverage; the policy header is already present in both allowlists.
- No production code changed in this test-contract update. Exact-head WAN route, Process and AI docs runs for the resulting commit are pending at commit creation. XY-C1 remains tied to product-source SHA `747bb165c61051000dfe6426c0de808cfd96f8b1` (run `44421896203`); local tests and physical-router L4 remain NOT RUN/NOT VERIFIED.

## 2026-11-19 — Narrow the AP LAN DHCP source contract to the post-notification gate

- Parent main SHA: `747bb165c61051000dfe6426c0de808cfd96f8b1`. Exact Process workflow run `44421896053` compiled and passed the GNU89/ASan/UBSan `service_start_status.c` regression and all earlier integration checks, then failed in the new source checker. The assertion that no `if (operation_status == 0)` existed anywhere in `start_lan()` was too broad: the existing required callback-link preflight uses that condition before the PHY wait. This was a test-contract defect, not a production-source or C-regression failure.
- Correct the contract to reject the stale operation-status gate only after `notify_run_detect_internet(60)`, while retaining the required ordered PHY capture, first-error aggregation, independent monitor notification and PHY-only DHCP gate. The production implementation and regression cases are unchanged. Also normalize the policy-header comment indentation.
- AI handoff docs run `44421896176` is SUCCESS on exact source SHA `747bb165c61051000dfe6426c0de808cfd96f8b1`. XY-C1 run `44421896203` and the corrected exact-head Process run remain pending at the time of this correction.
- No local compile/test was run (GitHub connector-only environment). Hardware L4 remains NOT VERIFIED.

## 2026-11-19 — Keep AP LAN DHCP launch gated by PHY readiness

- Parent main SHA: `a2d0b74112b4002073015dccc5fca88f6ccd43ab`. The preceding storage hotplug CI repair is verified: exact-head USB storage run `44420519515` completed SUCCESS on `a2d0b74112b4002073015dccc5fca88f6ccd43ab`; every step passed, including the shared-header-dependent RC service launch regression compile/run and firmware BusyBox ash Optware regressions.
- Review of `start_lan()` found the mandatory PHY settle wait and the independent `notify_run_detect_internet(60)` result reused `operation_status`. The monitor call overwrote the wait result before the DHCP gate and before first-error aggregation: a successful notification could therefore permit DHCP after a failed PHY wait, while a monitor notification error could suppress DHCP after a successful wait.
- Keep `phy_wait_status` separate, aggregate it before the independent monitor notification, and gate `start_udhcpc_lan()` only on the PHY wait postcondition. Monitor-notification errors remain visible in the aggregate status but do not change DHCP eligibility.
- Extend the GNU89 service-start policy regression with both cross-failure cases, and strengthen the Process integration checker to pin production ordering and Process/XY-C1 regression coverage. Existing workflow path filters already include every changed source/test/checker path; no workflow coverage was removed or broadened.
- No local compile/test was run (GitHub connector-only environment). Exact-head Process lifecycle, AI handoff docs and XY-C1 Actions for the resulting source commit are pending at commit creation. Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 07:24 CST (UTC+8) — Fix storage hotplug service-status test include path

- Parent main SHA: `b26b59b2175492567e2018a93bb41c78e1036bd0`.
- USB storage workflow run `44418388785` failed at `Compile and run RC service launch status regression`; its previous storage daemon stop policy regression passed. The new test includes `optional_daemon_policy.h`, which transitively includes `trunk/user/shared/src/process_inventory.h`, but this workflow passed only `-I trunk/user/rc/src`.
- THIS COMMIT adds the shared include directory and extends the workflow's inline source checker to enforce both include directories and push/pull_request filters for the policy and shared inventory headers.
- Crond/IPTV Process L2 is PASS on exact current parent main: Process umbrella `44419048581`, AI docs `44419048640`; dedicated crond lifecycle `44417198647`, inventory `44417198657` passed. MentoHUST source SHA `4f54d9f0b861091c884459bd178ab4b966439719` is XY-C1 L3 PASS on run `44415470536` (Build/TRX/artifact all successful). Crond/IPTV firmware L3 remains open because run `44418643024` skipped Build/TRX/artifact after host tests; the commit changed only CI.
- Local tests: NOT RUN (GitHub connector-only environment). Storage exact-head regression rerun pending; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 07:14 CST (UTC+8) — Add the missing crond policy-header regression trigger

- Parent main SHA: `aabe9a0c1a2574bde10cb7b3d60025db7a57e3b0`.
- Parent commit `aabe9a0c` fixed the shared include directory for the Process and XY-C1 `service_start_status.c` GNU89/ASan/UBSan compile commands. Before that correction, the exact Process and XY runs failed on `optional_daemon_policy.h` -> `process_inventory.h` resolution, not on production source or MIPS compilation.
- The Process workflow still lacked `optional_daemon_policy.h` in both push and pull_request path filters. THIS COMMIT adds the transitive policy header to both sets and makes `check_process_lifecycle_integration.py` enforce symmetric header coverage and both compile commands' shared include path.
- Corrected parent-main Process `44418643022` and XY-C1 `44418643024` were IN_PROGRESS at the last poll; AI docs `44418643233` passed. This follow-up's new Process trigger should run the strengthened source contract on the exact new SHA.
- Local tests: NOT RUN (GitHub connector-only environment). L1 dependency diagnosis PASS; L2/L3 reruns pending; physical-router L4 NOT VERIFIED.

---

## 2026-11-18 — Make crond startup idempotent under repeated RC start requests

- Parent main SHA: `a9d9594dc1875e5dd7adf2f894667d2e6566d76e`.
- L1 found `trunk/user/rc/src/services.c::start_crond()` had detached-child readiness but no initial process inventory before `_eval()`. A repeated start request could start a second BusyBox scheduler, while an UNKNOWN inventory did not block launch.
- THIS COMMIT calls `optional_daemon_start_status("crond", "crond")` before the launcher: RUNNING is a logged no-op, UNKNOWN returns the inventory error, and ABSENT alone reaches `_eval()`. Existing bounded readiness and stop-before-restart behavior remain unchanged.
- Added ABSENT/RUNNING/UNKNOWN/invalid-state assertions to `trunk/user/rc/src/tests/service_start_status.c`; extended `scripts/check_process_lifecycle_integration.py` with an order/branch contract. Existing Process workflow compiles this GNU89 ASan/UBSan test and executes the checker; `services.c` is already in its and XY-C1 path filters.
- MentoHUST L2 on exact source SHA `4f54d9f0b861091c884459bd178ab4b966439719`: Legacy wrapper `44415470631` PASS, AI docs `44415470654` PASS, Process inventory `44415470822` PASS, lifecycle fail-closed `44415470569` PASS. XY-C1 `44415470536` was IN_PROGRESS at last check, so MentoHUST L3 remains pending.
- Local compile/tests: NOT RUN. Exact-head Process lifecycle, process inventory, AI docs and XY-C1 for the crond commit are pending at commit creation. Physical router L4 remains NOT VERIFIED.

---

## 2026-11-19 06:50 CST (UTC+8) — Fix the MentoHUST production LF/CR sentinels

- Parent main SHA: `6cda7c56e6e69b4b1ea38b0d618ab87d0ee46ecf`.
- After the function-separator correction at `b9c9500aac4489f5d1f3841c1db8e3d19095d6e0`, Process lifecycle integration passed run `44414594807`. Legacy wrapper run `44414594963` reached runtime and failed because LF injection was accepted. A follow-up commit `6cda7c56e6e69b4b1ea38b0d618ab87d0ee46ecf` corrected the CR regression fixture to produce a real carriage return and extended the static checker, but exact legacy run `44415154101` still failed at the LF case.
- Inspection confirmed the production `newline=$(printf ...)` and `carriage_return=$(printf ...)` formats still contained two backslashes. THIS COMMIT reduces each format to one, so shell `printf` creates the actual LF and CR control characters matched by the rejection case.
- Exact-head legacy wrapper, Process lifecycle, Process inventory, AI docs and XY-C1 validation remain required. No local shell test was run; L4 physical router remains NOT VERIFIED.

---

## 2026-11-19 06:07 CST (UTC+8) — Protect MentoHUST credential configuration at creation and reuse

- Parent main SHA: `3561edf47144278a2fa386358930cda901206ccf`.
- L1 review found PID 1 calls `umask(0000)` in `init.c`; MentoHUST previously wrote plaintext username/password to `/var/mentohust.conf` using ordinary shell redirections, so the inherited umask could expose credentials to every local user. Existing `/etc/storage/mentohust.conf` files were reused without permission tightening. The old sed-based value substitution also made CR/LF-bearing NVRAM values capable of adding configuration directives.
- THIS COMMIT hardens `trunk/user/mentohust/scripts/mentohust.sh`: set umask 077; write fields literally from a fixed NVRAM-to-config mapping; reject line breaks; create a same-directory mode-0600 temporary file; atomically rename; verify ownership against the executing UID; refuse symlink/directory config targets; and restrict reused regular files before daemon launch. A failed config publication blocks start.
- Extended `scripts/tests/legacy_wrapper_pidfile_safety.sh` with private-mode, literal special-character value, existing-config mode repair, symlink refusal, newline/carriage-return rejection, and no-launch-on-failure cases. Extended `scripts/check_legacy_wrapper_pidfile_safety.py`, including repair of its over-escaped `kill -9` matcher. The existing workflow already covers all touched source/test/checker paths; the XY-C1 workflow already lists the MentoHUST production script.
- Local tests: NOT RUN (GitHub connector-only environment; no local repository checkout). Exact-head legacy wrapper and AI handoff Actions: PENDING at commit creation. XY-C1 Build/TRX/artifact: PENDING at commit creation. Hardware L4: NOT VERIFIED.
- Remaining compatibility boundary: verify BusyBox `stat -c`, `id -u`, `mktemp`, `readlink`, ash behavior and persistent-storage ownership on target; physical XY-C1 runtime remains unverified.

---

## 2026-11-19 06:05 CST (UTC+8) — Close Redsocks heredoc-aware source contract

- Parent main SHA: `81348ba7fc8cc3affcc2fa5a4b48c87ad48b906d`. Shadowsocks lifecycle push run `44406248171` passed after the oversea profile regression PASS marker was correctly read from `tests/start_status.sh`. Earlier exact `5debbf4` run had shell syntax and every runtime regression pass; its source checker failed only on that stale marker location. The `5debbf4` XY-C1 run `44406037894` remained IN_PROGRESS at checkpoint (Build in progress, TRX/artifact pending).
- Redsocks source/test SHA `e01cb8e8a74ed9a22f2b81374679be460dea6bfb`: shell syntax and runtime tests passed, but the checker parser's generic brace scan stopped at a brace-only line inside the config heredoc and missed both start call paths. Change checker extraction to terminate at the next top-level `func_clean(){` declaration; do not weaken the existing callsite or test assertions.
- Update the handoff and roadmap to reflect live main `81348ba`, the Shadowsocks L2 PASS, and the pending Redsocks checker rerun. The archived backup remains untouched. No Redsocks firmware L3 or physical-router L4 claim.

---

## 2026-11-19 05:15 CST (UTC+8) — Verify Redsocks detached startup readiness

- Parent main SHA: `3dd89aa3d401f82ab32216cafb6f896ce458e451`. Exact-main Process inventory `44402419113` and Process lifecycle regression `44402419118` both passed, including the existing WireGuard lifecycle test path.
- `trunk/user/redsocks/redsocks.sh` configured the daemon with `daemon = on`, but both `func_start()` paths treated a successful launch command as proof that the detached child survived and logged `STARTED.` unconditionally. An immediate bind/config/runtime exit could therefore produce a false success.
- Add a bounded tri-state startup wait and shared launch wrapper: only confirmed RUNNING succeeds; ABSENT gets four one-second retries then times out; UNKNOWN or sleep failure aborts; the original launcher error is preserved. Both existing-config and generated-config paths use the same postcondition.
- Extend `scripts/tests/redsocks_lifecycle.sh` for delayed RUNNING, timeout, UNKNOWN, sleep failure, exact launch-status propagation and generated-config startup. Extend `scripts/check_redsocks_lifecycle.py` to require the production integration and test cases. Existing Redsocks workflow push/pull_request paths already cover all changed source/test/checker files; no CI filters are changed.
- No local checkout/test execution is available in this connector-only environment. Exact-head Redsocks Actions are pending until the atomic main commit is pushed. This shell-only add-on change does not trigger or justify a new firmware L3 claim; hardware L4 remains NOT VERIFIED.

---

## 2026-11-19 — Harden privileged WAN callback symlink ownership

- Parent main SHA: `8cfe4a01f116dc9f08baae9c858b3b98f2242749`. Latest WAN teardown checker SHA `8cfe4a01f116dc9f08baae9c858b3b98f2242749` has exact VPN `44393711494` and Process `44393711289` PASS. It follows full WAN teardown status capture from `2c94a4e`.
- Review found `create_cb_links()` ignored all `symlink()` errors at PPP, DHCP, ZeroConf and WPA callback paths; `remove_cb_links()` unconditionally unlinked fixed names under shared `/tmp`. Foreign files or attacker-owned symlinks could remain as privileged callbacks or be deleted by teardown.
- Added `ppp_callback_link_policy.h`: shared `/tmp` must be owned and sticky when shared-writable; callback links must be effective-UID-owned and target exactly `/sbin/rc`; `lstat/readlink` validates type/owner/target; EEXIST is accepted only after exact-link revalidation; cleanup removes only owned matching symlinks.
- WAN startup fails closed, tracks links created during this attempt, rolls back those links on subsequent publication failure and preserves errno. Early cleanup failures are logged. WAN stop propagates callback cleanup failure and skips final WAN state clear when an unsafe callback path remains.
- Added GNU89/Werror ASan/UBSan regressions for sticky-directory protection, idempotent create/remove, regular-file and wrong-target collisions, non-owner exact target, injected symlink failure and exact-target EEXIST race. Local compile/runtime PASS. Exact WAN route/teardown/Process/XY-C1 Actions pending for this source; no MIPS/router L4 claim.
- Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Align WAN teardown lifecycle contracts with exact status capture

- Parent SHA: `bc90e3b44af85fe5ad18a58f8f39bf36e5de680d`. WAN source SHA `bc90e3b44af85fe5ad18a58f8f39bf36e5de680d` exposed stale tests in exact-head WAN `44390606879`, VPN server `44390606952`, HTTPD security `44390607054` and Process lifecycle `44390606840`. The compile/policy test stages of WAN, VPN and HTTPD and prior Process host regressions passed before each static integration assertion failed.
- All four failures were assertions for old PPP stop predicates/return expressions, not compile errors: WAN expected the removed ternary status return; VPN, HTTPD and Process still looked for `stop_vpn_client_under_lifecycle_lock() != 0` after production code started assigning its exact result to `stop_status` before gating.
- Updated the contracts to verify exact result assignment, its subsequent error gate, non-reentrant under-lock helper usage, and the new primary/resume status policy in both WAN abort paths. No production code and no CI triggers/path filters were changed.
- XY-C1 run `44390607038` was in progress at last inspection. No L3 claim for the WAN source SHA; no local tests or physical-router validation performed.

## 2026-11-19 — Preserve WAN PPP teardown and monitor-resume failure status

- Parent SHA: `d8b5e9206024ba45208d95fbb1ff4dc5d73d0b4d`. Exact-head AliDDNS `44388958868`, FRP `44388958846` and AI handoff `44388958941` passed. Autostart production SHA `51596e1acf6b4842b80e1246911aac59e9dd2baf` completed XY-C1 Build, TRX verification and artifact upload `44386912939` (L3).
- WAN teardown review found `stop_wan_ppp_locked()` initialized `stop_status=-1` but never assigned callback-session fencing, VPN client stop or PPP daemon-stop results; all failure paths discarded the initiating status. PPP and full WAN abort handlers also ignored Internet-monitor resume errors.
- Added `wan_teardown_abort_status()` to preserve the original operation failure and surface a resume error when it is the only failure. PPP teardown now captures and logs the exact return from each operation; both abort paths log a failed monitor resume.
- Extended the existing GNU89/Werror C policy regression for primary-error precedence, resume-only failure and fallback. Extended existing `wan-teardown-stability.yml` source contracts. Existing path filters cover all modified files.
- No local compile/test, full firmware build or hardware-router validation was run. Exact-head WAN Actions pending; physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Align AliDDNS and FRP static contracts with shared NVRAM helper

- Parent SHA: `e0e24637fcb2901bef453812747a5f2e7b74f864`. Static-contract repair SHA `e0e24637fcb2901bef453812747a5f2e7b74f864`: AI docs `44388094912` and Process inventory `44388094850` PASS; AliDDNS `44388094895` and FRP `44388094871` failed at static source-check steps.
- AliDDNS checker passed neither the NVRAM change nor a firmware condition: its generic function extractor was given a signature with an opening brace on the same line, while production declares the function and opening brace on separate lines. It now anchors only to the function declaration while retaining status and CA-order assertions.
- FRP checker still required direct `nvram get` assignments after the common checked helper replaced them. It now verifies both helper calls, exact read-status propagation and the combined client/server enable decision; it also requires the failure-injection regression cases.
- FRP extracted shell regression and its status-9 cases passed before its separate static source-contract step failed. No production source or workflow trigger changed. No local tests executed; exact-head CI is required.

## 2026-11-19 — Repair AliDDNS and FRP contracts for shared autostart NVRAM reader

- Parent SHA: `51596e1acf6b4842b80e1246911aac59e9dd2baf`. Production SHA `51596e1acf6b4842b80e1246911aac59e9dd2baf`: Process lifecycle `44386912755`, DNSMasq `44386912932`, AI docs `44386912783`, Process inventory `44386913112` and HTTPD `44386912691` passed. XY-C1 `44386912939` was IN_PROGRESS; AliDDNS `44386912869` and FRP `44386912971` failed.
- AliDDNS host transport/API suites passed; its static checker failed on a stale literal requirement for the old inline AliDDNS NVRAM expression. Updated it to assert `autostart_nvram_read()` preserves nvram status, the enable wrapper propagates it, and the CA bundle gate remains ordered before launch.
- FRP tests passed lifecycle/watchdog/crontab before the isolated autostart fixture failed with `autostart_nvram_read: not found`: its extractor sourced only FRP functions. The fixture now imports the production reader and injects status 9 for both client/server state reads, asserting no launch and key/status logging.
- No production source or CI trigger changed in this fix; existing workflow filters cover the checker, fixture and common script. No local Python/shell tests were run. Exact-head AliDDNS/FRP and XY-C1 results remain required.

## 2026-11-19 — Report autostart feature-switch NVRAM read failures

- Parent main SHA: `55cf20ecfec45a5b519996c85921bce722e54d99`. DDNSTO source SHA `55cf20ecfec45a5b519996c85921bce722e54d99` passed exact-head DDNSTO `44385101598`, Process inventory `44385101669` and AI docs `44385101578`; XY-C1 `44385101749` was still building. RStats source SHA `1bf9a30d1003b9ad5835c86a6dfaeab1fae15961` passed exact-head RStats/Process/HTTPD/AI/NVRAM and XY-C1, with Build/TRX/artifact complete.
- `autostart.sh` expanded feature NVRAM reads directly inside test expressions. Failed reads were silently treated as disabled switches, hiding why an enabled optional service was not launched. FRP returned a generic failure without identifying the failed key.
- Added status-preserving `autostart_nvram_read()` and boolean `autostart_nvram_enabled()`; migrated every boot feature guard and FRP client/server read. On command failure, the dependent service remains fail-closed and the key/original status are logged; independent launchers continue.
- Extended the existing host shell regression with injected status-9 cases for AdGuardHome, DDNSTO and FRP. It verifies the affected service is skipped, unrelated services still start, and precise NVRAM/FRP failure status is logged. Existing Process and XY-C1 path filters already include both modified files; no workflow was broadened or disabled.
- No local shell test, full repository build or MIPS firmware build was performed. Exact-head Process lifecycle and XY-C1 verification for this source SHA are pending; physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Preserve DDNSTO on NVRAM enable-state read failure

- Parent main SHA: `1bf9a30d1003b9ad5835c86a6dfaeab1fae15961`. The preceding RStats history-read commit passed RStats `44384384582`, Process inventory `44384384359`, HTTPD `44384384054` and AI docs `44384384820`; Process fail-closed `44384384017`, XY-C1 `44384384072` and NVRAM `44384384235` were still in progress at implementation start.
- DDNSTO review found that `[ "$(nvram get ddnsto_enable)" = "1" ]` discarded the command's exit status. A read error was treated as a successful disabled state; restart additionally stopped the existing daemon before making the unchecked read, risking an avoidable service outage.
- Added `ddnsto_read_enable_state()` to preserve the original NVRAM failure status and use one checked result for start/restart under the existing lifecycle lock. Restart now completes the read before TERM/KILL or replacement launch.
- Added host shell regression cases injecting status 9 for both start and restart with an already-running daemon; tests assert preserved status, no signal, no launch, unchanged daemon state and exactly one enable read. The DDNSTO source checker verifies helper use and ordering. Existing DDNSTO and XY-C1 workflow path filters already include the packaged launcher, test and checker; no workflow was broadened or disabled.
- Also corrected the roadmap's stale DDNSTO status and recorded the completed five-wrapper RC call-graph audit (no C callers; dispatch uses separate int restart entry points).
- No local shell test, full repository build or MIPS firmware build was performed. Exact-head DDNSTO lifecycle, Process inventory and XY-C1 results for this packaged-source SHA are pending; prior L3 on `3c7bd2d` does not transfer. Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Harden RStats raw history loading against short reads and EINTR

- Parent main SHA: `1b3117769d52021de09ba09ffbee0118fae159de3fe`. `rstats.c::f_read()` previously performed one `read()` and returned its byte count; legal short reads caused `load_history_raw()` to reject otherwise valid history, and EINTR also aborted the read.
- Add injectable `rstats_file_read_exact_with()` / `rstats_file_read_exact()` helpers: accumulate short reads, retry EINTR, return the actual short count at EOF, reject invalid arguments and oversized lengths, and propagate non-EINTR failures. `f_read()` retries interrupted `open()`, uses the exact-read helper and preserves the read errno across `close()`.
- Add RStats fixture tests for one EINTR followed by short chunks, truncated EOF, EIO after partial progress, zero-length input, invalid arguments and oversized request. Extend `rstats-stability.yml` source-contract checks; existing push/PR filters already watch all modified source/test paths.
- A standalone helper harness compiled and passed under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. This does not replace the full repository test or firmware build. New exact-head RStats/Process/AI/XY-C1 Actions must be inspected; XY-C1 L3 requires Build, TRX and artifact success on the exact source SHA. Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Align downstream checks with Inadyn locked helpers

- Inadyn SHA `3c7bd2d28bc6cc285fca3074dee4118fae159f66`: Inadyn regression `44381565885` (lock compile/run + production contract), Process inventory `44381563425`, Process fail-closed `44381563270`, HTTPD `44381563266`, miniupnpd `44381563068` and Samba `44381563561` PASS.
- WAN route `44381563038` failed because `scripts/check_wan_ddns_status.py` still parsed `notify_ddns_update(void)` rather than `notify_ddns_update_locked(void)`. DNSMasq lifecycle `44381563474` failed because its embedded checker expected the former public start/update bodies and nested stop call. This follow-up adjusts only those source contract signatures/calls, retaining all substantive assertions.
- AI docs `44381563154` failed because the prior handoff splice dropped the required `## Verification boundary` section. Restore it and record current exact-SHA evidence.
- XY-C1 `44381563540` remains IN_PROGRESS (Build active; TRX/artifact pending); NVRAM audit `44381563037` was pending. No firmware L3 or physical-router L4 claim.

## 2026-11-19 — Serialize Inadyn stop/start and configuration notifications

- Base main SHA: `b471b7e328b79c9eac412f2af701c51b9a573e19`. AI handoff documentation run `44377784741` PASS after restoring the literal Active task heading.
- Review found public `stop_ddns`, `start_ddns`, `notify_ddns_update` and `notify_ddns_manual_update` had no shared process lock. Concurrent RC processes could race stop/start for the `--no-pidfile` daemon or race config publication/reload/manual refresh against restart.
- Add root-owned `/tmp/.inadyn.lifecycle.lock` via `fcntl`; validate regular-file type, effective UID, single link and restrictive mode; open with `O_NOFOLLOW|O_CLOEXEC`; bound contention to 10 seconds. All public operations hold one lock through their transaction; internal `_locked` helpers avoid recursive acquisition.
- Add fork-based host regression for cross-process exclusion, release/reacquisition, hardlink/symlink rejection and invalid paths; compile/run it with GNU89, ASan and UBSan in the Inadyn workflow. Add explicit lock paths to XY-C1 and Process inventory workflow filters.
- Inadyn source SHA `3c7bd2d28bc6cc285fca3074dee4118fae159f66`: Inadyn `44381565885`, Process inventory `44381563425`, Process fail-closed `44381563270`, HTTPD `44381563266`, miniupnpd `44381563068` and Samba `44381563561` PASS. WAN route `44381563038` and DNSMasq `44381563474` failed due downstream checkers still expecting the pre-lock function bodies; AI docs `44381563154` failed due missing `## Verification boundary`. This follow-up aligns the checkers and restores the heading. XY-C1 `44381563540` remains IN_PROGRESS; NVRAM `44381563037` was also pending. L3/L4 are not claimed.

