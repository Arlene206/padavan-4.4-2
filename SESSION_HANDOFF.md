# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for Transmission directory-preparation and cleanup-state slice: a391be7a60c790eafd480cc4a70923e8c9e5c26b.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Recent main commits:
- a391be7a60c7 fix(storage): gate Transmission launch on setup status
- a9b32e72faab test(storage): fix launcher fixture syntax boundary
- 5ae02f33db8c fix(storage): stage Transmission default settings safely
- b488a561bb91 fix(rc): preserve alternate udpxy build path and CI contracts
- 50e24f32f310 fix(net): publish MSD Lite config atomically
- f36d6ec94650 fix(storage): isolate Aria concurrent config staging
- 13a16ec70f7f fix(storage): isolate concurrent Aria config staging
- 6ad669a672f8 fix(storage): use unique Aria config staging file
- 755e8c711f1b fix(storage): stage Aria config before daemon launch
- eddf9aac4d44 fix(storage): propagate Transmission and Aria start failures
- 401066ebf04c fix(wan): declare MAN route helper before use
- b1b8991aaba1 fix(wan): abort startup on failed WISP migration
- 3f6ca487db97 test(wan): align route workflows with MAN status API
- 41ea56088a7c fix(wan): propagate required MAN gateway route failures
- 2529f9a8aadd fix(wan): guard AP-client WISP interface migration
- 7ebeaa9e02e6 fix(wan): gate protocol startup on interface readiness
- 20f1bbd6eb13 fix(wan): fail closed on static IPoE address error
- 454f3869fc3c test(wan): scope physical WAN abort ordering
- WAN firmware source SHA 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 exact XY-C1 #43756373315 Build/TRX/artifact SUCCESS (L3 for that exact C source SHA). No physical router/L4 result.
- WAN source work includes direct IPoE static IPv4 fail-closed address application, checked initial physical WAN bring-up, safe AP-client WISP cleanup and required MAN gateway/default-route error separation for static PPTP/L2TP. Optional configured static/DHCP/DNS routes remain aggregate-only; DHCP/ZeroConf aggregate semantics and PPPoE async/no-wait are preserved.
- Storage startup work: Aria uses PID-unique `aria2.conf.tmp.$`; its two-phase concurrent test uses `entered.$` and `checked.$`, exact storage #43763413862 PASS. Transmission settings use private `settings.json.tmp.$` staging with checked generator, transform, rename and nonempty postconditions; exact a9 storage #43764529973 PASS. Transmission required mkdir paths and watch/root/download chmod plus config/incomplete chown are now checked and fail closed before daemon start; exact a391 storage #43765111683 PASS. This follow-up initializes `CONFIG_TMP` before any possible early failure and tests that an inherited environment value is not removed by error cleanup.
- Exact SHA `a391be7a60c790eafd480cc4a70923e8c9e5c26b` push Actions all PASSED: storage #43765111683, AI docs #43765111687, Process inventory #43765111678, Process lifecycle #43765111691, infosvr #43765111706, AliDDNS #43765111677, AliyunDrive #43765111712, AdGuardHome #43765111669 and NVRAM #43765111698. Prior a9 storage #43764529973 also PASS.
- The 5ae storage run #43764336064 initially failed on a malformed test function boundary (`}assert_failure()`); a9b32e7 restored the newline and exact a9 storage then passed. No unresolved exact-head Action failures remain on a391.
- XY-C1 run #43762907958 for C-source SHA b488a561bb91df62aca810a6ff2c191c17f82b2b completed SUCCESS, including Build XY-C1, TRX output verification and artifact upload. L3 applies only to that exact C-source revision; no physical router/L4 result. Later df267, 5ae, a9 and a391 changes are shell/test/docs only.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 claim.

## Active task

### Harden Transmission early-failure cleanup state

Transmission settings publication and directory/permission setup now fail closed. Exact host + BusyBox storage regression passed on a391 (#43765111683). Review identified one cleanup-state edge: `transmission_start_fail()` removes the path in `CONFIG_TMP`, but before this follow-up that variable was not initialized until after directory setup. A caller-inherited `CONFIG_TMP` could therefore be mistaken for the temporary file and deleted if an early mkdir/chmod failed.

This slice initializes `CONFIG_TMP` as empty at the start of `func_start()` and adds a regression that injects a config-directory mkdir failure while supplying an inherited `CONFIG_TMP` file; the file must remain untouched and the daemon must not be invoked.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in start_udpxy() previously truncated /etc/storage/msd_lite.conf before it opened /etc_ro/msd_lite.conf, and it ignored read/write/flush/close failures before enabling and launching the daemon. This slice replaces direct truncation with same-directory mkstemp staging, regular-file/source checks, mode preservation, input validation, checked read/write/flush/fsync/close and atomic rename. Publication failure logs and returns before setting msd_lite_enable or invoking msd_lite.

Changed: trunk/user/rc/src/net.c; new trunk/user/rc/src/msd_lite_config_file.h and tests/msd_lite_config_file.c; Process lifecycle and XY-C1 workflow path filters; scripts/check_process_lifecycle_integration.py. Local GNU89 ASan/UBSan regression PASS, including missing source/parent, invalid interface/port, directory/symlink destination, last-good preservation, mode preservation, RLIMIT_FSIZE write failure and temp cleanup.

The exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow for this source change are pending. Do not transfer the earlier 401066 L3 result to this new C source.

## Verification boundary

- Parent main for current cleanup-state fix: a391be7a60c790eafd480cc4a70923e8c9e5c26b.
- Aria PID uniqueness fix: `f36d6ec94650103134a55b2e9c9abca9894de075`; barrier race follow-up: `b488a561bb91df62aca810a6ff2c191c17f82b2b`; literal `$` barrier marker fix: `df267fd1457bb62e2a0c0fb0c359d3aa153f10e9`. Exact df267 storage #43763413862 PASS.
- MSD Lite source slice from 50e24f32f310998a71e976287981aeb45f3bcd99 adds checked staged config publication, regression, workflow filters and source contract; b488a561 restores non-APP_MSD_LITE udpxy fallback. Exact Process + XY-C1 checks on b488 passed.
- Local host command: cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I trunk/user/rc/src trunk/user/rc/src/tests/msd_lite_config_file.c -o msd-lite-config-regression; ASAN_OPTIONS=detect_leaks=1 ./msd-lite-config-regression — PASS.
- The b488 source SHA got exact XY-C1 cross-build PASS (#43762907958); L3 applies only to that source revision. No physical-router/L4 result.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; archive index/shards untouched.

## Exact first action for the next GPT

1. Inspect this cleanup-state fix's exact-head storage workflow; require host shell and firmware BusyBox ash PASS for all Transmission config-publication, mkdir, chmod, chown and inherited-`CONFIG_TMP` cases plus the Aria concurrency/fault-injection cases.
2. After storage PASS, audit Transmission stop/restart postconditions and failure status (killall, timeout, forced kill, final pidof) with regression tests.
3. Continue broader Padavan user-space stability/security work; do not repeat completed audits.
4. Preserve the frozen backup and archived dev-log index/shards. b488's exact C-source L3 remains revision-specific; no L4 without physical-router evidence.
