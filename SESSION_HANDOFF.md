## Current repository state

- Live parent `main`: `56931489867a3772ef4b8f45a8559705ebd0fbbc`; this commit is a focused FRP host-test fixture correction. The exact source SHA is rechecked before ref update.
- FRP process-inventory hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`: incomplete procfs metadata is UNKNOWN; a failed wait does not justify SIGKILL escalation.
- FRP run #45341083663 failed at `client missing` because the fake process-exit helper deleted only `cmdline`, leaving the synthetic PID directory and executable symlink present. The scanner correctly failed closed on this impossible-to-call-terminated state. This follow-up removes the whole synthetic PID directory for actual TERM/KILL exits; dedicated UNKNOWN tests still remove only `cmdline` and retain the executable link.
- Process inventory #45341083645 and AI handoff #45341083665 PASS on `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; FRP lifecycle on that SHA FAILED. Exact reruns are required on this commit.
- HTTPD security #45340186122 and AI handoff #45340186109 PASS on `8584845fa7ce78b928bfb6e47c5cf8eb8223982f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

FRP test fixture correction only:
- The production `frp_process_matches()` returns UNKNOWN when `/proc/PID` and its executable link still exist but `cmdline` is missing. This is deliberate fail-closed behavior.
- The old fake `remove_process()` represented a terminated process by unlinking only `cmdline`; this instead models metadata loss while a process remains visible.
- Both `frp_lifecycle.sh` and `frp_stop.sh` now remove the complete synthetic PID directory for actual process exit. The dedicated `TERM_MAKES_CMDLINE_UNKNOWN` and explicit missing-cmdline cases still preserve the executable link and directory, exercising UNKNOWN handling.

## Completed development slices

- FRP process inventory and stop escalation (`fa3dd001`): preserve RUNNING/ABSENT/UNKNOWN, propagate scan failures, and allow SIGKILL only after verified TERM timeout. Source/test checker and Process inventory regression are in place; exact FRP rerun was initially blocked by the inaccurate fake exit fixture.
- Inadyn process identity extension remains the next independent security task: existing stop/reload/manual-update identity validation is comm/argv/start-time bound but not bound to `/proc/PID/exe`; the installed binary is `/bin/inadyn`. No changes have been committed for it yet.
- Router hostname/domain callback failure propagation (`a26746d`) and HTTPD contract correction (`8584845`) remain completed predecessor slices with the exact action statuses recorded in `dev_log.md`.

## Verification boundary

- Predecessor `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`: FRP async startup regression #45341083663 FAILED at `client missing` due only to the test helper leaving an extant synthetic PID with missing cmdline; Process lifecycle inventory #45341083645 PASS; AI docs #45341083665 PASS.
- This follow-up test-only correction is pending exact-SHA FRP workflow rerun and AI handoff docs. No MIPS firmware, TRX artifact or hardware L4 result is claimed.
- The UNKNOWN metadata fixtures are intentionally retained: tests explicitly remove only `cmdline` while preserving `/proc/PID/exe` to prove a live but uninspectable process blocks unsafe escalation.

## Exact first action for the next GPT

1. Inspect exact-SHA FRP async lifecycle, FRP stop, Process inventory and AI handoff Actions for this commit; fix any failure.
2. If FRP/Process checks pass or remain pending, continue the Inadyn executable-bound stop/reload/manual-update fix and add GNU89 sanitizer regressions.
3. Keep the frozen backup immutable; main writes remain complete-tree atomic commits with expected-SHA non-forced updates.

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
