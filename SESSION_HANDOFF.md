## 2026-11-22 — Current development checkpoint

- Current product-source SHA: `9c9f120422bc5b3bd025a7a6325dacf12dba4a84` (main at read time).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged; read-only).
- Source slices now on main include bounded read/close handling, signed-byte-safe filename sanitation, checked add-folder initialization, and bounded account permission path formatting.
- Exact-head Actions:
- NVRAM transaction boundary audit #45018180705: IN_PROGRESS
- HTTPD Authentication and Security Regression #45018180700: COMPLETED/SUCCESS
- libdisk share-list output safety #45018180692: COMPLETED/SUCCESS
- Process lifecycle inventory - 9c9f120422bc5b3bd025a7a6325dacf12dba4a84 #45018180618: COMPLETED/SUCCESS
- XY-C1 Firmware Validation #45018180615: IN_PROGRESS
- XY-C1 L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS for one identical product-source SHA. Hardware L4 NOT VERIFIED.


---

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`; source checkpoint `9c9f120422bc5b3bd025a7a6325dacf12dba4a84`.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- libdisk stability fixes: checked readdir enumeration; atomic folder-list snapshot; atomic per-account defaults and new-folder permission update; checked metadata cleanup and stage propagation; bounded `read_whole_file()` with checked close/error errno; unsigned-byte filename sanitation; checked `add_folder()` orchestration; checked account-permission path construction.
- Related source commits: `fb430cb` bounded reader/tests/workflow triggers; `29187fd` signed-char correctness fix and regression; current `9c9f120422bc5b3bd025a7a6325dacf12dba4a84` includes add-folder orchestration and bounded permission pathname helper.
- Dedicated libdisk sanitizer workflow now compiles the production `disk_io_tools.c` together with the reader regression. XY-C1 input filter watches `disk_io_tools.c` as a production input.
- Exact-head Actions:
- NVRAM transaction boundary audit #45018180705: IN_PROGRESS
- HTTPD Authentication and Security Regression #45018180700: COMPLETED/SUCCESS
- libdisk share-list output safety #45018180692: COMPLETED/SUCCESS
- Process lifecycle inventory - 9c9f120422bc5b3bd025a7a6325dacf12dba4a84 #45018180618: COMPLETED/SUCCESS
- XY-C1 Firmware Validation #45018180615: IN_PROGRESS
- XY-C1 L3 remains unverified until same-SHA Build, TRX and artifact upload all pass; hardware L4 remains NOT VERIFIED.


---

## Completed source slices

- Inadyn/DNSMasq: DDNS/AP-mode handling and whitespace-resilient source assertions (see earlier dated log entries).
- detect-WAN: `07fb4ce` fixed the extra brace in `chk_udhcpc()`; exact detect-WAN source contract and host/process checks passed on that SHA.
- libdisk share-list and volume metadata: `c24dd1f` checked readdir and XY filter; `75f461d` source-contract whitespace tolerance; `9228416` atomic folder snapshot; `f39bc0c` atomic account defaults; `49e6cbf` atomic new-folder permission update; `2c099f7` checked cleanup and stage propagation.
- libdisk file handling and directory setup: `fb430cb` bounded checked reader; `29187fd` unsigned-byte sanitation correction; current source checkpoint `9c9f120422bc5b3bd025a7a6325dacf12dba4a84` checked folder initialization and permission-path formatting.
- Backup branch is frozen and unchanged.


---

## Active task

- Reader regression is designed to exercise successful text/empty reads, EINVAL/ENOENT, EFBIG for >1 MiB, and directory I/O error; source contract checks close/error cleanup.
- `add_folder()` now updates the folder snapshot first and delegates permission-file creation/update to the central checked initializer, returning failures rather than silently skipping users.
- Account permission path helper rejects truncation. `get_permission()` fails closed on a path that cannot be represented in its buffer; `set_permission()` reports the error.
- Next bounded review: `del_folder()` and folder-rename operations still need a focused audit for ignored metadata write/close/unlink failures; fix one operation at a time.


---

## Verification boundary

- Previous volume metadata exact libdisk run #45016804441 PASS and Process inventory #45016804504 PASS on ancestor `2c099f7`.
- The reader workflow's first attempt exposed the `sanity_name()` signed-byte warning under `-Werror`; `29187fd` fixes it and adds a regression. Do not carry forward the failed run as a pass.
- Current exact-head Actions:
- NVRAM transaction boundary audit #45018180705: IN_PROGRESS
- HTTPD Authentication and Security Regression #45018180700: COMPLETED/SUCCESS
- libdisk share-list output safety #45018180692: COMPLETED/SUCCESS
- Process lifecycle inventory - 9c9f120422bc5b3bd025a7a6325dacf12dba4a84 #45018180618: COMPLETED/SUCCESS
- XY-C1 Firmware Validation #45018180615: IN_PROGRESS
- No current L3 claim. Same-SHA XY-C1 Build + TRX + artifact upload are mandatory; hardware L4 NOT VERIFIED.


---

## Exact first action for the next GPT

1. Fetch live `main`; inspect current exact-head libdisk sanitizer/source-contract result for the reader, `add_folder()` and bounded permission-path helper.
2. Check same-SHA XY-C1 Build XY-C1, TRX verification and artifact upload; only mark L3 after all three pass on one product SHA.
3. Continue the bounded `del_folder()` / folder-rename audit, adding explicit failure propagation and atomic metadata updates with regression coverage.
4. Preserve `backup/pre-plugin-overhaul-2026-11-14` unchanged and keep hardware L4 NOT VERIFIED.

