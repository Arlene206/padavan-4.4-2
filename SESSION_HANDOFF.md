# Cross-session execution mandate

## 2026-11-21 — Current development checkpoint (newest; supersedes earlier status notes)

### Current repository state
- Main at checkpoint parent SHA: `77f7a711c576f4cc50f78ea5307a2bd561058aae`.
- Backup branch remains the frozen recovery branch; do not develop on it.
- Recent main history:
  - `77f7a711c576` fix(ppp): snapshot ip-up WAN state under publish lock
  - `efc579b8c3ba` fix(wan): snapshot DHCP renewal state with checked reads
  - `91697917a749` fix(wan): snapshot IPTV and zeroconf rollback state
  - `cc4887aa2cab` fix(wan): snapshot DHCP rollback state transactionally
  - `a9e0a055051f` fix(ppp): restore PPP MTU default local
  - `4377426a5b1e` fix(ppp): accept L2TP text option snapshots
  - `12251c76bb27` test(ci): fix PPP credential source checker typo
  - `513a2e16d3c7` fix(ppp): read only protocol-specific WAN text options
  - `749dfc34d456` test(ppp): align credential checker with snapshots
  - `a6cd934993c7` test(ppp): assert checked L2TP credential copies
- Exact-head Actions at checkpoint:
- #44910480110 NVRAM transaction boundary audit: IN_PROGRESS (77f7a711c576)
- #44910480109 WAN route operation status regression: FAILURE (77f7a711c576)
- #44910480105 Process lifecycle inventory - 77f7a711c576f4cc50f78ea5307a2bd561058aae: SUCCESS (77f7a711c576)
- #44910480086 VPN server lifecycle stability regression: SUCCESS (77f7a711c576)
- #44910480066 HTTPD Authentication and Security Regression: FAILURE (77f7a711c576)
- #44910480059 WAN DHCP callback state stability regression: FAILURE (77f7a711c576)
- #44910480057 XY-C1 Firmware Validation: IN_PROGRESS (77f7a711c576)
- Latest relevant workflow runs (may refer to earlier source SHAs):
  - #44910480110 NVRAM transaction boundary audit: IN_PROGRESS on `77f7a711c576`
  - #44910480109 WAN route operation status regression: FAILURE on `77f7a711c576`
  - #44910480066 HTTPD Authentication and Security Regression: FAILURE on `77f7a711c576`
  - #44910480059 WAN DHCP callback state stability regression: FAILURE on `77f7a711c576`
  - #44910480057 XY-C1 Firmware Validation: IN_PROGRESS on `77f7a711c576`
  - #44910277250 WAN DHCP callback state stability regression: FAILURE on `efc579b8c3ba`
  - #44910277143 WAN route operation status regression: FAILURE on `efc579b8c3ba`
  - #44910277121 HTTPD Authentication and Security Regression: FAILURE on `efc579b8c3ba`
  - #44910277100 Wireless radio state regression: SUCCESS on `efc579b8c3ba`
  - #44910277047 XY-C1 Firmware Validation: IN_PROGRESS on `efc579b8c3ba`
  - #44910277045 NVRAM transaction boundary audit: SUCCESS on `efc579b8c3ba`
  - #44910097464 Wireless radio state regression: SUCCESS on `91697917a749`
  - #44910097413 XY-C1 Firmware Validation: FAILURE on `91697917a749`
  - #44910097405 NVRAM transaction boundary audit: SUCCESS on `91697917a749`
  - #44910097396 WAN DHCP callback state stability regression: SUCCESS on `91697917a749`
  - #44910097353 WAN route operation status regression: SUCCESS on `91697917a749`
  - #44910097352 HTTPD Authentication and Security Regression: SUCCESS on `91697917a749`
  - #44909870768 Wireless radio state regression: SUCCESS on `cc4887aa2cab`
  - #44909870702 WAN DHCP callback state stability regression: SUCCESS on `cc4887aa2cab`
  - #44909870687 XY-C1 Firmware Validation: IN_PROGRESS on `cc4887aa2cab`
  - #44909870648 HTTPD Authentication and Security Regression: SUCCESS on `cc4887aa2cab`
  - #44909870603 NVRAM transaction boundary audit: SUCCESS on `cc4887aa2cab`
  - #44909870590 WAN route operation status regression: SUCCESS on `cc4887aa2cab`
  - #44909446644 WAN DHCP callback state stability regression: SUCCESS on `a9e0a055051f`
  - #44909446636 HTTPD Authentication and Security Regression: SUCCESS on `a9e0a055051f`
  - #44909446633 NVRAM transaction boundary audit: SUCCESS on `a9e0a055051f`
  - #44909446600 XY-C1 Firmware Validation: SUCCESS on `a9e0a055051f`
  - #44909446562 WAN route operation status regression: SUCCESS on `a9e0a055051f`

### Source work completed in this continuation
- PPP/WAN checked configuration path: integer settings are read with checked status before PPP session invalidation or options-file creation; protocol-specific text options are copied into bounded local snapshots; PPTP peer fallback is checked; PPP default-route ownership uses a checked `primary` read.
- Fixed the PPP text-snapshot protocol guard so L2TP (both PPPoE/PPTP flags false) remains valid; tests cover invalid flags and protocol-specific option reads.
- Fixed the XY-C1 compile blocker in `net_ppp.c` by restoring the `mtu_def` local declaration.
- `udhcpc_bound` now snapshots the previous DHCP rollback tuple through `rc_wan_nvram_copy_snapshot` under the NVRAM transaction instead of independent `nvram_safe_get` calls.
- IPTV DHCP, ZeroConf WAN and ZeroConf IPTV callbacks now use the same transactionally coherent rollback snapshot pattern before interface mutation.
- PPP ip-up now reads the previous five-field WAN tuple and publishes the new address tuple under the same NVRAM transaction. A preflight read failure does not enter the restore path because no write was attempted.
- Source regression checks were extended for the new callback integration. The generic snapshot policy already covers coherent copies, missing keys, oversize fields, backend failures and lock cleanup.

### Validation / honesty
- Exact-SHA WAN DHCP callback stability run on the first DHCP snapshot commit `cc4887aa2cab`: PASS.
- Exact-SHA WAN DHCP callback stability run on the IPTV/ZeroConf extension commit `91697917a749`: PASS.
- PPP ip-up same-lock snapshot commit `77f7a711c576`: inspect its exact-head WAN-route/security/build workflows below before claiming validation.
- Current XY-C1 build/TRX/artifact status is determined only by the matching exact-SHA run above; do not infer L3 from source tests alone. No hardware/L4 claim is made.
- Historical run sets for the three recent source checkpoints:
  - `77f7a711c576`:
    - #44910480110 NVRAM transaction boundary audit: IN_PROGRESS
    - #44910480086 VPN server lifecycle stability regression: SUCCESS
    - #44910480105 Process lifecycle inventory - 77f7a711c576f4cc50f78ea5307a2bd561058aae: SUCCESS
    - #44910480066 HTTPD Authentication and Security Regression: FAILURE
    - #44910480059 WAN DHCP callback state stability regression: FAILURE
    - #44910480057 XY-C1 Firmware Validation: IN_PROGRESS
    - #44910480109 WAN route operation status regression: FAILURE
  - `91697917a749`:
    - #44910097216 Router mode NVRAM update failure regression: SUCCESS
    - #44910097464 Wireless radio state regression: SUCCESS
    - #44910097425 ZeroTier lifecycle stability regression: SUCCESS
    - #44910097290 WAN interface teardown stability regression: SUCCESS
    - #44910097468 DNSMasq lifecycle stability regression: SUCCESS
    - #44910097295 watchdog signal lifecycle stability regression: SUCCESS
    - #44910097404 USB modem WDM/NDIS physical identity stability: SUCCESS
    - #44910097322 WAN gateway ARP probe socket stability regression: SUCCESS
    - #44910097462 miniupnpd atomic configuration publication regression: SUCCESS
    - #44910097203 Firewall Startup Stability Regression: SUCCESS
    - #44910097396 WAN DHCP callback state stability regression: SUCCESS
    - #44910097353 WAN route operation status regression: SUCCESS
    - #44910097232 detect_link signal lifecycle stability regression: SUCCESS
    - #44910097428 VPN server lifecycle stability regression: SUCCESS
    - #44910097258 Process lifecycle inventory - 91697917a7498e9f67f2605e3419c277cdc73ab7: SUCCESS
    - #44910097245 detect_internet signal lifecycle stability regression: SUCCESS
    - #44910097200 Process inventory and lifecycle fail-closed regression: SUCCESS
    - #44910097352 HTTPD Authentication and Security Regression: SUCCESS
    - #44910097405 NVRAM transaction boundary audit: SUCCESS
    - #44910097413 XY-C1 Firmware Validation: FAILURE
  - `cc4887aa2cab`:
    - #44909870687 XY-C1 Firmware Validation: IN_PROGRESS
    - #44909870714 VPN server lifecycle stability regression: SUCCESS
    - #44909870768 Wireless radio state regression: SUCCESS
    - #44909870702 WAN DHCP callback state stability regression: SUCCESS
    - #44909870751 Process lifecycle inventory - cc4887aa2cab4de68c9e99b6587ffa7d251c317d: SUCCESS
    - #44909870712 WAN interface teardown stability regression: SUCCESS
    - #44909870736 USB modem WDM/NDIS physical identity stability: SUCCESS
    - #44909870766 watchdog signal lifecycle stability regression: SUCCESS
    - #44909870728 detect_internet signal lifecycle stability regression: SUCCESS
    - #44909870606 detect_link signal lifecycle stability regression: SUCCESS
    - #44909870674 ZeroTier lifecycle stability regression: SUCCESS
    - #44909870604 Firewall Startup Stability Regression: SUCCESS
    - #44909870571 DNSMasq lifecycle stability regression: SUCCESS
    - #44909870664 miniupnpd atomic configuration publication regression: SUCCESS
    - #44909870590 WAN route operation status regression: SUCCESS
    - #44909870573 Router mode NVRAM update failure regression: SUCCESS
    - #44909870617 WAN gateway ARP probe socket stability regression: SUCCESS
    - #44909870648 HTTPD Authentication and Security Regression: SUCCESS
    - #44909870621 Process inventory and lifecycle fail-closed regression: SUCCESS
    - #44909870603 NVRAM transaction boundary audit: SUCCESS

### First task for the next GPT
1. Inspect all exact-head Actions for the current main SHA, especially PPP/WAN route, HTTPD security, XY-C1 firmware/TRX/artifact upload, and NVRAM transaction audit. Diagnose any first failure without weakening test coverage; while any are pending, continue an independent source-level reliability fix.
2. Keep the transactional snapshot work intact. The important invariant is that the previous tuple must be copied and the replacement published under the same NVRAM lock where practical; separate snapshot/publish transactions still permit a lost-update race.
3. Do not repeat the already completed startup audit or re-run historical passing checks unless a later source SHA invalidates them.

---


## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Current main before this RT3352 compile-safety correction: `7f5b2a1676aca910352ac9106c3c8289384aa963`. Product source includes PPP checked-option work from `bf43b52d3039ff89a882b8260eca4823c92b0ae2`, workflow path-filter correction `2e960bf`, and route status checker alignment `7f5b2a1`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Only main and backup branches exist.
- PPP WAN operation status Action #44908223067 PASS on exact main SHA `7f5b2a1`; AI handoff docs #44908223050 PASS on the same SHA. NVRAM audit #44907884214 and Wireless radio #44907884270 PASS on product SHA `bf43b52`; Process lifecycle #44907884319 and process inventory #44907884347 PASS on `bf43b52`; NFS/HTTPD/USB storage checks passed on the same source SHA.
- XY-C1 #44907884176 is still in progress against production-source SHA `bf43b52`; its Build XY-C1 step was in progress, with TRX verification and artifact upload pending at last inspection. No L3 claim.
- RT3352 MII iNIC checked-disable source fix in `75d7dee` assigned the result of `start_inic_mii(inic_enabled)` to `operation_status` under `USE_RT3352_MII` but did not declare that identifier in `start_wifi_ap_rt()` scope. The identifier is not declared in `rc.h` or the wireless policy header. This commit declares a function-local variable within the same conditional block and adds a source contract requiring it; behavior is otherwise unchanged. This closes a target-configuration compile blocker that the stand-alone policy tests do not compile.
- Existing completed source protections: transactional router-mode NVRAM update; checked NFS start/reload/restart; HTTPD hardening; USB storage gates; bounded MiniUPnP internal-port parser; RT3352 TxPower normalization; iNIC checked config preflight; checked SSH start/restart; PPP typed-option preflight and checked default-route primary. Do not repeat those audits absent fresh failure evidence. Physical router L4 remains NOT VERIFIED.

## Active task
1. Inspect exact-head Wireless radio and XY-C1 Actions triggered by this RT3352 compile correction. Resolve the first actual compiler or source-contract failure, if any.
2. Inspect XY-C1 Build, TRX verification and artifact upload on the new product-source SHA. Claim L3 only if all three pass on that same SHA.
3. Continue a separate confirmed P1 reliability/security issue after checking current main and preserving the existing regression set.

## Verification boundary
- PPP WAN route operation status and AI handoff documentation PASS on `7f5b2a1`.
- Wireless radio, Process lifecycle/inventory and NVRAM audit PASS on prior production-source SHA `bf43b52`; these do not validate the new RT3352 local declaration.
- XY-C1 Build remains in progress on `bf43b52`; TRX/artifact pending. This source correction should trigger a new same-SHA build. No current L3 or physical L4 claim.
- No local target cross-compile or physical router test is claimed.

## Exact first action for the next GPT
Fetch live main and inspect exact-head Wireless and XY-C1 runs for this RT3352 source correction. Review the actual compile log for any next failure; do not wait on pending jobs before continuing independent source work.

Before context exhaustion, update this handoff only when active task, exact-head result, verification boundary or first action materially changes; no no-op commit.