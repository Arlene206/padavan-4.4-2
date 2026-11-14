## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. The only development branch is `main`. Last verified main/product-source baseline: `d3951a276ed4002aff3232cf93c8d55662a94e95`. This handoff is updated by a user-requested, standalone governance change; later docs-only commits do not require rewriting this source baseline. Fetch the actual live main before every write.
- Immutable archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Read-only; never modify it.
- **Permanent user entrypoint:** `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. The user does not need a long prompt, previous chat, or repository upload. Never output a recursive next-GPT prompt in chat. Git documents and live GitHub state are the project memory.
- Recovery is read-only by default: read complete `AI_DEVELOPMENT_PROTOCOL.md` and `AI_BOOTSTRAP_PROMPT.md`; fetch live main + immutable backup; inspect exact-head Actions; then read this handoff, roadmap, newest 20 dev-log records, `docs/dev-log/INDEX.md`, and the semantic audit report for the active caller group. Do not create a commit to refresh SHA fields, record a takeover, or report a CI poll.
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

### Exact-head Actions on product-source baseline d3951a276ed4002aff3232cf93c8d55662a94e95

- P1 process scanner/lifecycle: run [43600235518](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235518), job 132463212570 — SUCCESS. GNU89 production scanner compile, ASan/UBSan fault injection, lifecycle compile/tests, and integration boundaries all passed.
- HTTPD: run [43600235494](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235494), job 132463213002 — SUCCESS, including bounded service termination and WAN failure gates. The prior zero-job failure was caused by the duplicate pull_request key and is fixed.
- AI handoff docs: run [43600235547](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235547), job 132463212630 — SUCCESS on this product-source baseline.
- NVRAM transaction boundary: run [43600235537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235537), job 132463212613 — SUCCESS, including userland ioctl/kernel NVRAM API audit.
- infosvr: run [43600235604](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235604), job 132463212766 — SUCCESS.
- AliDDNS: run [43600235647](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235647), job 132463213844 — SUCCESS.
- AliyunDrive: run [43600235663](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235663), job 132463213025 — SUCCESS.
- AdGuardHome: run [43600235516](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235516), job 132463212414 — SUCCESS.
- Watchdog, crond and GPIO lifecycle workflows also passed on the immediately preceding product-compatible parent e9ae56274e15cf79e25ec71346dee8c9ca9f08e7; they were not re-run solely to refresh docs.

### XY-C1 MIPS/uClibc target build — L3 VERIFIED

- Exact product-source SHA d3951a276ed4002aff3232cf93c8d55662a94e95: run [43600235493](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235493), job 132463212449 — SUCCESS.
- All build steps passed, including Build XY-C1, Verify TRX output, and artifact upload.
- Artifact [14116152532](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235493/artifacts/14116152532): XY-C1-firmware-d3951a276ed4002aff3232cf93c8d55662a94e95, 12,582,629 bytes, SHA-256 74dcad97bba22e3588a4cdac7fa1763da997f9ee43e2e0349237f20ea1b02ff6.
- This verifies the exact d395 product-source tree, not physical-device operation. L4 remains NOT VERIFIED.

## Explicitly uncompleted P1 work

- All 77 direct `pids()` callers remain legacy Boolean users. Migrate by semantic group; never change `pids()` to negative UNKNOWN until every caller is explicit.
- `kill_pidfile_s()` remains unchanged. Strict PID file parser, PID + start_time + expected executable identity validation, and producer/consumer coordination remain open. Producers include detect_internet, detect_link, watchdog, rstats, BusyBox udhcpc, PPPD, INICD. Find the actual writer of `/var/run/lpdparent.pid`; do not guess from filename.
- Upper-layer stop propagation remains open: VPN Server/Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown, firmware `stop_services()`/flash, Samba config publication, storage eject/unmount, NVRAM stopped-state publication.
- Add PPP/PID-file targets to the same full-set preflight before any signal. Audit direct `killall` and signal paths outside `kill_services()`.
- Linux 4.4 has no `pidfd_send_signal()`; PID/start_time validation leaves a check/kill TOCTOU window. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and was not changed.

## Verification boundary

- L1: previous lexical and semantic direct-call review PASS within its documented limits.
- L2: d395 exact-head P1, HTTPD, AI docs, NVRAM, infosvr, AliDDNS, AliyunDrive and AdGuardHome workflows PASS; Watchdog, crond and GPIO PASS on product-compatible predecessor e9ae562. The policy/checker changes in this governance commit require their own exact-head AI handoff docs Action.
- L3: PASS on product-source SHA d3951a276ed4002aff3232cf93c8d55662a94e95; XY-C1 build, TRX verification and artifact upload all succeeded (run 43600235493, artifact 14116152532).
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub. Never ask the user for the whole repository.

## Exact first action for the next GPT

1. User needs only `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. Read complete `AI_DEVELOPMENT_PROTOCOL.md` then `AI_BOOTSTRAP_PROMPT.md`.
2. Perform read-only recovery first: fetch live main and backup; confirm backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; inspect the exact-head Actions for the live SHA. Do not write a commit merely to refresh this handoff or synchronize SHA fields.
3. The product-source baseline d3951a276ed4002aff3232cf93c8d55662a94e95 has L2 process/HTTPD and L3 XY-C1/TRX evidence recorded above. Only inspect newer relevant failures or unrecorded exact-head checks; do not repeat the successful build or completed lexical/semantic census.
4. Resume P1 caller migration. First inspect kill_pidfile_s() and identify the actual writer of /var/run/lpdparent.pid; do not infer producer identity from the filename. Trace PID parsing, producer publication, expected executable, start_time validation, stop callers and failure propagation before editing.
5. Then migrate by semantic group: PPP/PID-file targets into full-set preflight; VPN Server/Client; USB modem DHCP/PPP removal; AP LAN DHCP/bridge teardown; firmware stop_services()/flash; Samba/storage/NVRAM postconditions. Preserve UNKNOWN/CHANGED/FAILED as non-success and add caller-level tests proving destructive follow-up is blocked.
6. Migrate remaining pids() and direct signal paths incrementally, keeping BusyBox's independent procps scanner out of the shared Padavan API work. Linux 4.4 PID/start_time revalidation still has a documented check/kill TOCTOU window.
7. Keep recovery read-only. Follow the documentation decision gate in AI_DEVELOPMENT_PROTOCOL.md; use one consolidated commit per real milestone, and leave this handoff untouched when its facts and first action remain accurate. The one-line user entrypoint remains sufficient.
