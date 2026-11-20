## 2026-11-20 — Establish iNIC isolation before enabling RT3352 MII radio

- Parent main SHA: `9075dc6385307f5df83ce9d13039462648395583`.
- Follow-up review found a boot/out-of-band state gap: enable assumed the RTL8367 iNIC port was already isolated and turned on radio power before checking switch state. A stale/unexpected unisolated port could expose the newly enabled radio during the transition.
- Enable now explicitly asserts switch isolation first; any initial-isolation error aborts enable, performs best-effort RadioOn=0, preserves the isolation error and does not publish state. Only then does it enable the driver, apply TxPower and unisolate. Existing power-on/unisolation rollback remains.
- Extended GNU89 regression coverage with initial-isolation failure and event-order assertions. Strengthened the Wireless source contract to require isolation-before-power-before-unisolation.
- Parent exact-SHA Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149 passed on `9075dc6385307f5df83ce9d13039462648395583`; NVRAM #44744845007 was pending. Product-source XY-C1 #44744529158 on `9e9d643d` was still running. New exact-head regression pending; no MIPS L3 or hardware L4 claim.

---

## 2026-11-20 — Move RT3352 MII policy regressions to file scope and restore handoff checkpoint contract

- Parent main SHA: `9e9d643dd8c3a78d135c8dae908857f8b90c0276`.
- Wireless radio-state run #44744529117 on `9e9d643dd8c3a78d135c8dae908857f8b90c0276` failed at GNU89 compilation: the added MII mock callbacks and test functions had been inserted inside `main()`, producing invalid storage-class errors. Moved the complete helper/test block to file scope while retaining the seven regression calls inside `main()`.
- AI handoff check #44744529015 failed because the shortened SESSION_HANDOFF omitted the mandatory checkpoint language and four required state/task/verification/next-action sections. Restored the exact governance phrases and required headings without changing bootstrap/protocol rules.
- On the source SHA, Process inventory #44744529131, Process regression #44744528956, HTTPD security #44744528974, watchdog #44744529024 and iNIC signal #44744528955 passed. XY-C1 #44744529158 and NVRAM #44744528984 were pending. Corrected Wireless/AI exact-head Actions and same-source L3 remain pending.

---

## 2026-11-20 — Make RT3352 MII radio/isolation transitions transactional

- Parent main SHA: `cb4cb1debdfc2b5ba3b773e2e5fbbb6b58449f96`.
- `mlme_radio_rt_apply()` ignored the `phy_isolate_inic()` switch ioctl result and published MLME/NVRAM plus LED state even when the RTL8367 iNIC isolation boundary had not transitioned successfully.
- Added an RT3352 MII-only injectable transition policy. Enable keeps the port isolated until radio power is on; failed power/unisolation triggers best-effort isolate/off rollback. Disable attempts isolation before radio-off but still powers the radio down if isolation fails. MLME/LED publication is gated on successful required hardware operations.
- Added GNU89 regressions for event ordering, failed power-on rollback, unisolation/isolation failure, power-off failure and no-publication behavior. Updated Wireless source-contract checks and the XY-C1 production-input trigger for the helper header.
- Local validation: standalone transition-policy harness passed `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`. Exact-head Wireless/Process/AI/NVRAM/XY-C1 checks are pending; no target MIPS L3 or physical L4 claim.

---

## 2026-11-20 — Fix MiniUPnP lease write errno source contract

- Parent main SHA: `3bdb1d9f68767aeea29ce14bc93959426ee382e4`.
- Exact-head MiniUPnP regression #44743068543 on `ee9428dcaedd7530695ddfeedcac7a326448a705` compiled and passed every C policy/sanitizer binary, then failed its Python source contract at the lease writer. The checker compared the first `errno = 0;` (inside the mutually exclusive record-format error branch) against `fwrite()`, so it did not test the reset on the normal write path.
- Corrected the contract to select the nearest errno reset before the write call; the final-close ordering assertion was already corrected by parent commit `3bdb1d9f`. Verified the write and close reset positions against current `upnpredirect.c`.
- Parent exact-SHA MiniUPnP regression #44743367478 and AI handoff #44743367373 PASS. The failed #44743068543 run is on the preceding SHA and is not a current-head failure. New exact-head Actions are pending; no L3 claim.

---

## 2026-11-20 — Escape untrusted UPnP mapping output before SOAP XML serialization

- Parent main SHA: a6897a11e4e19c6342f5a267ec28212467a5f443.
- Hardened SOAP mapping responses: stored descriptions, remote hosts, internal-client addresses and protocol labels are XML-escaped in GetSpecific/GetGeneric and the inner PortMappingList XML; invalid XML C0 controls are replaced with spaces and bounded output is checked.
- Restrict inbound WANIPConnection NewProtocol to case-insensitive TCP/UDP and parse NewPortMappingIndex as strict unsigned decimal 0..INT_MAX before conversion. This prevents arbitrary protocol strings being echoed into PortMappingList and closes the prior strtol trailing-junk/negative-index ambiguity.
- Added a GNU89/ASan/UBSan XML escaping regression and source contracts. Exact-head AI docs #44742412824 PASS. MiniUPnP #44742412812 failed only because the test used an 8-byte buffer for &amp; (6 bytes including NUL), so the helper correctly succeeded; changed the test to supply 5 bytes. XY-C1 #44742412813 was still running.
- Prior: SOAP numeric input SHA `2d124f3d` passed all exact-head Actions including XY-C1 Build/TRX/artifact; lease description serialization SHA `368f9fa` MiniUPnP host regression PASS, XY-C1 still in progress.

---

## 2026-11-20 — Prevent MiniUPnP lease-file description record injection

- Parent main SHA: 2d124f3dbf163982c3f6198baf8c176a035585ff.
- Confirmed `lease_file_add()` wrote client-supplied `NewPortMappingDescription` verbatim into a colon-delimited, newline-separated lease file. `reload_from_lease_file()` reads each line as a mapping record, so embedded CR/LF could forge additional persistent rules; unbounded descriptions could also exceed the fixed 128-byte read buffer.
- Added a bounded serializer that sanitizes CR/LF, caps the description to 63 bytes, and fails rather than emitting a record larger than the reload parser's buffer. Production lease writing now checks short writes and fclose errors, preserving the first errno.
- Added GNU89/`-Werror`/ASan/UBSan coverage for line-injection payloads, overlong descriptions, exact one-line framing, undersized buffers and invalid arguments. Wired source contract, Makefile dependency, MiniUPnP workflow and XY-C1 production-input detector.
- Prior SOAP numeric-input commit `2d124f3dbf163982c3f6198baf8c176a035585ff`: MiniUPnP #44740365744 and AI docs #44740365813 PASS; Process inventory #44740365812 PASS. XY-C1 #44740365797 was still in progress (no L3 claim). This lease-file slice is pending exact-head validation; no target L4 evidence.

---

## 2026-11-20 — Reject malformed and out-of-range MiniUPnP WANIPConnection numeric inputs

- Parent main SHA: 5b565e65f677f8f16b9a2012751e98510a40a536.
- SOAP handlers used `atoi()` followed by `unsigned short` narrowing for WANIPConnection ports and range endpoints. Values such as `65536` could wrap; signs, whitespace, trailing junk and empty strings were not consistently rejected. Lease duration and list count also lacked signed-range validation before conversion.
- Added a shared header-only decimal parser with overflow-safe bounds. Port inputs are limited to 0..65535 before narrowing; lease duration and listing count are limited to 0..INT_MAX. Existing wildcard/error behavior, zero lease defaults/cap and zero listing-count default remain intact.
- Added GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy coverage for boundaries, overflow, NULL/empty, signs, whitespace, junk and output preservation. Added source contracts, Makefile dependency and XY-C1 detector/path coverage for the production helper and SOAP source.
- Exact-head MiniUPnP/AI docs Actions and XY-C1 detector/Build/TRX/artifact are pending on the resulting SHA. No local compiler or router run was available in this API-only session; no L3/L4 claim is made.

---

## 2026-11-20 — Repair watchdog CPU policy contract and event triggers

- Parent main SHA: cc9d48f7ca5e1ecc39b99c1c0b7ca1218898f883.
- Watchdog workflow 44738607355: signal, NTP/service lifecycle and new CPU module restart regression binaries all passed. The final checker failed because it expected `module_smart_load(module_name, NULL)` inside the injected policy header; the real policy calls `ops->load(module_name, NULL)`, while `services.c` owns the concrete module_smart_load adapter. The source contract now checks the actual interface and ordering.
- The policy header/test had been inserted into the Python expected-path tuple but not YAML `push.paths`/`pull_request.paths`. Both are now added exactly once to both event filters; the existing loop enforces these entries.
- Active MiniUPnP SOAP validation remains the next product-source task from the current handoff. No production code modified in this workflow repair.

---

## 2026-11-20 — Current-main handoff and exact-SHA CI checkpoint

- Current main: `940cad61cf1171f713b3e37890334c9702f5f399`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Recent MiniUPnP commits: ee610712cc0c776c450c93cded990c8f8db64acd (ci(miniupnpd): fix daemon failure checker quoting); 885f212b6065ef15276f7d4f6189b02c75fbbc7c (fix(miniupnpd): abort on daemon detachment failure); 10b54df775dd8ecd80bb16a3d71928418c7e8c06 (fix(miniupnpd): serialize daemon startup and validate pidfile); 5778580685e496a4e3f170a91186675a3410f6c7 (ci(xy-c1): build on miniupnpd pidfile changes); 2ede97f6206c320ce2aedff90b331fc85116a01a (fix(miniupnpd): preserve pidfile umask and test build); 12e4a6f0d8a8c57cffeee70424b59dce8afb05b3 (fix(miniupnpd): publish PID files atomically).
- Exact-head Actions: AI handoff documentation contract #44738607273: completed/success; watchdog signal lifecycle stability regression #44738607355: completed/failure.
- This handoff records CI state without converting pending/running checks into PASS. Immediate next task is exact-head MiniUPnP regression/firmware verification, then independent userland P1 work while Actions run.

---

## 2026-11-20 — Wire the CPU watchdog restart policy regression into Actions

- Parent main SHA: ee610712cc0c776c450c93cded990c8f8db64acd.
- Review of `watchdog-signal-stability.yml` showed it compiled signal deferral, NTP and service-lifecycle regressions, but did not compile/run the existing `watchdog_cpu_restart_policy.c` test or watch its header/test paths.
- Added GNU89/`-Wall -Wextra -Werror` with ASan/UBSan compile/run, integration contracts for `start_watchdog_cpu()`/`restart_watchdog_cpu()`, policy behavior and regression-case markers, plus push/PR path filters for the policy header and test.
- The XY-C1 workflow already watches `services.c` and `watchdog_cpu_restart_policy.h`; the ongoing product-source build on SHA `885f212` includes the production watchdog policy code.
- New exact-head watchdog/AI docs Actions pending. The regression source had not previously run in the dedicated watchdog workflow, so no prior result is claimed.

---

## 2026-11-20 — Repair daemon detachment source-contract checker quoting

- Parent main SHA: 885f212b6065ef15276f7d4f6189b02c75fbbc7c.
- Exact MiniUPnP workflow run 44737808024 passed all C binaries, including the new PID-file atomic-write/read/strict-parse/cross-process-lock tests. It failed only when the source-contract Python heredoc parsed an assertion whose embedded double quote was incorrectly backslash-escaped twice.
- Replaced that one assertion with a single-quoted Python literal containing the exact C token `perror("daemon()")`. No production source changed in this repair.
- On parent 885, AI docs passed; Process, NVRAM and XY-C1 Actions were in progress. Exact-head reruns pending.

---

## 2026-11-20 — Fail closed when MiniUPnP daemon detachment fails

- Parent main SHA: 10b54df775dd8ecd80bb16a3d71928418c7e8c06.
- Verified `genconfig.sh` unconditionally emits `#define USE_DAEMON`. In that default branch, a failed BSD `daemon(0, 0)` only called `perror()` and continued in foreground, then initialized UPnP/NAT-PMP and could return success as a supposed detached service.
- The branch now saves and restores errno around `perror()` and returns failure before assigning `pid = getpid()` or continuing startup.
- MiniUPnP stability workflow now asserts the failure return and errno preservation. XY-C1 push path filter and detector unit test now include `miniupnpd.c`; this matters because the prior path set had not scheduled XY-C1 when only this product source changes.
- Parent SHA `10b54df` already has focused MiniUPnP host regression/source contract PASS, Process inventory PASS and AI docs PASS; NVRAM and XY-C1 were pending, and the latter had advanced into Build. Exact-head reruns on this new source SHA are pending.

---

## 2026-11-20 — Serialize MiniUPnP singleton startup and fail closed on invalid PID state

- Parent main SHA: 5778580685e496a4e3f170a91186675a3410f6c7.
- Confirmed checkforrunning() treated every open failure as “no daemon”, did one unchecked read and parsed with atol(); a FIFO PID path could block, malformed/trailing text could be accepted, and simultaneous starts could both pass the check before either wrote its PID.
- Added stable sidecar .lock inode, opened O_NOFOLLOW/O_CLOEXEC, checked as regular, singly linked, euid-owned and private, then exclusively F_SETLK-locked and held in a static process-lifetime descriptor. The lock is deliberately never unlinked, so its inode stays stable.
- PID-file validation now rejects symlinks/nonregular/hardlinked/foreign-owned files, reads boundedly with capped EINTR retries, requires exact positive decimal PID followed by newline, treats only ENOENT as initial absence and ESRCH as stale state, and ignores a stale PID equal to the newly reused current PID.
- Regression adds forked contender exclusion/release, lock symlink/FIFO rejection, short/interrupted read, bounded EINTR storm, malformed/overflow/zero PID cases. Prior atomic publication, short-write, ENOSPC, symlink and umask regressions remain.
- Standalone GNU89/-Wall -Wextra -Werror/ASan/UBSan suite PASS. Exact-head MiniUPnP/Process/AI/NVRAM and XY-C1 Build/TRX/artifact remain pending on new source SHA.

---

## 2026-11-20 — Trigger XY-C1 build for MiniUPnP daemon/PID-file inputs

- Parent main SHA: `2ede97f6206c320ce2aedff90b331fc85116a01a`.
- XY-C1 workflow path filters did not include MiniUPnP daemonize.c, the new PID publication helper header or its package Makefile, so the preceding MiniUPnP product-source commits did not schedule a cross-build even though the firmware-input detector would treat these paths as production inputs.
- Added all three paths to the XY-C1 push filter and added a detector regression requiring each to be present and classified as a firmware input. The workflow already lists itself as a path trigger, so this contract update schedules the exact new head.
- MiniUPnP host workflow, Process and AI docs passed on parent `2ede97f`; its NVRAM boundary audit was pending. Current-source XY-C1 Build/TRX/artifact has not yet run on that parent.

---

## 2026-11-20 — Preserve MiniUPnP PID-file umask and fix host-test include path

- Parent main SHA: `12e4a6f0d8a8c57cffeee70424b59dce8afb05b3`; source implementation is `12e4a6f0d8a8c57cffeee70424b59dce8afb05b3`.
- Exact MiniUPnP Action `44735307616` compiled and passed config-publication and daemon-lifecycle binaries, then failed compiling the new nested PID policy test because its quoted header is one directory above the test source. Added the package directory to the test's `-I` path.
- Code review found unconditional `fchmod(fd, 0644)` bypassed the daemon's established umask 0027 and changed the PID file from legacy effective 0640 to 0644. The helper now reads/restores umask and applies `0644 & ~old_umask`; regression asserts 0640 under umask 0027.
- Standalone GNU89/`-Wall -Wextra -Werror`/ASan/UBSan regression PASS after both changes. Exact-head Actions and XY-C1 on the resulting source SHA remain pending.

---

## 2026-11-20 — Publish MiniUPnP PID files atomically and reject incomplete writes

- Parent main SHA: `573d215607c8e8a4b7e22048c2bbdd633a3a2d5f`.
- Confirmed `writepidfile()` opened `/var/run/miniupnpd.pid` without truncating previous contents, checked only negative `write()` results, ignored short writes and returned success even when PID content was not fully persisted. A shorter new PID could retain stale trailing digits; interruption or storage failure could leave a partial PID that later blocks service startup.
- Added `miniupnpd_pidfile_policy.h`: stage in the destination directory with `mkstemp()`, apply the prior writer's intended 0644 mode, loop until every byte is written, bound repeated EINTR, sync and check close, then publish with one atomic `rename()`. Failed staging leaves the previous complete PID file untouched; an existing symlink destination is replaced as a directory entry, not followed.
- `writepidfile()` now validates PID text and propagates formatting/publication failure. The MiniUPnP object dependency, host regression, source contract and symmetric push/pull_request path filters are updated together.
- Standalone GNU89 `-Wall -Wextra -Werror` ASan/UBSan regression PASS: shorter replacement, EINTR plus one-byte writes, bounded EINTR storm, ENOSPC/zero-write failure, temporary cleanup and symlink-target preservation. Exact-head MiniUPnP/AI docs Actions and XY-C1 Build/TRX/artifact are PENDING on the new SHA; no L3/L4 claim.

---

## 2026-11-20 — Normalize APPE source-contract argument whitespace

- Parent main SHA: `8ded66573ba530efd6c66763981e136dc7a1097a`; APPE code/test implementation is `8ded66573ba530efd6c66763981e136dc7a1097a`.
- Exact VSFTPD run `44734275928` reached the Python source-contract stage after compilation was underway. The new API call correctly split `exclusive, append,` and `(mode_t)mode` across lines, while the checker expected one contiguous source substring.
- Normalize the extracted function body whitespace before asserting the argument sequence. The test still checks the exact `exclusive, append, (mode_t)mode` order; no production source or regression behavior is relaxed.
- Exact `8ded665` VSFTPD and XY-C1 outcomes were still pending at the time this checker correction was prepared.

---

## 2026-11-20 — Preserve APPE append semantics across independent FTP sessions

- Parent main SHA: `821c133887352c069067688f442cd5300bd619b2`; existing-STOR FIFO/special-file rejection commit `5c7fa81094c2e31ed58d0e8381201ca769db7a2c` has exact VSFTPD stability run `44733281243` SUCCESS. Its AI docs `44733281199`, Process inventory `44733281193`, and NVRAM boundary `44733281201` also PASS. Same-SHA XY-C1 run `44733281232` was IN_PROGRESS at last poll.
- APPE opened targets without `O_APPEND`, then performed one `lseek(SEEK_END)` before transfer. With configurable `lock_upload_files=NO`, two sessions can seek to the same old EOF and the later write overwrites bytes appended by the first session. The default advisory lock is not a semantic substitute because it is configurable.
- Threaded an explicit append flag through `handle_upload_common()` → `asus_open_upload_path()` → `ftp_acl_path_open_upload_entry()`. APPE now opens existing and newly created targets with `O_APPEND`; STOR/STOU preserve their prior truncate/exclusive behavior.
- Added a two-descriptor regression: both simulated APPE sessions seek to offset zero before either write, then append distinct bytes; final content must be `baseAB`. The VSFTPD source contract requires `is_append` to reach `O_APPEND` end-to-end.
- Parent FIFO source-contract host regression passed. This APPE slice requires exact-head VSFTPD Actions and same-SHA XY-C1 Build/TRX/artifact. Physical FTP device L4 remains NOT VERIFIED.

---

## 2026-11-20 — Zeroize Ralink 802.1X RADIUS secrets before freeing

- Parent main SHA: 5c7fa81094c2e31ed58d0e8381201ca769db7a2c.
- `Config_free_radius()` released each dynamically allocated RADIUS shared secret with plain `free()`, leaving credential bytes in reusable heap memory after configuration teardown. The two driver-import paths now allocate exactly `shared_secret_len + 1` bytes, so the cleanup boundary has an exact wipe length.
- Add a volatile-byte zeroization helper and wipe the complete allocation, including its terminator, before freeing; clear the pointer and stored length after release. NULL/zero-length cleanup remains harmless for partial configuration teardown.
- Add GNU89 sanitizer regression coverage that verifies every byte is zeroed while bytes outside the requested span remain untouched. Extend the Process source contract to prove wiping happens before `free()`.
- Exact-head Process/AI and XY-C1 Build/TRX/artifact validation still required; no physical-router L4 claim.

---

## 2026-11-20 — Reject FIFO and special-file targets for existing FTP STOR

- Parent main SHA: `107b57abb1374ff57b275dba2c78a0c5069dabf0`. Current exact-main Actions at checkpoint: Process inventory `44732688655` PASS, Process fail-closed `44732688588` PASS, AI docs `44732688571` PASS; XY-C1 `44732688670` and NVRAM boundary `44732688631` were IN_PROGRESS. WAN teardown exact rerun on `be32768262941acab4bebf9d747db381ec4a93b2` passed `44732616912`; WAN route contract passed on `505b26f` (`44732377886`). Same-SHA XY source L3 is not established until Build/TRX/artifact all pass together.
- FTP source review found an asymmetry: RETR/SIZE and ACL config reads reject non-regular descriptors, but the existing-file STOR branch in `ftp_acl_path_open_upload_entry()` reopened an existing leaf with `O_WRONLY|O_NONBLOCK|O_NOFOLLOW` and returned it without `fstat()`. A pre-existing FIFO with an active reader could therefore be accepted as a remote write destination; the upload code also does not reject non-regular descriptors after open.
- The existing STOR branch now fstats the descriptor actually returned by openat(), rejects non-regular inodes with `EINVAL`, closes on both fstat and type failures, and preserves the fstat errno. New O_EXCL-created upload targets remain regular by construction and keep their cleanup eligibility semantics unchanged.
- Added a FIFO regression with a nonblocking reader so the underlying O_WRONLY open itself succeeds; it asserts STOR helper rejects it, clears cleanup eligibility, and leaves the FIFO entry intact. VSFTPD workflow source contract requires the exact open→fstat→S_ISREG gate and the FIFO regression. Header/test already belong to both push and pull_request trigger paths.
- Target MIPS/uClibc and live FTP compatibility remain to be checked by exact-head Actions and current-source XY-C1. FTP physical-device L4 remains NOT VERIFIED.

---

## 2026-11-20 — Correct Ralink 802.1X diagnostic newline escapes

- Parent main SHA: be32768262941acab4bebf9d747db381ec4a93b2.
- Source review found five newly added diagnostics in the previous 802.1X hardening that used double-escaped `\\n`, printing a literal backslash-n instead of terminating the log line. Correct the NAS, EAP, pre-auth and BSS-interface diagnostic strings.
- Extend the Process source contract to reject double-escaped newline sequences in both the driver-configuration and socket-setup functions. This is a diagnostic follow-up; bounds checks and RADIUS ownership are unchanged.
- Exact-head Process, AI docs and XY-C1 checks still required for the complete 802.1X slice.

---

## 2026-11-20 — Complete deferred WAN policy path filters for pull requests

- Parent main SHA: `505b26f869c63261508e377360022e0fc0936707`. The WAN route correction from `505b26f869c63261508e377360022e0fc0936707` passed exact run `44732377886`; AI handoff docs passed `44732378051`.
- Exact WAN teardown run `44732377859` passed the C policy binaries and the corrected schedule-order assertions, then failed its final path-filter contract: the two new deferred-reconnect policy/test paths existed in the push filter but not in the pull_request filter. The earlier update had replaced only the first textual occurrence; this follow-up adds and asserts exactly one entry in each filter.
- No production source changed in this slice. Deferred reconnect production behavior remains in `ab02b36`; this is CI-trigger/contract completion only.
- Product source `8ce56358d91e9fb937417a30751fb86cd8c71f0e`: Process inventory `44732095102`, fail-closed `44732095105`, and NVRAM boundary `44732095026` PASS; XY-C1 `44732095014` remained IN_PROGRESS at last poll. Same-SHA Build/TRX/artifact is still required for L3. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Align WAN reconnect source contracts with checked scheduling

- Parent main SHA: `8ce56358d91e9fb937417a30751fb86cd8c71f0e`; this is a checker-only follow-up to source fix `ab02b36bc2cad3dc15f28bf950e347a0f93cf7b6`. Subsequent main commits `cd300104a22cd1d9d33e2c3f9fef4e970204344a` and `8ce56358d91e9fb937417a30751fb86cd8c71f0e` independently hardened Ralink 802.1X RADIUS bounds and preserve the correct per-BSS/global secret ownership; those changes are retained.
- Exact `ab02b36` WAN teardown run `44731843422` reached the inline source contract after all GNU89/Werror/ASan/UBSan C regressions had compiled and run. Its new order assertion mistakenly used the first `return status;` in all of `try_wan_reconnect()`, which belongs to an earlier stop/reset failure branch; it must find the return after the schedule-publication error gate.
- Exact `ab02b36` WAN route run `44731843190` passed its compiled regression suite and existing source checks until `scripts/check_wan_deferred_watchdog_status.py` expected the removed unchecked `nvram_set_int_temp("deferred_wanup_t", deferred_up_time)` call. Update that contract to require checked deadline publication, its failure return, watchdog launch, watchdog failure return and only then reconnect success. Also assert that no direct deferred-marker setter remains and the policy enforces the `INT_MAX` domain.
- This follow-up changes only the two affected source contracts and their durable checkpoint. It does not relax the code coverage: both scripts now assert branch-relative ordering and exact production checked-publication behavior.
- Exact `ab02b36` checks already PASS: Process inventory `44731843155`, WAN DHCP callback `44731843164`, AI docs `44731843180`, ZeroTier `44731843498`, Wireless `44731843320`, watchdog signal `44731842896`, VPN lifecycle `44731843303`, HTTPD security `44731843263`, Firewall startup `44731843262`, DNSMasq `44731843255`, miniupnpd `44731843327`, detect-internet `44731843226`, detect-link `44731843192`, USB modem identity `44731843412`, and WAN ARP `44731843120`. WAN teardown `44731843422` and WAN route `44731843190` were checker failures described above; Process fail-closed `44731843287` and NVRAM boundary `44731843056` were cancelled. XY-C1 run `44731843051` remained in progress at last poll.
- Current exact main `8ce56358d91e9fb937417a30751fb86cd8c71f0e`: AI docs `44732095055` PASS; Process inventory `44732095102`, Process fail-closed `44732095105`, NVRAM boundary `44732095026`, and XY-C1 `44732095014` were in progress at last poll. L3 for all changed product sources requires one exact-SHA Build, TRX and artifact upload. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Preserve per-BSS and global RADIUS secret destinations

- Parent main SHA: cd300104a22cd1d9d33e2c3f9fef4e970204344a.
- Post-edit source review caught an incomplete branch mapping in the preceding RADIUS hardening commit: the `MULTIPLE_RADIUS` path must assign the copied secret to `mbss_auth_server[i]`, while the single-server path assigns it to `auth_server`. The exact source-contract assertion also correctly rejects the still-present legacy `strdup` path.
- Correct both blocks explicitly and strengthen the Process contract to require both destination assignments. The full two-branch regression remains pending until this exact SHA passes.

---

## 2026-11-20 — Bound Ralink 802.1X RADIUS server and secret fields

- Parent main SHA: ab02b36bc2cad3dc15f28bf950e347a0f93cf7b6.
- The driver configuration ioctl also exposed two unchecked bounds: `radius_srv_num` indexes a fixed two-entry server array, and `radius_key_len` describes a fixed 64-byte secret. The old loop trusted the count, while `strdup()` scanned for an unrelated NUL and debug logging printed the shared secret.
- Validate each server count against the actual array capacity. Copy RADIUS keys by their checked byte length into an allocated, explicitly terminated buffer; reject lengths beyond the source field, propagate allocation failure, and preserve the exact protocol length.
- Remove credential contents from debug logs; retain only configured state and key length. Add GNU89 regressions for server-count boundaries, a full-width 64-byte non-NUL key, invalid lengths, and terminator placement.
- Exact-head Process/AI and XY-C1 build/TRX/artifact checks required. L4 remains NOT VERIFIED.

---

## 2026-11-20 — Fail closed on deferred WAN reconnect NVRAM errors

- Parent main SHA: `743f0158ff728358efc2026ef37e3ef094984560`; last verified product-source baseline: `7210c3a3a46093c633b56c8975ab2cd946b2e1bb`.
- `watchdog.c:inet_handler()` fires a PID 1 reconnect notification once `deferred_wanup_t` reaches its deadline. `stop_wan_ppp_locked()` and `stop_wan_locked()` previously cleared that deadline with unchecked `nvram_set_int_temp()`; a failed clear left a delayed-reconnect intent behind a manual disconnect/teardown. `try_wan_reconnect()` also ignored deadline publication failure and could return success without scheduling the requested reconnect, while `deferred_wan_connect()` ignored failure to clear the fired deadline before WAN activation.
- Added a shared deferred-reconnect policy and checked NVRAM publisher. Teardown now aborts before stopping VPN/PPP services if cancellation cannot be committed; deferred startup propagates clear failures before modem selection or WAN activation; delayed reconnect scheduling reports setter errors before claiming success.
- The schedule policy rejects negative values, zero/negative delays and deadlines outside the positive signed-int range consumed by `nvram_get_int()`; it checks overflow before addition. GNU89 regression covers clear/schedule success, injected ENOSPC/EIO, invalid arguments, integer-width overflow and deadline overflow. The WAN teardown workflow runs it with ASan/UBSan and enforces production call ordering and both push/PR trigger filters.
- Parent product-source SHA `7210c3a3a46093c633b56c8975ab2cd946b2e1bb` has exact XY-C1 run `44729001694` SUCCESS: Build XY-C1, TRX verification and artifact upload all passed; artifact `14625343431`, SHA256 `1312137c206644da15a8cb5db9f1507d63d3edc51f530d61eac453ed0fae6bf9`. NTP readiness source SHA `c10f4fd6be13a4dc915f397ccc3292d92509e689` has watchdog run `44731160914` PASS, Process inventory `44731160912`, fail-closed `44731160915`, DNSMasq `44731160963`, HTTPD lifecycle/security `44731160953`/`44731160941`, and NVRAM boundary `44731161012` PASS; XY-C1 `44731160909` was still pending at last poll. These results validate only the parent baselines, not this reconnect slice.
- The new exact-head WAN teardown regression, Process integration, AI documentation and XY-C1 firmware verification must be checked on this slice after commit. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Retry failed NTP readiness persistence

- Source commit: c10f4fd6be13a4dc915f397ccc3292d92509e689; documentation parent main: c10f4fd6be13a4dc915f397ccc3292d92509e689.
- `ntpc_updated_main()` previously incremented temporary `ntpc_counter` before checking whether the callback supplied an offset, then ignored the result of the persistent `nvram_set_int("ntp_ready", 1)` write. A transient readiness write failure could leave time-dependent services gated until the next NTP step.
- Now require a non-empty offset before counting a completed step, check/report both NVRAM writes, and retry `ntp_ready=1` from the watchdog timer whenever `ntpc_counter>0` while readiness remains clear. The retry is independent of WAN state and runs before `svc_timecheck()`.
- Added GNU89 policy cases for null/empty offsets and readiness retry conditions, plus source-contract assertions for the update ordering, retry, rate-limited error log and recovery log.
- Exact-main watchdog CI: PASS (run 44731160914); XY-C1: PENDING (run 44731160909). Product Build/TRX/artifact remain unverified until exact-SHA steps are all green.
- Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Bound Ralink 802.1X driver-supplied interface and NAS identifiers

- Parent main SHA: 80d8e6af6d2ba358ea29f0cfc067c8c8536ed3b8.
- Follow-up review of the authenticator found fixed-buffer hazards: driver-provided `EAPifname_len` and `PreAuthifname_len` were copied unchecked into 16-byte arrays, related `ifreq.ifr_name` buffers used `strcpy`/`sprintf`, and the NAS Identifier length was copied unchecked into a 32-byte array then logged with an unbounded `%s`.
- Added a bounded driver-name copier which validates source-field and destination capacity, accepts an optional trailing NUL, rejects embedded NULs and always terminates the destination. Reject oversized NAS-ID lengths before copying; use explicit precision for logging a full-width 32-byte identifier.
- Replaced remaining interface-name string copies with fixed-size copies from validated, zero-terminated fields. The per-BSS interface uses the checked formatter and closes its socket on invalid input.
- Added host regressions for raw and terminated names, the 15-character boundary, overlong/malformed input, source-length overflow, undersized buffers and invalid pointers. Process source contracts cover config and setup callsites. Added `config.c` to Process/XY path filters and XY production-input tests so a source change triggers same-SHA firmware validation.
- Exact-head Process/AI docs and XY-C1 Build/TRX/artifact verification required. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Retry failed Internet connectivity NVRAM mirror publication

- Parent main SHA: 7210c3a3a46093c633b56c8975ab2cd946b2e1bb; current documentation checkpoint child: 327b83d651b690c55626ac3c2bbd7ef6dcfd16ec.
- Source review found three unchecked temporary writes of `link_internet`: reset to `DI_STATUS_INIT`, state-transition publication and explicit pause-to-zero. The event cache was advanced before the NVRAM write, so a failed write could leave the external mirror stale indefinitely.
- Added a small callback policy that advances its cached value only after a successful NVRAM write. Reset, transition and pause now share the checked publisher; failed writes retry on the next timer tick with a one-second retry cadence while preserving one-shot state-transition side effects.
- Added a separate desired-value field. When startup is paused and the initial INIT write fails, retry ticks publish INIT; after explicit pause they publish zero. The main event loop now admits paused timer ticks only while the NVRAM publication error latch is set; successful retry disables the timer again.
- Added GNU89/`-Wall -Wextra -Werror`/ASan/UBSan regression and source contracts for failed initial/changed-state publication, retry, previous-cache preservation, invalid arguments, paused retry dispatch and recovery logging.
- Exact detect-internet regression `44729054343` and AI docs `44729054197` PASS on `327b83d6`. Earlier detector failures `44728739057` and `44728819718` were stale checker expectations; corrected and re-run successfully.
- Same-source XY-C1 Build/TRX/artifact remains pending: run `44729001694` on `7210c3a3` was building. Do not claim detect-internet L3 yet. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Bound Ralink 802.1X wireless interface-name formatting

- Parent main SHA: e2d0a6f65a91c57f033b5b28e01e9a9904b28869.
- `RT_ioctl()` used `sprintf(wrq.ifr_name, "%s%d", prefix_name, apidx)` with the fixed `IFNAMSIZ` array. An overlong `-i` prefix or multi-digit AP index could overwrite the stack-resident `struct iwreq`.
- Added a shared formatter using the real destination capacity. Empty/invalid input returns `EINVAL`; truncated names are cleared and rejected with `ENAMETOOLONG`. Every RT ioctl now uses this checked path; valid interface names preserve the original formatting.
- Added GNU89 regression coverage for ordinary names, the 15-character boundary, multi-digit indexes, overlong prefixes, undersized buffers and invalid arguments. The existing Process regression compiles this test with ASan/UBSan; its inline source contract now rejects the unbounded call.
- Exact-head Process/AI docs and XY-C1 verification required; L3 requires Build, TRX and artifact upload on this same SHA. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Create Samba private state directory with restrictive permissions

- Parent main SHA: c40acbe1e767de9bc1fed7b8f3001303670d3bdf.
- Found a transient exposure in `samba_config_transaction_prepare_parent_directory()`: it created the sensitive `/etc/samba` parent with mode 0755 and only later opened the directory and changed it to 0700. With PID 1's permissive umask, other local users could access the directory during that interval and race to plant passdb or transaction-journal entries.
- Create new directories with mode 0700 from the initial `mkdir()`. Existing owner validation, `O_DIRECTORY | O_NOFOLLOW`, descriptor-based mode repair and post-`fchmod` verification remain unchanged for legacy directories. Extend the Samba source contract to require the restrictive creation mode and reject 0755.
- Existing GNU89 regression already sets `umask(0000)` and exercises first creation plus repair of 0777/0711 directories. Exact-head Samba host regression/source contract and same-SHA XY-C1 build/TRX/artifact verification required; L4 remains NOT VERIFIED.

---

## 2026-11-20 — Remove duplicate Ralink authenticator workflow path filters

- Parent main SHA: 1f129cad823186909583b9fa2a1edd8754fd84e7.
- The Ralink 802.1X readiness handshake GNU89 regression passed on this exact SHA, but Process lifecycle run `44727530981` failed at the final workflow-integrity guard because the newly added header and test paths were each listed twice in both push and pull-request filters. AI handoff validation `44727531002` independently rejected the same duplicate path entries.
- Remove only the redundant copies; retain one trigger entry for the production helper and one for its regression test in each event filter. No source/test assertions are weakened.
- Exact-head Process, AI-docs and XY-C1 validation are required for the repaired tree. The earlier XY-C1 run had entered the actual Build step; no L3 claim until Build, TRX and artifact upload all pass on one SHA.

---

## 2026-11-20 — Confirm Ralink 802.1X child readiness before reporting launch success

- Parent main SHA: dc43d3d8fcfeaf8e730ec6f6d77170bbcd056d91.
- Source evidence: `rtdot1x.c` forked its authenticator worker and returned zero from the parent immediately. Shared `eval()` waits only for that parent, so it reported success even if the detached child failed allocation, `Apd_init()` or `Apd_setup_interface()`. Driver PID ioctl errors were also ignored.
- Added a bounded pipe readiness handshake: the worker reports ready only after AP setup and successful driver PID registration; failure paths clean partial state and send a failure byte. Parent polls for 15 seconds, treats EOF/timeout as failure, and kills/reaps a worker that never reports.
- OOM cleanup now guards a NULL authenticator array and calls `eloop_destroy()` only after `eloop_init()`.
- Added injectable pipe I/O and GNU89 regression tests for real pipe success, child failure byte, EOF, I/O errors, bounded EINTR retries and invalid arguments. Process/XY trigger filters now include production source, helper and regression test.
- Exact-head Process and same-SHA XY-C1 Build/TRX/artifact are pending; physical-router L4 NOT VERIFIED.

---

## 2026-11-20 — Fix detect_link signal-policy source-contract variable shadowing

- Parent main SHA: 843aa5e68b62b62d6f3354de42dc487565bd0910.
- The exact-head NVRAM mirror regression now compiles and passes under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. The first fixture was corrected to begin from the opposite cached WAN status so it exercises a real changed-state write and retry.
- Exact detect-link run `44726853240` on `982d559c0a535125f8b118a4d2db15dc25b82d11` then failed in the Python source contract: the added NVRAM policy file reused the existing `policy` variable, so subsequent `DL_PENDING_*` assertions accidentally inspected the NVRAM header instead of the signal-event header.
- Rename that variable to `nvram_policy`; preserve the original signal-event assertions and the independent NVRAM publication assertion. The adjacent exact-head run `44726853239` was still running when checked. The host-gate workflow reached the NVRAM policy step and skipped the production-input detector/build because that earlier compile step failed, so L3 remains NOT VERIFIED.
- THIS COMMIT: checker repair only; exact-head detect-link and AI-docs validation pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Align WAN route and modem contracts with checked identity pairs

- Parent main SHA: 786fb0ab4f2ffbec808eb7c67279c6959b5db2db.
- Exact WAN route run `44725040341` failed because the contract still counted three direct `ifname_t` setters after the callsites were moved into `rc_wan_nvram_publish_runtime_identity()`. Updated it to require exactly three pair publishers and preserve immediate failure aggregation, physical-WAN recovery and pre-firewall abort.
- Exact Process fail-closed run `44725040303` failed because `check_modem_wan_start_status.py` still located the old unchecked `proto_t` setter and separate `ifname_t` setter. Updated its ordered call markers and retained the two-error first-failure aggregation check.
- On the same exact SHA, WAN DHCP `44725040281`, AI docs `44725040258`, and Process lifecycle inventory `44725040116` passed. XY-C1 `44725040370` was in progress at the last poll.
- Checker-only change; exact-head reruns and same-SHA L3 remain pending.

---

## 2026-11-20 — Correct WAN identity regression literal and roadmap dashboard heading

- Parent main SHA: 1ba05070a5e33103c62bca466f70531b8fe2fb6e.
- WAN route `44724624287`, WAN DHCP `44724624228` and Process fail-closed `44724624179` on this exact SHA stopped because the new test compared against a two-backslash multi-character constant under `-Werror=multichar`. Replaced it with the single-backslash C null character constant; the test still verifies that the second write was not attempted after protocol publication fails.
- AI docs `44724623863` failed because the active roadmap checkpoint update accidentally removed the required `## Whole-repository module status` dashboard heading. Restored the heading ahead of the unchanged module inventory.
- Process lifecycle inventory `44724624195` passed. XY-C1 `44724624114` was still in progress at the last poll. This commit requires fresh exact-SHA results; no L3/L4 claim.

---

## 2026-11-20 — Restore omitted parent-tree integration blobs

- Parent main SHA: 818ff7b555c5620e39a3fde1fc7d2715beab2c8b.
- Post-commit tree review found that the preceding runtime-identity commit used the 6bc tree as its base while parenting its commit to 9230. Three 9230 changes were consequently absent from its resulting tree: the PID 1 status-accessor integration in `rc.c`, its declaration in `rc.h`, and the corresponding Process source contract.
- This child restores those exact three blob versions from 9230 on top of the runtime-identity tree. It preserves the new NVRAM policy, regression, WAN callers, checker and docs; it does not rewrite history or force-update main.
- The previous commit's three exact Actions were queued (WAN DHCP callback `44724488527`, iNIC `44724488648`, AI docs `44724488434`). The repair child requires its own exact-SHA verification. No L3/L4 claim.

---

## 2026-11-20 — Check WAN runtime protocol/interface identity writes

- Parent main SHA: 9230dacb50d48f003649dfb06e15a3e63d99f15f.
- Found three unchecked `set_wan_unit_value(..., "proto_t", ...)` writes in `start_wan()`: USB RAS, ordinary PPP and IPoE. Each occurred immediately before the checked `ifname_t` publication, so a failed protocol marker write could leave stale runtime metadata while startup advanced.
- Added `rc_wan_nvram_publish_runtime_identity()` as a checked, ordered pair publisher and replaced all three callsites. The caller retains the existing fail-closed physical-WAN recovery when either write fails.
- Added policy coverage for successful publication, failure of the first or second write, and invalid arguments; extended source contracts to require all three branches and reject unchecked `proto_t` writes.
- Fixed the stale WAN-reset contract in the same checker: the legacy void ABI is retained and the checked reset implementation remains private; PID 1 uses the public read-only status accessor.
- Exact-head WAN-route, Process, AI-docs and XY-C1 verification for this slice is pending at commit creation. No L3/L4 claim.

---

## 2026-11-20 — Preserve WAN reset ABI while exposing PID 1 result

- Parent main SHA: 6bc3495b402f8ad4818ce9ecbc58bfc57f34ae7a.
- Exact XY-C1 run `44723038042` for `72c299981fc7aeeb404c21232f38b8def41ff1db` stopped before Build at the WAN firewall contract: exporting `reset_wan_vars_status()` violated the established public void `reset_wan_vars()` ABI and its intentionally private implementation.
- Exact Process `44723038026` failed on a malformed regex; the escaping correction in `6bc3495b` was verified by exact Process `44723163334` and AI docs `44723163377`, both PASS.
- Restored private linkage for `reset_wan_vars_status()`, kept the public void wrapper, and added the narrow read-only `get_wan_vars_reset_status()` accessor. PID 1 now calls the legacy wrapper and reads the failure latch; the internal helper is not exported.
- Process and XY contracts now pin the ABI-safe design. Exact Actions and same-SHA MIPS Build/TRX/artifact for this production-source correction are pending; hardware L4 remains NOT VERIFIED.

---

## 2026-11-20 — Make WAN reset linkage regression assertion effective

- Parent main SHA: 72c299981fc7aeeb404c21232f38b8def41ff1db.
- Corrected an escaping mistake in the Process integration checker: the static-linkage regex previously searched for literal backslash characters rather than Python whitespace classes, so it could not detect a regression to `static int reset_wan_vars_status()`.
- The assertion now uses a real whitespace regex and continues to require the public declaration plus the implementation body. This is a test-only correction; production source is unchanged.
- Exact Process/XY-C1/AI Actions for parent `72c29998` were still in progress/queued at last poll; this test-only commit's exact Actions are pending.

---

## 2026-11-20 — Export checked WAN reset status to the PID 1 caller

- Parent main SHA: 99054c2e152aa28bde06fa1790abb46294038cae.
- Exact XY-C1 run `44721008402` for `f196902a3768f90c6cb60a0a40abe978661396f5` failed at the final RC link. All preceding compile steps passed, but `rc.c` had an undefined reference to `reset_wan_vars_status`: the implementation in `net_wan.c` was still declared `static`, so the new PID 1 boot gate could not link across translation units. TRX verification and artifact upload were skipped; this SHA is NOT L3 verified.
- Removed internal linkage from the checked WAN reset function and declared the status API in `rc.h`. The Process source contract now rejects a static definition and requires the public prototype.
- The LAN checked MAC/default/runtime boot gate from `99054c2e` remains in this parent; this separate atomic commit repairs the prerequisite WAN symbol linkage so the production image can link.
- At last poll, `99054c2e` had 39 exact push Actions queued/in progress (Process `44722781752` running; XY-C1 `44722781476` running). The current fix SHA's exact Actions and same-SHA MIPS Build/TRX/artifact checks are pending. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-20 — Fail closed on LAN startup NVRAM publication

- Parent main SHA: f196902a3768f90c6cb60a0a40abe978661396f5.
- `reset_lan_temp()` silently ignored failures while defaulting persistent LAN IP/netmask and publishing the temporary LAN IP, netmask, gateway, domain and DNS snapshot. `reset_lan_vars()` also ignored failure to publish `lan_hwaddr`.
- Added checked NVRAM read/write callbacks and status-returning LAN reset operations. The IP/netmask and hardware MAC are copied from checked NVRAM reads into bounded stable buffers before writes; the five runtime fields are published in fail-fast order. Legacy void wrappers remain for existing callers.
- `nvram_convert_misc_values()` now returns a LAN MAC/default/runtime write failure before timezone setup and before PID 1 can enter GPIO/service startup. Added a five-field failure-injection regression and Process integration assertions.
- L1 source integration and GNU89 host regression are delegated to exact-head Process Actions run. The parent SHA `f196902a` had 35/36 push workflows PASS; its exact XY-C1 Build/TRX/artifact run `44721008402` was pending. The previous checked-read source SHA `abc46fda` XY-C1 run `44720306486` subsequently completed SUCCESS.
- Product source changed. This commit's exact Actions and same-SHA MIPS build/TRX/artifact verification are pending; hardware L4 remains NOT VERIFIED.

---

## 2026-11-20 — Propagate WAN reset failure through router boot

- Parent main SHA: 6ebd91e9e14ab120de4e1f9db70e7cdda2d47d17.
- nvram_convert_misc_values() was already checking WAN0 defaults/runtime writes, but then called legacy void reset_wan_vars(), which discarded the error returned by reset_wan_vars_status(). That function checks a required sequence covering WAN protocol, DHCP/DNS options, authentication, routes, address state, PPP settings, hardware address and IPv6 runtime fields.
- Replaced the void call with reset_wan_vars_status(), logs the first failure and returns it through nvram_convert_misc_values(). init_router() already stops on nonzero initialization status before GPIO/service startup.
- Extended the Process lifecycle source contract to require the checked reset status gate, ensure it precedes LAN reset and reject use of the legacy void wrapper in this boot path.
- Preceding product-source 05e2cb L3 is now verified: XY-C1 run 44718942242 passed Build, TRX and artifact upload (artifact 14621980301, 12,690,602 bytes). Exact 6ebd Process 44720687519 and AI docs 44720687419 PASS; abc46fda's 7-case policy tests passed but its checker initially failed from undefined variable, fixed by 6ebd. abc46fda XY run 44720306486 pending at last poll.
- This commit changes rc.c production source; exact-head Process and XY-C1 checks are pending.

---

## 2026-11-20 — Repair Process checker variable for WAN0 checked-read policy

- Parent main SHA: abc46fda36f6f11479374fe123de35bf84d51cb0.
- The new checked-read policy's seven-case GNU89/ASan/UBSan regression compiled and ran successfully in exact Process run 44720306205. The final integration checker failed because its new assertion referenced policy_header, a local name not defined in that script section.
- Added an explicit wan_nvram_policy_header read next to the WAN0 test source and pointed the assertion at it. This is a checker-only repair; no runtime C or regression code changed.
- Exact AI docs run 44720306191 PASS. Product-source XY-C1 run 44720306486 and NVRAM transaction run 44720306517 were IN_PROGRESS at last poll. No new L3 evidence claimed.

---

## 2026-11-20 — Distinguish absent WAN0 defaults from NVRAM read failures

- Parent main SHA: 36519699b833ac0582b7f54f3d9d52b80bfb7245.
- The fail-closed WAN0 initializer still used legacy nvram_get() for the target override and shared default key. That wrapper maps backend/read/close errors to NULL, making an I/O failure indistinguishable from an absent key and potentially replacing a configured WAN0 override with a factory default.
- Confirmed the checked API tri-state contract and that the kernel single-key snapshot reads the live tuple table (including temporary values). The returned pointer is shared cache storage, so the new getter immediately duplicates it before any subsequent NVRAM call.
- Added rc_wan_nvram_copy_missing_default(): it preserves an existing empty target override, checks source only when target is absent, uses router_defaults fallback only when the shared source key is genuinely absent, and propagates all read/write errors.
- set_wan0_vars() now uses nvram_get_checked() owned-value callbacks; legacy nvram_get() calls are removed from this initialization path. Failures propagate through the existing fatal PID1 gate.
- Added seven injected cases covering empty override, normal source copy, missing-source fallback, target/source read errors, setter error and invalid arguments. The existing Process workflow compiles/runs this policy under GNU89, -Werror, ASan and UBSan.
- Parent CI: exact 365 Process 44719246958, HTTPD 44719247010 and AI docs 44719246942 PASS. Product-source 05e2 XY-C1 run 44718942242 remained IN_PROGRESS at last poll; no L3 claim. This commit's exact-head Actions pending at creation.

---

## 2026-11-20 — Update PID1 fatal-gate CI contracts for WAN0 NVRAM failure

- Parent main SHA: 05e2cb3822c1c4f8ddb70bf7af8a28376fb48d45.
- The new WAN0 fail-closed initializer added a fifth intentional INIT_ROUTER_RESULT_FATAL exit before GPIO/service initialization. Exact-parent Process lifecycle run 44718942446 and HTTPD security run 44718942454 both failed because independent checks in scripts/check_dev_init_boot.py and the HTTPD workflow inline script hardcoded the previous count of four.
- Updated both checks to expect five fatal exits and explicitly assert the WAN0 setup failure gate returns fatal before init_gpio_leds_buttons(). This corrects the verification contract; the implementation change itself was already committed and the earlier NVRAM test steps passed.
- Rerun of Process/HTTPD on this CI-only correction and exact-source XY-C1 status are pending; no L3 claim is made for this workflow/checker commit.

---

## 2026-11-20 — Fail closed when WAN0 boot NVRAM initialization fails

- Parent main SHA: f2bae37918b42648fd4d8ff6be95fdc372bf1472.
- set_wan0_vars() copied missing router_defaults WAN keys and wrote the unit/primary/ifname fields with unchecked NVRAM setters. The enclosing nvram_convert_misc_values() returned void, and init_router() continued despite an incomplete WAN0 configuration/runtime identity tuple.
- Checked each WAN default copy with rc_wan_nvram_write_status(). Migrated unit, primary, ifname, uptime, dltime, bytes_rx, bytes_tx and ifname_t to the existing required-value policy with a status-returning callback.
- nvram_convert_misc_values() now returns the first WAN0 setup error, and init_router() stops with INIT_ROUTER_RESULT_FATAL before GPIO setup, NVRAM commit and service startup.
- Added an injected regression that fails each of the eight WAN0 required writes in turn and verifies later writes do not run. Added Process workflow path filters, ASan/UBSan compile/run, and a source contract for the checked copy and PID1 fatal gate.
- f2bae379 exact Custom firmware defaults regression 44718064716 and AI docs run 44718065095 both PASS. This commit's Process/XY-C1 Actions and same-SHA L3 are pending at creation; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Escape C trigraphs in customized firmware defaults

- Parent main SHA: b8828568389b344795a761d923f1f09ff70147bc.
- The custom-build workflow accepts printable ASCII root passwords/Wi-Fi PSKs and serializes them into C string literals. c_string() escaped quotes and backslashes but did not escape question marks. A credential containing a C trigraph such as ??/ makes GCC emit -Wtrigraphs diagnostics with the generated source line, potentially exposing the credential in build logs and altering preprocessing on toolchains that enable trigraph conversion.
- Updated c_string() to encode each question mark as the standard C escape \?. Added a regression for all nine C trigraphs and compiled the resulting literal with GNU89 -Wall -Wextra -Werror.
- The prior public-release customization leak is closed on parent b8828568389b344795a761d923f1f09ff70147bc: Custom firmware defaults transaction run 44717413779 PASS; AI handoff docs run 44717413917 PASS. Product source f0c20ff has same-SHA XY-C1 Build/TRX/artifact PASS (run 44715584790, artifact 14620685104).
- This is build-time serialization only. Exact-head regression and AI docs Actions for this commit are pending; no new firmware L3/L4 claim applies.

---

## 2026-11-20 — Redact custom firmware credentials from public release notes

- Parent main SHA: `f0c20ff9df0c81c70f9ac0fd767aa4881a00eec9`.
- Found that `.github/workflows/custom.yml` expanded the entire `inputs.customization` JSON into the public GitHub Release body. The JSON accepts `signpassword` and Wi-Fi PSKs, so a custom firmware Release could disclose root and wireless credentials.
- Replaced the raw value with a non-sensitive “customization applied” marker. Added a Python regression that reads the actual `softprops/action-gh-release` block and asserts the raw JSON and password/PSK fields never appear there.
- Added `.github/workflows/custom.yml` to both path filters in `.github/workflows/customization-transaction.yml`, ensuring the release-note security regression runs on future workflow edits.
- This workflow/test-only slice does not change production firmware inputs. Exact-head customization workflow result is pending at commit creation; no new XY-C1 L3 result is applicable.

---

## 2026-11-20 — Publish DHCP 6rd lease options as a compensatable tuple

- Parent main SHA: `00cc18c9d51a9a8254d44366795c06140bee0b17`. `store_ip6rd_from_dhcp()` ignored three independent `nvram_set_temp()` errors for `addr6`, `6rd_size` and `6rd_relay`. An ioctl/close failure after an applied write could leave a mixed 6rd tuple, while WAN DHCP callback continued to route/start services using the partially updated state.
- Add `store_ip6rd_from_dhcp_status()` and keep the legacy void wrapper. Read and copy all three prior values under one NVRAM transaction lock, then use existing `dhcp_lease_nvram_publish()` to attempt/compensate the entire tuple in reverse order. If write or transaction-close fails, retry restoring all prior values under a fresh lock and return the original error; log an incomplete rollback. Invalid/absent optional DHCP option data remains the existing no-op behavior.
- The WAN DHCP bound callback now treats a 6rd publication failure as a snapshot publication failure and enters its existing DHCP address/route rollback path before WAN activation. Added a 6rd-specific regression case simulating a setter error after the write was applied and verifying complete old-tuple restoration. The existing callback/XY workflows already watch all changed files.
- Exact parent-SHA `00cc18c9d51a9a8254d44366795c06140bee0b17` had 40 successful Actions, including WAN callback `44714046971`, WAN route `44714046939`, AI handoff `44714047367`, NVRAM boundary `44714046370`, Process inventory `44714046948` and fail-closed `44714046997`. XY-C1 run `44714046405` was still IN_PROGRESS at last check; it is not yet L3 evidence.
- Previous source SHA `e8ef10b371372e7bfe17d552ea44150258296743` did complete XY-C1 Build, TRX verification and artifact upload in run `44712394978` / job `136207077488` (artifact `14618531515`). That L3 evidence does not apply to this new 6rd slice. Current slice exact-head Actions will be checked after commit.

---

## 2026-11-20 — Propagate failures while resetting the IPv6 WAN runtime snapshot

- Parent main SHA: `e8ef10b371372e7bfe17d552ea44150258296743`. A concrete failure path was found in `reset_wan_vars_status()`: it called the legacy void `reset_wan6_vars()` and then returned success, so failed IPv6 NVRAM writes could be silently followed by WAN activation. `build_dns6_var()` also discarded the DNS setter result. The reset code retained cache-backed `nvram_safe_get()` pointers across subsequent reads/writes, risking a mixed or stale gateway/DNS snapshot on cache invalidation.
- Add `reset_wan6_vars_status()` while preserving the legacy void API. Bounded-copy all address, gateway, 6rd and DNS values before the first write; publish them through the existing fail-fast NVRAM value policy; clear `ifname6` last so a failed configuration write does not discard the previous teardown/recovery handle. The WAN reset transaction now aborts on any IPv6 staging error, and `wan6_up()` blocks activation if configured DNS publication fails.
- Added policy tests proving that a DNS write failure stops before `ifname6` is cleared and that the identity is cleared only after all reset values succeed. Extended the IPv6 source contract and callback path filters for `net_wan.c`, `rc.h`, and the NVRAM status-policy header/test.
- Parent exact-SHA `e8ef10b371372e7bfe17d552ea44150258296743` results: WAN route `44712395177`, WAN DHCP callback `44712394929`, AI handoff `44712394830`, NVRAM transaction `44712394796`, process lifecycle `44712394846`, process fail-closed `44712394821`, HTTPD security `44712394800`, firewall startup `44712394853`, and USB modem identity `44712394963` all PASS. XY-C1 firmware run `44712394978` / job `136207077488` also PASS on this exact source SHA: `make XY-C1 TOOLCHAIN=mipsel-linux-musl`, non-empty TRX/file verification, and artifact upload all completed; artifact ID `14618531515`, SHA256 `b3ab4ead7771c828dc678ed70ad173ea4f0671c07fb5dab4aae5ddbb10b5b9b6`. This L3 evidence applies only to e8ef10b, not the reset slice below.
- This reset slice's tests/build have not yet run on its own SHA. L3 remains NOT VERIFIED until an exact-production-SHA MIPS build, TRX verification and artifact upload all succeed; physical hardware L4 remains NOT VERIFIED.

---

## 2026-11-20 — Retain native IPv6 identity until rollback is complete and repair exact-head contracts

- Parent main SHA: `7c48e205c192eb093c735ddc4be69bc1453e3054`. A source-path review found native `wan6_up()` cleared `ifname6` immediately after a failed checked identity publication, before router-advertisement, autoconfiguration, privacy, route, neighbour and address rollback ran. Since `rollback_status` was still zero at that point, a later cleanup failure could strand kernel state without the interface identity needed to tear it down.
- Move identity clearing after every independent rollback attempt. The new `wan6_clear_native_identity_after_rollback()` retains the recovery handle on any rollback error and clears it only when rollback fully succeeds; the primary startup failure remains the returned status. Add GNU89 regressions for incomplete rollback, successful clear and clear-write failure.
- Repair exact-head stale CI contracts: DHCP callback checks now use checked WAN IPv6 metadata helpers and the tenth teardown status aggregate; the XY-C1 modem checker requires the same ten aggregates and enforces native rollback-before-identity-clear ordering; XY-C1 path filters include the identity policy/test; the post-WAN hook checker now anchors the actual source comment while retaining the existing execution/status/order assertions.
- Parent SHA exact Actions `03041282c97b0a4d36e53ceefc8d2dc1e91fd2c3`: HTTPD `44711346041` PASS, Process `44711345873` PASS, NVRAM boundary `44711345856` PASS, AI docs `44711345981` PASS. WAN callback `44711345959` failed at stale IPv6 teardown/identity text, WAN route `44711345905` failed at the renamed post-WAN hook comment (`check_wan_detect_wan_status.py:102`), XY-C1 `44711345889` failed at the obsolete nine-aggregate check (`check_modem_wan_start_status.py:648`) before MIPS compilation. Earlier route run `44710438005` on ancestor `c0c2ff5` passed; it does not cover the later DNS source slice.
- No current-slice test or firmware result is claimed before the new exact-SHA Actions complete. L3 firmware and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-20 — Handoff checkpoint at `03041282c97b0a4d36e53ceefc8d2dc1e91fd2c3`

- Reconciled current main and the frozen backup branch; updated `SESSION_HANDOFF.md` with the exact SHAs and current Actions state.
- Latest main runs:
```
44711346041 HTTPD Authentication and Security Regression: completed/success (03041282c97b)
44711345873 Process lifecycle inventory - 03041282c97b0a4d36e53ceefc8d2dc1e91fd2c3: completed/success (03041282c97b)
44711345905 WAN route operation status regression: completed/failure (03041282c97b)
44711345981 AI handoff documentation contract: completed/success (03041282c97b)
44711345959 WAN DHCP callback state stability regression: completed/failure (03041282c97b)
44711345889 XY-C1 Firmware Validation: completed/failure (03041282c97b)
44711345856 NVRAM transaction boundary audit: completed/success (03041282c97b)
44711193886 HTTPD Authentication and Security Regression: completed/success (2a6b5b3f0f77)
44711193581 USB modem WDM/NDIS physical identity stability: completed/failure (2a6b5b3f0f77)
44711193928 watchdog signal lifecycle stability regression: completed/success (2a6b5b3f0f77)
44711193723 WAN interface teardown stability regression: completed/success (2a6b5b3f0f77)
44711193691 ZeroTier lifecycle stability regression: completed/success (2a6b5b3f0f77)
44711193566 detect_internet signal lifecycle stability regression: completed/success (2a6b5b3f0f77)
44711193811 VPN server lifecycle stability regression: completed/success (2a6b5b3f0f77)
44711193706 miniupnpd atomic configuration publication regression: completed/success (2a6b5b3f0f77)
44711193792 detect_link signal lifecycle stability regression: completed/success (2a6b5b3f0f77)
```
- DNS source SHA `03041282c97b0a4d36e53ceefc8d2dc1e91fd2c3` runs:
```
44711346041 HTTPD Authentication and Security Regression: completed/success (03041282c97b)
44711345873 Process lifecycle inventory - 03041282c97b0a4d36e53ceefc8d2dc1e91fd2c3: completed/success (03041282c97b)
44711345905 WAN route operation status regression: completed/failure (03041282c97b)
44711345981 AI handoff documentation contract: completed/success (03041282c97b)
44711345959 WAN DHCP callback state stability regression: completed/failure (03041282c97b)
44711345889 XY-C1 Firmware Validation: completed/failure (03041282c97b)
44711345856 NVRAM transaction boundary audit: completed/success (03041282c97b)
```
- Earlier IPv6 identity SHA `5e03588c33965328b8c33880504413a915cc47de` runs:
```
44709969998 XY-C1 Firmware Validation: completed/failure (5e03588c3396)
44709970019 NVRAM transaction boundary audit: completed/success (5e03588c3396)
44709970196 WAN DHCP callback state stability regression: completed/failure (5e03588c3396)
44709970368 HTTPD Authentication and Security Regression: completed/success (5e03588c3396)
44709970075 AI handoff documentation contract: completed/success (5e03588c3396)
44709970223 Process lifecycle inventory - 5e03588c33965328b8c33880504413a915cc47de: completed/success (5e03588c3396)
44709970108 WAN route operation status regression: completed/failure (5e03588c3396)
```
- No L3 firmware or L4 hardware claim is made unless exact build/TRX/artifact or physical-device evidence is present in the corresponding run.

---

## 2026-11-20 — Propagate DHCPv6 DNS metadata publication failures

- Parent main SHA: `2a6b5b3f0f77720aee236eaea17d63cd0a6ee568`. Found another unchecked critical NVRAM write in `store_wan_dns6()`: it returned `1` after calling the legacy void setter, even if NVRAM rejected a new DNS list or DHCPv6 DNS withdrawal. The callback could therefore report success while resolver metadata remained stale.
- Added `wan6_dns_store_if_changed()`: it preserves the no-change path, reports a changed list only after the checked setter succeeds, and maps setter failures to negative errno-style results. The production callback sets `errno`, logs and returns `-1` on failure.
- Added five GNU89 policy cases for successful changed-value publication, no-op unchanged values, withdrawal, NVRAM failure propagation and invalid writer. Extended the IPv6 semantic checker; its existing workflow already compiles this policy binary with ASan/UBSan.
- This commit does not change DHCPv6 wire behavior or the existing DNS option parser; it only makes resolver-state publication fail closed.

---

## 2026-11-20 — Snapshot WAN IPv4 metadata before NVRAM cache invalidation

- Parent main SHA: `c0c2ff55628dab5fdceb78695202c58a1a044bfe`. The WAN-up callback fetched `ipaddr`, `netmask`, and `gateway` as cache-backed pointers; the subsequent checked `wanx_gateway` write could invalidate the shared NVRAM cache, but the optional post-WAN hook still dereferenced the old `wan_addr` pointer at function end. The three sequential reads also retained pointers across later NVRAM lookups.
- Added a bounded alias-safe text-copy policy and regression for cache invalidation, exact IPv4-length values, missing values, overflow rejection, invalid destinations, and source/destination aliasing. WAN-up now copies each IPv4 field immediately after its getter, before the next read; malformed values are never truncated, invalid gateway snapshots cannot create gateway-dependent routes, and the post-WAN hook is skipped if its address snapshot is invalid.
- Extended the WAN route source checker and duplicated embedded workflow contract. Existing WAN Actions compile/run the policy regression under GNU89, `-Werror`, ASan and UBSan. No MIPS firmware or router result is claimed yet.

---

## 2026-11-20 — Align WAN route workflow's IPv6 teardown contract with checked metadata writes

- Parent main SHA: `6a5f2be3e590ecca6712a60b61612bf82e537b5d`. WAN route operation-status run `44709970108` passed its GNU89/ASan/UBSan policy executables and reached the embedded source audit, then failed because it still required direct unchecked `set_wan_unit_value(..., "dns6"/"ifname6", "")` writes in `wan6_down()`.
- The new code intentionally replaced those legacy writes with `wan6_clear_runtime_metadata()`, which checks both writes, attempts the second even if the first fails, and returns the first error. Updated the duplicate workflow contract and its path audit to require the new identity policy header/test. No production source assertion was weakened.
- The exact-head WAN DHCP callback source-check rerun after the previous assertion correction is still pending; the XY-C1 run on the original product-source SHA also remains under observation.

---

## 2026-11-20 — Correct IPv6 native rollback source-check ordering

- Parent main SHA: `5e03588c33965328b8c33880504413a915cc47de`. The exact-head WAN DHCP callback workflow `44709970196` passed its pre-existing C policy binaries and stopped in the new IPv6 source checker because the assertion searched for `rollback_status == 0` after the `wan6_clear_runtime_metadata()` call, while production correctly tests the rollback status before entering that call.
- Correct the checker to assert the actual order: DHCPv6 stop gate, rollback-status gate, then metadata clear. This changes no source behavior and does not loosen the retained-identity-on-incomplete-rollback contract.
- The XY-C1 firmware build `44709969998` was still in progress at the last poll; exact TRX/artifact results remain pending.

---

## 2026-11-20 — Make IPv6 WAN interface identity publication fail closed

- Parent main SHA: `47e66a0620ab143b5f5751f5de440d377bb12a00`. In `wan6_up()`, SIT and native IPv6 paths wrote `ifname6` through the legacy void NVRAM setter and then reported success without checking whether the runtime interface identity was committed. A failed write could leave an active IPv6 configuration without a teardown handle.
- SIT startup now uses a tested identity transaction: if the write fails, it removes the new tunnel; after successful removal it clears any partially published identity. If tunnel removal fails, it retains metadata and reports rollback failure separately while preserving the original write error.
- Native IPv6 startup uses the checked setter. A failed identity commit enters the existing failure/rollback path; after DHCPv6 is confirmed stopped, the attempted identity is cleared only when kernel/address/route rollback succeeded. If rollback is incomplete, the interface identity is retained for later teardown/recovery. The existing DHCPv6 stop safety gate remains in force.
- Both IPv6 teardown branches now check metadata writes. The vanished-interface path attempts both `dns6` and `ifname6` clears and returns the first error. Normal teardown does the same only after network cleanup succeeds; metadata-write failures are returned instead of reporting a successful stop.
- Added a testable identity policy plus seven GNU89/ASan/UBSan cases for successful SIT identity commit, tunnel rollback, rollback failure, metadata-clear failure, first-error aggregation, selective DNS retention and invalid arguments. Extended the checker and WAN DHCP callback workflow. XY-C1 watches `net_wan6.c`; its exact-head MIPS/TRX/artifact run will provide the target verification.

---

## 2026-11-20 — Fix PID 1 optional-device helper declaration after XY-C1 compile caught it

- Parent main SHA: `03f7d64c53a8156c563fcd9c5eba4d118aaa4b2b`. Process lifecycle run `44708880390` passed its exact-source host regressions; PID 1 mdev security `44709299228` and HTTPD security `44709436383` passed their checker follow-ups.
- The XY-C1 source build `44708879934` reached the RC target but failed compiling `init.c:536` with duplicate `static`. The function-body replacement had left the original `static int` prefix immediately before the new feature guard; the source contract and host binaries could not detect this invalid declaration boundary.
- Removed the orphaned qualifier so the conditional helper declaration starts at file scope. This is a production source correction; exact-head MIPS build, TRX and artifact verification are required again. The failed build did not produce a valid firmware artifact.

---

## 2026-11-20 — Fix HTTPD umbrella source contract for optional boot-node initialization

- Parent main SHA: `d8f3b8b81ae678958bb108a5774c52800ca0973e`. Process lifecycle validation passed on `75b3d0097558e685cf243047a8d77df77ee93f3d`; the PID 1 mdev checker follow-up is queued separately.
- The HTTPD umbrella run `44708880123` on `75b3d0097558e685cf243047a8d77df77ee93f3d` completed its C regression and the preceding NVRAM checks, then stopped at the RC NVRAM restore source contract because its local extractor still requested `init_nodes(void)`. The production function now accepts an optional-status output pointer; NVRAM's checked result and fatal boot gate are unchanged.
- Updated the one stale signature expectation. No production behavior changed; the exact-head HTTPD rerun is pending.

---

## 2026-11-20 — Align PID 1 device-node security regression with optional boot-node validation

- Parent main SHA: `75b3d0097558e685cf243047a8d77df77ee93f3d`. The Process lifecycle suite on `75b3d0097558e685cf243047a8d77df77ee93f3d` passed, including the new GNU89/ASan/UBSan optional device-node policy regression and the revised PID 1 boot source checker.
- The dedicated PID 1 mdev security run `44708880007` compiled and passed its three existing C policy tests, then failed in the inline source checker because it still searched for the old `init_nodes(void)` signature and the old standalone OpenVPN failure log. This was a stale test contract, not a C regression.
- Updated the dedicated checker to validate `init_nodes(int *optional_status)`, the exact-device `lstat()`/identity gate, first optional error retention and boot-degraded handoff. Added the optional boot-node test to this workflow's push/PR filters and GNU89/ASan/UBSan run. The follow-up exact-head run is pending.

---

## 2026-11-20 — Validate optional PID 1 device nodes and propagate boot degradation

- Parent main SHA: `462e3c811f4abd68583af11df51fbf66ebb8971e`. In `init_nodes()`, the `/dev/hwnat0` and `/dev/crypto` `mknod()` return values were discarded, and `EEXIST` was accepted without checking that the path was the expected character device. The OpenVPN TUN failure was logged but not merged into the PID 1 READY/DEGRADED decision.
- Added a shared boot-device identity policy and regression for character type, major/minor, root ownership, link count and safe permissions. Creation errors are retained; an existing path is accepted only after `lstat()` and full identity validation. Optional HW-NAT, MTK crypto and OpenVPN TUN failures now keep the first error and mark router startup DEGRADED; the separate NVRAM-device prerequisite remains fatal.
- Added the policy binary to the exact-head Process lifecycle workflow and extended the PID 1 boot source contract. The existing XY-C1 workflow already watches `init.c`, so this production edit also triggers firmware validation. Exact-head Process and XY-C1 Actions are pending after this source commit; no local firmware build or router test is claimed.

---

## 2026-11-20 — Fix NDIS identity checker formatting anchor and strict-C test include

- Parent main SHA: `a033d8e862681f0e90e29e6a3ac4364ba32ae995`. The USB modem exact-head run `44708363419` compiled and passed all five pre-existing GNU89/ASan/UBSan C regressions, passed the WDM/NDIS semantic workflow block and module-reload checker, then failed at one new checker assertion that expected no whitespace after a multiline function's opening parenthesis. The generated source is correct; the structural check was too formatting-sensitive.
- The new four NDIS identity regression functions also use `memset()`/`strcmp()`; add their explicit `<string.h>` declaration to keep the `-Werror` GNU89 build portable.
- This follow-up only adjusts the source checker and required standard header; rerun the exact-head USB workflow before treating the NDIS slice as verified.

---

## 2026-11-20 — Fail closed when NDIS WAN identity cannot be published

- Parent main SHA: `c26207d9cb5a6aa93169ff207799e5bdeca51c4f`. Found a real state-integrity gap in `launch_wan_usbnet()`: the NDIS protocol label and active `ifname_t` were written through legacy void setters, their NVRAM errors were discarded, and interface/firewall/control/DHCP startup proceeded as if the active WAN identity had been committed. Error-flag writes on migration, IFUP, control-session and DHCP branches were also unchecked.
- Exposed the existing checked WAN NVRAM setters/error-state publisher through `rc.h` and made string-key/value validation explicit. Added a GNU89 policy that publishes `proto_t` and `ifname_t` before IFUP; any failed write triggers reverse-order best-effort clearing of both fields while preserving the primary publication error and reporting rollback status separately.
- NDIS startup now aborts before IFUP, firewall, control or DHCP when identity publication fails. Migration/IFUP/control/DHCP error-state writes use the checked publisher, and the operation's first failure remains authoritative while a failed error-state update is still surfaced when no earlier failure exists.
- Added four regression cases for successful identity publication order, second-write failure with rollback failure, first-write failure cleanup and invalid arguments. Updated the USB modem semantic checker to pin the fail-closed order, checked error reporting, exported checked setter ABI and rollback tests. No ABI changes to legacy void APIs; new checked entry points are additive.
- No local checkout/compiler is available. Exact-head GNU89/ASan/UBSan USB modem workflow and XY-C1 MIPS/TRX/artifact pipeline are required to verify this source change.

---

## 2026-11-20 — Align shared WAN route-flush source contracts with recovery behavior

- Parent main SHA: `a97cbb5382eb3675eddfff39ecc58e244fa44866`. The exact-head WAN route regression for `a97cbb5` passed all GNU89/ASan/UBSan policy binaries and the updated firewall-specific semantic checker.
- Four unrelated suites (USB modem, process lifecycle, HTTPD security, and firewall startup) each stopped in the shared `scripts/check_modem_wan_start_status.py` at its obsolete assertion `return route_flush_status;`. The new production path correctly runs LAN route recovery and returns the preserved first error via `status`. HTTPD security has a duplicate inline assertion with the same stale expectation.
- Updated both contracts to require route-recovery callback ordering, secondary failure logging, and the registered ENOSPC regression. The failing test binaries and earlier checker steps had passed; no production code change is made in this checker-alignment commit.
- Exact-head results on `a97cbb5`: WAN route `44706848289` PASS; AI docs `44706848364` PASS; the four suites above FAIL only at the duplicated stale checker expectation; XY-C1 `44706848129` and NVRAM boundary `44706848481` were still in progress at last poll.

---

## 2026-11-20 — Recover LAN routes after partial interface/global route-flush failure

- Parent main SHA: `793bc85f44e75e64ad4ff74d3b53e6b4bc4e8afb`. Follow-up inspection of `clear_if_route4()` confirmed that `ip route flush dev IF scope global` is an independently failing command and the shared helper deliberately attempts both MAN and bridge flushes plus the global cache flush. Any of those commands can report failure after another command already removed bridge static routes.
- `full_restart_wan()` previously returned immediately on the aggregate flush status, leaving no attempt to restore the static LAN routes. This can strand LAN reachability even though WAN restart correctly aborts. The recovery now uses the generic first-error helper before return; `add_static_lan_routes()` uses idempotent EEXIST handling, so retrying the bridge route inventory is safe.
- Added an explicit ENOSPC route-flush regression for successful restoration and secondary restoration failure, with the original flush error preserved. Extended the WAN semantic checker to require recovery/logging before the PHY clear or any restart continuation.
- The immediately preceding PHY-clear recovery slice and this follow-up share the exact WAN route workflow; the first slice passed GNU89/ASan/UBSan policy binaries and the full WAN source checker on `793bc85`. This follow-up requires its own exact-head rerun.

---

## 2026-11-20 — Restore LAN static routes when switch MAC-table cleanup aborts WAN restart

- Parent main SHA: `2fb6879b0088fbd2d399b05865215e33f00d8d60`. Confirmed an uncovered failure path in `full_restart_wan()`: after both WAN and bridge routes plus the global route cache are successfully flushed, `phy_clear_mac_table()` can fail and return immediately. Unlike the later WAN protocol-reset failure branch, this early abort did not restore the static LAN routes removed by the preceding flush.
- Generalized the existing first-error route-recovery policy into `rc_wan_restore_lan_routes_after_failed_step()`, retaining the reset-specific wrapper for compatibility. The MAC-table failure branch now attempts `add_static_lan_routes(IFNAME_BR)` before aborting, logs a secondary route-recovery error separately and preserves the original MAC-table status.
- Extended the GNU89 policy regression for successful recovery, failed recovery/first-error preservation, zero-status no-op and invalid callback behavior. Updated the WAN semantic contract to require the MAC-clear recovery order and both failure logs. No target hardware or MIPS build is claimed by this host-source slice.

---

## 2026-11-20 — Correct integration fixture line endings after CI caught escaped literals

- Parent main SHA: `42b65bc59418e3ff4b36a6d81db1c9f1d19e6134`. The eight-case customization workflow `44705796914` ran all six transaction-helper tests and the same-path fail-closed integration case successfully; the end-to-end LAN-IP fixture failed because its Python test source contained literal backslash-`n` sequences, so the fixture did not contain distinct `#define` lines. Production code did not fail.
- Correct the two integration fixtures and the expected dictionary value to use actual newline escapes. The exact-head rerun will exercise the real CLI path against three separate macros and the language dictionary.
- The prior exact-head six-test run `44705563908` was PASS. This is a test-fixture-only correction; no implementation changes.

---

## 2026-11-20 — Cover the customization command-line transaction path

- Parent main SHA: `c037a5685a7c4381d59de250436287e844f11db9`. The exact-head customization regression workflow `44705563908` passed all six low-level failure-injection cases: successful two-file replacement, rollback after the second rename fails, retained original snapshot if rollback itself fails, pre-publication staging failure, symlink rejection and duplicate path rejection.
- Added two end-to-end `main()` tests to exercise the actual `CUSTOMIZATION_JSON`/CLI path used by `setip.sh`: updating the three LAN macros together with the language dictionary, and rejecting a shared header/dictionary target without changing it. The same dedicated workflow discovers these tests automatically.
- This closes the integration gap between the transaction helper and its production caller; no product C source changed.

---

## 2026-11-20 — Stage and compensate multi-file firmware customization writes

- Parent main SHA: `99b96ecf425e6986e4298b106a9e67f1545afd26`. Found a partial-update defect in `trunk/custom/scripts/apply_customization.py`: LAN-IP customization rewrote `defaults.h` in place, then rewrote the Chinese dictionary separately. A failure during the second write could leave the two build inputs inconsistent; either direct write could truncate its target before all new content was safely written.
- Replaced direct writes with same-directory staged files, file fsync, preserved mode bits, and atomic per-file replacement. Both new inputs and private rollback snapshots are staged before the first rename. If a later rename or directory sync fails, previously published files are restored in reverse order; if restoration itself fails, the original snapshot is retained and its path is reported. Final-component symlinks and duplicate path aliases fail closed.
- Added six failure-injection/unit cases for successful publication, second-rename rollback, rollback-snapshot retention, staging failure, symlink rejection and duplicate-path rejection. Added a dedicated path-filtered CI workflow for this build-time utility.
- The compensating rollback handles reported I/O errors; as documented in code, separate filesystem renames are not globally crash-atomic as one operation. No firmware C source changed in this slice.

---

## 2026-11-20 — Pin the old IPTV VLAN clear/remove compensation policy in CI

- Parent main SHA: `bb9d0ee5307911eeae81575f6ea916f54ef0e361`. This checker/log follow-up fixes the exact-head WAN route failure from the new VLAN transition commit; production C and policy tests remain unchanged in this checker-only slice.
- Exact WAN route run `44704885931` on `bb9d0ee5307911eeae81575f6ea916f54ef0e361` compiled and passed the route-cleanup, NVRAM snapshot and static IPTV host policy regressions, then failed in `scripts/check_wan_vlan_setup.py` because the old contract still expected `wan_viptv_ifname_publish_status("")` after `remove_vlan_iface()`. The production path now calls the transactional old-VLAN helper, which clears identity first, removes the VLAN, and restores the identity if removal fails. Update the source contract to inspect that policy order and assert all five registered injected cases.
- The exact source commit `bb9d0ee5` also triggered the XY-C1 firmware validation and broader WAN host workflows; final firmware status must be refreshed before any L3 PASS claim. No local compile/test was possible in this environment.

---

## 2026-11-20 — Preserve IPTV VLAN recovery identity during old-VLAN removal

- Parent main SHA: `e28b6a456d136e50b401886825658c2698a509b3`; re-applied on top of concurrent checker fix `e28b6a456d136e50b401886825658c2698a509b3` without overwriting its loader/source-check changes.
- Found an unsafe ordering in `launch_viptv_wan()`: for a stale interface named as a VLAN, it removed the old VLAN first and only then cleared `viptv_ifname`. If the NVRAM clear failed, the persistent recovery handle still pointed at an already-removed interface. Clearing first also requires compensation if the subsequent VLAN-removal ioctl fails.
- Added a transaction policy: clear the old recovery identity first; only then remove the old VLAN; if removal fails, restore the original identity so a later stop/reconfigure can retry. The old interface name is copied into a bounded local buffer before the NVRAM write (which may invalidate NVRAM-backed string storage). If the clear fails, the VLAN is left intact. Non-VLAN bridge/interface cleanup retains its prior branch behavior.
- Added failure-injection coverage for clear-before-remove ordering, clear failure skipping removal, remove failure restoring the recovery identity, restore failure preserving the primary remove error, and incomplete callback tables. Source contract now pins the transactional order and all registered tests.
- Parent `aa4021a190906132b3f73d68b1d76a4efb516526`: 19 exact-head runs completed; 17 passed and two source-contract failures were diagnosed (WAN route `44703962457` guard adjacency; HTTPD `44703962244` stale `return 0` check), fixed in `72eda3e6aa96a745811655a96b5dd36a0eec770d`. Exact `72eda3e` HTTPD `44704349353` and AI docs `44704349395` passed; WAN route `44704349402` failed due checker `NameError: policy` and a stale old clear expectation, fixed concurrently in `e28b6a456d136e50b401886825658c2698a509b3`; its exact WAN route rerun `44704616742` and AI docs `44704616750` passed. XY-C1 source run `44703962127` was still in progress at last poll, after 32 pre-build checks and entry into the MIPS build step; final build/TRX/artifact outcome not yet established. This commit changes C and must trigger fresh XY-C1 validation. No local checkout/compiler was available.

---

## 2026-11-20 — Fix remaining WAN IPTV checker dependency and checked-clear assertion

- Parent main SHA: `72eda3e6aa96a745811655a96b5dd36a0eec770d`. This follows the exact-head checker repair in `72eda3e6aa96a745811655a96b5dd36a0eec770d`.
- WAN route run `44704349402` on `72eda3e` compiled and passed all C policy regressions, then failed in `scripts/check_wan_vlan_setup.py` with `NameError: name 'policy' is not defined`. The source contract had been extended to inspect `wan_viptv_static_policy.h` but had not loaded that file; the loader is now explicit and shared by the static-start and VLAN-identity assertions.
- The same checker still contained a second stale direct-write assertion for clearing `viptv_ifname`. Replace it with an ordering check for `wan_viptv_ifname_publish_status("")`, consistent with the checked clear already pinned earlier in the script.
- On exact SHA `72eda3e`, HTTPD security `44704349353` and AI handoff docs `44704349395` passed; WAN route `44704349402` is the sole identified failed rerun. The previous source-producing `aa4021a` NVRAM transaction audit passed (`44703962341`); its XY-C1 Build/TRX/artifact job `44703962127` was still compiling at last poll. No L3 claim until all three firmware stages succeed on the same product-source SHA.
- The fix is checker-only: no C test, production assertion or compatibility gate is relaxed. Exact-head WAN route and AI docs reruns are required after this commit.

---

## 2026-11-20 — Align IPTV VLAN checks with checked identity writer and status return

- Parent main SHA: `aa4021a190906132b3f73d68b1d76a4efb516526`. Contract-only update to the WAN VLAN checker, the HTTPD embedded source contract, and this log.
- Exact-head WAN route run `44703962457` on `aa4021a190906132b3f73d68b1d76a4efb516526` failed because the checker required the GMAC bridge batch's closing `#endif` to be immediately adjacent to `rollback_viptv_vlan_setup()`. The checked `wan_viptv_ifname_publish_status()` helper now correctly sits between those two definitions; the contract now verifies the guard closes before the status writer and rollback adapter, preserving the board-configuration ordering check.
- Exact-head HTTPD regression run `44703962244` failed in a copied source-contract block at `assert "return 0;" in launch_iptv`. `launch_viptv_wan()` now correctly returns the actual status for same-VLAN, static, DHCP and ZeroConf paths; the stale contract is changed to require `return ret;`, not a swallowed success.
- In the same HTTPD job the earlier NVRAM profile sanitizer, restore rollback and switch status/link-refresh checks passed; the job failed only at the final WAN launch source assertion. Fresh exact-head runs will validate both contract updates. The current C source commit is `aa4021a190906132b3f73d68b1d76a4efb516526`; XY-C1 build validation for that source SHA remains pending/needs direct confirmation.

---

## 2026-11-20 — Fail closed when IPTV VLAN recovery identity cannot be persisted

- Parent main SHA: `9f9f35f4f354e27520ce543261ab40fa15f56bc2`. Updated `net_wan.c`, the WAN IPTV policy and injected regression, the VLAN source checker, and this log.
- Found an unchecked critical state write in `launch_viptv_wan()`: after creating and configuring a new VLAN (and optional bridge), it ignored `nvram_set_temp("viptv_ifname", ...)`. That setting is the recovery handle used by later teardown; a failed write could leave an active VLAN/service which a later stop/reconfigure cycle cannot identify. The same recovery metadata writes inside `rollback_viptv_vlan_setup()` were also unchecked.
- Added a checked writer using `rc_wan_nvram_write_status()`. The active interface identity is now committed through a policy that invokes the existing ownership-aware VLAN/bridge rollback if publication fails and preserves the original NVRAM error if rollback also fails. Rollback's own recovery-identity update is checked and accumulated into its status. Same-VLAN and stale-identity clear paths also report failures rather than silently continuing.
- Added injected regressions for success/no rollback, failed publication triggering rollback, rollback failure preserving the primary error, and invalid callback tables. Updated source checks to pin the active metadata commit and rollback behavior.
- On exact SHA `02285d690401d1a27d90354f9243b82b4ca8d2ee`, WAN DHCP callback and AI handoff Actions passed. The source-producing ancestor `3ca051d4590ef6a81f8c52074e262e4e4254c7f1` had NVRAM boundary audit PASS, and its XY-C1 MIPS cross-build was still running at last poll; the host-only firmware-validation run for documentation/checker-only SHA `ea98cd8` skipped the MIPS build as expected. The new C change needs fresh exact-head firmware/route checks. No local checkout/compiler was available.

---

## 2026-11-20 — Align WAN callback embedded static IPTV contract with transactional publisher

- Parent main SHA: `ea98cd8cdf5dd8d84585bef9a83a292dbc716d59`. Exact-head callback run `44702965265` on `ea98cd8cdf5dd8d84585bef9a83a292dbc716d59` passed its GNU89/ASan/UBSan lease and snapshot host binaries, but the embedded workflow contract then raised `ValueError: substring not found` while requesting the removed `wan_viptv_static_publish_address()` and `wan_viptv_static_publish_gateway()` callbacks.
- The static IPTV production policy now uses `wan_viptv_static_publish_snapshot()` and `wan_viptv_static_clear_address()`. Updated the embedded callback workflow to assert the compensatable three-key snapshot, address clear, route/multicast order and the new failure-path regressions rather than the retired void callbacks. No production checks or test coverage were removed.
- The previous exact-head run `44702791487` on `3ca051d4590ef6a81f8c52074e262e4e4254c7f1` had failed earlier in the standalone LAN checker because its C body extractor matched the static IPTV helper's forward declaration instead of its later definition; commit `ea98cd8cdf5dd8d84585bef9a83a292dbc716d59` corrects that prototype skip. The new callback workflow rerun will confirm both corrections together.
- Exact-head AI handoff validation `44702965262` on `ea98cd8` passed; XY-C1 run `44702965328` was in progress and had entered the MIPS cross-build stage. The earlier source SHA `3ca051d` had broad completed host passes; WAN callback was the only identified failure at this point. No local build was performed.

---

## 2026-11-20 — Skip forward declarations in LAN DHCP C source checker

- Parent main SHA: `3ca051d4590ef6a81f8c52074e262e4e4254c7f1`. Exact-head WAN DHCP callback run `44702791487` on `3ca051d4590ef6a81f8c52074e262e4e4254c7f1` compiled and passed both DHCP lease and WAN NVRAM snapshot sanitizer binaries, then failed in `scripts/check_lan_dhcp_deconfig.py` at line 333.
- Cause: the checker’s `c_function_body()` selected the newly added forward declaration of `wan_dhcp_nvram_publish_values()` near the static IPTV callbacks, then treated the next function's opening brace as the prototype's body. It therefore never inspected the actual later definition containing `nvram_transaction_begin(&transaction)`.
- The parser now skips a matching declaration when a semicolon occurs before the next opening brace, and then extracts the real definition. This preserves the transaction/rollback assertions rather than weakening them. The exact callback run must be repeated on the new SHA.
- On `3ca051d4590ef6a81f8c52074e262e4e4254c7f1`, completed exact-head checks included WAN teardown, AI handoff, ZeroTier, process inventory, wireless, detect_link, DNSMasq, firewall, HTTPD, WAN route, ARP probe, USB modem and miniupnpd PASS. WAN callback was FAIL for the parser issue; XY-C1 MIPS build, NVRAM boundary audit, VPN/process lifecycle/watchdog/HTTPD, and detect_internet were still in progress at this poll. The firmware job had entered `Cross-build XY-C1 firmware`, so it was not skipped.
- No local checkout/compiler was available; host binaries above are CI evidence only.

---

## 2026-11-20 — Fail closed on static IPTV NVRAM publication errors

- Parent main SHA: `ea976ca3a553f238d202d84be0b89022efed062b`. Updated the static IPTV startup policy, production callbacks, injected regressions, the VLAN/bridge source contract, and this log. Parent main also contains a concurrent LAN DHCP coherent-snapshot fix; that change is preserved.
- Static IPTV startup previously accepted `void` address/gateway publication callbacks. Those callbacks performed unchecked temporary NVRAM writes to `manv_ipaddr`, `manv_netmask`, and `manv_gateway`, after which the policy could install a default route and start IGMP proxy even if the recovery snapshot was partial or missing.
- Replaced those writes with one compensatable three-key tuple using the existing transaction/rollback publisher. The policy now requires an error-returning publisher and address-clear callback; if publication fails, it clears the just-configured interface address and stops before route/multicast startup. The original publication error remains primary if address cleanup also fails.
- Added injected coverage for publication failure, address cleanup, cleanup failure preserving the primary error, and no route/multicast side effects after failed snapshot. Existing successful, route-failure, missing-gateway, and multicast-failure ordering cases remain.
- On ancestor `e34572aed4510b50a3e1d171b5b44940c95e68ec`, 19 exact-head Actions were queued, including XY-C1 and NVRAM audit. Concurrent LAN fix `ea976ca3a553f238d202d84be0b89022efed062b` has since advanced main; its exact-head XY-C1 firmware and NVRAM audit were in progress when rebasing this candidate. No local checkout/compiler is available in this execution environment; exact-head Actions remain authoritative.

---

## 2026-11-20 — Check WAN-down statistics NVRAM writes without gating teardown

- Parent main SHA: `6b40725ac83447fec99ec79e0b6dfc79eefce83f`. This slice updates `net_wan.c`, its injected NVRAM policy regression, the existing WAN firewall/source contract, and this log.
- The legacy `wan_down_internal()` path still reset four WAN statistics with unchecked `nvram_set_int_temp()` wrappers even though the same module's checked NVRAM policy and status callback were already in use for runtime state. A failed write silently left stale uptime/traffic counters.
- Replaced those four writes with the checked attempt-all policy. Each counter clear is attempted and the first error is logged with unit/status; because these counters are diagnostic rather than route/interface prerequisites, a failed statistics write does not block remaining teardown or turn a completed WAN stop into a false failure.
- Added a GNU89-injectable regression where the first counter write fails with `ENOSPC` and all four ordered writes are still attempted. The source contract pins the production path to the checked writer and confirms the legacy unchecked calls are gone from `wan_down_internal()`.
- The parent exact-head IPTV callback and HTTPD contract reruns were pending at checkpoint on `6b40725ac83447fec99ec79e0b6dfc79eefce83f`; XY-C1's prior production-source detector skipped the expensive MIPS build because the triggering SHA changed only workflow/checker inputs, so the IPTV production slice still needs a source-triggered L3 build. No local repository checkout/compiler was available here.
- Exact-head Actions validation for this task is required after commit; L4 hardware behavior remains NOT VERIFIED.

---

## 2026-11-20 — Narrow IPTV deconfig source-contract scopes

- Parent main SHA: `7485455e53de44887907218c9ee22990974d11bf`. This contract-only commit updates the WAN DHCP callback and HTTPD workflow checks plus this log; production C and regression tests are unchanged.
- Exact-head failures on `7485455e`: callback run `44701130333` reached line 242 after earlier tests, then failed because the source assertion required a single-line `route_del(man_ifname, 10...)` call while the implementation intentionally wraps the call over several lines. The assertion now checks the stable `route_result = route_del(` prefix.
- HTTPD run `44701130327` failed at line 269 because its `manv_deconfig` text slice extended beyond the route function and included the shared five-key publisher callback; the contract incorrectly asserted that the publisher symbol was absent from that larger region. Narrow the slice to the end of `manv_route_deconfig()` before asserting separation.
- XY-C1 firmware was in progress at the last exact-head poll; AI handoff and multiple unrelated regression workflows passed. Fresh exact-head checks are required after this commit.

---

## 2026-11-20 — Make IPTV and ZeroConf lease teardown metadata atomic

- Parent main SHA: `86e30d782ba87f81817b45dad91c1a85558c494c`. Updated `net_wan.c`, DHCP lease policy regression tests, the WAN DHCP callback source contract, the detect-internet source contract, and this log.
- IPTV MAN route teardown now reads the saved gateway and all three DHCP route inventories through one checked NVRAM snapshot, rejects oversized values, and performs route deletion without independently clearing NVRAM. This prevents route cleanup from using a mixed renewal tuple and avoids committing half of a deconfig metadata transition.
- DHCP IPTV and ZeroConf-IPTV now share one five-key compensatable publication: `manv_ipaddr=0.0.0.0` plus `manv_gateway/manv_routes/manv_routes_ms/manv_routes_rfc` cleared together. Publication occurs only after interface and route teardown succeed; any metadata write error propagates to the callback return path so BusyBox can retry. Failed partial writes compensate the complete previous tuple under the existing transaction helper.
- Expanded injected NVRAM policy coverage from four route fields to the full five-field lease tuple. Tests cover successful tuple publication, a write-that-mutates-then-fails with reverse-order full restore, and a restore failure that is reported while later compensating writes still run. Existing deconfig retry-order tests remain enabled.
- Fixed a stale detect-internet workflow assertion that still expected direct `nvram_safe_get("wanx_gateway")` access after MAN gateway reads were centralized in `wan_man_gateway_snapshot()`. On parent SHA `73abb612`, detect-internet run `44700181923` failed solely at that stale source-contract substring (ValueError on line 145); watchdog, ZeroTier, wireless and AI handoff checks had passed. Fresh exact-head validation is required for all touched workflows and XY-C1 firmware.
- Parent `73abb612` had WAN route, HTTPD, NVRAM, WAN callback, USB modem, process inventory and XY-C1 runs pending at last poll; wireless, watchdog, ZeroTier and AI handoff passed. No local checkout/compiler run was available in this session.

---

## 2026-11-20 — Normalize WAN snapshot helper and MAN launcher source contracts

- Parent main SHA: `45d049e256afa4d4378bfb10e92b289a605a64a7`. This commit contains only source-contract corrections in `scripts/check_modem_wan_start_status.py`, `.github/workflows/wan-dhcp-callback-stability.yml`, and this chronological record; product C is unchanged by this commit.
- The static IPv4 snapshot helper's signature was separated from its body in the checker, but the helper/caller body assertions still mixed `compact()` (which retains single spaces) with no-space expected text. In the embedded WAN callback contract, `launch_wanx` is fully whitespace-stripped but its status-gate assertion still expected spaced text.
- Use consistent whitespace removal for both helper and MAN launcher bodies; keep signature checks on the actual signature slice and align the embedded predicate with its already-normalized body. No tests or checks were removed or weakened.
- Exact-head source-contract failures on `3fd870de`: WAN callback run `44699162824` failed at embedded line 1060; USB modem `44699162814`, Firewall `44699162872`, and Process lifecycle `44699162719` failed at the standalone helper assertion. The earlier `6ff4b9e7` failures `44698925841` and `44698925800` stopped at the signature-in-body check. The new source head `8e0e0386` triggered independent WAN-route/MAN-gateway validation runs; exact-head Actions on `8e0e0386` and its workflow-contract follow-up `45d049e2` were pending at this edit. Fresh CI is required on this commit; no local checkout was available.

---

## 2026-11-20 — Rotate the root development log and refresh the verification checkpoint

- Parent main SHA: `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49`.
- The root development log contained 120 complete dated records, and exact-head AI handoff run `44693247011` correctly failed its rolling-window guard. Archived the oldest 100 complete records, oldest-to-newest and byte-for-byte, in `docs/dev-log/2026/11/part-019.md`; retained the newest 20 records in the root before adding this checkpoint.
- Archive source root blob: `eb7190fedf832884fdaf9b48045223a91dcdcb22`; archive size: 114230 bytes; archive blob: `bf5f91f6e47631f2f1331a3be3dd46c3515d4a89`. No archived record was rewritten or omitted.
- The WAN callback source contract on `ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1` failed only at a stale workflow assertion requiring a direct ZeroConf interface-clear call after rollback had been centralized. Commit `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49` removes that obsolete assertion and pins the shared restore helper's set/clear branches; exact-head callback rerun `44693740862` was queued at the last poll.
- On `ee8ef67`, WAN route, HTTPD, AI docs, NVRAM boundary and the other completed host checks passed. XY-C1 firmware validation run `44693084076` was still in progress at the last poll; MIPS build/TRX/artifact and L3 remain unverified.
- Next source task: make IPTV MAN route-inventory and IPTV/ZeroConf-IPTV deconfig metadata clears transactional and status-returning. A failed NVRAM publish must remain visible to the deconfig retry path; add injected tests for partial writes, rollback and retry ordering before changing the callers.

---

## 2026-11-20 — Prevent truncated IPv4 rollback values across DHCP and ZeroConf callbacks

- Parent main SHA: ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1; commit under verification: ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1.
- Found a defensive rollback bug in all four bound callbacks: previous IP/netmask/gateway text was captured in larger snapshot storage then copied into INET_ADDRSTRLEN buffers without a fit check. An overlong corrupt string beginning with a valid IPv4 literal could be truncated to that different address and used for interface/route rollback. Added dhcp_lease_text_fits_buffer(); each callback now rejects an oversized old address before interface or route mutation.
- Added helper tests for empty, exact-fit, valid-prefix-overflow, NULL and zero-capacity values. Added an 11-field injected NVRAM regression: a setter that writes then returns ENOSPC must not stop remaining writes, and all old tuple fields must be restored in reverse order.
- Also made WAN callback and WAN route source-body extractors skip C prototypes/call sites so the publisher parser reaches the actual function implementation; this fixes the stale ZeroConf assertion exposed by 44692804497 on 5e2c845.
- Exact-SHA Actions on ee8ef67 were queued/in progress at the last poll: WAN callback 44693084363, WAN route 44693084041, HTTPD 44693084106, AI docs 44693084139, NVRAM 44693084087, XY-C1 44693084076. No result claimed until completion.

## 2026-11-20 — Align WAN/PPP and DHCP-renew source contracts with shared transactions

- Parent main SHA: `2c19a2172400fb3b775168c499f1148bb3c6b5a6`; production implementation under validation: `10b57237ced5487a0d17e0bb09425ea96973a78e`.
- WAN route `44689432208` and XY-C1 `44689432315` on `5e7d8cf`, plus XY-C1 `44689620163` on `b5146272`, failed in `scripts/check_wan_firewall_status.py` because its PPP snapshot marker expected `rc_wan_nvram_set_all_values()`. Production `ppp_wan_ipup_publish_address()` correctly calls `dhcp_lease_nvram_publish()` inside the NVRAM transaction and retries `dhcp_lease_nvram_restore()` under a fresh transaction; checker now matches that code.
- HTTPD `44689024189` on source `10b5723` failed because its embedded contract required removed comment text `dnsmasq domain-specific server routing too`. It now verifies DNS/domain changes set `resolver_changed`, and publication failure is checked before the resolver-refresh gate/call.
- WAN DHCP callback `44689620059` passed on `b5146272`; NVRAM boundary `44689024044` and injected GNU89/Werror/ASan/UBSan attempt-all/full-rollback regression passed on production source `10b5723`. AI docs `44689831389` passed on parent `2c19a2172400fb3b775168c499f1148bb3c6b5a6`.
- Validator/workflow/handoff change only; no production C changed in this slice. The XY-C1 attempts stopped before detector/MIPS build/TRX/artifact due to the stale source-check assertion; L3/L4 remain NOT VERIFIED.

## 2026-11-20 — Restore mandatory handoff status and next-action headings

- Parent main SHA: `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`.
- Exact-head AI handoff validation run `44689620108` failed with a concrete validator message: `SESSION_HANDOFF.md` was missing required sections `## Verification boundary` and `## Exact first action for the next GPT`. This was a handoff-structure defect, not a source/test failure; both sections are restored with current CI evidence and a specific next source task.
- Exact-head WAN DHCP callback run `44689620059` passed on this parent. NVRAM boundary run `44689024044` and the injected GNU89/ASan/UBSan attempt-all/full-rollback policy executable passed on production source SHA `10b5723`.
- XY-C1 run `44689620163` was still in progress at the last poll. Its outcome determines whether the current production slice reaches detector/MIPS build/TRX/artifact verification; no L3 result is claimed.
- The WAN route/XY-C1 stale MAN teardown contract was corrected to assert the checked `rc_wan_nvram_set_all_values()` path. The latest WAN DHCP callback PASS demonstrates that the LAN rollback checker no longer stops this workflow.
- Local tests remain NOT RUN because this execution environment has no local repository checkout. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Remove invalid test-source variable from LAN policy checker

- Parent main: `5e7d8cf074fcaf7bb392dd50373dfd435e518176`. The LAN deconfiguration checker was updated alongside the new shared DHCP rollback semantics, but the added test-name assertion referenced a `test` variable that this standalone checker does not define. Removed that redundant assertion; the WAN DHCP callback workflow independently checks the test function and invocation in `tests/dhcp_lease_policy.c`.
- The checker continues to require validate-all, attempt-all and reverse full-vector restoration (`for (j = count; j > 0; --j)`). This is a validator correction only; no production source change.
- Next exact-SHA Actions: WAN DHCP callback, WAN route, XY-C1, AI docs and NVRAM boundary must be polled. No new PASS is claimed. The production source SHA remains `10b57237ced5487a0d17e0bb09425ea96973a78e`; XY-C1 L3/TRX/artifact and physical L4 remain NOT VERIFIED.

## 2026-11-20 — Align shared DHCP policy and MAN teardown validators with current writers

- Parent main SHA: 9cadbbd9c116db4b8c0cc3d4f8e312800575f173; production implementation under validation: `10b57237ced5487a0d17e0bb09425ea96973a78e`.
- Exact-source WAN DHCP callback run `44689024028`, job `136130112379`, compiled and passed the GNU89/ASan/UBSan DHCP lease policy executable, including the new attempt-all/full-rollback regression. It then failed in `scripts/check_lan_dhcp_deconfig.py` because the checker still required prefix-only rollback loop `for (j = i + 1; j > 0; --j)`. The shared publisher now restores every tuple entry with `for (j = count; j > 0; --j)`; the validator now checks full-vector rollback plus the new regression.
- Exact-source WAN route run `44689024130`, job `136130113076`, and XY-C1 run `44689024255`, job `136130113735`, failed in `scripts/check_wan_firewall_status.py` because its MAN teardown check incorrectly expected `dhcp_lease_nvram_publish()`. MAN teardown correctly uses the checked all-fields `rc_wan_nvram_set_all_values()` helper and its errno-aware writer; this commit fixes the assertion only.
- Exact-doc SHA `9cadbbd9c116db4b8c0cc3d4f8e312800575f173` AI handoff validation passed (`44689130545`).
- NVRAM boundary run `44689024044` was still in progress at checkpoint. XY-C1 had not reached MIPS build/TRX/artifact steps because the shared source checker stopped first. L3/L4 are NOT VERIFIED.

## 2026-11-20 — Publish DHCP renewal options and PPP runtime tuple through shared rollback policy

- Policy/test commit: `6c0ba561b93e9e4c6ee647718a251d7e4173d727`. The shared `dhcp_lease_nvram_publish()` validates the full vector, attempts every candidate setter despite individual failures, preserves the first write error, and restores the complete previous tuple in reverse order. An injected regression covers invalid-tail validation before writes, a first-key error after applying its value, attempt-all behavior and full restoration.
- Product/source-contract commit: `10b57237ced5487a0d17e0bb09425ea96973a78e`. WAN `udhcpc_renew()` now publishes changed DNS/domain/WINS/lease duration as one serialized NVRAM transaction. Withdrawn/invalid DNS/domain/WINS normalize to empty; absent/invalid lease duration preserves the old value. Resolver refresh occurs only after the DNS/domain snapshot succeeds.
- PPP `ipup_main()` reuses the shared policy for `ipaddr/netmask/ifname_t/gateway/dns`, runs setters under one NVRAM transaction, retries full tuple restoration under a fresh lock after write or transaction-end failure, and retains the existing interface rollback/route gate.
- Updated `scripts/check_wan_firewall_status.py` and the WAN DHCP callback workflow to pin the new renewal and PPP transaction semantics. The exact-source Actions on `10b5723` were in progress at handoff: WAN DHCP callback `44689024028`, WAN route `44689024130`, XY-C1 `44689024255`, AI docs `44689024296`; broader lifecycle and NVRAM checks were also still running.
- Previous exact SHA `79d8f0a7cc59be4e96ee827ff09fcad0576b3720`: WAN route PASS (`44687337571`); AI docs FAIL (`44687337698`) only because the handoff used uppercase `One atomic main commit`. This handoff restores the required lowercase phrase.
- No local checkout is available here; local GNU89/ASan/UBSan tests were NOT RUN. Do not claim L3 until same-source-SHA detector, MIPS build, TRX verification and artifact upload all pass. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Correct the WAN resolver refresh assertion and handoff contract

- Parent main SHA: 971372f4a8a662eeda42e3d7b80ec6f9be6e3de2.
- Exact-SHA detect-internet run 44686752941 passed after its checked VPN resolver marker contract was updated. NVRAM boundary run 44686157053 also passed on the preceding product source.
- WAN route run 44686752930 and XY-C1 run 44686753007 still failed at the new WAN resolver-refresh assertion. The previous repair used startswith() on a string slice that begins at the setter assignment; that made the expected closing brace impossible at offset zero. Corrected it to require the close-of-conditional + unconditional refresh sequence as a substring after the checked setter. Production C remains unchanged.
- AI handoff run 44686752989 flagged two exact protocol phrases omitted while rewriting SESSION_HANDOFF.md. Restore the required "one atomic main commit" and "Before context exhaustion" contract wording without changing the execution rules.
- No same-SHA MIPS build/TRX/artifact result exists yet: XY-C1 stopped at the source contract. Current-source L3 and physical-router L4 remain NOT VERIFIED. The next exact-head WAN route, XY-C1 and AI docs runs are the verification gate.

## 2026-11-20 — Repair WAN resolver teardown source-contract boundaries

- Parent main SHA: 21e567affba4e992476aa7dc16c30be7fa6d4500; the product-source change under test is fix(wan): report resolver snapshot clear failures on teardown.
- Exact-head run 44686156823 (XY-C1, job 136120854113) and run 44686157087 (WAN route, job 136120854818) passed the GNU89/ASan/UBSan route and IPv6 policy executables, then failed in the shared scripts/check_wan_firewall_status.py source contract. The assertion built wan_dns_block with an exclusive end at wan_dns_refresh, then incorrectly required the refresh call to be inside that preceding slice. The production wan_down_internal() call is unconditional and correctly follows the checked conditional DNS clear.
- Exact-head detect-internet run 44686156914, job 136120854304, failed because the embedded contract still searched for the removed unchecked nvram_set_temp("vpnc_dns_t", ""); the current source uses set_nvram_value_status() and aggregates the error.
- Corrected the WAN source contract to assert the closing conditional and the unconditional resolver refresh directly after the checked clear. Updated the detect-internet callback-order marker to the checked setter. No production C behavior was changed in this checker repair.
- NVRAM transaction boundary run 44686157053 subsequently completed SUCCESS on the parent SHA. The XY-C1 job stopped at the WAN contract before the complete build/TRX/artifact gate; L3 is NOT VERIFIED. Physical-router L4 remains NOT VERIFIED. All corrected workflows must be checked again on the resulting exact SHA.
- The parent SHA had 17 successful Actions and the three failures above; the AI docs, Process inventory, VPN server, WAN DHCP, watchdog, DNSMasq, HTTPD, miniupnpd, ZeroTier, wireless, firewall, detect-link, gateway-ARP, WAN teardown and USB modem regressions passed.

## 2026-11-20 — Check WAN-down VPN and indexed DNS snapshot clears

- Parent `main` SHA: `b68158c3ee405bcdc3270db0e6adfd9de91828e5`.
- Found two unchecked temporary NVRAM writes in `wan_down_internal()`: the global `vpnc_dns_t` resolver marker was unconditionally cleared before the optional VPN-client stop, and the indexed `wanN_dns` snapshot was conditionally cleared before `refresh_resolvconf_nonfatal()`. Either failed write was invisible to the callback return status.
- Both clears now use the existing errno-aware NVRAM status setters and accumulate the first failure into WAN teardown status while continuing independent VPN stop, route/interface cleanup and resolver refresh. The existing demand-PPP preservation condition for indexed DNS is unchanged; resolver rebuild remains nonfatal.
- Added a route-status regression for two independent resolver-clear failures with later teardown steps still attempted, and extended the WAN source contract to pin both checked writes, their error aggregation/logging, the original conditional and resolver-refresh ordering.
- Exact-head WAN DHCP/route/HTTPD and XY-C1 validation is required; no firmware L3 or physical-router L4 claim is made here.

## 2026-11-20 — Align PPP ip-up Actions with checked five-field snapshot publication

- Parent `main` SHA: `b4ab85764446ab133873f9d931d411ef25e7761e`.
- Exact-head WAN DHCP and HTTPD Actions on `b4ab85764446ab133873f9d931d411ef25e7761e` failed only in embedded source contracts left over from the earlier callback shape. WAN DHCP still expected the publisher to call unchecked per-field setters and expected `ipup_main()` to perform identity/gateway writes after the policy call; HTTPD used the old identity setter as the first callback side effect.
- Updated the WAN DHCP contract to check the five-field temporary NVRAM snapshot, attempt-all publish + rollback, NVRAM return/errno handling, interface rollback, and no `wan_up()` before the checked policy succeeds. The HTTPD callback-lock contract now treats the policy entry as the first guarded mutation. Regression coverage is strengthened, not removed.
- The production PPP source is unchanged in this test-contract repair. Re-run the exact new SHA before marking these checks PASS; XY-C1 and NVRAM transaction Actions from the prior SHA were still pending at the last poll.

## 2026-11-20 — Publish PPP ip-up runtime state as a checked, rollback-capable snapshot

- Parent `main` SHA: `e4ffdf7e5f3fc201355f359325b637532e824cdc`.
- `ipup_main()` applied IPCP addresses, but the address callback was `void` and silently ignored `wanN_ipaddr/netmask` NVRAM failures. The subsequent `ifname_t/gateway/dns` copies were also unchecked, after which WAN route setup proceeded with an incomplete or mixed snapshot.
- Changed the callback contract to return status and expanded the production publication to five volatile fields: `ipaddr/netmask/ifname_t/gateway/dns`. The writer checks NVRAM return+errno, attempts the full tuple, snapshots the previous values and restores that tuple if any write fails. The policy then takes the newly configured PPP interface down and returns the original publication error; `wan_up()` cannot run unless the complete snapshot succeeds.
- Added policy regressions for publish failure, interface rollback and preserving the primary error when rollback also fails. The WAN route source contract pins all five keys, attempt-all/restore semantics, volatile checked writes and the `wan_up()` gate. Exact-SHA WAN, DHCP and XY-C1 Actions are the production verification gate.

## 2026-11-20 — Correct exact MAN teardown comment anchor in all source contracts

- Parent `main` SHA: `da06fe3916f3f643d833f738b5ab8652c4f4913f`.
- Exact-SHA Actions for `da06fe3916f3f643d833f738b5ab8652c4f4913f` reached 16 PASS, 3 FAIL and 1 PENDING. The failed WAN route, WAN DHCP callback and HTTPD embedded source-contract steps all raised `ValueError: substring not found` on the same test-only anchor: the test abbreviated the production comment `/* Attempt every independent clear even if the address write failed. */` to a shorter literal that does not exist in the C source.
- Updated all three workflow assertions to use the exact source comment. The production source and its semantics are unchanged in this repair. Re-run these three exact-head workflow contracts and keep XY-C1 firmware validation pending until its own run completes.

## 2026-11-20 — Clear PPP demand address snapshots before preinstalling routes

- Parent `main` SHA: `dff6f88a90d503332e27a01bea38b5e81140fb9a`.
- `wait_ppp_up()` previously wrote `wanN_ipaddr/netmask/gateway` through unchecked setters and skipped tuple updates when address/peer ioctls failed. Demand PPP may be administratively present before IPCP; stale values from a previous session could survive and later be read by WAN route/NAT/service setup.
- The demand path now stages the complete three-field tuple through the existing checked NVRAM sequence helper. It clears the local address/netmask when no local address exists, clears the peer when no P-t-P endpoint exists, and aborts before route preinstallation on any NVRAM write failure. No public ABI changes.
- The WAN source contract pins tuple ordering, unavailable-address clearing, checked write result propagation and route-preinstallation ordering. Existing injected helper tests cover ordered writes and first-error handling; exact-SHA WAN and XY-C1 Actions remain the production verification gate.

## 2026-11-20 — Repair MAN teardown source contracts after checked-clear refactor

- Parent `main` SHA: `84cc52a9602c769cbdb4bb0519e056841dabb508`.
- Exact-SHA Actions for `64e751a36d1b3cac83c605f390a8aea3b4d77624` reported failures in `wan-route-status`, `wan-dhcp-callback`, `httpd-security` and the XY-C1 workflow. The WAN route checker had a malformed Python `require()` expression; three workflows still asserted that removed direct `nvram_set_temp()` calls were present.
- Corrected the checker syntax and synchronized the existing route, DHCP callback and HTTPD embedded source contracts with the attempt-all temporary NVRAM helper, while retaining route-before-clear ordering and the conditional DHCP address clear. No production source behavior is changed in this CI repair.
- The next exact-SHA Actions run is the required verification; the preceding source task's standalone sanitizer smoke test passed, but its full source workflow had not yet passed.

## 2026-11-20 — Gate static MAN routes on the refreshed gateway snapshot

- Parent `main` SHA: `64e751a36d1b3cac83c605f390a8aea3b4d77624`.
- Traced `wan_up_internal()` into `add_static_man_routes()` and `control_static_routes()`: a route with no valid explicit gateway falls back to global `wanx_gateway`. The WAN-up path previously ignored the result of `nvram_set_temp("wanx_gateway", ...)`, so a failed snapshot write could make an ostensibly new WAN-up path install a route using the previous MAN gateway.
- WAN-up now uses the checked temporary NVRAM writer, preserves the first error in its return status, and skips the dependent static MAN route batch when the refreshed gateway cannot be published. Independent WAN route, DHCP, firewall and service attempts continue.
- Extended the WAN source contract to pin the actual `net.c` fallback dependency, checked volatile write and fail-closed route gate. Exact-head WAN route Actions will execute the contract; this change makes no firmware/hardware claim.

## 2026-11-20 — Attempt every MAN teardown snapshot clear and preserve the first error

- Parent `main` SHA: `885e71f9b3d7eeab9a2b39fc7602c11880450fec`.
- Confirmed `man_down_internal()` cleared `wanx_ipaddr/gateway/dns/domain/routes/routes_ms/routes_rfc` through unchecked `nvram_set_temp()` calls after route removal. A failed clear was invisible to the DHCP deconfig callback, which otherwise already propagates `man_down_internal()`'s status.
- Added an attempt-all NVRAM value policy: validate the complete operation list before any writes, attempt every independent clear, and return the first write failure. MAN teardown uses checked `nvram_set_temp()` writes (not flash-persistent `nvram_set()`) for the global gateway/DNS/domain/route snapshot and conditionally clears the DHCP address exactly as before. A failed clear joins the teardown result; unrelated later cleanup still proceeds.
- Added GNU89-injectable regressions for all writes being attempted, first-error preservation and invalid-input no-side-effects. Extended the WAN source contract to pin the MAN clear inventory, route-teardown ordering, conditional IP clear, unchecked-write removal and DHCP callback status propagation.
- Local standalone policy regression and exact-head WAN Actions are the required verification; no firmware or hardware result is claimed here.

## 2026-11-20 — Fail closed when clearing indexed WAN runtime snapshots

- Parent `main` SHA: `d59dc2bdea3307eac78fba6c6aabd7bdac7caf85`.
- Confirmed reset gap in `reset_wan_vars_status()`: the indexed WAN DNS, WINS, lease, domain, DHCP route lists and error snapshot were cleared through void setters. The non-static branch also zeroed the indexed IP address, netmask, gateway and MTU through unchecked setters. A failed NVRAM write could preserve a previous WAN's resolver/route/address state while reset continued toward protocol selection and activation.
- Added an injected ordered value-write policy and checked indexed setter callback. Every required clear now stops at the first error, latches `wan_vars_reset_status`, logs the affected snapshot class and aborts reset before WAN activation. The public `void reset_wan_vars(void)` ABI remains unchanged.
- Added policy regressions for ordered success, first-error short-circuiting, invalid field names and null values; the WAN source contract now verifies all eight runtime clears and all four non-static address clears are checked and fail closed. Existing WAN workflow path filters already cover every changed source/helper/test/checker file.
- Local isolated policy regression: GNU89, `-Wall -Wextra -Werror`, ASan and UBSan PASS. Full repository source-contract workflow and exact-head XY-C1 build/TRX/artifact status remain pending; no L3 firmware or L4 hardware claim is made.

## 2026-11-20 — Make WAN interface MAC snapshot writes fail closed

- Parent main SHA: `e487695db417478a5879b99da8d18871c3363d62`.
- The reset path selected `wan_hwaddr_x` or the `il1macaddr` fallback, but ignored failures from persistent `nvram_set("wan_hwaddr", ...)` and indexed `set_wan_unit_param(unit, "hwaddr")`. Later WAN VLAN and soft-bridge setup uses the global MAC for VLAN/interface `set_interface_hwaddr` operations.
- Added status-returning checks for the global persistent MAC and indexed per-WAN MAC snapshot. A failed write now latches the first reset error and prevents later interface/VLAN activation. Existing public void setter ABI is untouched.
- Extended the WAN source contract to require both writes and the fail-closed gate. Exact-head verification is pending; no firmware/L3 or hardware result is claimed.

---

## 2026-11-20 — Anchor WAN protocol checker to the protocol publication gate

- Parent main SHA: `eac0f17e12e394ede94e56dc4d432770256f9009`.
- Exact WAN-route job `136105910004` on parent `077a917` reached all policy compile/run steps, including GNU89/`-Werror`/ASan/UBSan, then failed at the final Python source contract: "failed WAN protocol publication must latch and return before protocol selection".
- The checker selected the first `if (proto_status != 0)` in the status-aware reset body. After adding checked MAN/virtual IPTV snapshot reset, this first failure gate occurs before the protocol-copy write and is unrelated to the protocol-copy contract. The checker now finds the protocol failure gate starting at the protocol-write position, preserving its intended order assertion rather than weakening it.
- Production code was not changed in this corrective commit. Exact-head rerun is pending.

---

## 2026-11-20 — Check DHCP client and PPP DNS negotiation snapshots

- Parent main SHA: `077a917ef22e4d5516289264e60b6c4b6e5607f1`.
- Source review showed `dnsenable_x` directly controls MAN DNS host routes and pppd `usepeerdns`; `vci` is passed as udhcpc's DHCP vendor-class option; `hostname` is passed to udhcpc as its client hostname. Previously all three were copied into the indexed WAN snapshot through unchecked setters.
- Added a checked required tuple for these three configuration inputs. Any write failure now latches WAN reset failure and prevents protocol activation, avoiding stale DNS behavior or DHCP identity/option reuse.
- Extended the WAN source contract to require exact field coverage, order and failure context. Exact-head CI is pending; no device/L3 claim.

---



## 2026-11-19 — Transactional LAN DHCP deconfig snapshot

- Added a coherent NVRAM snapshot to `lan_down_auto()` for the old DHCP gateway and the static LAN IPv4/domain tuple. Route deletion, interface restoration, and hosts restoration now use the same copied values.
- Snapshot failure returns before side effects and preserves DHCP lease metadata for retry. Regression coverage uses the existing checked NVRAM snapshot policy test under GNU89, `-Werror`, ASan, and UBSan in the WAN DHCP callback workflow.
- Implementation commit: `ea976ca3a553f238d202d84be0b89022efed062b`. Exact-SHA Actions state is recorded in `SESSION_HANDOFF.md`; no CI result is assumed from a pending run.
