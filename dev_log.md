## 2026-11-16 — Propagate LAN IPv6 address cleanup failures

- Parent main: 8befa2b20295be8182053dc4e224174db471ed82. clear_lan_addr6() currently discarded clear_if_addr6(IFNAME_BR)'s errno-style result, allowing stop_lan() to report success after failed IPv6 address cleanup.
- Changed clear_lan_addr6() and rc.h declaration to return int. stop_lan() captures/logs ipv6_status and aggregates first failure in conntrack → IPv4 route → IPv6 address → bridge-down order without skipping bridge shutdown.
- Added service_start_status regression cases for IPv6-only failure, prior route error precedence and bridge-down-only failure; Process checker validates wrapper return and production aggregation. WAN route checker validates updated result order; XY-C1 path filter adds net_lan6.c.
- Exact 8bef AI handoff 43979147640 PASS; Firewall Startup 43979147580, USB modem 43979148326, AdGuardHome 43979147629, infosvr 43979147611 and AliDDNS 43979147585 PASS. Process 43979148973 and XY-C1 43979147638 were pending at last poll; current-source L3 and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-16 — Align shared and HTTPD route-startup contracts with global cache flush API

- Parent main: `bb0c693125f0ddfd4e845d6c9d4f8fb58f68d2e2`; product-source route-cache change is `493f981e4db110923ca8d16328e29aec541a3734`. The WAN route workflow `43978255126` passed, including the injected GNU89/ASan/UBSan global-cache regression.
- Exact 493f Process regression `43978254976`, Firewall Startup `43978255243`, XY-C1 `43978254941`, and HTTPD security `43978255197` all failed at stale source contracts left behind by the intentional `full_restart_wan()` API change: the common checker still required the old direct `rc_wan_route_flush_apply()` plus separate `flush_route_caches()`, and HTTPD still required the pre-composite helper call. Production source was not implicated by these failures.
- Updated both shared checker contracts to assert the composite `rc_wan_route_flush_and_cache_apply()` call, the global cache callback, first-error gate and ordering before switch reconstruction. The exact 493f AI docs failure was separately fixed by `bb0c693`, which restored the 22 previous complete root records plus the 493f record; exact AI handoff run `43978638161` now PASSes with 23 entries.
- The exact 493f XY-C1 workflow stopped in its pre-build source checker, so route-cache product-source L3 is **NOT VERIFIED**. Exact 493f WAN route `43978255126` PASS; exact bb0c Process common checker `43978638091` still failed at the same stale assertion.
- THIS COMMIT is checker/documentation only and requires fresh exact-head Process, Firewall, HTTPD, WAN route and XY-C1 results. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate global route-cache flush failures through full WAN restart

- Parent main: `2c503c9ee1b3cda569700d3e85f614eed0317ad8`. `flush_route_caches()` discarded the result of `system("ip route flush cache")`, and its only RC caller, `full_restart_wan()`, ignored the result before switch/WAN reconstruction.
- Converted the wrapper to return `eval("ip", "route", "flush", "cache")`, updated the public prototype, and combined global-cache and per-interface route flush results using first-error semantics. The cache attempt still runs when an interface flush fails; route-dependent WAN restart work is blocked if either cleanup stage fails.
- Added injected regression cases for route-first failure, cache-only failure, successful cleanup, and missing callback. WAN route CI compiles the test with GNU89/ASan/UBSan and checks source API, caller wiring and restart gate.
- Parent exact WAN route `43977147385`, Process `43977147335`, and AI handoff `43977147281` passed; NVRAM `43977147536` was pending. Exact 759c18 XY-C1 `43976717898` was still building. This route-cache change needs its own same-SHA Build/TRX/artifact verification. Physical-router L4 remains NOT VERIFIED.
- Next: inspect exact-head WAN route/Process/AI/NVRAM/XY-C1 Actions and resolve first causal failure.

---

---

## 2026-11-16 — Align WAN route checker with LAN teardown status propagation

- Parent main: 759c18d860ba1c0cb547f43fa7ca8d21b6580987. The LAN route cleanup source change in 759c18d correctly captures clear_if_route4(IFNAME_BR), logs a nonzero result, and returns the first failure across conntrack, route cleanup and bridge shutdown while continuing IPv6/interface cleanup.
- Process integration and service-status regression passed on 759c18d (43976717949 and 43976718109); WAN DHCP callback 43976717974 and AI handoff 43976717913 passed. WAN route run 43976718132 failed at its inline stop_lan() contract because it still required the previous two-status return rc_first_service_start_status(conntrack_status, status).
- Updated the WAN route workflow assertion to require the nested conntrack/route/bridge status aggregator and verify route flush precedes IPv6 and interface cleanup. Checker-only change; production source unchanged. Exact-head WAN route rerun required.
- XY-C1 43976717898 was pending at last poll. Product-source baseline cffc149 remains L3 VERIFIED by successful Build/TRX/artifact; LAN route slice still awaits same-SHA L3 evidence.

---

## 2026-11-16 — Preserve LAN static-route flush failures during teardown

- Parent main: 344db6b6cb4309ad1a2cdbf29b2cc0ee50ad5898. In stop_lan(), clear_if_route4(IFNAME_BR) returned the route-flush result but its caller discarded it. If conntrack and bridge ifconfig succeeded, a failed static-route cleanup was silently reported as a successful LAN teardown.
- Captured and logged route_status, then nested first-error aggregation across conntrack cleanup, route flush and bridge shutdown. IPv6 cleanup and ifconfig remain attempted after a route failure; AP-mode behavior is unchanged.
- Added service_start_status regression coverage for route-only failure, earlier conntrack error precedence, and bridge-down-only failure. Added Process integration assertions for result capture, aggregation and route→IPv6→interface operation ordering. Existing Process and XY-C1 workflows already compile this test and trigger on net_lan.c.
- Parent source L3: XY-C1 run 43973861873 on cffc149 passed Build, TRX verification and artifact upload. The exact 344db6b handoff rerun and this slice's exact-head Process/WAN/XY-C1 checks are pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Update WAN DHCP callback contract for conntrack cleanup status

- Parent main: `44fd9bedf4b7b1ab4e9d4408a6157681158774d3`. Exact `44fd9bed` WAN route, Process inventory, AI handoff, NVRAM, AdGuardHome, AliDDNS, AliyunDrive and infosvr Actions passed.
- Exact cffc WAN DHCP callback run `43973861977` passed its C regressions, then failed its final source-contract assertion because the test still required `return route_status_accumulate(interface_status, cleanup_status);`. `udhcpc_deconfig()` now also aggregates the conntrack write result, so the old two-status assertion was stale.
- Updated the WAN DHCP workflow to require `conntrack_status` initialization, MAN address flush, error check, flush-before-cleanup order and the full nested first-error return. No production source changes; exact-head WAN DHCP rerun is pending.
- XY-C1 run `43973861873` for product source SHA cffc was still building at last poll. Current product L3 and physical L4 remain NOT VERIFIED.
- Next: inspect the exact-head WAN DHCP run and complete XY-C1 Build/TRX/artifact verification, then trace `flush_route_caches()` and every caller before any API change.

---

## 2026-11-16 — Fix second stale MAN conntrack status contract

- Parent main: c3bcb486aa97e0dbb478f05585e6c24ab33407bc. Exact c3b WAN route run 43974249410 passed earlier injected conntrack, bridge/IPv6 and route tests; its final source contract failed at Python line 304 because a legacy assertion still expected return route_status_accumulate(interface_status, cleanup_status). Since conntrack_status is now a first-class teardown result, production correctly returns the nested first-error aggregate that includes it.
- Updated that remaining assertion to remove whitespace before comparing the full expression. This is a checker-only correction; no production source changes. Exact c3b Process 43974249384 and AI handoff 43974249444 passed; cffc Firewall Startup 43973861431 passed. cffc XY-C1 43973861873 remained in progress at last poll.
- Current product-source L3 and physical-router L4 remain NOT VERIFIED until exact evidence exists.

---

## 2026-11-16 — Normalize conntrack deconfig status contract after exact-head test

- Parent main: `cffc1497fda9f65d9a0b163bad5acc1723c23f81`. Exact-head WAN route run `43973861975` compiled and passed the new conntrack errno policy regression (GNU89, ASan, UBSan), all existing route/IPv6/bridge tests and the WAN VLAN source check. Its final source-contract step failed only because the new assertion compared whitespace-formatted `udhcpc_deconfig()` code without removing all whitespace; the production return aggregation is intentionally split across lines.
- Exact-head Process `43973862332`, Firewall Startup `43973861431` and AI handoff `43973861519` passed. XY-C1 `43973861873` was queued at last poll. This fix removes whitespace before comparing the nested first-error aggregation expression; no production source changes.
- Earlier exact `1f829937` XY-C1 run `43971856235` passed the complete firmware build, TRX output verification and artifact upload. That L3 evidence predates the conntrack source/API change. Current cffc source L3 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate conntrack table flush failures through RC teardown

- Parent main: `1f8299371114d2fb846ec1c47a0116afd584668c`. Traced all RC references: `flush_conntrack_table()` was declared `void` and discarded the errno-style result of shared `fput_string("/proc/net/nf_conntrack", ip)`. All four callers ignored write failure: PID 1 `RCN_RESTART_NETFILTER`, LAN teardown, WAN teardown and MAN DHCP deconfig. The shared writer returns errors from buffered writes and close.
- Changed the API to return the writer result and added an injectable policy regression for targeted IP writes, full-table `"f"` flush, exact errno propagation and missing writer rejection. PID 1 merges the status into its notification result; LAN, WAN and MAN teardown retain the flush error while still performing later cleanup.
- Added the test to the GNU89/ASan/UBSan WAN route workflow and source-contract checks for every production caller. New exact-head WAN, Process, Firewall Startup, XY-C1 and AI handoff actions are required after publication.
- Exact `1f829937` Process `43971856210`, Firewall Startup `43971856255` and AI handoff `43971856365` passed; XY-C1 `43971856235` was still building at last poll. This evidence predates the conntrack source change. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Finish strict HW NAT CI contract and exact checkpoint wording

- Parent main: `99d8562d8b007e83ceaff397f7c11337b9c6277a`. Exact 99d856 actions exposed one last stale common source assertion: `reload_nat_modules()` now configures radio settings only under `hwnat_loaded == 1`, passing literal 1 to the policy callback, but the checker still required the old raw `hwnat_loaded` argument. Replaced it with explicit verified-loaded gating and no-legacy-helper assertions.
- Exact-head 99d856 Process `43971105805`, Firewall Startup `43971105848` and XY-C1 `43971105719` all failed at that same checker assertion before their downstream integration/build steps. The 99d856 AI handoff `43971105596` found the handoff omitted the exact sentence `The checkpoint is mandatory; a document edit is conditional.`; this commit adds the required exact contract wording.
- Exact 99d856 PASS: NVRAM `43971105616`, USB modem identity `43971105688`, infosvr `43971105912`, HTTPD `43971105585`, AliyunDrive `43971106167`, AliDDNS `43971105599`, AdGuardHome `43971106008`. No same-SHA firmware build/TRX/artifact completed because XY-C1 stopped at the shared checker; current-source L3 remains NOT VERIFIED.
- Source HW NAT/FTP/SFE code remains as in `1554a5f3`; this atomic follow-up changes only checker/documentation contracts. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Align NAT tri-state caller contracts across CI

- Parent main: `ebf2d285454eba3480fe594808aa31367513ffe4`; source implementation is already in `1554a5f3`. Exact-head 1554 Process `43969810170`, Firewall Startup `43969809710` and XY-C1 `43969809836` all reached `check_modem_wan_start_status.py` and failed at the same stale assertion: the source stores `-module_loaded` in `status` then returns `status`, while the checker incorrectly required a direct `return -module_loaded`. Exact 1554 HTTPD `43969809668` failed its stale HW NAT radio assertion, which still expected the pre-change `hwnat_loaded` argument.
- Updated the common source checker to assert the actual errno assignment and the HTTPD integration contract to require a successful tri-state probe before policy/config use and to guard radio writes on `hwnat_loaded == 1`. This is a test-contract correction; production source remains unchanged in this commit.
- Exact-head ebf2 AI handoff `43970209185`, NVRAM `43970209177`, AdGuardHome `43970209909`, infosvr `43970209228`, AliyunDrive `43970209300`, AliDDNS `43970209283` passed. Process inventory `43970209366` failed at the same stale `return -module_loaded` checker assertion; it is corrected here.
- d287 exact-head XY-C1 `43967039935` previously passed Build, TRX output and artifact upload; that L3 evidence does not cover the newer HW NAT/FTP/SFE source. Exact-head reruns are required after this checker correction. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Normalize whitespace in PPPoE sysctl status assertion

- Parent main: 1554a5f3fbb1bf13c3325ad765b7e806d74d1537. Exact-head Process run 43969335166 reached the new sysctl integration contract, then failed because the multiline rc_first_service_start_status() call left whitespace immediately after its opening parenthesis. The checker expected a no-space argument sequence.
- Updated the assertion to remove all whitespace from the function body before matching the exact helper call, without weakening the production definition/prototype/dispatcher contract.
- The d287 exact-head XY-C1 run 43967039935 passed Build, TRX verification and artifact upload; it predates the new sysctl source. Current-main Process/XY-C1 validation is pending.
- Physical-router L4 remains NOT VERIFIED.

---
## 2026-11-16 — Propagate unknown NAT module state through HW NAT, FTP and SFE

- Parent main: `d83760b4c8e5514fd996f986b1400b7b794876f4`. Confirmed a remaining module-state defect in `net.c`: HW NAT, FTP and SFE branches used the legacy Boolean probe before invoking strict load/unload helpers. A negative `/sys/module` probe could be treated as absence and silently skip a required teardown or configuration check.
- HW NAT now returns tri-state status; disabled mode invokes verified unload unconditionally, enabled mode surfaces initial probe errno, only verified-loaded modules receive radio sysfs writes, and configuration propagates unknown module state. FTP parameter reconfiguration is blocked on UNKNOWN rather than risking a live helper mutation. SFE enable/disable now uses verified load/unload on every transition and returns the actual failure status.
- d287 exact-head XY-C1 `43967039935` passed Build XY-C1, TRX verification and artifact upload; WAN route `43967040074`, Process `43967040068`, AI handoff `43967040010`, NVRAM `43967039938`, HTTPD/AdGuardHome `43967039819`, infosvr `43967039889`, AliyunDrive `43967040044`, AliDDNS `43967040108` all passed. This is L3 only for d287, not the new source.
- d837's unrelated sysctl checker correction has AI handoff `43969335259` PASS; Process regression `43969335166`, NVRAM `43969335303`, AdGuardHome `43969335428`, AliDDNS `43969335268` were in progress at last poll. New exact-head Process, Firewall Startup, XY-C1 and handoff tests must validate this module-state source slice. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Correct whitespace contract in sysctl status checker

- Parent main: 1e10514145025ceb0ce9e0a0cd5468e28e6b7fa0. The first follow-up tried to make the function-signature assertion independent of line formatting, but exact Process CI later showed the remaining failure was a separate multiline rc_first_service_start_status() call assertion: whitespace remained after the opening parenthesis. This is corrected by the next entry.
- XY-C1 exact-head run 43967039935 on d28729a completed successfully: Build XY-C1, TRX verification and artifact upload all PASS. The later 1e105 sysctl source change still requires its own exact-head Build/TRX/artifact run.
- The Process lifecycle regression on 1e105 was queued when last checked; this checker correction is included before its first execution. Exact-head Process/XY-C1 results for this repair remain pending.
- Physical-router L4 remains NOT VERIFIED.

---
## 2026-11-16 — Propagate PPPoE relay failure through sysctl notification

- Parent main: d28729af83c9692f376539c31d9578f129774d20. set_passthrough_pppoe(1) returns an errno-style status, but restart_all_sysctl() discarded it and its only RCN_RESTART_SYSCTL caller also ignored the helper result. PID 1 could therefore acknowledge a failed PPPoE relay startup/readiness operation.
- Changed restart_all_sysctl() to return/log the relay status, keep optional Samba fastpath work independent, update the public prototype and merge the result through rc_notify_merge_action_result() at the sole PID 1 dispatcher callsite.
- Added a host regression for positive errno retention across later independent work and conversion to a negative/unacknowledged notification result, plus an integration contract checking the actual definition, prototype and caller. Exact-head Process/XY-C1 verification is pending.
- Physical-router L4 remains NOT VERIFIED.

---
## 2026-11-16 — Restore strict module-probe CI contracts and developer-log history

- Parent main: `b6ff015c1a70e9b61bebfbcf33aae9d1de113946`. Commit `b6ff015c` finishes migration of all four module-load sites to the strict tri-state probe; the source checker now verifies the boot and wireless callbacks plus negative errno propagation.
- Exact-parent Firewall Startup `43966109926` passed. WAN route `43966109986` failed only at a whitespace-sensitive assertion for the multiline IPv6 cleanup call. XY-C1 `43966109916` stopped at a stale checker expecting the previous Boolean load policy. AI handoff `43966109747` found only one root developer-log entry; restored the full 109-entry log from the pre-truncation parent snapshot.
- Process regression `43966109822`, XY-C1 `43966109916`, NVRAM `43966109901`, GPIO `43966109653`, notification queue `43966109981`, AliDDNS `43966109957` and USB storage `43966109731` were pending/in progress at last poll. Other completed b6ff Actions passed except the failures above; query exact SHA for current outcomes.
- Earlier source SHA `eb566f3` passed XY-C1 Build/TRX/artifact, but predates later IPv6/module-load changes. b6ff XY-C1 did not reach Build; newer L3 remains NOT VERIFIED. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Preserve tri-state module-load probe failures

- Main parent: `beee6893b7c14eccc94634fe51dc41d2e748b8b7`. Strict `is_module_loaded_status()` already served unload checks, but the load verifier still treated any nonzero callback value as success and four production loaders still used the lossy Boolean wrapper.
- Updated `rc_module_load_verified()` for 1=loaded, 0=absent and negative errno=probe failure. Migrated NAT, boot hardware, wireless driver and LAN bridge module-load sites to the strict probe.
- Added injected tests for direct errno propagation and for trying all required modules while retaining the first failure. Extended Process/WAN source contracts to cover every production loader and both regression cases.
- Parent exact-head AI handoff passed; WAN route regression was running after correcting its path-filter assertion. The prior `eb566f3` XY-C1 Build/TRX/artifact passed, but the newer IPv6 cleanup change and this source slice require exact-head verification. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Fail closed when WAN interface route cleanup fails

- Parent main: `e928181da3a52b4035d1ccf03307f212cafa2a4e`. `full_restart_wan()` ignored both `clear_if_route4()` command results and continued into switch table clearing, WAN variable reset, static route restoration and WAN restart. A stale interface route could survive undetected.
- Changed `clear_if_route4()` to return the `eval()` status and pass the interface name as a single argv element rather than shell interpolation. Added `rc_wan_route_flush_apply()`: attempt both interface flushes, retain the first failure, and stop the WAN rebuild before switch/MAC and route reconfiguration when either flush fails.
- Added GNU89/`-Werror`/ASan/UBSan regression for two-interface attempt order, first-error retention, later-only failure, invalid interface and invalid policy arguments. Process lifecycle run `43960589874` passed this test on source SHA `58318f5`; WAN route regression `43960590043` also passed.
- Workflow path filters were repaired in `f2d9ccef50d3ebfc0f6f5dde94fae7893c7bb58d`. Exact-head Firewall Startup `43961249138` passed. HTTPD `43961249141` exposed a whitespace-sensitive checker for a multiline call (the compiled route test passed); checker normalization is fixed in `eb566f346f63d3d6ed295c08170ac6d834ff7f04`. Current-head HTTPD `43961727463` passed. Current-head XY-C1 `43961726533` passed all pre-build checks and route host regression; Build XY-C1 was in progress at last poll, with TRX/artifact pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate HW NAT radio configuration failures

- Parent main: `93d0254006bea2e5743715e9b039bb51a0c09f52`. HW NAT radio-registration commands for 2.4 GHz and 5 GHz plus the IPv6 debugfs write previously discarded their `doSystem()` results.
- Added wait-status normalization (including command-launch errno), independent attempt semantics for both radios and IPv6 debugfs, first-error propagation into the NAT reload result, and fault-injection coverage for success, nonzero command exit, signal termination, command-launch failure, disabled/unloaded module and invalid callback.
- Exact-head `0b32b9111c38e2cf7588cf6605b2491f25e3ce31` had Firewall and Process regressions pass; XY-C1 Build/TRX/artifact passed. HTTPD initially failed only on a newline-sensitive source assertion; it was corrected in `e928181da3a52b4035d1ccf03307f212cafa2a4e`. Exact-head `e928181` HTTPD, AI handoff and XY-C1 (including Build, TRX and artifact upload) all passed. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Propagate Adbyby update action failure to PID 1

- Main parent: `e09e044195fd936142ed0b42ead1bd73f6262c02`. In the RC notification dispatcher, `RCN_RESTART_UPDATEADB` called `update_adb()`; that void wrapper ignored the wait status returned by `eval("/usr/bin/adbyby.sh", "updateadb")`. A failed update therefore left the notification action result unchanged.
- Changed `update_adb()` to return int, updated its public prototype, and merged its result into `action_result` with `rc_notify_merge_action_result()`. Added an integration-checker contract for the helper, declaration and actual notification caller.
- Strict module-probe commit 4c435 had 38/39 Actions passed at last poll; its XY-C1 Build/TRX/artifact run `43953778444` was still in progress. The Adbyby fix requires a new exact-SHA verification. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Fail closed on RC module-state probe errors

- Main parent: `1aca5ab3a776a66138814769b84d5e5ceb43b46c`. Audited `common_ex.c:is_module_loaded()`: it returned 0 for all `opendir("/sys/module/<name>")` failures, conflating absence (ENOENT) with permission/resource/sysfs errors. `rc_module_unload_verified()` could consequently skip an unload and falsely report absence.
- Added tri-state `is_module_loaded_status()`: 1=loaded, 0=ENOENT only after the sysfs module root opens, negative errno=invalid path or probe failure. Preserved the legacy boolean `is_module_loaded()` wrapper and bound NAT unload verification to the strict probe.
- Added injectable regressions for an initial probe error (must not call unload) and a post-unload probe error (must return the error). Updated canonical source checks and push/PR path coverage.
- Exact 1aca runs before this slice: Process regression `43952107815`, HTTPD `43952107789`, AI handoff `43952107818` passed; XY-C1 `43952107870` was `in_progress`. This commit needs fresh exact-SHA results. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Include the inline service-start result policy in firewall_ex

- Exact-head 09ea XY-C1 `43947164750` reached MIPS link and failed with undefined references to `rc_first_service_start_status` from `firewall_ex.o`. The helper is static-inline in `service_start_status.h`; `firewall_ex.c` had not included that header. Added the include and added a canonical source assertion.
- Exact-head 89a43d9 Firewall Startup `43948306770`, Process regression `43948306885`, HTTPD `43948306816`, AI handoff `43948306828`, USB modem `43948306849`, infosvr `43948306784`, AliDDNS `43948306867`, AdGuardHome `43948306841`, and AliyunDrive `43948306780` passed. NVRAM `43948306748` remained pending and XY-C1 `43948306746` was still building.
- This is a production link correction. Exact-SHA Build/TRX/artifact must pass again; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align NAT module assertions with policy inputs and HW NAT API

- Exact-head 39190 Firewall Startup `43947724068` and Process regression `43947724044` failed at the new source checker because it reused `POLICY_SOURCE/POLICY_TEST`, which refer to modem-WAN policy files. Added dedicated `MODULE_POLICY_SOURCE/MODULE_POLICY_TEST` bindings for the wireless module policy and regression.
- Exact-head 39190 HTTPD `43947724129` failed because its inline checker expected the old single-argument `hwnat_configure` signature. Updated the contract to the allow-aware two-argument function and its call.
- 39190 USB modem identity `43947724775`, AI handoff `43947724226`, process inventory `43947724196`, and WAN route `43947724172` passed; NVRAM, USB storage and XY-C1 were pending. This is validation repair only; no firmware result inferred.
- NAT module unload policy remains under exact-head validation; physical-router L4 is NOT VERIFIED.

---

## 2026-11-17 — Normalize optional firewall hook ENOENT checker

- Exact-head 8c77 Firewall Startup `43946213665` and Process regression `43946213545` passed all preceding firewall/module/status policy tests but the canonical checker expected `errno!=ENOENT` after `compact()`; the normalized actual source is `errno != ENOENT`. Changed the assertion to a whitespace-tolerant `re.search`.
- Exact-head 8c77 XY-C1 `43946213553` stopped at this checker before Build/TRX/artifact. HTTPD `43946213772`, NVRAM `43946213544`, AI handoff `43946213616`, AliyunDrive `43946213652`, AliDDNS `43946213539`, AdGuardHome `43946213561`, USB modem `43946213534`, and infosvr `43946213737` passed.
- Next P1 remains NAT helper and HW NAT/SFE module unload/load postconditions. No physical-router L4 claim.

---

## 2026-11-17 — Repair escaped firewall checker regex and HTTPD hook helper signature

- Exact-head 8956 Firewall Startup `43945597477`, Process regression `43945597663`, and XY-C1 `43945597624` failed because the checker regex was emitted with two backslashes in a raw string, matching literal backslashes rather than whitespace. The production source contains nine status-aggregation calls; the regex now uses one escape level.
- Exact-head 8956 HTTPD `43945597526` failed at its inline helper boundary because the assertion expected `static void firewall_log_hook_stage_failure`, while the implementation now returns `int` to propagate staging failures. Updated the expected signature.
- Exact-head 8956 AliyunDrive `43945597544`, AliDDNS `43945597596`, AdGuardHome `43945597557`, AI handoff `43945597849`, USB modem `43945597613`, and infosvr `43945597566` passed; NVRAM `43945597771` was pending. The old SHA's failures are not treated as fixed until the new exact-head suites pass.
- Next P1: audit module unload/load postconditions for configured NAT helpers and optional HW NAT/SFE. No XY-C1 L3 or physical-router L4 claim for this source state.

---

