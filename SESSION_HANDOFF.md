# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main at the start of the Transmission settings-contract slice: 7bab2e0784e0e93e9defc451aed3fb8c89e448b9.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Recent main commits:
- 7bab2e0784e fix(storage): initialize Transmission cleanup state early
- a391be7a60c7 fix(storage): gate Transmission launch on setup status
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

### Enforce the generated Transmission settings contract

Transmission now clears CONFIG_TMP before any early setup failure (7bab) and checks its config/download/incomplete/watch directories plus required mode/ownership setup before launching (a391). Exact a391 storage #43765111683 passed the full host and firmware BusyBox ash regression. The default-settings publisher also stages privately as settings.json.tmp.$$ and checks generator/transform/rename status, but only checks nonempty content.

A malformed nonempty generator output with no umask key could still be published, and the transform stage does not verify umask 0 was actually written. This slice adds source/post-transform content checks and a missing-umask test. The XY-C1 workflow's push path filter also omits trunk/user/transmission/transmission.sh; add this path so this shell firmware source change schedules exact-SHA Build/TRX/artifact verification.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in start_udpxy() previously truncated /etc/storage/msd_lite.conf before it opened /etc_ro/msd_lite.conf, and it ignored read/write/flush/close failures before enabling and launching the daemon. This slice replaces direct truncation with same-directory mkstemp staging, regular-file/source checks, mode preservation, input validation, checked read/write/flush/fsync/close and atomic rename. Publication failure logs and returns before setting msd_lite_enable or invoking msd_lite.

Changed: trunk/user/rc/src/net.c; new trunk/user/rc/src/msd_lite_config_file.h and tests/msd_lite_config_file.c; Process lifecycle and XY-C1 workflow path filters; scripts/check_process_lifecycle_integration.py. Local GNU89 ASan/UBSan regression PASS, including missing source/parent, invalid interface/port, directory/symlink destination, last-good preservation, mode preservation, RLIMIT_FSIZE write failure and temp cleanup.

The exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow for this source change are pending. Do not transfer the earlier 401066 L3 result to this new C source.

## Verification boundary

- Parent main: 7bab2e0784e0e93e9defc451aed3fb8c89e448b9.
- Exact a391 push Actions all PASS: USB storage #43765111683; AI docs #43765111687; Process inventory #43765111691; Process lifecycle #43765111678; infosvr #43765111706; AliDDNS #43765111677; AliyunDrive #43765111712; AdGuardHome #43765111669; NVRAM #43765111698.
- a391 adds checked Transmission directory creation/postconditions and required chmod/chown gates; its exact storage action covered injected host/BusyBox mkdir/chmod/chown failures and passed. 7bab further initializes CONFIG_TMP before early errors and verifies inherited environment cleanup safety; exact 7bab Actions were in progress at last poll: storage #43765528314, Process inventory #43765528359, Process lifecycle #43765528378, NVRAM #43765528331, AliDDNS #43765528342 and AliyunDrive #43765528351; AI docs #43765528363, AdGuardHome #43765528318 and infosvr #43765528320 already passed.
- This slice adds expected umask 18 and transformed umask 0 content gates, a malformed nonempty-output regression, and trunk/user/transmission/transmission.sh to the XY-C1 push path filter. Exact-head storage and XY-C1 Build/TRX/artifact checks are pending.
- Exact XY-C1 #43762907958 Build/TRX/artifact PASS remains scoped to C-source SHA b488a561bb91df62aca810a6ff2c191c17f82b2b only; do not transfer it to later commits. No physical-router/L4 result.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; docs/dev-log/INDEX.md and archived shards untouched.

## Exact first action for the next GPT

1. Inspect exact-head USB storage host/BusyBox ash results and XY-C1 Build, TRX verification and artifact upload for the Transmission umask-contract slice; require all phases on one SHA before claiming L3.
2. If green, inspect Transmission stop/restart and remaining runtime lifecycle postconditions; add failure injection where a concrete defect is found.
3. Continue wider Padavan user-space reliability/security work without repeating WAN, Aria or MSD Lite slices.
4. Preserve the frozen backup and archived logs; no L4 without physical-router evidence.
