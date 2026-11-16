# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current state

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

## Active test-contract repair

Process run `43862663789` on parent `066697195b12b3bbc8bad1efa4a4c4abea5052fa` was in progress at the last check. That parent's code had updated all three WAN connect paths to return `merge_l2tp_server_start_status(status)`; the older `scripts/check_modem_wan_start_status.py` still expected raw `return status;`. This correction updates the checker to verify the shared helper's semantics and three callers.

## Verification boundary

- Exact-SHA Process and AI-doc runs for `066697195b12b3bbc8bad1efa4a4c4abea5052fa` were queued/in progress at last query: Process `43862663789`, AI docs `43862663478`, XY-C1 `43862663471`, NVRAM `43862663293`, AdGuardHome `43862663396`, infosvr `43862663310`, AliyunDrive `43862663359`, AliDDNS `43862663392`.
- On that parent, Process reached the modem WAN source checker after preceding compiled policy tests; its old expectation of raw `return status;` no longer matched L2TP merged semantics. THIS correction requires a fresh exact-head run.
- No same-source-SHA XY-C1 Build+TRX+artifact L3 or physical-router L4 result established.

## Exact first action for the next GPT

1. Check exact-head Process, AI docs, XY-C1, NVRAM and service/security Actions for THIS commit; inspect failures.
2. Verify WAN route start and teardown results are preserved, including stop-status combination with USB modem and ifdown operations.
3. Continue auditing ignored return values in `start_wan()`, `stop_wan_locked()`, DHCP callbacks and any MTU/address setup. Keep best-effort cleanup distinct from required forwarding/session setup.
4. Claim XY-C1 L3 only with Build, TRX verification and artifact upload on the same exact production-source SHA; hardware testing remains L4-only.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
