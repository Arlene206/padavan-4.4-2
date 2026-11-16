# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for this WAN-route contract repair: `3375e565983097af355395bffd32a41dcf442b18`; all writes are complete-tree atomic commits followed by non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery complete; do not repeat.
- Root dev log has 37 records before this entry; part-012 retains the oldest 100.
- Static MAN and static IPoE reject malformed IPv4/netmask before `ifconfig()`; WAN/LAN overlap returns EADDRINUSE. PPP ip-up requires and validates both IPCP endpoints before configuring the interface or publishing lease state.
- IPv6 teardown preserves the previous live `ifname6` through RA/autoconf, route and address cleanup; DHCPv6 callback generation fencing is retained.

## Active task

The exact-head WAN route workflow on `c5dfa15e8c0c4d3998ca01d2ee961f0897282855` failed only in its final source-contract step. The failing assertion still expected `udhcpc_bound()` to declare only `interface_status, rollback_status, route_setup_status`; static MTU result capture had correctly extended it with `mtu_status`. The workflow now checks the updated declaration and verifies MTU ioctl failures are logged but do not set lease/route failure state or return early. Product code and the DHCP WAN callback test already passed their own exact-head checks.

## Verification boundary

- Current main before this contract repair: `3375e565983097af355395bffd32a41dcf442b18`, parent of this commit. PPP strict IPCP validation is present.
- On product/workflow SHA `c5dfa15e8c0c4d3998ca01d2ee961f0897282855`: WAN DHCP `43873631601` PASS; WAN interface teardown `43873631728` PASS; AI docs `43873631558` PASS; WAN route `43873631807` FAIL only in final source-contract step due the stale declaration assertion; all six preceding route regressions passed.
- XY-C1 `43873631679`: Build XY-C1 still in progress at last poll; TRX/artifact pending. No same-SHA L3 claim.
- On latest main `3375e565983097af355395bffd32a41dcf442b18`: WAN DHCP `43873889736` PASS; AI docs `43873889717` PASS; Process inventory/lifecycle `43873889642` and NVRAM `43873889718` IN_PROGRESS. WAN-route contract repair has not yet run. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check the exact-head WAN route workflow triggered by this source-contract repair; repair any genuine first causal failure.
2. Check exact-head NVRAM, Process inventory and XY-C1 statuses; continue independent tasks while any Actions remain pending.
3. Continue PPP IPv4/IPv6 callback review only for concrete error-propagation or stale-state defects; preserve callback lock/generation fencing.
4. Claim L3 only after Build, TRX verification and artifact upload pass on one identical source SHA. Physical-router L4 remains unverified.
