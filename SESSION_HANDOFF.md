## 2026-11-22 — Current development checkpoint

- Live main source checkpoint: `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not move or delete).
- Current source slice: bounded and errno-preserving `read_whole_file()`, the signed-byte filename fix in `sanity_name()`, and fail-closed `add_folder()` initialization now live on main.
- Exact-head Actions:
- `Process lifecycle inventory - 43a5bfb5bc3f0195abfc00db9e810e58ad3514a1` #45018022701: IN_PROGRESS.
- `XY-C1 Firmware Validation` #45018022664: IN_PROGRESS.
- `NVRAM transaction boundary audit` #45018022657: IN_PROGRESS.
- `libdisk share-list output safety` #45018022649: IN_PROGRESS.
- `HTTPD Authentication and Security Regression` #45018022644: IN_PROGRESS.
- Do not claim L3 until Build XY-C1, TRX verification and artifact upload all pass for identical product-source SHA `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`. Hardware L4 NOT VERIFIED.


---

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`; checkpoint SHA `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Established libdisk fixes: readdir tri-state/error propagation; atomic folder-list snapshot; atomic per-account defaults; atomic permission-file update for new folders; checked metadata cleanup; volume setup stage failure propagation; checked `read_whole_file()`; signed-byte safe filename filtering; checked folder creation and permission initialization.
- Latest source slices:
  - `fb430cb`: bounded file reader, errno propagation, close-error handling, GNU89 read regression and host/firmware path filters.
  - `29187fd`: unsigned-byte comparison in `sanity_name()` plus regression, prompted by the first compile's `-Werror=type-limits` finding.
  - `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`: `add_folder()` now uses dynamic path allocation, reports duplicate/lookup/mkdir errors, publishes folder snapshot before running checked account metadata initialization.
- Existing firmware gate includes libdisk production source paths, including `disk_io_tools.c`; the workflow remains main-push-only, with host sanitizer regression in the dedicated libdisk workflow.
- Latest exact-head Actions:
- `Process lifecycle inventory - 43a5bfb5bc3f0195abfc00db9e810e58ad3514a1` #45018022701: IN_PROGRESS.
- `XY-C1 Firmware Validation` #45018022664: IN_PROGRESS.
- `NVRAM transaction boundary audit` #45018022657: IN_PROGRESS.
- `libdisk share-list output safety` #45018022649: IN_PROGRESS.
- `HTTPD Authentication and Security Regression` #45018022644: IN_PROGRESS.
- Same-SHA XY-C1 Build + TRX + artifact upload are mandatory for L3; no hardware L4 claim is made.


---

## Completed source slices

- Inadyn/DNSMasq: status-bearing DDNS enable/AP-mode handling and whitespace-resilient source assertions; exact checks recorded in prior log entries.
- Detect-WAN: `07fb4ce` fixes the stray brace that broke `chk_udhcpc()`; source contract and host/process checks passed on that source SHA.
- libdisk share-list and volume metadata: `c24dd1f` checked readdir and XY-C1 filter coverage; `75f461d` whitespace-safe source contract; `9228416` atomic folder snapshot; `f39bc0c` atomic account defaults; `49e6cbf` atomic new-folder permission update; `2c099f7` checked cleanup and setup-stage propagation.
- libdisk file I/O and directory management: `fb430cb` bounded checked reader; `29187fd` signed-byte conversion fix; current checkpoint `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1` fail-closed add-folder orchestration.
- Frozen backup branch remains unchanged.


---

## Active task

- The reader hardening now has explicit tests for valid text, empty content, invalid/missing paths, >1 MiB files and directory read errors; it also checks successful `fclose()` and preserves the primary errno while cleaning up error paths.
- `add_folder()` now creates a directory with checked dynamic path construction, atomically updates the folder-list snapshot, and invokes the centralized per-account initializer which already checks/atomically commits every account file. Failure no longer returns success or silently skips accounts.
- Next bounded task: audit `del_folder()` and the folder-rename path for the same ignored file-write, close, unlink and metadata-stage errors. Keep each operation as its own source/test slice.


---

## Verification boundary

- The preceding volume-metadata slice #45016804441 PASS and Process inventory #45016804504 PASS on `2c099f7`.
- The bounded-reader work initially exposed `sanity_name()` signed-char warning under GNU89 `-Werror`; `29187fd` corrects it and adds a regression.
- Exact current-head CI status is listed above; no unobserved test is claimed as PASS.
- XY-C1 L3 requires Build, TRX and artifact upload all PASS on the identical product-source SHA `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`. Hardware L4 NOT VERIFIED.


---

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head libdisk sanitizer/source-contract Actions for the bounded reader and `add_folder()` changes; fix the first causal failure without weakening the contracts.
2. Check XY-C1 for product SHA `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`: Build XY-C1, TRX verification and artifact upload must all pass on this one SHA before claiming L3.
3. Continue with `del_folder()` / folder-rename metadata updates: make read/write/close/unlink and per-account failures explicit and atomic where feasible; add regression/source-contract coverage.
4. Keep `backup/pre-plugin-overhaul-2026-11-14` immutable and hardware L4 marked NOT VERIFIED.

