## Current repository state

- Live parent main before this atomic change: `bc34d1db08a86f762ffad75bec8c8276629f29c3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

Close and verify the PID 1 MTD reboot checker before moving to the queued QMI defect:
- Source commit 6130ebd adds a checked MTD helper result and direct reboot syscall fallback.
- Exact Process #45345435889 and AliyunDrive WebDAV #45345435736 exposed the same over-broad checker assertion, which mistook the assigned `mtd_unlock_status = system(...);` call for an unassigned statement.
- Commit 33680078 attempted to narrow the assertion but left an unterminated Python raw-string expression, so its exact Process/AliyunDrive/HTTPD reruns must be triaged. Current follow-up changes the assertion to a complete line-anchored regex that rejects only a standalone unassigned helper call.
- Once the checker passes, continue QMI WAN start fail-closed handling: stale/unparseable `/tmp/qmi-handle` and ignored `--start-network` failures must not be accepted as successful session creation. Add a focused C policy test and static contract without changing the uqmi CLI.
- XY-C1 firmware must be checked per exact SHA; never claim L3 without same-SHA Build, TRX and artifact completion.

## Verification boundary

- Exact 33680078f7b33ac6e166fbeab5ed99d4a5e17558: checker-only commit attempted the MTD negative assertion correction; exact Actions #45345902653 (AliyunDrive), #45345902730 (Process inventory) and #45345902660 (HTTPD) were IN_PROGRESS at last poll. The checked-in Python expression is incomplete and is corrected in the current follow-up; these runs must be verified.
- Exact 6130ebd28962da5bf55439349632175dfc0d858e: DNSMasq #45345435699 PASS; PID1 notification queue #45345435667 PASS; Process inventory #45345435999 PASS; HTTPD #45345435733 PASS; PID1 mdev security #45345435825 PASS; DNS forwarder #45345435708 PASS; RC timezone #45345435849 PASS; USB storage #45345435532 PASS; NVRAM transaction audit #45345435848 PASS; AI handoff #45345435583 PASS. Process fail-closed #45345435889 and AliyunDrive #45345435736 failed on the old checker assertion. XY-C1 #45345435466 was still IN_PROGRESS at Build; TRX/artifact unconfirmed.
## Exact first action for the next GPT

1. Inspect exact-head Process inventory, AliyunDrive and HTTPD Actions for the complete regex fix; resolve any genuine failure.
2. Check exact-head AI handoff, then continue QMI start-network stale-PDH validation and regression coverage in the USB modem workflow.
3. Poll XY-C1 Build/TRX/artifact for the source SHA. Do not claim L3 without same-SHA Build, TRX and artifact PASS.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
