## Current repository state

- Live branch: `main`; parent for this SmartDNS checker/runtime-regression correction is `9416e328a409b4cc37a75c1bb7d6df436bec23f4`. The parent added PID-scoped signal assertions and a killall tripwire, but its exact-head checks exposed two test-harness issues now corrected: the Python checker encoded newline delimiters incorrectly when extracting the signal helper, and the lifecycle fixture lacked several documented stop-postcondition markers/a stuck-daemon-after-KILL case.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable. Legacy PR #1 remains closed and unmerged.
- Every logical task must be one atomic main commit: create one complete tree and one commit, then update `main` by non-forced expected-SHA compare-and-swap. No branches, PRs or force pushes.

## Active task

### Close the SmartDNS process lifecycle regression contract

- `scripts/check_smartdns_lifecycle.py` now checks separate `smartdns_named_process_state()` and exact SmartDNS process identity, plus PID/start_time revalidation before PID-scoped TERM/KILL. Parent `9416e328` added checks for the exact signal function, but mistakenly wrote a double-escaped newline in the Python `body()` extraction string; the Action fails at that assertion.
- The shell suite's existing cases already test valid/absent/UNKNOWN process inventory, executable/argv impostors, bounded graceful/force/UNKNOWN stop and PID replacement. The checker also expects named markers for unknown state, verified graceful absence, force-after-timeout, unknown post-signal no-KILL, and reporting a process that survives KILL. This correction restores those pass markers, asserts TERM-before-KILL order, and adds a stuck-daemon case proving stop returns running status after both signals. It also keeps the no-name-wide-killall tripwire.
- The Process workflow push/PR filters now include `trunk/user/smartdns/**`, so future source/helper/test changes trigger the Process regression.

## Verification boundary

- Exact parent `9416e328a409b4cc37a75c1bb7d6df436bec23f4`: Process inventory #45427406147 FAILED at the checker helper-body extraction (`expected one function marker`); the preceding SmartDNS shell regression in the separate Process lifecycle inventory #45427405894 passed. AI handoff #45427406412 FAILED because handoff lacked exact required wording `one atomic main commit`. This correction fixes both.
- Parent `9416e328`: Process lifecycle inventory #45427405894 PASS; DNSMasq #45427405995 PASS. USB storage #45427406211 was IN_PROGRESS; XY-C1 Firmware Validation #45427405793 was IN_PROGRESS. Check exact final Build/TRX/artifact steps before any firmware L3 claim.
- Previous 822d8a8 XY-C1 run #45425821347 is an older parent and must not be used to certify a newer production source tree. No L3 claim without same-SHA Build, TRX and artifact PASS. Physical L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current `main` and query its exact-head Process inventory, AI handoff, USB storage and XY-C1 Actions. Inspect logs for any failing job; in particular verify the new stuck-daemon test and corrected checker helper extraction.
2. Query `9416e328` XY-C1 #45427405793 for final Build, TRX and artifact status, while keeping its result tied to that exact older SHA.
3. After the regression suite is green, continue an independent P1 security/stability defect outside SmartDNS and DNS Forwarder. Do not idle on pending CI.

## Mandatory handoff checkpoint

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once, implement, test, and hand off. Use one complete Git tree and one atomic main commit per logical task, with a non-forced expected-SHA update. Before context exhaustion, record real tests and exact-head CI, and leave the next GPT one precise first action. Never force-push or overstate firmware/hardware evidence.
