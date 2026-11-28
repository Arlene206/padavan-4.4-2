## 2026-11-28 — Bound Inadyn configuration fsync retries

- The Inadyn configuration publisher still had two private unbounded EINTR loops: one for staged-file fsync before rename and one for the parent-directory barrier used by snapshot/commit operations. A persistent signal stream could pin PID 1 while preparing or rolling back DDNS configuration.
- Routed both barriers through the shared `config_file_sync_fd()` policy with its eight-interruption ceiling. The failure path preserves errno while closing the descriptor, retains the last-known-good active file, and removes an unpublished staged candidate.
- Added injected GNU89 host regressions for two transient interruptions followed by success, persistent staged-file EINTR (nine attempts, candidate removed, prior file retained), and persistent parent-directory EINTR (nine attempts, active file unchanged). Extended the Inadyn source-contract checker and path filter to pin the shared helper. No local compile or firmware result is claimed.

---

## 2026-11-28 — Align Ralink durability contract with shared fsync helper

- Exact-head HTTPD-security workflow on `e7b8267c4a1b17d81d731093442ff64faaf19635` compiled and ran the Ralink atomic-publication host regression successfully, then failed its follow-up source contract because it still required the removed raw `fsync(fd)` call.
- Updated the source contract to require `config_file_sync_fd(fd)`, the shared eight-interruption ceiling and the persistent-EINTR regression; it rejects the raw local fsync expression. Added `config_file_sync.h` to both workflow path filters so future policy edits re-run this regression.
- Checker/workflow correction only; no production Ralink code changed. Exact-head workflow must pass before treating the full job as green.

---

## 2026-11-28 — Complete handoff atomic-main execution contract

- AI handoff documentation run `46134978824` on `53dd5c67cc7df0073c447cd6725809c8660f3f9c` passed the checkpoint-heading/conditional-edit rule, then failed because the compact handoff's cross-session mandate omitted the exact atomic-main-commit and before-context-exhaustion requirements.
- Restored those two explicit requirements in the handoff's execution mandate. No source or workflow behavior changed.
- On the same SHA, PID 1 mdev regression `46134978792` and Process inventory `46134978799` PASS; NVRAM `46134978773` was queued and XY-C1 `46134978759` remained in progress. Their results must be matched to their own SHA.
- Result: THIS COMMIT.

---

## 2026-11-28 — Correct PID 1 mdev directory-sync fixture count and handoff contract

- Exact-head mdev run `46134787306` on `60a5a1bbc086ff06b87088d02e2d733ea7f6dbc8` compiled successfully after the stale local was removed, then failed the first publication assertion: it expected three injected fsync calls.
- The same injected adapter covers both barriers: two EINTRs plus successful staged-file fsync (three calls), followed by one parent-directory fsync (four total). Updated the regression and its workflow source contract to expect four. Persistent EINTR still fails before parent sync/rename after the shared initial-plus-eight retry budget.
- Exact-head AI documentation run `46134787245` also caught a missing mandated handoff sentence; restored the checkpoint's conditional-document-edit wording. No production source changed.
- No local host compile is available. Corrected exact-head mdev and AI docs Actions are required; L3/L4 are not claimed.
- Base: `e7b8267c4a1b17d81d731093442ff64faaf19635`; result: THIS COMMIT.

---

## 2026-11-28 — Bound Ralink wireless config fsync retries

- The Ralink wireless configuration publisher's `ralink_config_finish_stream()` still retried fsync indefinitely on EINTR. It now delegates to the shared `config_file_sync_fd()` helper and therefore has the repository's eight-interruption ceiling.
- Added persistent-EINTR coverage alongside the existing transient EINTR test. The regression verifies nine total attempts, candidate cleanup and preservation of the last-good wireless config.
- Added a path-filtered GNU89/ASan/UBSan workflow for the Ralink host regression; no local build or firmware result is claimed.

---

## 2026-11-28 — Fix PID 1 mdev bounded-fsync regression compile

- Exact-head PID 1 mdev security run `46134597261` on `ac811878adf63c4686af0acb59c0d11a26076133` failed its first GNU89/Werror host compile: `init_mdev_config_file.h` retained an unused `sync_result` local after its raw EINTR loop was replaced with `config_file_sync_fd()`.
- Removed the stale declaration. Tightened the injected fsync fixture so persistent-EINTR mode does not decrement the transient-interruption counter below zero; the bounded retry assertions remain unchanged.
- No local checkout/host compile is available in this session. Exact-head mdev, Process, AI docs, NVRAM and XY-C1 Actions are required after this commit; L3/L4 are not claimed.
- Base: `9ffa705ebf992cb803b17cee327e013313112350`; resulting commit: THIS COMMIT.

---

## 2026-11-28 — Bound L2TP config fsync retries

- The RP-L2TP atomic config publisher still used a private unbounded EINTR retry loop. Replaced it with the shared `config_file_sync_fd()` policy, retaining the existing single-EINTR recovery behavior.
- Extended the injected host regression to hold fsync at EINTR indefinitely. It now verifies the shared nine-attempt ceiling, prior config preservation and temporary-file cleanup.
- Added a dedicated path-filtered GNU89/ASan/UBSan GitHub Actions workflow because the repository had no workflow compiling and executing this L2TP configuration regression. No local host run or firmware build is claimed.

---

## 2026-11-28 — Bound PID 1 init-mdev config fsync retries

- The init-mdev publisher still carried a separate raw `while (sync_result < 0 && errno == EINTR)` retry loop despite already depending on the shared configuration durability helper. A persistent signal stream could therefore pin PID 1 while trying to publish `mdev.conf`.
- Routed staged-file synchronization through `config_file_sync_fd()`. Reused the shared injected fsync seam and added a persistent-EINTR regression: nine total attempts, no parent sync/rename, staged file removed, and the last published policy preserved. The existing two-transient-interruption success case remains.
- Updated the PID 1 security workflow contract to require the shared cap and bounded helper. No local host run or firmware build is claimed.

---

## 2026-11-28 — Repair workflow YAML indentation after static-contract edits

- The MiniUPnP and vsftpd workflows created on `dfdcd60c9b8478812a5b0ca583b7d15f0a752d18` were reported failed with zero jobs. Inspection showed newly inserted Python heredoc assertions had lost their ten-space YAML block indentation, making both workflow files syntactically invalid before any tests could start.
- Restored the heredoc indentation for all inserted MiniUPnP and vsftpd assertions. The source/test changes remain unchanged; both workflows must be re-triggered on this correction SHA to produce meaningful test results.

---

## 2026-11-28 — Bound vsftpd configuration fsync retries

- The FTP config publisher still had its own unbounded `do/while(errno == EINTR)` loop in both staged-file and parent-directory barriers, bypassing the shared `config_file_sync.h` eight-interruption ceiling.
- Kept the FTP wrapper API but delegated it to `config_file_sync_fd()`. Added an injected GNU89 regression: two transient EINTRs recover across the file and parent fsyncs (four total calls); persistent EINTR stops after the initial attempt plus eight retries, preserves the active config, and removes the staged candidate.
- Updated the vsftpd source contract to require the shared bounded helper and both fault-injection cases. No local compile or firmware build is claimed; the exact commit's vsftpd workflow must run.

---

## 2026-11-28 — Align MiniUPnP writer and Firefly fsync source contracts

- Exact SHA `0cd9b1e3ed2c46cc47c1648f8863f8cf1a7498bd` confirmed the MiniUPnP persistent-EINTR fixture now passed; its source-contract step still required the removed raw `fsync(fileno(stream))` expression. It now requires the shared bounded `config_file_sync_fd()` call before rename and rejects the raw fsync expression.
- The process-lifecycle contract also still expected Firefly's removed raw `fsync(fileno(output))` call. It now requires `config_file_sync_fd(fileno(output))` and rejects the raw expression.
- The MiniUPnP writer check now asserts both failure-checked writes structurally rather than comparing a brittle exact substring count. Checker-only changes; exact-head Actions remain required.

---

## 2026-11-28 — Keep MiniUPnP post-failure fixture expectations consistent

- Follow-up review of the new persistent-fsync-interruption fixture found that it correctly replaced the active file with a last-good sentinel before testing failure, but the remainder of the test still expected the earlier successful content. Added an explicit successful publish after resetting the injected fault, proving recovery from a failed candidate and restoring the original fixture before later validation.
- Test-only correction; production code unchanged.

---

## 2026-11-28 — Bound MiniUPnP and IGMP proxy config fsync retries

- The exact-SHA process-lifecycle regression on `9465ae8e81edd5ebcda3ee79cbdb412eb7f12b0c` passed MiniUPnP and IGMP proxy compilation/runtime publication tests, then failed the final durability contract because the shared checker still expected the obsolete `errno == EINTR` literal. Source inspection found a real remaining defect behind that stale assertion: both publishers still used independent unbounded file-fsync loops.
- Routed MiniUPnP and IGMP proxy staged-file synchronization through `config_file_sync_fd()`, which enforces the shared eight-interruption budget. The post-rename parent-directory fsync remains in place.
- Added injected transient-EINTR success coverage (two interruptions, staged-file sync and parent sync) and persistent-EINTR exhaustion coverage (nine total calls, no rename, temp cleanup and last-good config preservation) to both GNU89 host regressions. Updated the process-lifecycle source contract to require the bounded helper and reject raw fsync loops.
- No local host compile or XY-C1 target build is claimed; exact-head Actions will validate this slice.

---

## 2026-11-28 — Consolidate lifecycle sleeps on the shared bounded wait policy

- The previous lifecycle callback fix duplicated the bounded sleep-remainder policy even though `service_wait_policy.h` already provides the same production-tested 8-interruption limit and its own injectable regression. Consolidated the wrapper onto `service_wait_seconds()` to keep one policy implementation and one source of truth.
- Added a success-path regression that verifies an interrupted 3-second sleep resumes with the returned 2-second, then 1-second remainder. Existing shared tests continue to pin persistent EINTR exhaustion and error handling. The lifecycle adapter now has the exact `int` callback signature and returns the shared helper result.
- This is a small follow-up to `59ccb2df`; no change to the intended timeout or fail-closed escalation policy.

---

## 2026-11-28 — Fix service lifecycle sleep callback ABI and signal-storm handling

- A cross-layer review of `service_lifecycle_stop()` found the production adapter in `common_ex.c` was declared `void`, while `service_lifecycle_ops.sleep_seconds` requires an `int` result and the policy interprets nonzero as a sleep failure. Calling through that mismatched function pointer is undefined behavior; it also discarded the interrupted remainder returned by `sleep()`.
- Changed the adapter to return the policy result and resume the remaining sleep duration after EINTR. The shared retry helper has an eight-interruption ceiling; persistent signal storms return EINTR and block SIGKILL escalation/follow-up rather than hanging the stop transaction indefinitely.
- Added injectable regressions for interrupted-duration handling, bounded persistent EINTR and preserved hard errors. Extended the lifecycle integration contract to pin the production wrapper and tests. No local host build or firmware result is claimed.

---

## 2026-11-28 — Bound MiniDLNA and Firefly config fsync retries

- The process-lifecycle regression run on exact SHA `1e9b17efdc54970c76b80689f63c6804ae4ecc4b` passed 63 preceding lifecycle/config regressions, then exposed that MiniDLNA and Firefly still each had a local unbounded `do/while(errno == EINTR)` loop despite the shared bounded `config_file_sync.h` helper already being available.
- Routed both staged-file fsync operations through `config_file_sync_fd()`; parent-directory durability continues through the same helper after rename. Added injected tests for two transient EINTRs followed by success (including parent sync) and persistent EINTR exhaustion at exactly nine total attempts, verifying no staged temp remains and the prior active configuration is preserved.
- Updated the process-lifecycle source contract to require bounded helper usage and reject the duplicate raw fsync loops. No local host compile, XY-C1 firmware build or hardware result is claimed; exact-head Actions must validate the changes.

---

## 2026-11-28 — Repair stale watchdog and NVRAM source contracts

- Exact SHA `1e9b17efdc54970c76b80689f63c6804ae4ecc4b` exposed two false-negative CI contracts, not failures in the new compiled policy regressions. The watchdog-signal C/ASan test step passed, then its inline source check still demanded the removed shared-`/tmp` stale-file unlink policy. Removed that dead helper/test and the three obsolete assertions after the probe moved to an owner-only `mkdtemp` directory.
- The HTTPD security run's NVRAM C regression completed all injected flock, timeout, cross-process and crash-release cases successfully; its final inline contract still required a direct `flock(fd, LOCK_NB)` call after acquisition had been correctly routed through `nvram_transaction_lock_flock_with()`. Updated the contract to verify the bounded adapter and nonblocking acquisition arguments instead.
- These are checker/policy cleanup only; no NVRAM locking or production watchdog behavior changed in this slice. Re-run exact-head watchdog-signal and HTTPD-security Actions on this commit. No local build or hardware result is claimed.

---

## 2026-11-28 — Close root HTTPD watchdog /tmp symlink race

- The HTTPD watchdog removed a predictable `/tmp/httpd_check_result` and then invoked background `wget -O` on that path. Between unlink and wget's open, a local process with write access to shared `/tmp` could replace the pathname with a symlink; the root watchdog would then truncate/write the symlink target. The detached writer also outlived the probe routine's cleanup/decision window.
- Replaced the shared filename with an atomically created `mkdtemp()` directory under `/tmp` (owner-only 0700); the result path is confined inside it. Wget now runs synchronously with the existing one-second timeout, so the result cannot be raced by a detached child. All post-creation exits converge on cleanup; cleanup failure suppresses an HTTPD-dead decision.
- Added a GNU89/ASan/UBSan filesystem regression checking directory ownership/mode and regular result-file ownership/mode. Updated both watchdog and process-lifecycle source contracts to require private-directory creation, synchronous wget, and cleanup while rejecting the predictable shared path and background writer. Exact-head Actions and XY-C1 build are required; no local compile or hardware result is claimed.

---

## 2026-11-28 — Bound AliDDNS HMAC helper I/O under persistent signals

- Review of the already-integrated AccessKeySecret argv privacy helper found both its stdin read loop and stdout write loop retried EINTR indefinitely. A sustained signal storm could therefore pin the signing child and block DDNS request progress.
- Added a shared eight-retry budget for interrupted reads and writes. Progress resets the write retry budget; a zero-byte write now fails with EIO rather than returning a no-progress success or spinning. The API framing, EVP_MAC-SHA1 digest and base64 output are unchanged.
- Added an injected GNU89/ASan/UBSan regression for transient EINTR, fragmented input, partial output, persistent read/write EINTR exhaustion and zero-progress writes; wired it into the AliDDNS workflow and source contract. No local compile, XY-C1 firmware build or hardware result is claimed.

---

## 2026-11-28 — Align Samba durability audit with bounded shared fsync policy

- Samba transaction run #46130569334 on exact SHA `02dfbf9198cd7b5de10c4c902501249683ef26ef` passed the publication, service-failure and ACL/passdb checks, then failed at the final durability checker because it still required the obsolete unbounded `while (status != 0 && errno == EINTR)` loop.
- The shared `config_file_sync_fd()` helper now uses an eight-interruption ceiling. Updated the Samba source contract to verify the shared helper, injected fsync call, EINTR-only retry and cap instead of the removed loop. No Samba production code changed.
- Exact-head Samba and AI handoff Actions are required on this checker correction. No local checker run or firmware build is claimed.

---

## 2026-11-28 — Bound OpenVPN post-mode configuration fsync

- OpenVPN regression #46130569411 on exact parent SHA `02dfbf9198cd7b5de10c4c902501249683ef26ef` failed because the injected fsync counter observed five calls, not the expected six. Source review found the post-`fchmod` staged-file sync still used a raw unbounded EINTR loop; only the first and parent-directory syncs used the shared bounded helper.
- Routed the post-mode file sync through `config_file_sync_fd()` as well. Added a fault-injection case that lets the first file sync succeed, then supplies persistent EINTR at the second file sync; it must fail after nine attempts, remove the candidate, and preserve the previous configuration. The OpenVPN source contract now explicitly rejects the raw retry loop and requires the new regression.
- Exact-head OpenVPN regression and AI handoff Actions are required on this fix. No local host run, firmware build or hardware validation is claimed.

---

## 2026-11-28 — Align DHCPv6 fsync regression with shared directory barrier

- Exact-main DHCPv6 publication #46130700588 and WAN DHCP callback #46130700584 on `5a8a417a975547d6537e7e566ee97e36c2c910f1` failed at the same host-test assertion: it expected three injected fsync calls after configuring two transient EINTR returns.
- The DHCPv6 publisher now routes both staged-file and parent-directory sync through the shared injectable helper, so the adapter observes two interrupted attempts, one successful file sync and one directory sync. Corrected the expected total to four; production behavior is unchanged.
- Both exact-head workflows must pass on the correction SHA. No local host run or firmware build is claimed.

---

## 2026-11-28 — Share bounded fsync retries with DHCPv6 IA config publication

- The RC DHCPv6 config publisher included `config_file_sync.h` for parent-directory durability but still used its own `do/while (errno == EINTR)` loop for file fsync. A persistent signal storm could pin the DHCPv6 callback indefinitely.
- Removed the duplicate `DHCP6C_CONFIG_FILE_FSYNC` adapter and routed file synchronization through `config_file_sync_fd()`, applying the shared eight-retry cap.
- Extended the existing GNU89 DHCPv6 config publication test to inject persistent EINTR, require failure after nine attempts, remove the staged file, skip parent sync/rename, and preserve the last-good active configuration. Existing transient-EINTR and EIO cases remain.
- Exact-head DHCPv6/OpenVPN/AI Actions are required. No local compile, target firmware or hardware result is claimed.

---

## 2026-11-28 — Bound EINTR retries across OpenVPN config durability syncs

- Review found the shared `config_file_sync_fd()` retried EINTR without a ceiling, while `openvpn_config_file_publish()` bypassed it with two additional unbounded staged-file fsync loops. A persistent signal storm could pin a service process indefinitely during configuration publication.
- Applied one shared budget of eight EINTR retries after the initial attempt and routed both staged-file fsyncs through `config_file_sync_fd()`; the post-rename containing-directory sync already used the helper.
- Extended the existing OpenVPN config GNU89 regression with an injectable fsync adapter: three one-shot EINTR interruptions must recover across all three sync points, while persistent EINTR must terminate after nine attempts with EINTR. The source checker now pins the cap and shared-helper use.
- Exact-head OpenVPN and AI documentation Actions are required for this change. No local build, XY-C1 MIPS/TRX result or hardware validation is claimed.

---

## 2026-11-28 — Align NVRAM lock audit with the bounded flock adapter

- Exact NVRAM transaction-boundary run #46129272230 on `534075c653811287c9344da6ca667f561c82b7f5` passed all compile/runtime and policy steps 1–17, including the flock/unlock EINTR retry regression, then failed at repository-wide source audit step 18.
- The failure was a stale source-contract literal: it required `flock(fd, operation | LOCK_NB)`, while production correctly routes nonblocking acquisition through `nvram_transaction_lock_flock_with()` and keeps the real syscall in `nvram_transaction_lock_flock_real()` as `flock(fd, operation)`. This checker-only slice now asserts the bounded adapter, `LOCK_NB`, and isolated syscall. No firmware production source changed.
- Exact-head NVRAM and AI handoff Actions are required for this correction. No local full-repository audit, MIPS firmware build or hardware L4 result is claimed.

---

## 2026-11-28 — Align NVRAM static gate with shared EINTR counter and failure path

- Base main SHA: `8466913541cfcf4c1533081704b901ddce258d6d`; result: THIS COMMIT.
- The exact-head NVRAM run #46129176925 compiled the transaction-lock test and its runtime step (#17, lock inode/permissions/contention/crash release) completed successfully, including the new injectable flock retry cases. Its whole-repository source audit was still in progress. A review of the static gate found it still expected the older inline counter inside `nvram_transaction_lock_wait_interval()`, although the prior commit factored that logic into `nvram_transaction_lock_retry_interrupted()`; it also rejected the intentional EINTR-to-cleanup branch in acquisition. Updated the source contract to validate the shared helper and required failure cleanup rather than stale implementation details.
- Exact-head NVRAM audit, HTTPD security, Process inventory and AI handoff for this correction SHA remain required.

---

## 2026-11-28 — Bound EINTR retries for NVRAM flock and unlock operations

- Base main SHA: `5401539ce7c4af2f5b22267915c93b23db3c865d`; result: THIS COMMIT (resolve the resulting SHA from live `main` after publication).
- Review found two more potential unbounded interruption paths after the nanosleep fix: the nonblocking `flock(fd, LOCK_NB)` acquisition loop and `flock(fd, LOCK_UN)` release loop retried EINTR forever. A persistent signal storm could spin the caller and bypass bounded transaction timing/cleanup.
- Added a shared eight-interruption counter and injectable `nvram_transaction_lock_flock_with()` wrapper. Both acquisition and release now use this helper; after eight EINTR retries the operation fails with EINTR, and acquisition follows the existing descriptor cleanup path. Normal EWOULDBLOCK remains the timed contention path.
- Added fake-flock regressions for successful acquisition/unlock retry and bounded exhaustion. The NVRAM boundary checker statically enforces helper usage in both production call sites. Exact-SHA NVRAM/HTTPD/AI checks are required for this slice.

---

## 2026-11-28 — QMI retry waits and modem checker repair

- Extended the USB modem timing hardening from the QMI retry loop to all remaining one-second modem waits using the shared `modem_wait_full_delay(1U, sleep)` helper. The helper resumes the unslept remainder returned by `sleep(3)`; the policy regression validates successive remainder values and zero-delay behavior.
- The initial follow-up exposed stale `sleep(1)` expectations in `scripts/check_modem_module_reload.py`. Repaired the checker in `3d6f0dd96f18fa236bbbf522786ae59ce0e9a4d1` to assert helper use on unload/reload, no raw `sleep(1)` remains in `usb_modem.c`, all one-second waits use the helper, and the regression is wired.
- Exact SHA status snapshots at checkpoint: AI handoff #46127933674 0d757a438cd424c600be87724b65b473012310c6 completed/success; USB modem #46128756167 3d6f0dd96f18fa236bbbf522786ae59ce0e9a4d1 completed/success; earlier XY-C1 source builds #46127604081 d8c0568b30c07a5d18d4eca786d1ed53a9d5ef41 completed/success and #46127680458 7b56260076b8d05b521b3caf3f90afffc65642dc completed/success. The current main SHA is `1151dd8237ea16e079d9989d3ba4f760ad783894`; require a complete exact-source XY-C1 MIPS/TRX/artifact pass before claiming L3.
- No local full firmware build or physical router validation was performed. L4 remains NOT VERIFIED.
---
## 2026-11-28 — Relax process-inventory checker to test the actual retry contract

- Base main SHA: `656733c2e9a7299494e69312217016fbeba64b2e`; result: THIS COMMIT.
- Exact source SHA `3fb57acc623b3fd227c6d0aa3646733cbcb6dce2` Process inventory/lifecycle #46128583279 failed in `scripts/check_process_lifecycle_integration.py`: a new negative assertion banned the text `errno != EINTR` from the entire scanner body, although that text is used legitimately by unrelated error handling. The two positive assertions already pin the bounded open/read retry calls. Removed only the over-broad negative assertion; production code and injected retry tests were not implicated.
- Exact parent SHA `656733c2e9a7299494e69312217016fbeba64b2e` AI handoff #46128844188 PASS; NVRAM and HTTPD reruns were still in progress at last poll. Check all exact-head Actions again after this correction.

---

## 2026-11-28 — Fix bounded NVRAM test warning and static-gate syntax

- Base main SHA: `3d6f0dd96f18fa236bbbf522786ae59ce0e9a4d1`; result: THIS COMMIT.
- Exact parent SHA `ac141ff53f1d29e25f6279d762c064de123092dc` NVRAM audit #46128697949 failed its GNU89/Werror compile because the signal counter (`sig_atomic_t`) was compared with an unsigned interruption constant. The broad HTTPD/security workflow #46128697956 failed at the same shared NVRAM lock regression step; its failed-step report points to this same compile error, not an independent HTTPD defect.
- Cast the bounded minimum to `sig_atomic_t`. Also fixed the NVRAM checker condition grouping that source inspection caught after the compile gate short-circuited the run; the checker now tests cap, increment, remainder and EINTR behavior in one valid Python condition. Exact-head NVRAM and HTTPD reruns must pass before closing the incident.

---

## 2026-11-28 — Bound NVRAM lock-wait retries under persistent signals

- Base main SHA: `3fb57acc623b3fd227c6d0aa3646733cbcb6dce2`; result: THIS COMMIT (resolve the resulting SHA from live `main` on the next recovery).
- Follow-up review of the previous EINTR fix found that its `nanosleep()` remainder loop had no interruption ceiling. A persistent signal storm could keep retrying the same one-second wait forever, defeating the transaction lock's configured bound and pinning the caller.
- Added `NVRAM_TRANSACTION_LOCK_MAX_INTERRUPTS` (8). After eight EINTR retries, the wait returns EINTR through the existing lock-acquisition cleanup path; it never falsely reports EBUSY or proceeds without the lock. Kept the successful contention regression below the cap (200 ms periodic signals) and added a separate timer-injected test proving the wait aborts after the bounded interruption budget.
- The NVRAM static checker now pins the maximum, counter and bounded regression. The exact-head NVRAM transaction-boundary workflow must pass before closing this slice.

---

## 2026-11-28 — Retry interrupted /proc directory enumeration

- Base main SHA: `0a77a0064de6b1644b4a43bfac9509c46d7d92f3`; result: THIS COMMIT (resolve the resulting SHA from live `main` on the next recovery).
- Process identity file reads already retry EINTR through `process_io_retry_interrupted()`, but the surrounding `opendir("/proc")` and `readdir()` callbacks were each called once. A signal interrupting open/getdents could therefore make the entire process inventory return UNKNOWN and block an otherwise safe lifecycle operation.
- Added bounded EINTR retries around both injected `dir_open` and `dir_read` operations in `process_inventory_scan_with_ops()`, using the existing eight-interruption cap. Non-EINTR and exhausted-retry errors remain fail-closed as UNKNOWN with the original errno.
- Added injected regressions for successful open/read retries and exhaustion on both operations. Extended the process lifecycle integration checker to pin the scanner retry paths and all new cases. Exact-head Process inventory, Process lifecycle and XY-C1 Actions are required; L3/L4 are not claimed.

---

## 2026-11-28 — Fail closed when service-stop wait is interrupted

- Base main SHA: `1a33a6bcf9401810bc24858471e4867b37744bf6`; result: THIS COMMIT (resolve the resulting SHA from live `main` on the next recovery).
- The shared `service_lifecycle_wait_for_exit()` called a `void` sleep callback and always counted the iteration as one full elapsed second. Both production adapters, Inadyn and USB printer shutdown, used bare `sleep(seconds)` and discarded its unslept remainder. A signal during the graceful-stop wait could therefore shorten the timeout and make the policy treat a still-running process as timed out, proceed to SIGKILL, or report a misleading stop timeout.
- Changed the callback contract to return status. The wait loop now propagates sleep errors as `SERVICE_LIFECYCLE_FAILED` (preserving errno and supplying EIO when absent), which cannot authorize force-kill or follow-up. Both production adapters use the existing bounded `service_wait_seconds()` EINTR-remainder helper; after its interruption budget is exhausted, stop fails closed instead of escalating on a shortened wait.
- Added `test_sleep_failure_blocks_force_kill()` to inject EINTR and assert exactly one TERM, zero KILL, preserved errno and denied follow-up. Extended `scripts/check_process_lifecycle_integration.py` to pin the callback contract and both real adapter implementations. Exact-head Process lifecycle and XY-C1 Actions for this commit are required; no local full build or hardware result is claimed.
 
---

## 2026-11-28 — Preserve the NVRAM transaction-lock wait budget across EINTR

- Base main SHA: `0d757a438cd424c600be87724b65b473012310c6`; result: THIS COMMIT (resolve the resulting SHA from live `main` on the next recovery).
- `nvram_transaction_lock_acquire_path()` counted every contention retry as a full second, then discarded the unslept remainder from `sleep(1)`. A signal storm could therefore exhaust the configured 15-second wait budget and return `EBUSY` after only a fraction of the intended interval.
- Replaced the interrupted one-second pause with a `nanosleep()` remainder loop. Only `EINTR` retries the exact remaining interval; other sleep errors fail through the existing descriptor-cleanup path with the original errno. Extended the cross-process lock regression: a child receives periodic `SIGALRM` while contending, the parent releases the lock before the one-second attempt budget expires, and the child must still acquire after completing the remainder. The NVRAM checker now pins the helper, callsite and signal-injection regression.
- Local GNU89/Werror/ASan/UBSan host regression of the interrupted wait passed. The prior NVRAM `O_CLOEXEC` change `d8c0568b30c07a5d18d4eca786d1ed53a9d5ef41` was subsequently covered by NVRAM boundary audit #46127680492 on descendant source SHA `7b56260076b8d05b521b3caf3f90afffc65642dc` (PASS). AI handoff #46127933674 on `0d757a438cd424c600be87724b65b473012310c6` PASS. Exact-head NVRAM and XY-C1 Actions for THIS COMMIT remain required; L3/L4 are not claimed.

---

## 2026-11-28 — Harden config publication and close the cross-session checkpoint

- Implemented NFS and AliDDNS interrupted-stop handling, VPN sleep-remainder handling, and EINTR-safe durability boundaries for Samba, DNSMasq, L2TP/Ralink, USB modem and QMI writers across atomic source commits `3a910eec` through `2a257c21`. Fixed the follow-on checker and inline-helper issues in `cab010ea`, `89fc4e9a`, `58896aef`, `5652f1c8` and `385dd12d`; related exact-SHA host regression workflows passed where recorded in the handoff.
- Integrated the concurrent NVRAM `O_CLOEXEC` hardening commit `d8c0568b`. Added MSD Lite post-rename parent-directory durability sync plus EINTR/EIO fault-injection tests and a dedicated workflow in `7b56260076b8d05b521b3caf3f90afffc65642dc`. Exact MSD Lite #46127680489, storage hotplug #46127680481, NVRAM boundary #46127680492 and process fail-closed #46127680468 passed on that source SHA; process inventory #46127680472 passed. XY-C1 #46127680458 and #46127604081 were still in progress at the next poll.
- AI handoff #46127700728 on `d0a576acd59bfac3daab35885f6203a7fa66d993` failed only because the handoff lacked the mandatory Cross-session execution mandate; this checkpoint restores the exact required section and updates the first next action. The resulting exact-head AI documentation run must pass before closing the documentation failure.
- No local full firmware build or hardware run was performed. XY-C1 Build/TRX/artifact and physical-router L4 remain unverified until their exact-source evidence is available.
---
## 2026-11-28 — Prevent NVRAM device descriptors from escaping across exec

- Base main SHA: `385dd12da8e6f6f968d9633b82d0575ee34a2370`; result: THIS COMMIT (resolve the resulting SHA from live `main` during the next recovery).
- The five shared-adapter operations that issue raw `/dev/nvram` ioctls opened the character device with `O_RDWR` only. The transaction flock is close-on-exec, but a device fd inherited by a fork/exec child would remain open after the parent's transaction ends; that child could issue raw ioctls outside the shared-userland lock boundary.
- Added `nvram_device_open_flags()` with atomic `O_CLOEXEC` and applied it to checked-get, getall, set, commit and clear. Added a runtime regression that opens `/dev/null` through the production flags helper and verifies both read/write access and `FD_CLOEXEC`; the whole-repository NVRAM boundary checker now pins all five production call sites and the workflow runs the regression.
- Changed paths: `trunk/user/shared/src/nvram_linux.c`, new `nvram_open_policy.h`, new `tests/nvram_open_policy.c`, `scripts/check_nvram_transaction_boundary.py`, `.github/workflows/nvram-transaction-boundary.yml`, `dev_log.md`, `SESSION_HANDOFF.md`.
- Local host regression: `cc -std=gnu89 -Wall -Wextra -Werror` and execution PASS (`NVRAM device descriptor close-on-exec policy: PASS`). Full repository checker and exact-head NVRAM/Process/XY-C1/AI workflows have not yet run on this commit; parent SHA `385dd12da8e6f6f968d9633b82d0575ee34a2370` validation must not be substituted. Firmware L3 and hardware L4 are NOT VERIFIED for this source change.

- AI handoff #46127604042 on this commit initially failed its final documentation contract because the rewritten handoff omitted the repository-required explicit end-of-session accuracy checkpoint. Restored the mandatory checkpoint section verbatim in the follow-up; this is a documentation-contract correction, not a source/test failure. Exact-head AI handoff rerun pending.
---

## 2026-11-28 — Validate Shadowsocks wait-function definitions instead of nonexistent callsites

- Exact-head Shadowsocks #46124213141 on `852214915ed8275c529baa3c352d9b9d5d28b47d` passed shell syntax, startup status, DNSMasq cleanup, the full process lifecycle regression and adblock publication. The final checker still failed its bounded-wait block because it expected wait-function callsites inside the helper, while these are function definitions whose callers live elsewhere. Those old source strings were not a valid contract.
- Replaced the nonexistent callsite assertions with checks for actual `ss_wait_named_processes()` / `ss_wait_process_pattern()` definitions and their state-inventory calls, retaining the three `ss_wait_count` comparisons, five checked sleeps and separate startup/pattern bounded-wait counters.
- AI handoff #46124213071 PASS on `852214...`; corrected Shadowsocks and AI checks pending on this follow-up. XY-C1 #46123853244 for production source `03817...` was still building at step 38 with TRX/artifact pending. No local test or firmware build is claimed.

---
## 2026-11-28 — Align Shadowsocks bounded-wait checker with identity wait

- Shadowsocks lifecycle #46124074457 on `6afe2356d5d8f2e8adbca5c8598af7629a706cc9` passed shell syntax, startup status, DNSMasq cleanup, the expanded process lifecycle regression and adblock publication regression. The final static checker alone failed because it still expected exactly two `ss_wait_count`/`ss_wait_limit` comparison sites.
- The new `ss_wait_identity_absent()` adds a third independently bounded wait loop; the helper contains three comparison sites and five checked `sleep 1 || return 2` sites. Updated the checker count to three and clarified that the named, pattern and executable-identity waits are all bounded. No production source or runtime test change.
- AI docs #46124074432 and Process inventory #46124074460 PASS on `6afe...`. XY-C1 #46123853244 for production source `03817...` was still IN_PROGRESS at Build step 38; TRX and artifact pending. No local test/build result is claimed.

---
## 2026-11-28 — Add procfs fixtures for Shadowsocks DNS proxy teardown tests

- Shadowsocks lifecycle #46123853300 on source SHA `03817bd30d363097549b458c32bbbbfe2127b1dc` passed shell syntax, startup status, DNSMasq directive cleanup and the earlier process-lifecycle cases, then failed at the DNS proxy stop assertion. The mock launcher created only a process-state file; the new stop helper correctly scans procfs for exact executable identities and therefore had no verified `dnsproxy` process to signal.
- Added explicit fake procfs tasks for `/usr/bin/dnsproxy` (PID 5116) and `/usr/bin/dns2tcp` (PID 5117), and made the PID-scoped test signal shim remove the matching process-state marker after a simulated exit. Updated the static checker to require both backend fixtures. No production helper change in this test-repair slice.
- On `03817...`, Process inventory #46123853269 PASS and AI handoff #46123853317 PASS. XY-C1 #46123853244 remained IN_PROGRESS at Build step 38; TRX and artifact pending. Corrected test/checker reruns are pending; no local runtime or firmware result is claimed.

---
## 2026-11-28 — Bind Shadowsocks shutdown to executable and PID generation

- On parent `76f95c69dca1325985e9f38c7dc9bb0faf40ec7b`, review found the shared Shadowsocks lifecycle helper still used name-wide `killall` and regex-based `pkill -f` for stop/escalation. A same-comm foreign executable or a replacement process could therefore be included by a later process-name rematch.
- Replaced teardown with a complete procfs snapshot requiring the exact installed executable path (or exact `/usr/bin/ss-monitor` argv[1] under a known shell interpreter), valid task stat and start_time. Signals are PID-scoped and identity is revalidated immediately before TERM and KILL. KILL is restricted to the original PID/start_time snapshot; new instances and PID reuse cannot inherit the captured process's escalation authorization. Incomplete/deleted executable identity and same-comm foreign binaries remain UNKNOWN and fail closed.
- Expanded regressions for malformed/deleted executable metadata, same-comm impostors, monitor argv position, unknown-after-TERM, PID reuse with both foreign and same executable, normal TERM/KILL, and stuck processes. Removed obsolete killall/pkill test mocks and strengthened the static contract.
- No local shell regression or firmware build is claimed. Prior-head Storage #46123661683, AI handoff #46123661589 and XY-C1 #46123661636 were still in progress on `76f95...`. The prior L3 baseline is `2d038...` (#46122904412), but the new Shadowsocks source commit requires its own exact-SHA Actions, including XY-C1 Build/TRX/artifact.
 
---

## 2026-11-28 — Fix Python quoting in MTD rstats identity checker

- Exact `e87be480e8f1e714a53de04f7313a444c54205bf` Storage hotplug run #46123234076 reached the final static gate and failed because two new Python string literals embedded unescaped `"$RSTAT_KILL_LOG"` quotes. This was a checker syntax error, not an MTD runtime failure.
- MTD restore staging/rollback step 21 and erase/reset postcondition step 23 passed on `e87`; AI handoff #46123234008 passed. The current atomic patch rewrites the checker assertions as valid Python substrings while still requiring both exact PID 321 TERM and absence of PID 322 TERM.
- XY-C1 #46122904412 passed MIPS Build, TRX verification and artifact upload on product source `2d0388614f8a1f3f3a11ce2bf57f7f63a19d879c`; checker/docs-only `e87` preserves the same firmware source. This is L3, not physical-router L4.

---

## 2026-11-28 — Align MTD rstats source contract with executable-bound stop

- Storage hotplug #46122904484 on `2d0388614f8a1f3f3a11ce2bf57f7f63a19d879c` passed MTD restore staging/rollback (step 21), load, erase/reset and the related storage regressions; final step 42 failed because `check_storage_safe_remove.py` still asserted the removed comm-only `found=0`/`killall -q rstats` implementation.
- Updated the static contract to require a complete proc stat record, exact resolved `/sbin/rstats` executable identity, start_time matching, PID/start_time snapshot and revalidation before BusyBox PID-scoped TERM. Added checks that the restore fixture contains a same-comm/different-executable decoy and asserts only PID 321 receives TERM.
- Exact parent AI handoff #46122904358, process inventory #46122904384, GPIO #46122904392 and HTTPD security #46122904374 PASS. XY-C1 #46122904412 was still at MIPS Build step 38, with TRX/artifact pending; no L3 result is claimed for `2d038`.

---

## 2026-11-28 — Bind MTD restore/reset rstats stop to executable and PID identity

- Whole-userland process review found `mtd_storage.sh` treated any live `/proc/PID/stat` entry with `comm=rstats` as the storage daemon, then issued `killall -q rstats`. A same-comm process from another executable could therefore be stopped during settings restore/reset.
- Replaced name-wide signalling with an exact `/sbin/rstats` executable-path match, live task-state and proc start_time validation. The scan snapshots PID/start_time tuples and revalidates each tuple immediately before PID-scoped BusyBox TERM; the existing bounded postcondition scan remains fail-closed. Deleted or unreadable executable identity returns UNKNOWN rather than authorizing storage mutation.
- Expanded the restore-transaction regression with a same-comm/different-executable decoy and assertions that only PID 321 is signalled. Both host and BusyBox-ash workflow variants must pass; no local shell or hardware test is claimed.

---

## 2026-11-28 — Fix iperf3 awk parser reserved identifier

- Exact-head iperf3 #46121969720 on `831435216159d084e648b2701378e52a3c3d64ab` passed POSIX shell syntax but the 20-token proc stat fixture still classified the server as UNKNOWN (status 2). The `/proc/PID/stat` AWK parser assigned to `close`, the built-in AWK function name; this caused stat parsing to fail before executable/argv checks. Renamed the delimiter offset to `close_pos` and pinned this against regression.
- On the same parent Process lifecycle #46121969777 was still running; AI handoff #46121969724 and process inventory #46121969765 were also in progress at last observation. Fresh exact-head iperf and Process lifecycle Actions required.

---

## 2026-11-28 — Repair iperf3 proc stat fixture and align aggregate regression

- Exact-head iperf3 run #46121754059 on `7627a0840d64b18be01b93c6cecbd8597f7f64ec` again passed POSIX syntax but failed to recognize its server fixture. The stat line supplied only 19 fields from state through start_time; Linux `/proc/PID/stat` requires fields 3–22 (20 tokens), so the scanner correctly returned UNKNOWN. Added the missing numeric field and made the test failure report the actual status.
- Exact-head Process lifecycle #46121754078 failed at the optional-app aggregate checker because `check_optional_app_start_status.py` still required the retired shared-helper source string and duplicated an obsolete pidof/killall fixture. Replaced that check with source assertions for exact executable/argv/start_time verification, PID-scoped BusyBox signalling, no process-name kill, package path contracts, and execution of the dedicated iperf3 lifecycle regression.
- On `7627...`, AI docs #46121754074 PASS and process inventory #46121754112 PASS; Process lifecycle #46121754078 FAIL for the stale iperf contract; iperf3 #46121754059 FAIL at the under-sized stat fixture. XY-C1 #46121754091 was still compiling at Build step 38 when last inspected; new exact-head build is triggered by this source/test change.

---

## 2026-11-28 — Correct iperf3 proc-fixture argv encoding and use BusyBox PID signals

- Exact-head iperf3 run #46121497489 on `0b7dbdd00e11e93f422da8ba773d7f5856c88083` passed POSIX syntax but failed its first server fixture. The fixture wrote escaped backslash text rather than NUL-delimited `/proc/PID/cmdline`; the scanner correctly did not recognize it as a server. The follow-up writes actual NUL argv separators and corrects the proc stat fixture so the twenty-field tail places start_time at field 22.
- Removed the configurable kill-command environment hook to avoid executing a caller-supplied command as root. Production now uses the firmware BusyBox `kill` applet with PID-scoped TERM/KILL after fresh executable/argv/start_time validation. Also accept the Linux proc stat `P` state and use a POSIX awk whitespace class.
- The previous failure was in the test fixture, not evidence of a production signal attempt. A new exact-head iperf3 run must validate the corrected tests and process stop implementation.

---

## 2026-11-28 — Bind iperf3 stop to executable, server argv and PID start time

- Whole-userland Process inventory on `ce8e38ec928c8798b4785d58e6ddffee01cccbe6` found `trunk/user/iperf3/iperf3.sh` still used `killall -q iperf3` after only a comm-name process check. A same-name executable impostor or an iperf3 client could therefore be included in the signal set.
- Replaced comm-only stop eligibility with a fail-closed `/proc` scan requiring the exact resolved `$IPERF3_BIN` executable, server argv (`-s`), valid live stat data and captured start_time. Each TERM/KILL revalidates the same PID/start_time/executable/argv tuple immediately before PID-scoped signalling; absence requires two complete scans. No generic `killall` fallback remains.
- Expanded the shell regression for same-comm wrong executable, exact-binary client mode, incomplete argv metadata (UNKNOWN), PID-scoped TERM, bounded TERM→KILL escalation, and absence of name-wide killall. This host regression awaits exact-head Actions; Linux 4.4 has a residual validate-to-signal race without pidfd, and hardware L4 is not claimed.

---

## 2026-11-28 — Compare USB printer legacy-stop calls as exact statements

- Parent main `ef122034b4524ec1ce6768dd169ebee62a7da2ee`. The previous Process lifecycle run #46120573849 failed before its new USB printer executable-filter tests: source contract used substring checks for `kill_services(service_names, 3, 1)` and `kill_services(`, which also match the intended `usb_printer_kill_services()` wrapper. USB modem #46120573957, USB storage #46120573847 and AI docs #46120573872 had passed on ancestor 4b0fe.
- Changed both negative assertions to compare stripped source lines against the exact legacy `return kill_services(...);` statements. This still rejects a true generic-stopper fallback but does not reject the executable-bound helper. Exact-head Process lifecycle validation is now triggered by this workflow-file change.
- Concurrent source fix on `ef122034b4524ec1ce6768dd169ebee62a7da2ee` moved `chown_succeeded` from RETR scope to `handle_upload_common()` after XY-C1 #46119302438 exposed the MIPS compiler error. Current parent VSFTPD publication #46120731445 PASS, Process inventory #46120731490 PASS and AI docs #46120731486 PASS; XY-C1 #46120731426 and NVRAM #46120731399 pending at last poll.
- This commit changes the Process workflow checker and handoff/log only. No local build/test or hardware L4 result is claimed.

---

## 2026-11-28 — Fix VSFTPD upload chown local scope exposed by XY-C1 build

- XY-C1 run #46119302438 on ancestor `9edd01a13d7c861a1c2e246220f2096134679ca1` reached the real MIPS compiler and failed at `postlogin.c:1271`: `chown_succeeded` was undeclared in `handle_upload_common()`. The variable had accidentally been declared in the unrelated `handle_retr()` function, so host source contracts missed the production build error.
- Moved the local declaration into `handle_upload_common()` immediately beside its other locals. Extended the VSFTPD workflow's source contract to require exactly one declaration inside the upload function and reject it in RETR. The next exact-head VSFTPD and XY-C1 runs must pass; no local cross-compile is available.
- Parent main `4b0fe352c64fc2aebdbca526ce573a62d0c739b8`: AI docs #46120573872 PASS and USB modem #46120573957 PASS; Process lifecycle #46120573849 and USB storage #46120573847 were still running. XY-C1 #46120573889 was before its build step; no L3 result is claimed.

---

## 2026-11-28 — Repair executable-bound USB printer stop validation and rotate the handoff log

- Parent main `9178b6538b49b2bbd78f5039a61fb3397d3e1f3f`; exact-head run results for the USB printer executable-bound stop change:
  - Process inventory #46120124952 PASS; NVRAM boundary #46120124986 PASS.
  - Process lifecycle #46120124982 FAIL at its inline source contract: the new stop-body parser split on a literal backslash-n rather than the newline in the C source, so it did not isolate the three stop functions.
  - USB modem identity #46120124951 FAIL at compile: the new executable-policy regression includes `process_inventory.h`, but that test command omitted `-I trunk/user/shared/src`.
  - USB storage hotplug #46120124991 FAIL because `scripts/check_storage_safe_remove.py` still required the generic name-only `kill_services()` even after stop paths were deliberately moved to executable-bound `usb_printer_kill_services()`.
  - AI handoff #46120125048 FAIL because `dev_log.md` had reached exactly 120 complete records and the 100-oldest/20-newest rotation had not yet been performed.
  - XY-C1 Build/TRX/artifact #46120124993 was still IN_PROGRESS at last poll; no L3 PASS is claimed.
- Fixed the Process source checker to split on `chr(10) + "}"`, added the missing shared include directory to the USB modem regression compile, and updated the storage checker to require the executable-bound stop wrapper while rejecting a fallback to generic `kill_services()`.
- Rotated the oldest 100 complete dev-log records to `docs/dev-log/2026-11/part-040.md` in chronological order, recorded its exact byte count/Git blob SHA in the archive index, and retained the newest 20 records in the root log. This entry is the 21st root record.
- No production source was changed in this CI-repair commit. The new executable-bound stop runtime/source checks must pass on the next exact head before closing the USB task.

---

## 2026-11-28 — Bind USB printer stop/rollback to the installed executable

- Parent main `9edd01a13d7c861a1c2e246220f2096134679ca1`: VSFTPD #46119302446 PASS, Process inventory #46119302432 PASS, AI docs #46119302480 PASS and NVRAM boundary #46119302447 PASS. Full XY-C1 Build/TRX/artifact run #46119302438 was still IN_PROGRESS at last poll.
- Found an identity mismatch in USB printer lifecycle hardening: startup/readiness used `/proc/PID/exe` to ensure a same-name process was the installed `/usr/sbin/u2ec`, `lpd` or `p910nd`, but `stop_u2ec()`, `stop_lpd()`, `stop_p910nd()` and failed-start rollback still used generic `kill_services()` matching mutable process `comm`/argv. A spoofed daemon title could therefore be killed even though it had been rejected as the expected executable during readiness.
- Added a reusable filtered `process_snapshot` policy, with fail-closed UNKNOWN when any candidate executable cannot be verified. Added a USB-specific `service_lifecycle_ops` adapter so stop/rollback retain strict identity rechecks, TERM/wait/KILL escalation and stop-complete gating, but only signal exact installed printer executables.
- Added ASan/UBSan-backed tests for verified-executable-only filtering, spoof-only absence and unreadable-candidate failure. Extended Process lifecycle push/PR path filters and source contracts. Exact-head Process Actions pending; no local build or hardware result claimed.

---

## 2026-11-28 — Return anonymous-upload metadata errors without terminating VSFTPD

- Parent main: `dbf39a4f7adfdecf23dfdb05893b69207a3aabaa`. Exact VSFTPD #46119095990 and AI docs #46119095970 PASS; the production sysutil ftruncate runtime regression, FTP policy binaries and source contracts are green. NVRAM #46118906849 and Process #46118906883 on e0acf also PASS.
- Found a separate failure mode in the anonymous upload metadata branch: `vsf_sysutil_fchmod()` and `vsf_sysutil_fchown()` were fatal wrappers. A normal file permission/ownership error could abort the FTP session; in the two-process mode the privileged broker could terminate while servicing a single upload.
- Added `vsf_sysutil_fchown_ret()`. The privileged file-chown operation now returns status but keeps invalid descriptor, non-regular-file and untrusted-owner checks fatal. Both privilege models propagate ordinary failure; the privileged broker closes the received descriptor and replies `PRIV_SOCK_RESULT_BAD`. STOR now checks `vsf_sysutil_fchmod_ret()` and the chown result, reports `FTP_UPLOADFAIL` and follows the existing failed-upload cleanup path.
- Extended the production sysutil runtime test and VSFTPD source/path contracts to cover all updated implementations, headers and IPC. Exact-head VSFTPD, Process, NVRAM, AI and XY-C1 Actions are required; no local test/build or hardware L4 result is claimed.

---

## 2026-11-28 — Fix the VSFTPD const-cursor source-contract false positive

- VSFTPD #46118906872 on e0acf8bdbe29da9044df29981e4d8f33044412ad passed the GNU89/Werror runtime regression (including the production sysutil.c truncate test) and earlier FTP policy binaries, then failed the embedded Python contract. Root cause: checking that the substring "char *text;" was absent also matched the valid "const char *text;" declaration. This was a checker false positive, not a production compile/runtime failure.
- Replaced it with an exact stripped-line comparison against "char *text;", while retaining the positive check for "const char *text;". Fresh exact-head VSFTPD validation required.
- Main parent: 91101b663bae449687d860fc3e6a6529d6cff240. Detect-internet #46118970037 and AI docs #46118969986 on this SHA PASS. Parent Process #46118906883 and AI docs #46118906844 PASS; NVRAM #46118906849 was still in progress at last poll.

---

## 2026-11-28 — Align firmware-update failure contract with checked service stop

- Legacy detect-internet stability run #46114250052 failed at an embedded source assertion requiring the obsolete literal `if (stop_services(0) != 0)`. Current `flash_firmware()` captures `stop_upnp()` and `stop_services(0)` results, preserves the first errno, aborts with `return -1` if either checked stop fails, and only then invokes the custom shutdown script.
- Updated the workflow's source contract to assert the actual fail-closed ordering: UPnP stop, service stop, errno capture, combined abort/return, then custom shutdown. This is a CI contract correction only; no product source changed. Exact-head rerun is required to discover any later stale assertions.
- Parent main SHA: `e0acf8bdbe29da9044df29981e4d8f33044412ad`; its four Actions (VSFTPD, Process inventory, NVRAM, AI docs) were in progress at this write. No local build or hardware claim.

---

## 2026-11-28 — Make VSFTPD sysutil runtime regression warning-clean

- Exact VSFTPD run #46118727525 on `3625933d3f1300302a06d32ce8ad497ec852e4ca` passed after the truncate regression compared wrapper errno with the native `ftruncate(2)` errno (this runner returned `EINVAL` for a read-only-file descriptor; invalid fd separately verified `EBADF`). Process inventory #46118727520 and AI docs #46118727498 also passed; NVRAM #46118727500 was still running at last poll.
- The passing compile exposed a discarded-const warning in `vsf_sysutil_syslog()`: the local cursor only reads/advances through the caller's const string. Changed it to `const char *`, pinned this source contract, and enabled `-Werror` for the runtime compile linking the real `sysutil.c`.
- Exact-head VSFTPD and full XY-C1 validation are required. Current source includes prior watchdog, IPTV, IAPP and VSFTPD changes; older product L3 results do not validate this head. No local build or physical-router L4 claim.

---

## 2026-11-28 — Make the VSFTPD truncate regression errno-portable

- Exact VSFTPD run #46118624438 on `77b18fedaf56b275fb1efd1161592bb8f2e6c2ef` ran all prior host regressions successfully and compiled the new test against actual `sysutil.c`. The new read-only descriptor assertion expected `EBADF`, while this host filesystem returned `EINVAL`; the production wrapper correctly propagated that errno. This was a test assumption, not a production failure.
- Updated the regression to first record the native `ftruncate(2)` failure and require the wrapper to preserve that exact errno; an invalid fd (-1) independently verifies `EBADF`. The runtime test still proves the process continues after failure.
- Added source-contract checks for both scenarios. Exact-head VSFTPD/Process/NVRAM/XY-C1 reruns required; AI docs #46118624429 already PASS on the previous SHA.

---

## 2026-11-28 — Return VSFTPD truncate errors through upload cleanup

- Parent main SHA: `a82345cece5c2d68889c6bd10e62d0ed56577e7d`. In `sysutil.c`, `vsf_sysutil_ftruncate()` called `die()` on a normal filesystem error, killing the FTP session process before the upload handler could emit a protocol failure or execute its cleanup label.
- Changed the wrapper to return `ftruncate(2)` directly; the STOR path checks the result, sends `FTP_UPLOADFAIL`, and follows the existing cleanup path. A newly created partial file is therefore eligible for the configured failed-upload cleanup; an existing file is not marked as a newly created cleanup target.
- Added a runtime regression linked to the production `sysutil.c`: writable-fd truncation reaches size zero; truncating an O_RDONLY descriptor returns `EBADF` and the process continues. Added source-order assertions and push/PR path coverage.
- Exact-head VSFTPD, Process and XY-C1 Actions for the new commit are required. No local test, MIPS build or hardware result is claimed. Parent IAPP #46118354302 PASS; parent XY-C1 #46118354306 and NVRAM #46118354305 were still in progress.

---

## 2026-11-28 — Respect platform byte order and guard the IAPP OS default

- Exact IAPP run #46118049024 and AI docs #46118048938 on `88695c8189774f5108ff82d0b7c621000c2d76d6` both PASS. The host lifecycle suite passed daemon, dependency, FD, datagram, event-log and deadline checks; the separate target job verified the pinned MIPS musl archive and syntax-compiled `rtmpiapp.c` and `mt_iapp_util.c` with `IAPP_EVENT_LOG` under XY-C1 MIPS32r2/1004kc.
- The successful target compile exposed two warnings: the vendor header unconditionally redefined libc's reserved `__BYTE_ORDER`, and `rtmpiapp.h` redefined `IAPP_OS_LINUX` already supplied by the build flags. Wrapped both legacy defaults with `#ifndef` so platform/toolchain definitions remain authoritative; added both headers to push/PR workflow path filters and pinned these guards in the source contract.
- This source-header compatibility cleanup is not a full firmware Build/TRX result. Exact-head IAPP and full XY-C1 validation are required; no local build or physical-router L4 result is claimed.

---

## 2026-11-28 14:47 CST (UTC+8) — Fix IAPP target-toolchain checksum and source-contract parsing

- Parent main SHA: `eb6d784401b5a2953df0dea3c3549cf590e81398`. Exact IAPP run #46117932056 failed before compiler execution. In the host job, shell syntax, daemon lifecycle, Make header-dependency and select/FD regressions all passed, then the Python source contract did not find the target job because its raw-regex escaped the newline twice. In the target job, the 44.96 MiB archive downloaded successfully, but the checksum input used a literal backslash-n; `sha256sum` therefore attempted to open the archive path with an appended newline.
- Fixed the checksum input to emit one real line terminator, replaced the newline-sensitive job regex with an exact marker search, and added a source-contract assertion for the exact printf form. The MIPS compiler was NOT executed in the failed run; no compile result is claimed.
- Exact-main IAPP and AI handoff reruns for this correction commit are pending. No firmware production source changed; default XY-C1 L3 remains bound to product SHA `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` (#46116630792).

---

## 2026-11-28 14:42 CST (UTC+8) — Add XY-C1 target compilation for optional IAPP event logging

- Base main SHA: `1acc4300c58e9f5324cf7f4c4d75a9a3a191ff94`. Added an independent `iappd-event-log-xy-c1-target-compile` job to `.github/workflows/iappd-lifecycle.yml`; it downloads the published 47 MB `mipsel-linux-musl` release toolchain and verifies its pinned SHA-256 (`7c1dc5c5...`) before use.
- The target compiler must report `mipsel-linux-musl`; the check syntax-compiles both `rtmpiapp.c` and `mt_iapp_util.c` under GNU89, XY-C1 MIPS32r2/1004kc flags, `IAPP_EVENT_LOG` and `IAPP_OS_LINUX`, with the existing packed-member and conversion diagnostics treated as errors. The default firmware configuration remains unchanged.
- Extended the existing IAPP source/workflow contract to reject a host-compiler substitution or loss of the cross-compiler, pinned archive, target flags, macros, or either translation unit. Existing host lifecycle/event-log regressions remain intact.
- Exact-main Actions validation for THIS commit is PENDING; no local checkout or MIPS compile is claimed. This CI-only slice changes no firmware production source, so the last XY-C1 full Build/TRX/artifact L3 evidence remains product SHA `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` (#46116630792). Physical L4 remains NOT VERIFIED.

---

## 2026-11-28 — XY-C1 L3 validation completed for IPTV port source

- Exact XY-C1 run #46116630792 for product SHA `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` completed SUCCESS: Build XY-C1, TRX output verification and artifact upload. Artifact `XY-C1-firmware-58700dc70b54d931aabdb6701f0ec88c14b9d1b4`, ID `15252233106`, size 12,838,305 bytes. L3 is VERIFIED for this product SHA.
- Current main before this docs checkpoint: `a747994f8892363f94938655b6db68195d44c9dd`. Compare from 58700 to a747 confirms all later commits touched only workflow/test/handoff/log files; no firmware production path changed, so the L3 result covers current main's product source.
- The Process fail-closed regression #46117034795 on 6a1b and AI docs #46117034802 both PASS. This checkpoint's exact AI docs #46117399122 also PASS. No local build or physical-router L4 verification is claimed.
- Next unresolved roadmap task: optional Ralink `IAPP_EVENT_LOG` target compilation is explicitly unverified. Trace conditional source/build flags and add a bounded compile check; do not repeat the already-passing default IAPP safety tests.

---

## 2026-11-28 — Close IPTV port regression and LAN source-contract correction

- On product source `58700dc70b54d931aabdb6701f0ec88c14b9d1b4`, the Process fail-closed test initially failed because the new range-test fixture omitted `<stddef.h>`; the corrected test then exposed an embedded LAN-startup checker still requiring the old `if (xport < 1024)` literal. The production helper now correctly validates bounds before its boolean disabled gate.
- `6a1b28a152bed4341c912ae797945ee83f852d1c` updated the workflow contract for xupnpd and udpxy helper-call ordering; exact Process fail-closed #46117034795 PASS, including the IPTV port regression and LAN startup propagation checks. Exact AI docs #46117034802 PASS. The separate USB storage #46116630890 and NVRAM #46116630796 runs on 58700 also PASS.
- Compare 58700 → 6a1b confirms only workflow/test/handoff/log paths changed after the product commit. XY-C1 #46116630792 for product SHA 58700 remains IN PROGRESS at Build; TRX and artifact upload pending. No current-head L3 claim until all three steps pass. No local build and no physical-router L4 claim.

---

## 2026-11-28 — Align LAN startup source contract with IPTV range policy

- After `173fae1685922a5d94d90cb32f370e212eea137e` fixed the IPTV regression test's missing `<stddef.h>`, exact Process fail-closed run #46116838163 passed the IPTV compile/run step but failed later in the embedded LAN startup contract with `ValueError: substring not found`.
- Root cause was a stale checker expression requiring `if (xport < 1024)`. Production `start_xupnpd()` now correctly delegates bounds to `iptv_proxy_enable_port_status(xport, &xport_enabled)` and gates with `if (!xport_enabled)`. Updated the workflow contract to pin this order and added corresponding `start_udpxy()` policy/gate/process-preflight checks. No product source changed.
- Requires another exact-head Process fail-closed run. Product-source XY-C1 #46116630792 for SHA `58700...` remains IN PROGRESS at Build; NVRAM and USB storage results are tracked in the handoff.

---

## 2026-11-28 — Fix IPTV port regression test compile failure

- Exact Process fail-closed run #46116630850 on product SHA `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` reached the new IPTV port regression and failed compilation because the test referenced `NULL` without including `<stddef.h>`. This was a host-test compile failure, not a production selector-policy failure.
- Added the missing standard header to `iptv_proxy_enable_port_policy.c`. This follow-up is host-test-only; production source is unchanged. The new exact-head Process fail-closed run must pass before closing the failure.
- At 58700, Process inventory, HTTPD auth, DNSMasq lifecycle, WAN DHCP callback, WAN route, firewall startup and AI docs passed; Process fail-closed failed as above; USB storage, NVRAM boundary and XY-C1 were still in progress. L3 for the IPTV source change requires same-product-SHA Build/TRX/artifact success.

---

## 2026-11-28 — Exact validation checkpoint for watchdog scheduler snapshot

- `04f55a0098f9328c7bd563fccbbafd21013e03b3`: Watchdog, Process, NVRAM, AI docs and XY-C1 all PASS. XY-C1 #46115631593 completed Build, TRX verification and artifact upload; artifact ID `15251593325` (12,838,666 bytes). This L3 result is limited to this exact SHA.
- `f5bcffd73428bab5b4818e44465f3d25f8b7563a`: watchdog #46115995003 FAILED because the optimization read zero-initialized selectors before assigning the values from NVRAM. Corrected in `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5`, whose Watchdog #46116069843, Process #46116070166, NVRAM #46116069885 and AI docs #46116069828 all PASS. XY-C1 #46116069855 remains IN_PROGRESS at Build; TRX/artifact pending at this checkpoint.
- Current main before this documentation checkpoint: `6599ea954a69c70b4f811f66519e01b00671c634`, a test-only follow-up that explicitly covers both enabled schedules being skipped while `ntp_ready=0`. Exact Watchdog #46116463184, Process #46116463183, NVRAM #46116463221 and AI docs #46116463180 all PASS. No firmware production source changed since `9e154...`.
- No local build is claimed. L3 for the current production source awaits the exact `9e154...` XY-C1 Build/TRX/artifact result; L4 physical hardware remains NOT VERIFIED.

---

## 2026-11-28 15:36 CST (UTC+8) — Reject negative IPTV proxy port selectors

- Base main SHA: `6599ea954a69c70b4f811f66519e01b00671c634`. Exact-base Process #46116463183, watchdog lifecycle #46116463184 and AI handoff #46116463180 PASS; NVRAM audit #46116463221 remained in progress.
- `start_udpxy()` and `start_xupnpd()` checked `port < 1024` before checking the lower bound, so negative values (for example NVRAM `-1`) were silently treated as disabled; values above 65535 were rejected.
- Added shared `iptv_proxy_enable_port_status()`: negative and >65535 return ERANGE, 0..1023 remain disabled for compatibility, 1024..65535 enable the proxy. Both production launch paths invoke it before the disabled return. Added GNU89/Werror/ASan/UBSan boundary coverage and Process/XY-C1 workflow integration. Exact-head Build/TRX/artifact validation is required.

---

## 2026-11-28 — Explicitly cover pre-NTP schedule-read suppression

- Added a missing edge-case regression to the watchdog schedule snapshot: both actions may be enabled while `ntp_ready` is false; neither schedule string may be fetched, and failures of those unused keys must not invalidate the selectors snapshot. The source contract now pins this scenario and the three-read bound.
- Parent main SHA: `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5`. At that SHA the watchdog regression, AI handoff documentation, Process inventory and NVRAM transaction audit were PASS; XY-C1 Build/TRX/artifact remained in progress. This test-only follow-up requires exact-head watchdog and documentation checks; no production source changed.

---

## 2026-11-28 — Fix selector ordering in conditional schedule snapshot

- Exact-head watchdog run `46115995003` on `f5bcffd73428bab5b4818e44465f3d25f8b7563a` failed in the new cache-snapshot regression: schedule gating consulted the zero-initialized output selectors before assigning the values just read, so enabled schedule strings were skipped.
- Moved selector publication immediately after the three checked selector reads, before conditional schedule reads; retained whole-snapshot clearing on any failure. Updated the missing-key regression for the intentional three-read fast path and strengthened source ordering checks. Rerun the watchdog, NVRAM and XY-C1 workflows on this exact correction SHA.

---

## 2026-11-28 — Avoid unused schedule reads on watchdog ticks

- Follow-up to `04f55a0098f9328c7bd563fccbbafd21013e03b3`: preserve the same-transaction schedule snapshot while fetching schedule strings only when NTP is ready and the corresponding action is enabled.
- Added tests proving disabled schedule values are not fetched and a backend error for an inactive schedule cannot suppress the independent enabled action. Exact-head watchdog, NVRAM and XY-C1 checks remain required.

---

## 2026-11-28 14:10 CST (UTC+8) — Snapshot watchdog scheduled actions through checked NVRAM

- Base main SHA: `2e57ff8e98ba55aac56fc2a8d01d1b6ff8c6d19c`. `svc_timecheck()` read the reboot/SS enable flags, NTP-ready selector and two schedules through independent legacy `nvram_match()` / `nvram_safe_get()` calls. Those reads could not distinguish a backend failure from a missing/empty value and could mix values from different NVRAM cache generations while evaluating time-triggered actions.
- Added one bounded NVRAM transaction snapshot for all five keys. Each transient cache pointer is copied before the next read; absent keys preserve disabled/empty defaults, enable flags require the exact legacy token `1`, overlong schedule strings are rejected rather than truncated, and any read/close failure invalidates the entire snapshot and defers both actions. Added GNU89/Werror/ASan/UBSan policy regressions for cache reuse, missing keys, exact selectors, read failure and overlong data, plus call-path/source contracts.
- No local checkout/build is available. Exact-head watchdog lifecycle and XY-C1 Build/TRX/artifact Actions are required; physical-router L4 remains unverified.

---

