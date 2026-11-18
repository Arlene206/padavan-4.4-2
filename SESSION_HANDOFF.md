# 2026-11-18 continuation checkpoint — fail closed on TTY metadata publication

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS TTY metadata publication fix: `9ff3bc667ea0507f311b417f16ed3b324cf19ae6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- Exact parent storage hotplug workflow `44214586174` PASS and process lifecycle inventory `44214586292` PASS. XY-C1 firmware validation `44214586399`, NVRAM audit `44214586157`, and several other exact-parent workflows were IN_PROGRESS at last poll.
- The NDIS metadata newline defect was corrected in `e52d186a`; the exact e52 USB modem workflow `44214240090` PASS. SG/ZeroCD status propagation is in `b103a8e4`, and the SG policy include was fixed/verified in `2e01dae1`.
- Found a stale-identity risk in `mdev_tty_main()`: when `modem_node_file_write()` fails, the handler leaves the previous TTY metadata file in place, then returns success. The line-reader can later accept that stale `portid/devnum` record if a node name and USB identity are reused. Unlike NDIS, the TTY path did not use `modem_node_file_publish_identity()`, which removes the old record after a failed replacement and distinguishes cleanup failure (-2).
- THIS fix uses the cleanup-aware identity publisher for TTY metadata, returns a nonzero status on publication or stale-file removal failure, and keeps the existing benign behavior when topology is unavailable and stale metadata is successfully removed. The existing `modem_node_file.c` regression exercises failed-publication cleanup; the USB workflow now pins the TTY caller contract.
- No same-SHA XY-C1 Build/TRX/artifact evidence yet; firmware L3 and physical-router L4 NOT VERIFIED.

## Active task
- Prevent failed TTY metadata replacement from preserving stale USB identity and being reported as success; make cleanup status observable.

## Verification boundary
- Exact parent storage hotplug and process lifecycle runs PASS.
- XY-C1 firmware, NVRAM audit and several other Actions were IN_PROGRESS at last poll.
- THIS TTY source-contract slice's exact-head USB workflow is pending. Firmware L3 and physical L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head USB modem workflow; require TTY cleanup caller assertions, existing metadata cleanup tests, NDIS/WDM identity tests and SG/ZeroCD/CD policy tests to PASS.
2. Inspect the exact-parent XY-C1 Build/TRX/artifact steps, NVRAM audit and remaining lifecycle Actions; L3 only if Build, TRX and upload pass on one SHA.
3. Continue an independent P1 userland reliability/security defect while pending Actions run.
4. Keep the frozen backup unchanged and use one full tree/commit with expected-SHA update for each logical change.


## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
