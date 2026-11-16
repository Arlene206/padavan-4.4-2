# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS IPTV restart source-contract correction: c066d1f230a2cadfceb091ccc9f45233f17dff09; refetch live main before any later write and use expected-SHA non-forced CAS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Startup governance/archive/PR recovery are complete; do not repeat.
- Root `dev_log.md` has 99 newest-first records before THIS entry; this entry makes 100.

## Active task

Repair the storage workflow's stale source contract for the already-changed IPTV restart result semantics:
- Parent source commit `b81207777615d31b096c1425ff1fa8110898b0c2` changed `restart_iptv()` to return the first result across `config_bridge()`, router proxy start, AP proxy stop, and enabled proxy launches. The AP branch correctly retains the stop-before-replace safety gate: failed stop returns before starting UDPxy/XUPnPD.
- USB storage workflow run `43850461648` on exact source SHA `b81207777615d31b096c1425ff1fa8110898b0c2` passed the storage, lock, BusyBox/ash and caller-graph checks, then failed in `scripts/check_storage_safe_remove.py` because its old IPTV assertion still expected `return start_igmpproxy(wan_ifname);` and bare `return stop_status;`.
- THIS correction asserts the new result aggregation and the AP failed-stop early-return ordering without weakening the behavioral check. Workflow push filters already include this checker path.
- The bridge Process checker correction was committed at `c066d1f230a2cadfceb091ccc9f45233f17dff09`; exact-head Process `43850767730` passes all steps including the bridge policy GNU89/ASan/UBSan regression and source integration. AI docs `43850767697` and selected service/security runs also passed. NVRAM `43850767802` was pending at checkpoint.
- No physical-router/L4 result is established. Exact b812 XY-C1 run `43850461476` was still at Build when last checked; TRX/artifact pending.

## Verification boundary

Exact-head observations on parent `c066d1f230a2cadfceb091ccc9f45233f17dff09`:
- Process regression `43850767730`: SUCCESS. All steps completed, including bridge multicast status regression and production source/caller contract.
- Process inventory `43850461826` on ancestor `b81207777615d31b096c1425ff1fa8110898b0c2`: SUCCESS.
- AI handoff documentation `43850767697`, infosvr `43850767611`, AdGuardHome `43850767872`, AliDDNS `43850767755`, AliyunDrive `43850767862`: SUCCESS.
- NVRAM transaction boundary `43850767802`: IN_PROGRESS at last inspection.
- USB storage hotplug `43850461648` on source SHA `b81207777615d31b096c1425ff1fa8110898b0c2`: FAILED only in the source checker at obsolete `restart_iptv()` return assertions. Storage/eject lock/BusyBox and all prior caller-graph assertions passed. THIS COMMIT updates that stale assertion; its new exact-head storage run is not yet known.
- XY-C1 Firmware Validation `43850461476` on `b81207777615d31b096c1425ff1fa8110898b0c2`: IN_PROGRESS at Build XY-C1; TRX verification and artifact upload were pending at last check. It is a firmware-source ancestor of this checker-only commit, not an exact run for current main. No physical-router/L4 result.

## Exact first action for the next GPT

1. Fetch live main and inspect THIS COMMIT's exact-head USB storage workflow. Require the revised IPTV result contract, all storage lock/eject gates, BusyBox/ash build and integration checks to pass; diagnose any subsequent failure from the first causal log.
2. Recheck NVRAM `43850767802` on parent `c066d1f230a2cadfceb091ccc9f45233f17dff09` and XY-C1 `43850461476` on production source ancestor `b81207777615d31b096c1425ff1fa8110898b0c2`. Record target L3 only if Build, TRX verification and artifact upload all pass on the same production-source SHA.
3. Continue the next independent RC boot status slice (loopback interface/route or remaining service launcher) after resolving this known red storage contract. Keep optional bridge capability failures distinct from required service failures.

## Governance constraints

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One complete Git tree and one atomic commit per logical task, non-forced expected-SHA update. Update dev_log.md for each source/documentation change; keep 20–119 records and preserve archive shards/indexes. Distinguish host tests, source contracts, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
