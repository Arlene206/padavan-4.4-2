## 2026-11-16 — Restore IGMP regression fixture before legacy failure cases

- Exact e349 Process lifecycle #43770121710 failed in the IGMP config regression: the new token-injection tests wrote `last-good-token-validation`, while existing missing-parent/rename assertions still expected `last-good-configuration`. The production helper built and the run reached the later fixture comparison.
- Restored the original last-good fixture after the new altnet whitespace, interface comment and downstream-tab rejection assertions, keeping all older error-path tests independent.
- Exact e349 AI docs #43770121750, inventory #43770121725, AdGuardHome #43770121883, infosvr #43770121820, AliyunDrive #43770121777 and AliDDNS #43770121720 PASS; NVRAM #43770121752 and XY-C1 #43770121908 were in progress. Exact abd40 storage #43769497238 PASS.
- No local checkout/test run is claimed. Corrected exact-head Process lifecycle regression and XY-C1 Build/TRX/artifact validation required.
- Parent main: `e3499afc01bdc42cd1620eb18a6eec0e20541d36`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Reject IGMP proxy configuration token injection

- Review of the atomic IGMP proxy publisher found that the old CR/LF-only validation still accepted spaces/tabs and '#' inside NVRAM-derived `altnet` and interface values. These characters can split a generated directive into unintended parser tokens or comment out its remainder without a newline.
- Added a printable single-token validator for interface names (reject controls, whitespace and '#') and restrict `altnet` to numeric IPv4/CIDR punctuation (digits, '.' and '/'). Tests verify altnet whitespace, interface comment/tab injection all return EINVAL and leave the last-good config unchanged.
- Parent abd40 USB storage #43769497238 PASS, AI docs #43769497316 PASS, Process lifecycle #43769497255 PASS, AdGuardHome #43769497209 PASS and infosvr #43769497293 PASS. XY-C1 #43769497292, NVRAM #43769497231, inventory #43769497244, AliDDNS #43769497245 and AliyunDrive #43769497299 were pending at last poll.
- No local checkout was available; no local C test is claimed. Exact-head Process lifecycle (GNU89 ASan/UBSan) and XY-C1 Build/TRX/artifact are pending.
- Parent main: `abd40abba2c3b78bf02fb262110f485534ba8b4b`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Repair Transmission stop PID validation and regression harness

- Exact parent 814d storage #43768621241 failed before executing tests: POSIX `sh -n` rejected the unquoted-space glob `*[!0-9 ]*` in the new Transmission stop PID validation.
- Replaced the malformed class with per-token numeric validation (`''|*[!0-9]*`) before sending any signals. Also corrected the test harness's AWK extraction (now uses a sed function range), changed double-escaped newline format strings into real line delimiters, and added an assertion that restart is gated by `func_stop || exit 1`.
- Parent exact AI docs #43768621220 PASS; NVRAM #43768621188, Process inventory #43768621322, Process lifecycle #43768621183, infosvr #43768621156, AliDDNS #43768621223, AdGuardHome #43768621236 and AliyunDrive #43768621152 PASS. Parent XY-C1 #43768621137 was in progress at last inspection.
- No local repository checkout was available; no local shell test is claimed. Exact-head storage host + BusyBox ash and XY-C1 Build/TRX/artifact Actions are required for this repair.
- Parent main: `814d4639643675ed9da0b315e7bbdcabaaa89061`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Make Transmission stop reporting depend on process exit

- The existing Transmission script's timeout path passed the whitespace-separated result of `pidof transmission-daemon` as one quoted argument to `kill -9`, so multiple PIDs were not signaled individually. It printed `[KILLED]` without checking whether any daemon remained; signal and killall statuses were ignored.
- Reworked `func_stop()` to reject unexpected pidof errors and malformed PID tokens, signal each validated PID independently after the graceful timeout, and verify a final empty PID snapshot before reporting successful forced termination. A surviving PID now returns failure; a kill race is accepted only when the final snapshot proves absence.
- Added injectable tests for graceful termination, a two-PID forced stop, SIGKILL failure with surviving PIDs, and malformed pidof output (no signals permitted). Exact-head host/BusyBox storage Actions will verify the change.
- Parent main: ba51619ceea8efc084d28f6f38bb5a8bc084e8d7. Exact parent storage #43768122780 had passed host syntax/startup tests and was still building/running BusyBox ash. AI docs #43768122829 PASS; Process lifecycle #43768122785, Process inventory #43768122777, infosvr #43768122801, AdGuardHome #43768122808, AliDDNS #43768122886 and AliyunDrive #43768122908 PASS. XY-C1 #43768122787 and NVRAM #43768122782 were in progress. New exact-head storage and XY-C1 runs required. No L3/L4 claim.

---

## 2026-11-16 06:05 Beijing — Correct Transmission PID-unique settings staging

- Exact c0 USB storage run #43767296645 failed in the pre-existing-stage collision regression: the test fixture correctly rewrites a PID-qualified suffix, but the live launcher still used `settings.json.tmp.$`, so the intended collision did not occur and daemon startup proceeded.
- The intervening b21 checkpoint left the production source blob unchanged; this commit now changes the assignment to `CONFIG_TMP="${CONFIG_FILE}.tmp.$$"`. The existing collision regression will exercise the real noclobber failure and the ownership-aware cleanup guard, requiring the foreign file to remain byte-for-byte unchanged.
- No local repository checkout or local shell execution was available. Host shell and firmware BusyBox ash validation are pending on the exact new SHA; do not claim L2 until they pass.
- Parent main: `b21b95b3e1858ae752ef7dc34259204524e484e4`. Exact c0 AI handoff docs #43767296578 PASS; c0 Process inventory #43767296590, Process lifecycle #43767296606, infosvr #43767296612, AliDDNS #43767296621, AliyunDrive #43767296670 and AdGuardHome #43767296717 PASS. c0 XY-C1 #43767296597 predates this fix. New exact-head storage and XY-C1 checks required. No L3/L4 claim.

---

## 2026-11-16 — Repair Transmission storage regression and protect pre-existing settings stages

- Exact parent 4d2afeddba949b77dcbe1a8f6f950e9093438910 storage run #43765741973 failed before running the first launcher assertion: under set -u, run_transmission_config_failure assigned optional argument $6 without a default. The function now uses a zero default, so existing generator/empty/transform/rename calls remain valid while the missing-umask case can pass 1 explicitly.
- Source review found a data-loss path in Transmission's exclusive PID-qualified settings stage. When noclobber rejected an existing settings.json.tmp.$$ path, transmission_start_fail() still removed that unowned path. Cleanup now requires CONFIG_TMP_OWNED=1; ownership is set only after successful exclusive creation and cleared after atomic publication. Later generator/transform/rename failures still remove the stage owned by this process.
- Added a deterministic pre-existing stage collision regression. It requires failure before daemon start/publication and verifies the prior stage file remains byte-for-byte unchanged.
- The parent AI documentation check #43765741952 failed because dev_log.md reached 120 records. Applied the mandated batch rotation: oldest 100 complete entries moved verbatim, oldest first, to part-011; the newest 20 remain in root before this new record. Updated the archive index with exact bytes and Git blob SHA.
- Parent main: 4d2afeddba949b77dcbe1a8f6f950e9093438910. Source/test/archive/handoff/roadmap changes are in one atomic commit (THIS COMMIT). Parent XY-C1 run #43765742001 was still building at last check; it is not validation for this new SHA. No L4 claim.

---

## 2026-11-16 06:50 Beijing — Enforce generated Transmission settings content contract

- Exact a391 storage-hotplug run #43765111683 PASS across host shell and firmware BusyBox ash, including mkdir/chmod/chown failure injection; AI docs #43765111687, Process inventory #43765111691, Process lifecycle #43765111678, infosvr #43765111706, AliDDNS #43765111677, AliyunDrive #43765111712, AdGuardHome #43765111669 and NVRAM #43765111698 also PASS.
- Exact 7bab cleanup-state follow-up changed Transmission CONFIG_TMP initialization and inherited-environment cleanup guard; at last poll storage #43765528314, Process inventory #43765528359, Process lifecycle #43765528378, NVRAM #43765528331, AliDDNS #43765528342 and AliyunDrive #43765528351 were running; AI docs #43765528363, AdGuardHome #43765528318 and infosvr #43765528320 passed.
- The settings publisher still accepted nonempty malformed JSON lacking umask 18 and did not verify transformed umask 0. This slice adds both content checks and a malformed-output test.
- Added trunk/user/transmission/transmission.sh to the XY-C1 workflow push path so shell changes trigger an exact firmware build/TRX/artifact check.
- Parent main: 7bab2e0784e0e93e9defc451aed3fb8c89e448b9. Exact-head storage and XY-C1 checks are pending; C-source L3 remains scoped to b488a561 only. No L4.
- Next: verify this exact SHA, then inspect Transmission stop/restart lifecycle postconditions.

---

## 2026-11-16 06:22 Beijing — Initialize Transmission cleanup state before any fail-closed setup

- `transmission_start_fail()` cleans `CONFIG_TMP`. Review found directory setup can fail before the previous assignment to `CONFIG_TMP`, allowing an inherited environment variable to be mistaken for the active stage path. `func_start()` now clears `CONFIG_TMP` immediately on entry.
- Added host/BusyBox regression: force config-directory creation to fail while passing `CONFIG_TMP` as a pre-existing external file; require the file to remain nonempty and no daemon invocation.
- Exact a391 main push Actions all PASS: storage #43765111683, AI docs #43765111687, Process inventory #43765111678, Process lifecycle #43765111691, infosvr #43765111706, AliDDNS #43765111677, AliyunDrive #43765111712, AdGuardHome #43765111669 and NVRAM #43765111698. Exact a9 storage #43764529973 PASS. b488 XY-C1 run #43762907958 passed Build XY-C1, TRX verification and artifact upload (L3 only for b488 C source).
- Parent main: `a391be7a60c790eafd480cc4a70923e8c9e5c26b`. This follow-up is shell/test/docs only; exact-head Actions pending. Frozen backup unchanged; no L4 claim.
- Next: verify exact-head storage result and then audit Transmission stop/restart postconditions.

---

## 2026-11-16 06:05 Beijing — Fail closed on Transmission storage setup and ownership errors

- Replaced unchecked Transmission `mkdir -p` calls with `transmission_require_dir()`, which checks both command status and resulting directory postcondition for config, downloads, incomplete and watch paths. A failed watch chmod now aborts startup.
- Under SVC_ROOT=0, checks chmod of the storage root and download directory and chown of config/incomplete directories. Any failure is logged, returns nonzero and prevents `start-stop-daemon` invocation.
- Extended the shared launcher harness with injectable mkdir/chmod/chown failures and cases for each required directory/permission/ownership operation. Tests require that no daemon launch occurs after any setup failure and no config staging file is left behind.
- Parent main: `a9b32e72faab490c0bf5fbcd6680ff55d568324e`. Source/test/docs are staged for one atomic commit; exact-head storage Actions pending. No firmware C changes or new L3/L4 claim.
- Next: check exact-head storage host + BusyBox ash workflow, then inspect remaining unchecked optional setup and service postconditions.

---

## 2026-11-16 05:51 Beijing — Fix storage regression shell boundary and refresh PID-path docs

- Review of exact committed test source found the regenerated `run_launcher()` block had been concatenated directly with `assert_failure()` as `}assert_failure()`. Added the required newline so the shell parses the two function definitions separately.
- Corrected stale handoff/roadmap paths that had been rendered with one `$`: Aria config stage and barrier markers are `aria2.conf.tmp.$$`, `entered.$$`, `checked.$$`; Transmission uses `settings.json.tmp.$$`.
- Parent main: `5ae02f33db8cace2b45054df9e32df8c064b9d4c`. This follow-up changes test syntax and checkpoint wording only. Its exact-head storage CI run is pending; the earlier run on parent SHA is not treated as validation of this correction.

---

## 2026-11-16 05:45 Beijing — Stage Transmission default settings and fail closed

- Replaced the global `/tmp/settings.json` path with private, same-directory `settings.json.tmp.$$` staging. The stage is exclusively created under `umask 077`/shell noclobber, and the generator command, nonempty output, `sed` transformation, post-transform nonempty output, atomic same-directory `mv`, and final regular/nonempty settings file are checked.
- Existing symlink or non-regular settings paths now fail closed. A zero-byte regular settings file is treated as incomplete and regenerated; nonempty regular settings remain preserved.
- Added regression stubs for the Transmission generator and sed. Fault-injection cases cover generator nonzero, empty generator output, transform failure and rename failure; each requires no daemon launch, no published invalid settings and no leftover staging file. Success case starts with an empty config, checks transformed `"umask": 0,` content and a nonempty final file before daemon invocation. Test plumbing passes under host shell and firmware BusyBox ash.
- Parent main: `df267fd1457bb62e2a0c0fb0c359d3aa153f10e9`. Source/test/docs changed in one atomic commit. Full storage Actions for the new exact SHA are pending; no new firmware C change. b488 XY-C1 source L3 remains limited to that C source; no L4 claim.
- Next: check the exact-head storage host and BusyBox ash regressions. Then audit Transmission mkdir/chmod/chown and ownership preparation failure paths as a separate slice.

---

## 2026-11-16 05:31 Beijing — Preserve literal PID expansion in the Aria two-phase barrier

- Review of the committed b488a561 test file found its marker assignments still used constant `entered.$` and `checked.$` filenames. This follow-up uses replacement callbacks so both marker filenames preserve the shell PID expansion `$$` literally.
- Verified in the final test source that both marker assignments contain two consecutive dollar characters. The previous 05:27 note claimed this had already been applied; this entry corrects that premature statement.
- Parent main: `b488a561bb91df62aca810a6ff2c191c17f82b2b`. Changes: storage concurrency test plus handoff/roadmap/log. Exact-head Actions triggered; no new firmware C changes or L3/L4 claim.
- The two-phase barrier logic passed a focused host harness when marker names were PID-unique. Rerun the full storage regression under host shell and firmware BusyBox ash on this exact SHA.

---

## 2026-11-16 05:22 Beijing — Repair exact-head storage, handoff and non-MSD-Lite regressions

- Exact SHA `50e24f32f310998a71e976287981aeb45f3bcd99` storage-hotplug run #43762316227 passed, but a deterministic host reproduction showed the one-phase cat barrier could race: after two entries appeared, the first cat stub could return and let its launcher rename the temp file before the second stub counted it (observed counts `2` then `1`). Added a second `checked.$$` rendezvous; a two-shell host harness confirmed both stubs observe two staged files before either returns.
- AI docs #43762316097 failed because the handoff lacked the required literal `Before context exhaustion`. Infosvr #43762316266 passed its packet/signal policy binaries but its source checker rejected the handoff heading after it had been pluralized. The handoff restores the exact singular `## Exact first action for the next GPT` and adds the required phrase.
- Process lifecycle #43762316246 passed all compile/sanitizer tests, including the new MSD Lite config host test, then failed its integration checker because `start_udpxy()` no longer contained the `#else` `/usr/sbin/udpxy` launch path. That path is required on non-APP_MSD_LITE builds and has been restored; the checker already asserts its presence.
- Other exact-50e24 push Actions: storage #43762316227, AliDDNS #43762316320, AliyunDrive #43762316276, WAN route #43762316273, AdGuardHome #43762316224, HTTPD #43762316127, NVRAM #43762316105, Process inventory #43762316049 all SUCCESS. AI docs, infosvr and Process lifecycle failed as detailed above. XY-C1 #43762316142 was in progress at last check.
- Correction retained from previous entries: Transmission's live launcher still has unchecked settings generation, `sed`, and `mv`; it has no PID-qualified stage and remains open. Parent main: `50e24f32f310998a71e976287981aeb45f3bcd99`. No new L3/L4 claim.
- Next: run exact-head Actions for these three fixes, then implement Transmission's staged, checked `settings.json` generation/publication and host + BusyBox failure-injection tests.

---

## 2026-11-16 — Preserve last-good MSD Lite configuration on publication failure

- start_udpxy() previously opened the live MSD Lite config with "w" before validating the read-only template; missing source or buffered write/close failures could destroy the persistent config while service enable/launch still proceeded.
- Added a checked same-directory mkstemp publisher with source/destination file-type checks, mode preservation, interface/port validation, write/flush/fsync/close checks and atomic rename. Publication failure now aborts before msd_lite_enable or daemon launch.
- Added GNU89 ASan/UBSan regression coverage for exact transformed config, preserved unrelated lines and existing mode, missing source/parent, invalid input, directory/symlink targets, buffered RLIMIT_FSIZE failure, last-good preservation and staging cleanup.
- Local host regression PASS. Exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact are pending for the new C source; no L3/L4 claim.
- Parent main: f36d6ec94650103134a55b2e9c9abca9894de075. Frozen backup and archive index/shards unchanged.

---

## 2026-11-16 — Fix Aria concurrent-start staging collision and barrier

- Exact 13a16 storage-hotplug run #43760834648 failed in host startup testing with "First concurrent Aria start failed".
- Root cause was two literal-dollar mistakes: aria.sh used CONFIG_TMP="\${FILE_CONF}.tmp.$" instead of PID-qualified .tmp.$$; the test cat stub wrote its barrier marker to "$STORAGE_TEST_CAT_BARRIER/$", so both concurrent shells appeared as one participant and timed out.
- Changed the production config temp suffix and barrier marker to use "$$"; the start-daemon marker now records the PID too. The existing regression checks simultaneous distinct staged files, successful publication by both starts, dual daemon launch, and no leftover temp files.
- Exact 13a16 Process lifecycle #43760834624, Process inventory #43760834666, AI docs #43760834673, AliyunDrive #43760834634, AliDDNS #43760834671, infosvr #43760834623, AdGuardHome #43760834594 and NVRAM #43760834637 PASS; USB storage #43760834648 FAIL as detailed above.
- Exact-head storage validation for this repair pending. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; no new L3 or L4 claim.

---

## 2026-11-15 — Refresh exact-main handoff after storage launcher fixes

- Handoff checkpoint parent main: 13a16ec70f7f78f9a8114582a51e7d9e88bd9644; frozen backup branch: 72f4f157a084f8184b9597ca5bf334b8080e5807 (unchanged).
- Current-source storage workflow state: #43760834648 USB storage hotplug stability regression: completed/failure. Exact-main Actions: #43760834673 AI handoff documentation contract: completed/success
- #43760834671 AliDDNS signed API transport security regression: completed/success
- #43760834666 Process lifecycle inventory - 13a16ec70f7f78f9a8114582a51e7d9e88bd9644: completed/success
- #43760834648 USB storage hotplug stability regression: completed/failure
- #43760834637 NVRAM transaction boundary audit: completed/success
- #43760834634 AliyunDrive WebDAV lifecycle stability regression: completed/success
- #43760834624 Process inventory and lifecycle fail-closed regression: completed/success
- #43760834623 infosvr packet and signal lifecycle stability regression: completed/success
- #43760834594 AdGuardHome config security regression: completed/success.
- Work completed in this session includes WAN status/error propagation and L3 verification on source 401066, Transmission/Aria launch result fixes, Aria atomic default config and failure cleanup, and Transmission staged settings with generator/transform/rename checks plus host/BusyBox regression.
- No physical-router L4 result. Immediate next task is exact-SHA storage workflow verification and correction of any failing test/source path.

---

## 2026-11-16 04:50 Beijing — Enforce unique Aria default-config staging under concurrent starts

- The current main source still had `CONFIG_TMP="${FILE_CONF}.tmp.$"`, a constant literal suffix; this contradicted the previous log entry that said the PID expansion had been corrected. The preceding storage-hotplug run #43759497496 passed the existing write/rename fault tests but did not detect the collision.
- Changed the path to `aria2.conf.tmp.$$` and added a barrier-controlled concurrent-start regression: both shells must simultaneously observe two distinct staged files, both publish successfully, both reach daemon launch, and no temporary file remains. The test is wired to existing host-shell and firmware BusyBox ash workflow steps; no workflow filter change was needed.
- Parent main: 6ad669a672f8274f523723df4315e523841de5d5. Changed paths: trunk/user/aria2/aria.sh, trunk/user/scripts/tests/storage_app_start_status.sh, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. Exact-head storage Actions for THIS COMMIT pending; no local checkout/build or new MIPS/L4 claim.
- Next after exact-head validation: inspect Transmission default settings generation and every pre-launch setup/publish failure path. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-15 — Make Aria staged config filename process-unique

- Review of the just-committed Aria staging change found the JS-generated literal had collapsed the shell PID expansion from $$ to a single trailing dollar. That would have made the temporary name constant across concurrent starts.
- Corrected the assignment to CONFIG_TMP="${FILE_CONF}.tmp.$$" so concurrent invocations get distinct same-directory staging files.
- Parent main: 755e8c711f1becc9a33c369f156d7b66def904cb; the Aria config write/rename fault-injection checks are present but await exact-head storage CI. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-15 — Stage Aria default config and fail closed on setup errors

- Aria previously ignored config-directory/session-list/download-directory creation, permission, and default-config creation/publication failures, then could launch with missing or incomplete state.
- Added checks for config/session and downloads setup plus required permissions/ownership. The default config writes to aria2.conf.tmp.$$ in the same directory, checks the write and atomic rename, removes temporary files on failure, and returns before daemon launch.
- Extended storage_app_start_status.sh with cat/write and mv/rename fault injection. It asserts neither failure invokes start-stop-daemon or leaves a final/temporary config behind.
- Parent eddf9aac storage workflow #43758894731 completed SUCCESS: host shell syntax/status regression and firmware BusyBox ash regression all PASS. Current Aria staging follow-up requires a new exact-SHA storage run.
- No firmware C change; L3 remains the exact 401066 boundary only. Frozen backup unchanged; no L4 claim.

---

## 2026-11-15 — Preserve Transmission and Aria start command status

- Both launchers had false-success status propagation: failed start-stop-daemon was followed by echo and returned zero; successful launch was followed by logger, whose failure could falsely report startup failure.
- Changed both launchers to test start-stop-daemon directly and explicitly return 0 on success / 1 on failure, independently of logging.
- Added storage_app_start_status.sh with temporary directories and stubbed pidof/NVRAM/start-stop-daemon/logger/chmod/chown; it verifies launch failure with normal or failed logger and successful launch with failed logger.
- Added path filters plus host-shell and BusyBox ash syntax/regression steps to storage-hotplug-stability.yml.
- Parent main: 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6; XY-C1 #43756373315 Build/TRX/artifact all PASS on source 401066; firmware C unchanged. New storage workflow pending.
- Frozen backup unchanged; no L4 result.

---

## 2026-11-15 — Add forward declaration for early MAN route-status call

- `launch_wanx()` calls static `man_up_internal(char *, int, int, int *)` before the function definition later in `net_wan.c`. Added the file-scope prototype with unnamed parameter types to avoid implicit/non-static declaration errors under GNU89.
- Added a WAN source-contract assertion for the prototype. The parent b1b899 WAN callback #43756151245 and WAN route #43756151031 both PASS, including the required/optional MAN route checks and WISP abort tests.
- Parent exact module Actions: AI docs #43756151101, Process inventory #43756151205, HTTPD #43756151156, infosvr #43756151158, AliyunDrive #43756151160, AdGuardHome #43756151126, DNSMasq #43756151038, WAN teardown #43756151129, WAN ARP #43756151066, miniupnpd #43756151110, VPN #43756151125, USB identity #43756151191, detect_link #43756151181 and detect_internet #43756151047 PASS. Process lifecycle #43756151162, NVRAM #43756151222 and AliDDNS #43756151138 still running at last poll.
- Parent XY-C1 #43756151170 was in Build XY-C1; TRX/artifact pending. This source prototype triggers a fresh L3 validation; no current-source L3 or L4 claim.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-15 — Propagate WISP cleanup failure and repair WAN callback contract

- Exact parent 3f6 WAN route Action #43755792869 PASS; the WAN callback Action #43755792972 failed only in its Python source-contract step after all compiled policy executables passed. The failure was `NameError: wanx is not defined`; the subsequent static MAN assertion also still looked for the obsolete void `man_up()` call.
- `config_apcli_wisp()` now returns 0 for no-op/success and the final down status on unrecoverable old-MAN cleanup. `start_wan()` checks the status, removes callback links and aborts before IPTV DHCP/ZeroConf clients or later WAN startup.
- The workflow now binds the `launch_wanx()` body before route-specific assertions, expects required gateway status propagation, checks WISP abort ordering and ensures WISP cleanup runs before IPTV client startup.
- Parent main: `3f6ca487db9729987cdc6b3a49a2facf479bae7e`. Parent WAN callback #43755792972 FAIL as detailed above; WAN route #43755792869, AI docs #43755792956, Process inventory #43755793007, infosvr #43755792908, AdGuardHome #43755793061, AliyunDrive #43755792904 and AliDDNS #43755792868 PASS. Prior 41ea module Actions were PASS; NVRAM #43755793079 remained in progress.
- No local target compile was run. Exact new-head WAN and XY-C1 Build/TRX/artifact validation is required; frozen backup and archive unchanged; no L4 claim.
---
## 2026-11-15 — Correct MAN helper API expectations in route workflows

- Exact 41ea WAN callback #43755418926 failed at a stale source assertion expecting `route_setup_status = man_up_internal(wan_ifname, unit, 0)`; DHCP callback now intentionally passes `NULL` for the optional required-gateway out parameter. All six prior GNU89/host policy steps PASS.
- Exact 41ea WAN route #43755418941 failed at stale three-argument helper extraction for `add_man_gateway_routes()` after its required-route output parameter was added. Compile/sanitizer, resolver, bridge ownership and VLAN/bridge propagation checks PASS.
- Updated both workflow contracts to the extended signatures and added explicit critical host/default versus optional DNS route assertions. No product-source change.
- Parent main: `41ea56088a7c8cb8bddd16b3d59ce168c0784c91`; exact AI docs #43755418869, Process inventory #43755418919, Process lifecycle #43755418852, infosvr #43755418948, USB identity #43755418970, detect_link #43755418966, WAN teardown #43755418845, WAN ARP #43755418839, VPN #43755418832, AliDDNS #43755418826, AliyunDrive #43755418815, HTTPD #43755418812, detect_internet #43755418808, miniupnpd #43755418910 and DNSMasq #43755418936 all PASS. NVRAM #43755418947 still running.
- XY-C1 #43755418990 was at checkout/setup; no Build/TRX/artifact result. Frozen backup unchanged; no latest-source L3 or L4 claim.

---

## 2026-11-15 — Separate required MAN gateway routes from optional route errors

- `man_up_internal()` previously returned one aggregate containing critical gateway/default-route setup and optional configured static/DHCP/DNS routes. The static MAN caller used the void `man_up()` wrapper, so PPTP/L2TP could not abort for a required gateway failure without also treating optional routes as fatal.
- Added a separate `gateway_route_status` output covering only the off-subnet gateway host route and default route. User static routes, DHCP routes and DNS host routes remain in the aggregate return and are not fatal to static MAN PPP startup.
- Static MAN `launch_wanx()` now persists `wanx_err` and returns the required gateway error; the pre-existing non-PPPoE PPTP/L2TP guard blocks pppd. DHCP/ZeroConf callback callers retain aggregate semantics; PPPoE asynchronous/no-wait remains unchanged.
- Added workflow contracts for critical/optional route separation and exact caller propagation. Exact current-head WAN/XY-C1 Actions pending.
- Parent main: `2529f9a8aadd378347fa47578a1f048a793b0c1c`; exact parent WAN #43754701672, AI docs #43754701543, Process inventory #43754701614 and NVRAM ioctl audit #43754701523 PASS. XY-C1 #43754701433 was at Build with TRX/artifact pending.
- Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 claim.

---

## 2026-11-15 — Guard AP-client WISP migration on old WAN address cleanup

- `config_apcli_wisp()` ignored failure to clear the previous MAN interface address, then published the WISP interface as the active WAN. A stale address could remain on the old physical interface during migration.
- The function now records/logs address-clear failure. On non-single-MAC hardware it attempts to bring the old interface down; on single-MAC hardware it checks the existing down operation and retries a down-only ioctl if needed. If it cannot ensure the old interface is down, it refuses to publish the WISP interface, retaining the prior WAN interface selection.
- Added source-contract coverage for checked clear, both hardware-specific shutdown paths, retry/failure handling and migration publication ordering. Exact current-head Actions are pending.
- Parent main: `7ebeaa9e02e6249ac787497429defab2f726e5a0`; exact parent WAN callback #43754281208 PASS; AI docs #43754281251 PASS; Process inventory #43754281242 PASS; other finished module Actions include DNSMasq #43754281246, WAN route #43754281323 and HTTPD #43754281200 PASS. XY-C1 #43754281438 remained in Build XY-C1; NVRAM #43754281347 was still auditing ioctl call paths.
- Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 result is claimed.

---

## 2026-11-15 — Check physical WAN interface bring-up before protocol startup

- `start_wan()` previously issued `doSystem("ifconfig ... mtu 1500 up 0.0.0.0")` and discarded the `system()` return status. PPPoE with no MAN preseed could therefore proceed to pppd even when its physical interface bring-up failed; DHCP/static paths also lacked an early fail gate.
- Replaced the shell command with checked `set_interface_mtu()` and direct `ifconfig()`. MTU failure is logged but remains nonfatal (the interface may still operate with its existing MTU); a failed interface-up/address-clear operation logs, attempts to leave the interface down, and skips startup for that WAN unit.
- Added source-contract coverage for both status paths, cleanup and abort-before-ApCli/protocol startup. Direct IPoE static address failure handling is already present from parent `20f1bbd`.
- Parent main: `20f1bbd6eb138ec87b451cd2d7f424337db798bb`; exact-head WAN #43753689812 PASS, AI docs #43753689582 PASS, Process inventory #43753689669 PASS, WAN route status #43753689651 PASS. XY-C1 #43753689746 was still building at the last poll; NVRAM audit #43753689581 was still running.
- Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 result is claimed.

---

