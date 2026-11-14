# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Only main is writable; the preserved backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. The last full XY-C1/TRX PASS is older source d3951a276ed4002aff3232cf93c8d55662a94e95 and is not current. Exact-head XY-C1 run 43622638409 on main SHA 733463cb9e3289c62836dd1b7405a30ec1a424cc failed compiling rstats.c:722 because saved_errno was undeclared after the PID migration. This commit fixes that source error; its exact-head rebuild must pass before advancing to watchdog migration. Fetch live main before every write.
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

### In-tree PID-file identity helper — partially migrated; target build correction pending

- Added process_pidfile.c/.h to the shared library: strict PID + start_time record parsing, expected-process inventory matching, identity revalidation immediately before signaling, and atomic same-directory temporary-file publication. The parser avoids variable 64-bit division so the MIPS shared object does not require hidden __udivdi3.
- Identity-bound publication/signaling is implemented for detect_internet (producer, two RC paths, HTTPD SIGHUP), detect_link (producer and both RC paths), and rstats (producer, RC SIGHUP, HTTPD refresh/history). LPD has its own strict parser and identity-validated consumer.
- Exact-head XY-C1 run 43622638409 failed compiling rstats.c:722: saved_errno was declared only inside earlier nested error blocks but was reused in PID-publication failure handling. This commit moves it to function scope and reuses it across the error paths. Inspect the fresh exact-head target build; do not infer L3 from helper-only or ancestor builds.
- Next in-tree daemon migration after the build passes: watchdog PID publisher and its time/Wi-Fi notification callers. BusyBox udhcpc, PPPD and INICD remain separate PID producers.

### Exact-head Actions on main SHA 733463cb9e3289c62836dd1b7405a30ec1a424cc

- AI handoff docs run 43622638346 — SUCCESS.
- Process inventory/lifecycle regression run 43622638410 — SUCCESS.
- HTTPD Authentication and Security Regression run 43622638413 — SUCCESS, including the updated rstats process-identity assertion.
- NVRAM transaction boundary, AliDDNS, AliyunDrive and AdGuardHome workflows also completed SUCCESS on this SHA.
- XY-C1 Firmware Validation run 43622638409, job 132531489683 — FAILURE at rstats.c:722: saved_errno undeclared. Setup and pre-build contracts passed; Build XY-C1 stopped while compiling RC, so TRX verification and artifact upload were skipped. This is a source compile error, not an infrastructure failure. The current commit fixes it and requires a fresh exact-head run.

### XY-C1 MIPS/uClibc target build — current HEAD NOT VERIFIED

- The last full successful build/TRX verification/artifact upload remains run 43600235493 on older product-source SHA d3951a276ed4002aff3232cf93c8d55662a94e95, artifact 14116152532 (12,582,629 bytes, SHA-256 74dcad97bba22e3588a4cdac7fa1763da997f9ee43e2e0349237f20ea1b02ff6).
- Later PID-helper caller migrations are not covered by that older TRX. The exact 733 build above failed before image generation. Do not claim L3 for current main until a corrected exact-head build completes Build XY-C1, TRX verification and artifact upload.
- L4 remains NOT VERIFIED; no physical XY-C1 flash/boot/runtime claim.

## Explicitly uncompleted P1 work

- All 77 direct `pids()` callers remain legacy Boolean users. Migrate by semantic group; never change `pids()` to negative UNKNOWN until every caller is explicit.
- LPD PID publication writes through an exclusive same-directory temporary file and atomically renames only after a complete checked write; startup fails closed on publication errors. Its duplicate-instance consumer strictly parses decimal PID text, treats missing as stale/absent but malformed or unreadable as UNKNOWN, scans the process inventory, and validates PID/start_time/comm/argv before refusing a duplicate. Publication follows successful socket bind; stop does not unlink because that can delete a replacement daemon's newly published identity. detect_internet, detect_link and rstats now also use the shared PID+start_time helper. The watchdog migration remains next; BusyBox udhcpc, PPPD and INICD remain separate PID producers.
- Upper-layer stop propagation remains open: VPN Server/Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown, firmware `stop_services()`/flash, Samba config publication, storage eject/unmount, NVRAM stopped-state publication.
- Add PPP/PID-file targets to the same full-set preflight before any signal. Audit direct `killall` and signal paths outside `kill_services()`.
- Linux 4.4 has no `pidfd_send_signal()`; PID/start_time validation leaves a check/kill TOCTOU window. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and was not changed.

## Verification boundary

- L1: prior lexical inventory and direct-call semantic review PASS within their recorded scope. Do not repeat the completed census.
- L2: on exact SHA 733463cb9e3289c62836dd1b7405a30ec1a424cc, process-lifecycle run 43622638410, HTTPD run 43622638413 and AI-doc run 43622638346 PASS. The rstats compile correction in this current commit has fresh exact-head validation pending.
- L3: current source NOT VERIFIED. The last full XY-C1 MIPS/uClibc/TRX PASS is older source d3951a276ed4002aff3232cf93c8d55662a94e95 (run 43600235493); exact 733 build 43622638409 failed compiling rstats before TRX generation. Re-run after this fix and do not infer L3 from an ancestor/helper-only build.
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub. Never ask the user for the whole repository.

## Exact first action for the next GPT

1. Recover read-only from GitHub: read this protocol/bootstrap, fetch live main and the immutable backup, compare refs, and inspect push Actions by exact head SHA. The one-line repository prompt is the only user entrypoint.
2. Verify the rstats compile correction in this commit. Inspect the exact-main XY-C1 run through Build XY-C1, TRX verification and artifact upload; also inspect exact-head process-lifecycle and AI-doc runs. The known failure to close is run 43622638409 at rstats.c:722 (saved_errno undeclared); do not repeat unrelated subsystem audits.
3. Once rstats and current target validation pass, migrate watchdog's PID writer and time/Wi-Fi notification callers to process_pidfile_write/signal(). Keep UNKNOWN fail-closed. Do not unlink the PID pathname on shutdown because a fast restart can publish a replacement record.
4. Then include PPP/PID-file targets in full-set preflight; migrate VPN Server/Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown, firmware stop_services()/flash, Samba/storage/NVRAM postconditions, and remaining direct signal/pids callers by semantic groups. Preserve UNKNOWN/CHANGED/FAILED as non-success.
5. Do not change pids() UNKNOWN semantics until all 77 direct callers are migrated or explicitly adapted. Do not change shared kill_pidfile_s() until its five direct callers and all 15 call expressions are migrated/adapted. Keep BusyBox's independent procps scanner separate.
6. Linux 4.4 PID/start_time validation retains a check/kill TOCTOU window. Do not claim pidfd-equivalent identity binding or L4 hardware validation without evidence.
7. Maintain one atomic Git tree/CAS commit per real slice. Update only materially changed canonical records; never create SHA-refresh or CI-poll-only commits.
