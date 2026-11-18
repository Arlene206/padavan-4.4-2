# 2026-11-18 continuation checkpoint — align HTTPD USB metadata regression with TTY cleanup migration

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS test-contract correction: `7aca8abf312ed6ab029a8757a936a6a7f0793d07`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; untouched.
- Exact parent `7aca8abf` USB modem workflow `44215156701` PASS; AI handoff `44215156759` PASS; infosvr `44215156778` PASS; AdGuardHome `44215156718` PASS; AliyunDrive `44215156606` PASS; AliDDNS `44215156607` PASS; Process lifecycle `44215156605` PASS; Process inventory `44215156806` PASS; NVRAM audit `44215156739` PASS.
- Exact parent HTTPD Security run `44215156710` FAILed in “Verify USB modem node metadata staging” after its existing `modem_node_file.c` regression passed. The failure was a stale source contract: it still expected two direct `modem_node_file_write()` call sites and required the TTY handler to use that direct writer. The TTY source was intentionally migrated in `7aca8abf` to `modem_node_file_publish_identity()`, which removes stale metadata after failed replacement and returns -1/-2 according to cleanup outcome. The USB-specific workflow already verifies this newer caller; HTTPD's cross-cutting duplicate contract had not been updated.
- THIS correction updates only the HTTPD regression to expect one direct writer (the remaining WDM path), and requires TTY to use the cleanup-aware publisher and propagate `metadata_status`. No production source behavior changes.
- Firmware L3 / physical-router L4 NOT VERIFIED; no exact-parent XY-C1 run was present.

## Active task
- Restore the HTTPD security regression's USB metadata source assertions to match the current TTY cleanup-aware publication API; rerun HTTPD and USB exact-head workflows.

## Verification boundary
- Exact parent USB, AI, infosvr, AdGuardHome, AliyunDrive, AliDDNS, Process lifecycle, Process inventory and NVRAM Actions PASS.
- Exact parent HTTPD Action FAILed only at obsolete USB metadata caller assertions; preceding 19 HTTPD security/configuration checks and the metadata file regression passed.
- THIS test-only correction's exact-head Actions pending. Firmware L3 and hardware L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head HTTPD and USB modem workflows; both must PASS, confirming direct-writer count and TTY cleanup source contract are fixed.
2. Recheck NVRAM and all exact-head security/lifecycle Actions; check for same-SHA XY-C1 Build/TRX/artifact and only claim L3 if all pass.
3. Continue the sibling WDM stale-metadata issue: `mdev_wdm_main()` still uses direct write/remove calls and reports success after failure; migrate it to cleanup-aware publication and status propagation, updating both source-contract workflows.
4. Keep frozen backup unchanged; use one complete tree and non-forced expected-SHA update per logical change.


## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
