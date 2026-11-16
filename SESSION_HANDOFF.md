# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for this PPP IPCP validation slice: `c5dfa15e8c0c4d3998ca01d2ee961f0897282855`; all writes are complete-tree atomic commits followed by non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery complete; do not repeat.
- Root dev log has 36 records before this entry; part-012 retains the oldest 100.
- Static MAN and static IPoE reject malformed IPv4/netmask before `ifconfig()`; WAN/LAN overlap returns EADDRINUSE. PPP ip-up requires and validates both IPCP endpoints before configuring the interface or publishing lease state.
- IPv6 teardown preserves the previous live `ifname6` through RA/autoconf, route and address cleanup; DHCPv6 callback generation fencing is retained.

## Active task

The shared `ifconfig()` helper uses `inet_addr_safe()`, which returns `INADDR_ANY` for malformed address text. PPP ip-up previously rejected only missing/empty `IPLOCAL` and `IPREMOTE`; malformed non-empty strings such as `not-an-ip`, non-canonical dotted quads, or `0.0.0.0` could still be passed to ifconfig and published as a successful WAN lease. The PPP ip-up policy now reuses the existing strict `dhcp_lease_ipv4_is_valid()` validator for local and peer addresses plus `dhcp_lease_netmask_is_valid()` for the mask. Regression tests require EINVAL and no interface/NVRAM side effects for malformed values.

## Verification boundary

Parent `c5dfa15e8c0c4d3998ca01d2ee961f0897282855` was the latest verified source baseline when this slice started:
- WAN DHCP callback `43872745445`: PASS; WAN route `43872745447`: PASS; AI docs `43872745205`: PASS.
- Process lifecycle `43872746090`, HTTPD `43872745252`, AdGuardHome `43872745181`, AliDDNS `43872745603`, infosvr `43872745324`, AliyunDrive `43872745057`, NVRAM `43872745634`: PASS.
- Process inventory/lifecycle `43872745882` and XY-C1 firmware `43872745244` were still running at last inspection; XY-C1 Build/TRX/artifact not yet verified on that same SHA.
- This PPP strict-address validation commit has not yet had exact-head Actions. No same-SHA firmware L3 or hardware L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head WAN DHCP callback regression and source contract for strict PPP local/peer IPv4 validation.
2. Inspect XY-C1 Build, TRX verification and artifact upload on that identical SHA; repair any first causal failure.
3. Continue bounded WAN/PPP callback review only for concrete stability/security defects; preserve session generation and lock fencing.
4. Claim L3 only after same-SHA Build, TRX and artifact upload pass. Physical-router L4 remains unverified.
