# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only authorized development ref is `main`.
- Main at the start of the current WAN IFUP task: `f16026b0af5d00087e36929b14843cf6b94a8b6a`; all work must refetch main before the next write and use a non-forced expected-SHA update.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserved read-only. The startup branch/backup comparison was already performed once; do not repeat it.
- Startup governance and archive recovery are complete. Root `dev_log.md` has 118 complete entries before this milestone; this entry brings it to 119, below rotation thresholds.

## Engineering baseline

- LAN bridge/VLAN/switch required startup errors and required iNIC MII post-load verification propagate into PID 1; optional hw_nat/LED paths remain best-effort.
- Shared bridge-add and bridge-member ioctl helpers preserve errno across socket close.
- Full WAN restart fails closed before WAN startup on switch VLAN setup, static LAN route restoration and switch MAC-table clear errors.
- L2TP server startup results are merged across full restart, reconnect and deferred reconnect; positive already-running remains success.
- `start_wan()` propagates static WAN route/service setup and configured IPv6 tunnel setup errors. `stop_wan_locked()` preserves static route teardown and USB modem stop errors through interface teardown.
- This milestone closes one more false-success path: a failed physical WAN interface bring-up is now included in the first-error WAN startup status before best-effort down cleanup.

## Active task

The current source slice adds `rc_first_service_start_status(wan_start_status, interface_status)` inside the physical WAN IFUP failure branch in `trunk/user/rc/src/net_wan.c:start_wan()`. Previously the branch logged and attempted cleanup but continued without preserving the required IFUP failure. The policy regression in `trunk/user/rc/src/tests/modem_wan_start_policy.c` verifies that an additional cleanup error cannot replace the original IFUP failure; `scripts/check_modem_wan_start_status.py` checks the aggregation occurs before cleanup and continue. Existing Process and XY-C1 workflows already run both.

## Verification boundary

- Parent `f16026b0af5d00087e36929b14843cf6b94a8b6a`: Process `43863800540` SUCCESS; AI docs `43863800477` SUCCESS; infosvr `43863800722` SUCCESS; NVRAM `43863800497` SUCCESS; AliDDNS `43863800501` SUCCESS; AliyunDrive `43863800566` SUCCESS; AdGuardHome `43863800941` SUCCESS.
- Parent XY-C1 `43863800656` was IN_PROGRESS at last inspection. It does not validate this new product-source SHA.
- This IFUP change requires fresh exact-head Process and XY-C1 push Actions. No same-SHA Build/TRX/artifact L3 or physical-router L4 result is claimed.

## Exact first action for the next GPT

1. Inspect the exact-main-SHA Process and XY-C1 Actions triggered by this IFUP status fix. Diagnose any first causal failure; require Build, TRX verification and artifact upload on the same SHA before claiming L3.
2. Continue the bounded WAN review from `start_wan()`: propagate actual PPP launch / demand-readiness failures and static address or EAPOL startup failures to the PID 1 startup result without treating intentional best-effort cleanup or MTU tuning as fatal.
3. Inspect ignored `control_if_ipv6()` results in `config_vinet_wan()` and `config_apcli_wisp()`; required IPv6 enable/disable failures should not be silently ignored before interface state is published.
4. Keep optional MTU tuning best-effort unless a product-specific invariant establishes otherwise. No physical-router L4 validation is available.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
