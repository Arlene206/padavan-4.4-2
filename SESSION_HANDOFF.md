# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS WAN restart/return-status slice: `01d9147b742391933a22781b42eb1213063cb1c1`; fetch live main before the next write and use expected-SHA non-forced CAS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind; do not repeat archive recovery.

## Implemented source baseline

- Essential bridge/VLAN/switch startup status, required iNIC module verification, optional hw_nat offload verification and PID1 aggregation are in main.
- `full_restart_wan()` fails closed on static LAN route restoration and switch VLAN configuration failures.
- Shared bridge-create and bridge-member ioctl helpers preserve original errno across socket close.

## Active task

A further `full_restart_wan()` audit found two ignored int-returning operations:
- `phy_clear_mac_table()` returns an ioctl status on both MTK ESW and RTL8367 drivers. The full restart now fails before WAN start if the forwarding table cannot be cleared, avoiding WAN reactivation with possibly stale switch entries.
- `safe_start_xl2tpd()` returns negative on lock/process/config/start failure, zero on successful launch, and positive one when the shared daemon is already running. The previous WAN restart, regular reconnect and deferred reconnect ignored that status. New `merge_l2tp_server_start_status()` preserves an earlier WAN start error, treats positive "already running" as success, and returns L2TP startup failure when WAN startup itself succeeded. All three callers use the helper.
- Existing full restart fails closed before WAN start on static LAN routes, switch VLAN and board-specific bridge teardown/reconfiguration failures.

## Verification boundary

- Exact-head Process run `43860996821` and AI docs run `43860997587` on `43d0ebe973f06f578282531eed4abc0fdba1bb3b` were in progress at last check; check for completion.
- On parent `01d9147b742391933a22781b42eb1213063cb1c1`, Process `43861241910`, AI docs `43861242056`, NVRAM `43861243064` and XY-C1 `43861242089` were queued or in progress. No same-SHA firmware Build+TRX+artifact L3 or physical L4 was established.
- THIS slice requires fresh exact-head Process and WAN route Actions.

## Exact first action for the next GPT

1. Inspect exact-head Process, WAN route, AI docs, NVRAM and XY-C1 results for this commit, and diagnose any first causal failure.
2. Verify MAC table clear and L2TP service status contracts cover all three WAN connect call paths.
3. Continue source-backed callsite audit of int-returning operations in WAN and RC startup; preserve special result semantics rather than treating every positive return as failure.
4. XY-C1 L3 requires Build, TRX verification and artifact upload on the same exact production-source SHA; physical hardware remains L4-only.

## Verification and governance

Preserve security, architecture, compatibility, target toolchain and required tests. Main only; frozen backup read-only. One full Git tree and atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for source milestones; preserve archive shards. Distinguish host regression, exact-SHA Actions, firmware/TRX L3 and physical router L4. Never claim an unrun result.
