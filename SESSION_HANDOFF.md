## 2026-11-22 — Current development checkpoint

- Product source checkpoint: `c08483beb90245f70fabe5edb74fa6e6658f2bb6`; frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged).
- Recent source work: bounded `read_whole_file()` with close/error propagation; signed-byte filename sanitation fix; `add_folder()` stage propagation through the centralized account initializer; bounded account metadata paths for permission read/write; explicit `set_permission()` bootstrap failure handling.
- Exact-head Actions:
- Firewall Startup Stability Regression #45018314341: IN_PROGRESS.
- Process inventory and lifecycle fail-closed regression #45018314337: IN_PROGRESS.
- HTTPD Authentication and Security Regression #45018314301: IN_PROGRESS.
- Process lifecycle inventory - c08483beb90245f70fabe5edb74fa6e6658f2bb6 #45018314273: IN_PROGRESS.
- XY-C1 Firmware Validation #45018314239: IN_PROGRESS.
- NVRAM transaction boundary audit #45018314219: PENDING.
- WAN route operation status regression #45018314198: IN_PROGRESS.
- HTTPD daemon lifecycle stability #45018314192: IN_PROGRESS.
- WAN DHCP callback state stability regression #45018314188: IN_PROGRESS.
- FULLCONENAT lifecycle regression #45018314180: IN_PROGRESS.
- IPv6 service-type checked-read regression #45018314178: IN_PROGRESS.
- XY-C1 firmware L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload pass on the same source SHA. Hardware L4 NOT VERIFIED.


---

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`; source checkpoint `c08483beb90245f70fabe5edb74fa6e6658f2bb6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify.
- libdisk reviewed fixes now include tri-state directory enumeration; atomic folder-list and account-permission persistence; cleanup and initialization error propagation; bounded/errno-correct file reader; unsigned-byte safe filename sanitation; fail-closed add-folder setup; bounded permission pathname formatting; checked account-file bootstrap in `set_permission()`.
- Reader source/test workflow compiles `disk_io_tools.c` with GNU89 `-Wall -Wextra -Werror` and ASan/UBSan. The first run exposed the pre-existing signed-char warning in `sanity_name()`; the correction and regression are committed.
- Exact-head Actions:
- Firewall Startup Stability Regression #45018314341: IN_PROGRESS.
- Process inventory and lifecycle fail-closed regression #45018314337: IN_PROGRESS.
- HTTPD Authentication and Security Regression #45018314301: IN_PROGRESS.
- Process lifecycle inventory - c08483beb90245f70fabe5edb74fa6e6658f2bb6 #45018314273: IN_PROGRESS.
- XY-C1 Firmware Validation #45018314239: IN_PROGRESS.
- NVRAM transaction boundary audit #45018314219: PENDING.
- WAN route operation status regression #45018314198: IN_PROGRESS.
- HTTPD daemon lifecycle stability #45018314192: IN_PROGRESS.
- WAN DHCP callback state stability regression #45018314188: IN_PROGRESS.
- FULLCONENAT lifecycle regression #45018314180: IN_PROGRESS.
- IPv6 service-type checked-read regression #45018314178: IN_PROGRESS.
- L3 must be proven on one exact product SHA with Build + TRX + artifact upload all successful. Hardware L4 NOT VERIFIED.


---

## Completed source slices

- Inadyn/DNSMasq: DDNS/AP-mode handling and whitespace-resilient source assertions (see earlier dated log entries).
- detect-WAN: `07fb4ce` repairs the unmatched brace in `chk_udhcpc()`; source-contract and host/process checks passed on that SHA.
- libdisk share-list/metadata: `c24dd1f` checked readdir + XY path filter; `75f461d` whitespace-resilient contract; `9228416` atomic folder snapshot; `f39bc0c` atomic account defaults; `49e6cbf` atomic folder-permission update; `2c099f7` checked metadata cleanup and setup propagation.
- libdisk file and folder operations: `fb430cb` bounded file reader/tests; `29187fd` unsigned-char sanitation correction; later source slices at current checkpoint add folder setup propagation, bounded permission path formatting and checked set-permission bootstrap.


---

## Active task

- The file reader has bounded allocation and explicit invalid/oversized/read/close errors. ASan/UBSan coverage includes text, empty, invalid/missing, oversized and directory-read paths.
- Folder creation now publishes the directory snapshot and delegates all permission-file initialization to the checked central updater; failures are propagated instead of being swallowed.
- Permission readers/writers use a shared bounded pathname helper. `set_permission()` now propagates initial permission-file creation failure and removes the old fixed sleep/retry.
- Next bounded task: audit `del_folder()` and folder rename/update paths for ignored metadata write, close, unlink and per-account failures. Keep each change separately tested and committed.


---

## Verification boundary

- Previous libdisk volume metadata exact run #45016804441 PASS and Process inventory #45016804504 PASS on ancestor `2c099f7`.
- Reader compile initially found `sanity_name()` signed-char comparison under `-Werror`; `29187fd` adds the unsigned-byte fix and regression. Do not count the initial failed run as a pass.
- Exact current product-source Actions:
- Firewall Startup Stability Regression #45018314341: IN_PROGRESS.
- Process inventory and lifecycle fail-closed regression #45018314337: IN_PROGRESS.
- HTTPD Authentication and Security Regression #45018314301: IN_PROGRESS.
- Process lifecycle inventory - c08483beb90245f70fabe5edb74fa6e6658f2bb6 #45018314273: IN_PROGRESS.
- XY-C1 Firmware Validation #45018314239: IN_PROGRESS.
- NVRAM transaction boundary audit #45018314219: PENDING.
- WAN route operation status regression #45018314198: IN_PROGRESS.
- HTTPD daemon lifecycle stability #45018314192: IN_PROGRESS.
- WAN DHCP callback state stability regression #45018314188: IN_PROGRESS.
- FULLCONENAT lifecycle regression #45018314180: IN_PROGRESS.
- IPv6 service-type checked-read regression #45018314178: IN_PROGRESS.
- Same-SHA XY-C1 Build/TRX/artifact is still mandatory for L3. Hardware L4 NOT VERIFIED.


---

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head libdisk sanitizer and source-contract results for the bounded reader, add-folder setup, permission path formatter and set-permission bootstrap.
2. Check the same-SHA XY-C1 detector, Build XY-C1, TRX verification and artifact upload. Claim L3 only if all three build stages pass on one product SHA.
3. Continue with `del_folder()` and folder rename/update error propagation; add regression contracts and atomic replacement where feasible.
4. Keep the backup branch untouched and hardware L4 marked NOT VERIFIED.

