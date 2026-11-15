# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Current main source SHA before this test correction: 6052c4aaad370b67f529485b8effa14350435f2b. Samba process-inventory slice introduced `samba_process_policy.h` and production use in `services_stor.c::run_samba()`; the first exact-head Samba CI run [43659761190] failed because the EPERM fake supplied only two RUNNING validations instead of the required four (two identities × preflight + signal-time revalidation). This commit corrects that test fixture and restores the complete root development log from 525c046 (the previous handoff mistakenly replaced its history with a single entry). The follow-up `reload_nmbd()` source change was not published; do not assume it exists on main. Next inspect exact-head Samba + AI docs Actions, then continue WINS start/restart migrations. DNSMasq lifecycle exact-head PASS on 9d816 [43657650362]. L3 and physical L4 remain NOT VERIFIED. Immutable backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Immutable archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Read-only; never modify it.
- **Permanent user entrypoint:** `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. The user does not need a long prompt, previous chat, or repository upload. Never output a recursive next-GPT prompt in chat. Git documents and live GitHub state are the project memory.
- Recovery is read-only by default: read complete `AI_DEVELOPMENT_PROTOCOL.md` and `AI_BOOTSTRAP_PROMPT.md`; fetch live main + immutable backup; inspect exact-head Actions; then read this handoff, roadmap, newest 20 dev-log records, `docs/dev-log/INDEX.md`, and the semantic audit report for the active caller group. Do not create a commit to refresh SHA fields, record a takeover, or report a CI poll.
- Legacy PR #1 remains CLOSED / UNMERGED. Do not reopen it or use its head. No feature branches or PRs. Do not modify GPT Session Rotator.
- L1 lexical inventory and direct API semantic review are complete within their documented scope. Reuse `docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md`; do not repeat the 9,439 tracked-blob scan, 6,193 readable-file scan, 77 `pids()` census or 46 `kill_services()` census.

## Active task

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

### XY-C1 MIPS/uClibc target build — VERIFIED on current product-source baseline

- Latest product-source baseline: `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286`. Exact-head XY-C1 push [43648229489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43648229489) passed the complete MIPS/uClibc build, TRX output verification and artifact upload. Artifact [14135711084](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43648229489): 12,587,100 bytes; SHA-256 `bda83c9e71ed01a8708ad90a5104535e972bd30424bdf03ed24065b3a1a9407e`.
- The subsequent `7697c6fd` commit changed only handoff documentation. This checker correction also changes no firmware product source; L3 remains VERIFIED for the current product-source baseline `cc61cb5`. Any later production-source change requires a new exact-head target build before claiming current L3.
- L4 remains NOT VERIFIED; no physical XY-C1 flash/boot/runtime claim.

## Explicitly uncompleted P1 work

- All 77 direct `pids()` callers remain legacy Boolean users. Migrate by semantic group; never change `pids()` to negative UNKNOWN until every caller is explicit.
- LPD PID publication uses exclusive same-directory temporary-file creation and atomic rename after a complete checked write; its strict identity-aware duplicate consumer treats malformed/unreadable records as UNKNOWN. Publication follows successful socket bind; stop does not unlink because that could delete a replacement daemon's newly published record. Shared identity-bound PID publication/signaling now covers detect_internet, detect_link, rstats, watchdog, PPPD base/link daemons, BusyBox udhcpc and INICD. Stop gates also cover VPN client/server, WAN DHCP, AP-LAN DHCP and USB modem teardown. Remaining legacy process-presence callers and broader system shutdown/storage/Samba/NVRAM postconditions remain open.
- Upper-layer stop propagation still needs extension/audit beyond the completed VPN, USB-modem, WAN and AP-LAN gates: firmware `stop_services()`/flash, graceful shutdown sequencing, Samba config/daemon publication, storage eject/unmount and NVRAM stopped-state publication.
- Continue migrating remaining direct `pids()` caller groups and `kill_pidfile_s()` use sites; audit direct `killall` and signal paths outside `kill_services()`. PPP, udhcpc and iNICD identity producers plus current VPN/WAN/LAN/USB stop gates are already implemented; do not repeat those slices.
- Linux 4.4 has no `pidfd_send_signal()`; PID/start_time validation leaves a check/kill TOCTOU window. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and was not changed.

## Verification boundary

- L1: prior lexical inventory and direct-call semantic review PASS within the recorded scope; do not repeat completed inventories.
- L2: latest source-level exact-head evidence includes process inventory [43648229415], PID1 [43648229463], DNS Forwarder [43648229531], storage hotplug [43648229477], Samba [43648229471], NFS [43648229491], HTTPD [43648229544], WAN route [43648229405], AdGuardHome [43648229513], watchdog [43648229404], AliDDNS [43648229509], infosvr [43648229453], VPN/other module checks as recorded in the current-main Actions. On the follow-up docs-only head `7697c6f`, VPN [43648923434], AI docs [43648923326], process inventory [43648923376], AdGuardHome [43648923421], AliDDNS [43648923338], infosvr [43648923306] and AliyunDrive [43648923366] passed. Networkmap [43648923325] failed only on the incorrect checker prototype assertion; this commit fixes it. NVRAM [43648923357] was PENDING/IN_PROGRESS on `7697c6f`.
- L3: Current production source is NOT VERIFIED. XY-C1 [43654318351] on ea2163e failed at RC final link because disk_inventory_parse_decimal() introduced variable 64-bit division and a hidden __udivdi3 dependency in libdisk.so. This commit removes all division using a constant ULLONG_MAX cutoff and adds max-value parsing coverage. A new exact-head XY-C1 Build, TRX verification and artifact upload must pass before L3 can be claimed.
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub.

## HTTPD daemon lifecycle — current slice

- `start_httpd()` now uses `process_inventory_scan("httpd", ...)` with tri-state `ABSENT/RUNNING/UNKNOWN` semantics. Only an authoritative ABSENT result permits NVRAM configuration normalization, stale-state clearing, launcher execution, or firewall publication.
- A confirmed RUNNING daemon returns `EBUSY` and may publish `httpd_started=1`; UNKNOWN returns `EIO` (or scan errno) without mutating configuration or claiming the daemon is running.
- Detached-child readiness now requires RUNNING. ABSENT retries then returns `ESRCH`; UNKNOWN terminates the probe with an error and never publishes started state. GNU89 regression covers RUNNING, ABSENT, UNKNOWN, invalid states and preserved launcher errors.
- The first exact-head lifecycle Action exposed a stale source-contract assertion expecting the old `status == 0` form. The checker was corrected to assert the new fail-before-publish branch. Current exact run state is listed in the repository checkpoint above.

## Storage eject propagation — current committed slice

- `read_disk_data_checked()` now strictly validates `/proc/partitions`: canonical four-column header, blank separator, complete newline-terminated records, exactly four fields, unsigned decimal major/minor/block counts, and overflow-safe conversion. Malformed, signed, truncated, overlong, or overflowed records fail closed; the legacy `read_disk_data()` pointer wrapper is preserved.
- `safe_remove_all_stor_devices()` distinguishes unknown inventory from valid empty inventory, checks the per-device path and generic fallback, and requires a complete final mount snapshot to prove zero storage mounts. Firmware update does not persist storage MTD or execute `mtd_write` when eject is incomplete; normal reboot/halt proceeds but skips storage MTD persistence on failure.
- Parser regression is compiled as GNU89 with `-Wall -Wextra -Werror`. The exact c5 parser Action initially failed earlier in the job on BusyBox `writev` missing its declaration; `b022942` added `<sys/uio.h>`, after which the storage-hotplug exact-head run is represented in the latest status block above. Do not treat prior SHA results as evidence for later source.
- Backup branch remains immutable; physical-router L4 remains unverified.

## Exact first action for the next GPT

Main parent before this DNSMasq identity-bound cache-reload fix: 525c046d658ec4e3de0fd62f25c133dec603bbd9.
Immutable backup: backup/pre-plugin-overhaul-2026-11-14 = 72f4f157a084f8184b9597ca5bf334b8080e5807 (read-only).

### Completed in recent source slices
- 3ca4126: strict disk inventory decimal parser no longer uses 64-bit division; UINT_MAX and ULLONG_MAX boundary cases added.
- 322c17f: DNSMasq start preflight/restart post-stop gates distinguish RUNNING/ABSENT/UNKNOWN.
- 09268ce: WAN DHCP renewal is authorized only on confirmed RUNNING; UNKNOWN propagates scan errno and no SIGUSR1 is sent. Exact Process lifecycle Action [43657031251] on 525c046 PASSED after fixing the checker delimiter follow-up in 525c046.
- This slice: restart_dns() uses captured DNSMasq process identities and sends targeted SIGHUP only after full preflight and immediate per-PID revalidation; UNKNOWN and signal errors fail closed.

### Verification boundary before this commit
- Exact 525c046: Process lifecycle [43657031251], AliDDNS [43657031278], infosvr [43657031274], AdGuardHome [43657031239], AliyunDrive [43657031284], AI docs [43657031271] and NVRAM [43657031315] PASSED.
- Exact 09268ce XY-C1 [43656932522] remained IN_PROGRESS at Build XY-C1; 3ca XY-C1 [43656299052] also remained IN_PROGRESS. No TRX/artifact result was available at last poll.
- Exact 322c17f DNSMasq lifecycle [43656500091] PASSED.
- Current-source L3 is NOT VERIFIED until exact-source Build XY-C1, TRX verification and artifact upload pass. Physical L4 remains NOT VERIFIED.

### Next work
1. Verify exact-head Samba share-stability and AI documentation Actions for this correction commit; inspect and resolve any remaining failed steps.
2. Migrate `services_ex.c::start_wins()` from raw `pids()` and name-wide SIGHUP to a complete nmbd snapshot, fail-closed UNKNOWN before config publication, identity-bound targeted reload, and a fresh ABSENT check before launch.
3. Migrate `restart_nmbd()`: snapshot smbd/nmbd before config/stop, abort on UNKNOWN, require confirmed ABSENT before relaunch, and signal only the captured smbd identities.
4. Finish `services_stor.c` fastpath and `restart_smbd()` postconditions, optional wsdd2 handling, then Samba stop/shutdown/flash/NVRAM lifecycle work.
5. L3 needs the exact product-source SHA to pass Build, TRX verification and artifact upload. L4 physical XY-C1 flash/boot/runtime remains NOT VERIFIED.

Re-fetch live main before every write; do not redo bootstrap.
