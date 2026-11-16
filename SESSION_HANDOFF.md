# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main. Parent for the current handoff/log rotation: 0b93418caf9e04a41fbae0a23ebdcf234ee47ae7; last meaningful product-source commit: cffc1497fda9f65d9a0b163bad5acc1723c23f81.
- Conntrack flush status propagation is implemented in cffc149. The WAN route regression passed on 44fd after two stale checker assertions were corrected; the injected GNU89/ASan/UBSan conntrack policy test passed.
- XY-C1 run 43973861873 on exact product-source SHA cffc149 is SUCCESS. Its Cross-build XY-C1, Verify TRX output and artifact upload steps all completed successfully. This verifies L3 for product source cffc149; subsequent c3b/44fd/0b934 commits changed only workflow assertions and checkpoint/log documents, not firmware source.
- Exact 44fd Actions: WAN route 43974652676 PASS; Process 43974652670 PASS; AI handoff 43974652705 PASS; NVRAM 43974652715 PASS; AdGuardHome 43974653150 PASS; AliDDNS 43974652681 PASS; AliyunDrive 43974652664 PASS; infosvr 43974652631 PASS.
- Exact 0b934 Actions: WAN DHCP callback 43975364179 PASS; AdGuardHome 43975363862 PASS; infosvr 43975363999 PASS; AliyunDrive 43975363922 PASS. AI handoff 43975363964 failed only because root dev_log.md had 120 entries; this commit archives the oldest 100 into part-013 and leaves 20. Process 43975363957 and NVRAM 43975364308 were still running at last poll.
- Newly identified LAN teardown defect: stop_lan() ignored clear_if_route4(IFNAME_BR)'s errno-style result, so route-flush failure could be reported as success if conntrack and ifconfig succeeded. Next source slice captures this status and retains it while continuing IPv6/interface cleanup.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. Physical-router L4 remains NOT VERIFIED.

## Active task

Fix LAN router-mode teardown error loss in stop_lan(): capture clear_if_route4(IFNAME_BR), log/report failures, and aggregate conntrack, route-flush and bridge-down statuses in first-error order without skipping later cleanup. Add a regression to service_start_status.c and production source assertions to check_process_lifecycle_integration.py. Rerun exact-head Process and XY-C1 workflows.

## Verification boundary

- Product-source L3 is VERIFIED on cffc149: XY-C1 run 43973861873 succeeded, including the firmware build, TRX output verification and artifact upload.
- Exact 44fd WAN route, Process and AI handoff regressions passed. Exact 0b934 WAN DHCP callback 43975364179 passed. AI handoff 43975363964 failed at the root-log 120-entry guard; this rotation restores the required 20-record root window. Process 43975363957 and NVRAM 43975364308 were pending at last check.
- The new LAN route-flush fix/test needs exact-head Process, AI handoff and XY-C1 reruns. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect the exact-head Process/AI handoff/XY-C1 results for the LAN route-flush change. Diagnose first causal failures and confirm same-SHA firmware build/TRX/artifact.
2. Continue the RC teardown error-propagation audit, prioritizing status-returning route, firewall and interface cleanup calls that are currently discarded.
3. Keep one complete tree/commit per logical slice, use expected-SHA non-forced updates, and keep the backup branch immutable. Never claim hardware L4 without device evidence.
