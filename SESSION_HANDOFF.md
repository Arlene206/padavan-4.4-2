## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. The only development branch is `main`. This workflow syntax correction is being prepared against verified parent `e9ae56274e15cf79e25ec71346dee8c9ca9f08e7`; fetch the actual live ref before every subsequent write.
- Immutable archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Read-only; never modify it.
- **Permanent user entrypoint:** `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. The user does not need a long prompt, previous chat, or repository upload. Never output a recursive next-GPT prompt in chat. Git documents and live GitHub state are the project memory.
- Recovery: read complete `AI_DEVELOPMENT_PROTOCOL.md`, then `AI_BOOTSTRAP_PROMPT.md`; fetch live main + backup and exact-head Actions; then read this handoff, roadmap, newest 20 dev-log records, `docs/dev-log/INDEX.md`, and the semantic audit report for the active caller group.
- Legacy PR #1 remains CLOSED / UNMERGED. Do not reopen it or use its head. No feature branches or PRs. Do not modify GPT Session Rotator.
- L1 lexical inventory and direct API semantic review are complete within their documented scope. Reuse `docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md`; do not repeat the 9,439 tracked-blob scan, 6,193 readable-file scan, 77 `pids()` census or 46 `kill_services()` census.

## Active task

### Implemented P1 foundation

- New shared tri-state scanner: `trunk/user/shared/src/process_inventory.h`, `process_inventory.c`; states RUNNING/ABSENT/UNKNOWN are distinct, count is separate, identities capture PID/start_time/comm/argv hashes, procfs and allocation ops are injectable, EOF/errors distinguished, partial snapshots are discarded on UNKNOWN.
- `trunk/user/shared/src/Makefile` includes `process_inventory.o`; the source file and object now match. MIPS job confirmed this object compiled and `libshared.so` installed `process_inventory.h` into `trunk/stage/include/shared`.
- `/proc/PID/stat` field 22 start_time is token 20 after comm. Scanner test asserts concrete value `98765` and exercises disappearance between stat/cmdline.
- `pids_main()` is now a pure CLI exit-status API: complete RUNNING/ABSENT returns `EXIT_SUCCESS`; UNKNOWN returns `EXIT_FAILURE`; count/PIDs are printed only. This was required because `rc.c` returns its value directly from the `/sbin/pids` applet. Legacy `pids()` remains Boolean-compatible until all 77 callers are migrated.
- `service_lifecycle_policy.h` includes staged public header `<process_inventory.h>`; Watchdog/HTTPD host compile commands add `-I trunk/user/shared/src`. Their workflow path filters watch the public shared header in push and pull_request events.
- `kill_services()` preflights all configured service names before TERM, checks full plan membership before first signal, revalidates PID identity before each TERM/KILL, and signals the captured PID (not a later name-wide killall). UNKNOWN/CHANGED/TIMEOUT/FAILED are not STOPPED.
- Production-connected host tests and CI: `trunk/user/shared/src/tests/process_inventory.c`, `trunk/user/rc/src/tests/service_lifecycle_policy.c`, `.github/workflows/process-lifecycle-regression.yml`, `scripts/check_process_lifecycle_integration.py`.
- The one-line handoff rule is documented in `AI_BOOTSTRAP_PROMPT.md` and `AI_DEVELOPMENT_PROTOCOL.md`.

### Exact-head Actions on parent `e9ae56274e15cf79e25ec71346dee8c9ca9f08e7`

- P1 process lifecycle: run `43599910902`, job `132462258917` — SUCCESS.
- AI handoff docs: `43599910777` — SUCCESS.
- infosvr: `43599910813` — SUCCESS.
- Watchdog: `43599910757` — SUCCESS.
- crond: `43599910794` — SUCCESS.
- GPIO button lifecycle: `43599910812` — SUCCESS.
- AdGuardHome: `43599910716` — SUCCESS.
- AliDDNS: `43599910707` — SUCCESS.
- AliyunDrive WebDAV: `43599910744` — SUCCESS.
- L1 inventory workflow: `43599910764` — SUCCESS.
- NVRAM transaction audit: `43599910753` — still IN PROGRESS at last check.
- XY-C1 firmware: run `43599910863`, job `132462258703` — still IN PROGRESS at last check, at `Build XY-C1` after dependencies and staged-header contract checks. No TRX/artifact conclusion yet.
- HTTPD workflow run `43599909724` on the same SHA failed immediately with zero jobs. Git compare exposed the precise cause: duplicate consecutive `pull_request:` keys in `.github/workflows/httpd-security.yml` (one extra line at around line 246). The service lifecycle test compile command and push/pull_request header paths themselves are otherwise updated. This follow-up removes the duplicate key. Do not describe this as a product-code regression.
- Parent SHA `91deae...` MIPS run `43598951803` failed at RC header lookup `../../shared/src/process_inventory.h`; the shared object was already built/installed. Parent SHA `e9ae562...` XY-C1 run above is the retest using `<process_inventory.h>`, not yet complete.
- No TRX artifact should be claimed until the new exact-head XY-C1 run completes the `Verify TRX output` and artifact-upload steps.

## Explicitly uncompleted P1 work

- All 77 direct `pids()` callers remain legacy Boolean users. Migrate by semantic group; never change `pids()` to negative UNKNOWN until every caller is explicit.
- `kill_pidfile_s()` remains unchanged. Strict PID file parser, PID + start_time + expected executable identity validation, and producer/consumer coordination remain open. Producers include detect_internet, detect_link, watchdog, rstats, BusyBox udhcpc, PPPD, INICD. Find the actual writer of `/var/run/lpdparent.pid`; do not guess from filename.
- Upper-layer stop propagation remains open: VPN Server/Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown, firmware `stop_services()`/flash, Samba config publication, storage eject/unmount, NVRAM stopped-state publication.
- Add PPP/PID-file targets to the same full-set preflight before any signal. Audit direct `killall` and signal paths outside `kill_services()`.
- Linux 4.4 has no `pidfd_send_signal()`; PID/start_time validation leaves a check/kill TOCTOU window. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and was not changed.

## Verification boundary

- L1: previous lexical and semantic direct-call review PASS within its documented limits.
- L2: parent `e9ae562...` P1, Watchdog, crond, GPIO, infosvr and other listed host Actions PASS; HTTPD workflow syntax correction is in this follow-up and must be retested. New exact-head Actions after this follow-up must be inspected.
- L3: `e9ae562...` XY-C1 MIPS/uClibc build remains IN PROGRESS at last check; no target-build result until exact-head job and TRX validation finish. Previous `91deae...` MIPS attempt failed at header path. 
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub. Never ask the user for the whole repository.

## Exact first action for the next GPT

1. User needs only `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. Read complete `AI_DEVELOPMENT_PROTOCOL.md` then `AI_BOOTSTRAP_PROMPT.md`.
2. Fetch live main and backup. Confirm backup stays `72f4f157a084f8184b9597ca5bf334b8080e5807`.
3. Inspect the exact-head HTTPD workflow after this duplicate-key fix; verify it now has a job and its bounded service termination/WAN stop tests pass. Also inspect Watchdog, process lifecycle, AI docs and all other current-main push Actions.
4. Inspect exact-head XY-C1 run `43599910863` and NVRAM `43599910753` until terminal; inspect every step, log and artifact. If XY-C1 fails, repair its first causal issue on main; do not claim L3 until TRX validation and upload succeed.
5. Then continue P1 with strict PID file identity (first identify the true `lpdparent.pid` writer), then VPN Server/Client, USB modem, AP LAN, and firmware shutdown propagation. Only full STOPPED result may authorize dependent cleanup/restart/unmount/flash.
6. Migrate remaining `pids()` and direct signals by semantic group. Add caller-level tests proving UNKNOWN blocks cleanup, NVRAM state publication, restart and firmware progression.
7. Before writes re-fetch main. Commit directly to main only. Append dev log for substantive work, update handoff on status/next-action changes, roadmap only for global status changes. Keep these four heading strings exactly.
8. At the next stop, update this Git handoff for the following conversation. Do not generate recursive chat prompts. The one-line user entrypoint remains sufficient.
