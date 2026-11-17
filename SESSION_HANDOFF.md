# 2026-11-18 continuation checkpoint — capture optional post-WAN hook failures

## Current repository state
- Parent main before THIS source slice: `93c8ab702d886e7da1fe145bef5655c37dbe4a6d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Factory-reset source `0d58928e`: GPIO workflow `44197451556` PASS; Process `44197451701` PASS; MiniUPnP `44197451163` PASS; AI handoff rerun `44197796498` on `b83c1cb` PASS. Exact XY-C1 run `44197454090` on `0d58928e` completed SUCCESS: Build XY-C1, TRX verification and artifact upload all PASS.
- Static IPoE address cleanup source `93c8ab702d886e7da1fe145bef5655c37dbe4a6d`: exact WAN route `44198950271` PASS (all GNU89/ASan/UBSan regressions and source contracts); AI handoff `44198949896` PASS. Exact XY-C1 run `44198950404` was `IN_PROGRESS` at last poll; inspect Build/TRX/artifact before claiming L3.
- The earlier EAPOL-launch cleanup fix `5d0618c` and checker repair `80dcdc6` are complete; WAN route run `44197177575` PASS. No full local checkout/MIPS toolchain; no local target build or physical-router L4 claim.

## Active task
- THIS slice captures and aggregates the exit status from the optional `SCRIPT_POST_WAN` hook in `wan_up_internal()`. Previously `eval(script_postw, "up", ...)` was discarded, making a failed configured post-WAN action invisible to callers even though the function returns accumulated startup status.
- The change logs nonzero hook status and preserves it as the first route status without short-circuiting later independent ZeroTier/PPPoE-MWAN startup. A GNU89 first-error/later-action regression and the existing WAN gateway/post-start source checker enforce the behavior.
- Exact-head WAN route and XY-C1 Actions for THIS commit remain pending at handoff time.

## Verification boundary
- Exact `93c8ab7` WAN route `44198950271`: PASS, including the static-IPoE address-cleanup regression and source-order gate. AI handoff `44198949896`: PASS.
- Exact `93c8ab7` XY-C1 run `44198950404`: `IN_PROGRESS`; require Build XY-C1, TRX verification and artifact upload.
- Exact `0d58928e` XY-C1 run `44197454090`: SUCCESS on the exact product source, including Build, TRX and artifact upload.
- No local compiler/MIPS toolchain and no hardware L4 claim.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-`head_sha` WAN route, AI handoff and XY-C1 runs for THIS commit; require the post-WAN hook source contract and GNU89 policy binary to pass.
2. Require exact-head XY-C1 Build, TRX verification and artifact upload. Recheck `93c8ab7` run `44198950404` and record any completion without conflating it with THIS newer SHA.
3. Repair the first causal failure in a separate atomic commit; continue an independent P1 issue while any build is PENDING.
4. Keep backup immutable and physical-router L4 NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state, exact-SHA Actions that materially changed, current handoff/roadmap facts, and the first actionable next task before ending. The checkpoint is mandatory; a document edit is conditional. Record new PASS/FAIL/PENDING state accurately. If the facts and first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
