# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS bridge source-checker literal correction: b81207777615d31b096c1425ff1fa8110898b0c2; refetch live main before any later write and use expected-SHA non-forced CAS.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 = 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable. Startup governance/archive/PR recovery are complete; do not repeat.
- Root dev_log.md has 98 newest-first records before THIS entry; this entry makes 99.

## Active task

The bridge status implementation is in b81207777615d31b096c1425ff1fa8110898b0c2:
- A policy test and production wiring now aggregate all seven bridge/port/PHY results through config_bridge(), start_lan() and restart_iptv().
- GNU89/Werror/ASan/UBSan regression passed locally before commit; exact-head Process workflow is still running.
- Review found the new Process source checker used a doubled backslash escape for the function return-type check. This correction uses chr(10) to match the actual C newline without ambiguous escaping. The prior source check is not yet claimed PASS.
- XY-C1 run 43850461476 was queued/in progress after dependency setup at last observation; other exact-head Actions are still running/queued.

## Verification boundary

Exact-head observations on parent b81207777615d31b096c1425ff1fa8110898b0c2:
- Process regression 43850461801: IN_PROGRESS at "Compile and run RC boot service status aggregation regression" when last inspected; the bridge-policy regression/source checks follow.
- XY-C1 43850461476: IN_PROGRESS during build dependency setup; no Build/TRX/artifact L3 result yet.
- Process inventory 43850461826: SUCCESS.
- Other parent Actions are queued/in progress; fetch the live exact-head run list and inspect any failing job logs before claims.
- No physical-router/L4 result.

## Exact first action for the next GPT

1. Inspect the corrected Process bridge regression/source checker and the exact-head XY-C1 Build/TRX/artifact sequence. Correct any compile, sanitizer, source-contract or firmware failure before selecting another task.
2. Continue capability-aware bridge/IPTV result handling. Unsupported multicast sysfs or switch ioctl operations may be board-dependent; preserve observability without aborting unrelated service starts.
3. Continue broad RC stability/security governance. Selected NDIS interface binding remains unproven separately from module-family presence.

## Governance constraints

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One complete Git tree and one atomic commit per logical task, non-forced expected-SHA update. Update dev_log.md for each source/documentation change; keep 20–119 records and preserve archive shards/indexes. Distinguish host tests, source contracts, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
