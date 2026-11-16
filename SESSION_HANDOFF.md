# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every logical change must be created as one complete Git tree and one atomic commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; main at handoff checkpoint: `435293278a5deca3b80bd5a21f876a49445d7430`.
- Latest product-source commit found in recent main history: `339d62070960aea11fee1d0b5b18fbf7e373cb86`.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `not returned`; read-only.
- Storage hotplug now propagates FTP/Samba/NFS start failures through `start_stor_apps()` and `try_start_stor_apps()`, while continuing configured starts and preserving one-shot event handling. Safe-removal restart failures are logged.
- Storage media/download helpers `run_dms()`, `run_itunes()`, `run_torrent()`, and `run_aria()` return status; the shared post-launch check reports launch failure, absent process and unknown process-inventory failure. The aggregator preserves first failure across all seven configured daemons.
- USB modem hotplug returns WAN reconnect failures, retains coalesced replug events until unplug completes, restores failed event flags, and schedules retry through the existing five-second alarm.
- WAN IPv6 restart/teardown propagates the first failed stop/start status. LAN/Wi-Fi startup changes propagate service-start failures rather than silently reporting success.
- Mount-link errno normalization on current main: NOT CONFIRMED — inspect create_mp_link() and ensure media/download callers do not inherit stale errno.

## Active task

Continue P1 storage and router-service reliability. The immediate storage edge is `create_mp_link()`: keep its boolean success API for existing callers, but make failure errno deterministic, ensure the errno-normalizing wrapper is declared before every media caller, and retain the first useful filesystem/inventory error. Verify all paths that can fail (invalid input, mount inventory open/read/parse/close, path length, directory preparation, symlink creation, incomplete inventory cleanup). Do not change the existing mount selection/fallback semantics without tests.

The errno-isolation wrapper is not confirmed in current main. First implement it with a GNU89 regression and source-contract test, then verify the storage workflow.

## Verification boundary

- Exact product-source SHA for current checkpoint: `339d62070960aea11fee1d0b5b18fbf7e373cb86`.
- XY-C1 Firmware Validation exact-SHA status: UNVERIFIED.

- Exact-SHA workflow runs observed: `AliDDNS signed API transport security regression`=success (run 43894246294); `AliyunDrive WebDAV lifecycle stability regression`=success (run 43894245942); `Process inventory and lifecycle fail-closed regression`=success (run 43894246097); `infosvr packet and signal lifecycle stability regression`=success (run 43894245954); `AI handoff documentation contract`=success (run 43894246002); `AdGuardHome config security regression`=success (run 43894246085); `USB storage hotplug stability regression`=success (run 43894246341); `NVRAM transaction boundary audit`=success (run 43894245938).
- A prior Build/TRX/artifact PASS is valid only for the SHA on which all three passed; do not reuse it for later source commits.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA storage-hotplug, process-lifecycle, WAN-route, USB-modem, and XY-C1 runs for `339d62070960aea11fee1d0b5b18fbf7e373cb86`; read failing job logs instead of repeating old audits.
2. Compile and run `trunk/user/rc/src/tests/storage_nfs_export_policy.c` in GNU89 mode with `-Wall -Wextra -Werror -fsanitize=address,undefined`; run `scripts/check_storage_safe_remove.py` and the relevant workflow source contracts.
3. If `create_mp_link()` errno handling passes, move to the next storage lifecycle defect; if it fails, fix the exact cause in one atomic commit. Do not wait on PENDING jobs—work on independent source slices.
4. Do not claim current L3 without Build XY-C1, TRX verification and artifact upload passing on the same source SHA. Preserve the frozen backup. Physical-router L4 remains unverified.
