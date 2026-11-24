## Current repository state

- Active branch: `main`; parent for the DHCP policy test-runner correction: `4015869670026b37843d043045a9805cdead925b`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPD selector and WAN source-PHY range fixes retain recorded exact-SHA L3 Build/TRX/artifact evidence. QMI WDS/PDH teardown on `b05eae9` has same-SHA XY-C1 Build/TRX/artifact PASS (L3). Physical-router L4 remains unverified.
- DHCPDECLINE retry/send-result changes on `4ca919b` and follow-up `02ddda7` / `4015869`: packet send semantics are now correctly treated as failed only for negative return values. Python source contract and AI handoff contract were repaired; however exact udhcpc #45400399177 on `4015869` failed GNU89 compilation because `main()` still invoked the old test name after its function was renamed. This commit aligns the invocation with the renamed regression function.
- On `4015869`, AI handoff #45400399146 PASS; Process lifecycle #45400399189 PASS. USB storage #45400399201, NVRAM #45400399144, XY-C1 #45400399157 and Process inventory #45400399232 were still running at last poll.

## Active task — Verify DHCPDECLINE retry and packet-send semantics

- The current slice retains a failed DHCPDECLINE packet in `DECLINE_PENDING` and retries only after any prior lease deconfig succeeds.
- BusyBox send helpers return positive byte counts on success and `-1` on failure. `udhcpc_packet_send_failed(status < 0)` drives both DECLINE retry and RELEASE diagnostics.
- Exact-head compile failure was purely a stale test-runner function name: definition and source checker already use `test_failed_packet_send_blocks_dhcp_reselection_until_retry()`; `main()` still called `test_failed_dhcpdecline_blocks_reselection_until_retry()`. Updated that call; production code unchanged.

## Verification boundary

- Parent SHA `4015869670026b37843d043045a9805cdead925b`: AI handoff PASS, Process lifecycle PASS; udhcpc GNU89 ASan/UBSan compile failed on the stale test invocation name. The corrected commit must pass the exact-head regression and source contract.
- Same-SHA XY-C1 Build/TRX/artifact are still required before L3 for DHCPDECLINE. DHCP server ARP-conflict simulation and target MIPS/uClibc runtime L4 remain unverified.

## Exact first action for the next GPT

1. Fetch live `main` and query exact-head Actions for the test-runner correction.
2. Verify the udhcpc GNU89 ASan/UBSan binary and Python source contract; fix any causal failure atomically.
3. Check XY-C1 Build/TRX/artifact and the USB/NVRAM/Process runs for the same production SHA, then continue an independent stability/security defect. XUPnP's two-file Lua configuration publication still needs a rollback-capable pair transaction.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, relevant exact-head Actions, validation boundary, blockers and the first next action. If all checkpoint facts remain accurate, make no file change and no commit.

# Cross-session execution mandate

Recover once and continue on main. Never create a development branch or PR; the backup ref is immutable.

For every multi-file logical slice, create all blobs, one complete tree based on latest main, one commit with the exact parent, then update main with a non-forced expected_sha compare-and-swap. This is one atomic main commit. Never create partial commits, force-push or rewrite history.

Continue implementing safe fixes rather than stopping at audits. PASS advances; FAIL is diagnosed and corrected; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc and router compatibility.

Before context exhaustion, finish the current safe slice, record the latest SHA/tests/CI/unresolved issue and one first action, append material milestones to dev_log.md, and update roadmap only for a material module/status/priority change.