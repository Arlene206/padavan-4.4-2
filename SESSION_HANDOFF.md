# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Main at this checkpoint: `f0df8c0b2a9f9e07178e733e9945e6fa325b8f75`. Immutable backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (must remain untouched).
- Recent main commits (newest first): `f0df8c0b2a9f` test(rc): cover optional daemon stop postconditions; `51a9b516d188` fix(rc): verify optional daemon stops before reporting success; `d84637c86f01` ci(storage): restore complete push and pull request filters; `60b01dc472ff` ci(storage): repair workflow and assert tri-state daemon gates; `d86eaf91dc8d` test(storage): verify tri-state daemon lifecycle contracts; `6ad97a7e5967` fix(storage): distinguish unknown daemon inventories; `b6f9d1aaaa73` test(rc): assert checked telnet restart result merging; `52e9af13ad72` ci(rc): repair PID 1 workflow path filters; `a4935c5f4e5c` fix(rc): verify sshd stop and propagate restart status; `8b8da33fd9c4` fix(rc): gate telnetd restarts on successful shutdown; `f4d7967ede94` test(infosvr): include shared process inventory header; `a16ed39c359f` fix(rc): fail closed on unknown infosvr process state.
- This work extended the process-inventory UNKNOWN fail-closed policy into the remaining high-risk service transitions: networkmap now starts only on confirmed ABSENT and sends SIGUSR1 only after PID/start-time identity preflight and revalidation; infosvr replacement requires confirmed ABSENT; SSH restart verifies stop and absence before start and reconciles firewall on unknown transitions; telnetd restart merges checked stop/start results; storage-backed FTP/DLNA/iTunes/Transmission/Aria state uses tri-state inventory, and FTP stop verifies absence before safe removal; SCUT/MentoHUST/VLMCSd/iperf3 stop wrappers verify the process is actually absent and start results propagate.
- Exact-main Actions at `f0df8c0b2a9f9e07178e733e9945e6fa325b8f75`: `XY-C1 Firmware Validation` #43666310042 in_progress; `WAN gateway ARP probe socket stability regression` #43666310087 success; `VPN server lifecycle stability regression` #43666310138 success; `DDNSTO lifecycle stability regression` #43666310183 success; `watchdog signal lifecycle stability regression` #43666310126 success; `AI handoff documentation contract` #43666310150 success; `detect_link signal lifecycle stability regression` #43666310000 success; `Crond daemon lifecycle stability regression` #43666309991 success; `ZeroTier lifecycle stability regression` #43666310180 success; `DNS Forwarder launch path stability regression` #43666310155 success; `AliyunDrive WebDAV lifecycle stability regression` #43666310178 success; `infosvr packet and signal lifecycle stability regression` #43666310088 success; `GPIO button daemon startup and press timing stability regression` #43666310152 success; `networkmap signal lifecycle stability regression` #43666310027 success; `detect_internet signal lifecycle stability regression` #43666310137 success; `AdGuardHome config security regression` #43666310192 success; `HTTPD daemon lifecycle stability` #43666310034 success; `Adbyby download and dnsmasq configuration stability regression` #43666309974 success; `Process lifecycle inventory - f0df8c0b2a9f9e07178e733e9945e6fa325b8f75` #43666310148 success; `USB storage hotplug stability regression` #43666309989 failure; `FRP asynchronous startup lifecycle regression` #43666310162 success; `SkipDBv2 Regression` #43666310085 success; `AliDDNS signed API transport security regression` #43666310250 success; `HTTPD Authentication and Security Regression` #43666310117 success; `NVRAM transaction boundary audit` #43666309980 success; `.github/workflows/rc-notification-queue-stability.yml` #43666308732 failure.
- L3 target status: NOT VERIFIED unless the exact current-source XY-C1/MIPS/uClibc workflow, TRX verification, and artifact upload all pass on this source. Historical artifacts from older SHAs do not prove this tree. Physical L4 remains NOT VERIFIED (no router flash/boot/runtime evidence in this session).
- No local checkout or target cross-compiler execution is claimed. GitHub source/Actions are the only verification evidence used here. Legacy PR #1 remains CLOSED / UNMERGED; no feature branch or PR was opened.

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

At handoff commit parent, main is `f0df8c0b2a9f9e07178e733e9945e6fa325b8f75`; immutable backup is `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).

### Completed source slices
- Shutdown and firmware propagation: `fd108bf` captures LAN/WAN stop status and gates MTD persistence; `874bfbd` aggregates optional-service stop failures in firmware update and blocks replacement starts after failed stops; `189e532` propagates WAN/modem teardown failures through PID 1 notifications.
- `13447400`: networkmap tri-state start/restart plus snapshot PID/start-time preflight and targeted identity-revalidated SIGUSR1; no broad killall.
- Infosvr restart now refuses to start unless its stop succeeded and process inventory confirms ABSENT.
- SSH restart now uses tri-state inventory, refuses UNKNOWN/RUNNING after stop, propagates launcher errors, and conservatively reconciles firewall state.
- Telnetd manual and PID1 restart paths stop-before-start and merge stop/start status.
- Storage-backed FTP/DLNA/iTunes/Transmission/Aria use process-inventory tri-state; unknown state blocks startup/removal, FTP stop verifies process absence, and firewall reconciliation treats UNKNOWN as a change.
- SCUT/MentoHUST/VLMCSd/iperf3 status wrappers now use process inventory, stop commands are verified against actual daemon absence, and restart/start statuses propagate.
- Exact-main Actions at the checkpoint: - `XY-C1 Firmware Validation` #43666310042: **in_progress**
- `WAN gateway ARP probe socket stability regression` #43666310087: **success**
- `VPN server lifecycle stability regression` #43666310138: **success**
- `DDNSTO lifecycle stability regression` #43666310183: **success**
- `watchdog signal lifecycle stability regression` #43666310126: **success**
- `AI handoff documentation contract` #43666310150: **success**
- `detect_link signal lifecycle stability regression` #43666310000: **success**
- `Crond daemon lifecycle stability regression` #43666309991: **success**
- `ZeroTier lifecycle stability regression` #43666310180: **success**
- `DNS Forwarder launch path stability regression` #43666310155: **success**
- `AliyunDrive WebDAV lifecycle stability regression` #43666310178: **success**
- `infosvr packet and signal lifecycle stability regression` #43666310088: **success**
- `GPIO button daemon startup and press timing stability regression` #43666310152: **success**
- `networkmap signal lifecycle stability regression` #43666310027: **success**
- `detect_internet signal lifecycle stability regression` #43666310137: **success**
- `AdGuardHome config security regression` #43666310192: **success**
- `HTTPD daemon lifecycle stability` #43666310034: **success**
- `Adbyby download and dnsmasq configuration stability regression` #43666309974: **success**
- `Process lifecycle inventory - f0df8c0b2a9f9e07178e733e9945e6fa325b8f75` #43666310148: **success**
- `USB storage hotplug stability regression` #43666309989: **failure**
- `FRP asynchronous startup lifecycle regression` #43666310162: **success**
- `SkipDBv2 Regression` #43666310085: **success**
- `AliDDNS signed API transport security regression` #43666310250: **success**
- `HTTPD Authentication and Security Regression` #43666310117: **success**
- `NVRAM transaction boundary audit` #43666309980: **success**
- `.github/workflows/rc-notification-queue-stability.yml` #43666308732: **failure**. Any IN_PROGRESS run must be re-polled; inspect failing job logs before changing assertions. The source may be newer by the time the next GPT starts.
- Current source L3 and physical L4 are NOT VERIFIED unless the conditions above are met on the exact current source.

### Next work
1. Re-fetch live `main`, the immutable backup ref, and exact-head Actions. Do not repeat the startup protocol or completed source fixes.
2. Continue the P1 caller audit with `trunk/user/rc/src/net.c::set_passthrough_pppoe()`: a remaining name-based `pids("pppoe-relay")` existence check can conflate inventory UNKNOWN with ABSENT. Migrate to tri-state inventory; only confirmed ABSENT may launch, UNKNOWN must log and refuse.
3. Then inspect remaining direct `kill_pidfile_s()` and broad name-wide signals by semantic caller group. Preserve legacy helper semantics; do not globally redefine `pids()`.
4. Poll the exact-source XY-C1 MIPS/uClibc Build + TRX verification + artifact upload workflow. Physical L4 needs actual router evidence.

Each task: source change + production-connected regression + atomic commit + exact-SHA Actions. On FAIL inspect the failed job log; on PENDING continue independent work. No force-push.

