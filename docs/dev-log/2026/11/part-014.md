## 2026-11-17 — Repair escaped firewall checker regex and HTTPD hook helper signature

- Exact-head 8956 Firewall Startup `43945597477`, Process regression `43945597663`, and XY-C1 `43945597624` failed because the checker regex was emitted with two backslashes in a raw string, matching literal backslashes rather than whitespace. The production source contains nine status-aggregation calls; the regex now uses one escape level.
- Exact-head 8956 HTTPD `43945597526` failed at its inline helper boundary because the assertion expected `static void firewall_log_hook_stage_failure`, while the implementation now returns `int` to propagate staging failures. Updated the expected signature.
- Exact-head 8956 AliyunDrive `43945597544`, AliDDNS `43945597596`, AdGuardHome `43945597557`, AI handoff `43945597849`, USB modem `43945597613`, and infosvr `43945597566` passed; NVRAM `43945597771` was pending. The old SHA's failures are not treated as fixed until the new exact-head suites pass.
- Next P1: audit module unload/load postconditions for configured NAT helpers and optional HW NAT/SFE. No XY-C1 L3 or physical-router L4 claim for this source state.

---

## 2026-11-17 — Normalize optional firewall hook ENOENT checker

- Exact-head 8c77 Firewall Startup `43946213665` and Process regression `43946213545` passed all preceding firewall/module/status policy tests but the canonical checker expected `errno!=ENOENT` after `compact()`; the normalized actual source is `errno != ENOENT`. Changed the assertion to a whitespace-tolerant `re.search`.
- Exact-head 8c77 XY-C1 `43946213553` stopped at this checker before Build/TRX/artifact. HTTPD `43946213772`, NVRAM `43946213544`, AI handoff `43946213616`, AliyunDrive `43946213652`, AliDDNS `43946213539`, AdGuardHome `43946213561`, USB modem `43946213534`, and infosvr `43946213737` passed.
- Next P1 remains NAT helper and HW NAT/SFE module unload/load postconditions. No physical-router L4 claim.

---

## 2026-11-17 — Align NAT module assertions with policy inputs and HW NAT API

- Exact-head 39190 Firewall Startup `43947724068` and Process regression `43947724044` failed at the new source checker because it reused `POLICY_SOURCE/POLICY_TEST`, which refer to modem-WAN policy files. Added dedicated `MODULE_POLICY_SOURCE/MODULE_POLICY_TEST` bindings for the wireless module policy and regression.
- Exact-head 39190 HTTPD `43947724129` failed because its inline checker expected the old single-argument `hwnat_configure` signature. Updated the contract to the allow-aware two-argument function and its call.
- 39190 USB modem identity `43947724775`, AI handoff `43947724226`, process inventory `43947724196`, and WAN route `43947724172` passed; NVRAM, USB storage and XY-C1 were pending. This is validation repair only; no firmware result inferred.
- NAT module unload policy remains under exact-head validation; physical-router L4 is NOT VERIFIED.

---

## 2026-11-17 — Include the inline service-start result policy in firewall_ex

- Exact-head 09ea XY-C1 `43947164750` reached MIPS link and failed with undefined references to `rc_first_service_start_status` from `firewall_ex.o`. The helper is static-inline in `service_start_status.h`; `firewall_ex.c` had not included that header. Added the include and added a canonical source assertion.
- Exact-head 89a43d9 Firewall Startup `43948306770`, Process regression `43948306885`, HTTPD `43948306816`, AI handoff `43948306828`, USB modem `43948306849`, infosvr `43948306784`, AliDDNS `43948306867`, AdGuardHome `43948306841`, and AliyunDrive `43948306780` passed. NVRAM `43948306748` remained pending and XY-C1 `43948306746` was still building.
- This is a production link correction. Exact-SHA Build/TRX/artifact must pass again; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Fail closed on RC module-state probe errors

- Main parent: `1aca5ab3a776a66138814769b84d5e5ceb43b46c`. Audited `common_ex.c:is_module_loaded()`: it returned 0 for all `opendir("/sys/module/<name>")` failures, conflating absence (ENOENT) with permission/resource/sysfs errors. `rc_module_unload_verified()` could consequently skip an unload and falsely report absence.
- Added tri-state `is_module_loaded_status()`: 1=loaded, 0=ENOENT only after the sysfs module root opens, negative errno=invalid path or probe failure. Preserved the legacy boolean `is_module_loaded()` wrapper and bound NAT unload verification to the strict probe.
- Added injectable regressions for an initial probe error (must not call unload) and a post-unload probe error (must return the error). Updated canonical source checks and push/PR path coverage.
- Exact 1aca runs before this slice: Process regression `43952107815`, HTTPD `43952107789`, AI handoff `43952107818` passed; XY-C1 `43952107870` was `in_progress`. This commit needs fresh exact-SHA results. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate Adbyby update action failure to PID 1

- Main parent: `e09e044195fd936142ed0b42ead1bd73f6262c02`. In the RC notification dispatcher, `RCN_RESTART_UPDATEADB` called `update_adb()`; that void wrapper ignored the wait status returned by `eval("/usr/bin/adbyby.sh", "updateadb")`. A failed update therefore left the notification action result unchanged.
- Changed `update_adb()` to return int, updated its public prototype, and merged its result into `action_result` with `rc_notify_merge_action_result()`. Added an integration-checker contract for the helper, declaration and actual notification caller.
- Strict module-probe commit 4c435 had 38/39 Actions passed at last poll; its XY-C1 Build/TRX/artifact run `43953778444` was still in progress. The Adbyby fix requires a new exact-SHA verification. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate HW NAT radio configuration failures

- Parent main: `93d0254006bea2e5743715e9b039bb51a0c09f52`. HW NAT radio-registration commands for 2.4 GHz and 5 GHz plus the IPv6 debugfs write previously discarded their `doSystem()` results.
- Added wait-status normalization (including command-launch errno), independent attempt semantics for both radios and IPv6 debugfs, first-error propagation into the NAT reload result, and fault-injection coverage for success, nonzero command exit, signal termination, command-launch failure, disabled/unloaded module and invalid callback.
- Exact-head `0b32b9111c38e2cf7588cf6605b2491f25e3ce31` had Firewall and Process regressions pass; XY-C1 Build/TRX/artifact passed. HTTPD initially failed only on a newline-sensitive source assertion; it was corrected in `e928181da3a52b4035d1ccf03307f212cafa2a4e`. Exact-head `e928181` HTTPD, AI handoff and XY-C1 (including Build, TRX and artifact upload) all passed. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Fail closed when WAN interface route cleanup fails

- Parent main: `e928181da3a52b4035d1ccf03307f212cafa2a4e`. `full_restart_wan()` ignored both `clear_if_route4()` command results and continued into switch table clearing, WAN variable reset, static route restoration and WAN restart. A stale interface route could survive undetected.
- Changed `clear_if_route4()` to return the `eval()` status and pass the interface name as a single argv element rather than shell interpolation. Added `rc_wan_route_flush_apply()`: attempt both interface flushes, retain the first failure, and stop the WAN rebuild before switch/MAC and route reconfiguration when either flush fails.
- Added GNU89/`-Werror`/ASan/UBSan regression for two-interface attempt order, first-error retention, later-only failure, invalid interface and invalid policy arguments. Process lifecycle run `43960589874` passed this test on source SHA `58318f5`; WAN route regression `43960590043` also passed.
- Workflow path filters were repaired in `f2d9ccef50d3ebfc0f6f5dde94fae7893c7bb58d`. Exact-head Firewall Startup `43961249138` passed. HTTPD `43961249141` exposed a whitespace-sensitive checker for a multiline call (the compiled route test passed); checker normalization is fixed in `eb566f346f63d3d6ed295c08170ac6d834ff7f04`. Current-head HTTPD `43961727463` passed. Current-head XY-C1 `43961726533` passed all pre-build checks and route host regression; Build XY-C1 was in progress at last poll, with TRX/artifact pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Preserve tri-state module-load probe failures

- Main parent: `beee6893b7c14eccc94634fe51dc41d2e748b8b7`. Strict `is_module_loaded_status()` already served unload checks, but the load verifier still treated any nonzero callback value as success and four production loaders still used the lossy Boolean wrapper.
- Updated `rc_module_load_verified()` for 1=loaded, 0=absent and negative errno=probe failure. Migrated NAT, boot hardware, wireless driver and LAN bridge module-load sites to the strict probe.
- Added injected tests for direct errno propagation and for trying all required modules while retaining the first failure. Extended Process/WAN source contracts to cover every production loader and both regression cases.
- Parent exact-head AI handoff passed; WAN route regression was running after correcting its path-filter assertion. The prior `eb566f3` XY-C1 Build/TRX/artifact passed, but the newer IPv6 cleanup change and this source slice require exact-head verification. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Restore strict module-probe CI contracts and developer-log history

- Parent main: `b6ff015c1a70e9b61bebfbcf33aae9d1de113946`. Commit `b6ff015c` finishes migration of all four module-load sites to the strict tri-state probe; the source checker now verifies the boot and wireless callbacks plus negative errno propagation.
- Exact-parent Firewall Startup `43966109926` passed. WAN route `43966109986` failed only at a whitespace-sensitive assertion for the multiline IPv6 cleanup call. XY-C1 `43966109916` stopped at a stale checker expecting the previous Boolean load policy. AI handoff `43966109747` found only one root developer-log entry; restored the full 109-entry log from the pre-truncation parent snapshot.
- Process regression `43966109822`, XY-C1 `43966109916`, NVRAM `43966109901`, GPIO `43966109653`, notification queue `43966109981`, AliDDNS `43966109957` and USB storage `43966109731` were pending/in progress at last poll. Other completed b6ff Actions passed except the failures above; query exact SHA for current outcomes.
- Earlier source SHA `eb566f3` passed XY-C1 Build/TRX/artifact, but predates later IPv6/module-load changes. b6ff XY-C1 did not reach Build; newer L3 remains NOT VERIFIED. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate PPPoE relay failure through sysctl notification

- Parent main: d28729af83c9692f376539c31d9578f129774d20. set_passthrough_pppoe(1) returns an errno-style status, but restart_all_sysctl() discarded it and its only RCN_RESTART_SYSCTL caller also ignored the helper result. PID 1 could therefore acknowledge a failed PPPoE relay startup/readiness operation.
- Changed restart_all_sysctl() to return/log the relay status, keep optional Samba fastpath work independent, update the public prototype and merge the result through rc_notify_merge_action_result() at the sole PID 1 dispatcher callsite.
- Added a host regression for positive errno retention across later independent work and conversion to a negative/unacknowledged notification result, plus an integration contract checking the actual definition, prototype and caller. Exact-head Process/XY-C1 verification is pending.
- Physical-router L4 remains NOT VERIFIED.

---
## 2026-11-16 — Correct whitespace contract in sysctl status checker

- Parent main: 1e10514145025ceb0ce9e0a0cd5468e28e6b7fa0. The first follow-up tried to make the function-signature assertion independent of line formatting, but exact Process CI later showed the remaining failure was a separate multiline rc_first_service_start_status() call assertion: whitespace remained after the opening parenthesis. This is corrected by the next entry.
- XY-C1 exact-head run 43967039935 on d28729a completed successfully: Build XY-C1, TRX verification and artifact upload all PASS. The later 1e105 sysctl source change still requires its own exact-head Build/TRX/artifact run.
- The Process lifecycle regression on 1e105 was queued when last checked; this checker correction is included before its first execution. Exact-head Process/XY-C1 results for this repair remain pending.
- Physical-router L4 remains NOT VERIFIED.

---
## 2026-11-16 — Propagate unknown NAT module state through HW NAT, FTP and SFE

- Parent main: `d83760b4c8e5514fd996f986b1400b7b794876f4`. Confirmed a remaining module-state defect in `net.c`: HW NAT, FTP and SFE branches used the legacy Boolean probe before invoking strict load/unload helpers. A negative `/sys/module` probe could be treated as absence and silently skip a required teardown or configuration check.
- HW NAT now returns tri-state status; disabled mode invokes verified unload unconditionally, enabled mode surfaces initial probe errno, only verified-loaded modules receive radio sysfs writes, and configuration propagates unknown module state. FTP parameter reconfiguration is blocked on UNKNOWN rather than risking a live helper mutation. SFE enable/disable now uses verified load/unload on every transition and returns the actual failure status.
- d287 exact-head XY-C1 `43967039935` passed Build XY-C1, TRX verification and artifact upload; WAN route `43967040074`, Process `43967040068`, AI handoff `43967040010`, NVRAM `43967039938`, HTTPD/AdGuardHome `43967039819`, infosvr `43967039889`, AliyunDrive `43967040044`, AliDDNS `43967040108` all passed. This is L3 only for d287, not the new source.
- d837's unrelated sysctl checker correction has AI handoff `43969335259` PASS; Process regression `43969335166`, NVRAM `43969335303`, AdGuardHome `43969335428`, AliDDNS `43969335268` were in progress at last poll. New exact-head Process, Firewall Startup, XY-C1 and handoff tests must validate this module-state source slice. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Normalize whitespace in PPPoE sysctl status assertion

- Parent main: 1554a5f3fbb1bf13c3325ad765b7e806d74d1537. Exact-head Process run 43969335166 reached the new sysctl integration contract, then failed because the multiline rc_first_service_start_status() call left whitespace immediately after its opening parenthesis. The checker expected a no-space argument sequence.
- Updated the assertion to remove all whitespace from the function body before matching the exact helper call, without weakening the production definition/prototype/dispatcher contract.
- The d287 exact-head XY-C1 run 43967039935 passed Build, TRX verification and artifact upload; it predates the new sysctl source. Current-main Process/XY-C1 validation is pending.
- Physical-router L4 remains NOT VERIFIED.

---
## 2026-11-16 — Align NAT tri-state caller contracts across CI

- Parent main: `ebf2d285454eba3480fe594808aa31367513ffe4`; source implementation is already in `1554a5f3`. Exact-head 1554 Process `43969810170`, Firewall Startup `43969809710` and XY-C1 `43969809836` all reached `check_modem_wan_start_status.py` and failed at the same stale assertion: the source stores `-module_loaded` in `status` then returns `status`, while the checker incorrectly required a direct `return -module_loaded`. Exact 1554 HTTPD `43969809668` failed its stale HW NAT radio assertion, which still expected the pre-change `hwnat_loaded` argument.
- Updated the common source checker to assert the actual errno assignment and the HTTPD integration contract to require a successful tri-state probe before policy/config use and to guard radio writes on `hwnat_loaded == 1`. This is a test-contract correction; production source remains unchanged in this commit.
- Exact-head ebf2 AI handoff `43970209185`, NVRAM `43970209177`, AdGuardHome `43970209909`, infosvr `43970209228`, AliyunDrive `43970209300`, AliDDNS `43970209283` passed. Process inventory `43970209366` failed at the same stale `return -module_loaded` checker assertion; it is corrected here.
- d287 exact-head XY-C1 `43967039935` previously passed Build, TRX output and artifact upload; that L3 evidence does not cover the newer HW NAT/FTP/SFE source. Exact-head reruns are required after this checker correction. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Finish strict HW NAT CI contract and exact checkpoint wording

- Parent main: `99d8562d8b007e83ceaff397f7c11337b9c6277a`. Exact 99d856 actions exposed one last stale common source assertion: `reload_nat_modules()` now configures radio settings only under `hwnat_loaded == 1`, passing literal 1 to the policy callback, but the checker still required the old raw `hwnat_loaded` argument. Replaced it with explicit verified-loaded gating and no-legacy-helper assertions.
- Exact-head 99d856 Process `43971105805`, Firewall Startup `43971105848` and XY-C1 `43971105719` all failed at that same checker assertion before their downstream integration/build steps. The 99d856 AI handoff `43971105596` found the handoff omitted the exact sentence `The checkpoint is mandatory; a document edit is conditional.`; this commit adds the required exact contract wording.
- Exact 99d856 PASS: NVRAM `43971105616`, USB modem identity `43971105688`, infosvr `43971105912`, HTTPD `43971105585`, AliyunDrive `43971106167`, AliDDNS `43971105599`, AdGuardHome `43971106008`. No same-SHA firmware build/TRX/artifact completed because XY-C1 stopped at the shared checker; current-source L3 remains NOT VERIFIED.
- Source HW NAT/FTP/SFE code remains as in `1554a5f3`; this atomic follow-up changes only checker/documentation contracts. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate conntrack table flush failures through RC teardown

- Parent main: `1f8299371114d2fb846ec1c47a0116afd584668c`. Traced all RC references: `flush_conntrack_table()` was declared `void` and discarded the errno-style result of shared `fput_string("/proc/net/nf_conntrack", ip)`. All four callers ignored write failure: PID 1 `RCN_RESTART_NETFILTER`, LAN teardown, WAN teardown and MAN DHCP deconfig. The shared writer returns errors from buffered writes and close.
- Changed the API to return the writer result and added an injectable policy regression for targeted IP writes, full-table `"f"` flush, exact errno propagation and missing writer rejection. PID 1 merges the status into its notification result; LAN, WAN and MAN teardown retain the flush error while still performing later cleanup.
- Added the test to the GNU89/ASan/UBSan WAN route workflow and source-contract checks for every production caller. New exact-head WAN, Process, Firewall Startup, XY-C1 and AI handoff actions are required after publication.
- Exact `1f829937` Process `43971856210`, Firewall Startup `43971856255` and AI handoff `43971856365` passed; XY-C1 `43971856235` was still building at last poll. This evidence predates the conntrack source change. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Normalize conntrack deconfig status contract after exact-head test

- Parent main: `cffc1497fda9f65d9a0b163bad5acc1723c23f81`. Exact-head WAN route run `43973861975` compiled and passed the new conntrack errno policy regression (GNU89, ASan, UBSan), all existing route/IPv6/bridge tests and the WAN VLAN source check. Its final source-contract step failed only because the new assertion compared whitespace-formatted `udhcpc_deconfig()` code without removing all whitespace; the production return aggregation is intentionally split across lines.
- Exact-head Process `43973862332`, Firewall Startup `43973861431` and AI handoff `43973861519` passed. XY-C1 `43973861873` was queued at last poll. This fix removes whitespace before comparing the nested first-error aggregation expression; no production source changes.
- Earlier exact `1f829937` XY-C1 run `43971856235` passed the complete firmware build, TRX output verification and artifact upload. That L3 evidence predates the conntrack source/API change. Current cffc source L3 remains NOT VERIFIED.

---

## 2026-11-16 — Fix second stale MAN conntrack status contract

- Parent main: c3bcb486aa97e0dbb478f05585e6c24ab33407bc. Exact c3b WAN route run 43974249410 passed earlier injected conntrack, bridge/IPv6 and route tests; its final source contract failed at Python line 304 because a legacy assertion still expected return route_status_accumulate(interface_status, cleanup_status). Since conntrack_status is now a first-class teardown result, production correctly returns the nested first-error aggregate that includes it.
- Updated that remaining assertion to remove whitespace before comparing the full expression. This is a checker-only correction; no production source changes. Exact c3b Process 43974249384 and AI handoff 43974249444 passed; cffc Firewall Startup 43973861431 passed. cffc XY-C1 43973861873 remained in progress at last poll.
- Current product-source L3 and physical-router L4 remain NOT VERIFIED until exact evidence exists.

---

## 2026-11-16 — Update WAN DHCP callback contract for conntrack cleanup status

- Parent main: `44fd9bedf4b7b1ab4e9d4408a6157681158774d3`. Exact `44fd9bed` WAN route, Process inventory, AI handoff, NVRAM, AdGuardHome, AliDDNS, AliyunDrive and infosvr Actions passed.
- Exact cffc WAN DHCP callback run `43973861977` passed its C regressions, then failed its final source-contract assertion because the test still required `return route_status_accumulate(interface_status, cleanup_status);`. `udhcpc_deconfig()` now also aggregates the conntrack write result, so the old two-status assertion was stale.
- Updated the WAN DHCP workflow to require `conntrack_status` initialization, MAN address flush, error check, flush-before-cleanup order and the full nested first-error return. No production source changes; exact-head WAN DHCP rerun is pending.
- XY-C1 run `43973861873` for product source SHA cffc was still building at last poll. Current product L3 and physical L4 remain NOT VERIFIED.
- Next: inspect the exact-head WAN DHCP run and complete XY-C1 Build/TRX/artifact verification, then trace `flush_route_caches()` and every caller before any API change.

---

## 2026-11-16 — Preserve LAN static-route flush failures during teardown

- Parent main: 344db6b6cb4309ad1a2cdbf29b2cc0ee50ad5898. In stop_lan(), clear_if_route4(IFNAME_BR) returned the route-flush result but its caller discarded it. If conntrack and bridge ifconfig succeeded, a failed static-route cleanup was silently reported as a successful LAN teardown.
- Captured and logged route_status, then nested first-error aggregation across conntrack cleanup, route flush and bridge shutdown. IPv6 cleanup and ifconfig remain attempted after a route failure; AP-mode behavior is unchanged.
- Added service_start_status regression coverage for route-only failure, earlier conntrack error precedence, and bridge-down-only failure. Added Process integration assertions for result capture, aggregation and route→IPv6→interface operation ordering. Existing Process and XY-C1 workflows already compile this test and trigger on net_lan.c.
- Parent source L3: XY-C1 run 43973861873 on cffc149 passed Build, TRX verification and artifact upload. The exact 344db6b handoff rerun and this slice's exact-head Process/WAN/XY-C1 checks are pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Align WAN route checker with LAN teardown status propagation

- Parent main: 759c18d860ba1c0cb547f43fa7ca8d21b6580987. The LAN route cleanup source change in 759c18d correctly captures clear_if_route4(IFNAME_BR), logs a nonzero result, and returns the first failure across conntrack, route cleanup and bridge shutdown while continuing IPv6/interface cleanup.
- Process integration and service-status regression passed on 759c18d (43976717949 and 43976718109); WAN DHCP callback 43976717974 and AI handoff 43976717913 passed. WAN route run 43976718132 failed at its inline stop_lan() contract because it still required the previous two-status return rc_first_service_start_status(conntrack_status, status).
- Updated the WAN route workflow assertion to require the nested conntrack/route/bridge status aggregator and verify route flush precedes IPv6 and interface cleanup. Checker-only change; production source unchanged. Exact-head WAN route rerun required.
- XY-C1 43976717898 was pending at last poll. Product-source baseline cffc149 remains L3 VERIFIED by successful Build/TRX/artifact; LAN route slice still awaits same-SHA L3 evidence.

---

## 2026-11-16 — Propagate global route-cache flush failures through full WAN restart

- Parent main: `2c503c9ee1b3cda569700d3e85f614eed0317ad8`. `flush_route_caches()` discarded the result of `system("ip route flush cache")`, and its only RC caller, `full_restart_wan()`, ignored the result before switch/WAN reconstruction.
- Converted the wrapper to return `eval("ip", "route", "flush", "cache")`, updated the public prototype, and combined global-cache and per-interface route flush results using first-error semantics. The cache attempt still runs when an interface flush fails; route-dependent WAN restart work is blocked if either cleanup stage fails.
- Added injected regression cases for route-first failure, cache-only failure, successful cleanup, and missing callback. WAN route CI compiles the test with GNU89/ASan/UBSan and checks source API, caller wiring and restart gate.
- Parent exact WAN route `43977147385`, Process `43977147335`, and AI handoff `43977147281` passed; NVRAM `43977147536` was pending. Exact 759c18 XY-C1 `43976717898` was still building. This route-cache change needs its own same-SHA Build/TRX/artifact verification. Physical-router L4 remains NOT VERIFIED.
- Next: inspect exact-head WAN route/Process/AI/NVRAM/XY-C1 Actions and resolve first causal failure.

---

---

## 2026-11-16 — Align shared and HTTPD route-startup contracts with global cache flush API

- Parent main: `bb0c693125f0ddfd4e845d6c9d4f8fb58f68d2e2`; product-source route-cache change is `493f981e4db110923ca8d16328e29aec541a3734`. The WAN route workflow `43978255126` passed, including the injected GNU89/ASan/UBSan global-cache regression.
- Exact 493f Process regression `43978254976`, Firewall Startup `43978255243`, XY-C1 `43978254941`, and HTTPD security `43978255197` all failed at stale source contracts left behind by the intentional `full_restart_wan()` API change: the common checker still required the old direct `rc_wan_route_flush_apply()` plus separate `flush_route_caches()`, and HTTPD still required the pre-composite helper call. Production source was not implicated by these failures.
- Updated both shared checker contracts to assert the composite `rc_wan_route_flush_and_cache_apply()` call, the global cache callback, first-error gate and ordering before switch reconstruction. The exact 493f AI docs failure was separately fixed by `bb0c693`, which restored the 22 previous complete root records plus the 493f record; exact AI handoff run `43978638161` now PASSes with 23 entries.
- The exact 493f XY-C1 workflow stopped in its pre-build source checker, so route-cache product-source L3 is **NOT VERIFIED**. Exact 493f WAN route `43978255126` PASS; exact bb0c Process common checker `43978638091` still failed at the same stale assertion.
- THIS COMMIT is checker/documentation only and requires fresh exact-head Process, Firewall, HTTPD, WAN route and XY-C1 results. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate LAN IPv6 address cleanup failures

- Parent main: 8befa2b20295be8182053dc4e224174db471ed82. clear_lan_addr6() currently discarded clear_if_addr6(IFNAME_BR)'s errno-style result, allowing stop_lan() to report success after failed IPv6 address cleanup.
- Changed clear_lan_addr6() and rc.h declaration to return int. stop_lan() captures/logs ipv6_status and aggregates first failure in conntrack → IPv4 route → IPv6 address → bridge-down order without skipping bridge shutdown.
- Added service_start_status regression cases for IPv6-only failure, prior route error precedence and bridge-down-only failure; Process checker validates wrapper return and production aggregation. WAN route checker validates updated result order; XY-C1 path filter adds net_lan6.c.
- Exact 8bef AI handoff 43979147640 PASS; Firewall Startup 43979147580, USB modem 43979148326, AdGuardHome 43979147629, infosvr 43979147611 and AliDDNS 43979147585 PASS. Process 43979148973 and XY-C1 43979147638 were pending at last poll; current-source L3 and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-16 — Make LAN IPv6 address reload fail-visible and shell-safe

- Parent main: `7c46a6055c60c2fc534610cedd9189cf30ace4d6`. The previous `reload_lan_addr6()` returned void, discarded stale-address flush and address-add results, and interpolated the NVRAM address into shell-formatted `doSystem()`. LAN startup and IPv6 restart could report success after address setup failed.
- Added an injectable reload policy: stale-address clear must succeed before publishing a new address; empty configuration only clears; add failures are returned; missing callbacks fail closed. Production addition uses argv-based `eval("ip", "-6", "addr", "add", ...)` rather than shell interpolation.
- Changed `reload_lan_addr6()` to return int and propagate its result through `start_lan()` and `full_restart_ipv6()` while preserving first-error semantics and continuing independent bridge/WAN/DNS work.
- Added GNU89/ASan/UBSan tests for clear-failure gating, add failure, empty configuration and missing callbacks. Updated Process, Firewall, WAN route and XY-C1 checks.
- Parent exact Actions: Process `43979698102`, Firewall `43979698600`, AI docs `43979698471`, HTTPD security `43979698034` PASS; WAN route `43979698355` in progress; XY-C1 `43979698609` building after host/source tests, with TRX/artifact pending. New source requires exact-SHA verification. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Correct LAN IPv6 reload checker normalization

- Parent: `176e93e3e20889bdddcdab8769f3a4e9665ef01e`. Exact-source CI exposed a checker-only regression: `compact()` collapses whitespace to a single space, but newly added assertions expected all whitespace to be removed.
- Fixed those assertions to match the checker's established normalization contract. No production C code or policy test changed.
- Exact parent runs: WAN route `43982636461` PASS, including new IPv6 reload regression and inline contracts; AI handoff `43982635895` PASS. Process `43982636314`, Firewall `43982636272`, XY-C1 `43982636425` failed at that same assertion; the new C policy test passed in Process before the assertion. XY build/TRX/artifact skipped and remain unverified.
- Rerun all exact-head suites after this commit. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Pass dynamic WAN IPv6 address arguments without a shell

- Parent: `4cdf85227650738b7a9c105322fab12e5b5d06cc`. Audited `net_wan6.c`: three dynamic IPv6 address additions used `doSystem("ip -6 addr add %s dev %s", ...)`. The IPv6 address strings are generated from validated binary IPv6 structs or passed through `format_ipv6_literal()`; replacing shell formatting with argv execution removes unnecessary shell parsing and prevents future input regressions.
- Converted SIT WAN address, derived SIT LAN address and native static WAN address additions to `eval("ip", "-6", "addr", "add", ...)`. Existing WAN startup macros retain the exact command status and failure rollback path.
- Added shared checker and WAN route workflow assertions for all three argv calls, absence of the shell-formatted form, and preservation of address construction/validation.
- Exact parent checks: Process `43983013930`, Firewall `43983014009`, AI handoff `43983014049` PASS; XY-C1 `43983014153` was building with TRX/artifact pending. WAN route had passed on preceding source SHA `176e93e`; this new slice needs its own exact-head checks.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Align WAN DHCP callback contracts with argv-based IPv6 address adds

- Parent source SHA: `b7aa7296aab8274461a68182402357352a9d905e`. WAN DHCP callback Actions run `43983424171` failed in its inline Python source contract because it still expected the three IPv6 address-add operations to use shell-formatted `doSystem()` calls.
- The production source had intentionally converted the SIT WAN, derived SIT LAN and native static WAN address-adds to argv-based `eval()`; WAN fail-fast macros still capture their status. This is a stale checker failure, not a runtime/test failure.
- Updated the callback workflow's required-operation and status-gating assertions to require the argv calls and added guards against reintroducing shell-formatted address additions. No production source changed.
- On exact `b7aa7296aab8274461a68182402357352a9d905e`, WAN route `43983424396`, Process `43983424164`, Firewall `43983424297`, AI handoff `43983424148` and HTTPD `43983424199` PASS. XY-C1 `43983424207` had passed all pre-build checks and was building; TRX/artifact pending. Product-source L3 and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-16 — Propagate IPv6 global sysctl failures

- Parent: `8291da3892430ca915c5414c609aca246f78c7aa`. `control_if_ipv6_all()` previously discarded all `set_interface_conf_int()` and neighbour-cache `fput_int()` results; callers could not report partial configuration failure.
- Added injectable `rc_ipv6_conf_apply()`, attempting every write while retaining the first error and failing closed on invalid inputs. The production writer skips only per-device paths whose interface is not yet registered; kernel `all` and `default` paths remain mandatory.
- Changed `control_if_ipv6_all()` to return int, bounded path construction with `snprintf()`, checked neighbour-cache writes, logged init/runtime failures, and merged disable/enable errors into IPv6 restart status.
- Added GNU89 ASan/UBSan tests for ordered all-write execution, first-error retention, success, no-op and invalid inputs. Wired into Process, Firewall, WAN route and XY-C1 CI and shared source assertions.
- Parent exact Actions: WAN DHCP callback `43984154569`, AI handoff `43984154581`, Process inventory `43984154609`, AdGuardHome `43984154572`, AliDDNS `43984154561`, infosvr `43984154692`, AliyunDrive `43984154565`, NVRAM audit `43984154656` PASS. This production slice requires fresh exact-SHA checks; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Keep per-interface IPv6 cleanup arguments out of the shell

- Audit confirmed `clear_if_addr6()`, `clear_if_route6()` and `clear_if_neigh6()` interpolated runtime interface names into `doSystem()`; `doSystem()` formats a command and passes it to `system()`, so those values crossed a shell boundary.
- Replaced all three operations with argv-based `eval()`. Existing empty-name rejection, absent-interface no-op behaviour and command-result propagation are unchanged.
- Extended the WAN route workflow's source regression to require the exact argv vectors and forbid shell-based execution in all three functions.
- The new source contract passes locally against the edited source. Exact-head push Actions are not exposed by the available run-list integration (the commit-status response is empty and its run-list wrapper returns PR-triggered runs only); do not treat that as CI PASS. MIPS build/TRX and physical-router L4 remain unverified.

---

## 2026-11-16 — Correct WAN route and DHCP IPv6 flush source contracts

- Exact parent `f1f624e3f667e67376031233f6e2f9a53f6f31df` produced 14 push Actions. Eleven completed successfully; XY-C1 run `43987743135` remained in progress at last poll.
- WAN route run `43987743317` failed only because its inline checker still required `clear_if_neigh6()` to call `doSystem()`. WAN DHCP callback run `43987743207` failed only because its inline checker required both per-interface address/route flushes to call `doSystem()`. All earlier compiled regression steps in both jobs passed; no production failure was reported.
- Updated both inline contracts to require the exact argv-based `eval()` calls and reject shell-based execution. Focused source/checker assertions pass locally. The corrected workflows will rerun on this atomic commit; exact-head success is not yet claimed.

---

## 2026-11-16 — Remove shell-formatted dynamic commands from WAN IPv6 setup

- Audited all remaining `doSystem()` calls in `net_wan6.c`. Runtime interface names, validated IPv4/IPv6 literals, generated 6rd prefixes and clamped MTU/TTL values were interpolated into shell commands for SIT tunnel lifecycle, 6rd configuration, IPv6 route addition and native/SIT teardown.
- Converted every remaining command in the file to argv-based `eval()`; WAN SIT/native fail-fast macros still wrap each forward operation and return/cleanup paths still propagate status. Numeric MTU/TTL operands are rendered into fixed-size checked decimal buffers before tunnel mutation.
- Extended the WAN DHCP callback workflow to assert argv forms, macro gating, bounded numeric conversion and zero remaining `doSystem()` use in the file. Focused local source contracts pass; exact-head GitHub Actions and XY-C1 L3 must still be checked.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Repair WAN IPv6 argv regression assertions

- Exact-head WAN route run `43989785616` passed on `8b0ad1bb5760c282b1b85482be6b14ba0974720f`. WAN DHCP run `43989785873` reached its final source-contract step after all six compiled regression steps passed, then failed because several adjacent Python string literals in operation/marker tuples had no commas and concatenated into one impossible assertion; one route-order assertion also still searched for the former shell command.
- Restore each tuple item as a separate comma-delimited marker, assert the exact SIT/native argv vectors including both SIT default-route operations, and update the route-order check to locate the argv route command.
- Add the same no-`doSystem()` and status-gated argv checks to `scripts/check_modem_wan_start_status.py`, already consumed by Process, Firewall and XY-C1 workflows. Exact-head reruns are pending this correction.
- XY-C1 Build/TRX/artifact on source SHA `8b0ad1bb5760c282b1b85482be6b14ba0974720f` remained in progress at last poll; current-source L3 and physical-router L4 are NOT VERIFIED.

---

## 2026-11-16 — Keep PPP and custom WAN hook arguments out of the shell

- Audited the remaining dynamic command strings in `net_wan.c`. `wait_ppp_up()` interpolated its kernel-retrieved IPv4 peer into `ping`; `wan_up_internal()` and `wan_down_internal()` interpolated the WAN interface name and address into the fixed custom post-WAN script path.
- Replaced all three with argv-based `eval()`. The PPP probe retains its existing IPv4 validation gate; custom hook script path and the four expected arguments are unchanged as distinct argv elements.
- Extended the shared `check_modem_wan_start_status.py` regression to require the exact argv calls and reject the shell-formatted alternatives. Focused local source contracts pass.
- Parent `8e09623b275fec23b35952a992e2abcbb98f39fb` exact Actions: WAN DHCP `43990568766` PASS, WAN route `43989785616` parent PASS, and nine other completed workflows SUCCESS. XY-C1 `43990568825` and NVRAM `43990568681` remained IN_PROGRESS at last poll. Product-source L3 is not yet verified.

---

## 2026-11-17 — Remove unnecessary shell execution from PPP control commands

- `net_ppp.c` had two remaining process-shell calls: the static `l2tp-control start-session` request and `mwan.sh --rc-lifecycle-locked` launch. Neither needed shell parsing.
- Replaced them with direct argv-based `eval()` calls while preserving the exact L2TP argument, mwan lifecycle flag, existing nonzero checks and return-status propagation.
- Added shared contracts for exact argv vectors and for the absence of `system()`/`doSystem()` in `net_ppp.c`. The first exact-head run exposed a double-escaped regex in this assertion; this follow-up corrects it.
- The same exact-head run exposed stale VPN/HTTPD source contracts, now updated to require the argv vectors. No production C rollback is needed.

---

## 2026-11-17 — Restore development-log history and align PPP argv contracts

- Exact-head run set for `243c2514e56474ed779a4057a1277e2ae178091c`: WAN DHCP `43991768681`, WAN route `43991768587`, USB modem identity `43991768563`, infosvr `43991768585`, AdGuardHome `43991768660`, AliDDNS `43991768674`, and standalone process inventory `43991768704` PASS.
- Firewall `43991768509` and Process `43991768591` exposed a malformed double-escaped regex in the new PPP shell-API assertion (`re.PatternError`); VPN server `43991768846` and HTTPD `43991768656` exposed stale source contracts expecting the former `doSystem()`/`system()` calls after their argv conversions.
- AI handoff `43991768626` failed because `dev_log.md` still contained only two entries. This correction restores the full 314-line historical log from parent `5327f7626fc307c8db5e2de670e5433cd0eef55b` and prepends complete records for the checker repair and PPP control conversion.
- XY-C1 `43991768599`, NVRAM `43991768624`, and AliyunDrive `43991768665` were still in progress at last poll; current-source MIPS/uClibc L3 and physical-router L4 are NOT VERIFIED.

---

## 2026-11-16 — Normalize xl2tpd launch failures before lifecycle callers

- Parent `f87343b7e7a2da599c16baf1192ec1561dc44a9b`: HTTPD `43992613516`, AliyunDrive `43992613319`, AliDDNS `43992613480`, Process lifecycle `43992613370`, VPN lifecycle `43992613425`, AI handoff `43992613327`, USB WDM/NDIS `43992613561`, Firewall `43992613416`, AdGuardHome `43992613329`, infosvr `43992613374` and NVRAM `43992613294` all PASS. XY-C1 `43992613368` had passed host regressions through step 22 and was in Build; TRX/artifact pending.
- Defect: `safe_start_xl2tpd_locked()` uses `1` to report an already-running daemon, but directly returned `eval("/usr/sbin/xl2tpd", ...)` when the process was absent. `eval()` returns the child's positive exit code; an `xl2tpd` exit status of 1 therefore looked exactly like the success sentinel. The WAN-start merge path checks only `< 0`, so it could report WAN/L2TP server start success even though launching xl2tpd failed.
- Added pure policy mapping: a confirmed running daemon with no launch attempt maps to 1; a confirmed absent daemon plus command status 0 maps to 0; any nonzero command result or unknown/invalid presence maps to -1. The starter preserves errno (fallback EIO) and logs the nonzero child status. WAN PPP launch maps the documented already-running result 1 to its normal success status 0.
- Extended `tests/xl2tpd_start_policy.c` with exit statuses 1 and 127, unknown/invalid state and WAN-launch normalization cases. The VPN workflow already compiles this test with GNU89/Werror. Extended its source checker to require the normalized policy callsites and regression markers.
- Exact production C / host CI for this slice is pending on the new SHA; parent XY-C1 had not yet completed Build/TRX/artifact. Product-source L3 and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-16 — Accept an already-running xl2tpd during VPN server restore

- Base main SHA: `b9850f9917a6eb77b7c6d021abe75c88b4ca444c`; result: THIS COMMIT.
- Defect: `safe_start_xl2tpd_locked()` returns `1` when a complete process inventory confirms the shared daemon is already running, `0` when it starts the daemon, and `-1` on failure. `restart_vpn_server()` still treated every nonzero restore result as failure. When server restart had already relaunched shared xl2tpd, the subsequent WAN/VPN-client restoration probe returned the valid `1` sentinel and falsely marked the completed restart as failed.
- Changed the server-restart restoration gate to reject only negative results. Added a lifecycle source contract that pins this callsite to the documented tri-state success semantics; the existing xl2tpd policy regression exercises the `1` already-running result.
- Exact-base Actions: VPN lifecycle `43993836496`, AI handoff `43993836752`, Process inventory `43993836708`, process lifecycle `43993836554`, HTTPD `43993836692`, WAN route `43993836463`, WAN DHCP `43993836575`, infosvr `43993836668`, AliDDNS `43993836617`, AliyunDrive `43993836669`, and AdGuardHome `43993836497` PASS. XY-C1 `43993836487` and NVRAM `43993837253` were still running at Build/audit respectively.
- This focused source/checker repair has not yet run on its own SHA. Target firmware build/TRX/artifact and physical-router L4 for the post-normalization source remain NOT VERIFIED.

---

## 2026-11-17 — Flush stale IPv6 neighbour entries during WAN cleanup

- Predecessor `f87343b7e7a2da599c16baf1192ec1561dc44a9b` subsequently completed XY-C1 Build, TRX verification and artifact upload successfully on the same SHA (run `43992613368`, job `133736749350`); product-source MIPS/uClibc L3 is VERIFIED for that baseline.
- Parent `79ba2d6f818d0cb541032b3440834b883b1e531d` extends the xl2tpd lifecycle repair by accepting the already-running daemon sentinel during VPN server restore; exact-head Actions were pending at last poll.
- Review of `wan6_down()` found that interface-scoped IPv6 route/address flushes left the IPv6 neighbour cache untouched on a still-live native WAN interface. Native startup rollback had the same omission when undoing route/address side effects.
- Added `clear_if_neigh6()` to native rollback when route or address side effects occurred, and to normal WAN teardown between route and address flushes. Teardown accumulates neighbour-flush status, logs failures and retains saved IPv6 WAN state when cleanup is incomplete.
- Extended shared source contracts plus WAN DHCP callback and WAN route ordering tests to assert both paths and preserve route → neighbour → address order. Exact-head validation and a same-SHA XY-C1 build are pending this commit.

---

## 2026-11-17 — Align handoff and process integration contracts with VPN restore semantics

- Exact parent `79ba2d6f818d0cb541032b3440834b883b1e531d`: VPN lifecycle, Firewall, HTTPD, infosvr, AdGuardHome, AliyunDrive, main Process inventory, AliDDNS, OpenVPN credential and NVRAM passed. AI handoff `43994270363` failed because `SESSION_HANDOFF.md` omitted the validator's mandatory accuracy-checkpoint wording. Process fail-closed `43994270241` failed because `scripts/check_process_lifecycle_integration.py` still asserted `restore_status != 0` after `restart_vpn_server()` correctly changed to `restore_status < 0` to accept an already-running xl2tpd daemon.
- Restored the explicit mandatory checkpoint wording and updated the process integration contract to require negative-only restore failure and reject the stale nonzero assertion. No production source change was needed for these two CI failures.
- The IPv6 neighbour-cache cleanup slice was prepared but not yet committed while these exact-parent regression failures were resolved. Current f943 Actions were still running at last poll.

---

## 2026-11-16 — Complete the mandatory handoff checkpoint contract

- Base main SHA: `830d56c243ea0c220ede0b32762b723fccf83a69`; result: THIS COMMIT.
- The exact-head AI handoff workflow `43994914093` failed after the prior phrase repair because `SESSION_HANDOFF.md` still omitted the required `Before context exhaustion` marker. Restored that exact cross-session requirement while retaining the validator-required mandatory-checkpoint and conditional no-churn wording.
- The stale VPN restore assertion in `scripts/check_process_lifecycle_integration.py` was corrected in the parent commit; exact-head Process inventory/lifecycle `43994914203` now PASS. AliyunDrive `43994914128` also PASS on the parent after the one readiness failure at f943 `43994632492`; no Aliyun source change was needed. NVRAM `43994914119` PASS.
- Other exact-head PASS on parent `830d56c`: AdGuardHome `43994914137`, infosvr `43994914142`, AliDDNS `43994914157`. AI handoff remains the only failed workflow on that docs-only SHA; its sole logged failure is the missing marker. XY-C1 remains in progress on source commit `f943ccbd8efe3e125afcec8165bf9e235b390e06` at Build; current-source same-SHA TRX/artifact completion is still pending. Product-source L3 and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-17 — Propagate NVRAM ioctl file-descriptor close failures

- Base main SHA: `a7e3eda16eca5d20701a0240287ebecffed443d7`; result: THIS COMMIT.
- Audit finding: `nvram_set_unlocked()`, `nvram_commit_unlocked()` and `nvram_clear_unlocked()` returned the ioctl result while ignoring `close(/dev/nvram)`. A successful ioctl followed by a close error was therefore exposed to callers as success. If ioctl and close both failed, the close could also overwrite the primary ioctl errno.
- Added a pure result policy: ioctl failure has precedence; close failure converts an otherwise successful operation to failure; absent errno values fail closed with EIO. Applied it to SET/COMMIT/CLEAR through one fd-finishing helper. Added host regression cases and made the NVRAM workflow compile/run them; the source audit now pins all three callsites.
- Exact-head a7e3 AI handoff `43995311437`, Process fail-closed `43995311251`, infosvr `43995311336`, AdGuardHome `43995311368`, AliDDNS `43995311339`, and AliyunDrive `43995311301` PASS. NVRAM `43995311260` was still scanning kernel callers. XY-C1 run `43994632505` remained in Build on source SHA f943, not a same-head validation for docs-only a7e3.
- This source repair is not locally cross-built because no complete firmware checkout/toolchain is mounted. New exact-head NVRAM and XY-C1 Build/TRX/artifact checks must pass before updating the L3 verification status; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Restore mandatory end-of-session handoff marker

- Exact-head AI handoff run `43996118214` on source SHA `c221420d22838ec1632c975095c3bafa3830d6c9` failed because `SESSION_HANDOFF.md` lacked the exact required phrase `Before context exhaustion`; the rest of the validator had advanced to that rule.
- Restored the explicit checkpoint sentence and reconciled the handoff/roadmap with the live c221420d source and its current Action states. This is a documentation-contract correction only; it does not change the NVRAM source verification boundary.
- Current c221420d NVRAM, HTTPD, AliDDNS, AliyunDrive and Process Actions were pending at the time of this update. Same-SHA NVRAM and XY-C1 Build/TRX/artifact remain to be confirmed.

---

## 2026-11-17 — Restore shared xl2tpd after incomplete VPN teardown

- Base main SHA: `10d50854e2ffc456d689e93c196f8c271f99cf1c`; NVRAM close-result production slice at `c221420d22838ec1632c975095c3bafa3830d6c9` has its exact-head NVRAM audit `43996118191` PASS; the IPv6 neighbour-cleanup baseline f943 also completed XY-C1 Build/TRX/artifact `43994632505` PASS.
- Defect: `stop_vpn_server_locked()` could successfully stop shared `xl2tpd`, then abort because a later OpenVPN stop, up/down script unlink, or VPN lease cleanup failed. `restart_vpn_server()` returned immediately on this incomplete stop and skipped the shared WAN L2TP/VPNC restoration that was only present on its normal tail. Additionally, the old restoration flag was set only after the entire service-stop transaction succeeded, so a partial stop failure could also lose evidence that `xl2tpd` might already have been terminated.
- Record the shared-daemon stop attempt before entering the multi-target stop transaction. Factor the restore condition/start into one locked helper, normalize the documented positive already-running sentinel to success, and call it both on early stop-abort and on normal restart completion. The original teardown error remains the first returned failure even if restoration and lock release also fail.
- Extend the VPN restart policy test with the shared restore decision truth table and the Process/XY-C1 integration source checker with assertions for pre-stop tracking, restore-before-unlock on abort, and normal-tail restoration. Exact-head host checks and current-source XY-C1 build are pending this commit.
- The first exact-head VPN Actions run `43996866623` compiled and passed the extended GNU89 policy binary, then the independent `check_vpn_server_lifecycle.py` source audit failed because its old ordering assertion expected the pre-fix early return before restore handling. Updated that second source checker to require both locked restoration paths and the pre-stop attempt marker; no production source change was needed for this checker-only follow-up.
- The VPN checker rerun `43997214339` then exposed a second obsolete direct-call expectation (`restore_status = safe_start_xl2tpd_locked()` inside `restart_vpn_server()`), while the Process lifecycle checker `43997214335` still expected the removed direct stop-status return. Both now assert the shared restore helper and its early-abort/normal-path ordering; the policy binaries had already passed on the production SHA `acf6bc69`.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Restore storage apps after a failed safe-eject stop preflight

- Base main SHA: `19b24e9dd2aec153a60667925f129ad6e8218c0b`.
- Defect: `safe_remove_stor_device()` and `safe_remove_all_stor_devices()` stop storage-dependent services one by one. If a later stop fails, both correctly abort before unmounting, but earlier daemons remain stopped even though the storage is still mounted. This turns a safely aborted eject into a persistent service outage.
- Added `storage_rollback_stor_apps_after_stop_failure()`: while the storage-operation lock is held, it runs the existing tri-state/idempotent `start_stor_apps()` path, logs an incomplete rollback, preserves the original stop errno and returns the first stop failure. Both targeted and eject-all paths use it before releasing the lock or returning.
- Extended the storage stop policy regression to assert first-error preservation and the whole-storage source checker to pin the rollback helper and both abort paths. Storage-hotplug and XY-C1 workflows already include `services_stor.c`; exact-head CI and same-SHA Build/TRX/artifact validation are pending.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Update storage hotplug workflow source contract for rollback

- Exact-head storage run `43998879671` on SHA `10380657db522193a781d7e3068c0c2f02a8b78f` passed storage daemon policy compilation/regression and the standalone storage source checker, then failed at the embedded workflow source graph because it expected the old combined `stop_stor_apps()` conditions.
- Updated the embedded contract to require captured stop status, rollback helper execution, cleanup/unlock order, and original errno preservation for both targeted eject and eject-all. No production source change in this checker-only correction.
- Same-source XY-C1 Build/TRX/artifact run `43998879438` remained in progress at this point.

---

## 2026-11-17 — Correct storage and Samba integration contracts for rollback

- Storage-hotplug run `43999509853` on checker SHA `20a1266e5f82d19d3729be24daa32a9bece75022` passed its inline full-caller graph after the prior repair, then standalone `scripts/check_storage_safe_remove.py` failed because its compacting assertion removed the significant space inside the quoted operation label and checked against the function's first unrelated unlock instead of the unlock after rollback. Rewrote the check to normalize C whitespace without damaging string literals and to scope ordering to the rollback call.
- Samba share stability run `43998879597` on source `10380657db522193a781d7e3068c0c2f02a8b78f` failed because its embedded source contract retained the old exact safe-removal log sentence. Updated it to check the new failure wording and require the rollback helper in the storage path.
- XY-C1 exact-source run `43998879438` on production source `10380657` was still building when inspected; wait for Build/TRX/artifact conclusion only by polling state, and continue independent work while pending.
- These are test-contract corrections; no additional production firmware source changed.

---

## 2026-11-17 — Preserve UNKNOWN CPU-watchdog module probe state

- Base main SHA: `6697ca6020b261725655e12c3e5a5252ffb0e9c6`; result: THIS COMMIT.
- Confirmed a fail-open verification defect: `watchdog_cpu_restart_module_is_loaded()` called legacy Boolean `is_module_loaded()`, which maps negative `/sys/module` probe errors to false. The restart policy could then report successful disable when the module state was actually UNKNOWN; boot-time startup also collapsed the probe failure to generic EIO.
- Changed the policy callback contract to tri-state (1=loaded, 0=absent, negative errno=UNKNOWN). Restart now returns the positive probe errno, rejects invalid positive states with EIO, and retains the existing EIO/EBUSY postcondition decisions. Boot startup uses `is_module_loaded_status()` and preserves the probe errno.
- Added injected regression cases for UNKNOWN after both load and unload attempts and invalid probe results. Existing Process lifecycle and XY-C1 workflows already compile/run the policy with GNU89, `-Werror`, ASan and UBSan and include the changed source/test/checker paths; the source integration checker now pins strict probe wiring.
- Storage rollback exact-main storage-hotplug (`44000009287`), Samba (`44000009517`), Process lifecycle (`44000009230`) and AI handoff (`44000009196`) passed on parent `6697ca60`. The previous `10380657` product source passed XY-C1 Build/TRX/artifact (`43998879438`), but that result predates this `services.c` change. Current-slice L2/L3 Actions must be checked by exact main SHA.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align watchdog boot source contract with tri-state probing

- Exact-head Process lifecycle run `44000632768` on `aeddc15730e1a23419b1516ed88c330e148ac883` completed the watchdog policy regression and preceding integration checks, then failed because a legacy source assertion still searched for `if (!is_module_loaded("rt_timer_wdg"))`.
- Production boot startup intentionally uses `is_module_loaded_status()` so UNKNOWN probe errors are not converted into ABSENT. Replaced the stale Boolean assertion with checks for strict probe ordering, UNKNOWN propagation and known-absent handling, and asserted the tri-state helper contract. Checker/docs-only; no firmware C source changed.
- Exact-head AI handoff run `44000632603` PASS. XY-C1 run `44000632827` on production source `aeddc157` had passed the watchdog policy regression and was still building; final Build/TRX/artifact status must be confirmed.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Fail closed on UNKNOWN USB modem driver probes

- Base main SHA: `58fe2c5a4d5276a57c15afd66d3f1e84756d1a58`; result: THIS COMMIT.
- Found tri-state loss in `rc_modem_module_load_group_verified()`: any nonzero `is_loaded()` result incremented the available-driver count. Negative probe errno therefore counted as a confirmed loaded module and could return success when every `/sys/module` query was UNKNOWN.
- USB modem readiness now uses `is_module_loaded_status()`. The policy counts only exact value 1, preserves the first negative probe errno when no driver is confirmed available, rejects invalid positive states with EIO, and still succeeds when at least one alternative is confirmed loaded.
- Added GNU89 regression cases for all-UNKNOWN results, mixed confirmed-loaded/UNKNOWN alternatives, and invalid probe states. Updated `check_modem_module_reload.py` to enforce strict production callback wiring and the tri-state contract.
- Watchdog exact-head Process, AI handoff and NVRAM Actions passed on parent `58fe2c5a`: `44001230583`, `44001230567`, and `44001230573`. XY-C1 run `44000632827` on production source `aeddc157` had passed the watchdog policy regression but had not completed Build/TRX/artifact before this later modem source change.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Update XY-C1 modem readiness source contract for tri-state results

- Exact-head XY-C1 run `44002424380` on SHA `766832746b81a67faaa5d88e3ba78cbd1228faef` reached the USB modem driver configuration checker and failed because `scripts/check_usb_modem_driver_config.py` still required the old any-nonzero-is-available expression and legacy Boolean state callback.
- The dedicated USB modem GNU89 policy regression, strict source checker, Process lifecycle, inventory, HTTPD security, infosvr, and AI handoff Actions all passed on that exact SHA. Firmware Build had not started.
- Updated the XY-C1 source contract to require `is_module_loaded_status()`, count only exact probe state 1, and preserve UNKNOWN/invalid-state failures. Checker-only; no production firmware C changes.
- Watchdog production source `aeddc157` completed same-SHA XY-C1 Build, TRX verification and artifact upload in run `44000632827`.

---

## 2026-11-17 — Propagate IPv6 global sysctl initialization status into boot results

- Base main SHA: `f7b975c6bbe3f891498602c6363017cf6e6cc4c8`; result: THIS COMMIT.
- Finding: `control_if_ipv6_all()` already attempted all global IPv6 sysctl writes and retained the first error, but `init_ipv6()` exposed a void API. `init_router()` therefore could not include this early network-configuration failure in its existing first-error `boot_services_status` summary.
- Changed `init_ipv6()` to return its sysctl status and merged that result into `init_router()` before later network startup. Existing best-effort boot ordering and the precise local syslog message are preserved; the boot-level first-error record now includes this failure.
- Added a semantic integration contract for the production return path, header declaration, init_router aggregation and workflow path coverage. The existing GNU89 IPv6 global-configuration policy test verifies all writes are attempted and the first failure is retained.
- No local checkout/toolchain is mounted; local execution was NOT RUN. Parent `f7b975c6` had Process lifecycle run `44002780748` and XY-C1 run `44002780753` in progress; those do not validate this source change. Exact-head Process/XY-C1 Actions and same-SHA Build/TRX/artifact are pending.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Correct IPv6 boot status contract after exact-head failure

- Exact-head Process lifecycle run `44003333415` on `37e414f2d4f6a80c7addeaca04c14f4fdcdf5f5d` reached the new IPv6 source contract and failed because its raw regex used double-escaped whitespace tokens; the production source checks before this assertion passed. This is a checker defect, not a C compile or policy-test failure.
- Replaced the fragile pattern with a whitespace-normalized semantic assertion for both the `init_ipv6()` return capture and the exact first-error aggregation call.
- XY-C1 run `44003333586` on the same product-source SHA was still in progress at last check; its IPv6 policy regression had started, but Build/TRX/artifact is not claimed until the job concludes. The corrected source contract will rerun on this checker-only child SHA.
- No production firmware C changes in this correction. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Fail closed on UNKNOWN firewall module-table probes

- Base main SHA: `b447358e3c999c5f56ba915872d2243c91c7a962`; result: THIS COMMIT.
- IPv4 mangle/raw and IPv6 mangle builders used legacy Boolean `is_module_loaded()`; sysfs UNKNOWN collapsed to false/ABSENT and could skip empty-table restore while stale rules remained in a loaded table.
- Added `firewall_module_table_policy.h`: 1=loaded applies, 0=absent skips unless explicit IPv4 TTL-fix requires restore, negative errno propagates, malformed positive values and INT_MIN fail closed.
- Wired all three production gates to `is_module_loaded_status()`. Probe errors mark the active core restore transaction failed; existing transaction checks abort and roll back previously modified tables.
- Added GNU89/Werror/ASan/UBSan tests for loaded, absent, forced restore, UNKNOWN, malformed values and invalid output pointer. Firewall Startup Stability now compiles/runs the test and checks production wiring on push/PR.
- On parent `b447358e`, Process `44003684238`, AI handoff `44003684154`, NVRAM `44003684202`, infosvr `44003684216`, AliyunDrive `44003684191`, AliDDNS `44003684173`, and AdGuardHome `44003684129` passed. XY-C1 run `44003333586` on earlier production source `37e414f2` was still building; it cannot validate this later firewall source change.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align HTTPD firewall contract with tri-state module gating

- Exact-head HTTPD umbrella run `44005296061` on firewall source SHA `736ebe8638d253ae7b870d8a4554b15b7709f30b` passed the preceding auth/access checks, then failed only because its IPv6 mangle source assertion required legacy `is_module_loaded("ip6table_mangle")`.
- Firewall code now intentionally routes IPv6 mangle readiness through `firewall_module_table_restore_required()`, backed by `is_module_loaded_status()`; UNKNOWN marks the enclosing firewall transaction failed so changes are rolled back. Replaced the obsolete checker assertion with the tri-state helper/module-token contract and explicit no-legacy-Boolean assertion.
- Checker/docs-only; no production C changes. Exact-source Firewall Startup Stability `44005295947`, Process `44005296116`, inventory `44005296027`, FULLCONENAT `44005295950`, and AI handoff `44005296129` passed on `736ebe86`. XY-C1 `44005295961` was still building.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Register DNS Forwarder in checked RC service shutdown

- Base main SHA: `49f798836772a802efad324498b875fa6a05b94c`; result: THIS COMMIT.
- Finding: `start_services_once()` starts DNS Forwarder when `APP_DNSFORWARDER` is compiled, and `stop_dnsforwarder()` already delegates to the add-on's serialized, bounded, fail-closed stop action. However, `stop_services()` omitted that hook from its return-status accumulator.
- Consequence: normal reboot/halt could leave the daemon running after the RC service-stop pass; a firmware update could enter flash preparation while DNS Forwarder remained alive. The missed status also could not participate in the existing shutdown MTD-persistence gate.
- Added `stop_dnsforwarder()` to `stop_services()` under the matching `APP_DNSFORWARDER` guard using `record_service_stop_status()`. Stop failure now reaches `stop_services()`, blocks firmware update through its existing abort gate and blocks shutdown storage persistence through `service_stop_status`.
- Extended `scripts/check_optional_app_start_status.py` with a brace-aware C function extractor and an integration contract covering the matching start/stop guards, wrapper, shutdown storage-persistence gate and firmware-update abort gate. The Process workflow watches both changed files for push and pull_request; XY-C1 watches and compiles the changed RC source.
- Exact-head Process lifecycle run `44006695435` and DNS Forwarder launch-path regression `44006695180` PASS on this SHA. XY-C1 run `44006695535` remains in progress. AI handoff run `44006695428` failed only because this handoff rewrite omitted the required literal `Before context exhaustion`; this docs-only follow-up restores it. No local repository checkout or MIPS toolchain is mounted, so local host/firmware execution was NOT RUN.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Restore mandatory handoff checkpoint wording

- Base main SHA: `8b63b6ea70536e2677204e6da8741a662145ba5f`; result: THIS COMMIT.
- Exact-head AI handoff run `44006695428` failed in `scripts/check_ai_handoff_docs.py` because the current handoff no longer contained the mandated literal `Before context exhaustion`. Restored the required checkpoint sentence; no workflow or archive-index changes were needed.
- The DNS Forwarder source integration contract and Process lifecycle run `44006695435` PASS; DNS Forwarder launch-path regression `44006695180` PASS. XY-C1 `44006695535` remains in progress; no Build/TRX/artifact claim.
- Documentation-only correction; production source is unchanged. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Fail closed when configured RWFS cannot be mounted

- Base main SHA: `db73892db0692050eca7249e5ac55d4eb35bb3b9`; result: THIS COMMIT.
- Finding: `mount_rwfs_partition()` ignored the UBI module probe, `ubiattach`, mountpoint creation and UBIFS mount results, and returned void. A failed mount could leave `/media/mtd_rwfs` as an ordinary rootfs directory; `start_rwfs_optware()` used directory existence as readiness and could start services with non-persistent data.
- RWFS boot setup now uses the tri-state UBI module probe, verifies the exact MTD-to-UBI sysfs attachment before and after attach, rejects symlink/non-directory mountpoints, and returns mount errors. `init_router()` adds the mount result to its first-error service summary. Optware startup is gated on the successful mount postcondition; a newly-created empty mountpoint is removed after failed setup.
- Added a callback-injected GNU89 policy regression for disabled/unsupported configurations, UNKNOWN/missing module, unknown/absent UBI attachment, attach postcondition, mountpoint/mount errors and command/postcondition disagreement. Both Process lifecycle and XY-C1 workflows compile it under ASan/UBSan and run a source integration contract.
- DNS Forwarder stop registration source `8b63b6ea`: Process `44006695435` and launch-path `44006695180` PASS; current docs-only handoff follow-up `db73892d` repaired the required checkpoint wording. XY-C1 `44006695535` was pending last check; it does not validate RWFS.
- Prior firewall tri-state source `736ebe86`: Firewall Startup Stability `44005295947`, Process `44005296116`, inventory `44005296027`, FULLCONENAT `44005295950`, and AI handoff `44005296129` PASS. HTTPD checker follow-up `49f79883` passed `44005782392`.
- IPv6 global sysctl status source `37e414f2`: Process `44003684238` PASS on checker-only descendant, same-source XY-C1 `44003333586` PASS including Build, TRX and artifact.
- No local checkout/toolchain is mounted; local execution was NOT RUN. Exact-head Actions for THIS RWFS commit are pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Exercise Optware result decoder in RWFS host regression

- Base main SHA: `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`; result: THIS COMMIT.
- Exact-head Process run `44007851661` on `5f1d87f6` failed while compiling the new RWFS policy test under `-Werror`: including the shared Optware policy header left its existing non-inline `storage_optware_start_decode_result()` unused in that test translation unit. The policy and production integration checks had not run because compilation stopped first.
- Kept the shared header and production behavior unchanged. The RWFS test now includes explicit decoder success/failure/incomplete-pipe assertions, using the helper and removing the warning while adding coverage. The integration contract pins this regression.
- Exact-head Process, XY-C1 and storage hotplug reruns are pending on this checker/test-only child. The original XY-C1 run `44007851467` was still in progress; it does not validate the new test change. No Build/TRX/artifact claim for RWFS yet.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Correct RWFS source contracts after exact-head failures

- Base main SHA: `fd9ab12eb7102eda5ef081b52160fd569e30aa72`; result: THIS COMMIT.
- Process inventory run `44008320003` on `fd9ab12e` ran the RWFS GNU89 policy binary successfully, then failed in `check_rwfs_mount_integration.py`: production callbacks use the normal multiline `static int\nfunction_name(...)` style, while the checker searched for a single-line signature. The checker now resolves those functions by their unique names.
- XY-C1 run `44008319840` on the same production-source SHA failed before firmware compilation in `check_rc_boot_module_loads.py`: its old assertion required `boot_services_status = boot_modules_status;`. The RWFS source now correctly merges the earlier mount result with that module status using `rc_first_service_start_status()`; the source contract now verifies both terms and their order.
- The RWFS policy test passed under GNU89, ASan and UBSan in run `44008320003`; the broader RWFS source integration step was the only failed Process step. The USB storage hotplug run `44007851445` and HTTPD run `44007851498` passed on production SHA `5f1d87f6`.
- This correction changes checker expectations only; no production C changes. Exact-head Process/XY-C1 reruns and same-source Build/TRX/artifact remain pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Match RWFS mountpoint cleanup integration assertion to production guard

- Base main SHA: `f73b2d55d61a9f8e007555b1d2e8b2cb5ee4f595`; result: THIS COMMIT.
- Exact Process run `44008834957` and XY-C1 run `44008834951` both compiled and passed the GNU89 RWFS policy binary, then failed on one remaining assertion in `check_rwfs_mount_integration.py`. Production cleanup correctly invokes `rmdir(mp_rwfs)` inside a combined condition guarded by `rwfs_mountpoint_created` and excludes `ENOENT`; the checker incorrectly required a standalone `if (rwfs_mountpoint_created)`.
- Replaced the over-specific text check with assertions on the actual combined cleanup guard and its `ENOENT` handling. The preceding exact-head XY boot-module checker passed after the prior fix to assert RWFS-first-error aggregation.
- This is checker-only; no production code changed. Exact-head reruns are pending; XY-C1 has not yet reached Build/TRX/artifact for RWFS. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align Process umbrella boot-status contract with RWFS first-error merge

- Base main SHA: `45def115a2b7f52a7b495750ae815223f016ccff`; result: THIS COMMIT.
- Exact Process run `44009283857` on `45def115` passed the dedicated GNU89 RWFS policy regression and `check_rwfs_mount_integration.py`, then failed in the broader `check_process_lifecycle_integration.py` at two stale assertions requiring `boot_services_status = boot_modules_status;`. Production `init_router()` now correctly seeds the first-error boot summary with `rc_first_service_start_status(rwfs_mount_status, boot_modules_status)`; the checker assertions lagged the source.
- Updated both repeated contract assertions to require the RWFS mount status capture and its first-error merge with module-load status. No production code changes.
- On `45def115`, XY-C1 run `44009283846` had passed the RWFS policy/integration checks and was actively compiling Build XY-C1; TRX verification and artifact upload had not started yet. This checker-only correction needs fresh exact-head Process and XY-C1 workflows.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Stop Ralink IAPP during checked RC shutdown

- Base main SHA: `45def115a2b7f52a7b495750ae815223f016ccff`; result: THIS COMMIT.
- Finding: `start_services_once()` unconditionally executes `/usr/bin/iappd.sh restart` and then the script's post-start `test`; the package Makefile installs the script into ROMFS. The idempotent `stop` action already performs bounded, fail-closed daemon stop and postcondition checks, but `stop_services()` omitted it.
- Added `system("/usr/bin/iappd.sh stop")` to `stop_services()` through `record_service_stop_status()`. A nonzero wait status now enters the common first-error result and therefore reaches the existing reboot/halt persistence and firmware-update abort gates.
- Extended `check_optional_app_start_status.py` to pin the boot restart/test call, shutdown stop result, ROMFS installation and existing lifecycle regression wiring. The Process workflow already runs `sh -n` and `trunk/user/ralinkiappd/tests/iappd_lifecycle.sh`.
- The Ralink IAPP script regression was previously executed successfully in Process CI, but exact-head Process and XY-C1 verification for this new RC hook is pending. RWFS policy regression passes; its exact-head Process/XY-C1 source checker runs are still pending after the mountpoint-cleanup assertion correction in `45def115`.
- No local checkout/MIPS toolchain is mounted; local build/test execution was NOT RUN. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Remove remaining obsolete boot-module direct-assignment assertion

- Base main SHA: `f5a42763bfcc08e5ea9fad31e717c7dd608ced9a`; result: THIS COMMIT.
- Process run `44009629348` on `3233dc7b` proved the first of two duplicate stale expectations had been replaced, but failed at the remaining second assertion in `check_process_lifecycle_integration.py` (`boot_services_status = boot_modules_status;`). The production boot status deliberately includes the earlier RWFS mount result; no firmware source defect was involved.
- Replaced the remaining stale assertion with a normalized check for the second argument of `rc_first_service_start_status(rwfs_mount_status, boot_modules_status)`. The focused RWFS GNU89 policy and integration steps passed on the prior SHA; no C source changed.
- Same-source XY-C1 run `44009283846` on `45def115` had passed RWFS policy and source checks and was still in Build XY-C1; TRX/artifact had not started at last poll. It does not validate the follow-up checker SHA.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Require a confirmed DDNSTO process after asynchronous startup

- Base main SHA: `b95bb7350785a1a4c5db8881e2ecd764d2b918f3`; result: THIS COMMIT.
- Finding: `start_zero()` returned immediately after shell backgrounding. A missing ROMFS executable, failed/empty `ddnsto_id`, or an immediately exiting child could therefore be reported as a successful start to RC/autostart callers.
- Startup now validates the executable and NVRAM identity **before** stopping a healthy existing instance. After launch, it requires a validated RUNNING result from the existing tri-state `pidof ddnsto` probe within five bounded polls. Only confirmed ABSENT is retried; UNKNOWN and sleep errors abort, and a five-poll ABSENT timeout returns nonzero. This is a process-presence gate, not an upstream/network readiness claim.
- The production-linked shell regression now covers missing executable, NVRAM read/empty identity, no disruption of a running instance on preflight failure, immediate exit/timeout, UNKNOWN inventory, sleep failure, delayed success, and retains the existing concurrency/lock/restart tests. The semantic checker pins the bounded tri-state algorithm and prerequisite/launch ordering.
- Exact-head DDNSTO, Process and XY-C1 Actions are pending for THIS COMMIT. No local checkout/toolchain is mounted; local execution was NOT RUN. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Fix DDNSTO empty-identity regression fixture

- Base main SHA: `574caf1b5abe240ea7dd025120a74414c83cefc3`; result: THIS COMMIT.
- Exact-head DDNSTO run `44010845222` failed at the assertion `empty identity disrupted the existing daemon`; production `start_zero()` validates the ID before invoking `kill_z()`, so this failure was traced to the test's NVRAM mock.
- The mock used a two-backslash printf format (`%s\\n`) inside single quotes. With an empty test value, printf emitted the literal two-character sequence `\n`, so the production command substitution saw a nonempty ID and legitimately proceeded to stop the old daemon. Corrected the fixture to a real newline format (`%s\n`), which now supplies an actually empty value and tests the intended fail-before-stop invariant.
- No production DDNSTO behavior changed. Exact-head rerun pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Correct DDNSTO delayed-start regression PID fixture

- Base main SHA: `9c7c393d45617f2820b1285452226759a94b43c5`; result: THIS COMMIT.
- Exact-head DDNSTO run `44011291797` on `9c7c393d45617f2820b1285452226759a94b43c5` failed only in the delayed-start regression: the mock daemon wrote the literal character `$` into `TEST_DAEMON_PID` (`echo $`) instead of its numeric shell PID. The production `ddnsto_process_state()` correctly rejected that malformed pidof inventory as UNKNOWN, so the fixture could never satisfy the expected RUNNING postcondition.
- Corrected both delayed and normal mock daemon PID writes to `echo $$`, and added a source contract requiring the fixture to publish an actual numeric PID. No production DDNSTO code changed.
- Process lifecycle run `44011291959` on the base SHA was still in progress at last poll; AI handoff, AliyunDrive and AdGuardHome exact-head checks passed. Fresh exact-head DDNSTO/Process checks are required.
- RWFS exact-source XY-C1 run `44009838585` completed SUCCESS on production-source descendant `f5a42763`, including RWFS policy/integration, Build XY-C1, TRX verification and artifact upload. Process `44009960277` passed on descendant `b95bb735`; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Make DDNSTO PID inventory mock model empty PID files correctly

- Base main SHA: `419b46899ac5d85588753683cf39a3fe44d1eb5c`; result: THIS COMMIT.
- Exact-head DDNSTO run `44011740060` on `419b4689` failed in the test case that verifies UNKNOWN after TERM must not trigger SIGKILL. Production stop logic was fail-closed: the mock set state to RUNNING but the prior delayed-start cleanup had truncated the PID file; mock `pidof` used `cat PID || echo 12345`, so a successful empty `cat` produced an empty PID list and production correctly returned UNKNOWN before sending TERM. The test then misreported the missing TERM log as SIGKILL escalation.
- Changed the mock to use `-s` to distinguish a populated PID file and emit a valid fallback PID when empty. Retained the `echo $$` writes for actual daemon fixtures and tightened the source contract to require both behaviors. No production DDNSTO code changed.
- Process run `44011740122` on the base SHA and same-source XY-C1 history `44009838585` were checked separately; RWFS source `f5a42763` has same-source Build/TRX/artifact PASS. This DDNSTO test-only correction has fresh exact-head CI pending.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Stop the GPIO button daemon during shutdown

- Base main SHA: `6e997ddb7eeccdee56ecd6eaa2d1affb401d154e`; result: THIS COMMIT.
- Finding: `start_services_once()` registered `start_gpio_btn()`, and `gpio_btn.c` already installed a SIGTERM handler that clears its daemon loop flag. However, `stop_misc()` omitted `gpio_btn`, so the independent uevent listener could remain active while services and storage were being torn down.
- Added `gpio_btn` to the existing miscellaneous daemon stop list and incremented its explicit process count. This uses the same graceful `kill_services(..., 1)` path as `ntpd`, `detect_wan` and `watchdog`; the existing shutdown persistence gate still blocks flash writes when `stop_misc()` reports failure.
- Extended `check_gpio_btn_lifecycle.py` to pin the stop registration, daemon SIGTERM handler, and shutdown persistence gate ordering. Existing GPIO lifecycle workflow already includes `services.c` and this checker in push/PR filters and runs GNU89 ASan/UBSan tests.
- DDNSTO fixture correction `6e997ddb` (PID mock empty-file fallback) needs exact-head Actions; the preceding failure on `419b4689` was a test-only empty PID inventory mismatch. RWFS same-source XY-C1 L3 remains PASS on production source `f5a42763` (run `44009838585`: Build/TRX/artifact).
- No local checkout/toolchain is mounted; local execution was NOT RUN. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Repair DDNSTO source checker and add firmware trigger

- Base main SHA: `e48aca6bcd346f4813b6edb22974cb366423e340`; result: THIS COMMIT.
- Exact DDNSTO runs `44012778484` and `44013229590` passed shell syntax and every lifecycle shell case, including bounded start readiness and the delayed-start scenario. The semantic checker then failed on a stale ordering assertion: it compared `kill_z` with the first `$DDNSTO_BIN` reference, which is correctly the earlier executable preflight rather than the later launch command. A separate check expected obsolete `echo $` despite the fake daemon now writing `echo $$`.
- Both checks now pin the actual command invocation and the numeric PID fixture. The XY-C1 path filter now includes `trunk/user/ddnsto/ddnsto.sh`, so changes to this packaged launcher schedule firmware Build/TRX/artifact validation; its source checker pins this path.
- GPIO stop-registration change `e48aca6b` passed GPIO, Process inventory and Process lifecycle; its same-head XY-C1 run remained pending. HTTPD umbrella `44013229543` failed and awaits job-log diagnosis. No local checkout/toolchain; local execution was NOT RUN. Physical L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align DDNSTO source checker with the numeric mock PID

- Base main SHA: `e5d3cdfdca314e8d8b548c5b5c2a990a6f67d226`; result: THIS COMMIT.
- Exact DDNSTO run `44013905030` passed both shell syntax checks and all production-linked lifecycle tests, including the delayed startup readiness and the post-cleanup process-state regressions. The Python source checker then failed because its fake PID assertion still expected `echo $`; the current mock daemon intentionally publishes its real PID using `echo $$`.
- Updated the semantic checker to match the actual numeric PID fixture. No production source or shell test behavior changed.
- On `e5d3cdfd`, Process inventory, AI docs, infosvr, AliyunDrive, AdGuardHome and AliDDNS passed; same-SHA XY-C1 firmware and NVRAM remained in progress at last poll. Local execution was NOT RUN; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Correct final DDNSTO fake-PID checker expectation

- Base main SHA: `3c2ee531e5b6f5b96a004732d82fe45ec882a5a8`; result: THIS COMMIT.
- Exact DDNSTO run `44013905030` passed shell syntax and every production-linked lifecycle test. The remaining failure was solely a stale semantic checker assertion looking for `echo $` while the actual mock daemon writes its real PID as `echo $$`.
- Corrected the checker to require the numeric PID fixture. Production launcher and test cases are unchanged.
- Same-source XY-C1 `44013905031` was compiling at last poll; NVRAM audit `44013905187` PASS. Exact DDNSTO rerun pending. Local execution NOT RUN; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align HTTPD stop_misc contract with GPIO daemon registration

- Base main SHA: `4c499369ec8f9aedd2643e0f012b12752ab67b7d`; result: THIS COMMIT.
- Exact HTTPD umbrella run `44013229543` on `e48aca6b` failed at the inline source assertion requiring `return kill_services(svcs, 3, 1);`. The production `stop_misc()` currently stops `ntpd`, `detect_wan`, `watchdog`, and `gpio_btn`, then calls `kill_services(svcs, 4, 1);`. This is a stale workflow contract after the GPIO stop registration, not a production defect.
- Updated the workflow to assert both the `gpio_btn` entry and the four-service stop count. Its push path filter includes the workflow itself, so the change should schedule an exact-head HTTPD rerun.
- DDNSTO checker correction `4c499369` exact run `44015005394` was still running at last poll. XY-C1 `44013905031` was at artifact upload; no final result claimed. Local execution NOT RUN; physical L4 remains NOT VERIFIED.

---

## 2026-11-17 — Fail closed when configured static WAN MTU cannot be applied

- Base main SHA: `c5fc913650e3ae70b8056f63eb70c8db49737505`; result: THIS COMMIT.
- Defect: for static IPoE MTU values in the supported override range (1300–1499), `start_wan()` logged a failed `set_interface_mtu()` call but left `wan_err == 0`, continued assigning the static address, and could call `wan_up()` while returning success. This made the configured WAN MTU failure invisible to RC callers and falsely published an operational WAN.
- The failure now sets `wan_err`, preserves the first status in `wan_start_status`, and gates static address assignment/route publication. The interface remains at its pre-address `0.0.0.0` state and the per-WAN NVRAM error flag is set by the existing path.
- Added source-contract assertions for the failure flag/first-error merge/address gate and a GNU89 route-status regression proving later successful operations do not overwrite the MTU error. Exact WAN workflow pending. Local execution NOT RUN; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Propagate firmware-update aborts into PID 1 notification results

- Base main SHA: `9e3316f6740a77fd8db26d1f51af4183dd071021`; result: THIS COMMIT.
- Finding: `flash_firmware()` returned `void`. Its notification handler removed the request marker and called it without merging a status into `action_result`; preparation failures (VPN/service shutdown, auth/relay teardown, custom shutdown script, storage eject/persistence, printer/IGMP teardown) and a failed `/tmp/mtd_write` therefore appeared successful to the notification queue.
- Changed `flash_firmware()` to return `-1` on all ten abort paths and on a failed firmware-write command, return zero only after a successful write command, and preserve the original write errno across watchdog recovery. The notification event now merges this result using `rc_notify_merge_action_result()`; failures retain the incomplete marker.
- Added a Python source contract and wired it plus the existing GNU89 `rc_notify_batch_policy.c` regression (including EBUSY/EIO normalization) into Process and XY-C1 workflows. Also removed the stale duplicate module-dashboard/checkpoint block from `REFACTOR_ROADMAP.md`, preserving the canonical full table and merging the current GPIO/HTTPD/DDNSTO statuses. Local checkout/toolchain unavailable; local execution NOT RUN. Exact-head CI pending; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Wire notification regression into Process and align WAN-MTU checker

- Base main SHA: `1d695e75ce20ee1a5055eb355bc5373f7f8e8608`; result: THIS COMMIT.
- Exact-head CI for `1d695e75` found three confirmed checker/integration issues. XY-C1 `44016786510` failed because `check_firmware_update_notification.py` requires both Process and XY-C1 to run the GNU89 notification policy test/source contract, but the Process workflow had not been modified. The XY-C1 workflow had compiled and passed the policy binary before the checker failed.
- Process inventory `44016786493` failed at the existing static-WAN MTU assertion in `check_modem_wan_start_status.py`: production now logs that failed MTU application skips address/route setup, while the checker expected the older generic “setup incomplete” message. Updated the assertion to match the production fail-closed log; existing ordering checks still ensure MTU handling precedes address publication.
- AI handoff `44016786535` failed because `SESSION_HANDOFF.md` lacked the literal required phrase `Before context exhaustion`; restored the mandatory checkpoint sentence. Standalone Process lifecycle `44016786612` and PID 1 notification queue regression `44016786729` passed on the base SHA.
- Process workflow now compiles/runs `rc_notify_batch_policy.c` with GNU89/Werror/ASan/UBSan and executes `check_firmware_update_notification.py`, matching XY-C1. Re-run exact-head Process, AI handoff and XY-C1; previous XY run failed before same-source Build/TRX/artifact could be claimed. Local checkout unavailable; local execution NOT RUN. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Update RC integration checker for status-returning firmware write

- Base main SHA: `97bd094ca5d05472861bbc7e79450aa1ce77c855`; result: THIS COMMIT.
- Process inventory run `44017512634` on `97bd094c` passed the added GNU89 notification policy binary and dedicated `check_firmware_update_notification.py` contract, then failed later in `check_process_lifecycle_integration.py:589`.
- The stale integration assertion required the old inline `if (eval("/tmp/mtd_write"...))` expression. Production now captures `flash_status`, enters an explicit nonzero branch, saves `flash_errno`, attempts watchdog recovery, restores the original errno and returns `-1`; only successful write returns zero. Updated the checker to enforce those failure semantics rather than the obsolete call shape.
- AI handoff `44017512559` on `97bd094c` passed. XY-C1 `44017512514` was still in progress at last poll; no same-SHA Build/TRX/artifact claim. Local execution NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-16 — Propagate WAN firewall rebuild failures through startup

- Base main SHA: `a84174f9f5719272c1cc111c07be083790ae5734`; source change: THIS COMMIT.
- Defect: `start_wan()` discarded the integer result from `start_firewall_ex()` in the USB modem RAS first-stage rebuild, normal PPP first-stage rebuild and final static-IPoE rebuild. `restore_wan_after_ppp_start_failure()` was also `void`, dropping both the UPnP interface-migration error and the firewall restore result on three PPP failure/recovery branches.
- All four firewall calls now capture status and merge it into the first-error WAN/modem startup result. PPP recovery returns UPnP/firewall failures and each caller merges that status into `wan_start_status`; subsequent safe setup remains best-effort where existing logic permits.
- Added `scripts/check_wan_firewall_status.py` to assert every caller/status path and wired it into WAN-route and XY-C1 workflows and their path filters. Exact-head CI is pending after this commit. Local checkout/toolchain unavailable; local execution NOT RUN. Physical-router L4 NOT VERIFIED.

---

## 2026-11-16 — Correct WAN firewall caller-contract checks

- Parent main SHA: `9a0256f5d4cd97a902e48e32839fde7e1391d370`; the implementation commit is `9a0256f5d4cd97a902e48e32839fde7e1391d370`.
- Exact-head CI on 9a exposed only source-contract assertions: WAN-route `44019982102` rejected the new checker’s over-escaped regex; Process `44019982105`, XY-C1 `44019982156` and Firewall Startup `44019982218` still expected three modem startup status merges, not the new four. AI handoff `44019982167` PASS; XY-C1 stopped before compilation.
- Replaced regex matching with deterministic source-marker positions and updated the shared modem/WAN checker to require the fourth RAS firewall status merge. The existing checks for WAN migration, RAS launch and first-error behavior remain in place. Exact-head reruns are pending; this correction changes tests only, not production C code.

---

## 2026-11-16 — Preserve Inadyn failure status during WAN-up

- Parent main SHA: `9116f521c171d011f45a9a19abebcf57cdbedcc9`; source and regression changes are THIS COMMIT.
- Defect: `wan_up_internal()` called `notify_ddns_update()` but discarded its result. When configured DDNS could not be started, its process inventory was UNKNOWN, its configuration could not be published, or its HUP failed, WAN-up still returned success.
- A direct aggregation alone would misreport normal routers: the notifier also ran when DDNS was disabled or in AP mode, while `start_ddns()` correctly rejects those configurations. Added a shared eligibility policy so those cases are successful no-ops before any process scan or daemon start.
- WAN-up now merges the notifier result into its existing first-error status and logs failures. Added the four-case enable/AP policy regression, a source-level caller contract, and exact workflow gates.
- Exact-head Actions for THIS COMMIT were pending at commit creation. Local compile/test was not run because no checkout/toolchain is available; physical-router L4 remains unverified.

---

## 2026-11-16 — Correct exact-head WAN/DDNS integration gates

- Parent main SHA: `01bc69f31c7a40591587e095f8be8d634b43ef23`; this commit repairs test contracts exposed by `01bc69f3`.
- WAN-route regression compile now includes `trunk/user/shared/src`, where `process_inventory.h` resides.
- Restored mandatory handoff checkpoint and `## Verification boundary`; these satisfy the live AI-handoff and infosvr source-contract consumers without weakening those validations.
- MiniUPnPd and HTTPD PPP rollback assertions are updated to enforce the current checked `upnp_status` early return and captured `firewall_status`, rather than obsolete unchecked calls and void returns.
- The WAN DDNS semantic checker now handles escaped C strings and line comments correctly.
- Exact-head `01bc69f3` failures were diagnosed from job logs. Corrective descendant CI has not yet run. Local toolchain remains unavailable; physical router L4 is not verified.

---

## 2026-11-16 — Complete mandatory handoff checkpoint wording

- Parent main SHA: `0c786821fd1bb8b571c51c1c7abf32780f481157`.
- Exact-head AI handoff workflow `44026236466` on `0c786821` identified one final required literal, `Before context exhaustion`. Added it to the existing mandatory checkpoint without changing any source behavior or weakening the checker.
- On `0c786821`, WAN route, MiniUPnPd, HTTPD security, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS all passed. XY-C1's compile gates passed and Build XY-C1 was in progress; TRX/artifact steps were pending.
- This documentation correction awaits its own exact-head AI handoff rerun. Local toolchain remains unavailable; L4 unverified.

---

## 2026-11-16 — Propagate configured ZeroTier startup failures through WAN-up

- Parent main SHA: `06ed43217d0e47b54f17fc0416a81e7341593824`.
- Defect: `wan_up_internal()` launched `zerotier.sh start` through `doSystem()` and dropped the status even though `start_zerotier()` already applies the enabled/disabled gate and returns the RC launch result.
- Replaced the direct shell launch with the checked RC wrapper; merge the startup result into WAN-up's first-error status and log the failure. Guard the call with `APP_ZEROTIER` to match the wrapper's compile-time declaration.
- Extended the ZeroTier shell lifecycle test with an injected `start_instance` failure and added a source-level caller contract. Both WAN-route and ZeroTier workflows now trigger on the changed WAN caller and execute the contract checker.
- Exact-head CI for this source-bearing commit is pending at commit creation. Local shell/C test execution is unavailable; physical-router L4 remains unverified.
- Next P1 caller: PPPoE-MWAN `detect.sh` currently masks many command failures and exits after lock cleanup; harden its own failure/cleanup contract before passing its status into WAN-up.

---

## 2026-11-16 — Repair WAN workflow pull_request filter placement

- Parent main SHA: `d2a41360466574898f7fc9fe224236d5a51517a7`.
- Exact-head WAN route workflow run `44027994137` for `d2a41360` failed before any job was created (`jobs.total_count=0`). The four newly added pull_request paths were appended after the workflow body because the file has no top-level permissions block.
- Moved those paths into `on.pull_request.paths`, validated the push and pull_request filter regions, and kept the caller semantic check step in the job. No production source behavior changed.
- On `d2a41360`, ZeroTier lifecycle and Process checks passed; XY-C1 Build was still in progress at last poll. Repair commit exact-head CI awaits execution.

---

## 2026-11-16 — Harden PPPoE-MWAN detect.sh failure and cleanup semantics

- Parent main SHA: `42097d6c2ac28c37960bbb5b4940f2db22bf30f2`.
- Reworked `trunk/user/pppoe/detect.sh` so failed `iptables-save`, rule deletion/insertion, `ip rule`, per-table routes, default-route replacement and route-cache cleanup are no longer silently discarded. The first nonzero operation status is preserved while independent cleanup operations still run.
- Removed shell `eval` of `iptables-save` output. A bounded Bash tokenizer converts selected `-A` rules into argv while preserving quoted comments and treating command substitutions/backticks as inert data.
- NVRAM counters/selectors and IPv4 inputs are validated before arithmetic or rule construction. Missing `pppN` interfaces remain expected during WAN reconvergence. Lock creation is exclusive; only the invocation that acquired the lock removes it, and EXIT cleanup failures become visible without masking an earlier operation failure.
- Added `trunk/user/pppoe/tests/detect_status.sh`; local `bash -n` checks and the mock-command suite PASS. Coverage includes normal multipath, save/delete/insert/route failures, continued cache cleanup, invalid bridge/NVRAM/IPv4/counters, absent PPP device, leading-zero decimal input, lock cleanup and shell-metacharacter injection.
- WAN-route push/PR filters now include the script and test, and the job runs the Bash syntax/mock suite. XY-C1 push trigger includes the production script. Exact-head Actions and XY-C1 Build/TRX/artifact verification are pending for this source commit; no firmware/L4 result is claimed.

---

## 2026-11-16 — Propagate PPPoE-MWAN detector status through WAN-up and WAN-down

- Parent main SHA: `4f32e40fdd1817a48840abf42373a483dff20d91`.
- Defect: both `wan_up_internal()` and `wan_down_internal()` gated `/usr/bin/detect.sh` on `pppoemwan_enable` but invoked it via `doSystem()` and discarded the result. Failed multi-WAN policy/firewall rebuilds could therefore leave either WAN transition reporting success.
- Replaced both unchecked calls with `eval("/usr/bin/detect.sh")`, merged each result into the existing first-error `status` through `route_status_accumulate()`, and logged failures while allowing the remainder of the lifecycle to finish.
- Added `scripts/check_pppoe_mwan_status.py`; it extracts the C function bodies, verifies both enabled gates, checked launch, ordered aggregation/diagnostics, absence of the old ignored call and returned status from both public wrappers. WAN CI runs it alongside the script-level Bash mock suite.
- Detector-source exact-head WAN run `44037585283` passed, including syntax and command-failure mocks. Its same-SHA XY-C1 Build `44037585221` was still in progress (TRX and artifact pending) at the prior poll. This up/down caller commit's exact-head Actions and XY-C1 remain to be verified; no target/L4 result claimed.

---

## 2026-11-17 — Propagate gateway ARP checker startup failures through WAN-up

- Parent main SHA: `5525430a2ee5ff16f3d5b84912cd183e195808df`; result: THIS COMMIT.
- Finding: `wan_up_internal()` only starts `detect_wan` after process inventory reports ABSENT, but discarded the `eval("detect_wan")` result. A failed gateway ARP checker launch could therefore leave WAN-up reporting success with monitoring unavailable.
- Captured the launch status, merged it into the existing first-error accumulator and logged failures while preserving the existing RUNNING/UNKNOWN gates and continuing independent WAN setup.
- Added `scripts/check_wan_detect_wan_status.py` to verify process-scan/decision/launch/aggregation/diagnostic/return ordering. The WAN workflow now runs it; the existing GNU89/ASan/UBSan `route_status_policy.c` regression continues to exercise first-error retention.
- Parent SHA `5525430a` had all 22 exact-head push Actions PASS, including WAN route `44039076856` and XY-C1 `44039076948` (Build, TRX verification and artifact upload). Those results predate this source change. New exact-head CI is pending; local execution NOT RUN (no local checkout/toolchain), physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Propagate UNKNOWN gateway-monitor inventory failures through WAN-up

- Parent main SHA: `ac9b8936d7359d4016db8475c4ae459d17f73a2f`; result: THIS COMMIT.
- Finding: after the gateway ARP monitor was changed to capture launch failures, an UNKNOWN process-inventory scan still only logged and skipped startup. The configured monitor could remain unavailable while WAN-up returned success.
- Added `process_presence_start_status_for_action()`: a clean ABSENT scan returns the launch result; RUNNING remains a successful no-op; UNKNOWN returns the procfs scan error or EIO. Invalid actions fail closed with EIO.
- WAN-up now merges both launch and UNKNOWN-inventory failures into its existing first-error status, logs the effective error, and continues independent setup without risking a duplicate monitor. Extended GNU89 policy coverage for launch error, RUNNING, UNKNOWN, missing errno and invalid action; extended the WAN semantic checker.
- Parent `ac9b8936` exact-head push Actions were last observed at 13 SUCCESS / 9 IN_PROGRESS, including WAN route `44041394533` SUCCESS, Process lifecycle `44041394265` IN_PROGRESS and XY-C1 `44041394404` IN_PROGRESS. Those runs validate only the parent SHA. New commit Actions and XY-C1 are pending; local execution NOT RUN (no checkout/toolchain in this runtime); router L4 NOT VERIFIED.

---

## 2026-11-17 — Repair gateway-monitor semantic checker ordering

- Parent main SHA: `9722f273f005eaf323da7d80a7d0cfbc398faa73`; result: THIS COMMIT.
- The exact-head WAN route run `44042062613` failed in `check_wan_detect_wan_status.py`, not in production code or compilation. The checker searched for the status merge before the enclosing nonzero guard; production correctly performs the merge inside that guard, so the search skipped the matching UNKNOWN branch.
- Corrected the semantic assertion order to require: status mapping → nonzero guard → first-error merge → UNKNOWN/launch diagnostics. Process lifecycle `44042062610`, AI handoff `44042062563`, and process inventory `44042062765` passed on parent SHA `9722f273`; XY-C1 `44042062896` was still running at last poll (Build running; TRX/artifact pending). The isolated GNU89/ASan/UBSan policy fixture passed locally. Corrected WAN exact-head CI awaits execution.

---

## 2026-11-17 — Propagate physical WAN default-MTU setup failures

- Parent main SHA: `d20696fc602ce90167342629ced95765a9078e29`; result: THIS COMMIT.
- Finding: `start_wan()` attempted to set every physical WAN interface to MTU 1500, logged ioctl failure, then returned success if later setup succeeded. The shared `set_interface_mtu()` returns `-1` on socket/ioctl failure, so this error was silently dropped from WAN startup status.
- Added a bounded policy exception for static IPoE: when the configured MTU is 1300–1499, skip the redundant initial 1500 request because the later static-IPoE transaction applies that MTU and already gates address/route publication on its result. Other WAN modes and static settings outside that valid range still attempt MTU 1500.
- Default-MTU errors now enter `wan_start_status` using first-error aggregation and are logged; physical interface bring-up and independent protocol/service setup still continue. The static path reuses the same configured MTU read.
- Added GNU89 boundary/failure-continuation cases to `service_start_status.c`, wired its policy header into WAN and Process push/PR filters, and added `scripts/check_wan_initial_mtu_status.py` to the WAN regression job.
- Parent `d20696fc` had 8/8 exact-head selected Actions SUCCESS, including WAN route `44042507581`, Process inventory `44042507344`, and AI handoff `44042507552`. XY-C1 run `44042062896` for production-source SHA `9722f273` was still in Build XY-C1 at last poll; TRX verification and artifact upload remained pending. This source change triggers a new exact-head XY-C1 build. No full local checkout/toolchain is available; exact-head L2/L3 validation pending; router L4 NOT VERIFIED.

---

## 2026-11-17 — Make WAN DHCP callback source contract independent of brace formatting

- Parent main SHA: `bb427148c3367bdb69139ca493caf6e2ffe193ed`; result: THIS COMMIT.
- The exact-head `bb427148` WAN route checker and Process lifecycle regression both passed. The WAN DHCP callback job failed only in its inline Python source checker: it sliced the static-IPoE post-configuration path by searching for the literal token `"} else {"`, then raised `ValueError: substring not found`. The production source still contains the static branch and DHCP fallback; the checker used formatting as a boundary.
- Replaced the fragile brace-token boundary with the next semantic operation, `dhcp_status = start_udhcpc_wan(...)`. Assertions remain scoped to the static branch and still verify WAN-up → UPnP update ordering without relying on brace whitespace.
- Exact-head XY-C1 `44043949016` was in progress at last poll (dependency installation; Build XY-C1/TRX/artifact pending). Process inventory `44043948999` and Process lifecycle passed; WAN DHCP callback retry validation and current-source XY-C1 remain pending after this checker-only commit. No firmware/router L4 claim.

---

## 2026-11-17 — Remove the remaining brittle WAN DHCP source-slice boundary

- Parent main SHA: `038b2aa97c48a5003a0ccea4f4e368595c582846`; result: THIS COMMIT.
- The first checker-only repair removed the literal `"} else {"` search but the exact-head WAN DHCP callback run `44044646933` still raised `ValueError: substring not found` on the replacement DHCP-launch delimiter. The inline source-slice helper did not reliably expose that later boundary.
- Removed the unnecessary branch-end slice altogether. The contract starts at the unique `We are done configuration` marker and still requires static WAN-up to precede UPnP update; no brace or downstream DHCP marker is needed for this assertion.
- Exact-head `bb427148` WAN route `44043948927` and Process lifecycle `44043948999` passed. XY-C1 `44043949016` remained in progress at last poll; Build/TRX/artifact pending. The WAN DHCP callback exact-head rerun is pending. Router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Update WAN DHCP integration contract for MTU status propagation

- Parent main SHA: `8b950c04a80b88e475b670f02510bf67aac70db4`; result: THIS COMMIT.
- The previous exact-head WAN DHCP callback reruns still failed at inline Python line 779. Inspecting that exact line identified the real mismatch: the existing checker located the physical setup using the removed comment `Prepare the physical WAN interface without a shell command` and expected the old log string that did not aggregate MTU failures. The replacement tests had targeted an unrelated later branch-boundary expression.
- Updated the integration contract to begin at the new configured-MTU marker and assert that it reads the configured MTU, applies the static-IPoE override policy, captures/aggregates/logs default setter failure, and still attempts physical IFUP/cleanup.
- This is test-only; production code remains in `bb427148`. WAN route `44043948927` and Process lifecycle `44043948999` passed on that source SHA. The corrected WAN DHCP exact-head workflow is pending. XY-C1 `44043949016` for `bb427148` was still in progress at last poll; Build/TRX/artifact not yet confirmed. No router L4 claim.

---

## 2026-11-17 — Preserve static MAN custom-MTU failures in launch_wanx()

- Parent main SHA: `cffd4a9145b2993b41a340cb9d7e42449c97c292`; result: THIS COMMIT.
- Finding: the static MAN/PPTP/L2TP physical-interface path attempted `set_interface_mtu(man_ifname, man_mtu)` for configured MTUs 1300–1499, but only logged ioctl/socket failure. It then configured the address/routes and returned `launch_status == 0` if those later operations succeeded, hiding a known MTU mismatch from WAN startup and the caller's rollback/error handling.
- Now the MTU setter result is merged into `launch_status` with first-error aggregation and logged. Address, route and independent IPv6 setup are still attempted; the accumulated failure is returned and existing callers decide whether PPP startup can continue or must restore the prior WAN state.
- Extended `service_start_status.c` with failure-retention/continued-address-and-route coverage and expanded `check_wan_initial_mtu_status.py` to enforce MTU → status merge → diagnostic → address/route ordering.
- Parent `cffd4a91` exact-head WAN DHCP callback `44045389889`, Process inventory `44045389739`, NVRAM `44045389782`, AI handoff `44045389796`, infosvr `44045389876`, AdGuardHome `44045389845`, AliDDNS `44045389821`, and AliyunDrive `44045389731` all completed SUCCESS. The preceding production-source `bb427148` XY-C1 run `44043949016` completed Build XY-C1, TRX verification and artifact upload SUCCESS. This new MAN MTU source slice awaits its own exact-head WAN/Process and XY-C1 validation.
- No local full checkout/toolchain; router L4 NOT VERIFIED. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Retry DHCP-advertised WAN/IPTV MTU failures across renewal callbacks

- Parent main SHA: `5b9cc6d16b8676e30a12002212f8440673a15937`; result: THIS COMMIT.
- Finding: both `udhcpc_bound()` and `udhcpc_viptv_bound()` only invoked the DHCP MTU setter for an initial `bound`, although their renewal handlers explicitly route changed MTU options and failed route/setup states back through `bound(..., 1)`. The `!is_renew_mode` guard therefore skipped the required reapply on renew. Setter failures were logged but not included in the existing `route_setup_status`/retry state, so later successful address/route work masked the MTU failure.
- Both callbacks now reapply supported advertised MTUs during bound and renew. Setter failure is retained with first-error aggregation while routes and IPTV multicast still proceed; it is persisted through the existing `route_err`/`manv_route_err` retry path and returned to BusyBox's synchronous DHCP event runner. A later successful callback clears the retry state.
- Added `dhcp_wan_interface_policy.c` coverage for MTU failure retention across successful route work and the retry predicate; expanded `check_wan_initial_mtu_status.py` to assert both callback paths, renewal re-entry, status aggregation and route-error persistence.
- Parent `5b9cc6d1` exact-head WAN route run `44046257356` passed including the expanded physical/MAN MTU contract; WAN DHCP callback `44046257418` passed; Process inventory `44046257192` passed; Process lifecycle `44046257377` passed including `Compile and run RC boot service status aggregation regression`. XY-C1 run `44046257252` was still building at last poll; TRX verification and artifact upload pending. This new source slice needs its own exact-head WAN/Process/XY-C1 checks.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No local MIPS toolchain; router L4 NOT VERIFIED.

---

## 2026-11-17 — Correct DHCP MTU retry checker contracts

- Parent main SHA: `02ec8ab34bf54b5e37de6e0081df2ea2aeb37ef5`; result: THIS COMMIT.
- The exact-head WAN route run `44047414939` reached the new DHCP MTU assertions but the checker incorrectly selected the first `udhcpc_bound(..., 1)` call (the earlier route-retry branch) as the callback for a changed MTU. It now locates and asserts the separate bound re-entry after `value = getenv("mtu")`; IPTV renew uses the same ordered check.
- WAN DHCP callback run `44047414751` compiled and passed the expanded C interface/route regression, then failed only because its existing inline contract still expected the old diagnostic text `accepted but route setup is incomplete`. The production callback now correctly labels the broader MTU + route error as `accepted but interface/route setup is incomplete`; both WAN and IPTV assertions are updated.
- Production source and the new retry regression remain unchanged in this test-contract correction. Exact-head rerun is pending; the XY-C1 workflow for source `02ec8ab3` was still awaiting its build stage at last poll.
- Parent `5b9cc6d1` WAN route `44046257356`, WAN DHCP `44046257418`, Process inventory `44046257192`, and Process lifecycle `44046257377` passed; XY-C1 `44046257252` had started but was not yet through Build/TRX/artifact at the last poll.

---

## 2026-11-17 — Preserve diagnostic-only policy for optional DHCP MTU ioctl failures

- Production policy correction landed in `7f7728ab553426965c47e72812c3b2a8c6ca6338` (parent `6fa744ffe62e8b5e6795675d01cae50ff0c7871a`); THIS COMMIT records the exact-head CI diagnosis and corrects the handoff.
- Policy correction: the earlier DHCP renewal slice `02ec8ab3` correctly removed the `!is_renew_mode` guard so a changed advertised MTU is reapplied, but it also incorrectly promoted an optional MTU ioctl failure into `route_setup_status`. The pre-existing WAN callback contract explicitly keeps optional DHCP MTU setter failures diagnostic-only; they must not fail an otherwise valid lease callback or activate route retry.
- Restored that separation in both WAN and IPTV callbacks: MTU setter failures are logged only; real route errors alone populate `route_err`/`manv_route_err` and trigger existing retry logic. Applying supported MTU changes during renew remains enabled.
- Replaced the incorrect “MTU failure requests route retry” fixture with a regression proving optional MTU failure leaves route retry state clear while a genuine route error still requests retry. Expanded `check_wan_initial_mtu_status.py` to enforce the diagnostic-only/renew contract; restored WAN DHCP inline assertions to the route-only diagnostic text.
- Exact-head `6fa744ff` WAN DHCP callback `44047855549`, AI handoff `44047855547`, AliyunDrive `44047855608`, AdGuardHome `44047855578`, infosvr `44047855540`, AliDDNS `44047855453`, and NVRAM `44047855349` passed. WAN route `44047855382` failed only at its inline contract's stale route-only diagnostic assertion; Process inventory/lifecycle `44047855451` failed because `check_modem_wan_start_status.py` rejects MTU errors being merged into route status. THIS COMMIT restores the asserted policy, addressing both failures.
- XY-C1 for production source `02ec8ab3`, run `44047414713`, completed FAILURE; its build log must be checked for the first compiler error before recording a source-build result. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No local MIPS toolchain; router L4 NOT VERIFIED.

---

## 2026-11-17 — Tighten optional IPTV MTU policy checker to match the actual call

- Parent main SHA: `482b6f7dcaffd2dac29235097c221d20cd62b595`; result: THIS COMMIT.
- The production correction landed in `7f7728ab`: optional DHCP MTU ioctl failures are diagnostic-only again, while a changed advertised MTU is still reapplied on renew. Exact-head WAN DHCP, Process lifecycle, and the other triggered stability/security checks passed. WAN route `44049696922` failed only because the new semantic assertion searched for the substring `route_setup_status, mtu_status`, which also legitimately appears in the local variable declaration, despite no aggregation call.
- Replaced that broad substring check with the exact prohibited call `route_status_accumulate(route_setup_status, mtu_status)`. Source inspection confirms IPTV MTU status is only initialized, passed to the setter, tested and logged; route status is separately aggregated from route results and multicast.
- XY-C1 run `44049696160` remains IN_PROGRESS in the cross-build job at last poll. Current-source Build/TRX/artifact completion is still pending; exact-head WAN route rerun is pending.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; router L4 NOT VERIFIED.

---

## 2026-11-17 — Apply all parser-supported DHCP MTU values, including explicit 1500 reset

- Parent main SHA: `de70a509a14fd343bc3ee1fbfb9c6b2078bbff6d`; result: THIS COMMIT.
- Both `udhcpc_bound()` and `udhcpc_viptv_bound()` parse DHCP MTU in the range 576..1500 but previously applied it only for 1300..1499. Valid 576..1299 values were silently ignored; explicit 1500 could not restore the interface default after a smaller lease.
- Added `wan_dhcp_mtu_is_supported()` for 576..1500 and used it in both callbacks. Static/UI custom-MTU policy remains 1300..1499. Added GNU89 boundary assertions for 575, 576, 1299, 1300, 1499, 1500 and 1501, and semantic assertions that parser/helper/callback ranges stay aligned.
- Existing semantics remain: apply on renew, optional setter failure diagnostic-only, omitted option no-op preserving prior NVRAM state. Added the policy header to WAN DHCP workflow path filters.
- Production-source baseline `7f7728ab`: WAN DHCP `44049696447`, Process lifecycle `44049696038`, Process inventory `44049696133`, XY-C1 `44049696160` all PASS. XY-C1 artifact `14314335144`, 12,643,773 bytes; Build/TRX/upload completed. Exact-head descendant `de70a509` WAN route `44050750448` and Process `44050750354` PASS. THIS COMMIT's tests/build are pending.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---

