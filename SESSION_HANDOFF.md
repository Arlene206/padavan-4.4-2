# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development on `main` only.
- Parent `main` for this DHCPv6 parser slice: `9ac2adf871a85e91a2a05518e47aba18c99df9be`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed/unmerged.
- Bootstrap/protocol recovery was completed earlier; do not repeat it.

## Active task

- P1 DHCPv6 parser allocation safety. CLIENTID, SERVERID, RELAY_MSG and INTERFACE_ID are stored in one vbuf slot per option type. Before this slice, duplicate options overwrote an existing allocated pointer and silently used the last value. The decoder now rejects a duplicate before allocating again; its malformed-option path calls `dhcp6_clear_options()` to free the first allocation.
- Added a shared vbuf-unset predicate and four boundary cases (NULL/zero, non-NULL/zero, populated, NULL/nonzero). The Python source contract pins all four production guards and verifies parser-failure cleanup. The DHCPv6 IA workflow already includes these files in push/PR filters and compiles/runs the policy regression.
- Adjacent WAN6 fixes already on main: empty caller-interface teardown guard; SIT owner-record IFNAMSIZ bound; strict SIT gateway literal validation; strict DHCP 6rd relay literal validation; static and DHCPv6 DNS literal validation; first-error aggregation during delegated-prefix cleanup.

## 2026-11-26 implementation continuation — DHCPv6 cleanup shutdown

- New confirmed P1 defect fixed in the next commit (parent `f55521e7bc74691d18d4a6f62a6f9e326730b970`): on SIGTERM/SIGQUIT, `dhcp6c` previously called forced `check_exit(1)`; `free_resources(1)` discarded the 5-second retry timers that `remove_iaprefix()` schedules after a delegated downstream-address deletion fails. The WAN caller also used a 3-second stop with SIGKILL escalation, so a transient kernel delete error could strand a delegated prefix without a retry owner.
- The client now enters a shutdown-drain state, runs `free_resources(0)`, keeps the event loop alive until every configured IA queue is empty, checks readiness after timer callbacks and received packets, and suppresses new Solicit/Information-Request events (including the `client6_start()` triggered after successful cleanup). SIGHUP cannot restart a client already shutting down.
- Normal and failed-start stop paths use a 16-second non-forced wait. Persistent cleanup errors therefore cause a fail-closed timeout instead of SIGKILL; normal stop does not advance callback generation on timeout.
- Added policy cases plus Python source assertions for queue drain, shutdown/restart ordering and non-forced stop; DHCPv6 IA workflow filters now include `dhcp6c_config.h`.
- Exact parent `9ac2adf871a85e91a2a05518e47aba18c99df9be`: DHCPv6 IA #45788196340 PASS; WAN DHCP #45788196312 PASS; AI #45788196513 PASS; process inventory #45788196355 PASS; XY-C1 #45788196523 was still building; NVRAM #45788196283 and USB storage #45788196181 were cancelled.
- Intermediate parent `f55521e7bc74691d18d4a6f62a6f9e326730b970`: DHCPv6 IA #45788409998 PASS; DHCPv6 script retry #45788410243 PASS; HTTPD #45788410121 PASS; AI #45788410109 PASS; Process #45788410216 PASS; NVRAM #45788410050 PASS; USB storage #45788410075 PASS. XY-C1 #45788410074 is still in progress with Build XY-C1 running; TRX and artifact steps are pending.
- These results are parent-SHA facts only. Hardware remains untested; exact-SHA CI must validate the shutdown commit.

## Verification boundary

- Parent `89b80f1505508bb8ae96466cfe5d0c7587a6be67`: all 11 exact-SHA Actions PASS. WAN DHCP #45785983731, WAN route #45785983765, DHCPv6 IA #45785983610, AI #45785983636, HTTPD #45785983585, NVRAM #45785983758, process #45785983770, 6rd #45785983604, service type #45785983716, WAN DNS #45785983910 and XY-C1 #45785983682. XY Build, TRX verification and artifact upload all PASS; artifact #15101485510 (12,823,157 bytes). This is the last confirmed product-source L3 baseline.
- Parent `21d71c59e186de568492b0f31e4518396ed63b35`: WAN DHCP #45787666271, WAN route #45787666333, AI #45787666397, 6rd #45787666495, WAN DNS #45787666335, HTTPD #45787666332, process #45787666517, service type #45787666373 and DHCPv6 IA #45787666291 PASS. NVRAM #45787666341 and XY-C1 #45787666437 were in progress at the last check.
- Parent `9ac2adf871a85e91a2a05518e47aba18c99df9be`: latest Process #45788196355, XY-C1 #45788196523, AI #45788196513, NVRAM #45788196283, DHCPv6 IA #45788196340, WAN DHCP #45788196312 and USB hotplug #45788196181 were queued/in progress at last inspection. Inspect exact conclusions before treating any as verified.
- No local repository checkout is available (Git clone failed because github.com could not resolve); no local full repository suite or firmware build is claimed. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head DHCPv6 IA, WAN DHCP callback, Process, AI and XY-C1 workflows for the shutdown-drain commit. Confirm the C policy test and Python source checker pass.
2. For the same product-source SHA, require `Build XY-C1`, `Verify TRX output` and artifact upload all PASS before advancing L3. A cleanup timeout must leave the daemon alive and normal stop must not advance callback generation; do not force-kill it.
3. Continue the remaining DHCPv6 state-machine/callback live-transition audit after the shutdown drain is validated. Hardware L4 remains NOT VERIFIED.

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
