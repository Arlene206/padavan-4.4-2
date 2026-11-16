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

- Process `43862972148` on `5825e35e0987fab26b4f588f879a885094e1708b`: FAILED in `scripts/check_modem_wan_start_status.py` after compiled host policy binaries passed. The first failure was two string assertions that did not allow the line break after the helper opening parenthesis; a later assertion also still expected the old `return modem_start_status;` even though `start_wan()` now returns the combined modem + route result.
- AI handoff docs `43862972023`: FAILED because `SESSION_HANDOFF.md` did not contain the required literal heading `## Current repository state`; the handoff had renamed it to `## Current state`.
- infosvr `43862972198`: packet initialization, SIGTERM and daemon policy binaries all passed; its final source-contract checker failed on the same missing `## Current repository state` heading.
- WAN route status regression `43862971906`: SUCCESS. WAN interface teardown regression `43862972252`: SUCCESS.
- AdGuardHome `43862971988`, AliyunDrive `43862972110`, AliDDNS `43862972291`, detect_internet `43862972308`, detect_link `43862972331`, DNSMasq `43862972062`, WAN gateway ARP `43862971946`, miniupnpd `43862972021`, VPN server `43862971917`, USB modem identity `43862972228`, and HTTPD security `43862972414`: SUCCESS.
- NVRAM audit `43862972028`: IN_PROGRESS at raw ioctl/kernel API audit.
- XY-C1 `43862972095`: FAILED before `Build XY-C1`; the preceding RC source-contract step failed, so Build/TRX/artifact were skipped. This is not a firmware compile failure and no L3 result is claimed.
- No physical-router L4 result is claimed.

## Exact first action for the next GPT

1. Verify Process, AI handoff and infosvr exact-head reruns after the whitespace-tolerant WAN checker and canonical handoff-heading repairs.
2. Recheck NVRAM audit `43862972028` and rerun XY-C1; the prior XY-C1 run stopped before Build because the source-contract checker failed.
3. Continue auditing ignored `set_interface_mtu()`, `ifconfig()` and `control_if_ipv6()` results in WAN startup/teardown, distinguishing required interface readiness from best-effort optional MTU configuration.
4. Claim XY-C1 L3 only when Build, TRX verification and artifact upload all pass on one exact production-source SHA; hardware testing remains L4-only.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
