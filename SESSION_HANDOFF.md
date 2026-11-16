# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS module-load verification slice: `59047eb21838e0cf78dd24e0a6eab78bea7fc886`; fetch live main and use expected-SHA non-forced CAS on the next write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. The one-time startup comparison found main 588 commits ahead / 1 behind; do not repeat branch/archive recovery.
- The latest source milestone is `2c0a0f1be6edf5a604a71ce02e726d0b2393a00c`: switch base/storm/link PHY failures now reach PID1/runtime notification status. A separate documentation-contract correction is `59047eb21838e0cf78dd24e0a6eab78bea7fc886`.

## Active task

A review found two unverified module-load call sites inside LAN bridge initialization:
- `module_smart_load()` is documented by `wireless_module_load_policy.h` as returning zero both for already-loaded modules and failed `modprobe`. Existing `rc_module_load_verified()` checks post-load `/sys/module` state and has a dedicated regression.
- On `USE_RT3352_MII` boards, `iNIC_mii` is required for the MII/iNIC LAN path. It now uses post-load verification and records failure into `init_bridge()`/PID1 startup health.
- `hw_nat` is optional L2 offload. It now uses post-load verification and reports enabled/unavailable accurately without classifying offload absence as essential bridge failure.
- Existing verified module policy test is already compiled/run by Process workflow; new source contracts assert both call sites and their required/optional classification.
- The switch status result propagation from the previous source milestone remains in place: base (jumbo/green/EEE), four storm settings and WAN+LAN port link controls are checked; unsupported MTK optional features are successful no-ops, LEDs remain best-effort.

## Verification boundary

- Exact-head Process run on earlier bridge commit `aa5d3c9f2566927a4b17fce5d391f525cccc8095`: `43857640159`, job `133265909696`, SUCCESS; bridge init/member policy tests and LAN source contract passed.
- Exact-head AI handoff documentation run `43858670960` on `2c0a0f1be6edf5a604a71ce02e726d0b2393a00c` failed because a required explicit end-of-session checkpoint sentence was missing. Job `133269446982` identified the exact validator failure; checkpoint wording was restored in `59047eb21838e0cf78dd24e0a6eab78bea7fc886`. Its exact rerun was in progress at last inspection.
- On `2c0a0f1be6edf5a604a71ce02e726d0b2393a00c`, Process `43858670762` and XY-C1 `43858670860` were queued at last query; several service and security runs were in progress. Recheck exact run IDs and outcomes rather than assuming them.
- THIS module verification change requires fresh exact-SHA Process and XY-C1 Actions. No same-SHA firmware Build+TRX+artifact L3 or physical router L4 result is currently established.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process, XY-C1, NVRAM, HTTPD security, AI docs and relevant service Actions for THIS change; repair any causal failure.
2. Verify the module post-load source contract and tests pass; no host-only result substitutes for target toolchain.
3. Continue scanning RC initialization for unchecked required modules or startup operations, while keeping HW offload/LED/other optional hardware accelerators non-fatal.
4. XY-C1 L3 is established only when Build, TRX validation and artifact upload all pass on one identical production-source SHA; physical hardware remains L4-only.

## Verification and governance rules

Preserve security, architecture, compatibility, target toolchain and required tests. Main only; frozen backup read-only. One full Git tree and atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for source milestones and keep its 20–119 newest entries. Distinguish host regression, exact-SHA Actions, firmware/TRX L3, and physical-router L4. Never claim an unrun result.
