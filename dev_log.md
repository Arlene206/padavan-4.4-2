## 2026-11-17 00:39 CST — Keep storage checker self-contained

- Parent main: `528da87938e245b3c45c0bf95839b9a0d9676ec1`. The prior checker correction normalized whitespace with `re.sub()` but the standalone checker did not import `re`; it would raise `NameError`. Replaced it with Python's built-in `" ".join(text.split())`, requiring no additional import or dependency.
- On parent c1aca, USB storage `43907888057` passed steps 18–24 and failed step 25 at the formatting-sensitive source assertion. The first checker retry on parent `528da87938e245b3c45c0bf95839b9a0d9676ec1` was queued as run `43908320511`; its final state must be checked, but this new commit removes the unimported regex dependency. No product source changed.
- Parent c1aca XY-C1 run `43907888399` and NVRAM `43907888090` were in progress at last poll; exact source L3 remains tied to whichever product SHA completes all three gates. No local checkout/compile is available. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 00:32 CST — Normalize multiline targeted-eject source assertions

- Parent main: `c1aca046300ec7738a3455c779afb988f5d71cb6`. The targeted-eject source, policy and regression binary all passed steps 18–24 of USB storage run `43907888057`; step 25 failed only because `scripts/check_storage_safe_remove.py` compared a function call split across two C source lines against a single-line exact substring. No product-source/test failure was reported.
- Normalized C function-body whitespace before asserting the retry helper calls, so the source contract is formatting-robust while still requiring both the failed-start and UNKNOWN-inventory paths. The rest of step 25's scheduling-after-unlock contract remains unchanged.
- Exact-head Process lifecycle run `43907888085`, AI handoff docs `43907888032`, HTTPD security `43907888445`, Samba `43907888436`, NFS `43907888144`, vsftpd `43907888072`, modem identity `43907888115`, infosvr `43907888362`, AdGuardHome `43907887961`, AliDDNS `43907888339` and AliyunDrive `43907888387` passed on parent c1aca. XY-C1 run `43907888399` and NVRAM audit `43907888090` were IN_PROGRESS at last poll. The parent storage run's only failure is this checker assertion; exact-head reruns are pending for THIS COMMIT.
- No local checkout is available; no local compile/checker pass is claimed. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 00:25 CST — Preserve storage-app retries after targeted eject

- Parent main: `8fca58d5479d65fcc8811934f41a71026a240c37`. Found a reliability gap in `safe_remove_stor_device()`: after stopping integrated storage services, a failed restart while other mounts remained was logged but did not restore `usb_hotplug_ms` or arm the deferred five-second retry. An UNKNOWN final mount inventory also left services safely stopped but had no scheduled re-check.
- Added `storage_ejected_storage_start_retry_required()`: retry when remaining mount count is UNKNOWN, or when mounts remain and service startup failed; never retry solely because a service failed when no mounts remain. The real eject path now sets the existing `usb_hotplug_ms` event and calls `alarm(5)` only after freeing the inventory and releasing the storage-operation lock.
- Extended the existing GNU89 policy regression for positive/zero/unknown mount counts and success/failure statuses. Added source contracts for the failed-start and UNKNOWN branches plus event re-arming/lock ordering. The existing storage workflow already includes these source/test paths and runs the policy test with compiler warnings as errors and sanitizers.
- No local repository checkout is available, so no local compile or full source checker is claimed. Process inventory/lifecycle run `43907096476` was IN_PROGRESS on parent `8fca58d5479d65fcc8811934f41a71026a240c37` at last poll. The source slice's exact-head USB storage and XY-C1 Build/TRX/artifact checks are pending. Product SHA `d6fac506d564e8277e6c81ec2dd18620b6509144` previously passed the storage regression and XY-C1 L3. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 00:21 CST — Add the missing local C-function extractor to process checker

- Parent main: `d935aed8045f940a42d4c19d46b26bcfdcdd09cb`. Exact-head Process inventory/lifecycle run `43906449302`, job `133436469037`, failed at step 38 because the checker-only fix called `body()`, but `scripts/check_process_lifecycle_integration.py` has no such helper (the similarly named helper exists only in a different standalone checker and is not imported). The failure was `NameError: name 'body' is not defined`.
- Added a local brace-aware `c_function_body()` implementation that skips C comments, strings, character literals and forward declarations; mount-link implementation, adapter and four callers now use that local parser. This corrects the checker without modifying product source.
- AI handoff docs run `43906449223` passed on parent d935. XY-C1 product build `43904136433` has since passed Build XY-C1, TRX verification and artifact upload on exact product SHA `d6fac506d564e8277e6c81ec2dd18620b6509144`. Exact-head Process and docs checks for THIS COMMIT are pending. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 00:13 CST — Make process lifecycle storage contract use structural C function extraction

- Parent main: `e8a7aedab3bf7d1cc5b5487234ef2efc3b53ba5b`. Exact-head process-lifecycle run `43905023740`, job `133431436756`, failed only at step 38 (`Verify shared-library and RC integration boundaries`): the newly added mount-link adapter assertion searched for the literal characters `\\n` rather than the actual source newline and raised `IndexError`.
- Replaced fragile newline-delimited source slicing for `create_mp_link()`, `create_mp_link_status()`, and its four media callers with the checker's existing brace-aware `body()` extractor. This keeps the assertions tied to actual function bodies and avoids matching prototypes or consuming subsequent functions.
- Product source was unchanged. No local repository checkout was available, so no local full checker run is claimed. Exact-head process-lifecycle and AI handoff documentation Actions for THIS COMMIT are pending; current XY-C1 L3 remains tied to the d6 product-source SHA only until its same-SHA build/TRX/artifact stages finish. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Align process-lifecycle source contract with deterministic storage mount-link API

- Parent main: `d6fac506d564e8277e6c81ec2dd18620b6509144`. Exact-head process-lifecycle run `43904136542`, job `133428303695`, passed all preceding shared file, MiniDLNA, Firefly, UPnP, VPN, DDNS, logger, optional-app, IAPP, IGMP, XUPnPD, MSD Lite, SSH and Transmission regressions, then failed only at `scripts/check_process_lifecycle_integration.py:200`: its source assertion still expected the former one-line cleanup predicate and `return link_created;`.
- The source implementation in `d6fac506d564e8277e6c81ec2dd18620b6509144` intentionally stages cleanup and returns explicit boolean branches so it can normalize errno for read/close/parse/symlink failures. Updated the checker to assert the actual staged cleanup, boolean API, status adapter and all four media callers. No product source was changed.
- Exact-head USB storage workflow `43904136310` PASS, including the new GNU89/ASan/UBSan mount-link policy test and storage source contract. Wireless `43904136184`, AI docs `43904136356`, HTTPD `43904136370`, AliDDNS `43904136262`, AliyunDrive `43904136387`, NFS `43904136537`, Samba `43904136473`, infosvr `43904136412`, vsftpd `43904136251`, modem identity `43904136447`, AdGuardHome `43904136471` also passed on the parent SHA.
- XY-C1 Build `43904136433` was still in progress at last poll; current-SHA L3 and physical-router L4 are NOT VERIFIED.

---

## 2026-11-16 23:57 CST — Make storage media mount-link failures deterministic

- Parent main: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`. Implemented deterministic errno handling in `create_mp_link()` while retaining its boolean API and current mount-selection/fallback behavior. Invalid input, mount inventory open/read/parse/close, path length, directory preparation, symlink creation and incomplete-inventory cleanup now produce normalized errors; the first actionable non-ENOENT error is preserved.
- Added `create_mp_link_status()` before all four media callers. MiniDLNA and Firefly keep their existing mode-0 then mode-1 fallback; Transmission and Aria retain mode 0 only. Callers return normalized status rather than inheriting stale errno.
- Added GNU89 errno-policy cases and storage source contracts; wired the test into the storage workflow and both push/PR filters. Corrected the wireless force-disable contract to require the current bridge-membership helper. Added the literal `one atomic main commit` handoff invariant required by the documentation validator.
- Exact parent-SHA evidence: XY-C1 run 43897222785 passed Build XY-C1, TRX verification and artifact upload on `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`; that result does not verify this new source commit. Parent Wireless run 43897222713 and AI handoff run 43897222818 failed on stale contracts, both corrected in this milestone. New exact-head checks are pending.
- Local full-repository compile/source checks were not run because no local repository checkout is available in this execution environment; do not treat this as a local test pass. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-16 — Stabilize mount-link errno and storage daemon status reporting

- Main parent at checkpoint: `435293278a5deca3b80bd5a21f876a49445d7430`; latest source commit identified: `339d62070960aea11fee1d0b5b18fbf7e373cb86`.
- Storage service startup status is propagated for FTP/Samba/NFS and media/download daemons; deferred hotplug merges the first integrated service error with Optware worker state. USB modem coalesced unplug/replug state and retry restoration are covered by a pure GNU89 policy regression. WAN IPv6 and LAN/Wi-Fi startup failure propagation are also in the source history.
- Mount-link errno normalization on current source: not confirmed; the next GPT must verify/implement it before treating media startup errors as deterministic.
- Exact product-source SHA: `339d62070960aea11fee1d0b5b18fbf7e373cb86`. XY-C1 exact-SHA status: UNVERIFIED; individual steps: not returned.
- Do not treat missing checks as PASS or carry forward older Build/TRX/artifact results to this SHA. Physical-router L4 remains NOT VERIFIED.
- Next: verify storage mount-link error normalization and its GNU89/source-contract tests, then continue storage lifecycle hardening.

---

## 2026-11-16 — Remove duplicate escaped storage declaration assertions

- Parent main: 8f0d792ec0ab7569ffd1ecf7216008b2396b2faf. Exact-head USB storage run 43893228398 failed at source-check line 218 because an earlier editing script accidentally duplicated four declaration checks with literal double-escaped \\n sequences. The original newline-aware C declaration assertions immediately above were correct; they remain unchanged.
- Removed only the four redundant, incorrectly escaped assertions. No product source or host regression was changed. WAN route 43893228430, NVRAM transaction 43893228512, process inventory/fail-closed 43893228310, process lifecycle inventory 43893228288 and the other named exact-head checks passed. XY-C1 run 43893228097 was still IN_PROGRESS at last query.
- This checker-only correction requires its own exact-SHA storage run before the storage workflow is considered green.

---

## 2026-11-16 — Preserve failed integrated storage startup events for retry

- Parent main: 7b8b3129261fc846cb972cee4fdc73dfca0344bd. This source slice fixes a real retry-loss path: `try_start_stor_apps()` formerly cleared `usb_hotplug_ms` after a failed FTP/Samba/NFS/media-daemon start, despite returning a failure that caused `on_deferred_hotplug_dev()` to schedule `alarm(5)`. The event now clears only after status 0; failures remain queued.
- The nonblocking Optware child result is read once before integrated startup retries. A still-running child defers the retry without launching duplicate integrated daemons; completed child outcomes are consumed once, and the Optware hook remains outside `storage.lock`.
- Added a production-used event-completion predicate, host test for positive/negative startup errors retaining the event, and stronger storage source contracts for function declarations, single result read, worker-running deferral and success-only acknowledgement.
- Local GNU89/Werror/ASan/UBSan host policy test: PASS. Exact-head Actions after publication are the required next verification; firmware L3 and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-16 — Align RC process inventory checks with status-bearing storage APIs

- Commit: 7b8b3129261fc846cb972cee4fdc73dfca0344bd. Exact-head Process inventory and lifecycle fail-closed run 43890908528 PASS; AI handoff docs 43890908405 PASS.
- Updated the process-lifecycle integration checker to follow the status-returning storage/media daemon signatures. This was a checker adjustment; the latest source implementation is inherited from a706e247.

---

## 2026-11-16 — Assert storage restart wrapper stop-status propagation

- Commit: 72064974e7b099112e587d1c5aad1c410c2b3789. Exact-head USB storage run 43890786101 PASS; AI docs 43890785916 PASS.
- Extended the storage workflow contract to verify that media restart wrappers preserve stop failures and do not launch replacement daemons after an unsuccessful stop.

---

## 2026-11-16 — Propagate optional media daemon restart results

- Commit: a706e2470bbb381cbd8acd2afb150db30e069839. Exact-head storage run 43890645158 PASS; Process lifecycle inventory run 43890651174 PASS; broad lifecycle/security workflows mostly PASS. Process-lifecycle regression 43890644968 and WAN route result 43890645171 FAILED their source-check steps. XY-C1 Firmware Validation 43890644424 was IN_PROGRESS at last inspection; do not claim same-SHA L3.
- Updated `rc.c`, `rc.h` and `services_stor.c` so optional media daemon restart results are propagated through the notification path. Follow-up work must isolate the WAN-route checker AssertionError and rerun the process-lifecycle regression on a current SHA.

---

## 2026-11-16 — Update storage restart wrapper source contracts

- Commit: 185df64bc5fe1474c088ec802cea4737f09dc4f3. Exact-head USB storage run 43890141411 PASS; AI docs run 43890141441 PASS.
- Updated the embedded storage checker for the status-bearing media daemon restart contract after the production API changed. No product source changed in this commit.

---

## 2026-11-16 — Propagate IPv6 restart failures through notifications

- Commit: 43ba6d5379fe15294cf67774638cd470fda069b9. Exact-head WAN route status 43889906249 FAILED its result-propagation checker; USB storage 43889906361 FAILED an outdated media-daemon signature assertion; Process lifecycle regression 43889906502 FAILED in the checker with an IndexError. XY-C1 43889906595 was IN_PROGRESS at last inspection.
- Added `ipv6_restart_policy.h` and returned full IPv6 restart errors to notification aggregation, preserving runtime mode until restart success. The WAN-route source-contract failure remains open for a precise first-failure diagnosis; later runs still reproduced it.

---

## 2026-11-16 — Propagate media daemon readiness failures

- Commit: c7f26f0291b52b3e8ee5ade59f30635064575afb. Exact-head XY-C1 43889104925 PASS; Process inventory job 43889104967 PASS; Process-lifecycle regression 43889104590 FAILED with a checker IndexError; USB storage 43889104698 FAILED because its checker still expected the old `void run_dms` definition after the production API became status-returning.
- Extended `start_stor_apps()` to include MiniDLNA, Firefly, Transmission and Aria readiness statuses in addition to FTP/Samba/NFS. Later workflow-only fixes addressed the stale media signature assertion, but process-lifecycle integration still needs an exact-current-head regression run.

---

## 2026-11-16 — Restore valid storage startup function declarations

- Commit: 0111177585699cfacbf7878d1554eeb6bc861537. Exact-head XY-C1 Build/TRX/artifact workflow 43888486194 PASS; USB storage 43888486392 PASS; modem identity 43888486233 PASS; Process inventory 43888486156 PASS; NFS 43888486225 PASS; VSFTPD 43888486195 PASS; HTTPD security 43888486191 PASS; Samba 43888486235 PASS; AI docs 43888486129 PASS.
- Corrected the duplicate `void int start_stor_apps` and `static int static int try_start_stor_apps` declaration errors introduced in 847089c. The same-SHA XY-C1 stages completed successfully on this SHA; later product-source commits require their own build evidence.

---

## 2026-11-16 — Update the storage FTP startup source contract

- Commit: d9c3a5f5b7909da615ecce96b2bcd30f55472a99; exact-head VSFTPD run 43887302690 PASS and AI handoff documentation run 43887302364 PASS.
- Updated only `.github/workflows/vsftpd-stability.yml` so its source contract expects `run_ftp()` status capture and first-error aggregation after the storage startup API became status-returning. No product source changed.

---

## 2026-11-16 — Align the USB modem hotplug contract with retry-state handling

- Commit: 9e06fc918801d858c2ddd51e2dd88bb93d0dc4d3. Exact-head USB modem identity run 43887173584 and AI docs run 43887173552 both PASS.
- Updated USB modem workflow assertions to match the production retry-mask policy for coalesced unplug/replug events. No production source changed in this checker-only correction.

---

## 2026-11-16 — Normalize USB modem retry source assertions

- Commit: d191f227845d5a7ff13960c07ac29c32abf86a76. Exact-head USB modem identity run 43886677055 FAIL; AI docs 43886677149 PASS.
- Normalized whitespace in the retry-policy expression assertion. A later exact-head contract revision (9e06fc9) aligned the remaining status-variable assertions, and the workflow passed on 9e06fc9.

---

## 2026-11-16 — Preserve coalesced USB modem hotplug events

- Commit: f38dc1c89f7f0d506dfa673bc145d65f6fcaa3d3. Exact-head storage 43885842312, XY-C1 43885842274, Process inventory 43885842187, Process lifecycle inventory 43885842518, HTTPD security 43885842320, and AI docs 43885842849 PASS. USB modem identity run 43885842214 FAIL; its stale checker was corrected in subsequent checker-only commits d191f22/9e06fc9, with exact-head pass on 9e06fc9.
- Added a retry mask so simultaneous modem unplug/replug preserves the replug event until the unplug transition completes. Failed reconnects re-arm the appropriate event flags and schedule the existing five-second retry.

---

## 2026-11-16 — Restore complete development history after log truncation

- Parent main: be65a873365598d99cccce9513e1e720203b6d8a. While recording the new modem WAN hotplug result-propagation task, the prior commit accidentally replaced the entire root dev_log.md rather than prepending its new entry. Detected immediately from the commit diff (+8/-432); restored the complete prior root log from ea794e3ef680da5b03ca0ffdd621601c97dc7d86 and preserved this recovery entry above it.
- No source files or test code were changed by this repair. The current modem source implementation remains in be65a873365598d99cccce9513e1e720203b6d8a; its exact-SHA USB modem identity, XY-C1 firmware, storage and process lifecycle checks were queued/in progress at last inspection.
- Next: inspect those exact SHA Actions and repair any causal failure before continuing broader P1 service-start audits.

---

## 2026-11-16 — Propagate USB modem reconnect failures and preserve hotplug retry events

- Product-source commit: be65a873365598d99cccce9513e1e720203b6d8a. Review found try_start_usb_modem_to_wan() discarded try_wan_reconnect() status, while on_deferred_hotplug_dev() cleared usb_unplug_md/usb_hotplug_md before reconnect and never re-armed them after failure.
- Converted the helper/prototype to int. Existing no-op eligibility paths still return 0; attempted reconnect returns actual try_wan_reconnect() status. Deferred modem unplug and plug flows now log failure, restore usb_unplug_md or usb_hotplug_md respectively, and schedule the existing five-second alarm retry.
- Extended usb-modem-identity-stability.yml with helper-to-caller status/retry assertions and added services_stor.c/services_usb.c to both push and pull_request filters; rc.h is included in the checker path contract.
- Parent ea794e3 storage 43882865182 passed all 24 steps. AI docs 43882864787 failed only because the handoff lacked its literal mandatory checkpoint statement; the current handoff now restores that required wording. Other parent actions listed in SESSION_HANDOFF.md passed.
- Earlier firmware product-source baseline cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d passed same-SHA XY-C1 Build, TRX and artifact upload. Current modem source commit be65a873365598d99cccce9513e1e720203b6d8a must pass a new same-SHA firmware run; physical-router L4 remains NOT VERIFIED.
- Next: inspect modem identity, XY-C1, storage, AI docs and remaining exact-head Actions before expanding the service-start status audit.

---

## 2026-11-16 — Align storage safety checker with checked printer restart API

- Parent main: 52a5cc63419686af16b23c9aee973d262f3e4529. Exact-head storage run 43882073147 passed all BusyBox ash, storage persistence/eject, C regressions and the whole caller graph/lock-order checker (step 23), but final scripts/check_storage_safe_remove.py failed at line 294.
- The checker still expected the old void API shape (direct stop comparison and bare return). Current restart helpers correctly retain int stop_status, return that error, and only call exec_printer_daemons() after a successful stop. Updated the checker to assert this status-return contract and ordering.
- XY-C1 run 43880814725 on product-source SHA cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d passed Build, TRX verification and artifact upload on the identical SHA. Commit 52a5 contains only workflow/docs changes. Physical-router L4 remains NOT VERIFIED.
- Next independent defect: USB modem hotplug loses WAN reconnect failure status and does not re-arm the cleared event flag. Convert the called helper to int and preserve retry state on failed reconnect.
- Re-run storage on this checker commit's exact SHA; modem source slice should trigger USB modem identity and XY-C1 validation.

---

## 2026-11-16 — Restore the complete USB storage stability workflow

- Parent main: `cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d`. Its storage-hotplug run `43880814702` / job `133345880304` failed at the embedded source-contract step with an unterminated Python string. The workflow file ended at `push_filters, pr_filters = filters.split("`, deleting the remaining storage caller graph, locking and teardown assertions, and the checked firmware-persistence gate.
- Restored the entire suffix from intact workflow parent `0409c2b90d8da55531b8370dbb16e7a1ed978d67`, preserving the current prefix's newer USB printer source checks, first-error aggregation tests and both push/PR path filters. Structural validation: 1,163 lines; source-check heredoc terminator and final `check_storage_safe_remove.py` step restored.
- Parent XY-C1 `43880814725` was IN_PROGRESS at last poll; AI docs `43880814989`, infosvr `43880814630`, NVRAM `43880814712`, AdGuardHome `43880814663`, AliDDNS `43880814812`, AliyunDrive `43880814843`, process lifecycle `43880814751`, process inventory `43880814602`: PASS.
- Firmware L3 remains verified only for baseline `f6fba229aba497988d2308c8c99879aca5da1214`; no current-SHA L3 claim. Physical-router L4 NOT VERIFIED.
- Next: inspect exact-head storage-hotplug and XY-C1 Actions for the restored workflow/source.

---

## 2026-11-16 — Repair USB storage workflow PR filters and report optional hook failures

- Parent main: `0409c2b90d8da55531b8370dbb16e7a1ed978d67`. Exact-head storage-hotplug run `43880100890` / job `133343396256` failed at embedded source-check line 138: the checker correctly required every test input in both event filters, but `service_start_status.h` and `tests/service_start_status.c` were included only in push paths. Added both to pull_request paths.
- The latest source also uses a fork/exec FD_CLOEXEC pipe for u2ec/LPRng, preventing background-shell success from masking exec/setup failures. It confirms exec handoff, not daemon runtime readiness.
- Found another error propagation gap: an installed `on_hotplug_printer.sh` hook returned a status that was logged but not merged. Added the hook status to first-error aggregation while preserving all later p910nd/u2ec/lpd attempts, with GNU89 regression and source assertion.
- Storage step 22 compiled and ran `service_start_status.c`; all prior ash/storage checks passed. Same-SHA XY-C1 `43880100797` and NVRAM `43880100873` were still in progress at last inspection; AI docs `43880101001`, process inventory `43880100870` and process lifecycle `43880100789` passed. Earlier `f6fba...` firmware L3 does not transfer to newer SHA; L4 remains NOT VERIFIED.
- Next: verify exact-head storage and XY-C1 Actions, then resume P1 cross-userland RC startup-result propagation.

---

## 2026-11-16 — Propagate USB printer daemon launch failures

- Parent main: `f6fba229aba497988d2308c8c99879aca5da1214`. Review of `services_usb.c` found that `start_p910nd()` discarded `eval()`, `start_u2ec()` and `start_lpd()` discarded `system()`, and `exec_printer_daemons()` plus both public spooler entry points were void. The actual callers lost failures as well: `handle_notifications()` completed spooler requests and `on_deferred_hotplug_dev()` cleared the pending printer hotplug flag without checking launch results.
- Converted the three launcher APIs and spooler entry points to status-returning functions. The printer loop records the first nonzero launch status while continuing attempts for every detected printer and both shared spoolers. Optional user hook failures are logged separately and do not suppress core printer daemon startup.
- `handle_notifications()` now merges spooler restart status into the notification result so failed launches retain the incomplete marker. Deferred hotplug logs failure, restores `usb_hotplug_lp` and schedules its existing five-second retry.
- Expanded the GNU89 service-start policy regression with printer first-error/continue cases, and the storage workflow now checks all source/caller edges and compiles/runs this test. Exact-SHA validation is pending for the commit produced by this slice.
- Base SHA `f6fba229aba497988d2308c8c99879aca5da1214` has XY-C1 Build, TRX and artifact upload PASS on the same SHA; product-source L3 VERIFIED, physical-router L4 NOT VERIFIED.

---

## 2026-11-16 — Fix the DHCP IPTV optional-MTU compile failure

- Parent main: `17df1d524e5f0ac72951381d56cf238c8699c1e9`. Exact-head XY-C1 run `43874973723` failed at `net_wan.c:4344`: `udhcpc_viptv_bound()` used `mtu_status` without declaring it. Pre-build regression/source steps 9–16 passed; TRX/artifact steps were skipped after compile failure.
- Added the missing `mtu_status` local to the IPTV DHCP callback. Extended the WAN DHCP embedded source contract and `scripts/check_modem_wan_start_status.py` to require both declaration and MTU assignment, catching this GNU89 compile error before firmware packaging.
- Parent exact-head NVRAM `43874973747`, Process inventory `43874973647`, AI docs `43874973791`, AdGuardHome `43874973840`, AliDDNS `43874973707`, AliyunDrive `43874973891`, infosvr `43874973811`: PASS.
- Next: inspect exact-SHA XY-C1 Build/TRX/artifact and WAN DHCP/route Actions. Do not claim firmware L3 until all firmware stages pass on one SHA; physical-router L4 remains unverified.

---

## 2026-11-16 — Trigger XY-C1 firmware validation for PPP policy dependencies

- Parent main: `f16167cf2ea3666810341f1edd65bfcafeda28dc`. PPP ip-up strict-address validation lives in `ppp_wan_ipup_policy.h` and its `dhcp_lease_policy.h` dependency, with production-linked regression coverage in `tests/ppp_wan_ipup_policy.c`. WAN DHCP regression `43873889736` had passed on the previous tested SHA, but the XY-C1 workflow only watched `net_ppp.c`, so header/test-only changes did not schedule a firmware cross-build.
- Added all three policy/test paths to `.github/workflows/xy-c1-firmware-validation.yml`. The workflow watches its own file on push, so this workflow-only commit should dispatch XY-C1 validation against the current strict-address source.
- Parent exact-head WAN route `43874341443`, AI docs `43874341634`, Process inventory/lifecycle `43874341728`, AliDDNS `43874341573`, AdGuardHome `43874341534`, AliyunDrive `43874341457`, infosvr `43874341851`: PASS. NVRAM `43874341561` was IN_PROGRESS at last poll.
- Next: verify exact-SHA XY-C1 Build, TRX and artifact upload. No firmware L3 or physical L4 claim until the required evidence exists.

---

## 2026-11-16 — Fix the WAN route source contract after optional MTU reporting

- Parent main: `3375e565983097af355395bffd32a41dcf442b18`. Exact-head WAN route run `43873631807` for source parent `c5dfa15e8c0c4d3998ca01d2ee961f0897282855` failed only at the final source-contract step. Six preceding compile/regression steps passed.
- Cause: the checker required the old exact declaration `int interface_status, rollback_status, route_setup_status;` in `udhcpc_bound()`, but the preceding source slice correctly added `mtu_status` to capture the optional MTU ioctl result. The workflow now checks the extended declaration and verifies the MTU failure diagnostic does not set `i_err`, mutate `route_setup_status` or return early.
- Latest main `3375e565983097af355395bffd32a41dcf442b18`: WAN DHCP callback `43873889736` and AI docs `43873889717` PASS. Process inventory/lifecycle `43873889642` and NVRAM `43873889718` were running at last poll. XY-C1 `43873631679` on older SHA `c5dfa15e8c0c4d3998ca01d2ee961f0897282855` was still building; TRX/artifact pending. No L3/L4 claim.
- Next: verify the WAN route contract repair on its exact SHA, then continue independent process/NVRAM/PPP tasks.

---

## 2026-11-16 — Validate PPP IPCP IPv4 text before passing it to ifconfig

- Parent main: `c5dfa15e8c0c4d3998ca01d2ee961f0897282855`. Shared `ifconfig()` relies on `inet_addr_safe()`; unparsable IPv4 text becomes `INADDR_ANY`, while interface setup may still appear successful. PPP ip-up required `IPLOCAL` and `IPREMOTE` to be non-empty, but did not validate their text or its netmask before calling that helper.
- Reused strict `dhcp_lease_ipv4_is_valid()` for both negotiated IPCP endpoints and `dhcp_lease_netmask_is_valid()` for the mask. This rejects malformed/non-canonical dotted quads and `0.0.0.0` before the interface callback or lease publication.
- Added regression coverage for malformed local/peer strings, zero addresses, non-canonical dotted quads and non-contiguous masks. Invalid callbacks must return EINVAL and leave fake interface/NVRAM state untouched.
- Parent exact-head WAN DHCP callback `43872745445`, WAN route `43872745447`, AI docs `43872745205`, Process lifecycle `43872746090`, HTTPD `43872745252`, AdGuardHome `43872745181`, AliDDNS `43872745603`, infosvr `43872745324`, AliyunDrive `43872745057`, and NVRAM `43872745634`: PASS. Process inventory `43872745882` and XY-C1 `43872745244` were pending at last inspection.
- Next: validate exact-head PPP callback and firmware Build/TRX/artifact workflows; no same-SHA L3 or hardware L4 result is claimed.

---

## 2026-11-16 — Reject invalid static MAN masks and report optional MTU failures

- Parent main: `b4c6a520bf3d95822bb7757987a51bc3b37b5a7a`. In `launch_wanx()`, malformed MAN netmask text was converted to `NULL`; shared `ifconfig()` still brought the interface up and assigned the IP while skipping netmask configuration, then returned success. A non-contiguous dotted mask could also pass legacy `is_valid_ipv4()`.
- Static MAN now validates its host IPv4 address and contiguous nonzero netmask with the shared strict DHCP lease predicates before setting MTU or calling `ifconfig()`. Invalid static values set `wanx_err`, return EINVAL through `launch_status`, and do not configure the interface.
- Optional MTU syscall failures are logged on static MAN, static IPoE, DHCP WAN/MAN and DHCP IPTV paths; they deliberately remain nonfatal to preserve the existing compatibility contract.
- Extended the GNU89 modem WAN policy regression with malformed host/netmask cases and expanded `check_modem_wan_start_status.py` to enforce validation-before-side-effects and nonfatal MTU diagnostics.
- Parent exact-head WAN DHCP callback `43872745445`, WAN route `43872745447`, AI docs `43872745205`, Process lifecycle inventory `43872746090`, HTTPD `43872745252`, AdGuardHome `43872745181`, AliDDNS `43872745603`, infosvr `43872745324`, AliyunDrive `43872745057`: PASS. Process inventory `43872745882`, NVRAM `43872745634`, XY-C1 `43872745244` were in progress at last poll.
- Next: inspect exact-SHA checks for this source/test/checker slice; firmware L3 requires Build, TRX and artifact upload on one SHA. Physical-router L4 remains unverified.

---

## 2026-11-16 — Preserve the selected previous interface through IPv6 route cleanup

- Parent main: `ee722b0f0c475812c3fad536944fb521c4bf97d7`. Exact-head WAN DHCP callback `43872288966` failed at an outdated source assertion that still required `wan6_down()` to contain `if (ipv6_type == IPV6_DISABLED)`; it also encoded six accumulator calls from before the additional previous-interface RA/autoconf error gates.
- While updating the contract, found a real production defect: after selecting saved runtime `ifname6`, a legacy non-SIT `else` reset `wan6_ifname = wan_ifname`. This silently discarded the selected PPP/native interface during mode transitions, so route/address cleanup still targeted the new interface. Removed that overwrite so the helper's selection is retained through `clear_if_route6()` and `clear_if_addr6()`.
- Updated WAN DHCP workflow to track the IPv6 helper/test, inspect their policy contract, require the selected interface to survive through route/address teardown, and check eight status accumulations (including previous-interface RA/autoconf disable). Updated final state assertions to distinguish the no-interface disabled fast path from the common success path.
- On SHA `ee722b0f0c475812c3fad536944fb521c4bf97d7`, WAN route `43872288931` and AI docs `43872288772` PASS. Process inventory `43872289006`, Process lifecycle `43872288846`, HTTPD `43872289253`, AdGuardHome `43872289035`, AliDDNS `43872288789`, infosvr `43872289116`, AliyunDrive `43872288716` PASS. NVRAM `43872289074` and XY-C1 `43872288829` were pending at last poll.
- Next: verify new WAN DHCP, route, teardown, docs and XY-C1 exact-head runs; only claim firmware L3 after Build, TRX and artifact upload succeed on one SHA. Hardware L4 unverified.

---

## 2026-11-16 — Tear down the last live IPv6 interface after configuration changes

- Parent main: `fbd9a1554d30f394e9e0090b9148bdb574cc9f6a`. `full_restart_wan()` calls `stop_wan()` before `reset_wan_vars()`, and the latter invokes `reset_wan6_vars()`; therefore previous runtime `ifname6` is still available during stop even if NVRAM already describes the next IPv6 mode.
- Defect: `wan6_down()` returned success immediately when the *new* `get_ipv6_type()` was `IPV6_DISABLED`. This skipped RA/autoconfiguration shutdown, IPv6 route/address cleanup and SIT teardown when disabling IPv6; switching modes could also select the new interface instead of the old live one.
- Added `wan6_teardown_policy.h` to prefer the saved runtime interface while it exists. Disabled mode cleans a surviving SIT device or safely skips a vanished interface whose kernel link teardown already purged its routes/addresses. `dns6` and `ifname6` are cleared only after successful cleanup, retaining retry state on failure.
- Added GNU89/Werror/ASan/UBSan policy coverage for disabled native/SIT, vanished PPP interface, orphaned SIT, and active-mode reconfiguration. Extended WAN route workflow path filters, compile step and semantic source checks.
- Parent exact-head WAN route `43870829310`, WAN DHCP `43870829604`, WAN teardown `43870829555`, Process lifecycle `43870829665`, NVRAM `43870829549`, HTTPD `43870829629`, miniupnpd `43870829330`, DNSMasq `43870829617`, AliDDNS `43870829627`, VPN `43870829584`, AdGuardHome `43870829532`, AliyunDrive `43870829550`, infosvr `43870829594`, detect_internet `43870829447`, detect_link `43870829377`, USB identity `43870829507`, and WAN ARP `43870829586`: PASS.
- AI docs `43870829566` failed because the preceding source commit truncated the whole roadmap dashboard; the complete previous dashboard is restored here. XY-C1 `43870829606` was still building; no L3/L4 claim.
- Next: validate exact-head WAN route, teardown, AI docs and XY-C1 Actions; continue only on a concrete failure.

---

## 2026-11-16 — Validate static IPoE IPv4 configuration before ifconfig

- Parent main: `c3b02529703add66cbd811b8dfa729edef50ad74`. The shared `ifconfig()` helper converts invalid address/netmask text to `INADDR_ANY` and can still return success, so a malformed static WAN config could be published as a successful WAN start.
- Static IPoE now validates the IPv4 address and contiguous nonzero netmask with `dhcp_lease_ipv4_is_valid()` and `dhcp_lease_netmask_is_valid()` before MTU/address configuration. Invalid values set the unit error and aggregate `EINVAL`; WAN/LAN subnet overlap remains `EADDRINUSE`.
- Added GNU89 regression cases for invalid address and netmask text and source-contract checks that validate before `ifconfig()`. Also normalized any remaining old-arity PPP IP-up test calls to pass `NULL` for an absent peer address after the latest test failure.
- Parent run `43870165716` had failed compiling the IP-up test because two invocations omitted the optional peer-address parameter. This source/test commit includes the arity correction; its exact-SHA callback run must verify it.
- Next: inspect exact-head Process, WAN DHCP and XY-C1 Actions; no L3 claim until same-SHA Build, TRX verification and artifact upload all pass.

---

## 2026-11-16 — Fix PPP IPCP regression call arity after exact-head compile failure

- Parent main: `bacfb1d9d9c814fdc6d122de5e8e34cf85dcb900`. Exact-head WAN DHCP callback run `43870165716` reached the PPP IPv4 callback test compile and failed because `test_interface_failure_preserves_published_state()` retained a four-argument call after `ppp_wan_ipup_apply_address()` gained its required remote-address argument. The failure was confined to the regression call site; the production change was not compiled by that step.
- Added the missing peer address to the test invocation. Cleaned duplicate handoff headings left by the prior concurrent doc merge and updated the CI ledger.
- Parent PASS: AI docs `43870165549`, AdGuardHome `43870165477`, AliyunDrive `43870164931`, Process final inventory `43870164946`, AliDDNS `43870164965`, WAN route `43870164920`, infosvr `43870164981`, VPN `43870164985`, HTTPD `43870164960`. NVRAM `43870165218`, Process lifecycle `43870164923`, and XY-C1 `43870165029` were pending at last poll.
- Next: verify the new exact-head WAN DHCP compile, policy regression and source contract; continue the PPP IPv6/DHCPv6 callback audit only after the compile gate is green.

---

## 2026-11-16 — Reject incomplete PPP IPCP addresses before publishing WAN state

- Parent main: `8ab4d6ecb4e6b39078aaf5b944c91085f9bc8668`. `ipup_main()` treated missing `IPLOCAL` as optional, skipping interface-address application but still publishing interface identity and continuing into `wan_up()`. Missing `IPREMOTE` left the previous NVRAM gateway in place. PPPD invokes `ip-up` when IPCP has come up and provides both IPv4 endpoint variables; absent/empty values are an inconsistent callback contract.
- `ppp_wan_ipup_apply_address()` now requires non-empty local and remote IPCP endpoints before calling ifconfig or publishing the lease. `ipup_main()` retrieves both unconditionally and writes the current peer gateway only after validation.
- Added GNU89 policy coverage for NULL/empty local and remote endpoint values plus no side effects; strengthened the WAN DHCP callback workflow to prevent reintroducing the optional branches.
- Parent exact-head WAN DHCP callback `43869140586`, WAN route `43869140499`, Process lifecycle `43869140664`, Process inventory `43869140439`, miniupnpd `43869140553`, AI docs `43869140667`, HTTPD `43869140668`, infosvr `43869140608`, AliDDNS `43869140787`, VPN `43869140953`, DNSMasq `43869140657`, AdGuardHome `43869140393`, AliyunDrive `43869140626`, WAN teardown `43869140587`, WAN ARP `43869140602`, detect_internet `43869140477`, detect_link `43869140381`, USB identity `43869140444`, and NVRAM `43869140525` passed. XY-C1 `43869140714` was still building.
- This commit requires exact-head WAN DHCP and XY-C1 validation. Firmware L3 remains unverified until one SHA passes Build, TRX and artifact upload; hardware L4 remains unverified.
- Next: inspect exact-head Actions, then continue the bounded PPP IPv6/DHCPv6 callback state audit without weakening generation fencing.

---

## 2026-11-16 — Propagate static IPoE WAN/LAN subnet overlap to startup status

- Parent main: `c38c9d288707e98616c796bb0724e64fd3885eab`. Static IPoE WAN/LAN subnet overlap prevented `wan_up()` but only set local `wan_err`; it did not update `wan_start_status`, allowing PID 1 to observe false success. The branch now propagates `EADDRINUSE` with first-error semantics.
- Added a WAN DHCP callback regression asserting that the exact overlap branch records the error through `rc_first_service_start_status()`.
- Next: verify exact-head WAN DHCP, Process and miniupnpd checks; inspect XY-C1 firmware Build/TRX/artifact and NVRAM audit before continuing the WAN IPTV/PPPoE adjunct status audit.

---

## 2026-11-16 — Propagate static IPoE subnet conflict to WAN startup status

- Parent main: `68643ba87132dedc5945679d13e6b0e216861cdc`. Static IPoE WAN/LAN subnet overlap already set `wan_err` and blocked `wan_up()`, but failed to update `wan_start_status`. This could return success to PID 1 for a rejected WAN configuration. The branch now aggregates `EADDRINUSE` using first-error semantics.
- Extended the WAN DHCP callback workflow to assert that the exact overlap branch contains the `EADDRINUSE` aggregation, before static address application proceeds.
- The prior miniupnpd source-contract count was repaired to account for both PPP setup branches capturing migration status. Remaining next tasks are exact-head verification and the WAN IPTV/PPPoE adjunct startup status pass.
- No same-SHA firmware Build/TRX/artifact L3 or physical-router L4 claim is made.

---

## 2026-11-16 — Return configured IPTV and PPPoE passthrough startup failures

- Parent main: 168527e76b4ba78f45d2268ecf8992563d93fbf6. Review of the WAN startup tail found two configured adjunct services whose failures were logged but discarded: launch_viptv_wan() (IPTV VLAN/static/DHCP service setup) and set_passthrough_pppoe(1) (including PPPoE-relay launch/readiness or stale-relay stop failure when passthrough is disabled).
- Both statuses now enter wan_start_status using first-error semantics while allowing the core WAN/other services to continue. No required WAN core startup is aborted solely because the IPTV or passthrough adjunct fails; PID 1 now receives the first configured-service error.
- Added a GNU89 policy regression for preserving the first IPTV/passthrough failure and strengthened the shared WAN checker to bind both statuses to their production call sites.
- Parent d8d59 exact-head WAN route 43868343214, Process inventory/lifecycle 43868343273, HTTPD 43868343265, AI docs 43868343106, infosvr 43868343256, AdGuardHome 43868343268, AliyunDrive 43868343283 and AliDDNS 43868343245 passed. miniupnpd 43868343344 failed only at a stale call-count assertion (fixed in 168527). XY-C1 43868343338 was still building and NVRAM 43868343257 auditing.
- Next: verify the configured adjunct-service change with Process/XY-C1/WAN route and the miniupnpd call-count repair; check same-SHA Build/TRX/artifact before any L3 claim.

---

## 2026-11-16 — Correct miniupnpd migration call-count contract after PPP refactor

- Parent main: d8d59acfb40c8d439682d8764e872283bed4a23c. WAN route `43868343214`, Process inventory/lifecycle `43868343273`, HTTPD security `43868343265`, AI docs `43868343106`, infosvr `43868343256`, AdGuardHome `43868343268`, AliyunDrive `43868343283` and AliDDNS `43868343245` passed.
- The miniupnpd workflow reached its final source contract then failed because it expected one captured PPP interface-migration call in start_wan(). There are intentionally two: one for the USB RAS modem path and one for ordinary PPP. The IPoE migration path is a separate wan_ifname call. All three now use the same captured-status gate; the rollback helper retains its independent stop gate.
- Updated the workflow to require both PPP calls rather than weakening the count to an unbounded presence check. No production source change.
- XY-C1 `43868343338` was still building; TRX verification and artifact upload were pending. NVRAM `43868343257` was still auditing. No firmware L3 claim.
- Next: inspect this miniupnpd workflow repair plus XY-C1 Build/TRX/artifact and NVRAM exact-SHA checks; then continue a bounded WAN adjunct-service status pass.

---

## 2026-11-16 — Align WAN/PPP integration checks with the explicit startup status contract

- Parent main: ccca92b531d44c2476b6041a8adc3cbdec2f3a9f. Modem WAN policy and RC first-error tests compiled and passed under GNU89 + ASan/UBSan. The shared checker then failed because it expected raw interface_status, while the implementation converts negative ifconfig results to captured errno and returns failure_status.
- The WAN route workflow searched for a multi-line first-error C assignment as a single-line string. The miniupnpd and HTTPD workflows still expected pre-refactor inline migration/launch checks and boolean demand readiness.
- These are stale cross-module source assertions; no HTTPD or miniupnpd production source failed or was modified. The validator repair updates them to check captured status aggregation before fallback/cleanup.
- Exact-head AI docs, infosvr, VPN, WAN gateway ARP, detect_link, USB modem identity, WAN DHCP callback, DNSMasq, process inventory, detect_internet, AliDDNS, AdGuardHome, WAN teardown and AliyunDrive passed. NVRAM was in progress at last inspection.
- XY-C1 Build/TRX/artifact, WAN route, miniupnpd and HTTPD did not pass because of these contract mismatches; Process inventory failed at the shared WAN checker. No L3/L4 claim.
- Next: inspect exact-head repair Actions; continue WAN error-return review after the validators are green.

---

## 2026-11-16 — Propagate required PPP and IPoE startup failures to PID 1

- Parent main: `48ff82881c7e83c29ab64a5285ee9d2bf8be6955`. Targeted review of `start_wan()` found required PPP/IPoE transitions that aborted startup or set a per-WAN error flag without entering the status returned to PID 1: PPP/IPoE UPnP migration stop, PPTP/L2TP physical setup, pppd launch, demand PPP readiness, static IPv4 assignment, EAPOL startup and DHCP client launch.
- Each required failure now enters the first-error WAN accumulator before `continue` or best-effort recovery. PPPoE's intentional best-effort physical setup behavior and optional MTU/UPnP notification behavior are unchanged.
- `wait_ppp_up()` now returns zero only for readiness success and retains the underlying interface, socket or route error for the caller.
- Added GNU89 policy regressions for PPP/IPoE first-error retention and source-contract coverage for each mandatory operation; updated WAN route workflow demand-readiness assertions to the explicit status contract.
- Parent `48ff8288` AI docs, NVRAM, AliDDNS, infosvr, AliyunDrive, AdGuardHome, Process inventory and WAN DHCP callback runs were queued at last inspection; no exact-head source tests or firmware L3 are claimed yet.
- Next: inspect exact-head Process, XY-C1, WAN route/DHCP and AI docs; continue checking required callback/lease failure propagation. No physical-router L4 claim.

---

## 2026-11-16 — Align WAN DHCP callback contract with the static MAN address status variable

- Parent main: `3c7a5a125f670335cb1b305674790eece80b1dcc`. WAN route operation regression `43866768822` and AI handoff docs `43866769102` passed. WAN DHCP callback `43866769025` passed all six native GNU89 regression binaries, then failed in its final source-contract step because it still expected `launch_status = ifconfig(...)`. The MAN startup fix intentionally records the direct address result in `address_status` before aggregating route and gateway failures.
- Updated the WAN DHCP workflow to assert the current `address_status` contract and verify its order before MAN route setup. Product source is unchanged.
- Next: re-run exact-head WAN DHCP callback and then continue the required PPP/IPoE start-status propagation slice. No L3 claim.

---

## 2026-11-16 — Align WAN route regressions with first-error MAN status accumulation

- Parent main: `acf73fd0fa217560ed670beb2d92bdd0d0a4a924`. The MAN route source fix correctly replaced `launch_status = gateway_route_status` with first-error `route_status_accumulate()` for both the complete MAN route result and the separate gateway-route result. The WAN route and WAN DHCP callback workflows still asserted the old overwrite assignment, so exact-head runs `43866344779` and `43866344884` failed at those stale expectations; no product source/build failure was indicated.
- Updated both workflow source contracts to require preservation of the first failure from both route results. The source change is retained unchanged.
- On parent acf73: AI docs `43866344868`, infosvr `43866344724`, USB modem identity `43866344763`, VPN `43866344749`, miniupnpd `43866345059`, WAN gateway ARP `43866344761`, detect_internet `43866344886`, WAN teardown `43866344963`, AdGuardHome `43866344879`, Process inventory `43866344891`, DNSMasq `43866344739`, detect_link `43866344715`, AliDDNS `43866344897` passed. Process `43866345006`, XY-C1 `43866344767`, NVRAM `43866344923` and HTTPD `43866344906` remained in progress at last inspection.
- Next: verify both corrected WAN regression workflows on the exact repair SHA; continue the required PPP/IPoE status propagation review without claiming L3 until Build/TRX/artifact pass on one SHA.

---

## 2026-11-16 — Propagate manual WAN route and physical IPv6 startup failures

- Parent main: `06bfa191302f0d53567a1cdb10652913b93cd92a`. `launch_wanx()` previously discarded a failed `man_up_internal()` static MAN route result unless the separate gateway-route out parameter also failed. It now preserves first-error status across static address setup, complete MAN route setup, gateway route and configured physical IPv6 tunnel setup, preventing false-success WAN bring-up.
- Added source-contract assertions in the WAN route workflow for the MAN address, full route, gateway and physical IPv6 status paths.
- Parent AI docs `43866012138`, infosvr `43866012145`, AdGuardHome `43866012153`, AliyunDrive `43866012166` and AliDDNS `43866012103`: PASS. Process `43866012142`, XY-C1 `43866012127` and NVRAM `43866012113` were still running at last inspection; no L3/L4 claim.
- The immediately preceding main commit fixed the IPv6 source-checker gate ranges and rotated the 120-record root log; this entry keeps the root rolling log at 22 complete records.
- Next: check exact-head Process, XY-C1, WAN route and AI docs; XY-C1 L3 still requires Build, TRX verification and artifact upload on the same source SHA.

---

## 2026-11-16 — Correct WAN IPv6 contract scope and rotate the rolling development log

- Parent main: `de6799e468caa60bfcd086b263e82bb149efd362`. Exact-head Process `43864900710` and XY-C1 `43864901114` failed at the new WAN source checker because its slices started at the `control_if_ipv6()` call and excluded the enclosing runtime `get_ipv6_type() != IPV6_DISABLED` guard. Both GNU89 C policy binaries passed before the checker failed; XY-C1 Build, TRX verification and artifact upload were skipped. This was a checker-scope false positive, not a product compile failure.
- The checker now captures each complete IPv6 guard, control call, required error return and interface publication boundary.
- AI docs run `43864900749` failed because root `dev_log.md` reached 120 dated entries. This commit preserves the oldest 100 complete entries byte-for-byte and oldest-first in `docs/dev-log/2026/11/part-012.md`, retaining the newest 20 existing records in root plus this correction record (21 total). The archive index is updated with the exact UTF-8 byte count and Git blob SHA.
- PR #1 was reviewed as a closed, unmerged historical branch. The HMAC helper source/Makefile, HMAC test, CA bootstrap test and transport fixture already match main; the live package, launcher, workflow and semantic checker have evolved but retain the HMAC and synchronous CA trust-store contracts. No stale PR branch commits were cherry-picked.
- Next: verify this correction's exact-head Process, AI docs and XY-C1 Actions; do not claim firmware L3 until same-SHA Build, TRX verification and artifact upload all pass.

---

## 2026-11-16 — Propagate required IPv6 control failures during WAN interface publication

- Parent main: `cf45552d54488f6d64cc3d2c1118ba9385a0ea7b`. A targeted WAN call-site audit found `config_vinet_wan()` ignored the result of disabling IPv6 on the physical VLAN parent and enabling it on the selected WAN interface; `config_apcli_wisp()` likewise published the AP-client interface after discarding its IPv6 enable result.
- These controls are required whenever IPv6 WAN mode is active: an incomplete parent disable can leave IPv6 on the wrong VLAN device, while failed enable can publish an interface whose IPv6 stack is not ready. The code now checks each return, logs the exact interface/status, aborts the migration before publishing `ifname`, and skips unnecessary parent-disable work when IPv6 is globally disabled.
- Strengthened `scripts/check_modem_wan_start_status.py` with semantic source contracts for all three checked operations and publication/ordering gates. The existing Process and XY-C1 workflows already run this checker.
- Parent exact-head Process `43864471819` SUCCESS. XY-C1 `43864471994` remained IN_PROGRESS in `Build XY-C1`; TRX and artifact steps were still pending. The new source change needs its own exact-head Process/XY-C1 validation; no L3/L4 claim.
- Next: inspect the exact-head Actions for both WAN slices, then continue tracing unchecked PPP, static-address and DHCP launch results. Root `dev_log.md` will contain 119 complete records after this entry, below its 120-record rotation threshold.

---

## 2026-11-16 — Propagate physical WAN interface bring-up failure to PID 1

- Parent main: `f16026b0af5d00087e36929b14843cf6b94a8b6a`. `start_wan()` checked and logged a failed physical WAN `ifconfig(..., IFUP, ...)`, attempted cleanup and continued, but never merged that required interface-readiness failure into its return status. PID 1 could therefore observe a successful WAN start when the physical interface was unavailable.
- The failure now enters the first-error WAN startup accumulator before best-effort interface-down cleanup. The original IFUP error is retained even if cleanup also fails.
- Added a GNU89 policy regression for first-error preservation and strengthened `check_modem_wan_start_status.py` to require the production IFUP failure path to aggregate before cleanup/continue. Both Process and XY-C1 workflows already compile/run this test and checker; no workflow expansion was necessary.
- Parent exact-head Process `43863800540`, AI docs `43863800477`, infosvr `43863800722`, NVRAM `43863800497`, AliDDNS `43863800501`, AliyunDrive `43863800566` and AdGuardHome `43863800941` passed. Parent XY-C1 `43863800656` was still running; this source change requires fresh exact-head verification. No L3 or L4 claim.
- Next: verify this commit's Process and XY-C1 Actions; continue the remaining WAN failure-propagation review, especially PPP launch/readiness and ignored IPv6 interface-control results.

---

## 2026-11-16 — Repair WAN checker formatting and canonical handoff headings

- Parent main: `5825e35e0987fab26b4f588f879a885094e1708b`. Process `43862972148` passed compiled host policy binaries then failed at whitespace-sensitive `wan_start_status` assertions and an obsolete expectation of `return modem_start_status;`. Updated the checker to accept formatting across helper-call line breaks and assert the combined modem/route return.
- AI docs `43862972023` and infosvr `43862972198` both failed because `SESSION_HANDOFF.md` renamed canonical `## Current repository state` / `## Active task` headings. Restored both names so existing validators and cross-repository consumers keep one stable handoff schema.
- WAN route status `43862971906` and WAN interface teardown `43862972252` passed on the parent SHA. AdGuardHome, AliyunDrive, AliDDNS, detect_internet, detect_link, DNSMasq, WAN gateway ARP, miniupnpd, VPN server, USB modem identity and HTTPD security also passed.
- NVRAM `43862972028` remained in progress. XY-C1 `43862972095` stopped before firmware Build because the pre-build source-contract step failed; TRX/artifact were skipped.
- Next: check exact-head Process/AI docs/infosvr reruns, then audit ignored MTU/interface/IPv6 configuration return values.

---

## 2026-11-16 — Repair exact-head LAN and handoff CI contract failures

- Parent main: `90ac78ccccb7acbf6ab75956999b4847a3628fd8`; product-source baseline under validation: `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`.
- Exact-head Process regression `43859397503` reached the final LAN startup source-contract step after process inventory, GNU89/Werror/ASan/UBSan and policy regressions passed, then failed at the broad raw LAN member-call assertion. Tightened the contract to require exactly two router/AP LAN member calls and require each to assign `operation_status`; the existing source assertions continue to require both first-error records.
- Exact-head AI handoff docs `43859397894` failed because `SESSION_HANDOFF.md` omitted the required end-of-session accuracy-checkpoint statement. Restored the exact validator-required wording.
- XY-C1 `43859397498` was still building the firmware; TRX verification and artifact upload were pending. NVRAM `43859397571` was still tracing raw ioctl callers. Other exact-head module Actions were passing as listed in the handoff.
- This is a workflow/governance repair only; no firmware C source changed. No L3/L4 claim is made.
- Next: verify exact-head Process/AI docs reruns, inspect XY-C1/NVRAM results, and continue the RC initialization audit for unchecked required startup operations.

---

## 2026-11-16 — Propagate WAN route setup and teardown results

- Parent main: `066697195b12b3bbc8bad1efa4a4c4abea5052fa`. Call-site audit found static IPoE `start_wan()` ignored the int result of `wan_up()`, which reports default/static WAN route and related session setup failures. It now aggregates this result into WAN startup status rather than returning success on an incomplete route setup.
- Configured IPv6 SIT tunnel startup failure is now also included in WAN startup status while the code continues attempting the independent remaining setup.
- `stop_wan_locked()` ignored `wan_down()` route teardown result and then reset `stop_status` to zero. It now preserves the route teardown failure and merges USB modem stop result before physical interface teardown; the existing final fail-closed state-clear gate now sees either error.
- Expanded `scripts/check_modem_wan_start_status.py` to assert the WAN route/IPv6 start results, static route teardown result and modem-stop aggregation.
- Parent exact-head Process `43862663789`, AI docs `43862663478`, XY-C1 `43862663471`, NVRAM `43862663293`, AdGuardHome `43862663396`, infosvr `43862663310`, AliyunDrive `43862663359` and AliDDNS `43862663392` were pending at last query. The previous Process reached a stale raw-return assertion in the modem WAN checker after compiled policy tests; the checker was corrected in `066697195b12b3bbc8bad1efa4a4c4abea5052fa`. This source slice requires new exact-head CI; no L3/L4 claim.
- Next: validate all exact-head checks and continue the remaining WAN MTU/address result audit.

---

## 2026-11-16 — Update modem WAN checker for merged L2TP start status

- Parent main: `e97f95dc097e7faa8cd00aa491de9f1fb4ad519a`. Exact-head Process run `43862009119`, job `133280916393`, completed all compiled inventory/policy regressions and the modem WAN policy binary successfully, then failed in `scripts/check_modem_wan_start_status.py` because its old contract expected `return status;` from full WAN restart, reconnect and deferred reconnect.
- This expectation became stale after `merge_l2tp_server_start_status()` was introduced to preserve negative L2TP server startup results without treating positive already-running as failure.
- Updated the checker to assert the helper's status semantics and require each of the three WAN start callers to return `merge_l2tp_server_start_status(status)`. Product WAN C source unchanged in this checker fix.
- Exact-head rerun is required; no same-SHA XY-C1 firmware/TRX/artifact L3 or physical-router L4 result is claimed.
- Next: verify Process/AI docs and current XY-C1/security Actions; continue the call-site audit of WAN startup/teardown and interface MTU return values.

---

## 2026-11-16 — Correct bridge-member errno assertion close ordering

- Parent main: `73940a656e2c62e7d7d172b1ef4c622e07715ca4`. Process run `43860996821` on `43d0ebe973f06f578282531eed4abc0fdba1bb3b`, job `133277465787`, failed at the bridge-member errno source assertion after all compiled policy tests passed.
- Root cause: `br_add_del_if_internal()` has an earlier `close(sockfd)` on the SIOCGIFINDEX error path; the checker used `.index("close(sockfd);")`, accidentally comparing errno save to that earlier close. The actual post-SIOCBRADDIF/DELIF close is the final close in the helper. Changed the assertion to compare against `.rindex("close(sockfd);")`.
- No product source behavior changed in this contract repair. Exact-head Process/AI documentation results for the combined WAN restart work remain pending; verify this checker correction on its own SHA.

---

## 2026-11-16 — Check WAN restart MAC table and L2TP service results

- Parent main: `01d9147b742391933a22781b42eb1213063cb1c1`. Further `full_restart_wan()` tracing found ignored int-returning operations: `phy_clear_mac_table()` and `safe_start_xl2tpd()`. The former is an ioctl on MTK ESW and RTL8367; the latter returns negative on lock/process/config/start failure, zero on launch success, and positive one when the shared daemon is already running.
- `full_restart_wan()` now fails before WAN startup if the switch MAC table cannot be cleared. New `merge_l2tp_server_start_status()` preserves WAN-start failure, treats positive already-running as success, and surfaces configured L2TP server launch failure when WAN start succeeded.
- Applied L2TP result aggregation in full restart, normal reconnect and deferred reconnect; none of the three connect paths now silently discards a negative L2TP server start status.
- Process source contract checks MAC clear ordering and the normalized L2TP result across all three call paths. Exact-parent Process `43861241910`, AI docs `43861242056`, NVRAM `43861243064` and XY-C1 `43861242089` were pending/queued; this slice requires fresh exact-head checks. No L3/L4 claim.
- Next: validate exact-head Process/WAN route and inspect remaining int-returning WAN/RC callsites.

---

## 2026-11-16 — Restore static LAN routes before restarting WAN

- Parent main: `43d0ebe973f06f578282531eed4abc0fdba1bb3b`. `full_restart_wan()` had already been updated to stop before WAN startup when switch VLAN reconfiguration failed, but it still discarded the int result from `add_static_lan_routes(IFNAME_BR)`.
- The helper delegates to `control_static_routes()` and reports an error if the required static LAN routes are not restored after WAN teardown. The full restart now captures that result, logs the failure and returns before `start_wan()`; an incomplete route table is not treated as a successful WAN restart.
- Process semantic source contract now asserts route restoration is checked and the failure return precedes WAN startup.
- The previous exact-head Process run on `90ac78ccccb7acbf6ab75956999b4847a3628fd8` failed only at its final static source-contract assertion; commit `43d0ebe973f06f578282531eed4abc0fdba1bb3b` corrected the checker to require exactly two assigned LAN-member calls. Exact rerun Process `43860996821` and AI docs `43860997587` were still pending at last query.
- THIS source slice needs fresh exact-head CI. No same-source-SHA firmware L3 or physical-router L4 result is claimed.
- Next: verify exact Process/AI docs and inspect any additional int-returning `full_restart_wan()` callers before proceeding.

---

## 2026-11-16 — Preserve bridge membership ioctl errno across close

- Parent main: `1ce4ffe3b71e87bdc5143ccc802d252f190ab5b0`. The preceding commit preserves ioctl errno for `br_add_del_bridge()`; a sibling review found the same issue in `br_add_del_if_internal()` for bridge port add/delete.
- If SIOCBRADDIF/SIOCBRDELIF failed with an error other than a subsequently-confirmed idempotent race, the helper called `close(sockfd)` and returned `-1` without restoring ioctl errno. Now save errno immediately after ioctl and restore it after close for any still-failed operation. Successful concurrent idempotency re-probes remain success.
- Process source contract now checks the production bridge membership helper as well as bridge creation for save-before-close and restore-on-failure behavior.
- Exact parent Actions Process `43860541389` and AI docs `43860541304` were pending; the preceding SHA `7ce9978...` Process `43860044486` and AI docs `43860044408` passed. This change needs exact-head verification; no L3/L4 claim.
- Next: inspect exact-head Process/AI docs and prior XY-C1 `43860044821`, then continue focused ioctl errno/callsite checks.

---

## 2026-11-16 — Preserve bridge ioctl errno across socket close

- Parent main: `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`. The bridge-add idempotency wrapper distinguishes EEXIST from other bridge-add failures before probing kernel bridge membership. The shared `br_add_del_bridge()` helper did not preserve the ioctl errno across its subsequent `close(sockfd)`.
- Save errno immediately after ioctl and restore it after close on ioctl failure, so bridge-init classification always sees the actual bridge operation result rather than a later close-side errno.
- Extended Process source contract to assert save-before-close and restore-on-failure ordering. Existing bridge initialization policy regression continues to cover EEXIST and fail-closed unknown inventory.
- On the parent SHA, Process `43860044486` and AI docs `43860044408` passed; XY-C1 `43860044821` and NVRAM `43860044318` were still in progress. This source slice requires fresh exact-head Actions. No same-SHA firmware L3 or router L4 claim.
- Next: validate exact-head Process/AI docs and check whether previous XY-C1/NVRAM runs completed; continue focused network utility caller audit.

---

## 2026-11-16 — Fail closed on switch VLAN reconfiguration during WAN restart

- Parent main: `0828b3a8acf119345bc2381f89772f213288862d`. Call-site review found `full_restart_wan()` still ignored the newly status-returning `switch_config_vlan(0)` and continued to `start_wan()` regardless of VLAN ioctl failures.
- `full_restart_wan()` now captures switch VLAN configuration status, logs the failure and returns before WAN startup if VLAN rules or switch bridge mode could not be configured. This avoids publishing a WAN session on unverified switch VLAN state after the old WAN has been stopped.
- Process source checker now loads `net_wan.c` and verifies the failure return occurs before `start_wan()`; workflow path filters include this file.
- Exact-head Process and AI docs results for `0828b3a8acf119345bc2381f89772f213288862d` were pending at last inspection after correcting the previous run's false-positive source assertion and handoff wording. THIS fix requires new exact-head Actions; no firmware L3 or physical L4 claim.
- Next: inspect exact Actions, then continue searching callers of recently converted status-returning switch/bridge APIs.

---

## 2026-11-16 — Correct exact-head Process and handoff contracts

- Parent main: `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`. Exact-head Process run `43858811934`, job `133269937845`, failed only at the new negative bridge-member source assertion after all compiled policy regressions (including bridge, wireless-module post-load and bridge membership) had passed. The check used substring containment and falsely matched the unchecked-call text inside the assignment form; changed it to compare complete stripped source lines.
- Exact-head AI handoff docs run `43858811955`, job `133269937387`, failed because the handoff lacked the validator-required literal `Before context exhaustion`. Restored the full mandatory end-of-session live-state checkpoint and explicit pre-exhaustion instruction.
- These are test/governance contract fixes, not new product-source changes. Exact reruns on THIS commit are required; no build or hardware outcome is claimed.

---

## 2026-11-16 — Verify required and optional LAN bridge modules after load

- Parent main: `59047eb21838e0cf78dd24e0a6eab78bea7fc886`. The existing `rc_module_load_verified()` policy explicitly documents that `module_smart_load()` can return zero after a failed `modprobe`; post-load `is_module_loaded()` state is authoritative. Its regression is already part of Process CI.
- Found ignored module load calls in `init_bridge()`: required `iNIC_mii` on `USE_RT3352_MII` boards and optional `hw_nat` bridge offload. The former could fail silently and leave an incomplete iNIC LAN path; the latter was logged as enabled unconditionally even when it had not loaded.
- Added a LAN wrapper using `rc_module_load_verified()`. Required iNIC load status now joins bridge/PID1 startup error aggregation. Optional hardware NAT offload is verified and reports enabled/unavailable, but never makes bridge startup fail.
- Process source contracts now classify both paths; the existing verified module loader unit test remains in the Process workflow. Exact-head CI is pending after publication; no firmware L3 or router L4 claim.
- Next: inspect current exact-head Process/XY-C1 and prior pending docs/security/NVRAM runs, then continue the remaining RC boot operations audit.

---

## 2026-11-16 — Restore mandatory handoff checkpoint contract

- Parent main: `2c0a0f1be6edf5a604a71ce02e726d0b2393a00c`. Exact-head AI handoff documentation run `43858670960`, job `133269446982`, failed because `SESSION_HANDOFF.md` no longer carried the validator-required explicit end-of-session accuracy-checkpoint wording.
- Restored the mandatory live-state/source-baseline/Actions/first-action comparison wording and the conditional-document-edit anti-churn rule in the canonical handoff. No source behavior changed.
- Re-run the AI handoff documentation contract on this exact commit; the prior failure is a real governance-check blocker, not a source-test failure.

---

## 2026-11-16 — Propagate switch base, storm and link configuration errors

- Parent main: `4322737edbbe77196e4380fc7506da3f0851dc0d`. The previous GMAC2 router conditional and AP-tagged LAN member accounting correction is in `4322737edbbe77196e4380fc7506da3f0851dc0d`; exact-head Actions were pending at last inspection.
- Further source tracing showed `switch_config_base()`, `switch_config_storm()`, and `switch_config_link()` discarded all return-bearing PHY ioctl results. The switch API returns int on MTK ESW and RTL8367; MTK ESW unsupported optional storm/green/EEE capabilities explicitly return success/no-op.
- Converted the three helpers to return first-error status while still attempting each independent setting. PID1 bridge startup and the switch-config notification now aggregate those results. LED indication remains explicitly best-effort and is not part of the forwarding status.
- Process semantic source assertions check all base settings, four storm controls, WAN plus each LAN link setting, and PID1/notification aggregation. Exact-head tests are pending after publication; no firmware L3 or physical router L4 result is claimed.
- Next: inspect exact-head Process, XY-C1, storage, NVRAM, docs and HTTPD/security Actions; then continue evidence-backed switch/PHY or other userland reliability fixes.

---

## 2026-11-16 — Correct conditional LAN bridge member status accounting

- Parent main: `aa5d3c9f2566927a4b17fce5d391f525cccc8095`. Exact-head Process run `43857640159` on that parent passed. Job `133265909696` confirms the bridge-init policy binary, multicast policy binary, bridge-membership regression and LAN production source contract all passed.
- A post-commit review found the router GMAC2 conditional `else` path recorded its status outside the conditional block, which could record the preceding eth2.1 attach twice; AP tagged mode also discarded its LAN member attach result.
- Scoped the router fallback attach and status record inside the preprocessor-controlled else block, and added status capture for the AP tagged LAN port. Strengthened the Process source contract to require both branches and reject the unchecked bare call.
- All unrelated exact-SHA checks listed in the preceding entry were passing; NVRAM `43857640139`, storage `43857640310`, HTTPD authentication `43857640193`, AI docs `43857640173`, VPN `43857640155`, and XY-C1 `43857640160` were still running at that inspection. No L3/L4 claim.
- Exact-head checks for THIS correction are pending. Next: inspect Process and the remaining same-SHA checks, then continue switch base/storm/link coverage.

---

## 2026-11-16 — Propagate essential bridge/VLAN initialization status

- Parent main: `9f71cebf2e323d8dc84bca7cfd312d027ca5aab1`. This slice covers required Ethernet/bridge/VLAN initialization; the existing `config_bridge()` multicast policy remains a distinct optional-feature path.
- Found `init_bridge()` discarded results from switch bridge/VLAN setup, MAC interface setup, switch PHY power, VLAN creation, bridge creation/configuration, member attachment and final bridge IFUP. `switch_config_vlan()` also discarded every VLAN rule and bridge-mode ioctl result.
- Added a separate bridge-init first-error policy; all independent VLAN and bridge operations are attempted. Duplicate `br_add_del_bridge()` EEXIST is treated as idempotent only after a successful kernel bridge inventory probe. Existing bridge-port attachment and VLAN helper operations retain their fail-closed/idempotent semantics.
- `init_bridge()` now returns status to PID 1, `switch_config_vlan()` and its restart notification propagate results, and switch configuration notifications preserve `config_bridge()` status. Optional iNIC isolation errors are diagnostic only; multicast controls remain distinct.
- GNU89/Werror/ASan/UBSan bridge-init policy regression passed locally. Process workflow now compiles this regression and the existing bridge-membership policy regression, and checks production definitions/callers/ordering. Exact-head CI is pending after publication; no firmware L3 or hardware L4 result is claimed.
- Next: inspect exact-head Process, XY-C1, storage and NVRAM runs; then continue independent RC status coverage around switch base/storm/link setup.

---

## 2026-11-16 — Gate loopback route setup on successful IFUP

- Parent main: 1f232209a9aa41d520939c18ab783609e9b791b0. Process run 43852281303 failed only at the new loopback source return-type assertion after preceding lifecycle and bridge regressions passed.
- Review found the first init_loopback() status patch attempted route_add() even when ifconfig("lo", ...) failed. Since route installation depends on a configured loopback address, init_loopback() now returns the IFUP error immediately, and only attempts the route after successful address setup. Route-add failures are logged and returned.
- Corrected the Process source contract to use a newline-safe return-type assertion and verify the early-return ordering ahead of route_add().
- XY-C1 43852281447 was still IN_PROGRESS at ccache setup at the last inspection; storage 43852281602 and NVRAM 43852281469 were also pending then. No L3 or L4 result is claimed in this record.
- Next: inspect exact-head Process, XY-C1 Build/TRX/artifact, storage and NVRAM; then audit init_bridge()/switch_config_vlan() required versus optional status handling.

---

## 2026-11-16 — Propagate loopback interface and route startup failures

- Parent main: `b85754180a56755c97a03bb5ab3013b30561f48c`. USB storage hotplug `43851311951` and NVRAM transaction boundary `43851311965` passed; storage included BusyBox 1.24.2/ash lifecycle, lock/eject and persistence regressions. Process inventory `43851311871`, AI docs `43851312213`, AdGuardHome `43851311939`, infosvr `43851311935`, AliDDNS `43851311972`, AliyunDrive `43851311982` passed.
- Found that PID 1 `init_loopback()` silently discarded both loopback `ifconfig()` and route-add failures because it returned `void`.
- Converted it to return first-error status while still attempting route setup after an interface failure; `init_router()` now merges and logs the result in `boot_services_status`. Added GNU89 first-error regression and a Process source contract for definition, operation order, both status aggregations and caller wiring.
- XY-C1 `43850461476` on ancestor production SHA `b81207777615d31b096c1425ff1fa8110898b0c2` was still at Build XY-C1 last checked; no TRX/artifact L3 or physical-router L4 result is claimed.
- Exact-head Actions for THIS commit pending after publication.
- Next: validate Process loopback source/test contract, then implement capability-aware essential bridge/VLAN initialization status propagation without promoting optional multicast features to fatal boot errors.

---

