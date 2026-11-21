# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Live main parent for this slice: `cd04be437693e91e029f4b2f64d415ae7017e9eb`; this commit changes RC USB boot selection, its semantic source contract, the roadmap and handoff/log records.
- Radio stabilization on product-source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e` is L2 PASS: Wireless #44897978738 SUCCESS on checker child `310e7c8668db0eda362a0400775d5670cf7a2242`; iNIC #44898111180 SUCCESS on checker child `cd04be437693e91e029f4b2f64d415ae7017e9eb`; AI docs #44898111202 SUCCESS. The XY-C1 run #44897419694 for exact production-source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e` completed Build XY-C1, TRX verification and artifact upload successfully (L3 VERIFIED). No physical router L4.
- Newly confirmed P1 defect fixed in this commit: `load_usb_modules()` used error-collapsing `nvram_get_int()` for `modem_rule` and `modem_type`; transient NVRAM failures silently became zero and could bypass the configured modem driver readiness gate. It now uses checked NVRAM reads, preserves absent-key zero defaults, fails the boot-status accumulator on backend/malformed values, and only reloads the modem family once both values are valid. Independent printer/storage/USB host module attempts continue.
- `scripts/check_modem_module_reload.py` now checks the checked reader, no unchecked modem rule/type getters, fail-closed ordering and independent host-module continuation. The existing USB workflow compiles the checked-int policy regression under GNU89/Werror/ASan/UBSan and has push/PR path filters for both changed source and checker.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable. Legacy PR #1 remains closed/unmerged.

## Active task
- Verify the exact-head USB modem identity workflow, Process lifecycle umbrella and AI handoff documentation Actions for this boot NVRAM fix.
- On any FAIL, inspect the earliest failing job log and repair the actual source/test contract without weakening the checked read/error gates.
- Inspect the current exact-head XY-C1 run and report L3 only when Build, TRX verification and artifact upload all pass for the same production-source SHA.
- Next distinct P1 area: continue startup-result propagation outside the already covered radio and modem configuration reads; prioritize concrete independent defects.

## Verification boundary
- Radio L2: PASS on source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e` (Wireless + iNIC regressions and docs passed on checker/docs descendants).
- Radio L3: PASS on exact source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e`, XY-C1 run #44897419694 Build + TRX + artifact all SUCCESS.
- USB modem boot NVRAM slice: source/test checker changes committed; exact-head USB/Process/AI Actions pending. No local test run.
- Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect the USB modem identity, Process lifecycle and AI documentation push runs whose `head_sha` equals it.
2. Read the first causal failure log for any failing workflow; fix the specific issue in one atomic main commit.
3. Inspect the current exact-source XY-C1 Build, TRX and artifact steps; do not infer L3 from host checks.
4. Continue with a new independent P1 startup/reliability/security issue outside the recently closed radio and modem-selection slices.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
