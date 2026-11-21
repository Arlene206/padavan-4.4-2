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
