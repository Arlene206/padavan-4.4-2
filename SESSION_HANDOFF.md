# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `d935aed8045f940a42d4c19d46b26bcfdcdd09cb` (checker-only follow-up to product source SHA `d6fac506d564e8277e6c81ec2dd18620b6509144`). Process lifecycle run `43906449302` failed at step 38 because the previous fix called `body()` without defining it in this standalone checker; THIS COMMIT adds a local brace-aware `c_function_body()` parser. AI handoff documentation `43906449223`, AliyunDrive `43906449278`, infosvr `43906449636`, AdGuardHome `43906449172`, and AliDDNS `43906449314` passed on d935; NVRAM audit `43906449401` was IN_PROGRESS at last poll. On product SHA d6fac, USB storage `43904136310` passed and XY-C1 `43904136433` has now passed Build, TRX verification and artifact upload on that same SHA. Physical-router L4 remains NOT VERIFIED.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

The current product-source baseline is `d6fac506d564e8277e6c81ec2dd18620b6509144`; mount-link errno handling and its GNU89 regression are implemented there, USB storage run `43904136310` passed, and XY-C1 Build/TRX/artifact run `43904136433` passed on that exact product SHA. The checker-only follow-ups `e8a7aedab3bf7d1cc5b5487234ef2efc3b53ba5b` and `d935aed8045f940a42d4c19d46b26bcfdcdd09cb` exposed checker bugs in turn (escaped-newline split, then undefined `body()` helper). THIS COMMIT defines a local brace-aware `c_function_body()` parser and uses it for the mount-link implementation, adapter and four media callers. Re-run Process lifecycle against THIS COMMIT; do not change product source to satisfy brittle text slicing.

After exact-head checks pass, continue P1 storage/router-service reliability with the next concrete defect in the remaining hotplug/eject/service-restart call graph. Do not repeat already-completed process inventory, modem retry, WAN IPv6, or mount-link audits without a new failure signal.

## Verification boundary

- Product-source SHA: `d6fac506d564e8277e6c81ec2dd18620b6509144`; exact-head USB storage run `43904136310` PASS; exact-head XY-C1 run `43904136433` PASS with Build, TRX verification and artifact upload all successful on that same SHA.
- Checker-only parent SHA: `d935aed8045f940a42d4c19d46b26bcfdcdd09cb`; Process lifecycle run `43906449302` FAILED at `scripts/check_process_lifecycle_integration.py:195` because `body()` was undefined. THIS COMMIT adds the local parser. Process and AI handoff exact-head checks for THIS COMMIT are pending; NVRAM audit `43906449401` was still in progress on parent d935.
- Physical-router L4 remains NOT VERIFIED.

- Product-source parent SHA: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`; this commit contains a new product-source change and requires its own exact-head checks.
- XY-C1 Firmware Validation on parent SHA `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`: PASS (run `43897222785`; Build XY-C1, Verify TRX output, and artifact upload all succeeded on that exact SHA). This L3 result does not transfer to THIS COMMIT.
- Exact-head checks on `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`: iNIC signal `43897223046` PASS; watchdog signal `43897222831` PASS; HTTPD authentication/security `43897223223` PASS; process lifecycle inventory `43897222738` PASS; process inventory/fail-closed `43897222797` PASS; NVRAM transaction audit `43897222771` PASS.
- Two parent-SHA failures are corrected in THIS COMMIT: Wireless radio state `43897222713` failed because its checker expected a direct bridge ioctl instead of the current bridge-membership helper; AI handoff docs `43897222818` failed because the handoff omitted the required literal `one atomic main commit` phrase.
- Exact-head storage, wireless, AI docs and XY-C1 Actions for THIS COMMIT: PENDING until their push runs are observed. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head Process inventory/lifecycle run for THIS COMMIT. Confirm step 38 now executes the local brace-aware parser and passes; read and fix any new causal failure.
2. Check AI handoff docs for THIS COMMIT. The product SHA `d6fac506d564e8277e6c81ec2dd18620b6509144` already passed exact-head USB storage and XY-C1 Build/TRX/artifact; do not repeat those validations without a new product-source change.
3. Continue P1 storage/eject reliability: repair the retry-loss path in `safe_remove_stor_device()` when remaining mounted storage exists but `start_stor_apps()` fails (or mount inventory is UNKNOWN). Reuse the existing `usb_hotplug_ms` / five-second deferred retry mechanism; add policy regression and source-contract coverage. Do not repeat the completed mount-link audit.
4. Preserve frozen backup `backup/pre-plugin-overhaul-2026-11-14` unchanged. Physical-router L4 remains unverified.
