# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Only main is writable; the preserved backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. The last full XY-C1/TRX PASS is older source d3951a276ed4002aff3232cf93c8d55662a94e95 and is not current. Exact-head XY-C1 run 43622638409 on main SHA 733463cb9e3289c62836dd1b7405a30ec1a424cc failed compiling rstats.c:722 because saved_errno was undeclared after the PID migration. Commit dd7a9b42f6f12b1a585e09cb54a1d69c25f69877 fixed that source error; its exact-head build was still in progress when this watchdog slice began. This commit adds watchdog identity binding and triggers another exact-head MIPS build. Fetch live main before every write.
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

### In-tree PID-file identity helper — partially migrated; watchdog now included

- Added process_pidfile.c/.h to the shared library: strict PID + start_time record parsing, expected-process inventory matching, identity revalidation immediately before signaling, and atomic same-directory temporary-file publication. Its parser avoids variable 64-bit division, so the MIPS shared object does not require hidden __udivdi3.
- Identity-bound publication/signaling now covers detect_internet (producer, two RC paths, HTTPD SIGHUP), detect_link (producer and both RC paths), rstats (producer, RC SIGHUP, HTTPD refresh/history) and watchdog (producer, RC time refresh and Wi-Fi notification). LPD has its own strict parser and identity-validated consumer.
- Watchdog publishes PID + start_time only after daemonization and fails startup closed if publication fails. Both RC notification callers use process_pidfile_signal() with expected name watchdog. Time refresh launches a replacement only when the legacy pids() guard confirms absence; UNKNOWN remains truthy and cannot create a duplicate. Shutdown deliberately retains the last record rather than unlinking by pathname, which could remove a fast-restarted watchdog's new record. Stale PID/start_time records are rejected by the helper.
- The rstats undeclared saved_errno compile error on 733 was fixed in dd7a9b42f6f12b1a585e09cb54a1d69c25f69877. Its exact-head host/integration runs passed; its XY-C1 run was still in progress when this commit was prepared. Watchdog's exact-head signal regression and new target build are now pending.
- Remaining PID producers include BusyBox udhcpc, PPPD and INICD; next broader process task is to include PPP/PID-file targets in the all-target stop preflight.

### Exact-head Actions on main SHA dd7a9b42f6f12b1a585e09cb54a1d69c25f69877

- Process inventory/lifecycle regression [43624870654] — SUCCESS.
- Process lifecycle inventory [43624870697] — SUCCESS.
- rstats persistence/signal regression [43624870719] — SUCCESS.
- HTTPD security [43624870704] and AI handoff docs [43624870860] — SUCCESS.
- XY-C1 Firmware Validation [43624870668] — IN_PROGRESS at handoff update; inspect the final Build XY-C1, TRX and artifact result. This commit changes watchdog.c and extends the XY-C1 path trigger, producing a new exact-head build as well as a watchdog-signal regression run.

### XY-C1 MIPS/uClibc target build — current HEAD NOT YET VERIFIED

- The last full successful TRX verification/artifact upload remains run [43600235493](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43600235493) on older product-source SHA d3951a276ed4002aff3232cf93c8d55662a94e95. Do not infer current L3 from this ancestor.
- The rstats compile defect discovered on 733 is fixed on dd7a9b42f6f12b1a585e09cb54a1d69c25f69877; the dd7a target build was still in progress, and watchdog's exact-head build will validate the complete migrated RC source.
- L4 remains NOT VERIFIED; no physical XY-C1 flash/boot/runtime claim.

## Explicitly uncompleted P1 work

- All 77 direct `pids()` callers remain legacy Boolean users. Migrate by semantic group; never change `pids()` to negative UNKNOWN until every caller is explicit.
- LPD PID publication uses exclusive same-directory temporary-file creation and atomic rename after a complete checked write; its strict identity-aware duplicate consumer treats malformed/unreadable records as UNKNOWN. Publication follows successful socket bind; stop does not unlink because that could delete a replacement daemon's newly published record. Shared identity-bound PID publication/signaling now covers detect_internet, detect_link, rstats and watchdog. Remaining PID producers include BusyBox udhcpc, PPPD and INICD.
- Upper-layer stop propagation remains open: VPN Server/Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown, firmware `stop_services()`/flash, Samba config publication, storage eject/unmount, NVRAM stopped-state publication.
- Add PPP/PID-file targets to the same full-set preflight before any signal. Audit direct `killall` and signal paths outside `kill_services()`.
- Linux 4.4 has no `pidfd_send_signal()`; PID/start_time validation leaves a check/kill TOCTOU window. Do not claim atomic identity binding.
- BusyBox `trunk/user/busybox/busybox-1.24.x/libbb/procps.c` remains a separate scanner and was not changed.

## Verification boundary

- L1: prior lexical inventory and direct-call semantic review PASS within the recorded scope; do not repeat completed inventories.
- L2: exact dd7a SHA process inventory [43624870654], process lifecycle inventory [43624870697], rstats [43624870719], HTTPD [43624870704] and AI docs [43624870860] passed. The new watchdog-signal regression and current source/target Actions for this commit are pending.
- L3: current main NOT VERIFIED. The last full XY-C1/TRX PASS is older product-source d395 [43600235493]. The dd7a target build [43624870668] was in progress when this commit began; this commit triggers another exact-head target build that includes watchdog.c.
- L4: physical XY-C1 flash, boot, runtime and functional hardware validation NOT VERIFIED.
- No full local checkout is available in this execution context; source and CI evidence are obtained through GitHub.

## Exact first action for the next GPT

1. Recover live main and the immutable backup read-only; inspect push Actions using the exact current main SHA. The one-line repository prompt is sufficient; do not request a repo upload or create a takeover commit.
2. Verify the exact-head watchdog-signal regression and XY-C1 build triggered by this commit. Also inspect the prior dd7a XY-C1 run 43624870668; its state was IN_PROGRESS at the last check. The rstats compile error from 733 was fixed on dd7a and its exact-head process, inventory, rstats, HTTPD and AI-doc checks passed.
3. When current-head CI passes, continue P1: add PPP/PID-file targets to the same all-target preflight before any signal. Then migrate VPN Server/Client, USB modem DHCP/PPP removal, AP LAN DHCP/bridge teardown, firmware stop_services()/flash, Samba/storage/NVRAM postconditions. Add caller-level regressions blocking destructive follow-up on UNKNOWN/CHANGED/FAILED.
4. Keep pids() Boolean-compatible until all 77 direct callers are migrated/adapted. Keep kill_pidfile_s() semantics unchanged until its five direct callers and all 15 call expressions are migrated/adapted. BusyBox's independent procps implementation stays separate.
5. Linux 4.4 PID/start_time validation retains a check/kill TOCTOU window; do not claim pidfd-equivalent atomic signaling or L4 hardware validation without evidence.
6. Continue one complete Git tree/commit with a non-forced expected-SHA main update for every multi-file slice. No per-file commits, force pushes, SHA-refresh commits or standalone CI-poll commits.
