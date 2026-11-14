# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Only `main` is writable. The latest product-source state before this workflow-contract slice is `946a8719c2ee01a49712740d513528799bcb3047`; the test-only/current tip before publication was `b98bbbb47cb4c8298f50bf5c6ee318a8baf1ead0`. XY-C1 run `43647325231` is PENDING on exact SHA b98bbbb; HTTPD rerun `43647325290` is PENDING after correction of its stale WAN stop-gate assertion. This commit adds missing MIPS build path filters and a regression guard and therefore triggers a new exact-head XY-C1 run. The last established L3 baseline remains historical SHA `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c` (run `43625410221`, artifact `14125764801`); do not claim L3 for newer source until exact-head build, TRX verification and artifact upload pass. L4 physical router validation remains NOT VERIFIED. The preserved backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and was verified read-only. Fetch live main before every write.
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

### XY-C1 MIPS/uClibc target build — latest source validation PENDING

- Historical L3 is verified on product-source baseline `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c`: XY-C1 run [43625410221](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43625410221) passed Build XY-C1, TRX verification and artifact upload; artifact [14125764801](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43625410221/artifacts/14125764801), 12,583,902 bytes. Newer production changes through `946a8719` are not yet L3-verified. XY-C1 run [43647325231](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43647325231) is PENDING on exact SHA `b98bbbb47cb4c8298f50bf5c6ee318a8baf1ead0`; this workflow-filter correction triggers a new target run. Do not claim L3 until Build XY-C1, TRX verification and artifact upload all pass; this remains distinct from L4 hardware validation.
- The rstats compile defect is fixed; both dd7a's follow-up build and the watchdog-inclusive current-main build passed. The f5cc target build validates the integrated RC source at L3.
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
- L2: all ten exact-head `push` workflows triggered by product-source SHA `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c` passed: XY-C1 [43625410221], AI docs [43625410189], watchdog [43625410193], process lifecycle [43625410154], process inventory [43625410226], NVRAM [43625410190], AdGuardHome [43625410170], AliDDNS [43625410262], AliyunDrive [43625410196] and infosvr [43625410180].
- L3: VERIFIED on product-source baseline `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c`; XY-C1 run [43625410221] passed firmware build, TRX verification and artifact upload. No L4 hardware validation is claimed.
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the immutable backup ref is unchanged. Inspect exact-head Actions for the live SHA, especially the XY-C1 run triggered by this workflow-filter commit, the process-lifecycle regression (now checking XY path coverage), and HTTPD run `43647325290` (the 946a failure was a stale assertion; b98 corrected it). At handoff preparation, XY run `43647325231` on exact SHA `b98bbbb47cb4c8298f50bf5c6ee318a8baf1ead0` and HTTPD run `43647325290` were PENDING; AI-doc run `43647325253` passed on b98. Process inventory/lifecycle, iNIC signal and wireless radio runs `43647169443`, `43647169424` and `43647169416` passed on exact SHA `946a8719c2ee01a49712740d513528799bcb3047`. Do not treat PENDING as a stop condition; repair exact-head failures and continue independent work.
2. Continue P1 implementation from remaining caller groups, not the completed PPP/udhcpc/iNICD identity publication or VPN/WAN/LAN/USB modem stop gates. Next inspect firmware `stop_services()`/flash and graceful-shutdown propagation, then Samba/storage/NVRAM postconditions. Add focused regressions for UNKNOWN/CHANGED/FAILED before destructive follow-up.
3. Keep `pids()` Boolean-compatible until all 77 direct callers are migrated/adapted. Keep `kill_pidfile_s()` semantics unchanged until its five direct callers and all 15 call expressions are migrated/adapted. BusyBox's independent `procps.c` implementation stays separate.
4. Linux 4.4 PID/start_time validation retains a check/kill TOCTOU window; do not claim pidfd-equivalent atomic signaling or L4 hardware validation without evidence.
5. Continue one complete Git tree/commit with a non-forced expected-SHA main update for every multi-file slice. No per-file commits, force pushes, SHA-refresh commits or standalone CI-poll commits.
