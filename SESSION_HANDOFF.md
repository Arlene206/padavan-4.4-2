## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Development is on `main` only. This bounded P1 phase-2 implementation is assembled against the verified parent main SHA `f32922d2afc9e8133625551b0dc3e6f361f378d5`; after the tree commit, resolve the actual current main SHA from GitHub rather than assuming this parent is still the tip.
- Immutable archive: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, rename, prune, reset, rebase, merge, or delete it.
- Legacy PR #1 remains CLOSED / UNMERGED. Do not reopen or use the PR head. Do not create development branches or PRs. Do not modify GPT Session Rotator.
- L1 lexical inventory and direct-call semantic review are complete for the previously documented scopes. Reuse `docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md`; do not repeat the 77 `pids()` / 46 `kill_services()` caller census or redesign the lexical scanner.
- The semantic review covers the specified direct public-API calls, not an interprocedural closure of all 5,664 broad lifecycle candidates. Preserve that limitation in reports.
- Baseline exact-head push Actions on parent SHA `f32922d2afc9e8133625551b0dc3e6f361f378d5` all passed: NVRAM `43590477651`, AI handoff docs `43590477637`, infosvr `43590477685`, AdGuardHome `43590477635`, AliDDNS `43590477629`, and AliyunDrive `43590477704`. NVRAM's job `132433928048` completed the raw ioctl/kernel API audit. These are parent-SHA baseline runs, not validation of this new P1 implementation.
- Current implementation files are enumerated under Active task below. BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate process scanner and was not changed.

## Active task

### Implemented in this bounded milestone

- Added `trunk/user/shared/src/process_inventory.h` and `process_inventory.c`. The API expresses `PROCESS_PRESENCE_RUNNING`, `PROCESS_PRESENCE_ABSENT`, and `PROCESS_PRESENCE_UNKNOWN` explicitly; process count and process identity snapshots are separate fields.
- The scanner has injectable directory open/read/close, file read, allocation/reallocation and release operations. It distinguishes clean directory EOF from `readdir()` error; treats stat/cmdline permission and I/O errors, malformed stat, argv0 allocation failure and PID-vector allocation/growth failure as UNKNOWN; permits only confirmed per-PID ENOENT/ESRCH disappearance to be skipped; and discards partial snapshots on any UNKNOWN.
- Process identities record PID, `/proc/PID/stat` start_time, comm truncation state and argv0/argv1 fingerprints. The shared library Makefile includes `proc_inventory.o`; the shared header install rule already installs the new `.h` file.
- `pids_main()` now prints an explicit inventory error and returns nonzero on UNKNOWN instead of reporting a false empty/partial result. Legacy `pids()` intentionally retains its old Boolean-compatible behavior in this milestone: do not introduce a negative UNKNOWN into any legacy truthiness branch.
- Reworked `service_lifecycle_policy.h` and the central `kill_services()` adapter in `common_ex.c`: all configured service names are inventoried before TERM; the plan is rescanned before the first signal; each captured PID/start_time identity is revalidated immediately before TERM and KILL; signals target the captured PID directly rather than issuing name-wide `killall` after a scan; UNKNOWN, identity change, timeout or signal failure blocks stop success.
- Added production-scanner fault-injection regression `trunk/user/shared/src/tests/process_inventory.c`, tri-state lifecycle regression `trunk/user/rc/src/tests/service_lifecycle_policy.c`, dedicated workflow `.github/workflows/process-lifecycle-regression.yml`, and supplemental integration contract `scripts/check_process_lifecycle_integration.py`.
- Updated this handoff, `REFACTOR_ROADMAP.md` and root `dev_log.md`. No log archive rotation was due; `docs/dev-log/INDEX.md` was intentionally unchanged.

### Explicitly NOT migrated / remaining P1 work

- The 77 direct `pids()` consumers still use the legacy API and remain to be migrated by semantic group (HTTPD/WebUI status, startup readiness, notification signals, Samba, watchdog, VPN, USB modem, WAN/LAN, storage and plugin hooks). Never switch `pids()` to return negative UNKNOWN until every such caller has explicit state handling.
- PID-file identity work remains open: `kill_pidfile_s()` is unchanged; its strict parser, PID/start_time/expected-name validation, and coordinated producer/consumer formats are not implemented. In-tree detect_internet/detect_link/watchdog/rstats, BusyBox udhcpc, PPPD and INICD are separate producers/routes. Identify the `/var/run/lpdparent.pid` writer before trusting or migrating it; never infer from its filename.
- Some callers still discard `kill_services()` errors or continue destructive cleanup/restart. The central service-name stop engine is safer, but this does NOT yet close VPN Server/Client, USB modem removal, AP LAN teardown, flash_firmware()/void stop_services(), Samba config publication, storage eject and other upper-layer propagation paths. Migrate these in priority order and make UNKNOWN block all dependent cleanup/state publication/restart.
- Direct `killall`/signal paths outside `kill_services()` remain. They require exact identity-aware handling or a documented best-effort classification; a successful inventory is not authorization to use a later name-wide `killall`.
- The current stop transaction preflights configured service-name targets. It does not yet include additional PID-file targets (for example PPP client PID files) in the same plan; VPN Server/Client and DHCP/PPP groups must add those identities before any signal.
- Linux 4.4 has no `pidfd_send_signal()`. PID + start_time and command validation still leave a check/kill TOCTOU window. Do not claim atomic or race-free signalling.

## Verification boundary

- L1: prior sparse inventory and semantic direct-call review remain PASS at their documented source SHAs; this implementation does not re-run or expand that census.
- L2: the new dedicated workflow is `.github/workflows/process-lifecycle-regression.yml`. Its exact-head push run and every job step MUST be inspected for the actual commit SHA before claiming PASS. Local build/test was not run because this runtime has no checkout and GitHub DNS is unavailable to the local shell. Source-code creation or an Actions queue state is not L2 PASS.
- L3: current-main MIPS/uClibc build NOT RUN. Historical evidence at `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` is not validation of this patch.
- L4: XY-C1 actual flash, boot, runtime and functional hardware verification NOT VERIFIED.
- An exact-head green Action does not establish target firmware packaging, target process behavior, L3 or L4.

## Exact first action for the next GPT

1. Read the complete `AI_DEVELOPMENT_PROTOCOL.md` first, then `AI_BOOTSTRAP_PROMPT.md`. Fetch live `refs/heads/main` and the immutable backup ref. Never trust the SHA in this handoff as the live tip.
2. Fetch Actions for the exact live main SHA. Inspect `process-lifecycle-regression.yml` run, all Jobs, every step, and logs for any failure. Only a `push` whose `head_sha` exactly equals the live SHA counts. Also inspect any other triggered exact-head workflows. If failed, fix on main with a new expected-head tree commit and re-run exact-head verification.
3. Read the latest `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, newest 20 complete `dev_log.md` records and `docs/dev-log/INDEX.md`; then reread the full `docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md` for the specific caller group being migrated.
4. Do NOT repeat the completed lexical scanner design, 9,439-blob inventory, 6,193-readable-file census, 77 direct `pids()` classifications or 46 direct `kill_services()` classifications. Do NOT ask for a whole-repository upload.
5. Continue with PID-file identity: design strict parse/status interfaces, bind PID + start_time + expected executable identity, inject file/process operations, and migrate each producer with all consumers as one transaction. First map the exact writer of `lpdparent.pid`; leave unknown ownership fail-closed. Keep BusyBox procps separate.
6. Then migrate upper-layer stop/failure propagation in this order: VPN Server (including all PPP PID targets in the preflight), VPN Client, USB modem DHCP/PPP removal, AP-mode LAN DHCP/bridge teardown, and `flash_firmware()` through aggregated `stop_services()`. Only STOPPED may authorize cleanup, NVRAM stopped-state publication, restart, storage unmount or flash.
7. Migrate the remaining direct `pids()` consumers by semantic purpose; do not change legacy `pids()` return semantics until all callers are explicit. Audit direct `killall` notifications and HTTPD/WebUI numeric status consumers too.
8. Expand fault-injection tests for PID-file strict syntax/identity, state transitions during TERM/KILL, caller-level cleanup gates and feature guards. Run GNU89 `-Wall -Wextra -Werror`, ASan/UBSan, then attempt real MIPS/uClibc builds across relevant STORAGE/USB/IPv6/BOARD/APP configurations.
9. Before writing, re-fetch main and construct one atomic Git tree/commit directly on main using the exact expected parent. Do not create branches or PRs. After commit, re-fetch main and backup, inspect exact-head push Actions and each job step; report L1–L4 separately.
10. Update `dev_log.md` for substantive milestones. Update this handoff only when state, verification or the exact next action changes. Keep all four fixed headings exactly. Do not rotate archives or edit `docs/dev-log/INDEX.md` unless the documented threshold is reached.
