# Cross-session execution mandate

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS switch configuration status slice: `4322737edbbe77196e4380fc7506da3f0851dc0d`; fetch live main and use expected-SHA non-forced CAS for the next write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind; do not repeat backup/archive recovery.
- Previous router GMAC2/AP-tagged member status correction is committed at `4322737edbbe77196e4380fc7506da3f0851dc0d`.
- Root `dev_log.md` had 104 newest-first records before this entry; keep 20–119 and preserve archive shards/indexes.

## Active task

All switch initialization configuration now preserves checked PHY errors:
- `switch_config_base()` captures jumbo frame, green Ethernet and EEE LPI results; LED indication stays best-effort because it does not determine forwarding readiness.
- `switch_config_storm()` captures all four unknown-unicast, unknown-multicast, multicast and broadcast storm-control operations.
- `switch_config_link()` captures WAN and every configured LAN port speed/flow-control result.
- The switch driver API already returns `int`. MTK ESW explicitly reports success/no-op for unsupported optional storm/green/EEE features; real nonzero ioctl failures can be returned without making unsupported board capabilities fatal.
- Each helper attempts its independent settings, preserves first failure, and logs the first failed setting. `init_bridge()` merges all three into PID1 boot health; `RCN_RESTART_SWITCH_CFG` merges them into runtime notification results.
- Process source contracts now verify setting coverage and caller status aggregation.

## Verification boundary

- Exact-head Process for `aa5d3c9f2566927a4b17fce5d391f525cccc8095`: run `43857640159`, job `133265909696`, SUCCESS. Bridge-init and bridge-membership policy binaries and LAN source assertions passed. This predates the GMAC2/AP-tagged branch correction `4322737...` and THIS switch status slice.
- Exact-head push Actions for `4322737edbbe77196e4380fc7506da3f0851dc0d` were IN_PROGRESS at last inspection: Process `43858257725`, XY-C1 `43858257611`, NVRAM `43858257798`, AI docs `43858257985`, HTTPD auth `43858257690`, plus multiple app/lifecycle runs. Recheck; do not wait on pending runs.
- THIS change requires fresh exact-SHA Actions. No same-SHA XY-C1 Build+TRX+artifact L3 or physical router L4 result is currently claimed.

## Exact first action for the next GPT

1. Inspect exact-SHA Process, XY-C1, storage, NVRAM, AI docs, HTTPD and dependent service runs for THIS commit; fix the first causal failure.
2. L3 requires Build, TRX verification and artifact upload on the same production-source SHA; hardware behavior is L4 only.
3. Continue switch/PHY status paths and broader userland reliability/security; keep explicitly unsupported board operations as successful no-ops.
4. Update this handoff with precise Action IDs/results at the next material commit.

## Governance

Preserve security, architecture, compatibility, target toolchain and necessary tests. Main only; frozen backup read-only. One complete Git tree and atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for each source commit. Distinguish host regression, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
