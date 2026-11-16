# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; only active development branch is `main`.
- Parent main for this WAN route status slice: `06bfa191302f0d53567a1cdb10652913b93cd92a`; refetch live state and use non-forced expected-SHA compare-and-swap for the next write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only. One-time startup/backup recovery is complete; do not repeat it.
- The rolling log rotation is complete; part-012 preserves the 100 oldest records and the root currently has 21 recent entries. This source record will bring the root to 22.

## Engineering baseline

- LAN bridge/VLAN/switch required startup errors and required iNIC MII post-load verification propagate into PID 1; optional hw_nat/LED paths remain best-effort.
- Shared bridge-add/member helpers preserve ioctl errno across socket close.
- WAN startup includes required physical interface IFUP failure, static WAN route/service setup, and IPv6 tunnel errors. Required IPv6 controls on the VLAN parent, WAN interface and AP-client WISP interface now return failure before publishing WAN identity. Static route teardown and USB modem stop errors are preserved.
- The current product-source SHA is `de6799e468caa60bfcd086b263e82bb149efd362`; this correction does not alter product source.

## Active task

`launch_wanx()` previously logged but discarded a nonzero `man_up_internal()` status for static MAN route setup unless the separate gateway-route output also failed. It now accumulates first-error status across static address, complete MAN route, gateway route and configured physical IPv6 setup. The WAN route workflow asserts these result paths.

## Verification boundary

On parent SHA `06bfa191302f0d53567a1cdb10652913b93cd92a`:

- AI handoff docs `43866012138`: SUCCESS; infosvr `43866012145`: SUCCESS; AdGuardHome `43866012153`: SUCCESS; AliyunDrive `43866012166`: SUCCESS; AliDDNS `43866012103`: SUCCESS.
- Process inventory/lifecycle `43866012142`: IN_PROGRESS at last inspection.
- XY-C1 firmware validation `43866012127`: IN_PROGRESS at last inspection; no Build/TRX/artifact result yet.
- NVRAM transaction audit `43866012113`: IN_PROGRESS at raw ioctl/kernel API audit.
- This parent was created to repair the IPv6 checker and log rotation; the WAN route regression is added in this source commit.
- No same-SHA XY-C1 Build + TRX + artifact L3 or physical-router L4 result is established.

## Exact first action for the next GPT

1. Inspect this correction's exact-head Process, AI handoff docs and XY-C1 Actions. Diagnose any remaining failure at its first causal step.
2. Once the source checker passes, continue the WAN startup failure-propagation slice: PPP interface migration/UPnP stop, PPTP/L2TP physical setup, PPPD launch, demand PPP readiness, static IP configuration, EAPOL startup and DHCP client launch failures must reach `start_wan()` status when required.
3. Preserve first-error semantics if best-effort cleanup also fails. Do not make optional MTU tuning or optional UPnP notification errors fatal without a verified invariant.
4. Claim firmware L3 only after Build, TRX verification and artifact upload all pass on one exact source SHA; physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
