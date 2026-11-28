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

