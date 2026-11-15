## 2026-11-15 — Trigger XY-C1 firmware validation for vsftpd source

- The XY-C1 workflow watched `trunk/user/vsftpd/src/asus_ext.h` but omitted the production implementation `asus_ext.c`, the upload caller `postlogin.c`, the descriptor-relative ACL helper `ftp_acl_path_policy.h` and its host regression. Therefore, a vsftpd implementation change could pass its host workflow without starting a product firmware build.
- Added those source/helper/test paths to the XY-C1 push filter. This exact-head commit triggers a fresh XY-C1 MIPS build, TRX verification and artifact upload for the failed-upload cleanup fix.
- Parent main: `079f35644072056f333cf04adffe63f8476d38bd`. Exact 079f vsftpd #43744355353, Process #43744355434, AI docs #43744355446, AliyunDrive #43744355411, AdGuardHome #43744355352 and AliDDNS #43744355620 PASS; NVRAM audit #43744355441 was still pending.

---

## 2026-11-15 — Check upload hardening flags at the owning helper

- Exact 9ec104c failed the next stale checker assertion: `O_NOFOLLOW` and `O_CLOEXEC` moved from `asus_ext.c` into `ftp_acl_path_policy.h` with the new descriptor-relative upload-open helper. Both source contracts now check the policy header where those flags are actually applied.
- Parent main: `9ec104c802859cab3d92fbbcd7f139e7044dbf0a`; compile/run passed on the preceding exact SHA, but the static checker still requires a clean exact-head rerun.

---

## 2026-11-15 — Align vsftpd upload implementation source assertion

- Exact 32edac failed the static contract at line 323 because the audit still required an inline `openat(parent_fd, p_leaf, flags, mode)` in `asus_ext.c`. The safe open logic is now factored into `ftp_acl_path_open_upload_entry()`; the contract was updated to require that helper call instead.
- The GNU89 helper/ACL host test compilation and execution had passed earlier in the same job; the remaining failure was this stale source-string assertion.
- Parent main: `32edacbd83385b5ab6c4600a5ce4dc49e8f6ad71`; exact-head vsftpd workflow must pass after the assertion correction.

---

## 2026-11-15 — Update vsftpd source contract for atomic upload creation

- Exact 8e7ac1a failed at the resolver test's obsolete cleanup-eligible expectation; 55bd710 corrected the expectation and the GNU89 compile/run stage then passed.
- Exact 55bd710 reached static verification but failed because the existing checker still expected direct `openat()` in `asus_open_upload_path()`. The implementation now delegates to the tested `ftp_acl_path_open_upload_entry()` helper, so the source-order contract was updated to assert the pinned-parent identity check precedes that helper call.
- Parent main: `55bd710eff29a822180ea996bb07c9f35c1a21ee`; exact-head vsftpd stability regression is the validation gate.

---

## 2026-11-15 — Tighten FTP upload cleanup regression expectations

- The follow-up host regression now explicitly asserts that path resolution alone never grants cleanup eligibility. The earlier draft still expected the old preflight flag and would have failed despite the new O_EXCL behavior.
- The STOU duplicate-open fixture now checks its independent cleanup flag, and C diagnostic strings use normal newline escapes.
- Parent main: `8e7ac1a9680d03e37c01d4701e80b2d0e19faf97`; exact-head vsftpd regression remains the validation gate.
 
---

## 2026-11-15 — Prevent failed FTP uploads from deleting existing files

- `ftp_acl_path_resolve_write_entry()` incorrectly marked an existing regular file cleanup-eligible. With `delete_failed_uploads=YES`, an interrupted non-exclusive STOR could unlink the pre-existing destination.
- Moved cleanup authorization to the successful `openat(O_CREAT|O_EXCL)` operation. Non-exclusive STOR now atomically creates a missing leaf or reopens an existing leaf without `O_CREAT`; only the session that won the exclusive create can roll back its partial file. Unique uploads retain exclusive-create semantics.
- Added host coverage for existing-file preservation, newly-created partial-file rollback, and duplicate STOU rejection, plus static source contracts.
- Parent main: `e3b14c7a0f3f5a3bb247489e4a8626044bc2739b`; exact-head vsftpd regression and XY-C1 build/TRX/artifact validation required.

---

## 2026-11-15 — Fail closed on incomplete storage mount-link inventory

- `create_mp_link()` accepted the first matching `/proc/mounts` entry, ignored malformed/truncated records and did not check `ferror()` or `fclose()`; a later inventory failure could still return a usable link to a storage-backed daemon.
- Reused the strict six-field mount-record parser and inventory completion policy already covered by the NFS export policy regression. The helper now validates the entire inventory, checks read/close status, and removes any link it created before returning failure.
- Added Process source-contract assertions for strict parsing, read/close checks and cleanup. Parent main: `03d251398ffe1dce36abb628f64502a0cfd4e0fb`; exact-head Process and XY-C1 verification required.

---
## 2026-11-15 — Align MiniDLNA status-returning API declaration

- The MiniDLNA publication slice changed `update_minidlna_conf()` from `void` to `int` so `run_dms()` can refuse to launch after a failed config write, but `rc.h` still declared the old `void` signature. That conflicting declaration can break the RC build.
- Updated the public prototype to `int` and extended the Process source-contract checker to assert declaration/definition agreement.
- Parent main: `c20bbfcfc8971a17dcea974f0e31416dafd8c1f4`. The MiniDLNA publisher GNU89 ASan/UBSan host regression passes locally; fresh exact-head Process and XY-C1 validation is required.

---
## 2026-11-15 — Make MiniDLNA config publication atomic and fail closed

- `update_minidlna_conf()` truncated `/etc/minidlna.conf` before writing, ignored buffered write/close errors and returned void; `run_dms()` attempted daemon launch regardless.
- Added a checked writer and same-directory temporary-file publisher with `mkstemp`, flush, `fsync`, close, atomic rename and failed-stage cleanup. `run_dms()` now refuses launch after config publication failure.
- Added a GNU89 ASan/UBSan host regression covering media-source selection, optional root-container/sort output, full replacement, 0600 mode and failed-rename cleanup; added static source contracts and Process/XY-C1 path filters.
- Parent main: `d3531270e9d3cf9523f2c0b8f0dc577b687acafb`. Exact-head Process and XY-C1 validation required; no DLNA runtime or hardware claim.

---
## 2026-11-15 — Make Firefly configuration publication fail closed

- `update_firefly_conf()` ignored source-open, write, close and rename failures; `run_itunes()` launched mt-daapd even when config publication failed.
- Added a unique same-directory temporary-file publisher checking read/write, flush, `fsync`, close and rename. Failed publication removes staging files and preserves the last good config; Firefly startup now checks directory creation and legacy migration and refuses launch after config failure.
- Added a host C regression for existing-config rewrite, default generation, last-good preservation on read/rename failures, 0600 mode and temporary-file cleanup. Process compiles/runs it under ASan/UBSan; Process and XY-C1 filters include the helper header.
- Parent main: `2bd4868069048e5470fcfd024678b0891575b12a`. Exact-head Process and XY-C1 validation required; no hardware runtime result claimed.

---
## 2026-11-15 — Complete mandatory handoff commit/exhaustion rules

- Rebuilt the optional RC startup source-contract checker in `1c9ef914b9d27396e90976d6eb0fdfa9ba961cf4`; exact Process #43741972950 passed.
- Exact AI docs #43741972993 then identified two further missing literal handoff requirements: `one atomic main commit` and `Before context exhaustion`. Restored both requirements without changing the bootstrap/protocol rules.
- Exact 8da XY-C1 #43741567717 remained in Build. The prior 28ccec XY-C1 #43741160797 completed SUCCESS through TRX verification and artifact upload; this does not verify 8da or later source.

---
## 2026-11-15 — Repair optional-service caller regression and handoff contract

- Exact 8da4 source commit captured/logged six optional startup results, but its Process workflow failed at the final Python integration checker because the inserted negative checks were spliced into an existing line, causing an unterminated string. The source and all earlier C/sanitizer steps passed.
- Rebuilt the integration checker from the known-good 28ccec version, preserving the existing IAPPD/telnetd/ttyd/storage contracts, then added a simple line-based standalone-call guard and feature/order assertions for the six optional services.
- Exact 8da4 AI handoff docs failed because the handoff omitted the required literal end-of-session checkpoint statement; restored it and recorded exact CI outcomes/next actions.
- Exact 8da4 XY-C1 #43741567717 was still at Build XY-C1; TRX/artifact pending. No firmware L3 result claimed.

---
## 2026-11-15 — Surface optional service launcher failures at the RC boot caller

- `start_services_once()` ignored six return values: `doSystem("/usr/sbin/skipd -d /etc/storage/db")`, `start_scutclient()`, `start_dnsforwarder()`, `start_vlmcsd()`, `start_iperf3()` and `start_mentohust()`. Their helpers return launch status, but the boot orchestrator discarded it, making enabled optional-service launch failures silent.
- Captured each return value and logged nonzero results. The original `APP_SKIPDBV2`, `APP_SCUT`, `APP_DNSFORWARDER`, `APP_VLMCSD`, `APP_IPERF3` and `APP_MENTOHUST` compile-time guards, relative call order and best-effort boot behavior are preserved.
- Added process-lifecycle source contracts for feature-gate containment, assignment, nonzero check, diagnostic, order and absence of standalone fire-and-forget calls. `services.c` is already present in Process and XY-C1 path filters.
- Parent main: `28ccec681975863be1dc59580166fdd3a7774655`; result: THIS COMMIT. Exact-head Process and XY-C1/TRX/artifact verification required; no hardware runtime result claimed.

---
## 2026-11-15 — Preserve storage-backed media daemon launch statuses

- `run_itunes()`, `run_torrent()` and `run_aria()` discarded the `eval()` status returned by Firefly, Transmission and Aria launch commands. Enabled services could fail their launch command without an RC-side diagnostic.
- Each function now captures the numeric status and logs a service-specific launch error on nonzero. Existing NVRAM enable guards, support checks, RUNNING idempotency and UNKNOWN process-inventory refusal are unchanged.
- Added source-contract regressions for result capture, failure-log ordering, enable guards and process-presence preflight. Added `services_stor.c` to the Process workflow trigger; XY-C1 already watches this source.
- Parent main: `4f2986624dc72c87f5453e47e822cef8a8e2d667`. Exact-head Process and XY-C1 validation required; no daemon-readiness or physical-storage claim is made.

---
## 2026-11-15 — Tighten the IAPPD caller fire-and-forget contract

- Exact c1987 Process lifecycle #43740697227 passed all C/sanitizer, optional-app, IAPPD host and prior RC integration steps, then failed at the final negative source assertion. The assertion searched for `system("...");` as a substring and incorrectly matched assigned expressions such as `int iappd_restart_status = system("...");`.
- Replaced the three false-positive checks with multiline-anchored regular expressions that match only a standalone `system()` statement. Positive checks still require all result assignments, nonzero diagnostics and call ordering. Product source is unchanged.
- Parent main: `c1987b2520611dd1adb1a54e655832c4c61df8b0`; result: THIS COMMIT. Exact process-lifecycle rerun required. AI docs #43740696916 PASS; NVRAM #43740696902 and XY-C1 #43740697118 pending at last poll.

---

## 2026-11-15 — Surface IAPPD wrapper results at the RC boot caller

- The IAPPD shell wrapper now propagates launcher, firewall, stop and process-postcondition failures, but `start_services_once()` discarded the results of `system("/usr/bin/iappd.sh restart")`, `system("modprobe xt_TPROXY")` and `system("/usr/bin/iappd.sh test")`. This hid both failed startup and failed post-start health verification at the top-level boot orchestrator.
- Capture and log each command's `system()` wait status independently while preserving existing order and best-effort boot policy. An optional IAPPD or xt_TPROXY module-load failure remains nonfatal to unrelated services but is no longer silent.
- Extend the process-lifecycle source contract to require the three assigned results, distinct nonzero diagnostics, exact call order and absence of raw unobserved calls. `services.c` is already included in the process and XY-C1 path filters.
- Parent main: `db6615af1928b3c7ea8cd564830d6ccc05d431c7`; result: THIS COMMIT. Exact Process lifecycle and XY-C1/TRX/artifact checks required; no target or physical IAPPD run claimed.

---

## 2026-11-15 — Report IPTV streaming proxy launch command failures

- `start_udpxy()` ignored the `eval()` result for both supported launch paths, `/usr/sbin/udpxy` and optional `/usr/bin/msd_lite`. `start_xupnpd()` ignored both its foreground `eval()` and the background-shell `system("/usr/bin/xupnpd &")` result. An enabled proxy could therefore fail at the launch command without a service-specific boot diagnostic.
- Capture and log the selected launch-command status while retaining the 1024+ port guard, xupnpd binary-support check, provider selection and existing foreground/background behavior. Added source-contract assertions for the two udpxy providers and both xupnpd modes.
- Limitation: successful `system("... &")` only reports the shell command result, not child readiness. Config-file creation/publication and running-process postconditions are not addressed by this slice.
- Parent main: `a3aa716b5205b0b48f0aa656234fa125e5251542`; result: THIS COMMIT. Exact-head Process lifecycle and XY-C1/TRX/artifact verification required; no firmware or physical IPTV test claimed.

---

## 2026-11-15 — Fix iperf3 ROMFS script installation path

- Exact XY-C1 push run #43737703922 on `776b47eea3728e44944e04aeef67e4e0fe750f9a` reached ROMFS and failed because the iperf3 Makefile used `$(THISDIR)`, which was empty under the submake and resolved the script source to `/iperf3.sh`. TRX verification and artifact upload were skipped; unrelated groff/SmartDNS download warnings were nonfatal.
- Switched the ROMFS script source to `$(CURDIR)/iperf3.sh` and added a semantic package-path assertion to `scripts/check_optional_app_start_status.py`, run by Process lifecycle regression. The XY-C1 workflow already watches the package Makefile.
- Parent main: `d2eebe4401dd9c7968334b71e748ad6c4312b20f`. Exact-head process-lifecycle and XY-C1 Build/TRX/artifact validation are required; no L3 or L4 result is claimed yet.

---

## 2026-11-15 — Report 802.1X authenticator launch failures

- The 5GHz and 2.4GHz Wi-Fi startup functions invoked `rt2860apd` and `rtinicapd` through `eval()` but discarded its return status. An enabled WPA/WPA2/RADIUS configuration could therefore fail to launch its authenticator without a service-specific boot diagnostic.
- Both paths now retain the original eval status and log radio-specific errors on failure. Existing radio/auth-mode guards, Wi-Fi ordering and `void` entry-point ABI are preserved.
- The process-lifecycle integration checker asserts result assignment, nonzero test and diagnostic ordering for both bands. `net_wifi.c` is already present in process/XY-C1 path filters.
- Parent main: `c0999a97f07435c11903dac04524a07844cc60a8`; result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 validations required.

---

## 2026-11-15 — Capture IAPPD regression command exit statuses correctly

- The first exact-head process-lifecycle run on `37204498a3ce81d6dd077fd551a66e15a29be344` passed every C/ASan/UBSan, process lifecycle and optional-app step, then failed at the first IAPPD test case. The test assigned `rc=$(command)`, which captures stdout rather than `$?`; since the wrapper correctly emits no stdout, the test compared an empty string to 127.
- Added `capture_status()` to execute a command, save its exit status, and print that numeric status for command substitution. All IAPPD test invocations now use the helper, including restricted-PATH start/stop, UNKNOWN inventory, launcher/firewall failures, stop/restart and RC `test` checks. Product source is unchanged in this correction.
- Parent main: `37204498a3ce81d6dd077fd551a66e15a29be344`; exact-head process-lifecycle rerun required. XY-C1 #43738581861 remained in progress at Build XY-C1; no TRX/artifact or physical-radio result claimed.

---

## 2026-11-15 — Verify and propagate Ralink IAPP daemon lifecycle status

- `ralinkiappd` forks its own long-running child in C `main()`, but the service wrapper also backgrounded that launcher, hiding fork/exec failure and allowing later `sysctl`/`iptables` statuses to mask it. The old stop path accepted missing `pidof` as absence, ignored `killall` results and never checked daemon disappearance. The existing RC boot path called `iappd.sh test`, but the shell case statement did not implement it.
- The replacement runs the self-forking parent in foreground, uses tri-state `pidof`, waits for bounded start/stop postconditions, skips duplicate starts, propagates launch/firewall/kill errors, gates restart on successful stop and implements RC's `test` action. Sysctl tuning stays best-effort for optional board interfaces.
- Added mocked host tests for missing/erroring process inventory, launcher failure, successful child startup/interface arguments, duplicate start, firewall failure, killall failure, false-success stop, successful stop, restart gating and `test`. Process-lifecycle and XY-C1 workflows now include the shipped script path; exact-head Actions required.
- Parent main: `607261a9f39e92b8167fbcc67cbfe8c84c0ab642`; result: THIS COMMIT. No MIPS firmware/TRX or physical-radio runtime result claimed.

---

## 2026-11-15 — Capture optional application launch failures

- The feature-gated RC wrappers for AdGuardHome, DDNS.to, Adbyby and AliDDNS discarded the return status of their `eval()` launch commands.
- Each wrapper now captures the original eval status and emits a named log message on nonzero status. Existing NVRAM feature guards, disabled-service no-op behavior and boot order are retained.
- Added source-contract coverage for the four wrappers: status declaration, enabled-service guard, assigned eval result and failure-log ordering. Existing process-lifecycle and XY-C1 workflows already watch `services.c` and the checker.
- Parent main: `479c05be3bef946f892f81a296d0ca3e709a698f`; backup branch remains untouched at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact-head process-lifecycle and XY-C1 validation are required.

---
## 2026-11-15 — Correct NFS stop regression fixture line delimiters

- The new NFS stop scenarios reached the etab preservation assertion after injecting daemon signal failures, but the Action exposed a fixture-construction error: three printf formats contained two backslashes before n, so printf wrote literal backslash-n characters instead of line breaks.
- Corrected the helper to use the same single-backslash newline escapes as the already-passing NFS fixtures. Production nfsd.sh remains unchanged.
- Exact SHA 56369810646910ccaf0f342b414dd09f83570843: NFS workflow #43737626899 FAIL at the first new stop fixture assertion; shell syntax PASS. Exact parent 925e6530bd27d1c7eea23984fbc867f3009648eb: AI docs #43737910530, lifecycle inventory #43737910499, infosvr #43737910533, AdGuardHome #43737910558, AliDDNS #43737910522 and AliyunDrive #43737910604 PASS; NVRAM #43737910496 IN_PROGRESS at last poll.
- Require a new exact-head NFS run before treating the stop failure/success postconditions as verified.

---

## 2026-11-15 — Capture optional application launch failures

- Four feature-gated RC wrappers for AdGuardHome, DDNS.to, Adbyby and AliDDNS discarded their `eval()` return values. The wrappers now capture the original status and log a service-specific error when nonzero; their existing NVRAM guards remain in force.
- The process-lifecycle source checker asserts each wrapper declares the status, retains an enabled-service guard, assigns the eval result and orders failure logging after the check.
- Parent main: `776b47eea3728e44944e04aeef67e4e0fe750f9a`; resulting exact SHA is recorded in this entry after publication. The existing process and XY-C1 workflows watch the changed C source/checker.

---

## 2026-11-15 — Record optional service launcher failures at RC boundary

- Four `void start_*()` wrappers for AdGuardHome, DDNS.to, Adbyby and AliDDNS called `eval()` without preserving its result. The scripts may emit their own details, but the RC boot entry point discarded nonzero exit status with no central launch diagnostic.
- Capture each `eval()` result and log the exact status when nonzero. The original NVRAM enabled/disabled condition remains the guard around the complete launch-and-log block; no service enable semantics or boot ordering change.
- Added source-contract regressions for each wrapper to assert declaration, retained feature guard, assigned `eval()`, failure check and log ordering. Existing process-lifecycle and XY-C1 path filters already watch `services.c` and the integration checker.
- Parent main: `56369810646910ccaf0f342b414dd09f83570843`; product/checker/docs changes are committed as one complete Git tree in THIS COMMIT. Exact-head Actions required.

---
## 2026-11-15 — Cover NFS server stop failure and completion postconditions

- The NFS stop path already requires both server daemons to be absent before unexporting, retains `etab` when teardown fails, and detaches rpc_pipefs/nfsd only in sequence. Its isolated host fixture previously tested startup/reload failures and the missing-`pidof` guard, but not normal stop transitions.
- Added four mocked stop cases: both graceful/forced killall attempts failing while daemons survive; rpc_pipefs unmount failure; exportfs withdrawal failure; and complete successful stop. Tests assert preserved `etab`/mount state on failures and removal of both daemon markers and both pseudo-mounts on full success.
- Production `nfsd.sh` is unchanged in this test-only slice; no target build or router runtime is claimed. Exact c736 NFS workflow #43737106642 passed the preceding missing-`pidof` tests; this new source SHA needs a fresh run.
- Parent main: `c736d4a8eccfe58f3106cdb63acc8b47963e6b98`; result: THIS COMMIT.

---

## 2026-11-15 — Fail closed when NFS process inventory tooling is unavailable

- NFS start/reload/stop infer rpcbind, nfsd and rpc.mountd state using `pidof`. If `pidof` is absent from PATH, command substitutions expand to empty strings; critically, `func_stop()` could treat that as confirmed daemon absence and continue to unexport NFS exports and unmount pseudo-filesystems.
- Added `func_require_pidof()` as the first operation in all three lifecycle actions. Missing tooling logs the failure and returns 127 before daemon, export or mount state can be mutated.
- Extended `nfsd_start_lifecycle.sh` with a restricted-PATH fixture containing only the mock logger. Start/reload/stop must each return 127; the stop case seeds both daemon markers and both pseudo-mount entries and verifies no killall/exportfs/umount and no state change. Added inline source/workflow contracts for the guard and cases.
- Parent main: `7e21f495aeaa60a85bdf839e9ee1bb514105c49a`; result: THIS COMMIT. Exact-head NFS workflow required; no target build or physical-router storage result claimed.

---
## 2026-11-15 — Scope telnetd/ttyd return-order checks to launch failure

- Exact-head Process inventory and lifecycle fail-closed run #43736811716 on `a71d73c0ff7776ae36dce19c5287ca3c141553cd` passed all GNU89/ASan/UBSan binaries, process/PID identity contracts, optional-app shell tests, and the normalized telnetd/ttyd diagnostic-string checks. It failed only at the return-order assertion: `start_status < 0` has an earlier legitimate `return start_status;` for UNKNOWN process inventory, so a whole-function `.index("return start_status;")` selected the wrong return.
- Bound the return search to the `if (start_status != 0)` launcher-failure block, for both telnetd and ttyd. The contract now proves launch invocation precedes launcher error handling, which precedes returning that launch status, without mistaking the separate UNKNOWN-inventory return.
- Exact a71 AI docs #43736811598 PASS; Process lifecycle inventory #43736811577 PASS; infosvr #43736811544 PASS; AdGuardHome #43736811570 PASS; AliyunDrive #43736811565 PASS; AliDDNS #43736811539 PASS. XY-C1 #43736811566 and NVRAM #43736811552 were IN_PROGRESS at last poll. Dropbear product wrapper changed on a71, so the exact SHA still needs Build, TRX verification and artifact upload before any L3 claim.
- Parent main: a71d73c0ff7776ae36dce19c5287ca3c141553cd; result: THIS COMMIT. Re-run exact-head process-lifecycle and AI handoff docs.

---
## 2026-11-15 — Fix Dropbear SSH error propagation and telnetd/ttyd source assertions

- Dropbear's alternate `sshd.sh` unconditionally exited 0 after start/stop/newkeys, masking host-key generation, daemon launch and killall errors; piping launch through `xargs` also translated daemon exit status.
- The wrapper now checks storage/key/argument/migration operations, logs failed keygen/start/stop and returns their original statuses. Dropbear is invoked directly after controlled whitespace splitting with pathname expansion disabled; default firmware paths and command names remain unchanged.
- Added shell syntax and ROMFS wiring assertions, mocked start status 53, stop status 54, keygen status 71, successful `-s` mode and invalid-action tests.
- The exact be92 process-lifecycle run #43736528747 passed all C regressions and optional-app tests but failed the two telnetd/ttyd source contracts because the expected strings omitted spaces after commas. This commit corrects those checker expectations and adds Dropbear coverage.
- Exact be92 AI docs #43736528725, infosvr #43736528664, AdGuardHome #43736528662, AliyunDrive #43736528653 and AliDDNS #43736528663 PASS; NVRAM #43736528727 was IN_PROGRESS. No exact-head result or XY-C1 firmware/TRX result is claimed yet.

---

## 2026-11-15 — Preserve string-literal spaces in telnetd/ttyd source checks

- Exact-head process-lifecycle #43736358298 on `ffaab4054d87afc23b410f1a560d0e6f561d7c43` passed every C regression and the optional-app shell suite, then failed at the new telnetd diagnostic source assertion. The checker used `"".join(split())`, which also removed spaces inside the C string literal, so the expected log text could never match.
- Corrected the whitespace compaction to `" ".join(split())`: it collapses indentation/newlines and repeated whitespace while retaining the single spaces in the diagnostic string. Applied to both telnetd and ttyd assertions.
- Exact ffa AI docs #43736358292 PASS, infosvr #43736358282 PASS, AdGuardHome #43736358220 PASS, AliDDNS #43736358252 PASS, AliyunDrive #43736358293 PASS. NVRAM #43736358258 was IN_PROGRESS. No product source changed; XY-C1 is not triggered by this checker-only correction.
- Parent main: ffaab4054d87afc23b410f1a560d0e6f561d7c43; result: THIS COMMIT. Exact-head process-lifecycle and AI docs reruns required.

---
## 2026-11-15 — Make telnetd/ttyd source contracts formatting-independent

- Exact-head process-lifecycle #43736203033 ran all GNU89/ASan/UBSan C regressions and the optional-app shell test successfully, then failed in the source checker because `start_telnetd()`'s long `logmessage()` argument was wrapped across C lines while the checker expected a one-line substring. `start_ttyd()` used the same formatting and had the same latent brittle assertion.
- Changed both source contracts to remove insignificant whitespace before matching the full call expression. No product code changed.
- Parent: `e882b68c6f9b6329793bd47a54cf37aa55642d14`; exact-head process-lifecycle, AI docs and XY-C1 checks must be re-evaluated after this checker update.

---
## 2026-11-15 — Fix OpenSSH mock logger test environment

- Exact-head process-lifecycle #43735937436 on `a769870fbc2b94d3a2ae46541b1911f17edd4b8a` passed all preceding GNU89/ASan/UBSan lifecycle, identity, PPPoE, WAN DHCP and buffered file-write checks, then failed in `check_sshd()` because the mock logger writes to `MOCK_LOG` but the SSH test environment provided only `MOCK_SSHD_LOG`.
- Added `MOCK_LOG` pointing to the temporary logger file; no product source changed. The existing `/bin/sh -n` and behavioral assertions will rerun on the new exact SHA.
- AI docs #43735937387 and process inventory artifact #43735937469 passed on the parent; XY-C1 #43735937368 and NVRAM #43735937325 remained IN_PROGRESS at last poll.
- Parent main: `a769870fbc2b94d3a2ae46541b1911f17edd4b8a`; result: THIS COMMIT. Fresh process-lifecycle and XY-C1 checks required.

---
## 2026-11-15 — Propagate OpenSSH daemon setup and lifecycle failures

- `sshd.sh` unconditionally exited 0 after its `case`, masking failures from host-key generation, config creation, sshd start, killall stop and SIGHUP reload. It now preserves the selected function status and logs failed start prerequisites/launch attempts.
- Host-key generation and config-file creation now abort startup on error instead of continuing with incomplete SSH material. Config/storage/key/daemon/utmp/pid paths have environment overrides solely to make host regression isolated; firmware defaults are unchanged.
- Added mocked SSH start-failure, successful-start and stop-failure cases. Process-lifecycle and XY-C1 path filters include `sshd.sh`.
- Parent main: `89c971cb95347ef7be7545989ea625345eaecf65`; result: THIS COMMIT. Exact-head Actions required.

---
## 2026-11-15 — Add fail-closed telnetd/ttyd start diagnostics

- `start_telnetd()` already preflighted daemon presence but did not log UNKNOWN inventory or launcher failure; it now records the original eval status, logs failures and preserves the existing disabled no-op and already-running no-op.
- `start_ttyd()` previously called its shell script without process inventory preflight or failure logging. It now refuses UNKNOWN inventory, avoids a duplicate start when ttyd is already RUNNING, and logs/returns the original `eval()` status. The `ttyd.sh` command itself already propagated `start-stop-daemon`/`killall` status; a host mock regression now pins that behavior.
- Extended the process-lifecycle source contract and optional-app shell test, and added `ttyd.sh` to process/XY-C1 path filters.
- Parent main: `af9676af871386982c4a50b12b7ade9a8d2b9ad5`; result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 checks required.

---
## 2026-11-15 — Propagate mentohust daemon launch and stop failures

- `start_mentohust()` already used the shared optional-daemon preflight, but `mentohust.sh` printed `[FAILED]` after a failed `start-stop-daemon` and still returned 0; its dispatcher also unconditionally exited 0 after start, stop or restart.
- The shell now logs and propagates the exact daemon-launch status, checks and propagates stop-signal failure, prevents restart from proceeding after an unsuccessful stop, and returns usage status 2 for invalid actions. Test-only environment overrides isolate the pidfile and config path without changing firmware defaults.
- Extended `check_optional_app_start_status.py` with mocked start success/failure and stop-failure cases. Process-lifecycle and XY-C1 workflows now watch the packaged mentohust script.
- Parent main: `ba90b6ae213b928748d700801740198dcfffd2a1`; result: THIS COMMIT. Exact-head Actions required.

---
## 2026-11-15 — Retire unused PID-only SIGKILL helper after RC callsite audit

- Audited all 36 top-level trunk/user/rc/src/*.c files for kill_process_pidfile(), kill_services_with_pidfiles(), kill_pidfile() and kill_pidfile_s() references. The only kill_process_pidfile() occurrence was its definition in common_ex.c; it had no RC C caller. The helper's force path used PID-only kill_pidfile() checks followed by kill_pidfile_s(..., SIGKILL), without binding the signal to process identity.
- Removed the dead helper and its rc.h declaration. The ten live kill_services_with_pidfiles() callsites in VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD route through process_pidfile_read_identity() and the shared service-lifecycle policy. That policy preflights targets, validates captured identities before TERM, rescans before KILL and revalidates before each signal.
- Existing service_lifecycle_policy.c regressions cover UNKNOWN preflight, PID-file identity replacement between TERM and KILL, and force-only PPPD PID-file targets. Extended the integration checker to enforce the legacy helper's absence in common_ex.c, rc.h and all RC C translation units, while retaining the escalation regression assertions. Process-lifecycle and XY-C1 workflows already watch common_ex.c and rc.h.
- Linux 4.4's final PID/start_time validation-to-kill window remains; this user-space cleanup does not eliminate it. No physical-router L4 result is claimed.
- Parent main: 0726f19c5a591b8f60745770b82fa592111f1525; resulting source/test/docs commit: THIS COMMIT. Exact-head process-lifecycle and XY-C1 reruns are required.

---
## 2026-11-15 — Add the missing iperf3 service lifecycle script

- `start_iperf3()` and `stop_iperf3()` invoked `/usr/bin/iperf3.sh`, but `trunk/user/iperf3/Makefile` installed only `/bin/iperf3`; the script did not exist in the tracked tree or firmware package.
- Added `iperf3.sh` and ROMFS installation. Start now launches `/bin/iperf3 -s` via `start-stop-daemon`, preserves the command failure status, waits for the daemon to appear, and skips duplicate starts. Stop uses a bounded wait and reports a failed signal if the process remains. Restart fails closed if stop does not complete.
- Extended the host mocked-command regression to cover launch status, ready state, duplicate start, bounded startup postcondition and stop-signal failure. Process-lifecycle and XY-C1 path filters cover the script and package Makefile.
- Parent main: `e950cee8d24ede2b355e18967a52a0b092b842fc`; resulting script/package/test/workflow/handoff checkpoint: THIS COMMIT.

---

## 2026-11-15 — Propagate SCUT and vlmcsd startup launch failures

- `scutclient.sh` printed `[FAILED]` when `start-stop-daemon` failed but unconditionally exited 0, hiding the failure from its RC wrapper. It now logs and propagates the original status, with a test-only PID-file path override for safe host testing.
- `vlmcsd.sh` used the final logger command's status as the start result, masking any daemon launch failure. It now logs the launch failure and returns the original `vlmcsd` exit status.
- Added a host-side regression with mocked `nvram`, `start-stop-daemon`, `vlmcsd` and `logger`; it verifies both success and failure status paths. Process-lifecycle executes the test; XY-C1 now watches both packaged app scripts.
- Parent main: `ba29a126870c7f57d4317edd2a0bd9c48508fa7d`; resulting app-script/test/workflow/handoff checkpoint: THIS COMMIT.

---

## 2026-11-15 — Restore required cross-session handoff checkpoint wording

- AI handoff documentation run #43733575878 on exact SHA `32e10009d28e4d37013b5defbdc526d7e9aa5063` failed because the refreshed `SESSION_HANDOFF.md` checkpoint did not contain the required literal `Before context exhaustion`. No source or test failure was reported by this documentation job.
- Restored the explicit end-of-session checkpoint phrase and synchronized the live roadmap with the 32e source SHA. Exact-head AI docs rerun is required.
- Parent main: `32e10009d28e4d37013b5defbdc526d7e9aa5063`; resulting SHA: THIS COMMIT.

---
## 2026-11-15 — Report enabled LLTD and rstats boot launcher failures

- `start_services_once()` invokes LLTD/rstats starts but discards their return values. Both wrappers returned direct `eval()` status without a diagnostic, so failed enabled launches were silent.
- `start_lltd()` and `start_rstats()` now capture the result, log nonzero launch status and return the original code. Their disabled-setting `return 1` sentinels remain unchanged and do not log an error.
- Added semantic source contracts for disabled guards, launch capture, nonzero diagnostics and return propagation. The existing process-lifecycle and XY-C1 workflows both path-filter `services.c`.
- Parent main: `022bc51507ca5f395c60a1f450cc461bf5403926`; resulting SHA: THIS COMMIT. Parent process-lifecycle #43732981693, HTTPD #43732981614 and AI docs #43732981749 PASS. XY-C1 #43732981606 remained IN_PROGRESS at Build; exact-head validation is required for this source.

---
## 2026-11-15 — Align LPD signal source with exact privileged-sink contracts

- The LPD PID identity production commit `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3` passed every C compile/runtime regression, including the shared PID-file and legacy LPD parser binaries, but process-lifecycle #43732716465 failed at the source contract expecting a contiguous call string.
- HTTPD security #43732716493 likewise failed only at the exact privileged-sink substring check; all preceding security checks passed. Reformatted the call to the single-line expression already required by both semantic contracts. No runtime behavior change.
- The same exact SHA had AI docs #43732716471, process inventory #43732716633 and infosvr #43732716469 PASS; XY-C1 #43732716622 was still IN_PROGRESS. Fresh exact-head process-lifecycle, HTTPD security and XY-C1 runs are required after this correction.
- The infosvr launch-status fix in parent `4e0e078cfac3a3db90d7f100e255e65b4bb03cbc` was already complete and exact-head workflow #43732716469 passed; Handoff/Roadmap now mark it done and resume the remaining startup caller audit.

---
## 2026-11-15 — Bind HTTPD LPD removal signal to verified process identity

- Process-lifecycle inventory identified the HTTPD lpr_remove handler as a raw numeric-PID signal path: it sent SIGUSR2 to /var/run/lpdparent.pid without verifying that the PID still belonged to LPD.
- LPD now writes shared strict PID/start_time records via process_pidfile_write(); startup validates them with process_pidfile_read_identity(). A PID-only fallback remains solely for older firmware compatibility and requires O_NOFOLLOW/O_NONBLOCK, a regular root-owned single-link file, no group/world write, strict decimal parsing and process-inventory validation.
- HTTPD now calls process_pidfile_signal(path, "lpd", SIGUSR2); ENOENT is harmless, while other identity/signal errors are logged and fail closed. Updated the process-lifecycle source contract and HTTPD privileged-script sink contract. Existing shared PID-file and legacy LPD parser regressions are already included in exact-head CI.
- Parent main: 4e0e078cfac3a3db90d7f100e255e65b4bb03cbc; resulting SHA: THIS COMMIT. Exact-head process-lifecycle, HTTPD security, AI docs and XY-C1 Actions required. No MIPS/TRX/L3 or physical-router L4 result is claimed.
- Next handoff item remains the boot-start status audit at start_infosvr(); watchdog startup status was completed by the parent commit. Linux 4.4 PID/start_time validation-to-kill TOCTOU is narrowed but not eliminated.

---
## 2026-11-15 — Log enabled infosvr launcher failures without treating disabled service as an error

- `start_infosvr()` correctly returns 1 when `adsc_enable` is disabled and its optional-daemon helper logs UNKNOWN process inventory. But once enabled and confirmed absent, it returned a failed `eval("/usr/sbin/infosvr", IFNAME_BR)` without a startup diagnostic.
- Capture the launcher result, log only nonzero launch failures, and return the original value unchanged. Disabled sentinel, already-running no-op and UNKNOWN-inventory diagnostic paths remain unchanged.
- Added ordered source assertions to `scripts/check_infosvr_lifecycle.py`; its existing workflow already filters `services.c` and this checker, and XY-C1 watches `services.c`.
- On parent SHA c75a2d8, watchdog #43732516198, infosvr #43732516176, Process inventory #43732516314, AliyunDrive #43732516228 and AdGuardHome #43732516280 passed; AI docs #43732516259 failed because the handoff lacked the exact required phrase `one atomic main commit`. The watchdog contract correction passed its exact-head run. The required phrase was restored in the follow-up handoff.
- Parent main: c75a2d8b02c6efe75ec0e9e001dcce173fcc8e16; result: THIS COMMIT.

---
## 2026-11-15 — Fix watchdog inline source-contract quoting

- Exact-head watchdog run #43732391911 on product-source SHA 5e16404907f16ac2321e69838b250d426c1aedd2 compiled and passed both GNU89 ASan/UBSan regression binaries and the NTP/service lifecycle checker. Its final inline Python source contract failed to parse because the added assertion double-escaped quotes inside a double-quoted Python literal.
- Changed the inline contract to use single-quoted Python literals around the exact C source text; no production source changed. The separate process-lifecycle integration workflow #43732391871 passed on the same SHA, as did infosvr #43732392035, AI handoff #43732391844, AdGuardHome #43732392074, DNSMasq #43732391865 and AliyunDrive #43732391870.
- XY-C1 #43732391824 for product-source SHA 5e164049 was still IN_PROGRESS; no L3 claim until Build, TRX verification and artifact upload pass. Current commit is workflow/docs-only; verify the new exact-head watchdog Action.
- Parent main: 5e16404907f16ac2321e69838b250d426c1aedd2; resulting checker correction: THIS COMMIT.

---
## 2026-11-15 — Report watchdog daemon launcher failures

- `start_watchdog()` already fails closed and logs when process inventory is UNKNOWN, and avoids duplicate launches when RUNNING. But after a confirmed ABSENT scan it returned `eval("/sbin/watchdog")` directly; a failed launcher had no diagnostic at the point of failure. This affects boot startup and other watchdog recovery callers.
- Capture the launcher status, log only when the launch itself fails, and return the original status unchanged. UNKNOWN/invalid-state diagnostics remain as before, and already-running remains a successful no-op. This avoids duplicate boot-level logging for the paths that already log internally.
- Added exact source-order/status assertions to the watchdog signal workflow and process lifecycle integration checker. Both workflows already filter `watchdog.c`; XY-C1 also watches this product source.
- Parent main: 7343fb3a798313ba8416df0741021efc6c536a84; resulting source/test-contract commit: THIS COMMIT. Exact-head watchdog, process lifecycle, AI docs and XY-C1 runs are required. The previous product-source SHA e902e806 still had XY-C1 #43731654808 IN_PROGRESS at the last check.
- `start_gpio_btn()` was inspected and already logs UNKNOWN inventory, launcher failure, UNKNOWN startup state and failure to remain running; no duplicate outer log was added. `start_infosvr()` still returns 1 when disabled and logs inventory UNKNOWN in its optional-daemon helper; enabled launcher failures are a separate remaining gap.

---
## 2026-11-15 — Repair networkmap source contract and restore mandatory handoff checkpoint

- Exact-head Networkmap run #43731654772 for product-source SHA e902e806f565e26d34406483ae2133c0b55acee2 failed after both GNU89 ASan/UBSan policy binaries passed: the added Python raw regex over-escaped its metacharacters, causing re.PatternError (unclosed group). Corrected it to use one backslash per regex escape; a local isolated regex/fixture check passes. The full workflow must rerun on this checker-fix SHA.
- Exact-head AI handoff documentation run #43731654776 for e902e806f565e26d34406483ae2133c0b55acee2 failed because the preceding handoff rewrite omitted the required Mandatory handoff checkpoint language and required section headings. Restored the mandatory verification/edit-conditional statement and the canonical Current repository state / Active task / Verification boundary / Exact first action sections. The existing protocol/bootstrap and all historical archive files remain unchanged.
- XY-C1 run #43731654808 for product-source SHA e902e806f565e26d34406483ae2133c0b55acee2 was still IN_PROGRESS at this checkpoint; no L3 claim until Build, TRX verification and artifact upload all pass. The follow-up only changes the checker and handoff records, not product source.
- Parent main: e902e806f565e26d34406483ae2133c0b55acee2; resulting checker/handoff correction commit: THIS COMMIT. Exact-head Networkmap and AI docs Actions must be rechecked.

---
## 2026-11-15 23:36 CST — Report networkmap boot startup failures

- start_networkmap(1) returns failure when the process inventory is UNKNOWN or the launcher fails, but start_services_once() discarded the status. The daemon could therefore fail to start without a boot-sequence diagnostic.
- Capture the result and log nonzero status while preserving best-effort boot continuation. Extend check_networkmap_signals.py with a semantic source assertion; the existing Networkmap push filters already include services.c and the checker, and XY-C1 already watches services.c.
- Parent main: 6832b792e4304114555ba054cd91f34ff4868fa2; resulting SHA: THIS COMMIT. On the parent exact-head Networkmap #43729759374 and AI handoff #43729759472 passed. XY-C1 #43729759471 remains IN_PROGRESS (job #132859395581, Cross-build XY-C1 firmware); current-source L3 is not verified until build, TRX verification and artifact upload all pass. 26 of 27 exact-head push Actions on the parent completed SUCCESS.
- P1 process-lifecycle sweep of all 36 trunk/user/rc/src/*.c files found no active in-tree kill_pidfile_s() callsites and no direct killall call outside the disabled #if 0 udhcpc block. The only active pids() call is the legacy is_dns_dhcpd_run() compatibility wrapper in services_ex.c, with no RC-source caller; kill_process_pidfile() also has no RC-source caller. Deliberate PID 1 / child-process signals and identity-bound service signals were left unchanged. This sweep is scoped to RC C sources, not every package in the repository.
- Other status-returning startup calls remain to be triaged. Inspect each return contract and disabled/no-op sentinel before adding diagnostics.

---
## 2026-11-15 23:20 CST — Log VPN server boot startup failure instead of discarding status

- `start_vpn_server()` is status-returning and the PPTP launcher result can fail, but `start_services_once()` discarded the return value. Boot therefore continued without a startup-level diagnostic for a failed VPN server launch (the public wrapper only logs lifecycle-lock acquisition/release failures).
- Capture the result and log a nonzero status while preserving best-effort boot continuation. Added a semantic source contract to `check_vpn_server_lifecycle.py`; the existing VPN workflow already path-filters `services.c` and the checker, so no workflow edit is required.
- Parent main: `e875f25d55a1e3523a1912368c5dd407b593946a`; resulting SHA: THIS COMMIT. Exact-head VPN/AI docs and XY-C1 validation required. Prior product source `30c5b48` had exact XY-C1 Build/TRX/artifact PASS (#43727223434; artifact #14166909490, 12,602,685 bytes), but this `services.c` change needs its own build result.
- DNSMasq direct callers, resolver writers and lock ordering were reviewed across RC source files; no additional same-process nested lifecycle lock release or reverse VPN/DNS edge was found. The Linux 4.4 PID/start_time check-to-kill race remains unmodified because revalidation alone cannot close it.

---

## 2026-11-15 23:06 CST — Repair ZeroTier restart contract after start-status propagation

- USB storage run #43727223550 on exact main SHA `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a` passed syntax, MTD/Optware regressions, storage gate, inventory, service-stop and the full caller graph. The final checker failed because it still required the obsolete `start_zerotier(); return 0;` pattern; production now correctly returns `start_zerotier()` so launcher failure is preserved.
- Updated only the stale contract to require the stop-failure gate and exactly one returned start call after it. No production source or regression fixture changed.
- Parent main: `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a`; resulting SHA: THIS COMMIT. Exact-head storage rerun required. XY-C1 #43727223434 was still IN_PROGRESS; AI docs #43727223494 and VPN server #43727223521 PASS on the parent. No local checkout/test, MIPS/TRX result for a later source SHA, or physical-router L4 result is claimed.

---

## 2026-11-15 22:58 CST — Log optional TTYD boot launcher failures

- `start_services_once()` previously discarded the returned status from optional TTYD startup during boot. It now logs a nonzero status while preserving the existing best-effort startup behavior for this optional service.
- Added the production source contract to the RC notification queue workflow. Parent `e6d78f8ce49ec0325674b1a3531635219502748c`; commit `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a`.
- Exact-head push checks on that source SHA: AI docs #43727223494, VPN server #43727223521, RC queue #43727223568 and ZeroTier #43727223519 PASS; USB storage #43727223550 failed only at its stale ZeroTier source assertion and is corrected in the next commit. XY-C1 #43727223434 was IN_PROGRESS at the checkpoint. No physical-router result claimed.

---

## 2026-11-15 22:54 CST — Propagate optional daemon start failures through restart APIs

- Converted optional TTYD, Shadowsocks, Shadowsocks tunnel, ZeroTier, SQM, AliyunDrive WebDAV, WireGuard and SmartDNS start wrappers from void to status-returning APIs. Disabled services remain successful no-ops; enabled starts return the launcher result; restart APIs now return the start result after the stop gate instead of silently reporting success.
- Updated RC notification and ZeroTier/AliyunDrive source contracts; follow-up test-only commits corrected the assertions to distinguish a returned start call from a discarded call. Main source commit `55947ba869421a8108d24898abb761ba58bb81a7` (parent `18c411ab0e5f7c9e8dfbbd96403a85317cc9a576`).
- The initial exact-head ZeroTier contract run #43726862606 failed on the stale expectation that restart returned zero; current-head ZeroTier #43727223519 and RC queue #43727223568 now PASS on `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a`. Other 55947 checks included DNSMasq #43726863022 and XY-C1 #43726862800 PASS. No L4 hardware claim.

---

## 2026-11-15 22:36 CST — Serialize IPv6 DNSMasq restart around the shared lifecycle lock

- `full_restart_ipv6()` now takes the DNSMasq lifecycle lock before stopping/restarting the service and uses the caller-held-lock helpers. For IPv6 enable, it releases the DNSMasq lock before `full_restart_wan()` (which owns VPN teardown), then reacquires it before scanning/starting DNSMasq, preventing the DNSMasq→VPN / VPN→DNSMasq lock cycle.
- Added DNSMasq and shared process/WAN source contracts. Product commit `8a90947493e7a9e3ef9e54ea2a75f99c6bbb88aa` (parent `779d409e004cc655813f1d8565ec45abfbad55a9`). XY-C1 #43725312979 and AI docs #43725313007 PASS on that SHA; its DNSMasq contract run #43725312974 was later corrected and exact-head DNSMasq #43726863022 passed on `55947ba869421a8108d24898abb761ba58bb81a7`, with unchanged `net6.c`.
- This is source/host regression evidence, not physical-router L4 verification.

---

## 2026-11-15 — Align VPN lifecycle checker with the under-lock WAN client stop helper

- VPN server lifecycle Action #43724605659 on production SHA `18c5f948efe89d9c8bcbff9caa14708686b2340d` failed in the Python source-contract stage, not in compilation or a C test. The legacy assertion required `stop_vpn_client()` inside WAN callbacks even after the production fix correctly replaced it with `stop_vpn_client_under_lifecycle_lock()`.
- Updated `scripts/check_vpn_server_lifecycle.py` to require the under-lock entry point in both WAN callbacks, forbid the reentrant public wrapper, prove the helper delegates to `stop_vpn_client_locked()` without acquiring/releasing the outer lock, and confirm the public declaration exists.
- Production-source exact-head results: DNSMasq #43724605898 PASS; WAN route #43724605834 PASS; AI docs #43724605883 PASS; OpenVPN secret #43724605815 PASS; WAN teardown #43724605876 PASS. XY-C1 #43724605847 was building; no current-source L3 claim.
- Parent main: `18c5f948efe89d9c8bcbff9caa14708686b2340d`; this workflow-only checker change requires a fresh VPN lifecycle/AI docs run. No physical-router result claimed.

---

## 2026-11-15 — Repair VPN/DNSMasq lifecycle lock nesting and inversion

- Confirmed nested same-process `fcntl(F_SETLK)` behavior in WAN shutdown: `stop_wan()` and `stop_wan_ppp()` acquired the VPN lifecycle lock around callbacks, but both callbacks called public `stop_vpn_client()`, which reacquired then released the same process-associated lock. Inner `F_UNLCK` could release the outer critical section prematurely.
- Added `stop_vpn_client_under_lifecycle_lock()` that delegates to the existing locked implementation without taking or releasing the lifecycle lock. Declared its caller-held-lock contract in `rc.h`; both WAN stop callbacks use it.
- Confirmed a cross-file cycle: LAN restart held DNSMasq then acquired VPN; static-IP WAN teardown held VPN and refreshed DNSMasq. Reordered LAN restart to stop VPN first, release it, then acquire DNSMasq. If DNS lock acquisition fails, VPN is restarted and the original lock errno returned.
- Added DNSMasq workflow source assertions for both WAN callbacks, helper non-reentrancy, process-associated F_SETLK/F_UNLCK semantics, VPN-before-DNS ordering and VPN recovery when DNS lock acquisition fails. Added modified VPN lifecycle lock header to XY-C1 triggers.
- Parent main: `0b9253aec5367c3c833ed54638b662f0784a1472`. DNSMasq #43723940207, AI docs #43723940125, process inventory #43723940114, WAN DHCP callback #43723940104, HTTPD #43723940105, AdGuardHome #43723940129 and Samba #43723940180 passed; NVRAM #43723940163 and XY-C1 #43723940080 remained in progress. Exact-head validation is required for this source change.
- No current-source target MIPS/TRX or physical-router result is claimed; L4 remains NOT VERIFIED.
- Changed paths: `vpn_client.c`, `rc.h`, `net_wan.c`, `net_lan.c`, `vpn_server_lifecycle_lock.h`, DNSMasq/XY-C1 workflows, SESSION_HANDOFF, REFACTOR_ROADMAP and dev_log.

---

## 2026-11-15 — Retry and report stage cleanup after publication failures

- The fdopen failure path is now retryable, but tracing the next failure edge found callers where dnsmasq_file_publish() attempted one best-effort stage unlink and callers returned without reporting a retained path if that unlink failed.
- Added a second checked discard attempt and path-specific logging when publication of the DNS server snapshot, the main DNSMasq configuration, an automatically learned LAN resolver file, or the deconfig resolver file fails. The original publication errno remains the returned status even if cleanup fails.
- Workflow source contracts now require each caller to preserve/report the stage on publication failure. Existing atomic-publication and injected unlink regressions cover the underlying helpers.
- Parent main: `5ea9fe1d0fb7527d6cf38c341bb9d27d7b192aac`. Exact-head DNSMasq and XY-C1 validation required. No local compile, current-source MIPS/TRX result or physical-router L4 result is claimed.

---

## 2026-11-15 — Preserve temporary-file names when fdopen cleanup fails

- Followed the retryable staging/backup cleanup path into its creator and found a remaining leak: after mkstemp succeeded, fdopen failure called raw unlink() and then cleared the caller's pathname unconditionally. A failed unlink therefore made the newly created stage untraceable.
- Moved the opener behind the injectable unlink primitive. fdopen failures now close the descriptor, attempt the same EINTR-retrying discard operation, and preserve the original fdopen errno while retaining the path if cleanup fails. A deterministic regression forces fdopen ENOMEM plus unlink EACCES, verifies the stage still exists and the original errno survives, then retries cleanup successfully. A separate test injects EINTR and verifies the unlink is retried.
- Added retry/log handling at the DNS server snapshot, DNSMasq main config, LAN DHCP resolver publication and LAN resolver deconfig open-failure callers. Successful callers keep their primary errno when best-effort cleanup fails.
- Parent main: `7e3e556f28cbd30296a8d91ad115e2556d43735f`. Exact-head DNSMasq and XY-C1 checks are required; the DNSMasq test is compiled with GNU89, warnings-as-errors and ASan/UBSan in the workflow. No local compile, current-source MIPS/TRX result or physical-router L4 result is claimed.

---

## 2026-11-15 — Make manual LAN resolver publication atomic and reject domain injection

- Audited remaining writers after the resolver-lock migration. `lan_up_manual()` was the outlier: while holding the shared `resolv` lock, it opened `/etc/resolv.conf` with `w+`, truncating the live file before writing. It ignored every write/close error, so ENOSPC or interruption could publish a partial resolver configuration.
- The same path interpolated `lan_dname` into a resolver directive without revalidating it. Manual AP startup logs a hostname-file validation failure but continues into this function, so relying on the earlier check did not protect the resolver file.
- Now reject null/invalid domains before route or file side effects, stage the resolver with a unique sibling file, and publish through the checked flush/close/atomic-rename helper under the existing cross-process lock. Open/publish failures preserve errno, retry/log leftover stage cleanup, release the resolver lock and skip time-sync/status updates.
- The existing DNSMasq file regression exercises private staging and atomic replacement; the workflow source contract now requires domain validation before lock acquisition, staged publication order, no direct truncating fopen, and explicit failure cleanup/logging.
- Parent main: `6484323f80d53f76c0b943a90a11b9c0f7de64a3`. Exact-head DNSMasq and XY-C1 checks are required. No local compile, current-source MIPS/TRX result or physical-router L4 result is claimed.

---

## 2026-11-15 — Repair DNSMasq source-contract variable in cleanup workflow

- The new rollback-snapshot cleanup source and its ASan/UBSan test compiled and passed all preceding steps on exact commit `d2784f00b3ffd5e1988a5dbf780f12061e4b93a4`.
- DNSMasq Action #43722144765 then failed at the final source-contract stage with Python `NameError: name 'wan' is not defined`. The workflow loads the file as `net_wan`; the new assertion referenced the wrong variable. Corrected the assertion to use `net_wan`.
- AI docs #43722144758 and WAN route #43722144902 passed; XY-C1 #43722144832 and NVRAM #43722144898 were still in progress at last poll. Rerun DNSMasq source contract and retain exact-SHA evidence.
- This is a workflow assertion repair, not a C-source/test failure. No current-source MIPS/TRX or physical-router result is claimed.

---

## 2026-11-15 — Make DNSMasq rollback-snapshot cleanup observable

- Found a related cleanup hole: `dnsmasq_file_backup_discard()` returned void. It preserved a path on unlink failure but exposed no status for callers, so leftover hard-link recovery snapshots could remain unnoticed.
- It now returns the retryable discard result and shares the injectable unlink implementation. Tests cover backup snapshot EACCES retention and successful retry.
- Three-output DHCP publication, router hostname/hosts publication and LAN hostname/hosts publication now retry residual backup cleanup while path buffers are alive, logging any persistent leftovers. Failure paths retain and log snapshot names for recovery rather than deleting them blindly.
- Parent main: `a3291427a15d8c7d55a38ebe20404e3a5af541ce`. Staging-discard cleanup exact DNSMasq #43721283505 and AI docs #43721283501 PASS; WAN route #43721283467, process inventory #43721283468 and NVRAM #43721283671 PASS. XY-C1 #43721283605 remained in progress; fresh exact-head validation required.
- No current-source target MIPS/TRX build or physical-router result is claimed; L4 remains NOT VERIFIED.
- Changed paths: `dnsmasq_file.h`, `tests/dnsmasq_file.c`, `services_ex.c`, `net_wan.c`, `net_lan.c`, DNSMasq workflow, SESSION_HANDOFF, REFACTOR_ROADMAP and dev_log.

---

## 2026-11-15 — Preserve DNSMasq staging paths when cleanup fails

- Found a resource-management defect: `dnsmasq_file_discard()` unconditionally zeroed its caller's pathname even when `unlink()` failed, leaving a private staging file orphaned with no retry handle and no cleanup error reported.
- Replaced it with a status-returning helper: successful unlink and ENOENT clear the path; other errors preserve the exact path and errno. Added an injectable unlink callback for deterministic regression coverage.
- Updated resolver, hostname/LAN, DNSMasq service-config and static-host cleanup paths to check and log leftover staging paths. The DNSMasq workflow now contracts the helper/test and path list; XY-C1 watches the DNSMasq file regression too.
- Parent main: `39a67af8d061f6f3eceeb2efbea7bd9a4c7cfe10`. Resolver fix exact-head DNSMasq #43719693642, WAN route #43719693710, AI docs #43719693546, process inventory #43719693606 and NVRAM #43719693596 passed; XY-C1 #43719693623 was IN_PROGRESS. Fresh exact-head Actions are required for this cleanup commit.
- No current-source target MIPS/TRX build or physical-router result is claimed; L4 remains NOT VERIFIED.
- Changed paths: `dnsmasq_file.h`, `tests/dnsmasq_file.c`, `services_ex.c`, `net_wan.c`, `net_lan.c`, DNSMasq/XY-C1 workflows, SESSION_HANDOFF, REFACTOR_ROADMAP and dev_log.

---

## 2026-11-15 21:35 CST — Propagate blocked DNSMasq upstream refresh errors

- Found a partial-success defect in `update_resolvconf()`: when `fill_dnsmasq_servers()` failed and the previous `DNS_SERVERS_FILE` was missing or unsafe, the code correctly suppressed DNSMasq cache notification but still returned success after publishing `/etc/resolv.conf`. WAN/PPP callers and PID 1 could acknowledge an incomplete refresh.
- Preserve the validation errno and propagate it after resolver publication and resolver-lock release, before any DNSMasq cache notification. A safe last-known-good upstream snapshot remains a valid fallback.
- Extended the GNU89 resolver policy regression for safe/unsafe/missing-errno states; added a production source-order contract and symmetric WAN push/PR filters; added the policy header to XY-C1 build triggers.
- Parent main: `fedd187503974f72651ea960b4d87f5a558c7c2f`. DNSMasq #43719236286, WAN route #43719236311, AI docs #43719236448 and process inventory #43719236422 passed on preceding product source `3fa8d08b968de653a7998175bdab897e51b91379`; XY-C1 #43719236268 remained in progress. Exact-head Actions for this resolver fix are required. No current-source MIPS build or physical-router result is claimed.
- Isolated GNU89 policy regression passed with ASan/UBSan; this is host-only evidence, not target MIPS validation.
- Changed paths: `net_wan.c`, `wan_resolver_refresh_policy.h`, resolver test, WAN and XY-C1 workflows, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-15 — Correct static-ARP transaction test literal after CI failure

- DNSMasq lifecycle #43718997744 on `d5660e3cdfa9804a23335656676c5f598c19a37e` initially failed when compiling the expanded static-host transaction regression: its mock compared `mac[0]` against a multi-character constant produced by over-escaping the backslash.
- Corrected the test-only literal to C `'\\0'` in `23cc4e9e509f13a4ea7636c067d6cfd869fde81b`. Exact-head DNSMasq #43719124911 then passed, including the GNU89 static-host regression; AI handoff #43719124917 and process inventory #43719124976 also passed.
- XY-C1 #43718997699 for the unchanged product source `d5660e3` was still in progress at checkpoint; L3 is unverified until build, TRX and artifact upload all pass. No local firmware or physical-router result is claimed.

---

## 2026-11-15 — Roll back prior static ARP additions after setup failure

- `fill_static_ethers()` previously stopped on a later failed `arp -s` without removing earlier static ARP entries installed during the same setup pass, leaving a partial kernel ARP table while DNSMasq refused to start.
- Added an injectable transaction policy that prevalidates all IP/MAC pairs, applies entries in order, and on failure deletes every previously successful add in reverse order. Rollback continues after an individual delete error and returns the rollback error (otherwise the original add error).
- The failed target is not deleted: in-tree BusyBox `arp_set()` returns success only after `xioctl(SIOCSARP)` succeeds; deleting a failed target could destroy a pre-existing neighbour mapping. The DNSMasq workflow asserts this applet contract.
- GNU89 regressions cover successful installation, reverse rollback, continued rollback after delete failure and rejection of an invalid set before side effects. The workflow checks that the production callback runs after all generated files have published.
- Parent main: `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`; exact-head DNSMasq/XY-C1 validation for this source change is pending. No local firmware build or physical-router L4 result is claimed.

---

## 2026-11-15 11:12 UTC — Reject multicast and all-zero static DHCP MACs

- Tightened dnsmasq_static_mac_is_valid(): 12 hex characters are necessary but insufficient. A DHCP hardware address must also be nonzero and unicast (I/G bit clear); multicast and 00:00:00:00:00:00 are now rejected.
- Updated GNU89 regression: accepts ordinary and locally administered unicast MACs; rejects zero, multicast, wrong-length and injection-character forms. DNSMasq source contract now requires the new negative test case.
- Baseline exact-head b39d7749: DNSMasq #43706419314, HTTPD #43706419305, AI docs #43706419277, Process inventory #43706419288 and NVRAM #43706419285 PASS. Earlier hostname-pair rollback test failed on bc91ec1 due to a stale test path; b39d7749 corrected the fixture and all these Actions passed.
- Parent main b39d77496885c65b1725dfc904788ec41f69f8fb. Changed: dnsmasq_static_host_policy.h, tests/dnsmasq_static_host_policy.c, DNSMasq source contract, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. No local compile/hardware result claimed.

---

## 2026-11-15 11:02 UTC — Record IPv6 stop failures and repair DNSMasq build triggers

- full_restart_ipv6() now captures and logs nonzero stop_dns_dhcpd() status before the existing fresh tri-state process inventory and locked start preflight.
- Added dnsmasq_file.h and tests/dnsmasq_file.c to DNSMasq push/PR path filters and explicit workflow path contract. Added dnsmasq_file.h and dnsmasq_domain_policy.h to XY-C1 push triggers for header-only product changes.
- Parent exact-head evidence 34ec88e1: DNSMasq #43705925858, HTTPD #43705926049, AI handoff #43705925896 and process inventory #43705926093 PASS. XY-C1 #43705925871 and NVRAM #43705925906 remain in progress. This commit's Actions remain pending.
- Parent main: 34ec88e12393d8c8645ad25674ccf44aacbe1256. Changed paths: net6.c; DNSMasq and XY-C1 workflows; SESSION_HANDOFF.md; REFACTOR_ROADMAP.md; dev_log.md. Frozen backup unchanged. No local or hardware test claimed.

---

## 2026-11-15 — Stage HTTPD restore uploads before replacing retained retries

- Fixed the restore upload transaction: `do_restore_st_post()` no longer unlinks or writes directly to `STORAGE_FIFO_FILENAME`. It authenticates the request, validates the multipart boundary and creates a unique mode-0600 stage in the same directory.
- The multipart parser now accepts a pre-opened stream and publish target. For storage restores, the old archive is replaced only after the complete payload/boundary validate, followed by checked fflush, fsync and fclose, then atomic rename. Parse/write/flush/fsync/close/rename failures remove only the new stage.
- Added a one-request readiness latch, reset at POST entry and consumed by the CGI output callback. A failed/malformed new upload cannot cause the callback to apply an older retained archive. A failed MTD restore keeps the new archive; a successful restore removes it.
- Extended `httpd_upload_file.c` with staging-open, flush, fsync, close and rename fault injection; asserts old archive bytes remain unchanged and the stage is removed. Successful replacement checks mode 0600 and confirms rename replaces a symlink entry rather than following its victim.
- Local helper test: `cc -std=gnu89 -Wall -Wextra -Werror -I <httpd-src> tests/httpd_upload_file.c` and execute — PASS in scratch workspace. Embedded HTTPD source contracts now check trusted auth, stage/publish ordering, stable retry retention, callback readiness and HTTPD input/output request order.
- Parent main SHA: `de14e2ec6c3365fd1e9be4cce5c13cb37ace60bc`. Parent HTTPD #43697297903 and current-baseline XY-C1 #43697297850 passed on source `8c95ef5ad8a2d4a2ac7755fbe02b7ac2211214b8`; both predate this HTTPD product change. Exact-head Actions for THIS COMMIT pending. No L4 hardware result claimed.
- Changed paths: `upload.c`, `httpd_upload_file.h`, `tests/httpd_upload_file.c`, `web_ex.c`, `httpd.h`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---
## 2026-11-15 — Keep the MTD save lock until blank-partition hash cleanup succeeds

- Auditing both zero-byte branches in `func_load()` found a persistence hazard: stale MD5 removal failures were ignored and the one-shot save lock could still be removed. A later identical save could match the stale checkpoint and skip writing an erased partition.
- Both branches now check `rm -f "$hsh"` before attempting `rm -f "$slk"`. If hash deletion fails, the old settings tree, checkpoint and lock remain intact, an error is logged, and load returns failure.
- Added failure injection for hash-unlink errors after (1) nonzero decompressor status with zero output and (2) nominally successful empty output. The fixture asserts the previous configuration/hash/lock survive; existing storage Actions run it under host shell and BusyBox 1.24.2 ash. The source contract requires checked hash-before-lock ordering and both injection cases.
- Parent USB storage #43696174488 on `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87` passed the complete storage workflow after earlier checker fixes. Parent AI docs #43696174518 passed. XY-C1 #43694958186 succeeded on source SHA `99eeb6fa237b529ec9b9615976961c7a60a02575`, but predates this `mtd_storage.sh` modification; a fresh build/TRX/upload is required.
- Changed paths: `mtd_storage.sh`, `mtd_storage_load_transaction.sh`, `check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent main: `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87`; result: THIS COMMIT. Exact-head USB storage, AI docs and XY-C1 runs required. No local test/build/hardware result is claimed; L4 remains NOT VERIFIED.

---

## 2026-11-15 17:05 CST — Parse MTD shell function contracts without the C brace parser

- Exact-head USB storage #43695808627 on `c2ee97b727e1fa16c86a93969129bf3c1c54f49f` passed syntax, MTD save retry, restore, host + BusyBox ash load/backup/erase-reset fixtures, GNU89 policy tests and the full storage caller graph. The final checker failed at `body(mtd_storage, "func_erase()")`: this helper parses C comments/strings and interpreted the apostrophe in a shell comment as a character quote, so it never recognized the closing function brace.
- Updated only the source checker to isolate shell `func_erase()` and `func_reset()` by their neighboring function declarations. C function checks continue to use the C-aware parser. No production behavior or regression fixture changed.
- Exact-head AI docs #43695808690, NVRAM #43695808608, infosvr #43695808631, process lifecycle #43695808654, AdGuardHome #43695808676, AliyunDrive #43695808670, AliDDNS #43695808625 succeeded on the same SHA. USB storage #43695808627 is the only listed failure; current-source XY-C1 result remains to be checked.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent: `c2ee97b727e1fa16c86a93969129bf3c1c54f49f`; resulting SHA: THIS COMMIT. Exact-head storage/docs rerun required. L3/L4 remain NOT VERIFIED.
- No local firmware build or router test is claimed.

---

## 2026-11-15 16:22 CST — Align empty-MTD hash assertion with combined cleanup

- Exact-head USB storage run #43694958246 on `99eeb6fa237b529ec9b9615976961c7a60a02575` passed production shell syntax, MTD save retry, transactional restore, host + BusyBox ash load/backup/erase-reset fixtures, GNU89 policy tests and the full caller graph. The final checker failed because it required a standalone `rm -f "$hsh"` in the successful empty-decompression branch, while source correctly issues `rm -f "$tmp" "$hsh"` and then checks one-shot lock removal.
- Changed the assertion to require the hash path token plus checked lock cleanup, matching the actual combined cleanup without weakening the invariant. No production source changed.
- AI handoff docs #43694958340, Process inventory/lifecycle #43694958225, infosvr #43694958242, AdGuardHome #43694958168, AliDDNS #43694958275, AliyunDrive #43694958193, process inventory #43694958234, NVRAM #43694958278 and HTTPD security #43694958209 succeeded on the same exact parent. XY-C1 #43694958186 was still in progress.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent: `99eeb6fa237b529ec9b9615976961c7a60a02575`; result: THIS COMMIT. Fresh exact-head storage/docs checks required; current-source L3/L4 are not yet verified.

---

## 2026-11-15 16:05 CST — Make MTD erase and reset fail closed

- `func_erase()` now establishes the persistent save lock before the irreversible MTD erase, checks the flash command and every hash/directory operation, and keeps the lock on failed or uncertain outcomes. A successful physical erase is not claimed to be reversible.
- `func_reset()` now locks persistence before deleting the stale hash and old settings directory, checks removal/recreation, and keeps the lock on partial failure. The dispatcher fills defaults before releasing the marker and does not start services after an incomplete preparation.
- Added deterministic regression coverage for flash failure, lock creation, hash removal, storage removal/recreation, default-fill and lock-release failures. USB storage Actions now run it under the host shell and BusyBox 1.24.2 ash. Corrected the exact-head MTD-load source contract from two to three checked lock-removal paths; the previous exact-head run failed only at that assertion after all preceding steps passed.
- Changed paths: `mtd_storage.sh`, `mtd_storage_erase_reset.sh`, `check_storage_safe_remove.py`, storage-hotplug workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `1eb57714cd9cbaaa71a4074615eaf472f053f359`; result: THIS COMMIT. Exact-head storage, AI docs and XY-C1 Actions required.
- No local checkout/test, current-source MIPS firmware build or hardware result is claimed. L4 remains NOT VERIFIED.


---

## 2026-11-15 15:44 CST — Repair MTD load source contract and restore full log window

- Exact-head USB storage #43691365991 on 361bf44c724e64f3826b876d43a876bb65fc6039 passed MTD syntax, save-retry, restore, host + BusyBox ash load/backup fixtures, GNU89 persistence/inventory/lock tests and the full caller graph. It failed only in scripts/check_storage_safe_remove.py line 502: the order assertion used index() and selected the first save-lock removal token in the blank-flash error path, rather than the final successful-load cleanup. The checker now explicitly requires both checked lock-removal paths and orders the final one with rindex().
- Exact-head AI handoff documentation run #43691366036 failed because the preceding commit unintentionally replaced dev_log.md with one entry, dropping 110 previously retained records. Restored all 110 records verbatim from parent 2feac289de28bca92469c304a0af13d90c1fb5e9 and retained the new 361bf44 entry; the root now contains 111 complete records, below the 120-record rotation threshold. No archive shard or index was changed.
- Changed paths: scripts/check_storage_safe_remove.py, dev_log.md, SESSION_HANDOFF.md and REFACTOR_ROADMAP.md. Parent SHA: 361bf44c724e64f3826b876d43a876bb65fc6039; resulting SHA: THIS COMMIT.
- New exact-head storage and AI docs Actions are pending. No firmware source changed; no local test, target build or hardware result is claimed.

---

## 2026-11-15 — Correct MTD load regression newline and exit-status handling

- Exact parent `2feac289de28bca92469c304a0af13d90c1fb5e9` USB storage #43690684317 passed shell syntax, MTD save retry and restore transaction, then exited nonzero in the combined MTD load host/BusyBox ash test. HTTPD security #43690684263, process lifecycle #43690684217, process fail-closed #43690684307, infosvr #43690684231, AI docs #43690684277, AdGuardHome #43690684211, AliyunDrive #43690684251 and AliDDNS #43690684338 passed; NVRAM #43690684273 and XY-C1 #43690684216 were still in progress.
- Two previous-checkpoint writes used a doubled backslash printf format, producing literal backslash-n rather than a newline. The fixture now writes a proper newline.
- The old cleanup trap could mask a nonzero assertion result because its final `rm -rf` succeeded. The EXIT trap now preserves the original status, and HUP/INT/TERM exit nonzero.
- Test/handoff/roadmap only; no product source behavior changed. Exact-head storage rerun required. No local test is claimed. Parent SHA: `2feac289de28bca92469c304a0af13d90c1fb5e9`.

---

## 2026-11-15 — Release the post-erase MTD save lock only after confirming blank flash

- Exact-head parent `546ca8e66a00aee08c53922a55368e61ceb507e2`: HTTPD security #43690054071 SUCCESS after the prior commit retained uploaded storage restore files on failure and fixed the escaped-newline source-boundary checks. USB storage #43690054009 FAILED only at the final trigger contract because `mtd_storage_load_transaction.sh` and `mtd_storage_backup_failure.sh` were not present in pull_request filters.
- `func_erase()` deliberately touches `$slk` after successful flash erase to prevent runtime from immediately repopulating the partition. On the next boot, blank flash causes bzcat to fail with a zero-byte output. The transactional loader previously returned early and left `$slk`, permanently disabling future saves. It now clears MD5 and the lock only when file size is exactly zero; nonempty partial/corrupt output preserves the previous checkpoint and lock.
- Added host/BusyBox ash cases for zero-byte failure, zero-byte success and nonempty decompressor failure; retained existing extraction/publication rollback cases. Added both MTD regression paths to storage-hotplug pull_request filters and removed a duplicated load path in the checker.
- Parent XY-C1 #43690054061 was IN_PROGRESS at last inspection. NVRAM #43690054024, HTTPD security #43690054071, FRP #43690054108, AdGuardHome #43690054047, AI docs #43690054062, Process inventory #43690054006, Process lifecycle #43690054021, infosvr #43690054113, AliDDNS #43690054011 and AliyunDrive #43690054109 SUCCESS. Fresh exact-head storage/XY-C1 results required.
- No local shell/build or physical hardware result claimed. Changed paths: `mtd_storage.sh`, `mtd_storage_load_transaction.sh`, `check_storage_safe_remove.py`, storage-hotplug workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `546ca8e66a00aee08c53922a55368e61ceb507e2`.

---

## 2026-11-15 — Preserve failed restore upload for retry; repair source contracts

- Parent exact-head USB storage #43689545333 and HTTPD security #43689545379 failed because the newly added Python boundary lookups searched for literal escaped backslash-n strings rather than actual source newlines. This commit uses `chr(10)` in the storage checker and HTTPD workflow contract.
- `do_restore_st_cgi()` unconditionally deleted `STORAGE_FIFO_FILENAME` even when `mtd_storage.sh restore` failed, destroying the exact uploaded archive needed for retry. The handler now unlinks it only when restore returns success; failure displays `UploadError.asp` and keeps the uploaded archive.
- Added HTTPD regression assertions requiring one unlink under `ret == 0` and none under the error branch. Exact-head Actions for this commit pending. No local compile/test, firmware build or router result claimed. Parent SHA `0baeb3d38480464bcfc7c3127847d234caca226a`.

---

## 2026-11-15 — Reject invalid MTD backup archives before HTTPD download

- Parent exact-head USB storage #43688959814 passed MTD syntax, save retry, restore, host/BusyBox ash load fixtures, persistence gates and embedded caller graph, then failed at the final path-list assertion because the load regression path was not listed in the checker’s push/pull filter contract. This commit adds it together with the new backup regression path.
- Old `func_backup()` could leave partial archive output on compressor failure, had no actual partition-capacity check and did not validate bzip2 or tar integrity. It also did not load the target MTD size. Backup now obtains the real size, checks compression return, size range, `bzcat` and `tar tf`, and cleans partial output on every failure.
- HTTPD `do_storage_file()` now streams only when backup returns success and its output exists; otherwise it sends HTTP 500 and removes the stale path.
- Added host and BusyBox ash coverage for partial compressor failure, oversized output, corrupt compressed stream, malformed tar and a valid archive; integrated source contracts. Parent SHA `c6ae3785862d7a0272f5ecc0c7e59199f3ee3593`; exact-head Actions for this commit pending. No local test/build/hardware result is claimed.

---

## 2026-11-15 — Correct the MTD-load regression source contract

- Exact-head storage-hotplug #43688365428 on `dbfcdaba996473dd135c93c574c88f02f6f3bcf0` passed script syntax, MTD save retry, restore staging/rollback, both POSIX-shell and BusyBox 1.24.2 ash load transaction fixtures, policy tests and the storage caller graph. The final standalone checker failed because it expected a literal `BUSYBOX_BIN=` assignment inside the fixture, although the fixture correctly reads `${BUSYBOX_BIN:-}` and the workflow supplies the environment variable.
- Corrected the source contract to verify the fixture's environment lookup and ash dispatch plus the workflow's explicit `BUSYBOX_BIN="$busybox_bin"` invocation. The regression cases and workflow execution are unchanged; this is not a test bypass.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent: `dbfcdaba996473dd135c93c574c88f02f6f3bcf0`; resulting SHA: THIS COMMIT. Fresh exact-head Actions pending. No local checkout/test, MIPS build or hardware result is claimed.
- Next: harden `func_backup()` and HTTPD `do_storage_file()` so failed/partial archives cannot be streamed as valid configuration downloads.

---
## 2026-11-15 — Make boot-time MTD load transactional

- Traced early boot: `dev_init.sh` creates `/etc/storage` and runs MTD load before defaults and the user start script. Old `func_load()` ignored decompression/extraction status, accepted partial output, advanced MD5 before extraction and unpacked over live data.
- New load validates decompression and nonzero numeric output, extracts into a private sibling, checks tar/chmod/hash staging, then moves the old directory to rollback and publishes the verified stage. A failed publish restores old settings; the old hash and lock marker remain on all pre-publication failures.
- Added host and BusyBox ash fault injection for decompression failure after output, partial extraction, failed publish/rollback and successful replacement. Integrated push/PR filters and production source contracts.
- Parent `b799ac5bb7588e015913a5bbc70e68efb5adec77`: USB storage #43687393291, HTTPD security #43687393326, Process inventory/lifecycle #43687393295, NVRAM #43687393286, AI docs #43687393272, infosvr #43687393356, AdGuardHome #43687393288, AliDDNS #43687393306 and AliyunDrive #43687393297 SUCCESS; XY-C1 #43687393279 IN_PROGRESS. New commit Actions pending. No local shell, MIPS build or hardware result is claimed.

---

## 2026-11-15 — Reconcile transactional restore contracts across storage and HTTPD Actions

- Exact-head USB storage #43687076965 on `0ed88bde4fe182f057291429e7a94af5149da772` passed shell syntax, MTD save retry, the new restore copy-failure/install-rename rollback regression, storage persistence gate, disk inventory, daemon stop and storage caller graph. Its final checker failed with `NameError: pr_filters is not defined`; it had already split the workflow into `push_filters` and `pull_filters`. The path contract now consistently checks `pull_filters`.
- HTTPD security #43687076956 passed 33 preceding policy checks, including NVRAM and storage safe-eject, then failed at its inline source contract because it still expected the old destructive `cp -rf "$tmp_storage"/. "$dir_storage"/` copy. Replaced that expectation with staging, same-filesystem rename, rollback and recovery checks matching the production restore implementation.
- Process lifecycle #43687076988, NVRAM transaction #43687076931, Process lifecycle inventory #43687076969 and AI handoff docs #43687076907 all passed exact-head. XY-C1 #43687076955 was still in progress at last poll.
- No production source semantics were changed here. Changed paths: `.github/workflows/httpd-security.yml`, `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent SHA: `0ed88bde4fe182f057291429e7a94af5149da772`.

---

## 2026-11-15 — Make MTD storage restore replacement transactional; repair exact-head contracts

- Exact-head USB storage #43686251138 on `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc` passed the MTD script syntax check and the injected flash-failure/identical-retry/unchanged-skip shell test, then passed storage persistence gates, inventory, daemon-stop and full caller-graph checks. The final standalone source checker crashed at line 470 because it searched for the two literal characters `\\n` before `func_backup()` rather than an actual newline. The checker now uses the real function separator.
- Process inventory/lifecycle #43686251333 failed at shutdown ordering line 286 because it expected the obsolete `write_storage_to_mtd();` token after the caller became a status-returning expression. The check now matches the call expression while keeping the storage-removal gate ordering. HTTPD security #43686251157 and Process lifecycle inventory #43686251172 succeeded on the same SHA.
- Auditing `func_restore()` found a data-loss branch: the old implementation removed the live storage directory before unchecked `mkdir`/copy. A failure could erase the old settings and still report success. Restore now prepares a complete tree in a same-filesystem stage, renames the existing tree to a rollback path, atomically installs the staged tree, and restores the original directory when installation fails.
- Added copy-failure and install-rename-failure injection, with checks for old-config preservation, uploaded archive retention, successful replacement and service restart; wired the test into storage-hotplug Actions and expanded source contracts.
- Changed paths: `mtd_storage.sh`, `mtd_storage_restore_transaction.sh`, `check_process_lifecycle_integration.py`, `check_storage_safe_remove.py`, storage-hotplug workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc`. New exact-head checks and current-source XY-C1 build remain required; no local full checkout or hardware test is claimed.

---

## 2026-11-15 — Fix GNU md5sum compatibility and checked-storage call contracts

- Exact-head USB storage run #43685359346 on `5338c5adfe54a7eedcfc460b284f4783c03a09cf` passed MTD script syntax and the baseline/write-failure/retry cases, then failed because the unchanged-save check used BusyBox's `md5sum -s` option. The GitHub-hosted GNU coreutils `md5sum` rejects `-s`, so every unchanged save fell through and rewrote MTD. Replaced it with portable `md5sum -c` and explicit output redirection; a local shell reproduction with GNU md5sum confirms the retry succeeds and the next identical snapshot is skipped.
- Exact-head Process inventory/lifecycle #43685359507 and HTTPD security #43685359427 exposed stale source-contract literals expecting `write_storage_to_mtd();`; production now invokes the status-returning function as an expression in a ternary. Updated both assertions to recognize the call while preserving their surrounding ordering checks.
- No lifecycle or storage semantics were relaxed. Fresh exact-head storage, process-lifecycle, HTTPD and XY-C1 Actions are required; the prior XY-C1 run #43685359467 was still IN_PROGRESS at last observation.
- Changed paths: `trunk/user/scripts/files/sbin/mtd_storage.sh`, `scripts/check_process_lifecycle_integration.py`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent SHA: `5338c5adfe54a7eedcfc460b284f4783c03a09cf`. Full repository regression suite and target build are not claimed locally.

---

## 2026-11-15 — Preserve retryability and propagate MTD storage save failures

- mtd_storage.sh previously published the changed archive MD5 to storage_md5 before invoking mtd_write. A failed flash then caused the next identical save to skip retrying and leave the old MTD snapshot active. The hash is now staged and published only after a successful flash; staging, compression, flash and checkpoint errors return non-zero.
- write_storage_to_mtd() propagates doSystem() status. Startup logs failed persistence and continues crond startup; shutdown reports failure but continues power-down; firmware update refuses to write the image unless timestamp and MTD persistence succeed and attempts watchdog recovery on abort.
- storage_save_time() now validates time/localtime and overflow, stages a same-directory temp file, checks write/flush/fsync/close, and atomically renames it. Failures clean up without truncating the prior timestamp.
- Added a shell regression for successful baseline → injected MTD failure → identical retry → unchanged skip, a GNU89 -Wall -Wextra -Werror persistence-policy test, and source/order contracts. Storage-hotplug and XY-C1 path filters include the source and tests.
- Changed paths: rc.c, rc.h, mtd_storage.sh, MTD retry test, persistence policy/header test, storage-hotplug and XY-C1 workflows, checker, handoff, roadmap and dev_log. Parent SHA: 31ae7b6f253734cc0abf5f67b5c34e861229155b. Exact-head Actions/local test/build/hardware are not claimed.

---

## 2026-11-15 — Stop Transmission and Aria daemons by verified process identity

- Transmission and Aria stop paths use process-inventory preflight, bounded kill_services(), and verified daemon absence. Public stop APIs hold the shared storage operation lock and re-check postconditions; RUNNING or UNKNOWN prevents eject success.
- Source commit 2aedad115793e7a6de4f2114272c845559105783. The initial source commit was not separately recorded in the top-level chronological dev log before this catch-up entry.

---

