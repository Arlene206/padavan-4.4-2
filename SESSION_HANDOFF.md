## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. This corrective milestone is being committed directly on top of verified live parent `6f683b421bd62b4d229a2206a62e239189cd4cb9`; resulting implementation commit is recorded as `THIS COMMIT` in the matching dev-log entry. Always fetch `refs/heads/main` before any write and resolve the actual live SHA; never use this parent as an assumed current tip.
- Immutable archival branch: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. It is read-only and must remain byte-for-byte unchanged.
- **Permanent minimal takeover entrypoint:** the user only needs to say `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. Never ask for a long prompt, prior transcript, source upload or recursive prompt. Never generate a next-GPT prompt in chat. All current instructions and the next exact action must be written into this file in Git.
- Read complete `AI_DEVELOPMENT_PROTOCOL.md` before source edits, then `AI_BOOTSTRAP_PROMPT.md`; then fetch live `main`, backup ref, Actions, this handoff, roadmap, newest 20 dev-log records, INDEX and the specific audit reports required by the active task.
- Legacy PR #1 remains CLOSED / UNMERGED. Do not reopen it or use its head. Do not create development branches or PRs. Do not modify GPT Session Rotator.
- Prior L1 lexical inventory and direct public-API semantic review are complete at their documented source SHAs. Reuse `docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md`; do not repeat the 9,439 tracked blob inventory, 6,193 readable-file scan, 77 direct `pids()` caller census or 46 direct `kill_services()` caller census. The review is not a complete interprocedural graph of all 5,664 broad lifecycle candidates.

## Active task

### P1 phase-2 foundation implemented

- Added `trunk/user/shared/src/process_inventory.h` and `process_inventory.c`: explicit `PROCESS_PRESENCE_RUNNING`, `PROCESS_PRESENCE_ABSENT`, `PROCESS_PRESENCE_UNKNOWN`, separate count/snapshot, PID/start_time/comm/argv fingerprints, injectable directory/file/allocation operations, clean EOF versus readdir error, and all-or-nothing snapshots on UNKNOWN.
- Added `process_inventory.o` to `trunk/user/shared/src/Makefile` (the previous `proc_inventory.o` spelling was wrong; this corrective commit changes it to match the actual `process_inventory.c` source file). The shared Makefile installs all root `.h` files, including the new public header.
- Fixed `/proc/PID/stat` parsing: start_time (field 22) is the 20th token after comm, not token 19. The scanner regression now asserts the concrete test start_time `98765`, not only PID equality. Added a fault-injection case where a PID disappears between the stat and cmdline reads and is confirmed absent by a second stat read.
- `pids_main()` explicitly reports UNKNOWN and returns an error. Legacy `pids()` remains Boolean-compatible until all 77 direct callers are migrated; never return negative UNKNOWN through its old truthiness API.
- Reworked `service_lifecycle_policy.h` and central `kill_services()`: full configured service-set preflight before TERM, a second whole-set validation before the first signal, PID identity validation before each TERM/KILL, direct captured-PID signal instead of name-wide `killall`, and explicit UNKNOWN/CHANGED/TIMEOUT/FAILED results that do not authorize follow-up cleanup/restart/publication.
- Added `trunk/user/shared/src/tests/process_inventory.c`, `trunk/user/rc/src/tests/service_lifecycle_policy.c`, `.github/workflows/process-lifecycle-regression.yml`, and `scripts/check_process_lifecycle_integration.py`.
- Updated Watchdog and HTTPD source-contract checks to assert the new tri-state policy rather than stale Boolean implementation details. Updated root bootstrap/protocol so the user can use only the minimal takeover entrypoint above.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and has not been changed.

### Exact-head validation on parent SHA `6f683b421bd62b4d229a2206a62e239189cd4cb9`

<badge color="success">L2 regression PASS</badge>

- P1 process inventory/lifecycle regression: run `43596037745`, job `132450612501` — all steps PASS: GNU89 `-Wall -Wextra -Werror` scanner build, ASan/UBSan scanner fault-injection run, lifecycle policy build/run, integration boundary contract.
- Watchdog: `43596037803` PASS.
- HTTPD security and service lifecycle/WAN contracts: `43596037779` PASS.
- infosvr lifecycle: `43596037800` PASS.
- AI handoff documentation contract: `43596037878` PASS.
- AdGuardHome: `43596037815` PASS.
- AliDDNS: `43596037860` PASS.
- AliyunDrive WebDAV: `43596037730` PASS.
- NVRAM transaction boundary: `43596037799` PASS, including raw ioctl/kernel API audit.

<badge color="danger">L3 attempt FAILED on parent SHA</badge>

- XY-C1 firmware build run `43596037729`, job `132450613310`, was an actual MIPS/uClibc cross-build attempt. It passed setup, dependency installation and three staged-header contracts, then failed during the shared package build:
  `make[5]: *** No rule to make target 'proc_inventory.o', needed by 'libshared.so'. Stop.`
- Cause: Makefile object spelling did not match `process_inventory.c`. The current corrective commit changes it to `process_inventory.o`. The same code review found a start_time token off-by-one and added a concrete regression assertion.
- No TRX artifact was produced; the workflow's TRX verification and upload steps were skipped. Do not claim L3 success based on this failed run.
- L3 for the corrective commit is PENDING its own exact-head XY-C1 build. Only inspect the new run matching the actual live main SHA.
- L4: XY-C1 physical flash, boot, runtime and functional validation NOT VERIFIED.

## Explicitly uncompleted P1 work

- All 77 direct `pids()` consumers are still unconverted. Group by semantic use (HTTPD/WebUI state, startup readiness, notifications/signals, Samba, watchdog, VPN, USB modem, WAN/LAN, storage, plugins). Do not change `pids()` return type/semantics until every direct caller has explicit tri-state handling.
- `kill_pidfile_s()` is unchanged. Strict PID-file syntax, PID + start_time + expected identity verification, read errors and producer/consumer coordination remain open. Producers include detect_internet, detect_link, watchdog, rstats, BusyBox udhcpc, PPPD and INICD. Locate the actual writer of `/var/run/lpdparent.pid`; do not infer identity from filename.
- Upper-layer failure propagation remains open: VPN Server/Client, USB modem safe removal, AP-mode LAN teardown and `flash_firmware()` / `stop_services()`; also Samba configuration publication, storage eject/unmount, NVRAM stopped-state publication and all dependent cleanup/restart. A failed/UNKNOWN stop must stop the caller transaction, not merely change a return type.
- The current multi-service preflight covers configured service-name targets only; extra PID-file targets (especially PPP client PID files) must be enrolled in the same full-set preflight before any signal.
- Audit direct `killall`/signal paths outside `kill_services()`; a process inventory cannot authorize a later name-wide signal.
- Linux 4.4 has no `pidfd_send_signal()`. PID + start_time/command verification leaves a check-then-kill TOCTOU race. Do not claim atomic identity binding.

## Verification boundary

- L1: previous lexical and direct-call semantic review PASS within its documented scope; no repeated census.
- L2: parent SHA `6f683b421bd62b4d229a2206a62e239189cd4cb9` exact-head workflow evidence listed above PASS. The corrective commit's own exact-head P1 and related tests are pending.
- L3: parent SHA XY-C1 run `43596037729` FAILED at shared object rule; corrective fix is in this commit and needs a new exact-head cross-build. No TRX artifact from the failed run.
- L4: NOT VERIFIED; no actual router flash/boot/runtime evidence.
- The current execution environment has no full local repository checkout; use GitHub and Actions for remote evidence rather than asking the user to upload the repository.

## Exact first action for the next GPT

1. The user needs only this entrypoint: `接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`. Do not ask for more instructions. Read `AI_DEVELOPMENT_PROTOCOL.md` completely, then `AI_BOOTSTRAP_PROMPT.md`.
2. Fetch live `refs/heads/main` and `backup/pre-plugin-overhaul-2026-11-14`; confirm backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never trust a SHA from this handoff without fetching.
3. Inspect every `push` Actions run whose `head_sha` exactly equals current main. First inspect the dedicated P1 regression workflow and XY-C1 firmware workflow. Confirm the object-name fix, the exact start_time assertion and mid-scan disappearance test pass. For XY-C1, inspect every job step, TRX validation and uploaded artifact. A green host job is not L3.
4. If the new MIPS build fails, inspect its first causal compiler/build error and fix it directly on main; do not stop at the host tests. Then obtain complete TRX artifact and checksum/output validation before marking L3.
5. Once the target build blocker is closed, continue the remaining P1 sequence from the semantic audit: first PID-file strict parsing/identity and the actual `lpdparent.pid` producer; then VPN Server stop transaction (include every PPP PID target in preflight), VPN Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown and firmware shutdown propagation. Only a fully confirmed STOPPED result permits cleanup/restart/unmount/flash.
6. Continue migrating all direct `pids()` call sites by semantic group, then direct signal/name-wide kill paths. Keep BusyBox procps separate.
7. Add caller-level tests proving UNKNOWN blocks cleanup, NVRAM state publication, restart, resource teardown and firmware progression. Keep GNU89, `-Wall -Wextra -Werror`, ASan/UBSan and feature guards. Attempt the relevant STORAGE/USB/IPv6/BOARD/APP target configurations after XY-C1.
8. Before every write re-fetch live main; commit directly to main with its exact expected parent. Never create a branch or PR. Preserve the backup ref. Append one substantive log record; update handoff/roadmap only when their state changes. Keep these four heading strings unchanged:
   - `## Current repository state`
   - `## Active task`
   - `## Verification boundary`
   - `## Exact first action for the next GPT`
9. Before ending, make this file the complete current handoff for the following conversation. Do not output a recursive prompt in chat; the same minimal user entrypoint must remain sufficient indefinitely.
