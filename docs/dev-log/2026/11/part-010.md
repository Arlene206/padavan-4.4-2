## 2026-11-14 — process lifecycle continuation

Source baseline: `ff34f0bf97fa1eef9b2787e3aca292baecd726c3`.

Storage checked eject: not verified; GPIO tri-state startup: present; xl2tpd tri-state startup: present; HTTPD tri-state startup with refreshed post-launch scan: not verified.

Exact-head Actions:
- Process lifecycle inventory - ff34f0bf97fa1eef9b2787e3aca292baecd726c3: in_progress (#43654870945)

Next task: continue the existing process-lifecycle semantic review with the isolated service-start candidate in `trunk/user/rc/src/services_ex.c`; do not change global `pids()` semantics. L3 still requires a fresh XY-C1 build on the latest product-source baseline; L4 is not verified.

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

## 2026-11-15 — Remove MIPS 64-bit division helper from PID parser

- Exact XY-C1 build for `b2500167cbacbbf161df5126ee49c008d7164f42` failed at the final RC link: shared `process_pidfile.o` referenced hidden libgcc symbol `__udivdi3`.
- Root cause: runtime division of a 64-bit value by 10 in the start_time overflow check. Replaced it with the exact decimal cutoff for ULLONG_MAX, avoiding a 64-bit division helper in the shared object.
- Added a positive ULLONG_MAX boundary test alongside the existing overflow rejection case.
- Host parser regression passed before this MIPS-only link failure; exact-head process and target builds are required after this fix.
- Changed paths: `trunk/user/shared/src/process_pidfile.c`, `trunk/user/shared/src/tests/process_pidfile.c`, `SESSION_HANDOFF.md`, and this log. Resulting commit: THIS COMMIT.
- Root log advances 100→101 records; no archive rotation or INDEX update is due.
- Next: verify the MIPS shared-library/RC link, then proceed to detect_link producer/consumer migration.

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

## 2026-11-15 — Update detect_link regression for identity-bound PID helper

- Exact-head detect_link workflow `43621732208` failed only because its existing caller-wiring assertion still required legacy `kill_pidfile_s()` calls after the producer and both consumers were migrated to `process_pidfile_write/signal()`.
- Updated the workflow contract to require PID+start_time publication and expected-process identity signaling, reject legacy calls, and watch the shared helper source/header in push and pull_request path filters.
- The GNU89 signal deferral regression itself passed. Production code did not change in this correction.
- Changed paths: `.github/workflows/detect-link-signal-stability.yml` and this log. Resulting commit: THIS COMMIT.
- Exact-head detect_link workflow will rerun automatically.

---

## 2026-11-15 — Complete detect_link workflow dependency path filters

- Follow-up review found the first checker correction added PID helper paths only to push filters, not pull_request filters, and omitted them from the workflow's own path-contract tuple.
- Added process_pidfile and process_inventory source/header dependencies to both event filters and the contract tuple. This prevents the dedicated regression from silently skipping relevant helper changes.
- Production source and test behavior are unchanged.
- Changed paths: `.github/workflows/detect-link-signal-stability.yml` and this log. Resulting commit: THIS COMMIT.
- The corrected exact-head detect_link workflow will verify both caller wiring and path-filter symmetry.

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

## 2026-11-15 — Update HTTPD bandwidth regression for rstats identity signaling

- Exact-head HTTPD security workflow `43622388659` failed only because its bandwidth refresh source assertion still required legacy `kill_pidfile_s` after the rstats PID consumer had migrated to `process_pidfile_signal()`.
- Updated the assertion to require the expected `rstats` process identity and reject the legacy call. Added shared PID helper source/header to HTTPD security workflow push and pull_request filters.
- No production source changed in this correction. The rstats-specific persistence/signal regression and shared process lifecycle regression passed.
- Changed paths: `.github/workflows/httpd-security.yml` and this log. Resulting commit: THIS COMMIT.
- Exact-head HTTPD security workflow will rerun.

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

## 2026-11-15 — Repair stale cross-session handoff and bootstrap contract

- User-requested root handoff review found the live handoff and roadmap still described the watchdog-inclusive current-main Actions as pending and current L3 as unverified, although main is `f5cc0bafdfa017e9019dbda991a3855a84bc2e1c` and all ten exact-head `push` workflows completed SUCCESS.
- Verified XY-C1 run [43625410221] passed Build XY-C1, TRX verification and artifact upload (artifact [14125764801], 12,583,902 bytes). Watchdog regression [43625410193], process lifecycle [43625410154], process inventory [43625410226], AI docs [43625410189], NVRAM [43625410190], AdGuardHome [43625410170], AliDDNS [43625410262], AliyunDrive [43625410196] and infosvr [43625410180] also passed on the exact same SHA. Prior dd7a XY-C1 run [43624870668] completed SUCCESS.
- Corrected SESSION_HANDOFF.md and REFACTOR_ROADMAP.md to reflect verified current-main state and set the next implementation task to PPP/PID-file targets in the all-target stop preflight. L4 hardware validation remains NOT VERIFIED.
- Fixed two malformed bootstrap sentences (a truncated documentation rule and an orphaned no-fabrication fragment). Clarified that an accurate handoff is not rewritten merely because a session ends; only material checkpoint changes justify updates.
- Extended `scripts/check_ai_handoff_docs.py` to reject the observed bootstrap truncation and unconditional handoff-churn contradiction. The preserved backup branch remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807; legacy PR #1 remains closed/unmerged.
- Root dev-log advances 109→110 records; no archive rotation or index update is due. This is a user-requested governance/handoff repair, not a product-source change. Exact-head AI documentation workflow is required after publication.

---

## 2026-11-15 — Correct handoff validator assertion after exact-head CI failure

- Exact-head AI handoff documentation run [43630358677] on docs commit f25b32fb639239351209335a039334f2c63b3bc4 failed because the new validator assertion expected uppercase “Do not claim” while the correctly punctuated bootstrap sentence used lowercase “do not claim” after a semicolon. This was a validator-test defect, not a documentation content or product-source failure.
- Corrected the assertion's expected case and clarified in SESSION_HANDOFF.md / REFACTOR_ROADMAP.md that f5cc0baf is the latest verified product-source baseline, while later documentation-only commits can advance live main without changing that engineering evidence.
- f25 exact-head process lifecycle [43630358776], infosvr [43630358668], AdGuardHome [43630358687] and AliyunDrive [43630358691] passed; AliDDNS [43630358673] passed. NVRAM [43630358670] was still running at last inspection. The corrected AI docs run on the new commit is pending.
- No firmware source changed. Root dev-log advances 110→111 records; no archive rotation or index update is due.

---

## 2026-11-15 — Anchor bootstrap orphan-fragment check to a full line

- Exact-head AI handoff run [43630474661] on e1af9e9d797a6eb015aeb924b8c4da75f457b128 failed because the orphan-fragment detector used substring matching: the valid sentence “do not claim tests” contains the substring “aim tests”. This was a second validator false positive, not malformed bootstrap content.
- Changed the check to a multiline, line-anchored regular expression matching only a standalone orphan line. Updated SESSION_HANDOFF.md to record both validator-only failures and direct the next GPT to inspect the corrected exact-head AI-doc run before resuming P1 source work.
- The current-main source baseline remains f5cc0baf; no firmware source changed. Root dev-log advances 111→112 records; no archive rotation or index update is due. New exact-head AI docs run is pending.

---

## 2026-11-15 — Make end-of-session handoff checkpoint mandatory without document churn

- User-requested governance correction: every GPT session must perform and complete a handoff accuracy checkpoint before ending; the previous wording could be read as making the check optional or deferring it to the next conversation.
- Clarified that the checkpoint verifies engineering/source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. Missing/stale/changed next-session-critical facts require one consolidated handoff update and commit; an already accurate handoff requires no edit and no no-op commit.
- Defined relevant CI transitions: PENDING→PASS/FAIL must be recorded when they close/change the active validation gate or next action; unrelated workflow noise and repeated polling are not documentation milestones. Source/test/build work remains the default, and handoff-only commits are limited to material continuity corrections that cannot safely wait.
- Updated bootstrap, development protocol, current handoff and validator together. No firmware source or roadmap priority changed; the first engineering task remains PPP/PID-file inclusion in the all-target stop preflight. Root dev-log advances 112→113 records; no archive rotation or index update is due. Exact-head AI documentation Actions must validate this commit.

---

## 2026-11-15 — Remove pending-CI gate that blocked independent engineering

- Cross-file rule review found a direct contradiction: bootstrap step 5 and protocol recovery text required resolving pending CI before selecting another task, while the continuous-execution section said not to burn the session polling pending runs. This contradicted the user's explicit PENDING→continue independent work rule and could stall every GPT session.
- Changed the rule so the active engineering task resumes immediately; a relevant FAILED check must be triaged before its slice is called validated, but PENDING is not a blocker. Record the exact run ID/head SHA, continue safe independent implementation, and revisit at a useful checkpoint or before handoff. Never idle-wait or repeatedly poll.
- Updated the handoff's exact first action to inspect AI-doc run 43631988702 for SHA 9ff6a875d31bbe832bac4ada583c7519d2a993ee, while explicitly continuing PPP/PID-file work if it remains pending. Added validator assertions to prevent regression to the old pending-CI gate.
- This is a newly discovered, independently material continuity defect found during the user-requested cross-file review; it is a second correction to the same governance request, not a routine SHA/status refresh. No firmware source changed. Root dev-log advances 113→114 records; no archive rotation or index update is due.

---

## 2026-11-15 — Eliminate remaining pending-CI recovery contradictions

- Final cross-file search after the previous correction found two additional legacy priority/recovery sentences in the protocol (global priority selection and Section 10 recovery step) that still treated pending exact-head CI as a prerequisite to choosing new work. These contradicted the new explicit rule and could reintroduce the same stall in a later GPT.
- Rewrote both: resume the active engineering task; FAILED relevant CI is triaged before claiming validation; PENDING is recorded with run ID/head SHA and does not block independent safe work. No idle waiting or repeated polling.
- Updated the handoff to record that AI-doc run 43631988702 on SHA 9ff6a875d31bbe832bac4ada583c7519d2a993ee passed, and to inspect run 43632050498 on exact SHA 812b335bb5fc234eb2f40f5489eec8a80479acf9 while continuing PPP/PID-file implementation if pending.
- This is a separate, newly found contradictory instruction in the authoritative protocol, not a cosmetic follow-up. No firmware source changed. Root dev-log advances 114→115 records; no archive rotation or index update is due.

---

## 2026-11-15 — Fix case-sensitive pending-CI validator assertion

- Exact-head AI handoff run [43632050498] on SHA 812b335bb5fc234eb2f40f5489eec8a80479acf9 failed at the new bootstrap assertion. The bootstrap correctly said “a PENDING check is not a stop condition”; the validator incorrectly required uppercase “A PENDING”.
- Corrected the assertion to compare normalized lowercase text, preserving the required semantic check while avoiding the case-sensitive false negative. The failure was in the validator, not the handoff policy.
- Updated SESSION_HANDOFF.md with the exact failing run/SHA, cause, correction and next exact-head validation action. No firmware source changed. Root dev-log advances 115→116 records; no archive rotation or index update is due.

---

## 2026-11-15 — Close XY-C1 build-trigger gaps for process identity sources

- The XY-C1 workflow's path filter did not include several production inputs changed by the identity-lifecycle migration: BusyBox udhcpc, PPPD identity PID publication, RC common/WAN/LAN/VPN/USB/iNIC lifecycle sources, and the shared PID-record header. A later change to one of these files could therefore miss the MIPS/uClibc build.
- Added the missing production paths to the XY-C1 push trigger. The process-lifecycle workflow now watches the XY workflow itself, and its integration checker asserts coverage for the affected production inputs.
- Exact-head evidence before this correction: process inventory/lifecycle [43647169443], iNIC signal lifecycle [43647169424], wireless radio state [43647169416] and AI handoff docs [43647325253] passed on their respective exact SHAs. HTTPD run [43647169410] failed on 946a8719 because its assertion still expected the old interface-only WAN gate; b98bbbb corrected it to the combined modem/interface gate, and rerun [43647325290] was pending. XY-C1 run [43647325231] was pending on b98bbbb and covered the latest source tree; this commit also changes the XY workflow and triggers a fresh exact-head target build.
- L3 is not claimed until the new XY-C1 run passes build, TRX verification and artifact upload. L4 hardware validation remains NOT VERIFIED. Root log now contains 117 records; no archive rotation is due.
- Resulting change: this commit.

---

## 2026-11-15 — Align networkmap and VPN lifecycle contracts with aggregated stop results

- Exact-head CI on `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286` exposed two stale source assertions after `stop_services()` was changed from ignored direct stops to explicit status aggregation. The networkmap checker expected the literal statement `stop_networkmap();`; the VPN checker expected `stop_vpn_server();`. Production now calls each through `record_service_stop_status()` and returns the aggregate, so the assertions were false negatives rather than runtime failures.
- Updated both checkers to require the exact named stop result to be aggregated and `stop_services()` to return the aggregate. This preserves the stronger fail-closed contract instead of weakening it to a mere symbol-presence check.
- Failed exact-head runs: networkmap [43648229545] (assertion at `scripts/check_networkmap_signals.py:142`) and VPN server [43648229565] (assertion at `scripts/check_vpn_server_lifecycle.py:122`). Both will rerun on this test-only fix. On the same SHA, process inventory [43648229499], AI handoff docs [43648229603], HTTPD lifecycle [43648229544], NFS start/reload [43648229491], and Samba share [43648229471] passed; XY-C1 [43648229489] and several independent workflows were still PENDING.
- The next product slice remains storage eject failure propagation: `safe_remove_all_stor_devices()` currently returns void, hiding lock/daemon/inventory/unmount failures from firmware flash and shutdown. The checked disk inventory must distinguish a valid empty device list from a failed `/proc/partitions` read before allowing persistence or flash.
- Root `dev_log.md` now contains 118 newest-first records; no archive rotation is due.
- Resulting change: this commit.

---

## 2026-11-15 — Correct networkmap stop prototype assertion after exact-head failure

- Exact-head networkmap push [43648923325] on `7697c6fd6664bb57019ae610bca642c73d88a8d9` passed the GNU89/ASan/UBSan signal-policy binary, then failed at `scripts/check_networkmap_signals.py:147`: the checker expected `void stop_networkmap(void);`, but `rc.h` correctly declares `int` and `services.c` returns the `kill_services()` result. This was a stale source-contract assertion, not a production defect.
- Updated the checker to require `int stop_networkmap(void);`, retaining the separate assertions that the stop result is aggregated in `stop_services()` and returned. The prior networkmap assertion failures on cc61 are resolved by the 7697 checker update; VPN rerun [43648923434] passed. AI docs [43648923326], process inventory [43648923376], AdGuardHome [43648923421], AliDDNS [43648923338], infosvr [43648923306] and AliyunDrive [43648923366] passed on exact 7697; NVRAM [43648923357] was still IN_PROGRESS.
- XY-C1 L3 is VERIFIED on product-source SHA `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286`: run [43648229489] passed firmware build, TRX verification and artifact upload [14135711084] (12,587,100 bytes; SHA-256 `bda83c9e71ed01a8708ad90a5104535e972bd30424bdf03ed24065b3a1a9407e`). No product source changed in the later 7697 handoff-only commit or this checker correction. L4 remains NOT VERIFIED.
- Changed paths: `scripts/check_networkmap_signals.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log. Resulting commit: THIS COMMIT.
- Root `dev_log.md` advances 118→119 records; no archive rotation or index update is due.

---

## 2026-11-15 — Gate firmware flash and shutdown persistence on verified storage eject

- `read_disk_data()` returned NULL for both a valid empty disk list and an unreadable/truncated `/proc/partitions` snapshot. Its parser also ignored disk-object allocation failure, allowing a partial inventory to appear usable. Added `read_disk_data_checked()`: status 0 means the full inventory is complete (including a valid empty result); -1 means unknown/incomplete. It checks header/read/close errors, rejects truncated or malformed data rows, and fails on disk/partition allocation errors while preserving the original pointer-return API as a compatibility wrapper.
- Changed `safe_remove_all_stor_devices()` to return status. It now stops before unmount on lock, service-stop or inventory errors; propagates per-device/fallback unmount failure; and requires a final complete `/proc/mounts` inventory with zero storage mounts. Added a host-testable fail-closed postcondition policy.
- Firmware flashing now aborts before storage persistence and `mtd_write` when eject is incomplete. Normal shutdown/reboot still continues, but skips persistent storage MTD writes when safe eject cannot be proven complete.
- Added source-contract checks and push/pull path coverage for the checked libdisk API and policy; current exact-head storage regression and XY-C1 MIPS build are triggered by this production-source commit. L3 must be re-established on the exact new source SHA; no L4 hardware validation is claimed.
- Root `dev_log.md` reached 120 entries and was rotated in this commit: the oldest 100 complete records are preserved byte-for-byte in `part-009.md`; the newest 20 remain in root.
- Resulting source commit: this commit.

---

## 2026-11-15 — Propagate eject-all applet failure to command status

- Final caller audit found the `ejall` RC applet still discarded the new `safe_remove_all_stor_devices()` result. It now assigns the result to the applet's `ret`, so shell callers receive a nonzero status when storage eject is incomplete instead of a false success.
- Extended the storage integration checker to require that result propagation. This follows the checked inventory/eject implementation on parent SHA `c2b393dba16ec8ab821fe0eea8e4e3e4a5a74bc6`; exact-head storage-hotplug run [43650708280] was QUEUED and XY-C1 [43650708273] IN_PROGRESS when audited. The follow-up commit triggers exact-head reruns; current-source L3 remains PENDING.
- Root `dev_log.md` now contains 21 newest-first records, below the 120-entry rotation threshold.
- Resulting change: this commit.

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

## 2026-11-15 — Diagnose intermittent AliDDNS concurrent resolver isolation failure

- Parent main SHA: `b740103a92cdc0a2d81f849f36d37307c4e0484b`.
- Crond's exact-head repair was completed by the parallel follow-up on `74be177adb256128c79018ad58c7ae36deed0191`: its lifecycle regression [43654459854] and AI-doc contract [43654459868] passed. HTTPD lifecycle [43654668592] and its AI-doc contract [43654668591] also passed on `b740103a92cdc0a2d81f849f36d37307c4e0484b`; the production HTTPD start gate already uses RUNNING/ABSENT/UNKNOWN before NVRAM/config mutation, and its policy tests cover UNKNOWN/invalid states.
- AliDDNS failed the same concurrent lookup assertion twice: [43653917007] on docs-only `ed7acbb`, then [43654226754] on `7911849`. In the second run the first and second lookup failures alternated (the test emitted only a generic expected-value failure), while API transport and address validation steps passed. Added a private mock-`mkdir` trace and actual a/b output values to the existing regression failure message. This preserves the assertion and should distinguish directory collisions from resolver timeout/parse failures on the next exact-head run.
- No production resolver behavior was changed in this diagnostic slice because the two CI logs did not identify the cause. A local shell simulation of the atomic-mkdir design passed 20 consecutive concurrent pairs; it is not a substitute for the target CI reproduction.
- L3 remains NOT VERIFIED for current source; L4 remains NOT VERIFIED.
- Result: THIS COMMIT.

---

## 2026-11-14 — Strict storage inventory, daemon startup gating, and lifecycle CI repairs

- `c5ce74a61c80`: replaced permissive `/proc/partitions` parsing with canonical header/separator validation, exact token counts, strict unsigned decimal and overflow-safe parsing. Added GNU89 regression for valid SCSI/MMC rows, malformed/truncated records, signed tokens, and UINT/ULLONG overflow.
- The parser Action initially stopped before the new test because BusyBox DHCPv6 used `writev()` without including `<sys/uio.h>`. `b022942` added the declaration and source contract; subsequent exact storage-hotplug status is recorded above.
- `ea2163e`: migrated HTTPD duplicate-start/readiness checks from ambiguous `pids()` truthiness to tri-state `process_inventory_scan()`. UNKNOWN now blocks config/file mutations and launch; only confirmed RUNNING publishes daemon state. Added policy tests for UNKNOWN, ABSENT, RUNNING and launcher failures.
- The first HTTPD lifecycle run failed in its embedded checker because it expected the old success branch. `b740` updated the checker to assert failed readiness returns before publishing started state.
- Crond CI exposed a missing Python `pids` source binding (NameError); `74be` adds the binding and path-filter coverage for `pids.c`.
- AliDDNS's prior exact run failed the second concurrent resolver output assertion. The same isolated-directory implementation passed local dash/BusyBox-ash reproduction; the regression was expanded to eight concurrent pairs with diagnostics to distinguish a race from a flaky runner. Await its exact-head result before calling it fixed.
- All source and workflow writes used complete atomic Git trees and non-force expected-SHA updates. No router L4 validation is claimed.

---

## 2026-11-15 — Fix AliDDNS concurrent lookup scratch-directory collision

- Parent main SHA: `87292f784af267dc5b97ea071815af1e0ee26c44`.
- Exact-head AliDDNS run [43655110946] failed reproducibly on `87292f7`: both concurrent calls returned `203.0.113.11`; diagnostics showed both calls using `/tmp/arNslookup.2741.0`. The production allocator derived its directory name from `$$` inside a POSIX-shell subshell. POSIX shells retain the invoking shell PID in `$$` across subshells, so simultaneous calls shared one directory and could read each other's resolver output.
- Replaced the inherited-shell PID seed with the PID of a short-lived external `sh -c` helper, then retained the exclusive `mkdir` retry loop and restrictive umask. The test clears its trace before the concurrent pair, fixes the logger's literal-`$` typo, and now requires two distinct successful directories plus the independent A/B resolver answers.
- The source contract now requires the helper PID and the two-directory assertion. No unsupported BusyBox `mktemp` dependency was added (`CONFIG_MKTEMP` is disabled in this router config).
- Exact 87292 Actions: AI docs [43655110888], process inventory [43655110892], AdGuardHome [43655110906], infosvr [43655110865], AliyunDrive [43655110878] and NVRAM [43655110876] passed. AliDDNS [43655110946] failed at the concurrent lookup case. New exact-head AliDDNS CI must verify this repair.
- Result: THIS COMMIT.

---

## 2026-11-15 — Make DNSMasq detached-child readiness tri-state and fail closed

- Parent main SHA: `06355c57f002132031a50ec56596a6a1a74a2b4f`.
- DNSMasq daemonizes before the child is ready. `dnsmasq_confirm_started()` previously used `is_dns_dhcpd_run()` (`pids()`) as a Boolean, so an incomplete process-table scan could be mistaken for a confirmed running daemon.
- Added `dnsmasq_process_presence()` around the shared scanner, preserving scan errno before releasing the snapshot. Startup now accepts only RUNNING, retries only confirmed ABSENT, and logs the scan error then aborts immediately on UNKNOWN. Policy returns ESRCH for confirmed ABSENT and EIO for UNKNOWN/invalid enum values; launcher errors retain precedence.
- Extended GNU89 policy tests and the DNSMasq workflow's embedded source contract. The workflow now watches the shared scanner C/H and compiles the policy test with its include path. XY-C1 now watches `services_ex.c`, previously missing from its firmware trigger filter.
- AliDDNS exact-head fix [06355c57] was IN_PROGRESS at last inspection; its parent failure [43655110946] diagnosed the shared-`$$` scratch-directory collision. Crond [43654459854] and HTTPD [43654668592] lifecycle checks passed on their exact source commits. Current-source L3 and physical L4 remain NOT VERIFIED.
- No local checkout is available; no local compiler or MIPS result is claimed. Exact-head DNSMasq and XY-C1 Actions are required.
- Result: THIS COMMIT.

---

## 2026-11-15 — Correct the AliDDNS helper-shell quoting

- Parent main SHA: `59853da9688974ee733d851785cd530d09c4c2db`.
- The first unique-PID patch still failed [43655773437]: the nested `sh -c` command string was missing its closing double quote around `$$`, so the helper shell reported an unterminated quoted string. The parent file's `sh -n` passed because the unmatched quote was consumed by later source text; runtime execution exposed it.
- Corrected the line to `lookup_owner_pid=$(sh -c 'printf "%s\\n" "$$"') || exit 1`. Also fixed the mkdir diagnostic logger to record the actual wrapper PID (`$$`, not a literal `$`). The test already clears the mkdir trace before the two parallel calls and requires two distinct successful scratch directories plus separate A/B results.
- Exact 59853 checks: DNSMasq lifecycle [43655773415], process inventory [43655773421], process lifecycle [43655773597], infosvr [43655773430], AdGuardHome [43655773400], miniupnpd [43655773467], Samba [43655773461] and AliyunDrive [43655773636] passed. AliDDNS [43655773437] failed at the nested shell command; HTTPD auth/security [43655773458] failed separately and is to be triaged. XY-C1 [43655773404] and NVRAM [43655773564] were still running at last inspection.
- Result: THIS COMMIT.

---

## 2026-11-15 — Repair nested shell command syntax in AliDDNS PID allocator

- Parent main SHA: `1091e2737d7886c60f529e3a9c6d3626a01e9717`.
- The exact 59853 AliDDNS job [43655773437] showed the previous commit's `sh -c` argument still had an unterminated double quote. The shell parser consumed later text, so static `sh -n` alone did not establish that the runtime helper command was correct.
- Corrected the exact command to `lookup_owner_pid=$(sh -c 'printf "%s\\n" "$$"') || exit 1`, with both the helper argument and PID expansion properly quoted. Corrected the test's mkdir wrapper to record `$$` rather than a literal dollar sign. The concurrency regression still requires two distinct successful directories and separate resolver answers.
- The previous DNSMasq lifecycle regression [43655773415], process inventory [43655773421], process lifecycle [43655773597], infosvr [43655773430], AdGuardHome [43655773400], miniupnpd [43655773467], Samba [43655773461] and AliyunDrive [43655773636] all passed on exact SHA `59853da`. HTTPD auth/security [43655773458] failed separately; it remains to be triaged. XY-C1 [43655773404] and NVRAM [43655773564] were pending.
- Result: THIS COMMIT.

---

## 2026-11-15 — Fix HTTPD security workflow dependency for DNSMasq policy

- Parent main SHA: `e2ae222bdacb7a628bc59c82cc2bba482ef1e3e3`.
- Exact 59853 HTTPD Authentication and Security workflow run [43655773458], job [132633829952], failed while compiling the DNSMasq restart policy regression: `dnsmasq_restart_policy.h` now includes the public `process_inventory.h`, but this second workflow compiled the test with only `-I trunk/user/rc/src`.
- The dedicated DNSMasq workflow already passed on 59853 [43655773415], confirming its compile command had the shared include directory. Added the same `-I trunk/user/shared/src` dependency to the HTTPD security workflow's embedded DNSMasq compile. This is a CI integration defect, not an HTTPD production regression.
- On exact e2ae222 main, AliDDNS [43656006432], AliyunDrive [43656006474], process lifecycle [43656006468], AdGuardHome [43656006400], infosvr [43656006452] and AI docs [43656006411] passed; NVRAM [43656006425] remained IN_PROGRESS at the last poll.
- Result: THIS COMMIT.

---

## 2026-11-15 — Remove variable 64-bit division from disk inventory parser

- Parent main SHA: a7bfbdf6de475f46469cc19661265b4801306f0f.
- XY-C1 [43654318351] on ea2163e failed the final RC link because disk_inventory_parse_decimal() divided an unsigned long long value by a variable 64-bit bound, adding a hidden __udivdi3 dependency from libdisk.so.
- Reworked strict decimal parsing to check the fixed ULLONG_MAX decimal cutoff 1844674407370955161 (final digit <=5) before multiply/add, then compare each safe value to the caller's UINT_MAX/ULLONG_MAX limit. No division remains in the parser.
- Added a valid maximum /proc/partitions row regression for UINT_MAX major/minor and ULLONG_MAX blocks. The storage integration checker now forbids reintroducing /10ULL or /10UL division. Existing storage and XY-C1 workflows already watch the relevant parser files.
- Local GNU89 -Wall -Wextra -Werror ASan/UBSan parser test: PASS. Exact-head storage and XY-C1 Actions from this commit are pending.
- At parent a7bfbdf, AliDDNS [43656161389], HTTPD security [43656161273], process inventory [43656161404], AdGuardHome [43656161276] and infosvr [43656161384] passed; NVRAM [43656161302] and AliyunDrive [43656161349] were still pending. Current L3 remains PENDING; L4 remains NOT VERIFIED.
- Result: THIS COMMIT.

---

## 2026-11-15 — Fail closed on DNSMasq preflight and post-stop state

- Parent main SHA: `3ca4126c209e5a76dad3d0802d02972b7e157172`.
- After detached-child readiness was migrated, DNSMasq still used Boolean `is_dns_dhcpd_run()` guards before configuration preparation, again before launch, and after stopping for restart. Those checks could collapse an incomplete procfs inventory into a false presence result.
- Replaced both start guards with shared-inventory preflight: only complete ABSENT allows preparation or launch; RUNNING returns EBUSY; UNKNOWN/invalid returns EIO or captured scan errno before config mutation. Restart now snapshots initial presence before preparation and aborts on UNKNOWN, then requires a final complete ABSENT snapshot after stop before any replacement launch. Stop errors retain precedence; RUNNING refuses duplicate launch and UNKNOWN fails closed.
- Added GNU89 enum-policy tests for ABSENT/RUNNING/UNKNOWN/invalid values and stop-error precedence. Updated DNSMasq workflow source contract to assert scan-before-prepare ordering, second scan before launch, restart pre/post-stop gates, and no Boolean checks in restart.
- Latest parent `3ca4126` adds strict storage inventory validation; exact-head Actions were still pending on its source at the last inspection. The DNSMasq detached-readiness workflow had passed on its earlier exact source; this guard change triggers a fresh DNSMasq regression. Current-source L3 and physical L4 remain NOT VERIFIED.
- No local compiler or firmware build is claimed; exact-head DNSMasq and XY-C1 Actions are authoritative.
- Result: THIS COMMIT.

---

## 2026-11-15 — Fail closed on UNKNOWN before WAN DHCP renewal signal

- Parent main SHA: 322c17f9ca546b6578304966f017a24c944dd883.
- Semantic audit found detect_wan.c::chk_udhcpc() relied on !pids("udhcpc"). The legacy API reports scan UNKNOWN as a truthy nonzero result, so incomplete /proc inventory could authorize killall -SIGUSR1 udhcpc.
- Added detect_wan_udhcpc_policy.h: only PROCESS_PRESENCE_RUNNING authorizes renewal, ABSENT is a no-op, and UNKNOWN/invalid returns EIO. Production calls process_inventory_scan(), captures snapshot.error before release, refuses signal on UNKNOWN and propagates failure through poll_gateway().
- Added GNU89 policy tests for RUNNING, ABSENT, UNKNOWN and invalid enum values; added source integration contracts for scan/error/release/signal order and no legacy pids("udhcpc"); expanded process regression and XY-C1 path filters.
- Local cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined policy regression: PASS (WAN DHCP renewal tri-state policy: PASS).
- At parent 3ca4126, storage, AliDDNS, HTTPD, process inventory/lifecycle, AdGuardHome, infosvr, AliyunDrive and NVRAM Actions were SUCCESS; XY-C1 [43656299052] remained IN_PROGRESS at Build. Parent 322c17f DNSMasq presence-gate Actions were PENDING at last poll. L3 and L4 remain NOT VERIFIED.
- Result: THIS COMMIT.

---

## 2026-11-15 — Correct WAN caller integration-check delimiters

- Parent main SHA: 09268ce8c68d7e884b02244558a92da017d9a3e4.
- Static review of the new process integration checker found its function-boundary delimiters were double-escaped, so its Python split would search for literal backslash-n instead of C source newlines. Corrected both chk_udhcpc() and poll_gateway() boundary strings.
- Added detect_wan.c and detect_wan_udhcpc_policy.h to the checker’s XY-C1 target-source trigger contract; the XY workflow already watches both paths.
- Source/test implementation remains unchanged. Exact process lifecycle Action [43656932626] had completed its scanner and generic lifecycle tests and was still running at the strict LPD PID-file test; the new WAN policy and integration steps were pending. Current exact process workflow will rerun on this checker correction.
- Result: THIS COMMIT.

---

## 2026-11-15 15/11/2026, 08:50 CST — Correct Samba policy regression and restore complete root development log

- Parent main SHA: 6052c4aaad370b67f529485b8effa14350435f2b.
- Exact-head Samba share-stability run [43659761190] exposed a faulty EPERM fixture: the production policy deliberately validates two captured identities twice (full-set preflight, then immediate per-PID revalidation), but the fake returned only two RUNNING states. Expanded the fixture to four RUNNING states so the signal callback is reached and EPERM propagation is tested without emitting a signal.
- The root `dev_log.md` had been accidentally reduced to a single entry by an earlier handoff update. Restored its complete pre-slice contents from main SHA `525c046d658ec4e3de0fd62f25c133dec603bbd9` and prepended this correction record; no earlier records were discarded.
- Exact-head Samba and documentation workflows must be rechecked on this correction commit. The previous Samba run [43659761190] failed; this correction fixes its EPERM test setup. No local compiler is claimed.
- L3/L4 remain NOT VERIFIED.
- Result: THIS COMMIT.

---

## 2026-11-15 — Stabilize Samba source-contract whitespace and handoff

- Parent main SHA: f4e78554c490c28697fc7819de8ba4947ab6eb48.
- Exact f4 Samba run [43660915754] passed every C regression executable (config file, passdb policy, paired transaction, tri-state daemon readiness and identity-bound process signaling) and failed only in the Python source-contract step at line 150. Production's compact C expression was semantically correct; the assertion expected spaced tokens. The check now removes whitespace before comparing the full helper-call contract.
- The f4 source addition adds a tested samba_process_require_absent() policy (ABSENT succeeds, RUNNING returns EBUSY, UNKNOWN preserves scan errno / EIO, invalid states return EIO) and verifies smbd/nmbd/(SMBD36 wsdd2) absence after the Samba stop before mount scan/restart. Fastpath uses full process inventory and leaves acceleration disabled on UNKNOWN.
- AI handoff [43660915715], process inventory [43660915717], NFS [43660915753], vsftpd [43660915711] and HTTPD [43660915835] passed on f4. XY-C1 [43660915762], storage [43660915718] and NVRAM [43660915714] were IN_PROGRESS at last poll. This commit changes only the Samba workflow contract and handoff/roadmap/log; new exact-head Samba and docs runs must be checked. Current L3/L4 NOT VERIFIED.
- Result: THIS COMMIT.

---

## 2026-11-14 — Exact-head validation and cross-session handoff checkpoint

- Parent main SHA: 4d1cc2d8f85839b13f1d4fcab9f17b173e94a1d5.
- Exact-head Actions at checkpoint: ai-handoff-docs.yml#43663194707=success; zerotier-lifecycle.yml#43663194683=failure.
- Updated the handoff and roadmap to record the live CI state, including pending/failed distinctions, and to preserve the immutable backup SHA 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Source state carried forward: Samba uses tri-state daemon process inventory and identity-bound operations; USB printer, IPTV/IGMP and optional shell-backed stop failures propagate; firmware refuses to proceed after required teardown failures; shutdown continues reboot/halt while skipping storage persistence after checked stop failures.
- Lifecycle source contracts now reflect status-returning stop helpers and guarded stop/start wrappers; the production first-failure accumulator has GNU89/ASan/UBSan regression coverage. Current-source L3 and physical L4 are NOT VERIFIED.
- Result: THIS CHECKPOINT COMMIT.

---

## 2026-11-14 — Extend fail-closed process inventory to service restarts and storage daemons

- Networkmap now distinguishes RUNNING / ABSENT / UNKNOWN. The first-call start path launches only on confirmed ABSENT; restart refreshes only a complete RUNNING snapshot and preflights/revalidates captured PID/start-time identities before targeted SIGUSR1. Broad `killall -SIGUSR1 networkmap` was removed. Regression covers unknown snapshots, multi-process identity checks, ESRCH/EPERM and no-signal guarantees.
- Infosvr restart now confirms process absence with `process_inventory_scan` after stop; UNKNOWN and still-running state both block replacement. SSH restart similarly gates startup on successful stop plus confirmed ABSENT and propagates launch result; UNKNOWN before/after causes conservative firewall reconciliation.
- Telnetd manual restart and PID1 notification now abort start on failed stop and merge both stop/start results.
- Storage daemons (vsftpd, minidlnad, mt-daapd, transmission-daemon, aria2c) now preserve UNKNOWN from process inventory; UNKNOWN blocks start, FTP stop verifies absence before eject, and firewall comparison treats UNKNOWN as a state requiring reconciliation. FTP wait regression now tests initial and confirmation UNKNOWN cases.
- Optional SCUT, MentoHUST, VLMCSd and iperf3 wrappers use tri-state process inventory. Stop status is successful only after command success and confirmed ABSENT; start command results propagate through restart wrappers.
- Storage workflow path filters had malformed duplicated push paths and were rebuilt with clean push + pull_request lists. PID1 workflow checker was refreshed for the checked telnet result; an isolated policy regression was added for optional-daemon stop status.
- Checkpoint main: `f0df8c0b2a9f9e07178e733e9945e6fa325b8f75`. Exact-head Actions: XY-C1 Firmware Validation #43666310042: in_progress; WAN gateway ARP probe socket stability regression #43666310087: success; VPN server lifecycle stability regression #43666310138: success; DDNSTO lifecycle stability regression #43666310183: success; watchdog signal lifecycle stability regression #43666310126: success; AI handoff documentation contract #43666310150: success; detect_link signal lifecycle stability regression #43666310000: success; Crond daemon lifecycle stability regression #43666309991: success; ZeroTier lifecycle stability regression #43666310180: success; DNS Forwarder launch path stability regression #43666310155: success; AliyunDrive WebDAV lifecycle stability regression #43666310178: success; infosvr packet and signal lifecycle stability regression #43666310088: success; GPIO button daemon startup and press timing stability regression #43666310152: success; networkmap signal lifecycle stability regression #43666310027: success; detect_internet signal lifecycle stability regression #43666310137: success; AdGuardHome config security regression #43666310192: success; HTTPD daemon lifecycle stability #43666310034: success; Adbyby download and dnsmasq configuration stability regression #43666309974: success; Process lifecycle inventory - f0df8c0b2a9f9e07178e733e9945e6fa325b8f75 #43666310148: success; USB storage hotplug stability regression #43666309989: failure; FRP asynchronous startup lifecycle regression #43666310162: success; SkipDBv2 Regression #43666310085: success; AliDDNS signed API transport security regression #43666310250: success; HTTPD Authentication and Security Regression #43666310117: success; NVRAM transaction boundary audit #43666309980: success; .github/workflows/rc-notification-queue-stability.yml #43666308732: failure. Treat PENDING as pending; L3 NOT VERIFIED until exact-source XY-C1/TRX/artifact passes; L4 NOT VERIFIED without hardware evidence.
- No local checkout, cross-compile, or physical router validation is claimed.

---

## 2026-11-15 10:12 CST — Repair PID 1 workflow trigger and regression integration

- Exact-head run `43666576198` on `ff59679e8114b2b6156c3f235c9ff4a665d4d802` failed before creating a job (GitHub reports zero jobs). The workflow trigger block still contained a stray unindented path fragment, had lost its `pull_request` section, and therefore could not provide the required symmetric path coverage.
- Rebuilt the trigger with matching `push` / `pull_request` filters for the PID 1 production, policy, test and workflow inputs. Added the optional-daemon policy header/test to the inline path contract and defined its `ROOT` path used by the policy assertion.
- Product C source is unchanged. The new exact-head workflow must execute the GNU89/ASan/UBSan regressions and complete the source/path contract; this correction itself makes no L3/L4 claim.
- Changed paths: `.github/workflows/rc-notification-queue-stability.yml`, `dev_log.md`. Result: THIS COMMIT.

---


## 2026-11-15 10:18 CST — Align storage stop contract with identity-verified optional daemons

- USB storage hotplug run `43666309989` on `f0df8c0b2a9f9e07178e733e9945e6fa325b8f75` passed the storage lifecycle graph checks but failed at `scripts/check_storage_safe_remove.py:255`. The checker still required four stop functions to directly return `eval(..., "stop")`; production correctly routes SCUT/MentoHUST/VLMCSd/iperf3 through `optional_daemon_stop_status()` so it can also require a complete process inventory proving ABSENT.
- Updated the contract to require the exact command-status argument and identity-specific process name for those four wrappers, and to verify UNKNOWN-preserving snapshot cleanup plus the shared policy's command-error precedence and confirmed-ABSENT requirement. Other daemon wrappers retain their direct launcher-status assertions.
- Added `optional_daemon_policy.h` to storage hotplug and XY-C1 push path filters because the production RC source includes it; this closes a target-build trigger gap as well as the host contract trigger.
- Changed paths: `scripts/check_storage_safe_remove.py`, `.github/workflows/storage-hotplug-stability.yml`, `.github/workflows/xy-c1-firmware-validation.yml`, `dev_log.md`. Result: THIS COMMIT.

---


## 2026-11-15 10:23 CST — Fail closed on unknown PPPoE relay inventory

- Continued the handoff's P1 task in `trunk/user/rc/src/net.c::set_passthrough_pppoe()`. The old `pids("pppoe-relay")` Boolean check did not encode the process inventory's RUNNING/ABSENT/UNKNOWN contract; start authorization now comes only from a complete ABSENT snapshot.
- A RUNNING relay is left alone. UNKNOWN captures the scanner error before releasing the snapshot, logs the reason, and refuses to launch a possible duplicate. The existing stop path is unchanged. Nonzero launcher status is now logged rather than silently discarded.
- Added a GNU89 policy regression for ABSENT/RUNNING/UNKNOWN/invalid states, wired it into the process-lifecycle workflow, extended production source integration assertions, and added the helper to the XY-C1 MIPS build trigger.
- Changed paths: `trunk/user/rc/src/net.c`, `trunk/user/rc/src/pppoe_relay_policy.h`, `trunk/user/rc/src/tests/pppoe_relay_policy.c`, `.github/workflows/process-lifecycle-regression.yml`, `.github/workflows/xy-c1-firmware-validation.yml`, `scripts/check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---


## 2026-11-15 10:28 CST — Correct PID 1 and process-inventory source contracts

- Exact-head PID 1 run `43666824125` on `b84bfafaed5745700782f3c8a3c54683c9126c5b` failed in the final inline checker: it required `PROCESS_PRESENCE_UNKNOWN` to appear inside `optional_daemon_policy.h`, although the header delegates the enum to `process_inventory.h` and the policy test exercises UNKNOWN. The check now validates ABSENT in the policy and UNKNOWN in its regression test.
- Exact-head process-inventory run `43667225953` on `0eb191bbe35c422a3bf3835b0cda9a97b3637b24` failed because the integration checker incorrectly required a host-only PPPoE policy test to appear in the XY-C1 firmware-build trigger. Removed that test from target-build sources while retaining the production policy header, and explicitly checked that net.c/header/test are all watched by the process-lifecycle regression workflow.
- Corrected the PPPoE source check's function delimiter so assertions are scoped to `set_passthrough_pppoe()`, not the remainder of net.c.
- No product implementation changed in this contract-only correction. Changed paths: `.github/workflows/rc-notification-queue-stability.yml`, `scripts/check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---


## 2026-11-15 10:51 CST — Bind gateway DHCP renewal to the intended udhcpc PID identity

- Replaced `detect_wan.c::chk_udhcpc()`'s name-wide `killall -SIGUSR1 udhcpc` with `process_pidfile_signal()`, validating the PID/start-time record and signaling only the client owned by this role. Router WAN uses `/var/run/udhcpc0.pid`; AP mode uses `/var/run/udhcpc_lan.pid`. The process-inventory UNKNOWN gate remains in front of the operation; a missing/stale/mismatched PID identity fails visibly rather than signaling other DHCP clients.
- Added the role-to-PID-file selector to the existing GNU89 DHCP policy and tests for both paths. Extended the process-inventory source contract to require identity-bound renewal and prohibit the broad signal.
- This change is based on freshly fetched main `d861bf83f3d215140d65c01a17129269fbfaabcc`; concurrent direct-start fail-closed fixes were preserved.
- Changed paths: `trunk/user/rc/src/detect_wan.c`, `trunk/user/rc/src/detect_wan_udhcpc_policy.h`, `trunk/user/rc/src/tests/detect_wan_udhcpc_policy.c`, `scripts/check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---


## 2026-11-14 — Harden direct starts and daemon stop postconditions

- Direct/manual starts now preserve process-inventory UNKNOWN for SSH, infosvr, telnetd, networkmap, PPPoE relay, and SCUT/MentoHUST/VLMCSd/iperf3; starts proceed only when the complete inventory proves ABSENT.
- Stop commands for networkmap, infosvr, SSH, telnetd, FTP, and Samba now verify that the daemon set is actually absent before returning success; storage-backed stop groups were extended where a matching stop/query pair existed.
- SSH status API preserves tri-state UNKNOWN; invalid process inventory enum results are normalized to UNKNOWN in the optional/storage/PPPoE scanners.
- Revalidation closes a time-of-check/time-of-use gap in networkmap startup; manual telnetd restart now routes through the same guarded start function as PID 1.
- PPPoE relay's direct process check is replaced by a complete process inventory gate with a GNU89 policy regression and dedicated Actions workflow.
- Source checkpoint: `a90014886d87c8fcf8cf09a16f2322240e75e265`; backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact-head Actions: watchdog signal lifecycle stability regression #43668416071: success; NVRAM transaction boundary audit #43668416001: in_progress; GPIO button daemon startup and press timing stability regression #43668416147: success; AdGuardHome config security regression #43668416084: success; Adbyby download and dnsmasq configuration stability regression #43668416078: success; USB storage hotplug stability regression #43668416119: in_progress; HTTPD daemon lifecycle stability #43668416103: success; infosvr packet and signal lifecycle stability regression #43668416183: success; VPN server lifecycle stability regression #43668416187: success; detect_link signal lifecycle stability regression #43668416017: success; WAN gateway ARP probe socket stability regression #43668416082: success; ZeroTier lifecycle stability regression #43668416038: success; XY-C1 Firmware Validation #43668416135: in_progress; DDNSTO lifecycle stability regression #43668416051: success; AliyunDrive WebDAV lifecycle stability regression #43668416137: in_progress; networkmap signal lifecycle stability regression #43668416118: failure; AI handoff documentation contract #43668416061: success; DNS Forwarder launch path stability regression #43668416176: success; Crond daemon lifecycle stability regression #43668416108: success; HTTPD Authentication and Security Regression #43668416021: success; AliDDNS signed API transport security regression #43668416132: success; PID 1 notification queue stability regression #43668416148: success; Process lifecycle inventory - a90014886d87c8fcf8cf09a16f2322240e75e265 #43668416168: success; detect_internet signal lifecycle stability regression #43668416090: success; SkipDBv2 Regression #43668416166: success; FRP asynchronous startup lifecycle regression #43668416052: success.
- L3/L4 remain NOT VERIFIED; no cross-compile, TRX artifact, or physical router validation is claimed.

---

## 2026-11-14 — Repair networkmap lifecycle checker after policy extraction

- Investigated networkmap signal Action `43668416118` on `a90014886d87c8fcf8cf09a16f2322240e75e265`. Both GNU89/ASan/UBSan policy executables passed: signal deferral, and tri-state daemon action plus identity-bound refresh. The final checker failed because it expected RUNNING and ABSENT enum literals inside `start_networkmap()`; the implementation explicitly handles RUNNING in that function and centralizes ABSENT-only launch admission in `networkmap_start_allowed()`.
- Corrected the source contract to assert the RUNNING fast path, the helper call, and the helper's exact ABSENT predicate. The UNKNOWN-abort assertion is retained. No production behavior changed.
- Changed path: `scripts/check_networkmap_signals.py`. A new exact-head run is required; the old failure is not being re-labelled as a pass.
- Result: THIS COMMIT.

---



## 2026-11-14 — Fail closed on miniupnpd inventory and bind reload to PID identity

- Replaced miniupnpd's Boolean `pids()` decisions in direct start, startup readiness, UPnP update and WAN-interface transition with process-inventory RUNNING/ABSENT/UNKNOWN. Duplicate starts are skipped only on confirmed RUNNING; launch proceeds only on confirmed ABSENT; UNKNOWN/invalid states return explicit errors and do not authorize duplicate launch.
- `stop_upnp()` now preserves UNKNOWN as an error after its identity-aware `kill_services()` transaction rather than allowing a Boolean probe to describe an unreadable inventory as stopped.
- Replaced name-wide `killall -SIGUSR1 miniupnpd` with a snapshot-bound notification: captured PID/start-time identities are preflight-validated, then revalidated before each exact-PID signal. UNKNOWN in preflight sends no signal; ESRCH for an exited target is harmless; the policy accepts only SIGUSR1.
- Extended GNU89 daemon-policy tests for tri-state start/update decisions, identity preflight, UNKNOWN, target disappearance and signal errors. Updated the miniupnpd source-contract workflow and added the policy header to XY-C1 path triggers. Exact-SHA CI must confirm these changes; no MIPS or runtime claim is made here.
- Changed paths: `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/miniupnpd_daemon_policy.h`, `trunk/user/rc/src/tests/miniupnpd_daemon_policy_test.c`, `.github/workflows/miniupnpd-config-stability.yml`, `.github/workflows/xy-c1-firmware-validation.yml`, `dev_log.md`. Result: THIS COMMIT.

---



## 2026-11-14 — Make Inadyn reload and restart decisions identity-safe

- Replaced `notify_ddns_update()`'s Boolean `pids("inadyn")` decision and process-name-wide `killall -SIGHUP` with tri-state process inventory and exact PID/start-time validation. UNKNOWN/invalid inventory aborts before writing config; a running instance must pass an all-target identity preflight before the atomic config writer publishes; the HUP phase revalidates identities before signaling.
- Changed `stop_ddns()` to return the existing identity-aware `kill_services()` status, and made `start_ddns()` refuse to launch if the previous instance did not stop. This closes the former ignored-stop-result path that could start a duplicate Inadyn process.
- Added GNU89/ASan/UBSan regressions for action selection, UNKNOWN preflight, no-signal behavior, PID identity revalidation, ESRCH handling, and rejection of non-HUP signals. Added a dedicated production integration workflow and XY-C1 header trigger.
- Changed paths: `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/inadyn_daemon_policy.h`, `trunk/user/rc/src/tests/inadyn_daemon_policy.c`, `scripts/check_inadyn_lifecycle.py`, `.github/workflows/inadyn-lifecycle-stability.yml`, `.github/workflows/xy-c1-firmware-validation.yml`, `dev_log.md`. Result: THIS COMMIT.

---



## 2026-11-14 — Repair cross-suite CI contracts for Inadyn lifecycle changes

- Exact-head DNSMasq workflow run `43669561401` on `61cf95e446a5777568e45c2cf3d589e74696d6f0` passed earlier build/test steps but failed in the inline Python source contract with `ValueError: substring not found`: it still expected literal `"-SIGHUP"` in `notify_ddns_update()`, while the implementation now uses `inadyn_signal_snapshot(..., SIGHUP, ...)`. Updated it to verify tri-state inventory, all-target preflight before atomic config publication, snapshot-bound signal after publication, cleanup, and absence of name-wide killall. Also strengthened the start contract for checked stop status.
- Added the new Inadyn lifecycle helper, test, checker and workflow paths to DNSMasq push/PR filters and its exact source-path contract. Removed a pre-existing duplicate pair of process-inventory paths from the checker tuple so the once-per-trigger assertion is meaningful.
- Exact-head vsftpd run `43669561432` on the same source SHA failed at compile time because `ftp_daemon_policy.h` includes `process_inventory.h` but the regression omitted `-I trunk/user/shared/src`. Added the required include path; product source and FTP policy are unchanged.
- Changed paths: `.github/workflows/dnsmasq-lifecycle.yml`, `.github/workflows/vsftpd-stability.yml`, `dev_log.md`. New exact-head Actions must pass before closure; the old failures remain historical failures.
- Result: THIS COMMIT.

---



## 2026-11-14 — Replace HTTPD name-wide logout with identity-preflight notification

- Migrated the LAN reconfiguration HTTPD logout from `killall -SIGUSR1 httpd` to `signal_services_by_identity()`. The new lifecycle-policy notification transaction captures all matching PID/start-time identities, performs complete inventory and identity preflight across the entire target set before the first signal, and revalidates each identity immediately before delivery.
- The notification API is deliberately limited to SIGHUP, SIGUSR1 and SIGUSR2. SIGTERM/SIGKILL remain exclusive to the existing stop transaction; SIGSTOP and unsupported signals are rejected. UNKNOWN/CHANGED/FAILED states abort and are logged instead of broadcasting to all same-name processes.
- Extended the GNU89 service-lifecycle regression with all-target preflight, zero-signal-on-UNKNOWN, identity-validation failure and stop/unsupported-signal rejection cases. Existing process workflow filters already watch every changed production/test source.
- Changed paths: `trunk/user/rc/src/service_lifecycle_policy.h`, `trunk/user/rc/src/tests/service_lifecycle_policy.c`, `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/net_lan.c`, `scripts/check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---



## 2026-11-14 — Keep RC lifecycle result constants out of net_lan translation unit

- Corrected the new HTTPD notification caller to compare the public integer status with `0`, rather than naming `SERVICE_LIFECYCLE_STOPPED` in `net_lan.c`. The enum belongs to the private lifecycle policy header, which is not included by that translation unit; the caller only needs the documented zero-success contract from `signal_services_by_identity()`.
- This avoids coupling the network/LAN unit to an internal enum and prevents a compile-time undeclared-identifier failure. The policy and adapter continue to use the enum internally.
- Changed paths: `trunk/user/rc/src/net_lan.c`, `dev_log.md`. Result: THIS COMMIT.

---



## 2026-11-14 — Fix identity-bound notification target double-free

- Exact-head process lifecycle run `43669932154` on `f6fdb37613baa40b6b198aefe1dd3de09a0f5c9e` failed its ASan/UBSan regression while exercising the new identity-bound service notification API. AddressSanitizer reported a double-free in `service_lifecycle_notify()`: `service_lifecycle_release_targets()` already releases each captured snapshot and frees the target array, but the caller immediately freed the same array again.
- Removed the redundant second `free(targets)`. The existing `test_identity_bound_notification_preflights_all_targets` exercises the exact successful notification path and reproduces the failure under ASan; the exact-head process lifecycle Action must pass after this correction.
- No signal-selection, inventory, identity validation, or stop transaction semantics changed.
- Changed path: `trunk/user/rc/src/service_lifecycle_policy.h`. Result: THIS COMMIT.

---

## 2026-11-14 — Close remaining lifecycle ownership and cross-suite trigger defects

- Followed up the exact-head process-lifecycle failure from `669483855e2dd246f66543fb2b9a9d9c4ebec174`. The concurrent main commit `39af9b16649128b07859f9da56b34df5de12bbda` had removed the new `service_lifecycle_notify()` double-free. A second ownership bug remained in `kill_services_with_pidfiles()`: both its invalid-pidfile branch and normal return called `service_lifecycle_release_targets()` (which already frees the target array) and then freed the same pointer again. Removed both redundant frees and extended the checker to assert single ownership.
- Made the new process-lifecycle checker define the HTTPD notification source and lifecycle test before using them in top-level assertions; the assertion ordering is now executable as written.
- DNSMasq's Inadyn cross-suite source contract has the new Inadyn policy/test/checker/workflow paths in both push and pull-request filters.
- Updated the vsftpd cross-suite liveness assertion to the actual tri-state `storage_daemon_presence()` callback rather than obsolete Boolean `pids()`; the stable-presence helper still rejects UNKNOWN.
- Changed paths: `trunk/user/rc/src/common_ex.c`, `scripts/check_process_lifecycle_integration.py`, `.github/workflows/dnsmasq-lifecycle.yml`, `.github/workflows/vsftpd-stability.yml`, `dev_log.md`. Result: THIS COMMIT.

---



## 2026-11-14 — Repair storage, VSFTPD and HTTPD lifecycle CI contracts

- Storage hotplug run `43669561488` on `61cf95e446a5777568e45c2cf3d589e74696d6f` failed because `scripts/check_storage_safe_remove.py` matched a forward declaration of `optional_daemon_stop_status()` and parsed the following unrelated body. The checker now skips declarations and has a synthetic prototype/definition regression.
- VSFTPD run `43670216329` on `d846387eeaa735265267cce54c270b1fe5e282c6` failed for the same prototype-selection defect in its inline `function_body()` checker for `write_vsftpd_conf()`. Added declaration skipping and an executable parser fixture.
- HTTPD security run `43670216352` on `d846387eeaa735265267cce54c270b1fe5e282c6` failed because a cross-suite contract still delimited `ddns_updated_main()` using a `void stop_ddns` declaration. The implementation now returns `int`; corrected the boundary.
- Both storage and VSFTPD regressions compile/use `ftp_daemon_policy.h`, which includes shared `process_inventory.h`. Added that dependency to push/PR filters and to each workflow's semantic path contract so changes to the shared header actually run the dependent suites.
- The prior exact-head d846 DNSMasq (`43670216322`) and process lifecycle (`43670216277`) regressions passed. Storage run `43670216319` was still in progress when last checked; this checkpoint creates fresh exact-head coverage.
- Product sources are unchanged in this CI-contract slice. Fresh exact-head storage, VSFTPD, HTTPD and dependency-triggered regressions are required; historical failures remain attributed to their original SHAs.
- Changed paths: `scripts/check_storage_safe_remove.py`, `.github/workflows/storage-hotplug-stability.yml`, `.github/workflows/vsftpd-stability.yml`, `.github/workflows/httpd-security.yml`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-14 — Restore target build integration for inadyn daemon policy

- Exact-head XY-C1 Firmware Validation run `43670514075` on `80952b0eb515c9fd46ed1b4608638b79a1797197` failed during the MIPS/uClibc build in `services_ex.c::notify_ddns_update()`: the function used `inadyn_daemon_policy.h` types and helpers without including the policy header. The compiler reported the incomplete enum type and undeclared action constants/functions.
- Added the direct policy-header include beside the existing process-inventory include. The XY-C1 workflow already watches both `services_ex.c` and `inadyn_daemon_policy.h`, so the next exact-head run exercises the full target build.
- Product behavior is unchanged; this repairs a missing compile-time dependency. L3 remains unverified until the new exact-head build, TRX verification and artifact upload all pass.
- Changed paths: `trunk/user/rc/src/services_ex.c`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-14 — Fix VSFTPD source-checker function signature parsing

- Exact-head VSFTPD stability run `43670514105` on `80952b0eb515c9fd46ed1b4608638b79a1797197` passed its GNU89 regression binary and failed only in the final Python source contract. The checker was asked to extract `handle_rnfr()` / `handle_rnto()` with signatures that already included the opening `{`; its parser searched for a second opening brace after the signature and falsely reported a missing definition.
- Updated the inline `function_body()` helper to recognize signatures that include their opening brace, while preserving forward-declaration skipping. Added a synthetic regression for this exact signature shape alongside the existing prototype fixture.
- Product sources are unchanged. A new exact-head VSFTPD workflow run is required; the previous failure remains attributed to its original SHA.
- Changed paths: `.github/workflows/vsftpd-stability.yml`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-14 — Preserve DNSMasq UNKNOWN during DDNS cache-purge notification

- Migrated the remaining `services_ex.c::ddns_updated_main()` legacy `pids("dnsmasq")` gate to the existing complete process-inventory scanner. A confirmed RUNNING DNSMasq triggers the best-effort cache purge; confirmed ABSENT skips the unnecessary restart; UNKNOWN or invalid inventory logs the scan error and does not run the purge path.
- Added an explicit pure policy mapping for RUNNING / ABSENT / UNKNOWN plus invalid enum values, with GNU89 unit regressions. The DNSMasq lifecycle workflow already covers the source, policy header and test; added the policy header to XY-C1 firmware workflow path filters so target builds rerun when this included production header changes.
- Changed paths: `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/dnsmasq_restart_policy.h`, `trunk/user/rc/src/tests/dnsmasq_restart_policy.c`, `.github/workflows/xy-c1-firmware-validation.yml`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-14 — Align VSFTPD checker with checked stop postcondition

- The exact-head VSFTPD run `43671217127` on `ae4fbc507488ac32318f966d9066a3a9b9561c9d` passed its GNU89 regression and the brace-bearing function-parser fixtures, then failed the next source assertion: the checker expected `stop_ftp_locked()` to return the raw `kill_services()` status. Production intentionally passes that status through `storage_service_stop_complete(status, is_ftp_run())`, returns `-1` if the daemon is not confirmed absent, and returns `0` only after that postcondition succeeds.
- Replaced the stale raw-status assertion with checks for the actual status merge, failure return, success return, and their required ordering. This preserves the stronger production contract instead of weakening it.
- Product source unchanged. Changed paths: `.github/workflows/vsftpd-stability.yml`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-14 — Make auxiliary detector start gates fail closed on UNKNOWN

- Removed three remaining lossy `pids()` start checks:
  - `notify_run_detect_internet()` now starts only on confirmed ABSENT, sends the PID-file identity-bound SIGUSR1 only on confirmed RUNNING, and does neither on UNKNOWN.
  - `notify_reset_detect_link()` follows the same three-way rule.
  - WAN IPoE gateway-ARP checker startup now occurs only on confirmed ABSENT; RUNNING is a no-op and UNKNOWN is logged without launching a duplicate.
- Added a shared, unit-tested process-presence action policy covering ABSENT, RUNNING, UNKNOWN and invalid states. The process-lifecycle integration checker verifies inventory capture/release ordering and the three production call sites; the GNU89 regression and XY-C1 target workflow watch the new policy and test files.
- Changed paths: `detect_internet.c`, `detect_link.c`, `net_wan.c`, `process_presence_start_policy.h`, `tests/process_presence_start_policy.c`, `process-lifecycle-regression.yml`, `xy-c1-firmware-validation.yml`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-14 — Stop watchdog recovery from treating UNKNOWN as daemon absence

- Replaced the watchdog's remaining lossy service-presence gates for HTTPD, DNSMasq and watchdog itself with complete process-inventory scans. HTTPD/DNSMasq recovery counters now remain unchanged on UNKNOWN; a watchdog start is authorized only by ABSENT, treated as already running on RUNNING, and rejected with the scan error on UNKNOWN.
- `notify_watchdog_time()` now delegates failed-PID-file recovery to the tri-state `start_watchdog()` gate, so a missing/stale PID record cannot cause a duplicate watchdog when the process is still present or inventory is unavailable.
- Extended the existing tri-state policy integration checker and added watchdog.c to the process-lifecycle regression's watched paths. The existing policy regression covers ABSENT/RUNNING/UNKNOWN/invalid inputs.
- Changed paths: `watchdog.c`, `process-lifecycle-regression.yml`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Repair process-presence regression assertions after tri-state migration

- Exact-head process lifecycle run `43672306838` on `bc7cc31b8e5e86b3bd1e7cc800ab27b79c552a44` passed all GNU89 binaries and prior integration checks, then failed because the new detector checker required the literal ABORT enum in each caller body even though both callers correctly route the final `else` to a logged UNKNOWN/no-action path. Tightened the contract to verify that explicit log path instead.
- Exact-head watchdog signal run `43672306842` failed because its legacy assertion still required the removed `pids("watchdog")` fallback in `notify_watchdog_time()`. Updated it to require PID-file signal-before-tri-state-start ordering and the ENOENT/ESRCH recovery condition; added the new shared policy/test to its push/PR path filters and contract.
- No product source changed. The previous process lifecycle failure at `a80a532fd6ed52c5775426fb314d64930abdd287` had the same detector-checker mismatch; this correction repairs both affected exact-head regressions.
- Changed paths: `scripts/check_process_lifecycle_integration.py`, `.github/workflows/watchdog-signal-stability.yml`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Correct tri-state source contract and watchdog PR path symmetry

- Rechecked exact-head runs on `707c43481d77f212559e0592c0710ab8cca99a0b`: process lifecycle `43672495607` reached the final source contract and failed on an invalid Python assertion for a C-style unbraced `else` branch; the actual fail-closed log statement was present. Removed the syntactically incorrect assertion while retaining the explicit UNKNOWN log check.
- Watchdog signal `43672495588` passed all functional signal tests and failed its path-contract check because the previous workflow update had added `process_presence_start_policy.h` and its regression to push filters but not pull_request filters. Added both PR entries; push/PR contract tuple already includes both.
- Changed paths: `scripts/check_process_lifecycle_integration.py`, `.github/workflows/watchdog-signal-stability.yml`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Make VPN server bcrelay callbacks respect UNKNOWN process state

- Migrated `ipup_vpns_main()` and `ipdown_vpns_main()` away from the lossy `pids("bcrelay")` predicate. Both callbacks now capture the scanner error before releasing the snapshot and use the shared tri-state action: RUNNING can be stopped when the VPN cast mode requires it; ABSENT can launch configured relay instances; UNKNOWN performs neither operation and marks callback cleanup incomplete.
- Propagated `kill_services()` failures and bcrelay launch failures into callback return status instead of silently reporting success. The VPN lifecycle source checker now enforces snapshot lifetime, explicit tri-state branches, and absence of legacy pids gates. The dedicated VPN workflow watches process-inventory implementation/header and the shared presence policy/test in both push and PR filters.
- Changed paths: `vpn_server.c`, `.github/workflows/vpn-server-stability.yml`, `scripts/check_vpn_server_lifecycle.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Preserve both VPN broadcast-relay traffic directions

- Immediate post-commit review of `a2b80bd2cc66244b2f75ef857224985cdfaa88b6` caught a transcription error in the second `bcrelay` launch argument: its output interface had been copied as the PPP pattern instead of `IFNAME_BR`. Restored the original PPP-to-bridge direction.
- Added VPN lifecycle source-contract assertions for both bridge-to-PPP and PPP-to-bridge invocation forms, so future tri-state gate edits cannot silently collapse the two relay directions.
- Changed paths: `vpn_server.c`, `check_vpn_server_lifecycle.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Repair VPN and process-lifecycle source checker integration

- Exact process lifecycle run `43672595568` (and its superseded rerun `43672782229`) passed all regression binaries and failed on a checker looking for `PROCESS_PRESENCE_UNKNOWN` in the start-action mapping header. The enum correctly lives in `trunk/user/shared/src/process_inventory.h`; corrected the checker to read that contract instead.
- Exact VPN server run `43672782153` passed the lifecycle-lock, callback identity, xl2tpd and lease-lock binaries, then found an `IndentationError` in the newly extended Python source checker: the original `ordered(up,[` call opener had been consumed by the inserted block. Restored the call boundary.
- No new production behavior changed in this correction. The VPN relay-direction correction remains in `bc238c9fdcd4b23ea4fb3ad8463257e289da60b3`.
- Changed paths: `scripts/check_process_lifecycle_integration.py`, `scripts/check_vpn_server_lifecycle.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Tighten explicit ABSENT handling and align watchdog checker

- Exact process-lifecycle run `43672907289` reached the source checker and found another over-specific condition: the DNSMasq recovery path correctly treats the only remaining known state after the UNKNOWN early return as RUNNING, resetting `dnsmasq_missing` without spelling the derived NOTIFY action. The assertion now checks the actual reset behavior.
- Exact VPN server run `43672907307` passed all compiled regression binaries, then found that the bcrelay launch branch relied on the helper's final `else` instead of spelling out ABSENT. Made the launch condition explicitly require `PROCESS_PRESENCE_START_NEW`; the checker now locks that fail-closed branch ordering.
- Changed paths: `vpn_server.c`, `check_process_lifecycle_integration.py`, `check_vpn_server_lifecycle.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Spell watchdog process-inventory UNKNOWN as an explicit abort

- Exact process-lifecycle run `43673038080` passed all unit binaries and earlier integration contracts, then failed because `start_watchdog()` used a generic fall-through error block rather than explicitly comparing `PROCESS_PRESENCE_START_ABORT`. Reworked the production branch so UNKNOWN is visibly rejected before either returning RUNNING or launching on ABSENT. The scan errno is preserved.
- The process-lifecycle push filter already covers `watchdog.c`; no new event triggers were introduced.
- Changed paths: `watchdog.c`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Serialize VPN client stop and fail closed on xl2tpd state/control errors

- `stop_vpn_client()` was the one VPN client lifecycle entry point that did not hold the shared VPN lifecycle lock. Split its body into `stop_vpn_client_locked()`, added a lock-owning public wrapper, and changed the already-locked restart path to call the internal helper to avoid recursive locking.
- Removed the remaining `pids("xl2tpd")` gate. A complete inventory now distinguishes RUNNING (send the VPNC disconnect command), ABSENT (skip it), and UNKNOWN (preserve VPN state and fail closed) before PPP teardown begins.
- The xl2tpd FIFO helper previously discarded write and close errors, causing VPN start/stop to report success when its command was not delivered. It now validates the formatted command and propagates short-write, write, and close failures; connect/disconnect callers preserve NVRAM state and return an error on failure.
- Extended the VPN lifecycle checker for public stop locking, internal non-recursive restart, the tri-state process gate, and control FIFO error propagation. Existing VPN workflow filters already include `vpn_client.c`, process inventory, and the shared tri-state policy/test for push and PR events.
- Changed paths: `vpn_client.c`, `check_vpn_server_lifecycle.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Match VPN client restart contract to locked stop helper

- Post-commit source review found the legacy VPN checker still expected `restart_vpn_client_locked()` to call the public `stop_vpn_client()`, which would now reacquire the lifecycle lock and deadlock. Production correctly calls `stop_vpn_client_locked()` while holding the lock; updated the old ordering assertion to match the non-recursive path.
- Changed paths: `check_vpn_server_lifecycle.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Update process-lifecycle contract for serialized VPN client stop

- Exact process-lifecycle run `43673507109` on `2761e92be05d7957beaa967290996d707aee6334` passed the sanitizer and PID-file policy binaries, then failed because the cross-cutting checker still extracted the public `stop_vpn_client()` wrapper and expected the inner PPP teardown in that wrapper. The teardown now correctly lives in `stop_vpn_client_locked()`; updated the checker to validate the internal stop body, public lock acquisition/release and the new xl2tpd tri-state preflight.
- Exact VPN server run `43673507149` failed the corresponding old restart-order assertion; that was corrected in `e95c0509e300a59ff8e4b7b34645c1d5e6817eb5`. New exact-head re-runs are queued.
- Changed paths: `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Require an explicit ABSENT action before watchdog launch

- Exact process-lifecycle run `43673617941` on `52c003b29a1dacccba89cb454e3cc359a864227e` passed all GNU89 sanitizer/policy binaries and earlier process-integration checks, then failed at the final watchdog contract: `start_watchdog()` rejected UNKNOWN and returned for RUNNING but let every other action fall through to `eval()`. The shared policy currently maps only ABSENT to NEW, but implicit fallthrough would also launch if a future/invalid action value were introduced.
- Changed the production gate to invoke `/sbin/watchdog` only under an explicit `PROCESS_PRESENCE_START_NEW` check; any unexpected action now returns EIO without launching. Extended the source contract to preserve that explicit allowlist. Existing GNU89 policy regression already covers ABSENT→NEW, RUNNING→NOTIFY, UNKNOWN→ABORT and invalid enum→ABORT.
- Changed paths: `trunk/user/rc/src/watchdog.c`, `scripts/check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 validation required.

---

## 2026-11-15 — Remove undefined-variable assertion from process-lifecycle checker

- Exact process-lifecycle run `43673792669` on `877e36b42fc1f3249fb0235c000655a402d89ebe` passed the sanitizer/policy binaries and all VPN, PPP, DHCP, WAN/LAN, USB and detector integration checks, then stopped at a Python `NameError`: the checker referenced undefined `processWf` in an assertion that had no valid semantic purpose. Removed only that stray assertion; the actual workflow path checks below remain intact.
- Changed paths: `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Bound the HTTPD health-check wget instead of killing all wget processes

- `watchdog.c::httpd_check_v2()` launched a background wget with the default 900-second network timeout, then used `pids("wget")` plus `killall wget` on two early-exit paths. That could terminate unrelated download jobs.
- Set the probe's BusyBox-supported `-T 1` network timeout and removed both process-name-wide cleanup paths. The URL is numeric loopback, so the timeout is not exposed to DNS lookup delays. The common BusyBox config enables `CONFIG_FEATURE_WGET_TIMEOUT`.
- Extended the process-lifecycle checker to enforce the bounded wget invocation, absence of broad wget cleanup, and the required BusyBox config feature.
- Changed paths: `watchdog.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Fix HTTPD watchdog checker extraction across nested preprocessor guards

- Exact process-lifecycle run `43673924543` on `3eee10383ac8f6b05554738a92dd13fbfcceeb24` passed all compiled GNU89 policy regressions and integration checks through the detector group, then failed the new HTTPD wget source assertion. The checker sliced the function at the first inner `#endif` (`SUPPORT_HTTPS`), truncating the body before the wget invocation; production code was not implicated.
- Reused the existing brace-aware C function extractor to inspect the complete `httpd_check_v2()` body, including nested preprocessor blocks.
- Changed paths: `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Replace USB modem-wide killall with fail-closed process preflight

- `safe_remove_usb_modem()` previously ran `killall usb_modeswitch` and `killall eject` before modem teardown. Those name-wide kills could terminate a mode-switch or storage-eject job belonging to another USB device, while the rest of the teardown already uses identity-bound PPP/DHCP stops.
- Added a shared-inventory preflight for both transient processes. Removal proceeds only after each is authoritatively ABSENT; RUNNING returns `EBUSY`, UNKNOWN preserves the scanner errno (or `EIO`). Snapshot errors are captured before release. Removed both global killall operations, preserving retry safety instead of killing unrelated jobs.
- Extended the cross-cutting process-lifecycle source contract to verify the complete scan/release/fail-closed ordering and absence of killall in the modem-removal transaction. Existing process-lifecycle and XY-C1 workflow filters already watch `usb_modem.c` and the checker.
- Changed paths: `usb_modem.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Use local bounded extraction for HTTPD probe contract

- Exact process lifecycle run `43674095479` on `624e8fa2c4a471ee90127e42bc972a8a1c005ddc` revealed the next checker-only issue: `check_process_lifecycle_integration.py` has no shared `body()` helper (that helper exists in a different workflow's checker), so the nested-preprocessor fix raised `TypeError: 'str' object is not callable`.
- Changed the probe extraction to bound the complete `httpd_check_v2()` source between its signature and the following `refresh_ntp()` definition. This includes the HTTPS inner `#endif` and the wget invocation while keeping the check local to this script.
- The process-lifecycle source tests themselves all passed through detector checks before this Python extraction error.
- Changed paths: `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Remove watchdog's global HTTPD kill fallback and honor stop result

- `httpd_process_check()` already used the process inventory to distinguish RUNNING/ABSENT/UNKNOWN, but after an HTTPD health-check failure it ignored `stop_httpd()`'s result and ran `killall -9 httpd` unconditionally. This could kill a replacement/parallel HTTPD and then launch another even when the identity-bound stop transaction had failed.
- Removed the name-wide SIGKILL. Watchdog now requires `stop_httpd()` to complete successfully before deleting the probe marker or starting a replacement; stop failure leaves the daemon state intact and aborts recovery. The restart result is now logged instead of discarded. `stop_httpd()` already performs the shared identity-bound TERM/KILL transaction and verifies absence.
- Extended the process-lifecycle source contract to enforce stop-result ordering and ban the old global kill path.
- Changed paths: `watchdog.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Consolidate watchdog HTTPD stop guard outside feature conditional

- Simplified the preceding HTTPD recovery change so the identity-bound `stop_httpd()` and its fail-closed status check execute once, regardless of whether the optional HTTPD content probe is compiled. Only probe-marker removal remains conditional on `HTTPD_CHECK`.
- No behavioral change; this keeps the single stop postcondition visible and avoids duplicating control flow.
- Changed paths: `watchdog.c`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Remove duplicate HTTPD stop invocation

- Review of exact main `c44b3f9b90ded466aea5a9160e8c957adf46eb26` found that `httpd_process_check()` called `stop_httpd()` once with the return value discarded, then immediately called it a second time and checked only the second result. The refactor had consolidated the #ifdef branches but retained the original ignored call; that doubled every stop transaction and could conceal the first transaction's failure.
- Removed the unchecked invocation so the identity-bound stop executes exactly once and its status gates marker removal and restart. Added a source contract asserting exactly one `stop_httpd()` call within the watchdog recovery body.
- Changed paths: `trunk/user/rc/src/watchdog.c`, `scripts/check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process lifecycle and XY-C1 validations required.

---

## 2026-11-15 — Propagate 802.1X stop failures through wireless restart and shutdown

- The 802.1X stop wrappers for `rt2860apd` and `rtinicapd` returned `void` and discarded `kill_services()` status. Both `restart_wifi_wl()` and `restart_wifi_rt()` then changed radio configuration and launched replacement wireless interfaces even when an old authenticator daemon had not stopped. Dual-band rollback also discarded authenticator cleanup results; shutdown did not include the all-radio 802.1X stop in its persistence gate.
- Converted all three stop wrappers to return the checked `kill_services()` result (the 5GHz no-radio build explicitly returns success), updated public prototypes, and gated each radio restart before NVRAM reload-state changes/startup when its own or paired DBDC authenticator fails to stop. Rollback now logs authenticator stop failures. Shutdown records the all-radio stop result before MTD persistence and skips persistence on failure.
- Extended the process-lifecycle source contract to validate wrapper results, WL/RT preflight ordering, DBDC rollback reporting and shutdown persistence gating. Added the exact production files to the path-filtered regression trigger; XY-C1 already watches these production sources.
- Changed paths: `net_wifi.c`, `rc.h`, `rc.c`, `check_process_lifecycle_integration.py`, `process-lifecycle-regression.yml`, `dev_log.md`. Result: THIS COMMIT. Exact-head sanitizer/process checks and XY-C1 MIPS/uClibc build required.

---

## 2026-11-15 — Correct HTTPD recovery ordering assertion

- Exact process-lifecycle run `43674418682` on `c44b3f9b90ded466aea5a9160e8c957adf46eb26` passed every compile-time and integration check through the USB, WAN and detector paths, then failed on an over-broad watchdog checker assertion: it searched for the first `return;` in the whole HTTPD recovery body, which is the legitimate early return from UNKNOWN process presence, not the stop-failure branch.
- Scoped the return assertion to the block between the checked `stop_httpd()` call and `start_httpd()`. The production source ordering is correct.
- Changed paths: `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Abort WAN authentication restart when old daemons fail to stop

- `start_auth_kabinet()` and `start_auth_eapol()` both called void stop wrappers that discarded `kill_services()` results, then rewrote credentials/configuration and launched new authentication daemons regardless of whether old `lanauth`, `wpa_cli` or `wpa_supplicant` instances remained.
- Converted the two stop wrappers to return the underlying lifecycle result and made both start paths fail closed before launching replacements or rewriting the EAPOL config when the prior stop is incomplete. Updated shared prototypes.
- Extended the process-lifecycle checker for wrapper result propagation and stop-before-config/launch ordering. Added `auth.c` to both the sanitizer source-contract and XY-C1 firmware build path filters.
- Changed paths: `auth.c`, `rc.h`, `check_process_lifecycle_integration.py`, `process-lifecycle-regression.yml`, `xy-c1-firmware-validation.yml`, `dev_log.md`. Result: THIS COMMIT. Exact-head sanitizer/source contract and XY-C1 MIPS/uClibc validation required.

---

## 2026-11-15 — Propagate WAN authentication lifecycle results to interface transitions

- Follow-up inspection of every RC callsite found that `net_wan.c` still discarded the newly exposed `start_auth_eapol()`, `start_auth_kabinet()`, `stop_auth_eapol()` and `stop_auth_kabinet()` results. In particular, IPoE DHCP continued to launch `udhcpc` after EAPOL startup had failed, while WAN teardown cleared passthrough/interface state without proving auth daemons had stopped.
- WAN startup now records EAPOL failure, marks the unit errored, clears an already-assigned static address and skips DHCP/UP handling. WAN teardown tries both auth stop paths, aggregates failures, and aborts before interface teardown. IPoE WAN up/down route handlers now accumulate kabinet start/stop failures into their returned status rather than discarding them.
- Extended the integration checker for EAPOL abort ordering, stop-before-interface teardown and WAN up/down status aggregation. The existing process and XY-C1 path filters already include `net_wan.c`.
- Changed paths: `net_wan.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process regression and XY-C1 firmware validation required.

---

## 2026-11-15 — Align lifecycle regression contracts with checked WAN/802.1X paths

- Exact-head run on `9f5556216f279b9ee7ea98f91be8d538e1cb666f` exposed three source-checker failures after the auth propagation slice: the process checker referenced `wan_stop` before its later declaration; WAN route status still expected five result accumulations after kabinet authentication became a sixth checked operation; and the HTTPD security workflow searched for bare `stop_8021x_*.();` calls although production restart code now uses guarded `stop_8021x_*( ) != 0` expressions.
- Moved the existing WAN teardown body extraction above its new assertions, updated the route-status count and explicit kabinet assertion, and changed wireless-order checker tokens to match the guarded function calls while preserving both-radio ordering.
- Exact 9f Actions: process lifecycle failed on the checker NameError; WAN route status failed on the stale aggregate count; HTTPD security failed because the stop-position list became empty. Process inventory, WAN interface teardown, WAN DHCP callback, WAN ARP and multiple other source-head checks passed. XY-C1 remained in progress at inspection.
- Changed paths: `check_process_lifecycle_integration.py`, `wan-route-status.yml`, `httpd-security.yml`, `dev_log.md`. Result: THIS COMMIT. Exact-head process, WAN route, HTTPD security and XY-C1 reruns required.

---

## 2026-11-15 — Gate logger restart and shutdown on confirmed stop completion

- `stop_logger()` discarded the result of `kill_services(klogd, syslogd)`. The SYSLOG and TIME notification handlers then unconditionally started replacement logger processes, risking duplicate daemons when stop did not complete. During shutdown, logger termination happened only after storage MTD persistence had already been authorized, and its failure could not suppress persistence.
- Converted `stop_logger()` to return the checked lifecycle status. SYSLOG and TIME restarts now abort replacement startup on stop failure and report start failures. Shutdown stops logging before the storage persistence decision, includes logger status in the fail-closed MTD gate, and retains the later LAN shutdown ordering without repeating the logger stop.
- Added process-lifecycle source contracts for helper result propagation, shutdown persistence order and both notification restart gates; added `services.c` to the process regression push path filter.
- Changed paths: `services.c`, `rc.h`, `rc.c`, `check_process_lifecycle_integration.py`, `process-lifecycle-regression.yml`, `dev_log.md`. Result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 MIPS/uClibc validation required.

---

## 2026-11-15 — Propagate PPPoE relay lifecycle errors through WAN transitions

- `set_passthrough_pppoe()` performed an inventory-guarded start but returned `void`, so callers could not observe launch failure or an UNKNOWN process inventory; its stop branch also discarded `kill_services()` failure. `stop_wan_locked()` consequently continued WAN interface teardown even when `pppoe-relay` might still be running.
- Converted the helper to return status for launch, already-running, inventory-unknown and stop outcomes; UNKNOWN now returns the scanner error (or EIO) with errno preserved. WAN startup logs optional passthrough failure. WAN teardown now aborts before WAN route/interface teardown when the relay stop is incomplete.
- Extended the process integration contracts for the helper's status paths and WAN stop ordering. The relevant production files are already covered by process and XY-C1 workflow path filters.
- Changed paths: `net.c`, `rc.h`, `net_wan.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process-lifecycle, HTTPD security and XY-C1 firmware validation required.

---

## 2026-11-15 — Restore WAN route workflow heredoc indentation

- The WAN route workflow had no job on the `d7889a818a36d0e2df4a15ef4f1c3b30b86efcb9` and `7fb6114a346fcf4a97ab478616a6f9134737de7b` pushes. The new kabinet-authentication assertion was inserted at column 1 inside a YAML block scalar, invalidating the workflow before GitHub could create a job; this was a workflow syntax failure, not a route-test result.
- Restored the required ten-space heredoc indentation while retaining the sixth WAN status accumulation and explicit `start_auth_kabinet()` assertion. The next push should now instantiate the route regression job.
- Changed paths: `.github/workflows/wan-route-status.yml`, `dev_log.md`. Result: THIS COMMIT. Exact-head WAN route regression required.

---

## 2026-11-15 — Update PPPoE checker for explicit running/unknown failure split

- Exact process-lifecycle runs `43675935125` (source `877fbb2b1ab09a1be429bb379844dddc05d2750f`) and `43676007129` (source `7733970303619f9288865b718a3d0411874c102e`) reached the Python integration checker and failed at an obsolete assertion requiring an `else if (presence == UNKNOWN)` branch. The production helper now correctly returns success only for RUNNING, while all other non-ABSENT states share the same fail-closed error path.
- Replaced the obsolete branch-shape assertion with checks that RUNNING is accepted and unknown/invalid states preserve scanner errno and return the error. No production code changed in this checker-only correction.
- Changed paths: `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process lifecycle and XY-C1 validation required.

---

## 2026-11-15 — Require PPPoE relay child readiness after successful launch

- Review of the PPPoE relay start path found that the pre-launch inventory gate proved only that no old daemon existed. `eval()` waits for the launcher child and does not prove the detached `pppoe-relay` process remains present; returning that launcher status directly could report success even if the relay immediately exited or could not be observed. Unlike the adjacent DNSMasq path, no post-launch process-presence check existed.
- Added a four-attempt bounded RUNNING postcondition after a successful launch. Only an authoritative RUNNING snapshot returns success; ABSENT after bounded retries returns `ESRCH`, UNKNOWN/invalid returns `EIO` with the scan error preserved, and an exec/launcher error is returned unchanged. The pre-start gate now shares one scanner wrapper so every snapshot is released consistently.
- Extended the standalone GNU89 PPPoE policy regression for RUNNING/ABSENT/UNKNOWN/invalid post-launch states and preserved launcher errors. Extended the process source contract for the bounded retry/readiness path; process and XY-C1 workflows already monitor the changed production/helper/test paths.
- Changed paths: `net.c`, `pppoe_relay_policy.h`, `tests/pppoe_relay_policy.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 firmware validation required.

---

## 2026-11-15 — Align PPPoE readiness checker with shared scan wrapper

- Exact process regression `43676517579` on `a8b84679c87bf02703e72cf032e8d9bedc031ed1` failed at a stale integration assertion that still required `process_inventory_scan()` and snapshot release inside `set_passthrough_pppoe()`. The implementation had deliberately centralized snapshot/error/release handling in `pppoe_relay_process_presence()`; the source contract had not been updated to follow that refactor. The standalone policy target and preceding steps were not implicated by this assertion.
- Updated the contract to validate the shared scanner wrapper and its error-capture-before-release ordering, then validate that the preflight call uses the wrapper before the start decision. Tightened readiness handling so invalid enum values fail immediately with `EIO` rather than consuming the ABSENT retry window; UNKNOWN keeps the underlying scanner error.
- Changed paths: `net.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 reruns required.

---

## 2026-11-15 — Keep firmware-update abort paths recoverable after service shutdown

- Review of `flash_firmware()` found that `stop_misc()` terminated the watchdog before several later fallible operations (optional-service stop, custom shutdown hook, storage eject, printer stop and IPTV proxy stop). Any subsequent abort returned with the watchdog still stopped. The firmware path also killed `wpa_cli/wpa_supplicant` but did not call the shared `stop_8021x_all()`, leaving AP authentication daemons outside its stop gate.
- Moved `stop_misc()` to after the other fallible preparation steps, immediately before storage persistence/MTD write. If its own partial stop fails, the path now attempts the tri-state-safe `start_watchdog()` recovery before returning. Added a checked `stop_8021x_all()` gate before the custom shutdown hook and storage mutations.
- Extended the process-lifecycle source contract for the AP-auth stop gate, late watchdog-stop order, abort recovery and storage-write ordering. Exact-head process/XY-C1 reruns required.
- Changed paths: `rc.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Stop all WAN authentication and PPPoE relay services before firmware flash

- Follow-up over the complete firmware stop set found that `flash_firmware()` still relied on its narrow ad-hoc `wpa_cli/wpa_supplicant` kill list. It left the separate WAN `lanauth` and AP `rt2860apd/rtinicapd` daemons outside the gate, and the newly status-returning PPPoE relay stop helper was not used. A successful firmware write could therefore proceed while those services were still active.
- Replaced the duplicate EAPOL process-name entries in the legacy kill list with the shared checked `stop_auth_eapol()`, `stop_auth_kabinet()`, `stop_8021x_all()` and `set_passthrough_pppoe(0)` helpers. All four are attempted before the custom shutdown hook or storage mutation, and any stop failure aborts the update. The L2TP/PPP daemon group retains its existing six-second stop policy.
- Extended the process-lifecycle source contract for the complete auth/relay stop set and its pre-hook ordering. Exact-head process and XY-C1 reruns required.
- Changed paths: `rc.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT.

---

## 2026-11-15 — Abort firmware update when the custom shutdown hook fails

- The firmware path invoked `/etc/storage/shutdown_script.sh` through `doSystem()` but discarded its exit status and continued toward storage persistence and MTD image write even when the configured operator hook failed.
- Captured the hook status and fail-closed the firmware update on non-zero exit, with a diagnostic. This check runs before later service/watchdog shutdown and before storage mutation, so a hook failure leaves the watchdog running.
- Added source-contract coverage for the checked hook return and abort-before-proxy/storage ordering.
- Changed paths: `rc.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process-lifecycle and XY-C1 validation required.

---

## 2026-11-15 — Report watchdog recovery failures on firmware abort

- The new firmware abort recovery attempted `start_watchdog()`, but discarded its result. If procfs inventory was UNKNOWN or the watchdog launcher failed, the update returned silently without confirming recovery of the safety monitor; the MTD-write failure branch had the same silent result handling.
- Both recovery branches now capture and report watchdog start status and errno when recovery fails. The source regression asserts the failure report is present for the early miscellaneous-stop abort and the late MTD launcher failure.
- Changed paths: `rc.c`, `check_process_lifecycle_integration.py`, `dev_log.md`. Result: THIS COMMIT. Exact-head process lifecycle validation required.

---

## 2026-11-15 — Gate shutdown MTD persistence on wireless radio teardown

- shutdown_router() discarded stop_wifi_all_wl() / stop_wifi_all_rt() return statuses and called both only after write_storage_to_mtd(). It now attempts both band teardowns before the MTD decision, logs each failure, includes both statuses in the fail-closed persistence gate, and removes the late duplicate calls.
- Added a production-connected process-lifecycle contract for both result captures, error reporting, pre-MTD ordering and exactly-once stop calls.
- Fixed two stale workflow assertions. Wireless DBDC rollback correctly uses guarded if (stop_8021x_*() != 0) to stop the primary radio authenticator; the previous checker searched for a standalone call. HTTPD/security expected stop_misc() before storage eject, while commit 4264332 correctly moved watchdog shutdown after all fallible preparation and restores it when stop_misc() fails; its checker now tests the real order and recovery.
- Changed paths: rc.c, check_process_lifecycle_integration.py, wireless-radio-state.yml, httpd-security.yml, dev_log.md, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md. Result: THIS COMMIT. Exact-head process-lifecycle, wireless-radio, HTTPD/security, AI docs and XY-C1 checks required. No local checkout/build/hardware result is claimed.

---

## 2026-11-15 — Gate normal shutdown persistence on custom hook success

- shutdown_router() previously discarded the exit status from the configured shutdown_script.sh hook and could still persist storage MTD after the hook failed. The firmware-update path already fails closed on the same hook.
- Captured and logged the hook result. Shutdown/eject sequencing continues, but a non-zero hook status now suppresses storage_save_time() and write_storage_to_mtd().
- Extended the process-lifecycle source contract to require status capture, failure diagnostic, exactly-once hook invocation and ordering before storage eject / MTD persistence.
- Caller audit: stop_syslogd()/stop_klogd() and stop_udpxy()/stop_xupnpd() discard kill_services() status but have no identified callsites in the RC C source set. stop_logger() and grouped stop_igmpproxy() already propagate status. Samba and storage public stop paths retain their checked postconditions and lock boundaries; no unchecked RC caller was established in those paths.
- Changed paths: rc.c, check_process_lifecycle_integration.py, dev_log.md, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md. Parent SHA: 180e90075a11ad1135ee6801915211548337b52e. Exact-head Process lifecycle, AI docs and XY-C1 checks required. No local test/build/hardware result is claimed.

---

## 2026-11-15 — Align HTTPD security resolver contract with tri-state DNSMasq status

- Exact-head HTTPD Authentication/Security run #43679783993 on db4cc443692afec592b224985bf08cb655ac1c04 failed at a stale assertion requiring is_dns_dhcpd_run() in update_resolvconf(). The same parent added dnsmasq_process_status() and deliberately removed that Boolean pids()-based check.
- Updated the workflow source contract to require the public tri-state status call, fail-closed UNKNOWN handling, RUNNING-only restart and absence of the obsolete Boolean helper. Production code was already correct; this commit repairs only the integration contract.
- Exact parent DNSMasq stability #43679783878, process inventory/fail-closed #43679783948, process lifecycle #43679783896, VPN #43679783979, AI docs #43679784041 and other listed subsystem workflows passed on db4cc443. XY-C1 #43679783864 and NVRAM #43679783958 were still in progress at the last poll. These do not validate this new workflow commit.
- Changed paths: .github/workflows/httpd-security.yml, dev_log.md, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md. No production source changed and no local checkout/build is available.

---

## 2026-11-15 — Gate storage persistence on checked RWFS/Optware teardown

- shutdown_router() previously called umount_rwfs_partition() only after storage_save_time()/write_storage_to_mtd(); the void helper discarded Optware stop-hook, filesystem unmount and UBI detach failures. MTD Storage could be published while Optware teardown remained incomplete.
- umount_rwfs_partition() now returns status, verifies the UBI device maps to the RWFS MTD through Linux 4.4's sysfs mtd_num attribute, and requires an ABSENT post-detach state. Already-detached UBI is a safe no-op; unknown identity or failed unmount/detach is an error.
- Shutdown now stops Optware hooks and unmounts RWFS before MTD persistence, includes its status in the fail-closed gate, and continues power-down after failure. Added policy assertions and production source/order checks.
- Changed paths: common_ex.c, rc.c, rc.h, storage_optware_start_policy.h and its test, check_storage_safe_remove.py, dev_log.md, SESSION_HANDOFF.md and REFACTOR_ROADMAP.md. Parent SHA: adaa637920c5322e92be2ea7a21d20ed735646b7. Exact-head USB storage, AI docs and XY-C1 checks required; no local build or hardware result is claimed.
- Exact-parent Actions: VPN server lifecycle stability regression #43682158745 completed/success; Process inventory and lifecycle fail-closed regression #43682158741 completed/success; XY-C1 Firmware Validation #43682158738 in_progress/pending; AI handoff documentation contract #43682158736 completed/success.

---

## 2026-11-15 — Repair stale RWFS contract in storage-hotplug workflow

- Exact-head USB storage run #43682722908 on `fc675527e58402dfab0bd25d7ea5dc620861d872` passed all syntax, GNU89 policy, BusyBox 1.24.2 ash, Optware lifecycle, disk inventory, FTP presence and storage-daemon stop steps, then failed in embedded source-contract Python at line 490. The inline contract expected the removed `void umount_rwfs_partition(void)` signature and bare `return;` although production `e89f8ca3` returns status and verifies RWFS unmount plus UBI identity/detach.
- The standalone `scripts/check_storage_safe_remove.py` already matches the current API. This patch repairs the duplicate inline workflow assertions to check int return type, Optware hook error, unmount error, UBI pre/post identity and detach completion ordering.
- Exact parent AI docs #43682722938 passed; USB storage #43682722908 failed only at the stale inline contract. Earlier steps passed; checked-eject script was skipped. This changes no production source and claims no local test.
- Changed paths: `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `fc675527e58402dfab0bd25d7ea5dc620861d872`.
- Next P1: trace real `write_storage_to_mtd()`, `storage_save_time()` and NVRAM commit callers for unchecked write/fsync/close or prerequisite-stop status.

---

## 2026-11-15 14:18 CST — Fix storage safe-remove checker literal-brace failure

- Exact-head USB storage run #43683399235 on `bda473715cf8bb2e1bbb8c704370639854f2a07c` passed the embedded storage caller-graph and lock-order contract, then failed when `scripts/check_storage_safe_remove.py` called `.format(daemon)` on a C array literal containing unescaped braces. Python raised `ValueError: unexpected '{' in field name` before completing the remaining source assertions.
- Replaced the fragile format expression with explicit concatenation for the expected `char *svcs[] = { "<daemon>", NULL };` declaration. The adjacent identity-validated stop and postcondition assertions are unchanged.
- Local validation: exercised the expected C-array string construction for both Transmission and Aria with Python 3 (PASS). Full checker execution is unavailable without a local repository checkout; fresh exact-head Actions validation is required. No production firmware source changed.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent SHA: `bda473715cf8bb2e1bbb8c704370639854f2a07c`. Exact-head storage Actions for this commit: PENDING.

---

