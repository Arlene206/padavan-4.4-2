# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main. Parent for this WAN checker correction: 759c18d860ba1c0cb547f43fa7ca8d21b6580987; LAN route cleanup source fix: 759c18d860ba1c0cb547f43fa7ca8d21b6580987.
- Product-source conntrack flush propagation commit cffc149 is L3 VERIFIED: XY-C1 run 43973861873 completed Build XY-C1, TRX verification and artifact upload. The subsequent LAN route teardown change is awaiting its own XY-C1 run.
- Exact 759c18d Actions: Process inventory 43976717949 PASS; Process lifecycle inventory 43976718109 PASS; WAN DHCP callback 43976717974 PASS; AI handoff 43976717913 PASS. WAN route 43976718132 failed only because its inline checker still expected the pre-route_status two-status stop_lan() return; this commit changes that contract to require conntrack→route→IPv6→bridge-down status handling.
- Exact 759c18d XY-C1 run 43976717898 was IN_PROGRESS at last poll. Other unrelated workflow runs may still be queued; do not wait on them before independent work.
- Root dev_log.md rotation is complete: part-013 archives the 100 oldest records and root retains 20 before this new entry.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. Physical-router L4 remains NOT VERIFIED.

## Active task

Re-run WAN route source-contract validation after correcting its stale stop_lan() expectation. The production change on 759c18d captures/logs clear_if_route4(IFNAME_BR), aggregates conntrack/route/bridge-down statuses and preserves the existing cleanup order. Confirm exact-head WAN route and XY-C1; resolve any further source-check failures from the first causal log.

## Verification boundary

- Product-source L3 remains VERIFIED on cffc149: exact XY-C1 run 43973861873 passed firmware build, TRX verification and artifact upload.
- Exact 759c18d Process 43976717949, Process lifecycle 43976718109, WAN DHCP 43976717974 and AI handoff 43976717913 passed. WAN route 43976718132 failed only at a stale source assertion for the old stop_lan() return expression; current commit corrects it. XY-C1 43976717898 was pending.
- This checker correction does not change production source. LAN route source/test changes still need exact-SHA XY-C1 Build/TRX/artifact evidence. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main; inspect the exact-head WAN route rerun, Process and AI handoff results. Fix any genuine remaining mismatch using the first causal job log.
2. Verify XY-C1 Build, TRX and artifact upload for the LAN route cleanup source SHA; continue independent RC teardown error-propagation work while it is pending.
3. Keep main-only atomic commits, expected-SHA non-forced updates, and frozen backup immutability. Do not claim physical-router L4 without device evidence.
