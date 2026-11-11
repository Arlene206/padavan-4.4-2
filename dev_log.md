## 2026-11-12 01:16 CST (UTC+8) — Correct DNS Forwarder CWD regression fixture line endings

- Base main SHA: `e6801775fc3af56dd084321eca9376899b2e7d76`; parent `31578abaefe5b2838b7c825460e21e730176b563`.
- Exact-head DNS Forwarder workflow [43066299135] failed during its temporary executable test. The old relative slash-containing path correctly returned ENOENT from the unrelated CWD, but the subsequent absolute-path run also returned ENOENT because the fixture wrote literal `\\n` sequences into the fake shell script instead of actual line terminators.
- AI handoff docs [43066299302] passed on this base (root log=25/119; archives=7/22/18). Networkmap [43066300888], GPIO [43066298902], VPN server [43066299119], WAN ARP [43066298966], HTTPD lifecycle [43066299031], HTTPD security [43066299164], detect_link [43066299084], infosvr [43066298895], watchdog [43066298861] and crond [43066299440] passed. SkipDBv2 [43066299131] remained in progress at last inspection.
- THIS COMMIT makes the test fixture write a valid executable with actual newline terminators and checks the marker as `start`. The production RC absolute path, relative-path failure assertion, source/installer contracts and workflow filters remain unchanged.
- This is a regression-test harness correction; no product-code failure was demonstrated. Exact-head DNS Forwarder and docs reruns remain required before L2 closure.
- Local checkout is unavailable. The corrected script was reviewed, and its temporary shell fixture behavior was independently exercised with an equivalent host snippet (PASS); the complete repository checker is not claimed locally.
- Root `dev_log.md` moves 25→26 complete entries; no rotation or INDEX update is due. Archive shards/indexes remain unchanged.
- Next GPT: inspect corrected DNS Forwarder and AI docs Actions on the resulting exact main SHA, all steps/full logs. Require relative CWD failure, absolute execution success, RC/ROMFS/execvp contracts, ten symmetric exact-once filters and docs root log=26/119, archives=7/22/18. Production MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:11 CST (UTC+8) — Make DNS Forwarder startup path CWD-independent

- Base main SHA: `31578abaefe5b2838b7c825460e21e730176b563`; parent `54504272e2c4ca0f7783ba78aa52a2bfe212de1c`. Main is the only branch; no remote checkpoint.
- Closed GPIO short-press timing L1/L2 on exact SHA `31578abaefe5b2838b7c825460e21e730176b563`: GPIO [43065042288], AI docs [43065042295] and infosvr [43065042298] all SUCCESS. Startup/timing GNU89 ASan/UBSan binaries and source/caller/path contracts passed; docs validator reported root log=24/119 and archive inventory=7/22/18.
- Audited DNS Forwarder service startup: `start_dnsforwarder()` passed `usr/bin/dns-forwarder.sh`, but ROMFS installs the launcher as `/usr/bin/dns-forwarder.sh`. The slash-containing relative pathname is resolved by `execvp()` from CWD, not via PATH; non-root CWD callers therefore receive ENOENT. PID 1's normal `chdir("/")` currently masks the dependency in known boot flow.
- THIS COMMIT changes the RC start invocation to the absolute installed path and adds `scripts/check_dns_forwarder_launch.py` plus `.github/workflows/dns-forwarder-launch-path.yml`. The regression proves relative-path failure/absolute-path success from an unrelated CWD and checks service start/stop/restart wiring, ROMFS destination, `eval()`/`execvp()` use and ten exact-once symmetric workflow filters.
- Local repository checkout is unavailable; the new Python regression/source contract is NOT RUN locally. Exact-head DNS Forwarder and AI docs Actions are required for L2.
- Root `dev_log.md` contained 24 complete records before this entry and contains 25 now; no rotation or archive INDEX update is due. Archive shards/indexes remain unchanged.
- Next GPT: fetch resulting live `main`; inspect exact-head DNS Forwarder and AI docs Actions, all job steps and full logs. Require CWD regression, source/ROMFS/caller checks, symmetric path filters and docs validator (root log=25/119, archives=7/22/18) to pass. On green, close DNS Forwarder L2 and rotate to another whole-userland stability defect. Production MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-11 — Fix GPIO timing workflow path-filter symmetry

- Base main SHA: `54504272e2c4ca0f7783ba78aa52a2bfe212de1c`; parent `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`. Main was rechecked before this commit; only main exists.
- Exact-head GPIO workflow [43064587271] compiled and ran both GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` binaries successfully: daemon startup policy PASS and hold-timing threshold policy PASS. Its source checker then failed at `push path count: trunk/user/rc/src/gpio_btn_timing_policy.h`.
- Root cause: an earlier string replacement matched the old path pair inside its expanded replacement before reaching the `pull_request` block. As a result, both new timing paths appeared twice in push and zero times in pull_request; the checker correctly rejected this.
- THIS COMMIT normalizes both workflow event-filter sections explicitly, placing each timing header/test path exactly once in push and once in pull_request. The source checker keeps strict count assertions.
- AI docs [43064587290] passed on the base (root log=23/119, archive counts dev-log=7, handoff=22, roadmap=18); infosvr [43064587210] also passed. Corrected exact-head GPIO/docs Actions remain pending.
- Root dev_log grows 23→24 records, below the 120-entry rotation threshold; archive index/shards remain unchanged.
- Validation boundary: exact-parent local/CI startup and timing sanitizer binaries PASS; no exact-head source-contract pass yet. MIPS/uClibc, full firmware/TRX, live GPIO/LED/uevent and L4 remain NOT VERIFIED. FTP cleanup TOCTOU remains OPEN/BLOCKED; NFS evidence-gated.
- Next GPT: inspect exact-head corrected GPIO and AI docs Actions, all steps/full logs. Require both binaries, source integration, exact-once symmetric filters and docs validator (root log=24/119, archives 7/22/18) to pass; then close L2 and move across the whole-userland ledger.

---

## 2026-11-11 — Correct the GPIO short-press 800 ms boundary

- Base main SHA: `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`; parent `d002eba06a914d75ac79b20867dbd3cdb6f416ca`. Main was rechecked before creating this commit; it is the only remote branch.
- Closed the resumed HTTPD daemon lifecycle task on the base SHA. HTTPD lifecycle [43063659893], HTTPD security umbrella [43063082646], AI handoff docs [43063659714] and infosvr signal/packet regression [43063659718] all SUCCESS. Dedicated HTTPD job 6/6 steps, umbrella 41/41 and docs 7/7; source contract printed PASS. Docs validator reported root log=22/119 and archives dev-log=7, handoff=22, roadmap=18.
- Found a separate front-panel timing defect in `trunk/user/rc/src/gpio_btn.c`: the initial press count is 1 and updates in 100 ms quanta (`floor(elapsed_ms / 100) + 1`). Thus 700–799 ms maps to count 8, but `btn_released_handle()` required `count < CANCEL_COUNT`, with `CANCEL_COUNT=8` described as 800 ms. The entire 700–799 ms WPS/FN short-action interval was suppressed.
- THIS COMMIT centralizes elapsed monotonic-time conversion and short/long/reset threshold predicates in `gpio_btn_timing_policy.h`. Short actions include counts 1–8 and exclude count 9 at 800 ms; >3 s long and >5 s reset thresholds are preserved. Receive timeout and hold-count accounting share the same 100 ms constant.
- New GNU89 regression covers 0/699/700/799/800 ms, 2999/3000 ms, 4999/5000 ms, negative-delta normalization and `INT_MAX` saturation. Local `-Wall -Wextra -Werror -fsanitize=address,undefined` with leak detection passed: `GPIO button hold timing and threshold policy: PASS`.
- Existing GPIO lifecycle Actions now compile/run both startup and timing policy binaries and statically check source integration, threshold boundaries, and symmetric push/pull_request path filters. Exact-head GPIO and AI docs Actions for THIS COMMIT are pending.
- Changed: `gpio_btn.c`, new timing policy header/test, `.github/workflows/gpio-btn-lifecycle.yml`, `scripts/check_gpio_btn_lifecycle.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and root `dev_log.md`. Root log grows 22→23 entries; no archive rotation/index update due.
- Validation boundary: L1 source review and isolated L2 GNU89 ASan/UBSan timing policy pass locally. No target MIPS/uClibc object, full firmware/TRX, actual key/LED/uevent or L4 router result is claimed. FTP cleanup TOCTOU remains OPEN/BLOCKED and NFS remains external-evidence gated.
- Next GPT: fetch the resulting main SHA; inspect exact-head GPIO startup/timing and AI docs Actions including every step/full logs. Require 700/799 ms inclusion, 800 ms exclusion, unchanged 3 s/5 s limits, source integration, both binaries, path filters and docs validator (root 23/119, archives 7/22/18) to PASS. Then close this L2 slice and rotate to a new whole-userland issue.

---

## 2026-11-12 01:10 CST (UTC+8) — Correct HTTPD lifecycle source-contract publication ordering

- Base main SHA: `d002eba06a914d75ac79b20867dbd3cdb6f416ca`; parent `1c4b9fe85df1ea2a8d9ec83035ea6cc295059b01`. Main was rechecked before editing; branch inventory remains main only.
- Exact-head HTTPD run [43063082671] on this SHA compiled the GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy binary and printed `HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS`. The Python integration checker then failed at its start-state ordering assertion: it compared the post-probe state publication against the first `httpd_started=1`, which is the valid pre-existing-daemon branch. This was a checker false positive, not a failed policy regression.
- Exact-head AI handoff validation [43063082981] passed and printed `AI handoff documentation check: PASS (root log=21/119 entries, batch=100, retain=20, legacy/current archives: dev-log=7, handoff=22, roadmap=18)`.
- THIS COMMIT changes the checker to locate the successful-start `httpd_started=1` only after the detached-child probe loop begins, then assert it follows the successful process-presence branch and precedes firewall restart. The production HTTPD source and policy regression are unchanged in this corrective slice.
- Exact-head HTTPD lifecycle and AI docs Actions for THIS COMMIT are pending. Do not claim the source contract as passing until the corrected workflow completes on the new SHA.
- Root `dev_log.md` grows from 21 to 22 records, well below the rotation threshold. Existing archive shards/index remain unchanged (dev-log=7, handoff=22, roadmap=18).
- The previous SHA already contains the HTTPD launch/stop/restart fix and duplicate-launch preflight. Production MIPS/uClibc, full firmware/TRX, target HTTPD socket readiness, cross-process simultaneous-start locking and L4 remain NOT VERIFIED. FTP cleanup TOCTOU remains OPEN/BLOCKED.

## 2026-11-12 00:55 CST (UTC+8) — Prevent duplicate HTTPD launches on an already-running daemon

- Base main SHA: `1c4b9fe85df1ea2a8d9ec83035ea6cc295059b01`; parent `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`. The branch ref was rechecked before editing; main remains the only branch.
- Follow-up review of the HTTPD lifecycle fix found that a direct repeated `start_httpd()` call could still launch a second copy even though restart callers now stop first. The service entry point now checks `pids("httpd")` before `_eval()`: an existing daemon is marked started and returns `EBUSY`; a stale started flag is cleared only when process absence has been confirmed.
- Added `httpd_start_can_launch()` and regression cases for absent versus already-present processes. The GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` test printed `HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS`.
- Updated the dedicated HTTPD workflow's source contract to require the pre-launch guard and state-reset ordering. Exact-head HTTPD and AI handoff Actions are pending; the preceding SHA's corresponding runs were queued at last inspection and are not claimed as passed.
- SESSION_HANDOFF and REFACTOR_ROADMAP were refreshed in place with the exact active task, verification boundary and next-GPT actions. Root `dev_log.md` grows from 20 to 21 records, below its 119-entry rotation threshold; existing archive parts remain unchanged.
- The PID 1 mdev fsync task remains L1/L2 CLOSED on exact SHA `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`. FTP cleanup TOCTOU remains OPEN/BLOCKED; production MIPS/uClibc objects, complete firmware/TRX and L4 router behavior remain NOT VERIFIED.

## 2026-11-12 00:20 CST (UTC+8) — Gate HTTPD service state on daemon lifecycle postconditions

- Base main SHA: `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`; parent `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`. Main was rechecked before preparing this commit; the branch inventory remains main only.
- Previous PID 1 mdev fsync task is L1/L2 CLOSED on exact SHA `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`: mdev [43055641071], infosvr [43055640661] and AI handoff docs [43055640617] all completed SUCCESS. The two-injected-EINTR plus real-fsync GNU89 ASan/UBSan regression and source contracts passed.
- New P2 finding in `trunk/user/rc/src/services.c`: `start_httpd()` discarded `_eval()` status and unconditionally published `httpd_started=1`; this could advertise a service and open firewall ports after a failed launch or a daemon that immediately exited. `stop_httpd()` cleared the started flag before checking bounded `kill_services()` completion and ignored failure, while `restart_httpd()` launched regardless. A surviving prior daemon could therefore be hidden and duplicated.
- THIS COMMIT changes the HTTPD start/stop/restart APIs to return status. Start preserves launcher errors and polls for a live detached `httpd` process before publishing the flag or reopening firewall access. Stop only clears state after the shared lifecycle helper confirms process absence. Restart refuses to launch if stop fails.
- New `httpd_daemon_policy.h` and `tests/httpd_daemon_policy.c` cover launcher failure, child-presence postcondition, stop-state publication and restart gating. The local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` regression printed `HTTPD daemon startup, stop-state and restart policy: PASS`. A dedicated path-filtered workflow adds service integration and RC declaration source contracts; exact-head workflow and AI-doc Actions are pending.
- Root `dev_log.md` reached 120 complete records including this entry. Per the 120-record rotation rule, the oldest 100 records are archived verbatim, oldest first, in `docs/dev-log/2026/11/part-007.md`; root retains the newest 20. Existing archive parts 001–006 are unchanged and the index is updated in this commit.
- FTP upload-cleanup TOCTOU remains P2 OPEN/BLOCKED until a Linux 4.4-safe design proves inode/parent safety while preserving STOR/APPE/REST/STOU semantics. Production MIPS/uClibc objects, complete firmware/TRX, target HTTPD socket readiness and L4 router behavior remain NOT VERIFIED.

## 2026-11-11 — Close firewall snapshot writer L2 and retry interrupted PID 1 mdev fsync

- Base main SHA: `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`; parent `42800e0caaf90e0230f4e30d268e8301d0da2d70`. Branch inventory remains main only, without a remote checkpoint.
- Firewall descriptor-bound snapshot task is L1/L2 CLOSED on exact SHA `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`: HTTPD [43053436665], infosvr [43053436537] and AI handoff docs [43053436425] all completed SUCCESS. The new descriptor-inheritance GNU89 ASan/UBSan regression printed PASS; transaction/status/startup binaries and corrected push/pull workflow path assertions passed; docs validator reported root log=118/119, archive counts dev-log=6, handoff=22, roadmap=18.
- New PID 1 mdev stability finding: `init_mdev_config_publish()` treated a single `fsync()` EINTR as a permanent failure, deleted the staged policy, and caused `init_mdev()` to return before kernel hotplug activation. A transient signal could leave this boot without the intended /etc/mdev.conf hotplug rules.
- THIS COMMIT retries file `fsync()` only for EINTR; other failures still fail closed. The host regression injects two EINTR returns before the real `fsync()`, requiring success after exactly three calls. Existing 0644 mode, old-policy preservation, atomic rename, and failed-publication cleanup checks remain.
- Local GNU89 `-Wall -Wextra -Werror` ASan/UBSan regression with leak detection passed: private staging + atomic publication + EINTR-safe fsync.
- Exact-head PID 1 mdev workflow and AI docs validator are pending on the resulting SHA; no repository Actions pass is claimed yet.
- Scope: `init_mdev_config_file.h`, `tests/init_mdev_config_file.c`, `.github/workflows/init-mdev-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Root log moves from 118 to 119 entries; archive rotation/index update is not due.
- Target boundary: production MIPS/uClibc, full firmware/TRX, BusyBox mdev/ash target and actual USB/storage/network hotplug L4 remain NOT VERIFIED. FTP cleanup TOCTOU stays OPEN/BLOCKED.
- Next GPT: inspect exact-resulting-SHA mdev security and AI docs Actions in full; require the injected EINTR regression and source/path checks to pass, then rotate to another userland subsystem.

## 2026-11-11 — Correct firewall CI path-filter contract after exact-head regression

- Base main SHA: `42800e0caaf90e0230f4e30d268e8301d0da2d70`; parent `02907c582c716a49c6255d2c06cbad429f5a3260`. Main ref was rechecked before editing.
- Exact-head FULLCONENAT [43052866763] and infosvr [43052866751] both completed SUCCESS on `42800e0caaf90e0230f4e30d268e8301d0da2d70`. HTTPD umbrella [43052866810] completed FAILURE at its Python source contract. The new `firewall_snapshot_fd.c` binary itself printed `firewall snapshot descriptor writer inheritance: PASS`; command-status, startup-policy and firewall transaction regression binaries preceding the checker ran without reported failure.
- Failure cause: `workflow.count('"trunk/user/rc/src/tests/firewall_snapshot_fd.c"') == 2` counted two push/PR path entries plus the checker assertion's own quoted path literal, so actual count was three. This was a test-contract defect, not a production firewall writer failure.
- THIS COMMIT replaces the self-referential count with independent parsing/assertions for the push and pull_request path blocks and separately confirms the firewall regression step references the test source. Production `firewall_ex.c` and the C regression are unchanged.
- The AI handoff docs run [43052866839] was still in progress when this correction was prepared. All exact-head jobs for the resulting SHA must be inspected before closing the task.
- Scope: `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Root log moves from 117 to 118 complete newest-first records; no archive rotation or index update is due. This is a CI-contract correction attached to the in-flight code task, not a standalone docs-only commit.
- Target boundary: L3 MIPS/uClibc, complete firmware/TRX and L4 remain NOT VERIFIED. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED pending a proved Linux 4.4-safe design.
- Next GPT: verify the new main SHA's HTTPD and AI docs Actions in full; require the corrected path contract and descriptor-writer regression to pass, then rotate to a different userland subsystem.

## 2026-11-11 — Close BusyBox ash lifecycle gate and bind firewall snapshots to their staged descriptor

- Base main SHA: `02907c582c716a49c6255d2c06cbad429f5a3260`; parent `21badf5dab5bc73f2768ccdbc381dcc70f6dcc0d`. Branch inventory: `main` only, no remote checkpoint.
- Exact-head Actions on the base SHA: storage [43042263807], infosvr [43042263817] and AI handoff docs [43042263966] all completed SUCCESS; all job steps and complete logs inspected. Storage built/linked repository BusyBox 1.24.2 and passed lock contention/release, teardown status, startup supervision/rollback, eject inventory, deferred Optware child-result/retry, daemon-stop postcondition and whole caller-graph/lock-order checks (steps 9–13). This closes the prior active handoff task at L1/L2 only.
- New firewall stability/security finding: `firewall_snapshot_core_table()` closed the mode-0600 `mkstemp()` descriptor before invoking `iptables-save` with shell redirection to the pathname. Reopening that shared-`/tmp` name loses the guarantee that the writer fills the exact private inode created by `mkstemp()`.
- THIS COMMIT keeps the descriptor open across `doSystem()`, explicitly clears `FD_CLOEXEC` for the system(3) shell, redirects stdout with `>&fd`, checks the writer's final close, and retains existing cleanup/restore path handling. A new GNU89 ASan/UBSan regression removes the staged pathname before shell output, then verifies bytes are written to the still-open inode. HTTPD workflow push/PR filters, compile/run step and source contract are extended accordingly.
- Local host probe for the exact descriptor-inheritance/redirection behavior passed under GNU89 with AddressSanitizer and UndefinedBehaviorSanitizer. The repository-level Actions test and docs validator are pending on the resulting commit SHA; no result is claimed before those runs complete.
- Relevant pre-existing firewall transaction cleanup test and HTTPD source contract passed in run [43009513122] at ancestor SHA `d2edf2dd08c85e0413f6469e2dccc6fa5a2df26a`; the transaction header, test, firewall integration and workflow blobs were confirmed byte-identical at base SHA `02907c582c716a49c6255d2c06cbad429f5a3260`.
- Scope: `firewall_ex.c`, new `tests/firewall_snapshot_fd.c`, `.github/workflows/httpd-security.yml`, and the live session/roadmap/log records. Root log moves from 116 to 117 records; no archive rotation or index update is due.
- Target boundary: production MIPS/uClibc, complete firmware/TRX, actual router /tmp semantics and L4 firewall/storage behavior remain NOT VERIFIED. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED pending a proved Linux 4.4-safe design.
- Next GPT: fetch resulting main SHA; inspect exact-head HTTPD and AI handoff-docs runs in full. Require the new descriptor-writer GNU89 ASan/UBSan regression, source contract and path filters to pass. Then close this bounded slice at L2 and rotate to a different evidence-actionable userland module.

## 2026-11-11 22:47 CST (UTC+8) — Use BusyBox 1.24.2-compatible integer sleep in the lock regression

- Base main SHA: `21badf5dab5bc73f2768ccdbc381dcc70f6dcc0d`; parent `abff1ed57217c1fe482bf80c2c419dde66437bf0`; remote branch inventory: `main` only, no checkpoint.
- Exact-head Actions: infosvr [43041511033] SUCCESS and AI handoff docs [43041510759] SUCCESS; storage [43041510879] FAILURE. The documentation check reported root log=115/119 and archives dev-log=6, handoff=22, roadmap=18.
- Storage steps 3–8 passed. Step 9 built and linked BusyBox 1.24.2; direct ash/awk execution, descriptor-backed `flock -n 9` / `flock -u 9` smoke and the production Optware script ash syntax checks passed. Removing the unsupported `command -v` preflight allowed the lock regression to advance further.
- New exact failure: `sleep: invalid number '0.1'` from `optware_lifecycle_lock.sh`. The script polls for the nonblocking lock result at 100 ms intervals, but the firmware-matched BusyBox 1.24.2 sleep applet accepts integer intervals. The POSIX shell version of the same test passed earlier because it used the host sleep implementation; the matching BusyBox regression correctly exposed the difference.
- Fix in THIS COMMIT: use `sleep 1` and reduce the retry ceiling from 50 to 5, keeping the wait bounded at about four seconds while remaining compatible with the actual firmware applet. No production lifecycle code or tracked BusyBox config changed.
- Validation boundary: L1 compatibility diagnosis PASS. On the base SHA, BusyBox 1.24.2 build/link, direct applet smoke, syntax checks and the three preceding POSIX-shell regressions passed. The under-ash lock test reached the polling loop but did not finish; teardown/startup under ash and storage steps 10–13 were skipped. Exact-head storage/docs reruns for THIS COMMIT are pending. No local checkout, target MIPS/uClibc build, full firmware/TRX or L4 router test is claimed.
- Changed paths: `trunk/user/scripts/tests/optware_lifecycle_lock.sh`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev_log.md had 115 complete newest-first records before this entry and has 116 afterward; archives remain dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact resulting main SHA's storage, infosvr and AI docs Actions. Require all under-ash Optware lifecycle regressions and storage steps 10–13 to pass; keep production MIPS/uClibc, firmware/TRX and L4 NOT VERIFIED.

---

## 2026-11-11 22:43 CST (UTC+8) — Remove optional ash command-builtin dependency from Optware lock regression

- Base main SHA: `abff1ed57217c1fe482bf80c2c419dde66437bf0`; parent `c4afed88518d2c4a9dc8c4e4dc2d0f2f129996f4`; remote branches: `main` only, no checkpoint.
- Exact-head Actions reviewed on the base: udhcpc [43040638862] SUCCESS; infosvr [43040638851] SUCCESS; AI handoff docs [43040638793] SUCCESS; storage [43040639192] FAILURE. The udhcpc GNU89 ASan/UBSan binary and corrected source contract passed; infosvr packet/SIGTERM sanitizer binaries and source contract passed; docs validator reported root log=114/119 and archive inventory dev-log=6, handoff=22, roadmap=18.
- Storage steps 3–8 passed. Step 9 successfully built/linked BusyBox 1.24.2; direct ash and awk calls, descriptor-backed `flock -n 9` / `flock -u 9` smoke, and all production-script ash syntax checks passed. The first lock regression then failed at its preflight `command -v flock`.
- Root cause: the exact firmware-derived BusyBox config disables `CONFIG_ASH_CMDCMD`; the oldconfig transcript confirms the optional ash `command` builtin was left unset. The regression harness incorrectly depended on `command -v`, even though the actual lock helper and the preceding binary smoke exercise the real flock applet. No production lifecycle function was implicated.
- Fix in THIS COMMIT: remove only that optional-builtin guard from `optware_lifecycle_lock.sh`. The real lifecycle lock acquisition remains the fail-closed test; do not enable CONFIG_ASH_CMDCMD just for the host test and do not change firmware config.
- Validation boundary: L1 source/config diagnosis PASS. On the base SHA the BusyBox host build/link, direct ash/awk/flock smoke and shell syntax checks passed. The ash lock/teardown/startup functional regressions did not complete: the first stopped at the optional command-builtin guard, and later ash tests plus storage steps 10–13 were skipped. This commit's exact-head storage/docs Actions are pending. No local repository checkout was available; no local rerun, production MIPS/uClibc build, full firmware/TRX or L4 router test is claimed.
- Changed paths: `trunk/user/scripts/tests/optware_lifecycle_lock.sh`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev_log.md had 114 complete newest-first records before this entry and has 115 afterward; archives remain dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Resulting commit: THIS COMMIT.
- Next: reconcile storage and docs Actions on the exact resulting main SHA. Require all three Optware lifecycle regressions under BusyBox ash and storage steps 10–13 to pass; keep target MIPS/uClibc, full firmware/TRX, real router behavior and L4 NOT VERIFIED.

---

## 2026-11-11 22:34 CST (UTC+8) — Make BusyBox 1.24.2 applet validation behavioral and repair the DHCP source contract

- Base main SHA: `c4afed88518d2c4a9dc8c4e4dc2d0f2f129996f4`; parent `de95a41f5c95bf6543c78cd05a4ca3228e4af7a8`; remote branch inventory: `main` only, no checkpoint.
- Exact-head Actions reviewed on the base: storage [43036593168] FAILURE; udhcpc [43036593353] FAILURE; AI handoff docs [43036593178] SUCCESS; infosvr packet/signal [43036593109] SUCCESS. Every job's steps and complete logs were inspected.
- Storage steps 3–8 passed the NFS/Optware syntax checks and existing lock, teardown and startup POSIX-shell regressions. Step 9 successfully configured, compiled and linked BusyBox 1.24.2 after the preceding DHCP helper spelling and host-glibc `stime()` problems were corrected. It then failed at the `busybox --list` applet-presence assertion for `flock`, before the regressions ran under the built ash. The firmware config contains `CONFIG_FLOCK=y`, the disposable host config verifies it, and the build log compiled `util-linux/flock.o`.
- Correction in THIS COMMIT: replace the text-list probe with direct ash and awk execution plus an actual descriptor-backed BusyBox flock acquire/release smoke test. Preserve CONFIG_FLOCK=y and all firmware applet selection; do not relax the requirement.
- The udhcpc GNU89 ASan/UBSan policy binary passed. Its Python checker then failed because it searched for `udhcp_script_failure_prepare_retry`; the real static-inline declaration and production call are `udhcpc_script_failure_prepare_retry`. THIS COMMIT corrects only the stale source-contract spelling; no DHCP runtime implementation changes.
- AI docs validated root log=113/119 with archives dev-log=6, handoff=22, roadmap=18. Infosvr packet initialization, SIGTERM deferral sanitizer tests and source contract all passed on the same base SHA.
- Validation boundary: L1 diagnosis/source review PASS. No local checkout was available, so local tests and the whole-repository docs validator were NOT RUN. The BusyBox host build/link passed in Actions, but none of the Optware regressions under the built ash ran on the base SHA because the applet gate stopped the step. New exact-head storage, udhcpc and docs Actions are pending. This is not a production MIPS/uClibc build, full firmware/TRX or L4 router test.
- Changed paths: `.github/workflows/storage-hotplug-stability.yml`, `scripts/check_udhcpc_script_status.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev_log.md had 113 complete newest-first records before this entry and has 114 afterward; archive inventory remains dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact resulting main SHA's storage, udhcpc and AI docs Actions, every step and full logs. Require the BusyBox 1.24.2 binary and applet smoke tests, all three Optware lifecycle regressions under ash, storage steps 10–13, the udhcpc sanitizer/source contracts and docs validator to pass. Keep production MIPS/uClibc, firmware/TRX, real router ash/runtime and L4 NOT VERIFIED.

---

## 2026-11-11 (UTC+8) — Repair the BusyBox host ash build exposed by Optware lifecycle validation

- Base main SHA: `de95a41f5c95bf6543c78cd05a4ca3228e4af7a8`; parent `2c769eac12cfdcb597644ebcf9c65b51423c8e4d`; remote branches: main only, no checkpoint.
- Exact-head Actions on the base: storage [43028546205] FAILURE, AI handoff docs [43028546042] SUCCESS, infosvr lifecycle [43028546103] SUCCESS. Storage steps 3–8 passed (script syntax plus lock, teardown and startup shell regressions); step 9 failed while linking BusyBox 1.24.2. The complete log showed an undefined reference caused by a misspelled `dhcpc.c` call to `udhcp_script_failure_prepare_retry()`; the header's actual static-inline API is `udhcpc_script_failure_prepare_retry()`.
- The same host build also linked BusyBox's legacy DATE applet, whose `stime()` reference is not exported by current hosted glibc. This is a host-toolchain mismatch: target firmware uses its own uClibc and deliberately enables DATE. No firmware config change is appropriate for an unrelated ash regression.
- THIS COMMIT fixes the DHCP helper call spelling and adds a source-contract check that the caller includes and uses the declared policy helper. The storage workflow derives a disposable host `.config` from the tracked board config and disables only CONFIG_DATE before `oldconfig`; it asserts DATE stays disabled in that host copy and separately asserts the tracked board config retains CONFIG_DATE=y. Firmware configuration and applet selection are unchanged; ash/awk/flock and all lifecycle-required applets remain required.
- Validation boundary: L1 diagnosis and source/config review PASS. No local repository checkout was available, so local host compilation and `scripts/check_ai_handoff_docs.py` were NOT RUN. On the base SHA, the pre-BusyBox shell regressions passed, but the BusyBox link failed as above. Exact-head storage and AI docs Actions for THIS COMMIT are pending. This is not a production MIPS/uClibc build, complete firmware/TRX or L4 router test.
- Root dev_log.md had 112 complete newest-first records before this entry and has 113 afterward; the existing archive inventory remains dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Changed paths: `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact resulting main SHA's storage and AI docs Actions, every job step and complete logs. Require the BusyBox link, required applet checks, ash syntax checks and all three Optware lifecycle regressions to pass. Keep target MIPS/uClibc, firmware/TRX, actual router ash/runtime and L4 NOT VERIFIED.

---

## 2026-11-11 (UTC+8) — Exercise Optware lifecycle with firmware BusyBox ash 1.24.2

- Base main SHA: \`2c769eac12cfdcb597644ebcf9c65b51423c8e4d\`; parent \`4ec9d637ca7daaff93296c494467b6e88c528311\`; branch inventory: main only, no remote checkpoint.
- Closed the previous GPT's active ASUS infosvr SIGTERM task at exact-head L2. Infosvr packet/signal workflow [43024714802] and AI handoff docs [43024714674] both completed SUCCESS on exact SHA \`2c769eac12cfdcb597644ebcf9c65b51423c8e4d\`; all job steps and complete logs were inspected. GNU89 ASan/UBSan packet and signal tests plus the UDP/SIGTERM source contract printed PASS; docs validator reported 111/119 records and archives 6/22/18. Production MIPS/uClibc, firmware/TRX and L4 signal behavior remain NOT VERIFIED.
- Investigated the separately tracked FTP failed-upload cleanup TOCTOU. On Linux 4.4, \`fstatat()\` followed by \`unlinkat()\` cannot atomically bind the unlink to the previously checked inode. \`renameat2()\` exchange/no-replace operations are not conditional inode comparisons; staging would require a wider upload-publication redesign and has unresolved STOR/APPE/REST visibility/overwrite semantics. No unsafe patch was made; the issue remains P2 OPEN/BLOCKED pending a proved compatible design.
- Selected the next evidence-actionable storage item: prior Optware lock/teardown/startup regressions ran under host \`/bin/sh\`, and their child fixture shebangs were hard-coded. That did not exercise the exact firmware BusyBox shell and applet implementation. Firmware selects BusyBox 1.24.2 and enables ash, awk and flock in the common config.
- THIS COMMIT makes the lifecycle teardown/startup fixture writer honor \`OPTWARE_TEST_SHELL\` (default \`/bin/sh\`) and updates the storage workflow to host-build the exact BusyBox 1.24.2 source using the firmware common config. CI verifies ash/awk/flock and required lifecycle applets, syntax-checks Optware production scripts with ash, then runs lock, teardown and startup regressions under that binary. Child mock hooks use the same ash shebang; BusyBox awk/flock resolve through applet symlinks. Path filters cover the firmware BusyBox config, configure selector, package Makefile and pinned source tree for both push and pull_request.
- Evidence boundary: source/build selection is L1 reviewed; exact BusyBox 1.24.2 host build and ash L2 execution are pending the resulting exact-head Actions. This is not a production MIPS/uClibc binary, complete firmware/TRX build, target kernel test or L4 router result.
- Root dev_log.md had 111 complete newest-first records before this entry and has 112 afterward. At 290,184 bytes before the entry, it remains below the 384 KiB rotation target and 512 KiB hard cap; archive inventory remains dev-log=6, handoff=22, roadmap=18. No archive or index update is due.
- Changed paths: \`trunk/user/scripts/tests/optware_lifecycle_teardown.sh\`, \`trunk/user/scripts/tests/optware_lifecycle_startup.sh\`, \`.github/workflows/storage-hotplug-stability.yml\`, \`SESSION_HANDOFF.md\`, \`REFACTOR_ROADMAP.md\` and this log.
- Resulting commit: THIS COMMIT.
- Next: fetch the resulting main SHA; inspect the exact-head storage and AI docs workflows, every job step and full log. Require the BusyBox 1.24.2 host build, applet checks, ash syntax checks and all three lifecycle regressions to pass. Keep MIPS/uClibc, complete firmware/TRX and L4 NOT VERIFIED.

---

## 2026-11-11 20:21 CST (UTC+8) — Defer infosvr SIGTERM cleanup out of signal context

- Base main SHA: 4ec9d637ca7daaff93296c494467b6e88c528311; parent 14e22191f48c9ad6f3804729ab502ac81670a30b; branch inventory: main only, no remote checkpoint.
- Resumed the previous handoff's Optware/storage investigation. The corrected exact-head storage [43017662766], infosvr packet [43017662767] and AI docs [43017662804] runs all completed SUCCESS on 4ec9d637ca7daaff93296c494467b6e88c528311; every step and complete log was inspected. The storage swapoff regression and full caller-graph/lock/failure contract now pass. The target source is BusyBox 1.24.2 with ash selected and required applets configured, but the matching executable is not available here; host BusyBox 1.37.0 is not target ash evidence. Storage is closed at L1/L2 and explicitly blocked at target-runtime level.
- After closing that bounded source/L2 slice, rotated to the separate P2 infosvr daemon lifecycle risk. Finding: catch_sig_infosvr(SIGTERM) called res_cleanup() (close/remove) and exit() in signal context. The old main loop used an unbounded select() and read_packet() retried every EINTR, so a flag-only handler without bounded waits would retain a check-to-block shutdown race.
- THIS COMMIT makes the handler a single sig_atomic_t stop-flag store, installs SIGTERM through checked sigaction() with SA_RESTART disabled, adds one-second select(), SO_RCVTIMEO and SO_SNDTIMEO bounds, stops receive/send EINTR retries after shutdown, and performs socket/PID cleanup only after the main loop observes the flag. The descriptor cleanup predicate now correctly accepts fd 0.
- Regression: add a GNU89 ASan/UBSan policy test for the stop flag/coalescing and timeout constants; extend the infosvr source checker to enforce handler purity, sigaction/no-restart ordering, bounded waits, stop observation and main-context cleanup; update the existing path-filtered workflow for push and pull_request.
- Validation boundary: L1 source review PASS. The isolated signal policy test passed locally with GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined and leak detection. The updated source-contract script and exact-head infosvr/AI docs Actions are pending for the resulting SHA. Target MIPS/uClibc, actual BusyBox ash 1.24.2, complete firmware/TRX and L4 signal timing remain NOT VERIFIED.
- Root dev_log.md had 110 complete newest-first records before this entry and has 111 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap; archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index update is due.
- Changed paths: trunk/user/infosvr/src/infosvr.c, new signal policy header/test, scripts/check_infosvr_packets.py, .github/workflows/infosvr-packet-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.
- Next: fetch resulting main SHA; inspect every infosvr and AI docs job step/full log; require the signal policy sanitizer regression and all signal/source contracts to pass before closing L2 and rotating to the next global governance item.

---

## 2026-11-11 20:08 CST (UTC+8) — Correct the storage caller-graph swapoff assertion

- Base main SHA: 14e22191f48c9ad6f3804729ab502ac81670a30b; parent 2968bb70c97e13f405389795628184f804abcf5d; branch inventory remains main only.
- Exact-head Actions: storage run [43017000972] completed FAILURE in the final “Verify whole storage caller graph, operation locks and failure propagation” step. The Optware teardown/helper syntax step and the entire Optware teardown regression step both PASSED. The failure was a stale source assertion at Python line 227 that still searched for the removed literal `swapoff /opt/.swap`; the implementation now calls `optware_lifecycle_swapoff_file /opt/.swap`. The traceback was `ValueError: substring not found`, not a product regression failure.
- Infosvr [43017000995] and AI handoff docs [43017000981] both completed SUCCESS on exact SHA 14e22191f48c9ad6f3804729ab502ac81670a30b; all steps passed, including the GNU89 ASan/UBSan infosvr policy test and documentation archive validator.
- THIS COMMIT updates the older whole-caller source contract to locate the explicit helper invocation, retaining the required ordering after service-stop failure handling and before the /opt unmount gate. No production source or shell regression behavior changes in this correction.
- Validation boundary: L1 source review unchanged; new Optware syntax and teardown host regression steps passed on 14e22191f48c9ad6f3804729ab502ac81670a30b. Matching target BusyBox ash, MIPS/uClibc objects, complete firmware/TRX and L4 hardware remain NOT VERIFIED. Corrected exact-head storage/docs Actions pending.
- Root dev_log.md had 109 complete records before this entry and has 110 afterward. Below rotation thresholds; archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index change is due.
- Changed paths: .github/workflows/storage-hotplug-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.
- Next: inspect corrected exact-head storage and AI docs Actions, including full logs and every step. If green, continue matching BusyBox ash and lifecycle lock/fd/caller audit rather than claiming L3/L4 completion.

---

## 2026-11-11 19:24 CST (UTC+8) — Fail closed when Optware swap release is uncertain

- Base main SHA: 2968bb70c97e13f405389795628184f804abcf5d; parent d2edf2dd08c85e0413f6469e2dccc6fa5a2df26a; branch inventory: main only, no remote checkpoint. Storage [43009772930] and AI docs [43009772871] both passed on this exact base SHA.
- Recovered the active Optware/storage lifecycle task and traced the installed firmware path: the Optware ROMFS Makefile installs opt-umount.sh, while the scripts package publishes the shared lifecycle helper consumed at /sbin/optware_lifecycle_lock.sh.
- Finding: opt-umount.sh ignored swapoff /opt/.swap failure and continued with /home/admin and /etc/localtime cleanup plus /opt unmount. File existence was used as a proxy for activation, and no postcondition proved the swap entry had disappeared.
- THIS COMMIT adds an exact-entry /proc/swaps reader to the shared lifecycle helper. It validates the header and complete records, including read/close errors; distinguishes active, inactive and unknown inventory; attempts swapoff only for the exact active /opt/.swap path; requires command success and re-reads /proc/swaps to confirm that the entry disappeared. Unknown/malformed inventory, swapoff failure or a still-active entry now blocks home/localtime cleanup and /opt unmount. Awk, swapoff and logger child processes close lifecycle fd 9.
- Regression: extend optware_lifecycle_teardown.sh with inactive, prefix-only, successful deactivation, command-failure, still-active, missing-inventory, malformed-inventory and lock-fd inheritance cases. The storage workflow adds syntax checks and a semantic source-order contract for the early failure gate.
- Validation: L1 source review PASS. Host shell regression and exact-head storage/AI docs Actions are pending for THIS COMMIT. The regression uses host sh; matching target BusyBox ash, production MIPS/uClibc, complete firmware/TRX and L4 hardware remain NOT VERIFIED.
- Root dev_log.md: 108 complete records before this entry; 109 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap; archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index update is due.
- Changed paths: trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh, trunk/user/optware/opt-umount.sh, trunk/user/scripts/tests/optware_lifecycle_teardown.sh, .github/workflows/storage-hotplug-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact-head storage and AI handoff workflow jobs/full logs; require all swap inventory regression cases and the unmount-order contract to pass. Then locate the matching firmware BusyBox ash implementation/runtime for the remaining lock and descriptor-inheritance audit.

---

## 2026-11-11 18:40 CST (UTC+8) — Gate storage removal on every mounted-media daemon stop result

- Base main SHA: f986ed961bbe14a8db4bc8d99219e5a1a1330826; parent 17b4583ce0511a3cbbd02cef18024fe7b30110e7; branch inventory: main only, no remote checkpoint. Exact-base AI docs [43004194407] and infosvr packet stability [43004194313] both SUCCESS; complete logs inspected.
- Traced the installed firmware path rather than treating K95 hooks as live callers. trunk/user/optware/Makefile installs opt-start/opt-mount/opt-umount but not K95aria2/K95transmission; scripts/Makefile publishes the shared /sbin lifecycle helper. Built-in Aria/Transmission use separate RC scripts and services_stor.c callers.
- Found that stop_stor_apps() propagated NFS/Samba/FTP failures but ignored MiniDLNA, Firefly, Transmission and Aria stop outcomes. The four stop wrappers discarded kill_services()/eval() status and did not confirm process absence, so safe removal could continue with a storage-backed daemon still observable.
- Changed each stop wrapper to return failure unless both the stop command succeeds and the daemon-absence postcondition holds. stop_stor_apps() now blocks removal on all four failures; manual restart callers also refuse to start over a daemon that failed to stop. Added a reusable GNU89 policy helper and a five-case regression.
- Host test: GNU89 -Wall -Wextra -Werror ASan/UBSan policy binary printed 'storage daemon stop postcondition policy: PASS'. Exact-head storage Actions are pending for THIS COMMIT; no target BusyBox ash, MIPS/uClibc, firmware/TRX or L4 result is claimed.
- Remaining next slice: opt-umount.sh ignores swapoff /opt/.swap failure before home/localtime cleanup and /opt unmount; add active/inactive /proc/swaps and swapoff-failure coverage, then continue matching-shell lock verification.
- Root dev_log.md: 107 complete newest-first records before this entry; 108 afterward. Below 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index update is due.
- Changed paths: .github/workflows/storage-hotplug-stability.yml, trunk/user/rc/src/services_stor.c, trunk/user/rc/src/rc.h, trunk/user/rc/src/storage_service_stop_policy.h, trunk/user/rc/src/tests/storage_service_stop_policy.c, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.

---
## 2026-11-11 18:18 CST (UTC+8) — Close udhcpc callback gate and hand off Optware lifecycle audit

- Base main SHA: 17b4583ce0511a3cbbd02cef18024fe7b30110e7; parent a3ee4d0e628434b1e16171f457afa0791c84f7a3; branch inventory: main only, no remote checkpoint.
- Closed the DHCP callback-status fix at L2 on its exact code SHA. Dedicated udhcpc workflow [43002980464], path-triggered infosvr packet regression [43002980467] and AI handoff documentation validation [43002980526] all completed SUCCESS on 17b4583ce0511a3cbbd02cef18024fe7b30110e7. Every job/step and complete log was inspected.
- The committed udhcpc GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer test printed "udhcpc script failure retry policy: PASS"; source checker printed "udhcpc event-script status and lease-state source contracts: PASS". AI docs validator printed "AI handoff documentation check: PASS (root log=106/119 entries, batch=100, retain=20, legacy/current archives: dev-log=6, handoff=22, roadmap=18)".
- L2 closure applies only to the bound/renew DHCPACK callback gate and retry/deadline policy. Production MIPS/uClibc BusyBox object, full firmware/TRX, live v4/v6 DHCP transitions and router behavior remain NOT VERIFIED. Other event-state semantics are still open.
- Selected the next cross-repository P2 investigation: Optware/storage lifecycle under BusyBox ash. Prior review notes K95 hooks are not ROMFS-installed/invoked by the in-tree S?? dispatcher, while built-in Aria2/Transmission use separate RC scripts. The next pass must prove active installation/call paths before treating K95 code as production behavior.
- Next task: inventory actual hook/RC/hotplug entrypoints; trace operation-lock ownership and descriptor lifetime, Optware startup/rollback/stop status, swapoff and /opt unmount ordering; add focused host regressions for the active path. Use BusyBox ash only when the shell implementation being tested is available and matched; do not substitute host POSIX shell claims for target ash evidence.
- Root dev_log.md: 106 complete records before this entry; 107 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index changes are due.
- Changed paths: SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log only. This is a state-transition closeout and next-task handoff after exact-head verification, not a code-verification-only note.
- Resulting commit: THIS COMMIT.

---

## 2026-11-11 18:08 CST (UTC+8) — Preserve udhcpc event-script status and gate DHCP lease publication

- Base main SHA: a3ee4d0e628434b1e16171f457afa0791c84f7a3; parent 65119b857a7684974caf2ae0e13d8112a0e035bd; branch inventory: main only, no remote checkpoint.
- Closed the previous BusyBox crond detached-daemon lifecycle task on exact SHA a3ee4d0e628434b1e16171f457afa0791c84f7a3: Crond workflow [42999257797] and AI docs [42999257844] both SUCCESS. All 25 Actions runs on that SHA completed successfully; every job step and the relevant full logs were inspected. Crond GNU89 ASan/UBSan regression and RC/BusyBox source contracts passed.
- Selected the LAN DHCP client callback-status issue from the cross-repository governance ledger. The fix is in BusyBox udhcpc rather than the previously touched SkipDBv2, HTTPD or RC-only code.
- Root cause: BusyBox `spawn_and_wait()` returns `wait4pid()` status, but `udhcp_run_script()` discarded it. On DHCPACK, udhcpc then advanced to BOUND, published the new requested address and refreshed the lease timer even if the RC `bound`/`renew` callback returned an error while applying the address/routes/resolver.
- THIS COMMIT changes the event runner to return and log its script result. DHCPACK processing now gates address/BOUND/renewal-timer publication on callback success. A failed initial bound callback retries the same selected request after the configured retry interval. A failed renewal preserves the pre-existing lease deadline, charges callback runtime against that deadline and lets the normal expiry path run if the callback consumed the remaining lease.
- Added `udhcpc_script_policy.h` plus a five-case GNU89 host test for initial retry, renewal deadline preservation, callback overrun, exact expiry and an already expired lease. Added a source/caller checker confirming the actual `spawn_and_wait()` contract, ACK gate order, policy integration and symmetric path filters; added dedicated path-filtered Actions workflow.
- Local host policy reproduction: GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`, leak detection enabled; PASS for all five retry/deadline cases. The committed repository harness and source checker remain pending exact-head Actions.
- Changed paths: BusyBox `dhcpc.c`, new DHCP retry policy header/test, new source checker and workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log. No archive/index change is due.
- Root `dev_log.md`: 105 complete records before this entry; 106 afterward. The file remains below the 120-record / 384 KiB rotation triggers and the 512 KiB hard cap; archive inventory remains 6/22/18.
- Resulting commit: THIS COMMIT.
- Next: inspect all exact-head jobs and full logs for the dedicated udhcpc event-script status regression and AI handoff docs; also inspect any triggered WAN/HTTPD umbrellas. Require the sanitizer policy test, actual BusyBox script-result and DHCPACK source contracts, and docs validator to pass on this exact SHA. Production MIPS/uClibc BusyBox object, firmware/TRX, DHCPv6 and L4 remain NOT VERIFIED.

---

## 2026-11-11 17:55 CST (UTC+8) — Gate crond lifecycle on detached process readiness and confirmed stop

- Base main SHA: 65119b857a7684974caf2ae0e13d8112a0e035bd; parent 947ee69cfbda917ec1728cdde39a00b6a7726eeb; branch inventory: main only, no remote checkpoint.
- Closed the previous infosvr packet parsing task on exact SHA 65119b857a7684974caf2ae0e13d8112a0e035bd: packet workflow [42997959662] and AI handoff docs [42997959722] both SUCCESS. All steps and complete logs inspected; GNU89 ASan/UBSan packet policy test, UDP/source-boundary contract and docs archive/heading validator passed. Root log=104/119; archive counts 6/22/18.
- Selected BusyBox crond as the next P2 reliability issue from the broader RC/userland lifecycle inventory.
- Root cause: RC _eval() forks and waits for the BusyBox launcher process, but crond_main() daemonizes before child xchdir(), crontab rescan and pidfile publication. The launcher parent exits successfully immediately after forking, so RC could report success even if the detached scheduler failed during initialization.
- A second lifecycle defect discarded kill_services() status in stop_crond(); restart_crond() then launched another scheduler unconditionally. If the previous process remained after bounded TERM/KILL, duplicate cron jobs could run.
- Fix in THIS COMMIT: return the shared stop result, gate restart on confirmed process exit, and add an initial grace plus bounded checks for a live non-zombie pids("crond") process after the daemonizing parent exits. Preserve disabled-service return value 1. Propagate negative restart failures from RC notification dispatch so the in-flight completion marker remains for retry; log failure after initial crontab regeneration.
- Host regression: GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer, run with leak detection; PASS, output crond daemon startup and restart policy: PASS. New source checker covers BusyBox fork/parent/child ordering, pid matching/zombie exclusion, lifecycle return propagation, all RC call sites and symmetric workflow filters.
- Changed paths: new crond workflow, policy header and test, source checker, services.c, rc.c, rc.h, this handoff, roadmap and this log. No archive/index rotation is due.
- Root dev_log.md: 104 complete records before this entry; 105 afterward, below the 120-entry/384 KiB rotation triggers and 512 KiB hard cap. Archives remain 6/22/18.
- Resulting commit: THIS COMMIT.
- Next: inspect all exact-head jobs/full logs for Crond lifecycle and AI docs, plus any path-triggered RC umbrellas. Require GNU89 sanitizer regression, source/caller contract and docs validator to pass on the same SHA. Production MIPS/uClibc, full firmware/TRX, live cron execution and L4 remain NOT VERIFIED.

---

## 2026-11-11 17:38 CST (UTC+8) — Restore required verification-boundary heading and guard handoff contract

- Base main SHA: `947ee69cfbda917ec1728cdde39a00b6a7726eeb`; parent `8ec8198c4564e57cd3956ea675e343bc35098e72`.
- Infosvr code/source-contract workflow [42997353568] completed SUCCESS on exact base SHA `947ee69cfbda917ec1728cdde39a00b6a7726eeb`. Full logs confirmed `infosvr packet initialization and length policy: PASS` and `infosvr packet receive and bounds source contracts: PASS`.
- AI docs run [42997352476] failed on that same base because the live handoff lacked the validator-mandated `## Verification boundary` heading. This was a documentation-structure defect, not a C regression or production failure.
- THIS COMMIT restores the required section with explicit local/CI/target evidence boundaries, updates the roadmap and next-step state, and adds checker assertions for all four mandatory handoff headings. The infosvr workflow now path-filters `SESSION_HANDOFF.md` on push and pull_request because the source contract reads it.
- No application source changed in this correction; the prior commit contains the packet fix and local test. This correction repairs the mandatory validator contract and makes that contract executable in the infosvr checker. Exact-head reruns remain pending; do not describe [42997352476] as green.
- Root `dev_log.md`: 103 records before this entry; 104 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap; archive shards/indexes remain untouched.
- Resulting commit: THIS COMMIT.
- Next: inspect both infosvr and AI docs workflows, every step and full logs, on the exact resulting main SHA. Require sanitizer test, packet source/ABI checks, required handoff sections and archive validator to pass; then select another user-space service. MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-11 17:31 CST (UTC+8) — Harden infosvr UDP receive-buffer initialization and wire-prefix validation

- Base main SHA: `8ec8198c4564e57cd3956ea675e343bc35098e72`; parent `a73ca2221712f16dd5aa4bab450eed18a6cd3f92`; branch inventory was `main` only, no checkpoint.
- Closed networkmap signal-context work on exact base SHA `8ec8198c4564e57cd3956ea675e343bc35098e72`: workflow [42996232222] and AI docs [42996232115] both SUCCESS. Complete logs inspected; GNU89 ASan/UBSan test and signal/lifecycle/handler-registration source contract passed; docs reported root log=102/119 and archive inventory 6/22/18. The earlier [42995791241] docs failure on parent SHA was the missing heading, corrected in the current base and rerun green.
- New P2 reliability/security finding in the independent ASUS `infosvr` discovery daemon. `read_packet()` used `sizeof(*packet)` where `packet` is `void *`; GNU89's `sizeof(void)` extension is one byte, confirmed with a host compiler. Thus only the first byte of the 512-byte stack packet buffer was cleared. Short UDP datagrams left the remaining bytes indeterminate. In addition, `read_packet()` returned a boolean `0/1`, while main tested it for `< 0`, so incomplete datagrams continued into `process_packet()`. The parser then dereferenced base/extended wire fields without the actual received size.
- Fix in THIS COMMIT: zero the complete supplied receive capacity; return actual `recvfrom()` byte count/error; reject packets shorter than the base IBOX header; pass actual received length into `process_packet()`; validate the extended prefix up to `offsetof(IBOX_COMM_PKT_HDR_EX, Password)` before reading MAC; and fail closed when the NVRAM hardware address is missing/invalid rather than comparing an uninitialized local buffer.
- Regression: new GNU89 test covers all-byte zeroing including unused tail, short-packet deterministic padding, NULL/zero-size errors and base/extended prefix boundaries. New source checker verifies helper integration, actual-byte propagation, base/extended guards, safe MAC conversion and symmetric workflow push/PR filters.
- Local compile/run: GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`; ASan leak detection enabled. Output: `infosvr packet initialization and length policy: PASS`.
- Exact-head infosvr workflow and AI docs are pending. No production MIPS/uClibc infosvr object, complete firmware/TRX or L4 packet interoperability test is claimed. Separate follow-up: infosvr SIGTERM handler still does cleanup/exit in signal context; this packet parsing change intentionally does not combine that lifecycle fix.
- Changed paths: `infosvr.c`, `asus.c`, new packet policy header/test, new source checker/workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root `dev_log.md`: 102 complete entries before this record; 103 afterward. Below 120-entry / 384 KiB rotation triggers and 512 KiB hard limit; no archive or index update is due.
- Resulting commit: THIS COMMIT.
- Next: verify the exact resulting main SHA and all job steps/full logs for infosvr packet regression and AI docs. Require sanitizer test, receive/parser/ABI contract and docs validator to pass on that exact SHA; then select another under-covered user-space module.

---

## 2026-11-11 17:21 CST (UTC+8) — Repair handoff contract and extend networkmap signal lifecycle coverage

- Base main SHA: `a73ca2221712f16dd5aa4bab450eed18a6cd3f92`; parent `ce7efad34dbd81097d923f9568f0ae9946694b58`.
- Closed the previous NVRAM HDR2 atomic-save task before opening this slice: exact-head HTTPD security [42992756073] and AI docs [42992755926] both SUCCESS on `ce7efad34dbd81097d923f9568f0ae9946694b58`. All 39 HTTPD steps passed, including the new profile atomic writer, original NVRAM profile/CLI policy tests, RC restore durability and HTTPD private export caller contract. Docs validator reported 100/119 records and archive counts 6/22/18.
- Networkmap signal fix from parent SHA `a73ca2221712f16dd5aa4bab450eed18a6cd3f92` passed its exact-head dedicated workflow [42995791190]: GNU89 `-Wall -Wextra -Werror` ASan/UBSan test printed `networkmap signal deferral policy: PASS`; source/lifecycle/build contract printed `networkmap signal deferral and lifecycle source contracts: PASS`.
- AI docs run [42995791241] on that same parent SHA failed at its first section assertion: the consolidated `SESSION_HANDOFF.md` had renamed the exact required heading `## Current repository state`. This is a handoff-validator contract failure, not a production or test failure. THIS COMMIT restores all exact required headings (`Current repository state`, `Active task`, `Verification boundary`, `Exact first action for the next GPT`) and keeps the checkpoint text intact.
- Additional checker coverage asserts SIGUSR1/SIGTERM handlers are installed before daemonization and before the event loop begins; this guards the lifecycle boundary where process signal handling becomes active. The changed checker path also re-triggers the networkmap source/test workflow alongside the docs workflow.
- Changed paths: `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `scripts/check_networkmap_signals.py`.
- Root dev_log.md: 101 complete entries before this record; 102 after. Below 120/384 KiB rotation triggers and 512 KiB hard limit; no archive or INDEX update is due.
- Resulting commit: THIS COMMIT.
- Next: inspect every step and full log for the networkmap and AI docs workflows on the exact resulting main SHA. Require the existing signal policy test, expanded signal-install ordering source contract, handoff required sections and archive validator all PASS; only then close the networkmap slice. L3/L4 remain NOT VERIFIED.

---

## 2026-11-11 17:16 CST (UTC+8) — Defer networkmap daemon signal handling and cleanup to main loop

- Base main SHA: `ce7efad34dbd81097d923f9568f0ae9946694b58`; parent: `534e4e38ed5e729af845d7ad69bb84d8b96a5c47`. Only remote branch was `main`; no checkpoint branch.
- Closed the previously pending NVRAM profile-save task on exact head `ce7efad34dbd81097d923f9568f0ae9946694b58`: HTTPD Authentication/Security [42992756073] and AI handoff docs [42992755926] both completed SUCCESS. Every one of the 39 HTTPD job steps succeeded. Full logs confirm `NVRAM profile policy: PASS`, `NVRAM profile atomic publication: PASS`, `NVRAM CLI assignment policy: PASS`, RC NVRAM persistence checks, private HTTPD export path, and the rest of the selected HTTPD/storage/wireless contracts. Docs validation reported root log=100/119 and archives dev-log=6, handoff=22, roadmap=18.
- New P2 reliability finding in `trunk/user/networkmap/src/networkmap.c`: SIGTERM called `clear_resources()` and `exit(0)` from the handler. The cleanup closes the ARP socket, frees the client list, writes NVRAM and removes the PID file, risking re-entry into libc/allocator/NVRAM state. SIGUSR1 also directly changed `networkmap_fullscan` and `scan_now` while the main loop used them. `function.c` declares `extern unsigned int scan_now`, so the previous volatile definition was inconsistent.
- Fix in THIS COMMIT: both handlers now expand a macro to direct `volatile sig_atomic_t` flag assignments only; no helper function or cleanup routine runs in signal context. The main loop temporarily blocks SIGUSR1 while snapshotting/clearing the refresh flag and restores the caller's previous mask, so a new pending signal is delivered on the next pass. Refresh flags/counters and all resource cleanup are applied only in normal process context. `scan_now` declaration now matches its non-volatile extern.
- Regression: new GNU89 ASan/UBSan test covers refresh/exit mapping, coalescing, ignored signals, exactly-once consumption, already-blocked mask preservation, pending SIGUSR1 delivery and NULL input. New source checker verifies handler purity, mask/snapshot/restore order, receive timeout, main-context cleanup, networkmap build/package wiring, RC lifecycle and symmetric push/PR path filters.
- Local test rerun using the final macro-based header: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`; run with `ASAN_OPTIONS=detect_leaks=1`: `networkmap signal deferral policy: PASS`.
- Exact-head networkmap and AI docs workflows are pending; do not claim them before inspecting their complete logs. No full networkmap production object, MIPS/uClibc image, firmware/TRX or router test is claimed.
- Changed paths: `trunk/user/networkmap/src/networkmap.c`, new policy header/test, new source checker/workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root `dev_log.md`: 100 complete records before this entry; 101 after. Below 120-record / 384 KiB rotation triggers and 512 KiB hard cap; no archive or index update is due.
- Resulting commit: THIS COMMIT.
- Next: inspect exact resulting main SHA and every job step/full log for the networkmap signal regression and AI docs. Require sanitizer test, signal/call-path/package/source contract and docs validator to pass on that same SHA; then rotate to an under-covered userland service.

---

