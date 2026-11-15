# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Base main SHA for THIS COMMIT: `bda473715cf8bb2e1bbb8c704370639854f2a07c`. Product-source work now on main includes checked buffered storage writes (`180e90075a11ad1135ee6801915211548337b52e`), wireless radio and custom-hook shutdown persistence gates (`335f137a831725f1ed241151d849e160578cca69`, `d16c7ed591fdec961992f6236a441f12c286fe61`), Optware worker/hook and RWFS/UBI teardown gates (`b41fa1fae03904078f0d765d157d2dfdf97daa51`, `e89f8ca3da85860e71acfd4f6a50eae4ce096311`), and identity-validated Transmission/Aria stop transactions (`2aedad115793e7a6de4f2114272c845559105783`). Immutable backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; DO NOT modify it. Legacy PR #1 remains CLOSED / UNMERGED; its AliDDNS HMAC helper and associated test files are present on main.
- Exact-head USB storage hotplug run #43683399235 on `bda473715cf8bb2e1bbb8c704370639854f2a07c` FAILED at step 15, `python3 scripts/check_storage_safe_remove.py`, with `ValueError: unexpected '{' in field name` at line 184. The checker used `.format(daemon)` on a C array initializer containing literal braces. Step 14's full storage caller-graph/lock-order contract PASSED; all earlier steps passed. This commit repairs the checker expression; a fresh exact-head storage run must verify the complete job.
- Other Actions on this exact SHA: Process inventory/lifecycle #43683399240 SUCCESS; AdGuardHome #43683399227 SUCCESS; infosvr #43683399233 SUCCESS; AI handoff docs #43683399266 SUCCESS; AliDDNS #43683399231 SUCCESS; AliyunDrive #43683399312 SUCCESS; NVRAM transaction audit #43683399234 SUCCESS. These do not turn the storage failure green.
- No full local checkout/toolchain is available. Do not claim full local workflow execution, current-source MIPS build/TRX validation, or hardware testing. Current L3 and physical L4 remain NOT VERIFIED.

## Active task

### Current P1 slice — repair storage safe-remove checker runtime failure (THIS COMMIT)

- Exact-head USB storage push #43683399235 on `bda473715cf8bb2e1bbb8c704370639854f2a07c` passed all syntax, GNU89 policy, BusyBox ash/Optware lifecycle, disk inventory, FTP presence, storage daemon stop and embedded caller-graph checks, then failed at the standalone `scripts/check_storage_safe_remove.py` invocation.
- The checker crashed before its remaining source assertions: line 184 called `.format(daemon)` on `'char *svcs[] = { "{}", NULL };'`, so Python interpreted the literal C initializer braces and raised `ValueError: unexpected '{' in field name`.
- Replaced that construction with explicit string concatenation while retaining the exact expected C declaration; the source contract still requires the identity-validated `kill_services()` transaction and verified daemon-absence postcondition.
- This is a regression-checker defect, not a product-source change. A fresh exact-head USB storage `push` run must pass the standalone checker and complete the job. No full local checkout/test or target build is available.
 
### Next P1 review — remaining NVRAM and storage publication callers

- After the exact-head storage checker passes, trace the actual remaining callers of `write_storage_to_mtd()`, `storage_save_time()` and NVRAM commit wrappers. Follow write/fsync/close and prerequisite stop/unmount statuses through each publication decision; identify a concrete fail-open branch before editing.
- Account for already-landed gates before selecting a change: checked buffered writes (`180e900`), radio/custom-hook shutdown gates (`335f137`, `d16c7ed`), Optware worker/hook stop and RWFS/UBI teardown (`b41fa1f`, `e89f8ca`), and Transmission/Aria identity-bound stop (`2aedad1`). Do not repeat those implementations; determine what remains unchecked in the current callers.
- Do not repeat safe-eject inventory, storage operation locks, or the completed shutdown persistence gates. Add a focused GNU89 regression and production source contract for the first actual remaining defect.

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
- Remaining P1 work: after exact-head validation, audit concrete NVRAM/storage writers whose callers can publish state after unchecked stop, unmount, write, fsync or close failures. Do not repeat the now-checked Optware worker, service-hook, RWFS unmount or UBI detach paths.
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

1. Fetch live main and inspect the new exact-head USB storage run for this checker fix. Confirm that `scripts/check_storage_safe_remove.py` executes its full assertion set and the workflow completes; inspect the first causal failure if it does not.
2. Once storage validation is clean, continue P1 by tracing the remaining `write_storage_to_mtd()`, `storage_save_time()` and NVRAM commit callers. Avoid redoing the write, shutdown-hook, radio, Optware, RWFS/UBI and torrent-stop gates listed above.
3. For the first concrete unchecked result, add a focused GNU89 policy regression and production-connected source contract. Publish all changed files as one atomic main commit using a freshly fetched parent SHA, complete tree and non-forced expected-SHA update.
4. Continue independent work while unrelated or pending Actions run; do not wait idly. Current-source L3 remains NOT VERIFIED until exact-source XY-C1 MIPS/uClibc build, TRX verification and artifact upload pass. Physical router L4 remains NOT VERIFIED.

Source/validation notes:
- Base main SHA for this checker correction: `bda473715cf8bb2e1bbb8c704370639854f2a07c`; exact-head USB storage run #43683399235 FAILED at line 184 with a Python literal-brace `.format()` ValueError after the embedded caller-graph step passed.
- Immutable backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserve it. PR #1 remains closed/unmerged; AliDDNS HMAC implementation and core test files match main.
- Preserve archived dev-log shards and indexes byte-for-byte.
- Parent main: `fc675527e58402dfab0bd25d7ea5dc620861d872`; immutable backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact parent AI docs #43682722938 SUCCESS; USB storage #43682722908 FAILED only at stale inline function signature/return assertion. Earlier steps passed; checked-eject script was skipped. New exact-head Actions required.
- Preserve archived dev-log shards and indexes byte-for-byte.

