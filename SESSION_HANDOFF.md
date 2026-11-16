# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent for THIS NDIS metadata cleanup fix: `d6738c390b0578ba07a7bd9f0c2c23a4a1bca764`. Re-fetch live `main` before each later write and use expected-SHA non-forced CAS.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable/read-only.
- Startup protocol, archive, backup and PR recovery are complete; do not repeat that audit.
- `dev_log.md` contains 91 newest-first records before THIS entry; this entry makes 92. Keep 20–119 and preserve archived shards/indexes.

## Active task

Fix stale network-node metadata after failed NDIS identity publication:
- `mdev_net_main()` previously left an older `wwanX/wethX` metadata file untouched when `snprintf()` validation or `modem_node_file_write()` failed.
- The metadata writer uses atomic replacement, intentionally retaining the previous target on a failed stage/write. Linux may reuse a `wwanX/wethX` name for a different physical USB device, so retaining the old `portid/devnum` could cause stale identity association.
- This change adds a tested publication helper: write valid complete metadata; on invalid content or write failure, remove the old identity entry; return a distinct cleanup-failure status for logging.
- `mdev_net_main()` uses the helper and distinguishes normal cleanup from cleanup failure. Hotplug remains locked while publishing/removing metadata.
- GNU89/Werror/ASan/UBSan regression injects write/cleanup outcomes; workflow source checks tie the policy to the production hotplug path.
- Previous exact-head RAS manual-index correction at `d6738c390b0578ba07a7bd9f0c2c23a4a1bca764` also makes explicit manual `modem_node` selection fail closed instead of falling back to another tty.

## Verification boundary

Exact-head observations on parent `d6738c390b0578ba07a7bd9f0c2c23a4a1bca764`:
- USB modem WDM/NDIS physical identity run `43846822594`: SUCCESS (includes RAS manual-index policy/source checks).
- Process lifecycle `43846822569` and `43846822993`: SUCCESS.
- HTTPD Security `43846822596`: SUCCESS.
- AI handoff docs `43846822710`: SUCCESS.
- WAN interface teardown `43846822592`: SUCCESS.
- infosvr `43846822597`, AdGuardHome `43846822892`, AliDDNS `43846822840`, AliyunDrive `43846822598`: SUCCESS.
- NVRAM transaction boundary `43846822673` and XY-C1 firmware `43846822718`: IN_PROGRESS at last inspection. No Build/TRX/artifact L3 or physical-router/L4 result is claimed.
- No physical-router/L4 result is established.

## Exact first action for the next GPT

1. Fetch live `main` and inspect this commit's exact-head USB modem identity Action, then inspect XY-C1/NVRAM status. Read any failures; only claim L3 after Build, TRX verification and artifact upload all pass.
2. Continue the device-aware `reload_modem_modules()` readiness review. `module_smart_load()` returns zero for both already-loaded and failed loads; verify actual `/sys/module` state where needed. Keep alternative drivers optional and verify selected-device readiness only after USB host initialization.
3. Broaden RC/service-start result propagation outside the modem subsystem. Identify enabled launch paths that discard failures and add caller-level regressions.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; the frozen backup is immutable. Update root `dev_log.md` for each source/documentation commit, keep 20–119 records and preserve archive shards/indexes verbatim. Distinguish source, CI, firmware and physical-router validation. Never claim an unrun test/build/hardware result.
