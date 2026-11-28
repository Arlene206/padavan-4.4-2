## 2026-11-28 — Update watchdog signal integration contract for identity-bound rollback

- The watchdog executable identity source change c9f5d0e1abd9df58576b18db4e8d10db0e448066 caused the separate NTP/watchdog signal CI to fail at `scripts/check_watchdog_ntp_lifecycle.py:247`: it asserted the old generic `kill_services()` rollback implementation.
- THIS commit updates that contract to require the watchdog-only SIGTERM/SIGKILL signal helper, bounded waits, and identity-scoped rollback scan, and adds the new executable policy header to the signal workflow's push/PR path filters. No product source changed.
- Exact c9f5d0e1 SHA: HTTPD security #46093743691 PASS; HTTPD lifecycle #46093743621 PASS; DNSMasq lifecycle #46093743721 PASS; Process inventory #46093743679 PASS; AI docs #46093743683 PASS; watchdog-signal #46093743633 FAIL for this stale assertion; Process lifecycle #46093743723 and XY-C1 #46093743713 remained IN_PROGRESS at last poll.
- After this correction, inspect exact-head Process/XY-C1/watchdog-signal runs, then check this SHA's reruns. USB storage on 363fd7fba6f90352d70bb581ea6d0690a573441c PASS (#46093641272); XY-C1 on that SHA PASS (#46093641344); AI docs PASS (#46093641318).

---

## 2026-11-28 — Restrict watchdog trigger-count assertions to workflow path filters

- The watchdog identity implementation was committed as c9f5d0e1abd9df58576b18db4e8d10db0e448066. Process lifecycle #46093743723, XY-C1 #46093743713, and AI docs #46093743683 were still running at handoff.
- During immediate contract review, the new Process workflow assertion was found to count every textual mention of the regression source, including both path filters plus its compile command. The correct expected textual total is 3, whereas the intended path-filter count is 2. THIS checker-only correction counts only the exact YAML path-list entries, avoiding false failures while preserving push and pull_request coverage.
- Previously fixed storage chain: exact SHA 363fd7fba6f90352d70bb581ea6d0690a573441c USB storage hotplug #46093641272 PASS; XY-C1 #46093641344 was still in progress at last poll; AI docs #46093641318 PASS.
- Next action: inspect exact SHA c9f5d0e1... Process/XY/AI run outcomes and this checker correction's CI. Then diagnose actual watchdog C build/integration issues if any. Backup remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-28 — Bind PID 1 watchdog readiness and rollback to executable identity

- The generic shared process inventory intentionally supports legacy daemon argv0/comm aliases. Source review found that `start_watchdog()` still consumed that generic match directly: any local process using `exec -a watchdog` could be mistaken for the PID 1 watchdog, suppressing startup and its HTTPD/DNS recovery loop. The ROMFS Makefile installs `/sbin/watchdog` as an alias to the single `/sbin/rc` binary.
- THIS source slice adds a watchdog-only identity gate requiring both argv0 basename `watchdog` and `/proc/PID/exe` resolving to the known `/sbin/rc`/watchdog alias; it revalidates PID/start-time/argv after the procfs reads. Startup probes and rollback scans use the guarded result. Rollback signals only individually revalidated, executable-matched instances (TERM then bounded KILL) instead of generic `kill_services()`, preventing cleanup from targeting argv0-spoof processes.
- Added GNU89/Werror/ASan/UBSan regression for executable aliases, deleted executable suffix, argv0 selection and invalid inputs. Process lifecycle contract and exact-head Process/XY-C1 triggers include the new policy/test. No local target build or hardware test is claimed.
- Exact preceding storage checker SHA aa155bd6b424603b75f5440d888d19885c8fcd86: USB storage #46093243935 FAIL was fixed by checker-only commit 363fd7fba6f90352d70bb581ea6d0690a573441c; XY-C1 #46093243944 PASS; AI docs #46093243938 PASS. Exact 363fd7 SHA checks were not returned in the most recent poll (no result claimed).
- Exact-head Process/XY-C1/AI docs checks for THIS commit are required. The immutable backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Correct staged storage-link mode contract

- Exact SHA aa155bd6b424603b75f5440d888d19885c8fcd86: AI handoff docs #46093243938 PASS; XY-C1 firmware validation #46093243944 PASS; USB storage hotplug #46093243935 FAIL at the Transmission publisher contract.
- The preceding lstat target-discovery assertion passed. The remaining stale test expected `create_mp_link_status(dest_dir, staged_path, 0)`, but the shared publisher intentionally forwards its validated `force_first_valid` argument to preserve MiniDLNA/Firefly's fallback behavior while Transmission/aria use 0. The checker already verifies the argument is constrained to 0/1 and callers pass their intended values.
- THIS commit aligns the publisher-body contract with the actual parameter forwarding; no production code changed. Exact storage/AI reruns are required for this SHA.
- Next independent reliability fix: bind watchdog startup/readiness/rollback to the actual `/sbin/rc` executable plus watchdog argv0, rather than generic process-inventory aliases. Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Align storage target discovery contract with lstat policy

- Exact SHA 670935e80e5e9519ea87e12d142924c6f6b05b1b: AI handoff docs #46092735083 PASS. USB storage hotplug #46092735162 initially QUEUED then failed on the retry at `scripts/check_storage_safe_remove.py:1657`; the Transmission/media caller-graph checks now pass.
- Failure is a stale contract: the checker still required `check_if_dir_exist(target_path)` and looked for ENOTDIR/EEXIST literals inside `create_mp_link()`. The implementation now deliberately uses `lstat()`, checks `S_ISDIR/S_ISLNK`, and delegates ENOTDIR/symlink/EEXIST semantics to `storage_media_link_target_status()` in `storage_media_link_policy.h`. The standalone policy regression already covers directories, symlinks, non-directories, missing targets and lookup errors.
- THIS COMMIT updates the integration contract to inspect the implementation and policy helper; it does not change production source. Exact storage-hotplug and AI docs reruns for THIS SHA are required.
- The next independent source slice is watchdog readiness identity: `start_watchdog()` still uses the generic argv0/comm inventory, so an unrelated process with argv0 `watchdog` can suppress the system watchdog. The firmware installs `/sbin/watchdog` as an alias to `/sbin/rc`; the scanner must require both that executable identity and the watchdog argv0, and rollback must not signal spoofed processes.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Anchor storage media-publisher contract to its definition

- The storage-hotplug run on 655d173234a14ce19b91f2fce9e68e1622bfc7cc passed the Transmission call-site correction but failed at the common publisher-body contract. The follow-up 0f92cf9ca01698e0c368060b9653d30b984a9d9a corrected the expected force_first_valid forwarding argument.
- Inspection found the underlying checker defect: function_body(storage, "storage_media_link_publish(") matched an earlier call from a media-daemon caller, not the later static int storage_media_link_publish(...) definition. The parser then inspected the following caller-side if block instead of the helper implementation. THIS COMMIT anchors extraction to the exact function-definition signature; no product source behavior changes.
- Parent SHA 0f92cf9ca01698e0c368060b9653d30b984a9d9a: USB storage hotplug #46092637018 and AI handoff docs #46092637068 were IN_PROGRESS at this checkpoint. Exact-head reruns for THIS correction are required; no firmware/L3 or hardware/L4 result is claimed.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807.

First next action: inspect the exact-main USB storage hotplug and AI docs runs for THIS checker correction. If the caller-graph contract passes, move to the next independent P1 module rather than repeating storage inventory; only claim XY-C1 L3 when Build, TRX and artifact are all confirmed on the same production SHA.

---

## 2026-11-27 — Fix final stale storage media-publisher test expectation

- Parent `655d173234a14ce19b91f2fce9e68e1622bfc7cc` corrected the Transmission call-site assertion to include mode `0`. Its exact storage-hotplug run `46092458013` then reached the next assertion and exposed another old assumption: the common helper body no longer hardcodes mode `0`, but forwards the validated `force_first_valid` argument to the staged-link creator.
- THIS commit updates the workflow contract to expect `create_mp_link_status(dest_dir, staged_path, force_first_valid)`. The independent assertions still require the boolean argument to be 0/1 and verify all four media daemons use their intended mode (DMS/iTunes fallback 0 then 1; torrent/aria 0 only).
- Parent SHA `655d173`: AI handoff docs `46092457928` PASS, storage-hotplug `46092458013` FAIL only on this stale helper-body assertion. Parent `adeeba571`: other regression workflows passed except the same storage-hotplug source contract; XY-C1 and NVRAM transaction boundary jobs remained pending at last poll. No firmware/hardware pass claim.
- Next: verify storage-hotplug and AI exact-head runs for THIS correction; address source failures only if surfaced, then continue next independent P1 issue. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Align storage integration checkers with Transmission's staged mount-link publisher

- Parent source `00e1d2efa475a2b693e26c481f230164285f1d05` replaced `run_torrent()`'s unchecked `unlink(link_path)` + direct final-path `create_mp_link_status()` with `transmission_mp_link_publish()`. Exact Process lifecycle `46091165092` and storage hotplug `46091165116` each failed only because their source contracts still demanded a direct adapter invocation from every caller. Their earlier test suites had already passed the preceding regressions.
- THIS checker follow-up makes the new Transmission helper the expected caller, validates that it obtains a staged link through the errno-normalizing adapter and publishes with `rename()`, and explicitly keeps the direct unlink/final-path creation forbidden. Existing MiniDLNA/aria2 adapter expectations remain unchanged.
- A separate storage-enable checker failure on parent `00e1d2e` (run `46091165081`) was updated in `b5b2339da4db5cf9c7a3e428cde5e3324028f844` to expect the new publisher while preserving the enable-read-before-side-effects rule.
- Next: check exact-SHA Process/storage/XY-C1/storage-enable/AI actions for THIS checker correction; no L3 or firmware build pass is claimed yet. Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Update Transmission run_torrent enable-gate source contract

- Source SHA `00e1d2efa475a2b693e26c481f230164285f1d05` replaces the unsafe `unlink(link_path)` with staged atomic publication. Exact storage-enable workflow `46091165081` passed the checked-enable C regression and failed only because its Python source contract still required the removed unlink call as the post-enable side effect.
- THIS checker-only follow-up expects `transmission_mp_link_publish(dest_dir,link_path)`, explicitly rejects direct unlink/final-path creation, and keeps the enable-status early return before that side effect. New policy/header test paths are covered by its push/PR filters.
- The new helper policy test passed locally under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. Parent exact checks already green: AI docs `46091165090`, firewall `46091165095`, NFS `46091165106`, Samba `46091165114`, vsftpd `46091165091`, MiniDLNA `46091165089`, IPv6 `46091165115`, libdisk `46091165100`, Firefly `46091165099`, Process inventory `46091165110`. Parent XY-C1 `46091165120`, storage hotplug `46091165116`, Process lifecycle `46091165092`, NVRAM `46091165098`, HTTPD `46091165085`, USB modem `46091165096` were in progress at last poll.
- Next: verify exact-head storage-enable, storage hotplug and XY-C1 for THIS follow-up; resolve any failures before another stability slice. Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Safely replace Transmission's /mnt compatibility link

- Found a concrete privileged filesystem issue in `services_stor.c:run_torrent()`: it unconditionally `unlink("/mnt/transmission")` before checking whether `/mnt` is a trusted real directory or whether the destination is actually a symlink. This could delete an unrelated regular file, and bypasses the later shell guard in `transmission.sh` when `/mnt` itself is a symlink. It also discards a previous working link if mount inventory or new-link creation fails.
- THIS slice replaces the pre-unlink with staged symlink publication. It verifies `/mnt` by lstat (root-owned, real directory, no group/world write), validates the destination as absent or symlink only, builds a temporary link using the complete `/proc/mounts` inventory, revalidates parent device/inode/owner/mode and destination, then atomically renames the staged symlink. A pre-existing regular file or directory is not removed; a failed discovery/publication preserves an old symlink. Added standalone GNU89 ASan/UBSan policy tests, workflow source contract, and XY-C1 header path trigger.
- Parent `8422435ebbd011065ced47c074fd722b0f36d16f`: exact USB policy regression `46090505594` PASS and AI docs `46090505656` PASS. The present source/test/workflow slice requires exact-head storage and XY-C1 verification; no L3/hardware claim.
- Next: check exact-head storage and XY-C1 Actions for THIS commit; fix any compiler/test issues before moving to another independent P1 defect. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Repair USB executable identity source-contract matcher

- Parent source `6939aa6b083e3fa3fa317b9c34ac0b4683d8480e` contains the corrected printer executable scan (removed accidental duplicate tail; preserves UNKNOWN from unreadable /proc executable candidates). Exact USB modem job `46090400869` compiled and ran the new GNU89/ASan/UBSan executable-policy test successfully; it failed only in the Python checker because the checker demanded bare `test_name();` call text while tests use `CHECK(test_name() == 0);`.
- THIS commit updates only `scripts/check_usb_printer_nvram.py` to check whitespace-normalized CHECK-wrapped test invocations. The policy test itself and production source are unchanged.
- Parent SHA exact Actions: AI docs `46090400943` PASS; Process lifecycle inventory `46090401067` PASS; USB modem `46090400869` FAIL at the checker matcher after all preceding policy binaries passed; Process lifecycle `46090400887`, NVRAM `46090400849`, storage `46090401078`, XY-C1 `46090401016` were PENDING at last poll.
- Next: verify exact-SHA USB/Process/NVRAM/storage/XY-C1/AI checks for THIS commit; fix any source/test failure, then move to next independent P1 reliability task. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Correct USB printer executable scan error aggregation

- Parent `84213bc5979cc8bdd4734b93c8e652092c539a35` introduced the scoped `/proc/PID/exe` identity gate. Review caught a duplicate old function tail caused by a source insertion offset, plus error aggregation where a later argv0-spoofed candidate could clear an earlier UNKNOWN readlink result. This follow-up removes the stale tail and preserves UNKNOWN until a confirmed real executable is found; the exact PID/start-time/argv revalidation remains in place.
- The policy test's failure message now emits an actual newline. Parent exact-SHA USB printer `46090313956`, Process `46090313942`, XY-C1 `46090313982`, NVRAM `46090313971`, storage `46090313925` and AI docs `46090313927` were all PENDING at the last poll; do not count those as verification.
- Next: verify every relevant check on THIS commit, fix any compile/test/source-contract failure, then return to another independent P1 reliability task. Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Prevent argv[0]-spoofed processes from satisfying USB printer readiness

- Confirmed process-identity gap: generic `process_inventory_scan()` accepts `argv[0]` basename matches even when the executed binary differs. A normal Linux process can choose argv[0] independently (`exec -a lpd sleep 5` reproduced cmdline `lpd` with `/proc/PID/exe=/usr/bin/sleep`), so a local unrelated process could suppress root's u2ec/lpd/p910nd launch by spoofing its title.
- THIS SLICE binds only these dedicated printer daemons to their installed executable identities: `/usr/sbin/u2ec`, `/usr/sbin/lpd`, and `/usr/sbin/p910nd` (for both p910nd and its intentional per-port argv[0] aliases). It revalidates PID/start-time/argv after readlink; unknown readlink/identity state is fail-closed. Generic process inventory semantics are unchanged to preserve BusyBox applets and other daemon aliases.
- Added a GNU89 ASan/UBSan policy test, exact source contract, and USB workflow path/compile step. This source has not yet been validated by exact-head Actions; no firmware build or hardware result is claimed.
- Parent SHA `375ab9f11b78d67a365b0c8885e1d5b617ebcc65`: Process `46089951824`, AliyunDrive `46089951825`, HTTPD `46089951826`, AI docs `46089951841` all PASS. Backup branch is still frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Next: inspect exact-SHA USB printer, Process, XY-C1 and AI handoff runs for THIS slice; fix failures before moving to another independent P1 stability task.

---

## 2026-11-27 — Repair duplicate PID 1 fatal-count assertion

- `ace9a30007db01654ad260423e46115322fcd972` updated one stale `router.count("return INIT_ROUTER_RESULT_FATAL;") == 6` assertion. The same source-contract script contains a second duplicated integration block at line 688; exact AliyunDrive run `46089841598` now passed the first block and all WebDAV lock/lifecycle tests, then exposed this second stale assertion.
- THIS COMMIT fixes the remaining duplicate to `>= 6`. Both integration blocks retain their explicit switch-node validation assertions; only the obsolete aggregate exact count is relaxed.
- Parent SHA exact checks: HTTPD `46089841552` PASS, AI docs `46089841639` PASS, AliyunDrive `46089841598` FAIL only at the second duplicate assertion, Process `46089841549` PENDING at last poll. Earlier HTTPD/AI on `dbbbba13342d9297f3f45ef15e956e03c92879fe` also PASS.
- Next: verify exact-SHA Process + AliyunDrive + AI handoff for THIS follow-up; then continue the next independent P1 service/storage/network stability issue. XY-C1 validation for the switch-node production source `78103b86` remains PENDING; no L3 claim. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Fix shared PID 1 boot-contract count after switch node hardening

- Source parent `78103b86d1e214dea0ad90cb72e9895c48b3df42` adds two legitimate FATAL gates for unsafe switch device nodes. Exact Process run `46089366834` and AliyunDrive run `46089366780` both failed only at `scripts/check_dev_init_boot.py:324`, which still pinned the older exact total of six. The GNU89/ASan/UBSan boot tests and WebDAV private-lock/lifecycle tests passed; the source checker below already validates each switch-node gate separately.
- THIS COMMIT changes only the aggregate assertion to `>= 6` and documents why; switch-node-specific checks remain strict. Process and AliyunDrive workflow path filters already include this script.
- Exact HTTPD + AI handoff checks on workflow-fix SHA `dbbbba13342d9297f3f45ef15e956e03c92879fe` PASS (`46089569822`, `46089569786`). Exact XY-C1 build for switch source `78103b86` remains PENDING at last poll; no build/L3 pass claim.
- Next: verify the exact-SHA Process, AliyunDrive, AI handoff and XY-C1 workflows for THIS checker correction; repair any failure, then return to an independent P1 stability defect. Backup `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Fix workflow block indentation for RC fatal-gate contract

- Parent `9356c97e7587c6c330370a6b7981a775ecb649b6` updated the stale exact-count assertion but static diff review caught that its replacement assertion lacked the YAML run-block indentation. Corrected the indentation before accepting any Actions result.
- Workflow-only correction; no product source changed. The exact-head HTTPD and AI handoff runs for this new commit are the validation gate.
- Next: inspect those exact SHA runs, fix any genuine failure, then resume an independent P1 userland stability issue. Frozen backup remains untouched at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — RC boot-fatal assertion accommodates secure switch-node gates

- Parent source SHA: `78103b86d1e214dea0ad90cb72e9895c48b3df42` (secure RTL8367/MediaTek device-node validation). Its exact-head HTTPD Authentication and Security Regression failed only in the embedded RC NVRAM source contract: the legacy assertion expected exactly six `INIT_ROUTER_RESULT_FATAL` returns, while the two newly required switch-node gates raised the count. The NVRAM sanitizer policy test passed; this was a stale checker expectation, not an observed production defect.
- Correction changes the broad contract to retain the six baseline fatal gates without hard-coding the total; the dedicated switch-device policy checker still validates the node gates. No production source change.
- Next: inspect exact-SHA HTTPD and AI handoff Actions for THIS COMMIT; if green, resume the next P1 userland stability item. Other independent jobs on parent SHA `78103b86` were still pending at last check. Frozen backup remains untouched at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — Secure RTL8367/MediaTek switch device-node initialization

- Last verified SMP status commit: `3c0ea0688d130bee8a0e78adb846155f666fd415`; exact-SHA process-lifecycle, inventory, NVRAM audit and AI handoff Actions all completed PASS.
- New stability/security slice: the two switch node setup routines used privileged `mknod` but discarded errors. A pre-existing symlink or wrong device node returns EEXIST, yet init_router proceeded to paths that open the unvalidated node. New shared policy validates with `lstat` and requires a root:root, single-link, exact 0600 character device with the expected rdev. Unsafe/missing nodes now stop router init before networking. Focused unit test and exact-SHA source checks are wired into process-lifecycle CI.
- Next: verify this commit's exact-SHA checks and XY-C1 firmware build, repair any genuine failures, then continue the P1/P2 backlog. Frozen backup remains untouched at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — SMP affinity regression test structure follow-up

- Implementation commit: `cbbca809367522b5cffe22923296aab1e21078ce`. Its exact-SHA workflow matrix was queued/in progress when last queried; no PASS claim yet.
- Follow-up gives the errno unit test explicit `test_optional_missing_nodes_do_not_degrade_startup()` and `test_real_write_errors_are_preserved()` functions, matching the source-contract checker assertions. Runtime code remains unchanged.
- Next: inspect exact-SHA Actions for both cbbca809 and this follow-up, fix any actual failure before resuming broad userland work. Frozen backup remains untouched at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-27 — SMP affinity write-status propagation

- Previous slice `6161145650c67c857bf7c5c64f1176c7134af4dd` has exact-SHA Redsocks lifecycle, process-inventory and handoff Actions PASS. No local repository checkout is available in this tool environment.
- New P2 fix: `smp.c` silently discarded errors from IRQ affinity and RPS/XPS pseudo-file writes, so `init_router()` could report READY despite real CPU/network queue tuning failures. The code now returns and aggregates write errors, ignores only optional ENOENT nodes, adds errno tests, and routes the result into existing DEGRADED startup aggregation. PPP ip-up logs PPPoE RPS failures without changing WAN route status.
- The SMP errno test and source-contract checks are wired into push/PR process-lifecycle Actions. Next: verify this commit's exact-SHA checks, fix any failures, then continue an independent userland stability defect. Frozen backup remains untouched at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Redsocks directory permissions and checker PID literal

- abf383ce1781943f598562c4b1a90210de6eb4ee: Redsocks shell syntax PASS; full lifecycle regression PASS; static checker FAIL because it expected the staging path to end with a single $ instead of $$ (the actual shell script uses the PID suffix). AI handoff workflow PASS.
- Follow-up restores chmod 700 on an already-existing config directory before stage creation and process changes, propagates failure, adds a 0755-to-0700 regression, and corrects the static check to literal $$.
- Verify this exact commit's Redsocks + handoff Actions, then continue independent stability work. Frozen backup remains untouched at 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-28 — Redsocks stage regression passes; static checker assertion normalized

- 8bde38fdf22bfd43b9bd0aaa04b55bb2ce4cf225: bash -n and complete Redsocks lifecycle regression PASS (including symlinked staging directory and destination publication race). The source checker failed at the final STARTED-log assertion because it counted only the legacy unquoted form; runtime code uses the safely quoted "$BINARY_NAME" form. Checker now accepts both and still requires exactly one STARTED log.
- Exact-SHA AI handoff workflow 46087370951 PASS; process-lifecycle inventory 46087370983 was in progress at last check. Previous SHA 5387 was a genuine FAIL because noclobber accepted the test symlink; the mkdir fix addresses that observed failure.
- Next: verify this checker-fix SHA Actions; then proceed to an independently identified boot/init defect. Frozen backup remains untouched at 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-28 — Correct Redsocks staging reservation after CI failure

- Commit 5387a1db85a9ed969c7f5ff3e18ea508a3a59636 introduced staging with shell noclobber, but its exact-SHA Redsocks lifecycle job failed: the pre-existing staging-symlink test was accepted. Bash syntax passed; the regression suite failed before static source-contract validation. Process inventory and handoff-record jobs passed.
- Follow-up replaces noclobber with atomic mkdir of a PID-scoped private staging directory (mode 0700), places the config inside it, then publishes via exclusive hard link. Regression now uses a symlinked staging directory and checks no daemon signal or target mutation. Verify exact-SHA job; do not assume pass.
- Continue to an independent userland stability issue while CI runs. Frozen backup remains untouched at 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-28 — Atomic Redsocks config publication

- Generated config now reserves a private same-directory staging file with noclobber before any daemon stop, writes the config there, then publishes by exclusive hard-link creation. This closes the check-then-redirection symlink window. Regressions cover a pre-existing stage symlink and a destination symlink created at publication time.
- Exact-SHA Redsocks, Process inventory, AI handoff docs and XY-C1 checks are unverified. Combined status and the PR-only workflow-run wrapper returned no push check/run results; do not infer PASS from empty results.
- Next: inspect exact Actions for the preceding f3960ba path-symlink fix and this commit, then continue an independent P1 RC startup-result or USB-printer readiness issue. Frozen backup remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-28 — Reject Redsocks config-directory and file symlinks

- Follow-on to endpoint preflight: `[ ! -f "$REDSOCKS_CONF" ]` treats a dangling symlink as absent, and the config heredoc follows it when creating the file. A symlinked `REDSOCKS_DIR` also redirects root writes into another storage path.
- Added `redsocks_config_paths_validate` before process lifecycle changes. It requires `REDSOCKS_CONF` to be the expected direct child, its parent directory to exist and not be a symlink, the config directory (if present) to be a real directory, and an existing target to be a regular non-symlink file. Revalidates after creating the storage directory. Tests cover direct directory and dangling leaf symlinks; invalid paths must not stop the active daemon or modify the referent.
- Rebuilt the Redsocks test file from the clean `c149a1f850ba7f074d5da757ed760b755284c017` baseline so each lifecycle/firewall test occurs once; this also preserves and re-adds the endpoint-injection regressions without the malformed duplicate block.
- Endpoint fixture repair at `2a2e5af846aa826b7d1613191b099e6077ec73b5` had not yet completed exact-SHA Actions at last poll; `9309abef` Redsocks workflow #46085873079 failed at `bash -n` due truncated test insertion; Process inventory #46085873084 PASS, AI handoff #46085874162 PASS. Verify current commit's Redsocks and process workflows.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Fix Redsocks endpoint-test shell syntax before rerun

- `9309abef4f9813d650ff41d6b0075f4567260fab` added pre-stop SOCKS5 IPv4/port validation and regressions, but exact Redsocks workflow #46085873079 failed immediately at `bash -n scripts/tests/redsocks_lifecycle.sh`: an incomplete insertion left the test file ending a section with bare `for bad_ip in`. No runtime regression tests executed. Replaced the malformed block with complete endpoint cases; re-run the Redsocks exact-SHA suite.
- On `9309abef`, AI handoff #46085874162 PASS and Process inventory #46085873084 PASS. Exact Redsocks suite FAIL (syntax only). The source endpoint preflight remains in place; this commit repairs tests, not production logic.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Harden Redsocks generated endpoint configuration

- Found that `func_start` interpolated caller-supplied SOCKS5 address and port into a root-generated config without validation; embedded newlines could append arbitrary Redsocks directives. The routine also stopped the existing daemon before validating input or checking that an endpoint existed.
- Added strict IPv4 validation and decimal port normalization/range check (1–65535) before any process signal or config creation. If a config is absent and no complete endpoint is supplied, start now returns the existing no-op result without stopping a healthy daemon. Tests cover injected newlines, invalid/out-of-range inputs, preservation of the original RUNNING process, and leading-zero normalization.
- The prior SmartDNS exact-SHA `380918ce60cb7d955d57b90ca42868b1df964de5` DNSMasq #46085481393 PASS; process regression #46085481385 PASS; process inventory #46085481419 PASS; USB storage #46085481386 PASS; AI handoff #46085481469 PASS. XY-C1 #46085481414 still pending at the last poll.
- Current main before this Redsocks slice was `c149a1f850ba7f074d5da757ed760b755284c017`; Transmission root symlink regression is part of this commit's process workflow. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-28 — Harden Transmission storage-root trust boundary

- Found a root-level storage symlink gap: `func_start` only used `[ -d "$DIR_LINK" ]`; that succeeds when `/mnt/transmission` is a symlink to an attacker-controlled directory. Descendant-only `transmission_require_dir` checks therefore did not protect privileged chmod/chown operations from a symlinked storage-root or immediate parent.
- Added `transmission_require_storage_root` before any descendant path derivation, requiring the root and its parent to be real directories. Regression tests cover real/missing roots, direct root symlinks, symlinked parents, and preservation of the symlink target.
- SmartDNS exact-SHA `380918ce60cb7d955d57b90ca42868b1df964de5` Actions: AI handoff #46085481469 PASS; DNSMasq #46085481393, process regression #46085481385, process inventory #46085481419, USB storage #46085481386 and XY-C1 #46085481414 were still in progress at last poll. The previous DNSMasq attempts at 53c095 and a0cba failed at distinct integration test fixture issues, now addressed by source helper loading and conditional validation of the optional dual-stack threshold. Exact rerun still required.
- Frozen backup stays at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Next: exact-SHA process/DNSMasq checks for the Transmission patch and SmartDNS fix chain; then continue a distinct stability/security issue.

---

## 2026-11-28 — Keep optional SmartDNS dual-stack threshold backward-compatible

- The new base-token validation initially treated `sdns_ip_change_time` as an unconditional numeric field, while the daemon only emits that threshold if dual-stack IP selection is enabled. Validate the value only when `sdns_ip_change=1`, preserving older NVRAM with an empty optional threshold when disabled. Regression now covers both cases.
- DNSMasq integration fixture at `a0cba1e2efe18728173493e0fa74b0b3b4956854` sources `smartdns_config_file.sh` alongside the DNSMasq helper so the extracted production generator has its token validator. The prior run failed before executing this fixture; exact rerun pending.
- Current code-security patch chain: `53c09530c676ef0096ca2a89a2175212cec47fd0` adds base and static upstream token validation; `a0cba1e2efe18728173493e0fa74b0b3b4956854` adds the missing integration-helper fixture dependency. No passing DNSMasq result yet.

---

## 2026-11-28 — Align SmartDNS integration fixture with config-token dependency

- Exact DNSMasq lifecycle run #46085311741 on `53c09530c676ef0096ca2a89a2175212cec47fd0` reached the SmartDNS/DNSMasq runtime ownership test and failed because that extracted-function fixture did not source `smartdns_config_file.sh`, so `smartdns_config_token_valid` was undefined. The production launcher already loads this helper; the test fixture now sources it as well.
- That run passed SmartDNS process-state, adblock preservation, firewall status, resolver ownership transitions and earlier DNSMasq cleanup checks. AI handoff #46085311728 PASS. Process inventory #46085311726, USB hotplug #46085311757 and XY-C1 #46085311799 were still running at last poll. Re-run DNSMasq exact-SHA after this fixture-only repair.
- Current security change remains `53c09530c676ef0096ca2a89a2175212cec47fd0`: base SmartDNS token/numeric validation before staging truncation and static upstream validation before directive emission. Do not claim the whole regression PASS until its rerun completes.

---

## 2026-11-28 — Harden SmartDNS generated configuration tokens

- At base `main` `e6b1ca75ca28153f5bff8d79abfc66714f60152f`, SmartDNS emitted NVRAM values directly into generated daemon configuration. Newlines in server name, speed mode, numeric settings or static-upstream address/group/port could inject extra directives. Base tokens/numbers/selectors are validated before staging is truncated; static upstream fields are validated before emitting each endpoint.
- Added generator regressions for base and static-upstream newline injection. DNSMasq lifecycle workflow includes `smartdns_config_file.sh`; verify its exact-SHA result and XY-C1 build before claiming validation. No local test or firmware build is claimed.
- Checked-NVRAM contract repair at `e6b1ca75ca28153f5bff8d79abfc66714f60152f`: AliDDNS #46085053434 PASS, AI handoff #46085053405 PASS, Process inventory #46085053436 PASS, and Process lifecycle #46085053411 PASS.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Next: verify SmartDNS/DNSMasq and XY Actions, then continue another userland stability defect.

---

## 2026-11-28 — Repair checked NVRAM startup contracts and log archive inventory

- Base `main` was `71d3bf2a3788f2ce59c1e5809c383a26841b2657`. That commit migrated the shared autostart NVRAM reader to `nvram get_checked`; exact Process regression #46084656129 then failed because its test stub only accepted legacy `get`, so the AdGuardHome enable read failed and the expected launcher was not called. AliDDNS #46084656095 reached the final source-contract step and failed because its checker still expected the legacy command. Runtime AliDDNS lifecycle, transport, CA bootstrap and HMAC tests preceding that checker passed.
- Updated both stale test contracts to checked reads. The same checkpoint exposed an AI-handoff inventory mismatch: archive shards are indexed by repository-root-relative paths in table rows, not only relative Markdown links; added the missing part-039 table entry.
- Base-SHA checks: Process inventory #46084656164 PASS; FRP #46084656173 PASS; DNSMasq #46084656127 PASS; HTTPD security #46084656049 PASS. Process lifecycle and AliDDNS need exact-SHA reruns after this repair. XY-C1 run #46084656050 was Build IN_PROGRESS; TRX verification and artifact upload PENDING at last check.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not update. Next: verify this commit's Process, AliDDNS and AI handoff Actions; poll the XY-C1 run without waiting; then continue a new userland stability defect. No local build/test result is claimed.

---

## 2026-11-28 — Complete DHCPv6 crash-durable configuration publication

- DHCPv6 IA configuration publication fsyncs staged contents after `fflush`, closes the file, atomically renames it, then fsyncs the parent directory. The shared parent barrier rejects a symlinked parent via `O_NOFOLLOW`; post-rename failures are returned because durability is uncertain after rename.
- The C regression covers success, missing and symlinked parent paths, and an injected parent-sync failure. Initial exact run on `8b1aa83d2f82da8743350bdc2aa056ebfa764649` found a test expectation mismatch after the injected post-rename failure; `0eef8eb09fd535d6266426ec1fa8a0b5d3ed8093` corrected the expected on-disk content. Verify the 0eef exact-SHA rerun and companion WAN IA callback regression before claiming PASS.
- Concurrent WAN IA callback work was preserved. No firmware L3 or physical L4 result inferred from workflow completion.
- Root log rotation: 100 oldest records archived verbatim to `docs/dev-log/2026-11/part-039.md`; newest 20 retained in root and index updated.
- Next: inspect exact-head DHCPv6/WAN IA tests, Process inventory and AI handoff; close modem/OpenVPN XY-C1 Build → TRX → artifact; continue an independent P1 issue.

---

## 2026-11-28 — Make USB modem identity metadata durable

- The USB modem metadata writer fsynced the staged file and atomically renamed it, but never synced the parent directory; its remover also reported success immediately after unlink. A sudden power loss could therefore lose a newly assigned USB identity or resurrect a removed stale `wwanX` identity. Both now fsync the parent after rename/unlink; directory preparation also syncs the parent after verifying the root-owned directory.
- Added missing-parent and symlink-parent checks to the existing metadata C regression. USB modem identity workflow now includes the shared durability helper and asserts post-rename/post-unlink ordering. Exact-SHA USB modem/Process inventory/AI docs/XY-C1 checks pending; no local compile claimed.
- Previous OpenVPN durability commit `48b617d5933f9b7bb88fe8ed194b1682985d3d7d`: exact OpenVPN tests and source contract pending at last poll. Proxy Process rerun #46082590403 and AI docs #46082590407 were in progress; #91db XY-C1 #46082011789 had completed success and required Build/TRX/artifact step inspection.
- Next: inspect USB modem metadata durability regression and exact-head Actions, then continue another source stability/security defect while independent Actions remain pending.

---









## 2026-11-27 — Repair exact-SHA Shadowsocks contract and handoff guard

- Production/test baseline: `f23be0cff7d7e0d1e942ebcf10094497b73be676`. BusyBox 1.24 status-1 fallback and fake-procfs regressions remain the production scope; no repeated source review.
- Its Shadowsocks regression #46079286423 failed only because the static checker searched for an incorrectly escaped procfs command expression; runtime/shell regressions and Process inventory #46079286495 PASSed. AI docs #46079286331 failed because the roadmap exceeded its 131072-byte limit by 317 bytes.
- Corrected the source assertion and compressed the stale governance entry. Shadowsocks #46079578628 and AI docs #46079578625 now PASS on `20e0a312051addf538ed6bf4ae85022e608ed4b3`. XY-C1 #46079286427 remains in progress on f23; Build was running and TRX/artifact pending at the last poll.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.


## 2026-11-26 — Autonomous development checkpoint

- Current `main`: `3cc85ff726be701162e820a6fed0f282f5eba355`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (not modified).
- This session's DNS Forwarder work:
- `fix(dns-forwarder): report CLI startup failures`: `93f2b137ea212a5fbc857fae09faec384bcb519b` — XY-C1 Firmware Validation #45848922656: in_progress/—; Process lifecycle inventory - 93f2b137ea212a5fbc857fae09faec384bcb519b #45848922607: completed/success; NVRAM transaction boundary audit #45848922583: completed/cancelled; DNS Forwarder launch path stability regression #45848922574: completed/failure; AI handoff documentation contract #45848922565: completed/success
- `fix(dns-forwarder): unwind listener startup failures`: `9a6d645f10ea2032819ac63467c0cdb2a10ca29f` — NVRAM transaction boundary audit #45848220502: completed/cancelled; Process lifecycle inventory - 9a6d645f10ea2032819ac63467c0cdb2a10ca29f #45848220500: completed/success; AI handoff documentation contract #45848220397: completed/success; XY-C1 Firmware Validation #45848220389: in_progress/—; DNS Forwarder launch path stability regression #45848220378: completed/failure
- `fix(dns-forwarder): validate upstream address and port before setup`: `a039b68d37cb3eb5f2acdd4ae7d448c6851d5223` — NVRAM transaction boundary audit #45847194072: completed/cancelled; AI handoff documentation contract #45847194027: completed/success; XY-C1 Firmware Validation #45847193944: completed/success; DNS Forwarder launch path stability regression #45847193941: completed/success; Process lifecycle inventory - a039b68d37cb3eb5f2acdd4ae7d448c6851d5223 #45847193923: completed/success
- `test(dns-forwarder): correct event-loop compile inputs`: `050365be1040bd88b2c06d0942f0d43f3f92dab6` — AI handoff documentation contract #45846263346: completed/success; DNS Forwarder launch path stability regression #45846263342: completed/success
- `fix(dns-forwarder): roll back failed event source registration`: `3ccc3c94fe83bc0acdcf5c4a86aa70e302040bb6` — AI handoff documentation contract #45846000394: completed/success; Process lifecycle inventory - 3ccc3c94fe83bc0acdcf5c4a86aa70e302040bb6 #45846000350: completed/success; NVRAM transaction boundary audit #45846000085: completed/success; DNS Forwarder launch path stability regression #45846000054: completed/failure; XY-C1 Firmware Validation #45846000051: completed/success
- Implemented: strict ASCII-decimal upstream port parsing (1–65535) and pre-resource upstream validation; transactional listener initialization that checks `FIONBIO`, `SO_REUSEADDR`, bind, source allocation/registration and unwinds all partial resources; CLI startup now propagates failures and correctly tracks ownership of the DNS-port string.
- Host regressions: parser boundary/error behavior and event-loop registration rollback/re-attach tests are wired into `.github/workflows/dns-forwarder-launch-path.yml`. Hardware/router runtime testing has not been performed here.
- First next task: add deterministic failure-injection coverage for DNS Forwarder constructor setup (`getaddrinfo`, `socket`, `ioctl(FIONBIO)`, `setsockopt`, bind and event-source registration), then fix any remaining event-loop/daemon shutdown defects. Continue checking the Actions attached to each exact commit SHA; do not repeat completed startup/handoff audits.

# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`; current base for the WSD HTTP-contract checker correction: `c4a627632235f2e240797a41294e5dc062848f06`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged; do not modify). PR #1 remains closed and unmerged.
- Bootstrap/protocol recovery was completed earlier in this continuous project; do not repeat it. Work on `main` only with full-tree atomic commits and non-forced expected-SHA updates.
- Exact-SHA evidence: MiniUPnP event-timeout subscriber regression #45845370609 and AI handoff #45845370434 PASS on `a271fcc0f818b834b21fe9681717a18dd07f3c50`; U2EC bounded TCP framing #45846984410 PASS on `4d146475b0a34f4971a364dd5ece2775dc433f8a`. DNS Forwarder source checks on `a039b68d` and its XY-C1 run were in progress at last poll.
- Latest confirmed full L3 evidence is production source SHA `b81ac7815d7fbe3b4573f742b00cbdba19c2e0d7`, XY-C1 run #45844543988 with Build XY-C1, TRX verification and artifact upload all PASS. Later production descendants require their own source-triggered build. No physical-router/L4 validation is claimed.
- Recent source work includes MiniUPnP event timeout parsing/deadline hardening, networkmap ARP socket lifecycle fixes, DNS Forwarder event-source rollback/upstream input validation (`3ccc3c94` and `a039b68d`), and U2EC TCP receive timeout hardening (`2b6d5db9`).

## Active task

- WSD SOAP Action/MessageID parsing is hardened at `c4a627632235f2e240797a41294e5dc062848f06`: raw `strstr()` tag hunting has been replaced with a bounded XML token scanner, and the GNU99 ASan/UBSan parser/reflection regression passed (#45847368757).
- The same source commit exposed a stale independent HTTP-framing checker: it still expected the old internal `strchr()` nested-tag implementation, rather than the new delegating bounded scanner. That exact-SHA workflow failed only at this obsolete source assertion (#45847368988). Updated the checker to assert the bounded wrapper and helper contract; this regression rerun is pending on the next commit.
- XY-C1 build for `c4a6276` had passed production-input detection and host checks and was compiling Build XY-C1 at last poll (#45847369057); TRX/artifact remain unverified until the run completes. No physical L4 validation.

## Completed production work — LAN DHCP resolver transaction

- Confirmed that the automatic LAN DHCP resolver publisher could return failure after `rename()` had already exposed the candidate `/etc/resolv.conf` (for example, a parent-directory fsync failure). The outer LAN DHCP transition could then restore the prior kernel address/routes while leaving the candidate resolver visible.
- Production fix `0c897b7ee4fd80bddad936b7d5bc3d8433917676`: expose the rename-completed boundary from the internal staged-file publisher while preserving the original API; add rollback-aware publication using the existing hard-link snapshot/restore primitives; route the automatic LAN DHCP resolver update through the rollback-aware helper.
- Deterministic GNU89 C regression injects a failure after successful rename and verifies restoring an existing resolver and removing a newly created resolver when no previous file existed. Follow-up diagnostics in `72225a52c467a09dcdd5a4e252451d8d9df7eaf1` distinguish rollback failure/durability uncertainty without falsely claiming a recovery link still exists.
- Checker/fixture follow-ups: `57bd20e60f73f20d41fed7f08fee5fc5bdde7448`, `24ec9ce37430eaadcdfd3edc3f325ed4737f25b4`, `e428e722c2fbabde120a832bf93316bcf513d7ea`, `8f96eb6ecee967d448667072393aa99758079e7e`, `8477a7777fbd49717ae89a93011671500ce1a9c4`, and `b25835d921e2f3d9b3ae8fec39cb93549aa26e5e`.

## Completed production work — WAN / IPTV DHCP BOUND callback status

- Confirmed that DHCP BOUND callbacks consumed the aggregate return from `wan_up_internal()` / `man_up_internal()`. Those aggregates include ancillary service startup failures after address/route setup (firewall, multicast/IGMP, DDNS, watchdog, monitoring and other follow-up services). A nonzero ancillary status could therefore make BusyBox treat an otherwise applied lease ACK as a failed script callback and unnecessarily retry/rebind.
- Production fix `c36ab9c49ebd68fde9b07454da65e19212c182fd`: introduce `wan_up_internal_with_route_status()` and `man_up_internal_with_route_status()`, which report the DHCP-critical address/IPv4-route result before ancillary services, while retaining old aggregate wrappers for existing non-DHCP callers. `udhcpc_bound()` consumes the scoped status.
- Production fix `15274181953dc258d6cec4e47d9038d2e3187b47`: generalize the lease callback selector to `dhcp_bound_callback_status(route_setup_status, mtu_status)` and use it for both normal WAN and IPTV DHCP. IPTV multicast startup remains separately logged/managed and no longer rejects a lease ACK; route, MTU and required NVRAM/retry-state publication errors remain visible to BusyBox. The MAN wrapper source-signature compatibility adjustment is `e49e86262257c592ad1e0be1b68cccd49c476517`.
- Added/extended GNU89 policy regression `test_bound_callback_status_ignores_auxiliary_service_failures()` and source contracts for the route-only boundary. Checker follow-ups: `45cb6f12843035d47ce61dd2d4fa6dd58cd39e79`, `3f54cf2ba737835e9ea1a028fbe0fd229fe113e9`, `cbb470d0054854b3d8469a699ce78601f537c881`, `2377c6094e9acc5358a8abc6873f039eb057438f`, `f14f2b4d4d450f8dec403ed17ca74533ba31d204`, `e41af1199ab8279421fbed098557181767b92f01`, `405bb48aad75b5ea89aaec668a1b1f79fb140f97`, and `21822988b5b299e4f931d6659c175515952bdc57`.
- WAN callback follow-up on exact SHA `405bb48aad75b5ea89aaec668a1b1f79fb140f97`: WAN DHCP callback #45824870841 PASS. The route-status workflow failed on stale assertions against the old callback source shape; those assertions were aligned in `21822988b5b299e4f931d6659c175515952bdc57`; exact rerun #45825386107 was QUEUED at the last poll.

## Completed production work — DHCPv6 config-writer token validation

- `start_dhcp6c()` now validates the interface token with `wan_ifname_copy()` before generating `dhcp6c.conf`. DHCPv6 IA #45835968592 PASSed with the GNU89 ASan/UBSan adversarial-token test; dependent WAN DHCP contract correction #45836670352 PASSed.

## Verification boundary

- Latest main parent before this source slice: `f2d7d6907939b93745eb67fdb51f89d75ba2ffc0`. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- DHCPv6 config-token validation source `3cb9d6b909da6ef09656567d244aee26d4a3c4ff`: dedicated DHCPv6 IA/source-contract #45835968592 PASS; dependent WAN DHCP contract correction #45836670352 PASS.
- AdGuardHome resolver-failure source `bbcfd8532dec133ff8c60b690f8aba9017af7acc`; test/checker descendant `b4758abe4d81752f604013d6819ce4ba9d29e9c6`: exact config-security #45837257425 PASS.
- XY-C1 #45837257027 on `b4758abe` PASSed host checks but skipped Build XY-C1, TRX verification and artifact upload because that commit changed only tests/checkers. Last confirmed full L3 build remains #45831671503 on production source baseline `43ba9c6`, artifact ID `15123331261`, SHA-256 `19dfd06a2ad7c6427a55e26505552a8a3bfbc6f6e1bff2259304426261f2bc9c`; it does not validate subsequent DHCPv6, AdGuardHome, networkmap, SmartDNS or LAN DHCP source changes.
- Physical-router/hardware L4 remains NOT VERIFIED. Never treat a green run that skipped Build/TRX/artifact steps as a firmware build.

## BusyBox udhcpc repeated-REBINDING failure regression

- Added `test_repeated_failed_rebinding_acks_preserve_server_and_deadline()` and source checks ensuring repeated callback failures cannot commit the alternate server, requested IP or BOUND state, or extend the old lease deadline.
- Exact test #45825922148 PASS on `7be435059c1f8ffbd757ea95ba41fe466557960f`. Production source remained unchanged because the tested review did not establish a state-machine defect.

## Inadyn SIGHUP configuration transaction

- `notify_ddns_update_locked()` snapshots the last-known-good config before replacement, restores it on write failure, retains the recovery link when SIGHUP delivery is uncertain, and commits the snapshot only after successful signal delivery and parent-directory sync.
- GNU89 rollback regression added. Production commit `4c0b2a02b021c813845e3a5356cdfd0cf987c4ab`; fixture correction `916fbec0fcc6739548870888fdbb24216683955c`; checker fixes `2b2b6ec122e5b4e9c7f94c3599ffd0c4da60c526` and `ece28b515b546cd8539089975322cb79ecbe6395`. Inadyn #45826971290 and DNSMasq #45826971254 PASS.

## VSFTPD configuration durability

- `ftp_config_file_publish()` flushes and fsyncs staged data, renames it, then fsyncs the parent directory with EINTR retry. If parent sync fails after rename, the error reaches `run_ftp()`, which refuses daemon launch. Parent lookup follows configured `/etc` resolution.
- GNU89/Werror regression and source-order contract passed VSFTPD #45828730341 on `0eaada3`. Exact XY-C1 #45829100040 on `f0fbe839` remains the required product build check.

## Exact first action for the next GPT

1. Inspect the exact-SHA WSD HTTP framing contract rerun; verify the updated checker no longer requires the removed `strchr()` implementation and that parser and XML token scanner invariants remain asserted.
2. Check the XY-C1 run for `c4a6276`; require Build XY-C1, TRX verification and artifact upload to PASS before marking this source SHA L3-verified.
3. Continue the next concrete P1 stability/security repair in another userland component. Do not repeat the WSD scanner review; frozen backup remains untouched and hardware/L4 remains NOT VERIFIED.

## Mandatory handoff checkpoint

Every session MUST perform an end-of-session accuracy checkpoint. Check the live `main` SHA, frozen backup SHA, exact-head Actions, completed/failed/pending tests, unresolved blockers, and the precise first action for the next session. Earlier-SHA PASS results do not validate a later SHA. **The checkpoint is mandatory; a document edit is conditional.** When all checkpoint facts and the first action are already accurate, conclude the check without a no-op file change or commit.

# Cross-session execution mandate

- Treat the repository as one continuous engineering effort across GPT conversations. Recover once, resume the exact unfinished task, and do not ask the user what to do next.
- Complete each logical source/test/workflow slice with one atomic main commit: create every required blob, one complete tree, and one commit; update `main` only through a non-forced expected-SHA compare-and-swap. Never create partial-file commits or alter the frozen backup.
- After every commit, inspect Actions for that exact SHA. PASS permits the next slice; FAIL requires diagnosis and repair; PENDING is not a stop condition while independent work remains.
- Before context exhaustion, finish and commit a safe bounded slice, verify its exact-SHA tests/build boundary, and update this handoff with the committed baseline and precise first action. Keep production-source and documentation-only SHAs distinct.
- Preserve hardware/L4 NOT VERIFIED unless actual hardware validation has been performed.

## 2026-11-26 GPT-6 continuation

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`; HEAD at handoff: `1e3154270732eef0314244dec117dd325971638a`.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (not modified).
- This session's source work before this handoff:
  - Samba paired passdb/config journal directory durability barriers; committed recovery-state logging is now truthful.
  - Ralink IAPP: bound driver-returned event-table length/count, copied packed event payloads to aligned storage, and corrected Linux TCP send/connect to use absolute monotonic deadlines.
  - DNSMasq: staged config fsync/parent sync and durable rollback links; failed start/restart restores the previous config and static-ARP snapshot only after daemon absence is confirmed. When process state is RUNNING/UNKNOWN, preserve the current config and retain rollback links for manual recovery.
  - Inadyn: introduced a private, inode-checked config hard-link snapshot; stage and publish keeps the previous config available through startup confirmation, and a confirmed-absent failed launch restores the previous config and restarts the prior instance. If daemon presence remains RUNNING/UNKNOWN, retain the snapshot instead of replacing a config that may still be in use. Host regressions cover restore-after-publish, first-file removal, successful commit, and symlink rejection.
- Inadyn follow-up hardening: snapshot restore now explicitly unlinks the extra backup hard link when destination and backup still reference the same inode. Plain `rename(backup,destination)` is a no-op for two hard links to one inode; without this branch the backup link could remain and violate the daemon's single-link config validation. A regression covers restore before any new config is published.
- Recent main commits (newest first):
- 1e3154270732eef0314244dec117dd325971638a fix(inadyn): release unchanged rollback hardlink safely
- 07bd0e547e92e9b6604f548872c489631afd693f test(inadyn): expect committed config after snapshot regression
- 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be fix(inadyn): restore last-known-good config after failed launch
- 89e7b85aebfeefa52496b4ef5480cf46c2c1372e docs(ai): checkpoint DNSMasq and QMI stability work
- 992bdf6da9ba45f217c88bdf42afea69932b45e7 docs(dnsmasq): clarify retained rollback link recovery
- 3147c02f2bddba93a6034e529c5d8935b2768a05 test(dnsmasq): verify rollback before success commit
- 9166a3d4f836612e0545cb0b255ba252674323e2 test(dnsmasq): distinguish stop-only and launch commits
- Targeted Actions attached to handoff HEAD:
  - WAN route operation status regression: completed/failure (run 45816775778, SHA 1e3154270732eef0314244dec117dd325971638a)
  - Process lifecycle inventory - 1e3154270732eef0314244dec117dd325971638a: completed/success (run 45816775719, SHA 1e3154270732eef0314244dec117dd325971638a)
  - NVRAM transaction boundary audit: in_progress (run 45816775763, SHA 1e3154270732eef0314244dec117dd325971638a)
  - DNSMasq lifecycle stability regression: completed/failure (run 45816775693, SHA 1e3154270732eef0314244dec117dd325971638a)
  - Inadyn fail-closed lifecycle regression: completed/failure (run 45816775662, SHA 1e3154270732eef0314244dec117dd325971638a)
- Inadyn checks for initial integration SHA `5fe2ed6d4c9e79c011f7c141a96b1f8744a040be`:
  - XY-C1 Firmware Validation: in_progress (run 45816170557, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - HTTPD Authentication and Security Regression: completed/success (run 45816170578, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Process lifecycle inventory - 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be: completed/success (run 45816170449, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - miniupnpd atomic configuration publication regression: completed/success (run 45816170951, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Samba share configuration stability regression: completed/success (run 45816170687, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - DNSMasq lifecycle stability regression: completed/failure (run 45816170586, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - WAN DHCP callback state stability regression: completed/success (run 45816170308, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Inadyn fail-closed lifecycle regression: completed/failure (run 45816170673, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - WAN route operation status regression: completed/failure (run 45816170314, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Process inventory and lifecycle fail-closed regression: completed/success (run 45816170451, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - NVRAM transaction boundary audit: completed/success (run 45816170429, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
- Local/host test and firmware build results are only those explicitly represented by the Actions records above; do not infer a full XY-C1 build from host policy tests. Any PENDING or FAIL state above must be resolved or superseded by an exact-SHA rerun.
- First next task: inspect the exact-SHA Inadyn lifecycle workflow for the latest snapshot commits, resolve any failed host regression/compile step before expanding scope. Then review `notify_ddns_update_locked()`'s write-plus-SIGHUP failure semantics and decide whether its config should be snapshotted and rolled back; preserve the current contract unless evidence shows a defect.
- Keep commits atomic: complete all blobs, create one Git tree, one commit, then non-force update `main` with `expected_sha`. Do not touch the frozen backup branch.

