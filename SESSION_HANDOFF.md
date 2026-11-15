# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this correction: 814d4639643675ed9da0b315e7bbdcabaaa89061.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and read-only.
- Exact parent 814d storage #43768621241 FAILED immediately at `sh -n trunk/user/transmission/transmission.sh`: the PID character-class pattern contained an unquoted space (`*[!0-9 ]*`) and was a shell parse error. The failure occurred before stop behavior tests ran.
- Exact parent 814d AI handoff #43768621220, NVRAM #43768621188, Process inventory #43768621322, Process lifecycle #43768621183, infosvr #43768621156, AliDDNS #43768621223, AdGuardHome #43768621236 and AliyunDrive #43768621152 PASS. XY-C1 #43768621137 was still in progress at last inspection.
- THIS COMMIT replaces that pattern with per-token numeric validation before any signal, repairs the stop-function extraction in the regression harness, fixes its printf newline escaping, and requires restart to abort if stop fails.
- Previous successful exact XY-C1 #43768122787 was for source SHA ba51619ceea8efc084d28f6f38bb5a8bc084e8d7 only. No current L3/L4 claim.

## Active task

### Verify Transmission stop postconditions and restart gating

Exact 814d USB storage #43768621241 failed at the shell syntax check, before running any launcher tests. The causal defect is the case glob `*[!0-9 ]*`; an unquoted space in that pattern breaks shell parsing. THIS COMMIT validates each whitespace-separated PID independently against `''|*[!0-9]*` before signaling any PID. It also fixes the regression harness to extract the function using a sed range and to write real newline-delimited PID fixtures. The restart command now exits immediately when stop fails, and the test statically asserts the gate.

Next verify the exact-head host and BusyBox 1.24.2 ash tests, including graceful stop, multi-PID forced stop, failed SIGKILL, malformed output and restart gating. If green, continue wider user-space reliability/security work from the roadmap.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: `814d4639643675ed9da0b315e7bbdcabaaa89061`; source, regression and docs are one atomic commit.
- Exact parent storage #43768621241 FAIL: syntax error at line 195 from the invalid character-class glob. This commit fixes syntax and additionally repairs two test-harness defects discovered by inspection (incorrect function extraction and double-escaped printf newlines).
- Exact parent AI docs #43768621220 PASS; NVRAM #43768621188, Process inventory #43768621322, Process lifecycle #43768621183, infosvr #43768621156, AliDDNS #43768621223, AdGuardHome #43768621236 and AliyunDrive #43768621152 PASS.
- Parent XY-C1 #43768621137 was in progress at last inspection; no TRX/artifact result. New exact-head storage and XY-C1 runs required for THIS COMMIT.
- Prior XY-C1 #43768122787 PASS (Build, TRX, artifact) applies only to source SHA `ba51619ceea8efc084d28f6f38bb5a8bc084e8d7`. No new L3 or physical-router/L4 claim.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archive shards/index unchanged.

## Exact first action for the next GPT

1. Inspect exact-head USB storage host-shell and BusyBox 1.24.2 ash execution. Require all startup/config tests, PID-unique stage collision, graceful stop, multiple-PID SIGKILL, failed-signal postcondition, malformed-PID rejection and restart gating to pass.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload. L3 only if all phases pass for this exact SHA.
3. Once green, continue the highest-priority uncompleted stability/security item from the whole-repository roadmap; do not repeat already-completed Transmission settings-stage or stop semantics.
4. Preserve the frozen backup and archive inventory; no L4 without physical-router evidence.

