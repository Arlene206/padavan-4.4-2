# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent for THIS HTTPD metadata-contract correction: `b83383f9798d5a3e43d86877f9f37c0b1594ffd0`. Re-fetch live `main` before each later write and use expected-SHA non-forced CAS.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable/read-only.
- Startup protocol/archive/backup/PR recovery are complete; do not repeat the initial audit.
- `dev_log.md` contains 92 newest-first records before THIS entry; this entry makes 93. Keep 20–119 and preserve archived shards/indexes.

## Active task

Synchronize the HTTPD umbrella's modem metadata test with the new fail-closed publication helper:
- Source fix `b83383f9798d5a3e43d86877f9f37c0b1594ffd0` adds `modem_node_file_publish_identity()` and changes `mdev_net_main()` to remove stale `wwanX/wethX` metadata when new identity content is invalid or atomic write fails.
- Exact-head USB identity workflow `43847475018` PASSED, including GNU89/Werror/ASan/UBSan and production-path checker; handoff validation `43847474937` PASSED.
- HTTPD umbrella `43847474960` failed only because its legacy nested metadata source checker still required three direct `modem_node_file_write()` calls and assumed the NDIS handler called that writer directly. NDIS now calls the policy helper, which invokes the writer and removes the old entry on failure. THIS commit updates the umbrella assertions without weakening the identity/cleanup requirements.
- The RAS manual-index selector hard-pin correction remains in `d6738c390b0578ba07a7bd9f0c2c23a4a1bca764`.

## Verification boundary

Exact-head observations on parent `b83383f9798d5a3e43d86877f9f37c0b1594ffd0`:
- USB modem identity `43847475018`: SUCCESS.
- AI handoff docs `43847474937`: SUCCESS.
- Process lifecycle `43847474942`: SUCCESS; infosvr `43847475001`: SUCCESS.
- HTTPD `43847474960`: FAIL only at its nested `Verify USB modem node metadata staging` Python source assertions; the C regression printed `USB modem node identity and private metadata staging: PASS` before the stale assertions failed.
- NVRAM `43847475238`: IN_PROGRESS at last inspection.
- XY-C1 run `43846822718` from ancestor `d6738c390b0578ba07a7bd9f0c2c23a4a1bca764` had previously been building; no current-source L3 Build/TRX/artifact or physical-router/L4 result is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-head HTTPD rerun, USB identity, AI handoff and current XY-C1/NVRAM Actions. Read any failures; claim firmware L3 only after Build, TRX verification and artifact upload all pass.
2. Continue the device-aware `reload_modem_modules()` readiness review. `module_smart_load()` returns zero for both already-loaded and failed loads; verify actual `/sys/module` state where appropriate. Do not require every alternative driver, and check selected-device readiness only after USB host initialization.
3. Broaden RC/service-start status propagation outside the modem subsystem with caller-level regressions.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup is immutable. Update root `dev_log.md` on each source/documentation commit; keep 20–119 records and preserve archive shards/indexes verbatim. Distinguish source, CI, firmware and physical-router validation. Never claim an unrun test/build/hardware result.
