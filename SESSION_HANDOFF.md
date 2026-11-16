# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active development ref: `main`.
- Parent main for this PPP ip-up regression follow-up: `bacfb1d9d9c814fdc6d122de5e8e34cf85dcb900`; all writes use a fresh parent/tree and non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup and backup recovery completed once; do not repeat.
- Root dev log remains below its 120-entry archive threshold.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII errors propagate; optional hw_nat/LED remain best-effort.
- WAN startup preserves required physical IFUP, IPv6 controls, MAN static address/routes/gateway, PPP/IPoE start failures, USB RAS launch and cleanup results.
- Configured IPTV and PPPoE passthrough failures and static-IPoE WAN/LAN overlap errors reach PID 1 through first-error WAN status. Optional MTU and UPnP notification failures remain nonfatal.
- PPP ip-up now requires both non-empty IPCP IPv4 endpoint values before applying an address, publishing interface identity, or replacing the NVRAM gateway.

## Active task

PPP `ipup_main()` now requires both negotiated IPCP IPv4 endpoints before touching interface or NVRAM state. Missing `IPLOCAL` previously skipped address application but still published interface identity; missing `IPREMOTE` could retain a stale gateway. The callback policy and source workflow now enforce fail-closed behavior.

## Verification boundary

Exact-head Actions for parent `bacfb1d9d9c814fdc6d122de5e8e34cf85dcb900`:
- WAN DHCP callback `43870165716`: FAIL during GNU89 compile of `ppp_wan_ipup_policy.c`; `test_interface_failure_preserves_published_state()` still used the old four-argument policy call after the policy required local, remote, netmask, ops and context. The call is corrected in the current follow-up.
- PASS: AI docs `43870165549`, AdGuardHome `43870165477`, AliyunDrive `43870164931`, Process final inventory `43870164946`, AliDDNS `43870164965`, WAN route `43870164920`, infosvr `43870164981`, VPN `43870164985`, and HTTPD `43870164960`.
- NVRAM `43870165218`, Process lifecycle `43870164923`, and XY-C1 firmware `43870165029` were still in progress at last poll.
- No same-SHA firmware Build + TRX + artifact L3 or physical-router L4 is claimed. Frozen backup remains read-only.

## Exact first action for the next GPT

1. Inspect the exact-head WAN DHCP callback run after the regression call-signature fix; verify that compilation, policy execution and the source-contract step all pass.
2. Inspect XY-C1 Build, TRX verification and artifact upload on one identical SHA; NVRAM audit on the parent was still in progress.
3. Continue the bounded PPP IPv6 up/down and DHCPv6 lease/generation audit, preserving stale-session fencing and rollback semantics.
4. Claim firmware L3 only after same-SHA Build, TRX and artifact upload pass. Physical-router L4 remains unverified.

