## 2026-11-26 — Keep DNSMasq rollback generation until daemon readiness is confirmed

- Confirmed a transaction-boundary defect in `start_dns_dhcpd_locked()` and `restart_dhcpd_locked()`: static-host output rollback snapshots were committed before the detached dnsmasq daemon passed its RUNNING postcondition. Successful staging/ARP apply did not prove the child remained alive; early commit could discard the only recovery state for an unready generation.
- Production fix `f7b61ba09b9750b3e85c3884b6101a511a12c6e1` keeps the static-host file and ARP snapshots through launch and `dnsmasq_confirm_started()`. Failure invokes a shared rollback helper: it stops/rechecks the candidate; prior files/ARP are restored only on a complete ABSENT process snapshot. RUNNING/UNKNOWN preserves the matched generation and private rollback links. The original launch/postcondition error remains authoritative.
- Checker repairs: `f6de10e6...` updated compacted helper-call matching; `9166a3d4...` corrected helper and success-commit ordering; `3147c02...` corrected inline workflow assertions to require rollback before success commit. DNSMasq #45814219448 PASS and AI handoff #45814219362 PASS on `3147c02f2bddba93a6034e529c5d8935b2768a05`. Current main later received comment-only manual-recovery logging clarification `992bdf6...`; its exact DNSMasq #45814745710 and XY-C1 #45814745724 were pending at this checkpoint.
- Same product-source SHA `f7b61ba...`: NVRAM #45813611887, WAN DHCP #45813611721, Process inventory #45813611693, miniupnpd #45813611534, WAN route #45813611530, Process #45813611502, HTTPD #45813611375, Samba #45813611309 and Inadyn #45813611245 PASS. No L3 claim; hardware L4 NOT VERIFIED.

---

## 2026-11-26 — Stop QMI WDS using the persisted session client ID

- QMI teardown without an explicit client ID may allocate/use a fresh WDS CID, which does not identify the persistent packet-data session established under the saved CID. Treating that command as success could discard the only marker for the still-active session.
- Production fix `4bfccc2e7c46df3495544ae0bb46f473775352f0` reads the persisted WDS CID and PDH marker, rejects unreadable/corrupt/inconsistent state before side effects, and supplies `--set-client-id wds,<saved-CID>` before `--stop-network`. After stop it checks PDH marker removal, releases that exact CID, then removes the CID marker. Each failure preserves recoverable state.
- Follow-up checker/workflow fixes culminated in `bdc8992f6e9a7a6884422c9af3c88177abc700c5`; exact USB modem #45813115174 PASS and AI handoff #45813115104 PASS. Host/CI L2 only; live modem hardware and WAN tests remain unverified.

---

## 2026-11-26 — Require truncated process comm prefix to agree with argv1 fallback

- Found a false positive in `proc_read_identity()`: for any process with a truncated `/proc/PID/stat` comm, an unrelated executable could be counted as a target service solely because its second argument (`argv1`) matched the requested process name. For example, a long-named worker invoked as `worker dnsmasq` could make a dnsmasq absence check report RUNNING.
- Tighten the fallback: `argv1` may disambiguate only when the first 15 bytes of the truncated comm match the requested process-name prefix, in addition to exact basename equality of `argv1`. Normal `argv0` and untruncated comm matching remain unchanged.
- Add GNU89 regressions for an unrelated long worker with `argv1=dnsmasq` (must be ABSENT) and for a matching long process name whose truncated comm prefix plus exact `argv1` identify it (must be RUNNING). The existing Process workflow compiles the scanner/test with GNU89, Werror, ASan and UBSan.
- Parent main: `4f688607c0ef60f829be7454c9c6ca891c9bf77f`; resulting commit: THIS COMMIT. No local compiler run or MIPS/firmware result is claimed; exact-head Process Actions and XY-C1/L3 remain pending.
- Beijing time at preparation: 26/11/2026, 19:17:48 (UTC+8).

---

## 2026-11-26 — Align ZeroTier source contracts with cached Moon preflight

- Exact-head ZeroTier lifecycle #45807554002 on `15bced4fa9c4b24b461ee663f062a49207f036ba`: the shell regression completed successfully, including the new Moon address/identity read-failure, invalid-selector, old-daemon-preservation and PPP fallback cases. The Python source checker failed on two obsolete expectations only: the marker list expected expanded shell loop text, and the removal function was required to perform a direct NVRAM read despite the read being intentionally moved into the shared preflight.
- Update the checker to assert the shell loop covers both `zerotiermoon_ip` and `zerotiermoon_id`, retain the running-daemon preservation assertions, and require `remove_moon()` to consume its preflight snapshot without a late read.
- Parent main: `15bced4fa9c4b24b461ee663f062a49207f036ba`; resulting checker-only follow-up commit: THIS COMMIT. Recheck ZeroTier lifecycle and AI handoff on the exact resulting SHA.
- Beijing time at preparation: 26/11/2026, 19:07:46 (UTC+8).

---

## 2026-11-26 — Preflight ZeroTier Moon inputs before stopping the current daemon

- Found a lifecycle gap in `start_zero()`: it read the five general selectors and route/NAT settings before `kill_z()`, but the Moon branch read `zerotiermoon_ip` and `zerotiermoon_id` later inside `start_instance()`, after the old ZeroTier daemon was already terminated. A read failure, invalid configured/PPP fallback IPv4 endpoint, or malformed Moon ID could therefore abort replacement startup while unnecessarily leaving the service down.
- Added a bounded Moon preflight for create mode and remove mode. It reads the relevant checked NVRAM values, resolves the PPP WAN fallback when the configured address is empty, validates IPv4 or the empty/10-hex Moon identity selector, and stores the values before any process/network side effect. `start_zero()` only exposes the snapshot to the one replacement `start_instance()` invocation; direct `creat_moon()` / `remove_moon()` calls still perform their own fresh preflight and do not consume stale cache values.
- Extended lifecycle regressions for Moon address and ID read failures, invalid Moon IPv4 and removal ID, preservation of a running daemon/no signals/no network mutation, and a valid PPP WAN address fallback. Updated source contracts to require all Moon reads and validation before `kill_z()` and prohibit late NVRAM reads in the mutating Moon handlers.
- The previous exact-head checks on `6fd35a9b57fc6aa85b95c52c62a1961a6c1ac373` are VPN lifecycle #45805581756 PASS and AI handoff #45805581869 PASS. Product L3 was verified only on earlier production SHA `f96dce30881937a686e6bc7e9407a41d47f27c2e` (#45805039346: Build + TRX + artifact PASS); this new ZeroTier source SHA requires its own tests and is not claimed L3-verified. Hardware L4 remains NOT VERIFIED.
- Beijing time at preparation: 26/11/2026, 19:06:27 (UTC+8).
- Parent main: `6fd35a9b57fc6aa85b95c52c62a1961a6c1ac373`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Fix the VPN marker checker by using one normalized source view

- Exact SHA `6edf19ecd1a31332ebc3e23465861ab9399099e8`: AI handoff #45805256973 PASS; VPN lifecycle #45805256951 compiled and passed all C policy regressions, then failed at the marker parser source assertion. The prior checker-only correction used a raw regular expression with double-escaped metacharacters, so it searched for literal backslashes rather than accepting the C function's wrapped formatting.
- Replace the regex with a whitespace-free view of the extracted launcher and compare the parser, strict 0/1 validation, process scan, atomic config publication, ownership policy, marker write and ownership assignment in that single coordinate space. This also makes the checked side-effect order explicit.
- No production C or policy semantics changed. Rerun exact-head VPN lifecycle and AI handoff; inspect the remaining f96dce workflow results and the current product-build boundary before further source work.
- Parent main: `6edf19ecd1a31332ebc3e23465861ab9399099e8`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Make the VPN marker source contract accept wrapped parser calls

- Exact-head VPN lifecycle #45805039088 on `f96dce30881937a686e6bc7e9407a41d47f27c2e` compiled and passed all C policy regressions, including the newly added existing-owner/no-owner marker cases. The final Python contract alone failed because the checked `rc_wan_nvram_parse_int_value()` call was intentionally formatted across two C lines while the checker matched a single-line substring.
- Replace that brittle substring lookup with a whitespace-tolerant regular expression; retain the later order assertions for successful parsing, strict 0/1 validation, ownership policy, marker write and ownership flag.
- This is a checker-only correction. VPN lifecycle and AI handoff must be checked again on this exact new SHA. The same production SHA's HTTPD #45805038950, WAN route #45805039104, AI #45805039195, WAN DHCP #45805039154 and Process inventory #45805038990 PASS; Process fail-closed #45805039322, NVRAM #45805039114 and XY-C1 #45805039346 were still running at last poll.
- Beijing time at preparation: 26/11/2026, 18:51:35 (UTC+8).
- Parent main: `f96dce30881937a686e6bc7e9407a41d47f27c2e`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Preserve pre-existing VPN-client ownership across shared xl2tpd start failures

- Audited every caller of `safe_start_xl2tpd_locked()` after the previous rollback fix. The helper unconditionally wrote `l2tp_cli_t=1` whenever the configured VPN client used L2TP, then marked the value as newly published. If an active VPN-client dependency had already published that marker, a later WAN PPP or VPN-server start failure would clear the existing marker as if this call owned it. Subsequent shared-daemon stop/restore logic could then miss a live client dependency.
- Extend the checked xl2tpd NVRAM transaction snapshot with `l2tp_cli_t`; parse and validate the prior state before process/config side effects. A pure policy gate now publishes only for `use_vpnc && marker_active == 0`; rollback ownership is recorded only after that transition succeeds. A pre-existing marker remains untouched on daemon failure. A newly acquired marker still rolls back, and the direct VPN-client caller retains cleanup for its pre-published marker.
- Add GNU89 policy cases for inactive/new/existing marker ownership and strengthen the VPN source contract to require the ninth snapshot field, strict marker validation, and snapshot → policy gate → write → owned-state ordering. Exact-head VPN lifecycle #45805039088 ran the full C regression set successfully, including the new marker policy cases, then failed only because the source checker expected the multi-line parse call on one line. The checker correction is committed next; the same SHA's HTTPD #45805038950, WAN route #45805039104, AI #45805039195, WAN DHCP #45805039154 and Process inventory #45805038990 PASS. Process fail-closed #45805039322, NVRAM #45805039114 and XY-C1 #45805039346 were pending at last poll; no L3 or hardware result is yet claimed.
- Beijing time at preparation: 26/11/2026, 18:50:09 (UTC+8).
- Parent main: `ce917d69b0436033f4c8c83384dcd0a9b61deb5c`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Rebind WAN PPP and HTTPD contracts to atomic xl2tpd publication

- Exact SHA `1d33cb41aca31eab3d58621a243272dc9cbd0000` exposed two more stale contracts after shared `/etc/xl2tpd.conf` publication moved to `l2tp_config_file_publish()`: WAN route run #45803174983 still compared protocol-read failure against the removed `fopen(l2tp_conf, "w")` position; HTTPD run #45803174928 raised `ValueError: substring not found` for the same obsolete anchor and still expected direct `fprintf` calls inside the launcher. The C policy regressions and the preceding independent HTTPD checks passed; these are checker-location failures, not evidence of a production regression.
- Update `scripts/check_ppp_wan_options.py` to require protocol read failure to return before the atomic publisher, reject the old truncate/unlink writer, and require the publisher helper. Update the HTTPD inline PPP confidentiality contract to inspect `xl2tpd_config_write()` for the preserved LNS/LAC sections and credentials, while checking launcher preflight order and atomic publication. Preserve all existing behavior assertions.
- The pool arithmetic contract has already been corrected in `aa1bd8367768ab66f62a22834861d05df41af0e6`; exact-head VPN lifecycle #45803534463 and AI handoff #45803534499 PASS on that SHA. This commit's WAN route, HTTPD and AI exact-head results must be checked after push. XY-C1 #45803175068 on product source SHA `1d33cb41aca31eab3d58621a243272dc9cbd0000` remains IN PROGRESS at Build; TRX and artifact upload pending. Older product SHA `3c36c0a0f780657b42e7b9d804064fadd9658c3c` has a successful same-SHA Build/TRX/artifact run #45801858571, but does not verify the later source SHA. Hardware L4 NOT VERIFIED.
- Beijing time at preparation: 26/11/2026, 18:44:11 (UTC+8).
- Parent main: `aa1bd8367768ab66f62a22834861d05df41af0e6`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Bind VPN pool address source assertions to the extracted atomic writer

- Exact-head VPN lifecycle run #45803175004 on `1d33cb41aca31eab3d58621a243272dc9cbd0000` passed every preceding GNU89/`-Werror`/ASan/UBSan policy binary, including the xl2tpd process-presence and rp-L2TP atomic config regressions, then failed in the final Python source contract. The old checker still searched `safe_start_xl2tpd_locked()` for the VPN pool address arithmetic after that arithmetic had intentionally moved into the new `xl2tpd_config_write()` callback. This was a stale source-contract location, not a test or production failure.
- Normalize the callback's whitespace and verify that both start/end addresses are derived from its validated `config->vpn_server_address`, `config->vpn_server_mask`, and pool offsets. No production source or policy semantics changed.
- Exact-head AI handoff #45803174976 PASS on the failed-checker SHA. The checker-only follow-up `aa1bd8367768ab66f62a22834861d05df41af0e6` then passed VPN lifecycle #45803534463 and AI handoff #45803534499. On the atomic publisher follow-up `ce917d69b0436033f4c8c83384dcd0a9b61deb5c`, WAN route #45804117385, HTTPD security #45804117258 and AI handoff #45804117195 all PASS. XY-C1 #45803175068 was still in progress before Build XY-C1 on product source 1d33, with TRX and artifact upload pending. Older product SHA 3c36 subsequently passed Build, TRX and artifact upload in #45801858571; this does not validate the later source SHA.
- Parent main: `1d33cb41aca31eab3d58621a243272dc9cbd0000`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Publish the shared xl2tpd configuration atomically

- Found that the shared xl2tpd writer still used `fopen("/etc/xl2tpd.conf", "w")`, unchecked individual `fprintf` calls, and then unlinked the live config if flush, close, or chmod failed. A full filesystem or interrupted write could therefore destroy the last-known-good config after truncating it; an existing symlink would also be followed.
- Reused the existing no-follow, same-directory `l2tp_config_file_publish()` helper for the shared daemon config as well as rp-L2TP. A dedicated writer callback now propagates formatting/write errors; the helper stages a private regular file, flushes/fsyncs it, applies the established 0644 mode, and atomically renames it over the target only after success. Failed staging leaves the previous live config intact and removes the temporary file. Checked NVRAM/process/pool preflights remain before publication.
- Extended the VPN lifecycle source contract to require all three existing config sections and inputs in the callback, reject the old truncating writer/unlink path, and enforce checked-snapshot/process/pool order before atomic publication. The existing GNU89 ASan/UBSan atomic-config regression already covers symlink-target protection and ENOSPC rollback. Exact-head VPN and AI workflows are required; no local firmware build or hardware result is claimed.
- Parent main: `cce2c18a0ba3a90570a733c526d900b47e5eec3e`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Fix the VPN-client marker regression helper return type

- Exact-head VPN lifecycle run #45801858401 on `3c36c0a0f780657b42e7b9d804064fadd9658c3c` exposed a test-only compile failure before the source checker: the new named regression helper used the suite's `CHECK` macro (which returns `1` on failure) but was declared `void`, triggering `-Werror=return-mismatch`. The production marker rollback patch was not reached by this run.
- Change the helper to return `int`, return zero after all cases pass, and assert its result from `main`. No production source or policy semantics changed.
- The corrected VPN workflow and other exact-head Actions must run on this commit; do not carry forward the failed parent result. The 3c36 parent test stopped at the xl2tpd policy compile step, so later workflow steps were skipped.
- Parent main: `3c36c0a0f780657b42e7b9d804064fadd9658c3c`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Roll back the VPN-client marker when shared xl2tpd launch fails

- Continued the shared L2TP lifecycle audit and found an ownership leak in `safe_start_xl2tpd_locked()`: when the coherent config says the VPN client should use xl2tpd, it publishes `l2tp_cli_t=1` before launching the daemon. If process inventory confirmed absence but `eval("/usr/sbin/xl2tpd", ...)` then failed, the helper returned without clearing the marker. The direct VPN-client caller had its own rollback, but WAN PPP and VPN-server callers of the same helper did not; they could leave a phantom VPN-client dependency that affected later shared-daemon stop/restore decisions.
- Track whether this invocation successfully published the VPN-client marker. On unsuccessful completion, clear only that marker, log rollback failure, and restore the original daemon-start errno. An already-running daemon remains a successful outcome and retains the marker. Added a pure policy helper, GNU89 regression cases, and source-order assertions for publication → completion failure → rollback → errno restoration.
- Exact-head VPN lifecycle run #45801196538 on prior checker-fix SHA `7be0f3b1bdc3d5c4fdc07dcd52990cc86e356d9a` was queued/in progress at last poll; this new production source slice requires its own exact-SHA Actions. No local build or hardware result is claimed.
- Parent main: `7be0f3b1bdc3d5c4fdc07dcd52990cc86e356d9a`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Normalize all VPN WAN marker rollback source-order offsets

- The exact-head VPN lifecycle run #45800802057 on `1a0ff27d90b1d6cb396fcc45d979bbde0ebeab50` reran all 14 preceding GNU89 policy regressions successfully, then failed in the Python source contract. The previous whitespace correction compared offsets from the whitespace-stripped source for the rollback gate/call against offsets from the original source for the marker, launch and failure branch. Those index spaces are not comparable, so a correctly ordered implementation was falsely rejected.
- Normalize every WAN marker/launch/failure/rollback anchor and the related errno-preservation checks against the same compact source string. This is a checker-only correction; the C rollback implementation and policy cases are unchanged.
- AI handoff #45800802088 PASS on the failed-checker SHA. Exact-head VPN lifecycle and AI handoff reruns are required for this correction; no firmware L3 or hardware L4 claim.
- Parent main: `1a0ff27d90b1d6cb396fcc45d979bbde0ebeab50`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Roll back the WAN xl2tpd dependency marker after failed launch

- Found that `launch_wan_pppd_locked()` publishes `l2tp_wan_t=1` before calling `safe_start_xl2tpd_locked()` so concurrent shared-transport lifecycle decisions preserve the intended WAN dependency. If checked configuration, process inventory, live config publication, or the daemon launch then fails, the common launch-error path invalidates the PPP callback generation and returns without clearing the marker. Later VPN server stop/restart can incorrectly conclude that a WAN L2TP dependency still owns the shared xl2tpd daemon and may stop or restore it based on a phantom client.
- Track only a successfully published WAN marker and roll it back on failed L2TP/xl2tpd launch. Preserve the original launch errno if marker cleanup also fails, and log the cleanup error. RPL2TP and already-running xl2tpd paths remain unchanged.
- Added a pure policy helper and GNU89 policy regression for published/unpublished markers across success and failure statuses; extended the VPN source contract to pin publication → launch → failure rollback ordering. Exact SHA `3c53eb5a5fd339d77bc650fc2fefc7d8b60e81b4`: the xl2tpd policy test and the other twelve preceding GNU89/Werror (including ASan/UBSan) policy regressions passed; VPN lifecycle run #45800460199 then FAILED in the Python source contract because it searched a multiline marker-set call with a single-line literal. AI handoff #45800460259 PASS; the source-checker whitespace correction is in THIS COMMIT. Process lifecycle #45800460310, WAN route #45800460260, WAN DHCP callback #45800460321 and HTTPD #45800460223 PASS; NVRAM #45800460212 and XY-C1 #45800460210 were still in progress at last poll. No L3/L4 claim.
- Parent main: `cd3d1a6ec469937f3488d073bf11c445c0aa4414`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Preserve the NoBinding watchdog when initial Request setup fails

- Follow-up review found an uncovered branch in the new NoBinding watchdog: `reestablish_ia()` still removed the existing T2 timer before constructing the first Request and returned immediately on preparation failure. The later `IAS_REQUEST` watchdog only existed after successful event creation, so a non-fatal allocation/authentication failure could still leave the old Renew/Rebind event as the sole owner; if that event exhausted retransmissions, the IA was orphaned.
- Enter `IAS_REQUEST` before preparing the event, reuse the existing IA timer or create one when absent, then arm the same watchdog for 5 seconds on setup failure or 4 minutes after successful Request creation. The 4-minute deadline still waits beyond the finite Request retransmission window; successful reply handling and duplicate-NoBinding gates remain unchanged.
- Extended the Python source contract to pin state-before-prepare and watchdog-before-return ordering. Existing GNU89 policy tests already cover the 5-second setup retry and 4-minute exchange watchdog. Exact-SHA Actions are the validation gate; no local checkout/build or hardware test is claimed.
- Parent main: `cfa384c7fe1ba6fdd87ca478a920012880d835c6`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Keep DHCPv6 IA recovery alive after a NoBinding Request exhausts

- Found that `reestablish_ia()` removed the IA timer and launched a DHCP6S_REQUEST event. Request has MRC=10 and MRT=30 seconds; if the server stayed silent, `client6_timo()` removed the finite Request event with no remaining IA timer to start another exchange. The IA stayed in Renew/Rebind recovery until prefix expiry, and infinite valid lifetimes could leave it stuck indefinitely.
- Added `IAS_REQUEST` plus a four-minute IA watchdog. This exceeds the worst-case ten jittered Request retransmission intervals; when it fires, it starts a fresh Request and is rearmed. Transient local event-construction failures retry after 5 seconds. A failed multi-IA Reply can be replayed while another IA retries: duplicate NoBinding for an IA already in Request state is ignored, and its watchdog is deferred after each local application failure to avoid duplicate active Requests.
- Added policy tests for the four-minute/5-second watchdog and duplicate-NoBinding gate, plus Python source contracts for state transition, watchdog ownership and ordering. Exact SHA `cfa384c7fe1ba6fdd87ca478a920012880d835c6`: DHCPv6 IA #45799187926 and AI handoff #45799187883 PASS. Initial-Request setup retry on `cd3d1a6ec469937f3488d073bf11c445c0aa4414`: DHCPv6 IA #45799579046 and AI handoff #45799579038 PASS; XY-C1 #45799579141 remained in progress, no L3/L4 claim.
- Parent main: `fa6281866dfedd5de66302770e6ca133c8190d90`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Retain DHCPv6 IA timer ownership until Renew/Rebind events are prepared

- Found a recovery hole in `ia_timo()`: at T2 it deleted `ia->timer` before building the Rebind event. If prefix-list or authentication allocation failed in `prepare_iaevent()`, the event was discarded and no timer remained to retry Rebind. Invalid IA states also left a live expired timer behind.
- Keep the IA timer until the Renew/Rebind event is successfully prepared. Failed Renew preparation retains the existing T2 deadline; failed Rebind preparation reschedules a bounded 5-second retry; successful Rebind preparation transfers retry responsibility to the DHCP event and removes the IA timer. Invalid-state cleanup now removes any remaining timer.
- Added policy-level GNU89 regression coverage and Python source-contract checks. The existing DHCPv6 IA workflow compiles the policy test with `-Werror`, ASan and UBSan. Exact SHA `1e34fd20be38043504f9c8afba5df2c6d79e10db`: all seven C policy binaries PASS, including timer traversal and IA ownership tests; DHCPv6 IA #45797533696 then FAILED in the Python checker because the new assertion used undefined name `policy_test` instead of existing input variable `test`. AI handoff #45797533572 PASS after log rotation. The checker binding is corrected in THIS COMMIT; XY-C1 #45797533586 was still in progress. Full firmware/hardware not claimed. Exact checker/source rerun SHA `fa6281866dfedd5de66302770e6ca133c8190d90`: DHCPv6 IA #45797751509 PASS and AI handoff #45797751700 PASS; the policy C binaries and complete Python source contract now pass. XY-C1 #45797533586 on product SHA `1e34fd20be38043504f9c8afba5df2c6d79e10db` remained in Build XY-C1, so L3 is not verified.
- Recovered the log governance contract after a concurrent milestone pushed the root log to 120 records: archived the oldest 100 complete records verbatim in `docs/dev-log/2026/11/part-038.md`, retained the newest 20 prior records, and updated the indexed blob SHA/byte count. Root now contains 21 records.
- Parent main: `8a484891c96894a5452710101a05145fab0fc7e6`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Validate one RFC 5908 NTP FQDN suboption

- The generic DHCPv6 domain-list decoder can expand multiple concatenated domain names. The RFC 5908 NTP FQDN suboption is a single FQDN, so its payload is now checked before being passed to that generic decoder.
- The wire validator requires one terminal root label, no compression or extended labels, ASCII LDH labels, no IDNA A-labels, label length <=63 octets and total wire length <=255 octets.
- Regression covers valid FQDNs and malformed/multiple/compressed/IDN/oversized inputs; the Python source contract pins the NTP parser guard.
- Parent main: `26842e306af329188b8aefd999cbbd9d94024ecd`; exact-SHA Actions pending.

---

## 2026-11-26 — Make DHCPv6 timer traversal safe against callback-side deletion

- Found a concrete iterator use-after-free: `dhcp6_timer_check()` saved `tm_next` before invoking a timer callback, but a delegated-prefix retry callback can complete cleanup and call `remove_ia()`, whose `pref_cleanup()` removes sibling prefix timers. The next loop iteration could dereference a timer node already freed by `dhcp6_timer_remove()`.
- Added per-timer check generations and a timer-list mutation generation. A callback that frees its timer or changes the timer list causes traversal to restart at the live list head; already visited timers and timers newly added during the callback are skipped until the next sweep. Recompute the next-expiry sentinel after all callbacks so rescheduling of previously visited timers is reflected.
- Added a GNU89 policy regression for saved-next removal, head insertion, generation wrap and callback invalidation; wired the source contract and sanitizer regression into the DHCPv6 IA workflow for both push and pull_request.
- Local host compile/run: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` regression PASS. Full-source and target checks await exact-SHA Actions; no L3/L4 claim.
- Exact SHA `a856ba60a4695276b5c484f7e8388fdaeaf1ced0`: DHCPv6 IA run #45795811813 FAILED only at the newly added compile invocation; all six pre-existing policy binaries passed. The workflow command contained doubled trailing backslashes, causing the linker to treat a literal backslash as an input file. This follow-up changes the compile invocation to one shell command line and needs exact-SHA rerun.
- Workflow correction parent main: `a856ba60a4695276b5c484f7e8388fdaeaf1ced0`; resulting commit: THIS COMMIT.
- Exact-SHA follow-up: DHCPv6 IA workflow #45796127157 on `c241c6a150f2c0bf8b6f2a89177780cb9766ad9f` PASSed the new ASan/UBSan timer traversal regression, all existing C policy binaries and the full Python source contract; AI handoff #45796127128 PASSed. The prior failure was limited to YAML shell escaping, not production C or regression logic.
- Parent main for the source/test implementation: `5b03226e811dbba4cba5d9219c43086b7962d6e7`.

---

## 2026-11-26 — Correct literal unlock counting in NVRAM source contract

- Corrected a double-escaped regex in the new NVRAM commit-order source check. A literal-token scan now counts both unlock sites and enforces mutex-before-snapshot plus mutex-through-flash-I/O ordering.
- No production source or regression behavior changed in this follow-up.
- Parent main: `02c27d988c9c3cdb7be180bb468da6a465633990`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Serialize NVRAM snapshot generation with flash commits

- Found a persistence lost-update race: `nvram_commit()` built its in-memory image before acquiring `nvram_sem`, so concurrent commits could write the newer snapshot first and then overwrite it with an older snapshot.
- Acquire `nvram_sem` before the snapshot spinlock/generation and hold it through flash compare/write. On image-generation failure, release the semaphore before freeing buffers.
- Added a deterministic pthread regression for the stale-snapshot and corrected orderings, plus Python source-contract assertions. The NVRAM workflow compiles/runs the regression under GNU89 with warnings as errors.
- Parent main: `d5ef589529bf19971f996265cffb92b9906c5635`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Fence DHCPv6 SIGHUP reload against pending IA cleanup

- Confirmed P1 lifetime defect: SIGHUP called `free_resources(0)` and immediately reloaded configuration. If `pref_cleanup()` failed, `remove_ia()` retained an IA and retry timer in `iac->iadata`, while `read_config6()` previously called `clear_ifconf(0)`/`clear_iaconf()` and freed objects still referenced by the timer callback.
- Added `restart_pending` fencing, blocked new client exchanges, deferred reload until both IA cleanup and outstanding RELEASE transactions drain, and handled SIGTERM during a pending restart without queuing duplicate Release events. Added a defense-in-depth `read_config6()` `EBUSY` guard before `cleanup_cflist()` and `clear_ifconf()`.
- Extended GNU89 policy regressions and the Python source contract. Corrected the DHCPv6 IA workflow pull-request filter to include modified `dhcp6c_config.h`; push filtering already covered it. No local C/Python test performed without checkout; exact-SHA Actions are the validation gate.
- Parent main: `6dbcf822bed1213baabef7639aa3f1ae68288f3f`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Enforce exactly one DHCPv6 NTP time-source suboption

- RFC 5908 requires one and only one time-source suboption within each OPTION_NTP_SERVER. The parser previously set its `source_seen` flag only for suboption codes 1–3. Because unknown future registry values were skipped without claiming the source slot, an empty container or a container with multiple unknown/known suboptions could pass framing checks without satisfying the one-source rule.
- The parser now counts every suboption code in the NTP Time Source Suboption namespace as the single source slot. Unknown future codes remain forward-compatible and their payload is skipped, but they cannot be combined with another source suboption. The decoder rejects an empty container and rejects a second known or unknown source.
- Added policy regressions for no source, one unknown future source, unknown+known, one known source and two known sources. Python source contract pins the final mandatory-source gate.
- Parent main: `734e9a86960d5f9158904a3e73280fbbae3b3842`; exact-SHA validation pending.

---

## 2026-11-26 — Enforce RFC 5908 NTP address suboption classes

- Exact-SHA DHCPv6 IA run #45792959729 on `c713dbc59c566254f736d21ff1e8b06de6f2ab43` passed all six GNU89/`-Werror`/ASan/UBSan policy binaries and the complete Python source contract; AI handoff #45792959822 also PASSed.
- Added a follow-up contract: RFC 5908 suboption 1 carries an IPv6 unicast server address, while suboption 2 carries an IPv6 multicast group address. The decoder now rejects an address whose multicast class does not match the suboption identifier. Tests cover both valid classes, both mismatches, and forward-compatible unknown suboption handling.
- Python source checks pin the `IN6_IS_ADDR_MULTICAST()` classification at the actual parser path. Parent main: `c713dbc59c566254f736d21ff1e8b06de6f2ab43`; exact-SHA validation pending.

---

## 2026-11-26 — Bound NTP parser source assertion by option case

- Exact-SHA DHCPv6 IA run #45792727216 on `5ed671f2247989d48b9420f445130aae01280c94` compiled and passed all six GNU89/`-Werror`/ASan/UBSan C policy binaries. The Python source checker alone failed because its 240-character substring ended before the continuation line containing `&optinfo->ad_list`; the production decoder and C tests were not failing.
- Replaced the brittle fixed-length window with the complete `D6_OPT_NTP_SERVER` case (up to the next option case), normalized whitespace, and pinned the parser call before the malformed-option branch.
- Parent main: `5ed671f2247989d48b9420f445130aae01280c94`; exact-SHA DHCPv6 IA rerun pending.

---

## 2026-11-26 — Correct DHCPv6 option-bounds test output assertion

- Exact-SHA IA run #45792454948 for `74a0dbccda2028b87f4fe7cd57dd0513608f8f39` compiled and passed all six GNU89/`-Werror`/ASan/UBSan policy binaries, including the newly added NTP suboption length/source tests.
- The Python checker alone failed because it still expected the pre-NTP test banner. Updated the assertion to the actual intentional output, `DHCPv6 option bounds and NTP suboption policy: PASS`. No production C or test behavior changed in this corrective slice.
- Same SHA: AI handoff #45792454964 PASS, DHCPv6 script retry #45792455152 PASS, Process inventory #45792454913 PASS. HTTPD/NVRAM/USB and XY-C1 #45792454987 were still running at last poll.
- Parent main: `74a0dbccda2028b87f4fe7cd57dd0513608f8f39`; rerun Actions required.

---

## 2026-11-26 — Parse RFC 5908 DHCPv6 NTP Server suboptions

- Confirmed standards mismatch: `D6_OPT_NTP_SERVER` (option 56) was parsed in the same branch as flat IPv6-address arrays, requiring its payload length to be a multiple of 16 and treating each 16-byte chunk as an address. RFC 5908 defines option 56 as a container of 4-byte (code/length) suboption headers, with time-source forms for a unicast address, multicast address, or FQDN; each option instance carries exactly one time source. Consequently, a normal address suboption uses a 20-byte payload (4-byte header + 16-byte address) and was incorrectly rejected, while FQDN and multicast suboptions were not decoded.
- Added a bounds-safe suboption decoder. Address and multicast-address suboptions require exactly 16 payload bytes; FQDN requires nonzero payload and uses the existing RFC 3397 domain decoder when enabled. Duplicate known time-source suboptions are rejected; unknown future suboptions are skipped. The old RFC 4075 `D6_OPT_SNTP_SERVERS` remains on its flat IPv6-list path. The option typelen entry is now variable-length and dispatches to this dedicated parser.
- Added unit coverage for truncated headers, length overruns, valid and invalid address/FQDN lengths, unknown suboptions and duplicate source rejection; extended Python source contracts to pin the production decoder and legacy SNTP separation. The existing DHCPv6 IA workflow already compiles the extended policy test and checks `dhcp6c_common.c`.
- Parent main: `fb01f7aa71477bc1dfc895bb81d368e6c69031e7`; Actions validation pending. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-26 — Normalize shutdown source contract whitespace

- Exact-SHA DHCPv6 IA run #45791468143 on `aa7181cdfdf98d283e0126db0726d0daa51fdc9d` compiled and passed all six GNU89/`-Werror`/ASan/UBSan policy tests, including IA-only, event-only, combined pending-work and drained shutdown cases.
- The Python source contract failed only because it compared a wrapped two-line C call against a single-line string. Changed the check to strip whitespace from the extracted function before matching the call. No production source or C test changed in this correction.
- Parent main: `aa7181cdfdf98d283e0126db0726d0daa51fdc9d`; exact-SHA rerun pending.

---

## 2026-11-26 — Preserve DHCPv6 RELEASE exchanges during graceful shutdown

- Confirmed follow-on bug in the SIGTERM drain: `release_all_ia()` calls `prepare_iaevent(ia, DHCP6S_RELEASE)` before removing each IA. `free_resources(0)` intentionally keeps Release events so the client can transmit and wait for Reply/timeout. The shutdown readiness check only examined `iac->iadata`; after successful local cleanup that queue became empty while Release events were still outstanding, causing `check_exit(1)` to discard the Release exchanges.
- Shutdown readiness now requires both all configured IA data queues and the global event list to be empty. This waits for each preserved Release transaction to complete by Reply or retransmission timeout, while still keeping failed delegated-prefix cleanup IAs pending through retry timers.
- Expanded the GNU89 policy test for IA-only, event-only, combined pending work and fully drained state. Python source contract pins Release event creation, `free_resources(0)` preservation, and the two-part exit gate.
- Parent main: `b3cfaab82ee2dfbe1b5982f9cc127b778ddb3c4e`; exact-SHA Actions pending after commit. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-26 — Reject malformed DHCPv6 Reconfigure Accept payload

- DHCPv6 Reconfigure Accept (option 20) is defined with no payload. The outer option parser previously silently ignored this known option after applying only the option table minimum length (zero), so it accepted malformed non-empty instances.
- Added a production zero-payload check before ignoring the unsupported feature. The existing GNU89/ASan/UBSan option policy regression now covers zero, one and maximum-size payload lengths; the Python source contract pins the actual `D6_OPT_RECONF_ACCEPT` parser branch.
- The existing DHCPv6 IA workflow path filters and test step already cover every changed source/policy/test/checker file. Exact-head validation pending after commit; no local repo checkout or full build is available.
- Parent main: `d739b7f409e57e9b7bbdfa85b7abfacbd8e5804d`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Exact-SHA DHCPv6 shutdown policy and source contract PASS

- DHCPv6 IA workflow #45790398561 on exact source/checker SHA `78205c10fb71cfb962917e65261fa499c0548573` PASSed: all GNU89/`-Werror`/ASan/UBSan C policy regressions and the complete Python source contract passed. AI handoff workflow #45790398455 also PASSed.
- WAN DHCP callback workflow #45790261953 on exact workflow/source SHA `48bec7b6ac28ea93394467d6814023884eadfa1c` PASSed after inline lock-order assertions were updated for the intentional 16-second non-forced client stop.
- XY-C1 firmware run #45789833788 remains in progress on product-source SHA `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5`: Build is running; TRX verification and artifact upload are pending. Hardware L4 is not tested.
- Main parent for this handoff-only update: `78205c10fb71cfb962917e65261fa499c0548573`.

---

## 2026-11-26 — Update remaining DHCPv6 stop-fencing source assertion

- The #8d2bf19 DHCPv6 IA rerun passed all C policy binaries but the Python checker still contained an older callback-stop assertion searching for `stop_status = kill_services(svcs, 3, 1);`. The new non-forced 16-second stop made that substring stale; it failed with `ValueError: substring not found` before subsequent source checks.
- Updated the existing callback-generation fencing assertion to the new stop contract. The WAN callback inline checks were separately updated in #48bec7b to expect 16 seconds without SIGKILL escalation. Re-run the exact-SHA IA checker before claiming the contract passes.
- Parent main: `48bec7b6ac28ea93394467d6814023884eadfa1c`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Align WAN DHCP source contract with graceful DHCPv6 stop

- Exact-SHA WAN DHCP callback run #45789833759 for `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5` passed all compiled policy tests but failed its final inline Python source contract with `ValueError: substring not found`. Two assertions still searched for `kill_services(svcs, 3, 1)` after the DHCPv6 stop was deliberately changed to `kill_services(svcs, 16, 0)`.
- Updated both the lock-order and stop-generation inline assertions to recognize the 16-second non-forced stop. This tests the new fail-closed contract while retaining the assertion that callback generation advances only after successful process exit.
- Parent main: `8d2bf194cdbc065eae75318ba01cbe36c04e2356`; resulting commit: THIS COMMIT. Exact-SHA WAN DHCP callback run pending.

---

## 2026-11-26 — Fix DHCPv6 shutdown source-check ordering

- Exact-SHA DHCPv6 IA workflow #45789833785 for `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5` compiled and passed all six GNU89/`-Werror`/ASan/UBSan policy binaries, including the new shutdown-state regression. The Python source checker then failed because the newly added stop assertion referenced `wan6` before the existing `wan6 = read(...)` assignment later in the script.
- Moved the two stop-timeout assertions below that existing assignment. No firmware/runtime source changed in this corrective slice. Exact-SHA checker result is pending; do not claim this source contract has passed until the next workflow run succeeds.
- Parent main: `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Drain delegated-prefix cleanup before DHCPv6 client exit

- Confirmed shutdown defect: `remove_iaprefix()` retains failed downstream prefix removals and schedules a 5-second retry, but SIGTERM entered `check_exit(1)`; `free_resources(1)` removed those timers and exited. The WAN stop caller used `kill_services(svcs, 3, 1)`, which could SIGKILL the process before the first cleanup retry. A transient kernel deletion failure could strand a delegated prefix without a retry owner.
- SIGTERM/SIGQUIT now set a shutdown flag and call `free_resources(0)`. The main loop services pending IA timers and exits only when every configured IA data queue is empty. `client6_start()` suppresses new Solicit/Information-Request events after shutdown, and SIGHUP no longer restarts the client once shutdown begins.
- Normal stop and failed-start cleanup now use a 16-second non-forced wait. Persistent cleanup errors time out instead of escalating to SIGKILL; normal stop therefore leaves callback generation unchanged and WAN teardown fail-closed. The timeout covers multiple 5-second retry intervals.
- Added policy unit cases and source contracts for IA-drain readiness, restart suppression, timer-drain ordering and the non-forced stop contract. Exact-SHA CI is required; hardware behavior is not verified.
- Parent main: `f55521e7bc74691d18d4a6f62a6f9e326730b970`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Reject duplicate DHCPv6 singleton vbuf options

- The outer DHCPv6 option decoder stores CLIENTID, SERVERID, RELAY_MSG and INTERFACE_ID in one owned `dhcp6_vbuf` field per option type. Repeated instances previously overwrote the prior allocation and silently kept the last value, leaking memory until the packet cleanup path.
- Added a shared unset-vbuf predicate and reject each duplicate before calling `dhcp6_vbuf_copy()` or `xmemdup()`. The existing malformed-option failure path clears the partially populated `dhcp6_optinfo`, releasing the first allocation. Added four policy boundary cases; the Python source contract pins all four production guards and the cleanup path.
- `dhcp6c-ia-publication-stability.yml` already compiles the policy test and runs the checker, with push/PR path filters for every changed source/test/checker file. Exact-head validation pending after commit.
- No local checkout/full original test suite is available because the container cannot resolve github.com; no local build or device test is claimed.
- Parent main: `9ac2adf871a85e91a2a05518e47aba18c99df9be`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Preserve first error across DHCPv6 delegated-prefix cleanup

- `remove_iaprefix()` iterates every delegated downstream interface and correctly attempts all removals, retaining failed entries for retry. However, each failed `ifaddrconf(IFADDRCONF_REMOVE)` overwrote `ret`; multiple failures therefore returned the last errno, contrary to the existing first-error aggregation policy used elsewhere in DHCPv6 IA.
- Routed each failed delete through `dhcp6c_ia_first_error()`, without changing retry or per-interface cleanup behavior. The host regression now drives three independent failed downstream deletions (`EBUSY`, `EINVAL`, `EIO`) and proves that all are attempted, all remain pending, and the first error is returned. The Python source contract pins the production call and the new regression.
- The existing DHCPv6 IA workflow compiles this test with GNU89/`-Werror`/ASan/UBSan and runs the source checker; exact new-SHA results are pending until Actions conclude.
- Parent main: `21d71c59e186de568492b0f31e4518396ed63b35`; resulting commit: THIS COMMIT.

---

### 2026-11-26 — autonomous stability continuation

Handoff HEAD: `1e3154270732eef0314244dec117dd325971638a`. Frozen backup ref remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

Implemented source/test slices in Samba paired-transaction durability, Ralink IAPP event-table/TCP deadline safety, DNSMasq staged publication and failed-start rollback, and Inadyn last-known-good config retention through startup confirmation. Inadyn follow-up covers same-inode hard-link cleanup on rollback before publication. See the corresponding commit history and exact-SHA Actions in `SESSION_HANDOFF.md`; this log does not treat a pending check as a pass.

Current first follow-up: verify the exact-SHA Inadyn workflow status for the latest snapshot rollback changes; fix any test or build regression before moving on.
