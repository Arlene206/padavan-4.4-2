## Current repository state

- Parent main for this AdGuardHome identity-hardening slice: `a26746d033ffa19d9b576010f4fe4a583b4a859a`; live main/tree were read immediately before atomic tree construction.
- Adbyby executable/start_time hardening: exact Adbyby host #45339125309 PASS and Process fail-closed #45339125213 PASS on `17d08de1ed22fe2ff102992d28c974b42b77de3a`.
- AdGuardHome baseline after test-only repairs: exact config-security #45339223189 PASS and Process inventory #45339223352 PASS on `9ce145922915fa4b09217b1253a5490a9e2fbda9`.
- This slice addresses the same confirmed PID-ownership weakness in AdGuardHome: numeric `pidof` candidates are no longer sufficient evidence to signal a process.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Main-only atomic complete-tree commits, expected-SHA non-forced updates, no force-push.

## Active task

Implement AdGuardHome process identity:
- Match `/proc/PID/exe` to `/usr/bin/AdGuardHome`, require the service's exact config/workdir argv tuple, parse non-zombie `/proc/PID/stat` start_time.
- Snapshot stores `PID:start_time`; TERM and refreshed SIGKILL each revalidate identity immediately before signaling.
- Host regression uses synthetic procfs for wrong executable, wrong config argv and malformed metadata; existing multi-PID and changed-PID escalation tests remain.
- Exact-head AdGuardHome config-security and Process inventory/fail-closed Actions are required after publication.

## Completed development slices

- Adbyby process identity hardening on parent `9ce145922915fa4b09217b1253a5490a9e2fbda9`: pidof candidates are bound to exact `/proc/PID/exe` and parsed non-zombie `/proc/PID/stat` start_time; stop revalidates both immediately before TERM. Host fixtures cover multiple PIDs, same-name wrong-executable decoys, malformed proc metadata and changed start_time.
- WAN router hostname/domain updater: this atomic commit; source regression integration is included. The on-disk file pair remains published if a kernel name syscall fails; the DNSMasq caller receives the failure before daemon preparation.
- Previously completed unrelated modules and checks are retained in `REFACTOR_ROADMAP.md`; do not repeat their audits without a new failing case.

## Verification boundary

- Baseline `9ce1459`: AdGuardHome config-security #45339223189 PASS; Process inventory #45339223352 PASS. Those checks predate the new executable/argv identity code in THIS COMMIT.
- Exact-head checks for THIS COMMIT are pending immediately after publication; no new MIPS build or physical-router result is claimed.
- Linux 4.4 lacks pidfd support in this firmware baseline; executable/argv/start_time are revalidated directly before each signal, but the final tiny validate-to-signal race cannot be eliminated here.
- No same-SHA L3 claim; target hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect this commit's exact-head AdGuardHome config-security, Process inventory/fail-closed and AI handoff Actions; fix any first failing assertion or fixture.
2. If these checks pass or remain pending, continue a separate evidence-backed P1 stability/security fix outside Adbyby, AdGuardHome and SmartDNS rather than re-auditing completed work.
3. Keep the frozen backup immutable and main updates atomic/non-forced.

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
