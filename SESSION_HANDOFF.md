# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Source checkpoint on main: `THIS_COMMIT_SHA`. Immutable backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only, unchanged).
- Recent main commits (newest first): `a90014886d87` fix(rc): normalize invalid SSH process states without changing daemon selection; `5e072e268065` fix(storage): verify all storage daemon stop postconditions; `f22a3cc0b297` fix(rc): revalidate networkmap absence before every start; `43fa094a8a45` fix(rc): route telnet manual restart through guarded start; `215f49668066` fix(net): signal the owned DHCP client by PID identity; `d861bf83f3d2` fix(rc): require confirmed absence before direct infosvr start; `9114b4c86e7d` fix(rc): gate direct SSH starts on confirmed process absence; `5fff9f220e29` fix(rc): verify telnetd process state before starting; `675862ab6f75` fix(rc): gate optional daemon starts on confirmed absence; `55917ae96056` test(ci): correct daemon source contract assertions.
- Lifecycle fixes now cover firmware/shutdown and WAN/modem stop propagation; networkmap tri-state launch and PID/start-time-bound refresh signalling; infosvr and SSH checked stop-before-start; telnetd stop postcondition and guarded direct/manual/PID1 start; storage-backed FTP/DLNA/iTunes/Transmission/Aria tri-state and post-stop checks; optional SCUT/MentoHUST/VLMCSd/iperf3 direct-start gates and stop postconditions; PPPoE relay start only on confirmed ABSENT.
- Current source guards:
  - networkmap: needs review
  - infosvr: needs review
  - sshd: needs review
  - telnet: present
  - storageSamba: needs review
- PPPoE relay gate: not verified in the current source and remains next priority.
- Historical exact-head Actions for source SHA `a90014886d87c8fcf8cf09a16f2322240e75e265` (do not carry results forward): watchdog signal lifecycle stability regression #43668416071: success; NVRAM transaction boundary audit #43668416001: in_progress; GPIO button daemon startup and press timing stability regression #43668416147: success; AdGuardHome config security regression #43668416084: success; Adbyby download and dnsmasq configuration stability regression #43668416078: success; USB storage hotplug stability regression #43668416119: in_progress; HTTPD daemon lifecycle stability #43668416103: success; infosvr packet and signal lifecycle stability regression #43668416183: success; VPN server lifecycle stability regression #43668416187: success; detect_link signal lifecycle stability regression #43668416017: success; WAN gateway ARP probe socket stability regression #43668416082: success; ZeroTier lifecycle stability regression #43668416038: success; XY-C1 Firmware Validation #43668416135: in_progress; DDNSTO lifecycle stability regression #43668416051: success; AliyunDrive WebDAV lifecycle stability regression #43668416137: in_progress; networkmap signal lifecycle stability regression #43668416118: failure; AI handoff documentation contract #43668416061: success; DNS Forwarder launch path stability regression #43668416176: success; Crond daemon lifecycle stability regression #43668416108: success; HTTPD Authentication and Security Regression #43668416021: success; AliDDNS signed API transport security regression #43668416132: success; PID 1 notification queue stability regression #43668416148: success; Process lifecycle inventory - a90014886d87c8fcf8cf09a16f2322240e75e265 #43668416168: success; detect_internet signal lifecycle stability regression #43668416090: success; SkipDBv2 Regression #43668416166: success; FRP asynchronous startup lifecycle regression #43668416052: success
- Networkmap Action `43668416118` failed only in the final source checker: both GNU89/ASan/UBSan policy binaries passed, but a stale assertion expected enum literals directly in the caller. The checker now validates the RUNNING branch and ABSENT-only helper in `THIS_COMMIT_SHA`; poll the newly triggered exact-head run.
- L3 is NOT VERIFIED until an exact-SHA XY-C1 MIPS/uClibc build, TRX verification, and artifact upload all pass. Physical L4 is NOT VERIFIED without actual router flash/boot/runtime evidence.
- No local cross-compile or physical router test is claimed. Evidence used here is GitHub source and Actions. Legacy PR #1 remains CLOSED / UNMERGED; no feature branch or PR was opened.

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

### XY-C1 MIPS/uClibc target build — historical baseline verified; current source pending

- Last fully verified product-source baseline: `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286`, exact-head XY-C1 [43648229489] passed MIPS/uClibc build, TRX verification and artifact upload [14135711084] (12,587,100 bytes; SHA-256 `bda83c9e71ed01a8708ad90a5104535e972bd30424bdf03ed24065b3a1a9407e`).
- Exact XY-C1 [43660915762] for product-source commit `f4e78554c490c28697fc7819de8ba4947ab6eb48` was IN_PROGRESS at last poll; it includes Samba WINS identity/restart and complete-ABSENT postcondition changes. This workflow/doc-only follow-up does not alter product source. L3 remains NOT VERIFIED until that run's Build, TRX check and artifact upload pass.
- Physical XY-C1 flash/boot/runtime L4 remains NOT VERIFIED.


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

Re-fetch live `main`, immutable backup, and exact-head Actions. Do not repeat the startup protocol or already-completed fixes. Source checkpoint before this handoff commit: `THIS_COMMIT_SHA`; immutable backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.

### Completed
- Firmware/shutdown stop propagation: `fd108bf` gates MTD persistence on LAN/WAN stop success; `874bfbd` aggregates optional-service stop failures and aborts firmware update before flash; `189e532` propagates WAN/modem stop failures through PID 1.
- `13447400`: networkmap tri-state and PID/start-time identity-bound SIGUSR1, with preflight and revalidation; no broad name-wide signal.
- Infosvr/SSH direct starts and restart paths require confirmed process absence; SSH status API preserves UNKNOWN, stop verifies postcondition and firewall reconciliation is conservative.
- Telnetd manual and PID1 paths are stop-before-start; stop verifies process absence; manual restart delegates to the same guarded start path.
- Storage-backed FTP/DLNA/iTunes/Transmission/Aria preserve UNKNOWN and fail closed; storage daemon stops verify absence, including Samba where the live source has the matching process set.
- SCUT/MentoHUST/VLMCSd/iperf3 start gates require ABSENT; stop wrappers verify post-stop absence and propagate launcher statuses.
- PPPoE relay only starts when the process inventory confirms ABSENT; UNKNOWN/RUNNING are rejected. A dedicated GNU89 policy regression and Actions workflow are present.
- Exact-head Actions at the source checkpoint: watchdog signal lifecycle stability regression #43668416071: success; NVRAM transaction boundary audit #43668416001: in_progress; GPIO button daemon startup and press timing stability regression #43668416147: success; AdGuardHome config security regression #43668416084: success; Adbyby download and dnsmasq configuration stability regression #43668416078: success; USB storage hotplug stability regression #43668416119: in_progress; HTTPD daemon lifecycle stability #43668416103: success; infosvr packet and signal lifecycle stability regression #43668416183: success; VPN server lifecycle stability regression #43668416187: success; detect_link signal lifecycle stability regression #43668416017: success; WAN gateway ARP probe socket stability regression #43668416082: success; ZeroTier lifecycle stability regression #43668416038: success; XY-C1 Firmware Validation #43668416135: in_progress; DDNSTO lifecycle stability regression #43668416051: success; AliyunDrive WebDAV lifecycle stability regression #43668416137: in_progress; networkmap signal lifecycle stability regression #43668416118: failure; AI handoff documentation contract #43668416061: success; DNS Forwarder launch path stability regression #43668416176: success; Crond daemon lifecycle stability regression #43668416108: success; HTTPD Authentication and Security Regression #43668416021: success; AliDDNS signed API transport security regression #43668416132: success; PID 1 notification queue stability regression #43668416148: success; Process lifecycle inventory - a90014886d87c8fcf8cf09a16f2322240e75e265 #43668416168: success; detect_internet signal lifecycle stability regression #43668416090: success; SkipDBv2 Regression #43668416166: success; FRP asynchronous startup lifecycle regression #43668416052: success. Re-poll pending runs; inspect failure job logs before touching checker assertions.
- L3/L4 remain NOT VERIFIED until exact source/hardware evidence satisfies the requirements in Current repository state.

### Next work
1. Poll the exact-head networkmap regression triggered by `THIS_COMMIT_SHA`; inspect and fix any remaining failing step.
2. Continue a focused audit of remaining `pids()`, `kill_pidfile_s()`, `killall`, and broad name-based signal callsites in `rc.c`, `net.c`, `services.c`, and hotplug code. Classify each caller; never globally redefine `pids()`.
3. Prioritize stop wrappers where command success can be reported while a daemon remains, and start wrappers that can launch on UNKNOWN. Migrate only semantically justified sites to process-inventory tri-state.
4. For each fix, add a production-connected regression, commit one atomic tree with expected-SHA non-force update, and inspect exact-SHA Actions. On PENDING, continue independent work.

