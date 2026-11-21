## 2026-11-21 — Normalize multiline Samba runtime flag source assertions

- Parent main SHA: `bc3cceebfcc4423d03acd30a5604e01c63b13419`.
- Samba regression #44916836779 compiled and ran the expanded GNU89 mode/boolean policy test successfully, then failed in the inline source checker at its first runtime-reader assertion. The production code wraps `samba_read_integer_status()` arguments over multiple lines, while the new assertion expected a single-line substring.
- Changed the contract to normalize whitespace before checking both calls; no production source or assertion coverage was weakened.
- Exact-head AI handoff #44916836461 PASS; Process inventory #44916836495 PASS; WAN route #44916836585 and DNSMasq #44916836494 PASS. NVRAM audit #44916836473 was still running. XY-C1 #44916836560 had completed all host checks and was compiling Build XY-C1; TRX and artifact upload remained pending. New push reruns Samba contract; no L3 claim yet.

---

## 2026-11-21 — Fail closed on Samba enable and WINS NVRAM read errors

- Parent main SHA: `fbec957a7aa7ecb91ec538b7fa0ebe8acfd324af`.
- Fixed four service decisions that previously used `nvram_match()` without distinguishing a backend read error from a real value: global Samba enable/share mode in `run_samba()`, the pre-stop decision in `restart_smbd()`, selective `nmbd` stop when WINS is enabled, and WINS startup in `start_wins()`.
- Added shared status-policy validation and GNU89 unit cases for backend errors, invalid booleans, disabled-service short-circuit, invalid modes and all supported modes. Runtime mode reads retain legacy missing-key default 1; configuration generation retains missing-mode default 0. WINS start/stop preserve their legacy missing-key defaults (1/0).
- Extended source contracts to require checks before teardown, publication and launch; added missing Samba header/test dependencies to the pull-request path filter. XY-C1 workflow already watches both production source files.
- Parent exact-head `fbec957`: WAN callback #44915851413, WAN route #44915851347 and AI handoff #44915851360 PASS. XY-C1 #44915851356 host validation PASS, but Build XY-C1, TRX verification and artifact upload were SKIPPED because the parent commit changed no production input. The current production-source commit requires a new full same-SHA Build/TRX/artifact result. No L3/L4 claim.

---

## 2026-11-21 — Align WAN callback and route regressions with split rollback status

- Parent main SHA: `6882aefa2c70b7bfc53da5e770610dc45dc802ba`.
- Exact-head WAN callback #44915375540 failed at its redundant publisher-signature count: the checker counted the literal escaped sequence rather than the C prototype, while the preceding signature assertions already verify one prototype and one body. Corrected the assertion to count the actual parsed publisher signature.
- WAN route #44915375609 and XY-C1 pre-build validation #44915375738 both exposed a stale contract in `scripts/check_wan_firewall_status.py`: it still expected recovery after every publication error. The publisher now distinguishes candidate status from in-transaction compensation status and retries only when compensation or transaction close failed. Updated the checker to require the separate status, callback argument and guarded-retry predicate.
- Other exact-head Actions on `6882aefa`: Process lifecycle/inventory, NVRAM transaction audit, AI handoff, WAN teardown, watchdog, DNSMasq, detect_link, USB modem, miniupnpd, Firewall, gateway ARP, Wireless, VPN, router-mode NVRAM, detect_internet and ZeroTier PASS. WAN callback, WAN route and XY-C1 pre-build step failed as described.
- XY-C1 did not reach firmware compilation: its pre-build WAN firewall contract failed. TRX verification and artifact upload did not run. Exact-head Build + TRX + artifact validation must rerun after these checker fixes. No production firmware build or hardware L4 pass is claimed.
- No production source changed in this slice. The next independent reliability item is fail-closed checked NVRAM reads for Samba global-enable/mode and WINS stop policy.

---

## 2026-11-21 — Gate WAN DHCP recovery retry on actual rollback failure

- Parent main SHA: `067d41bbfc677e65867a3ce434c688a4a04f29f9`.
- The publisher was retrying its guarded restore after any failed candidate publication, even when the first transaction had already compensated all attempted writes successfully. That redundantly re-applied the previous tuple and could convert a successful rollback into a spurious recovery failure.
- Split candidate-publication status from the in-transaction rollback status. The fresh guarded recovery now runs only when compensation failed or the transaction close itself failed; stale/read-preflight failures do not enter recovery, and successful in-lock compensation is reported as rollback status zero.
- Extended the WAN callback source contract to require the separate rollback-status output, the guarded retry predicate and exactly one publisher implementation.
- Exact-head WAN callback #44915189957 and AI handoff docs #44915189829 passed on the parent SHA `067d41b`. XY-C1 #44914789805 on `2bfb2aa` was still running at the last poll (Build in progress; TRX/artifact pending); this source change requires a fresh exact-SHA build.
- Updated `SESSION_HANDOFF.md` with the live source/checker evidence and first next action. The backup branch remains untouched.
- Next: inspect WAN callback and XY-C1 Build/TRX/artifact on this patch's SHA.

---

## 2026-11-21 — Fix WAN callback checker use-before-initialization

- Parent main SHA: `0c172fa0e5c7454af1052aa2fee84c2803d6e6b8`.
- WAN callback run #44915003721 on `0c172fa` passed all policy binaries and earlier source contracts, then failed at the first reference to `lease_policy`: its source/test files were initialized later in the same inline Python block.
- Moved the `dhcp_lease_policy.h` and `tests/dhcp_lease_policy.c` reads above their first use, removing the later duplicate reads. Existing compare-and-publish, stale-restore and reverse-snapshot assertions remain unchanged.
- AI handoff contract #44915003659 PASS on `0c172fa`; NVRAM audit #44914790008 PASS on product source `2bfb2aa`; Process regression/inventory PASS on that source SHA. XY-C1 #44914789805 still running on `2bfb2aa`, with Build XY-C1 active and TRX/artifact pending at last poll.
- This is a checker-only correction; no local test or build is claimed. L3 remains pending same-SHA Build/TRX/artifact; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Repair WAN workflow YAML indentation and restore handoff contract

- Parent main SHA: `2bfb2aa93f525762685d49ad50ff89d498eacdde`.
- WAN callback workflow #44914788251 was registered without any jobs because the whitespace-normalized reverse-snapshot assertion inserted in the previous slice was missing the YAML block's ten-space indentation. Restored that indentation so the inline Python remains inside the step block.
- AI handoff docs #44914790056 failed only because the live handoff omitted the validator's required exact phrase "one atomic main commit". Restored that cross-session atomic-main-commit rule and refreshed the checkpoint with exact SHA run statuses.
- Process lifecycle #44914789828 and process inventory #44914790193 PASS on `2bfb2aa`; XY-C1 #44914789805 and NVRAM audit #44914790008 remain IN_PROGRESS on the same SHA. A fresh WAN callback and AI handoff push run is required after this repair.
- No production source changed in this slice; no local regression or build is claimed. L3 remains NOT VERIFIED until Build, TRX and artifact upload pass on one exact SHA; L4 hardware validation remains NOT VERIFIED.

---

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

