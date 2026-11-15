# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for the IGMP regression baseline correction: e3499afc01bdc42cd1620eb18a6eec0e20541d36.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Exact parent e349 Process lifecycle run #43770121710 FAILED in the IGMP config regression because the newly added last-good token-validation fixture changed the preserved file contents but the following older failure case still expected the prior fixture string. The validator itself compiled and the run reached the assertion after the new cases.
- Exact parent XY-C1 #43770121908 and NVRAM #43770121752 were still in progress at last inspection. AI docs #43770121750, inventory #43770121725, AdGuardHome #43770121883, infosvr #43770121820, AliyunDrive #43770121777 and AliDDNS #43770121720 passed.
- THIS COMMIT restores the original `last-good-configuration` fixture after the new whitespace/comment rejection cases, keeping the independent pre-existing missing-parent/rename assertions unchanged.
- IGMP config validation now emits only single safe tokens: interface fields reject whitespace/control/comment chars; altnet allows only digits/dots/slashes. No production code change in this correction.

## Active task

### Verify IGMP proxy token validation after fixture correction

The exact e349 Process lifecycle run #43770121710 failed in the IGMP helper regression after the newly added token-injection tests: those tests changed the fixture file contents to `last-good-token-validation`, but the following existing missing-parent failure test still asserted the older `last-good-configuration` contents. The validator compiled and the run reached this assertion; the failure is test fixture sequencing, not a production parser/build error.

THIS COMMIT restores the original last-good fixture before the older missing-parent/rename/flush-failure tests. Exact Process lifecycle regression and XY-C1 Build/TRX/artifact checks on this corrected SHA are required.

Next task: continue the Aria launcher stop lifecycle audit (same multi-PID, forced-kill postcondition and restart-gate pattern previously corrected in Transmission), then wider outstanding P1 service lifecycle work.

## Completed source slice — Transmission launcher lifecycle

Transmission's shell stop now validates each PID independently before SIGKILL, confirms daemon absence after the forced path and gates restart on successful stop. The PID-unique config stage and ownership-aware cleanup remain. Exact abd40 USB storage #43769497238 PASS across host, BusyBox 1.24.2 ash and full storage checks. The exact-head XY-C1 workflow was pending at last inspection; no L3 for this SHA yet.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: `e3499afc01bdc42cd1620eb18a6eec0e20541d36`; this is a one-file regression fixture correction with an atomic handoff/roadmap/log update.
- Exact e349 Process lifecycle #43770121710 FAIL only at the IGMP regression's later last-good assertion: the inserted new tests had set another expected file value and did not restore the fixture before legacy failure-path assertions.
- Exact e349 AI docs #43770121750, Process inventory #43770121725, AdGuardHome #43770121883, infosvr #43770121820, AliyunDrive #43770121777 and AliDDNS #43770121720 PASS. NVRAM #43770121752 and XY-C1 #43770121908 were in progress at last poll.
- Exact abd40 storage #43769497238 PASS, including host + BusyBox 1.24.2 ash launcher tests; that result predates the IGMP-only changes.
- No new exact-head Process lifecycle/XY-C1 result yet; no L3/L4 claim. Frozen backup unchanged.

## Exact first action for the next GPT

1. Check exact-head Process lifecycle regression. Confirm IGMP token tests pass and the original missing-parent, directory-target and RLIMIT_FSIZE failure cases still prove last-good preservation.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload; L3 only if all phases pass on this SHA.
3. Then repair Aria `func_stop()` multi-PID signaling, final process-absence verification and restart gating with host + firmware BusyBox ash regressions.
4. Preserve frozen backup and archive shards; no L4 without physical-router evidence.

