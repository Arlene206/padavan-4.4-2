# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this source/test repair: 4d2afeddba949b77dcbe1a8f6f950e9093438910.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Exact parent 4d2 push results: USB storage #43765741973 FAIL at test line 209 because optional argument $6 was read under set -u; AI handoff docs #43765741952 FAIL because root dev_log.md reached 120 records. Both causes are repaired in THIS COMMIT. XY-C1 #43765742001 was still running at last check. AliDDNS #43765742065, infosvr #43765742025, AdGuardHome #43765742019, NVRAM #43765741983, AliyunDrive #43765741945, Process inventory #43765741935 and Process lifecycle #43765741927 passed.
- Aria staging and two-phase concurrency markers remain PID-qualified. Transmission checks directory creation and required permission/ownership changes, validates generated umask 18 and transformed umask 0, and checks generation/transform/rename before daemon startup.
- This source slice makes Transmission cleanup ownership-aware: failed noclobber creation never deletes an existing settings stage; only a successfully created file is removed on later failure. Regression coverage checks a deterministic existing-stage collision.
- Root dev_log.md was rotated from 120 to 20 prior records, then this new entry was added; it now retains 21 newest records. The oldest 100 complete records are preserved in docs/dev-log/2026/11/part-011.md with the exact size/hash recorded in docs/dev-log/INDEX.md.
- Exact XY-C1 #43762907958 passed Build/TRX/artifact only for C-source SHA b488a561bb91df62aca810a6ff2c191c17f82b2b (L3 scoped to that SHA). Parent run #43765742001 must complete all phases for 4d2; this commit will trigger a separate exact-SHA run because Transmission shell source changed. No physical-router/L4 result.

## Active task

### Verify Transmission settings-stage ownership and repair the exact-head storage regression

The previous exact-head storage run #43765741973 failed at line 209: a newly optional sixth function argument was read unconditionally while the test uses set -u. The helper now defaults it to 0, preserving old invocations and keeping the explicit missing-umask fault case.

Production cleanup previously deleted CONFIG_TMP whenever set, even when the shell's noclobber open had failed against a pre-existing path. CONFIG_TMP_OWNED now distinguishes a stage created by this invocation from a collision. It becomes 1 only after successful exclusive creation and is cleared after atomic rename; failure cleanup removes only owned files. The regression rewrites the stage suffix in a temporary launcher copy to a fixed test path, pre-creates that path, and verifies its bytes survive while no settings file or daemon launch is produced.

The same parent AI docs run #43765741952 failed because the root log had 120 records. The oldest 100 complete records were archived losslessly in part-011 and the root retains the newest 20 plus this commit's new record.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in start_udpxy() previously truncated /etc/storage/msd_lite.conf before it opened /etc_ro/msd_lite.conf, and it ignored read/write/flush/close failures before enabling and launching the daemon. This slice replaces direct truncation with same-directory mkstemp staging, regular-file/source checks, mode preservation, input validation, checked read/write/flush/fsync/close and atomic rename. Publication failure logs and returns before setting msd_lite_enable or invoking msd_lite.

Changed: trunk/user/rc/src/net.c; new trunk/user/rc/src/msd_lite_config_file.h and tests/msd_lite_config_file.c; Process lifecycle and XY-C1 workflow path filters; scripts/check_process_lifecycle_integration.py. Local GNU89 ASan/UBSan regression PASS, including missing source/parent, invalid interface/port, directory/symlink destination, last-good preservation, mode preservation, RLIMIT_FSIZE write failure and temp cleanup.

The exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow for this source change are pending. Do not transfer the earlier 401066 L3 result to this new C source.

## Verification boundary

- Parent main: 4d2afeddba949b77dcbe1a8f6f950e9093438910; source/test/doc/archive changes are in THIS COMMIT.
- Parent storage #43765741973 failed only because test line 209 evaluated optional $6 under set -u. Parent AI docs #43765741952 failed only because dev_log.md had 120 entries. These exact causes are repaired; new exact-head runs must verify them.
- Parent XY-C1 Build/TRX/artifact run #43765742001 was in progress at last poll and is for the parent SHA only. This commit changes Transmission shell source; inspect the new exact-head build, TRX verification and artifact upload separately.
- Other exact parent 4d2 checks passed: AliDDNS #43765742065, infosvr #43765742025, AdGuardHome #43765742019, NVRAM #43765741983, AliyunDrive #43765741945, Process inventory #43765741935 and Process lifecycle #43765741927.
- Root log rotation is indexed: part-011 contains 100 records in chronological order, and root contains 21 newest-first records. Existing archive shards are unchanged.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. No L4 claim.

## Exact first action for the next GPT

1. Inspect exact-head USB storage and AI handoff Actions for THIS COMMIT. Storage must pass the POSIX host test, firmware BusyBox 1.24.2 ash test, missing-umask case, pre-existing stage collision, inherited CONFIG_TMP, directory/permission failures and the Aria concurrency/fault cases. Repair the first causal failure.
2. Inspect the new exact-SHA XY-C1 run for Build XY-C1, TRX verification and artifact upload; claim L3 only if all phases pass for one exact source SHA. No L4 without physical-router evidence.
3. Once green, continue a new highest-priority user-space stability/security defect from the roadmap. Do not repeat the completed Transmission config-generation, directory/permission or stage-ownership audits.
4. Preserve the frozen backup and all existing archive shards/index; future root log additions must respect the 20–119 window and batch-100 rotation.

