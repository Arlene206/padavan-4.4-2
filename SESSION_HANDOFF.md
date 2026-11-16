# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active development ref: `main`.
- Parent main for this PPP ip-up IPCP address validation slice: `8ab4d6ecb4e6b39078aaf5b944c91085f9bc8668`; all writes use a fresh parent/tree and non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup and backup recovery completed once; do not repeat.
- Root dev log remains below its 120-entry archive threshold.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII errors propagate; optional hw_nat/LED remain best-effort.
- WAN startup preserves required physical IFUP, IPv6 controls, MAN static address/routes/gateway, PPP/IPoE start failures, USB RAS launch and cleanup results.
- Configured IPTV and PPPoE passthrough failures and static-IPoE WAN/LAN overlap errors reach PID 1 through first-error WAN status. Optional MTU and UPnP notification failures remain nonfatal.
- PPP ip-up now requires both non-empty IPCP IPv4 endpoint values before applying an address, publishing interface identity, or replacing the NVRAM gateway.

## Active task

Static IPoE WAN/LAN subnet overlap already prevented `wan_up()`, but the branch only set local `wan_err` and did not update the return status consumed by PID 1. It now accumulates `EADDRINUSE` using `rc_first_service_start_status()`. The WAN DHCP callback workflow checks that exact conflict branch and its caller-visible result.

## Active task

PPP `ipup_main()` previously treated missing `IPLOCAL` as optional and continued to publish callback state; if `IPREMOTE` was absent it retained a stale WAN gateway. Since pppd invokes `ip-up` when IPCP has come up and provides both local and remote IPv4 endpoints, the callback now validates both before interface/NVRAM mutation. The GNU89 policy test covers missing/empty endpoint values and verifies there are no side effects.

## Verification boundary

Exact-head Actions for parent `c38c9d288707e98616c796bb0724e64fd3885eab`:
- WAN DHCP callback `43869140586`: PASS.
- WAN route `43869140499`, Process lifecycle `43869140664`, Process inventory `43869140439`, miniupnpd `43869140553`, AI docs `43869140667`, HTTPD `43869140668`, infosvr `43869140608`, AliDDNS `43869140787`, VPN `43869140953`, DNSMasq `43869140657`, AdGuardHome `43869140393`, AliyunDrive `43869140626`, WAN teardown `43869140587`, WAN ARP `43869140602`, detect_internet `43869140477`, detect_link `43869140381`, USB identity `43869140444`: PASS.
- NVRAM `43869140525`: PASS.
- XY-C1 firmware `43869140714`: IN_PROGRESS at last poll; Build XY-C1 was active, TRX/artifact pending.
- New PPP ip-up change requires exact-head WAN DHCP and XY-C1 verification. No same-SHA firmware Build + TRX + artifact L3 or physical-router L4 result is claimed.
- Frozen backup remains unchanged and read-only.

## Exact first action for the next GPT

1. Verify exact-head WAN DHCP regression and Process/miniupnpd source checks for the static IPoE overlap correction; inspect the first causal failure if any.
2. Check XY-C1 Build, TRX verification and artifact upload on one identical source SHA, plus NVRAM audit completion.
3. Continue the WAN adjunct-service startup audit (IPTV and PPPoE passthrough), preserving WAN core availability while propagating configured-service failures to PID 1.
4. Keep optional notification, tuning and cleanup best-effort; physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Inspect exact-head WAN DHCP callback and XY-C1 Build/TRX/artifact Actions for this PPP ip-up IPCP endpoint validation; investigate any first causal failure.
2. Continue the bounded review of PPP IPv6 up/down callbacks and DHCPv6 lease/generation transitions, preserving stale-session fencing and rollback behavior.
3. Keep optional DNS refresh and notifications best-effort only where documented; preserve route and session-lock error returns.
4. Claim firmware L3 only after Build, TRX verification and artifact upload all pass on one identical SHA. Physical-router L4 remains unverified.

