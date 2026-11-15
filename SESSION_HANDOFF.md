# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main SHA at start of MSD Lite parallel source slice: f36d6ec94650103134a55b2e9c9abca9894de075.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Recent main commits:
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
- Storage startup work: Transmission and Aria return actual start-stop-daemon status independently from logger; Aria checks directory/session/download preparation and ownership/permissions, writes default config to PID-qualified same-directory temp and atomically renames after successful write; Transmission stages default settings in PID-qualified same-directory temp and checks generator, sed transform, mv and final nonempty file before daemon launch. Regression tests inject launch, logger, write, transform, rename and successful publication paths under host shell and BusyBox ash.
- Exact current-main Actions:
- #43760834673 AI handoff documentation contract: completed/success
- #43760834671 AliDDNS signed API transport security regression: completed/success
- #43760834666 Process lifecycle inventory - 13a16ec70f7f78f9a8114582a51e7d9e88bd9644: completed/success
- #43760834648 USB storage hotplug stability regression: completed/failure
- #43760834637 NVRAM transaction boundary audit: completed/success
- #43760834634 AliyunDrive WebDAV lifecycle stability regression: completed/success
- #43760834624 Process inventory and lifecycle fail-closed regression: completed/success
- #43760834623 infosvr packet and signal lifecycle stability regression: completed/success
- #43760834594 AdGuardHome config security regression: completed/success
- Current storage workflow state: #43760834648 USB storage hotplug stability regression: completed/failure.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 claim.

## Active task

### Repair Aria concurrent-start staging regression

The exact 13a16 storage-hotplug run #43760834648 failed in the host shell startup regression. The first concurrent Aria start exited before daemon launch. Source review found the production CONFIG_TMP and the test's cat-barrier marker both used a literal dollar sign rather than the shell PID.

This repair changes the default config path to aria2.conf.tmp.$$ and the two-process barrier marker to a PID-qualified filename. The regression then requires two staged files to coexist, both config writes/renames to complete, both processes to reach start-stop-daemon, and no temp files to remain. Exact-head host and BusyBox ash validation is required after commit.

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

## Exact first actions for the next GPT

1. Check exact-f36 storage-hotplug run #43761890951 and finish the Aria concurrent-start validation; inspect full logs if it fails.
2. Check the exact-head Process lifecycle regression for the MSD Lite source contract and host ASan/UBSan test.
3. Check XY-C1 Build, TRX verification and artifact upload on that exact same source SHA; only then claim L3.
4. Resume storage pre-launch review with Transmission settings and permission/postcondition edges, then other user-space startup reliability items.
5. Preserve the frozen backup and archive index/shards; do not claim L4 without physical-router evidence.
