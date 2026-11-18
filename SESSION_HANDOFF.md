# 2026-11-18 continuation checkpoint — migrate WDM metadata to cleanup-aware publication

## Current repository state
- Repository: Arlene206/padavan-4.4-2; parent main before THIS WDM metadata fix: c47a1b1fdfbd1d4061df4ef9294665fd21d3fc63.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; unchanged.
- Exact parent c47a1b1f HTTPD Security run 44216025750 PASS, confirming the revised TTY cleanup contract; AI handoff 44216026808 PASS, AdGuardHome 44216025560 PASS, infosvr 44216025661 PASS, AliDDNS 44216025635 PASS and AliyunDrive 44216025544 PASS.
- Exact parent NVRAM audit 44216025596, Process inventory 44216025643 and XY-C1 firmware validation 44216025599 remained IN_PROGRESS at last poll.
- Found the sibling stale-record bug in mdev_wdm_main(): it used direct modem_node_file_write(), ignored all cleanup remover results, and returned success when new metadata publication failed. A previous cdc-wdm identity could therefore remain associated with a reused node name. THIS fix migrates WDM to modem_node_file_publish_identity(), captures the -1/-2 cleanup outcome, logs and returns failure, and checks unplug/topology-removal errors. Successful cleanup on topology-not-ready remains a benign no-op.
- USB modem and HTTPD workflows are extended to pin WDM's cleanup-aware publication/status contract, and HTTPD now expects zero direct metadata writers across WDM/NDIS/TTY.
- No same-SHA XY-C1 Build/TRX/artifact evidence or physical-router L4; firmware L3 and hardware L4 NOT VERIFIED.

## Active task
- Ensure failed WDM metadata updates cannot preserve stale USB identity and are not reported as successful hotplug events.

## Verification boundary
- Exact parent HTTPD, AI, AdGuardHome, infosvr, AliDDNS and AliyunDrive runs PASS.
- NVRAM, Process inventory and XY-C1 firmware runs were IN_PROGRESS at last parent poll.
- THIS WDM source slice's exact-head USB/HTTPD Actions pending. Firmware L3 and physical L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head USB modem and HTTPD regressions; require both WDM cleanup contracts, TTY/NDIS metadata assertions, SG/ZeroCD/CD tests and existing policy binaries to PASS.
2. Check exact-head XY-C1 Build, TRX, artifact upload; inspect NVRAM and Process inventory; claim firmware L3 only if all three build stages pass on that SHA.
3. Continue an independent P1 storage/userland security/reliability fix while pending Actions run.
4. Keep frozen backup unchanged and publish each complete multi-file slice by one tree/commit with non-forced expected-SHA update.


## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
