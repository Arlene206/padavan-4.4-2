# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If facts remain accurate, do not create a no-op commit.

Finish the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue as one cross-chat engineering effort: recover once, avoid repeat audits, do not wait on pending CI when independent work is available, and do not ask the user for the next task.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA update. Never force-push, split a logical change into half-commits, use a work branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent for this source slice: `beee6893b7c14eccc94634fe51dc41d2e748b8b7`. This includes the IPv6 disable-cleanup result propagation introduced in `4fae20fcf182b8d8fbb31c445c59ca66ca46019a` and source-checker/path-filter corrections through `beee6893`.
- WAN route result propagation failure on `fb729ee` was isolated to a newline-sensitive workflow source-path assertion, then corrected in `b231b15` and `beee6893`. Exact-head WAN route run `43964529748` is in progress; exact-head AI handoff `43964529911` passed.
- The previous WAN route/IPv6 source parent `eb566f346f63d3d6ed295c08170ac6d834ff7f04` passed exact-head HTTPD `43961727463`, AI handoff `43961726579`, and XY-C1 `43961726533`; XY-C1 Build, TRX verification and artifact upload all succeeded.
- HW NAT radio/debugfs status propagation is in `0b32b9111c38e2cf7588cf6605b2491f25e3ce31`; WAN interface route cleanup is in `58318f53937622672e10d6bcea8b07f8a4753aa3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Never alter it.

## Active task

The strict `is_module_loaded_status()` probe distinguishes 1=loaded, 0=absent and negative errno=probe failure for unload operations. But `rc_module_load_verified()` treats every nonzero probe as success, and four production module-load callback sites (NAT, boot hardware, wireless driver and LAN bridge) still use the legacy Boolean `is_module_loaded()`. This source slice makes load verification preserve negative probe errno, migrates all four callers to the strict status callback, and adds direct plus multi-module injected regressions. Policy, callers, tests, source checkers and this handoff update are committed together on main.

## Verification boundary

- Parent `beee689` exact-head WAN route status regression was still running at last poll; the previous `fb729ee` run failed only at an over-broad assertion counting quoted workflow paths, fixed by the subsequent checker commits `b231b15` and `beee689`.
- Parent exact-head AI handoff contract `43964529911` passed.
- The earlier production parent `eb566f3` passed XY-C1 Build, TRX verification and artifact upload. The IPv6 cleanup changes introduced after it need source-SHA validation; do not reuse the earlier firmware result for those changes.
- New Process, WAN route, HTTPD and XY-C1 runs for this module-load source SHA must be checked by exact `head_sha`. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head WAN route status, Process lifecycle and XY-C1 runs. Repair any failure on that exact SHA; a pending build is not a reason to wait on unrelated independent work.
2. After this strict tri-state module-load fix passes, continue evidence-backed RC/service result-propagation work across userland without repeating completed audits.
3. Keep atomic full-tree main commits, expected-SHA non-forced updates and the frozen backup read-only. No force push or PR. Do not claim physical-router L4 without hardware evidence.
