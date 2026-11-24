## Current repository state

- Live parent `main`: `257ac6399c4c7e8ec3b518f8f8438532d0821c33`; this commit isolates the FRP crontab transaction regression from host `/proc`. Re-fetch and compare main before any further write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`; its dependent source-contract follow-up is `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process-inventory tri-state hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; test fixture correction is in `5687c609137bbba09c707896dd95216c3df9e961`. This commit fixes the next distinct FRP regression failure.

## Active slice: FRP crontab test isolation

- FRP run #45341607742 on `5687c609` showed `frp_lifecycle.sh` async/readiness and watchdog identity cases PASS, then `frp_crontab.sh` failed: `start rejected a valid crontab`.
- `frp_crontab.sh` replaces `frp_wait_started()` but had not replaced the default `FRP_PROC_ROOT=/proc`. Since `frp_start()` correctly calls `frp_stop_daemons()` before launch, the cron-only test depended on host procfs readability; the production scanner fails closed on unreadable candidate identities.
- This test-only commit uses an empty `$test_root/proc` and requires the source checker to pin that fixture. FRP process scan/stop behavior remains covered by the independent synthetic-process lifecycle/stop tests.

## Verification boundary

- Exact `79c5389`: Process inventory #45341769713 PASS; Process fail-closed #45341769769 PASS; HTTPD #45341769686 PASS; AI docs #45341769695 PASS; WAN DHCP #45341769830 PASS; miniupnpd #45341769855 PASS; Samba #45341769858 PASS; NVRAM audit #45341769831 PASS. Inadyn #45341769924 failed on a checker function extractor bug; DNSMasq #45341769848 and WAN route #45341769689 failed on stale raw-inventory assertions. These three contract failures were corrected in `257ac639`.
- Exact `257ac639`: Inadyn #45342407697, DNSMasq #45342407692, WAN route #45342407753 and AI docs #45342407729 were IN_PROGRESS at last poll. Recheck exact completion before claiming PASS.
- XY-C1 #45341769654 on `79c5389` had all preceding host validation steps pass and the firmware Build step still IN_PROGRESS; TRX and artifact upload had not completed. No L3 claim is made.
- Exact FRP #45341607742 on `5687c609` FAILED at the crontab test because its fixture used live `/proc`; the async lifecycle test passed. This commit isolates that test from host procfs. Exact FRP rerun required.
- No MIPS/TRX/artifact completion or hardware L4 is claimed. Linux 4.4 lacks pidfd; validation-to-signal races remain a documented platform limitation.

## Exact first action for the next GPT

1. Check exact FRP, Inadyn, DNSMasq, WAN route, Process, AI docs and XY-C1 Actions on this commit; fix any first failure. Claim firmware L3 only if Build, TRX and upload all pass for one SHA.
2. If FRP crontab passes, continue a P1 source stability/security fix outside the completed process-identity slices; if it fails, use its exact log to fix the actual regression rather than changing the fail-closed scan.
3. Keep the backup immutable and all main writes complete-tree atomic with expected-SHA non-forced ref updates.

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
