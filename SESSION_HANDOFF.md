# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main. Current parent for root-log restoration: 493f981e4db110923ca8d16328e29aec541a3734; last product-source commit: 493f981e4db110923ca8d16328e29aec541a3734.
- Product source in 493f adds checked global route-cache flushing: flush_route_caches() now returns eval() status, and full_restart_wan() merges that with interface route flushes before proceeding. The WAN route policy has injected coverage for interface failure, cache-only failure, success and missing callback.
- Exact 493f actions: WAN route 43978255126 PASS; Process inventory 43978254677 PASS; Process lifecycle 43978254742 PASS; WAN DHCP callback 43978255203 PASS; AI handoff 43978255204 failed only because a concurrent change truncated dev_log.md to one record. This commit restores all 22 preexisting post-rotation root records from parent 2c503 and retains the new 493f entry, giving 23 complete entries.
- Additional 493f PASS: VPN server 43978254778; rstats 43978255262; miniupnpd 43978255125; NFS 43978255151; AliDDNS 43978255190; ZeroTier 43978254753; iNIC 43978254862; wireless 43978254858; infosvr 43978255102; Crond 43978255000; Inadyn 43978254672; networkmap 43978254722; DDNSTO 43978254886; detect_internet 43978255129; detect_link 43978255100; FRP 43978255115; HTTPD lifecycle 43978255003.
- Exact 493f XY-C1 run 43978254941 was IN_PROGRESS at last poll; therefore route-cache source L3 is NOT YET VERIFIED. cffc149 remains previously L3 verified by XY-C1 43973861873 (Build/TRX/artifact).
- Next source defect: stop_lan() still ignores clear_lan_addr6()'s return because that wrapper is void and discards clear_if_addr6(IFNAME_BR) errno. Convert wrapper to return int, propagate through stop_lan while keeping bridge-down attempted, and add regression/source-contract tests.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. Physical-router L4 remains NOT VERIFIED.

## Active task

Fix LAN IPv6 address cleanup error loss: change clear_lan_addr6() to return clear_if_addr6(IFNAME_BR), update rc.h, capture/log ipv6_status in stop_lan(), and aggregate first error in conntrack→IPv4 routes→IPv6 address→bridge-down order without skipping later cleanup. Add injected service_start_status coverage and source checks in Process/WAN route CI; include net_lan6.c in XY-C1 path filters. Then inspect exact-SHA Process, WAN route, AI handoff and XY-C1 Actions.

## Verification boundary

- Product-source L3 for the earlier conntrack change is VERIFIED at cffc149 by XY-C1 run 43973861873 (Build, TRX verification and artifact upload).
- 493f WAN route 43978255126, Process inventory 43978254677, Process lifecycle 43978254742 and WAN DHCP 43978255203 passed. AI handoff 43978255204 failed because root dev_log.md had been reduced to one record by that commit; this repair restores 23 complete records. Exact rerun is required.
- 493f XY-C1 43978254941 was still in progress at last poll. No current-source route-cache L3 claim until same-SHA Build/TRX/artifact passes. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Confirm the AI handoff rerun for the restored 23-entry root log and inspect exact 493f XY-C1 Build/TRX/artifact status.
2. Implement clear_lan_addr6() errno propagation through stop_lan(), including injected first-error and cleanup-continuation tests, while XY is pending.
3. Continue using main-only complete-tree commits and expected-SHA non-forced updates; preserve frozen backup and do not claim L4 without device evidence.
