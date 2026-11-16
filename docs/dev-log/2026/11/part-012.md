## 2026-11-15 — Check physical WAN interface bring-up before protocol startup

- `start_wan()` previously issued `doSystem("ifconfig ... mtu 1500 up 0.0.0.0")` and discarded the `system()` return status. PPPoE with no MAN preseed could therefore proceed to pppd even when its physical interface bring-up failed; DHCP/static paths also lacked an early fail gate.
- Replaced the shell command with checked `set_interface_mtu()` and direct `ifconfig()`. MTU failure is logged but remains nonfatal (the interface may still operate with its existing MTU); a failed interface-up/address-clear operation logs, attempts to leave the interface down, and skips startup for that WAN unit.
- Added source-contract coverage for both status paths, cleanup and abort-before-ApCli/protocol startup. Direct IPoE static address failure handling is already present from parent `20f1bbd`.
- Parent main: `20f1bbd6eb138ec87b451cd2d7f424337db798bb`; exact-head WAN #43753689812 PASS, AI docs #43753689582 PASS, Process inventory #43753689669 PASS, WAN route status #43753689651 PASS. XY-C1 #43753689746 was still building at the last poll; NVRAM audit #43753689581 was still running.
- Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 result is claimed.

---

## 2026-11-15 — Guard AP-client WISP migration on old WAN address cleanup

- `config_apcli_wisp()` ignored failure to clear the previous MAN interface address, then published the WISP interface as the active WAN. A stale address could remain on the old physical interface during migration.
- The function now records/logs address-clear failure. On non-single-MAC hardware it attempts to bring the old interface down; on single-MAC hardware it checks the existing down operation and retries a down-only ioctl if needed. If it cannot ensure the old interface is down, it refuses to publish the WISP interface, retaining the prior WAN interface selection.
- Added source-contract coverage for checked clear, both hardware-specific shutdown paths, retry/failure handling and migration publication ordering. Exact current-head Actions are pending.
- Parent main: `7ebeaa9e02e6249ac787497429defab2f726e5a0`; exact parent WAN callback #43754281208 PASS; AI docs #43754281251 PASS; Process inventory #43754281242 PASS; other finished module Actions include DNSMasq #43754281246, WAN route #43754281323 and HTTPD #43754281200 PASS. XY-C1 #43754281438 remained in Build XY-C1; NVRAM #43754281347 was still auditing ioctl call paths.
- Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 result is claimed.

---

## 2026-11-15 — Separate required MAN gateway routes from optional route errors

- `man_up_internal()` previously returned one aggregate containing critical gateway/default-route setup and optional configured static/DHCP/DNS routes. The static MAN caller used the void `man_up()` wrapper, so PPTP/L2TP could not abort for a required gateway failure without also treating optional routes as fatal.
- Added a separate `gateway_route_status` output covering only the off-subnet gateway host route and default route. User static routes, DHCP routes and DNS host routes remain in the aggregate return and are not fatal to static MAN PPP startup.
- Static MAN `launch_wanx()` now persists `wanx_err` and returns the required gateway error; the pre-existing non-PPPoE PPTP/L2TP guard blocks pppd. DHCP/ZeroConf callback callers retain aggregate semantics; PPPoE asynchronous/no-wait remains unchanged.
- Added workflow contracts for critical/optional route separation and exact caller propagation. Exact current-head WAN/XY-C1 Actions pending.
- Parent main: `2529f9a8aadd378347fa47578a1f048a793b0c1c`; exact parent WAN #43754701672, AI docs #43754701543, Process inventory #43754701614 and NVRAM ioctl audit #43754701523 PASS. XY-C1 #43754701433 was at Build with TRX/artifact pending.
- Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 claim.

---

## 2026-11-15 — Correct MAN helper API expectations in route workflows

- Exact 41ea WAN callback #43755418926 failed at a stale source assertion expecting `route_setup_status = man_up_internal(wan_ifname, unit, 0)`; DHCP callback now intentionally passes `NULL` for the optional required-gateway out parameter. All six prior GNU89/host policy steps PASS.
- Exact 41ea WAN route #43755418941 failed at stale three-argument helper extraction for `add_man_gateway_routes()` after its required-route output parameter was added. Compile/sanitizer, resolver, bridge ownership and VLAN/bridge propagation checks PASS.
- Updated both workflow contracts to the extended signatures and added explicit critical host/default versus optional DNS route assertions. No product-source change.
- Parent main: `41ea56088a7c8cb8bddd16b3d59ce168c0784c91`; exact AI docs #43755418869, Process inventory #43755418919, Process lifecycle #43755418852, infosvr #43755418948, USB identity #43755418970, detect_link #43755418966, WAN teardown #43755418845, WAN ARP #43755418839, VPN #43755418832, AliDDNS #43755418826, AliyunDrive #43755418815, HTTPD #43755418812, detect_internet #43755418808, miniupnpd #43755418910 and DNSMasq #43755418936 all PASS. NVRAM #43755418947 still running.
- XY-C1 #43755418990 was at checkout/setup; no Build/TRX/artifact result. Frozen backup unchanged; no latest-source L3 or L4 claim.

---

## 2026-11-15 — Propagate WISP cleanup failure and repair WAN callback contract

- Exact parent 3f6 WAN route Action #43755792869 PASS; the WAN callback Action #43755792972 failed only in its Python source-contract step after all compiled policy executables passed. The failure was `NameError: wanx is not defined`; the subsequent static MAN assertion also still looked for the obsolete void `man_up()` call.
- `config_apcli_wisp()` now returns 0 for no-op/success and the final down status on unrecoverable old-MAN cleanup. `start_wan()` checks the status, removes callback links and aborts before IPTV DHCP/ZeroConf clients or later WAN startup.
- The workflow now binds the `launch_wanx()` body before route-specific assertions, expects required gateway status propagation, checks WISP abort ordering and ensures WISP cleanup runs before IPTV client startup.
- Parent main: `3f6ca487db9729987cdc6b3a49a2facf479bae7e`. Parent WAN callback #43755792972 FAIL as detailed above; WAN route #43755792869, AI docs #43755792956, Process inventory #43755793007, infosvr #43755792908, AdGuardHome #43755793061, AliyunDrive #43755792904 and AliDDNS #43755792868 PASS. Prior 41ea module Actions were PASS; NVRAM #43755793079 remained in progress.
- No local target compile was run. Exact new-head WAN and XY-C1 Build/TRX/artifact validation is required; frozen backup and archive unchanged; no L4 claim.
---
## 2026-11-15 — Add forward declaration for early MAN route-status call

- `launch_wanx()` calls static `man_up_internal(char *, int, int, int *)` before the function definition later in `net_wan.c`. Added the file-scope prototype with unnamed parameter types to avoid implicit/non-static declaration errors under GNU89.
- Added a WAN source-contract assertion for the prototype. The parent b1b899 WAN callback #43756151245 and WAN route #43756151031 both PASS, including the required/optional MAN route checks and WISP abort tests.
- Parent exact module Actions: AI docs #43756151101, Process inventory #43756151205, HTTPD #43756151156, infosvr #43756151158, AliyunDrive #43756151160, AdGuardHome #43756151126, DNSMasq #43756151038, WAN teardown #43756151129, WAN ARP #43756151066, miniupnpd #43756151110, VPN #43756151125, USB identity #43756151191, detect_link #43756151181 and detect_internet #43756151047 PASS. Process lifecycle #43756151162, NVRAM #43756151222 and AliDDNS #43756151138 still running at last poll.
- Parent XY-C1 #43756151170 was in Build XY-C1; TRX/artifact pending. This source prototype triggers a fresh L3 validation; no current-source L3 or L4 claim.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-15 — Preserve Transmission and Aria start command status

- Both launchers had false-success status propagation: failed start-stop-daemon was followed by echo and returned zero; successful launch was followed by logger, whose failure could falsely report startup failure.
- Changed both launchers to test start-stop-daemon directly and explicitly return 0 on success / 1 on failure, independently of logging.
- Added storage_app_start_status.sh with temporary directories and stubbed pidof/NVRAM/start-stop-daemon/logger/chmod/chown; it verifies launch failure with normal or failed logger and successful launch with failed logger.
- Added path filters plus host-shell and BusyBox ash syntax/regression steps to storage-hotplug-stability.yml.
- Parent main: 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6; XY-C1 #43756373315 Build/TRX/artifact all PASS on source 401066; firmware C unchanged. New storage workflow pending.
- Frozen backup unchanged; no L4 result.

---

## 2026-11-15 — Stage Aria default config and fail closed on setup errors

- Aria previously ignored config-directory/session-list/download-directory creation, permission, and default-config creation/publication failures, then could launch with missing or incomplete state.
- Added checks for config/session and downloads setup plus required permissions/ownership. The default config writes to aria2.conf.tmp.$$ in the same directory, checks the write and atomic rename, removes temporary files on failure, and returns before daemon launch.
- Extended storage_app_start_status.sh with cat/write and mv/rename fault injection. It asserts neither failure invokes start-stop-daemon or leaves a final/temporary config behind.
- Parent eddf9aac storage workflow #43758894731 completed SUCCESS: host shell syntax/status regression and firmware BusyBox ash regression all PASS. Current Aria staging follow-up requires a new exact-SHA storage run.
- No firmware C change; L3 remains the exact 401066 boundary only. Frozen backup unchanged; no L4 claim.

---

## 2026-11-15 — Make Aria staged config filename process-unique

- Review of the just-committed Aria staging change found the JS-generated literal had collapsed the shell PID expansion from $$ to a single trailing dollar. That would have made the temporary name constant across concurrent starts.
- Corrected the assignment to CONFIG_TMP="${FILE_CONF}.tmp.$$" so concurrent invocations get distinct same-directory staging files.
- Parent main: 755e8c711f1becc9a33c369f156d7b66def904cb; the Aria config write/rename fault-injection checks are present but await exact-head storage CI. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-16 04:50 Beijing — Enforce unique Aria default-config staging under concurrent starts

- The current main source still had `CONFIG_TMP="${FILE_CONF}.tmp.$"`, a constant literal suffix; this contradicted the previous log entry that said the PID expansion had been corrected. The preceding storage-hotplug run #43759497496 passed the existing write/rename fault tests but did not detect the collision.
- Changed the path to `aria2.conf.tmp.$$` and added a barrier-controlled concurrent-start regression: both shells must simultaneously observe two distinct staged files, both publish successfully, both reach daemon launch, and no temporary file remains. The test is wired to existing host-shell and firmware BusyBox ash workflow steps; no workflow filter change was needed.
- Parent main: 6ad669a672f8274f523723df4315e523841de5d5. Changed paths: trunk/user/aria2/aria.sh, trunk/user/scripts/tests/storage_app_start_status.sh, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. Exact-head storage Actions for THIS COMMIT pending; no local checkout/build or new MIPS/L4 claim.
- Next after exact-head validation: inspect Transmission default settings generation and every pre-launch setup/publish failure path. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

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

## 2026-11-16 — Fix Aria concurrent-start staging collision and barrier

- Exact 13a16 storage-hotplug run #43760834648 failed in host startup testing with "First concurrent Aria start failed".
- Root cause was two literal-dollar mistakes: aria.sh used CONFIG_TMP="\${FILE_CONF}.tmp.$" instead of PID-qualified .tmp.$$; the test cat stub wrote its barrier marker to "$STORAGE_TEST_CAT_BARRIER/$", so both concurrent shells appeared as one participant and timed out.
- Changed the production config temp suffix and barrier marker to use "$$"; the start-daemon marker now records the PID too. The existing regression checks simultaneous distinct staged files, successful publication by both starts, dual daemon launch, and no leftover temp files.
- Exact 13a16 Process lifecycle #43760834624, Process inventory #43760834666, AI docs #43760834673, AliyunDrive #43760834634, AliDDNS #43760834671, infosvr #43760834623, AdGuardHome #43760834594 and NVRAM #43760834637 PASS; USB storage #43760834648 FAIL as detailed above.
- Exact-head storage validation for this repair pending. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; no new L3 or L4 claim.

---

## 2026-11-16 — Preserve last-good MSD Lite configuration on publication failure

- start_udpxy() previously opened the live MSD Lite config with "w" before validating the read-only template; missing source or buffered write/close failures could destroy the persistent config while service enable/launch still proceeded.
- Added a checked same-directory mkstemp publisher with source/destination file-type checks, mode preservation, interface/port validation, write/flush/fsync/close checks and atomic rename. Publication failure now aborts before msd_lite_enable or daemon launch.
- Added GNU89 ASan/UBSan regression coverage for exact transformed config, preserved unrelated lines and existing mode, missing source/parent, invalid input, directory/symlink targets, buffered RLIMIT_FSIZE failure, last-good preservation and staging cleanup.
- Local host regression PASS. Exact-head Process lifecycle host regression and XY-C1 Build/TRX/artifact are pending for the new C source; no L3/L4 claim.
- Parent main: f36d6ec94650103134a55b2e9c9abca9894de075. Frozen backup and archive index/shards unchanged.

---

## 2026-11-16 05:22 Beijing — Repair exact-head storage, handoff and non-MSD-Lite regressions

- Exact SHA `50e24f32f310998a71e976287981aeb45f3bcd99` storage-hotplug run #43762316227 passed, but a deterministic host reproduction showed the one-phase cat barrier could race: after two entries appeared, the first cat stub could return and let its launcher rename the temp file before the second stub counted it (observed counts `2` then `1`). Added a second `checked.$$` rendezvous; a two-shell host harness confirmed both stubs observe two staged files before either returns.
- AI docs #43762316097 failed because the handoff lacked the required literal `Before context exhaustion`. Infosvr #43762316266 passed its packet/signal policy binaries but its source checker rejected the handoff heading after it had been pluralized. The handoff restores the exact singular `## Exact first action for the next GPT` and adds the required phrase.
- Process lifecycle #43762316246 passed all compile/sanitizer tests, including the new MSD Lite config host test, then failed its integration checker because `start_udpxy()` no longer contained the `#else` `/usr/sbin/udpxy` launch path. That path is required on non-APP_MSD_LITE builds and has been restored; the checker already asserts its presence.
- Other exact-50e24 push Actions: storage #43762316227, AliDDNS #43762316320, AliyunDrive #43762316276, WAN route #43762316273, AdGuardHome #43762316224, HTTPD #43762316127, NVRAM #43762316105, Process inventory #43762316049 all SUCCESS. AI docs, infosvr and Process lifecycle failed as detailed above. XY-C1 #43762316142 was in progress at last check.
- Correction retained from previous entries: Transmission's live launcher still has unchecked settings generation, `sed`, and `mv`; it has no PID-qualified stage and remains open. Parent main: `50e24f32f310998a71e976287981aeb45f3bcd99`. No new L3/L4 claim.
- Next: run exact-head Actions for these three fixes, then implement Transmission's staged, checked `settings.json` generation/publication and host + BusyBox failure-injection tests.

---

## 2026-11-16 05:31 Beijing — Preserve literal PID expansion in the Aria two-phase barrier

- Review of the committed b488a561 test file found its marker assignments still used constant `entered.$` and `checked.$` filenames. This follow-up uses replacement callbacks so both marker filenames preserve the shell PID expansion `$$` literally.
- Verified in the final test source that both marker assignments contain two consecutive dollar characters. The previous 05:27 note claimed this had already been applied; this entry corrects that premature statement.
- Parent main: `b488a561bb91df62aca810a6ff2c191c17f82b2b`. Changes: storage concurrency test plus handoff/roadmap/log. Exact-head Actions triggered; no new firmware C changes or L3/L4 claim.
- The two-phase barrier logic passed a focused host harness when marker names were PID-unique. Rerun the full storage regression under host shell and firmware BusyBox ash on this exact SHA.

---

## 2026-11-16 05:45 Beijing — Stage Transmission default settings and fail closed

- Replaced the global `/tmp/settings.json` path with private, same-directory `settings.json.tmp.$$` staging. The stage is exclusively created under `umask 077`/shell noclobber, and the generator command, nonempty output, `sed` transformation, post-transform nonempty output, atomic same-directory `mv`, and final regular/nonempty settings file are checked.
- Existing symlink or non-regular settings paths now fail closed. A zero-byte regular settings file is treated as incomplete and regenerated; nonempty regular settings remain preserved.
- Added regression stubs for the Transmission generator and sed. Fault-injection cases cover generator nonzero, empty generator output, transform failure and rename failure; each requires no daemon launch, no published invalid settings and no leftover staging file. Success case starts with an empty config, checks transformed `"umask": 0,` content and a nonempty final file before daemon invocation. Test plumbing passes under host shell and firmware BusyBox ash.
- Parent main: `df267fd1457bb62e2a0c0fb0c359d3aa153f10e9`. Source/test/docs changed in one atomic commit. Full storage Actions for the new exact SHA are pending; no new firmware C change. b488 XY-C1 source L3 remains limited to that C source; no L4 claim.
- Next: check the exact-head storage host and BusyBox ash regressions. Then audit Transmission mkdir/chmod/chown and ownership preparation failure paths as a separate slice.

---

## 2026-11-16 05:51 Beijing — Fix storage regression shell boundary and refresh PID-path docs

- Review of exact committed test source found the regenerated `run_launcher()` block had been concatenated directly with `assert_failure()` as `}assert_failure()`. Added the required newline so the shell parses the two function definitions separately.
- Corrected stale handoff/roadmap paths that had been rendered with one `$`: Aria config stage and barrier markers are `aria2.conf.tmp.$$`, `entered.$$`, `checked.$$`; Transmission uses `settings.json.tmp.$$`.
- Parent main: `5ae02f33db8cace2b45054df9e32df8c064b9d4c`. This follow-up changes test syntax and checkpoint wording only. Its exact-head storage CI run is pending; the earlier run on parent SHA is not treated as validation of this correction.

---

## 2026-11-16 06:05 Beijing — Fail closed on Transmission storage setup and ownership errors

- Replaced unchecked Transmission `mkdir -p` calls with `transmission_require_dir()`, which checks both command status and resulting directory postcondition for config, downloads, incomplete and watch paths. A failed watch chmod now aborts startup.
- Under SVC_ROOT=0, checks chmod of the storage root and download directory and chown of config/incomplete directories. Any failure is logged, returns nonzero and prevents `start-stop-daemon` invocation.
- Extended the shared launcher harness with injectable mkdir/chmod/chown failures and cases for each required directory/permission/ownership operation. Tests require that no daemon launch occurs after any setup failure and no config staging file is left behind.
- Parent main: `a9b32e72faab490c0bf5fbcd6680ff55d568324e`. Source/test/docs are staged for one atomic commit; exact-head storage Actions pending. No firmware C changes or new L3/L4 claim.
- Next: check exact-head storage host + BusyBox ash workflow, then inspect remaining unchecked optional setup and service postconditions.

---

## 2026-11-16 06:22 Beijing — Initialize Transmission cleanup state before any fail-closed setup

- `transmission_start_fail()` cleans `CONFIG_TMP`. Review found directory setup can fail before the previous assignment to `CONFIG_TMP`, allowing an inherited environment variable to be mistaken for the active stage path. `func_start()` now clears `CONFIG_TMP` immediately on entry.
- Added host/BusyBox regression: force config-directory creation to fail while passing `CONFIG_TMP` as a pre-existing external file; require the file to remain nonempty and no daemon invocation.
- Exact a391 main push Actions all PASS: storage #43765111683, AI docs #43765111687, Process inventory #43765111678, Process lifecycle #43765111691, infosvr #43765111706, AliDDNS #43765111677, AliyunDrive #43765111712, AdGuardHome #43765111669 and NVRAM #43765111698. Exact a9 storage #43764529973 PASS. b488 XY-C1 run #43762907958 passed Build XY-C1, TRX verification and artifact upload (L3 only for b488 C source).
- Parent main: `a391be7a60c790eafd480cc4a70923e8c9e5c26b`. This follow-up is shell/test/docs only; exact-head Actions pending. Frozen backup unchanged; no L4 claim.
- Next: verify exact-head storage result and then audit Transmission stop/restart postconditions.

---

## 2026-11-16 06:50 Beijing — Enforce generated Transmission settings content contract

- Exact a391 storage-hotplug run #43765111683 PASS across host shell and firmware BusyBox ash, including mkdir/chmod/chown failure injection; AI docs #43765111687, Process inventory #43765111691, Process lifecycle #43765111678, infosvr #43765111706, AliDDNS #43765111677, AliyunDrive #43765111712, AdGuardHome #43765111669 and NVRAM #43765111698 also PASS.
- Exact 7bab cleanup-state follow-up changed Transmission CONFIG_TMP initialization and inherited-environment cleanup guard; at last poll storage #43765528314, Process inventory #43765528359, Process lifecycle #43765528378, NVRAM #43765528331, AliDDNS #43765528342 and AliyunDrive #43765528351 were running; AI docs #43765528363, AdGuardHome #43765528318 and infosvr #43765528320 passed.
- The settings publisher still accepted nonempty malformed JSON lacking umask 18 and did not verify transformed umask 0. This slice adds both content checks and a malformed-output test.
- Added trunk/user/transmission/transmission.sh to the XY-C1 workflow push path so shell changes trigger an exact firmware build/TRX/artifact check.
- Parent main: 7bab2e0784e0e93e9defc451aed3fb8c89e448b9. Exact-head storage and XY-C1 checks are pending; C-source L3 remains scoped to b488a561 only. No L4.
- Next: verify this exact SHA, then inspect Transmission stop/restart lifecycle postconditions.

---

## 2026-11-16 — Repair Transmission storage regression and protect pre-existing settings stages

- Exact parent 4d2afeddba949b77dcbe1a8f6f950e9093438910 storage run #43765741973 failed before running the first launcher assertion: under set -u, run_transmission_config_failure assigned optional argument $6 without a default. The function now uses a zero default, so existing generator/empty/transform/rename calls remain valid while the missing-umask case can pass 1 explicitly.
- Source review found a data-loss path in Transmission's exclusive PID-qualified settings stage. When noclobber rejected an existing settings.json.tmp.$$ path, transmission_start_fail() still removed that unowned path. Cleanup now requires CONFIG_TMP_OWNED=1; ownership is set only after successful exclusive creation and cleared after atomic publication. Later generator/transform/rename failures still remove the stage owned by this process.
- Added a deterministic pre-existing stage collision regression. It requires failure before daemon start/publication and verifies the prior stage file remains byte-for-byte unchanged.
- The parent AI documentation check #43765741952 failed because dev_log.md reached 120 records. Applied the mandated batch rotation: oldest 100 complete entries moved verbatim, oldest first, to part-011; the newest 20 remain in root before this new record. Updated the archive index with exact bytes and Git blob SHA.
- Parent main: 4d2afeddba949b77dcbe1a8f6f950e9093438910. Source/test/archive/handoff/roadmap changes are in one atomic commit (THIS COMMIT). Parent XY-C1 run #43765742001 was still building at last check; it is not validation for this new SHA. No L4 claim.

---

## 2026-11-16 06:05 Beijing — Correct Transmission PID-unique settings staging

- Exact c0 USB storage run #43767296645 failed in the pre-existing-stage collision regression: the test fixture correctly rewrites a PID-qualified suffix, but the live launcher still used `settings.json.tmp.$`, so the intended collision did not occur and daemon startup proceeded.
- The intervening b21 checkpoint left the production source blob unchanged; this commit now changes the assignment to `CONFIG_TMP="${CONFIG_FILE}.tmp.$$"`. The existing collision regression will exercise the real noclobber failure and the ownership-aware cleanup guard, requiring the foreign file to remain byte-for-byte unchanged.
- No local repository checkout or local shell execution was available. Host shell and firmware BusyBox ash validation are pending on the exact new SHA; do not claim L2 until they pass.
- Parent main: `b21b95b3e1858ae752ef7dc34259204524e484e4`. Exact c0 AI handoff docs #43767296578 PASS; c0 Process inventory #43767296590, Process lifecycle #43767296606, infosvr #43767296612, AliDDNS #43767296621, AliyunDrive #43767296670 and AdGuardHome #43767296717 PASS. c0 XY-C1 #43767296597 predates this fix. New exact-head storage and XY-C1 checks required. No L3/L4 claim.

---

## 2026-11-16 — Make Transmission stop reporting depend on process exit

- The existing Transmission script's timeout path passed the whitespace-separated result of `pidof transmission-daemon` as one quoted argument to `kill -9`, so multiple PIDs were not signaled individually. It printed `[KILLED]` without checking whether any daemon remained; signal and killall statuses were ignored.
- Reworked `func_stop()` to reject unexpected pidof errors and malformed PID tokens, signal each validated PID independently after the graceful timeout, and verify a final empty PID snapshot before reporting successful forced termination. A surviving PID now returns failure; a kill race is accepted only when the final snapshot proves absence.
- Added injectable tests for graceful termination, a two-PID forced stop, SIGKILL failure with surviving PIDs, and malformed pidof output (no signals permitted). Exact-head host/BusyBox storage Actions will verify the change.
- Parent main: ba51619ceea8efc084d28f6f38bb5a8bc084e8d7. Exact parent storage #43768122780 had passed host syntax/startup tests and was still building/running BusyBox ash. AI docs #43768122829 PASS; Process lifecycle #43768122785, Process inventory #43768122777, infosvr #43768122801, AdGuardHome #43768122808, AliDDNS #43768122886 and AliyunDrive #43768122908 PASS. XY-C1 #43768122787 and NVRAM #43768122782 were in progress. New exact-head storage and XY-C1 runs required. No L3/L4 claim.

---

## 2026-11-16 — Repair Transmission stop PID validation and regression harness

- Exact parent 814d storage #43768621241 failed before executing tests: POSIX `sh -n` rejected the unquoted-space glob `*[!0-9 ]*` in the new Transmission stop PID validation.
- Replaced the malformed class with per-token numeric validation (`''|*[!0-9]*`) before sending any signals. Also corrected the test harness's AWK extraction (now uses a sed function range), changed double-escaped newline format strings into real line delimiters, and added an assertion that restart is gated by `func_stop || exit 1`.
- Parent exact AI docs #43768621220 PASS; NVRAM #43768621188, Process inventory #43768621322, Process lifecycle #43768621183, infosvr #43768621156, AliDDNS #43768621223, AdGuardHome #43768621236 and AliyunDrive #43768621152 PASS. Parent XY-C1 #43768621137 was in progress at last inspection.
- No local repository checkout was available; no local shell test is claimed. Exact-head storage host + BusyBox ash and XY-C1 Build/TRX/artifact Actions are required for this repair.
- Parent main: `814d4639643675ed9da0b315e7bbdcabaaa89061`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Reject IGMP proxy configuration token injection

- Review of the atomic IGMP proxy publisher found that the old CR/LF-only validation still accepted spaces/tabs and '#' inside NVRAM-derived `altnet` and interface values. These characters can split a generated directive into unintended parser tokens or comment out its remainder without a newline.
- Added a printable single-token validator for interface names (reject controls, whitespace and '#') and restrict `altnet` to numeric IPv4/CIDR punctuation (digits, '.' and '/'). Tests verify altnet whitespace, interface comment/tab injection all return EINVAL and leave the last-good config unchanged.
- Parent abd40 USB storage #43769497238 PASS, AI docs #43769497316 PASS, Process lifecycle #43769497255 PASS, AdGuardHome #43769497209 PASS and infosvr #43769497293 PASS. XY-C1 #43769497292, NVRAM #43769497231, inventory #43769497244, AliDDNS #43769497245 and AliyunDrive #43769497299 were pending at last poll.
- No local checkout was available; no local C test is claimed. Exact-head Process lifecycle (GNU89 ASan/UBSan) and XY-C1 Build/TRX/artifact are pending.
- Parent main: `abd40abba2c3b78bf02fb262110f485534ba8b4b`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Restore IGMP regression fixture before legacy failure cases

- Exact e349 Process lifecycle #43770121710 failed in the IGMP config regression: the new token-injection tests wrote `last-good-token-validation`, while existing missing-parent/rename assertions still expected `last-good-configuration`. The production helper built and the run reached the later fixture comparison.
- Restored the original last-good fixture after the new altnet whitespace, interface comment and downstream-tab rejection assertions, keeping all older error-path tests independent.
- Exact e349 AI docs #43770121750, inventory #43770121725, AdGuardHome #43770121883, infosvr #43770121820, AliyunDrive #43770121777 and AliDDNS #43770121720 PASS; NVRAM #43770121752 and XY-C1 #43770121908 were in progress. Exact abd40 storage #43769497238 PASS.
- No local checkout/test run is claimed. Corrected exact-head Process lifecycle regression and XY-C1 Build/TRX/artifact validation required.
- Parent main: `e3499afc01bdc42cd1620eb18a6eec0e20541d36`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Harden Aria2 stop lifecycle and align launcher regressions

- Aria `func_stop()` previously passed the entire `pidof aria2c` PID list as a single quoted `kill -9` argument, did not verify process absence afterward, and allowed `restart` to ignore stop failure.
- Aria now treats pidof status/output inconsistencies as UNKNOWN, rejects malformed and unsafe PID 0/1 values, signals each process separately, confirms post-SIGKILL daemon absence and fails closed on residual/unknown state. Restart now gates on successful stop.
- Transmission stop validation also now rejects PID 0/1 and pidof success with empty output rather than treating that inconsistent response as proof of absence.
- Generalized storage lifecycle regressions to both launchers: no daemon, graceful stop, multiple forced PIDs, killall failure followed by successful forced stop, failed SIGKILL, malformed/unsafe PID, pidof error, successful-empty pidof and restart gating. Workflow invokes the helper under host shell and firmware BusyBox ash.
- Exact parent 662 Process lifecycle #43770376409 PASS, inventory #43770376403 PASS, AI docs #43770376449 PASS, NVRAM #43770376543 PASS, AdGuardHome #43770376442 PASS, infosvr #43770376527 PASS, AliDDNS #43770376582 PASS and AliyunDrive #43770376415 PASS. XY-C1 #43770376467 was still building at last poll.
- No local checkout was available; no local execution is claimed. Exact-head storage host + BusyBox ash and XY-C1 Build/TRX/artifact validation are required.
- Parent main: `662c8d279a8bea874997e1d5fb74623d3a322640`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No L3/L4 claim.

---

## 2026-11-16 — Propagate RC syslogd/klogd startup failures

- `start_logger()` previously discarded both `start_syslogd()` and `start_klogd()` statuses and always returned zero. Existing syslog-restart and time-update notification paths check `start_logger(0) != 0`, so logger launch failures were silently acknowledged.
- It now attempts both daemons, logs both individual failures and returns the first nonzero status. A GNU89 regression covers success, each individual failure, simultaneous failures and negative statuses; production integration checks verify unconditional attempts and caller-facing propagation.
- Exact parent 828 USB storage #43771300948 PASS, including host and firmware BusyBox 1.24.2 ash tests for Aria/Transmission and full storage checks. Exact-parent Process #43771301002, inventory #43771301076, AI docs #43771300969, AdGuardHome #43771300992, AliyunDrive #43771300907, AliDDNS #43771300910, infosvr #43771301013 and NVRAM #43771300993 PASS. XY-C1 #43771300972 was still building.
- Exact-head Process lifecycle and XY-C1 Build/TRX/artifact checks are required for this RC source. No local cross-build or target/L4 result is claimed.

---

## 2026-11-16 — Propagate boot-service startup failures through start_services_once()

- `start_services_once()` ignored the status from HTTPD, telnetd, optional SSHD, VPN, watchdog, GPIO button, infosvr, UPnP, LLTD, crond, networkmap, rstats and already-checked optional app/command startups, then returned zero. Its sole RC caller also ignored the result. Failures could be individually logged but there was no truthful aggregate status at the boot boundary.
- It now preserves the first nonzero status while attempting every later startup action, and `rc.c` logs an aggregate failure. LLTD, crond, rstats and infosvr are aggregated only when their respective NVRAM feature switches are enabled; their disabled return sentinel (1) is not misreported. Void-returning helpers remain outside this slice.
- Added a GNU89 policy regression, source-level caller/feature-guard assertions and exact-path Process lifecycle + XY-C1 workflow coverage.
- Parent main: `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact-parent Process #43772230234 PASS and AI docs #43772230258 PASS. XY-C1 #43772230305 was in progress at last poll (Build XY-C1); TRX/artifact pending.
- No local firmware build or L3/L4 claim for this new source. Exact-head Actions are required.

---

## 2026-11-16 — Repair exact-head process, XY-C1 and handoff validation regressions

- Exact d444 Process lifecycle #43773527020 executed the new GNU89 boot-service status reducer and RC logger reducer successfully, along with the existing regression suite, then failed because the appended integration checker encoded the newline delimiter as a literal backslash-n. Corrected the Python delimiter so it splits the real C function source.
- Exact d444 XY-C1 #43773526843 failed before firmware build: `check_samba_shared_header.py` enforces that XY-C1 remains push-only. The new source/test push path entries are retained; the accidental pull_request trigger has been removed.
- Exact d444 AI handoff docs #43773527100 failed because the checkpoint contract requires the literal phrases `Mandatory handoff checkpoint` and `The checkpoint is mandatory; a document edit is conditional.`. Added the exact contractual wording.
- The target build, TRX and artifact steps were skipped after the Samba preflight failure. No L3 claim. New exact-head Process, AI-docs and XY-C1 runs must confirm this correction.

---

## 2026-11-16 — Correct networkmap status-binding checker and handoff contract wording

- Exact 0e27 Process lifecycle #43773995752 compiled and ran the GNU89 boot-service first-failure and logger policy binaries successfully, then failed in the final production integration checker. The assertion incorrectly expected `start_networkmap(1)` to be nested directly in the reducer; production captures it in `networkmap_start_status`, logs a useful diagnostic and then reduces that local. Updated the checker to verify the real binding + aggregation contract rather than require a different implementation.
- Exact 0e27 AI docs #43773995692 failed because the handoff used "conditional on material status changes" instead of the governance validator's exact sentence "The checkpoint is mandatory; a document edit is conditional." Restored exact wording and kept the materiality explanation as a separate sentence.
- Exact 0e27 XY-C1 #43773995690 remained in progress at last poll; the corrected workflow is push-only, with the new boot-service header/test paths present once in its push filter.
- No target Build/TRX/artifact result is inferred from a queued/in-progress run. No L3/L4 claim.

---

## 2026-11-16 — Complete the mandatory handoff checkpoint wording

- Exact main SHA 118 Process lifecycle #43774359240 PASS after correcting the networkmap checker to assert the captured local status and its subsequent aggregation.
- Exact 118 AI handoff docs #43774359299 failed on the handoff validator's required literal phrase `Before context exhaustion`. The checkpoint section now states that requirement alongside the exact mandatory checkpoint sentence.
- XY-C1 run #43773995690 remained in progress on source SHA 0e27 at last poll. No Build/TRX/artifact or L3 claim.
- This is a docs/validation-contract correction only; firmware source did not change.

---

## 2026-11-15 — Align GPIO, Crond and watchdog boot checks with aggregated startup status

- GPIO, Crond and watchdog source-contract tests required bare `start_gpio_btn();`, `start_crond();` and `start_watchdog();` calls even though the production boot routine intentionally wraps each status in `rc_first_service_start_status()`.
- Updated the three test contracts to require the service start call inside the production first-failure aggregation expression; this preserves the service-start requirement while validating status propagation.
- Parent main: `5cbf3a52a89a98646f195be34d9fb162c72b2bb3`. Exact AI handoff docs #43774559261 and Process lifecycle #43774559266, infosvr #43774559287, AliDDNS #43774559327, AliyunDrive #43774559248, AdGuardHome #43774559217 and NVRAM #43774559233 PASS. Exact-head tests for these three updated contracts are pending.
- Corrected XY-C1 push-only run #43773995690 applies to SHA 0e27 and was still in progress at last inspection; no current-main L3 claim. SkipDBv2 #43773526771 remains an independent unresolved inline source-contract failure.

---

## 2026-11-16 — Fail closed on unknown Transmission and Aria startup process state

- Replaced both launchers' stdout-only `pidof` checks with explicit exit-status/output classification. Only status 1 plus empty output is ABSENT; status 0 requires one or more valid, positive, canonical PID tokens and returns as already running.
- Unknown command status, success-with-empty/whitespace output, output accompanying status 1, error-with-output, malformed PID tokens and PID 0/1 now fail closed before storage setup or daemon launch.
- Extended `storage_app_start_status.sh` for both services with injectable pidof status/output and cases for all above states plus existing multi-PID RUNNING. Existing workflow exercises host shell and firmware BusyBox 1.24.2 ash.
- Parent main: `2c16e60838bcc066e34d5e7e8b70fae2f02a088d`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact-head storage CI is pending; no L2, firmware build, L3 or L4 claim is made before relevant evidence.

---

## 2026-11-15 — Align SkipDBv2 preflight and keep process CI push-only

- The SkipDBv2 pre-build source map still asserted a raw `start_gpio_btn();` call after RC boot startup began passing that service result through `rc_first_service_start_status()`. Updated the contract to require the reducer-wrapped call, preserving the actual GPIO startup requirement.
- Removed the accidental `pull_request` trigger from Process lifecycle regression; main push path filters are preserved.
- Parent main: `aebfd65ccf7cef183e6a2432c45866d273c17b8b`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact aebfd65 USB storage #43775390744 PASS including host shell and firmware BusyBox 1.24.2 ash Transmission/Aria startup-state regressions. Process inventory #43775390816, Process lifecycle #43775390764, AI docs #43775390851, infosvr #43775390910, AdGuardHome #43775390873, AliDDNS #43775390953 and AliyunDrive #43775390850 also PASS. NVRAM #43775390849 and XY-C1 #43775390773 were pending at last poll.
- XY-C1 must complete Build/TRX/artifact on aebfd65 before L3 is claimed. This validation-contract update has not yet been Actions-verified; no L4 claim.

---

## 2026-11-15 — Add full user-tree lifecycle inventory to Process CI

- All eight exact-head 4997 Actions PASS, including Process #43775696137, AI handoff #43775695951, NVRAM #43775696126 and SkipDB #43775696130. SkipDB passed its reducer-wrapped GPIO source contract, configuration, all targets and regression tests.
- Rechecked aebfd65 exact Actions: USB storage #43775390744, NVRAM #43775390849 and XY-C1 #43775390773 all PASS. The XY-C1 job completed Build XY-C1, TRX verification and artifact upload on that SHA; commits through 4997 changed CI/checkpoint files only, so product-source L3 evidence remains current. No L4.
- Added the whole tracked `trunk/user/**` tree to Process workflow triggers and execute `scripts/audit_process_lifecycle.py` against the exact checkout. The log prints direct `pids()` calls, `is_dns_dhcpd_run()`/DNSMasq status API references and legacy `killall` candidates; its output will drive bounded source migration rather than changing the old Boolean API without callsite analysis.
- Extended the lexical scanner symbol inventory with the two DNSMasq status API names. The workflow also runs the scanner self-test.
- Parent main: `4997afa4c480230727aa095f55b89b04f1e4fa73`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. This inventory integration awaits exact-head Process CI; no source migration is claimed in this commit.

---

## 2026-11-16 — Always execute fail-closed Samba/FTP stop from AiDisk UI

- Whole-tree lifecycle inventory on exact source SHA `8f55030bb5b7ec29f1cd6bc199a530b9eecfab53` passed: scanner self-test and 6,255 text files across 9,501 tracked tree entries; 0 missing candidates and 0 read errors. Artifact #14186597970 contains the complete JSON/Markdown report.
- The report found 27 actual `pids()` callsites; `is_dns_dhcpd_run()` has only its declaration and definition, with no caller in the tracked user tree. The highest-risk state-changing consumers were the Samba and FTP disable branches in `trunk/user/httpd/src/aidisk.c`.
- Fixed the disable path: removed legacy `pids()` prechecks that interpreted a NULL process list (including scan failure) as ABSENT and skipped shutdown; now both actions always call `/sbin/stop_samba` / `/sbin/stop_ftp`. Those RC handlers already own tri-state inventory, verify absence and return failures for UNKNOWN/failed stops, which the UI surfaces.
- Added a source integration regression requiring both stop handlers and rejecting legacy `pids()` checks in the AiDisk toggle callback. Added the AiDisk source path to XY-C1 firmware validation triggers so the changed HTTPD object is covered by a current MIPS build.
- Parent main: `8f55030bb5b7ec29f1cd6bc199a530b9eecfab53`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact parent Process inventory #43776730111 and AI docs #43776730107 PASS; inventory artifact #14186597970 uploaded.
- THIS COMMIT: exact-head Process lifecycle regression, XY-C1 Build/TRX/artifact and AI handoff documentation checks are pending; no L3/L4 result is claimed until the XY-C1 job passes on this SHA.

---

## 2026-11-15 — Route manual DDNS refresh through identity-bound signaling

- HTTPD `ddnsclient` used `pids("inadyn")` as a lossy presence gate followed by `killall -SIGUSR1 inadyn`. Procfs inventory errors were silently treated as absent, and name-wide signaling was not bound to identities captured by the scanner.
- Routed manual refresh through a dedicated RC notification. The helper preserves ABSENT as a no-op, aborts on UNKNOWN, validates the captured Inadyn snapshot and revalidates before sending SIGUSR1 only to captured PIDs. Existing SIGHUP configuration-reload behavior remains unchanged.
- Added GNU89/ASan/UBSan host coverage for SIGUSR1 success and UNKNOWN rejection, source contracts for HTTPD→notify_rc→RC→signal, dedicated Inadyn workflow path filters and XY-C1 product-source triggers.
- Parent main: `eee736ad62722846db11e035613a26a8433ffc12`; frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact eee Process #43778610041 and AI docs #43778609888 PASS. Parent XY-C1 runs #43777935841 / #43778456640 remained in Build with TRX/artifact pending at last poll.
- THIS COMMIT exact-head Inadyn lifecycle, Process, AI docs and XY-C1 Build/TRX/artifact validations pending. No L3/L4 claim.

---

## 2026-11-15 — Remove orphaned AiDisk CGI success label

- The AiDisk Samba/FTP disable fix removed both `goto SET_AIDISK_STATUS_SUCCESS` guards, but retained the now-unreferenced C label. A label with no goto can trigger `-Wunused-label` under `-Wall` in the target firmware build.
- Removed the unused label and changed the Process integration checker to delimit `ej_set_AiDisk_status()` at the following `ej_get_all_accounts()` definition; the source contract also asserts the orphan label is absent while retaining the no-`pids()` and stop-handler assertions.
- Parent main: `f71ee12cbca49972cc927457338d395e64818381`. Exact f71 Inadyn policy/integration #43779424646 PASS; Process #43779424653, inventory #43779424644, AI docs #43779424703 and XY-C1 #43779424777 had queued/in-progress states at last poll. Frozen backup unchanged.
- THIS COMMIT exact-head Process, inventory, AI docs and XY-C1 Build/TRX/artifact validation pending.

---

## 2026-11-15 — Align HTTPD security wiring assertion with safe DDNS signal route

- Exact 2d97 HTTPD Authentication and Security Regression #43779569517 failed at the final source-wiring assertion. The workflow still required `doSystem("killall ...", "-SIGUSR1", "inadyn")` in the `ddnsclient` handler, although the production code now queues `RCN_MANUAL_DDNS_UPDATE` and the dedicated Inadyn regression verifies tri-state scan + PID-identity-bound SIGUSR1.
- Updated the HTTPD security allowlist sink to `notify_rc(RCN_MANUAL_DDNS_UPDATE)`; no production behavior changed in this task.
- Parent main: `2d97d3b11b48464a21ace2113a04ab26769aff2a`. Exact Process #43779569397, inventory #43779569493 and AI docs #43779569387 PASS. XY-C1 #43779569458 and NVRAM #43779569419 pending at last poll.
- THIS COMMIT requires exact-head HTTPD security rerun and AI-doc validation.

---

## 2026-11-15 — Fix Shadowsocks adblock parser and atomic publication

- `start_AD()` used `check = `grep -wq ...``, which is not a shell assignment; quiet grep also emits no value for command substitution. The intended dnsmasq/address versus Adblock-list detection was therefore not reliable.
- Added `ss_publish_adblock_config()`: direct `address=` configurations are copied unchanged; only simple anchored `||hostname^` rules are converted to `address=/hostname/0.0.0.0`. Comments, allow rules, wildcard rules, unsupported forms and empty input are rejected if no supported rules remain.
- The publisher stages beside the destination, verifies non-empty output, sets mode 0644 and atomically renames it over the active config. Failed downloads/parsing leave the last-good file untouched; download input names are unique per invocation.
- Added `tests/adblock_config.sh` for dnsmasq passthrough, Adblock conversion, unsupported/empty input rejection, no temp-file leaks and last-good preservation; wired it into the Shadowsocks workflow and source checker.
- Parent main: `9cd5348bba7bee2a2f163d46cc553e32ffb44daa`; frozen backup unchanged. Exact 9cd HTTPD/Process/NVRAM/AI-doc/module checks PASS; XY-C1 #43779833547 remained at Build XY-C1 with TRX/artifact pending.
- THIS COMMIT requires exact-head Shadowsocks shell/source regressions and XY-C1 Build/TRX/artifact. No L3/L4 claim.

---

## 2026-11-15 — Correct shell mktemp template expansion and add XY-C1 source trigger

- Exact b33 Shadowsocks Action #43781109010 failed in the new adblock test: literal backslashes before `${TMPDIR:-/tmp}` and `${ss_adblock_output}` prevented shell expansion in mktemp templates. The shell syntax and existing process-lifecycle test both passed.
- Removed those escapes so POSIX shell expands both temporary directory and output-file templates. Added `trunk/user/shadowsocks/scripts/shadowsocks.sh` to the XY-C1 workflow's push paths so this packaged script is covered by firmware validation.
- Exact b33 Process #43781108999, inventory #43781109077, AI docs #43781108988, AdGuardHome #43781109013, infosvr #43781108991, AliyunDrive #43781109183 and AliDDNS #43781109014 PASS; NVRAM #43781109000 remained running at last poll.
- Exact 9cd XY-C1 #43779833547 PASS (Build, TRX and artifact upload). Exact follow-up Actions pending.

---

## 2026-11-16 08:35 Beijing — Use shell comment syntax for the Shadowsocks adblock publisher

- The explanatory preamble above `ss_publish_adblock_config()` used C-style `/* ... */` syntax in a shell script. Ash treated the opening token as a command, producing an avoidable command-not-found attempt whenever the script was parsed.
- Replaced the preamble with shell `#` comments. Strengthened `tests/adblock_config.sh` to extract from the comment line as well as the function body; the old C-style preamble therefore produces an empty extraction and fails closed before parser assertions.
- Local POSIX `sh -n` checks and the adblock publication regression PASS. A negative fixture restoring the former C-style preamble was rejected by the strengthened test.
- Parent main: `59feb2da538a282d9929d5a48bd97c4a8861c6ae`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Exact-parent Shadowsocks #43781489845 PASS; AI docs #43781489871, Process #43781489848, inventory #43781489882, NVRAM #43781489851, infosvr #43781489801, AliDDNS #43781489923, AdGuardHome #43781489834 and AliyunDrive #43781489841 PASS.
- Exact-parent XY-C1 #43781489913 remained at Build XY-C1 when inspected; TRX and artifact steps pending. This commit needs exact-head revalidation; no L3/L4 claim is made.

---

## 2026-11-15 — Reject injected dnsmasq directives in downloaded adblock lists

- Security review found the parser copied a complete remote file unchanged whenever it contained an `address=` line. A mixed file could therefore add unrelated dnsmasq directives such as `dhcp-script=` or `conf-file=`, despite the source being configurable remote input.
- Restricted the dnsmasq-format branch to blank lines, comments and `address=` rules; only address rules are emitted. All other directives are rejected. Grep errors are distinguished from normal no-match; rejection cleans temporary files and does not replace last-good output.
- Added a malicious mixed-input regression containing `dhcp-script=` and `conf-file=`; it must fail and preserve the prior output. The static source checker enforces the whitelist and both injection examples.
- Parent main: `a22914f4de9a47b7a7e6c28711b9927c93ae8120`; exact a229 Shadowsocks #43782262134, Process #43782262196, inventory #43782262138 and AI docs #43782262148 PASS. XY-C1 #43782262108, NVRAM #43782262086 and AliDDNS #43782262168 were pending at last poll.
- THIS COMMIT exact-head Shadowsocks/Process/AI-doc and XY-C1 checks pending.

---

## 2026-11-15 — RC lifecycle hardening handoff (97d4d6720b8a)

- Live source baseline at handoff: `97d4d6720b8aff12d6a10b8b816084f870d9e468`; backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- This cycle's delivered stability/security work: HTTPD watchdog-stop gate for NVRAM restore; Inadyn daemon postcondition verification; IPTV/udpxy/xupnpd startup status propagation; XUPnPD unique no-follow config staging and directory/symlink checks; 802.1X boot/restart failure propagation with rollback; CPU-watchdog module-load status propagation.
- XUPnPD directory regression now covers real directories, pre-existing symlinks, regular files, missing paths, and NULL input. The source checks `/etc/storage`, the XUPnPD root, and its config directory before privileged config mutations.
- Exact-SHA Actions for `97d4d6720b8aff12d6a10b8b816084f870d9e468` at handoff time:
  - Shadowsocks proxy process lifecycle regression — run 43793708284: PASS (completed, success).
  - AI handoff documentation contract — run 43793708307: PASS (completed, success).
- Next: continue auditing enabled RC service startup boundaries for concrete ignored launch/module failures; do not repeat already-completed audits or overfocus one module.

## 2026-11-16 — OpenSSH, Transmission and ZeroTier lifecycle fail-closed slices

- OpenSSH commit `952e97ccbf9bc8d913bfa6a084e59ae3cda0d8bc`: reload now validates PID-file shape, sshd procfs command identity and start-time identity, rereads the PID file before SIGHUP, and propagates signal errors. Added PID reuse, malformed/symlink/stale file and signal-failure regressions. Exact-SHA Process, inventory, AI-doc, NVRAM and XY-C1 Actions #43793042766, #43793042723, #43793042720, #43793042760 and #43793042756 PASS.
- Transmission commit `8e0556d2379f227da3e68ebf9e75668ddde09367`: reload now distinguishes absent from UNKNOWN pidof output, validates returned PID tokens and propagates SIGHUP failures. Added `tests/reload_status.sh`. Exact-SHA Process #43794003957, inventory #43794003936, storage hotplug #43794003942 and NVRAM #43794003930 PASS; XY-C1 #43794003979 was pending at the checkpoint. AI docs #43794003967 failed because the prior handoff missed mandatory checkpoint wording.
- ZeroTier commit `fc867b505ef94cb3579760a53c12923515888d34`: introduced RUNNING/ABSENT/UNKNOWN process inventory semantics; UNKNOWN blocks TERM/KILL escalation and stop cleanup. Regression test portion passed, but its first static-contract run #43794661493 failed because the checker still encoded the old inline-pidof implementation. Process run #43794661499 also hit a separate RC boot-source assertion, later aligned by main commit `69bf1373d07fa4c4810a2f122ed9fc9a272e3316`; that exact-SHA Process run #43794762732 PASS.
- ZeroTier checker follow-up `0731e7aa932661260456a3f95f0fbebbecc42fa2` validates tri-state semantics and UNKNOWN escalation gating. Exact-SHA ZeroTier workflow #43794876436 PASS. AI docs #43794876451 failed only because the prior handoff lacked the mandatory end-of-session checkpoint section; corrected by this checkpoint.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 validation was performed. First next source investigation: Adbyby stop postcondition / cleanup gating.

---

## 2026-11-16 — Gate Adbyby teardown on confirmed daemon absence

- Confirmed the lifecycle lock serializes Adbyby actions, but `adbyby_close()` previously removed firewall/cron/DNS state and restarted dnsmasq immediately after an unchecked `killall`. A still-running or unobservable daemon could therefore survive after its traffic/DNS integration was removed.
- Added strict pidof tri-state classification (RUNNING / confirmed ABSENT / UNKNOWN), a five-second bounded stop postcondition, and fail-closed handling for malformed output, pidof errors, sleep errors and timeout. Firewall, cron, DNS and dnsmasq restart now occur only after confirmed absence; an already-absent daemon remains an idempotent success.
- Added `trunk/user/adbyby/tests/adbyby_stop_postcondition.sh` for UNKNOWN/malformed/stubborn process blocking, confirmed-absent idempotence and successful signal-to-absence cleanup. Wired it into Process lifecycle regression.
- Parent main: `b3e550a5dadf0607f2d7aba5b93db6da8e1c9d4d`. Exact-head Actions pending; no local BusyBox/target firmware or L3/L4 validation claimed. XY-C1 `8e0556d` run #43794003979 is SUCCESS; `fc867b5` run #43794661521 was still in progress at the latest check. Frozen backup unchanged.

---

## 2026-11-16 — Gate Adbyby rule updates on verified daemon shutdown

- Parent main: `ca6f0786aa35f95331d031e38bca53dea189743a`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-SHA runs on `ca6f078`: Adbyby stability `43797496515` PASS; Process lifecycle `43797496441` PASS; inventory `43797496503` PASS; AI docs `43797496481` PASS; HTTPD security `43797496369` PASS. NVRAM boundary `43797496462` and XY-C1 `43797496451` were PENDING at the latest check.
- Found that `adbyby_uprules()` ignored `adbyby_close` failure, so a daemon with UNKNOWN/timeout stop state could be followed by new rules and a replacement process. The follow-up makes the updater abort before staging, validates that contract statically, and adds a shell regression that checks no rules, dnsmasq or daemon launch side effects occur.
- No physical-router test is claimed. This atomic source/test/checker/handoff transaction is based on `ca6f0786aa35f95331d031e38bca53dea189743a`; exact-SHA CI for the follow-up must be checked after publication.
- Next: verify the current exact-SHA Actions and proceed to the next highest-risk remaining lifecycle stop/signal caller.
---

## 2026-11-16 — Gate Shadowsocks watchcat restarts on verified shutdown

- Adbyby updater stop-gate regression correction `867e4812019a43276249b144a1d49cb1d90691c8` fixed a malformed shell assertion; exact-head inventory, process lifecycle, handoff, Adbyby config and NVRAM checks all PASS.
- Found unchecked name-wide restart signals in `trunk/user/shadowsocks/scripts/ss-monitor`: SSR server and proxy restart used `killall -9`, while ChinaDNS-ng used `kill -9 chinadns-ng` (invalid name-as-PID usage). The monitor now uses the shared tri-state bounded stop helper and refuses replacement launches on unverified stop; DNS proxy restarts are also gated.
- Source/checker/workflow commit `ecaabcad6af552b21f135b90d9109962df5d8daa`; path and stop-before-restart contract commit `2498b18c2c98fd588529fd96676dee5de03985fe`; ChinaDNS proxy failure gate and checker syntax repair `a8d50172afff0c081eb1b5c0f12d45dd07dc94f8`.
- Exact-head Actions for `a8d5017`: inventory `43798722181`, process lifecycle `43798722252`, handoff `43798722254`, Shadowsocks lifecycle `43798722278`, NVRAM `43798722201` were in progress at last check. No target firmware or hardware validation claimed.
- Next: verify exact-head Actions, then use the inventory artifact to select the next direct process signal/stop caller.

---

## 2026-11-16 — Gate Redsocks stop and restart on verified daemon absence

- Commit `39a2cb22ee211a42ab84214b4268b7d541c1babe` adds tri-state pidof inventory, bounded TERM/KILL waits, fresh pre-KILL revalidation and start/restart fail-closed gating.
- Its first exact-head Redsocks workflow failed because a sed range truncated the heredoc inside production `func_start`. Follow-up `081c53bab84b5850dec61f2fe8cc607c6e6b9d08` adds a test-only dispatcher guard and sources the complete production script instead of extracting partial functions.
- Exact-head `081c53b` inventory `43799550922`, process lifecycle `43799550880` and handoff `43799550886` PASS; NVRAM `43799550890` was IN PROGRESS; Redsocks `43799550943` failed because the source harness omitted the script's second argument. Follow-up `fbc61e15eb5365bbcc7b000e99e053ebf1adf734` supplied `check nolog` but exposed a missing third argument; `31a0841d657ef8be12457c828a04c46945d02bf5` now supplies `check nolog test`. `31a0841` then reached the sleep-failure test, but the fixture defaulted TERM to immediate stop and never exercised sleep. Follow-up `92ead5be2f584c10391c9b9b1cf2bcff48d5921f` sets `TEST_TERM_MODE=keep`; its next failure showed that sourcing the full production script overwrote the earlier `func_redsocks` stub. `63d66068464aed9a194e72323f1511e32fe462b6` reinstalls launch/save stubs after source import; the dynamic regression passed. The source checker then failed because its function-body regex stopped at a `}` inside the configuration heredoc. `831ebfbe3cf15d88d9f943c80d26545c6794e88b` checks launch ordering against the full source text. Exact Redsocks `43800078490` and handoff `43800078518` both PASS: shell syntax, dynamic regression and source checker are green. No target BusyBox/MIPS or L3/L4 claim.

---

## 2026-11-16 — Remove unsafe stale PID-file signals from legacy RC wrappers

- Commit `1e43d33ce171a0ea4fd5c9cc81ff67a21d017468` removes transient PID-file handling from scutclient and mentohust wrappers. The old code could SIGKILL a reused unrelated PID, rewrite the file with the short-lived shell PID, and remove it after the action.
- Added a regression that seeds a nonexistent high PID and verifies both wrappers leave the stale marker untouched while preserving their daemon stop/logout commands.
- Exact-head legacy-wrapper regression, process lifecycle, inventory, handoff and NVRAM checks PASS. XY-C1 run `43799216333` on this exact SHA completed PASS: MIPS TRX recognized and artifact uploaded as ID `14195992622` (SHA256 `22759358b13a0a64624d4a6b4c97e2fe5e9fcf663810dd36a972aa32f40b6fbc`). L3 is limited to `1e43d33`; no physical-router/L4 claim.

---

## 2026-11-16 — Complete DDNSTO tri-state stop and restart gating

- Source commit `8e1b0a3c10b8518726d2f6333e10887fffff6444` adds validated RUNNING/ABSENT/UNKNOWN process inventory and bounded waits; UNKNOWN after TERM and sleep failures block SIGKILL. Test helper extraction and contract follow-ups landed in `21c4d645b5b5cd0ccd1a1eb555d3181da57955fc`, `f78ac4a4542a60988fc295d21e3b5afdf9175171`, and `7968c1eaeaa324066d850cef020ccebc238c8821`.
- Initial exact-head failures were test/checker contract issues: production start legitimately reads the enable switch before the stop gate, and the checker initially required the state helper inline rather than through `kill_z`. Corrected both without weakening the production gate.
- Exact-head DDNSTO lifecycle and handoff checks on `7968c1e` PASS. No target/L4 claim.

---

## 2026-11-16 — Fail closed on unknown AliyunDrive WebDAV process state

- Commit `982ec548a132a5a06f4a81872f5ae920fb685da3` adds tri-state pidof validation, bounded waits, UNKNOWN propagation and fresh name revalidation before SIGKILL. UNKNOWN blocks duplicate start, successful stop and destructive escalation.
- Added error, malformed, empty-success, contradictory-status, post-TERM UNKNOWN and sleep-failure coverage. Exact-head inventory, process lifecycle, handoff, AliyunDrive lifecycle and NVRAM checks all PASS.
- No target BusyBox/MIPS/uClibc, firmware/TRX or L4 validation is claimed.

---

## 2026-11-16 — Add verified shared process shutdown for SCUTClient and MentoHUST

- Parent main: 9e9e0a42da9154b497c9dbaba7319bdbae5eaf61; frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Parent exact-head checks completed SUCCESS: Process lifecycle/inventory 43800309439; AI handoff 43800309443; AliDDNS 43800309472; AliyunDrive 43800309429; AdGuardHome 43800309435; infosvr 43800309421; NVRAM boundary 43800309440.
- Found both wrappers relied on killall exit status rather than verified process absence. SCUTClient used unconditional SIGKILL, ran logoff even if shutdown was not verified, and its stop action reported success unconditionally; its restart ignored stop failure. MentoHUST also treated successful killall invocation as proof of daemon exit.
- Added a shared POSIX process lifecycle helper installed by the always-built shared package. It classifies pidof state as RUNNING / confirmed ABSENT / UNKNOWN, validates positive decimal PIDs, waits with bounded polling, verifies TERM shutdown, revalidates before SIGKILL, and only returns success after a fresh ABSENT state.
- Updated SCUTClient and MentoHUST wrappers for duplicate-start prevention, fail-closed UNKNOWN handling, verified stop, and restart gating. Kept SCUTClient's logoff action after confirmed stop only.
- Added helper-level shell regression and real-wrapper host tests in check_optional_app_start_status.py. Expanded process-lifecycle pull_request path filters to cover helper/package/test, both wrappers, and the checker.
- This atomic source/test/checker/workflow/handoff transaction is based on parent 9e9e0a42da9154b497c9dbaba7319bdbae5eaf61. The resulting exact-SHA Actions must be checked after publication. No firmware/TRX or physical-router L4 test is claimed for this change.
- Next: validate exact-head CI, then continue with the next inventory-ranked lifecycle consumer.
---

## 2026-11-16 — Correct SCUT/MentoHUST lifecycle regression integration

- Parent main: dc740accb110bfb88d0003e9142c04113b6b4921.
- Exact-SHA run 43801979640 (Process inventory and lifecycle fail-closed regression) failed because the static checker expected the phrase "UNKNOWN after TERM", while the shell test's actual negative assertion is "post-TERM UNKNOWN accepted". Correct the contract to match the regression.
- Exact-SHA run 43801979688 (Legacy wrapper stale pidfile safety) failed because the existing legacy test invoked wrappers without the newly installed shared helper and expected direct SIGKILL commands. It now supplies PROCESS_LIFECYCLE_LIB and a pidof stub returning confirmed ABSENT; the correct behavior is no killall invocation, with SCUT's one-shot logoff action preserved.
- Job logs isolated these integration failures. This follow-up modifies regression wiring only; production helper/wrapper source remains unchanged. The follow-up SHA must run all affected Actions again before PASS is claimed.
- Next: check exact-head CI for this regression repair and continue only after the checks are reconciled.
---

## 2026-11-16 — Preserve SCUTClient stop failure status

- Parent main: `8d47577174673576fa6b820d431b8a723ac4c8d0`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head Process inventory/lifecycle run `43802207082` on parent `8d47577174673576fa6b820d431b8a723ac4c8d0` failed in `scripts/check_optional_app_start_status.py::check_scutclient`: the UNKNOWN-state stop test expected status 2 but received 0. The other seven exact-head push workflows on that SHA completed SUCCESS (AI handoff, AdGuardHome, infosvr, legacy wrapper stale-pidfile, AliyunDrive, AliDDNS and NVRAM boundary).
- Root cause was production status loss: `func_stop()` correctly returned the shared helper's UNKNOWN/failure result and withheld logoff, but the SCUTClient `stop)` dispatcher overwrote it with `action_status=0`. The dispatcher now captures `$?`, preserving the fail-closed contract. Existing dynamic regression exercises UNKNOWN for start/stop/restart and asserts no signal, duplicate launch or logoff side effects.
- Focused local POSIX smoke fixture: UNKNOWN stop returned 2, with no killall or logout invocation; `sh -n` passed for the wrapper and shared helper. Full repository checker was not run locally (no checkout); the failing Actions job did execute it.
- Exact-head Actions for THIS COMMIT are pending after publication. No current-source XY-C1 build or physical-router/L4 result is claimed.
- Next: check the Process lifecycle/inventory workflow and AI handoff workflow on this exact pushed SHA. Fix any remaining failure from its first causal log; after relevant checks pass, continue to the next inventory-ranked direct process signal/stop consumer.
---

## 2026-11-16 — Gate ttyd lifecycle on verified process state

- Parent main: 0c9fa82488838d7069ad7a043e3307a88ed19afb; frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Closed the prior SCUTClient fix: exact-head Process lifecycle 43803219812, process inventory 43803220093, AI handoff 43803219836, legacy wrapper safety 43803219806, infosvr 43803220008, AdGuardHome 43803220094, AliyunDrive 43803219984, AliDDNS 43803219861, and NVRAM boundary 43803219986 all completed SUCCESS on 0c9fa82488838d7069ad7a043e3307a88ed19afb. XY-C1 firmware run 43803219866 remained IN_PROGRESS at last observation; no L3 claim is made for that SHA.
- Process inventory artifact 14197222958 (exact source SHA 0c9fa82488838d7069ad7a043e3307a88ed19afb) covered 6,269 text files and identified ttyd as a remaining direct stop consumer. trunk/user/ttyd/ttyd.sh previously trusted bare killall -q ttyd; UNKNOWN process inventories could be treated as a successful stop, and its top-level default-port code wrote/committed NVRAM for stop actions.
- The ttyd wrapper now uses the installed shared RUNNING/ABSENT/UNKNOWN lifecycle helper, rejects unknown start/stop states, verifies bounded TERM/KILL postconditions, propagates failures, and blocks replacement start after failed stop. It preserves the existing br0/login launch arguments. Default port persistence now occurs only on a confirmed-ABSENT start and NVRAM read/set/commit errors block launch.
- Replaced the old killall-exit-code-only ttyd checker fixture with a real-helper host regression. It covers absent/unknown/malformed/inconsistent/error inventories, graceful stop, escalation, post-TERM UNKNOWN, sleep failure, stubborn daemon, duplicate start, launcher error propagation, stop-gated restart, configured/default port behavior, and NVRAM write/commit failures. The shell regression and syntax checks passed locally against a temporary POSIX helper-compatible fixture; full GitHub regression must pass on THIS COMMIT.
- Exact-head Process lifecycle, AI handoff, process inventory and XY-C1 Actions for THIS COMMIT are pending after publication. No current-source MIPS/TRX or physical-router/L4 result is claimed.
- Next: inspect the exact-SHA Process workflow (including the ttyd regression) and AI handoff check. Fix first causal failures if any; once green, continue the roadmap P1 RC startup-result propagation queue rather than broadening this ttyd slice.
---

## 2026-11-16 — Correct ttyd workflow pull-request path coverage

- Parent main: `081cc03d1a24ffbd363bef4c34dde5324274607f`; product-source commit `081cc03d1a24ffbd363bef4c34dde5324274607f` passed exact-head Process regression `43805106747`, process inventory `43805106755`, AI handoff `43805106748`, NVRAM audit `43805106742`, AliDDNS `43805106695`, AliyunDrive `43805106786`, infosvr `43805106821`, and AdGuardHome `43805106813`. XY-C1 run `43805106803` was still IN_PROGRESS at last check.
- The Process workflow's pull_request path filter did not yet include the newly added ttyd wrapper/regression, because the earlier path insertion had matched the push section; that also left a duplicate push `ttyd.sh` entry. Moved one path into the PR filter, retained the new test's push monitoring, added both ttyd paths to PR monitoring, and removed the duplicate push entry.
- The exact-head Process job passed the ttyd shell regression against the repository's actual shared `process_lifecycle.sh`. The workflow filter correction is workflow-only; no new firmware or device validation is implied.
- Next: exact-SHA check this commit's AI handoff/triggered workflow; then investigate the roadmap's P1 discarded RC startup results, beginning with `void start_adguardhome()` and `void start_ddnsto()` and tracing all callers before API changes.
---

## 2026-11-16 — Verify CPU watchdog hot-restart module state

- Parent main: `c5e4ae21a3a634cb513b48e26f15deb2fbb7b050`. The preceding workflow-filter checkpoint `c5e4ae21a3a634cb513b48e26f15deb2fbb7b050` exact-head Actions all completed SUCCESS: Process lifecycle `43805487835`, AI handoff `43805487861`, infosvr `43805487901`, AdGuardHome `43805487863`, AliDDNS `43805487891`, AliyunDrive `43805487934`, and NVRAM boundary `43805487932`.
- Roadmap P1 trace found a live discarded result: `rc.c` handles `RCN_RESTART_WDG` by calling `restart_watchdog_cpu()` and ignoring its result; `rc.h` declared it `void`. When watchdog_cpu is enabled, `module_smart_load()` returns zero both for already-loaded and failed modprobe, so the old notification handler could acknowledge a failed module load. When disabled, `module_smart_unload()` may leave a referenced module loaded, yet the old void handler reported nothing.
- Added a small injected-operations policy that verifies post-operation /sys module state: requested load that remains absent returns `EIO`; requested unload that remains present returns `EBUSY`; the actual postcondition wins over an ambiguous helper status. `restart_watchdog_cpu()` now returns that status, logs the failed desired state, and the RCN_RESTART_WDG caller passes it through `rc_notify_merge_action_result()`, preserving the incomplete notification marker.
- Added GNU89/Werror/ASan/UBSan regression cases for already-loaded/newly-loaded/failed load, postcondition precedence, successful/absent/busy unload, and invalid ops. Local compile/run of the policy test passed. Exact-head Process/XY-C1 Actions for THIS COMMIT are pending after publication; no firmware/TRX or physical-device result is claimed.
- The previously inspected `start_adguardhome()` and `start_ddnsto()` void wrappers have no call sites in any `trunk/user/rc/src/*.c` file; they were not changed absent an actual caller path. Continue P1 by tracing other live service notification/module-load paths whose status is discarded, including caller aggregation, not by changing unused signatures in isolation.
---

## 2026-11-16 13:10 Beijing — Align VPN restart checker and execute status regression

- Parent main: `f9e24531c59e681cf866acc00e2534911f7219b9`. The product-source commit `f9e24531c59e681cf866acc00e2534911f7219b9` changes `restart_vpn_server()` from a void best-effort operation to an int result, retains the first stop/start/restore/unlock failure, and merges the notification result into PID 1's `RCN_RESTART_VPNSVR` status.
- Exact-head VPN server lifecycle run `43809878340` failed in the Python source contract after all three host policy executables passed. The stale ordering assertion still required two bare `return;` statements from the former void implementation; production now releases the lifecycle lock and returns `vpn_server_restart_first_error(stop_status, ...)`. This is a checker/API expectation mismatch, not a failure in the compiled lock, xl2tpd or lease-lock regressions.
- Updated the semantic checker for the int return path, explicit notification error aggregation and first-error policy; wired `vpn_server_restart_policy.h` and its test into both push/pull_request path filters and added a GNU89/Werror compile/run step. This does not change production behavior.
- Exact `f9e2453` evidence: Process inventory `43809878266`, AI docs `43809878572`, HTTPD lifecycle `43809878807`, Inadyn `43809878543`, AliDDNS `43809878657`, FRP `43809878472`, miniupnpd `43809878648`, and multiple other returned workflows PASS; VPN server `43809878340` FAILS at the stale checker; XY-C1 `43809878569`, Process `43809878580`, DNSMasq `43809878632`, USB storage `43809878424`, and several other exact-head workflows were PENDING at last inspection. No L3/L4 completion is claimed for `f9e2453`.
- THIS COMMIT exact-head VPN server and AI handoff checks are pending. Next independent source slice: propagate UPnP stop failures from `restart_upnp()` into PID 1 notification aggregation, with a callback regression proving that a failed stop suppresses firewall restart.
---

## 2026-11-16 13:12 Beijing — Use direct notification-body assertion in VPN checker

- Parent main: `f59d0a228a1c1c7bf3196c886d4a68baeccb35e6`. The preceding checker repair commit `f59d0a228a1c1c7bf3196c886d4a68baeccb35e6` corrected the VPN restart int-return ordering contract and added compilation for the new first-error regression.
- Before the workflow completed, source review caught over-escaped backslashes in its newly added raw Python regular expression. Replace the fragile regex with a direct assertion against `handle_notifications()`: the `RCN_RESTART_VPNSVR` branch must call `rc_notify_merge_action_result(action_result, restart_vpn_server())`. This avoids making the source checker depend on regex metacharacter escaping.
- The preceding exact-head VPN workflow `43810407149` and AI handoff `43810407150` were IN_PROGRESS at last observation; no PASS/FAIL is claimed for that SHA. THIS COMMIT's exact-head VPN/AI checks are pending after publication.
- Next: check the exact-head VPN lifecycle job and confirm the new `vpn_server_restart_policy.c` compile/run plus checker pass. Then continue the independent UPnP notification-result fix; do not wait on other unrelated pending Actions.
---

## 2026-11-16 14:05 Beijing — Preserve full LAN restart service results

- Parent main: `2c75de8133751e8063ded244faa225011f2891b6`. `full_restart_lan()` returned only DNSMasq status even though PID 1 merges its int return into the LAN notification. It discarded status-bearing logger/LLTD/infosvr/networkmap/miniupnpd stops; VPN server start/restart and IPTV restart; NFS/NetBIOS reloads; infosvr/LLTD/networkmap starts; IAPP restart; and HTTPD identity-notification results. That could acknowledge the notification while an operation failed.
- Added a first-error recorder using the existing `rc_first_service_start_status()` policy. It captures these operations plus recovery and DNSMasq-lock-release failures, then returns the first nonzero status. Existing VPN-before-DNSMasq lock order, DNSMasq start gate, early VPN/LAN aborts and restoration paths are preserved.
- Extended the GNU89/Werror/ASan/UBSan service-status regression with recovery and signed/positive status preservation cases, and extended the Process source-integration checker. Added LAN trigger paths to Process pull_request and XY-C1 workflows; XY-C1 now runs the expanded service status regression before firmware build.
- On exact `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`, Process `43811174980`, miniupnpd `43811175012`, AI docs `43811174933`, and XY-C1 `43811175036` (Build, TRX verification, artifact upload) PASS. This remains the latest L3 baseline before the newest parent changes.
- Exact parent `01b7fc9163a8445ada6b96a7bfdee6305a2c95fa`: Process lifecycle `43813748377`, process inventory `43813748353`, VPN lifecycle `43813748332`, AI docs `43813748506`, WAN DHCP callbacks `43813748381`, and other listed workflows SUCCESS. XY-C1 `43813748358` and NVRAM `43813748378` were IN_PROGRESS at last observation. THIS COMMIT's exact-head Process/XY/AI runs pending after publish.
- No current-head firmware/TRX/artifact or L4 physical-router result is claimed.
---

## 2026-11-16 14:18 Beijing — Align LAN restart checker and handoff checkpoint

- Parent main: `f66dc26260ba426245ad403b6703b5ba6d52336a`. Commit `b2d3288b6282c16cf23859eb9e7f8fdce083cd62` implemented first-error aggregation for the full LAN restart. Its extended service-status regression passed in both Process and XY-C1 host-test steps.
- Exact b2 Process workflow `43814394160` failed before jobs due a malformed PR path list; commit `f66dc26260ba426245ad403b6703b5ba6d52336a` repaired the YAML.
- Exact f66 Process workflow `43814587870` ran the host regressions successfully but failed in the old source checker at literal `stop_lltd();` ordering (the source now wraps that call in the status recorder). This commit adjusts the ordering contract to assert the recorder call; it preserves the VPN stop/abort and DNS-lock-before-VPN-restore assertions.
- Exact b2 AI docs `43814395799` and f66 AI docs `43814587897` both failed because the handoff did not contain the mandatory end-of-session checkpoint wording. This commit restores the validator phrase and required headings. Exact b2 XY-C1 `43814395678` was still IN_PROGRESS at last check; its LAN service status regression had passed but Build/TRX/artifact steps were pending.
- Exact f66 Adbyby `43814588010`, AliDDNS `43814587964`, and AliyunDrive `43814587985` PASS. No L3 evidence is established for b2/f66. Latest L3 remains c6 `43811175036` only (Build/TRX/artifact all PASS).
- THIS COMMIT exact-head Process and AI documentation Actions are pending after publication. No L4 physical-router test is claimed.
---

## 2026-11-16 14:22 Beijing — Complete canonical handoff checkpoint marker

- Parent main: `7ac958281702b5d3b39284a3666a90b17e01f75f`. Exact `7ac958281702b5d3b39284a3666a90b17e01f75f` Process lifecycle `43815025586` now PASS after correcting the LAN source-order checker. The full set of compile/regression steps and final source-integration checker passed.
- AI handoff `43815025491` on exact 7ac failed because the validator requires the literal heading marker `Mandatory handoff checkpoint` as well as the sentence `The checkpoint is mandatory; a document edit is conditional.`; the previous repair had restored only the latter sentence. This commit adds the required marker while preserving the mandatory checkpoint and conditional-edit semantics.
- Exact b2 XY-C1 run `43814395678` remained IN_PROGRESS at last check after all staged policy regressions including the new LAN status test passed; Build, TRX and artifact upload had not yet been confirmed.
- THIS COMMIT exact-head AI/Process checks remain to be verified. No L3 evidence is claimed for b2/7ac, and no physical-router L4 test has been performed.
---

## 2026-11-16 — Restore log retention and align LAN restart CI contracts

- Parent main: `7b82bc0826ebf93c66b6ed42811990222f2b98e5`; latest product-source baseline remains `b2d3288b6282c16cf23859eb9e7f8fdce083cd62`.
- Exact b2 Actions exposed two stale CI contracts: DNSMasq lifecycle `43814395657` and HTTPD security `43814395687` still required the old `full_restart_lan()` status returns. Their embedded checks now require `return restart_status;` and verify DNSMasq stop/restart error recording plus VPN restoration when DNSMasq lock acquisition fails.
- Exact b2 XY-C1 run `43814395678` completed Build XY-C1, TRX verification and artifact upload, including the LAN service-status regression. Exact 7ac Process run `43815025586` passed the updated source integration checker.
- Exact 7b AI handoff run `43815228477` failed because `dev_log.md` had only 3 entries. Restored the 61 complete records from parent `2c75de8133751e8063ded244faa225011f2891b6` verbatim after the three newer records; added this entry, retaining 65 total records. Archived dev-log shards and indexes are unchanged.
- Updated the init/PID1 roadmap row to distinguish exact-head L2 PASS and L3 PASS on product-source SHA b2 from still-open whole-system status propagation and L4 hardware work.
- THIS COMMIT exact-head AI handoff, DNSMasq, HTTPD and Process checks are pending after publication. No L4 physical-router validation is claimed.
---

## 2026-11-16 14:36 Beijing — Align DNSMasq lock-recovery contract with LAN status aggregation

- Parent main: `e2a43d79ca372a70af9262e1b0463945d64d644c`; product-source baseline remains `b2d3288b6282c16cf23859eb9e7f8fdce083cd62`.
- Exact `e2a43d79ca372a70af9262e1b0463945d64d644c`: AI handoff `43815932791` PASS; Process lifecycle `43815932847` PASS; HTTPD security `43815932882`, AliyunDrive `43815932706`, AliDDNS `43815932894`, AdGuardHome `43815932884`, and infosvr `43815932852` PASS. XY-C1 `43815932794` was IN_PROGRESS. DNSMasq `43815932909` failed in its final embedded source contract because it still searched for the former `return dnsmasq_lock_errno;` after the LAN transaction correctly began returning the aggregate `restart_status` and recording VPN recovery status. The host policy regressions preceding that source checker passed.
- Updated the DNSMasq checker to assert the actual recorder call, its nested `restart_vpn_server()`, preserved `errno` and `return restart_status;`. No production source behavior changes.
- Exact b2 XY-C1 `43814395678` completed SUCCESS including Build XY-C1, TRX verification and artifact upload. This establishes L3 for b2 source only; no L4 hardware validation exists.
- Next source task: propagate discarded IGMP-proxy start/stop statuses through the int-return MAN/WAN/ZeroConf IPTV paths in `net_wan.c`, extending the existing route-status policy regression and callback policy where applicable.
---

## 2026-11-16 — Restore YAML block indentation in DNSMasq source contract

- Parent main: `78af5220da96994440003d1e794ea948457dbd5f`; product-source baseline remains `b2d3288b6282c16cf23859eb9e7f8fdce083cd62`.
- Exact e2 DNSMasq workflow `43815932909` got through host regressions but its source integration check still expected the old LAN lock-acquisition return. Commit `78af5220da96994440003d1e794ea948457dbd5f` changed the assertion to the aggregate return and added rollback checks, but the three new assertions lost the surrounding YAML block's indentation. Exact 78 workflow `43816311414` consequently failed before creating a job.
- THIS COMMIT restores the heredoc Python lines to the block indentation. No source behavior changed.
- Exact e2 AI docs `43815932791`, Process `43815932847`, HTTPD `43815932882`, AliyunDrive `43815932706`, AliDDNS `43815932894`, AdGuardHome `43815932884`, infosvr `43815932852`, and NVRAM `43815932797` PASS. The e2 XY-C1 run `43815932794` was still IN_PROGRESS at the last check; current product-source L3 remains b2 run `43814395678` (Build/TRX/artifact PASS).
- THIS COMMIT exact-head DNSMasq and AI/XY checks remain to be verified. No L4 physical-router test is claimed.
---

## 2026-11-16 14:39 Beijing — Propagate IGMP proxy lifecycle results through WAN/IPTV callbacks

- Parent main: d552d85d79196f95d88d22ea2b29f45061cda4f2; frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Confirmed start_igmpproxy() / stop_igmpproxy() already return meaningful status, but MAN/WAN up/down and static, DHCP, and ZeroConf IPTV paths discarded it. Static IPTV's policy callback was void and its policy returned only route-add status.
- THIS COMMIT propagates first-error status through all identified callers, preserves independent DHCP route retry state, and still attempts multicast after an earlier route failure. ZeroConf teardown reports proxy-stop failure without changing the existing address/route identity cleanup condition.
- Added a GNU89/Werror/ASan/UBSan static IPTV callback regression for multicast failure and first-error preservation; extended WAN source checker with semantic call-site contracts and the workflow to compile/run this test.
- L1 review: PASS. Local host execution: NOT RUN (no local repository checkout). Exact-head WAN-route and XY-C1 Actions: PENDING until publication. No L4 hardware validation is claimed.
- Next: inspect exact-head WAN-route and XY-C1 push runs; then continue P1 RC startup-result audit in services_ex.c with full caller tracing.
---

## 2026-11-16 — Fix exact-head WAN proxy source-contract false negative

- Parent: 3e7acc5caeb8b8a9d0588edbcc8ab9292b750479. The exact-SHA WAN route run 43820348062 reached all host tests; the static IPTV regression printed PASS under GNU89/Werror/ASan/UBSan. The source-contract script alone failed because it searched for a whitespace-sensitive one-line rendering of a correctly multi-line nested route_status_accumulate() return.
- Updated the checker to normalize the function body before matching the return expression. No product source or runtime behavior changed.
- XY-C1 run 43820348179 on parent 3e7acc5... was still compiling when last inspected; all pre-build policy regressions completed, including static IPTV, wireless module, watchdog, VPN, UPnP, and LAN restart tests. TRX/artifact steps pending at that observation.
- Next: check exact-head WAN route and XY-C1 runs for this checker-only commit; then continue the boot hardware module-load status audit after verifying built-in module detection semantics.
---

## 2026-11-16 — Update MAN teardown regression count for captured IGMP status

- Parent: 73be5eb266a47b7b610a07880ee791d98e1a1279. Exact-head WAN workflow `43821468810` passed its first source checker after whitespace normalization, including static IPTV host regression, then failed the second route/cleanup checker because MAN teardown's expected aggregation count was stale (5; now 6 after adding the IGMP proxy stop result).
- The workflow contract now expects the sixth legitimate first-error accumulation. Production source and all C host tests are unchanged.
- XY-C1 firmware run `43820348179` for source parent `3e7acc5...` completed SUCCESS: build, TRX verification and artifact upload all passed. The current commit is checker-only.
- Next: check exact-head WAN-route and XY-C1 runs; continue independent boot module result propagation after checker is green.
---

## 2026-11-16 — Propagate boot hardware module load failures into init_router()

- Baseline main: 5095abd71bf199547e2ba846d3e3d94343d03ebc; immutable backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Confirmed dropped status: init_router() aggregated wireless module failures but ignored module_smart_load() outcomes for USB/XHCI/EHCI/OHCI, AHCI, MMC/SDHCI, MTK AES and IPSet. common_ex.c confirms is_module_loaded() verifies the live /sys/module/<name> directory after load.
- Generalized verified post-load policy while keeping the legacy wireless wrapper; forwarded XHCI usb3_disable unchanged; made module groups return first failure while still attempting remaining modules; aggregated enabled group results into init_router(). reload_modem_modules() remains unchanged pending its own internal/caller review.
- Extended GNU89/Werror/ASan/UBSan policy regression for non-NULL module parameter forwarding. Added scripts/check_rc_boot_module_loads.py and wired semantic checking into XY-C1 validation.
- L1 source review PASS. Local full MIPS build NOT RUN. Exact-head WAN-route and XY-C1 Actions PENDING after publication. No L4 hardware claim.
- Next: inspect exact-head Actions and XY-C1 build/TRX/artifact; then trace reload_modem_modules() before any API change.
---

## 2026-11-16 — Make MAN teardown source-check count resilient

- Parent: 8a8dfed5441936dc9fcf8e9414f45a21684aff33. WAN run 43821921846 passed sanitizer host tests and the first source checker, then failed at the second source-contract assertion for the total MAN teardown status-aggregation count. The targeted source is independently checked for each route delete and the new stop_igmpproxy() result, so a hard-coded exact count is unnecessary; the regression now requires the original minimum count (>=5) and the specific new proxy assertion remains separately required.
- This is checker-only; no C source, test binary or behavior changed.
- Boot-module source commit 8a8dfed... exact-head XY-C1 / WAN actions are running or queued at the time of this handoff.
- Next: verify exact-head WAN-route and XY-C1; continue modem reload caller audit.
---

## 2026-11-16 — Relax MAN/WAN up operation counts in WAN source contract

- Parent: ea22f196ad50bfc07aaa64547e02001f5946ae99. Exact-head WAN run 43823569962 passed C host tests and earlier cleanup checks but failed at a stale MAN-up aggregation-count assertion. Inspection of source at that exact SHA confirmed MAN-up has four accumulations (the added IGMP start is the fourth) and WAN-up has seven (the added IGMP start is the seventh). Previous exact equality expectations were 3 and 6.
- Replaced only those exact equality checks with lower-bound checks; semantic caller assertions separately require the IGMP proxy status to be stored and accumulated.
- XY-C1 run 43823036488 for boot module source 8a8dfed... was still building XY-C1 at last inspection.
- Next: check exact-head WAN and target build, then audit reload_modem_modules() before any API change.
---

## 2026-11-16 15:34 Beijing — Align RC process integration checker with generalized module loader

- Parent main: `f9de514198a1b6cb90b1e12528b87e7d16f955c2`. Exact-head Process run `43823916381`, job `133153102957`, passed all compile/runtime policy tests, then failed only in the final RC source-integration checker because it still required `ops->load(module_name, NULL)`, the wireless-only helper call and `wireless_modules_status`.
- Source commit `8a8dfed...` intentionally generalized post-load verification to forward module parameters and aggregate wireless plus USB/ATA/MMC/AES/IPSet outcomes as `boot_modules_status`. Updated the checker for the generic helper, retained wireless compatibility wrapper, boot aggregate, and XHCI parameter-forwarding regression.
- The Process workflow already includes `scripts/check_process_lifecycle_integration.py` in both push and pull_request filters; this commit's checker change triggers the exact-head rerun without a workflow-path adjustment.
- WAN-route exact-head run `43823916390` on parent `f9de514...` is SUCCESS. XY-C1 run `43823036488` for source SHA `8a8dfed...` completed SUCCESS: boot-module policy/source checks, Build XY-C1, TRX verification and artifact upload.
- THIS COMMIT is test/docs-only; exact-head Process rerun pending. Next source task after a green run: trace `reload_modem_modules()` implementation/callers and failure semantics before changing its legacy void API.
---

## 2026-11-16 15:38 Beijing — Fix boot module aggregate call-order contract

- Parent main: `4bfc47d4aec1548c600039c89a98dc7464e022dc`. Process exact-head run `43825080891`, job `133156882058`, completed all compiled/runtime tests successfully, then failed its final source integration checker at line 1298 because the checker searched for a bare `load_mmc_modules();` call.
- The production code correctly calls that function as an argument to `rc_first_service_start_status()`. Replaced the obsolete bare-call comparison with ordered checks for the actual aggregate expressions for MMC, USB, ATA, crypto and IPSet after the initial wireless load.
- This is checker/docs-only; no firmware source or policy test changed. Previous source SHA `8a8dfed...` has exact XY-C1 Build/TRX/artifact SUCCESS on run `43823036488`; prior WAN-route exact main `f9de514...` passed run `43823916390`.
- Exact-head Process rerun required. Next production investigation remains `reload_modem_modules()`, with special care because driver lists contain alternative modules and unload return values are overloaded.
---

## 2026-11-16 15:52 Beijing — Enable XY-C1 USB serial option modem driver

- Parent main: `3905bc882d54ee5a37f5dd0493742d9a752565ff`. Process lifecycle exact-main run `43826215394`, job `133160569142`, and the exact-head AI handoff, AdGuardHome, AliDDNS, infosvr, AliyunDrive and NVRAM checks all completed SUCCESS on this parent SHA.
- Completed the deferred `reload_modem_modules()` trace across `usb_modem.c`, its boot-time and `RCN_RESTART_MODEM` callers in `rc.c`, helper implementations in `common_ex.c`, and target Linux Kconfig/Makefile.
- Confirmed a concrete target-profile mismatch: RC always requests `option` for RAS modem mode, but XY-C1 explicitly disabled `CONFIG_USB_SERIAL_OPTION`. The in-tree Linux 4.4 Kconfig identifies this as the GSM/CDMA serial driver for Option, Huawei, Audiovox, Novatel Wireless and Anydata devices, and its Makefile builds `option.o`. The module therefore could not be loaded in the XY-C1 firmware despite being requested by RC.
- Enabled `CONFIG_USB_SERIAL_OPTION=m`. Added `scripts/check_usb_modem_driver_config.py` to verify that all 11 modules requested by `reload_modem_modules()` are configured as loadable modules and that the option Kconfig/Makefile wiring is present; wired the check and all relevant paths into `.github/workflows/xy-c1-firmware-validation.yml`.
- `module_smart_load()` has ambiguous zero returns (already loaded or failure). `module_smart_unload()` returns one both for busy/refcount-positive and successful removal, and zero for absent/refcount-read failure/removal failure. Driver lists include alternatives, so no unsafe blanket error propagation or void-API change was made.
- THIS COMMIT has no local checkout execution. Exact-head XY-C1 build/TRX/artifact Actions are pending. Prior source `8a8dfed...` had XY-C1 L3 SUCCESS on run `43823036488`, but that image predates this config change. No L4 hardware validation.
- Next: inspect the exact-head XY-C1 workflow and repair any config/kernel build failure; after a green target build, continue device-aware reload status and regression design.

---

## 2026-11-16 13:16 Beijing — Aggregate all return-bearing init_router service starts

- Parent main: `7630958f12938ae6014ee859d9eccfd21cdf7503`. All ten exact-head push Actions on that parent completed SUCCESS, including Process inventory `43827946437`, AI handoff `43827946538`, NVRAM `43827946578`, and XY-C1 Firmware Validation `43827946528`.
- Exact 763 XY-C1 run `43827946528` passed the modem module configuration checker, all staged host regressions, Build XY-C1, TRX verification and artifact upload. TRX size: 12,680,451 bytes; artifact `14207562837`, 12,633,711 bytes, SHA-256 `bf5a475c7d2b599cc07a6ce5c3b6a0ed89410c1857f45fb4bc8d462ded0ac9c8`. This is L3 evidence for 763 only.
- Source review found `init_router()` discarded both `start_logger()` outcomes, `start_detect_link()`, `start_detect_internet()` and `start_wins()`; it logged `start_dns_dhcpd()` failure but did not include it in the consolidated boot-service result. Thus the final first-error health summary could report success despite one of these startup failures.
- Captured and merged those return values into the existing first-nonzero `boot_services_status`; added explicit detector and WINS failure logs. The existing `service_start_status.c` GNU89/Werror/ASan/UBSan regression already verifies first-error preservation and signed/positive statuses. Extended `scripts/check_rc_boot_module_loads.py` to enforce all init-router capture/aggregate call sites.
- No local checkout was available for direct execution. THIS COMMIT's exact-head XY-C1/Process checks are pending after publication; 763's L3 result must not be generalized to the new `rc.c` changes. No L4 router validation is claimed.
- Next: inspect THIS COMMIT's exact-head source checker, first-error regression and XY-C1 Build/TRX/artifact results. Then continue the device-aware USB modem loader status contract without treating every alternative driver as mandatory.

---

## 2026-11-16 16:35 Beijing — Align boot-service source contract with separated status declaration

- Parent main: `760ecfa9d1ff115e62b2df45afb9fda398229ba6`. Exact-head Process run `43831778050`, job `133178573988`, passed all earlier GNU89/ASan/UBSan host regressions and subsystem/source checks, then failed at line 1307 because the checker expected `int boot_services_status = boot_modules_status;`.
- Production `init_router()` intentionally declares `int boot_modules_status, boot_services_status;` together and separately assigns `boot_services_status = boot_modules_status;`. Updated both stale checker assertions to verify this real declaration and assignment; no C source behavior changed.
- Exact-head XY-C1 modem-option run `43827946528`, job `133166106689`, completed Build XY-C1, TRX verification and artifact upload SUCCESS on the previous source/config SHA `7630958...`. The new boot-services source SHA `760ecfa...` run `43831778029` was IN_PROGRESS when last checked.
- THIS COMMIT requires exact-head Process rerun. Next source slice: NDIS modem launcher drops miniupnpd interface-migration and DHCP-client launch errors, and its start/restart callers currently fail to propagate modem startup status. No L4 physical-router validation claimed.
---

## 2026-11-16 16:45 Beijing — Propagate router bridge STP configuration failures

- Parent main: `d4616682786618314044bcb29ef3c103905247b1`. Early `init_router()` launch-status aggregation is present via `760ecfa`; parent Process run `43832485163`, job `133180887500`, passed after the stale declaration checker was corrected. Parent XY-C1 run `43831778029`, job `133178574224`, passed Build XY-C1, TRX verification and artifact upload.
- Audited the remaining router-mode startup path in `start_services_once()`. With LAN STP enabled, `br_set_stp(IFNAME_BR, 1)` and `br_set_fd(IFNAME_BR, 15)` returned fallible sysfs write statuses that were discarded. A lost STP-enable failure can conceal a bridge loop-prevention configuration fault.
- Captured/logged both return values and merged them into `start_status` with first-nonzero semantics. The forward-delay write is still attempted after STP fails; AP-mode and disabled-STP conditions are unchanged.
- Added GNU89 status-policy regressions for both error orderings and extended `check_process_lifecycle_integration.py` to bind the new contract to production `services.c`. The checker also retains the corrected boot-status declarations from parent `d461668`.
- THIS COMMIT's exact-head Process and XY-C1 Actions are pending. No local checkout execution or L4 physical-router validation is claimed.

---

## 2026-11-16 — Propagate NDIS modem WAN startup failures

- Parent main: `d0d75c6925c2640df348f9a16e55f612356a88ac`. Exact-head Process run `43833861533` and XY-C1 run `43833861360` both SUCCESS; XY-C1 completed build and TRX/artifact verification. These runs verify the preceding bridge STP/forward-delay status fix.
- `launch_wan_usbnet()` previously discarded miniupnpd interface-migration and DHCP-client launch results, then returned success unconditionally. It now fails closed when `check_upnp_wanif_changed()` is nonzero, preserves the original status, captures `start_udhcpc_wan()`, records WAN error state and returns the DHCP result.
- `start_wan()` aggregates NDIS/RAS modem migration and launch failures with first-error semantics while continuing unrelated best-effort setup. The result reaches full restart, reconnect/deferred/automatic/manual wrappers, `init_router()` boot status and PID1 notification action results. Stop-before-reload and WAN reset/cache order remain unchanged.
- Added GNU89/Werror/ASan/UBSan policy regression for migration gating, DHCP result propagation and first-error preservation. Added source-contract checker and wired both into Process and XY-C1 push/PR path filters and execution.
- Host policy regression PASS. Full-tree source checker and exact-head Process/XY-C1 Build/TRX/artifact for THIS COMMIT remain pending its push Actions; no new L3/L4 or physical-router result is claimed.
- Next: verify exact-head Actions, then bind NDIS control TTY selection to exact physical USB port ID + devnum while preserving same-device manual/preferred selection.

---

## 2026-11-16 — Correct WAN modem status propagation integration checks

- Parent main: `8c3772e33127c3870c4465c45a8fedc1142a06b3`, whose exact-head Process run `43838099251` and XY-C1 run `43838100294` failed at the added source checker. The host modem policy test passed; the checker failure exposed its over-escaped whitespace regex and a missing RAS migration error aggregation.
- Also corrected a misplaced RAS status merge in the unrelated IPoE EAPOL failure branch. The full WAN restart implementation is restored to its original ordered `stop_wan()`, bridge teardown and LAN bridge setup gates; its new int return contract uses `-1` for abort and returns the final `start_wan()` status.
- Updated existing source-contract checks for the deliberate int-return migration, captured RAS miniupnpd status, and the IPTV callback's route+multicast accumulated result. WAN teardown, miniupnpd, DHCP callback and WAN route workflows had failed on `8c3772e`; this commit updates their contracts and restores the missing route-gate source shape.
- The previous modem policy regression compiled and passed under GNU89/Werror/ASan/UBSan. Re-run local policy test plus exact-head Process, XY-C1, WAN teardown, miniupnpd atomic publication, WAN DHCP callback and WAN route checks on this corrective SHA. None of those new exact-head results is claimed yet.
- Next: verify all corrective exact-head checks; once green, resume NDIS control-TTY physical USB identity matching and regression coverage.

---

## 2026-11-16 — Repair WAN modem and callback regression contracts

- Parent main: `58fced5de7155e7d90d5307f81a6f0a56c56ea39`. Exact-head Process `43841042079` and XY-C1 `43841041502` both ran the GNU89/Werror/ASan/UBSan policy binaries successfully, then failed in `check_modem_wan_start_status.py` because the checker counted a literal one-line C call while the valid source wraps its arguments across lines. Replace this with a whitespace-tolerant regex and retain the required three call sites.
- WAN DHCP callback `43841041827` failed a stale ZeroConf IPTV deconfig assertion. Production correctly accumulates interface, route and multicast stop results; update the source contract to verify the nested first-error accumulation.
- miniupnpd config run `43841038765` failed without creating a job because two new assertions were outdented from the YAML heredoc. Restore the script indentation.
- WAN teardown `43841041737`, WAN route `43841041407`, USB modem identity `43841041397`, Process inventory `43841041807` and AI handoff docs `43841041398` were SUCCESS on the exact parent SHA.
- XY-C1 did not reach Build/TRX/artifact on the parent SHA, so no L3 result is claimed. HTTPD auth run `43841041572` failed an independent switch-port ioctl contract unrelated to this slice.
- THIS COMMIT contains checker/workflow corrections only; exact-head Process, XY-C1, WAN DHCP and miniupnpd Actions must verify them. Next production task remains NDIS control-TTY physical identity matching; no L4 result is claimed.

---

## 2026-11-16 — Make modem status source checker whitespace-safe

- Parent main: `3d0a5692490b6eb1bc35fae74c74764be4fa234d`. Review of the newly committed checker correction found that the Python raw regex still contained doubled backslashes, so it matched a literal backslash instead of C whitespace; the adjacent RAS-order assertion also still assumed a single-line call.
- Replaced both brittle checks with one whitespace-tolerant regex, counted exactly three modem first-error aggregations and required one such aggregation after the RAS migration call. This keeps the production contract strict while accepting its multiline formatting.
- Parent `3d0a569` triggered Process run `43842343441` (queued at last inspection); no exact-head result is claimed. XY-C1, miniupnpd and WAN DHCP trigger coverage must also be confirmed on this correction. The preceding parent `58fced5` host policy binaries passed, but Process/XY source checks still failed before firmware build.
- Next: inspect this correction's exact-head Process and XY-C1 results and verify miniupnpd/WAN DHCP workflows; then proceed with NDIS control-TTY physical identity matching. No L3/L4 result is claimed.

---

## 2026-11-16 — Accept multiline boot-status aggregation in modem checker

- Parent main: `1e9f5f17d51ac43e9e6bd8911ba1e140f45634ab`. Exact-head Process run `43842801289` and XY-C1 run `43842801305` both passed the RC boot service and modem policy host regressions, then stopped at the modem source checker because `init_router()` wraps its `rc_first_service_start_status()` arguments across lines.
- The previous checker correction made the three `start_wan()` modem aggregation checks whitespace-tolerant; this follow-up applies the same semantic regex rule to the boot-service WAN aggregation assertion. No C behavior changed.
- XY-C1 did not reach its cross-build, TRX or artifact steps on this parent. No L3/L4 result is claimed. New exact-head Process/XY Actions must verify the checker before the TTY identity slice is considered covered.
- Next: bind NDIS TTY fallback selection to the exact physical USB port ID + devnum, preserving manual-index and preferred-node behavior only within that device.

---

## 2026-11-16 — Retain RAS physical WAN setup failures in modem startup status

- Parent main: `d4952540d5742868cf5cbe8d84bc4bc53e9450ad`. Process run `43843270795` failed at the source checker’s exact RC notification-call formatting assertion after the boot-status and modem policy binaries passed. XY-C1 run `43843270817` was still in progress at last inspection but had hit the same checker failure after both host policy binaries passed; it had not reached Build/TRX/artifact.
- The RAS modem branch retained miniupnpd migration and RAS modem-launch failures, but a nonzero `launch_wanx()` physical-WAN result was only logged. Thus `start_wan()` could report success while its required physical WAN setup failed.
- Added first-error aggregation of RAS `wanx_status` while preserving the best-effort sequence and still attempting `launch_wan_modem_ras()`. Extended the GNU89 policy regression and source checker to bind this behavior.
- Replaced line-wrap-sensitive RC notification matching with a whitespace-tolerant source contract, fixed the WAN DHCP workflow's stale void-return `start_wan()` signature, and restored the required `SESSION_HANDOFF.md` checkpoint wording and `## Verification boundary` section.
- AI handoff run `43843270727` failed the missing checkpoint wording; NVRAM run `43843270745` was IN_PROGRESS at last inspection. AdGuardHome `43843270763`, AliDDNS `43843270787` and AliyunDrive `43843270760` were SUCCESS on the exact parent. THIS commit's exact-head Actions remain pending. No current-SHA L3 or L4 result is claimed.
- Temporary GNU89/Werror/ASan/UBSan policy harness for the RAS first-error/best-effort case: PASS. Full-tree compilation and source integration checks remain pending exact-head Actions.
- Next: inspect this commit's exact-head Process, XY-C1, WAN DHCP callback, infosvr and AI handoff Actions and read any failing logs. Then continue the P1 NDIS TTY physical-identity selection fix, followed by device-aware modem module readiness.

---

## 2026-11-16 — Match RAS WANX status aggregation across C formatting

- Parent main: `f0902af4861fdbf496958b22871ae43e51bdcfd5`. Process run `43844256457`, job `133220324178`, passed both the RC boot service and modem WAN host policy binaries, then failed the new RAS physical WAN status source assertion because `rc_first_service_start_status()` is wrapped over multiple C lines.
- Production now aggregates the nonzero RAS `launch_wanx()` physical WAN preparation result before continuing to the RAS modem launcher; the added source contract should match that exact call with a whitespace-tolerant regex. No behavior change is needed for this checker failure.
- USB modem identity `43844254366`, WAN DHCP callback `43844254326`, miniupnpd `43844254468`, WAN teardown `43844254482`, WAN route `43844254309`, AI handoff docs `43844254359` and Process inventory `43844254197` passed on this exact SHA. XY-C1 `43844254098` was still running; no L3/L4 result is claimed yet.
- HTTPD Authentication `43844254517` failed an unrelated switch-port ioctl contract. Other selected Actions were still in progress at last inspection.
- Next source slice: make NDIS TTY fallback selection honor the selected interface's exact USB port ID + devnum, including constrained manual/preferred selection.

---

## 2026-11-16 — Bind NDIS control TTY selection to physical USB identity

- Parent main: `37f00465dec9af78f3e366a16e903ddc600603c2`. NDIS control path previously selected `ttyUSB` through the generic `get_modem_node()` selector, which compares the shared USB devnum but does not bind the candidate TTY to the selected NDIS interface's physical USB port ID. On multi-modem hubs this can select another device's AT/control port.
- Updated the metadata reader to validate `pref=0|1`, `devnum` and `portid` together, rejecting missing, duplicate or malformed priority fields. Added a pure identity eligibility policy.
- NDIS TTY fallback now executes only after resolving the NDIS interface's USB parent and matches the exact port ID + devnum. A manual `modem_node` index is a hard pin only when its node belongs to that same device; automatic selection prefers a same-device preferred TTY and then allows a same-device fallback. WDM identity matching remains same-device and does not use manual TTY preference.
- Added GNU89 host assertions for preferred-first, fallback, wrong port, wrong devnum, manual-index exact match and manual cross-device rejection. Extended the exact production source contract.
- Parent `37f00465dec9af78f3e366a16e903ddc600603c2` Process run `43844536051` was still running with the prior RAS WANX checker failure not yet updated; XY-C1 `43844536067` and NVRAM `43844536110` were in progress; AI handoff `43844536085` was SUCCESS. The earlier parent `f0902af` passed identity, WAN DHCP, miniupnpd, WAN teardown, WAN route, AI docs and Process inventory Actions; its Process WAN-start checker failed on multiline WANX formatting. No current-slice L3/L4 result is claimed.
- THIS source/test commit's identity, Process, XY-C1 and handoff Actions must be checked exactly. XY-C1 must complete build, TRX verification and artifact upload to establish L3.
- Next: inspect exact-head results and then trace `reload_modem_modules()` readiness/error semantics without treating alternative drivers as mandatory.

---

## 2026-11-16 — Fix process lifecycle and HTTPD WAN return contracts

- Parent main `9e284e758526ac281228e86b81dc349adbeff783` introduced NDIS control-TTY same-device matching by physical USB port ID + devnum; USB modem WDM/NDIS identity run `43844807532` passed. Process lifecycle run `43844807348` still contained an old `void\nstart_wan(void)` parser; updated it to the current int-return signature.
- HTTPD Authentication/Security run `43844807351` failed because the embedded source check looked for `return;` after callback and VLAN setup. Updated it to check `return errno ? errno : EIO;` and `return vlan_result;` in the expected order.
- At last inspection AI handoff `43844807461`, Infosvr `43844807516`, WAN teardown `43844807410`, AdGuardHome `43844807404`, AliyunDrive `43844807369` and AliDDNS `43844807524` passed. Process `43844807348` and XY-C1 `43844807421` were still running; no L3 result is claimed.
- No product C source change in this contract repair. Exact-head Process, XY-C1 and HTTPD reruns are required after publication.
- Next independent product investigation: trace `reload_modem_modules()` and all `module_smart_load/unload` callers to distinguish optional modem-driver absence from selected-device readiness failures.

---
## 2026-11-16 — Correct USB TTY selector contract in HTTPD workflow

- Parent main: `70e20841ef6dc6198fda5036521760f99d43a46d`. HTTPD Authentication run `43845149417`, job `133223331331`, passed the modem metadata policy binary and hotplug metadata atomic-staging source contract, then failed only because the workflow still expected the removed generic `get_modem_node("ttyUSB", devnum, NULL)` call.
- The NDIS TTY path now uses `get_modem_node_by_usb_identity("ttyUSB", ndis_port_id, devnum)`; updated the workflow contract to assert the physical-identity-bound call instead of preserving a stale selector.
- Process inventory run `43845149493` passed after the int-return `start_wan()` source contract correction. AI docs `43845149351`, infosvr `43845149279`, AdGuardHome `43845149408`, AliDDNS `43845149268`, and AliyunDrive `43845149455` passed.
- XY-C1 `43845149433` was still building at last inspection; NVRAM `43845149529` remained in progress. No current-parent L3 or L4 result is claimed.
- Next: inspect exact-head HTTPD/XY-C1/NVRAM results, then determine how to verify modem driver loading without treating optional alternative NDIS drivers as mandatory.

---

## 2026-11-16 — Align HTTPD reconnect and AI handoff contracts

- Parent main: `1b9571f272b74fcbd72085d229e17ca855344432`. HTTPD Authentication run `43845655325` failed at embedded Python line 62 because its contract searched for the obsolete direct `if (stop_wan() != 0)` in `try_wan_reconnect()`. Production uses `status = stop_wan();`, checks `if (status != 0)`, then resets WAN state. Updated the contract to verify this actual status flow and ordering.
- AI handoff docs run `43845655144` failed because the handoff omitted the required literal `Before context exhaustion`; restored the mandatory checkpoint sentence.
- Process inventory `43845655106`, AdGuardHome `43845655273`, infosvr `43845655392`, AliDDNS `43845655240` and AliyunDrive `43845655067` passed on the exact parent.
- XY-C1 `43845655207` and NVRAM `43845655244` were in progress at last inspection. No L3/L4 result is claimed.
- Next: inspect exact-head HTTPD/AI docs plus XY-C1/NVRAM, then assess `reload_modem_modules()` readiness without making alternative drivers mandatory.

---

## 2026-11-16 — Make manual RAS modem-node selection fail closed

- Parent main: `1c1fa9679e625a468b04fc12cbd2794683e810fc`. Exact-head Process, HTTPD, AI handoff, infosvr, AdGuardHome, AliDDNS, AliyunDrive and NVRAM regressions all passed. XY-C1 run `43846110844` remained in progress after its policy tests; target Build/TRX/artifact was not yet complete.
- Found a remaining legacy RAS selector defect: `find_modem_node()` returned the last existing node when the explicitly configured `modem_node` index was absent. `get_modem_node_ras()` uses this for both `ttyACM` and `ttyUSB`, so manual selection could silently substitute a different modem.
- Changed manual lookup to inspect only the configured index and return `-1` when that exact node is unavailable. Automatic preferred/any-node search remains unchanged.
- Added GNU89 policy cases for auto selection, exact manual match, mismatched candidate and missing candidate; extended the USB identity workflow's semantic production-source contract to ensure the old `node_valid_last` fallback cannot return.
- Next: inspect this commit's exact-head USB identity and XY-C1 Actions; then complete the device-aware module readiness review and resume broad RC startup-result coverage outside the modem subsystem.

---

## 2026-11-16 — Clear stale NDIS identity after publication failure

- Parent main: `d6738c390b0578ba07a7bd9f0c2c23a4a1bca764`. Exact-parent USB modem WDM/NDIS run `43846822594`, Process runs `43846822569`/`43846822993`, HTTPD `43846822596`, AI docs `43846822710`, WAN teardown `43846822592`, infosvr `43846822597`, AdGuardHome `43846822892`, AliDDNS `43846822840` and AliyunDrive `43846822598` passed. XY-C1 `43846822718` and NVRAM `43846822673` were still in progress; no L3 claim.
- Found `mdev_net_main()` retained old `wwanX/wethX` identity metadata on truncated content or failed atomic replacement. This differed from WDM/TTY handlers, which remove stale metadata on publication failure.
- Added injectable fail-closed identity publication policy: valid replacement succeeds; invalid/write failure removes the prior entry; cleanup failure returns a distinct status for explicit logging. Wired this into `mdev_net_main()`.
- Added GNU89/Werror/ASan/UBSan failure-injection regression and exact production-path source checks to the USB modem identity workflow.
- Next: inspect exact-head USB identity and XY-C1/NVRAM Actions; then continue module readiness and broad RC startup status governance.

---

## 2026-11-16 — Align HTTPD modem metadata contract with fail-closed publisher

- Parent main: `b83383f9798d5a3e43d86877f9f37c0b1594ffd0`. Source commit `b83383f9798d5a3e43d86877f9f37c0b1594ffd0` introduced a production metadata publication helper that removes stale network-node identity after invalid content or write failure.
- Exact-parent USB identity `43847475018` and AI handoff `43847474937` passed. The C regression printed PASS in HTTPD run `43847474960`, which then failed its stale source-only assumptions: it expected three direct calls to `modem_node_file_write()` and required that call directly inside `mdev_net_main()`.
- Updated the HTTPD umbrella to assert two remaining direct writers (WDM and TTY), the NDIS handler's use of `modem_node_file_publish_identity()`, the cleanup-failure branch, the helper's remove-on-failure semantics and the regression marker.
- NVRAM `43847475238` was in progress at last inspection; XY-C1 ancestor run `43846822718` had been building. No L3/L4 claim.
- Next: inspect exact-head HTTPD/USB/AI and XY-C1/NVRAM results; then continue module readiness and broad RC startup-status governance.

---

## 2026-11-16 — Report modem module family readiness through boot and RC actions

- Parent main: `a445812cf9e168d21f24f51e035cddf6934c12d9`. Exact-parent USB identity `43847475018`, Process inventory `43847475193`, AI docs `43847474937`, infosvr `43847475001`, AdGuardHome `43847475008`, AliDDNS `43847475086`, AliyunDrive `43847474932` and HTTPD check `43847474960` were observed; NVRAM `43847475238` and Process integration `43847474942` were in progress, while HTTPD reported failure in the parent run. Do not treat this parent evidence as current-SHA verification.
- `module_smart_load()` returns zero for both already-loaded modules and failed loads. Added `rc_modem_module_load_group_verified()`: attempt every optional candidate, then check observable module state. A family is available when at least one alternative is present; this does not make every NDIS/RAS driver mandatory.
- Changed `reload_modem_modules()` to return explicit NDIS/RAS family readiness. NDIS preserves all seven candidate attempts and supplemental serial AT/control loads; RAS preserves its four serial candidate attempts. Existing unload behavior and delays remain intact.
- Aggregated readiness into initial USB boot module health and both explicit modem-reconfiguration notification results.
- Added GNU89/Werror/ASan/UBSan policy regressions for one available alternative, already-loaded state, all candidates absent, parameter forwarding, all-candidates-attempted semantics and invalid-input preflight; added semantic source checking and workflow filters/steps.
- No Actions have run for this new source slice. Module presence does not prove that the selected USB interface bound to a driver; interface binding, exact-SHA XY-C1 build/TRX/artifact and L4 hardware remain unverified.
- Next: inspect this commit's exact-head USB identity, module policy, Process, XY-C1 and AI docs Actions; then investigate selected-interface-to-driver binding only if current sysfs/hotplug data supports it.

---

## 2026-11-16 — Propagate LAN boot and restart setup failures

- Parent main: `aeba90439f0b68cd35a67be7d3e821164597bc4f`. USB identity `43848100315` passed. AI docs `43848100602`, HTTPD `43848100398`, WAN route `43848100725`, WAN teardown `43848100704`, infosvr `43848100855`, Adbyby `43848100618`, OpenVPN client `43848100354`, AliyunDrive `43848100655`, ZeroTier `43848100633`, Crond `43848100348`, NFS `43848100601`, iNIC `43848100591`, Firewall `43848100711`, VPN server `43848100283`, miniupnpd `43848100903`, wireless `43848100599`, rstats `43848100463`, watchdog `43848100503`, PID 1 notifications `43848100441`, networkmap `43848100396`, detect_internet `43848100598`, detect_link `43848100654`, FRP `43848100482` passed.
- Process inventory `43848100470`, XY-C1 `43848100619`, USB storage `43848100656`, Inadyn `43848100652`, AliDDNS `43848100545`, Samba `43848100636`, DDNSTO `43848100671`, AdGuardHome `43848100509`, DNSMasq `43848100501`, NVRAM `43848100605` were still in progress at last inspection.
- Found that `start_lan()` discarded bridge IFUP, AP DHCP launch, manual resolver/gateway and router static-route statuses, so neither PID 1 nor `full_restart_lan()` could account for these failures.
- Converted `start_lan()` and `lan_up_manual()` to status-returning APIs, aggregate first errors while continuing independent follow-up work, and propagated status through `init_router()` and `full_restart_lan()`. Added GNU89 policy regression cases and a Process CI semantic source contract.
- New exact-head Actions pending. No local full target build or hardware test is claimed.
- Next: inspect exact-head Process/XY-C1, then audit remaining `config_bridge()` and `init_bridge()` ignored status operations.

---

## 2026-11-16 — Align XY-C1 modem config checker with alternative driver arrays

- Parent main: 5c74753c5af2951f477f733b1f3d6e1d78dc1824. Prior XY-C1 run 43848100619 failed before firmware Build because the checker still required module_smart_load("cdc_ether", ...) to be a direct call in reload_modem_modules(); the loader now dispatches NDIS/RAS array candidates through load_modem_module_group().
- Reworked scripts/check_usb_modem_driver_config.py to verify exact NDIS/RAS candidate arrays, parameter alignment (cdc_ncm: prefer_mbim=0), family branch routing, verified generic loader callbacks and post-load /sys/module probe order. It still enforces all 11 driver Kconfig symbols as modules plus USB serial option Kconfig/Makefile wiring.
- Live-source precommit checks confirmed both arrays, the branch calls and group loader semantics. The prior exact-head modem module regression 43848100315 passed; this is a checker-only correction, not a driver-code change.
- Parent 5c74753c5af2951f477f733b1f3d6e1d78dc1824 XY-C1 run 43848474467 and Process/LAN run 43848475098 were in progress at last inspection. No Build/TRX/artifact L3 or physical-router/L4 result is claimed.
- Next: inspect exact-head checker rerun and current LAN/Process/XY results; then continue evidence-based device binding and broader RC service reliability work.

---

## 2026-11-16 — Fix LAN status function return types and resolver contract

- Parent main: `be64a1990828e92895b4ce507216c509e1b35ff3`. Process inventory/lifecycle run `43849012514` passed the earlier LAN function-body/caller checks; AI docs `43849012491`, infosvr `43849012433`, AdGuardHome `43849012473`, AliDDNS `43849012496`, AliyunDrive `43849012441` also passed.
- Source review found the previous conversion left stale `void` tokens before `start_lan()` and `lan_up_manual()`, yielding invalid/conflicting return declarations despite int prototypes and return statements. Removed both stale tokens and added exact production-definition assertions to Process CI.
- XY-C1 `43849012639` passed the corrected driver-candidate checker and all preceding policy tests; Build XY-C1 was in progress at last inspection on the parent SHA. The parent still contained the LAN prefix defect; no TRX/artifact L3 result has been established.
- DNSMasq `43848474699` on ancestor `5c74753` failed because its lock-failure source contract still expected bare `return;`; the status-returning implementation returns `saved_errno`. Updated the contract accordingly.
- Exact-head DNSMasq/Process/XY-C1 runs for THIS commit are pending. No full firmware/TRX or physical-router result is claimed.
- Next: inspect the exact-head reruns and confirm corrected LAN definitions compile; then continue capability-aware bridge setup audit.

---

## 2026-11-16 — Propagate bridge multicast setup errors through LAN/IPTV status

- Parent main: 47f1534749d11be908c9af57d5f1d31e28029ff8. Process inventory 43849407767 and Process/LAN regression 43849407856 passed; DNSMasq 43849407985, AI docs 43849407655, infosvr 43849407642, AdGuardHome 43849407657, AliDDNS 43849407685, AliyunDrive 43849407661, HTTPD 43849408102, WAN DHCP 43849408056, detect_internet 43849407600, detect_link 43849407631 and NVRAM 43849408107 passed.
- Found config_bridge() discarded seven status-returning bridge/port/PHY operations. Added an injected-operation policy that attempts all seven, preserves the first error and reports failure count/first operation; GNU89/Werror/ASan/UBSan host regression passed locally.
- config_bridge() now returns status. start_lan() aggregates it after other LAN setup; restart_iptv() aggregates it with IPTV proxy actions while still attempting independent starts and preserving the AP stop-before-replace safety gate.
- XY-C1 run 43849407925 was still at Build XY-C1 on parent 47f1534749d11be908c9af57d5f1d31e28029ff8; no TRX/artifact L3 or physical-router/L4 result was available. This commit requires new exact-head Process and XY-C1 validations.
- Next: check exact-head bridge policy tests, Process source wiring, XY-C1 Build/TRX/artifact and NVRAM; continue capability-aware bridge setup audit.

---

## 2026-11-16 — Correct bridge return-type source assertion escaping

- Parent main: b81207777615d31b096c1425ff1fa8110898b0c2. The bridge policy source and test commit b81207777615d31b096c1425ff1fa8110898b0c2 is published; local GNU89/Werror/ASan/UBSan policy test had passed.
- Exact-head Process run 43850461801 was in progress at the earlier LAN status regression; the new bridge C test and source contract had not yet completed. XY-C1 run 43850461476 was still before Build. Process inventory 43850461826 passed.
- Source review found the newly inserted C return-type assertion was over-escaped in Python text. Replaced it with a chr(10)-based exact declaration check; no production C change.
- Next: verify Process bridge test/source checker and XY-C1 current-SHA Build/TRX/artifact; then continue capability-aware bridge/IPTV result handling.

---

## 2026-11-16 — Align storage IPTV source contract with aggregated restart results

- Parent main: `c066d1f230a2cadfceb091ccc9f45233f17dff09`. USB storage workflow run `43850461648` on source commit `b81207777615d31b096c1425ff1fa8110898b0c2` passed storage/eject lock, BusyBox 1.24.2/ash, caller graph and persistence checks, then failed at `scripts/check_storage_safe_remove.py` because it expected the old direct `return start_igmpproxy(...)` and bare stop return.
- Production `restart_iptv()` now merges `config_bridge()`, router proxy start, AP proxy stop and enabled UDPxy/XUPnPD start outcomes through the first-error policy; the AP stop-before-replace gate must remain before starting either replacement proxy. Updated the source contract to check those current semantics and ordering, without relaxing the safety invariant.
- Process bridge source/regression validation `43850767730` passed on exact main `c066d1f230a2cadfceb091ccc9f45233f17dff09`, as did AI docs `43850767697`, infosvr `43850767611`, AdGuardHome `43850767872`, AliDDNS `43850767755` and AliyunDrive `43850767862`. NVRAM `43850767802` was pending.
- XY-C1 `43850461476` for production source SHA `b81207777615d31b096c1425ff1fa8110898b0c2` was still at Build; TRX verification/artifact upload had not completed. No L4 result.
- THIS COMMIT triggers the storage hotplug workflow through the checker path. Next: inspect the exact-head storage rerun and resolve any further first-causal failure before selecting another RC reliability fix.

---

