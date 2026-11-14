## 2026-11-14 — Strict storage inventory, daemon startup gating, and lifecycle CI repairs

- `c5ce74a61c80`: replaced permissive `/proc/partitions` parsing with canonical header/separator validation, exact token counts, strict unsigned decimal and overflow-safe parsing. Added GNU89 regression for valid SCSI/MMC rows, malformed/truncated records, signed tokens, and UINT/ULLONG overflow.
- The parser Action initially stopped before the new test because BusyBox DHCPv6 used `writev()` without including `<sys/uio.h>`. `b022942` added the declaration and source contract; subsequent exact storage-hotplug status is recorded above.
- `ea2163e`: migrated HTTPD duplicate-start/readiness checks from ambiguous `pids()` truthiness to tri-state `process_inventory_scan()`. UNKNOWN now blocks config/file mutations and launch; only confirmed RUNNING publishes daemon state. Added policy tests for UNKNOWN, ABSENT, RUNNING and launcher failures.
- The first HTTPD lifecycle run failed in its embedded checker because it expected the old success branch. `b740` updated the checker to assert failed readiness returns before publishing started state.
- Crond CI exposed a missing Python `pids` source binding (NameError); `74be` adds the binding and path-filter coverage for `pids.c`.
- AliDDNS's prior exact run failed the second concurrent resolver output assertion. The same isolated-directory implementation passed local dash/BusyBox-ash reproduction; the regression was expanded to eight concurrent pairs with diagnostics to distinguish a race from a flaky runner. Await its exact-head result before calling it fixed.
- All source and workflow writes used complete atomic Git trees and non-force expected-SHA updates. No router L4 validation is claimed.

---

## 2026-11-15 — Diagnose intermittent AliDDNS concurrent resolver isolation failure

- Parent main SHA: `b740103a92cdc0a2d81f849f36d37307c4e0484b`.
- Crond's exact-head repair was completed by the parallel follow-up on `74be177adb256128c79018ad58c7ae36deed0191`: its lifecycle regression [43654459854] and AI-doc contract [43654459868] passed. HTTPD lifecycle [43654668592] and its AI-doc contract [43654668591] also passed on `b740103a92cdc0a2d81f849f36d37307c4e0484b`; the production HTTPD start gate already uses RUNNING/ABSENT/UNKNOWN before NVRAM/config mutation, and its policy tests cover UNKNOWN/invalid states.
- AliDDNS failed the same concurrent lookup assertion twice: [43653917007] on docs-only `ed7acbb`, then [43654226754] on `7911849`. In the second run the first and second lookup failures alternated (the test emitted only a generic expected-value failure), while API transport and address validation steps passed. Added a private mock-`mkdir` trace and actual a/b output values to the existing regression failure message. This preserves the assertion and should distinguish directory collisions from resolver timeout/parse failures on the next exact-head run.
- No production resolver behavior was changed in this diagnostic slice because the two CI logs did not identify the cause. A local shell simulation of the atomic-mkdir design passed 20 consecutive concurrent pairs; it is not a substitute for the target CI reproduction.
- L3 remains NOT VERIFIED for current source; L4 remains NOT VERIFIED.
- Result: THIS COMMIT.

---

## 2026-11-15 — Reject UNKNOWN from crond detached-child readiness

- Parent main SHA: `ed7acbbcf90a37a5cba2d2d939d6da47752f4866`; latest product-source baseline before this slice is `b022942bf30e6899c18d4bf40aaa528fb17a011f`.
- The crond launcher forks and exits before the detached child completes initialization. Its readiness probe used legacy `pids("crond")`, so an incomplete procfs scan could be treated as a positive process count and falsely satisfy startup. The postcondition policy now requires explicit `PROCESS_PRESENCE_RUNNING`. Startup probes use the shared tri-state inventory, retry only confirmed ABSENT, and fail immediately on UNKNOWN while logging the scan error.
- Extended the GNU89 policy regression for RUNNING, ABSENT, UNKNOWN and invalid enum values. The dedicated workflow compiles against the staged shared API, watches both scanner files, and its source contract verifies error capture, release ordering and the fail-fast UNKNOWN branch. The XY-C1 workflow now includes `services.c` in its source trigger so this production change cannot miss the target build.
- Related exact-head evidence on source SHA `35e411b4132e50d6d491a3594504038f276c08f1`: VPN [43653685672], WAN DHCP callback [43653685643], OpenVPN credential staging [43653685707], HTTPD [43653685717], WAN route [43653685904], process inventory [43653685618], process lifecycle inventory [43653685630] and AI docs [43653685685] passed. XY-C1 [43653685645] and NVRAM [43653685594] were IN_PROGRESS. On docs-only head `ed7acbb`, AI docs [43653917017], infosvr [43653916947] and AdGuardHome [43653916989] passed; process inventory [43653916959], NVRAM [43653917013], AliDDNS [43653917007] and AliyunDrive [43653916997] were IN_PROGRESS.
- No local checkout is available, so local compiler/contract execution was NOT RUN. New exact-head crond, XY-C1 and AI-doc Actions are authoritative after publication. L3 remains NOT VERIFIED for the new source until its exact-head MIPS build, TRX verification and artifact upload pass; L4 remains NOT VERIFIED.
- Changed paths: `services.c`, crond policy/test/workflow/checker, XY-C1 workflow, and this log/handoff/roadmap.
- Result: THIS COMMIT.

---

## 2026-11-15 — Propagate eject-all applet failure to command status

- Final caller audit found the `ejall` RC applet still discarded the new `safe_remove_all_stor_devices()` result. It now assigns the result to the applet's `ret`, so shell callers receive a nonzero status when storage eject is incomplete instead of a false success.
- Extended the storage integration checker to require that result propagation. This follows the checked inventory/eject implementation on parent SHA `c2b393dba16ec8ab821fe0eea8e4e3e4a5a74bc6`; exact-head storage-hotplug run [43650708280] was QUEUED and XY-C1 [43650708273] IN_PROGRESS when audited. The follow-up commit triggers exact-head reruns; current-source L3 remains PENDING.
- Root `dev_log.md` now contains 21 newest-first records, below the 120-entry rotation threshold.
- Resulting change: this commit.

---

## 2026-11-15 — Gate firmware flash and shutdown persistence on verified storage eject

- `read_disk_data()` returned NULL for both a valid empty disk list and an unreadable/truncated `/proc/partitions` snapshot. Its parser also ignored disk-object allocation failure, allowing a partial inventory to appear usable. Added `read_disk_data_checked()`: status 0 means the full inventory is complete (including a valid empty result); -1 means unknown/incomplete. It checks header/read/close errors, rejects truncated or malformed data rows, and fails on disk/partition allocation errors while preserving the original pointer-return API as a compatibility wrapper.
- Changed `safe_remove_all_stor_devices()` to return status. It now stops before unmount on lock, service-stop or inventory errors; propagates per-device/fallback unmount failure; and requires a final complete `/proc/mounts` inventory with zero storage mounts. Added a host-testable fail-closed postcondition policy.
- Firmware flashing now aborts before storage persistence and `mtd_write` when eject is incomplete. Normal shutdown/reboot still continues, but skips persistent storage MTD writes when safe eject cannot be proven complete.
- Added source-contract checks and push/pull path coverage for the checked libdisk API and policy; current exact-head storage regression and XY-C1 MIPS build are triggered by this production-source commit. L3 must be re-established on the exact new source SHA; no L4 hardware validation is claimed.
- Root `dev_log.md` reached 120 entries and was rotated in this commit: the oldest 100 complete records are preserved byte-for-byte in `part-009.md`; the newest 20 remain in root.
- Resulting source commit: this commit.

---

## 2026-11-15 — Correct networkmap stop prototype assertion after exact-head failure

- Exact-head networkmap push [43648923325] on `7697c6fd6664bb57019ae610bca642c73d88a8d9` passed the GNU89/ASan/UBSan signal-policy binary, then failed at `scripts/check_networkmap_signals.py:147`: the checker expected `void stop_networkmap(void);`, but `rc.h` correctly declares `int` and `services.c` returns the `kill_services()` result. This was a stale source-contract assertion, not a production defect.
- Updated the checker to require `int stop_networkmap(void);`, retaining the separate assertions that the stop result is aggregated in `stop_services()` and returned. The prior networkmap assertion failures on cc61 are resolved by the 7697 checker update; VPN rerun [43648923434] passed. AI docs [43648923326], process inventory [43648923376], AdGuardHome [43648923421], AliDDNS [43648923338], infosvr [43648923306] and AliyunDrive [43648923366] passed on exact 7697; NVRAM [43648923357] was still IN_PROGRESS.
- XY-C1 L3 is VERIFIED on product-source SHA `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286`: run [43648229489] passed firmware build, TRX verification and artifact upload [14135711084] (12,587,100 bytes; SHA-256 `bda83c9e71ed01a8708ad90a5104535e972bd30424bdf03ed24065b3a1a9407e`). No product source changed in the later 7697 handoff-only commit or this checker correction. L4 remains NOT VERIFIED.
- Changed paths: `scripts/check_networkmap_signals.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log. Resulting commit: THIS COMMIT.
- Root `dev_log.md` advances 118→119 records; no archive rotation or index update is due.

---

## 2026-11-15 — Align networkmap and VPN lifecycle contracts with aggregated stop results

- Exact-head CI on `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286` exposed two stale source assertions after `stop_services()` was changed from ignored direct stops to explicit status aggregation. The networkmap checker expected the literal statement `stop_networkmap();`; the VPN checker expected `stop_vpn_server();`. Production now calls each through `record_service_stop_status()` and returns the aggregate, so the assertions were false negatives rather than runtime failures.
- Updated both checkers to require the exact named stop result to be aggregated and `stop_services()` to return the aggregate. This preserves the stronger fail-closed contract instead of weakening it to a mere symbol-presence check.
- Failed exact-head runs: networkmap [43648229545] (assertion at `scripts/check_networkmap_signals.py:142`) and VPN server [43648229565] (assertion at `scripts/check_vpn_server_lifecycle.py:122`). Both will rerun on this test-only fix. On the same SHA, process inventory [43648229499], AI handoff docs [43648229603], HTTPD lifecycle [43648229544], NFS start/reload [43648229491], and Samba share [43648229471] passed; XY-C1 [43648229489] and several independent workflows were still PENDING.
- The next product slice remains storage eject failure propagation: `safe_remove_all_stor_devices()` currently returns void, hiding lock/daemon/inventory/unmount failures from firmware flash and shutdown. The checked disk inventory must distinguish a valid empty device list from a failed `/proc/partitions` read before allowing persistence or flash.
- Root `dev_log.md` now contains 118 newest-first records; no archive rotation is due.
- Resulting change: this commit.

---

## 2026-11-15 — Close XY-C1 build-trigger gaps for process identity sources

- The XY-C1 workflow's path filter did not include several production inputs changed by the identity-lifecycle migration: BusyBox udhcpc, PPPD identity PID publication, RC common/WAN/LAN/VPN/USB/iNIC lifecycle sources, and the shared PID-record header. A later change to one of these files could therefore miss the MIPS/uClibc build.
- Added the missing production paths to the XY-C1 push trigger. The process-lifecycle workflow now watches the XY workflow itself, and its integration checker asserts coverage for the affected production inputs.
- Exact-head evidence before this correction: process inventory/lifecycle [43647169443], iNIC signal lifecycle [43647169424], wireless radio state [43647169416] and AI handoff docs [43647325253] passed on their respective exact SHAs. HTTPD run [43647169410] failed on 946a8719 because its assertion still expected the old interface-only WAN gate; b98bbbb corrected it to the combined modem/interface gate, and rerun [43647325290] was pending. XY-C1 run [43647325231] was pending on b98bbbb and covered the latest source tree; this commit also changes the XY workflow and triggers a fresh exact-head target build.
- L3 is not claimed until the new XY-C1 run passes build, TRX verification and artifact upload. L4 hardware validation remains NOT VERIFIED. Root log now contains 117 records; no archive rotation is due.
- Resulting change: this commit.

---

## 2026-11-15 — Fix case-sensitive pending-CI validator assertion

- Exact-head AI handoff run [43632050498] on SHA 812b335bb5fc234eb2f40f5489eec8a80479acf9 failed at the new bootstrap assertion. The bootstrap correctly said “a PENDING check is not a stop condition”; the validator incorrectly required uppercase “A PENDING”.
- Corrected the assertion to compare normalized lowercase text, preserving the required semantic check while avoiding the case-sensitive false negative. The failure was in the validator, not the handoff policy.
- Updated SESSION_HANDOFF.md with the exact failing run/SHA, cause, correction and next exact-head validation action. No firmware source changed. Root dev-log advances 115→116 records; no archive rotation or index update is due.

---

## 2026-11-15 — Eliminate remaining pending-CI recovery contradictions

- Final cross-file search after the previous correction found two additional legacy priority/recovery sentences in the protocol (global priority selection and Section 10 recovery step) that still treated pending exact-head CI as a prerequisite to choosing new work. These contradicted the new explicit rule and could reintroduce the same stall in a later GPT.
- Rewrote both: resume the active engineering task; FAILED relevant CI is triaged before claiming validation; PENDING is recorded with run ID/head SHA and does not block independent safe work. No idle waiting or repeated polling.
- Updated the handoff to record that AI-doc run 43631988702 on SHA 9ff6a875d31bbe832bac4ada583c7519d2a993ee passed, and to inspect run 43632050498 on exact SHA 812b335bb5fc234eb2f40f5489eec8a80479acf9 while continuing PPP/PID-file implementation if pending.
- This is a separate, newly found contradictory instruction in the authoritative protocol, not a cosmetic follow-up. No firmware source changed. Root dev-log advances 114→115 records; no archive rotation or index update is due.

---

## 2026-11-15 — Remove pending-CI gate that blocked independent engineering

- Cross-file rule review found a direct contradiction: bootstrap step 5 and protocol recovery text required resolving pending CI before selecting another task, while the continuous-execution section said not to burn the session polling pending runs. This contradicted the user's explicit PENDING→continue independent work rule and could stall every GPT session.
- Changed the rule so the active engineering task resumes immediately; a relevant FAILED check must be triaged before its slice is called validated, but PENDING is not a blocker. Record the exact run ID/head SHA, continue safe independent implementation, and revisit at a useful checkpoint or before handoff. Never idle-wait or repeatedly poll.
- Updated the handoff's exact first action to inspect AI-doc run 43631988702 for SHA 9ff6a875d31bbe832bac4ada583c7519d2a993ee, while explicitly continuing PPP/PID-file work if it remains pending. Added validator assertions to prevent regression to the old pending-CI gate.
- This is a newly discovered, independently material continuity defect found during the user-requested cross-file review; it is a second correction to the same governance request, not a routine SHA/status refresh. No firmware source changed. Root dev-log advances 113→114 records; no archive rotation or index update is due.

---

## 2026-11-15 — Make end-of-session handoff checkpoint mandatory without document churn

- User-requested governance correction: every GPT session must perform and complete a handoff accuracy checkpoint before ending; the previous wording could be read as making the check optional or deferring it to the next conversation.
- Clarified that the checkpoint verifies engineering/source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. Missing/stale/changed next-session-critical facts require one consolidated handoff update and commit; an already accurate handoff requires no edit and no no-op commit.
- Defined relevant CI transitions: PENDING→PASS/FAIL must be recorded when they close/change the active validation gate or next action; unrelated workflow noise and repeated polling are not documentation milestones. Source/test/build work remains the default, and handoff-only commits are limited to material continuity corrections that cannot safely wait.
- Updated bootstrap, development protocol, current handoff and validator together. No firmware source or roadmap priority changed; the first engineering task remains PPP/PID-file inclusion in the all-target stop preflight. Root dev-log advances 112→113 records; no archive rotation or index update is due. Exact-head AI documentation Actions must validate this commit.

---

## 2026-11-15 — Anchor bootstrap orphan-fragment check to a full line

- Exact-head AI handoff run [43630474661] on e1af9e9d797a6eb015aeb924b8c4da75f457b128 failed because the orphan-fragment detector used substring matching: the valid sentence “do not claim tests” contains the substring “aim tests”. This was a second validator false positive, not malformed bootstrap content.
- Changed the check to a multiline, line-anchored regular expression matching only a standalone orphan line. Updated SESSION_HANDOFF.md to record both validator-only failures and direct the next GPT to inspect the corrected exact-head AI-doc run before resuming P1 source work.
- The current-main source baseline remains f5cc0baf; no firmware source changed. Root dev-log advances 111→112 records; no archive rotation or index update is due. New exact-head AI docs run is pending.

---

## 2026-11-15 — Correct handoff validator assertion after exact-head CI failure

- Exact-head AI handoff documentation run [43630358677] on docs commit f25b32fb639239351209335a039334f2c63b3bc4 failed because the new validator assertion expected uppercase “Do not claim” while the correctly punctuated bootstrap sentence used lowercase “do not claim” after a semicolon. This was a validator-test defect, not a documentation content or product-source failure.
- Corrected the assertion's expected case and clarified in SESSION_HANDOFF.md / REFACTOR_ROADMAP.md that f5cc0baf is the latest verified product-source baseline, while later documentation-only commits can advance live main without changing that engineering evidence.
- f25 exact-head process lifecycle [43630358776], infosvr [43630358668], AdGuardHome [43630358687] and AliyunDrive [43630358691] passed; AliDDNS [43630358673] passed. NVRAM [43630358670] was still running at last inspection. The corrected AI docs run on the new commit is pending.
- No firmware source changed. Root dev-log advances 110→111 records; no archive rotation or index update is due.

---

## 2026-11-15 — Repair stale cross-session handoff and bootstrap contract

- User-requested root handoff review found the live handoff and roadmap still described the watchdog-inclusive current-main Actions as pending and current L3 as unverified, although main is `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c` and all ten exact-head `push` workflows completed SUCCESS.
- Verified XY-C1 run [43625410221] passed Build XY-C1, TRX verification and artifact upload (artifact [14125764801], 12,583,902 bytes). Watchdog regression [43625410193], process lifecycle [43625410154], process inventory [43625410226], AI docs [43625410189], NVRAM [43625410190], AdGuardHome [43625410170], AliDDNS [43625410262], AliyunDrive [43625410196] and infosvr [43625410180] also passed on the exact same SHA. Prior dd7a XY-C1 run [43624870668] completed SUCCESS.
- Corrected SESSION_HANDOFF.md and REFACTOR_ROADMAP.md to reflect verified current-main state and set the next implementation task to PPP/PID-file targets in the all-target stop preflight. L4 hardware validation remains NOT VERIFIED.
- Fixed two malformed bootstrap sentences (a truncated documentation rule and an orphaned no-fabrication fragment). Clarified that an accurate handoff is not rewritten merely because a session ends; only material checkpoint changes justify updates.
- Extended `scripts/check_ai_handoff_docs.py` to reject the observed bootstrap truncation and unconditional handoff-churn contradiction. The preserved backup branch remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807; legacy PR #1 remains closed/unmerged.
- Root dev-log advances 109→110 records; no archive rotation or index update is due. This is a user-requested governance/handoff repair, not a product-source change. Exact-head AI documentation workflow is required after publication.

---

## 2026-11-15 — Migrate watchdog PID publication and signal consumers

- Base main SHA: dd7a9b42f6f12b1a585e09cb54a1d69c25f69877.
- watchdog_main() now publishes PID + kernel start_time through process_pidfile_write() after daemonization and aborts startup if the atomic identity record cannot be published.
- notify_watchdog_time() and notify_watchdog_wifi() now use process_pidfile_signal() with expected process name watchdog. The time path retains a pids() absence check before starting a replacement, so UNKNOWN cannot trigger a duplicate launch.
- Watchdog shutdown no longer unlinks WD_PID_FILE by pathname. A stale PID/start_time record is harmless and can be atomically replaced; unconditional cleanup unlink could race and remove a fast-restarted watchdog's new record.
- Extended watchdog signal regression assertions and push/pull_request path filters to cover process_pidfile.c/.h. Added watchdog.c to the XY-C1 trigger so the MIPS/uClibc RC compile validates the new integration.
- On base SHA dd7a, process-lifecycle [43624870654], process inventory [43624870697], rstats [43624870719], HTTPD [43624870704] and AI docs [43624870860] passed. XY-C1 [43624870668] was still IN_PROGRESS when this commit was prepared; this watchdog slice triggers a fresh target build and a watchdog-signal regression. No L3 result is claimed yet.
- Changed paths: trunk/user/rc/src/watchdog.c, watchdog-signal-stability.yml, xy-c1-firmware-validation.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log. Resulting commit: THIS COMMIT.
- Root log advances 108→109 records; no archive rotation or INDEX update is due.
- Next: inspect exact-head watchdog/XY-C1 Actions. Then migrate PPP/PID-file targets into the complete all-target stop preflight.

---

## 2026-11-15 — Fix rstats PID publication target compile error

- Base main SHA: 733463cb9e3289c62836dd1b7405a30ec1a424cc.
- Exact-head XY-C1 run 43622638409 failed in rstats_main() at rstats.c:722: the PID identity publication failure path referenced saved_errno, but that variable was declared only inside earlier nested error blocks.
- Added one function-scope saved_errno and reused it across signal setup, SIGPIPE setup, daemon setup and PID-file publication error handling. No PID lifecycle behavior changed.
- Exact-head process-lifecycle workflow 43622638410, HTTPD security workflow 43622638413 and AI docs workflow 43622638346 had passed on the failing base SHA; the XY-C1 build stopped before TRX generation. This correction triggers fresh rstats host/integration and MIPS target validation.
- Changed paths: trunk/user/rc/src/rstats.c, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log. Resulting commit: THIS COMMIT.
- Root log advances 107→108 records; no archive rotation or INDEX update is due.
- Next: inspect exact-head XY-C1 and process-lifecycle Actions for this correction. After PASS, migrate watchdog PID publication and both notification callers.

---

## 2026-11-15 — Update HTTPD bandwidth regression for rstats identity signaling

- Exact-head HTTPD security workflow `43622388659` failed only because its bandwidth refresh source assertion still required legacy `kill_pidfile_s` after the rstats PID consumer had migrated to `process_pidfile_signal()`.
- Updated the assertion to require the expected `rstats` process identity and reject the legacy call. Added shared PID helper source/header to HTTPD security workflow push and pull_request filters.
- No production source changed in this correction. The rstats-specific persistence/signal regression and shared process lifecycle regression passed.
- Changed paths: `.github/workflows/httpd-security.yml` and this log. Resulting commit: THIS COMMIT.
- Exact-head HTTPD security workflow will rerun.

---

## 2026-11-15 — Migrate rstats PID producer and RC/HTTPD signal consumers

- Base main SHA: `13b1f602ac8a04f070b6d7ce4a88b2e6a05f6166`.
- `rstats` now publishes PID + kernel start_time with `process_pidfile_write()`; failure frees initialized state, restores the signal mask and aborts startup.
- RC time refresh and HTTPD live/history refresh now call `process_pidfile_signal()` with expected process name `rstats`. No legacy numeric PID signaling remains for this daemon.
- Added source integration assertions and process-lifecycle/XY-C1 path filters.
- Shared PID helper host regression passed and XY-C1 exact-head build `43621293292` passed, including the fix for hidden `__udivdi3`. The detect_link-inclusive build remains in progress; this rstats slice triggers fresh host and target validation.
- Changed paths: `trunk/user/rc/src/rstats.c`, `trunk/user/httpd/src/web_ex.c`, integration checker, both workflows, handoff and this log. Resulting commit: THIS COMMIT.
- Root log advances 103→104 records; no archive rotation or INDEX update is due.
- Next: migrate watchdog PID producer and both RC signal consumers together.

---

## 2026-11-15 — Complete detect_link workflow dependency path filters

- Follow-up review found the first checker correction added PID helper paths only to push filters, not pull_request filters, and omitted them from the workflow's own path-contract tuple.
- Added process_pidfile and process_inventory source/header dependencies to both event filters and the contract tuple. This prevents the dedicated regression from silently skipping relevant helper changes.
- Production source and test behavior are unchanged.
- Changed paths: `.github/workflows/detect-link-signal-stability.yml` and this log. Resulting commit: THIS COMMIT.
- The corrected exact-head detect_link workflow will verify both caller wiring and path-filter symmetry.

---

## 2026-11-15 — Update detect_link regression for identity-bound PID helper

- Exact-head detect_link workflow `43621732208` failed only because its existing caller-wiring assertion still required legacy `kill_pidfile_s()` calls after the producer and both consumers were migrated to `process_pidfile_write/signal()`.
- Updated the workflow contract to require PID+start_time publication and expected-process identity signaling, reject legacy calls, and watch the shared helper source/header in push and pull_request path filters.
- The GNU89 signal deferral regression itself passed. Production code did not change in this correction.
- Changed paths: `.github/workflows/detect-link-signal-stability.yml` and this log. Resulting commit: THIS COMMIT.
- Exact-head detect_link workflow will rerun automatically.

---

## 2026-11-15 — Migrate detect_link PID producer and both signal consumers

- Base main SHA: `3dba5c47aa5986bb5530370ff1aa436ce70aa113`.
- `detect_link` now publishes PID + kernel start_time atomically via `process_pidfile_write()`; failure aborts daemon startup rather than running without an identity-bound record.
- Its SIGUSR1 refresh and SIGHUP LED-refresh callers now use `process_pidfile_signal()` with expected process name `detect_link`. No legacy numeric PID signal fallback remains for this daemon.
- Added production-source integration assertions and process-lifecycle/XY-C1 path filters.
- The shared helper's GNU89 host regression and the detect_internet migration's host workflows passed. Exact-head MIPS build for the helper's 64-bit overflow fix remains in progress; this commit triggers another target build with detect_link wired.
- Changed paths: `trunk/user/rc/src/detect_link.c`, integration checker, both workflows, handoff and this log. Resulting commit: THIS COMMIT.
- Root log advances 101→102 records; no archive rotation or INDEX update is due.
- Next: verify exact-head MIPS link, then migrate rstats producer and both signal consumers.

---

## 2026-11-15 — Remove MIPS 64-bit division helper from PID parser

- Exact XY-C1 build for `b2500167cbacbbf161df5126ee49c008d7164f42` failed at the final RC link: shared `process_pidfile.o` referenced hidden libgcc symbol `__udivdi3`.
- Root cause: runtime division of a 64-bit value by 10 in the start_time overflow check. Replaced it with the exact decimal cutoff for ULLONG_MAX, avoiding a 64-bit division helper in the shared object.
- Added a positive ULLONG_MAX boundary test alongside the existing overflow rejection case.
- Host parser regression passed before this MIPS-only link failure; exact-head process and target builds are required after this fix.
- Changed paths: `trunk/user/shared/src/process_pidfile.c`, `trunk/user/shared/src/tests/process_pidfile.c`, `SESSION_HANDOFF.md`, and this log. Resulting commit: THIS COMMIT.
- Root log advances 100→101 records; no archive rotation or INDEX update is due.
- Next: verify the MIPS shared-library/RC link, then proceed to detect_link producer/consumer migration.

---

## 2026-11-15 — Migrate detect_internet PID producer and all signal consumers

- Base main SHA: `b2500167cbacbbf161df5126ee49c008d7164f42`.
- `detect_internet` now publishes PID + kernel start_time via `process_pidfile_write()`; publication failure aborts daemon startup instead of silently running without a usable record.
- Both RC notification paths and HTTPD's SIGHUP hook now call `process_pidfile_signal()` with the expected process name. No legacy numeric-PID signal fallback remains for this daemon.
- Added integration assertions and process-lifecycle/XY-C1 workflow path filters for both RC and HTTPD source files.
- The shared helper/parser host regression passed on its exact head; this producer/consumer migration has new exact-head host and MIPS validation pending.
- Changed paths: `trunk/user/rc/src/detect_internet.c`, `trunk/user/httpd/src/web_ex.c`, integration checker, both workflows, handoff and this log. Resulting commit: THIS COMMIT.
- Root log advances 99→100 records; no archive rotation or INDEX update is due.
- Next: migrate detect_link producer and both signal consumers together.

---

## 2026-11-14 — autonomous process-lifecycle and storage continuation

Source baseline: `b022942bf30e6899c18d4bf40aaa528fb17a011f`.

Storage eject work: checked disk inventory, strict final mount inventory, valid-empty inventory fallback, firmware and shutdown persistence gates, and RC applet status propagation. Exact storage hotplug run `43652052169` on `9d85739a978d2f65d87bbe27f8d50f2bdbff3000`: `completed/success`.

GPIO tri-state source present: true. xl2tpd tri-state source present: true. Current exact-head Actions snapshot:
- XY-C1 Firmware Validation: in_progress (run 43653875470)
- Process lifecycle inventory - b022942bf30e6899c18d4bf40aaa528fb17a011f: in_progress (run 43653875506)
- USB storage hotplug stability regression: in_progress (run 43653875448)
- HTTPD Authentication and Security Regression: in_progress (run 43653875605)
- AI handoff documentation contract: in_progress (run 43653875488)

Next audited task: convert HTTPD start duplicate-launch and readiness gates in `trunk/user/rc/src/services.c` to explicit tri-state process inventory semantics; UNKNOWN must not pass as ready. Revalidate XY-C1 firmware build for current product-source baseline before claiming L3. L4 remains unverified.

## 2026-11-14 — process lifecycle continuation

Source baseline: `ff34f0bf97fa1eef9b2787e3aca292baecd726c3`.

Storage checked eject: not verified; GPIO tri-state startup: present; xl2tpd tri-state startup: present; HTTPD tri-state startup with refreshed post-launch scan: not verified.

Exact-head Actions:
- Process lifecycle inventory - ff34f0bf97fa1eef9b2787e3aca292baecd726c3: in_progress (#43654870945)

Next task: continue the existing process-lifecycle semantic review with the isolated service-start candidate in `trunk/user/rc/src/services_ex.c`; do not change global `pids()` semantics. L3 still requires a fresh XY-C1 build on the latest product-source baseline; L4 is not verified.
