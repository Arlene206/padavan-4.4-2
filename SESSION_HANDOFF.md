# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `d6fac506d564e8277e6c81ec2dd18620b6509144`; the mount-link errno implementation and storage workflow regression passed exact-head storage run `43904136310`. Exact-head Wireless `43904136184`, AI docs `43904136356`, NFS lifecycle `43904136537`, Samba `43904136473`, infosvr `43904136412`, vsftpd `43904136251`, modem identity `43904136447`, AdGuardHome `43904136471`, HTTPD security `43904136370`, AliDDNS `43904136262`, and AliyunDrive `43904136387` passed. Process lifecycle run `43904136542` failed at job `133428303695`, step 38 only because its source checker still expected the removed `return link_created` / single-line cleanup form; this checker-only correction is included in THIS COMMIT. XY-C1 run `43904136433` was still building at last poll; parent e05's success does not transfer to d6fac or this commit.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

The process-lifecycle failure on `d6fac506d564e8277e6c81ec2dd18620b6509144` was not a product-source failure: the checker still asserted the old mount-link implementation. The current source now returns explicit boolean branches and stages cleanup/read/close status separately. This commit aligns that checker with the current implementation and checks the errno adapter plus its four media callers. Re-run process-lifecycle on this exact new SHA; do not revert correct source code to satisfy a stale assertion.

The bounded mount-link reliability slice is implemented in THIS COMMIT. Do not repeat its audit. It covers invalid input, `/proc/mounts` open/read/parse/close failures, path length, directory preparation, symlink creation, and failed link cleanup after an incomplete inventory. The GNU89 policy regression and storage source contract are wired into the USB storage workflow. Existing mount selection and fallback semantics are unchanged.

After exact-head checks pass, continue P1 storage/router-service reliability with the next concrete defect in the remaining hotplug/eject/service-restart call graph. Do not repeat already-completed process inventory, modem retry, WAN IPv6, or mount-link audits without a new failure signal.

## Verification boundary

- Source parent for this checker-only commit: `d6fac506d564e8277e6c81ec2dd18620b6509144`.
- On parent d6fac: USB storage run `43904136310` PASS (includes the new GNU89/ASan/UBSan errno policy test and final source contract); Wireless `43904136184` PASS; AI docs `43904136356` PASS; process lifecycle `43904136542` FAILED only at stale assertion in `scripts/check_process_lifecycle_integration.py:200`; XY-C1 `43904136433` remained IN_PROGRESS at `Build XY-C1` at last poll. Other results listed in Current repository state.
- The checker correction below must pass on its exact new SHA. XY-C1 current source L3 remains NOT VERIFIED until Build, TRX check and artifact upload all pass on one exact source SHA. Physical-router L4 remains NOT VERIFIED.

- Product-source parent SHA: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`; this commit contains a new product-source change and requires its own exact-head checks.
- XY-C1 Firmware Validation on parent SHA `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`: PASS (run `43897222785`; Build XY-C1, Verify TRX output, and artifact upload all succeeded on that exact SHA). This L3 result does not transfer to THIS COMMIT.
- Exact-head checks on `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`: iNIC signal `43897223046` PASS; watchdog signal `43897222831` PASS; HTTPD authentication/security `43897223223` PASS; process lifecycle inventory `43897222738` PASS; process inventory/fail-closed `43897222797` PASS; NVRAM transaction audit `43897222771` PASS.
- Two parent-SHA failures are corrected in THIS COMMIT: Wireless radio state `43897222713` failed because its checker expected a direct bridge ioctl instead of the current bridge-membership helper; AI handoff docs `43897222818` failed because the handoff omitted the required literal `one atomic main commit` phrase.
- Exact-head storage, wireless, AI docs and XY-C1 Actions for THIS COMMIT: PENDING until their push runs are observed. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Process inventory/lifecycle regression for THIS COMMIT and verify the updated storage mount-link assertions pass; read the full log if another assertion fails.
2. Inspect XY-C1 Build, TRX verification and artifact upload for THIS COMMIT as one same-SHA L3 gate; do not claim the parent e05 result for newer source.
3. With those checks running, continue independently on concrete storage hotplug lifecycle defects (automount failure reporting/notification semantics) only after checking `notify_rc` and `_eval` contracts; do not repeat the mount-link task.
4. Preserve frozen backup `backup/pre-plugin-overhaul-2026-11-14` unchanged. Physical-router L4 remains unverified.
