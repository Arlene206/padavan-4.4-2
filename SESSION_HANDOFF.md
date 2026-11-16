# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only authorized development ref is `main`.
- Main at the start of this IPv6 WAN-control task: `cf45552d54488f6d64cc3d2c1118ba9385a0ea7b`; refetch live main before every write and use a non-forced expected-SHA update.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup branch/backup comparison was already performed once; do not repeat it.
- Startup governance and archive recovery are complete. Root `dev_log.md` contains 118 records before this commit; this entry brings it to 119, below the 120-record rotation threshold.

## Engineering baseline

- LAN bridge/VLAN/switch required startup errors and required iNIC MII post-load verification propagate into PID 1; optional hw_nat/LED paths remain best-effort.
- Shared bridge-add and bridge-member ioctl helpers preserve ioctl errno across socket close.
- Full WAN restart fails closed on switch VLAN setup, static LAN route restoration and switch MAC-table clear errors. L2TP server startup results are merged across full restart, reconnect and deferred reconnect.
- `start_wan()` propagates static WAN route/service setup, configured IPv6 tunnel setup and physical WAN IFUP failures. `stop_wan_locked()` preserves static route teardown and USB modem stop errors through interface teardown.
- `config_vinet_wan()` now returns required IPv6 disable/enable failures for the physical VLAN parent and published WAN interface. `config_apcli_wisp()` returns IPv6 enable failure before publishing the AP-client WAN interface. These checks are conditional on IPv6 WAN mode being enabled.

## Active task

The current slice fixes three discarded `control_if_ipv6()` results in `trunk/user/rc/src/net_wan.c`: disabling IPv6 on the physical WAN VLAN parent, enabling it on the resulting WAN interface, and enabling it on an AP-client WISP WAN interface. The return status now aborts required interface setup and prevents `ifname` publication on failure. The parent Process run `43864471819` passed; parent XY-C1 `43864471994` was still compiling at last inspection. This source slice needs fresh exact-head validation.

## Verification boundary

- Prior source SHA `cf45552d54488f6d64cc3d2c1118ba9385a0ea7b`: Process inventory / WAN checker `43864471819` SUCCESS. XY-C1 `43864471994` was IN_PROGRESS at `Build XY-C1`, with TRX/artifact pending.
- That parent commit's remaining Actions were still queued/in progress; no summary of uninspected checks is claimed.
- No same-SHA firmware Build/TRX/artifact L3 or physical-router L4 result is claimed for the current IPv6-control changes.

## Exact first action for the next GPT

1. Check exact-main-SHA Process and XY-C1 Actions for the IPv6 WAN-control commit; inspect their job steps/logs, and claim L3 only after Build, TRX verification and artifact upload all pass on one SHA.
2. Continue checking PPP launch and demand-readiness failures, static IP address setup, EAPOL startup and DHCP client launch so required WAN startup failures reach PID 1 while optional cleanup stays best-effort.
3. Inspect the remaining ignored IPv6 control calls outside `config_vinet_wan()` / `config_apcli_wisp()` with their call-site purpose before deciding whether they are required.
4. Keep optional MTU tuning best-effort unless the relevant product invariant establishes otherwise. Physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
