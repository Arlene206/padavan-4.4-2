# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for current WAN result-contract correction: `e97f95dc097e7faa8cd00aa491de9f1fb4ad519a`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. The one-time comparison found main 588 commits ahead / 1 behind; do not repeat branch/archive recovery.
- Startup governance and backup/archive recovery are complete.

## Current engineering baseline

- LAN bridge startup returns and aggregates essential switch VLAN, switch base/storm/link, interface/member, and required iNIC MII module-load failures into PID1. Optional hw_nat offload and LED reporting remain best-effort.
- Switch VLAN rule writes and bridge-mode ioctl results are preserved. Duplicate bridge creation is accepted only after kernel bridge identity confirmation; bridge membership is idempotent only after inventory confirms the requested state.
- `br_add_del_bridge()` and `br_add_del_if_internal()` preserve the original bridge ioctl errno across socket close.
- `full_restart_wan()` fails closed before WAN start if switch VLAN reconfiguration, static LAN route restoration, or switch MAC table clearing fails.
- WAN L2TP server start results are merged for full restart, normal reconnect and deferred reconnect. Positive `safe_start_xl2tpd()` means already running and is success; negative startup errors are returned when WAN start succeeded.

## Active checker repair

Process exact-SHA run `43862009119` on parent `e97f95dc097e7faa8cd00aa491de9f1fb4ad519a`, job `133280916393`, passed the compiled process/policy regression steps including RC startup aggregation and modem WAN start policy binaries, then failed `scripts/check_modem_wan_start_status.py` at the old assertion `return status;`. The script predates L2TP status propagation and still expected each WAN reconnect wrapper to return raw WAN status.

THIS correction updates that source contract to verify `merge_l2tp_server_start_status()` semantics and ensure all three callers return the merged status. The source code was not changed in this correction.

## Verification boundary

- Exact-parent Process run `43862009119`: FAIL only in the stale modem WAN checker assertion described above; compiled unit/policy test steps had passed before the checker.
- Other exact-head Actions for parent `e97f95dc097e7faa8cd00aa491de9f1fb4ad519a` have not yet been reconciled in this handoff. Query exact SHA before claiming any pass/fail.
- No same-source-SHA XY-C1 Build+TRX+artifact L3 or physical-router L4 result is established for this branch head.

## Exact first action for the next GPT

1. Check exact-head Process and AI docs plus XY-C1/NVRAM/security/service Actions for this checker correction.
2. Confirm modem WAN source contract now accepts the L2TP status wrapper and still covers modem migration, RAS physical setup and PID1/runtime aggregation.
3. Continue auditing int-returning operations in `start_wan()`, `stop_wan_locked()`, `wan_up()`, `wan_down()` and DHCP callbacks. Distinguish required forwarding/session operations from optional diagnostics and cleanup.
4. XY-C1 L3 requires build, TRX verification and artifact upload on one identical production-source SHA; hardware testing remains L4-only.

## Governance

Main only; frozen backup read-only. Every logical change: one complete tree and atomic commit, then non-forced update with expected SHA. Update `dev_log.md` for source milestones and keep 20–119 newest records. Keep exact CI, host regression, firmware/TRX and physical-router evidence separate. Never claim an unrun result.
