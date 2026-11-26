# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development on `main` only.
- Parent `main` for the RFC 5908 DHCPv6 NTP parser and source-checker correction: `5ed671f2247989d48b9420f445130aae01280c94`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed/unmerged.
- Bootstrap/protocol recovery was completed earlier; do not repeat it.

## Active task

- Completed parser-safety slices: the DHCPv6 outer parser rejects duplicate CLIENTID, SERVERID, RELAY_MSG and INTERFACE_ID vbuf options before allocation; malformed-option cleanup frees the first owned buffers. Reconfigure Accept (option 20) is rejected when its payload is nonzero. Policy tests and source contracts cover both changes.
- The graceful DHCPv6 shutdown now waits for both IA cleanup retries and outstanding `DHCP6S_RELEASE` transactions. Exact-SHA DHCPv6 IA run #45791657631 on `fb01f7a` PASSed all six GNU89/ASan/UBSan policy binaries and the full source checker after normalizing the wrapped two-argument exit gate. This prevents shutdown from discarding Release exchanges deliberately preserved by `free_resources(0)`.
- Current P1 parser slice: `D6_OPT_NTP_SERVER` (option 56) was incorrectly grouped with flat arrays of IPv6 addresses, although RFC 5908 defines it as a container of 4-byte suboption headers. The parser now validates each nested header/payload, handles address/FQDN forms, rejects multiple recognized time sources in one option instance, skips unknown future suboptions, and keeps RFC 4075 `D6_OPT_SNTP_SERVERS` as the separate flat-address option. Tests cover truncated headers, length overruns, payload lengths, duplicate source suboptions, and the RFC 5908 requirement that address suboption 1 be unicast while suboption 2 be multicast. Production/test change is commit `74a0dbcc`; the six C policy binaries passed. Checker fixes: stale test-output banner corrected in `5ed671f`; bounded option-case assertion corrected in `c713dbc`. The exact-SHA C regression and complete Python source contract passed in DHCPv6 IA run #45792959729 on `c713dbc`.
- Adjacent WAN6 fixes already on main: empty caller-interface teardown guard; SIT owner-record IFNAMSIZ bound; strict SIT gateway literal validation; strict DHCP 6rd relay literal validation; static and DHCPv6 DNS literal validation; first-error aggregation during delegated-prefix cleanup.

## 2026-11-26 implementation continuation — DHCPv6 cleanup shutdown

- New confirmed P1 defect fixed in the next commit (parent `f55521e7bc74691d18d4a6f62a6f9e326730b970`): on SIGTERM/SIGQUIT, `dhcp6c` previously called forced `check_exit(1)`; `free_resources(1)` discarded the 5-second retry timers that `remove_iaprefix()` schedules after a delegated downstream-address deletion fails. The WAN caller also used a 3-second stop with SIGKILL escalation, so a transient kernel delete error could strand a delegated prefix without a retry owner.
- The client now enters a shutdown-drain state, runs `free_resources(0)`, keeps the event loop alive until every configured IA queue is empty, checks readiness after timer callbacks and received packets, and suppresses new Solicit/Information-Request events (including the `client6_start()` triggered after successful cleanup). SIGHUP cannot restart a client already shutting down.
- Normal and failed-start stop paths use a 16-second non-forced wait. Persistent cleanup errors therefore cause a fail-closed timeout instead of SIGKILL; normal stop does not advance callback generation on timeout.
- Added policy cases and Python source assertions for queue drain, shutdown/restart ordering and non-forced stop; DHCPv6 IA workflow filters include `dhcp6c_config.h`. Exact-SHA run #45789833785 compiled and passed all six GNU89/ASan/UBSan C regressions; its Python checker first exposed a checker-only variable-order issue, corrected in #8d2bf19. The same `a0fa423` WAN DHCP workflow then exposed two inline source-contract assertions still expecting the former 3-second forced stop. This commit aligns those assertions with the intentional 16-second non-forced behavior; rerun exact-SHA WAN/IA checks before claiming PASS. The #8d2bf19 rerun then caught one remaining legacy `3, 1` expectation in the IA checker’s existing callback-generation fencing assertion; this commit updates that final assertion to `16, 0`.
- Exact parent `9ac2adf871a85e91a2a05518e47aba18c99df9be`: DHCPv6 IA #45788196340 PASS; WAN DHCP #45788196312 PASS; AI #45788196513 PASS; process inventory #45788196355 PASS; XY-C1 #45788196523 was still building; NVRAM #45788196283 and USB storage #45788196181 were cancelled.
- Intermediate parent `f55521e7bc74691d18d4a6f62a6f9e326730b970`: DHCPv6 IA #45788409998 PASS; DHCPv6 script retry #45788410243 PASS; HTTPD #45788410121 PASS; AI #45788410109 PASS; Process #45788410216 PASS; NVRAM #45788410050 PASS; USB storage #45788410075 PASS. XY-C1 #45788410074 is still in progress with Build XY-C1 running; TRX and artifact steps are pending.
- These results are parent-SHA facts only. Hardware remains untested; exact-SHA CI must validate the shutdown commit.

## 2026-11-26 validation update — shutdown drain source contract PASS

- Exact SHA `78205c10fb71cfb962917e65261fa499c0548573`: DHCPv6 IA workflow #45790398561 PASS. The GNU89/`-Werror`/ASan/UBSan compile-and-run step and the complete `scripts/check_dhcp6c_ia_publication.py` source contract both passed. AI handoff workflow #45790398455 PASS.
- Exact SHA `48bec7b6ac28ea93394467d6814023884eadfa1c`: WAN DHCP callback workflow #45790261953 PASS after inline stop-order checks were updated for the 16-second non-forced shutdown contract.
- Product firmware source SHA `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5`: XY-C1 run #45789833788 remains in progress at `Build XY-C1`; TRX verification and artifact upload are pending. Later commits adjusted only checker/workflow/handoff material, not product firmware source. L3 is not yet verified.
- Hardware L4 remains NOT VERIFIED.

## 2026-11-26 validation update — RFC 5908 NTP parser checker correction

- Exact SHA `74a0dbccda2028b87f4fe7cd57dd0513608f8f39`: DHCPv6 IA run #45792454948 FAILED only in the Python source checker. All six GNU89/`-Werror`/ASan/UBSan C policy binaries passed, including the new NTP bounds/suboption policy test. The checker expected the previous output string `DHCPv6 option and UDP datagram bounds policy: PASS`; the test correctly prints `DHCPv6 option bounds and NTP suboption policy: PASS`. The expected string is updated in this commit.
- Same SHA AI handoff #45792454964 PASS, DHCPv6 script retry #45792455152 PASS, Process inventory #45792454913 PASS. HTTPD #45792454943, NVRAM #45792454938 and USB #45792455009 were still running at last poll.
- XY-C1 #45792454987 on exact product-source SHA `74a0dbcc` was still in progress; Build/TRX/artifact conclusions were not yet available. L3 is not verified; hardware L4 NOT VERIFIED.

## 2026-11-26 validation update — NTP parser source checker PASS

- Exact SHA `c713dbc59c566254f736d21ff1e8b06de6f2ab43`: DHCPv6 IA #45792959729 PASS. All six GNU89/`-Werror`/ASan/UBSan policy binaries and the complete Python parser/source checker passed after the checker was scoped to the full NTP option case and normalized whitespace. AI handoff #45792959822 PASS.
- The NTP decoder from `74a0dbcc` is therefore regression-verified on that source baseline. This follow-up adds RFC 5908 unicast/multicast address-class checks and must receive its own exact-SHA Actions result.
- XY-C1 #45792454987 on product-source SHA `74a0dbcc` was still in progress at the last poll; Build/TRX/artifact result not verified. Hardware L4 NOT VERIFIED.

## Verification boundary

- Parent `78205c10fb71cfb962917e65261fa499c0548573`: DHCPv6 IA #45790398561 PASS, including the GNU89/`-Werror`/ASan/UBSan C policy binaries and complete Python source contract; AI handoff #45790398455 PASS.
- Parent `48bec7b6ac28ea93394467d6814023884eadfa1c`: WAN DHCP callback #45790261953 PASS with the corrected 16-second non-forced stop source contract.
- Handoff-only parent `d739b7f409e57e9b7bbdfa85b7abfacbd8e5804d`: AI handoff #45790741509 was in progress at the last poll. The Reconfigure Accept parser edge added in this commit requires exact-head DHCPv6 IA revalidation.
- Latest fully verified product-source L3 baseline remains `89b80f1505508bb8ae96466cfe5d0c7587a6be67`: same-SHA Build XY-C1, TRX verification and artifact upload all PASS (artifact #15101485510, 12,823,157 bytes). Later production source changes are not yet shown to pass all three on one SHA. Physical router L4 NOT VERIFIED.
- No local repository checkout/full firmware build is available because the shell environment cannot resolve GitHub. Do not claim local full-suite or hardware verification.

## Exact first action for the next GPT

1. Check exact-SHA DHCPv6 IA run after the NTP address-class hardening; ensure all C policy binaries and the Python parser contract pass. Confirm the already passing `c713dbc` baseline run #45792959729 is not superseded by a failing new run.
2. Inspect current XY-C1 run #45792454987 on product-source SHA `74a0dbcc` (or any later exact-source run); require Build XY-C1, Verify TRX output and artifact upload all PASS together. L3 is not yet verified; hardware L4 remains NOT VERIFIED.
3. Continue DHCPv6 option parser audit for RFC 5908 option appearance restrictions and single-FQDN payload validity; only implement confirmed defects with regression coverage.

## Mandatory handoff checkpoint

**Mandatory handoff checkpoint:** Every session MUST compare the live repository and exact-head Actions with this file before ending. Verify source baseline, active task/progress, test/CI boundary, blockers and exact next action. Do not repeat completed recovery or audits. The checkpoint is mandatory; a document edit is conditional. If facts remain accurate, do not create a no-op commit.

# Cross-session execution mandate

Continue as the same maintainer across conversations. Every reviewable source/test milestone must be one atomic main commit: create all changed blobs, a complete Git tree and one commit, then update `main` non-forced with the expected parent SHA. Never publish partial-file commits or force-push.
Before context exhaustion, complete a safe bounded slice, inspect exact-SHA Actions and leave this handoff with the precise next task and validation boundary.

## Execution invariants

- Main only; full tree/atomic commit; expected-SHA non-forced update; never force-push.
- Preserve the frozen backup and do not reopen PR #1.
- PENDING never blocks independent safe work.
- Product L3 requires same-product-source-SHA Build XY-C1, TRX verification and artifact upload all PASS. Physical L4 requires actual device testing.
- Update dev_log for completed engineering milestones, handoff for material progress/validation changes, and roadmap only for material module/status changes.
