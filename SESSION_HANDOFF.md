# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for Transmission fail-closed settings slice: df267fd1457bb62e2a0c0fb0c359d3aa153f10e9.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Recent main commits:
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
- Storage startup work: Transmission and Aria return actual start-stop-daemon status independently from logger. Aria checks directory/session/download preparation, permissions/ownership and default-config write/rename before launch; its config stage is `aria2.conf.tmp.$$`. This commit adds Transmission PID-qualified same-directory `settings.json.tmp.$$` staging, exclusive private creation, checked generator/transform/rename, nonempty-file gates, and cleanup on failure; host/BusyBox fault-injection CI is pending for this exact SHA. Other Transmission directory/permission setup commands remain a separate open audit.
- Exact SHA `df267fd1457bb62e2a0c0fb0c359d3aa153f10e9` push Actions all PASSED: storage #43763413862, AI docs #43763413814, Process inventory #43763413812, Process lifecycle #43763413775, infosvr #43763413761, AliDDNS #43763413799, AliyunDrive #43763413767, AdGuardHome #43763413821 and NVRAM #43763413864.
- Earlier SHA 50e24f32f310998a71e976287981aeb45f3bcd99 failures AI docs #43762316097, infosvr #43762316266 and Process lifecycle #43762316246 were repaired in b488a561 and all respective exact df267 Actions now PASS.
- XY-C1 run #43762907958 for exact C-source SHA b488a561bb91df62aca810a6ff2c191c17f82b2b completed SUCCESS (cross-build job PASS). This establishes L3 for that exact C-source revision; no physical router/L4 result. The later df267 change is shell/test/docs only.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 claim.

## Active task

### Implement fail-closed Transmission default settings publication

Aria's production temp suffix and the test's two-phase barrier are now PID-qualified; exact-head storage #43763413862 passes on df267. The handoff literal checker, infosvr heading contract, and alternate udpxy feature-path regression are also closed (all df267 push workflows PASS).

Aria's source and two-phase test markers are now PID-unique (`aria2.conf.tmp.$`, `entered.$`, `checked.$`); exact df267 storage run #43763413862 passed.

The prior b488/df267 repairs also restored the literal AI/docs and infosvr heading contracts and the alternate `/usr/sbin/udpxy` non-MSD-Lite build path; all current exact-head checks are green. the handoff adds the required `Before context exhaustion` phrase and restores the exact singular heading `## Exact first action for the next GPT`; `start_udpxy()` restores the legacy `/usr/sbin/udpxy` `#else` branch for firmware builds without `APP_MSD_LITE`. The process lifecycle checker had caught that real feature-path regression after the MSD Lite refactor.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in start_udpxy() previously truncated /etc/storage/msd_lite.conf before it opened /etc_ro/msd_lite.conf, and it ignored read/write/flush/close failures before enabling and launching the daemon. This slice replaces direct truncation with same-directory mkstemp staging, regular-file/source checks, mode preservation, input validation, checked read/write/flush/fsync/close and atomic rename. Publication failure logs and returns before setting msd_lite_enable or invoking msd_lite.

Changed: trunk/user/rc/src/net.c; new trunk/user/rc/src/msd_lite_config_file.h and tests/msd_lite_config_file.c; Process lifecycle and XY-C1 workflow path filters; scripts/check_process_lifecycle_integration.py. Local GNU89 ASan/UBSan regression PASS, including missing source/parent, invalid interface/port, directory/symlink destination, last-good preservation, mode preservation, RLIMIT_FSIZE write failure and temp cleanup.

The exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow for this source change are pending. Do not transfer the earlier 401066 L3 result to this new C source.

## Verification boundary

- Parent main for Transmission slice: df267fd1457bb62e2a0c0fb0c359d3aa153f10e9.
- Aria PID uniqueness fix: `f36d6ec94650103134a55b2e9c9abca9894de075`; barrier race follow-up: `b488a561bb91df62aca810a6ff2c191c17f82b2b`; literal `$` barrier marker fix: `df267fd1457bb62e2a0c0fb0c359d3aa153f10e9`. Exact df267 storage #43763413862 PASS.
- MSD Lite source slice from 50e24f32f310998a71e976287981aeb45f3bcd99 adds checked staged config publication, regression, workflow filters and source contract; b488a561 restores non-APP_MSD_LITE udpxy fallback. Exact Process + XY-C1 checks on b488 passed.
- Local host command: cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I trunk/user/rc/src trunk/user/rc/src/tests/msd_lite_config_file.c -o msd-lite-config-regression; ASAN_OPTIONS=detect_leaks=1 ./msd-lite-config-regression — PASS.
- The b488 source SHA got exact XY-C1 cross-build PASS (#43762907958); L3 applies only to that source revision. No physical-router/L4 result.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; archive index/shards untouched.

## Exact first action for the next GPT

1. Inspect this follow-up's exact-head storage Actions and the preceding b488 exact-head AI docs, infosvr and Process lifecycle checks; confirm PID-qualified `entered.$$` and `checked.$$` rendezvous markers. Repair the first causal failure, not just the symptom.
2. Check XY-C1 Build/TRX/artifact status for the MSD Lite source; claim L3 only if that exact C-source SHA has all three phases PASS.
3. After the storage and required source contracts pass, implement Transmission's checked PID-unique same-directory `settings.json` publication with generator/transform/rename fault injection under host shell and BusyBox ash.
4. Continue broader user-space stability/security work without repeating already-closed audits.
5. Preserve frozen backup and archive index/shards; do not claim L4 without physical-router evidence.
