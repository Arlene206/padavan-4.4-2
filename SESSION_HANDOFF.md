# 2026-11-18 continuation checkpoint — propagate NDIS metadata cleanup failures

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS NDIS caller fix: `0985b536e5dd2a40387fcc2069f392519d17df49`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- Commit `7f50d54c` fixed WDM stale metadata cleanup; exact USB modem, HTTPD, process inventory/lifecycle, infosvr, AI docs, AdGuardHome, AliDDNS, AliyunDrive and NVRAM runs passed on that SHA. XY-C1 `44216537251` completed Build, TRX and artifact upload successfully (L3 for 7f50 only).
- HDD tuning commit `7f10d225` aggregates `hdparm` failures while continuing subsequent options/disks, and SD hotplug reports tuning status without blocking partition checks/automount. Its shell regression passed. Storage workflow first failed at an obsolete return assertion; the first correction `695d2450` mistakenly applied the SD-only status to MMC, then `0985b536` split those assertions correctly. Exact storage run `44218995113` PASS, including the new runtime test and whole caller-graph check.
- Exact `0985b536` AI handoff `44218995112`, infosvr `44218995163`, AliyunDrive `44218995175`, AliDDNS `44218995159`, and AdGuardHome `44218995324` PASS; NVRAM `44218995246` and Process inventory `44218995232` remained in progress at last poll.
- Found the remaining NDIS metadata-status bug: `mdev_net_main()` ignored all metadata removals, returned zero after failed `modem_node_file_publish_identity()` (including stale cleanup error -2), and therefore reported success even when old USB identity could remain.
- THIS migrates NDIS to checked removal on unplug/topology/invalid-identity paths, sets a failure status on publisher/removal errors, logs whether stale metadata was removed, and returns the result after unlocking. Topology-not-ready stays benign when removal succeeds. USB/HTTPD contracts are extended; XY-C1 path filters now include USB hotplug and metadata helper dependencies.
- The 7f10 XY-C1 run `44218214468` was still running at last poll, with pre-build regression steps passed; no same-source L3 claim. No physical-router L4 evidence.

## Active task
- Verify exact-head USB modem, HTTPD and XY-C1 Actions for NDIS metadata failure/cleanup propagation.

## Verification boundary
- Exact parent 0985 storage and AI docs PASS; infosvr, AliyunDrive, AliDDNS and AdGuardHome PASS. NVRAM and Process inventory were pending.
- Prior XY-C1 L3 is proven only for 7f50 via run `44216537251`; the newer 7f10 source run remained in progress.
- THIS NDIS change's exact-head USB/HTTPD/XY-C1 Actions are pending. Physical L4 is NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head USB modem and HTTPD contracts; require NDIS, WDM and TTY checked cleanup/status assertions and metadata helper regression to PASS.
2. Inspect exact-head XY-C1 Build, TRX and artifact upload; claim L3 only when all pass on the same SHA. Check current NVRAM/Process and storage results as well.
3. Continue concrete P1 reliability fixes beyond USB/storage across the Padavan userland.
4. Keep frozen backup unchanged; commit complete trees using non-forced expected-SHA updates.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, explicitly conclude the check with no file change and no commit.

Do not create no-op documentation commits; update this file only at meaningful active-task/verification boundaries.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical multi-file change is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
