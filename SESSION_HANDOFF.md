# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS WAN route status slice: `066697195b12b3bbc8bad1efa4a4c4abea5052fa`; refetch live main before the next write and use non-forced expected-SHA update.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. One-time comparison found main 588 commits ahead / 1 behind; do not repeat recovery.
- Startup governance/archive recovery is complete.

## Engineering baseline

- LAN bridge/VLAN/switch startup errors and required iNIC MII post-load verification propagate into PID1. Optional hw_nat offload and LED paths remain best-effort.
- Shared bridge-add and bridge-member ioctl helpers preserve ioctl errno across socket close.
- Full WAN restart fails closed before starting WAN on switch VLAN failure, static LAN route restoration failure, and switch MAC-table clear failure.
- L2TP server start failures are merged for full restart, reconnect and deferred reconnect; positive already-running status remains success.
- `start_wan()` now also propagates static WAN route/service setup errors and configured IPv6 tunnel setup failure. `stop_wan_locked()` records static WAN route teardown errors and preserves them through USB modem shutdown/physical interface teardown.

## Active task

Process run `43862663789` on parent `066697195b12b3bbc8bad1efa4a4c4abea5052fa` was in progress at the last check. That parent's code had updated all three WAN connect paths to return `merge_l2tp_server_start_status(status)`; the older `scripts/check_modem_wan_start_status.py` still expected raw `return status;`. This correction updates the checker to verify the shared helper's semantics and three callers.

## Verification boundary

- Process run `43863503850` on `4516b9c3f3fc8e117bf654698f515a63551b9ddc`: FAILED after the compiled RC boot and modem policy binaries passed. The checker used over-escaped raw-regex patterns (`\\s` rather than `\s`); the replacement uses single-backslash whitespace expressions and is tested against the exact source text.
- AI handoff documentation `43863503814`: SUCCESS. infosvr packet/signal regression `43863503915`: SUCCESS, including its source-contract check. The restored canonical section names resolve both earlier failures.
- AdGuardHome `43863503943`, AliDDNS `43863503892`, AliyunDrive `43863503887`: SUCCESS.
- XY-C1 firmware validation `43863503907`: IN_PROGRESS at last inspection; verify Build, TRX verification and artifact upload before any L3 claim.
- NVRAM transaction audit `43863503976`: IN_PROGRESS at last inspection.
- No same-SHA XY-C1 L3 or physical-router L4 result is claimed yet.

## Exact first action for the next GPT

1. Verify the exact-head Process rerun against the corrected single-backslash regex and check AI docs/infosvr remain green.
2. Recheck XY-C1 `43863503907` for completed Build, TRX verification and artifact upload, and NVRAM `43863503976` for completion.
3. Continue auditing `set_interface_mtu()`, `control_if_ipv6()` and interface-address setup calls for ignored return values; distinguish required interface readiness from best-effort operations.
4. Claim XY-C1 L3 only when Build, TRX verification and artifact upload all pass on one exact production-source SHA; physical-router testing remains L4-only.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
