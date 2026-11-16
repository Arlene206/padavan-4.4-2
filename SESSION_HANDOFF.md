# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main. Parent for the LAN IPv6 address cleanup slice: 8befa2b20295be8182053dc4e224174db471ed82; latest product-source baseline before this slice: 493f981e4db110923ca8d16328e29aec541a3734.
- Product source cffc149 passed exact XY-C1 Build, TRX verification and artifact upload (run 43973861873). LAN IPv4 route cleanup fix 759c18d also passed exact XY-C1 run 43976717898; route-cache source change 493f was validated by WAN route run 43978255126, but its XY-C1 run 43978254941 FAILED. Do not claim route-cache L3 from this result.
- Commit bb0c693 restored the 22 complete post-rotation log records from 2c503 and retained the 493f entry, bringing root dev_log.md to 23. Exact bb0 AI handoff 43978638161 PASS. It also exposed a Process failure in scripts/check_modem_wan_start_status.py:334 because an assertion expected the pre-route-cache call rc_wan_route_flush_apply; exact bb0 Process run 43978638091 failed after all C policy tests passed.
- Concurrent commit 8befa2b updated the shared modem/WAN checker to assert the current combined route+cache helper. Exact 8bef AI handoff 43979147640 PASS; Firewall Startup 43979147580 PASS; USB modem 43979148326 PASS; AdGuardHome 43979147629 PASS; infosvr 43979147611 PASS; AliDDNS 43979147585 PASS. Process 43979148973 and XY-C1 43979147638 were IN_PROGRESS at last poll; NVRAM 43979147499, HTTPD 43979147572 and AliyunDrive 43979147568 were also pending.
- This slice fixes another LAN teardown status-loss path: clear_lan_addr6() currently returns void and discards clear_if_addr6(IFNAME_BR)'s status. The change makes the wrapper return int, updates rc.h, and aggregates IPv6 cleanup between IPv4 route flush and bridge-down while continuing all cleanup.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. Physical-router L4 remains NOT VERIFIED.

## Active task

Validate LAN IPv6 address cleanup error propagation. clear_lan_addr6() returns clear_if_addr6(IFNAME_BR); stop_lan() captures/logs it and retains the first failure in conntrack → IPv4 route → IPv6 address → bridge-down order while still attempting later cleanup. The regression covers IPv6-only failure, earlier-route precedence and bridge-down-only failure. Inspect exact-SHA Process, WAN route, AI handoff and XY-C1 Actions.

## Verification boundary

- Exact cffc149 XY-C1 run 43973861873 passed firmware build, TRX verification and artifact upload (L3 VERIFIED for that product-source SHA). Exact 759c18d XY-C1 run 43976717898 also completed successfully for its LAN IPv4 route cleanup source.
- Exact 493f WAN route 43978255126 PASS. Exact 493f XY-C1 run 43978254941 FAILED; no L3 claim for the newer route-cache source. Exact 8bef AI handoff 43979147640 PASS; Firewall Startup 43979147580, USB modem 43979148326, AdGuardHome 43979147629, infosvr 43979147611 and AliDDNS 43979147585 PASS. Process 43979148973 and XY-C1 43979147638 were pending; NVRAM, HTTPD and AliyunDrive were pending at last poll.
- The current LAN IPv6 source change adds net_lan6.c to the XY-C1 path filter and requires exact-SHA Process/WAN/AI/XY verification. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process, WAN route, AI handoff and XY-C1 runs for the LAN IPv6 wrapper/status change. Resolve first causal failures and verify target Build/TRX/artifact on one SHA.
2. Continue auditing cleanup wrappers, starting with reload_lan_addr6() which also discards clear_if_addr6() and the IPv6 address-add status.
3. Keep commits atomic with a full Git tree and expected-SHA non-forced main update. Preserve frozen backup and do not claim physical L4 without device evidence.
