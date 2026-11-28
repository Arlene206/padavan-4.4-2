## 2026-11-28 13:45 CST (UTC+8) — Align ACK server-ID checker with tri-state option API

- Base main SHA: `84d77f3ae53d0f152237b3595c69da80f6005f22`. Exact-head run #46112122647 passed the GNU89/Werror/ASan/UBSan C policy binary and reached a stale source-checker assertion: it still expected `server_id_option != NULL` in the ACK commit section. The refactored code now records the tri-state result and commits the server address only when `server_id_status > 0` after checking the server-ID option length. AI handoff docs #46112122746 PASS.
- Updated the assertion to the actual presence contract without weakening the four-byte validation or server address commit ordering. Exact-head rerun is required; no local compile is claimed.

---

## 2026-11-28 13:41 CST (UTC+8) — Fix DHCP server-ID checker symbol typo

- Base main SHA: `6d5ef6e12897aa7af5e88c9c59bc6977c6a426e6`. Exact-head udhcpc #46111906660 again compiled and ran the GNU89/Werror/ASan/UBSan policy binary successfully, then stopped in the Python checker because it searched for the misspelled symbol `udhcp_server_id_option_status_valid` (missing the second `c`). The production helper and policy test correctly use `udhcpc_server_id_option_status_valid`. Exact-head AI docs #46111906681 PASS.
- This commit changes only the checker string to the actual production symbol. No test, policy or source assertion is removed or weakened. New exact-head udhcpc/docs checks are required; no local run is claimed.

---

## 2026-11-28 13:38 CST (UTC+8) — Update scanner checker for deferred match publication

- Base main SHA: `de9207aa81d8d67b86a79d30b0bd41906a303cd9`. Exact-head udhcpc #46111728564 again passed its GNU89/Werror/ASan/UBSan C policy binary, then the checker failed at the now-obsolete assertion expecting `*option_value = optionptr + OPT_DATA` inside the iterator. The implementation deliberately stores a private first match and publishes it only after all TLVs/overloaded fields validate. AI docs #46111728566 PASS; Process inventory #46111728525 PASS; NVRAM #46111728560 and storage #46111728542 were running.
- Updated the source checker to assert `found_value/found_length` capture, post-scan output publication, and that full cursor traversal precedes the successful return. Added an explicit source contract that output pointers are cleared before rejecting a NULL packet. No checks are deleted.
- Exact-head tests for THIS checker update are required. The previous checker-only failures do not indicate a C policy test failure, but they prevent declaring the DHCP regression green. No local compilation is claimed.

---

## 2026-11-28 13:34 CST (UTC+8) — Validate the full DHCP option stream before returning a match

- Base main SHA: `af3cded3c44d45c8827a5217fff2cfdbcdf71a4b`. The corrected checker anchor reached the next assertions, then #46111559507 failed because two whitespace-normalized assertions still expected spaces ("option_length, 1)") in strings where all whitespace had been removed. The GNU89/Werror/ASan/UBSan policy binary passed; AI docs #46111559510 passed.
- Corrected the checker to inspect the new internal scanner rather than the legacy wrapper (the old assertions referenced TLV/overload locals no longer in that wrapper), and fixed both normalized call signatures without dropping any contracts.
- Source review found a further issue in the just-added API contract: it promised `-1` for a malformed option stream, but returned immediately after the first match. A valid option 54 followed by a malformed TLV or malformed overloaded field could therefore be accepted without validating the remainder. The scanner now records the first match, continues validating every active main/file/sname option field, and publishes outputs only after a complete successful scan. On malformed trailing data it returns -1 with cleared outputs. Null packet errors also clear caller outputs before returning.
- The old public `udhcp_get_option()` semantics still skip zero-length matches (include_empty=0); the new API still reports them (include_empty=1). Existing callers are unaffected except that malformed trailing TLVs are now correctly rejected instead of being hidden by early return. Static checker now pins complete-scan-before-return and first-match retention.
- Parent af3 exact udhcpc #46111559507: C policy binary PASS, Python checker FAIL at normalized `option_length, 1)` assertion. AI docs #46111559510 PASS. The corrected complete-scan commit needs fresh exact-head udhcpc/docs/XY Actions. No local build/test is claimed.

---

## 2026-11-28 13:27 CST (UTC+8) — Match DHCP option-scanner checker to split C signature

- Base main SHA: `2643d05714776b43bf850fb2d41aa1e3c191ba74`. Exact-head udhcpc run #46111402271 compiled and ran the GNU89/Werror/ASan/UBSan policy binary successfully, then failed only in the Python source checker: `function_body()` searched for the literal `static int udhcp_get_option_internal(`, but the production definition deliberately formats the return type and function name on separate lines.
- Updated the source checker to anchor on the function name and first parameter instead of the split return-type spelling. No source, regression or validation assertion was removed or weakened.
- On the same parent SHA, AI handoff docs #46111402320, DHCPv6 Reply #46111402363, WAN DHCP callback #46111402372, Process fail-closed #46111402276, Process inventory #46111402310 and USB storage #46111402425 PASS. NVRAM #46111402311 and XY-C1 #46111402307 remained in progress; the parent c21 NVRAM audit #46111075506 is now PASS, while its XY-C1 #46111075565 was still building. The correction SHA needs its own exact-head Actions. No local test is claimed.

---

## 2026-11-28 13:22 CST (UTC+8) — Distinguish malformed DHCP server-ID options from absence

- Base main SHA: `c21c9f0c5fd6b603befbbf84d4ddbc34eaafad62`. The new address-less compatibility path still used `udhcp_get_option32()`, which returns NULL for both an absent option and any present option whose payload length is not exactly four bytes. In address-less mode that collapsed malformed option 54 data into the deliberately supported "no server ID" case; zero-length instances were also skipped by the legacy finder.
- Refactored the option iterator behind a compatible wrapper and added `udhcp_get_option_with_length()`: return -1 for malformed TLV streams, 0 only for a truly absent option, and 1 for a present option including zero-length; return the exact payload length. The original `udhcp_get_option()` continues to skip zero-length options for all existing callers.
- DHCPOFFER, DHCPACK and DHCPNAK now require every present server-ID option to have exactly four payload bytes before any address/state/script decision. Truly absent option 54 and explicit 0.0.0.0 remain supported only by the documented address-less mode; a present length 0/3/5 or malformed option stream is rejected. The all-ones endpoint filter remains in place.
- GNU89 regression covers the tri-state/length matrix (absent, valid IPv4 length, malformed parser status, zero, three, five, and inconsistent absent output), as well as a zero-length server-ID TLV. The Python checker pins the shared scanner semantics and validation order in OFFER/ACK/NAK. Parent exact-head udhcpc #46111075543 PASS and AI docs #46111075486 PASS; THIS commit's exact-head runs are pending. No local compiler run is claimed.
- Parent c21 additional PASS: DHCPv6 Reply #46111075672, WAN DHCP callback #46111075595, Process lifecycle #46111075501, Process inventory/fail-closed #46111075531, USB storage #46111075489. Parent NVRAM #46111075506 still in progress at its final ioctl/caller audit; XY-C1 #46111075565 still building. XY-C1 #46109982320 for the earlier HTTPD source SHA `856fe202e000de0b751c3f1ace304a208e7c0b37` completed Build, TRX verification and artifact upload successfully, but is not a substitute for the c21/c22 firmware SHA. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-28 13:10 CST (UTC+8) — Preserve address-less DHCP servers and reject invalid DHCPOFFER destinations

- Base main SHA: `7cc7bef24789998d0ebf63436835257a8006baf4`. Review of the selected-offer path found a compatibility regression in the prior server-ID hardening: legacy DHCP servers documented in `dhcpc.c` may omit option 54 or explicitly send 0.0.0.0. The preceding filter rejected explicit zero even when no server address had been selected, so those clients could receive an OFFER but never accept its ACK.
- The shared ACK/NAK policy now treats explicit 0.0.0.0 like an absent option only while `expected_server_id == 0` and not REBINDING. It remains rejected once a real server was pinned or during REBINDING. 255.255.255.255 is rejected unconditionally.
- The DHCPOFFER path now runs the same validation before publishing REQUESTING state, preventing a limited-broadcast identifier from being sent in DHCPSELECT. Missing and explicit-zero server IDs retain the documented address-less-server behavior.
- GNU89 regression additions cover address-less ACK/NAK compatibility, selected-server and REBINDING rejection of zero, all-ones rejection in ACK/NAK, retained zero server address, and source-order validation before REQUESTING. The exact-head workflow #46110863007 on parent `7cc7bef24789998d0ebf63436835257a8006baf4` PASS, but it predated this compatibility correction. THIS commit's exact-head udhcpc and docs runs are pending. No local test is claimed.
- On parent `7cc7bef`, Process #46110863026 and AI docs #46110862998 PASS; storage-hotplug #46110863015, NVRAM #46110862963 and XY-C1 #46110862988 were IN_PROGRESS at last poll. No target L3 or hardware L4 claim is made for THIS commit.

---

## 2026-11-28 13:04 CST (UTC+8) — Reject limited-broadcast DHCP server identifiers

- Base main SHA: `6b897adfbe68c7b3ab9d0eb040181b806662e3ef`. THIS COMMIT updates `trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcpc_script_policy.h`, `trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcpc_script_policy.c`, `scripts/check_udhcpc_script_status.py`, `dev_log.md` and `SESSION_HANDOFF.md`.
- Confirmed edge case: the shared ACK/NAK server-ID policy rejected an explicit 0.0.0.0 value but accepted 255.255.255.255. RFC 2131 §4.1 defines the server identifier as both server identity and a destination for later unicast requests, and requires the server to choose an address reachable from the client; the limited-broadcast address cannot identify a reachable unicast server endpoint.
- The shared policy now rejects both unspecified and limited-broadcast values when the option is present, without changing the legacy absent-option compatibility or accepting/rejecting alternate identified servers during REBINDING. GNU89 regression cases cover ACK and NAK in normal and REBINDING flows, including a previously selected all-ones identifier; the Python source checker pins the policy and each test.
- Parent exact-head udhcpc regression #46109616336 PASS on `c2445cf35c055bee356cd400294736314395f5c2`; it included the pre-existing 0.0.0.0 checks but not this new edge case. THIS commit's exact-head regression and AI handoff documentation Actions are pending. No local test is claimed because no checkout is available.
- Same-parent exact-head Process, storage-hotplug, NVRAM and XY-C1 Actions #46109616357, #46109616322, #46109616330 and #46109616326 PASS on `c2445cf`. The newer HTTPD source fix at `856fe202e000de0b751c3f1ace304a208e7c0b37` still has XY-C1 run #46109982320 IN_PROGRESS at Build; TRX and artifact steps are pending. L3 for that newer HTTPD source and physical-router L4 are NOT VERIFIED.

---

## 2026-11-28 13:42 CST (UTC+8) — Make Shadowsocks procfs source contract compare literal escapes

- Exact-head Shadowsocks #46107441785 on `2baf290f67eb662699b9edf6ae1eec97f51f5806` again passed shell syntax, startup-status, DNSMasq cleanup, process lifecycle fake-procfs runtime tests and adblock config regression, then failed only at `scripts/check_shadowsocks_lifecycle.py`'s `tr` source-string assertion.
- Root cause: the previous checker correction used a normal Python string containing `\000` and `\n`; Python evaluates those as NUL/newline, so the expected value still does not equal the shell source's literal backslash sequences. THIS checker-only change adds a raw-string prefix, so it compares the exact source text without altering the production shell helper.
- Current main before this correction: `437b6ffd7fc1deb784ff86257a1271c22fa54874`. Storage-hotplug #46107555462 and its companion docs check were in progress/queued; inspect exact outcomes after this commit. No local shell run claimed.
- VSFTPD source SHA `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` L2 host regression and L3 XY-C1 build/TRX/artifact upload are all PASS; artifact #15247998322 is preserved in the handoff. L4 not performed.

---

## 2026-11-28 13:30 CST (UTC+8) — Align storage safety checker with FTP final-absence stop contract

- Storage-hotplug run #46107190352 on main `e1bd1892de53163bf044f636b000f4add5b9d574` passed the entire embedded whole-storage caller graph after the two workflow assertion fixes, then failed in `scripts/check_storage_safe_remove.py` at its FTP stop postcondition. This third stale contract expected `storage_service_stop_complete(status, final_state)` (twice) and the removed `ftp_was_present` pre-stop refresh exception.
- THIS change updates all three stale checker blocks to assert `ftp_stop_status_result`, preserved `stop_errno`/probe errno and final procfs presence, first-error group-stop merging, and firewall/UPnP refresh only on final ABSENT. Product source and existing host regressions are unchanged.
- XY-C1 on exact VSFTPD source SHA `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` completed PASS: Build, TRX and artifact upload; artifact #15247998322 (12,837,319 bytes; SHA-256 `9d387a4418292dfbd642f2d0271ec00a4e0227a65ac02e50ef7ce83e3e6f56c9`). L3 default XY-C1 verified; physical L4 not run.
- Checker fix exact-head Actions pending. Shadowsocks procfs runtime tests all passed on `f23be0cff7d7e0d1e942ebcf10094497b73be676`; its separate Python checker escape mismatch was corrected in `2baf290f67eb662699b9edf6ae1eec97f51f5806`, whose exact Actions are pending at this log point.

---

## 2026-11-28 13:15 CST (UTC+8) — Fix Shadowsocks procfs checker escape mismatch

- Exact source SHA `f23be0cff7d7e0d1e942ebcf10094497b73be676`: Shadowsocks shell syntax, startup-status regression, DNSMasq cleanup regression, process lifecycle fake-procfs runtime regression and adblock config regression all PASS. Process inventory #46079286495 and XY-C1 Firmware Validation #46079286427 PASS.
- The final Shadowsocks Python source-contract step failed because its expected `tr` literal encoded two backslash characters, while the helper correctly contains one backslash for each of the NUL/newline escapes. Corrected the checker expectation; production source and runtime tests are unchanged.
- The old AI docs #46079286331 failed solely on the 131,389-byte roadmap (>131,072 hard limit). Later exact-main docs checks #46106904624 on `c57f036a` and #46107190369 on `e1bd1892` passed, so no current docs-size failure remains.
- Exact-main Shadowsocks source-contract rerun and its AI docs check are pending on THIS correction commit. No local shell test is claimed.

---

## 2026-11-28 13:02 CST (UTC+8) — Correct second storage-hotplug FTP caller assertion

- Checker correction commit `c57f036a7c16c7d8d91fe9e95731da5eeb8e03dc` successfully removed the first obsolete FTP stop-status assertion, allowing the USB storage hotplug workflow to progress through all host regressions to the full caller-graph contract.
- Exact run #46106904590 then failed at the next obsolete assertion: it assumed `stop_ftp_locked()` was called as an `if` condition in `stop_stor_apps()`. The correct implementation stores the detailed status in `ftp_stop_status`, checks that status, and separately gates firewall/UPnP refresh on final procfs ABSENT. THIS workflow-only follow-up asserts all three implementation facts for FTP while retaining the existing conditional-call contract for other daemons.
- Product source and host regressions are unchanged. Exact rerun is pending; XY-C1 #46106551564 remained in progress on product SHA `4dd43a97904e7bec0c8ae094ff7c25ff82312b07`.

---

## 2026-11-28 12:48 CST (UTC+8) — Correct storage-hotplug FTP stop source contract

- Exact source SHA `4dd43a97904e7bec0c8ae094ff7c25ff82312b07`: VSFTPD regression #46106551633 PASS (GNU89/Werror/ASan/UBSan and source-order contract); AI handoff #46106551579 PASS; Process inventory #46106551548 PASS; fail-closed process inventory #46106551859 PASS.
- Storage-hotplug umbrella #46106551612 failed only at its caller-graph Python assertion for `stop_ftp_locked`: it still expected the replaced `storage_service_stop_complete(status, final_state)` expression, while the new helper preserves the original `kill_services()` result via `ftp_stop_status_result(status, final_state)`. All earlier hotplug host tests in that run passed. Updated the checker to assert the new errno/status contract; no production behavior was weakened.
- Same-source XY-C1 #46106551564 was still building (TRX/artifact pending) and NVRAM audit #46106551639 was still running at last poll. Exact-head reruns after this checker correction are required.
- iperf3 regression #46079981839 and Process inventory #46079981918 both PASS on `6d659d767e8ac236527ca5cf02f2daedaf386507`; compare against `4dd43a9` confirms iperf3 and the shared process helper files are unchanged. L2 is VERIFIED.

---

## 2026-11-28 12:30 CST (UTC+8) — Gate VSFTPD storage-stop firewall cleanup on final absence

- Base main: `badf4b0a7b0b94f2c774134da72978474bd859d9`. THIS COMMIT changes `trunk/user/rc/src/services_stor.c`, `ftp_daemon_policy.h`, `tests/ftp_daemon_policy.c` and `.github/workflows/vsftpd-stability.yml`; updates the VSFTPD roadmap and session handoff.
- Confirmed defect: `ftp_storage_stop_needs_firewall_refresh(was_present, final_presence)` authorized firewall/UPnP refresh whenever the daemon was RUNNING/UNKNOWN before the stop, even when final inventory still reported RUNNING/UNKNOWN. This could mutate firewall policy while daemon shutdown was not proven and storage removal was being aborted.
- Fix: require final `PROCESS_PRESENCE_ABSENT` for every stop-path firewall refresh; remove the pre-stop-state exception; preserve the detailed nonzero `kill_services()` result through `stop_ftp_locked()` and preserve the first FTP error during storage-stop aggregation.
- Regression: extend the GNU89 FTP daemon policy test for ABSENT/RUNNING/UNKNOWN/invalid final states, detailed stop status preservation and first-error aggregation; update exact source-order workflow contracts. Existing workflow push and pull_request path filters already include each changed source/test path symmetrically and are unchanged.
- Validation: no local checkout/compiler is available in this environment. Exact-head VSFTPD host regression, AI docs validator and XY-C1 Build/TRX/artifact checks are pending for THIS COMMIT; no L2/L3 pass is claimed. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-28 12:10 CST (UTC+8) — Align handoff headings and cross-session gate with validator

- The AI handoff documentation contract on docs SHA `bbd09ac2ce47b99468795e4df4f497098d07f3da` failed at its handoff section checks: required canonical section headings and the exact cross-session execution heading/phrases were absent, although the underlying facts were present.
- THIS documentation-only correction makes the handoff conform to the live validator without weakening the checker: adds canonical repository/verification/active-task/next-action headings and explicitly states one atomic main commit plus the before-context-exhaustion checkpoint.
- No product source, tests, firmware settings or workflow triggers changed. Exact-head AI documentation validation on this commit is required; no local checkout was available.

---

## 2026-11-28 12:01 CST (UTC+8) — Verify IAPP monotonic hardening and repair handoff checkpoint contract

- Product-source/test baseline: `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`. Exact-main IAPP lifecycle #46096900784, Process inventory #46096900769 and NVRAM transaction audit #46096900816 all PASS.
- XY-C1 Firmware Validation #46096900847 PASS on that exact source SHA: cross-build, TRX verification and artifact upload all completed successfully (artifact #15243698792, 12,837,390 bytes; SHA-256 `c77d7d4117396efe6e0f560c72b3f9f557a5397493955a2636e99b8a40fc0d63`). This is L3 default-configuration evidence only; physical-router L4 and macro-enabled optional IAPP target compilation remain unverified.
- AI handoff documentation contract #46096900817 failed because SESSION_HANDOFF.md lacked the mandatory checkpoint heading and conditional-edit statement. THIS documentation-only commit adds the required handoff section and updates the IAPP roadmap row with exact current L2/L3 evidence. Product source and firmware build configuration are unchanged.
- The AliDDNS source/test/build changes from closed PR #1 were compared with current main; no missing production or regression path was found to port. Current main contains the HMAC helper and CA trust-store bootstrap with later hardening.
- Root log had 72 complete records before this entry; no archive rotation is due. Exact-head AI docs validation for THIS commit is pending.

---

## 2026-11-27 — Reject malformed negative CLOCK_MONOTONIC samples

- The monotonic event-log helper now rejects negative `tv_sec` values as invalid, preventing malformed signed timespec inputs from reaching elapsed-time subtraction. Valid CLOCK_MONOTONIC samples are nonnegative; normal poll timing is unchanged.
- Added negative-second cases to the GNU89/Werror/ASan/UBSan regression and updated the source contract. Exact-head IAPP and firmware CI required.

---

## 2026-11-27 — Checkpoint DHCP checker repairs and handoff

- Current main parent: a6faece21549c01ad0161c7c18d9a26725c3a6fd. Frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- DHCP production source slice db92b3164321618e559051b471f560c7a863d25d: ACK callback timing accounts from ACK receipt; late successful callbacks preserve remaining lease budget; expired callbacks deconfigure; failed initial bound hooks clean up before discovery, with DECONFIG_PENDING if cleanup fails.
- Checker descendants 6a89890e... and c9b0d86... corrected the multiline cleanup helper match and remaining stale retry_continue references. At the end of this checkpoint the exact-head udhcpc rerun was not yet verified; the next session must inspect run state on current main.
- Watchdog executable identity gate, IAPP monotonic event-log timer, and storage media-link checker repairs are summarized in SESSION_HANDOFF.md. Prior PASS results are SHA-specific. XY-C1 #46095697024 on db92 source SHA was still under observation; Build/TRX/artifact status not confirmed as PASS. No L3/L4 claim.

---

## 2026-11-27 — Remove final stale udhcpc checker retry marker

- Follow-up to the c9b0d86... checker correction: exact-head run still exposed one last `retry_continue` reference in the later lease-plan ordering assertion at line 274. Replaced with `final_continue`; runtime code unchanged.

---

## 2026-11-27 — Fix udhcpc checker stale retry_continue reference

- The exact 6a89890e57e3be78d921a8fbd703ce245cf70790 udhcpc policy binary passed; its integration checker failed with NameError because `retry_continue` was removed by the prior contract refactor but still referenced by three assertion lines. Replaced those references with the already-defined `final_continue`.
- Checker-only correction. Runtime code unchanged.

---

## 2026-11-28 — Make udhcpc bound-cleanup assertion tolerate wrapped C call

- Exact db92b3164321618e559051b471f560c7a863d25d BusyBox udhcpc sanitizer test passed, then the source checker failed on a brittle one-line assertion for `udhcpc_bound_script_failure_requires_deconfig(state == REQUESTING)`. The C source wraps the argument onto the following line.
- Updated the contract to find the function call and separately assert the REQUESTING argument. No product source change.

---

## 2026-11-28 — Deconfigure partial initial DHCP binds after event-hook failure

- Fixed a failed-`bound` rollback gap: the client previously rejected the lease state commit but retried DHCPREQUEST immediately, leaving any partial address/route configuration made by the failed hook in place. The initial-acquisition branch now runs `deconfig` before returning to INIT_SELECTING; failed cleanup uses DECONFIG_PENDING and retries. Renewal/rebind failures deliberately retain the current lease and do not deconfigure.
- Added a policy helper/test for the acquisition-vs-renewal distinction and source-contract checks for cleanup ordering.

---

## 2026-11-28 — Remove undeclared regex dependency from DHCP source checker

- The DHCP checker used `re.search` but did not import `re`, causing NameError after the sanitizer policy regression had passed. Replaced it with whitespace-stripped string matching; runtime source unchanged.

---

## 2026-11-28 — Make udhcpc lease planner call assertion whitespace-tolerant

- The earlier udhcpc checker failed at a literal one-line string assertion for `udhcpc_lease_script_plan(lease_seconds, script_elapsed,`; the C invocation wraps the parameters on separate lines. Replaced this brittle check with a whitespace-tolerant regex.
- The standalone policy binary had already passed under GNU89/Werror/ASan/UBSan.

---

## 2026-11-28 — Read IAPP timer helper contract from its header

- IAPP lifecycle #46095191121 passed test compilation and lifecycle regressions but failed because the source checker looked for the helper declaration in `rtmpiapp.c`, while the implementation correctly lives in `iapp_event_log_poll_policy.h`.
- Updated the checker to read the actual policy header and assert both nanosecond fields are validated. Runtime source unchanged.

---

## 2026-11-28 — Update IAPP ioctl regression for timespec poll contract

- IAPP lifecycle #46095024005 failed in the interface regression compile because the older `iapp_ioctl_ifname_policy.c` test still passed integer elapsed seconds to the new two-timespec helper.
- Updated that call site to verify <1s, exactly 1s, >1s, backwards, and invalid arguments. Production source unchanged.

---

## 2026-11-28 — Tighten IAPP monotonic timer diagnostics and GNU89 test ordering

- Corrected the new IAPP timer diagnostic string escapes to emit real newlines, and moved the event-log test call below all declarations in main to keep GNU89/C90 declaration ordering warning-free.
- No timer behavior change; exact-head IAPP/XY-C1/AI CI required.

---

## 2026-11-28 — Make optional IAPP event-log polling monotonic

- `IAPP_Start()` event logger relied on gettimeofday()/timersub() for a periodic one-second snapshot. Wall-clock changes can suppress or accelerate event capture. Replaced with CLOCK_MONOTONIC timespec polling; backwards samples and malformed inputs do not trigger an early poll. On a transient clock-read failure, the loop logs once and retries; first valid recovery sample captures events immediately.
- Added a timespec boundary regression under GNU89/Werror/ASan/UBSan and wired it into iappd-lifecycle. The existing optional IAPP_EVENT_LOG syntax check remains.
- No local target cross-build or physical AP test.

---

## 2026-11-28 — Correct DHCP lease planner integration assertion

- The udhcpc integration check on c94fb8058b32b46b9a3b98c10605772799f82739 failed because its older ordering assertion still searched for `already_waited_sec = script_elapsed` and `timeout = lease_seconds / 2` after the ACK branch had been refactored to call `udhcpc_lease_script_plan()`. The helper now owns those calculations.
- THIS is checker-only: assert the expiry plan and deconfig retry happen before server/rebind/IP/BOUND commits. No runtime source changed.
- The standalone GNU89/Werror/ASan/UBSan policy test passed; exact-head integration rerun pending.

---

## 2026-11-28 — Start DHCP ACK lease accounting before ARP/script work

- Moved the ACK lease start timestamp from immediately before the bound/renew callback to the start of the accepted DHCPACK branch. This includes ARP validation and all ACK handling time in the lease budget; it also prevents failed-renew retry timing from ignoring time spent in ARP validation.
- Added source contract assertion that the ACK clock starts before optional ARP probing and the script. Exact-head tests pending.

---

## 2026-11-28 — Preserve valid DHCP leases when ACK callbacks run past T1

- Fixed a DHCP client timing bug: successful bound/renew callback runtime greater than T1 was previously treated as full lease expiry, resetting timers to zero and allowing the BOUND → RENEWING → REBINDING path to immediately deconfigure a still-valid lease. The new timing policy starts renewal immediately after T1, caps subsequent retry budget to the actual remaining lease, and deconfigures before discovery only when callback execution reaches/passes the full lease deadline.
- Failed expiry cleanup enters the existing DECONFIG_PENDING retry state. Added GNU89 regression for pre/at/post-T1 callback runtimes, lease expiration and invalid pointers; updated `check_udhcpc_script_status.py` to assert cleanup/commit ordering.
- Exact-head tests/build pending. No local MIPS build or hardware validation.

---

## 2026-11-28 — Compact current handoff and checkpoint watchdog/storage CI

- Exact parent main SHA: 3d6d2d712a3c5368f327810dd5adb73c8af70932; frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Process lifecycle #46094087141 PASS on 44f801f34ef7e8d908b9a5d7f5bac5e56d0ac4d2, including the new GNU89/Werror/ASan/UBSan watchdog executable identity regression. Watchdog signal #46094212414 PASS on 3d6d2d712a3c5368f327810dd5adb73c8af70932.
- AI docs #46094212400 failed only because SESSION_HANDOFF.md exceeded 64 KiB; THIS docs-only commit replaces redundant current-session copies with a concise active handoff. Exact-head rerun required.
- XY-C1 #46093910770 on production source SHA 559289198b598cc2531ce7f37d14e1737a1e51ee was still building: host regressions PASS, Build in progress, TRX/artifact pending. No L3 or L4 claim.
- Storage hotplug, AI docs and XY-C1 workflow runs on 363fd7fba6f90352d70bb581ea6d0690a573441c completed PASS, but Build/TRX/artifact were SKIPPED, not L3 evidence.

---

## 2026-11-28 — Add watchdog identity regression source to both path filters

- Exact watchdog-signal run #46094036822 on 42b1578798b30147ba099200a7563529ab4e94ab failed its inline path contract because the new regression C file was in the expected list but absent from YAML push/pull_request path filters.
- THIS commit adds the test source to both workflow filters and the standalone checker path list. Runtime code unchanged.

---

## 2026-11-28 — Separate watchdog-specific scan from generic service inventory contract

- Exact 559289198b598cc2531ce7f37d14e1737a1e51ee Process lifecycle regression #46093910751 failed because the integration checker still expected generic `process_inventory_scan("watchdog", ...)` directly inside `start_watchdog()`. Startup now calls a watchdog-only wrapper which performs that inventory but validates argv0 and /proc/PID/exe before recognizing a real daemon. HTTPD/dnsmasq retain generic process scans.
- THIS commit updates the checker accordingly and asserts the guarded scan, error propagation and snapshot-release ordering. No runtime source changed.

---

## 2026-11-28 — Repair inline watchdog signal CI for executable identity gate

- The watchdog-signal workflow's executed inline Python heredoc duplicated the rollback source assertions from `scripts/check_watchdog_ntp_lifecycle.py`. The prior correction updated the standalone script but not the heredoc, so exact 4acd703a... run #46093868038 still failed on legacy `kill_services()`/generic scan assertions.
- THIS commit updates the inline contract and verifies the new watchdog executable identity policy/test path filters and XY-C1 policy trigger. Runtime C source unchanged.

---

## 2026-11-28 — Remove unused watchdog identity-check local

- Removed the unused `saved_error` automatic variable from `watchdog_process_identity_matches()`. This avoids a likely `-Wunused-variable` failure under the repository's GNU89/Werror builds; no behavior changed.
- c9f5d0e1 checks: HTTPD security/lifecycle, DNSMasq lifecycle, Process inventory audit and AI Docs PASS; watchdog signal failed at a stale rollback assertion, corrected in 4acd703; Process lifecycle was cancelled; XY-C1 was still running at last observation.

---

## 2026-11-28 — Update watchdog signal integration contract for identity-bound rollback

- Exact c9f5d0e1abd9df58576b18db4e8d10db0e448066 watchdog-signal run #46093743633 failed at the checker assertion that still required `kill_services(service_names, 3, 1)` and generic rollback inventory. The source intentionally replaced that path with executable/argv0-bound SIGTERM/SIGKILL targeting.
- THIS commit updates the test contract to validate identity-aware signaling, bounded waits and the guarded rollback scan. It also adds the new header to the watchdog signal workflow path filters. No production source changed.
- Other exact c9f5d0e1 runs: HTTPD security/lifecycle, DNSMasq lifecycle, Process inventory audit and AI docs PASS; main Process lifecycle and XY-C1 remained pending when recorded.

---

## 2026-11-28 — Restrict watchdog trigger-count assertions to workflow path filters

- Added a watchdog executable-identity policy and gated watchdog startup/readiness/rollback scanning/signalling in c9f5d0e1abd9df58576b18db4e8d10db0e448066. No local cross-build was available; exact GitHub Actions are the verification gate.
- The new workflow path-check incorrectly counted a test-file path in its compile command as well as the two push/PR filters. THIS commit changes the checker to count the exact YAML path-list entries. Runtime source is unchanged.
- At time of update, Process lifecycle #46093743723, XY-C1 #46093743713 and AI Docs #46093743683 on c9f5d0e1... were still IN_PROGRESS. Storage hotplug on 363fd7fba6... PASS (#46093641272), XY-C1 on that SHA still pending, AI Docs PASS (#46093641318).

---

## 2026-11-28 — Bind PID 1 watchdog readiness and rollback to executable identity

- Generic process inventory matches argv0 basename and comm aliases to preserve BusyBox/legacy compatibility. The watchdog startup path was still using this generic result, allowing an unrelated local process with argv0 `watchdog` to suppress the actual PID 1 watchdog. The firmware ROMFS Makefile links `/sbin/watchdog` to the `/sbin/rc` binary.
- THIS source slice adds a watchdog-scoped gate requiring both watchdog argv0 and the resolved `/sbin/rc` executable, revalidates process identity after /proc reads, and routes startup/readiness/rollback scans through it. Failed-start cleanup signals only verified instances with TERM then bounded KILL, not generic matching processes. The GNU89/Werror/ASan/UBSan policy regression and both Process/XY-C1 workflow triggers are added.
- No local MIPS build or physical-router test was run. Exact-head Process, XY-C1 Build/TRX/artifact and AI docs checks remain required. Shared generic process-inventory behavior is unchanged.

---

## 2026-11-28 — Correct staged storage-link mode contract

- Exact SHA aa155bd6b424603b75f5440d888d19885c8fcd86: AI docs #46093243938 PASS and XY-C1 Firmware Validation #46093243944 PASS. USB storage hotplug #46093243935 failed only at the Transmission publisher body assertion after the updated lstat/policy contract and caller graph passed.
- Source helper deliberately calls `create_mp_link_status(dest_dir, staged_path, force_first_valid)`; its input is range-checked to 0/1. Hardcoding 0 in the checker was another stale expectation; MiniDLNA/Firefly need the second-pass fallback, while Transmission/aria call it with 0. THIS commit fixes only the integration expectation.
- Exact-head storage and AI docs checks required. No source behavior changed.

---

## 2026-11-28 — Align storage target discovery contract with lstat policy

- Exact SHA 670935e80e5e9519ea87e12d142924c6f6b05b1b: AI handoff docs #46092735083 PASS. USB storage hotplug #46092735162 progressed through the corrected media-publisher extractor and full caller graph, then failed only at `scripts/check_storage_safe_remove.py:1657`.
- The source now discovers mount targets using `lstat()`, `S_ISDIR/S_ISLNK`, and `storage_media_link_target_status()`; ENOTDIR/EEXIST behavior is defined in `storage_media_link_policy.h`. The checker still expected the obsolete `check_if_dir_exist()` path and demanded those errno tokens in the caller body. The dedicated media-link policy regression already covers the actual behavior.
- THIS checker-only correction inspects the lstat flow and policy helper separately. No runtime product source changed. Exact-head storage + AI docs reruns pending.
- Next independent source issue: prevent argv0/comm spoofing from suppressing the PID 1 watchdog. Require `/sbin/rc` executable identity plus argv0 basename `watchdog` in startup/readiness/rollback scans; use identity-bound rollback signaling.

---

## 2026-11-28 — Anchor storage media-publisher contract to its definition

- Exact storage-hotplug run #46092458013 on 655d173234a14ce19b91f2fce9e68e1622bfc7cc passed the updated Transmission call-site assertion but failed at the common publisher-body contract. Commit 0f92cf9ca01698e0c368060b9653d30b984a9d9a corrected the expected force_first_valid forwarding argument.
- Source inspection confirmed the deeper checker bug: function_body(storage, "storage_media_link_publish(") matched an earlier caller invocation (MiniDLNA) before the helper definition and therefore extracted the following if block, not the implementation. THIS COMMIT anchors the lookup to static int followed by storage_media_link_publish( so staged-link, parent revalidation and rename assertions inspect the actual function.
- No production source changed and no local test was run. On parent SHA 0f92cf9ca01698e0c368060b9653d30b984a9d9a, USB storage hotplug #46092637018 and AI handoff docs #46092637068 were IN_PROGRESS at the time of this commit. Exact-head reruns are required; no L3/L4 claim.

---

## 2026-11-27 — Align storage-hotplug publisher contract with selectable mount mode

- Exact-SHA storage-hotplug run `46092458013` on `655d173234a14ce19b91f2fce9e68e1622bfc7cc` passed the earlier media-publisher call-site assertions but failed the helper contract at the next line: the checker still expected `create_mp_link_status(dest_dir, staged_path, 0)`. The shared helper correctly accepts the validated `force_first_valid` parameter so MiniDLNA/Firefly retain their original second-pass fallback while Torrent/Aria use first-valid-only mode.
- THIS checker-only follow-up expects `create_mp_link_status(dest_dir, staged_path, force_first_valid)`; adjacent source assertions continue validating the boolean range and caller-specific 0/1 use. No product source behavior changed.
- Exact SHA `655d173`: AI handoff docs `46092457928` PASS; USB storage `46092458013` FAIL only at the stale helper-body expectation. Parent SHA `adeeba571bf9388b7da34259a90524a2bf18d4be`: AI docs, service-enable, MiniDLNA, Firefly, IPv6, USB modem, firewall, libdisk, HTTPD, Process inventory/lifecycle, NFS, vsftpd and Samba checks PASS; storage-hotplug `46092345628` exposed the same stale source-contract chain; XY-C1 `46092345566` and NVRAM audit `46092345437` were pending at last poll. No firmware build or hardware result is claimed.
- Next: verify exact-SHA storage-hotplug and AI docs for THIS correction; then inspect remaining storage/media path races and complete the current handoff. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Update storage callers' integration contracts for atomic Transmission publication

- Exact SHA `00e1d2efa475a2b693e26c481f230164285f1d05` exposed two more stale source contracts after the Transmission security fix: `scripts/check_process_lifecycle_integration.py` required `create_mp_link_status()` in every storage caller, while `scripts/check_storage_safe_remove.py` required the same direct final-path helper in `run_torrent()`. The production code intentionally routes Transmission through `transmission_mp_link_publish()`, which still uses the errno-normalizing adapter for a staged path and performs the final atomic rename.
- Updated both integration checkers to recognize the new staged publisher, assert no direct final-path unlink/create remains, and retain explicit checks that aria2 and other media callers still use the original adapter semantics. No firmware source changed in this follow-up.
- Exact SHA `00e1d2efa475a2b693e26c481f230164285f1d05`: storage hotplug `46091165116` failed only on the old direct-adapter assertion in the safe-remove checker; Process lifecycle `46091165092` failed only on its corresponding old assertion. Storage-enable `46091165081` failed only because it expected the removed unlink and was addressed in parent `b5b2339da4db5cf9c7a3e428cde5e3324028f844`. XY-C1 `46091165120` was in progress at last poll; the standalone policy C test passed locally with GNU89, warnings-as-errors, ASan and UBSan.
- Next: inspect exact-SHA Process, storage hotplug, storage-enable, XY-C1 and AI docs runs after these contract corrections; repair any genuine compile/test failure, then continue independent stability work. Backup remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Keep checked-enable source contract aligned with atomic Transmission link publishing

- Exact-SHA storage enable workflow `46091165081` on `00e1d2efa475a2b693e26c481f230164285f1d05` passed the standalone checked-NVRAM policy test, then failed its source contract because the run_torrent tuple still expected the removed `unlink(link_path)` side effect. Production code correctly now calls the staged `transmission_mp_link_publish(dest_dir, link_path)` helper. This is stale checker coupling, not a runtime or policy-test failure.
- Updated the contract to require the new publisher in the same enable-gate ordering, explicitly forbid unlinking the live path or directly creating a link at the final destination, and added the new policy/test files to the workflow's push/PR filters. No production source changed.
- Parent source SHA `00e1d2efa475a2b693e26c481f230164285f1d05`: local standalone GNU89/ASan/UBSan policy test PASS; AI handoff, firewall, NFS, Samba, vsftpd, MiniDLNA, IPv6, libdisk, Firefly and Process inventory workflows PASS. Storage-enable run `46091165081` failed only on the stale unlink expectation. USB storage `46091165116`, XY-C1 `46091165120`, Process lifecycle `46091165092`, NVRAM `46091165098`, HTTPD `46091165085`, modem `46091165096` were still in progress at the last poll.
- Next: inspect exact-SHA storage-enable, storage-hotplug and XY-C1 runs for THIS checker correction, address any actual failure, then continue another independent P1 stability issue. Backup remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Publish Transmission mount symlink without unsafe pre-unlink

- Review found `run_torrent()` unconditionally called `unlink("/mnt/transmission")` and ignored its result before scanning `/proc/mounts` and creating a replacement. A non-symlink file at that privileged path could be deleted, and the destructive operation happened before `transmission.sh` checked whether `/mnt` itself was a symlink. Mount-inventory or link-publication failures also discarded the previous valid compatibility link.
- Replaced the pre-unlink with staged publication: verify `/mnt` using `lstat` (real directory, root-owned, not group/world writable), allow only a missing destination or an existing symlink, create and validate a temporary mount-target symlink, recheck the parent device/inode/owner/mode and destination type, then atomically `rename()` over the old link. Regular files/directories are preserved and rejected; failures leave an old link intact and clean up only the staged link created by this attempt.
- Added GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy regression for parent trust, symlink/writable-parent denial, safe destination types and error preservation; updated storage source contract and XY-C1 header path filter. Exact-head tests/build remain pending; no firmware or hardware result is claimed.
- Parent `8422435ebbd011065ced47c074fd722b0f36d16f`: USB executable-policy regression `46090505594` PASS; AI handoff docs `46090505656` PASS. Earlier printer-source SHA `6939aa6b083e3fa3fa317b9c34ac0b4683d8480e`: Process lifecycle `46090400887`, Process inventory `46090401067`, USB storage `46090401078` PASS; USB checker failure `46090400869` was fixed by `8422435`; NVRAM `46090400849` and XY-C1 `46090401016` were pending at last poll.
- Next: inspect exact-SHA storage and XY-C1 checks for this Transmission slice; then continue an independent P1 userland stability task. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Correct USB executable identity test source-contract matcher

- Exact USB modem / printer workflow on `6939aa6b083e3fa3fa317b9c34ac0b4683d8480e` reached the new executable-policy regression: GNU89 + `-Wall -Wextra -Werror` + ASan/UBSan compile and run PASS (`USB printer executable identity policy: PASS`). Existing p910nd and modem-WAN policy regressions and their prior source contracts also passed.
- The workflow then failed only because `scripts/check_usb_printer_nvram.py` expected each test function call as a bare `name();`, while the test intentionally uses the repository's `CHECK(name() == 0);` convention. THIS checker-only correction whitespace-normalizes the test and asserts the exact CHECK-wrapped invocation; it does not weaken source/test assertions or alter production code.
- Parent `6939aa6` exact Actions: AI docs `46090400943` PASS; Process inventory `46090401067` PASS; USB modem `46090400869` FAIL only at the stale checker invocation matcher; Process lifecycle `46090400887`, NVRAM `46090400849`, storage `46090401078`, and XY-C1 `46090401016` were IN_PROGRESS at last poll. No firmware/L3 or hardware claim.
- Next: verify USB, Process, NVRAM, storage, XY-C1 and AI docs on this checker correction; then continue an independent P1 issue. Backup remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Correct printer executable-scan error aggregation and function boundary

- Source review after `84213bc5979cc8bdd4734b93c8e652092c539a35` caught two issues before accepting its CI: an insertion offset had left a duplicate tail after the new scan function, and one UNKNOWN candidate's readlink failure could be overwritten by a later mismatching candidate. The scan now preserves the first unknown error across all candidates, lets a confirmed matching executable prove RUNNING, and otherwise fails closed if any candidate identity could not be checked. The stale tail is removed, and the readlink buffer now accepts any path that fits with a terminating NUL.
- Cleaned the policy test's failure diagnostic to emit a real newline. Exact-head checks on `84213bc` were pending at last poll; they are not being counted as pass. No product firmware or hardware result is claimed.
- Next: inspect exact-SHA USB printer, Process, XY-C1, NVRAM/storage and AI checks for THIS corrected source. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Bind USB printer readiness to the installed executable identity

- Review of `process_inventory.c:383` found that its generic inventory intentionally accepts an exact `argv[0]` basename match even when `/proc/PID/stat` comm names another executable. Linux lets a process choose `argv[0]` at `execve()`; a local unrelated process can therefore present itself as `u2ec`, `lpd` or a per-port `p910Nd` title and make the privileged USB-printer startup check report RUNNING, suppressing a real daemon launch. A local proof using `exec -a lpd sleep 5` showed cmdline `lpd` while `/proc/PID/exe` is `/usr/bin/sleep`.
- Added a printer-specific, compatibility-scoped executable identity gate: u2ec and lpd titles must resolve to their ROMFS paths; p910nd and p9100d..p9109d titles resolve to `/usr/sbin/p910nd` (matching p910nd's intentional argv[0] rewrite). The scanner reads `/proc/PID/exe`, validates the same PID/start-time/argv identity after the read, and only treats the known installed executable as RUNNING. Readlink/identity uncertainty remains UNKNOWN/fail-closed; deleted-but-still-running instances retain their exact executable identity.
- Added GNU89 ASan/UBSan policy regressions for all title mappings, malformed titles, exact paths, spoof paths, deleted executables and invalid arguments; extended the USB printer source contract and workflow path filters. Production source and host tests have not yet run on this exact SHA; no L3/hardware claim.
- Parent SHA `375ab9f11b78d67a365b0c8885e1d5b617ebcc65`: HTTPD `46089951826`, AI handoff `46089951841`, AliyunDrive `46089951825`, and Process lifecycle `46089951824` all PASS. This new slice requires exact-head USB printer, Process, XY-C1 and AI handoff validation.
- Next: inspect those exact-SHA checks; specifically verify all U2EC/LPRng/p910nd regression and source contracts, then proceed to another independent P1 task. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Fix second duplicated PID 1 fatal-count contract

- The first correction `ace9a30007db01654ad260423e46115322fcd972` updated the earlier occurrence of the exact-six assertion, but `scripts/check_dev_init_boot.py` contains a second equivalent integration-contract block. Exact AliyunDrive run `46089841598` passed the first block and all private-lock/WebDAV regression tests, then failed at line 688 on the duplicate assertion; exact Process run `46089841549` was still running at last poll and could hit the same assertion.
- THIS follow-up changes the remaining duplicate to the same minimum-six contract. The independent switch-device assertions still verify both node-status functions and the security properties, so the weaker aggregate count does not weaken the actual new source contract.
- On parent SHA `ace9a30007db01654ad260423e46115322fcd972`: HTTPD `46089841552` PASS and AI handoff `46089841639` PASS; AliyunDrive `46089841598` FAIL (stale duplicate assertion); Process inventory/lifecycle `46089841549` PENDING at last poll. Fresh exact-SHA Process/AliyunDrive reruns are required for THIS correction.
- Next: verify Process/AliyunDrive/AI exact-head runs, then inspect remaining independent P1 queue. Switch-device source SHA `78103b86` XY-C1 run `46089366718` was still pending at last poll; no L3 claim. Backup remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Decouple PID 1 boot contract from secure switch-node fatal gates

- Exact-SHA Process lifecycle run `46089366834` and AliyunDrive WebDAV run `46089366780` on `78103b86d1e214dea0ad90cb72e9895c48b3df42` each reached the shared `scripts/check_dev_init_boot.py` source contract and failed only its stale assertion that `init_router()` had exactly six fatal returns. The two secure RTL8367/MediaTek node checks correctly added early FATAL exits. All preceding GNU89/ASan/UBSan boot-policy tests and the private-lock/WebDAV lifecycle regressions passed. No production runtime failure was observed.
- Changed the aggregate assertion to require the six established fatal return paths while the same checker separately validates both new switch-node status guards, mode/owner/type/rdev rules and init ordering. Process and AliyunDrive workflows both include this checker path in push/PR filters.
- Previous workflow-only contract fixes: exact HTTPD + AI handoff Actions on `dbbbba13342d9297f3f45ef15e956e03c92879fe` PASS (`46089569822`, `46089569786`). The exact XY-C1 validation for switch-node source SHA `78103b86` was still PENDING at last poll; 39/43 other parent jobs passed, with Process and AliyunDrive failures being the stale assertion above and HTTPD repaired on dbbb. This checker correction requires fresh exact-SHA Process/AliyunDrive checks; no L3 claim yet.
- Next: verify exact-SHA Process, AliyunDrive, AI handoff and XY-C1 checks for this correction, then resume independent P1 work. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Preserve YAML heredoc indentation in RC contract follow-up

- The initial HTTPD contract correction `9356c97e7587c6c330370a6b7981a775ecb649b6` changed the RC fatal-return threshold but placed the new Python assertion outside the workflow's indented shell block. This follow-up restores the embedded command-block indentation before running the exact-head Actions checks.
- No production source behavior changed. Exact-SHA HTTPD and AI handoff verification are pending on THIS COMMIT.

---

## 2026-11-27 — Keep RC boot-fatal contract independent of switch-device gate count

- Exact-main SHA `78103b86d1e214dea0ad90cb72e9895c48b3df42` introduced two additional fatal early-boot exits for unsafe RTL8367/MediaTek switch device nodes. The HTTPD umbrella's embedded RC NVRAM check still required exactly six fatal returns, so that unrelated contract failed even though the NVRAM policy binary and production source regression passed.
- Relaxed only the aggregate-count assertion to require the six established fatal return paths remain present; the dedicated switch-device policy contract separately verifies the new switch-node gates and status checks. No production source behavior changed.
- Workflow-only correction; no local checkout was available. Exact-main HTTPD and AI handoff Actions are pending on THIS COMMIT; no firmware build or hardware result is claimed.

---

## 2026-11-27 — Fail closed on pre-existing switch device-node paths

- `rtl8367_node()` and `mtk_esw_node()` previously issued privileged `mknod` calls and ignored their results. An existing symlink, regular file, wrong major/minor, non-root owner, or over-permissive mode made `mknod` fail with EEXIST; router initialization then continued and later switch ioctl paths could open the unvalidated `/dev` entry.
- Added a shared ensure/validate policy: create only a root-private character node (0600); treat EEXIST as a reason to inspect, not as success; use `lstat` to reject final symlinks; require root:root, one link, exact char type/mode and expected device number. Both switch node setup paths expose a status result, and `init_router()` stops with FATAL before networking initialization if an enabled switch node cannot be proven safe.
- Added policy regressions for valid nodes, wrong type/device, symlink, permissions, owner/group and link count; process-lifecycle CI compiles/runs the test and checks source integration. Local tests were not run because no checkout is available; exact-SHA Actions are pending.

---

## 2026-11-27 — Name SMP affinity errno regression cases for the boot contract

- Follow-up to `cbbca809367522b5cffe22923296aab1e21078ce`: split the focused errno assertions into named optional-node and real-write-failure test functions. This matches the source-contract checker and makes the policy cases independently readable; no runtime behavior changed.
- The exact-SHA workflow matrix for the preceding implementation was queued/in progress when this follow-up was made. Verify both SHAs; no local compiler is available here.

---

## 2026-11-27 — Propagate SMP IRQ/RPS/XPS affinity write failures

- `smp.c` previously discarded every `fput_int`/`fput_string` result while configuring IRQ affinity and per-interface RPS/XPS masks; router boot status therefore missed real kernel tuning failures. The three writers now return their status, the configuration pass attempts every eligible entry and preserves the first real error, and `ENOENT` remains optional for board/driver-specific nodes that do not exist.
- `init_router()` now logs and folds SMP affinity failures into its degraded boot status. PPP ip-up logs PPPoE RPS failures without changing the independent WAN route result. Non-SMP builds return success from no-op macros.
- Added errno-policy unit tests, source-contract assertions, and push/PR workflow filters for the SMP source/header/test. Local compilation was not available without a repository checkout; exact-SHA CI is pending.

---

## 2026-11-28 — Secure existing Redsocks config directory and repair exact source contract

- Exact-SHA CI for abf383ce1781943f598562c4b1a90210de6eb4ee: shell syntax and lifecycle regressions PASS; static source contract failed because its staging-path literal mistakenly expected one dollar instead of the shell PID suffix $$. The AI handoff contract passed.
- Code review also found that the previous edit had dropped the mode-0700 repair for a pre-existing Redsocks config directory. Startup now chmods the validated real config directory to 0700 and checks the result before changing daemon state. Added a regression starting from mode 0755 and verifying mode 0700 plus successful config publication. Static checker now checks the exact PID suffix and chmod-before-process requirement.
- Exact-SHA verification pending. No local checkout available.

---

## 2026-11-28 — Redsocks atomic staging: regression passes, source checker format corrected

- Exact-SHA run 46087370957 for 8bde38fdf22bfd43b9bd0aaa04b55bb2ce4cf225: shell syntax PASS and bounded lifecycle regressions PASS, including the staging-directory symlink rejection and publication-race test. The static source-contract step failed only because its legacy STARTED logger assertion required an unquoted constant tag, while func_start consistently quotes "$BINARY_NAME". This is an assertion-format mismatch, not a runtime regression; checker now accepts either spelling while preserving the exactly-one STARTED log requirement.
- Exact-SHA AI handoff run 46087370951 PASS. Process inventory run 46087370983 was still running when last inspected; recheck later.
- New checker update is committed separately and requires exact-SHA CI confirmation. No local checkout is available.

---

## 2026-11-28 — Replace unsafe noclobber staging with atomic directory reservation

- Exact-SHA Redsocks CI for 5387a1db85a9ed969c7f5ff3e18ea508a3a59636 failed at the pre-existing staging-symlink regression: shell noclobber redirection did not reject the symlink. Bash syntax passed; the lifecycle test stopped before source-contract validation.
- Corrected the reservation primitive to atomic mkdir -m 700 on a PID-scoped same-directory staging directory. A pre-existing file, directory or symlink now causes mkdir to fail without following it. The config file is created inside that newly owned mode-0700 directory, published with an exclusive hard link, then the stage file and directory are removed. Regression now plants a symlink to a directory and verifies its sentinel and running daemon remain untouched.
- No local checkout is available; this correction is committed separately and requires exact-SHA CI confirmation.

---

## 2026-11-28 — Atomically publish generated Redsocks configuration

- The config path was already checked for symlinks, but generation still used direct shell redirection to the destination. A symlink inserted after validation could redirect a privileged write. Generation now reserves a same-directory PID-scoped staging file with umask 077 and noclobber before process stop, writes the complete config there, then publishes through an exclusive hard link. A destination that appears during publication is not followed or overwritten.
- Regression covers a pre-existing staging symlink (no process signal and no referent modification) and a destination symlink inserted immediately before publication (link remains, referent unchanged, daemon launch blocked). Exact-SHA Redsocks, Process inventory, AI handoff and XY-C1 checks remain unverified; no local checkout is available in this environment.
- The staging step avoids relying on mktemp, which is disabled in the router BusyBox configuration.

---

## 2026-11-28 — Reject Redsocks configuration symlink paths

- The endpoint-validation fix still left a privileged write-through path: when a missing `redsocks.conf` was a dangling symlink, `[ ! -f ]` treated it as absent and the config heredoc followed the link; a symlinked `/etc/storage/redsocks` directory similarly redirected generated files.
- Added a config path preflight that requires the expected config target directly inside a real, non-symlink parent directory, and rejects a symlinked config directory, symlinked config file or non-regular target before any process stop. Directory creation and path revalidation failures now propagate. Regression covers directory and dangling-file symlinks and confirms the running daemon and referent remain untouched.
- Reconstructed the Redsocks lifecycle test from its clean pre-endpoint baseline to remove malformed duplicate test fragments left by the preceding test insertion. The endpoint and config-path cases are now each included once.

---

## 2026-11-28 — Repair Redsocks endpoint regression fixture syntax

- Exact Redsocks workflow #46085873079 for `9309abef4f9813d650ff41d6b0075f4567260fab` failed at the initial `bash -n` check because the newly inserted endpoint test loop had been truncated after its `for bad_ip in` header. The production source itself passed its preceding parse stage; no runtime tests ran. Replaced the incomplete fixture section with complete explicit newline-injection, IPv4 and port cases, plus the valid normalized endpoint case.
- Exact Process inventory #46085873084 succeeded; AI handoff #46085874162 succeeded. Re-run Redsocks and Process regression on the syntax repair commit.

---

## 2026-11-28 — Validate Redsocks generated SOCKS5 endpoints before restart

- `func_start` wrote caller-supplied address and port directly into a privileged Redsocks config heredoc. An address/port containing a newline could inject additional directives. It also stopped a healthy daemon before discovering that a missing config had no endpoint or invalid values.
- Added strict IPv4 validation, decimal port normalization/range enforcement (1–65535), and a preflight before process-stop operations. Missing config with no endpoint remains a safe no-op; malformed input leaves a running instance untouched. Regression covers newline injection, invalid/octal IPv4, port injection, zero/out-of-range/non-numeric ports and leading-zero normalization.

---

## 2026-11-28 — Reject symlinked Transmission storage roots

- `func_start` previously checked only that `/mnt/transmission` was a directory before deriving and permission-changing its config/download descendants. A symlinked storage root (or symlinked immediate parent) still passed the directory test and allowed privileged chown/chmod to follow the link into attacker-controlled storage.
- Added a dedicated root preflight that rejects a symlinked root or parent and preserves the previous missing-root failure behavior. Regression covers a symlinked root, a symlinked parent, a real root and a missing root without creating targets.
- SmartDNS directive-injection Actions are still in progress at SHA `380918ce60cb7d955d57b90ca42868b1df964de5`; this Transmission slice is independent and is validated by the process lifecycle workflow.

---

## 2026-11-28 — Preserve legacy empty SmartDNS threshold while disabled

- Reviewing the SmartDNS DNSMasq integration fixture exposed that `sdns_ip_change_time` is a conditional directive value, not an always-emitted numeric option. Validate it only when `sdns_ip_change=1`; otherwise older NVRAM without this optional value remains compatible. Added regressions for disabled empty and enabled empty threshold states.
- The previous exact DNSMasq run on `53c09530c676ef0096ca2a89a2175212cec47fd0` showed the integration fixture omitted the production config-token helper; commit `a0cba1e2efe18728173493e0fa74b0b3b4956854` now sources it. Re-run the full DNSMasq workflow to validate both corrections.

---

## 2026-11-28 — Load SmartDNS config-token helper in DNSMasq integration regression

- Exact DNSMasq lifecycle run #46085311741 for `53c09530c676ef0096ca2a89a2175212cec47fd0` passed the process, adblock, firewall and resolver ownership checks, then failed the final runtime config integration because its extracted-function fixture sourced the DNSMasq helper but not the newly required `smartdns_config_file.sh` token validator. The production launcher already loads that helper; the fixture now mirrors the production dependency.
- The exact-SHA run verified the new token-validation code up to this fixture failure. Re-run DNSMasq lifecycle after this test-harness repair; AI handoff #46085311728 passed. Process inventory, USB storage and XY-C1 were still in progress at the last poll.

---

## 2026-11-28 — Reject SmartDNS NVRAM configuration-line injection

- `Get_sdns_conf` interpolated base settings and static-upstream address/group/port values into daemon directives without enforcing single-token boundaries. Newlines could append attacker-controlled SmartDNS directives. The generator now validates base tokens, numeric fields and selectors before truncating staging, then validates static upstream endpoints and options before emission.
- Added regressions for injected server-name, speed-check-mode, cache-size, static endpoint, group and port values. The DNSMasq lifecycle workflow runs this generator test; exact-SHA verification follows the commit.

---

## 2026-11-28 — Align checked NVRAM startup contracts with CI fixtures

- The FRP boot-state hardening changed the shared autostart reader to `nvram get_checked`, but the optional-start fixture still accepted only legacy `get`. Its selectors consequently appeared disabled and AdGuardHome was not invoked. Updated the fixture to model the checked API.
- The AliDDNS transport checker also expected the old unchecked command in the shared autostart helper; aligned that contract. Added part-039 to the archive index table with the repository-root-relative path required by the inventory checker.
- Exact base SHA `71d3bf2a3788f2ce59c1e5809c383a26841b2657`: Process lifecycle #46084656129 failed only at the stale autostart fixture; AliDDNS #46084656095 failed only at the stale source assertion. Re-run both against the repair commit.

---

## 2026-11-28 — Complete DHCPv6 crash-durable configuration publication

- DHCPv6 IA configuration publication fsyncs staged contents after `fflush`, closes the file, atomically renames it, then fsyncs the parent directory. Post-rename parent-sync errors are reported because crash durability is uncertain although the new name may already be visible.
- The staged-config regression covers successful publication, missing-parent and symlink-parent paths, and injected parent-directory-sync failure. The initial exact-SHA run on `8b1aa83d2f82da8743350bdc2aa056ebfa764649` exposed a test expectation mismatch after the injected post-rename failure; `0eef8eb09fd535d6266426ec1fa8a0b5d3ed8093` corrected the expected on-disk content. Inspect the exact-head rerun and companion WAN IA callback regression before claiming PASS.
- Concurrent WAN IA callback changes were preserved. No firmware L3 or physical L4 result is inferred from workflow completion alone.
- Next: inspect exact-head DHCPv6 publication/WAN IA tests, Process inventory and AI handoff; close outstanding XY-C1 Build → TRX → artifact chains for modem/OpenVPN, then continue an independent P1 issue.

---

## 2026-11-28 — Make USB modem identity metadata durable

- `modem_node_file_write()` already fsynced the staged metadata file but omitted parent-directory fsync after atomic rename. `modem_node_file_remove()` omitted it after unlink; power loss could lose a new USB identity or resurrect stale `wwanX`/WDM metadata. The directory prepare path, publisher and remover now sync their containing directory and propagate failures.
- Extended the existing C regression for successful/missing/symlink parent behavior and added source-order assertions to the USB modem identity workflow.
- Parent SHA: `48b617d5933f9b7bb88fe8ed194b1682985d3d7d`. Exact USB modem identity, Process inventory, AI docs and XY-C1 Build/TRX/artifact pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Persist OpenVPN credential and config publication

- OpenVPN client credential and config publishers previously fsynced staged file contents but did not sync the containing directory after rename. Both now call the shared parent-directory fsync helper and propagate errors; the helper refuses symlinked parent directories via `O_NOFOLLOW`.
- Extended both ASan/UBSan C regressions for successful parent sync, ENOENT missing-parent failure and symlink-parent rejection. OpenVPN workflow push/PR filters and source checker require the common helper and sync-after-rename ordering.
- Parent SHA: `00234eecd99526edb36f7a91581ec0614716c085`. Exact-head OpenVPN/Process inventory/AI docs/XY-C1 Build/TRX/artifact checks pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Repair media-config directory sync CI contract

- e0 Process regression #46082405947 failed at the existing MiniDLNA/Firefly source contract because it expected the pre-hardening `O_RDONLY | O_DIRECTORY` open form; the shared helper now correctly uses `O_RDONLY | O_DIRECTORY | O_NOFOLLOW`. The newly added MiniUPnP and IGMP compile/regression steps were skipped by the earlier assertion failure.
- Update the static contract to require `O_NOFOLLOW`; exact-SHA rerun will execute the new proxy configuration tests.
- Base SHA: `e0cc8d5834cc26b2527176a0e0fd1dfd1095f8dd`. Process inventory #46082405929 and AI handoff #46082405924 PASS. e0 XY-C1 #46082405926 still pending Build/TRX/artifact; NVRAM audit #46082405853 still running.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Persist MiniUPnP and IGMP proxy config renames

- MiniUPnP and IGMP proxy staged configuration writers fsynced file contents before atomic rename but did not sync the parent directory. They now reuse the shared parent-directory fsync barrier after rename and propagate errors instead of reporting success when the directory entry is not known durable. The helper refuses symlinked parent directories with `O_NOFOLLOW`.
- Extended both C regressions for successful parent sync, missing-parent failure and symlink-parent rejection. Added MiniUPnP C compile/run to Process lifecycle workflow; IGMP proxy compile/run remains, and a static contract checks each publisher performs the directory barrier after rename.
- Parent SHA: `91db105a946c0397c9875a1090cfc8fa1d0495fd`. Exact-head Process regression/Process inventory/AI docs and XY-C1 Build/TRX/artifact pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Persist MiniDLNA and Firefly config renames

- MiniDLNA and Firefly each fsynced staged config contents before atomic rename but did not fsync the containing directory. File fsync alone does not guarantee the new directory entry survives sudden power loss. Added shared `config_file_sync.h` with EINTR-safe file and parent-directory fsync; both publishers now call it after rename and propagate failures.
- Added success and ENOENT missing-parent cases to both C regression programs. Process lifecycle workflow checks that directory sync follows rename and compiles/runs the existing ASan/UBSan tests. The new shared header is included in push/PR path filters.
- Parent SHA: `d3090c3a55ebda73b2680055e04f7324bfe56565`. Exact-head Process inventory, AI docs and XY-C1 Build/TRX/artifact pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Enable BusyBox ash command builtin for firmware lifecycle scripts

- Storage #46081098105 first exposed Transmission's `command -v stat` failure; after applet PATH diagnostics, #46081453045 on `e69afd9` made the root cause explicit: BusyBox ash reported `command: not found`. The common board config disabled `CONFIG_ASH_CMDCMD` even though Transmission, Aria2 and shared process lifecycle scripts use `command -v`.
- Enabled `CONFIG_ASH_CMDCMD=y`; Storage CI now requires the feature in the built and source config and runs an ash smoke through the exact `STUB_DIR:$PATH` before starting the launchers. Exact SHA `25b0f34f41206b0d16ceed7b8190202a53b89d63`: USB storage #46081670821 PASS (including BusyBox 1.24.2/ash and all remaining storage regressions); Process #46081670853 PASS; AI docs #46081670826 PASS. AliDDNS #46081670844, AliyunDrive #46081670881, DNS Forwarder #46081670945, FRP #46081671085 and AdGuardHome #46081670829 PASS. NVRAM #46081670831 and XY-C1 #46081670815 were still running; Build XY-C1 was active and TRX/artifact pending.
- Test harness: `9bf676f` linked stat in STUB_DIR; `7b05465` added an exact ash PATH smoke; its duplicate symlink was removed in `e69afd9`. No production fail-closed check was relaxed.
- No exact-current-source L3 firmware claim until #46081670815 Build, TRX and artifact upload all pass; no hardware L4 result.

---

## 2026-11-28 — Make BusyBox stat visible in storage launcher stubs

- Exact d79 USB storage #46081098105: Aria2 directory/file symlink regression and Transmission/Aria launch-status regression PASS; the BusyBox 1.24.2/ash step compiled BusyBox, ran Optware lifecycle tests, then failed at Transmission process identity because `command -v stat` returned unavailable. Workflow applet PATH already included stat and direct smoke check, but the test harness prepends a separate `STUB_DIR` to the BusyBox launcher's PATH, so the actual child could not resolve the applet.
- Link the actual built Busybox binary as `STUB_DIR/stat`; add an ash smoke using the exact `STUB_DIR:$PATH` form. No production logic or fail-closed checks are weakened. d79 AI docs #46081098106 PASS; d79 XY-C1 #46080742734 Build was IN_PROGRESS, TRX/artifact pending. eab6 XY #46080815493 reports success but its Build/TRX/artifact steps were skipped, so it is not an L3 result.
- Parent SHA: `9bf676f90ae2ee58dcaef408015d57fd32bbf056`. Exact USB/ash, Process inventory, AI docs and XY-C1 checks pending. No local ash test claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Block Aria2 storage symlink redirection

- Aria2 startup previously followed symlinks for `config`, the download directory and `incomplete.lst`; privileged startup can touch session files, publish RPC secrets/configuration and recursively chmod/chown these paths. Added directory checks for existing/dangling symlinks and rechecking after `mkdir -p`, plus regular-file checks for `aria2.conf` and `incomplete.lst` before `touch`. The configured download path is checked before recursive chmod.
- Added `tests/storage_path_symlink.sh` with real/new path, existing/dangling symlink and sentinel-preservation cases, and verified guard order before side effects. Wired into storage-hotplug push/PR CI.
- Parent SHA: `284e35183959b8cc78d28c82959e5e8815774531`. Exact storage-hotplug/Aria process/Process inventory/AI docs/XY-C1 checks pending; no local shell test claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 07:00 CST (UTC+8) — Make iperf3 process discovery fail closed

- `iperf3_process_state()` previously accepted BusyBox `pidof` status 1 with empty output as confirmed ABSENT. BusyBox 1.24 can omit a running task when its /proc scan is incomplete, allowing a duplicate iperf3 start or premature stop success.
- Reused the always-built shared `process_lifecycle.sh` tri-state scanner and its full-procfs/post-scan confirmation. Added fake-procfs integration coverage for RUNNING, UNKNOWN, stop verification, and confirmed ABSENT. Exact workflow #46079911385 exposed a test-only permission error (tracked launcher mode is not executable); follow-up `6d659d767e8ac236527ca5cf02f2daedaf386507` runs it through `sh`.
- Source commit: `3832167000112d33e6c2d30dbb8c0008ba29e119`. Test correction commit: `6d659d767e8ac236527ca5cf02f2daedaf386507`. Exact iperf3 rerun #46079981839 and Process inventory #46079981918 were queued; source-SHA inventory #46079911367 passed. XY-C1 #46079911435 was still building; no firmware artifact or hardware L4 claim.
- The independent Transmission TLS check passed on `9972e8531add4ce5bc64c3d5690892e30b80ade4`; its broader Process workflow was cancelled after that step. USB storage hotplug #46079785505 separately failed during BusyBox 1.24.2/Optware ash regression and remains to diagnose. No local shell test was executed in this connector-only session.

## 2026-11-28 — Restore Transmission HTTPS tracker certificate verification

- `transmission.sh` previously exported `TR_CURL_SSL_NO_VERIFY=1`; Transmission 3.00 treats this variable as an opt-out from HTTPS tracker certificate and hostname validation. The launcher now clears any inherited value before startup and keeps verification enabled by default.
- Added `tests/tls_verification.sh`: it injects the opt-out into the environment, sources the exact launcher preamble, verifies the variable is cleared and statically rejects any forced assignment. Wired into Process lifecycle push/PR checks. The XY-C1 workflow already watches the launcher.
- Base SHA: `20e0a312051addf538ed6bf4ae85022e608ed4b3`. Exact-head Transmission/Process/AI and XY-C1 Build/TRX/artifact checks are pending. No local shell test claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 03:55 CST (UTC+8) — Correct Shadowsocks procfs source contract and handoff byte guard

- Exact SHA `f23be0cff7d7e0d1e942ebcf10094497b73be676`: Shadowsocks lifecycle #46079286423 passed shell syntax and all runtime regressions, then failed only because the Python source checker expected incorrect escaping for the literal procfs path expansion and `tr` delimiter. This checker-only correction now matches the actual shell source.
- AI handoff #46079286331 failed because the roadmap exceeded its UTF-8 hard limit by 317 bytes; compressed the new top roadmap status heading. No production behavior changed.
- Process inventory #46079286495 PASS. The corrected checker and documentation contract require new exact-push runs. XY-C1 #46079286427 is still the required production-source L3 gate; no local test or hardware L4 result is claimed.


## 2026-11-27 — Confirm Shadowsocks ABSENT lookups with complete procfs scans

- BusyBox 1.24 `procps_scan` skips entries when stat or cmdline reads fail; `pidof`/`pgrep` can consequently return status 1 despite a live matching process. Shadowsocks previously accepted that as ABSENT.
- Added a mode-aware procfs inventory for named and `pgrep -f` lookups. It validates numeric PID/stat records, live task state and cmdline readability; mirrors BusyBox short-comm/argv0/argv1 name matching and pgrep's empty-cmdline comm fallback; detects malformed/inaccessible entries as UNKNOWN; and performs a fresh lookup after a clean zero-match scan.
- Added fake-procfs regressions for both false-negative lookups and malformed stat input. Exact-SHA Actions pending; no local shell test claimed.

## 2026-11-27 — Bind Redsocks stop signals to captured process identities

- Replaced Redsocks name-wide `killall` TERM/KILL with exact-PID signalling. Each candidate must match the configured executable via `/proc/PID/exe`, match the process comm, and retain the same `/proc/PID/stat` field-22 start time across separate reads. Stop retains the first PID/start-time snapshot and refuses signalling if a new PID appears or a PID is recycled before either signal.
- Extended `redsocks-lifecycle.yml` regression coverage for exact PID signals, foreign same-comm executable rejection and PID reuse between TERM and KILL.
- Exact-SHA Actions on 7e1557690aa6aa996007b5ed4e1aecd170b44eed: Bash syntax and Redsocks runtime regression PASS; static source contract FAIL was a checker bug (stat field 22 belongs to `redsocks_read_stat_identity`), corrected in the follow-up. Process inventory PASS; AI docs FAIL was roadmap byte-limit overrun, shortened in the follow-up. No local shell run is claimed. Linux 4.4 has no pidfd, so a small validate-to-signal race remains; physical L4 is NOT VERIFIED.

## 2026-11-27 — Reject symlinked SSH host-key storage directories

- Both OpenSSH and Dropbear previously validated individual host-key leaf paths but accepted a configured storage-directory symlink. A root-started daemon helper could therefore chmod, delete, generate or publish keys/configuration in an unintended target directory. Added no-follow directory checks and post-mkdir revalidation; all key/config/argument operations are gated before they follow the configured leaf directory.
- Extended Dropbear runtime coverage to symlink, dangling-symlink and non-directory stores, including preservation of outside keys/extra_args and no keygen/start. Extended OpenSSH launch regression to prove a symlinked store cannot mutate external host keys or publish sshd_config. Added exact source-contract assertions.
- Process lifecycle, AI handoff and XY-C1 Actions are required on the exact implementation SHA. Local full-repository execution is unavailable in this GitHub-connector-only session; do not claim local shell results or physical-router L4.

## 2026-11-27 — Correct OpenSSH required-host-key source contract

- Process lifecycle regression #46073989322 on `2ffccd914287ad96840fd92aeb850ed49c828f56` reached `check_optional_app_start_status.py` after preceding lifecycle and compiled regression cases passed, then failed because its OpenSSH source assertion expected a `case` statement that the implementation does not use. The intended branch is an explicit `if [ "$key_requirement" = "required" ]`; the omitted-DSA generation fixture already verifies that missing required keys prevent sshd launch.
- Updated the checker to assert the actual required-key branch and its missing-key diagnostic. No product source behavior changed. Exact-push Process regression on the new commit is required; host execution is not available in this connector-only session.
- On the source baseline SHA `2ffccd9`, Process inventory #46073989223 PASS and XY-C1 Firmware Validation #46073989253 PASS, including Build XY-C1, TRX verification, and artifact upload. Physical-router/L4 remains NOT VERIFIED.

## 2026-11-28 — Quote AliDDNS provider TTL data before testing or persisting defaults

- The AliDDNS startup block used `[ -z $aliddns_ttl ]` and an unquoted `nvram set` expansion. Since NVRAM can be written by scripts or direct requests, values containing spaces or glob characters could be split or expanded by the shell.
- Added a quoted default helper preserving non-empty values and applying 600 only to an empty value. Added regressions for empty, wildcard, whitespace and numeric TTL inputs; static checker pins the helper and quoted persistence.
- Exact-SHA AliDDNS transport/AI Actions pending; local shell execution unavailable in this connector-only session.

## 2026-11-28 — Align XUPnP source checker with shared DNSMasq pair transaction

- DNSMasq exact-SHA run #46065129548 on `b0ccae9ca4fae122093a48790211f8526278a129` completed the `dnsmasq_file.c` GNU89/Werror/ASan/UBSan compile and runtime regression successfully, then failed in `scripts/check_xupnpd_config_pair.py` because the checker still expected pair-specific backup/restore calls in `dnsmasq_file_publish_pair()`.
- Updated the checker to verify that the XUPnP pair wrapper delegates both original path/backup tuples to the shared two-member set transaction, and that the set helper creates per-destination snapshots, includes a failed-but-renamed member in rollback, then restores published paths in reverse order. This is checker-only; production source is unchanged.
- Exact push rerun required on this commit. Host-side execution unavailable in this connector-only session.

## 2026-11-28 — Roll back DNSMasq outputs whose rename completed before a durability error

- `dnsmasq_file_publish_internal()` reports a parent-directory fsync failure after a successful `rename()`, correctly exposing that the new pathname is already visible but not durably committed. The multi-file `dnsmasq_file_publish_set()` and `dnsmasq_file_publish_pair()` callers previously incremented their rollback count only on a zero return, omitting the failing-but-renamed output. That could leave a mixed generation of DNSMasq configuration after an error.
- Added `dnsmasq_file_publish_set_with_ops()`; it tracks `rename_completed` independently from the callback return and rolls back the failed member plus prior successful members in reverse order. The public pair helper now routes through the same transaction implementation.
- Added GNU89 host regressions that inject EIO after the first pair rename and after the second of three set renames, then verify every prior destination content is restored. Extended the DNSMasq workflow's source contract to pin the rollback accounting. Exact-SHA Actions pending; local compiler execution unavailable here.

## 2026-11-28 — Add WAN IPv6 teardown header to XY-C1 build path filters

- Added trunk/user/rc/src/wan6_teardown_policy.h to the XY-C1 push and pull-request path filters because it is included by the production WAN IPv6 implementation. A header-only change to this helper must trigger the cross-build.
- Added a detector unit test requiring exactly two workflow entries and ensuring it is treated as a firmware input. No production behavior changed.

