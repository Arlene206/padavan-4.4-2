# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for IGMP proxy config-token validation: abd40abba2c3b78bf02fb262110f485534ba8b4b.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Exact parent abd40 USB storage #43769497238 PASS, including POSIX launcher and firmware BusyBox 1.24.2 ash lifecycle tests. Exact parent AI handoff docs #43769497316 PASS. Process lifecycle #43769497255 and AdGuardHome #43769497209 PASS; infosvr #43769497293 PASS. XY-C1 #43769497292, NVRAM #43769497231, Process inventory #43769497244, AliDDNS #43769497245 and AliyunDrive #43769497299 were still in progress at last poll.
- This commit strengthens IGMP proxy config input validation: the prior helper rejected CR/LF but allowed spaces/tabs and comment characters inside interface/altnet values, which could split a generated directive into extra parser tokens. Interface values are now emitted as printable non-whitespace tokens without '#'; altnet accepts only digits, '.' and '/'.
- The existing atomic same-directory publication / flush / fsync / rename contract remains unchanged. New host tests verify no-token-splitting and last-good config preservation.

## Active task

### Verify IGMP proxy input-token hardening

The IGMP proxy publisher already staged its configuration, checked writes/flush/fsync/close and atomically renamed only complete output. A source review found a remaining configuration-injection/stability gap: `igmpproxy_config_single_line()` rejected only CR/LF, so a tab/space or '#' in NVRAM-derived `altnet` or interface names could split/comment a generated directive without containing a newline.

THIS COMMIT changes the validation contract: interface names must be one printable non-whitespace token without comment markers; altnet is restricted to numeric IPv4/CIDR punctuation (digits, dot, slash). Regression tests cover same-line whitespace/comment injection and preserve the existing last-good file. Exact Process host regression and XY-C1 build/TRX/artifact checks for THIS SHA are pending.

After validation, continue the next uncompleted P1 service/lifecycle boundary from the roadmap; avoid re-auditing the completed atomic publication path.

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

- Parent main: `abd40abba2c3b78bf02fb262110f485534ba8b4b`; IGMP validator, regression, static checker and handoff/roadmap/log update are one atomic commit.
- Exact parent abd40 USB storage #43769497238 PASS; AI handoff #43769497316 PASS; Process lifecycle #43769497255 PASS; AdGuardHome #43769497209 PASS; infosvr #43769497293 PASS. XY-C1 #43769497292, NVRAM #43769497231, Process inventory #43769497244, AliDDNS #43769497245 and AliyunDrive #43769497299 were pending at last poll.
- New Process lifecycle and XY-C1 workflows must compile/run IGMP tests on this SHA. No L3/L4 claim until exact-head CI completes.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archive shards/index untouched.

## Exact first action for the next GPT

1. Inspect this SHA's Process lifecycle regression: compile/run IGMP proxy config publication tests (new whitespace/comment rejection plus the existing failure/rollback tests) and the whole caller graph.
2. Inspect this SHA's XY-C1 Build, TRX verification and artifact upload; claim L3 only if all phases pass.
3. Continue the next uncompleted P1 enabled-service startup/lifecycle boundary in the roadmap; do not repeat Transmission launcher or IGMP atomic publication audits.
4. Preserve the frozen backup and archive inventory. No L4 without physical-router evidence.

