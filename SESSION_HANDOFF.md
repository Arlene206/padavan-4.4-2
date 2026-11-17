# 2026-11-18 continuation checkpoint — align WAN teardown source contract

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS checker correction: `853748bd0a49661ac29c7b854a58e40571393a20`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Ralink IAPP firewall ownership work: exact a700 IAPP `44201354894` PASS; AI `44201354982` PASS; Process lifecycle `44201355348` and Process fail-closed `44201354872` PASS. Exact a700 NVRAM `44201355174` and XY-C1 `44201354975` were pending at last poll.
- THIS parent `853748bd` added a fail-closed WAN teardown fix: if static-IPoE address clear fails, it immediately tries to take the physical WAN interface down; clear and force-down errors are both aggregated/logged, while later independent teardown continues. Exact WAN interface teardown `44201908894` PASS; WAN route workflow `44201908899` FAIL only because its embedded source contract still expected the old declaration without `interface_cleanup_status`. AI handoff `44201908973` PASS. Exact XY-C1 `44201909002` and NVRAM `44201909080` still IN_PROGRESS at last poll.
- Older factory-reset source `0d58928e` is L3 verified by exact XY-C1 `44197454090` Build/TRX/artifact. Do not infer L3 for newer WAN source.
- Backup unchanged; local MIPS build not available; physical-router L4 NOT VERIFIED.

## Active task
- THIS commit corrects only the embedded WAN source-contract assertion. Production code's function-local state now declares `int status = 0, route_status, interface_cleanup_status;` so teardown can preserve the forced-interface-down result after an address-clear failure. The previous test still expected the old two-variable declaration and failed at Python assertion line 220.
- The actual teardown policy regression and source implementation were otherwise accepted by the separate WAN interface teardown workflow; no production source changes in THIS commit.

## Verification boundary
- Parent 853 WAN interface teardown `44201908894`: PASS.
- Parent 853 WAN route `44201908899`: FAIL at embedded source-contract line 220 for stale declaration assertion; THIS correction updates it.
- Parent 853 AI `44201908973`: PASS. XY-C1 `44201909002` and NVRAM `44201909080`: PENDING at last poll. Exact WAN-route rerun and current-source XY build are pending.
- No local compile/MIPS toolchain; physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head WAN-route rerun; require the embedded ordering checker, policy C regression and standalone WAN checker to pass.
2. Inspect exact-head XY-C1; require Build XY-C1, TRX verification and artifact upload before recording L3.
3. Recheck the separate 853 XY-C1 `44201909002` and NVRAM `44201909080` outcomes, then continue next independent P1 stability/security issue.
4. Keep backup unchanged; do not claim local target compile or hardware L4.

## Mandatory handoff checkpoint
At session end, inspect live main, exact-SHA Actions, current handoff/roadmap and first actionable task. A checkpoint is mandatory; a documentation-only commit is conditional on facts needing an update. Record PASS/FAIL/PENDING accurately.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and atomically commit. Do not wait idly on pending CI or ask the user what to do next.

Every logical multi-file change must create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main, and publish with a non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
