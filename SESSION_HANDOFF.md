# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for the validation correction: `1fe9fd834f4fb735b7b23fb7005b5dd9a61f7e86`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Exact-head 2ed Firewall Startup `43944601860` and Process regression `43944601739` failed in the same canonical source assertion: checker expected no whitespace around `status = rc_first_service_start_status(status, hook_error)`, but the C source has nine valid merges. The checker is fixed with a whitespace-tolerant regex.
- Exact-head 2ed HTTPD `43944601745` failed because its inline checker expected bare hook-file remove calls; the source now checks their return values. All three hook cleanups are aligned to the status-aware API.
- Exact-head 2ed XY-C1 `43944601903` hit the shared WAN source checker before Build/TRX/artifact. NVRAM `43944601815`, AI handoff `43944601727`, Process inventory `43944601706`, FULLCONENAT `43944601842`, and USB modem identity `43944601920` were IN_PROGRESS at last poll.
- Helper parent c0 exact Firewall Startup/HTTPD/NVRAM/AI/Process inventory/Process regression all passed.

## Active task

Optional Shadowsocks, persistent post-firewall and Optware hooks now propagate non-ENOENT staging errors, command failures and staged-file cleanup failures to start_firewall_ex(), while continuing independent hooks. Missing optional hooks remain no-op; hook errors do not disable successful built-in firewall rules. Failed unlink preserves the staged path.

The implementation was green in the focused helper parent, but the subsequent source commit exposed stale exact-string assertions in two integration suites. Those checker contracts are now aligned; fresh exact-SHA verification must prove the complete source path and XY-C1 build.

Next P1: after validation, audit NAT helper and HW NAT/SFE module load/unload postconditions in net.c, keeping required ALG helpers observable and optional accelerators non-blocking.

## Verification boundary

- 2ed exact Firewall Startup `43944601860` and Process regression `43944601739` failed at one whitespace-sensitive count assertion in the canonical source checker, after prior module, UPnP, firewall and cleanup checks passed.
- 2ed HTTPD `43944601745` failed at its stale assertion that hook-file removal was a bare call; it now checks the status-aware return condition.
- 2ed XY-C1 `43944601903` stopped at the shared source checker before firmware build/TRX/artifact.
- This commit updates only checker contracts and handoff records. No new-SHA verification or L3/physical-device claim yet; router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-SHA Firewall Startup, Process, HTTPD, XY-C1 Build/TRX/artifact and remaining NVRAM/AI/service runs after this checker repair.
2. If these pass, continue NAT helper and optional HW NAT/SFE load/unload postcondition auditing, with targeted tests for failed unload and stale module-state reports.
3. Main-only complete-tree atomic commit, non-forced expected-SHA CAS. Frozen backup read-only; physical-router L4 NOT VERIFIED.
