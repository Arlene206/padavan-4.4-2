## 2026-11-21 — Assert the duplicate-Authorization auth gate in HTTPD session contract

- Parent main SHA: `91ab0fb6ceaf0c2fa068cd9cad32f57422bfa438`.
- Exact-head HTTPD security #44931678295 reached the final login activity-refresh contract and failed at Python line 1012: the checker still looked for `if (!auth_check(authorization))`. The production gate now rejects repeated Authorization fields with `if (authorization_header.duplicate || !auth_check(authorization))`; updated the checker to assert the actual branch and preserve the ordering check before refreshing login activity.
- AI handoff #44931678330 and Process inventory #44931678314 passed on exact 91ab. VPN server lifecycle #44931678366 also passed. XY-C1 #44931678300 and NVRAM audit #44931678280 were still in progress at last poll.
- HTTPD workflow source-contract only; no production source changed. The XY-C1 host-only run on parent 4e2 had its product Build/TRX/artifact steps skipped, so L3 for the current HTTPD source remains unverified until a product-source commit triggers a full exact-head build.

---

## 2026-11-21 — Fail closed VPN server startup on unreadable enable/type configuration

- Parent main SHA: `4e2e701f33d2be2c35004015d6625b718be67a7f`.
- `start_services_once()` invokes `start_vpn_server()` unconditionally. Its private helper used `nvram_invmatch("vpns_enable", "1")`; the compatibility getter maps backend errors to NULL and `nvram_invmatch()` returns false for NULL, so an absent or failed enable read could fall through to the PPTP startup path. The type reader used `nvram_get_int()`, which also maps backend errors to 0 (PPTP). The reapply helper repeated both unsafe reads.
- Added checked enable/type readers shared by startup and reapply. Missing enable means disabled; backend errors abort. Missing type retains the old PPTP default only after an explicit enable; malformed, unsupported and failed type reads abort before lease inventory cleanup.
- Added GNU89 ASan/UBSan policy regression for missing, explicit, malformed, unsupported and failed reads; added exact workflow filters, test execution and semantic source contracts. Exact-head VPN stability and XY-C1 Build/TRX/artifact checks are pending for the new code.
- On parent 4e2, XY-C1 #44931211445 and Process #44931212103 passed. HTTPD #44931211349 remains failed at source-contract line 1012; AI handoff #44931211395 failed because the required `## Active task` heading was missing. This commit restores that heading; HTTPD source-contract follow-up remains an independent correction.

---

## 2026-11-21 — Repair HTTPD regression locator and enable XY-C1 coverage

- Parent main SHA: `f7a3a520f9ae0df659c651ce99b4ced20d103e23`.
- Exact-head HTTPD security run #44930885718 reached the final authentication/access source-contract block, then failed at Python line 502 with `ValueError: substring not found`. The new contract attempted to find an escaped-newline function boundary that is not a literal sequence in the source; it now stops at the actual following symbol `reset_lang_dict(void)`.
- XY-C1 validation did not trigger for the HTTPD production change because its push path filter omitted `httpd.c` and the new `httpd_basic_auth.h` policy header. Added those paths and `base64.c` to ensure HTTPD authentication changes receive the MIPS cross-build; the XY-C1 workflow already includes itself in its path filter, so this workflow correction also triggers validation for the current head.
- On parent SHA f7, AI handoff #44930885671 and Process lifecycle inventory #44930885725 passed; HTTPD security #44930885718 failed only at the source-contract locator described above. NVRAM audit #44930885723 was still in progress at the latest poll.
- Workflow/docs only in this commit; the HTTPD production change is in parent SHA f7. Exact-head HTTPD security and XY-C1 Build/TRX/artifact validation are required before declaring this slice complete.

---

## 2026-11-21 — Reject ambiguous duplicate HTTP Authorization fields

- Parent main SHA: `95180e9d7b1d86676928ebb4c58c8af4af4adbab`.
- HTTPD request parsing previously overwrote the saved `Authorization` pointer whenever another case-insensitive Authorization field arrived. That made credentials depend on the last repeated field, while intermediaries and parsers can interpret repeated authentication fields differently. Added an explicit one-field state helper: the first value is retained, any second value marks the request credential ambiguous and clears the value, and subsequent duplicates remain invalid. Protected routes now fail the Basic-auth check when the duplicate flag is set.
- Added GNU89 unit coverage for first-header acceptance, duplicate invalidation, and continued invalidity after a third header; extended the HTTPD security workflow's source contract to require the parser/state/auth-gate wiring.
- Previous WAN route verification on exact SHA `95180e9d7b1d86676928ebb4c58c8af4af4adbab` passed (#44930698354), as did AI handoff (#44930698407). XY-C1 Build, TRX verification and artifact upload all passed on product SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192` (#44929756345); L3 is VERIFIED for that product-source SHA. Hardware L4 remains NOT VERIFIED.
- This is a production HTTPD security change. Fresh exact-head HTTPD security and XY-C1 Build/TRX/artifact Actions are required for this commit; no local build is claimed.

---

## 2026-11-21 — Normalize gateway-read count in WAN teardown contract

- Parent main SHA: `7de7b00754ffcb5c34b20db82e60c0069efd204c`.
- Exact-head WAN route #44930308919 reached the final teardown-order contracts and failed at Python line 390. The assertion counted a split C call using raw source text, so it did not match the multiline gateway getter. Replaced it with an exact count over the whitespace-compacted WAN teardown body; the expected two calls remain enforced (failed-tuple default-route fallback and PPP default-route read).
- AI handoff #44930308925 passed on the same SHA. The prior route run #44930083634 reached the next source assertion after earlier formatting repairs; this correction targets the exact current failure only.
- Workflow/docs only; no production source changed.

---

## 2026-11-21 — Normalize PPP-only WAN address checker

- Parent main SHA: `4dc1e9db6dd81451530593170d0679c79d4aab91`.
- Exact-head WAN route #44930083634 progressed through all policy and sanitizer tests and the standalone DNS checker, then failed at inline workflow assertion 370. The PPP address getter is intentionally split across C source lines, so direct substring counting returned zero. Changed the assertion to count the complete getter call in `wan_down_compact`; changed the independent-netmask exclusion to use the same compact representation.
- The prior exact-head WAN route failed at assertion 366 for the same formatting class; the new commit already corrected the gateway fallback contract.
- AI handoff #44930083703 passed on exact SHA 4dc. d0 NVRAM audit #44929756500 passed; XY-C1 #44929756345 remains in progress at last poll. WAN teardown and DHCP callback passed on d0.
- Workflow contract/docs only; no production source changed.

---

## 2026-11-21 — Fix the WAN route snapshot source-contract formatting

- Parent main SHA: `a760c37dd793f0497907fb5b30a871be6497415b`.
- Exact-head WAN route #44929756615 on `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192` passed compilation, policy tests and the standalone DNS/read checker, then failed at inline workflow assertion 366. The production fallback call is formatted across C lines; the assertion searched for an unnormalized spaced token after joining source lines. Updated it to compare the already whitespace-compacted function body, preserving the requirement that a checked gateway-only fallback follows a failed route tuple.
- WAN DHCP callback #44929756539 passed on d0. WAN teardown #44929756599, Process inventory #44929756463, AI docs #44929756531, and the listed userland stability workflows on d0 passed. XY-C1 #44929756345 and NVRAM audit #44929756500 were still running at last poll.
- Current process checker corrections are on a760; Process regression #44929876370 and AI handoff #44929876383 both passed on that exact SHA.
- Workflow assertion/docs only; no production source changed.

---

## 2026-11-21 — Align optional boot-daemon checks with checked enable gates

- Parent main SHA: `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192`.
- Exact-head Process regression on `e302ebc82d5c6b7b464db9c153dbdae1e740580d` passed crond startup contracts and then failed because the later infosvr/LLTD/rstats checker still expected raw `nvram_invmatch()` guards. All three source functions already use `service_nvram_match_status()`, return the config-read error, and treat disabled as a successful no-op. Replaced the stale source assertions with normalized checked-read/error/disabled/preflight ordering for all three daemons.
- New exact-head WAN route verification continues against the PPP-safe address fallback and transactional tuple changes; XY-C1 Build/TRX/artifact and NVRAM audit on SHA d0 were still running at last poll.
- Checker-only change; no production source changed.

---

## 2026-11-21 — Preserve PPP conntrack address after coherent route snapshot

- Parent main SHA: `e302ebc82d5c6b7b464db9c153dbdae1e740580d`.
- Review of the transactional route snapshot exposed an adjacent regression: the old unconditional address read also supplied the later WAN conntrack flush. The first snapshot implementation skipped that read for PPP links, leaving `wan_addr` uninitialized while `wan_addr_status` remained zero. PPP teardown now initializes all route buffers and performs a separate checked address read solely for conntrack; PPP remains excluded from gateway /32 ownership decisions and still uses a checked gateway-only read for default-route cleanup.
- Exact-head WAN route #44929460883 on `a1c74ff496f9302f76310e3fd6fbd51e43192735` failed in `scripts/check_wan_dns_read_status.py`, whose route contract still required three independent address/gateway/netmask getters. Replaced that contract with transactional tuple, fail-closed /32, PPP conntrack-address and default-route fallback assertions; updated the WAN workflow's corresponding source contract.
- Exact-head WAN DHCP callback #44929461023, USB modem #44929460928, Process inventory #44929461012 and AI handoff #44929460906 passed on `a1c74ff496f9302f76310e3fd6fbd51e43192735`. XY-C1 #44929461024 and NVRAM audit #44929460857 were still running at latest poll; the corrected crond Process integration run is pending.
- This changes production WAN source; fresh exact-head WAN, DHCP, Process and XY-C1 Build/TRX/artifact results are required. L3 remains VERIFIED only for product SHA 335f.

---

## 2026-11-21 — Normalize crond return-value contracts with compact source

- Parent main SHA: `a1c74ff496f9302f76310e3fd6fbd51e43192735`.
- Follow-up inspection of the whitespace-normalized crond checker found three return-value expectations still searched for spaced C tokens inside the compacted function body. Corrected them to match the normalized source, so the test still verifies the config-read error return and disabled no-op return rather than weakening the checks.
- Exact-head Process/WAN/XY-C1 runs for the transactional route snapshot are still being evaluated; the prior WAN route and WAN DHCP failures on SHA 743 were due to the two-backslash NUL literal and have been fixed in `a1c74ff496f9302f76310e3fd6fbd51e43192735`.
- Checker-only correction; no production source changed.

---

## 2026-11-21 — Correct the next CI-exposed checker and C test issues

- Parent main SHA: `74329b3f1e1ae376930dbeb1143d305fb3f6d646`.
- Exact-head Process integration on `0b97739fdc80a5273b5388f327469e95df4bfb05` exposed that the checker matched a split C call as though the function name and string argument were adjacent. Both duplicated crond contracts now normalize C whitespace before asserting the checked enable/error/disabled sequence.
- Exact-head WAN route, WAN DHCP callback and USB modem regressions on `74329b3f1e1ae376930dbeb1143d305fb3f6d646` found the new test used a two-character C literal for NUL; corrected it to the single-character `'\\0'`. The WAN workflow checker also now loads the WAN NVRAM test source before asserting the new test marker, avoiding an undefined Python variable.
- These are test/checker corrections only; no production source changed. Re-run exact-head Process, WAN route, WAN DHCP callback, USB modem and affected XY-C1 validations.

---

## 2026-11-21 — Serialize WAN route ownership snapshot across NVRAM reads

- Parent main SHA: `0b97739fdc80a5273b5388f327469e95df4bfb05`.
- Fixed a concrete teardown TOCTOU hazard: `wan_down_internal()` previously read `wanN_ipaddr`, `wanN_gateway` and `wanN_netmask` through three separately locked API calls, allowing a concurrent writer to mix generations before deciding whether the WAN-owned gateway /32 should be deleted.
- Added `wan_unit_route_snapshot()`, which reads the three route-ownership fields under one exclusive NVRAM transaction using the existing checked snapshot helper. PPP links continue to use a gateway-only checked read because they do not own this /32 route.
- Failure handling deliberately preserves default-route cleanup: when the complete snapshot fails, every ownership field is invalidated and its status denies /32 deletion, then a separate checked gateway-only fallback may still remove the default route. A successful tuple uses the same generation for default and host-route decisions.
- Added a three-field transactional regression with success and failure-at-each-field coverage; updated the exact WAN route workflow's source contract to enforce transaction wiring, deny-on-failure and the separate default-route fallback.
- XY-C1 validation remains VERIFIED only for product-source SHA `335fddd5b61d20e22987da948136c412cf4bbf81`; this changes production source, so the current WAN behavior needs a fresh exact-head regression and XY-C1 L3 validation. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-21 — Align crond lifecycle contracts with checked enable reads

- Parent main SHA: `fac45a05fa27110c2301e72829666e2fbe0d32fc`.
- Exact-head Process regression #44928653157 reached the final process/lifecycle integration checks, then failed because two duplicated checker sections still expected the retired `nvram_invmatch("crond_enable", "1")` enable gate. Production `start_crond()` now uses `service_nvram_match_status()`, aborts on read error, and returns a disabled no-op before process preflight. Updated both checker paths to assert that checked read/error/disabled ordering while retaining RUNNING no-op and UNKNOWN fail-closed assertions.
- XY-C1 firmware validation #44927981534 completed successfully on product-source SHA `335fddd5b61d20e22987da948136c412cf4bbf81`: cross-build, TRX output verification and artifact upload all PASS. L3 is VERIFIED for that exact product-source SHA only; this does not establish hardware L4.
- fac45 WAN route #44928653174 and AI handoff #44928653213 passed; Process regression #44928653157 needs a fresh rerun after this checker-only correction.
- No production source changed in this slice.

---

## 2026-11-21 — Restore WAN route workflow YAML and ttyd rollback contracts

- Parent main SHA: `a77ed612c19ac0b4c29d88f4b547fb7e077bd0d9`.
- Exact-head Process regression #44928210224 passed process inventory and GNU89/ASan/UBSan execution before failing at a stale ttyd local-variable contract. Production `start_ttyd_with_mode()` now preserves `start_errno` and routes launcher, wait, UNKNOWN-inventory and timeout failures through `optional_daemon_rollback_failed_start()`; both duplicated checker blocks now assert that actual rollback sequence.
- Exact-head WAN route workflow #44928208859 created no job. Inspection found six gateway address/netmask validation assertions accidentally outdented to YAML root level below `jobs:`; re-indented them into the final inline Python `run: |` block. The `require()` helper is already defined within that block.
- On product-source SHA 335fddd5b61d20e22987da948136c412cf4bbf81, NVRAM transaction audit #44927981607 and WAN DHCP callback #44927981545 passed; Process integration #44927981517 failed on its earlier stale Telnetd assertion; XY-C1 firmware validation #44927981534 remained in progress at the latest poll. Exact-head reruns for the current checker/workflow correction are pending.
- Checker/workflow/docs only; no production source changed. L3 remains unverified until XY-C1 build, TRX verification and artifact upload all pass on one exact product SHA; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Repair WAN and telnetd integration contracts

- Parent main SHA: 335fddd5b61d20e22987da948136c412cf4bbf81.
- Exact-head 285f WAN route #44927593606 failed because the inline Python checker invoked require() without defining an assertion helper. Process inventory #44927593544 failed at telnetd because its static contract still required the removed raw nvram_match("telnetd", "1") gate, while production already uses service_nvram_match_status().
- Added the local require(condition, message) helper to the WAN workflow inline checker and updated both duplicated telnetd source contracts to verify checked enable read, error propagation, disabled no-op, and ordering before optional daemon startup. No production behavior changed in this correction.
- On 335f, WAN gateway ARP, wireless radio, USB modem identity, ZeroTier, detect_link, router-mode NVRAM, firewall, VPN, detect_internet, watchdog, AI handoff and WAN interface teardown passed. Process, WAN DHCP, NVRAM audit and XY-C1 Build/TRX/artifact were still in progress at the last poll. Fresh exact-head WAN route and Process runs are pending.

---
## 2026-11-21 — Fail closed WAN gateway host-route cleanup on invalid snapshot values

- Parent main SHA: 285f00cc0b4918670afbeed98e6848de2c9b93d0.
- Exact-head AI handoff #44927593523 passed; WAN route #44927593606 and Process inventory #44927593544 were still running when this source slice began. On 387591, infosvr and crond passed while WAN route and Process inventory failed at checker source assertions; both checker defects were corrected on 285f.
- Review of wan_down_internal() found that successful NVRAM reads do not prove valid route inputs: an empty/malformed previous IPv4 address or a non-contiguous netmask could make is_same_subnet() return false and be mistaken for proof that the gateway /32 had been installed by WAN-up.
- Added explicit host-address and contiguous-netmask validation before subnet classification. Invalid values accumulate EINVAL, are logged, and cannot authorize gateway /32 deletion. The policy helper independently requires both validity flags; GNU89 regression cases reject invalid-address and invalid-netmask snapshots. Default-route and independent teardown paths remain outside this gate.
- Extended WAN route source contract to require validators before is_same_subnet(), and sanitizer-compiled policy tests to exercise malformed successful-read values. Exact-head WAN route, Process inventory and XY-C1 Build/TRX/artifact for the resulting SHA are pending.

---
## 2026-11-21 — Correct remaining exact-head checker mismatches

- Parent main SHA: 387591fe0a0bdc20507f8853e70d033ffa3661c4.
- Exact-head infosvr #44927382658, crond #44927382660 and AI handoff #44927382685 passed. WAN route #44927382675 failed at a checker assertion whose Python string represented backslash+n instead of the newline in the C definition. Process inventory #44927382674 failed because the checker passed a combined return-type/signature token, while the source definition has the return type and function name on separate lines; the function parser should key off restart_lltd(void) and restart_rstats(void).
- Corrected the WAN function-definition assertion with chr(10) and normalized the LLTD/rstats restart signature lookups. No production source changes in this correction. Exact-head checks and XY-C1 #44927382664 remain pending.

---

## 2026-11-21 — Align service and WAN teardown regressions with checked readers

- Parent main SHA: 43fb2c87c144efe369970cd3ad6e0b43a0ba174a.
- Exact-head Process inventory #44926550385 and WAN gateway ARP #44926550439 passed. WAN route #44926550424 failed at its old pointer-variable expectation after the WAN teardown checked-read refactor. Infosvr #44926550285 and crond #44926550347 expected raw nvram_invmatch gates, while their service start routines now use service_nvram_match_status(). Process integration #44926550298 expected process_presence_start_postcondition() to be returned directly, while production evaluates it as a predicate and returns zero in the success branch.
- Updated the four stale source contracts to test checked configuration-read/error/disabled branches and actual postcondition control flow. Updated the WAN route inline contract for checked gateway/address/netmask copies and helper-gated host-route deletion; added trigger paths and a GNU89 ASan/UBSan execution step for wan_teardown_policy.c. No production behavior changed in this checker-alignment slice.
- Exact-head validation for the resulting commit is pending; XY-C1 #44926550475 was still in progress at the parent SHA.

---

## 2026-11-21 — Preserve checked LLTD/rstats restart failures

- Parent main SHA: cb3a9460635a3754e3317d6340ac6b6749aef4ff.
- `start_lltd()` and `start_rstats()` now use `service_nvram_match_status()` and return checked configuration-read errors. Their restart wrappers nevertheless re-read the same enable keys through unchecked `nvram_match()`; if the startup/read had failed and the second read also failed, the wrapper converted the error to success.
- Simplified both restart wrappers to return the status from `start_lltd()`/`start_rstats()` after a successful stop. Disabled services already return zero from their checked start functions, so this preserves disabled behavior while preventing restart wrappers from swallowing read/launch/postcondition errors.
- Extended `check_process_lifecycle_integration.py` to assert both wrappers propagate start status and do not re-read the keys unchecked. Process integration run #44925798100 had previously passed all process/daemon source checks until a stale pre-refactor LLTD assertion; current checker now expects checked enable reads. Exact-head Process and XY-C1 validation pending.

---

## 2026-11-21 — Check WAN gateway/address/netmask reads during teardown

- Parent main SHA: 7f3a752aec75f9b59984bd83cb40d8cbf0cbc5af.
- `wan_down_internal()` previously used `get_wan_unit_value()` (backed by `nvram_safe_get()`) for the WAN gateway, address and netmask. A transient backend failure was indistinguishable from an empty value: teardown could skip default-route removal and conntrack cleanup without reporting it, or guess incorrectly whether to remove the off-subnet gateway /32 route.
- Replaced those reads with bounded `get_wan_unit_value_copy_status()` calls. The address is read once and reused for conntrack and the post-WAN hook; gateway/address/netmask read errors enter the first-error accumulator and are logged. Default-route removal proceeds when the checked gateway is valid; host-/32 removal requires all relevant reads to succeed and the original off-subnet condition; conntrack flush runs only for a successfully read valid IPv4 address, otherwise the hook receives the legacy `0.0.0.0` sentinel.
- Added GNU89 policy coverage proving host-route removal is denied on gateway/address/netmask read errors, with source-contract assertions for checked copies and error propagation. Exact-head WAN route/teardown and XY-C1 Build/TRX/artifact checks pending; L3/L4 not claimed.

---

## 2026-11-21 — Preserve checked service-enable errors through boot

- Parent main SHA: 38bc699b31a449f9708daf57e114de02560f17cc.
- Exact-head Process integration #44925798100 on the prior checker-alignment SHA failed at an LLTD source assertion which still expected raw nvram_invmatch. Inspecting the full boot call path found a production bypass: start_services_once() applied unchecked nvram_match outer gates for infosvr, LLTD, crond and rstats, so an NVRAM backend error could silently skip the checked service start routine and never reach boot status aggregation.
- Removed all four outer pre-gates. The service start routines already call service_nvram_match_status() and return its read error; missing or disabled flags remain successful no-ops, while backend errors now propagate through rc_first_service_start_status().
- Updated both duplicate process-integration source contracts to require the checked read/error/disabled behavior in all four service routines and ensure boot invokes each one without raw nvram_match/nvram_invmatch gates. New exact-head Process and XY-C1 validation is pending.

---

## 2026-11-21 — Fix XY-C1 linker failure in USB modem selection

- Parent main SHA: 432005050395db4136cd80407f37000c5dede114.
- XY-C1 build run #44925528552 on product source SHA `e1003f0cb6871aad9479a3757cb7c36368780578` compiled the RC objects but failed at final link with two undefined references to `modem_wan_select_device_status` from `net_wan.o`.
- The helper is a `static inline` implementation in `modem_wan_start_policy.h`, but `net_wan.c` called it without including that header. Added the missing include and extended the USB modem source contract to prevent a future translation-unit link regression. This is a compile/link fix; exact-head XY-C1 Build/TRX/artifact verification is pending.

---

## 2026-11-21 — Align gateway-ARP and process integration contracts

- Parent main SHA: e1003f0cb6871aad9479a3757cb7c36368780578.
- Exact-head WAN route #44925528581 failed in `check_wan_detect_wan_status.py` after its WAN DNS/ZeroTier/firewall checks passed: the VPN-notification ordering assertion still searched for the old gateway-ARP comment, which had been replaced by the checked-enable-read safety comment. Also simplified a dead conditional in the new gateway-ARP source assertion.
- Exact-head gateway-ARP socket workflow #44925528501 failed only because its source contract still required the removed unchecked `nvram_match("gw_arp_ping", "1")`; it now asserts the checked exact-string reader is present and raw matching is absent.
- Exact-head Process integration #44925528510 passed its prior compile/test/source contracts and then failed because it sliced WAN gateway startup using the removed old comment and next unrelated `if (wan_gate)` marker. It now slices from the new checked-read comment to the following `if (gateway_valid)` block boundary while retaining all process tri-state assertions.
- These are stale regression-contract updates only; exact-head WAN route, gateway-ARP socket, Process integration and XY-C1 reruns are pending.

---

## 2026-11-21 — Propagate gateway-ARP enable-state read failures

- Parent main SHA: dc9a86c9f74a6b327d215bfdb26ac63420060e59.
- The DHCP WAN-up gateway ARP checker still used unchecked nvram_match("gw_arp_ping", "1"). If the NVRAM backend failed, a requested optional checker was silently skipped as though disabled, and WAN-up could return success.
- The DHCP/no-modem branch now uses the checked exact-string reader already exercised by wan_nvram_status_policy tests. Missing or values other than exact "1" remain a disabled no-op; read errors enter the WAN-up first-error accumulator and are logged before skipping checker startup. Existing process tri-state inventory/start semantics are unchanged.
- Extended check_wan_detect_wan_status.py to assert checked-read → error aggregation/logging → enabled gate → process inventory order and reject the old raw matcher. Exact-head WAN route and XY-C1 Build/TRX/artifact verification pending; L3/L4 not claimed.

---

## 2026-11-21 — Update PPPoE-MWAN and detached-daemon source contracts

- Parent main SHA: 6f90222e0d35cb04bf640aef51957b02efa3cf60.
- WAN route run #44925135400 on 855d98387f10a802420d8da99d7e97d227970a97 passed firewall, DDNS, ZeroTier and other preceding checks, then failed because check_wan_detect_wan_status.py still searched for the removed raw PPPoE-MWAN nvram_match gate after the WAN post-down hook.
- XY-C1 run #44925135581 reached the PPPoE-MWAN test matrix (all shell-level PPPoE tests shown before the failing Python check passed) and failed because check_pppoe_mwan_status.py still expected the old unchecked gate. Both source contracts now verify checked exact-match read → error accumulation → confirmed-enabled launch ordering.
- Process run #44925135670 failed at the detached-start source contract because C rollback calls span multiple lines while the Python checker used whitespace-sensitive comparisons. It now normalizes the C function body before matching the exact calls. The production implementations are unchanged by this checker-only repair; exact-head reruns are pending.

---

## 2026-11-21 — Check infosvr/crond enable gates and crond log mode

- Parent main SHA: 855d98387f10a802420d8da99d7e97d227970a97.
- `start_infosvr()` and `start_crond()` still used unchecked exact-string `nvram_invmatch(..., "1")` startup gates. A transient NVRAM error was indistinguishable from disabled state and silently skipped the requested daemon. `start_crond()` also used an unchecked `crond_log` match to choose debug mode.
- Replaced the enable gates with the shared checked exact-string reader and propagate errors before daemon probing/launch. Crond's optional debug-mode read now uses the same checked reader; on failure it logs the configuration error and continues with the safe default logging mode so a diagnostic-only setting cannot block the scheduler.
- Added both daemons to the Process source contract and asserted checked logging-mode fallback. Existing matcher regression covers exact string, missing key, malformed output and backend-error behavior. Exact-head Process/XY-C1 verification pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Check rstats and LLTD enable reads before daemon startup

- Parent main SHA: f64ba04148274ce2433e4028eae50efd28b047fb.
- `start_rstats()` and `start_lltd()` still used unchecked `nvram_invmatch(..., "1")` gates. Backend/parse failures were indistinguishable from disabled state, so a requested daemon could be silently skipped with a successful return.
- Both now use the shared checked exact-string reader and propagate read errors before inventory probing or launching. Missing keys and values other than exact `"1"` retain the disabled no-op. Added Process source-contract assertions for checked-read → error gate → disabled gate ordering and rejection of unchecked NVRAM readers; existing matcher regressions cover backend failure and exact string semantics.
- Exact-head Process/XY-C1 verification pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Align WAN firewall/ZeroTier contracts with checked WAN status

- Parent main SHA: dd84e61cee454714642c3677e9223da732b3f2ec.
- Exact-head ZeroTier #44924793056 and WAN route #44924793115 failed only in stale source-contract assertions: the ZeroTier checker still searched for the removed unchecked PPPoE-MWAN `nvram_match()` gate, and the firewall checker still searched for the removed raw `strlen(nvram_safe_get("wanx_dns"))` condition.
- The current implementation correctly uses `get_nvram_match_checked_status()` to accumulate PPPoE-MWAN read failures before `detect.sh`, and uses a checked bounded MAN DNS snapshot plus `wan_teardown_should_clear_dns_snapshot()` to retain indexed WAN DNS on read error while continuing resolver refresh. Updated both contracts to assert read → error/status accumulation → conditional clear ordering and forbid the raw getter. Product source unchanged.
- Exact-head ZeroTier/WAN route reruns pending. XY-C1 validation status must be read for the resulting source SHA before claiming L3; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Point WAN DNS teardown assertions at the correct regression file

- Parent main SHA: 5ffef15005c54494712920ae2cf3983c648c6384.
- WAN route run #44924697344 on `cd33dbd6c9fa9797edb495f07cf40b7018099a67` passed all prior policy checks but failed at the new DNS teardown test-name assertions. The checker was looking for cases implemented in `tests/wan_teardown_policy.c` inside the unrelated `tests/wan_nvram_status_policy.c` file.
- Corrected the checker to read the dedicated WAN teardown policy test file. This fixes a test-source selection mistake only; production source/policy and the four DNS regression cases are unchanged. The same checker now also contains PPPoE-MWAN exact-match assertions for both WAN event handlers.

---

## 2026-11-21 — Check PPPoE-MWAN enable state in WAN-up/down callbacks

- Parent main SHA: 782adf90be019c2a9741a3e57cb981e4989207fc.
- Both `wan_up_internal()` and `wan_down_internal()` gated `detect.sh` with unchecked `nvram_match("pppoemwan_enable", "1")`. A backend read failure was indistinguishable from a disabled setting, silently skipping multi-WAN route reconciliation while the WAN event still reported success.
- Added a checked exact-string global NVRAM matcher that preserves missing-key false behavior and rejects backend errors. Both callbacks now accumulate/log the read error and launch `detect.sh` only after a confirmed exact `"1"`; the launcher status remains aggregated as before.
- Added strict exact-match/error policy tests and source-order assertions for both callbacks. Exact-head WAN-route and XY-C1 Build/TRX/artifact runs are pending; L3/L4 are not claimed.

---

## 2026-11-21 — Roll back ttyd, rstats, LLTD and SkipDBv2 after uncertain detached starts

- Parent main SHA: cd33dbd6c9fa9797edb495f07cf40b7018099a67.
- `ttyd`, `rstats`, `lld2d` and `skipd` daemonize before their startup wrappers can prove the child survived. Launcher failure, initial/retry wait failure and UNKNOWN process-inventory branches previously returned without stopping a possible child; final timeout likewise did not perform a post-stop rescan.
- Routed every such failure and the bounded startup timeout through the established `optional_daemon_rollback_failed_start()` helper. It performs daemon-name-specific stop, fresh process-inventory scan, conservative RUNNING/UNKNOWN reporting and preserves the original status/errno. SkipDBv2 passes its executable name `skipd`, not its UI service label.
- Extended the Process source regression to assert the generic rollback helper's stop/scan operations and every failure branch for each daemon. Existing ASAN/UBSAN optional-daemon policy tests cover conservative tri-state handling. Exact-head Process and XY-C1 validation pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Normalize the WAN DNS teardown checker token

- Parent main SHA: b1e1927079a6189fbf5cb95460a45b294b97b51c.
- WAN teardown regression #44924417107 passed, including the new GNU89 DNS-clear policy cases. WAN route #44924416973 then failed only in the new source contract: the checker removes all whitespace from the function body but expected the branch token `else if(clear_wan_dns)`, which normalizes to `elseif(clear_wan_dns)`.
- Corrected only that expected token. The production change and policy implementation are unchanged; exact-head WAN-route rerun is pending.

---

## 2026-11-21 — Preserve demand-PPP DNS when MAN DNS state is unreadable

- Parent main SHA: 9bdc5821ada33a0983a3c4c0c5fda1999e1df68f.
- `wan_down_internal()` decided whether to erase the indexed WAN DNS snapshot with unchecked `strlen(nvram_safe_get("wanx_dns"))`. A transient NVRAM read failure collapsed to an empty string, so PPP teardown could erase the demand-DNS snapshot even though the code could not establish whether MAN DNS remained configured.
- The teardown now reads `wanx_dns` through the checked bounded-copy helper. Static/DHCP IPoE retains its unconditional clear; other protocols clear only on a successful read proving the global MAN DNS value is longer than six bytes. Missing remains empty. Backend/truncation errors preserve the indexed snapshot, enter the first-error accumulator, and do not block independent cleanup.
- Added GNU89 policy cases for forced IPoE clear, present/short/empty MAN DNS, backend failure and invalid arguments; extended the WAN DNS caller contract. Exact-head WAN-route, WAN-teardown and XY-C1 Build/TRX/artifact checks are pending; L3/L4 are not claimed.

---

## 2026-11-21 — Fix optional-daemon source checker path NameError

- Parent main SHA: af276eab4c5af6750345da77274a2271d1910b5c.
- Process inventory/lifecycle exact-head run #44924180053 passed all preceding GNU89/ASan/UBSan, process inventory and service-policy steps, then the optional-app source contract raised `NameError: RC_ROOT is not defined` while trying to inspect `optional_daemon_policy.h`.
- Added an explicit `OPTIONAL_DAEMON_POLICY` path rooted at `ROOT` and used it in the assertion. This is checker-only; the production policy and test behavior are unchanged. Exact-head Process rerun pending.

---

## 2026-11-21 — Repair exact-head Process, ZeroTier and PID 1 regression contracts

- Parent main SHA: 206ee98f72b29555c325efb93f3287e8f6e530fd.
- Exact-head Process lifecycle #44923859668 reached the new optional-daemon policy compile and failed because `optional_daemon_policy.h` includes shared `process_inventory.h`; the test command had only added the RC include directory. Added `-I trunk/user/shared/src`.
- Exact-head ZeroTier #44923859847 passed shell lifecycle cases but its source checker still required the removed integer getter / old `!= 1` branch. It now verifies checked exact-string NVRAM status propagation, disabled no-op and launcher status. The PID 1 notification regression reused the same stale `!= 1` assumption in its cross-service checker; it now has an explicit checked-ZeroTier branch while preserving other mode checks.
- These are test/workflow-only corrections; no firmware source changed. Exact-head reruns pending. PID 1's log also contains intentionally failing gzip/mkdir fixtures whose assertions passed; its actual failure was the stale ZeroTier wrapper assertion, not the fixture diagnostics.

---

## 2026-11-21 — Roll back Telnet after uncertain detached startup

- Parent main SHA: d3fdae5b35e6d615515c7ea37f236390f0e97e26.
- `start_telnetd()` daemonizes via BusyBox `eval("telnetd")`, but launcher failure, wait interruption, retry wait failure or an UNKNOWN procfs inventory returned immediately. A child listener could therefore survive without the caller being able to verify its state.
- Added `telnetd_start_failure_recover()`: invoke the identity-checked `stop_telnetd()`, perform a fresh process-inventory scan, log RUNNING/UNKNOWN/invalid as a possible survivor, and restore the original failure status/errno. All four uncertain post-launch failure paths now share it; repeated, confirmed ABSENT still returns the existing postcondition/timeout result.
- Added a conservative tri-state policy helper plus GNU89/ASAN/UBSAN regression cases for ABSENT/RUNNING/UNKNOWN/invalid state. The Process workflow now compiles this policy test and its source checker verifies stop → rescan → survivor report → original status ordering. Exact-head Process/XY-C1 validation pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Remove the unchecked WAN-up ZeroTier pre-gate

- Parent main SHA: 5c2123ee1810790fb19e0b8a9a24f47ae2f8012f.
- Exact-head ZeroTier and WAN-route checks failed because WAN-up still wrapped the checked RC ZeroTier start function in an older unchecked `nvram_match("zerotier_enable", "1")` pre-gate. A read error was therefore swallowed as “disabled” before the new checked reader could propagate its status. The old source contract also still required the removed raw integer getter.
- WAN-up now always invokes `start_zerotier()` under `APP_ZEROTIER`; that wrapper owns a checked exact-string `"1"` read, preserves the legacy disabled no-op, propagates backend errors, and returns the launcher status. Updated the ZeroTier/WAN checker and optional-app checker; the latter also normalizes whitespace inside the AliyunDrive WebDAV service-name literal.
- Exact-head WAN/ZeroTier and Process regression reruns pending. No L3/L4 claim.

---

## 2026-11-21 — Propagate checked Telnet service-enable reads

- Parent main SHA: 4c79d11ce89248d828f8f4ae263fa918081c2797.
- `start_telnetd()` used an unchecked exact-string `nvram_match("telnetd", "1")`; read errors silently looked like a disabled service and returned success. Replaced it with the shared checked exact-match reader, preserving the original exact-`1` behavior and missing-key default while returning backend/parse errors before the process-inventory scan or launch.
- Extended the optional-app checker to enforce checked-read → error gate → enabled gate ordering and reject the raw matcher. Exact-head Process workflow and XY-C1 build/TRX/artifact validation pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Check optional add-on enable reads before daemon startup

- Parent main SHA: 1f6de563ff8ac2b748368bb96101e8759ec42e04.
- Seven optional RC startup wrappers still treated unchecked `nvram_get_int()` errors as a disabled setting: AdGuardHome, ZeroTier, DDNS.to, SQM, AliyunDrive WebDAV, WireGuard and Adbyby. This silently skipped requested service startup during transient backend/parse failures and made the integer getter's fallback indistinguishable from a readable disabled value.
- Replaced each wrapper's raw read with the existing checked `service_enabled_mode_status()` gate. Integer-returning wrappers propagate the read status; legacy void wrappers keep the running state untouched and rely on the shared reader's explicit error log. No shell launcher is invoked after an unsuccessful read. Extended the optional-app checker with source-order and unchecked-reader assertions for all seven wrappers.
- GNU89 compile and exact-head Process/XY-C1 workflows pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Check SmartDNS and AliDDNS enable reads before launch

- Parent main SHA: 6875d107ea47ab16ee9549ee780f878f29d50276.
- The RC wrappers for SmartDNS and AliDDNS still used unchecked `nvram_get_int()` reads for `sdns_enable` and `aliddns_enable`. A transient NVRAM backend/parse failure was indistinguishable from a readable disabled setting; SmartDNS reported a successful no-op and AliDDNS silently skipped startup.
- Both wrappers now use the existing `service_enabled_mode_status()` checked reader. SmartDNS returns the read error to its caller; the legacy void AliDDNS API logs through the shared reader and leaves the current daemon state untouched. No command launches when the setting is unreadable. Extended the optional-app source checker to enforce read → error gate → launch ordering and forbid unchecked getters.
- GNU89 firmware build and exact-head Process regression pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Correct HTTPD security policy-test source contract

- Parent main SHA: a655d440c8c77fd46160f81344ad5912b8082b4b.
- Exact-head HTTPD lifecycle and Process checks passed, but HTTPD Authentication/Security run #44922846367 failed in its duplicate source-contract script: the new listener-firewall selector test marker was checked against `tests/httpd_lifecycle_lock.c`, while the test is defined in `tests/httpd_daemon_policy.c`.
- The workflow now reads the policy test file separately and checks the marker there. No production code or test binary changed. HTTPD Authentication/Security exact-head rerun pending; XY-C1 run on older SHA 8b remains in progress and does not validate current main.
 
---

## 2026-11-21 — Correct SSHD rollback policy assertion to inspect test cases

- Parent main SHA: 8b0c30acf066c10beb620236210ae9cb5661047b.
- The new rollback policy intentionally encodes the conservative rule as “anything other than confirmed ABSENT”, not as an explicit UNKNOWN enum comparison. Updated the source checker to assert that implementation and inspect the regression function's ABSENT/RUNNING/UNKNOWN/invalid-state cases, instead of demanding an implementation spelling that was not part of the policy contract.
- Checker-only; no product source changed. Exact-head Process lifecycle and XY-C1 reruns pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Roll back SSH after an unconfirmed detached start

- Parent main SHA: 9d65ccd958bafeb9bd38820978ff38641721aa04.
- SSH/Dropbear detached startup returned immediately after launcher failure, initial/retry wait failure, or an UNKNOWN process inventory. In those cases the launcher could have forked a listener, or the process scan could not prove it absent; a direct start therefore could leave an untracked SSH listener.
- Added a shared failure recovery path that invokes the identity-bound SSH stop operation, performs a fresh process-inventory scan, reports RUNNING/UNKNOWN as a possible survivor, and restores the original launcher/wait/scan errno and status. Launcher, initial wait, retry wait and unknown inventory branches now use it; repeatedly confirmed ABSENT still ends with the existing timeout result.
- Added GNU89 tri-state regression coverage and exact source ordering assertions for stop → rescan → conservative survivor report → original error return. Exact-head Process/SSH and XY-C1 validation pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Fail closed on SSH public-firewall NVRAM errors

- Parent main SHA: d399f744701e24284cbc0ca92d4b314d44e20ae2.
- SSH restart used unchecked exact-string matches for sshd_wopen and fw_enable_x. A transient NVRAM failure was indistinguishable from a disabled public-access setting and could skip firewall reconciliation after the daemon state changed.
- Promoted the checked exact-string matcher to a shared service helper and use it for both HTTPD and SSHD. SSH restart preserves the previous short-circuit order (read firewall enable only when SSH public access is exactly "1"); any read/backend error is recorded as a firewall reconciliation failure and runs the existing identity-safe stop + fresh process-inventory postcondition. Missing keys and any value other than exact "1" still retain the legacy false result.
- Added GNU89 policy cases for exact/mismatched strings, absent-key defaults, inconsistent API output and backend-error preservation. Extended the SSH source checker and its path filters, and updated both HTTPD contracts for the shared helper name. Exact-head SSH/HTTPD/Process Actions pending; XY-C1 L3 remains NOT VERIFIED.

---

## 2026-11-21 — Restore HTTPD firewall preflight selection from the config snapshot

- Parent main SHA: 8ea482693c8c5de9fa4dfe8226d2f658372afe4d.
- Review of the live source found the previous firewall-enable check was syntactically present but its preflight selector had no body: both restart_fw_need assignments remained in the later listener-argv blocks. As a result, the pre-launch check always saw zero and the post-launch gate could skip firewall refresh even when HTTP/HTTPS public access was configured.
- Added httpd_start_config_firewall_required() to derive the required firewall work from the checked protocol snapshot before port normalization and launch. Removed the late flag mutations. HTTP/HTTPS protocol selection, separate listener access flags and irrelevant cross-protocol flags are covered by GNU89 regression cases; both HTTPD workflows now assert preflight derivation → checked enable read → first port write ordering.
- Exact-head HTTPD lifecycle/security and XY-C1 runs pending. This correction is a follow-up to the previous HTTPD firewall-read hardening; do not treat the older green lifecycle run as validation of this corrected ordering. L3/L4 remain NOT VERIFIED.

---

## 2026-11-21 — Fail closed on WAN VLAN and bridge setting read errors

- Parent main SHA: `956925458e646947d05930ab6ba00bf064f34482`.
- LAN/WAN bridge setup previously used unchecked `nvram_get_int("wan_stb_x")` and `nvram_get_int("wan_stb_iso")`. Backend/parse failures became disabled-bridge/no-isolation defaults, allowing topology changes inconsistent with saved configuration. WAN VLAN setup also reread bridge mode after earlier interface work.
- Added a checked integer-range policy that preserves backend/malformed-value errors while retaining disabled/no-isolation compatibility for readable out-of-range settings. LAN bridge setup, WAN bridge isolation and WAN VLAN setup now preflight and abort before interface probes or topology changes; WAN VLAN setup reuses one bridge-mode snapshot.
- Active additional WAN bridge ports now read each `vlan_vid_lanN` / `vlan_pri_lanN` pair through checked access before port probes or bridge mutations. The helper publishes both local outputs only after both reads succeed; priority masking and invalid-VID PVID fallback remain unchanged.
- Added GNU89 policy tests for boundaries, fallback, read-error/output preservation and invalid arguments, plus source-order contracts in `check_wan_vlan_setup.py`. Exact-head WAN route and XY-C1 Build/TRX/artifact checks are pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Fail closed on HTTPD firewall-enable read errors

- Parent main SHA: `130f1432d738f53ef4db77ed8f9219699714bcc2`.
- After checked listener settings were added, the post-launch firewall refresh decision still used unchecked `nvram_match("fw_enable_x", "1")`. A backend read error looked like a disabled firewall and could skip the firewall rebuild while leaving a listener exposed.
- Read `fw_enable_x` through `nvram_get_checked()` + the existing exact-string `firewall_nvram_match_status()` policy before any listener-port normalization or daemon launch, but only under the previous `restart_fw && restart_fw_need` short-circuit conditions. Missing remains false; present values still require exact `"1"`; backend errors abort startup. Replaced the post-launch raw match with an explicit policy gate.
- Added GNU89 tests for exact-string semantics, absent-key default, backend error/output preservation and all firewall-refresh gate combinations; both HTTPD lifecycle workflows assert the checked read precedes listener writes and the unchecked match is gone. Exact-head HTTPD security/lifecycle, Process and XY-C1 validation pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Update HTTPD retry rollback integration contract

- Parent main SHA: `955c3130f9d9aa3d750da39ef3080a728eaaa070`.
- The exact-head HTTPD daemon lifecycle run #44921809902 compiled and passed the GNU89/ASAN/UBSAN policy regression, then failed a stale source contract that still expected the initial wait-failure cleanup to be inlined inside `start_httpd_locked()`.
- The code intentionally centralizes initial and retry wait cleanup in `httpd_start_wait_failure_recover()`. Updated the contract to require the initial wait branch to call that helper and validate the complete stop/rescan/conservative-state-publication/errno sequence in the shared helper. The retry branch assertion remains in place. No product source or policy was changed.
- Exact-head rerun pending. HTTPD security, Process inventory, WAN route, networkmap, detect-link, AI handoff docs and several unrelated exact-head checks passed; XY-C1 remains pending build/TRX/artifact on this source SHA.

---

## 2026-11-21 — Roll back HTTPD when a delayed startup probe wait fails

- Parent main SHA: `4aead28cd9b3e2802922e4935bad22fbe742c3a9`.
- A retry probe wait could fail after `_eval()` had already launched HTTPD. Unlike the initial wait-failure path, the retry loop returned immediately, leaving a potentially active listener without a best-effort stop, authoritative process rescan or `httpd_started=1` recovery publication.
- Centralized startup wait-failure cleanup in `httpd_start_wait_failure_recover()` and route both initial and delayed retry wait errors through it. The helper stops HTTPD, rescans process inventory, conservatively republishes the started state for RUNNING/UNKNOWN, logs incomplete rollback, and restores the original wait errno/status.
- Extended exact source contracts for both wait sites and the rollback/rescan/state-publication ordering. Existing GNU89/ASAN/UBSAN policy tests cover ABSENT versus RUNNING/UNKNOWN rollback-state decisions. Exact-head HTTPD lifecycle/security, Process and XY-C1 validation pending; L3/L4 NOT VERIFIED.

---

## 2026-11-21 — Align HTTPD security contract with checked startup snapshot

- Parent main SHA: `6976792a61c28d782232e13a576e806e05b35731`.
- Exact-head HTTPD daemon lifecycle run #44921484050 passed, including the new GNU89/ASAN/UBSAN startup-settings snapshot regression and its source contract. The separate HTTPD security umbrella #44921484155 failed only because its duplicate lifecycle checker still expected the removed single-field `http_proto` read and the old insecure-fallback diagnostic.
- Updated that contract to require the checked adapter and all-or-nothing `httpd_start_config_read()` gate before listener selection, port writes, service-state publication and daemon launch. It rejects all four unchecked port/firewall-setting getters and pins the new policy regression; both new helper/test paths are now in the security workflow's push and PR path filters.
- No production code changed in this checker follow-up. Exact-head HTTPD security / Process / XY-C1 reruns are pending. XY-C1 run #44921484318 on `6976792a61c28d782232e13a576e806e05b35731` had not yet reached Build; L3 and L4 remain NOT VERIFIED.

---

## 2026-11-21 — Fail closed on HTTPD startup setting read errors

- Parent main SHA: `772916f1f377917ba9c6262b7b421f61dc79e6b6`.
- `start_httpd_locked()` already checked `http_proto`, but still consumed `http_lanport`, `misc_http_x`, `https_lport` and `https_wopen` through unchecked integer getters. A backend failure could overwrite a configured listener port with the legacy fallback or skip the firewall refresh needed for a public HTTP/HTTPS listener.
- Added an injectable all-or-nothing startup-settings snapshot using the existing checked NVRAM integer reader. Missing keys retain zero defaults and HTTP-only / HTTP / HTTPS / dual-listener selections read only their relevant values. All reads complete before port normalization writes, service-state publication or daemon launch; any read/parse failure aborts startup without changing the config snapshot.
- Added GNU89 policy regressions for protocol-specific read ordering, missing defaults, each of five injected read failures, output preservation and invalid arguments. Extended HTTPD lifecycle source contracts to require the checked preflight before writes/launch and forbid the four unchecked getters. Exact-head HTTPD / Process / XY-C1 validation pending; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Normalize multiline USB modem selector source assertion

- Parent main SHA: `af16c58c166b915e8847d1e30325ccea5bc78460`.
- The exact-head `usb-modem-identity` workflow (#44920895340, job `136896471678`) passed its GNU89/ASAN/UBSAN policy binaries, including `USB modem WAN startup status policy: PASS`, then failed in the Python source contract at the assertion for `get_nvram_int_checked_status("modem_prio", &modem_priority)`.
- The production call is intentionally split across lines; the checker expected one contiguous string. It now removes whitespace from the extracted selector before matching the complete checked-reader invocation. This is checker-only; no policy or product code was changed. Exact-head USB, WAN and Process reruns are pending; XY-C1 Build on the preceding product-source SHA was still running, so L3 remains NOT VERIFIED.

---

## 2026-11-21 — Fail closed on uncertain USB modem WAN selection

- Parent main SHA: `939e06459c5cb83b563758c30fc7371141e673f1`.
- `select_usb_modem_to_wan()` used unchecked `nvram_get_int("modem_prio")`; a transient NVRAM read/parse failure collapsed to zero and silently selected the wired WAN even when a ready USB modem was configured. For priority 2, a failed `get_wan_ether_link_direct()` probe was also forced to zero, which treated an unknown Ethernet state as confirmed link absence and could select the modem while the physical WAN state was unknown.
- Selection now reads `modem_prio` through the checked integer helper (missing key remains zero), checks `wan_src_phy` before PHY probing, retains PHY-probe errno, and applies a pure selection policy. Priority/read or link-probe errors abort before `set_usb_modem_dev_wan()`; priority 2 still prefers a confirmed wired link, uses the modem only after confirmed link absence, and retains APCLI/WISP behavior.
- Added GNU89/ASAN/UBSAN policy coverage for missing defaults, backend/link errors, priority branches, WISP, absent modem and invalid inputs, plus exact USB workflow source assertions. USB / WAN / Process / XY-C1 exact-head CI is pending; L3/L4 remain NOT VERIFIED.

---

## 2026-11-21 — Retry failed WAN DHCP renew signals after link restoration

- Parent main SHA: `b846d1035e0702815b360c1caa8c83a1b5a6980c`.
- `dl_handle_link_wan()` previously cleared its only DHCP-renew deadline before calling a void helper. If the DHCP pidfile was stale/missing or process inventory could not be read, `process_pidfile_signal()` failed and the caller discarded the result; no retry occurred.
- The helper now returns normalized errors and `detect_link` schedules two retries 4 seconds apart before logging exhaustion. Link-down/reset/success clear the retry state; intentional static-IP/USB/WISP skips remain success. Added a GNU89 ASAN/UBSAN policy test and exact detect_link workflow source/path coverage.
- L3/L4 remain NOT VERIFIED pending exact-head firmware gates and physical-router DHCP behavior.

---

## 2026-11-21 — Normalize WAN reconnect call token in source checker

- Parent main SHA: `9b7eecf72aa3cd7224d4597e8410d5555463bd8e`.
- Exact-head XY-C1, USB modem identity, Firewall and Process runs on `9b7eecf72aa3cd7224d4597e8410d5555463bd8e` reached `pause_reconnect.index(pause_start)` and failed: the test strips all whitespace from the extracted C body but the expected reconnect statement retained the space after `return`.
- Changed the expected token to `returntry_wan_reconnect(1,pause_seconds);`. This is checker-only; the ordered read, error gate, diagnostic, error return and reconnect operation remain required. XY-C1 build was not reached on the failing SHA; L3 remains NOT VERIFIED.

---

## 2026-11-21 — Normalize the delayed WAN reconnect return token

- Parent main SHA: 5f9aa9a3259def1f668e359fff1237aca3808466.
- The updated WAN source checker still failed on exact-head XY-C1/USB modem/Firewall checks at `pause_reconnect.index(pause_return, ...)`. `pause_reconnect` is constructed with `''.join(...split())`, which removes every whitespace character from the extracted C body, including whitespace inside string literals. The checker had normalized `pause_error` but left `pause_return` as `return status;`, so the expected token could never match the actual `returnstatus;`.
- Changed only the expected normalized token to `returnstatus;`. The production WAN reconnect logic is unchanged; the checker still asserts the complete ordered sequence: checked NVRAM read → error branch → diagnostic → status return → reconnect call, and still rejects the unchecked getter.
- On parent 5f9aa9a3, AI handoff documentation #44919803077 passed. XY-C1 #44919803037 was running; Firewall #44919803049 and USB modem #44919803091 failed at the same checker token, and Process #44919803014 was also running the same failing check. Fresh exact-head reruns are pending.

---
## 2026-11-21 — Normalize WAN reconnect diagnostic assertion

- Parent main SHA: `b40c3ec707896cd119d513775ef6d224049ea0d3`.
- The previous checker correction stripped all whitespace from the compacted `auto_wan_reconnect_pause()` body, but its expected diagnostic string still contained spaces. Exact-head XY-C1, USB modem, Firewall and Process jobs therefore failed on `pause_reconnect.index(pause_error)`; prior GNU89/ASAN/UBSAN policy binaries and earlier source assertions passed.
- The expected diagnostic fragment is now normalized with the same whitespace removal. This is test-only and retains the requirement that the checked NVRAM read, fail-closed return and reconnect call occur in order. The XY-C1 build had not started; L3 remains NOT VERIFIED pending Build, TRX and artifact on the same exact product-source SHA.

---

## 2026-11-21 — Correct whitespace normalization in WAN caller contract

- Parent main SHA: `1770074742e98f13155fab91703fc74ca975c248`.
- The exact-head USB modem identity and Firewall checks on `9341a1ce8e47c74787c4db5426ecdd49eab0392f` failed at a new source-order assertion because `compact()` converts whitespace runs to one space rather than removing whitespace. The Process lifecycle regression stopped at the same source checker. No C policy binary or source behavior failed before this assertion.
- The checker now removes whitespace from the compacted `auto_wan_reconnect_pause()` body before searching the ordered NVRAM-read, error-gate, early-return and reconnect fragments. The DNSMasq WINS fix was concurrently committed in `1770074742e98f13155fab91703fc74ca975c248`; exact-head DNSMasq workflow passed. This commit makes no production changes; affected XY-C1 / USB modem / Firewall / Process checks must rerun.
- L3 remains NOT VERIFIED until XY-C1 Build, TRX and artifact pass on one exact product-source SHA.

---

## 2026-11-21 — Fail closed on DNSMasq explicit DHCP WINS address reads

- Parent main SHA: 9341a1ce8e47c74787c4db5426ecdd49eab0392f.
- The DHCP WINS preflight checked `wins_enable`, but still used unchecked `nvram_safe_get("dhcp_wins_x")` twice. A NVRAM backend failure could look like an absent explicit address, causing the generated DHCP config to fall back to the router LAN IP when global WINS is enabled.
- Added a checked string-value policy and GNU89 regressions for present/missing/backend/inconsistent/oversized values, with output preservation on every failure. `prepare_dns_dhcpd()` now snapshots `dhcp_wins_x` before DHCP-pool repair or generated-file changes, fails on read errors, and uses that same bounded snapshot for both precedence and emitted option 44; valid explicit WINS addresses still bypass the global WINS flag.
- Added source-contract checks for checked-read ordering, last-known-good behavior and removal of the second unchecked getter. Exact-head DNSMasq/XY-C1 tests are pending; no firmware build or target-router DHCP behavior is claimed yet.

---
## 2026-11-21 — Align XY-C1 WAN caller contract with checked delay read

- Parent main SHA: `0097f377a1155be4ec09c2fe38334aa985c39b2b`.
- The first XY-C1 validation of the deferred reconnect source change stopped before firmware compilation at an older `check_modem_wan_start_status.py` assertion that still expected `nvram_get_int("di_recon_pause")`. The production function correctly uses the checked integer reader introduced in `136db90e2f84513d9bf3da297802b708b0811b44`.
- The checker now asserts the checked read, fail-closed return before `try_wan_reconnect()`, and absence of the legacy unchecked getter. The XY-C1 workflow already includes and executes this checker; this is a test-only correction.
- The XY-C1 Build/TRX/artifact gate for the product-source SHA must be rerun to establish L3. The WAN-route checker correction is on main and its exact-head rerun is queued.

---

## 2026-11-21 — Normalize deferred WAN reconnect source-check formatting

- Parent main SHA: `136db90e2f84513d9bf3da297802b708b0811b44`.
- The first exact-head WAN route run for `136db90e2f84513d9bf3da297802b708b0811b44` passed all GNU89/ASan/UBSan policy tests and earlier WAN source contracts, then failed on a source checker assertion that compared a multiline C helper invocation as one unbroken string.
- The checker now normalizes whitespace before matching the checked-reader call. No production source or test semantics changed; the WAN route workflow will rerun on this checker-only commit. The original source SHA's XY-C1 build is still the product-source validation target; L3 awaits its Build, TRX and artifact results.

---

## 2026-11-21 — Abort deferred WAN reconnect when delay cannot be read

- Parent main SHA: `8c79215790b8e56300d0c07201d62c90dfcd76e2`.
- `auto_wan_reconnect_pause()` previously used unchecked `nvram_get_int("di_recon_pause")`. A transient NVRAM read or malformed-value failure became zero, bypassing the configured delay and immediately invoking `try_wan_reconnect()`, which can stop and restart the live WAN path.
- The wrapper now uses the shared checked integer reader before logging/dispatching the reconnect. Backend and parse errors return before any WAN stop/start; a genuinely absent key keeps the legacy zero-second default. The existing GNU89/ASan/UBSan NVRAM policy regression already covers absent defaults, malformed values and backend failures; the deferred-watchdog source contract now pins the caller ordering and forbids the unchecked read.
- Exact-head WAN route and XY-C1 Build/TRX/artifact checks are pending. Physical router reconnect timing remains L4 NOT VERIFIED.

---

## 2026-11-21 — Fail closed on FTP enable-state read errors

- Parent main SHA: `ae4ae4a57b2c34d6dccf2541161a51606170047c`.
- `run_ftp()` previously relied on `nvram_match("enable_ftp", "0")`; a failed NVRAM backend read could be confused with enabled state and continue to process inspection, configuration publication and daemon launch. It now uses `nvram_get_checked()`, the shared strict integer parser and explicit 0/1 validation. Missing-key default remains enabled for compatibility.
- `restart_ftpd()` performs the same checked preflight while holding the storage-operation lock and before process inspection or daemon stop. An unreadable flag preserves the current daemon state.
- Added GNU89 policy cases for backend errors, negative parser status, malformed boolean, both valid values and NULL output; the vsftpd contract verifies startup and restart ordering.
- The complete repository tree was restored by recovery commit `954d1dc4d11e5518e6e69e86a167fcfd0e5efd33` after a transient incomplete tree from a concurrent session. No history rewrite/force reset was used. This production-source commit requires exact-SHA vsftpd and XY-C1 Build/TRX/artifact checks before any L3 claim; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Cover all captured-mode restart wrappers in storage contract

- Parent main SHA: 985e86b3d85cfc3abf99ec5c13d5988eb3acd866.
- The next USB storage hotplug rerun passed the SCUT/MentoHUST checker branch but exposed the same stale inline-declaration assumption at `ttyd`. Inspection confirmed `ttyd`, `ss`, `ss_tunnel`, `vlmcsd` and `iperf3` also preflight their exact NVRAM key, preserve the captured mode, stop with a checked status and call `start_*_with_mode()` only after stop succeeds.
- Generalized the checker over all seven captured-mode wrappers with per-service key, display name and mode variable assertions. Kept the stricter inline `int stop_status = stop_*()` check for `sqm`, `aldriver` and `wireguard`, which use that actual pattern. Production code unchanged; no test relaxed.
- Fresh USB storage hotplug and exact-head Actions pending.

---
## 2026-11-21 — Preserve integer WINS flag semantics in DNSMasq checked reads

- Parent main SHA: a27d1c9b91e31f173bcf36860029a3e16a77af77.
- Review after the DHCP WINS fail-closed patch found a compatibility mismatch: the former call was `nvram_get_int("wins_enable")`, which treats any nonzero integer as enabled; the generic optional-option policy intentionally preserves `nvram_match(...,"1")` and would treat `2` as disabled.
- Added a distinct checked integer-boolean parser using the shared complete-decimal NVRAM parser. It preserves zero/nonzero integer semantics and the absent-key default 0 while distinguishing backend failures and rejecting malformed/overflow values. The DHCP WINS preflight now uses it; ordinary resolver flags continue exact `nvram_match("1")` semantics.
- Added GNU89 regression cases for `2`, `-1`, `0`, malformed suffixes, overflow, missing key and backend failure, plus source contracts. Exact-head DNSMasq, Samba and XY-C1 runs pending.

---
## 2026-11-21 — Correct storage hotplug checker for checked-mode restart wrappers

- Parent main SHA: bd0a0252aacbcf63ee0525d017236c23a4b52aa7.
- The storage hotplug workflow repeatedly failed at `scripts/check_storage_safe_remove.py` line 740 for `scutclient`. Production `restart_scutclient()` (and `restart_mentohust()`) deliberately declares `int config_status, stop_status;`, reads the configured enable mode before stopping, propagates config/stop errors, and passes that captured mode to `start_*_with_mode(enabled)`. The checker incorrectly required an inline `int stop_status = stop_*();`, so it rejected the intended safer wrapper shape.
- Updated the contract to validate the exact preflight mode key and display name, require config validation before stop, require nonzero stop propagation before launch, and preserve the wrapper's captured-mode startup. Other wrappers retain their existing declaration/stop/start checks. No production behavior or test requirement was weakened.
- This assertion had failed in runs #44917653988, #44917367023 and #44916836630; the first two independent binaries/most other storage checks passed. Fresh exact-head storage workflow is pending.

---
## 2026-11-21 — Check DNSMasq WINS advertisement before preparing DHCP configuration

- Parent main SHA: e9ce710c6acc5138400faf6238d85fcd0197ce23.
- prepare_dns_dhcpd() previously used nvram_get_int("wins_enable") only when no explicit DHCP WINS address was configured. A backend read failure was indistinguishable from disabled, silently removing DHCP option 44 while the rest of the new config was still published.
- Reused the existing checked dnsmasq_read_boolean_option() / dnsmasq_nvram_option_status() contract. When DHCP is enabled and dhcp_wins_x is not a valid explicit address, wins_enable is now read before DHCP pool correction, generated host/ARP updates, resolver creation or staged config open. Backend/inconsistent results abort preparation; a genuinely missing flag retains the legacy default 0. An explicit valid WINS address still bypasses the optional flag read and keeps precedence.
- The DNSMasq NVRAM policy tests already cover missing-key defaults, malformed/present values, backend errors and output preservation. Added workflow source contracts for preflight ordering and removal of the unchecked getter. Exact-head DNSMasq/Samba/XY-C1 checks are pending; L3 requires Build + TRX + artifact on one SHA.

---
## 2026-11-21 — Restore the complete repository tree after Git tree assembly error

- The Samba WINS header source slice was first committed as b53d27269e6de0a3b56f51e044714f7a4fa91dd1. That tree accidentally omitted the base tree SHA when calling the Git tree API and therefore contained only the five updated paths; the exact-head Samba job #44917536389 correctly failed because tests/samba_config_file.c was absent.
- Rebuilt a complete tree from the actual parent Git tree 12f4a8fe707916dc3fb2550850642b1c9c3b5dae and made a normal non-forced child commit 954d1dc4d11e5518e6e69e86a167fcfd0e5efd33; no history was rewritten. Verified README.md, the restored Samba test source, the firmware build workflow, services_stor.c and services_ex.c are readable on main.
- The recovery commit queued the full exact-head workflow suite, including XY-C1 #44917653757. The source change remains intentionally unclaimed at L2/L3 until the Samba regression recompiles/runs against the restored tree and the same-SHA XY-C1 Build/TRX/artifact gates pass.
- Added one more Samba source-contract assertion to prove the WINS NVRAM error is propagated before the first global smb.conf output. This change triggers a fresh Samba run on the complete tree.

---
## 2026-11-21 — Fail closed on Samba WINS state before smb.conf header publication

- Parent main SHA: bd63d60932e5c1d7a90cbe9a19a3d8a0c72c153c.
- Exact-head Samba regression #44917367040 passed both the GNU89 policy/config test step and the complete source-contract step. AI handoff documentation #44917367092 also passed on this parent; XY-C1 #44917367085 was still running at the last poll.
- Follow-up source inspection found write_smb_conf_header() still used unchecked nvram_get_int("wins_enable"). A NVRAM backend failure could silently omit WINS support from the generated global config. It now uses the checked boolean adapter with the legacy absent-key default 0 and returns before the first fprintf on failure.
- Added a source-contract requirement for the checked call, its declaration before the caller, read-before-first-output ordering and removal of the unchecked read. The existing GNU89 test binaries remain unchanged; the new exact-head Samba and XY-C1 gates are pending.

---
## 2026-11-21 — Align Samba restart and optional-daemon mode source contracts

- Parent main SHA: `b0a6a07f6bbb533867a300603d1c1a379e2ea4be`.
- Samba #44917175074 passed its earlier NVRAM reader and WINS checks, then failed at a legacy source assertion expecting `restart_smbd()` to recompute `samba_enabled` with unchecked `nvram_match()`. The runtime flag is now read once, checked and used before any stop; updated the stale assertion to require that safe path.
- USB storage hotplug #44916836630 failed its generic manual restart check for `scutclient`: production `restart_scutclient()` correctly checks mode, propagates a failed stop, then calls `start_scutclient_with_mode(enabled)` to avoid a second unchecked read. The contract now recognizes the mode-preserving helper for both SCUT and MentoHUST and explicitly checks stop-failure ordering.
- Product-source XY-C1 #44916836560: `in_progress/pending`; NVRAM audit #44916836473: `completed/success`; AI docs #44917017123: `completed/success`. No L3 claim until same-SHA build, TRX and artifact succeed.

---

## 2026-11-21 — Normalize multiline WINS flag call contracts

- Parent main SHA: `90de7e3f2ea3d246738ccabc9148810c1104b45d`.
- Samba run #44917017193 reached source-contract line 88 after the policy unit binary passed. It failed because production `start_wins()` and non-forced stop split checked WINS helper arguments across lines; fixed both call-presence/order assertions by normalizing function-body whitespace. Behaviour assertions remain intact.
- Parent product-source XY-C1 #44916836560 status: `in_progress/pending`; NVRAM audit #44916836473: `completed/success`. XY-C1 had reached the Build XY-C1 step; TRX and artifact gates had not yet completed.
- AI handoff #44917017123 was in progress on parent `90de7e3`; exact-head Samba and docs reruns follow THIS COMMIT. Production source SHA remains `bc3cceeb`; no L3 claim until Build/TRX/artifact all pass on that exact SHA.

---

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

