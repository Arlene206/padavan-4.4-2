## 2026-11-21 — Restore the WAN DHCP publisher forward declaration and remove duplicate definition

- Parent main SHA: 188a2b85bc0b80246bfc1a37f2030f7ceec85e29; the latest product-source baseline had failed XY-C1 Build #44913834107.
- The MIPS compiler found a real blocker in net_wan.c: an earlier prototype at line 6127 had been accidentally replaced by a full wan_dhcp_nvram_state_publish definition in commit 114a7711. The existing implementation remained later in the same translation unit. This produced an undeclared transaction-set callback, an implicit restore helper declaration, and a duplicate static definition. The first block is restored to its forward declaration; the single lower implementation retains compare-and-publish and guarded stale-writer rollback behavior.
- Extended the WAN callback source contract to require exactly one function definition plus the forward declaration, normalized the multiline reverse-snapshot call, and aligned the rollback assertion with the lower implementation's publish_attempted && status != 0 guard.
- Previous XY-C1 run #44913834107 on source baseline 9a25b71 failed at net_wan.c before Build completion; TRX verification and artifact upload were skipped. NVRAM transaction audit #44913834104 passed on that baseline. WAN callback #44914630325 failed at the multiline reverse-snapshot source assertion; Process #44914630350 was pending at last inspection. Exact-head reruns after THIS COMMIT are required.
- No claim of local compilation, L3 firmware validation, or hardware L4 validation.

---

## 2026-11-21 — Normalize WAN6 and detached-daemon source contracts

- Parent main SHA: `e86520924f8e4bee54ba744d7506d9966e5e4291`.
- Exact-head WAN callback #44914227457 reached the WAN6 policy checker and failed because its reverse-snapshot assertion expected one-line spacing while the production helper call is intentionally wrapped over three C lines. The helper call and guarded rollback are present; the checker now compacts whitespace before comparing.
- Process integration #44914308142 failed in its generic detached-daemon probe loop because it extracted public `start_ttyd()` (the NVRAM mode wrapper) while checking startup waits that belong to `start_ttyd_with_mode()`. Pointed the probe contract at the actual launch helper and made the call assertions whitespace-independent; the public wrapper's NVRAM-read ordering remains covered by separate checks.
- The policy regression and WAN callback source checks before the WAN6 assertion passed on #44914227457. Exact corrected WAN/Process runs and XY-C1 Build/TRX/artifact are still required.
- Next: verify these exact-head checker changes, then fix the first genuine source/build regression.

---

## 2026-11-21 — Align PID 1 optional-daemon queue contract with mode wrappers

- Parent main SHA: `a275bd99a3f4c3ac6c0ab67b7ddcb9a81eff27d5`.
- The PID 1 notification queue workflow's source contract still assumed SCUT, MentoHUST, vlmcsd and iperf3 launch functions directly contained `optional_daemon_start_status()` and called `return start_<daemon>()`. The implementation now deliberately separates the checked NVRAM mode wrapper (`start_<daemon>()`) from the process-presence launch helper (`start_<daemon>_with_mode()`), and boot invokes the public wrapper without returning it from its own source file.
- Reworked the contract to verify the actual boundaries: boot calls each public wrapper; each wrapper checks NVRAM mode before delegating; each mode helper performs tri-state presence checks and propagates start errors; stop handlers retain their postcondition checks.
- This is a stale static-contract failure from PID 1 notification queue run #44913692593, not a product-code failure. The workflow path includes its own YAML, so the corrected contract will run on this commit.
- Next: verify the queue contract, Process inventory integration and WAN DHCP/XY-C1 validation.

---

## 2026-11-21 — Point ttyd startup postcondition checks at the launch helper

- Parent main SHA: `f405b71f7a6b5c8504da9296b73914c9328fd415`.
- The second Process integration assertion now extracted the public `start_ttyd()` wrapper, but the assertions immediately below check launch status, retries, and detached-child postconditions that live in `start_ttyd_with_mode(int ttyd_mode)`. The checker therefore failed at its first declaration assertion after successful extraction.
- Changed that block to extract the actual launch helper with the existing balanced C-function extractor. The public wrapper's checked NVRAM mode-read ordering remains independently asserted.
- Exact-head Process integration #44914072650 failed at this mismatched function body; rerun on the new SHA is pending.
- Next: verify the Process regression and inspect XY-C1 Build/TRX/artifact on the current source SHA.

---

## 2026-11-21 — Centralize and test reverse DHCP lease tuple construction

- Parent main SHA: `0b51276c02bbbcdde34de71435a1569dbd643b23`.
- The post-commit 6rd failure path manually swapped each lease value and prior value in `net_wan.c`. That duplicated tuple semantics in production and left the transformation itself untested.
- Added `dhcp_lease_nvram_reverse_snapshot()`, which validates the complete source tuple before writing any destination field, rejects aliasing, and creates a reversed candidate/prior vector. The WAN callback now uses this helper before guarded compare-and-publish recovery.
- Added GNU89 regression coverage for successful reversal, malformed late-entry rejection without partial output, zero count, null destination, and aliased vectors. Updated WAN callback and WAN6 source contracts to require the shared helper.
- Exact-head WAN callback #44913833976 had failed at a now-obsolete 6rd status assertion; the corrected contract is included here and will be exercised on this source SHA. XY-C1 build remains pending after the prior stale accumulator compile fix.
- Next: verify WAN callback, GNU89 policy, XY-C1 Build, TRX and artifact on this SHA.

---

## 2026-11-21 — Correct the remaining ttyd integration checker extractor

- Parent main SHA: `74b1381c7e4c66a9095d046b9ae1f99a239144df`.
- The first ttyd source-contract block now uses the whitespace-normalized function helper, but a second, older integration block still extracts `start_ttyd` using a non-existent line break between the return type and function name. Process regression #44913870486 reached that stale extractor and failed with `IndexError`.
- Updated the second extractor to match the actual `int start_ttyd(void)` declaration. The detailed startup ordering and checked mode-read assertions remain intact.
- A fresh Process integration run is pending this checker-only commit.
- Next: verify this Process run and the corrected WAN callback/XY-C1 builds.

---

## 2026-11-21 — Align WAN callback source contract with restored 6rd abort flow

- Parent main SHA: `6085986f64e65078c8116d7398c5fb62139ea00d`.
- After removing the stale undeclared `ip6rd_status` accumulator (the compile error from the prior XY-C1 attempt), the WAN callback workflow still asserted that the removed variable was accumulated into the callback result. The exact-head WAN run therefore failed its static contract at that obsolete assertion.
- Updated the contract to require the old, intended all-or-nothing 6rd failure path and forbid the stale accumulator. The callback still aborts through guarded lease-tuple restoration and route rollback.
- Exact-head WAN callback #44913833976 failed only at this outdated source assertion; rerun on the new SHA is pending.
- Next: check XY-C1 Build/TRX/artifact and the normalized ttyd lifecycle checker run.

---

## 2026-11-21 — Make ttyd enable-reader checker whitespace-independent

- Parent main SHA: `9a25b71fb74932dc27063432b2f342d9724d60e2`.
- The Process inventory contract compared an exact single-line literal against a deliberately wrapped multi-line `service_enabled_mode_status()` call in `start_ttyd()` and `restart_ttyd()`. This produced a false failure even though the checked read precedes all start/stop operations.
- Normalize whitespace in the ttyd wrapper bodies and retain the existing ordering/fail-closed assertions.
- Exact-head Process regression #44913492523 failed only on this raw-whitespace assertion; a new Process run on this corrected checker is pending.
- Next: verify the latest Process integration result and keep the XY-C1 build attempt focused on the fixed WAN DHCP source.

---

## 2026-11-21 — Remove stale 6rd status from WAN bound callback

- Parent main SHA: `3f5b12d138085b21c4919e3f5596a4bfa668ca4c`.
- The XY-C1 cross-build for `943b2a3` reached `net_wan.c` and failed at line 6546 because an obsolete `ip6rd_status` accumulator remained after the callback was restored to the all-or-nothing `snapshot_status` path. The variable was no longer declared or assigned.
- Removed the one stale accumulator line. The 6rd writer failure still enters the guarded reverse-tuple rollback path; the callback result continues to use the existing snapshot/route failure statuses.
- The prior exact-head WAN callback static test passed, but Build had failed before TRX/artifact; reruns on the corrected SHA are pending.
- Next: verify GNU89 compile and Build/TRX/artifact on one SHA.

---

## 2026-11-21 — Correct the conditional-restore read-error fixture

- Parent main SHA: `114a7711d29303b7b860ab31bdf4910dfd826a7b`.
- The new stale-writer case intentionally leaves a third-party gateway value in the fake NVRAM store. Reset the full tuple to the expected candidate before injecting the later read error so that the regression reaches the backend-error branch rather than returning `EAGAIN` early.
- Test-only correction; production behavior is unchanged. Exact-head DHCP callback and XY-C1 Actions for the previous source commit remain the verification targets.
- Next: confirm the GNU89/ASan/UBSan conditional-restore regression and inspect same-SHA firmware build results.

---

## 2026-11-21 — Guard failed DHCP tuple recovery against newer callbacks

- Parent main SHA: `943b2a30bfe7ede0ae6d70e7cd98771c5420187c`.
- The first-transaction publisher already compensates failed writes in reverse order under its exclusive NVRAM lock, but the wrapper retried restoration unconditionally after releasing that lock. A newer DHCP callback could commit in the gap and then be overwritten by the stale retry.
- Added `dhcp_lease_nvram_restore_if_current()`: under the recovery transaction it accepts only fields still equal to this attempt's candidate or captured prior values (including a partially compensated tuple), refuses any distinct newer value with `EAGAIN`, and never re-applies the candidate after a failed restore.
- Changed the WAN wrapper to run fresh recovery only when in-lock compensation or transaction close reported an error, avoiding redundant NVRAM writes when the first rollback already succeeded. Added GNU89 regression coverage for candidate, partial-restore, newer-writer and read-error cases; WAN source contract now checks the guarded recovery path.
- Exact-head DHCP callback and XY-C1 workflows are pending; Build/TRX/artifact results must be checked on the same SHA.
- Next: inspect current WAN DHCP/XY-C1/NVRAM audit results and continue any independent reliability fix.

---

## 2026-11-21 — Make post-commit 6rd failure roll back the committed DHCP tuple safely

- Parent main SHA: `36b227a751934a9efafb42884e7795cb86fcc271`.
- Review of the existing DHCPv6 DNS contract showed that a failed 6rd NVRAM write is intended to abort the bound transition, not silently keep only the IPv4 half. Restored the original abort semantics and fixed the missing NVRAM compensation: when the IPv4 tuple has already committed, build a reverse tuple and use the same compare-and-publish transaction before restoring the interface/routes.
- Reverse publication verifies every current field still matches this callback's committed tuple. A newer callback wins with `EAGAIN`; in that case the failure path preserves current interface/routes rather than overwriting newer state.
- Extended GNU89/ASan/UBSan policy coverage for successful reverse publication and stale-writer rejection, plus WAN callback and WAN6 source contracts. Exact-head WAN callback and XY-C1 validation are pending.
- Next: fix any subsequent checker/build issue and require Build, TRX and artifact upload on one SHA.

---

## 2026-11-21 — Keep committed IPv4 lease when optional 6rd metadata fails

- Parent main SHA: `6144640848b15f47a8e6df2e9f679421a45fcddc`.
- The DHCP bound path published the complete IPv4 lease tuple, then attempted an optional DHCP 6rd metadata write. A failed 6rd NVRAM update jumped to the pre-publication rollback label, reverting the interface/routes while leaving the already committed IPv4 tuple in NVRAM.
- Changed 6rd failure handling to keep the accepted IPv4 lease and continue the IPv4 route/MTU path, while logging and accumulating the 6rd status into the callback result. The optional IPv6 failure remains observable; it no longer treats the committed IPv4 lease as uncommitted.
- Extended the WAN callback source contract to require that 6rd failures do not enter the old lease rollback path and that their status remains observable. Exact-head WAN callback and XY-C1 firmware validation are pending.
- Follow-up: review the IPv6 6rd metadata writer's recovery details and inspect same-SHA Build/TRX/artifact results.

---

## 2026-11-21 — Align WAN source contracts with guarded DHCP publication

- Parent main SHA: `59268855734cc173815b1a99dc5aebd8ac1ea507`.
- Exact-head WAN callback Actions exposed two source-check regressions in the compare-and-publish slice: the inline callback contract referenced lease-policy variables before loading their files, and `check_wan_firewall_status.py` still required the old direct `dhcp_lease_nvram_publish()` call inside the WAN wrapper.
- Fixed the workflow variable declarations and asserted that the guard executes strictly between transaction begin/end. Updated the firewall contract to inspect the policy helper and require checked snapshot comparison, guarded publication, attempt-gated recovery and a held transaction; no behavior or coverage was removed.
- Exact-head WAN callback #44912297779 failed on the undefined Python variable. Exact-head XY-C1 #44912297675 failed its pre-build firewall source contract ("WAN DHCP renewal snapshot needs transaction and full rollback"), so Build/TRX/artifact were not reached. These are source-check failures, not product-build results.
- Next: rerun WAN callback and XY-C1 on the corrected SHA; diagnose any next failure and inspect the NVRAM boundary audit. Do not declare L3 until Build, TRX and artifact upload all pass on one SHA.

---

## 2026-11-21 — Reject stale WAN DHCP rollback snapshots before publication

- Parent main SHA: `ef85dc0c5528b7a0e6e5fe87767ee0c515c42453`.
- Follow-up review found a TOCTOU gap in DHCP/IPTV/ZeroConf callback rollback handling: callbacks captured a checked prior tuple under a transaction, performed interface/route operations, then published that candidate under a later transaction without checking whether another callback had already changed the tuple. On a conflict, the old compensation path could overwrite the newer callback's committed state.
- Added an under-lock compare-and-publish guard. It checks every expected prior value under the same exclusive transaction used for publication and returns `EAGAIN` on a stale tuple. The publisher records whether writes actually began and only runs recovery for a failed attempted publication; stale/read-failed preflight never restores stale data.
- Added GNU89/ASan/UBSan policy regressions for current snapshots, stale-key conflicts with zero writes, checked read errors, and missing-key empty defaults. Extended the WAN callback workflow source contract. A standalone GNU89/ASan/UBSan harness passed locally; the checked-in full policy test and exact-head Actions are pending.
- The earlier optional-daemon enable reader implementation defect was fixed in source commit `c812c6759791`; exact-head Process run on that source is tracked separately.
- Next: inspect exact current-head Process/WAN/NVRAM/XY-C1 results; fix first failure, and require Build + TRX + artifact upload before claiming L3.

---

## 2026-11-21 — Restore checked optional-daemon enable reader

- Parent main SHA: `ac05b377f8cb4cd28ac68f426bf6280bb6abeb6c`.
- The optional-daemon enable-read change introduced checked SCUT/MentoHUST start/restart call sites and a static reader contract, but the reader definition itself was absent. The Process workflow therefore failed at its exact-head source check (#44911903470 on `ac05b377`), and the source also lacked the implementation needed by the new call sites.
- Added `service_enabled_mode_status()`: it reads the configured mode through `nvram_get_checked()`, preserves the legacy disabled default for a genuinely absent key, parses backend results through `rc_wan_nvram_read_int_status()`, and preserves the current daemon state on read/parse failure.
- Existing GNU89 policy regression covers success, disabled default, backend errors and output preservation. The Process source contract verifies the checked reader and call ordering; exact-head reruns are pending this source commit.
- Next independent task: close the WAN DHCP rollback-snapshot TOCTOU window with an under-lock compare-and-publish guard; do not revert transactional snapshots.

---

## 2026-11-21 — PPP and WAN rollback-state consistency

- Main parent checkpoint: `77f7a711c576f4cc50f78ea5307a2bd561058aae`.
- `cc4887aa2cab`: replaced independent previous-lease `nvram_safe_get` calls in `udhcpc_bound` with a checked transaction snapshot before interface mutation.
- `91697917a749`: extended transaction snapshots to IPTV DHCP, ZeroConf WAN and ZeroConf IPTV; the shared DHCP callback source checker now asserts all four callbacks snapshot before applying link changes. Exact WAN DHCP callback Actions: PASS.
- `77f7a711c576`: PPP ip-up now copies its previous WAN address tuple and publishes the new tuple while holding one NVRAM transaction; recovery is attempted only if publication started.
- PPP source work also includes checked integer/text option preflight, protocol-scoped string reads, checked primary-route handling and the `mtu_def` build fix.
- Exact-head status at the checkpoint is captured in `SESSION_HANDOFF.md`; pending build/security/route checks must not be reported as PASS until their exact run completes.
- Next: inspect exact current-head Actions and fix any first failure; then continue a separate source-level reliability defect.
 
---

## 2026-11-21 — Declare RT3352 iNIC startup result before propagation

- Parent main SHA: `7f5b2a1676aca910352ac9106c3c8289384aa963`; product base includes the PPP checked-option and route-primary work from `bf43b52`.
- While reviewing the earlier iNIC checked-read startup path, found that `start_wifi_ap_rt()` assigns `operation_status = start_inic_mii(inic_enabled)` under `USE_RT3352_MII` without declaring that identifier in this function's scope. The other use is local to a different function, and neither `rc.h` nor `wireless_radio_policy.h` declares it. Added the missing function-scope local in the RT3352 declaration block; behavior is unchanged.
- Extended the Wireless source contract to require the local declaration alongside helper result propagation. This closes a target-configuration compile blocker that the policy-only unit test cannot catch. Exact-head Wireless and XY-C1 validation pending.
- WAN PPP route operation status #44908223067 and AI docs #44908223050 PASS on exact main SHA `7f5b2a1`. Earlier product SHA `bf43b52`: Wireless #44907884270, Process #44907884319, and NVRAM audit #44907884214 PASS. XY-C1 #44907884176 was still at Build XY-C1 with TRX/artifact pending at last poll. No current L3/L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Restore indentation in Wireless inline source contract

- Parent main SHA: 7344df1f59b04155c8cbffd8a96ee3a34600ac9d; production source is unchanged from 75d7dee28c30bca036a0287c5124dfb475e3b684.
- The workflow event #44907150820 on 7344df1 registered zero jobs. Inspection of the committed YAML diff found one added Python assertion missing the ten-space block indentation, which moved it outside the existing run script and made the workflow invalid.
- Restored the assertion's indentation. Its contract remains unchanged: require the new single inic_enabled health-monitor gate and forbid unchecked nvram_get_int("inic_disable") reads.
- AI handoff docs #44907152023 PASS on 7344df1. Process lifecycle #44907026283, process inventory #44907026337, HTTPD #44907026445, NVRAM audit #44907026244, iNIC signal #44907026286 and watchdog signal #44907026275 PASS on product SHA 75d7dee.
- XY-C1 #44907026295 remains at Build XY-C1 on 75d7dee; TRX/artifact pending. Correctly parsed Wireless rerun awaits this commit. No L3/L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align iNIC monitor gate contract with single checked snapshot

- Parent main SHA: 75d7dee28c30bca036a0287c5124dfb475e3b684; product source contains the one-read fail-closed inic_disable transition.
- Wireless radio #44907026259 passed the GNU89 wireless policy binary, bridge membership/teardown and guest-LAN policy regressions, then failed only at Python source contract line 318. The checker still required the old health-monitor condition nvram_get_int("inic_disable") != 1, but the production fix intentionally reuses the single checked inic_enabled value for monitor, radio and guest gates. Updated the test to require that same gate and assert unchecked reads are absent; no test stage or substantive guard was removed.
- Process lifecycle and AI docs PASS on exact checker/document SHA c1d2f8d. XY-C1 #44907026295 is still running on product SHA 75d7dee; build/TRX/artifact pending. NVRAM audit #44907026244 is still in the kernel/user caller audit. No current L3/L4 claim.
- Resulting main commit: THIS COMMIT. Exact-head Wireless/XY reruns pending.

---

## 2026-11-21 — Fail closed on RT3352 iNIC disable-flag read errors

- Parent main SHA: c1d2f8d9dee93bb374d3674958d6090d5de1e68a; product-source base includes the RT3352 MII TxPower fix in 917415b.
- Found an additional RT3352 startup status-loss defect: start_inic_mii() used nvram_get_int("inic_disable"), which maps nvram_get_checked() backend failure to zero; zero means enabled. The start_wifi_ap_rt() caller then made two more unchecked reads for guest-AP and health-monitor gates, permitting inconsistent decisions if the backend failed or value changed during one transition.
- Added a checked NVRAM read and strict decimal parser at the top of start_wifi_ap_rt(), before MLME, bridge, GPIO, RGMII or interface state changes. The missing-key zero default and legacy "exactly 1 disables" behavior are preserved; malformed/backend values fail closed.
- The validated enabled decision is passed to start_inic_mii(int inic_enabled) and reused for guest AP and iNIC health-monitor gates; all direct raw reads of inic_disable are removed from the radio start path. The public caller now preserves the helper's negative errno status.
- Added GNU89 wireless policy regression for backend error, missing/default enabled, disabled, legacy non-1 enabled and invalid output pointer. Updated Wireless source contract to require read/error gate before MLME and disallow unchecked duplicate reads.
- Process lifecycle #44906611688 and AI docs #44906611710 PASS on parent c1d2f8d; NVRAM audit #44906405138 PASS on product SHA 917415b. XY-C1 #44906404894 on 917415b remains at Build; TRX/artifact pending. Exact-head Wireless/XY verification for this new product change pending; no L3/L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align SSH settling-wait checker with checked start delegation

- Parent main SHA: 917415b910965a663b0d0869dec75639e93b277d; product-source baseline unchanged.
- Exact Process lifecycle #44906404890 ran all earlier daemon, shell, PID-file, WAN/LAN, VPN, UPnP, IPv6 and startup integration regressions successfully, then failed at check_process_lifecycle_integration.py:3348. It incorrectly expected service_start_probe_wait() inside public start_sshd(void). The checked public wrapper correctly reads sshd_enable and delegates to start_sshd_with_mode(int sshd_mode), which contains the detached-child settle/probe loop. Updated the checker to inspect wait/retry in that helper and separately assert checked-read/delegation plus absence of the legacy unchecked nvram_get_int() in the wrapper.
- RT3352 TxPower Wireless #44906405015, iNIC signal #44906405082, watchdog signal #44906405010, process inventory #44906404949, AI docs #44906404948 and HTTPD security #44906404924 PASS on exact SHA 917415b. XY-C1 #44906404894 remains in Build XY-C1; TRX/artifact pending. NVRAM audit #44906405138 remains in caller audit. No L3/L4 claim.
- Resulting main commit: THIS COMMIT. Exact Process/AI reruns pending.

---

## 2026-11-21 — Normalize RT3352 MII TxPower across config and live radio paths

- Parent main SHA: `b33f359bf38874d530db2a99c7ca345cb55f0a80`.
- Found a real config/runtime mismatch in RT3352 MII: `ralink.c` clamps invalid persisted TxPower outside 0..100 to 100 while emitting AP.dat, but `net_wifi.c:wireless_inic_radio_prepare_on()` passed the raw NVRAM integer to `iwpriv`. A malformed/stale value could therefore fail a live radio-enable transaction despite the generated driver configuration using 100.
- Added the shared GNU89-compatible `wireless_txpower_config_value()` policy (valid 0..100 preserved, all out-of-range values default to 100); used it in both the Ralink config writer and live iNIC radio enable path.
- Extended `wireless_radio_policy.c` with INT_MIN, -1, 0, 1, 100, 101 and INT_MAX regression cases. Updated Wireless CI source assertions and push/PR path filters to cover `ralink.c`, runtime helper use, config helper use and the regression.
- PID-1 queue #44906182543 and AI handoff docs #44906182540 PASS on parent `b33f359`. Process lifecycle #44906182539 was still running at the required PID 1 boot-environment source gate. Earlier XY-C1 #44905883001 on source `79e1932` remained at Build XY-C1 with TRX/artifact pending. The TxPower product commit should trigger new same-SHA Wireless and XY-C1 checks.
- Resulting main commit: THIS COMMIT. Exact-head verification pending; no current L3/L4 claim.

---

## 2026-11-21 — Align SSH lifecycle contracts with checked start wrapper

- Parent main SHA: `79e1932c74dc289e710c82ba3f184c9ee2bd785e`; production source SHA includes the checked SSH mode read and restart preflight.
- PID 1 notification queue #44905882925 passed all notification batch, queue trust-boundary, enqueue status, optional-daemon and GNU89/ASan/UBSan SSH policy tests, then failed its Python source checker at the stale assertion that launcher `eval()` calls reside in `start_sshd(void)`. That public entry point is now correctly a checked NVRAM wrapper delegating to `start_sshd_with_mode(int sshd_mode)`. Updated the checker to assert checked-read/delegation in the wrapper and the two legacy launch modes/postconditions in the implementation.
- Process lifecycle #44905882972 also passed the GNU89/`-Werror`/ASan/UBSan SSH policy binary, then failed because `check_sshd_firewall_status.py` searched for `sshd_nvram_mode_status` inside the function body extracted from that function. Replaced the impossible self-name assertion with concrete checks for backend-error errno propagation and inconsistent API-result/value rejection; existing parsing and range contracts remain.
- AI handoff docs #44905882928, process inventory #44905882756 and NVRAM audit #44905882978 PASS on exact SHA `79e1932`. XY-C1 #44905883001 is still at Build XY-C1; TRX/artifact pending. Exact-head reruns for the two source-checker repairs are pending. No L3 or L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Preserve SSH remote management on enable NVRAM read errors

- Parent main SHA: e84b9a30b63f1ed62ed96e361d5b085f70f8be3c.
- Confirmed availability defect in services.c: start_sshd() read sshd_enable through nvram_get_int(), so backend errors collapsed to disabled/success. restart_sshd() stopped the active SSH/Dropbear listener before calling start_sshd(), leaving remote management unavailable on the same transient failure.
- Added a strict checked mode parser for sshd_enable. Missing/empty remains mode 0; valid modes 1/2 retain Dropbear/OpenSSH behavior; malformed, overflowing and inconsistent API results return errors without overwriting the output. Direct start reads the mode before process inventory/start.
- Restart now checks sshd_enable before process inventory or stop, then passes the same validated mode into the start implementation, avoiding a second read failure after teardown. The public start API retains its signature.
- Extended the existing SSH lifecycle policy tests with GNU89-compatible cases for modes 0/1/2, missing/empty, API failure, inconsistent pointer/status, malformed values, overflow, null output and output preservation. Extended the existing source contract to enforce checked reads and read-before-stop.
- Exact process-lifecycle regression and XY-C1 production-source build are pending for this SHA. No local RC compile, MIPS firmware/TRX or hardware L4 result is claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Repair required handoff headings and correct firmware validation boundary

- Parent main SHA: 2222f27be79a7f785a48887047403e29f360ef59; AI handoff documentation #44905479770 failed because the handoff used noncanonical headings instead of required "## Current repository state" and "## Active task".
- Renamed those headings and retained the mandatory checkpoint, verification boundary and exact first action.
- Corrected XY-C1 status: run #44905479795 concluded success on 2222f27, but Build XY-C1, TRX verification and artifact upload were all SKIPPED because that head contained checker/documentation changes only. It does not prove L3 for product-source baseline d9bffce5.
- Process inventory #44905479794, USB modem #44905479781 and Firewall Startup #44905479780 PASS on 2222f27. AI docs rerun pending. NVRAM transaction audit #44904764756 PASS on d9bffce5; router-mode policy/wiring #44904910387 PASS on 0398903.
- Verified MiniUPnP internal-port-zero handling already exists in source and workflow tests (commit 1eb75818300dbefc79fbd3f706670df0e3011a6a); do not repeat it.
- Next P1 item: preserve SSH daemon state when sshd_enable NVRAM reads fail during start/restart.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Normalize whitespace inside shared WAN NVRAM call contracts

- Parent main SHA: `a0b9a4f09d8091b427eda457a3a06b78e3cfd1df`; product source baseline remains `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`.
- Exact-head USB storage #44905236668 PASS on `a0b9a4f`, including the corrected checked NFS wrapper/start/reload/restart source contract.
- Firewall #44905236641, XY-C1 #44905236647, USB modem #44905236704 and Process inventory #44905236635 all reached the same WAN caller checker and failed the raw compact-count assertion. Evidence: the NETFILTER call has whitespace directly after the opening parenthesis, so the normalizer that collapses whitespace to a single space is insufficient. Corrected the count by removing all source whitespace before matching the exact invocation; exact count=2 remains required. The earlier policy binaries and WAN caller status checks in these jobs passed. XY-C1 stopped before target build/TRX/artifact.
- AI docs #44905236737 failed because the current handoff omitted the required literal “Before context exhaustion” continuous-execution line; restored it without changing the handoff's materiality gate. Exact docs rerun pending.
- NVRAM audit #44904764756 PASS on source `d9bffce5`; router-mode source regression #44904910387 PASS on `0398903`. No current-source XY-C1 L3 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Correct shared WAN call normalization and NFS storage contract

- Parent main SHA: `474eec0dc8b19630c62b13713731c4a3976073ce`; product-source baseline remains `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`.
- Follow-up review proved the prior regex repair used double backslashes inside a raw pattern. It therefore searched for literal slash sequences, not line breaks/whitespace. USB modem #44905019162, Firewall #44905019100, Process #44905019098 and XY-C1 #44905019103 all stopped at this same shared assertion after their preceding policy tests passed. Replaced the regex with the checker's existing whitespace compactor and retained the exact two-call requirement.
- USB storage #44904764585 had independently failed its final source contract after all preceding storage policy and BusyBox/ash lifecycle checks passed. The contract still placed the NFS launcher in `run_nfsd_locked()`; it now verifies the checked NVRAM reader, fail-closed start/reload wrappers, export-policy-before-launch implementations and restart read-before-inventory/stop sequence.
- No product source, workflow trigger set or test execution was removed. Exact-head reruns pending. NVRAM audit #44904764756 PASS on `d9bffce5`; router-mode workflow #44904910387 PASS on `0398903`; AI docs #44905019105 PASS on `474eec0`. No current L3/L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Normalize WAN notification source assertion for router-mode gate

- Parent main SHA: `0398903bb6475668035313743f51a827cfe7f919`; XY-C1 validation #44904764739 on router-mode source SHA `d9bffce5` stopped before firmware cross-build, after earlier policy steps passed, because `check_modem_wan_start_status.py` expected a one-line NETFILTER call. The router-mode failure guard intentionally wraps `rc_notify_merge_action_result(...reload_nat_modules())` across lines.
- Replaced the literal count with a whitespace-tolerant regular-expression count that still requires exactly two call sites. The independent FIREWALL notification call remains present; no production source or assertion strength was removed.
- Router-mode dedicated regression #44904910387 PASS on `0398903bb6475668035313743f51a827cfe7f919`. HTTPD #44904764718 and NFS #44904764717 PASS on prior source SHA `d9bffce5`. XY-C1 and NVRAM audit reruns pending after this checker correction.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Normalize multiline router-mode source contract

- Parent main SHA: `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`; Router mode NVRAM update regression #44904764742 compiled and ran its GNU89 ASan/UBSan policy tests successfully, then failed in the source checker on a formatting-sensitive assertion. The production call intentionally breaks after the comma, so the checker searched for a one-line spelling that did not exist.
- Normalized the extracted `update_router_mode()` function by collapsing whitespace before asserting the complete checked-write status call. This preserves the assertion and does not change production source or relax behavior.
- Exact-head rerun pending. HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`; XY-C1 #44904764739, NVRAM audit #44904764756 and AI docs #44904764588 were still running/queued at last poll.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Make router-mode refresh fail closed on NVRAM errors

- Parent main SHA: `624dd2468ec8bc6422a4278c55779b69222c8f48`.
- Confirmed defect in `common_ex.c:update_router_mode()`: `nvram_get_int("sw_mode")` and `nvram_get_int("wan_nat_x")` collapse backend errors into zero. A transient read failure could therefore persistently rewrite `sw_mode` to Gateway (4) or Router (1).
- Replaced the lossy reads with a single checked NVRAM transaction. The update preserves AP-mode short-circuit behavior (no WAN NAT read in AP mode), legacy missing-key zero defaults, validates full decimal values, publishes the mode only after required reads succeed, and checks the write and transaction-close results.
- `full_restart_wan()` now performs the refresh before `stop_wan()` and aborts before teardown on failure. The NETFILTER notification skips NAT-module/firewall reload if the refresh fails, reports the action failure and still performs independent conntrack cleanup.
- Added GNU89 ASan/UBSan unit coverage for AP/missing/present values, backend and inconsistent API results, malformed/overflow values, output preservation and write-status mapping. New source checker verifies transaction order and both callsite gates. Exact-head Actions pending; no local RC or firmware build claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align HTTPD umbrella contract with checked NFS start wrappers

- Parent main SHA: `b284c28532f2913f4327b02ec59abaf97671be08`; NFS lifecycle Action #44903863601 PASS after the restart preflight placement correction.
- HTTPD umbrella #44903863573 failed only in its duplicated NFS export source contract: it still expected `run_nfsd_locked()` / `reload_nfsd_locked()` to contain export generation and the `eval()` launch, but the checked-enable fix deliberately separates that implementation into `run_nfsd_enabled_locked()` / `reload_nfsd_enabled_locked()` after `nfsd_enabled_config_read()`. The failure was a stale contract, not an NFS policy/test failure.
- Updated the HTTPD checker to enforce the checked NVRAM read and error gate in the wrappers, and preserve export-generation/start-eligibility-before-launch assertions in the new enabled-only implementations. Legacy unchecked NVRAM match calls are still explicitly forbidden.
- Exact HTTPD rerun pending. XY-C1 #44903863658 (Build running; TRX/artifact pending) and NVRAM audit #44903863567 remain IN_PROGRESS on parent SHA. AI docs #44903863682 and Samba #44903863669 PASS.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Place the NFS restart preflight at its destructive boundary

- Parent main SHA: `ce227363a7cd0aee0beab112bca9773a79e4630f`; exact NFS source-contract Action #44903638098 failed after the policy compile/run passed. The checker showed the enable-read block had been placed in unrelated `storage_has_mounted_devices()`, while `restart_nfsd()` still stopped the server before checking enablement. This was a genuine placement defect.
- Removed the misplaced block from the storage inventory helper. The checked `nfsd_enabled_config_read()` now runs immediately after acquiring the NFS storage lock and before mount inventory, `stop_nfsd_locked()`, or module unload. Restart passes that same validated decision to `run_nfsd_enabled_locked()`.
- Strengthened the NFS workflow source contract to require this ordering and forbid NFS config state in `storage_has_mounted_devices()`; no test was weakened.
- Exact NFS rerun pending. On parent `ce227363`, AI docs #44903638076 and Samba #44903638071 PASS; XY-C1 #44903638079 and NVRAM audit #44903638191 were IN_PROGRESS. No current-source firmware result claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Preserve NFS daemon state when enable NVRAM read fails

- Parent main SHA: `0b0df9fd43ab9d674ed4a55d1d6c52857cca7c61`.
- NFS start/reload used `nvram_invmatch("nfsd_enable", "1")`; restart used `nvram_match()` only after stopping the running server. Since those wrappers collapse backend errors into a non-match, start/reload could report success without acting and restart could stop NFS then unload its modules during a transient NVRAM failure.
- Added exact-match checked NVRAM policy for legacy `nfsd_enable == "1"` semantics (missing remains disabled; backend/inconsistent results return errno without overwriting the output). Start/reload use the checked decision; restart reads it before the first stop side effect, then reuses the validated state through its locked start path.
- Added GNU89/Werror cases for enabled, disabled, missing, backend failure, inconsistent API results and output preservation. NFS workflow source assertions now require read-before-stop and reject legacy unchecked checks.
- MiniUPnP lifecycle #44903303300, AI handoff #44903303195, Samba #44903303343, WAN route #44903303205, DNSMasq #44903303199, Inadyn #44903303186, HTTPD #44903303211 and Process #44903303308/#44903303227 PASS on `0b0df9fd43ab9d674ed4a55d1d6c52857cca7c61`. XY-C1 #44903303317 and NVRAM audit #44903303263 were still IN_PROGRESS. NFS exact-head checks pending. No local MIPS build or hardware L4 is claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Keep MiniUPnP running when service-policy NVRAM reads fail

- Parent main SHA: `f99d7a43be07851f39b0773b62d91aa29fc05235`.
- Source review found `start_upnp_locked()` and `update_upnp_locked()` used error-collapsing `nvram_get_int()` reads for `upnp_enable_x` / `wan_nat_x` and `get_ap_mode()` for `wan_route_x`. A temporary backend failure could suppress a needed start or stop a working UPnP daemon and discard live port mappings; a bridge-mode read failure also appeared as non-bridged mode.
- Added checked integer and exact bridge-value policy helpers with explicit missing-key defaults and output preservation on API/read errors. All three settings are read under one NVRAM transaction; the decision is published only after transaction close succeeds. Start/update now abort on read/close errors; update releases its process snapshot and preserves the active daemon.
- Added GNU89/Werror regression cases for present/missing, malformed/overflow, bridge comparison, backend/API inconsistencies and output preservation. MiniUPnP workflow source assertions now require transactional checked reads before daemon start/stop actions.
- Existing MiniUPnP lifecycle test is compiled by CI with GNU89 and `-Wall -Wextra -Werror`; exact-head Actions and XY-C1 cross-build are pending. No local firmware build or hardware L4 is claimed.
- Product-source baseline before this slice: `f99d7a43be07851f39b0773b62d91aa29fc05235`; resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Preserve URL and LWF firewall filtering on NVRAM read errors

- Parent main SHA: `dffca184e64dc44d8cfae11a43bbcebe21d9e0b6`.
- Source review found unchecked `nvram_match()` reads for `url_enable_x` in both IPv4/IPv6 filter builders, `fw_lw_enable_x` in IPv4, and `url_inv_x` at both URL-filter policy sites. A backend error was interpreted as false, which could silently omit an enabled URL/webstr filter chain, LWF chain or inverse-MAC policy from a newly staged firewall ruleset.
- Replaced those reads with the existing `firewall_read_enabled_setting()` checked helper. IPv4 checks URL, LWF and (only when URL filtering is enabled) inverse mode before opening its rule file; IPv6 checks URL and inverse mode before staging. Missing keys keep the previous false default. Backend/invalid results abort and invoke the existing multi-table restore rollback path.
- Extended the Firewall Startup Stability source contract to require each checked gate before `firewall_open_rule_file()` and reject all three unchecked `nvram_match()` patterns. The existing GNU89/Werror/ASan/UBSan firewall NVRAM test fixture is reused unchanged.
- Exact WAN route #44902460823 PASS and AI docs #44902460812 PASS on parent `dffca184`; DNSMasq #44902327424 PASS and AI docs #44902327438 PASS on `5a436698`. XY-C1 #44902070311 on production SHA `232e31d7` had passed all pre-build checks but its Build XY-C1 job remained IN_PROGRESS at last poll; TRX/artifact pending.
- No local full RC/MIPS build or hardware L4 is claimed. Exact firewall/XY-C1 validation for this slice pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Match DNSMasq preflight assertion to normalized call formatting

- Parent main SHA: `2a2238af3fb43b28a1147fa264459f004359795f`.
- Follow-up review of the exact Python checker found the preflight calls are split after the opening parenthesis. `compact()` normalizes that newline to one space, so the gate token now includes that space while retaining the five exact option names and before-side-effects ordering assertions.
- No product source or policy test changed; exact DNSMasq/WAN reruns on this checker state remain pending.
- Resulting main commit: THIS COMMIT.

---
## 2026-11-21 — Correct DNSMasq and WAN checker formatting assumptions

- Parent main SHA: `232e31d7eeaa5a6a5e7510ce74a59d9b2f9a7950`.
- DNSMasq exact run #44902070166 passed the new GNU89/ASan policy test and all preceding lifecycle regressions, then failed at the Python source checker because the inserted signature contained a literal double-backslash-n rather than the actual newline escape. Corrected the checker to search for the actual C source signature.
- WAN route exact run #44902070187 passed WAN DNS-mode, MTU, EAPOL and firewall result checks, then failed one transaction helper token because the implementation ternary did not wrap the entire expression in an extra pair of parentheses. Corrected the expected normalized expression; transaction ordering and read-error-before-write assertions remain.
- AI handoff exact run #44902070141 PASS; miniupnpd #44902070251, HTTPD #44902070259, Samba #44902070353, Inadyn #44902070265, WAN DHCP #44902070191 and Process inventory #44902070152 PASS on `232e31d7`.
- XY-C1 #44902070311 and NVRAM audit #44902070127 were in progress at the last exact-head poll.
- No production source changed in this checker repair; exact-head reruns pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Keep DNSMasq resolver security options enabled across NVRAM read failures

- Parent main SHA: `04b8537ac3810740e15ec901fe14594b7383d3df`.
- Source review found `prepare_dns_dhcpd()` used unchecked `nvram_match()` for five router-mode resolver options, including `dhcp_stop_dns_rebind` (DNS rebinding protection) and `dhcp_proxy_dnssec` (DNSSEC proxying). Since the safe matcher maps backend errors to false, a transient read failure could silently publish a replacement config that drops an enabled security option.
- Added a checked NVRAM policy helper which preserves exact legacy `nvram_match(..., "1")` behavior and absent-key defaults, but returns backend/invalid API errors without changing the output. `prepare_dns_dhcpd()` now reads all five options before DHCP pool writes, generated hosts, resolver-file creation or opening the replacement config. Any read error therefore leaves the last-known-good dnsmasq configuration unpublished.
- Added GNU89/Werror/ASan/UBSan host regression cases for present true/false values, missing defaults, backend errors, inconsistent API results and output preservation. DNSMasq workflow runs the binary and verifies source gates; XY-C1 now watches the production policy header.
- Host policy test compiled and passed locally. Full RC/MIPS build and router behavior remain exact-head CI/hardware validation items.
- The exact WAN checker/handoff repair is commit `04b8537a`; its Actions were pending when this DNSMasq slice began. Exact-head DNSMasq/XY-C1 and WAN reruns will be checked after this commit.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Repair exact WAN protocol-copy checker and handoff wording

- Parent main SHA: `b4bdfa7803678887021284b8876251043e723d85`.
- Exact WAN route #44901566662 failed only in the new checker section: the source uses multiline formatting for the transaction copy expression. Updated the checker to normalize whitespace, then check the same transaction begin/get/status/set/end sequence and verify the read-status gate precedes the write.
- Exact XY-C1 #44901566573 failed the WAN firewall source contract because its assertion still expected the old unsafe `nvram_safe_get("wan_proto")` expression; updated it to require `set_wan_unit_param_status(unit, "proto")` while retaining status-latch and pre-`get_wan_proto` ordering checks. Firmware Build, TRX and artifact steps were skipped by this earlier checker failure.
- AI handoff docs #44901566570 required the exact sentence `The checkpoint is mandatory; a document edit is conditional.`; restored it and documented the exact CI outcomes, including that Build XY-C1 was skipped rather than failed.
- No production source changed in this checker repair. The existing checked NVRAM transaction behavior is unchanged.
- Resulting main commit: THIS COMMIT; exact-head reruns pending.

---

## 2026-11-21 — Stop WAN reset from publishing an empty protocol on NVRAM read failure

- Parent main SHA: `4a02532fb9082af0872240cd7b70330426332b2a`.
- Source review found `reset_wan_vars_status()` copied global `wan_proto` to `wan0_proto` using `nvram_safe_get()` followed by a separate write. The safe getter maps an NVRAM backend/lock/ioctl/close error to an empty string. A transient failure could therefore overwrite the per-unit protocol marker and make later WAN startup select an invalid/no protocol instead of preserving a coherent configuration.
- Replaced that error-collapsing read/write pair with the existing `set_wan_unit_param_status(unit, "proto")`: it reads `wan_proto`, distinguishes absent from backend errors, and publishes `wan0_proto` under one NVRAM transaction. Missing key continues to copy as an empty value; backend/read/write/transaction-close failures propagate before startup advances.
- Extended `check_wan_dns_read_status.py` with a production source-order contract requiring the transaction helper, checked read-status gate and no `nvram_safe_get("wan_proto")` fallback. Existing GNU89 WAN NVRAM status tests and WAN-route workflow run alongside it.
- Full router RC/MIPS build and physical WAN behavior remain pending; exact-head WAN-route and XY-C1 Actions will be checked for this source SHA.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Preserve MAC access-control and DoS gates on NVRAM errors

- Parent main SHA: `ee01141a9761cd702ef71b6f8dbae2ad48604f6a`.
- Follow-on review found three more firewall policy reads that were still error-collapsing: `macfilter_enable_x` was read with `nvram_get_int()`, and `fw_mac_drop` and `fw_dos_x` with `nvram_match()`. A transient backend failure could silently remove the MAC filtering chain/jump or DoS limit rules from a newly generated IPv4/IPv6 ruleset.
- Extended the checked firewall NVRAM policy with strict, range-checked decimal parsing. Both IPv4/IPv6 builders now read the MAC mode plus its enforcement flag before opening a rule file; IPv4 also checks the DoS enable flag. Missing keys retain zero/disabled defaults; malformed, overflow and backend errors abort staging and roll back the active core restore transaction.
- Extended the existing GNU89/Werror/ASan/UBSan fixture for MAC modes 0/1/2, negative and INT_MAX values, missing defaults, backend errors, malformed values, and overflow. Workflow source contracts prohibit the error-collapsing reads and require checked gates before file staging.
- Local standalone policy regression: GNU89, `-Wall -Wextra -Werror`, ASan/UBSan: PASS. Full RC/MIPS build and router L4 not claimed. Exact-head Firewall and XY-C1 Actions pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Fail closed when firewall enable NVRAM reads fail

- Parent main SHA: `6dc8c0bb9759e63624b65a8285a86a9a408cc9cf`.
- Five firewall builders in `firewall_ex.c` used `nvram_match("fw_enable_x", "1")`. `nvram_match()` hides checked-read failures as false, making an unavailable NVRAM setting indistinguishable from an absent/disabled setting. The four IPv4/IPv6 filter builders could therefore generate ACCEPT base-chain policies after a backend/lock/ioctl/close error.
- Added a checked reader backed by `nvram_get_checked()` and a GNU89 policy helper: key absence retains the legacy disabled default; present values compare exactly; backend errors and inconsistent API results return errno/EIO without modifying the result. All five firewall enable reads (IPv4/IPv6 filter, both default filter builders and IPv4 NAT/local-port policy) now fail before opening a rule file.
- A failed read aborts the active multi-table core restore transaction through the existing rollback path, preventing earlier table changes from being published as a partial ruleset. Added GNU89/Werror/ASan/UBSan regression coverage and firewall/XY-C1 workflow source contracts.
- Local host test: the standalone policy binary compiled with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan and passed. No full RC cross-build, MIPS firmware, TRX/artifact or router L4 is claimed.
- Exact-head Firewall and same-source XY-C1 Actions: pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Complete the watchdog NVRAM regression pull-request path filter

- Parent main SHA: `fee2d5d2d016853fc6d35c4f4247c3fc451f2323`.
- Exact-head watchdog Action #44900457471 compiled and passed the GNU89/Werror/ASan/UBSan CPU watchdog policy regression, including the new missing/default, nonzero enable and backend-error-preserves-output cases. Its embedded integration contract then failed because the new `wan_nvram_status_policy.h` dependency had been added to the workflow push filter and embedded filter inventory, but not to the workflow's pull_request path list.
- Added the same production header to the PR filter. This restores push/PR symmetry; no product source or regression condition changed.
- Exact-head XY-C1 #44900457461 remains IN_PROGRESS (cross-build job active; Build XY-C1, TRX verification and artifact upload have not yet completed). Exact-head Process #44900457513 and other independent jobs remain in progress.
- No local checkout/runtime; no local test/build claimed. The follow-up has its own push workflow run pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Preserve CPU watchdog module state when its NVRAM setting is unreadable

- Parent main SHA: `47335ed2ce9e949aded6c361e7164ac51851ff95`.
- `start_watchdog_cpu()` and `restart_watchdog_cpu()` read `watchdog_cpu` through `nvram_get_int()`. That API collapses a backend/lock/ioctl/close error to zero: boot could silently skip the configured CPU watchdog, while a PID-1 restart during a transient NVRAM failure could interpret the setting as disabled and unload an already-running watchdog module.
- Added `watchdog_cpu_config_enabled()` using `nvram_get_checked()`, the strict integer reader and a status policy. Missing keys retain the historical disabled default; malformed, overflow and backend failures are returned unchanged. Both start and restart now return before any module load, unload or postcondition probe if the setting cannot be read.
- Extended the GNU89 watchdog policy test for missing/default, nonzero enable, error preservation and invalid output pointer. Added exact source/call-order contracts and the NVRAM status header to watchdog workflow push/PR filters and XY-C1 firmware-input regression.
- Parent Samba slice on `47335ed2ce9e949aded6c361e7164ac51851ff95`: Samba #44900045476 PASS, Process inventory #44900045461 PASS, USB identity #44900045444 PASS, vsftpd #44900045350 PASS, NFS #44900045396 PASS, AI docs #44900045349 PASS. XY-C1 #44900045362 was IN_PROGRESS at last poll; exact Build/TRX/artifact pending. Process inventory/lifecycle umbrella and other independent workflows required continued polling.
- No local checkout/runtime; no local test, sanitizer or MIPS build is claimed. This watchdog regression and same-SHA XY-C1 require exact-head Actions.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Fail closed on Samba share-mode NVRAM errors and unsupported values

- Parent main SHA: `c9eff6c602124abc3d389272d3f0a98bbc6d092d`.
- `write_smb_conf_unlocked()` used `nvram_get_int("st_samba_mode")`. A transient NVRAM backend failure therefore became the default zero mode; malformed or unsupported values also fell through to `confpage` with `result == 0`. The function then atomically published a header-only/partial `smb.conf` as success and could reload Samba over the last-known-good share configuration.
- Added `samba_read_config_mode_status()` using `nvram_get_checked()` plus the existing strict decimal parser. Missing keys retain the historical disabled-mode zero default, but backend, malformed and overflow errors now abort. A small allowlist accepts only modes 0 (disabled), 1, 3 and 4; all other values return EINVAL. The writer validates before opening a staged file, and its explicit mode branch now allows header-only publication only for mode 0.
- Added a GNU89 policy regression for all four supported modes and invalid negative, gap, high and integer-extreme values. Extended Samba push/PR contracts and XY-C1 firmware-input coverage for the new production policy header.
- Exact-head `c9eff6c602124abc3d389272d3f0a98bbc6d092d` OpenVPN key regression #44899519368 PASS; Process inventory #44899519328 and Process lifecycle inventory #44899519428 PASS; AI handoff docs #44899519407 PASS. XY-C1 #44899519362 remains IN_PROGRESS (Build started; TRX/artifact pending). Parent USB source XY-C1 #44898901805 also remains IN_PROGRESS at last poll.
- No local checkout/runtime is available; no local unit test, sanitizer or firmware build is claimed. New Samba and same-SHA XY-C1 Actions pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Protect OpenVPN TLS-crypt-v2 server keys at creation and on reuse

- Beijing time: 2026-11-21 19:36 (UTC+8). Parent main SHA: `c58272b954ad175b16bb85879d3913d384035704`.
- Security review of `trunk/user/openvpn/openvpn-cert.sh` found `make_ta_tc2()` explicitly applied mode 0644 to `stc2.key` after generating it with OpenVPN's `--genkey tls-crypt-v2-server`. This is the server-side wrapping key used to create client-specific TLS-crypt-v2 keys; exposing it to every local user lets them produce client keys accepted by the router. The routine also skipped permission repair when a key already existed, and process umask 0022 left RSA/private key files readable during creation before their later chmod.
- Changed the script-wide umask to 0077. TLS-auth and TLS-crypt-v2 server keys now require successful generation, non-empty output and chmod 0600; existing files are also re-protected on every invocation. Generation and chmod errors propagate instead of being hidden by the progress message.
- Added a mocked OpenSSL/OpenVPN shell regression. It checks initial private-key modes at file creation, verifies repair of pre-existing 0644 TLS keys, and confirms TLS-crypt-v2 server-key generation failure returns nonzero. Added a dedicated push/PR workflow and ensured the installed certificate-generation script triggers the XY-C1 firmware workflow and its firmware-input regression.
- Parent USB modem-removal slice on `c58272b954ad175b16bb85879d3913d384035704`: USB identity #44898901905 PASS, Process inventory #44898901784 PASS, AI handoff docs #44898901797 PASS; WAN DHCP callback #44898901879 and WAN teardown #44898901886 PASS. XY-C1 #44898901805 was IN_PROGRESS at last check (Build XY-C1 running, TRX/artifact pending); L3 not claimed for `c58272b954ad175b16bb85879d3913d384035704`.
- No local checkout/runtime is available; no local shell regression or MIPS firmware build is claimed. New OpenVPN, XY-C1, and handoff Actions require exact-head verification.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Fail closed when USB modem removal cannot read its transport type

- Beijing time: 2026-11-21 19:26 (UTC+8). Parent main SHA: `6f47ae209274dd312c30499e8028ab059b68df72`.
- Follow-up source review found `safe_remove_usb_modem()` still selected NDIS teardown vs RAS PPP teardown with `nvram_get_int("modem_type")`. On a transient NVRAM read/close/lock failure, the getter returned zero and incorrectly selected RAS. For a configured NDIS modem this could skip the WAN DHCP and usbnet teardown, clear the selected-modem marker and allow physical removal while the NDIS lease/network remained active.
- Added `usb_modem_read_transport_status()`: it uses `nvram_get_checked()`, the existing strict integer parser and `rc_wan_nvram_modem_is_ndis_status()`. Missing key keeps the legacy RAS default; backend, malformed and overflow errors retain their status. `safe_remove_usb_modem()` now aborts before either transport-specific stop path when the transport is unknown, preserving the modem and its current network state.
- Extended `scripts/check_modem_module_reload.py` to pin checked transport selection and the error return before NDIS/RAS teardown. Existing `wan_nvram_status_policy.c` GNU89/ASan/UBSan coverage exercises the checked integer parser and NDIS/RAS status policy; USB workflow already watches the affected source/checker and runs the test.
- Prior USB boot-selection slice on parent `6f47ae209274dd312c30499e8028ab059b68df72`: USB modem identity #44898560333 SUCCESS, including checked WAN NVRAM policy binary and modem module source contract; Process lifecycle #44898560487 SUCCESS; AI docs #44898560510 SUCCESS. XY-C1 #44898560501 was still IN_PROGRESS at last poll (Build running, TRX/artifact pending); no L3 claim for `6f47ae209274dd312c30499e8028ab059b68df72` yet.
- No local checkout/runtime is available; no local tests/build are claimed for this follow-up. Exact-head USB/Process/AI runs and XY-C1 Build/TRX/artifact are pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Preserve NVRAM failures in USB modem boot selection

- Beijing time: 2026-11-21 19:20 (UTC+8). Parent main SHA: `cd04be437693e91e029f4b2f64d415ae7017e9eb`.
- Source review confirmed `load_usb_modules()` used `nvram_get_int("modem_rule")` and `nvram_get_int("modem_type")` to select and initialize the configured USB modem driver family. `nvram_get_int()` calls the error-collapsing getter, so a transient NVRAM device/lock/read/close failure becomes zero; the boot path then silently skips the required driver readiness check and may continue to WAN startup.
- Added a checked integer reader based on `nvram_get_checked()` + `rc_wan_nvram_read_int_status()`. Missing keys retain the former zero defaults; backend, close and malformed/overflow errors now join the boot first-error accumulator. Modem-family reload occurs only after both required reads succeed. Independent USB printer/storage/host module attempts continue even when the modem configuration read fails.
- Extended `scripts/check_modem_module_reload.py` to pin checked reads, no unchecked modem-rule/type getters, and the read → error gate → modem reload → independent host-module ordering. The USB workflow already watches `rc.c`, the checker and the existing checked-integer policy test in both push and pull_request filters; its GNU89/Werror/ASan/UBSan test covers absent defaults, backend errors, malformed decimal and overflow cases.
- Radio stabilization is now L2 PASS on product source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e`: Wireless #44897978738 PASS; iNIC #44898111180 PASS; AI docs #44898111202 PASS (on checker/docs descendants of the same source). XY-C1 #44897419694 on that exact source completed Build XY-C1, TRX verification and artifact upload successfully; L3 VERIFIED for this production-source SHA. Physical router L4 remains NOT VERIFIED.
- No local checkout/runtime is available; no local test or local MIPS build is claimed for this commit. Exact-head USB / Process / XY-C1 Actions pending.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Point iNIC UNKNOWN-state regression assertion at its test fixture

- Beijing time: 2026-11-21 19:14 (UTC+8). Parent main SHA: `310e7c8668db0eda362a0400775d5670cf7a2242`.
- On exact SHA `310e7c8668db0eda362a0400775d5670cf7a2242`, Wireless radio state #44897978738 and AI handoff docs #44897978824 passed. iNIC #44897978753 compiled and passed the GNU89/Werror/ASan/UBSan policy binary, then failed because `scripts/check_inicd_signals.py` searched for the test invocation `wireless_guest_isolation_link_state(-1, EACCES, ENODEV,` in the production policy header. The actual regression call is in `tests/wireless_guest_isolation_policy.c`; the header contains the function definition, not the test invocation.
- Corrected the assertion to inspect the test fixture that actually contains the call. No production source or test behavior changed, and no assertion was removed.
- No local checkout/runtime is available; no local tests or firmware build are claimed. XY-C1 #44897419694 remains in progress on source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e` with Build running and TRX/artifact pending. Fresh exact-head iNIC and AI handoff runs are required; no L3/L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Scope iNIC guest-state error assertions to the checked probe

- Beijing time: 2026-11-21 19:08 (UTC+8). Parent main SHA: `60ad278e7056ace912066b090fdf20d0fd874f3e`.
- Exact-head Wireless run #44897419595 and iNIC run #44897419703 both reached their source contracts and failed on the same brittle ordering assertion. The checker used the first `if (status != 0)` in `check_inic_mii_rebooted()`, which belongs to the earlier radio force-disable path, rather than the error branch following `wireless_guest_interface_state_checked()`. The iNIC GNU89/Werror/ASan/UBSan policy binary passed; the Wireless bridge, guest-isolation and PID-1 status policy checks also passed before its inline contract failed.
- Updated `scripts/check_inicd_signals.py` and the embedded Wireless workflow contract to locate the error branch starting at the checked guest-state call, then verify the call → error gate → return → DOWN-only recovery order. This corrects the test contract without weakening it or changing production behavior.
- No local checkout/runtime is available; no local tests or firmware build are claimed. On parent SHA, AI handoff docs #44897419640, Process umbrella #44897419609, Process inventory #44897419663, watchdog #44897419667, HTTPD #44897419727 and NVRAM #44897419737 passed. XY-C1 #44897419694 was still in progress. Fresh exact-head iNIC, Wireless and AI handoff runs are required; no new L3/L4 claim.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Keep iNIC recovery fail-closed when guest link state is unknown

- Parent main SHA: `ed7c42675c8784123f1ad14cf098b2d31be6ddcc`.
- Follow-up source review found `is_interface_up()` collapses `get_interface_flags()` errors to false. In `check_inic_mii_rebooted()`, a socket/ioctl error was therefore misread as a down guest VIF and could trigger a recovery bring-up despite unknown driver state.
- Replaced the boolean probe with the existing checked `wireless_guest_interface_state_checked()`. A generic probe error now logs and returns before changing the guest AP; only a successfully observed DOWN state can enter recovery. This reuses the existing `wireless_guest_isolation_link_state()` policy and `test_unknown_guest_link_state_still_requests_down` coverage, with both iNIC and Wireless workflow source contracts updated.
- iNIC and Wireless workflows passed on the earlier status-propagation source/checker SHA `07e68ff`; the new UNKNOWN-state behavior itself is pending exact-head validation.
- No local checkout/runtime; no local tests/build claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Correct iNIC checker test-path lookup

- Parent main SHA: `ca6862c97fd964d0aaa9451e9fb7116773411c8d`.
- Exact-head iNIC run `#44897089095` passed the GNU89/Werror/ASan/UBSan signal policy test, then failed on a checker-only TypeError: `rc` is source text for `rc.c`, not a pathlib.Path, so `rc / "tests/wireless_radio_policy.c"` is invalid. Switched the assertion to the checker’s `read("trunk/user/rc/src/tests/wireless_radio_policy.c")` helper. No source or regression logic changed.
- Prior exact-head Wireless `#44896945159` and AI docs `#44896945122` passed; the iNIC failures so far were checker issues, not C test failures.
- No local tests/build claimed. Re-run exact-head iNIC and AI docs.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Fix iNIC retry-wait checker to match the sigsuspend gate

- Parent main SHA: `07e68ffbe95b7d8954274207e4712997a3c1548a`.
- Exact-head iNIC run `#44896945147` passed the GNU89/Werror/ASan/UBSan signal-policy executable, then its source contract raised ValueError because it searched for `if (events == 0)` while the main loop uses `if (events == 0 && sigsuspend(...)`. Changed the checker to anchor on the actual retry-wait prefix and still require the health-check error log to precede the wait and forbid a poll failure from breaking the daemon loop.
- Exact-head Wireless `#44896945159` and AI docs `#44896945122` passed on the same SHA. This is checker-only; no product source changes.
- No local tests/build claimed. Exact-head iNIC and AI docs must be rechecked on the new SHA.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Correct iNIC callback source signature checker formatting

- Parent main SHA: `215892b1acde4eea1be4be7785f4ef2b2b197e55`.
- The Wireless radio state workflow `#44896853653` compiled and passed all C radio, bridge-membership and guest-isolation policy tests, then failed in the embedded source contract because it expected the iNIC function declaration `int check_inic_mii_rebooted(void)` on one line. The production source uses the repository's valid two-line style (`int` newline function name). Corrected both iNIC and Wireless source checkers to assert the actual multiline declaration; no production code or test semantics changed.
- iNIC workflow `#44896853546` was still in progress at the time of this correction. Other exact-head results: WAN route, USB modem, infosvr, vsftpd, VPN server, networkmap, MiniUPnP, Samba and Adbyby PASS; XY-C1 remained in progress.
- No local checkout/runtime; no local test/build claimed. Re-run exact-head iNIC and Wireless Actions.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Propagate iNIC MII health-recovery failures to periodic monitor

- Parent main SHA: `6dd282fc7112b86ccdb167100e44765fbca97b73`.
- Source review found `check_inic_mii_rebooted()` silently ignored the `doSystem()` result when forcing RT3352 iNIC radio off for a disabled radio. Guest AP recovery errors were only logged; the void callback gave `inicd_main()` no status to report.
- Changed the callback to return int, normalize both command failure paths through the existing tested `wireless_radio_restart_status()`, and have the daemon log an incomplete health poll while continuing to the next timer tick. Updated both iNIC and Wireless source contracts to require returned status and retry semantics. No timer/stop signal behavior changed.
- Existing Wireless C policy regression already exercises positive and negative `system()` wait status normalization/sticky first error; existing iNIC signal GNU89 ASan/UBSan test remains wired.
- No local checkout/runtime available; no local C/Python test, sanitizer or firmware build is claimed. Exact-head iNIC and Wireless Actions are the verification gate.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Align IPTV static-copy bounds contract with shared helper

- Parent main SHA: `8a2f50bedb44fdff0cb8b322372090fc190d978c`.
- Exact-head WAN DHCP callback `#44896283833` passed all earlier policy/renewal and callback checks, then failed because the static IPTV startup checker still required three hand-written `strlen() >= sizeof()` checks. Production now uses `rc_wan_nvram_copy_value()` for IP address, netmask and gateway; that shared helper validates destination size and returns `ENAMETOOLONG` rather than truncating. Updated the embedded contract to require the three checked bounded-copy calls with `sizeof(destination)`, their shared error-return gate, and absence of obsolete manual checks.
- AI docs `#44896283589` and WAN route `#44896283559` passed on the exact parent SHA. This fix is checker + handoff only; no product source changed.
- No local tests/build claimed. Exact-head WAN DHCP and docs Actions on the resulting SHA are required.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Align IPTV renewal and MTU checks with checked NVRAM status

- Parent main SHA: `728468e145d625164bf2a6aaba6883b183cb20c2`.
- Exact-head WAN DHCP callback `#44896094291` passed the C policy regressions and the physical-WAN renewal contract, then failed at the remaining IPTV renewal assertions, which still expected the removed unchecked retry helper and direct NVRAM getters. Updated them to require checked `err`/`route_err`/`mtu_err` reads, decision/error/retry ordering, checked `manv_mtu`, and no legacy getters.
- Exact-head WAN route status `#44896094274` passed the revised VLAN checker, then failed in `scripts/check_wan_initial_mtu_status.py` because its physical and IPTV renewal checks still looked for the removed retry helper. Migrated both contracts to the checked status decision and fail-closed ordering, retaining the post-retry MTU-triggered rebind checks.
- Exact-head AI handoff docs `#44896094292` requires the precise phrase `one atomic main commit` and standard checkpoint section headings. Rebuilt the handoff with the protocol's required sections and first actionable task.
- No product source changed. No local test/build claimed; validate this change only through exact-head Actions.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Align WAN DHCP and IPTV cache-copy source contracts

- Parent main SHA: `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6`.
- Exact-head HTTPD `#44895865685` and WAN DHCP callback `#44895865642` still asserted the pre-migration direct helper and unchecked `nvram_get_int()` status reads. Updated the source contracts to require all three checked status reads, first-error fail-closed ordering, checked stored-MTU reads, and rebind only after the decision helper succeeds. The HTTPD route-option assertion now checks each of the three supported route variables is validated before comparison rather than requiring an obsolete duplicate count.
- Exact-head WAN route `#44895865688` exposed stale `launch_viptv_wan()` assertions for `nvram_get_int()` and borrowed static setting pointers. Updated the contract to require checked `viptv_mode` reads, fail-closed error gates, copied address/netmask/gateway values, validation before local copy, and completion before VLAN teardown.
- Exact-head AI handoff `#44895865686` exposed a missing handoff checkpoint rule; restored the mandatory state-accuracy check and explicit no-op-commit prohibition.
- No production source changed in this commit. Remote exact-head CI on the new SHA is the verification gate; no local test/build is claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Preserve IPTV VLAN settings across NVRAM cache reuse

- Parent main SHA: `87807147e186061a8450680de567edc13311826a`.
- `launch_viptv_wan()` retained pointers returned by `get_wan_unit_value()` / `nvram_safe_get()` across later reads. The shared `nvr_cache` buffer was overwritten between the old WAN VLAN and IPTV interface names, and between static IPTV address and netmask; the old code could remove the wrong VLAN or use the netmask as the static address. The WAN MAC pointer was also retained through VLAN setup.
- Replaced these pointers with checked, bounded copies into local arrays. VLAN filtering, VID, priority and IPTV address mode now use checked reads and fail before any VLAN removal/creation if configuration cannot be read. Static address/netmask/gateway and WAN MAC are separately copied; missing gateway retains the previous empty/default behavior.
- Added a fake backend with a single mutable cache slot and a regression proving the old/new VLAN names, address, mask, gateway and WAN MAC remain distinct after six sequential reads. Extended the WAN route source contract to require checked configuration reads and reject the legacy pointer pattern.

---

## 2026-11-21 — Update HTTPD DHCP renew contract for checked-state retry flow

- Parent main SHA: c14651353b1096db701f560729c0052d0275f560.
- HTTPD security run #44895295713 on 8700539 failed in its embedded Python source contract after the WAN renewal migration. The contract still required dhcp_wan_bound_retry_required() and three nvram_get_int() reads in both the physical WAN and IPTV renew callbacks.
- Updated the workflow contract to require all three get_nvram_int_checked_status() reads, dhcp_wan_bound_retry_decide(), checked stored-MTU reads, and error-return ordering before retry/bind side effects. It still rejects the old error-collapsing getters and preserves the surrounding DNS/resolver/route assertions.
- This is a checker-only change; exact-head HTTPD, WAN DHCP and WAN route Actions on the new SHA are required. No local workflow script execution is claimed.

---
## 2026-11-21 — Tighten RAS credential snapshot source-contract ordering

- Parent main SHA: 72b789bc6c027d1bb9d36fdbc9473a00de9bb13f.
- Firewall startup run #44895295888 on ancestor 8700539 exposed the only failure in scripts/check_modem_wan_start_status.py: the checker searched a whitespace-compacted function but included an extra space in the expected modem-type parser token. Production write_pppd_ras_conf() already snapshots modem_type/user/pass/isp under one transaction and returns on snapshot/strict-parse errors before opening the peer file.
- Corrected the compact token and made the contract enforce snapshot -> strict integer parse -> peer-file open, with both snapshot_status and modem_type_status error branches before file creation. No production code or checks were removed.
- Exact 72b789 main had just retriggered Firewall startup run #44895636746, AI handoff run #44895636818, USB identity #44895636794, Process inventory/lifecycle #44895636775 and XY-C1 #44895636822; those were still in progress when queried. This checker change requires fresh exact-head runs.

---
## 2026-11-21 — Correct RAS modem snapshot source-contract whitespace

- Parent main SHA: c6c1d7a78213f9525560a5fe8602a6ec07957357.
- Firewall startup run #44895295888 on ancestor 8700539 passed the native UPnP/firewall policy binary then failed at scripts/check_modem_wan_start_status.py:162. Production write_pppd_ras_conf() correctly snapshots modem type/user/password/ISP transactionally, parses the copied modem-type string, and refuses to open the peer file on snapshot or parse error. The checker compared a whitespace-compacted C function against an expected token that still contained a space after the comma.
- Corrected the token and strengthened ordering assertions: snapshot -> strict modem-type parse -> PPP peer open, with both snapshot and parse failure gates before file creation. No production code or coverage is weakened.
- Exact-head c6c1d7a actions were in progress at this checkpoint; the new commit must rerun Firewall startup and related suites. HTTPD security has a separate stale DHCP-renew source assertion queued for repair.

---
## 2026-11-21 — Repair shared-cache NVRAM test fixture for interface names

- Parent main SHA: `8700539128d7865df7d4e706fe710603e772c2c9`; exact-head commit `8700539128d7865df7d4e706fe710603e772c2c9`.
- WAN DHCP callback run `#44895295803`, WAN route run `#44895295721`, and USB modem identity run `#44895295781` all reached the GNU89/ASan/UBSan `wan_nvram_status_policy.c` binary and failed in its `fake_nvram_get()` fixture at line 660. The new cache-overwrite test requested `wan0_man_ifname` and `wan0_ifname_t`, but the fake reader still hard-coded the older address/mask key sequence.
- Extended the fixture with optional per-call expected names (retaining the original defaults for existing tests) and initialized the two interface-name keys in the new regression. This restores the intended shared-cache overwrite coverage; it does not alter production code or weaken assertions.
- The same exact-head Actions also exposed independent stale checker failures: Firewall startup `#44895295888` expects an obsolete direct modem-type parse, while HTTPD security `#44895295713` fails a DHCP callback ordering assertion. These are the next diagnosis targets. WAN route and DHCP tests did not reach the new checked-renew tests because the fixture aborted first.

---

## 2026-11-21 — Fail closed on unreadable DHCP renewal status and MTU

- Parent main SHA: `a68bac734fd3f232fb16f095bed9bba83813da24`.
- Both physical-WAN `udhcpc_renew()` and IPTV `udhcpc_viptv_renew()` read prior lease, route and MTU error flags with `nvram_get_int()`. A temporary NVRAM backend error collapsed to zero, so a renew path could classify an incomplete lease transition as healthy and skip the full bound/rebind repair.
- Replaced those reads with `get_nvram_int_checked_status()`, and added `dhcp_wan_bound_retry_decide()` to preserve the first read error and refuse a retry decision from a partial tuple. A read failure is logged and returned before lease comparisons or publication; absent keys retain the default zero.
- Physical and IPTV renewers now also use checked getters for stored MTU comparisons; backend or malformed-value errors abort renewal rather than being treated as zero.
- Added GNU89 policy regression cases for each status-read failure, first-error ordering, complete tuple evaluation and independent retry reasons. Extended the WAN DHCP callback source checker to require checked reads and fail-closed ordering. Exact-head WAN DHCP, WAN route and handoff Actions are the validation gate; no local tests were run because this session has no checkout/runtime.

---

## 2026-11-21 — Preserve WAN interface names across cache-backed NVRAM reads

- Parent main SHA: `f268eab8f685b67a59736ce716a9bf88e031f69c`.
- Inspection of `nvram_linux.c` confirmed all `nvram_get_checked()` / `nvram_safe_get()` values point into a single static `nvr_cache` buffer reused on the next NVRAM read. `get_wan_ifname()` and `get_wan_ip4()` fetched the management interface and `wan0_ifname_t` into pointers, then called `get_usb_modem_wan()`, checked modem-type reads or WAN-protocol lookups before consuming them. The pointers therefore no longer represented their original interface names.
- Both paths now copy the management interface immediately and fetch/copy the WAN temporary interface with a checked, bounded helper before any further NVRAM access. Backend errors / oversized values propagate from `get_wan_ifname()`; `get_wan_ip4()` returns `INADDR_ANY` instead of querying an unverified interface.
- Added a regression proving both management and temporary interface strings remain distinct after the fake NVRAM backend overwrites its shared cache. Extended the WAN route static checker to require both safe-copy sequences.

---

## 2026-11-21 — Snapshot USB modem RAS credentials before shared NVRAM cache reuse

- Parent main SHA: `d8ff11c5c24230e88a7adc9464bb89d34eeb5036`.
- Inspection of `trunk/user/shared/src/nvram_linux.c` confirmed `nvram_get_checked_unlocked()` returns a pointer into one static `nvr_cache` buffer. Each subsequent NVRAM get overwrites that buffer. The RAS peer writer fetched `modem_user`, `modem_pass` and `modem_isp` into pointers before using them, so the earlier values could all resolve to the last setting read (or change under concurrent configuration updates).
- Replaced the independent reads with one bounded `nvram_transaction` snapshot of `modem_type`, username, password and ISP. The existing snapshot helper copies each cache-backed value before the next read, preserves a coherent tuple and invalidates it if any read or transaction close fails. No peer file is opened until the whole snapshot and strict modem-type parse succeed.
- Added a GNU89 policy regression using the fake backend's single mutable cache slot to prove all four RAS settings survive subsequent reads. Extended USB modem static checks; workflow already compiles/runs this policy test with `-Werror`, ASan and UBSan.
- This change prevents stale/misbound RAS credentials and carrier configuration; it does not alter the NVRAM keys or missing-key defaults.

---

## 2026-11-21 — Make RAS modem-type checker whitespace independent

- Parent main SHA: `912fbff13b652c09563159f3357a21732db35f4b`.
- Exact-head USB modem run `#44894556779` and firewall startup run `#44894556782` both reached `scripts/check_modem_wan_start_status.py` and failed at the new `modem_type_parse` assertion. Production code has the intended checked read before peer-file open; the checker first normalized whitespace to single spaces, then expected a call with no space after the opening parenthesis.
- Changed the source contract to compare a whitespace-free compacted function body, including the checked getter, strict parser arguments, saved errno/early return and read-before-open ordering. This is a test-only correction; the WAN NVRAM policy GNU89/ASan/UBSan regression had already passed in the USB workflow before the static step.
- Firewall run's UPnP/firewall restart result policy also passed before this shared checker failed. Rerun USB modem and Firewall workflows on the new exact SHA.

---

## 2026-11-21 — Fail closed on unreadable USB modem type while writing RAS peer config

- Parent main SHA: `b107e60a4c30b838ab963d3dc8ccdabeb4fe62e8`.
- `write_pppd_ras_conf()` still performed a second, unchecked `nvram_get_int("modem_type")` after opening/truncating the RAS peer file. If the NVRAM backend failed at this point, the error collapsed to legacy value zero and could select `Generic_conn.scr`; a partially rewritten peer file could then be published using a guessed transport profile.
- Moved the modem-type lookup ahead of peer-file creation, routed it through `nvram_get_checked()` plus the existing strict `rc_wan_nvram_read_int_status()` parser, and aborts with the preserved status on backend, malformed-value or range errors. Missing-key behavior remains the legacy default zero.
- Extended the USB-modem source contract to require the checked read and reject the unchecked getter. The USB-modem workflow now includes the existing WAN NVRAM policy regression, compiles it as GNU89 with `-Werror`/ASan/UBSan, and runs it on changes to the checked-reader header/tests.

---

## 2026-11-21 — Align deferred reconnect checker with watchdog rollback contract

- Parent main SHA: `2c0f9ce2b316798109289cdabcdcb20c1d22c21d`.
- Exact-head WAN route run `#44894284811` passed the earlier network sysctl and initial physical-MTU checks plus the policy binaries, then stopped at `Verify deferred WAN reconnect watchdog launch status`. Its checker required the literal `return launch_status;`, but production intentionally normalizes launcher failure and invokes `watchdog_rollback_failed_start(launch_status, launch_errno, "launcher")`; readiness timeout also runs through the same rollback path. This was a stale source expectation, not a missing production status return.
- Updated the contract to require explicit unknown-inventory handling and the rollback helper on both launcher failure and readiness timeout. The checked deferred deadline and watchdog call ordering remains required.
- Exact-head AI documentation run `#44894284801` PASS. New WAN/AI push Actions are the validation gate. No production source changed; current-source L3 and hardware L4 are not claimed.

---

## 2026-11-21 — Correct MTU checker position space after checked-read migration

- Parent main SHA: `751cb64ff459a1d5afebc836996dd43f810172f6`.
- Exact-head WAN route run `#44894158533` passed the new network-sysctl assertion and all earlier route policy binaries/contracts, but the MTU source checker failed at its checked getter lookup. The checker mixed an offset taken from the original C function text with a lookup in whitespace-compacted text; those indices are not interchangeable, so it failed to find a real, correctly ordered `get_wan_unit_value_int_status()` call.
- Reworked the initial-MTU contract to use one source coordinate space throughout. It now checks the checked per-unit MTU read, error aggregation/logging and `continue` before physical default MTU application, then verifies the existing default-write error handling still precedes `ifconfig(IFUP)`. Production source is unchanged.
- On this parent, WAN teardown run `#44894158502` PASS and AI handoff docs run `#44894158457` PASS. The new checker commit needs a fresh exact-head WAN/AI run. Current firmware L3 and hardware L4 are not claimed.

---

## 2026-11-21 — Align WAN MTU and teardown checks with checked NVRAM reads

- Parent main SHA: `2811673b34fdf7629dc5bb04d1e488a2a56d4a23`.
- WAN route run `#44893956383` reached `scripts/check_wan_initial_mtu_status.py` after the WAN sysctl and earlier route contracts passed, then failed because this older checker still expected the removed unchecked `get_wan_unit_value_int()` MTU read. Updated it to require `get_wan_unit_value_int_status()`, verify the per-unit error/continue path occurs before default MTU setup, and keep the default-write error aggregation ordering. Its `start_wan()` extraction now handles mutually exclusive preprocessor brace branches with a regression fixture.
- The WAN teardown contract previously expected the removed direct `nvram_get_int("modem_type")` branch. It now requires checked transport selection before monitor pause and the NDIS decision before adding the USBnet interface to teardown.
- No product behavior changed in this commit; this aligns source-contract checks with the checked modem-type and MTU changes already present.

---

## 2026-11-21 — Align network sysctl checker with checked SYN-cookie gate

- Parent main SHA: `7af88e6e0cf704717511efd808a758611d2fca2b`.
- WAN route run `#44893645481` on its exact parent exposed a stale integration assertion: production `start_wan()` had moved `fw_syn_cook` from `nvram_get_int()` to `get_nvram_int_checked_status()`, but `scripts/check_network_sysctl_status.py` still required the old getter. The same checker assertion remained at step 29 of WAN run `#44893837491` on this descendant while that workflow was in progress.
- Updated the contract to require the checked read, preserve the separate `tcp_syncookies_status` write result, and prove the NVRAM error return precedes sysctl application. Production behavior and workflow coverage are unchanged.
- AI handoff run `#44893837451` and XY-C1 run `#44893837554` were in progress on the parent SHA when inspected. New exact-head runs are required; no current-source L3 or hardware L4 claim is made.

---

## 2026-11-21 — Fail closed on unreadable USB modem transport during stop

- Parent main SHA: `19b5cdee226daa53db575fb82b02dd8b5c3c4077`.
- Source review found three remaining unchecked `nvram_get_int("modem_type")` decisions in WAN teardown/interface selection. A backend error was collapsed to the legacy zero/RAS value: manual disconnect could select the PPP-only stop path for an NDIS modem, the full stop could omit the USBnet interface from ifdown, and `get_wan_ifname()` could report the wrong transport interface.
- Added one checked transport-selection helper and a GNU89 policy regression for NDIS, legacy RAS default, invalid output and backend-error preservation. Full WAN stop now reads the type before pausing/stopping services; manual disconnect and interface lookup propagate the read failure instead of choosing a guessed path.
- The WAN route workflow already compiles the checked NVRAM policy regression and runs the expanded source contract. Firmware Build/TRX/artifact verification remains a separate exact-SHA requirement.

---

## 2026-11-21 — Normalize SYN-cookie and modem-type checked-read contracts

- Parent main SHA: `35967d072836add527cbfec20d1865088b5cd8c3`.
- Exact-head WAN route run `#44893536012` passed DNS, PPPoE, MAN/physical MTU, EAPOL/Kabinet and MAN DNS-route contracts, then found the same line-break mismatch in the checked `fw_syn_cook` call. Source inspection also showed the adjacent `modem_type` call and rollback call are split over lines, so their source assertions were normalized in the same pass.
- The checker now matches compacted source for the exact security-setting getter and verifies its fail-closed return precedes `set_tcp_syncookies()`; modem transport failure must still restore the WAN interface and continue before choosing NDIS/RAS.
- AI handoff documentation run `#44893536072` passed on the same parent SHA. No product C behavior changed.

---

## 2026-11-21 — Normalize MAN DNS-route source contracts

- Parent main SHA: `590ae64484500b91695076220961e3baabc929bc`.
- Exact-head WAN route run `#44893438430` passed the GNU89/ASan/UBSan policy binary, DNS, PPPoE, WAN MTU and EAPOL/Kabinet checks, then failed at the MAN route-creation assertion. The checked `dnsenable_x` call is split over lines; the test flattened line breaks but still required the exact unflattened call string.
- Compacted both MAN route setup and teardown slices before matching their checked getters and fail-closed/best-effort cleanup branches. No source behavior changed.
- AI handoff documentation run `#44893438413` passed on the same parent SHA.

---

## 2026-11-21 — Normalize EAPOL checked-read contract

- Parent main SHA: `ef17608b90343776a20ec2e7591b885b8be54359`.
- Exact-head WAN route run `#44893354019` passed the GNU89/ASan/UBSan policy binary, DNS-mode and PPPoE checks, and both MAN/physical MTU checks. It then failed the EAPOL checked-read source assertion because `start_wan()` splits the getter call over multiple lines.
- Updated the checker to compact the extracted function body and compare the exact checked getter, key and output variable without formatting sensitivity. The existing `auth_status` cleanup-path requirements remain.
- AI handoff documentation run `#44893354037` passed on the same parent SHA. No product C behavior changed.

---

## 2026-11-21 — Normalize physical WAN MTU checked-read contract

- Parent main SHA: `35f7f642c0115cbb238190256d498ad75cbb4fc1`.
- Exact-head WAN route run `#44893239999` passed the GNU89/ASan/UBSan policy binary, DNS-mode checks and PPPoE read/rollback contracts, and the MAN MTU contract after its whitespace normalization. It then exposed the same newline-sensitive source assertion for the physical WAN MTU getter.
- The production `start_wan()` call is split across lines. Updated the contract to compact the extracted C body before matching the checked getter, finding the NVRAM failure guard and verifying the unit is skipped before `ifconfig(...IFUP...)`.
- AI handoff documentation run `#44893240012` passed on the same parent SHA. No production C behavior changed.

---

## 2026-11-21 — Normalize multiline MAN MTU checked-read contract

- Parent main SHA: `703ff6abcffa17313482ac2ca772fc0c79afb826`.
- Exact-head WAN route run `#44893094794` passed the GNU89/ASan/UBSan policy binaries and the DNS-mode + PPPoE read contracts, then stopped at the MAN MTU static assertion. The implementation uses `get_wan_unit_value_int_status()` and returns before the MAN address snapshot on read error; its call is split across lines, but the checker first flattened whitespace and then compared against a string with no space after the function name.
- Changed the assertion to compare a whitespace-compacted call expression, preserving the exact getter, `unit`, `man_mtu` key and output pointer.
- AI handoff documentation run `#44893094799` passed on the same parent SHA. No product C behavior changed.

---

## 2026-11-21 — Normalize PPPoE rollback call arguments in source checker

- Parent main SHA: `7f5221f1fc861e1b9191ddec2ba5aca106741bc8`.
- Exact-head WAN route run `#44892863922` reached the idle-time rollback assertion but rejected the source call because the regex was emitted with doubled backslashes. The C branch uses the expected restore helper and arguments; this checker-only issue does not imply missing rollback.
- Replaced the regex with a whitespace-insensitive extraction of the call expression, then compare its compact form to the exact helper, unit and interface arguments and require the subsequent `continue;`.
- WAN DHCP and AI handoff checks passed on the same parent SHA.

---

## 2026-11-21 — Correct WAN rollback regex escaping

- Parent main SHA: `49cde0cd18521a94466b3bac2017d3edab346e88`.
- Exact-head WAN route run `#44892863922` passed the GNU89/ASan/UBSan policy binaries and the preceding WAN source contract, then failed the new PPPoE rollback regex. The source contains the expected multiline `restore_wan_after_firewall_failure(unit, wan_ifname)` call; the checker expression again contained doubled backslashes and matched literal backslashes instead of whitespace.
- Corrected the regex to use one backslash per whitespace escape. The assertion still pins both the restore helper and the exact `unit` / `wan_ifname` arguments.
- WAN DHCP callback run `#44892863908` and AI handoff docs `#44892863965` both passed on the same parent SHA.

---

## 2026-11-21 — Scope WAN failure-path assertions to their own branches

- Parent main SHA: `b1a6dc2eef817674065df02bcb08c8bce95ca8ac`.
- Exact-head WAN route run `#44892718181` reached the PPPoE idle-time rollback assertion. Source inspection showed the checked-read failure branch does restore WAN and continue; the call is split after the opening parenthesis, while the checker required a single exact spacing form. Changed it to a whitespace-tolerant regex that still verifies the unit and interface arguments.
- Exact-head WAN DHCP run `#44892718115` reached physical-interface cleanup ordering. The MTU-read error branch contains an earlier `continue;`, so searching the whole physical-setup slice falsely treated it as the interface-failure branch's continue. Scoped cleanup ordering to the `if (interface_status != 0)` block.
- Both are static-contract corrections; no production code or failure guards were weakened.

---

## 2026-11-21 — Scope DNSMasq resolver-lock assertion to its failure branch

- Parent main SHA: `68e10bad396c35c000a17cd26c6763573b13413e`.
- Re-examined DNSMasq lifecycle run `#44891688020` on c930b7e. The C implementation correctly returns on resolver-lock acquisition failure without unlocking an invalid descriptor. The workflow sliced from that branch through the later staging-file open, inadvertently including a separate, valid `file_unlock(lock)` when WAN DNS-mode lookup fails after lock acquisition.
- Changed the regression to extract only the `if (lock < 0)` block before asserting that it never unlocks. This preserves the lock-failure guarantee while permitting cleanup on subsequent post-acquisition failures.
- No production source changed. The DNSMasq workflow will rerun on this workflow-file update.

---

## 2026-11-21 — Require an actual column-zero C function terminator

- Parent main SHA: `c3fe4cfceb16f9fb140316ad789a45bebe2a60cb`.
- The first preprocessor-aware extractor revision still accepted any whitespace-only line prefix as a function terminator; its own regression fixture exposed that this stopped at an indented inner block close rather than the function close. The same predicate existed in the WAN DHCP workflow helper.
- Tightened both extractors to accept only a literal column-zero closing brace (`line_start == brace_position`), matching the repository's C function layout. The conditional-branch fixtures assert statements after the inner brace remain in the extracted body.
- No production code changed. Both WAN route and WAN DHCP workflows will rerun because their checker files are part of the updated commit.

---

## 2026-11-21 — Use preprocessor-aware extraction in WAN DHCP regression

- Parent main SHA: `a4a60944890edae9dc6821eec906d3c4c6d881b0`.
- Exact-head WAN DHCP callback run `#44892451936` passed the updated checked-MTU read assertions, then failed at the ordering assertion between default MTU setup and physical interface activation. The source order is correct, but the existing C brace counter treated mutually exclusive `#if/#else` branches in `start_wan()` as simultaneously active and terminated the extracted body at the wrong closing brace.
- Added a focused extractor that finds this repository's column-zero function terminator while continuing to skip comments and literals, with an alternate-preprocessor-branch regression fixture. The two WAN DHCP workflow extractions of `start_wan()` now use it; other function-body extraction remains unchanged.
- No production C behavior changed. Rerun the WAN DHCP exact-SHA Action after this checker correction.

---

## 2026-11-21 — Handle conditional-compilation braces in WAN startup checker

- Parent main SHA: `fee0fe11f4a7d12da0238e3f548fcc7b57b93215`.
- Exact-head WAN route run `#44892366354` passed the policy binaries and initial DNS-mode checks, then failed the PPPoE idle-time failure-path assertion. The production function contains the checked read, WAN restoration and `continue`; the generic brace counter counted both mutually exclusive `#if/#else` branches as simultaneous C blocks and truncated the extracted function body.
- Added a regression fixture with alternate preprocessor branches and a function extractor that uses the repository's column-zero C function terminator while still ignoring comments/literals. All `start_wan()` source-contract slices now use this preprocessor-aware boundary; the existing generic extractor remains unchanged for other functions.
- This repairs a false-negative static contract, not WAN runtime behavior. Rerun WAN route Actions on the resulting exact SHA.

---

## 2026-11-21 — Align WAN DHCP MTU regression with checked NVRAM read

- Parent main SHA: `b816880f652adcec58168be3911d38dec75a22b1`.
- Reviewed the earlier WAN DHCP callback run `#44891687966` (on c930b7e). Its inline checker still expected `wan_mtu = get_wan_unit_value_int(unit, "mtu")`, while current `start_wan()` reads MTU through `get_wan_unit_value_int_status()` and aborts the unit before default-MTU/interface setup on read failure.
- Updated the existing physical-WAN setup contract to require the checked getter, verify the `nvram_status != 0` guard and `continue` precede the default-MTU branch, and reject the legacy error-collapsing getter.
- This is a stale test expectation, not a new production change. The current exact SHA's WAN DHCP workflow will verify the updated contract.

---

## 2026-11-21 — Correct PPPoE whitespace regex escape in WAN checker

- Parent main SHA: `d2e41ab61db5607c7d075b65df3de9ae081cc83a`.
- Exact-head WAN route run `#44892123843` passed the GNU89/ASan/UBSan policy binaries and the preceding WAN DNS-mode contract, then failed the PPPoE demand-publication assertion.
- The C source confirms the demand setter is present immediately after the checked idle-time read; inspection found the raw Python regex contained doubled backslashes (`\\s`) and therefore searched for literal backslash characters instead of whitespace.
- Corrected the raw regex to use single regex escapes (`\s`), retaining the requirement that demand publication follows the checked read.
- XY-C1 run `#44892123837` and process lifecycle run `#44892123811` both completed SUCCESS on this parent SHA; XY-C1 Build/TRX/artifact details must be checked before assigning L3.

---

## 2026-11-21 — Align WAN regression contracts with checked EAPOL and PPPoE reads

- Parent main SHA: `cb21893b8651dc975646c9ac0bb08cf923a29030`.
- Exact-head WAN route run `#44891999134` passed its GNU89/ASan/UBSan policy binaries and the DNS-mode contract, then exposed a second formatting-sensitive check: idle-time publication used a multi-line call and was searched as one literal.
- The checked PPPoE demand-publication assertion now tolerates C whitespace while still requiring a call after the checked idle-time read.
- The XY-C1 and process integration workflows also expected the pre-migration direct EAPOL assignment. Updated them to require the current sequence: checked auth-mode read initializes `auth_status`, conditional EAPOL launch overwrites it, and the error branch consumes it. No production behavior or guard was weakened.
- Re-run the WAN route, XY-C1, and process checks on the resulting exact SHA; firmware L3 must only be recorded after build, TRX and artifact upload all pass on one SHA.

---

## 2026-11-21 — Restore regex import in checked WAN source contract

- Parent main SHA: `839922fedd844174ca6553c50901c5a9c942cc44`.
- Exact-head WAN route run `#44891897645` passed the GNU89/ASan/UBSan policy binaries and DNS-mode static contract, then stopped with `NameError: name 're' is not defined` after the PPPoE checked-read assertions were converted to whitespace-tolerant regex matching.
- Added the missing standard-library `re` import. No product C source changed in this checker-only correction.
- The WAN route workflow must pass on the resulting exact SHA; firmware L3 and physical L4 are not claimed.

---

## 2026-11-21 — Enforce zero unchecked WAN integer getter callsites

- Parent main SHA: `b8cb95b34d1362b23c32cf5c58538b57667e65cc`.
- Audited the current `net_wan.c` after migrating DNS mode, PPPoE manual/demand, WAN/MAN MTU, EAPOL/Kabinet auth and MAN DNS route setup/teardown. The legacy `get_wan_unit_value_int()` remains only as a public compatibility definition; there are no remaining internal callsites.
- Added a whole-file source-contract assertion so future WAN code cannot silently reintroduce the error-collapsing getter into an internal network decision.
- This is a static integration guard; exact-head CI is still required for C policy tests and downstream workflow gates.

---
## 2026-11-21 — Make WAN source-contract parser skip prototypes

- Parent main SHA: `de62205e3e02687e5908e863312945ff4b81bc39`.
- Hardened the WAN source-contract helper so function lookups skip forward declarations and select the actual implementation body. This matters for `get_wan_dns_static_status()` and route helpers, where a prototype can precede the definition; the previous naïve first-brace search could inspect the next function body and produce a false failure.
- No production code changed. Exact-head WAN route CI will validate the current DNS, PPPoE, MTU, authentication and MAN route assertions.

---
## 2026-11-21 — Preserve WAN DNS-mode NVRAM read failures

- Parent main SHA: `f2920b4d320be4bbb7340673f1b29a9c7c0d2b62`.
- Narrow WAN DNS slice: added GNU89-compatible strict decimal parsing and a checked integer-read policy around `nvram_get_checked()`. Missing keys preserve the existing numeric default, successful values must be a complete base-10 integer within `int` bounds, malformed prefixes/overflow fail, and backend/read/close errors remain distinct.
- Migrated `get_wan_dns_static()` internals to an errno-style status helper for both USB modem `modem_dnsa` and normal WAN `dnsenable_x`. The exported legacy getter retains its prior conservative boolean fallback and reports the error through `errno`.
- `update_resolvconf()` now aborts before staging/opening `/etc/resolv.conf` if the DNS-mode read failed; it releases the resolver lock and preserves the last-known-good file.
- Added missing/default, valid signed/whitespace/boundary, malformed, overflow, backend-error and null-pointer policy regressions; added a source-contract checker and exact workflow path/step wiring.
- No local compiler/checkout is available. Exact-head WAN-route, Process, AI-docs, Storage and XY-C1 Actions must be inspected on the resulting SHA; host regression PASS is not firmware L3.

---
## 2026-11-21 — Fix archive index path/size and Storage filter assertion

- Parent main SHA: `d397166be2fcd3582492a8004eac5e8dec216929`.
- AI handoff run `#44889732024` on `d397166be2fcd3582492a8004eac5e8dec216929` found that the archive itself was restored, but its index row used only `part-021.md` rather than the repository-relative path and recorded 107,416 rather than the actual 107,650 UTF-8 bytes. Corrected the index path and byte count; the original blob SHA and 100 headings are unchanged.
- Storage run `#44889614290` on `3791719dff38276df830dafe1ffbb2f501f0b13b` passed BusyBox 1.24.2 ash lifecycle tests and every earlier runtime check, then failed because a checker counted three Optware paths across the entire YAML file. Embedded test-script references made the total exceed two even though each path existed exactly once in push and pull_request filters. Corrected the checker to inspect those filter sections separately.
- XY-C1 run `#44889614321` completed SUCCESS on `3791719d`, but Build XY-C1, TRX verification and artifact upload were all SKIPPED by production-input detection for documentation/checker-only changes; this is not L3 verification. The last verified product-source baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Re-run exact-head Storage Hotplug and AI handoff checks on this correction. No production source behavior changed.
- Resulting main commit: THIS COMMIT

---

## 2026-11-21 — Restore the indexed November log archive object

- Parent main SHA: `3791719dff38276df830dafe1ffbb2f501f0b13b`.
- Exact-head AI handoff run `#44889614300` on `3791719dff38276df830dafe1ffbb2f501f0b13b` failed because `docs/dev-log/2026/11/part-021.md` was referenced by `docs/dev-log/INDEX.md` but missing from the Git tree.
- Recovered the original complete 107,416-byte archive from its existing Git blob `f80bf2d087fa1c313cb671f2108e3279092db234` and restored the exact indexed path; no archive content was regenerated or truncated.
- At last poll, Storage Hotplug run `#44889614290` and XY-C1 firmware run `#44889614321` were still in progress on exact SHA `3791719dff38276df830dafe1ffbb2f501f0b13b`. Their results must remain attached only to that source SHA.
- Re-run AI handoff and Storage Hotplug exact-head checks on this archive-restoration commit. Product L3 remains verified only on `20e139d230b73eabc5f25a2b17d56a3a5768af11`; physical-router L4 remains unverified.
- Resulting main commit: THIS COMMIT

---

## 2026-11-21 — Align Optware tri-state checker and end-session handoff contract

- Parent main SHA: `fa888e9c9323a555d63b2d84a9bd4bd82a181764`.
- Exact Storage Hotplug run `#44889271838` on source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17`: the BusyBox 1.24.2 ash lifecycle regression and all shell regression steps passed, including the explicit UNKNOWN/malformed-mount case; only the final static source checker failed because it expected `case _mount_match_result in` while the actual shell correctly uses `case $_mount_match_result in`. Corrected the checker to match the actual variable expansion; no production shell behavior was changed.
- Exact Process inventory `#44889271901` passed on `421042f3`. Exact AI handoff run `#44889426235` on `fa888e9` failed because the handoff was missing the required literal `one atomic main commit` and the required four structured sections. Reorganized the live handoff into those required sections while retaining the mandatory end-of-session accuracy checkpoint.
- Fixed the dev-log archive index to record the actual part-021 blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`.
- Storage Hotplug and AI handoff exact-head Actions must be rechecked on this commit. XY-C1 run `#44889271879` remained PENDING/IN_PROGRESS on exact source SHA `421042f3` at the last poll; no result is inferred for this documentation-only child. Product L3 remains verified only on `20e139d230b73eabc5f25a2b17d56a3a5768af11`; physical-router L4 is not verified.

---
## 2026-11-21 — Preserve UNKNOWN mount inventory status under BusyBox awk

- Parent main SHA: `065837e4dcc383ebe2d5276b8165ffa834f9dd3f`.
- Exact-head Storage Hotplug run `#44888827697` passed all earlier shell/source checks but failed in the BusyBox 1.24.2 ash lifecycle step. The truncated five-field `/proc/mounts` test expected UNKNOWN (2) but observed 1.
- The mount matcher encoded its tri-state contract in AWK process exit values 0/1/2. BusyBox 1.24.2 did not preserve the third value through the ash test path, collapsing malformed-inventory UNKNOWN into ordinary non-match. Replaced the nonzero AWK exits with explicit `match` / `different` / `unknown` output tokens; the shell maps those tokens back to 0/1/2 and still maps AWK execution/read failure to UNKNOWN.
- Extended the standalone source checker to enforce this portable result protocol. Existing regressions continue to cover exact source+mountpoint matching, device-prefix collision, malformed/truncated rows, duplicate rows, empty and unreadable inventories.
- Exact-head Storage Hotplug regression must confirm the correction. Process inventory and AI handoff passed on the parent SHA; XY-C1 run `#44888827678` was still running on parent SHA at last poll. No new firmware source changed.
- Resulting main commit: THIS COMMIT.

---

---

## 2026-11-21 — Match Optware teardown to exact mount ownership

- Parent main SHA: `94c0e2c11f6e73b90366ed6e74cb292858acb6e7`.
- Fixed an unsafe `opt-umount.sh` source check that grepped `/proc/mounts` for an unquoted device substring and ignored the supplied mountpoint. For example, `/dev/sda1` could match `/dev/sda10`, causing Optware teardown during an unrelated partition eject.
- Added `optware_lifecycle_mount_matches()`: require target `/opt`, validate every six-field mount record, compare source and mountpoint as exact fields, reject duplicate matches and return UNKNOWN for malformed/empty/unreadable inventory. Teardown proceeds only on one exact owner row and fails closed on UNKNOWN.
- Added shell regression cases for exact match, device-prefix collision, wrong mountpoint, malformed row, duplicate row, empty/unreadable inventory; mirrored these contracts in the Storage Hotplug workflow and standalone static checker.
- Prior validations: Process lifecycle `#44887824016` and AI handoff `#44887824043` PASS on `94c0e2c`; XY-C1 Build, TRX verification and artifact upload `#44887656164` PASS on product source `20e139d2` (L3 verified on that SHA).
- No physical-router L4 evidence is available. This Optware patch requires exact-head Storage Hotplug Actions after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Repair PID-file regression workflow YAML indentation

- Parent main SHA: `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Exact-head Process workflow run `#44887655190` failed before job creation because the new parser step had one extra leading space, breaking the workflow's step-level YAML indentation. Corrected the step to match adjacent six-space step/eight-space `run` indentation; no test or workflow coverage was removed.
- On the parent SHA, Process inventory `#44887656234` and AI handoff `#44887656223` both passed. The standalone parser regression also passed local GNU89/ASan/UBSan compilation and execution; the corrected exact-head Process workflow must still confirm integration.
- XY-C1 exact-source L3 is now verified: run `#44885238962` passed build, TRX validation and artifact upload on SHA `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Harden exported legacy shared PID-file signalling; confirm XY-C1 L3

- Parent main SHA: `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`.
- The legacy `kill_pidfile_s()` helper used `fgets` followed by `strtoul(..., NULL, 0)` and cast before range checking. This accepted numeric prefixes such as `2junk` and could wrap out-of-range records on wider hosts.
- Hardened the exported compatibility API without changing PID-only record syntax: nonblocking open plus regular-file validation, bounded full-record read, strict trailing/NUL checks, and `INT_MAX` validation before casting. Legacy decimal/octal/hex forms and surrounding whitespace remain accepted. Added GNU89 parser regressions, source integration assertions and Process-workflow push/PR path filters.
- Exact-source XY-C1 `#44885238962` completed SUCCESS on `8a55d24a4b797c96ca94bc8ece112966d86fa204`; Build XY-C1, TRX output verification and artifact upload all passed. Firmware build L3 is verified on that SHA; physical-router L4 remains unverified.
- The new standalone parser regression was compiled and run locally with GNU89, warnings-as-errors, ASan and UBSan: PASS. Exact-head Actions are still required for repository integration.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Remove obsolete watchdog ETIMEDOUT errno checker assertions

- Parent main SHA: `6ce50b1f558a99db1569fa6fd1a03aff4075e012`.
- Exact Process lifecycle run #44885848598 failed in `scripts/check_process_lifecycle_integration.py` at two duplicate assertions requiring `errno = ETIMEDOUT;` inside `start_watchdog()`.
- Production watchdog now supplies `ETIMEDOUT` as the explicit primary error argument to `watchdog_rollback_failed_start(-1, ETIMEDOUT, "readiness timeout")`; the preceding source-contract loop already verifies that exact readiness-timeout rollback call in both duplicated checker sections. Removed only the obsolete errno-in-body assertions, leaving the timeout policy check intact.
- AI handoff #44885848590 passed on the same parent SHA. Exact Process lifecycle Actions must rerun after this checker-only correction; no production code changed.

---

## 2026-11-21 — Correct Process watchdog rollback contract and handoff execution metadata

- Beijing time: 2026-11-21 16:31 (UTC+8). Parent main SHA: `accf1726b163d54bd269019fd590554fc430fa0e`.
- Watchdog signal workflow `44885579764` passed on parent `9a1175e8`. AI handoff `44885716352` then failed because the exact validator also requires the phrase `one atomic main commit`; updated the handoff mandate with that exact branch/commit invariant.
- Aggregate Process lifecycle `44885238958` on product-source SHA `8a55d24a` failed in the final source integration check because two copies of the watchdog contract still required `return launch_status;`. Both copies now require all six failure exits to invoke `watchdog_rollback_failed_start()`, check targeted stop/fresh-inventory adapters, verify errno preservation and assert every failure operation marker.
- No firmware production source changed. No local Python/C tests are claimed; exact-head Process and AI handoff Actions are required after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Match the handoff checkpoint heading to the exact validator contract

- Parent main SHA: `9a1175e8c5332751b68b7861c1ea5b043a67547d`.
- The exact Watchdog workflow passed on parent SHA (`44885579764`). AI handoff `44885579772` still failed: the validator requires the literal heading text `Mandatory handoff checkpoint` plus the exact conditional-document-edit sentence. The prior heading inserted `end-of-session` between those words, so it did not satisfy the contract.
- Changed the heading to `## Mandatory handoff checkpoint — end-of-session accuracy`; the required sentence remains present. This is a material correction to a failed exact-SHA contract, not a no-op documentation rewrite.
- Exact-SHA AI handoff must be rechecked after publication; no production files changed.

---

## 2026-11-21 — Fix duplicate watchdog workflow filters and restore mandatory handoff checkpoint wording

- Parent main SHA: `759eb1181483a74f8f4103ec7a5827fcac375dd4`.
- Exact Watchdog Action #44885340239 failed because the detached-daemon rollback header and regression test were each listed twice in both push and pull_request path filters. Removed only duplicates; the inline source-contract path list remains single-copy.
- Exact AI handoff Action #44885340194 failed because the rewritten handoff omitted the validator's exact end-of-session checkpoint heading and wording. Restored both required phrases.
- This follow-up changes no production behavior. Exact-SHA Watchdog and AI handoff results are required after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align watchdog workflow contract with targeted startup rollback

- Parent main SHA: `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- The first exact-head watchdog run for the detached-start rollback change still contained a stale inline assertion requiring `return launch_status;`. Production correctly routes the launcher failure through `watchdog_rollback_failed_start()` and therefore the old assertion would fail despite correct behavior.
- Updated the workflow to require all six post-launch failure exits to use the rollback helper, check the stop/inventory adapters, and include the shared rollback policy header/test in push and PR path coverage.
- No production source changed in this follow-up. Exact-SHA Watchdog, Process lifecycle and XY-C1 Actions must pass after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Roll back the watchdog daemon when detached startup cannot be verified

- Parent main SHA: `e6b112eb34b7e7fc7f3571e8e823e1ee91a9ea19`.
- `start_watchdog()` launched `/sbin/watchdog` through `eval()`, but `watchdog_main()` daemonizes before publishing its PID. Launcher failure, interrupted initial/retry waits, UNKNOWN/invalid process inventory and readiness timeout all returned directly without stopping a child that could still appear later.
- Added a targeted detached-start rollback adapter: stop only the `watchdog` process family, perform a fresh tri-state inventory scan, log incomplete cleanup, and preserve the original launch/wait/scan/timeout status and errno.
- Existing-running startup remains a no-op, and UNKNOWN preflight still fails closed before launch. The shared rollback policy regression already covers confirmed cleanup, a still-running child, UNKNOWN cleanup inventory and primary error preservation.
- Extended the watchdog source contract to require every post-launch failure branch to use rollback. Added the rollback-policy header and unit test to watchdog workflow path filters; XY-C1 already tracks the header and its input contract now asserts it remains a firmware input.
- Exact-SHA watchdog, Process lifecycle, AI handoff and XY-C1 Actions are required after publication; no local tests or new firmware L3 result are claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Ensure crond timeout always executes rollback

- Parent main SHA: `7085606f16b27e7ebc693cb0c3d3af29eb8e117b`.
- A post-commit source review caught a real status-propagation bug not covered by the current static tests: after a successful retry sleep followed by all-ABSENT probes, `crond` retained `status == 0`. Passing that zero into `detached_daemon_start_rollback_with_ops()` made its documented success fast path skip cleanup even though the function logged a startup timeout.
- The timeout now passes the explicit failure status `-1`; the crond lifecycle contract asserts this exact argument. Other core daemon timeout paths already pass `-1`/ETIMEDOUT.
- This is a production correctness fix discovered during review after the preceding Actions passed; exact-head Process Lifecycle and XY-C1 source validation are required again.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Update daemon lifecycle source contracts for rollback

- Parent main SHA: `1b25077e6e5a9f8750d4b5c76637beec5ca354d1`.
- The core RC daemon rollback change is now present on main. Its first exact-head test run compiled and passed the crond/infosvr C policy unit tests and most Process Lifecycle contracts, then failed three Python source validators that still expected the old direct-return behavior: infosvr, crond and the aggregated Process Lifecycle checker.
- Updated those validators to require the new targeted `optional_daemon_rollback_failed_start()` path, primary errno capture, UNKNOWN rollback handling and bounded timeout cleanup instead of the previous direct postcondition/status returns. The Process Lifecycle checker already verifies the shared adapter uses identity-fenced `kill_services()` and a fresh inventory.
- No production source changed in this commit. Exact-head CI is the verification target; no local checkout was available.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Serialize the SCUT watchdog stale-PID reclaim transaction

- Parent main SHA: `9ff6c64b9731a848ed07c0a08500d2c8608b09e7`.
- The stale-PID branch compared the old PID twice and then unlinked/recreated the PID file without inter-process serialization. Two overlapping cron invocations could both validate the stale record, race at unlink, and allow one invocation to remove the PID record claimed by the other.
- Added a stable adjacent lock file and BusyBox `flock -n 9` around the complete identity-check/reclaim/publish transaction. The persistent lock inode is never unlinked; FD 9 is closed immediately after the PID claim so health-check children and restarted daemons do not inherit the lock.
- Added a deterministic race regression that blocks the first invocation inside PID-file `rm` after the stale comparison, then starts a second invocation. It requires exactly one health check, retains the stable lock inode and rejects a symlink lock path.
- Confirmed BusyBox 1.24.x `CONFIG_FLOCK` defaults to y. Dedicated SCUT workflow guards the Kconfig dependency and host utility; XY-C1 path classification/trigger tests include the config source.
- Exact-head SCUT, AI handoff and XY-C1 Actions are required; no firmware L3 or hardware L4 result is claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Roll back failed detached starts for core RC daemons

- Parent main SHA: `c8923187b3027da02b41dd6d76ae00b2dc0b12cd`.
- Closed the late-child/orphan race in syslogd, klogd, infosvr, crond and networkmap. Each already waited for a detached RUNNING postcondition, but launcher errors, interrupted initial/retry waits, UNKNOWN inventories and readiness timeouts returned without stopping a child that could still have started.
- Added a shared RC adapter using `kill_services()` for only the affected process name, followed by fresh tri-state inventory. The common detached rollback policy keeps the original startup status/errno; incomplete cleanup is logged without masking the primary failure. Existing RUNNING preflight remains a no-op and UNKNOWN preflight still aborts before launch.
- Reused the previously ASan/UBSan-tested detached rollback policy and strengthened the Process Lifecycle source contract to require rollback wiring for all five daemons. Process Lifecycle and XY-C1 already watch `services.c` and the shared helper/test.
- Local build is unavailable; exact-head Process Lifecycle and XY-C1 Actions must validate the C integration. Router startup behavior remains unverified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Fix UDPXY checker NameError after detached-daemon policy validation

- Parent main SHA: `41023c0835fc399ea521e397a8fb5563dd86f02e`.
- Exact WAN route Action #44883618627 compiled and passed the detached-daemon policy regression, then failed in `scripts/check_udpxy_startup.py` because `igmpproxy_start` was used in a rollback assertion before the checker assigned its function body.
- Moved the XUPnPD and IGMPproxy function-body extraction ahead of their rollback assertions. No production behavior or assertions were removed or relaxed.
- Process lifecycle inventory #44883618647, process fail-closed regression #44883618648 and AI handoff contract #44883618619 passed on the parent SHA. Exact-head WAN route, XY-C1 and NVRAM runs remain required.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align IPTV rollback regressions with existing checker contracts

- Parent main SHA: `daf8248436c650eb10429d7098e52535c436ae4d`.
- Exact-SHA Actions for the IPTV rollback patch compiled and passed the new GNU89/ASan/UBSan unit test, but two existing source-check contracts needed alignment: the process checker assumed XUPnPD used the generic `iptv_proxy_confirm_started` wrapper (it has its own `xupnpd_confirm_started` wrapper), and the UDpxy checker deliberately expects the original test PASS line. Corrected the per-daemon confirm expectation and preserved the established stdout string.
- Extended the UDpxy checker to verify the new rollback policy, targeted identity-safe stop callback and rollback test cases across UDPXY/MSD Lite, XUPnPD and IGMPproxy.
- Run failures on SHA `734f8116` were checker-contract mismatches after the C unit itself passed; exact-head reruns on this fix and current main are authoritative.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Reset errno before UDPXY/MSD Lite launch

- Parent main SHA: `734f811620f596b9ea72863261d8cc4a7103bee2`.
- Tightened failed-launch diagnostics and rollback context: the UDPXY and MSD Lite launcher paths now clear ambient `errno` immediately before `eval()`, so a nonzero launcher result is not paired with an unrelated errno left by an earlier successful config/procfs operation.
- Added a source-contract guard for the ordering.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Roll back IPTV daemons after failed detached startup

- Parent main SHA: `9df0573782afb3a3e19864ac17250d7358ef26ca`.
- Fixed a lifecycle gap shared by UDPXY/MSD Lite, XUPnPD and IGMPproxy: each launcher required a bounded RUNNING postcondition, but a failed launcher/readiness check returned without reaping a child that appeared late or survived a nonzero launcher result. RC could report startup failure while the proxy remained active.
- Added a generic detached-daemon rollback policy that invokes a targeted stop callback, re-inventories process state, and preserves the primary startup status/errno. Production IPTV callbacks use `kill_services()` with only the failed daemon name; that existing stop path is PID/start-time identity checked. Cleanup logs confirmed absence versus remaining RUNNING/UNKNOWN state.
- Applied rollback independently to each failed IPTV daemon, leaving successfully started sibling services alone. Added GNU89 ASan/UBSan unit coverage for confirmed absence, stubborn RUNNING child, UNKNOWN inventory, successful-start no-op, invalid callbacks and primary-error preservation.
- Extended Process Lifecycle triggers and compilation, plus XY-C1 source-path filters. No local checkout was available; exact-SHA host CI and XY-C1 build are the required verification. Physical-router IPTV behavior remains unverified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Reset preserved UNKNOWN PID fixture before symlink case

- Parent main SHA: `bc72bd5df72543ced814d565177932db253c47a6`.
- The next exact-SHA watchdog run progressed through active-instance, recycled-PID, malformed-PID and UNKNOWN-procfs checks, then found the test reused the intentionally preserved UNKNOWN PID file when creating the symlink fixture. The fixture now explicitly removes that prior file before creating the symlink case.
- The failure was confined to test setup; production source behavior was not implicated.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Correct watchdog PID-file and procfs fixture escaping

- Parent main SHA: `59284736191e9480cdd7fcc77156bd8a1f9925a4`.
- The exact-SHA watchdog test exposed an escaping error in the generated shell text: the PID writer used a literal backslash-`n` instead of a line terminator, and the mock procfs fixture used literal backslash sequences instead of NUL delimiters. Corrected the two PID writes and all mock `printf` formats. The process-command-line reader already used the proper single-backslash `tr '\\000'` syntax.
- The earlier watchdog Actions failed only in the fixture path (first workspace allocation, then active-PID identity); this change addresses the second root cause. No extra production behavior was added beyond proper PID-file line termination.
- Resulting commit: THIS COMMIT.
