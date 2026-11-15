# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for current barrier-marker follow-up: b488a561bb91df62aca810a6ff2c191c17f82b2b.
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
- Storage startup work: Transmission and Aria return actual start-stop-daemon status independently from logger. Aria checks directory/session/download preparation, permissions/ownership, and default-config write/rename before launch; its PID-qualified config temp is now `aria2.conf.tmp.$`. Transmission default `settings.json` generation is NOT yet staged or fail-closed: live source still ignores generator, `sed`, and `mv` failures. This remains the next storage reliability implementation after current exact-head regressions are green.
- Exact `50e24f32f310998a71e976287981aeb45f3bcd99` push Actions: storage #43762316227 SUCCESS; AliDDNS #43762316320 SUCCESS; AliyunDrive #43762316276 SUCCESS; WAN route #43762316273 SUCCESS; AdGuardHome #43762316224 SUCCESS; HTTPD #43762316127 SUCCESS; NVRAM #43762316105 SUCCESS; Process inventory #43762316049 SUCCESS.
- Exact-head failures requiring repair: AI handoff docs #43762316097 FAIL because `Before context exhaustion` was absent; infosvr #43762316266 FAIL because the handoff changed its required heading from singular to plural (all compiled packet/signal tests passed); Process lifecycle #43762316246 FAIL in the integration checker because the non-`APP_MSD_LITE` `/usr/sbin/udpxy` branch was removed from `start_udpxy()`.
- Exact-head XY-C1 build #43762316142 was `in_progress` at last check; no L3 claim for this source SHA until Build/TRX/artifact completion. No L4 claim.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 claim.

## Active task

### Close Aria test race, handoff contract and MSD Lite feature-path regressions

The exact 13a16 storage-hotplug run #43760834648 failed in the host shell startup regression. The first concurrent Aria start exited before daemon launch. Source review found the production CONFIG_TMP and the test's cat-barrier marker both used a literal dollar sign rather than the shell PID.

Aria's PID-qualified path is in place and exact-head storage #43762316227 passed, but the test barrier had a measurement race: the first cat stub could return and allow its launcher to rename the temp file before the second stub counted it. This repair adds a second `checked.$$` barrier so both concurrent cat stubs record the common two-file state before either returns. A two-shell host harness passed with both counts equal to 2.

The same current-head repair restores two unrelated required contracts found by exact-head Actions: the handoff adds the required `Before context exhaustion` phrase and restores the exact singular heading `## Exact first action for the next GPT`; `start_udpxy()` restores the legacy `/usr/sbin/udpxy` `#else` branch for firmware builds without `APP_MSD_LITE`. The process lifecycle checker had caught that real feature-path regression after the MSD Lite refactor.

## Parallel source slice completed — MSD Lite configuration publication

The APP_MSD_LITE branch in start_udpxy() previously truncated /etc/storage/msd_lite.conf before it opened /etc_ro/msd_lite.conf, and it ignored read/write/flush/close failures before enabling and launching the daemon. This slice replaces direct truncation with same-directory mkstemp staging, regular-file/source checks, mode preservation, input validation, checked read/write/flush/fsync/close and atomic rename. Publication failure logs and returns before setting msd_lite_enable or invoking msd_lite.

Changed: trunk/user/rc/src/net.c; new trunk/user/rc/src/msd_lite_config_file.h and tests/msd_lite_config_file.c; Process lifecycle and XY-C1 workflow path filters; scripts/check_process_lifecycle_integration.py. Local GNU89 ASan/UBSan regression PASS, including missing source/parent, invalid interface/port, directory/symlink destination, last-good preservation, mode preservation, RLIMIT_FSIZE write failure and temp cleanup.

The exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow for this source change are pending. Do not transfer the earlier 401066 L3 result to this new C source.

## Verification boundary

- Parent main for this MSD Lite slice: f36d6ec94650103134a55b2e9c9abca9894de075.
- Aria concurrent-start repair f36d6ec94650103134a55b2e9c9abca9894de075 changed aria.sh plus storage_app_start_status.sh after exact 13a16 storage #43760834648 failed; exact-f36 storage workflow #43761890951 was in progress at the last poll. Other f36 Actions #43761890935, #43761890866, #43761890893, #43761890999, #43761890904, #43761890891, #43761890896 and #43761890879 were queued/in progress at that poll.
- MSD Lite files in this atomic slice: net.c, new msd_lite_config_file.h, new tests/msd_lite_config_file.c, Process/XY-C1 workflow path filters, and Process source contract. Parent main f36d6ec94650103134a55b2e9c9abca9894de075.
- Local host command: cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I trunk/user/rc/src trunk/user/rc/src/tests/msd_lite_config_file.c -o msd-lite-config-regression; ASAN_OPTIONS=detect_leaks=1 ./msd-lite-config-regression — PASS.
- New exact-head Process lifecycle and XY-C1 Build XY-C1, TRX verification and artifact upload results must be checked on the new source SHA before claiming L3. No physical-router/L4 result.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; archive index/shards untouched.

## Exact first action for the next GPT

1. Inspect this follow-up's exact-head storage Actions and the preceding b488 exact-head AI docs, infosvr and Process lifecycle checks; confirm PID-qualified `entered.$$` and `checked.$$` rendezvous markers. Repair the first causal failure, not just the symptom.
2. Check XY-C1 Build/TRX/artifact status for the MSD Lite source; claim L3 only if that exact C-source SHA has all three phases PASS.
3. After the storage and required source contracts pass, implement Transmission's checked PID-unique same-directory `settings.json` publication with generator/transform/rename fault injection under host shell and BusyBox ash.
4. Continue broader user-space stability/security work without repeating already-closed audits.
5. Preserve frozen backup and archive index/shards; do not claim L4 without physical-router evidence.
