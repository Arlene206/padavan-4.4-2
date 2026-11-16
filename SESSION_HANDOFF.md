# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active development ref: `main`.
- Parent main for the static IPoE overlap correction: `c38c9d288707e98616c796bb0724e64fd3885eab`; subsequent writes must re-fetch main/tree and use non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable/read-only. Startup and backup recovery was completed once; do not repeat it.
- The rolling dev-log archive remains in part-012; root log currently has 29 records.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII errors propagate; optional hw_nat/LED remain best-effort.
- WAN startup now preserves required physical IFUP, IPv6 controls, MAN static address/routes/gateway, PPP/IPoE start failures, USB RAS launch and cleanup results.
- This slice adds first-error status return for configured IPTV setup and PPPoE pass-through/relay setup, without preventing the main WAN protocol and other adjunct services from starting. Optional MTU and UPnP notification failures remain nonfatal.

## Active task

Static IPoE WAN/LAN subnet overlap already prevented `wan_up()`, but the branch only set local `wan_err` and did not update the return status consumed by PID 1. It now accumulates `EADDRINUSE` using `rc_first_service_start_status()`. The WAN DHCP callback workflow checks that exact conflict branch and its caller-visible result.

## Verification boundary

Exact-head Actions for parent `c38c9d288707e98616c796bb0724e64fd3885eab`:

- `VPN server lifecycle stability regression` `43869140953`: completed / success
- `AliDDNS signed API transport security regression` `43869140787`: completed / success
- `AI handoff documentation contract` `43869140667`: completed / success
- `WAN route operation status regression` `43869140499`: completed / success
- `WAN interface teardown stability regression` `43869140587`: completed / success
- `detect_internet signal lifecycle stability regression` `43869140477`: completed / success
- `infosvr packet and signal lifecycle stability regression` `43869140608`: completed / success
- `WAN gateway ARP probe socket stability regression` `43869140602`: completed / success
- `XY-C1 Firmware Validation` `43869140714`: in_progress
- `WAN DHCP callback state stability regression` `43869140586`: completed / success
- `USB modem WDM/NDIS physical identity stability` `43869140444`: completed / success
- `AliyunDrive WebDAV lifecycle stability regression` `43869140626`: completed / success
- `detect_link signal lifecycle stability regression` `43869140381`: completed / success
- `miniupnpd atomic configuration publication regression` `43869140553`: completed / success
- `Process lifecycle inventory - c38c9d288707e98616c796bb0724e64fd3885eab` `43869140439`: completed / success
- `Process inventory and lifecycle fail-closed regression` `43869140664`: completed / success
- `AdGuardHome config security regression` `43869140393`: completed / success
- `DNSMasq lifecycle stability regression` `43869140657`: completed / success
- `HTTPD Authentication and Security Regression` `43869140668`: completed / success
- `NVRAM transaction boundary audit` `43869140525`: completed / success

- This source correction has not yet been verified on its new commit SHA.
- No same-SHA XY-C1 Build + TRX + artifact L3 or physical-router L4 result is claimed without all required successful evidence.

## Exact first action for the next GPT

1. Verify exact-head WAN DHCP regression and Process/miniupnpd source checks for the static IPoE overlap correction; inspect the first causal failure if any.
2. Check XY-C1 Build, TRX verification and artifact upload on one identical source SHA, plus NVRAM audit completion.
3. Continue the WAN adjunct-service startup audit (IPTV and PPPoE passthrough), preserving WAN core availability while propagating configured-service failures to PID 1.
4. Keep optional notification, tuning and cleanup best-effort; physical-router L4 remains unverified.

