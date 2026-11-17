# 2026-11-18 continuation checkpoint — capture optional post-WAN down-hook failures

## Current repository state
- Parent main before THIS source slice: `2a98a8303c26414a8f5084b78f1642ea282d164c`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Factory-reset source `0d58928e`: GPIO workflow `44197451556` PASS; Process `44197451701` PASS; MiniUPnP `44197451163` PASS; AI handoff rerun `44197796498` on `b83c1cb` PASS. Exact XY-C1 run `44197454090` completed SUCCESS with Build XY-C1, TRX verification and artifact upload.
- Static IPoE address cleanup source `93c8ab7`: WAN route `44198950271` PASS; AI docs `44198949896` PASS. Exact XY-C1 run `44198950404` remains IN_PROGRESS at Build XY-C1; TRX and artifact steps pending.
- Optional WAN-up hook source `2a98a83`: WAN route `44200079558` PASS and AI docs `44200079479` PASS. Exact XY-C1 run `44200079523` is `IN_PROGRESS` at last poll, currently waiting at Build XY-C1; TRX/artifact not yet verified.
- The earlier EAPOL-launch cleanup fix `5d0618c` and checker repair `80dcdc6` are complete; WAN route run `44197177575` PASS. No full local checkout/MIPS toolchain; no local target build or physical-router L4 claim.

## Active task
- THIS slice captures and aggregates the exit status of the optional `SCRIPT_POST_WAN` down hook in `wan_down_internal()`. Previously `eval(script_postw, "down", ...)` was discarded even though teardown returns an accumulated status.
- The change logs a nonzero hook status without short-circuiting later PPPoE-MWAN detection/reconfiguration. A GNU89 first-error/later-action regression and the existing WAN route source contract enforce this behavior.

## Verification boundary
- Exact `2a98a83` WAN route `44200079558`: PASS; AI handoff `44200079479`: PASS.
- Exact `2a98a83` XY-C1 `44200079523`: `IN_PROGRESS` at last poll, with Build XY-C1 still in progress; TRX and artifact pending.
- Exact `93c8ab7` XY-C1 `44198950404`: IN_PROGRESS at Build XY-C1 at last poll; TRX and artifact pending.
- Exact `0d58928e` XY-C1 `44197454090`: SUCCESS for Build, TRX and artifact upload.
- THIS down-hook source slice has not yet run its exact-head CI. No local compiler/MIPS toolchain and no physical-router L4 claim.

## Exact first action for the next GPT
1. Fetch live `main`; inspect exact-`head_sha` WAN route, AI docs and XY-C1 Actions for THIS down-hook commit.
2. Require the WAN GNU89/ASan regression and complete up/down-hook source contract to pass. For XY-C1 require Build, TRX verification and artifact upload.
3. Recheck `93c8ab7` run `44198950404` and `2a98a83` run `44200079523` separately; do not infer one source SHA's build from another.
4. Repair the first causal failure in a separate atomic commit; continue an independent P1 issue while builds remain PENDING. Keep backup immutable and physical-router L4 NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state, exact-SHA Actions that materially changed, current handoff/roadmap facts, and the first actionable next task before ending. The checkpoint is mandatory; a document edit is conditional. Record new PASS/FAIL/PENDING state accurately. If the facts and first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
