# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main for THIS COMMIT: `180e90075a11ad1135ee6801915211548337b52e`. This parent includes the concurrent RC buffered file-write/close failure fix in `common_ex.c`, its shared helper, regression test, workflow trigger and source contract. Preserve those changes.
- Immutable backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and must remain unchanged. Legacy PR #1 is CLOSED / UNMERGED; its CA bootstrap and AliDDNS HMAC/package/test work is already present in main, so do not replay its divergent branch.
- Exact-main Actions on parent `180e90075a11ad1135ee6801915211548337b52e`: Process inventory and lifecycle fail-closed #43679234773 completed/success; AI handoff documentation contract #43679234764 completed/success; watchdog signal lifecycle #43679234947 completed/success; WAN DHCP callback #43679234703 completed/success. Process lifecycle inventory #43679234730, XY-C1 Firmware Validation #43679234754, HTTPD Authentication/Security #43679234699, Crond lifecycle #43679234687, USB storage #43679234861 and NVRAM audit #43679234792 were IN_PROGRESS at the last poll. Outcomes apply only to that SHA and must be re-fetched.
- The preceding source SHA `335f137a831725f1ed241151d849e160578cca69` passed Process lifecycle #43677484589, wireless radio #43677484449, HTTPD security #43677484423, AI docs #43677484498, NVRAM audit #43677484415 and XY-C1 #43677484373. XY-C1 completed MIPS/uClibc build, TRX verification and artifact upload #14147062315 (12,595,529 bytes). This establishes L3 only for `335f137`; current source requires a fresh XY-C1 pass. L4 physical-router validation remains NOT VERIFIED.
- No local checkout is available in this execution context. Do not claim local host tests, full builds or hardware validation; use exact-head Actions evidence.

## Active task

### Current P1 slice — normal shutdown custom-hook postcondition (THIS COMMIT)

- `shutdown_router()` invoked the configured `SCRIPT_SHUTDOWN` hook but discarded `doSystem()` status. A failing operator hook could therefore be followed by `storage_save_time()` and `write_storage_to_mtd()`, unlike the already checked firmware-update path.
- This slice captures/logs the hook status, preserves the remaining power-down and storage-eject sequence, and requires a zero hook status before storage-time or MTD persistence.
- `scripts/check_process_lifecycle_integration.py` checks capture, diagnostic, exactly-once invocation and ordering before storage eject / MTD persistence. It preserves the concurrent file-write close-status coverage introduced on parent `180e900`.
- Parent `180e90075a11ad1135ee6801915211548337b52e` exact Actions: Process inventory/fail-closed #43679234773 and AI docs #43679234764 passed; watchdog #43679234947 and WAN DHCP #43679234703 passed. Process lifecycle inventory #43679234730, XY-C1 #43679234754, HTTPD #43679234699, Crond #43679234687, USB storage #43679234861 and NVRAM #43679234792 were pending at last poll. Fresh exact-head Actions are required.
- The previous source SHA `335f137a831725f1ed241151d849e160578cca69` passed exact Process lifecycle, wireless radio, HTTPD, AI docs, NVRAM and full XY-C1 build/TRX/artifact upload. Its MIPS artifact does not validate this later source.
- Caller audit: stop_syslogd/stop_klogd and stop_udpxy/stop_xupnpd wrappers discard kill status but have no callsites identified in the RC C source set. stop_logger() and stop_igmpproxy() propagate their grouped stop status. Samba/storage public stop APIs retain locked checked postconditions. Next inspect stop_rwfs_optware() and other legacy void shutdown hooks; do not repeat completed caller scans.



### Implemented P1 foundation

- New shared tri-state scanner: `trunk/user/shared/src/process_inventory.h`, `process_inventory.c`; states RUNNING/ABSENT/UNKNOWN are distinct, count is separate, identities capture PID/start_time/comm/argv hashes, procfs and allocation ops are injectable, EOF/errors distinguished, partial snapshots are discarded on UNKNOWN.
- `trunk/user/shared/src/Makefile` includes `process_inventory.o`; the source file and object now match. MIPS job confirmed this object compiled and `libshared.so` installed `process_inventory.h` into `trunk/stage/include/shared`.
- `/proc/PID/stat` field 22 start_time is token 20 after comm. Scanner test asserts concrete value `98765` and exercises disappearance between stat/cmdline.
- `pids_main()` is now a pure CLI exit-status API: complete RUNNING/ABSENT returns `EXIT_SUCCESS`; UNKNOWN returns `EXIT_FAILURE`; count/PIDs are printed only. This was required because `rc.c` returns its value directly from the `/sbin/pids` applet. Legacy `pids()` remains Boolean-compatible until all 77 callers are migrated.
- `service_lifecycle_policy.h` includes staged public header `<process_inventory.h>`; Watchdog/HTTPD host compile commands add `-I trunk/user/shared/src`. Their workflow path filters watch the public shared header in push and pull_request events.
- `kill_services()` preflights all configured service names before TERM, checks full plan membership before first signal, revalidates PID identity before each TERM/KILL, and signals the captured PID (not a later name-wide killall). UNKNOWN/CHANGED/TIMEOUT/FAILED are not STOPPED.
- Production-connected host tests and CI: `trunk/user/shared/src/tests/process_inventory.c`, `trunk/user/rc/src/tests/service_lifecycle_policy.c`, `.github/workflows/process-lifecycle-regression.yml`, `scripts/check_process_lifecycle_integration.py`.
- The one-line handoff rule is documented in `AI_BOOTSTRAP_PROMPT.md` and `AI_DEVELOPMENT_PROTOCOL.md`.

### In-tree PID-file identity helper — partially migrated; PPPD, udhcpc and iNICD now included

- Added process_pidfile.c/.h to the shared library: strict PID + start_time record parsing, expected-process inventory matching, identity revalidation immediately before signaling, and atomic same-directory temporary-file publication. Its parser avoids variable 64-bit division, so the MIPS shared object does not require hidden __udivdi3.
- Identity-bound publication/signaling now covers detect_internet (producer, two RC paths, HTTPD SIGHUP), detect_link (producer and both RC paths), rstats (producer, RC SIGHUP, HTTPD refresh/history) and watchdog (producer, RC time refresh and Wi-Fi notification). LPD has its own strict parser and identity-validated consumer.
- Watchdog publishes PID + start_time only after daemonization and fails startup closed if publication fails. Both RC notification callers use process_pidfile_signal() with expected name watchdog. Time refresh launches a replacement only when the legacy pids() guard confirms absence; UNKNOWN remains truthy and cannot create a duplicate. Shutdown deliberately retains the last record rather than unlinking by pathname, which could remove a fast-restarted watchdog's new record. Stale PID/start_time records are rejected by the helper.
- The rstats `saved_errno` compile error found on `733463cb` was fixed by `dd7a9b42`; the dd7a XY-C1 run `43624870668` subsequently completed SUCCESS. The watchdog-inclusive current-main build and watchdog signal regression also completed SUCCESS on `f5cc0baf`.
- Identity-bound publication/stop coverage now also includes PPPD base and PPP link PID records, BusyBox udhcpc WAN/LAN PID records, and INICD. VPN client/server stop paths include PPP identity preflight; WAN, AP-LAN and USB-modem teardown propagate stop failures and restore the internet monitor on aborted teardown. Process inventory/lifecycle [43647169443], iNIC signal [43647169424] and wireless-radio [43647169416] passed on exact SHA `946a8719`.

### Historical exact-head Actions on product-source SHA `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c`

All ten `push` workflows below checked out this exact product-source SHA (then the main tip) and completed SUCCESS:

- XY-C1 Firmware Validation [43625410221] — Build XY-C1, Verify TRX output and artifact upload all SUCCESS; artifact [14125764801] (12,583,902 bytes).
- AI handoff documentation contract [43625410189] — SUCCESS.
- Watchdog signal lifecycle stability [43625410193] — SUCCESS.
- Process inventory/lifecycle regression [43625410154] — SUCCESS.
- Process lifecycle inventory [43625410226] — SUCCESS.
- NVRAM transaction boundary audit [43625410190] — SUCCESS.
- AdGuardHome config security [43625410170] — SUCCESS.
- AliDDNS signed API transport [43625410262] — SUCCESS.
- AliyunDrive WebDAV lifecycle [43625410196] — SUCCESS.
- infosvr packet/signal lifecycle [43625410180] — SUCCESS.

The earlier dd7a XY-C1 run [43624870668] also completed SUCCESS; it is superseded by the successful current-main build.

### XY-C1 MIPS/uClibc target build — historical baseline verified; current source pending

- Last fully verified product-source baseline: `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286`, exact-head XY-C1 [43648229489] passed MIPS/uClibc build, TRX verification and artifact upload [14135711084] (12,587,100 bytes; SHA-256 `bda83c9e71ed01a8708ad90a5104535e972bd30424bdf03ed24065b3a1a9407e`).
- Exact XY-C1 [43660915762] for product-source commit `f4e78554c490c28697fc7819de8ba4947ab6eb48` was IN_PROGRESS at last poll; it includes Samba WINS identity/restart and complete-ABSENT postcondition changes. This workflow/doc-only follow-up does not alter product source. L3 remains NOT VERIFIED until that run's Build, TRX check and artifact upload pass.
- Physical XY-C1 flash/boot/runtime L4 remains NOT VERIFIED.


## Explicitly uncompleted P1 work

- All 77 direct `pids()` callers remain legacy Boolean users. Migrate by semantic group; never change `pids()` to negative UNKNOWN until every caller is explicit.
- LPD PID publication uses exclusive same-directory temporary-file creation and atomic rename after a complete checked write; its strict identity-aware duplicate consumer treats malformed/unreadable records as UNKNOWN. Publication follows successful socket bind; stop does not unlink because that could delete a replacement daemon's newly published record. Shared identity-bound PID publication/signaling now covers detect_internet, detect_link, rstats, watchdog, PPPD base/link daemons, BusyBox udhcpc and INICD. Stop gates also cover VPN client/server, WAN DHCP, AP-LAN DHCP and USB modem teardown. Remaining legacy process-presence callers and broader system shutdown/storage/Samba/NVRAM postconditions remain open.
- Remaining P1 review: inspect `stop_rwfs_optware()` and other legacy void hooks reachable during normal shutdown; then trace any NVRAM/storage publication that can still follow an unchecked stop. Firmware flash, logger, radio, authentication, WAN, Samba post-stop ABSENT checks and storage-eject gates have already been inspected; do not repeat them.
- Continue migrating remaining direct `pids()` caller groups and `kill_pidfile_s()` use sites; audit direct `killall` and signal paths outside `kill_services()`. PPP, udhcpc and iNICD identity producers plus current VPN/WAN/LAN/USB stop gates are already implemented; do not repeat those slices.
- Linux 4.4 has no `pidfd_send_signal()`; PID/start_time validation leaves a check/kill TOCTOU window. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and was not changed.

## Verification boundary

- L1: prior lexical inventory and direct-call semantic review PASS within the recorded scope; do not repeat completed inventories.
- L2: latest source-level exact-head evidence includes process inventory [43648229415], PID1 [43648229463], DNS Forwarder [43648229531], storage hotplug [43648229477], Samba [43648229471], NFS [43648229491], HTTPD [43648229544], WAN route [43648229405], AdGuardHome [43648229513], watchdog [43648229404], AliDDNS [43648229509], infosvr [43648229453], VPN/other module checks as recorded in the current-main Actions. On the follow-up docs-only head `7697c6f`, VPN [43648923434], AI docs [43648923326], process inventory [43648923376], AdGuardHome [43648923421], AliDDNS [43648923338], infosvr [43648923306] and AliyunDrive [43648923366] passed. Networkmap [43648923325] failed only on the incorrect checker prototype assertion; this commit fixes it. NVRAM [43648923357] was PENDING/IN_PROGRESS on `7697c6f`.
- L3: Current production source is NOT VERIFIED. XY-C1 [43654318351] on ea2163e failed at RC final link because disk_inventory_parse_decimal() introduced variable 64-bit division and a hidden __udivdi3 dependency in libdisk.so. This commit removes all division using a constant ULLONG_MAX cutoff and adds max-value parsing coverage. A new exact-head XY-C1 Build, TRX verification and artifact upload must pass before L3 can be claimed.
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub.

## HTTPD daemon lifecycle — completed slice

- `start_httpd()` now uses `process_inventory_scan("httpd", ...)` with tri-state `ABSENT/RUNNING/UNKNOWN` semantics. Only an authoritative ABSENT result permits NVRAM configuration normalization, stale-state clearing, launcher execution, or firewall publication.
- A confirmed RUNNING daemon returns `EBUSY` and may publish `httpd_started=1`; UNKNOWN returns `EIO` (or scan errno) without mutating configuration or claiming the daemon is running.
- Detached-child readiness now requires RUNNING. ABSENT retries then returns `ESRCH`; UNKNOWN terminates the probe with an error and never publishes started state. GNU89 regression covers RUNNING, ABSENT, UNKNOWN, invalid states and preserved launcher errors.
- The first exact-head lifecycle Action exposed a stale source-contract assertion expecting the old `status == 0` form. The checker was corrected to assert the new fail-before-publish branch. Current exact run state is listed in the repository checkpoint above.

## Storage eject propagation — completed slice

- `read_disk_data_checked()` now strictly validates `/proc/partitions`: canonical four-column header, blank separator, complete newline-terminated records, exactly four fields, unsigned decimal major/minor/block counts, and overflow-safe conversion. Malformed, signed, truncated, overlong, or overflowed records fail closed; the legacy `read_disk_data()` pointer wrapper is preserved.
- `safe_remove_all_stor_devices()` distinguishes unknown inventory from valid empty inventory, checks the per-device path and generic fallback, and requires a complete final mount snapshot to prove zero storage mounts. Firmware update does not persist storage MTD or execute `mtd_write` when eject is incomplete; normal reboot/halt proceeds but skips storage MTD persistence on failure.
- Parser regression is compiled as GNU89 with `-Wall -Wextra -Werror`. The exact c5 parser Action initially failed earlier in the job on BusyBox `writev` missing its declaration; `b022942` added `<sys/uio.h>`, after which the storage-hotplug exact-head run is represented in the latest status block above. Do not treat prior SHA results as evidence for later source.
- Backup branch remains immutable; physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live main and exact-head Actions once. Confirm Process lifecycle inventory, Process inventory/fail-closed, AI handoff docs, XY-C1 and the parent workflows by `head_sha`; inspect any failed job logs. The parent `180e90075a11ad1135ee6801915211548337b52e` is not evidence for THIS COMMIT. Its last known statuses are recorded above.
2. Continue P1 at `stop_rwfs_optware()` and other legacy void stop hooks called during normal shutdown. Trace implementation, all callsites and what state / persistent writes can follow before changing signatures. Preserve the sequencing needed to stop late Optware workers.
3. Audit NVRAM/storage publication only where an unchecked stop could still race or invalidate persisted state. Do not repeat checked firmware flash, normal shutdown hook, logger, IPTV, radio, Samba or storage public stop paths.
4. Keep changes small and atomic: one complete Git tree/commit parented to fresh main, update main with expected SHA and force=false, then inspect exact-SHA Actions. Preserve the immutable backup and do not claim local or L4 results without evidence.

Source/validation notes:
- Parent main SHA for THIS COMMIT: `180e90075a11ad1135ee6801915211548337b52e`; immutable backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Historical L3 passed on `335f137a831725f1ed241151d849e160578cca69`; the XY-C1 run on `180e900` was pending at the last poll. Revalidate any later source SHA with full Build, TRX verify and artifact upload.
- Root `dev_log.md` had 94 records before this entry; keep archive shards and index byte-for-byte.

