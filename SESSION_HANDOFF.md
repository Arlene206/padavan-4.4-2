# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent for this checker-correction slice: `f1f624e3f667e67376031233f6e2f9a53f6f31df`.
- Per-interface IPv6 address/route/neighbour flushes in `net6.c` use argv-based `eval()`, preserving invalid-name rejection, absent-interface no-op and command-result propagation.
- Exact parent push Actions: 11 SUCCESS. WAN route `43987743317` and WAN DHCP callback `43987743207` failed in source-contract-only steps because the checkers still expected `doSystem()` for the three flush calls; preceding compiled test steps passed. Both stale assertions are corrected in this commit. XY-C1 run `43987743135` remained IN_PROGRESS at last poll.
- The separate IPv6 global sysctl policy regression compiled and passed locally with GNU89, ASan and UBSan. The per-interface argv assertions passed locally.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Verify the corrected WAN route and WAN DHCP Actions on this commit's exact SHA, diagnose any remaining failure, and inspect XY-C1 Build/TRX/artifact completion. After that, continue the bounded `net_wan6.c` route/tunnel shell-boundary audit; do not re-audit the completed `net6.c` flush change.

## Verification boundary

- Parent `f1f624e3f667e67376031233f6e2f9a53f6f31df`: 11 push workflow runs SUCCESS, two inline-checker-only failures identified above, XY-C1 still running at last poll.
- The current commit changes only workflow regression expectations and handoff/roadmap/log records; its exact-head checks must complete before marking the checker correction green.
- Product-source MIPS/uClibc L3 (XY-C1 Build, TRX and artifact on one SHA) and physical-router L4 remain **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch Actions `/actions/runs?head_sha=<current main SHA>`; confirm WAN route, WAN DHCP callback and XY-C1 on that exact SHA. Resolve the first causal failure; do not rely on parent results.
2. Resume `net_wan6.c` only after the test correction is green. Validate provenance of SIT/6rd parameters and convert remaining dynamic shell-formatted tunnel/route operands to argv while preserving fail-fast macros and rollback order.
3. Keep the backup branch frozen. Do not claim product-source L3 until Build, TRX verification and artifact upload pass on one exact SHA; physical-router L4 remains NOT VERIFIED.
