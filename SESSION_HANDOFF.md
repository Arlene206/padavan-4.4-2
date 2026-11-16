# Cross-session execution mandate

Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Current main before this checker/handoff repair: `f66dc26260ba426245ad403b6703b5ba6d52336a`.
- Latest product-source commit: `b2d3288b6282c16cf23859eb9e7f8fdce083cd62` (LAN restart first-error propagation).
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, move, rewrite or delete.
- Startup protocol, bootstrap, handoff, roadmap, log archive/index, branch inventory and legacy PR #1 were audited earlier. PR #1 is closed, valid AliDDNS work already on main. Do not repeat the audit.
- UPnP stop-result work at `68da0e2bd19dab24b3ab0b576c87cbf43e25832a`, with workflow follow-up `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`, is complete. Exact c6 Process `43811174980`, miniupnpd `43811175012`, AI docs `43811174933`, and XY-C1 `43811175036` all PASS; XY Build, TRX and artifact upload succeeded on c6 only.

## Active task

The LAN restart source commit `b2d3288` now aggregates first failures from status-bearing stop, restore, restart, reload, start and HTTPD logout operations in `full_restart_lan()`. It reuses `rc_first_service_start_status()`, extends the GNU89 service-status regression, and checks PID 1 notification aggregation. The test itself passed in both Process and XY-C1 jobs on b2.

There are two concrete follow-ups from exact Actions:

1. Process workflow was malformed in b2 because PR path entries were concatenated on one line. Commit `f66dc26260ba426245ad403b6703b5ba6d52336a` repaired the YAML. The resulting Process run `43814587870` reached its final source checker but failed at the pre-existing assertion expecting literal `stop_lltd();`, which the production change intentionally wrapped in `lan_restart_record_status(..., stop_lltd())`. THIS COMMIT updates the ordering contract to match the new checked callsites and preserve the lock/rollback ordering assertion.
2. AI handoff run `43814587897` failed because the current handoff did not preserve the exact required checkpoint sentence and canonical section headings. THIS COMMIT restores the mandatory checkpoint language and all validator-required headings.

No production source behavior is changed in this checker/document repair.

## Verification boundary

- Exact `b2d3288b6282c16cf23859eb9e7f8fdce083cd62`: service-status regression passed in Process and XY-C1. Process workflow run `43814394160` failed before a job ran due to malformed YAML. AI docs `43814395799` failed because the handoff omitted the mandatory end-of-session accuracy checkpoint. XY-C1 run `43814395678` remained IN_PROGRESS at last check, after the LAN service status test passed; Build, TRX and artifact upload were still pending.
- Exact `f66dc26260ba426245ad403b6703b5ba6d52336a`: Process `43814587870` failed only at the old literal stop-call ordering assertion (compile and preceding regressions passed); AI docs `43814587897` failed on the same missing handoff phrase. Other exact-head Adbyby/AliDDNS/AliyunDrive workflows `43814588010`, `43814587964`, `43814587985` passed.
- The latest confirmed L3 firmware baseline is older exact SHA `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`. No Build/TRX/artifact success is claimed for b2, f66, or current main. No L4 physical-router test has been performed.

## Exact first action for the next GPT

1. Check the exact-head Process lifecycle and AI handoff Actions after THIS repair. Confirm the process source checker reaches PASS and the handoff validator accepts this record; inspect first causal logs if not.
2. Check b2 XY-C1 run `43814395678` for its final Build, TRX verification and artifact upload status. For current L3, require all three on the latest product-source SHA; do not transfer c6 evidence.
3. Continue the P1 full-repository RC status-return audit outside `full_restart_lan()`, fixing only confirmed dropped results with caller-aware regression tests, and work on independent modules while unrelated checks are pending.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root dev_log for every code/documentation commit. Keep root log within 20–119 records and 384 KiB; preserve archives and index. At every handoff checkpoint distinguish product-source SHA from current docs/checker SHA and record exact run IDs/statuses.
