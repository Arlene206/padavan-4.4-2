# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this slice: 46f7a9128cdc5c66d943d9d8093c002b336b5701. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807` and read-only.
- Exact-head 595bc Firewall Startup `43940508528`, AI handoff `43940508597`, and NVRAM audit `43940508621` passed.
- Exact-head 46f7 HTTPD `43941275581`, AI handoff `43941275602`, NVRAM `43941275573`, Process inventory `43941275869`, AliDDNS `43941275686`, AdGuardHome `43941275586`, AliyunDrive `43941275788`, and infosvr `43941277064` passed. Exact-head XY-C1 `43941275592` was still building when polled; TRX/artifact stages pending.
- This commit closes partial-cleanup path diagnostics and broadens firewall helper regression CI. Its own exact-SHA Actions are pending; no build result is transferred from parent SHA.
- Historical PR #1 remains closed/unmerged; backup remains read-only.

## Active task

Core firewall status propagation is in place: failed built-in table restore/rollback stays fail-closed, and restart_firewall() attempts UPnP independently while returning the first failure. Cleanup-only partial snapshot state is now explicitly reported even when snapshot_count is zero; exact cleanup paths and separate table/partial-path counts are logged. The transaction regression asserts EIO for that state.

Firewall Startup CI now covers all standalone firewall helper/security tests (Adbyby restore, command wait status, filter validation, hook-file staging, atomic rule files and snapshot-FD behavior) under GNU89/-Werror/ASan/UBSan.

Next P1: inspect extension hook staging, execution, and temporary-file cleanup status for Shadowsocks, persistent firewall hook, and Optware. Decide best-effort versus required behavior explicitly; always attempt independent hooks, but surface nonzero outcomes in start_firewall_ex() when policy requires. Then audit NAT module unload/load postconditions in net.c.

## Verification boundary

- Exact-head 46f7 non-firewall Actions listed above passed; XY-C1 `43941275592` was still building at last check. Parent SHA 595bc6 Firewall Startup and NVRAM passed, but these do not validate the new diagnostic/test-wiring commit.
- This commit has not yet received exact-SHA Actions. Its Firewall Startup workflow adds six existing helper regressions plus an explicit cleanup-only EIO assertion and canonical source contract.
- No local firmware build or physical-router validation is claimed. XY-C1 L3 requires same-SHA Build, TRX validation and artifact upload. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect this commit's exact-SHA Firewall Startup helper/security regressions, XY-C1 Build/TRX/artifact, and AI handoff contract. Diagnose any first failing job.
2. Continue with Shadowsocks, persistent post-firewall, and Optware hook staging/command/cleanup status propagation; add deterministic failure-injection coverage and preserve independent-hook attempts.
3. Then audit NAT module unload/load postconditions. Main-only complete-tree atomic commits with non-forced expected-SHA CAS; frozen backup read-only; router L4 NOT VERIFIED.
