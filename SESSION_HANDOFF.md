## Current repository state

- Live parent `main`: `5687c609137bbba09c707896dd95216c3df9e961`; this atomic continuation adds executable-bound Inadyn process identity. Re-fetch `main` and compare expected SHA before every further write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- FRP process-inventory tri-state hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its host-test fixture correction is in `5687c609137bbba09c707896dd95216c3df9e961`. In the fixture, actual TERM/KILL exit now removes the complete synthetic PID directory; dedicated UNKNOWN tests still remove only `cmdline` and preserve the directory/exe link.
- Router hostname/domain status propagation is in `a26746d033ffa19d9b576010f4fe4a583b4a859a`; HTTPD contract correction is in `8584845fa7ce78b928bfb6e47c5cf8eb8223982f`.
- AdGuardHome executable/config/workdir argv + start_time identity passed config-security #45339988745, Process inventory #45339988758 and AI docs #45339988750 on `cbbb27636807b7f195d7d87ccefb476366a07995`.
- Adbyby executable/start_time identity passed host #45339125309 and Process fail-closed #45339125213 on `17d08de1ed22fe2ff102992d28c974b42b77de3a`.

## Active task

Inadyn executable-bound identity and stop/reload/manual refresh:
- Previous validator used `process_identity_validate("inadyn", identity)` only; that validates comm, argv hashes and start_time but does not bind the process to the installed executable.
- `trunk/user/inadyn/Makefile` installs the daemon at `/bin/inadyn`. The new policy requires exact `/proc/PID/exe` match, accepts only the kernel's exact ` (deleted)` suffix, maps ENOENT/ESRCH to ABSENT, and fails closed on other procfs read errors.
- A filtered inventory removes wrong-executable same-name decoys from both the normal Inadyn process-presence probe and the shared service-lifecycle stop wait loop. UNKNOWN from any candidate blocks signals.
- The Inadyn stop path now uses the shared bounded service lifecycle policy; config reload SIGHUP and manual refresh SIGUSR1 validate the same exact executable identity immediately before signaling.
- Host policy tests and the Inadyn lifecycle source-contract checker/workflow are updated. Exact Actions are pending on the commit that adds this slice.

## Recent regression correction

- FRP exact run #45341083663 on `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a` failed at `client missing` because `remove_process()` in host fixtures removed only `cmdline`, leaving the synthetic PID and executable link visible. Production correctly treats that state as UNKNOWN.
- Commit `5687c609137bbba09c707896dd95216c3df9e961` fixes `frp_lifecycle.sh` and `frp_stop.sh` to remove the complete synthetic PID directory on actual process exit. Explicit missing-cmdline and post-TERM UNKNOWN fixtures still preserve `/proc/PID/exe`.
- On `5687c609`, FRP asynchronous startup #45341607742, Process inventory #45341607727 and AI handoff docs #45341607707 were IN_PROGRESS at last poll. Recheck their completed state; no result is assumed.

## Verification boundary

- This Inadyn slice includes GNU89-compatible injectable executable validation tests and workflow execution, but exact-head Actions are not yet available. Do not claim PASS before the exact SHA run finishes.
- The Inadyn workflow runs GNU89 with `-Wall -Wextra -Werror -fsanitize=address,undefined`; Process source contract and XY-C1 build validation are also required for this production `services_ex.c` change.
- No MIPS firmware build/TRX/artifact result or physical-router L4 result is claimed for this commit.
- Linux 4.4 lacks pidfd; final `/proc` validation-to-`kill(2)` race remains a platform limitation, even with executable, argv and start-time binding.

## Exact first action for the next GPT

1. Inspect exact-SHA Inadyn lifecycle policy, Process lifecycle, AI handoff and XY-C1 Actions for this commit; fix any first failure.
2. Poll FRP #45341607742, Process inventory #45341607727 and AI docs #45341607707 on `5687c609`; fix a regression if any remains.
3. Verify XY-C1 Build, TRX and artifact upload all pass on the same production-source SHA before any L3 claim; then continue the next P1 stability/security issue outside closed Adbyby/AdGuardHome/FRP scopes.
4. Keep the backup immutable; all main changes remain complete-tree atomic commits with expected-SHA non-forced updates.

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
