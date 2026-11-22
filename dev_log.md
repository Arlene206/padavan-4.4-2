## 2026-11-22 10:16 CST — Propagate OpenVPN certificate generation failures

- Parent main SHA: `3118578664b49741a149c40fdd509b9a0bd807ef`. Audit found `make_cert()` ignored EC/RSA/Ed keygen and CSR return codes, `sign_cert()` removed the CSR even when signing failed and returned success, `make_dh()` ignored DH generation errors, and server/client wrappers continued after a failed prerequisite.
- Production script now checks OpenSSL keygen/CSR/sign/DH command results and non-empty outputs, removes incomplete newly generated key/DH outputs, protects private keys before request generation, propagates chmod errors, stops server/client workflows at the first failure, and preserves the CSR when signing fails.
- The mocked crypto regression now injects partial files plus failure/empty-output paths across RSA/EC/Ed keygen, CSR, signer, DH and chmod, and verifies client orchestration stops. Existing TLS-key permission repair and TLS-crypt-v2 error checks remain.
- Exact OpenVPN private-key workflow is triggered by the changed script/test paths; results pending. No local test run or XY-C1 result is claimed.

---

## 2026-11-22 — Reconcile HTTPD firewall validator source contract

- HTTPD #44974691100 on source SHA `6ca55ecdaca95858aadbf77c4fd1af49b0e1644c` failed because the workflow hard-coded ten occurrences of `firewall_ipv4_address_is_safe()`, while the committed production source has nine. This was a stale checker expectation, not a C source or regression-test failure.
- Checker-only commit `544ef61b244c13b8a2220f3e21d91a257f065fd3` changes the brittle exact count to a minimum floor and directly asserts the default NAT builder validates its copied LAN IP and contains no unchecked NVRAM reads. Follow-up `a05cb570eea8deb76d67fece4994ee415924c67e` replaces escaped-newline function markers with stable source slicing.
- Exact rerun HTTPD #44974951724 PASS and AI docs #44974951739 PASS on `a05cb57`; no production HTTPD code changed.

---

## 2026-11-22 — Transactional LAN DHCP bound-state snapshot

- Parent main SHA: `6d03a4b1d63ab5cf058e8c270a76c50740de6c2b`; production source commit: `6ca55ecdaca95858aadbf77c4fd1af49b0e1644c`.
- `udhcpc_lan_bound()` previously read the seven old DHCP tuple fields individually with `nvram_safe_get()`. A transient NVRAM read/close error was indistinguishable from a missing/empty key, so rollback and changed-field detection could consume an incomplete previous tuple. The no-`subnet` path also read the lease netmask and configured fallback separately without status.
- Added one transaction snapshot for `lan_ipaddr_t`, `lan_netmask_t`, `lan_gateway_t`, `lan_dns_t`, `lan_domain_t`, `lan_wins_t`, `lan_lease_t` and `lan_netmask`. The callback rejects the event on any backend, close or destination-capacity error before kernel, file or NVRAM publication. Both rollback comparisons and netmask fallback use the copied snapshot.
- Updated DNSMasq source contract to pin the eight-key list, begin/get/end transaction callbacks, fail-closed ordering and removal of unchecked getters. DNSMasq #44974691080 PASS, WAN DHCP callback #44974691075 PASS and NVRAM transaction boundary #44974691126 PASS on this exact source SHA. XY-C1 #44974691162 was still building; TRX and artifact upload pending.

---

## 2026-11-22 10:07 CST — Align HTTPD firewall boot contract and correct L3 checkpoint

- Parent main SHA: `ac8436355e5081e81737ffa758bd78fc2d73112e`; firewall product-source SHA: `e9ef41bd5a103b1693d5f0044d3ec5e5532c1661`.
- Exact HTTPD auth/security run #44974210716 failed at the embedded firewall boot assertion still requiring the removed unchecked `nvram_match("wan_nat_x", "1")` assignment. The workflow now verifies both checked boot-enable readers, read-before-NAT-restore order, the unreadable-selector regression, and checked WAN-start policy order.
- Firewall Startup #44974378538 PASS on checker-only descendant `ef20a36`; NVRAM #44974210645, Process inventory #44974210694, AI docs #44974210724 and WAN route #44974210718 PASS on product SHA `e9ef41`. AI handoff #44974465784 PASS on documentation checkpoint `ac843635`.
- Important L3 correction: XY-C1 #44974378528 on `ef20a36` completed SUCCESS but Build/TRX/artifact were SKIPPED because that push changed only the checker. Product-source run #44974210522 on `e9ef41` remains IN_PROGRESS at Build XY-C1; TRX/artifact are not yet verified.
- No local checkout/toolchain; local tests were NOT RUN. No L3/L4 claim. THIS commit atomically updates HTTPD workflow contract, handoff, roadmap and this log; exact HTTPD/AI reruns remain pending.

---

## 2026-11-22 — Firewall boot enable-state hardening and exact-head CI reconciliation

- Production source slice: `e9ef41bd5a103b1693d5f0044d3ec5e5532c1661`. PID 1 no longer maps failed reads of `fw_enable_x` or `wan_nat_x` to false; checked status gates `start_wan()`. `ipt_nat_default()` now preflights all applicable NVRAM inputs before creating the restore file. Added GNU89 policy regressions and source-order assertions.
- Initial Firewall Startup run #44974210810 exposed a stale caller signature in `scripts/check_forwarding_control_status.py`; all preceding firewall policy/NVRAM and forwarding regression binaries passed. Updated the checker in `ef20a36bf970ec3c3c487cb71c2e0bdc615bf073` without changing production code.
- Exact descendant Firewall Startup rerun #44974378538 PASS on `ef20a36`. PID 1 notifications #44974210641, Process inventory #44974210706, AI docs #44974210724, WAN DHCP callback #44974210669 and infosvr lifecycle #44974210764 PASS on production source `e9ef41`.
- XY-C1 validation #44974378528 is tied to exact code SHA `ef20a36` and remained IN_PROGRESS at this checkpoint. No L3 Build/TRX/artifact completion or hardware L4 result is claimed.

---

## 2026-11-22 — Fix missing PID 1 main-loop binding in retry workflow contract

- Parent main SHA: `3e99d89dd56e70d1c59230d42d33da166b9d2cc2`.
- PID 1 notification #44973128571 compiled and executed every GNU89/ASan/UBSan regression successfully, including the newly registered deferred hotplug marker mapping and retry lifecycle tests. Its final Python source contract failed with `NameError: main_loop is not defined` because the checker used that local in the new SIGALRM ordering assertion without first extracting `init_main_loop()`.
- Added the missing `main_loop = function_body(init, "init_main_loop(void)")` binding. The new assertion continues to require queue marker retry before deferred-device dispatch inside the PID 1 alarm path.
- Exact parent-SHA HTTPD auth/security #44973128500 PASS, HTTPD daemon lifecycle #44973128575 PASS, PID 1 mdev security #44973128472 PASS, AI handoff #44973128460 PASS, DNS Forwarder #44973128486 PASS, DNSMasq #44973128493 PASS and Process inventory #44973128482 PASS. USB storage #44973128576 and XY-C1 #44973128574 were still running at last poll.
- THIS COMMIT atomically updates the workflow contract, handoff and log; fresh exact-SHA CI and target Build/TRX/artifact results remain required.

---

## 2026-11-22 — Register deferred USB retry regressions and align exact-head CI contracts

- Parent main SHA: `2f93266c59b9a10fec33589ada3e2e9c7b803b84`.
- PID 1 notification #44972905893 failed because the regression functions had not been included/registered in `rc_notify_batch_policy.c`. This commit adds both GNU89 regression bodies before `main()` and executes them: fixed event mapping and rejection of unknown names; marker-write error accumulation; and retry-bit retention/clearing based on queue wakeup completion.
- HTTPD auth/security #44972905619 exposed a legacy source-shape contract in `init.c`. Retry remains conditional on `router_ready` and precedes deferred USB handling, but the existing standalone `if (router_ready) on_deferred_hotplug_dev();` is preserved.
- HTTPD daemon lifecycle #44972905858 had a stale assertion demanding an explicit `presence == PROCESS_PRESENCE_UNKNOWN` branch. Production delegates that check to `httpd_start_postcondition_requires_recovery(presence)`; the workflow now asserts the delegation and the helper's non-ABSENT condition.
- USB storage #44972905435 and XY-C1 #44972905721 were in progress at the prior poll. No same-SHA target build/TRX/artifact result or hardware verification is claimed.
- THIS COMMIT atomically updates the regression, init source shape, HTTPD daemon source contract, handoff and historic dev log.

---

## 2026-11-22 — Retry failed deferred USB event marker writes

- Parent main SHA: `e26890edfef8dd47aa1bbb1d514a315933de2571`.
- `rc.c::handle_notifications()` removed queued RC events before writing temporary NVRAM markers for printer add/remove, modem add/remove and mass-storage add. The ignored setter result could discard the only wakeup when NVRAM write failed; the existing alarm did not recreate the notification.
- Added a fixed-event retry mask and queue recovery worker. Marker failures now propagate to `action_result`, preserving the incomplete marker. PID 1 retries safe queue marker creation on SIGALRM and sends SIGUSR1; retry bits clear only after queue marker creation and wakeup succeed.
- Added GNU89 policy tests for all five mappings, unknown names, failure recording and wakeup completion. Extended the RC notification source contract.
- Corrected `check_storage_safe_remove.py`: it incorrectly prohibited the single initial `storage_should_start = 0` output initializer. The check now requires exactly one initializer before checked marker reads and dispatch.
- Parent exact-SHA Actions: USB modem identity #44972041142 PASS; PID 1 notification #44972041292 PASS; USB storage #44972041244 failed only on the stale checker assertion; AI handoff #44972041355 PASS; XY-C1 #44972041318 still IN_PROGRESS. Fresh source-SHA tests/build/TRX/artifact are required; no target or hardware pass is claimed.
- THIS COMMIT atomically includes RC source/API, policy and regression, notification contracts, storage checker correction, handoff and log.

---

## 2026-11-22 — Gate modem link, module restart and WAN LED decisions on checked identity

- Parent main SHA: `4949f8f41ccd23df2e657477ede2e667410f2e19`.
- `usb_modem.c::notify_modem_on_wan_ether_link_changed()` previously used the legacy boolean selector and could queue an automatic reconnect from ambiguous state. It now uses `get_usb_modem_wan_status()`, logs and returns on failure. The disabled legacy internet-state helper was migrated as well.
- `rc.c::RCN_RESTART_MODEM` previously used the conservative legacy boolean as a destructive restart decision. It now gates WAN stop, modem module load/unload/reload, persistent runtime rule/type checkpoints and full WAN restart on a successful identity read, preserving a read error in the RC action result.
- Both WAN LED paths in `detect_link.c` now use the checked selector plus `usb_modem_wan_legacy_boolean()`; errors conservatively select the modem display branch. The USB identity workflow reads these sources, asserts their ordering/gates, and includes `detect_link.c` symmetrically in push/pull_request triggers and its inline source allowlist.
- Added a pure regression for the modem restart identity gate to the existing GNU89/ASan/UBSan policy test.
- THIS COMMIT atomically includes three source migrations, policy/test, workflow contracts and handoff/log. Current exact-head CI must confirm the full regression suite and XY-C1 target stages; no local target or hardware result is claimed.
- Next: reconcile exact-head CI, then select a distinct whole-userland P1/P2 stability/security defect from the roadmap ledger.

---

## 2026-11-22 — Align unplug identity source contract with conservative cleanup

- Parent main SHA: `be7b6c73d92af9de68a365156d69ad84e1eb81ab`.
- Exact USB modem identity run #44971644620 failed at the pre-existing comment assertion `Always notify the WAN owner`. The new production behavior intentionally notifies only for known modem identity or an unreadable identity; a known non-modem unplug is not a WAN modem event.
- Updated the contract to require the accurate `Preserve mandatory WAN cleanup on physical unplug` statement. This is a checker-only correction; no production behavior changed.
- All eight preceding GNU89/ASan/UBSan compile-and-run regression steps passed on behavior SHA `ff6e5b90051317d81b14427829ccc1999c2f572b`, including the added unknown-identity policy tests. AI handoff docs, HTTPD auth/security and Process lifecycle inventory also passed on that SHA; XY-C1 was still in progress at last poll.
- THIS COMMIT updates the workflow assertion, handoff and log atomically. Fresh exact-head USB identity/AI docs and target Build/TRX/artifact results remain required.
- Next: confirm current-head CI, then migrate the remaining selectors in `usb_modem.c`, `rc.c` and `detect_link.c`.

---

## 2026-11-22 — Include modem WAN start policy in USB service translation unit

- Parent main SHA: `ff6e5b90051317d81b14427829ccc1999c2f572b`.
- Post-commit source integration review found that `services_usb.c` called the newly added `modem_wan_auto_start_allowed()` static-inline policy without including its defining header. Added the explicit `modem_wan_start_policy.h` include to prevent an implicit declaration/unresolved external at compile/link time.
- This is a build-integration correction to the preceding modem identity source slice; no behavior or configuration format changed.
- Exact Actions for preceding SHA `ff6e5b90051317d81b14427829ccc1999c2f572b` were still in progress at review: AI docs #44971644605, XY-C1 #44971644588, NVRAM audit #44971644611, Process inventory #44971644652, USB storage #44971644630, HTTPD #44971644660, process fail-closed #44971644640 and USB identity #44971644620. The new SHA must independently pass.
- THIS COMMIT changes the include and updates the handoff/log as one atomic source-integration correction. No local target build or hardware result is claimed.
- Next: verify exact-head checks, then migrate the remaining checked modem selectors in `usb_modem.c`, `rc.c` and `detect_link.c`.

---

## 2026-11-22 — Preserve modem unplug cleanup and reject ambiguous auto-start

- Parent main SHA: `6444ce9468a81e8f52b7636990b93ba994b7235a`.
- Exact parent-head Actions: USB modem identity #44971100959 failed only in the final source contract because it searched for the checked modem identity call on one line; production wraps the call across two lines. AI handoff #44971100969, WAN route #44971100970 and Process lifecycle #44971100963 passed. The eight preceding USB host compile/regression steps passed.
- `hotplug_usb.c`: both NDIS and TTY physical-unplug handlers now use `get_usb_modem_wan_status()`. A confirmed modem or an unreadable identity queues `on_unplug_usb_modem`; known non-modem state does not. This preserves mandatory WAN teardown when NVRAM identity reads fail.
- `services_usb.c`: automatic modem selection now requires a successful identity read proving that the current WAN is not already a modem. Read errors are logged and returned before any reconnect request.
- Added policy regressions for unknown-identity unplug cleanup and auto-start rejection, and source contracts for both physical unplug paths and the modem start gate. Repaired the multiline reconnect contract in the existing USB identity workflow. Existing GNU89/ASan/UBSan test integration is retained.
- THIS COMMIT includes source, tests, workflow contract, handoff and this log entry atomically. Exact-head Actions and XY-C1 target Build/TRX/artifact are required; no local target build or hardware result is claimed.
- Next: verify exact-head Actions, then migrate the remaining selectors in `usb_modem.c`, `rc.c` and `detect_link.c`.

---

## 2026-11-22 — Correct exact-SHA WAN route, modem identity and process lifecycle contracts

- Parent main SHA: `c4a27498de42e68bf21a358e4938008ff43ba0e7`. AI handoff #44970629909 PASS, Wireless #44970629880 PASS, WAN DHCP #44970629930 PASS, NVRAM transaction #44970629921 PASS and process lifecycle inventory scan #44970629864 PASS. XY-C1 #44970629875 remained IN_PROGRESS at last poll.
- WAN route #44970629902 failed in its resolver source contract because it searched for the deleted comment `/* force notify dnsmasq for modem */`. The production code now checks the modem identity status and forces DNSMasq reload on unknown identity. Updated the source assertion to bind to that behavior.
- USB identity #44970629920 failed because the checker body helper matched a forward declaration for `get_wan_dns_static_status()`; it therefore extracted the wrong subsequent brace body. Anchored DNS, WAN interface, WAN IP and reconnect checks to actual function definitions.
- Process lifecycle #44970629903 failed because its script expected a stale compound condition. Production currently aggregates protocol, interface and callback failures through `wan_teardown_first_error_status()`, then gates final WAN state clear; updated the assertion to check that actual ordering and argument set.
- This commit is checker/handoff/log only. No production source changes or firmware-build result are claimed.
- Next production slice: migrate RC-wide legacy USB modem selector consumers, with special care not to skip physical-unplug cleanup when the selector read itself fails.

---

## 2026-11-21 — Fail closed in legacy USB modem identity wrapper

- Parent main SHA: `95bfb65ed32a6c507fb93bd442086496fccacb9d`.
- Changed `get_usb_modem_wan()` compatibility behavior so failed `get_usb_modem_wan_status()` reads map to boolean true (conservative modem classification), not false. This prevents legacy callers from misclassifying an unknown WAN as non-modem while they are migrated to explicit status handling.
- Added pure policy regression for absent/present identities and EIO/ENOSPC read failures; extended USB modem workflow source assertions to forbid the old zero-on-error wrapper.
- Exact-head USB modem parser and XY-C1 firmware checks are pending for this source change.

---

## 2026-11-22 — Remove final lossy modem selectors from WAN resolver and address paths

- Parent main SHA: 2ea191e751da5bbf4c6cdfb76a76e7324314168a. On exact source SHA 2ea191e751da5bbf4c6cdfb76a76e7324314168a, AI handoff #44970372450 passed. WAN route #44970372464 failed in scripts/check_wan_dns_read_status.py line 334 due a duplicated print() statement; all compiled route/policy steps 3–10 passed. WAN DHCP #44970372463 failed at embedded Python line 207 due a duplicated viptv_bound assignment; all preceding DHCP policy/compile/order checks 3–19 passed.
- Replaced the last five get_usb_modem_wan() calls in net_wan.c: automatic reconnect priority now refuses to queue on unknown identity; resolver publication forces DNSMasq refresh if identity is unknown; static DNS mode, WAN interface selection and WAN IPv4 lookup all use the status API and propagate failures.
- Removed direct legacy selector consumers from net_wan.c entirely while keeping the shared legacy boolean ABI wrapper for compatibility. Updated source contracts for identity/error ordering, fixed the two duplicated Python lines, and changed the stale static-IPoE teardown assertion in check_wan_detect_wan_status.py.
- New exact-head USB identity, WAN route, WAN DHCP and XY-C1 Build/TRX/artifact results are pending; no firmware pass is claimed.
- Next: search the wider RC userland for direct legacy selector calls and migrate only lifecycle-sensitive consumers with focused tests.

---

## 2026-11-22 — Align WAN identity lifecycle source contracts

- Parent main SHA: `05aebf7d3bf2242fd967cb825a928693f120a545`.
- Updated WAN-up and DHCP-bound contracts for the checked modem identity helper, requiring a status-returning helper call and error return before protocol/runtime or DHCP snapshot/interface mutation; also check that modem_dev and ifname_t are fetched through status APIs.
- Added the `re` import inside the DHCP workflow's embedded Python heredoc (not at YAML root) for the identity-call regular expression.
- This commit changes regression contracts/handoff only; exact-head workflow and XY-C1 validation remain required.

---

## 2026-11-22 — Refresh WAN-up and interface-teardown contracts for status API

- Parent main SHA: 7e8aee2ce10e3ad893015516395d82317d06a77d. The code commit 7e8aee2ce10e3ad893015516395d82317d06a77d passed USB identity #44970128339, WAN DHCP #44970128247 and AI handoff #44970128213.
- WAN route #44970128170 passed all compiled route, teardown-policy, PPP and interface-copy regressions, then failed at scripts/check_wan_dns_read_status.py line 320 because its WAN-up assertion still named the removed is_ifunit_modem() helper. Updated it to require is_ifunit_modem_status(), its explicit error return and ordering before checked protocol selection. The existing WAN-down status assertions remain in place.
- WAN interface teardown #44970128270 failed at an embedded workflow assertion requiring is_usb_modem = get_usb_modem_wan(unit) in stop_wan_locked(); production code now uses get_usb_modem_wan_status() and returns before transport/teardown side effects on read failure. Updated the workflow assertion to verify this checked order.
- XY-C1 run #44970128330 was still in progress at last poll. No target build or artifact outcome is claimed.
- Next source slice migrates reconnect-priority, resolver-notification, DNS-mode and WAN interface/address selectors.

---

## 2026-11-22 — Fail closed on USB modem identity in WAN start/stop and MAN routes

- Parent main SHA: a103c9ac29a3c353592119bbb3e458529378c1f1. The previous checker-only correction commit a103c9ac29a3c353592119bbb3e458529378c1f1 passed AI handoff documentation check #44969951549. Exact reruns of the updated DHCP and WAN-route contracts had not yet appeared at the last poll.
- WAN startup now checks modem identity after the configured interface is resolved but before MTU/interface-up changes; selector errors are retained and the unit is skipped instead of falling through to ordinary Ethernet/PPP startup.
- stop_wan_locked() checks the selector before pausing the monitor or stopping services, and refuses teardown when identity is unknown.
- MAN route setup records identity errors but still adds required gateway/default routes while suppressing optional DNS route additions; MAN teardown preserves the error and still attempts scoped idempotent DNS route cleanup when modem identity is unknown.
- Manual WAN disconnect refuses to stop services until modem identity is verified. Added USB identity workflow source contracts covering these orderings and cleanup semantics.
- Exact-head USB identity, WAN DHCP, WAN route, WAN teardown and XY-C1 Build/TRX/artifact checks are pending; no runtime hardware result is claimed.
- Next: migrate remaining boolean selectors in automatic reconnect, resolver notification, DNS-mode selection and WAN interface/address getters, then search the wider RC userland for lifecycle-critical reads.

---

## 2026-11-22 — Align WAN lifecycle regression contracts with checked modem identity

- Parent main SHA: 4a353d031429acdb49c6fe8a6d8aebedda9a6846. The committed lifecycle change 4a353d031429acdb49c6fe8a6d8aebedda9a6846 passed USB modem identity #44969756150, WAN interface teardown #44969756159 and AI handoff validation #44969756161.
- WAN DHCP run #44969756169 failed only in its final source-order contract, which still named the retired boolean is_ifunit_modem() helper. Earlier compile/policy checks 3–19 passed. Updated the expected classifier symbol to is_ifunit_modem_status() while retaining the ordering assertions that protocol validation precedes lease snapshot and interface mutation.
- WAN route run #44969756230 failed only in the DNS lifecycle contract, whose auth/DNS/static-IP assertions still encoded pre-migration boolean identity semantics. All compiled route/policy checks 3–10 passed. Updated it to assert accumulated modem identity errors, conservative lease/auth cleanup and preservation of DNS/static-IP state.
- XY-C1 run #44969756217 was still in progress when recorded; exact Build/TRX/artifact results must be checked. This commit changes checkers and handoff only; no production source changes are claimed.
- Next source slice migrates remaining direct get_usb_modem_wan() callsites beyond WAN-up/down/DHCP-bound and adds fail-closed source coverage.

---

## 2026-11-22 — Propagate checked USB modem identity through WAN lifecycle

- Parent main SHA: 7f44877976cd0c2399975adf09eb97c24a366aee. The checked selector API landed in f1327a7988b9552c2ce3294c641d27838efd9664; its identity workflow command escaping was corrected separately in 7f44877976cd0c2399975adf09eb97c24a366aee.
- Replaced is_ifunit_modem() with is_ifunit_modem_status(), which surfaces get_usb_modem_wan_status() failures and uses a bounded checked ifname_t copy. WAN-up and DHCP-bound now abort before interface/lease/route/NVRAM side effects when identity is unknown.
- WAN-down accumulates the selector error, still removes DHCP lease routes conservatively, retains WAN DNS snapshots and avoids static-IPoE address clearing when modem identity is unknown. Independent auth teardown remains best-effort.
- Extended the USB modem identity workflow source contract to guard the classifier, both early-abort paths and conservative teardown behavior.
- Exact-head USB identity, WAN DHCP, WAN route, AI docs and XY-C1 Build/TRX/artifact checks are pending for this source SHA; no target-side execution is claimed.
- Next source audit: remaining direct get_usb_modem_wan() consumers in WAN start/stop, MAN route setup/teardown, manual disconnect and resolver notification.

---

## 2026-11-22 — Repair checked modem identity workflow compiler command

- Parent main SHA: `f1327a7988b9552c2ce3294c641d27838efd9664`; source/API commit under test: `f1327a7988b9552c2ce3294c641d27838efd9664`.
- Exact USB modem identity run #44969481834 failed in the added parser compile step because over-escaped line continuations passed backslash arguments to the linker. Earlier host policy steps passed. Flattened the GNU89/ASan compiler invocation to one shell line; production source and parser tests are unchanged.
- XY-C1 run #44969481753 remained in progress on f1327 at last poll. Exact rerun and Build/TRX/artifact results remain required.

---

## 2026-11-22 — Add checked USB modem WAN identity reader

- Parent main SHA: `c6bda35a8e872f040534ef67069e3f30af467928`.
- Added `get_usb_modem_wan_status(unit, &is_modem)` backed by `nvram_get_checked()`; missing/empty/non-numeric values preserve the legacy zero default, numeric prefixes preserve `atoi()` behavior, while backend errors and integer overflow return errno-style failures. The old boolean `get_usb_modem_wan()` remains a compatibility wrapper.
- Extended the shared policy header and host regression for missing key, zero/nonzero and negative values, whitespace/numeric prefixes, malformed input, backend errors, unexpected API results, null output and integer overflow.
- Extended USB modem identity Actions to compile/run the parser regression under GNU89 + ASan/UBSan, include shared paths in push/PR filters and verify the new API contract. Existing lifecycle callsites remain for the next atomic migration slice.
- Exact-head USB modem identity and XY-C1 checks are pending.

---

## 2026-11-22 — Correct the part-024 archive byte count

- Parent main SHA: `8dd57dffb63cfc11dc0459e7afb3b6d02c1ed9aa`.
- Exact-head WAN route regression #44968740776 passed on `09232ef0e8677c59bfa49a538fb908ce31259d0f`. The AI handoff check first caught the 120-entry root log; the subsequent rotation commit `8dd57dff` archived 100 records into part-024 but its index recorded JavaScript character count (129192) rather than UTF-8 byte count (129424).
- Re-read the immutable archive: 100 records, first `Snapshot VPN remote-LAN route ACL before route mutation`, last `Fail closed on WAN startup protocol reads`, blob SHA unchanged `359eae06c5b550cf68dc48710a91ea546551ec8f`. Corrected only the index byte metadata; no archived/root log records changed.
- Exact-head AI docs run #44968969967 failed solely on that byte-count mismatch; rerun required on this correction SHA. The prior product SHA `02857320194ebdda3b451ea3d6de07fa212995c0` has XY-C1 Build, TRX verification and artifact upload steps 37/38/39 all PASS in run #44968037242.
- Next source slice remains the checked USB-modem WAN selector: `wanN_modem_dev` currently has a lossy `nvram_get_int()` reader.

---

## 2026-11-22 — Preserve WAN protocol error in DHCP deconfig regression contract

- Parent main SHA: `8998530e9609b5e401c15a3a63f23b20ebd27d60`.
- Exact-head WAN route run #44968619661 failed after all compiled policy regressions and earlier source contracts passed. Embedded Python line 654 still expected the old `interface_status → conntrack_status → cleanup_status` return chain and omitted the already-implemented `proto_status` first-error preservation in `udhcpc_deconfig()`.
- Verified the current `trunk/user/rc/src/net_wan.c` implementation reads the MAN protocol with `get_wan_unit_proto_checked_status()`, preserves the underlay address on read failure, continues independent teardown, and returns the accumulated protocol/interface/conntrack/cleanup status. Updated the WAN route contract to require that complete aggregation; no production source changed.
- AI handoff documentation check #44968619684 passed on the parent SHA. Exact-head WAN route rerun and this commit's documentation check are pending; XY-C1 validation remains tracked against product-source SHA `02857320194ebdda3b451ea3d6de07fa212995c0`.
- Next source slice: add a status-returning USB modem identity reader and migrate lifecycle-critical consumers without treating NVRAM backend errors as “not a modem”.

---

## 2026-11-22 — Update WAN route contract for checked protocol lookup

- Parent main SHA: `8ed832e39248e1ac0011dccfd247fba0414bac30`.
- Exact `8ed832e39248e1ac0011dccfd247fba0414bac30` WAN route run #44968296991 failed at embedded checker Python line 465. The checker still located `wan_proto = get_wan_proto(unit);` after the VPN-stop notification, but WAN teardown now uses `proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);`. Updated the contract to require the checked call and explicitly forbid the legacy getter in `wan_down_internal()`.
- On that same SHA, HTTPD authentication/disclosure policy and AI handoff checks passed. This change is checker-only.
- The next source task is an unchecked modem identity read: legacy `get_usb_modem_wan()` collapses an NVRAM backend read error into zero, potentially treating a modem as non-modem in lifecycle paths. A status-returning migration is queued as the next source slice.

---

## 2026-11-22 — Align WAN route and HTTPD source contracts with retained status

- Parent main SHA: `02857320194ebdda3b451ea3d6de07fa212995c0`; source under test: `02857320194ebdda3b451ea3d6de07fa212995c0`.
- Exact WAN route #44968037254 reached its cleanup contract and failed because the workflow expected the old `udhcpc_deconfig()` return expression that omitted `proto_status`. Production source deliberately accumulates protocol-read failure, interface reset, conntrack flush and WAN/MAN cleanup; checker now validates the complete nested first-error accumulation.
- Exact HTTPD security #44968037240 failed because its lifecycle check expected the pre-change combined stop/final interface cleanup gate. It now checks that callback cleanup precedes `wan_teardown_first_error_status()`, and that the resulting failure gate precedes clearing WAN state.
- On SHA `0285732...`, WAN teardown, DHCP callback, USB modem identity and handoff checks all passed. XY-C1 build remained in progress at the last poll; the previous build failure had been the four illegal assignments removed in `0285732...`.
- This is a test-contract alignment slice; no production source changes in this commit.

---

## 2026-11-22 — Fix XY-C1 WAN reset compile blocker and refresh source contracts

- Parent main SHA: `6cdaaccda83823f252dc537e4d9bfd358bfe845e`; product-source target was `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact XY-C1 run #44967290950 completed FAILURE. The production build reached `net_wan.c` and reported four errors in `reset_wan_vars_status()`: assigning string literals to fixed-size arrays `man_addr`, `man_mask`, `man_gate`, and `man_mtu`. Those buffers already have valid initializers; removing the redundant assignments fixes the C type error without changing defaults or the manual endpoint snapshot.
- Exact WAN route #44967576615 failed at the stale static-IPoE teardown parser in `check_wan_detect_wan_status.py`; its anchor is updated to match the checked-protocol condition and an array-initialization regression was added.
- Teardown run #44967575277 on `6cdaacc...` had zero jobs; inspection found two Python lines de-indented outside the YAML `run: |` block. Their indentation is repaired.
- Handoff validator #44967576593 passed. XY-C1 TRX and artifact steps were skipped after the compile failure; no L3 claim is made. A fresh exact-head firmware run remains mandatory.

---

## 2026-11-22 — Fix WAN IPv4 and teardown source-contract locators; refresh handoff

- Parent main SHA: `4c1f1be93e4c0436d0b20a3b4ddfbb314d07914c`; product source remains `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact checker SHA `4c1f1be93e4c0436d0b20a3b4ddfbb314d07914c`: WAN route #44967410609 failed only because the IPv4 lookup contract expected a brace-less error return; the source uses a braced checked-protocol error path. WAN teardown #44967410572 reached its final ordering assertion but located an earlier generic stop-status gate. Both source-contract locators are corrected without relaxing production behavior.
- AI handoff #44967410603 failed because a prior docs checkpoint renamed the required first-action heading. Restored the exact protocol heading and updated the handoff with product SHA, checker SHA, actual run states, the frozen backup and the first next action.
- WAN DHCP #44967410597 PASS on checker SHA `4c1f1be...`. The previous firmware run for product SHA `4a6df662...` remained in progress at last poll; L3 is not claimed.
- Next: verify exact-head route/teardown/handoff reruns, inspect XY-C1 Build/TRX/artifact, then continue checked modem identity audit.

---

## 2026-11-22 — Repair exact-head checked-protocol regression contracts

- Parent main SHA: `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact-head `4a6df662b6c34d2282e81b3ef8054e80e0894393` completed with three actionable workflow issues: WAN route checker still expected the removed `rc_first_service_start_status(stop_status, proto_status)`; WAN teardown workflow still located the old combined error gate rather than the new first-error aggregator; the DHCP callback embedded Python used an undefined `require()` helper.
- Updated those source contracts to verify protocol-error aggregation after callback cleanup and switched the inline DHCP check to a standard assertion. These are CI/test harness corrections; the production source slice remains unchanged in this commit.
- The WAN route, WAN DHCP callback and WAN teardown Actions on SHA 4a6df66 failed at the above contract/assertion issues. The host C policy builds preceding the WAN teardown Python contract succeeded. XY-C1 Build/TRX/artifact and the independent repository inventory remain under exact-head verification at this checkpoint; no unobserved result is treated as PASS.
- Next: inspect exact-SHA reruns for these three contracts and the WAN DHCP source gate; then review any compiler/build failure or other independent regression.

---

## 2026-11-22 — Fail closed on remaining unchecked WAN protocol consumers

- Parent main SHA: `d1108616599c54ff5268c780b9119d276b1e82fb`.
- The live source audit found four remaining unchecked `get_wan_proto(unit)` consumers in `add_man_gateway_routes()`, `man_down_internal()`, `wan_up_internal()` and `udhcpc_bound()`. They controlled whether MAN DNS routes were added/removed, which WAN route/address class to initialize, and which DHCP lease namespace was mutated.
- THIS source slice replaces those protocol guesses with `get_wan_unit_proto_checked_status()`: MAN-up continues essential gateway routes but skips optional DNS routes on an unknown protocol; MAN-down still attempts idempotent DNS cleanup while retaining the read error; WAN-up and non-modem DHCP bound refuse protocol-dependent changes before runtime-state/interface mutation. Confirmed modem callbacks retain their transport-specific path.
- A second teardown defect was fixed: `stop_wan_locked()` previously stored a checked protocol error in `stop_status`, then overwrote it with successful VPN/PPP stop results. Final teardown now aggregates the protocol status after independent interface/callback cleanup and refuses final WAN-state clear if it remains nonzero.
- Added executable C policy regressions for conservative MAN DNS route add/remove and preservation of the first protocol teardown error, plus source-contract checks for callback ordering and fail-closed behavior. Exact-SHA Actions are required for validation; no local firmware toolchain is available here.
- Next: inspect exact-head WAN route, DHCP callback, teardown and XY-C1 runs; address any true source/test failures before expanding the audit.

---

## 2026-11-22 08:32 CST — Exact-head WAN protocol hardening checkpoint

- Product source head at inspection: `6a7639cc946d9277a4a54db6c695bd816c472761`; `net_wan.c` blob: `d1315536f781f632aa2c1839eefb01619614b86d`.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head Actions for this SHA: WAN route operation status regression: #44966882221 completed/failure; WAN DHCP callback state stability regression: #44966882242 completed/success; XY-C1 Firmware Validation: #44966882256 completed/success; AI handoff documentation contract: #44966882224 completed/success. A missing workflow is not a PASS; in-progress jobs remain pending.
- WAN-protocol call-site scan still finds 5 direct `get_wan_proto()` occurrences in `net_wan.c`; next inspect `udhcpc_bound()` before changing lease publication, ensuring old lease-route cleanup is not skipped on read failure.
- Firmware L3 remains unverified unless Build XY-C1, TRX verification and artifact upload all pass on one product source SHA. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Align WAN source contracts with checked protocol reads

- Parent main SHA: `d004901f403430f32fd73ea950ed947bc287a6f1`.
- Exact-head WAN route #44966559531 reached the WAN DNS/interface source contract after its host regressions passed, then failed because the checker expected a brace-less error block while `get_wan_ifname()` correctly uses a braced checked-read gate. WAN DHCP callback #44966559464 and the WAN teardown contract #44966559612 likewise reached stale assertions expecting the old unchecked `get_wan_proto()` assignment after `wan_down_internal()` and `stop_wan_locked()` had migrated to the checked reader. Their preceding host regression binaries passed.
- XY-C1 #44966559594 reached the PPPoE-MWAN lifecycle contract and stopped at the same stale WAN-down getter anchor; the preceding detector and multi-WAN host tests passed. Firmware input detection/Build/TRX/artifact were not reached on that SHA.
- THIS checker-only slice updates the four affected contracts to require the actual checked protocol read, its error gate/status aggregation, and ordering before independent IPv4 cleanup. No product source changed.
- Exact-head reruns are required before claiming these checks pass. The same-source WAN-route/DHCP/teardown and XY-C1 results remain pending.
- Next: inspect exact-head reruns; continue the live audit of `udhcpc_bound()`, `add_man_gateway_routes()`, `man_down_internal()` and other remaining direct protocol consumers, preserving best-effort teardown.

---

## 2026-11-22 08:25 CST — Preserve cleanup while failing closed on WAN teardown protocol errors

- Parent main SHA: `14791ee073da672bb0a66c2ac618b786e9b57890`.
- Audit found two remaining unchecked protocol consumers in destructive teardown. `stop_wan_locked()` could skip its WAN-down route cleanup when `get_wan_proto()` failed. `wan_down_internal()` could skip lease-derived DHCP route removal and defensive Kabinet auth shutdown, while also risking a wrong DNS or static-address decision.
- THIS source change uses the checked per-unit protocol reader in both functions. Stop retains the protocol error as its first teardown status and invokes conservative `wan_down()` cleanup for an unknown protocol; subsequent independent interface and service teardown continues.
- `wan_down_internal()` now attempts lease-route prefix removal and defensive auth shutdown when non-modem protocol state is unreadable, propagates that protocol error, and refuses to clear indexed WAN DNS or a possible MAN underlay IPv4 address based on unknown protocol. Default/gateway routes, static routes, resolver, conntrack, statistics and post-down cleanup remain independent of this read.
- Added source-contract assertions to `check_wan_dns_read_status.py` for both checked reads, fail-safe lease/auth cleanup, DNS/IP preservation, and route/resolver/conntrack ordering. The WAN route workflow already filters both modified paths for push and PR.
- No local checkout or firmware toolchain is available; this source commit requires exact-SHA GNU89/ASan/UBSan Actions and XY-C1 firmware validation. The preceding checker-only descendant `14791ee073da672bb0a66c2ac618b786e9b57890` still needs its exact WAN route result recorded.
- Next: inspect exact-head WAN route and DHCP callback/XY-C1 runs; audit remaining direct `get_wan_proto()` in `udhcpc_bound()` and `add_man_gateway_routes()` for stale lease/gateway route decisions on unknown protocol.

---

## 2026-11-22 08:22 CST — Narrow WAN interface protocol contract to the unsafe getter

- Parent main SHA: `0ae742a0e277c75dd5b335f4a62098da5ab6c030`.
- Exact WAN route #44966179307 failed at the newly added `get_wan_ifname()` source assertion. The implementation contains the checked protocol read and returns its status before publishing the interface name; the assertion's broad substring check for `get_wan_proto(unit)` also matches unrelated text in the extracted function body and is redundant with the narrower obsolete assignment check.
- THIS checker-only fix narrows the stale-getter prohibition to `wan_proto=get_wan_proto(unit)`, while retaining requirements for the checked reader and immediate error return. No product source changed.
- On the same SHA, WAN DHCP callback #44966179303 and AI handoff contract #44966179398 completed successfully. WAN route host regressions before this one source-contract assertion all passed.
- Next: rerun WAN route on this checker descendant, then resume `stop_wan_locked()` / `wan_down_internal()` teardown audit. Firmware L3/L4 remains unverified until same-SHA target Build/TRX/artifact and hardware validation exist.

---

## 2026-11-22 08:20 CST — Repair WAN protocol source-check syntax and DHCP deconfig contract

- Parent main SHA: `c2fcb3bb5375637cf0ff9a48cc5cd47eb7b62514`.
- Exact WAN route #44965962990 failed in `check_wan_dns_read_status.py` because a newly added source-contract assertion used an over-escaped quote sequence in a Python string literal. THIS fix shortens the ordering anchor to `ifconfig(wan_ifname,IFUP,`, which still proves the checked protocol read precedes address clearing.
- Exact WAN DHCP callback run #44965963007 failed its previous return-contract assertion: `udhcpc_deconfig()` now correctly folds `proto_status` into the first-error accumulator, but the older workflow assertion still expected the pre-change two-status expression. THIS workflow-only adjustment now requires the nested `proto_status` → interface → conntrack → route-cleanup aggregation.
- All other WAN callback test stages before these assertions passed. XY-C1 #44965962988 was still in progress at this checkpoint; target Build/TRX/artifact outcomes must be verified separately on the exact source SHA.
- Next: check exact-head WAN route, DHCP callback and XY-C1 reruns; then continue the `stop_wan_locked()` / `wan_down_internal()` cleanup audit. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 08:18 CST — Preserve WAN identity and MAN underlay on protocol-read errors

- Parent main SHA: `e29f9cd724cd52d1f501ec48425e4f96fd080587`.
- `get_wan_dns_static_status()`, `get_wan_ifname()`, `get_wan_ip4()` and Ethernet link-restored DHCP notification still consumed unchecked `get_wan_proto()`. Read failures were silently interpreted as non-static DNS, a MAN interface, MAN IPv4 or a DHCP-capable WAN.
- THIS source change uses `get_wan_unit_proto_checked_status()` before each protocol-dependent decision: DNS mode returns the protocol error rather than entering the legacy `dnsenable_x` fallback; interface lookup returns the error rather than guessing a MAN/PPP identity; WAN-IP lookup returns `INADDR_ANY` with errno preserved; link restoration refuses to signal DHCP renewal when protocol state is unknown.
- `udhcpc_deconfig()` previously cleared a MAN interface to `0.0.0.0` whenever its protocol read failed, even though `is_ifunit_man()` conservatively classifies unknown protocol as MAN. It now retains the MAN underlay address on protocol error, while still running `man_down_internal()` route teardown and aggregating protocol, interface, conntrack and cleanup status.
- Extended `check_wan_dns_read_status.py` to pin the checked DNS/interface/IP/link-restored/deconfig paths and cleanup ordering. Both changed paths are already included in WAN route workflow push/PR filters.
- The preceding checker-only fix `e29f9cd724cd52d1f501ec48425e4f96fd080587` has exact-head WAN route #44965890151 and XY-C1 #44965890114 in progress at the time of this commit. No local full checkout/toolchain is available; executable host tests and target-build results must come from exact-SHA Actions.
- Next: inspect exact-head WAN route and XY-C1; audit `stop_wan_locked()` / `wan_down_internal()` for protocol-error static-address/DHCP-route/auth cleanup gaps without bypassing mandatory teardown.

---

## 2026-11-22 08:17 CST — Scope MAN classifier source contract to its definition

- Parent main SHA: `dc5df775b1ec1043b5f4c1ce42f472533f886682`.
- Exact WAN-route #44965681798 failed after earlier WAN tests passed because the final manual-PPP assertion called `body(wan, "is_ifunit_man(")`. A comment in the source contains that token before the definition, so the generic extractor selected the following unrelated function body instead of `is_ifunit_man()`.
- Exact XY-C1 #44965681825 also failed at this WAN checker. Its job log additionally reported an independent Samba contract extraction error (`unterminated function: run_samba`); this checker-only fix does not alter Samba code and that failure remains to diagnose separately.
- THIS checker-only change scopes the extraction to the exact definition signature `static int\nis_ifunit_man(char *wan_ifname, int unit)`, matching the already-correct earlier assertion. The negative protocol sentinel and explicit true classification remain required.
- Next: rerun exact-head WAN route/XY-C1; inspect the Samba checker failure independently. Then proceed with status-aware interface/DHCP protocol changes; hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 08:14 CST — Align manual-PPP checker with explicit error sentinel

- Parent main SHA: `76ddd91f39790262b8a373977d1a3b44f119a638`.
- Exact-head WAN route #44965482671 failed at `scripts/check_wan_firewall_status.py`: an earlier assertion still required `get_wan_unit_proto_for_manual_check()` to return `IPV4_WAN_PROTO_PPTP`, while the newer implementation deliberately returns `-1` and makes `is_ifunit_man()` explicitly return true for any negative protocol sentinel.
- The same checker later correctly required `return -1;` and `if (wan_proto < 0) return 1;`; the duplicate stale expectation made it internally contradictory. THIS checker-only commit changes the earlier assertion to require the explicit error sentinel while retaining the boolean underlay-protection assertion.
- All WAN route host regressions before the firewall contract passed. XY-C1 #44965482665 stopped at the same checker failure, so firmware-input detection, Build XY-C1, TRX and artifact upload were skipped. This is not a compiler failure.
- Next: inspect exact-head WAN route rerun after this checker repair, then continue status-bearing WAN interface selection and DHCP callback protocol reads; hardware L4 remains NOT VERIFIED.

---

## 2026-11-21 — Make manual-PPP protocol failure sentinel explicit

- Parent main SHA: `58903be39199a36dce04f1f87d43561c26e04446`.
- The previous checked wrapper returned PPTP as a conservative stand-in on read failure. That relies on downstream classifier branches treating PPTP as manual without consulting a PPTP-specific selector, which is less explicit than the intended safety property.
- THIS commit makes the wrapper return `-1` on read failure and makes the boolean `is_ifunit_man()` immediately return `true` for that sentinel. Thus an unavailable protocol never gets mistaken for a known protocol while the legacy bool ABI still protects the underlay.
- Added source-contract assertions for the negative sentinel and explicit `return 1` behavior. Exact-head Actions pending.
- Next: inspect the WAN route/NVRAM/Process/XY-C1 results and continue remaining route-gateway/teardown/DHCP reads without bypassing unconditional cleanup.

---

## 2026-11-22 08:10 CST — Fail closed on WAN protocol errors in disconnect and xl2tpd setup

- Parent main SHA: `bbc0dad3aa568ed4885e96cd6022b501fad6f609`.
- Continued the shared `get_wan_proto()` audit. Two concrete error fallthroughs remained: `manual_wan_disconnect()` mapped a failed non-modem protocol read to generic `stop_wan()`; `safe_start_xl2tpd_locked()` mapped an unreadable protocol to “not L2TP” and could rewrite `xl2tpd.conf` without the active WAN LAC section.
- THIS source commit exports the existing bounded checked protocol mapper through `rc.h`; manual disconnect checks protocol only for non-USB WANs and retains USB modem shutdown selected by checked transport type; xl2tpd refuses to rewrite its config on missing, unknown or backend-failed WAN protocol.
- Extended existing WAN DNS and PPP source contracts to pin error propagation, USB/non-USB selection and the protocol gate before `fopen()` truncates the live config. Workflow push and pull_request filters already include all changed sources/checkers.
- The preceding MTU-checker repair `bbc0dad3aa568ed4885e96cd6022b501fad6f609` passed WAN route #44965084237 and AI handoff #44965084235. No local full checkout/toolchain is available here; targeted source preflight confirmed branches, prototypes, checker assertions and workflow path coverage. XY-C1 firmware Build/TRX/artifact was not triggered by the checker-only repair.
- Next: inspect exact-SHA WAN route GNU89/ASan/UBSan and XY-C1 Build/TRX/artifact for THIS source change; continue `get_wan_proto()` audit in `get_wan_ifname()`, `get_wan_ip4()` and `udhcpc_deconfig()`, preserving unconditional lease/route/DNS/conntrack cleanup.

---

## 2026-11-22 08:07 CST — Update initial-MTU contract for checked WAN protocol acquisition

- Parent main SHA: `f1fc3d9d03ed18b6f432a5b1591b8ce2d4a031ab`.
- Exact WAN route run #44964927034 failed after the firewall status contract passed. The WAN initial-MTU checker raised `ValueError: substring not found` because it still located `wan_proto = get_wan_proto(unit);` inside `start_wan()`; the startup protocol read had already migrated to the checked status-bearing helper.
- THIS checker-only fix anchors MTU-order validation to `nvram_status = get_wan_unit_proto_checked_status(unit, &wan_proto);`, then retains all existing assertions that the checked MTU read/failure gate precede default MTU configuration and interface bring-up.
- Exact-source preflight confirmed the checked protocol assignment occurs before `get_wan_unit_value_int_status()`. No product source changed in this repair.
- Exact WAN route/XY-C1 runs for the previous source SHA were still underway at inspection; this descendant's exact-head reruns remain pending.
- Next: inspect exact-head WAN route and XY-C1; continue direct protocol-reader auditing for WAN interface selection, manual disconnect and xl2tpd. L3/L4 remain unverified until same-SHA Build/TRX/artifact and hardware evidence exist.

---

## 2026-11-21 — Preserve manual PPP underlay on protocol-read errors

- Parent main SHA: `506a91e599331c58caf6582b9de6075c472455da`.
- `is_ifunit_man()` is a boolean helper used by WAN callbacks. Its old `get_wan_proto()` call collapsed backend/missing/unknown errors to `-1`, which could be interpreted as “not manual” and route DHCP cleanup through an unknown PPP underlay.
- THIS source slice adds `get_wan_unit_proto_for_manual_check()`, using the checked protocol reader. On error it logs the status and conservatively returns PPTP classification so legacy boolean callers preserve the PPP underlay; valid protocols retain their exact mapping.
- It also includes the checked `start_wan()` protocol/first-error fix if that was not already present in the parent.
- `check_wan_firewall_status.py` statically pins conservative behavior. Source-level preflight passed; exact-head WAN and XY-C1 Actions pending.
- Next: examine remaining direct `get_wan_proto()` uses in teardown, gateway routes and DHCP callbacks; keep mandatory cleanup unconditional on protocol read errors.

---

## 2026-11-22 08:04 CST — Repair WAN startup checker to scope the matching error gate

- Parent main SHA: `33258a680e052c9334e64758b70f7dce3754becb`.
- Exact push WAN-route run #44964696301 and XY-C1 run #44964696249 both stopped at the dependent `scripts/check_wan_firewall_status.py` contract. All 27 preceding WAN-route host regressions and the WAN VLAN/bridge contract passed; the checker then failed its combined startup assertion. XY-C1 likewise stopped at the same contract before firmware-input detection, Build, TRX or artifact upload. This was not a compiler result.
- The defect was in the static contract: `start_wan()` also contains an earlier `nvram_status != 0` branch for the SYN-cookie security preflight. The checker used the first such branch while validating the later WAN-protocol read and compared unrelated gates.
- THIS checker-only commit scopes the error-gate search to the position immediately after the checked protocol read. It retains the first-error aggregation, diagnostic, and interface-lookup ordering requirements.
- Targeted preflight against the exact fetched `start_wan()` source confirmed the protocol read, following error gate, first-error aggregation statement, diagnostic, and gate-before-interface-lookup ordering. No product source changed in this repair.
- Next: inspect exact-head WAN route and XY-C1 reruns. Require WAN route PASS and all firmware stages on one exact source SHA before claiming L3; hardware L4 remains NOT VERIFIED.

---

