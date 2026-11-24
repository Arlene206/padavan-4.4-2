## Current repository state

- Active development target: `main`. Source parent for the current checker correction: `822d8a8c40808290825d838d746d7fec034bb2fc`; this atomic commit aligns the SmartDNS source checker with the tri-state helpers already present on that parent.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Legacy PR #1 is closed and unmerged.
- DNS Forwarder procfs confirmation fix `0761c14cd929958b6398fa862f93e74b635d82ce` has DNS Forwarder #45424750772, AI docs #45424750770 and Process inventory #45424750807 PASS. Its missing XY-C1 source-path filter was added in parent `822d8a8c`.

## Active task — validate SmartDNS lifecycle checker correction

- Process inventory #45425821194 on exact SHA `822d8a8c40808290825d838d746d7fec034bb2fc` ran `smartdns_process_lifecycle.sh` successfully through all runtime cases. It failed afterward in `scripts/check_smartdns_lifecycle.py` because stale assertions still expected `_smartdns_process_name=${1:-smartdns}`, `pidof "$_smartdns_process_name"`, `smartdns_process_state adbyby` and `killall smartdns`.
- Actual code now uses `smartdns_named_process_state()` with `_smartdns_name=$1` for Adbyby, `smartdns_process_state()` for exact SmartDNS identity and procfs completeness, then PID-scoped TERM/KILL with per-PID identity revalidation and a fresh stopped-state postcondition. The checker now asserts those current behaviors.

## Verification boundary

- Parent `822d8a8c40808290825d838d746d7fec034bb2fc`: AI handoff #45425821211 PASS; Process inventory #45425821194 FAILED only at the obsolete Python source assertion after the shell runtime regression passed; XY-C1 #45425821347 was IN_PROGRESS at last poll.
- The checker correction requires a fresh exact-head Process inventory and AI handoff run. The pending XY-C1 action belongs to parent `822d8a8c`; check its final conclusion separately. Do not claim L3 unless Build, TRX verification and artifact upload all pass on the same production-source SHA.
- Physical-router L4 and target runtime behavior remain unverified unless new exact hardware evidence is produced.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head Process inventory and AI handoff runs for this checker correction; fix any remaining stale contract assertion.
2. Resolve parent `822d8a8c` XY-C1 run #45425821347 and verify Build/TRX/artifact on the same SHA; diagnose any failure.
3. Continue the next independent P1 stability/security issue rather than waiting for pending CI. Do not repeat the already-passing DNS Forwarder runtime/Process/AI checks without new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live `main`, exact-head relevant push Actions, the validation boundary, blockers and the precise next task. Update this file only when resumable facts materially change.

# Cross-session execution mandate

Recover once, then implement. Keep work source-led: a bounded source fix, meaningful regression, necessary validation, one complete Git tree and one atomic main commit, followed by a non-forced compare-and-swap. Pending CI does not block independent work. Before context exhaustion, finish the safe slice and leave the next GPT one precise first action. Never force-push, use work branches/PRs, or overstate host, firmware or hardware evidence.
