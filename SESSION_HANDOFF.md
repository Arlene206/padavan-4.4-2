# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this repair: b21b95b3e1858ae752ef7dc34259204524e484e4.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- Exact c0 storage run #43767296645 FAILED at the pre-existing staging-collision regression. The fixture replaced only a PID-qualified suffix, but live source had a single literal dollar; the fixture therefore did not collide and detected an unintended daemon start.
- The b21 checkpoint commit did not change the Transmission source blob: it still contained `CONFIG_TMP="${CONFIG_FILE}.tmp.$"`. THIS COMMIT changes it to `CONFIG_TMP="${CONFIG_FILE}.tmp.$$"`; the existing collision regression will now exercise the production noclobber collision path.
- Exact c0 AI handoff docs #43767296578 PASS. Exact c0 Process inventory #43767296590, Process lifecycle #43767296606, infosvr #43767296612, AliDDNS #43767296621, AliyunDrive #43767296670 and AdGuardHome #43767296717 PASS. c0 XY-C1 #43767296597 and NVRAM #43767296583 were pending at last inspection; neither validates this repair.
- Transmission's ownership-aware cleanup removes a staging file only after this process successfully creates it. Generated content is checked for umask 18, transformed umask 0, and publication status before daemon start.
- Earlier XY-C1 Build/TRX/artifact #43765742001 is an exact pass for SHA 4d2afeddba949b77dcbe1a8f6f950e9093438910 only. No L3 for this repair and no L4 hardware evidence.

## Active task

### Verify PID-unique Transmission settings staging, then inspect stop postconditions

The c0 exact-head storage workflow #43767296645 failed because the production temp path had regressed to `settings.json.tmp.$`, while the collision fixture rewrites `settings.json.tmp.$$`. A first follow-up checkpoint did not change the source blob; the live parent still had the single-dollar path. THIS COMMIT now restores the PID-qualified suffix. The existing regression requires a collision to abort startup without deleting or altering the pre-existing stage.

After the exact-head storage host and BusyBox ash regression passes, inspect `func_stop()` in `trunk/user/transmission/transmission.sh`. It passes the whitespace-separated `pidof` result as one quoted argument to `kill -9` and does not verify the daemon is absent after the forced kill. Confirm the behavior with injectable tests, including multiple PIDs and signal failure, before changing the stop path.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: `b21b95b3e1858ae752ef7dc34259204524e484e4`; this commit restores the actual PID-qualified staging path and refreshes the handoff, roadmap and current log record in one atomic change.
- Exact c0 USB storage #43767296645 FAIL: collision test observed a daemon start because source suffix was a single dollar. The source blob remained unchanged in b21; THIS COMMIT corrects it. Exact-head host and BusyBox ash verification required.
- Exact c0 AI handoff docs #43767296578 PASS. Exact c0 Process inventory #43767296590, Process lifecycle #43767296606, infosvr #43767296612, AliDDNS #43767296621, AliyunDrive #43767296670 and AdGuardHome #43767296717 PASS.
- c0 XY-C1 #43767296597 was pending at last inspection and predates this source correction. Exact-head XY-C1 must complete Build, TRX verification and artifact upload before L3 is claimed.
- Earlier XY-C1 #43765742001 is scoped to SHA 4d2afeddba949b77dcbe1a8f6f950e9093438910 only. No physical-router/L4 evidence.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; prior archive shards and index remain untouched.

## Exact first action for the next GPT

1. Inspect exact-head USB storage host and firmware BusyBox 1.24.2 ash runs. Confirm the pre-existing stage-collision case now fails closed, leaves the foreign stage byte-for-byte intact, and does not invoke Transmission.
2. Inspect exact-head XY-C1 Build XY-C1, TRX verification and artifact upload. Claim L3 only if all phases pass on this exact source SHA.
3. Then inspect Transmission `func_stop()` with regression coverage for multiple PIDs, kill failure and final daemon absence; implement only the demonstrated behavior.
4. Continue wider user-space stability/security work from the roadmap. Preserve the frozen backup and archive shards; no L4 without physical-router evidence.

