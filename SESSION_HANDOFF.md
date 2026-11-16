# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for this static MAN validation / MTU diagnostics slice: `b4c6a520bf3d95822bb7757987a51bc3b37b5a7a`; use fresh-tree commits and non-forced expected-SHA updates.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery complete; do not repeat.
- Root dev log has 36 records including this slice; part-012 keeps the oldest 100 records.
- Static IPoE rejects malformed IPv4/netmask before `ifconfig()` (EINVAL); WAN/LAN overlap returns EADDRINUSE. PPP ip-up rejects missing IPCP endpoints before state publication.
- IPv6 teardown is generation-safe: stop DHCPv6 and invalidate callbacks before any route/address/SIT cleanup; saved runtime `ifname6` identifies the previous live interface until reset.

## Active task

`launch_wanx()` has a false-success path for static MAN: malformed netmask text is normalized to `NULL`, then `ifconfig(ifname, IFUP, addr, NULL)` raises the interface and sets the address but skips netmask configuration while returning success. Non-contiguous dotted masks also pass legacy `is_valid_ipv4()`. The fix uses `dhcp_lease_ipv4_is_valid()` and `dhcp_lease_netmask_is_valid()` before MTU/address setup, returns EINVAL and sets `wanx_err` on invalid configuration. Optional MAN, static IPoE, DHCP WAN/MAN and DHCP IPTV MTU ioctl failures are logged but remain nonfatal. Existing GNU89 policy coverage and the modem WAN source checker verify validation order and nonfatal diagnostics.

## Verification boundary

Exact-head validation for parent `b4c6a520bf3d95822bb7757987a51bc3b37b5a7a`:
- WAN DHCP callback `43872745445`: PASS.
- WAN route operation `43872745447`: PASS.
- AI handoff documentation `43872745205`: PASS.
- Process lifecycle inventory `43872746090`, HTTPD `43872745252`, AdGuardHome `43872745181`, AliDDNS `43872745603`, infosvr `43872745324`, AliyunDrive `43872745057`: PASS.
- Process inventory/lifecycle `43872745882`, NVRAM `43872745634`, and XY-C1 firmware `43872745244`: IN_PROGRESS at last poll. XY-C1 Build/TRX/artifact not yet verified on this exact SHA. No L3 claim.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head XY-C1, WAN DHCP callback, WAN route, and AI-doc Actions for this static MAN validation / optional MTU diagnostic slice.
2. Repair only the first causal failure. Keep optional MTU ioctl failures diagnostic/nonfatal; do not convert them into startup failure.
3. Continue auditing concrete false-success/error-propagation defects across WAN and remaining userland modules; avoid repeating the completed IPv6 source-contract audit.
4. Claim L3 only after Build, TRX verification and artifact upload pass on one identical source SHA. Physical-router L4 remains unverified.
