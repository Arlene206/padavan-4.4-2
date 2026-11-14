## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. This corrective follow-up is being committed on top of verified live parent `91deae70b0582f2925f95679ab722f97b41637a6`; resulting commit is marked `THIS COMMIT` in the matching dev-log record. Always fetch live `refs/heads/main` before any future write.
- Immutable archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only, never modify.
- Permanent minimal takeover: user only says `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. Never request a long prompt, prior transcript, repository upload or recursive prompt. The repository docs and live GitHub state are the only durable project memory.
- Before source edits read complete `AI_DEVELOPMENT_PROTOCOL.md`, then `AI_BOOTSTRAP_PROMPT.md`; recover live main/backup and exact-head Actions; read this file, roadmap, newest 20 dev-log entries, index and the semantic audit required by the selected task.
- Legacy PR #1 remains CLOSED / UNMERGED. Do not reopen/use it. Never use development branches/PRs. Do not modify GPT Session Rotator.
- Prior L1 lexical inventory and direct API semantic review are complete within their stated scope. Reuse `docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md`; do not repeat the 9,439 tracked blob scan, 6,193 readable-file scan, 77 `pids()` callers or 46 `kill_services()` callers census.

## Active task

### Implemented P1 foundation and corrective work

- Added `trunk/user/shared/src/process_inventory.h` and `process_inventory.c`: separate RUNNING / ABSENT / UNKNOWN, separately stored count and identity snapshot, PID/start_time/comm/argv fingerprints, injected procfs and allocator operations, EOF/error distinction, and discard-all partial results on UNKNOWN.
- Shared library Makefile now requests `process_inventory.o`, matching `process_inventory.c`; target build run `43598951803` confirmed the object compiled and `libshared.so` installed, including `process_inventory.h`.
- `/proc/PID/stat` field 22 start_time is parsed as token 20 after comm. Scanner regression asserts concrete `start_time == 98765`; an injected case covers disappearance after stat and before cmdline, then verifies ENOENT on stat re-read.
- `pids_main()` is now a CLI status API, not a count API: RUNNING or ABSENT with a complete scan returns `EXIT_SUCCESS`; UNKNOWN returns `EXIT_FAILURE`; PIDs/count are printed only. It does not return negative UNKNOWN or a process count. Legacy `pids()` remains Boolean-compatible until its 77 direct callers are migrated.
- Service policy/header uses the staged shared include `<process_inventory.h>`, expected in `trunk/stage/include/shared` by the MIPS build. Host Watchdog/HTTPD test compile commands add `-I trunk/user/shared/src` and both workflow filters watch `process_inventory.h`.
- Central `kill_services()` preflights all configured service-name targets before TERM, checks complete membership before first signal, verifies PID/start_time before each TERM/KILL, and signals captured PIDs rather than rerunning `killall` by name. UNKNOWN/CHANGED/TIMEOUT/FAILED cannot authorize cleanup or restart.
- Regression and workflow paths: `trunk/user/shared/src/tests/process_inventory.c`, `trunk/user/rc/src/tests/service_lifecycle_policy.c`, `.github/workflows/process-lifecycle-regression.yml`, `scripts/check_process_lifecycle_integration.py`.
- Permanent simple entrypoint is encoded in `AI_BOOTSTRAP_PROMPT.md` and `AI_DEVELOPMENT_PROTOCOL.md`; no long prompt is needed from the user and no recursive prompt should be generated.

### Exact-head validation on SHA `91deae70b0582f2925f95679ab722f97b41637a6`

- P1 process inventory/lifecycle regression: run `43598951793`, job `132459367869` — PASS; GNU89 `-Wall -Wextra -Werror`, ASan/UBSan scanner fault injection, lifecycle tests and integration contract all passed.
- AI handoff docs: `43598951775` PASS.
- infosvr: `43598951786` PASS.
- AdGuardHome: `43598951840` PASS.
- AliDDNS: `43598951800` PASS.
- AliyunDrive WebDAV: `43598951838` PASS.
- process inventory lexical workflow: `43598951813` PASS.
- NVRAM transaction audit: `43598951790` PASS.
- Watchdog/HTTPD runs for this SHA were not triggered by this particular path set; previous parent SHA `91deae70...` predecessor has earlier passing runs, but the present follow-up updates their compile include paths and should trigger them.
- XY-C1 firmware run `43598951803`, job `132459367722` — FAILED at `Build XY-C1` while compiling `common_ex.o`:
  `service_lifecycle_policy.h:9:10: fatal error: ../../shared/src/process_inventory.h: No such file or directory`.
- This is a staging/include-path issue, not a process_inventory object failure. The job log confirms `process_inventory.o` built, `libshared.so` installed, and `process_inventory.h` copied to `trunk/stage/include/shared`; RC compiler has `-I trunk/stage/include/shared`. The corrective commit changes the policy include to `<process_inventory.h>`.
- MIPS build did not reach TRX verification; no artifact was uploaded. L3 remains FAILED on SHA `91deae70...` pending the next exact-head XY-C1 run.

## Explicitly uncompleted P1 work

- The 77 direct `pids()` callers remain on legacy Boolean semantics. Migrate by intent groups; do not change legacy return semantics until every caller handles tri-state explicitly.
- `kill_pidfile_s()` remains unchanged. Strict PID file parser, PID + start_time + expected process identity validation, all producers and consumers must be coordinated. Identify the real writer of `/var/run/lpdparent.pid`; do not guess from its name.
- Upper-layer propagation remains open: VPN Server/Client, USB modem DHCP/PPP removal, AP-mode LAN teardown, firmware shutdown through `stop_services()`, Samba configuration publication, storage eject/unmount and NVRAM stopped-state publication.
- Add extra PID-file processes (especially PPP client processes) to the same whole-set preflight before any TERM.
- Audit direct signal/killall paths outside `kill_services()`; a prior snapshot does not authorize a later name-wide kill.
- Linux 4.4 lacks `pidfd_send_signal()`; PID + start_time and command fingerprint checking leaves residual check/kill TOCTOU. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` is a distinct scanner and remains unchanged.

## Verification boundary

- L1: prior inventory and direct public API semantic review PASS within documented limits.
- L2: SHA `91deae70b0582f2925f95679ab722f97b41637a6` P1 regression and listed workflows PASS. This follow-up's own exact-head P1/Watchdog/HTTPD/XY-C1 results remain pending.
- L3: SHA `91deae70...` attempted real MIPS/uClibc XY-C1 build and failed only at RC header include resolution after the shared object was compiled/installed. Corrective include and host include paths are in this commit; rebuild and TRX verification remain mandatory.
- L4: physical XY-C1 flash, boot and functional validation NOT VERIFIED.
- Local repository checkout is unavailable in this runtime; remote CI and GitHub source inspection were used. Do not ask the user to upload the whole repository.

## Exact first action for the next GPT

1. User entrypoint only: `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. Read complete `AI_DEVELOPMENT_PROTOCOL.md`, then `AI_BOOTSTRAP_PROMPT.md`.
2. Fetch live main and immutable backup refs; confirm backup SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
3. Inspect exact-head `process-lifecycle-regression.yml`, Watchdog, HTTPD, AI docs and XY-C1 workflows for this commit. Confirm policy header resolves via staged `-I trunk/stage/include/shared`, host tests resolve via `-I trunk/user/shared/src`, and `pids_main()` is pure exit status. Inspect every job step and log.
4. If XY-C1 progresses, verify the TRX file size/name/checksum and uploaded artifact on the exact current SHA. If it fails, inspect the first causal compiler or packaging error and repair directly on main. No target build is PASS before TRX verification; no L4 without actual hardware.
5. Once build wiring is green, resume P1 with strict PID-file identity and the true `lpdparent.pid` producer, then VPN Server (include all PPP PID targets in global preflight), VPN Client, USB modem, AP LAN and firmware shutdown cleanup gates.
6. Migrate remaining direct `pids()` callers and direct signal/killall paths by semantic group. Add tests proving UNKNOWN blocks teardown, restart, NVRAM status publication and firmware progression.
7. Re-fetch main before each write; commit directly to main only. Never branch/PR. Update dev log for substantive milestones, handoff when the next action/status changes, roadmap when global priority changes. Preserve all four fixed headings above.
8. Before ending, write the complete instructions for the following GPT into this Git handoff. Do not generate recursive prompts in chat; one-line user entrypoint remains sufficient.
