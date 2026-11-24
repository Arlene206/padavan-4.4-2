## 2026-11-23 — Fix AiDisk checker escape semantics

- Exact eae527b HTTPD run #45302091248 exposed a false-negative in the new source-contract check: Python interpreted the C source's two-character backslash-n sequence as an actual newline. The preceding JSON/HTML sanitizer tests, JS NVRAM checks, dynamic hook checks and WOL DOM checks had passed.
- Changed the two helper assertions to Python raw strings so they compare literal C `\\n` text. This is a test-only follow-up; all 48 AiDisk dictionary message callsites remain routed through write_script_message().
- Exact eae527b AI handoff #45302091049 and Process lifecycle #45302091220 passed; NVRAM audit #45302091118 and XY-C1 #45302091120 were still running. No child-SHA result claimed.

---
## 2026-11-23 — Fix AiDisk dynamic JavaScript error-message output

- Found 48 AiDisk action-error callsites that formatted translated dictionary messages directly into JavaScript source. The legacy formats did not protect backslashes, quotes, controls, script terminators or Unicode line separators; several also emitted escaped apostrophes outside a JavaScript string literal.
- Reused httpd_json_output_policy.h for AiDisk's JSON/JavaScript string writer and routed every dictionary-backed error callback through write_script_message(). This emits a complete double-quoted JavaScript string and prevents message text from breaking out of the script element.
- Added source-contract coverage for all 48 messages and the shared encoder. Parent a5794a0 WOL DOM XSS and related Actions were IN_PROGRESS at checkpoint; no result for this new SHA is claimed.

---
## 2026-11-23 — Render WOL client and vendor data as text

- The WOL device table concatenated static/DHCP client MACs and names into HTML, while vendor labels from the external MAC lookup API and localStorage were inserted with `.html()`. Client labels and third-party vendor strings are untrusted; either path could inject active markup into the authenticated router UI.
- Rebuilt table rows with jQuery DOM elements and `.text()` for MAC, name and vendor cells. Normalize the API vendor field to a string before displaying and caching it. Added `check_wol_dom_sinks()` to the existing HTTPD output-context regression so raw row concatenation and vendor HTML sinks fail CI.
- Exact parent `fbbedd7`: HTTPD `#45301473424` and AI handoff `#45301473510` PASS. The earlier `fd68cee` HTTPD failure `#45301344235` was the malformed checker boundary (fixed and revalidated on `fbbedd7`); its XY-C1 run `#45301344269` was still in progress at last check. This UI/source-contract slice awaits exact-head Actions; no local test, L3 or L4 result is claimed.

---

## 2026-11-23 — Correct checker source-index escapes for WOL JSON contract

- The WOL JSON migration added source-range assertions to the HTTPD checker. Inspection found its source.index() needles contained doubled backslashes and would search for literal backslash-n rather than actual line boundaries.
- Corrected both function-boundary needles to use Python newline escapes. This is a regression-test-only follow-up; the source implementation from fd68cee remains unchanged.
- Parent fd68cee exact-SHA Actions were queued/in progress; no result for this follow-up is claimed.

---
## 2026-11-23 — Use the shared JSON encoder for the WOL response

- `wol_action_hook()` already enforces a strict 12/17-character MAC allowlist before waking a client. Replaced its manual JSON `%s` interpolation with `write_json_string()` as defense in depth and moved the function prototype to the common declaration block.
- Extended the HTTPD source contract to require the shared JSON encoder and preserve strict hex/colon input validation. This keeps the endpoint's JSON response safe even if future MAC validation is changed.
- Parent exact `89128ba`: HTTPD `#45300665513`, AI `#45300665430`, Process inventory `#45300665463`, lifecycle inventory `#45300665517`, NVRAM audit `#45300665431`, FRP `#45300665527`, Inadyn `#45300665514`, AdGuardHome `#45300665471`, USB storage `#45300665447` and AliyunDrive `#45300665429` all passed. XY-C1 `#45300665497` detector passed but Build was still running; TRX/artifact pending. No result for the child SHA is claimed.

---

## 2026-11-23 — Escape remaining status and action-mode JavaScript strings

- Routed the static LAN fallback `"No link"` through `write_js_string()` as well, then removed the checker exception and required that no raw `return '%s'` emitter remains in `web_ex.c`.
- Converted both fixed validation action-mode strings from direct double-quoted JavaScript interpolation to `write_js_string()`. This centralizes output escaping for dynamic C-side script strings and preserves the same runtime string values.
- Parent `c6f5805` HTTPD run `#45300471389` was pending; exact follow-up SHA Actions unverified.

---

## 2026-11-23 — Narrow dynamic-hook regression assertion

- Tightened the new C-side JavaScript hook contract check to reject the dynamic LAN port-status formatter specifically while allowing the literal `"No link"` fallback. This avoids a false positive from a safe source-controlled string without weakening coverage of runtime `port_status` escaping.
- Parent `dd099f6` Actions had started; exact child-SHA results pending. Backup remains untouched.

---

## 2026-11-23 — Escape dynamic HTTPD status-hook strings for JavaScript

- Audited C-side JavaScript emitters in `web_ex.c`. WAN status strings (link state, connection type, IP/gateway fields, DNS and MAC), per-port LAN status, captive-client version and route/connection counters were interpolated directly between single quotes with `websWrite("%s")`. Routed these values through the existing tested `write_js_string()` encoder. Numeric fields remain numeric; existing DNS `<br/>` display markup is preserved after JavaScript parsing.
- Added source-contract checks to the HTTPD template-context checker so those runtime strings cannot regress to raw quoted `%s` output.
- Parent `08c6d43` exact HTTPD `#45299995452` and AI handoff `#45299995515` passed; XY-C1 `#45299995388` was in progress with Build pending. Exact new SHA is unverified until its Actions complete.

---

## 2026-11-24 — Fix JSON list newline escaping and remaining legacy JavaScript getter

- The 174-asset context scan identified sixteen JavaScript `get_nvram_list` array insertions. They flowed through `write_json_string()`, but its newline case emitted a literal LF inside a quoted JSON string, making the output invalid JavaScript/JSON. Added a bounded GNU89 encoder that escapes JSON controls, quotes, backslashes, HTML script delimiters and U+2028/U+2029 while preserving UTF-8.
- Migrated the remaining `validator.js` `nvram_get("rc_support")` sink to the complete-literal `nvram_get_js("", "rc_support")` callback. Extended the template checker to reject raw `nvram_get`, list, buffer and table getters in JavaScript and to verify the JSON-list path uses the encoder.
- Added an ASan/UBSan byte-level regression and wired it into HTTPD push/PR CI. Parent exact `212aed2` XY-C1 detector regression `#45299517972` passed; Build XY-C1 was in progress, with TRX/artifact pending. AI handoff `#45299517974` and Process inventory `#45299517987` passed. No result for this new SHA is claimed.

---

## 2026-11-24 — Restore XY-C1 detector-test workflow trigger

- The `5c8c62f` workflow cleanup accidentally removed `scripts/tests/test_xy_c1_firmware_inputs.py` from XY-C1 `push.paths`, even though the workflow runs this test. A test-only edit could therefore bypass its own regression job.
- Restored the test path and added a regression asserting it appears exactly once in the workflow filter while `is_non_firmware_input()` still classifies it as host-only. Expected behavior: run the detector tests, skip Build/TRX/artifact for test-only changes.
- Parent main `861e3fe19d903541717f010f9ab636db8894f487`: HTTPD `#45298769831`, AI handoff `#45298769865`, Process inventory `#45298769872`, Process lifecycle `#45298769967` and NVRAM transaction `#45298769873` PASS. XY-C1 `#45298770098` was still building; no L3 result claimed.

---

## 2026-11-24 — Complete HTTPD NVRAM HTML/JavaScript output-context isolation

- Parent main SHA: `5c8c62f51b722ebeda9cbce811dd571392832a5a`. Continued the previous seven-page JS-string migration (`51bd80e`) and four-page follow-up (`453aefd`): migrated the remaining 89 executable JavaScript-source `nvram_get_x` template sinks across 27 ASP/JS assets, bringing the verified total to 117. All three `nvram_get_ddns` script sinks now emit complete JS string literals while retaining their DDNS reset behavior.
- Hardened generic HTML `nvram_get_x` output to encode apostrophes and controls alongside ampersand/angle/quote syntax, protecting single-quoted attributes without altering unrelated HTML contexts.
- Added a GNU89 ASan/UBSan byte-level encoder regression and a repository-wide ASP/JS/HTM/HTML context checker for comments, quoting, callback registration and DDNS delegation. HTTPD path filters now include the complete UI subtree and run both checks on push/pull request.
- Parent exact verification: `453aefd` HTTPD `#45297345701`, Process inventory `#45297345736`, AI handoff `#45297345889` PASS; XY detector regression `a2310ad` `#45297887131` PASS with firmware Build/TRX/artifact skipped (test-only); `5c8c62f` XY build `#45298161504` was IN_PROGRESS, AI `#45298161525` and Process `#45298161485` PASS. This production source/UI SHA needs fresh exact-head Actions; no hardware result claimed.

---

## 2026-11-24 — Correct XY-C1 web-asset path test fixture

- Exact main `453aefd46d577206fc606fc0f58b9f51a4b2d15b` HTTPD regression `#45297345701`, Process inventory `#45297345736` and AI handoff contract `#45297345889` passed. XY-C1 run `#45297345772` stopped in `test_web_ui_asset_push_triggers_firmware_build` before firmware compilation.
- Root cause was in the new test fixture, not the path detector: Python test source used double-escaped newline and NUL separators (`"\\n"`, `"\\0"`) so the mocked `rev-parse HEAD` and `git diff -z` responses were not the protocol responses consumed by the detector. Replaced them with actual Python newline/NUL escapes; production fail-closed behavior remains unchanged.
- This atomic correction updates only the regression fixture plus the handoff/log. Exact-SHA rerun must confirm the detector test now passes and the XY-C1 build, TRX check and artifact upload actually run. No firmware/hardware result is claimed.

---

## 2026-11-23 — Correct handoff byte limit and refresh verification

- The handoff CI on 58cbb67 failed because REFACTOR_ROADMAP.md was 131,598 UTF-8 bytes, 526 bytes above the 128 KiB limit. Replaced its redundant leading status block with a shorter current summary.
- Handoff now distinguishes source baseline 51bd80e from the docs-only 58cbb67 checkpoint and includes the missing Active task heading. Exact source checks: HTTPD #45296325091 PASS (51/51), process #45296325017 PASS (5/5); handoff #45296533165 failed only on roadmap byte size; NVRAM audit #45295392634 PASS (15/15); XY-C1 #45295392702 still building older edc313c sources.
- Backup 72f4f157 remains read-only. No hardware L3/L4 claim.

---

## 2026-11-23 — Extend context-safe NVRAM output across web UI templates

- Confirmed additional JavaScript-source NVRAM interpolations in WAN, LAN, DHCP, IPTV, Services, SettingBackup and MACFilter ASP templates; HTML form contexts were intentionally left on nvram_get_x.
- Reused nvram_get_js for those JS literal/argument sinks and extended HTTPD workflow path filters plus exact source-marker assertions. Main commit: 51bd80e.
- Prior exact 2a50ce0 HTTPD run #45295707346 PASS; NVRAM transaction audit #45295392634 PASS (15/15) on unchanged transaction code. Exact 51bd80e HTTPD terminal status and a firmware build including the seven ASP changes were not confirmed at checkpoint.
- Next: verify exact-SHA HTTPD workflow and ensure XY-C1 build/TRX/artifact cover main 51bd80e. No hardware L3/L4 claim.

---

## 2026-11-23 — Fix DDNS JavaScript string injection

- Confirmed a stored-XSS sink: Advanced_DDNS_Content.asp initialized ddns_hostname_x inside a single-quoted JavaScript literal using the HTML-only nvram_get_x output helper. An apostrophe in the stored hostname could terminate the script string.
- Added nvram_get_js in web_ex.c, reusing the existing tested JavaScript string writer, classified it as HTTPD_DIRECT_EJ_ESCAPED_OUTPUT, extended direct-EJ registry tests/contracts, and switched only the JavaScript sink. The separate HTML input value continues through HTML escaping.
- Source commit edc313c; workflow source-contract corrections 6cb5c16 and 2a50ce0. Exact main HTTPD security run #45295707346 PASS; AI handoff check #45295707326 PASS. NVRAM boundary audit #45295392634 PASS (15/15).
- XY-C1 build run #45295392702 for production source edc313c was still at Build XY-C1 (37/44) at last check. TRX and artifact remain unconfirmed; do not claim product L3. Backup branch 72f4f157 unchanged.

---

## 2026-11-23 — Harden HTTPD group operations and bounded request parsing

- Refreshed the live handoff from the stale 3ec4e18 checkpoint to exact main c7bcd00; backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157 and untouched.
- Fixed NVRAM group compaction's stale count race (09576b6), completed AiDisk folder-tree HTML escaping and Node regression (beb5777), made HTTPD group Add transaction-locked with count-last publication and rollback (c89d1d7 / 6262b52), and added incomplete-rollback/count-read fault injections (3e02326 / 0136ceb).
- Hardened websScan group-delete index parsing against oversized values whose valid numeric prefix could previously survive fixed-buffer truncation; preserved the legacy exact "-1" terminator and added a GNU89 policy regression. Final source/test chain: 96431bb, 2b8333e, c7bcd00.
- Exact c7bcd00 HTTPD regression #45294291762 PASS (51/51, including group-delete, group compaction/add, AiDisk XSS); Process lifecycle inventory #45294291601 PASS. NVRAM boundary #45294292010 remained IN_PROGRESS at 12/14. XY-C1 run #45294157650 for the current production-source SHA 96431bb remained IN_PROGRESS at Build (37/44); no L3 claim until Build/TRX/artifact all pass.
- Independent IPTV/process follow-ups on main include 99ad939, d38250b and b645113; corrected oversized IPTV fixture and stale checker contracts, with earlier exact-SHA WAN route, Process, storage, and firmware passes recorded in SESSION_HANDOFF.
- No local MIPS build or hardware verification performed. Next: poll #45294292010 and #45294157650; continue targeted userland security work while pending.

---

## 2026-11-23 — Repair stale handoff and record userland stability work

- Updated the live checkpoint from stale main `02f174a93bf275930cede271d233dcf2c57438fa` to `3ec4e18b735a1e132884d91ab259b365a420aebc`; backup branch remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and was not modified.
- Atomic source slices completed: IPTV proxy NVRAM preflight and legacy missing-key defaults (`75a29ff...`, corrected by `05de0ec...`); transactional HTTPD Basic-auth credential snapshot (`6a9358e...`, ABI-compatible follow-up `50b712a...`); HTTPD ACL checked reads and strict numeric validation (`2b07887...`, `108c1c7...`, corrected regression literals `3ec4e18...`); HTTPD NVRAM group compaction read-error handling and transaction exclusion (`ebb80e3...`, `4363bd2...`, CI contract `7a3e304...`).
- Exact `3ec4e18...` Actions: HTTPD security #45289740712 PASS; process lifecycle inventory #45289740731 PASS; NVRAM transaction boundary audit #45289740708 PASS.
- Exact product-source SHA `05de0ec...` XY-C1 run #45289029423 remained IN_PROGRESS at the last check; all other listed exact-SHA userland/storage/network checks on that SHA passed. The later `main` SHA has no observed exact-head XY-C1 build/TRX/artifact run, so current-tree L3 is NOT VERIFIED; no local cross-build or hardware test is claimed.
- Next: poll #45289029423, then investigate (without assuming exploitability) AiDisk `ej_get_folder_tree` symlink/path behavior and its consumers before choosing a fix.

---

## 2026-11-23 — Update EPHY calibration handoff and verification boundary

- Recorded live main `02f174a93bf275930cede271d233dcf2c57438fa`, latest raeth source SHA `02f174a93bf275930cede271d233dcf2c57438fa`, exact push Actions and backup branch `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Captured FE/GE checked analog I/O, bounded retries, EPHY init/batch and ioctl error propagation status. XY-C1 L3=NOT VERIFIED; no physical hardware L4 claim.
- Next independent development slice is userland stability/security outside raeth after exact-head checks are reconciled.

---

## 2026-11-24 07:24 CST (UTC+8) — Keep interrupts enabled during PHY calibration ioctl

- Base main SHA: d855df7965ca1a0e393cb9af420491eccff6d089. EPHY ioctl runs synchronous FE/GE calibration across multiple ports with bounded retry and MDIO polling loops. ei_ioctl() previously reacquired page_lock with spin_lock_irq immediately before ephy_ioctl(), leaving local IRQs disabled throughout the full calibration.
- Moved the relock until after ephy_ioctl() returns. The calibration functions use their dedicated checked PHY-page/MDIO transaction locks and do not access the CR register block protected by page_lock. The copy_from_user failure path still reacquires page_lock before the shared function-exit unlock.
- Extended the EPHY ioctl source contract to require IRQ unlock before calibration and relock after the call. Source preflight confirms no successful-path reacquisition occurs until ephy_ioctl() completes.
- Parent exact-SHA Switch CLI #45285332081 and AI handoff #45285331846 PASS. Older source SHA 6b1c7c4... target build was pending; new source commit triggers a fresh XY-C1 build.
- No local MIPS build or physical hardware test. No L3/L4 claim.

---

## 2026-11-24 07:20 CST (UTC+8) — Update FE contract for extracted cleanup callback

- Base main SHA: 6b1c7c42edb393ffa9f38ac35bd62dac66cb8147. Exact Switch CLI #45285025962 FAIL, job #138079002575, step 30. The old contract still expected teardown writes and saved-snapshot restore conditions to remain inline in the orchestrator. The source change intentionally moved them to ra_switch_fe_analog_cal_cleanup().
- Updated the contract to inspect the callback for saved-snapshot restoration and analog clear, and assert that the orchestrator invokes the setup-gated cleanup policy. Existing checked-MDIO, child-failure and retry-order assertions remain unchanged.
- The GNU89 cleanup-policy regression and FE gate contract were skipped because step 30 failed earlier in the job; no host test PASS is claimed. Parent AI handoff #45285025900 PASS. XY-C1 #45285025914 and NVRAM #45285025931 were IN_PROGRESS at last query.
- Workflow-only correction; product source remains SHA 6b1c7c42edb393ffa9f38ac35bd62dac66cb8147. No local MIPS build or hardware test.

---

## 2026-11-24 07:18 CST (UTC+8) — Gate FE analog teardown on setup attempt

- Base main SHA: `f7183840c0de8e8096d60940eb2e5d4b60687839`. The FE all-analog orchestrator's first two local register snapshot reads can fail before this invocation mutates hardware, yet the common cleanup previously cleared shared analog controls and wrote local page1/register26, BMCR `0x3100`, and advertisement `0x05e1`. This may reset a PHY before calibration setup has begun.
- Added `setup_attempted` immediately before the first mutating MDIO write. The marker is deliberately before the transaction because a reported MDIO failure may still mean the PHY received the write. Early snapshot failures skip teardown; failures after setup attempt retain all existing best-effort cleanup operations and valid-snapshot restore behavior.
- Extracted cleanup into `ra_switch_fe_analog_cal_cleanup()`. Added `ra_switch_mdio_cleanup_if_started()` and a GNU89 callback fault-injection regression covering pre-setup skip, post-setup invocation, timeout propagation, positive-status normalization and NULL callback handling. Added explicit ASan/UBSan host compile/run and source-order contract to Switch CLI workflow.
- Parent exact push Actions: Switch CLI #45284655414 SUCCESS; AI handoff #45284655394 SUCCESS. Product parent `30f6a0e1694d3f0d1b0e35e78c2aac9e725ae2a2`: XY-C1 #45284437855 was IN_PROGRESS at last check; NVRAM #45284437839 completed SUCCESS. New product commit requires its own XY-C1 run.
- No local MIPS/kernel build or hardware test. L3 requires Build, TRX and artifact upload all PASS on the same source SHA.

---

## 2026-11-24 06:52 CST (UTC+8) — Scope Leopard EPHY init contract to Leopard branch

- Base main SHA: `e77a76bc61a790cd6282ac0d0d70b01f081cb4a8`. Switch CLI #45283594271 failed at workflow step 28, `Validate Leopard EPHY initialization checks each local MDIO operation`. Its extracted block started immediately after `mt7622_ephy_cal();`, therefore it also counted the MT7622 error gate and Leopard batch-runner error gate in addition to the ten Leopard page operations; the assertion expected ten gotos and was off by two.
- The contract now starts after `status = leopard_ephy_cal();`, isolating the Leopard-only local setup: nine checked page writes, one checked page read and ten corresponding MDIO error gates. This is a workflow-scope correction; production code remains unchanged.
- Parent exact push Actions: AI handoff #45283594232 success; Switch CLI #45283594271 FAIL (false count due to over-wide block). Product-source SHA `5157b869cb62a22e04057033ffc5d6e1149a5e6e`: XY-C1 #45283497318 in_progress; NVRAM #45283497339 success. The target build is not yet established.
- No local MIPS/kernel build or hardware test. The corrected checker requires its own exact-head Actions.

---

## 2026-11-23 — Exact-head handoff refresh

- Source checkpoint at inspection: `ee0bf75292e0436e8820d3a30cd8b9a28c2bfee1` (`fix(raeth): check FE VBG calibration MDIO`).
- Exact SHA workflow runs: NVRAM transaction boundary audit #45279437099: in_progress; XY-C1 Firmware Validation #45279437381: in_progress; AI handoff documentation contract #45279437408: success; Switch CLI ioctl failure-status regression #45279437249: failure
- Exact SHA check-runs: validate-handoff-records: success; Cross-build XY-C1 firmware: in_progress; switch-cli-contract: failure; nvram-transaction-boundary: in_progress
- Backup branch kept read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no target L3 or hardware L4 claim made without same-SHA evidence.

---
## 2026-11-23 — Make FE VBG calibration fail closed on MDIO errors

- Converted FE VBG setup, page-7 control RMW, local page-3 setup, comparator reads, wait and result programming to checked MDIO. Transport errors stop calibration and withhold completion.
- Added a pure six-bit calibration-code step helper with GNU89 host coverage for both wrap directions. The Leopard REXT table index is bounded before access; ordinary timeout/saturation and exhausted search retain the 0x20 fallback.
- Every exit attempts shared analog clear and both port/common page-3 resets, preserving the first error; the VBG completion flag is published only after successful output programming and cleanup. Added Switch CLI source contract; exact-head Actions and XY-C1 L3 are pending.

---

## 2026-11-23 — FE R50 checked MDIO and safe table indexing

- Converted FE R50 setup, shared page-7/register-24 bit updates, FE page-3/4 state, comparator reads and both calibration waits to checked APIs. MDIO errors now abort the calibration instead of being treated as zero-valued samples.
- Added checked Leopard/MT7622 R50 output helpers while retaining the existing public void setters for compatibility. The MT7622 path now validates the per-port subtraction offsets before indexing the 64-entry lookup table, preventing unsigned-underflow out-of-bounds reads.
- Not-ready/saturation still uses the 0x20 fallback; a bounded search that exhausts all 254 attempts also falls back instead of publishing an unverified code. Best-effort cleanup always attempts the common analog clear and both local page-3 register resets; the completion flag is set only after all output and cleanup operations succeed.
- Added Switch CLI source-contract checks for the production FE R50 path and table bounds. The pre-existing GNU89/Werror/ASan/UBSan policy suite continues to test the checked update-bits primitive; exact-head target build is still required.

---
## 2026-11-23 — GE all-analog orchestration checked I/O

- Top-level C22/C45 setup and RX-offset/eye/bias accesses now use checked APIs; BMCR and 1e:145 restore only when snapshots succeeded.
- Clear/check R50, TX-offset and TX-amplitude child flags before continuing. Failed child stops later work; all exits attempt entry-state and BG-output cleanup. Source contract added; exact-head CI pending.

---
## 2026-11-23 — GE TX-offset checked MDIO

- Replaced unchecked setup, four snapshots, comparator, waits and result writes with checked APIs; I/O errors abort, ordinary not-ready retains the 0x20 fallback.
- Every exit attempts analog clear and all force-DAC/DB/DC/3E/DD resets; first error wins and the flag is published only after cleanup. Added source contract; exact-head CI pending.

---
## 2026-11-23 — Make GE TX-amplitude calibration MDIO-status aware

- Base main SHA: `ca05cf915058cbe2141f38bff01575f9133767dd`; converted the GE TX-amplitude setup, four-pair search, comparator reads and per-pair result writes from value-only C45/paged PHY wrappers to checked operations.
- The checked GE analog wait distinguishes transport errors from ordinary not-ready timeout; MDIO failures stop the remaining pair loops. A timeout keeps the existing 0 dB fallback behavior. Successful calibration is published only after cleanup.
- Added a reusable Clause 45 cleanup-sequence helper that attempts every register write and retains the first error. TX-amplitude cleanup clears common analog state, restores the eight forced TX control registers and supporting C45/global registers, then disables DB/DC/3E/DD; each independent cleanup group is attempted even when an earlier write fails.
- Added GNU89 policy fault injection for complete cleanup after a middle write failure and a Switch CLI source contract covering the production TX-amplitude function. Exact-head CI required; no MIPS build, XY-C1 L3 or hardware L4 is claimed.

---
## 2026-11-23 — Make GE R50 calibration propagate MDIO failures

- Base main SHA: `d524e61986a573eaa92c800893273e79f78f2f6f`; this slice covers the GE R50 common setup and four-pair calibration routine.
- Added a status-bearing common-setup helper while preserving the legacy public `fe_ge_r50_common()` ABI. Converted port-local page 3/4 operations, global page-7 register-24 RMW, pair-select Clause 45 writes, comparator reads and each pair's packed result read/modify/write/readback to checked APIs.
- GE R50 now uses the checked GE calibration wait; a transport error aborts remaining pairs. Timeout/saturation retains the legacy 0 dB fallback behavior. Common analog cleanup and the final Clause 45 calibration-disable write are both attempted on every exit, preserving the first error; `ge_cal_r50_flag` is not set on transport/cleanup failure.
- Reused the existing tested MDIO update-bits policy and added workflow source contracts for both checked common setup and the full R50 path. Exact-head Actions remain required; no MIPS L3 or hardware L4 is claimed.

---
## 2026-11-23 — Make GE Rext calibration fail closed on MDIO errors

- Base main SHA: `2dfd89151b59774bad5598636fde26a98ba2b48b`; product changes are limited to `ge_cal_rext()` plus shared RAETH policy and regression coverage.
- Replaced legacy value-only Clause 45 reads/writes throughout Rext calibration with checked operations. Snapshot, setup, bounded wait, comparator reads and result writes now abort on transport failure rather than consuming fabricated zero values or publishing a false calibration success.
- Added a cleanup policy that restores the saved Rext control to the existing 0 dB fallback when calibration did not complete, then always attempts to clear the calibration-enable register. The first MDIO error is preserved while disable is still attempted.
- Added GNU89 fault-injection coverage for restore+disable success, restore failure with disable still attempted, absent snapshot, disable failure and invalid callback; added Switch CLI source-contract coverage.
- Base SHA `2dfd891` had Switch CLI #45274289879 and AI docs #45274290063 PASS; NVRAM #45274289994 and XY-C1 #45274289973 were pending. This slice requires its own exact-head CI; no local MIPS build, XY-C1 L3 or hardware L4 is claimed.

---
## 2026-11-23 — Propagate MDIO errors through FE calibration waits

- Base main SHA: `bff3d15326e573f5210ac2a121e21c7485744ece`; its preceding documentation child restored the full chronological development log. The analog cleanup RMW source parent is `51f29580ebfb44d9ecbcbd4ec6bf8a55ff6e77c7`.
- `51f2958`: Switch CLI #45273676193 PASS. AI docs #45273675795 failed because that source commit had one log entry after an accidental history overwrite; docs child `bff3d15` restored the complete archive. XY-C1 #45273675906 was still building and NVRAM #45273675699 still running at the last poll.
- Both FE analog wait variants previously used value-only global PHY reads/writes and local page-4 writes, so MDIO errors could look like ordinary timeouts and local calibration state could remain enabled.
- Added one checked FE wait path. It preserves TX-amp's original pre-trigger reset of FE_CAL_COMMON only, while the normal FE wait resets both common and selected port. Both paths delegate global page-7/reg24 trigger/poll/cleanup to the checked GE wait and always attempt both local page-4/reg23 cleanup writes, including when pre-trigger or polling fails.
- The existing `u8` interfaces stay unchanged; transport or cleanup errors map to `ANACAL_ERROR`, while an ordinary not-ready timeout still returns zero. New paired-cleanup policy tests prove both writes are attempted and the first error is retained.
- Added Switch CLI source assertions for the production FE wrappers and paired-cleanup policy. Long FE/GE calibration bodies still contain value-only MMD calls and need separate register snapshot/restore migration. No MIPS build or hardware L4 is claimed.
- Next: check exact-SHA Actions for this FE wait change and verify the active XY-C1 Build/TRX/artifact stages; then select another bounded calibration RMW.

---
## 2026-11-23 — Preserve development log history and checkpoint analog cleanup

- Source parent: `87e233d30dd532b6bfd1589d2469ced8d8f42a16`; analog cleanup RMW source commit: `51f29580ebfb44d9ecbcbd4ec6bf8a55ff6e77c7`.
- The analog cleanup change adds checked update-bits, stops intermediate RMW after the first MDIO error, and still attempts the original final page-7/register-24 zero write. Switch CLI/AI/XY-C1/NVRAM Actions for `51f2958` are being checked; do not claim exact-head PASS yet.
- Corrected a documentation regression in `51f2958`: its log update accidentally replaced the earlier chronological entries. This commit restores the full log content from its parent `87e233d` and prepends this checkpoint; no source files are changed here.
- On `87e233d`, Switch CLI #45273294951 PASS, AI docs #45273294849 PASS and NVRAM #45273294824 PASS. XY-C1 #45273294825 remained in progress on that older SHA at the last poll.
- Backup branch remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`. No MIPS/kernel build or hardware L4 is claimed.

---
## 2026-11-23 — Make GE analog-calibration wait status-bearing

- Base SHA: `f432a3ef15430bd40828cd05c54898cbaf19ce25`. Its Switch CLI #45272997503 PASS and AI docs #45272997319 PASS, validating the checked C22/Clause45 write policy and legacy wrapper adapters. XY-C1 #45272997410 and NVRAM #45272997327 were still in progress at last poll.
- Reviewed the shared GE calibration wait used by the REXT, R50, TX-amplitude and TX-offset algorithms. It previously used value-only page-register wrappers; an MDIO timeout returned zero and was indistinguishable from a genuine calibration timeout, while the trigger bit clear at the end was best-effort only through a void wrapper.
- Added a callback-driven calibration wait policy: checked initial read, trigger-clear, re-read, trigger-set and bounded ready polling; every exit attempts a fresh read followed by trigger-bit clear. A transport failure or failed cleanup returns its errno, invalidates the ready result, and is distinct from an ordinary not-ready timeout.
- Wired the production GE wait to page-7 register 24 through the existing IRQ-safe page accessors. The exported legacy `u8 all_ge_ana_cal_wait()` signature remains unchanged and maps transport failures to the existing `ANACAL_ERROR` sentinel so current calibration callers enter their established error/cleanup paths.
- Added failure injection for every setup/poll/cleanup transaction, positive-status normalization, timeout, successful ready and invalid arguments. Switch CLI adds source-contract assertions for real page-7 access and trigger cleanup.
- This slice only hardens the shared wait handshake; surrounding calibration algorithms still contain many legacy value-only MMD calls. Those remain a separate migration requiring explicit snapshot/restore design. No MIPS kernel build or physical hardware L4 is claimed.
- Next: inspect exact-SHA Switch CLI/AI docs and XY-C1 job outcome; then trace one calibration algorithm's full register rollback path before expanding the conversion.

---
## 2026-11-23 — Fix checked PHY write policy declaration order

- Exact-head Switch CLI for `d8870638f6add35d455e0fdbf482508d05d7e15d` failed before running the RAETH regression: the new write-policy inline functions appeared above their callback typedefs and status-normalizer declaration, producing unknown-type errors under GNU89/Werror.
- Moved the checked-write helpers below the callback typedefs and `ra_switch_mdio_normalize_status()`; no semantics changed. The next exact-SHA Switch CLI run recompiles and executes the full failure-injection suite.
- AI docs, XY-C1 and NVRAM results for `d887063` are tracked separately; no MIPS kernel build/hardware L4 is claimed.
- Next: inspect the corrected run and continue the GE calibration wait helper audit.

---
## 2026-11-23 — Make legacy PHY wrapper MDIO errors visible

- Current base main SHA: `48bc28ab58076e9ac90a0a9546e8ca91996caa56`. The GE2 polling source-contract delimiter correction passed Switch CLI #45272254777 and AI docs #45272254645 on that exact SHA.
- The test-fixture SHA `5abf4a5c4b3fb08b6c3f5e8d13418679aec4dd58` also passed NVRAM #45271922399 and the XY-C1 workflow #45271922442. XY-C1 job details show Build, TRX and artifact upload were SKIPPED because the firmware production inputs did not change; this is host-only coverage, not an L3 firmware build.
- Found another concrete diagnostic gap: the four legacy `tc_phy_read/write_dev_reg()` and `tc_mii_read/write()` adapters ignored the MII manager's boolean result. MII manager clears read outputs on failure, so callers silently consumed a zero register value and writes failed without a call-site diagnostic.
- Added normalized status-bearing C22/Clause45 write policy helpers and callback fault-injection tests. Existing public/legacy signatures remain unchanged; internal checked adapters now back all four wrappers, initialize read outputs to zero, and emit rate-limited diagnostics for failed reads and writes.
- This exposes failures but does not yet abort the long GE calibration algorithms: their value-only read/write call graph and register restore paths still need a separate status/rollback design. No global call-site substitution was made.
- GNU89/Werror/ASan/UBSan regression will execute in exact-head Switch CLI Actions; no MIPS kernel build or hardware L4 is claimed.
- Next: inspect exact-head Switch CLI/AI docs; then convert one bounded GE calibration polling helper with explicit cleanup semantics, or continue another independent stability fix if the calibration transaction scope is too coupled.

---
## 2026-11-23 — Fix GE2 polling source-contract newline delimiters

- The corrected GE2 test fixture follow-up `5abf4a5c4b3fb08b6c3f5e8d13418679aec4dd58` passed AI docs #45271922401. Its Switch CLI #45271922464 then passed the compiled RAETH policy binary (including the corrected half-duplex/EEE case) and existing source contracts, but failed in the new GE2 source-contract step because escaped newline delimiters were emitted as literal backslash-n text.
- Replaced those delimiters with `chr(10)` boundaries; this is a checker-only correction and does not weaken production assertions. The policy regression itself had already passed in the failing run.
- XY-C1 #45271922442 and NVRAM #45271922399 were still running at last poll on the test-fixture SHA; this follow-up's exact-head Actions are required for current verification.
- Next: confirm the exact reruns, then continue tracing GE analog calibration cleanup with the status-bearing helper design in mind.

---
## 2026-11-23 — Correct the GE2 polling EEE regression fixture

- The first exact-head Switch CLI run for source commit `bfca47b0c33cae7679ea2e671cc71a015ebfc5d4`, #45271645960, reached the GNU89/Werror/ASan/UBSan RAETH policy binary and failed one newly added assertion: the intended half-duplex fixture used raw link-speed `0x24`, whose `0x20` duplex flag correctly denotes full duplex. This was a test-data error, not a production-code failure.
- Corrected the test vector to `0x04` (100M half-duplex) so it independently exercises clearing the duplex flag alongside stale EEE bits.
- A standalone GNU89/Werror/ASan/UBSan policy harness with the corrected sample passes the snapshot failure-injection and EEE rebuild cases. Exact repository regression and source contracts require a rerun on this correction SHA.
- On `bfca47b`: AI handoff #45271646030 PASS; XY-C1 #45271645953 and NVRAM #45271645977 were IN_PROGRESS at the last poll. No kernel/MIPS build or hardware L4 is claimed.
- Next: verify exact-head Switch CLI, AI docs and XY-C1 for this test correction; then continue the GE analog-calibration caller/restore audit.

---
## 2026-11-23 — Make GE2 PHY link polling fail closed

- Baseline main SHA: `72076155464bb033d5dc61d5d141e76cd08c2181`; source debug transaction fix `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` is already in main.
- Found an unchecked-MDIO defect in `phy_polling_thread()`: it ignored all six Clause 22/45 read results. The MII manager clears failed read outputs to zero, so a timeout at the first read looked like a real link-down event; later failures could publish fabricated pause/speed/EEE capabilities.
- The poll now obtains one complete status-bearing PHY snapshot. It publishes MAC link capabilities only on full success; failed samples are logged and leave the prior hardware state untouched. A confirmed link-down sample still invokes the existing force-down path.
- Fixed a second state bug: EEE bits were only ORed into persistent `val_tmp` and never cleared when renegotiation stopped advertising EEE. Capability bits are now rebuilt on each successful sample while unrelated bits are preserved.
- Added GNU89 policy regressions for all six MDIO read failure points, positive callback normalization, complete-sample ordering, link-down short-circuit, invalid callbacks, state preservation and stale EEE clearing. Added exact-source assertions to Switch CLI workflow; production/test/header paths are in both Switch CLI and XY-C1 push filters.
- Previous PHY debug SHA `ad5bcd2`: Switch CLI #45270470838 PASS; AI docs #45270470371 FAIL only because its tree inherited the oversized roadmap (131457/131072 bytes), corrected on docs-only main child `7207615` with AI docs #45270615085 PASS. XY-C1 #45270470239 and NVRAM #45270470405 were IN_PROGRESS at last check. This new slice's Actions have not yet run.
- No local MIPS/kernel build or hardware L4 is claimed.
- Next: inspect this commit's exact-head Switch CLI, AI docs, NVRAM and XY-C1 runs; then trace the GE analog-calibration save/restore graph before converting legacy wrappers.

---
## 2026-11-23 — Fail closed on PHY debug-register MDIO errors

- Source commit `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` adds a checked Clause 45 read policy with failure injection, checks each token-ring Clause 22 write/read, serializes the complete page/control/data sequence under `ra_switch_phy_page_lock`, attempts to restore page 0 on every attempted transaction, and suppresses partial debug output. Leopard MMD diagnostic reads stop on the first failed sample. The legacy `phy_tr_dbg()` value-returning signature remains available.
- Added a dedicated Switch CLI source-contract step for the checked adapter, token-ring sequence, page restoration/locking and the four MMD failure guards. Host regression is part of the existing GNU89/Werror/ASan/UBSan policy test step; exact-head run pending at this documentation update.
- Source SHA `638dd487`: Switch CLI #45268911721 PASS; AI docs #45268911599 PASS; NVRAM #45268913631 PASS; Process #45268911977 PASS; XY-C1 #45268911701 was IN_PROGRESS at last check. Docs-only child `7ec66ff` failed AI docs #45269398685 because REFACTOR_ROADMAP.md measured 131457 UTF-8 bytes against a 131072-byte cap.
- The docs-only child `7ec66ff` failed AI handoff validation solely because the roadmap reached 131457 UTF-8 bytes, 385 bytes above the 131072 limit. This commit compresses the duplicate historical WAN-route narrative to retain the transactional route behavior and checker-failure provenance; roadmap UTF-8 size is now 130000 bytes.
- No local kernel/MIPS build, XY-C1 Build/TRX/artifact result for the debug source commit, or hardware L4 is claimed.
- Next: inspect exact-SHA Switch CLI, AI docs, NVRAM, Process and XY-C1 Actions for `ad5bcd2`; then migrate one bounded PHY calibration caller family away from value-only MDIO wrappers without changing public ABI.

## 2026-11-23 — RAETH MDIO 错误传播扩展

- 本轮代码基线 SHA：`638dd487b626b0aac718eec046573dcd1393274c`（提交：fix(raeth): report proc MIB MDIO failures）。
- 将关注面从 MII manager 扩展到 RAETH switch startup：分页 PHY 的 page-select/data 事务、Clause 45 访问、MT7530 reset-ready 等待、内部 switch 配置、MT7530 PHY 配置、TRGMII calibration、MT7621 VLAN partition、GE PHY identification，以及 embedded-switch GPIO PHY 配置。
- 错误处理采用 checked MDIO adapter，将失败转为负 errno，并在初始化链路中停止后续寄存器写入；VLAN busy 轮询增加有界超时；PHY read wrapper 清零输出，避免失败时返回未初始化数据。
- MIB 计数器快照测试覆盖逐寄存器失败注入，并要求完整快照成功后才发布；MDIO timeout 使用单调纳秒时钟，避免 IRQ-off 区域依赖 jiffies。
- 回归测试已在 `switch-cli-stability.yml` 与 XY-C1 workflow 中接线；具体 PASS/FAIL/PENDING 以 exact-SHA Actions 状态为准，不把 PENDING 记作通过。
- 备份保护分支仍为 `backup/pre-plugin-overhaul-2026-11-14` @ `72f4f157a084f8184b9597ca5bf334b8080e5807`，本轮未修改。
- 下一步：按接力文档中记录的 exact SHA 对 Actions 做确认；优先修复失败检查/构建，再清理 RAETH 中剩余的兼容 MDIO wrappers，并继续向其他 userland 可靠性问题扩展。

---
## 2026-11-23 — Report proc MIB read and snapshot-control failures

- RAETH's `DUMP_EACH_PORT` macro ignored `mii_mgr_read()`'s boolean result. The driver clears the output to zero on failure, so proc output could report an MDIO timeout as a legitimate zero counter.
- Failed per-port counter reads now print an aligned `ERR` marker; successful reads keep the existing numeric format. Both MIB snapshot-control writes to `0x4fe0` now emit explicit failure messages. Register values, ordering and write attempts are unchanged.
- Added exact-source workflow assertions for read-result handling, numeric/error output and both snapshot write guards/messages. Added `ra_dbg_proc.h/.c` to Switch CLI push/PR and XY-C1 push path filters, plus path-filter contract checks.
- Parent exact-SHA Actions: Switch CLI #45268640001 PASS and AI docs #45268640240 PASS on `2a45c674`; XY-C1 and NVRAM did not trigger on that workflow-only commit. This commit's exact-head runs are pending. No local kernel/MIPS build or hardware L4 is claimed.
- Next: inspect exact-head Actions, then continue a targeted audit of RAETH proc/debug telemetry for any other concrete ignored MDIO status.

## 2026-11-23 — Preserve switch link state when PHY MDIO reads fail

- Both MTK/GSW and legacy ESW link interrupt paths ignored the success result from `mii_mgr_read()`. That API clears its output on failure, so an MDIO timeout could be mistaken for a genuine link-down sample; the GSW path also acknowledged all port-change flags even when an individual port-status read failed.
- Added checked MDIO read/link-sample policy. GSW callbacks now run only after a successful sample, and W1C acknowledgement is restricted to successfully sampled port flags so failed samples remain pending. The legacy ESW path retains its previous cached state and skips transition/debug reporting on a failed read.
- Added failure-injection coverage for negative/positive callback errors, successful link-bit decoding, invalid callbacks/masks, zeroed checked output and preservation of the caller's previous link state. Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan host regression: PASS. Exact-source workflow contract and exact-head Actions are pending for this commit.
- Parent main: `5ba21118a12464be1623b03c35dc3b48cb43f66d`. This atomic commit contains driver, policy, test, source checker, dev log and handoff. No local kernel/MIPS compile, same-source XY-C1 Build/TRX/artifact result or hardware L4 is claimed.
- Parent exact-SHA Actions snapshot: Switch CLI #45266083220 IN_PROGRESS; XY-C1 #45266083296 IN_PROGRESS; AI handoff docs #45266083382 IN_PROGRESS; NVRAM #45266083302 QUEUED. These parent-head results do not validate this new commit.
- Next: inspect definitions/callers of `esw_get_port_mib_rgpc()` and `esw_get_port_mib_tgpc()` used by WAN traffic telemetry; ensure packet and octet counters preserve MDIO read failures without changing exported ABI.

## 2026-11-23 — Propagate MTK ESW IGMP/MLD MDIO failures

- Found that the optional IGMP flood-to-CPU, port-learning and MLD snooping paths still used unchecked esw_reg_get()/esw_reg_set() operations, and both IGMP ioctl handlers discarded their result.
- Source commit 81c514f33641d47aa49062443e8fb23e80807765 moves IMC to a checked masked update, ISC to a checked masked update preserving unrelated bits, and PIC writes to checked register writes. Independent per-port operations continue while preserving the first error; bridge reconfiguration and init-post stop on failure. Runtime IGMP/static-port selectors restore their cached state on failed programming, and ioctl results now reach userland.
- Switch CLI exact-SHA run #45257584087 PASS. AI docs #45257584133 failed because SESSION_HANDOFF.md lacked its required ## Current repository state heading; this docs-only child restores it. NVRAM #45257583956 and XY-C1 #45257583931 were in progress at the last poll. No local MIPS/kernel compile or hardware L4 is claimed.
- Next: inspect status/error propagation for MTK ESW link-status and MIB telemetry reads.

## 2026-11-23 — Commit WAN fallback PVID only after confirmed switch programming

- Found that `esw_vlan_apply_rules()` published `g_vlan_pvid_wan_untagged` before resetting/programming VLAN table entries or applying per-port PVID/attribute/accept/ingress registers. A later MDIO timeout could therefore make `MTK_ESW_IOCTL_VLAN_PVID_WAN_GET` report a new fallback VID while hardware remained on the previous configuration.
- Added a checked applied-value commit policy and failure-injection regression: negative and positive callback failures preserve the previous value; only successful programming publishes the requested VID. Moved the cache update to the successful tail after all required hardware operations.
- `vlan_accept_port_mode()` now attempts every selected independent port and returns the first normalized MDIO error instead of leaving later ports untouched after the first failure. The exact-source Switch CLI checker asserts publication ordering and best-effort multiport error aggregation.
- Parent main: `2ea2105e917f1b00e7269650f46dbe2831f877f7`. Exact-SHA Actions for this source commit are pending; no local kernel/toolchain compile or physical hardware test is claimed.

## 2026-11-23 — Propagate MTK ESW port-register MDIO failures

- Found that `esw_reg_get()` returns zero on MDIO read failure and `esw_reg_set()` only logs write failure. Port PVID, ingress, attribute, frame-accept and forwarding-matrix helpers were void, allowing VLAN/bridge setup to report success after failed hardware programming.
- Source commit `3ab6e43a2ca7db2ab7521962566c34a4b8b70b1e` converts those helpers to checked status APIs. Masked operations use `mtk_esw_register_update_bits()`; direct PVID and MAC-to-PHY writes use `mtk_esw_register_write()`, which validates callbacks and normalizes positive callback failures to `-EIO`.
- Propagated errors through VLAN rule/VID-1 setup, custom VLAN creation, accept-mode ioctl, bridge forwarding matrix, MAC-to-PHY setup and init-post. Independent per-port matrix/MAC updates preserve the first failure while attempting other selected ports; sequential VLAN transitions stop on first failure.
- Prep commit `a0ff74d73e6d8b9ecb45ba9937c267bf3f4d4916` added GNU89/Werror/ASan/UBSan regression for checked-write success, negative errno, positive-status normalization and invalid callbacks; the source workflow now checks helper/caller propagation.
- Earlier source SHA `59622f3c9612949ddd642fc738e02463f5400def` passed XY-C1 Build/TRX/artifact #45252058834, NVRAM #45252058851 and AI docs #45252059088. Switch CLI #45252851887 and AI docs #45252851808 passed on descendant `6acc500`. Exact-head validation for `3ab6e43a2ca7db2ab7521962566c34a4b8b70b1e` must be checked separately. No local MIPS build or physical-device result is claimed.

---

## 2026-11-23 — Align Switch CLI checker with transactional VLAN-entry helper

## 2026-11-23 — MT7530/VLAN reliability continuation

- Current `main`: `d86e5b1d6aa745ba1db87ec21b153e35c80a4446`; backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged).
- Implemented/continued checked VLAN data-word + VTCR staging, retry-safe pool publication, status propagation, and table-before-port preflight; regression coverage includes injected MDIO failures.
- Additional checked register work covers jumbo frames, broadcast storm control and LED mode, with cache updates only after successful hardware writes.
- Exact-SHA Actions snapshot: **PENDING / not all checks completed**.
- XY-C1 Firmware Validation: in_progress (run 45253098709)
- NVRAM transaction boundary audit: in_progress (run 45253098328)
- AI handoff documentation contract: completed/success (run 45253098335)
- Switch CLI ioctl failure-status regression: completed/success (run 45253098429)
- Next: propagate MDIO status through remaining port/PVID/ingress/attribute/accept helpers and verify XY-C1 on the new exact SHA.
- No local firmware compile is claimed; use Actions results for build verification.


- Exact Switch CLI #45252058892 on `59622f3c9612949ddd642fc738e02463f5400def` ran the host MDIO policy binary successfully, then failed in the Python source checker with `ValueError: substring not found`. The stale checker required direct `esw_write_vtcr(1, cvid)` and raw VAWD1/VAWD2 writes inside `esw_vlan_set()`.
- Source commit `59622f3` intentionally factored VAWD1, VAWD2 and the VTCR wait into `mtk_esw_vlan_entry_apply()`; the helper test passed, and `g_vlan_pool` is published only after the transaction succeeds. Updated the checker to assert that helper call, its error gate, and the software-pool commit ordering.
- On source SHA `59622f3`, NVRAM audit #45252058851 PASS and AI docs #45252059088 PASS. XY-C1 #45252058834 was still running at the last poll; exact Build/TRX/artifact status must be checked.
- Next source reliability issue: MTK ESW configuration setters still use `esw_reg_get()` / `esw_reg_set()`, which collapse or swallow MDIO errors and can falsely report success after a failed VLAN/bridge hardware update. Convert the paths to checked status operations and add regression coverage.

---

## 2026-11-23 — Restore complete development log after VLAN compile fix

- Source fix `aacd6876408dfcd742528ac8ba0bc7d03357d909` restores the MTK ESW VLAN table-slot constant and lookup helpers, bounds VID search and fixes missing/misplaced success returns exposed by XY-C1 #45250584713.
- Switch CLI #45251664495 PASS on that exact source SHA. XY-C1 #45251664679 and NVRAM transaction audit #45251664582 were in progress at the last poll.
- AI handoff docs #45251664481 failed on the exact source SHA because the preceding commit replaced `dev_log.md` with one new entry instead of prepending. Restored the full prior archive from parent `c730d8dfe9cd37007ca0f11aa90a378f3c69e4a2` (20+ existing newest sections) and prepended this record. No previous log entries were dropped.
- Next: rerun AI docs on this recovery SHA; record XY-C1 Build/TRX/artifact outcome from the exact source SHA before making an L3 claim.

---

## 2026-11-23 — Correct VPN lifecycle source-checker input binding

- Exact VPN lifecycle run #45250371491 reached the integration checker after the selector-policy and VPN client runtime NVRAM/DNS checks passed, then failed because the newly added VPN-client helper assertions called `body(vpn, ...)` even though the checker stores the VPN client source in `vpn_client`. Corrected all three lookups (ownership snapshot, shared-dependency snapshot, and restore helper) to inspect the actual VPN client translation unit.
- This was a test-harness source-variable mismatch; no new production-source failure was demonstrated by that run. Exact-head VPN lifecycle must be rerun after this correction.
- Switch helper restoration commit `895a2f060cae5893b9daf3c3b76575bfda5ca479` restores the two definitions whose absence caused XY-C1 compile errors on `26f2617`; Switch CLI #45250584832 and XY-C1 #45250584713 are queued/in progress. Their outcomes, especially actual firmware compilation, must be verified before claiming recovery.
- AI handoff documentation #45250371547 PASS on `1915008aa573073bb5533b08f7a74e034bc9373f`; the historical `dev_log.md` was restored and is no longer truncated.

---

## 2026-11-23 — Restore MTK ESW ioctl helper definitions after XY-C1 compile failure

- XY-C1 run #45249235493 on `26f2617f744227f906b9e15baad4ae52b2d8e3be` failed while compiling `ioctl_mt762x.c`: `ioctl.c` referenced missing `change_vlan_rule()` and `esw_link_status_changed_state`, producing implicit-declaration / undeclared-identifier errors. Both static helper definitions existed in known-good baseline `7391f2e6245e64941ca709784719ebfb723043dc` but were absent after the recent switch refactor.
- Restored the VLAN rule selector validation/update helper and the link-status callback before `ioctl_mt762x.c` includes `ioctl.c`. Added workflow source-contract assertions for selector bounds, desired-rule update, callback side effects and both actual call sites. Retained the status-returning VLAN table and ioctl changes.
- This is the source fix for the real compile failure; it requires exact-head Switch CLI plus XY-C1 Build/TRX/artifact reruns. No local kernel toolchain or hardware test is claimed.

---

## 2026-11-23 — Match VPN restore helper call signature in integration contract

- Exact VPN lifecycle run #45249835903 still failed at the integration checker: its newly updated expectation searched for `vpn_client_restore_shared_xl2tpd_locked()`, but the function intentionally accepts a phase string and the call is `vpn_client_restore_shared_xl2tpd_locked("after ...")`. Changed the contract to match the call prefix while preserving order relative to the restart sequence.
- VPN client selector-policy and NVRAM/DNS source-contract stages had passed; this was a checker token mismatch, not a demonstrated C runtime defect. Exact VPN lifecycle must be rerun on the new source SHA.
- AI documentation contract #45249835953 PASS on source/docs SHA `1a2d8a749653c06ea4006ad4f43404c51bf28808` after restoring the full 20+ entry development log. Historical entries remain intact.
- XY-C1 #45249235493 on source SHA `26f2617` failed kernel compilation because `ioctl.c` references `change_vlan_rule` and `esw_link_status_changed_state`, but both definitions are missing from the current `ioctl_mt762x.c`; both existed in the last known successful baseline `7391f2e`. Next source task: restore the two local helpers, retain the new VLAN failure return semantics, and add exact-source compile-contract checks. No L3 build PASS is claimed for `26f2617`.

---

## 2026-11-23 — Reconcile exact-source contracts and restore complete development log

- VPN source SHA `26f2617f744227f906b9e15baad4ae52b2d8e3be` implemented checked NVRAM transactions for VPNC xl2tpd ownership and shared WAN/server dependencies. Its policy regression steps and `check_vpn_client_nvram_state.py` passed; VPN server lifecycle #45249235472 failed only because `scripts/check_vpn_server_lifecycle.py` still required `safe_start_xl2tpd_locked()` directly in `restart_vpn_client_locked()`. Restoration is deliberately delegated to `vpn_client_restore_shared_xl2tpd_locked()`; the checker now validates that helper call.
- AI handoff #45249235356 and subsequent exact run #45249619156 both exposed the same documentation regression: the preceding update replaced `dev_log.md` instead of prepending. Restored the complete prior chronological log (20+ entries) and prepended this record; no historical entries were discarded.
- Switch VLAN source contract follow-up: MDIO test binary now passes after correcting the fake VLAN busy bit; the checker was then stale on `static void` return-type markers. Commit `429b2361eb8f285970cdf95f94cc6b158c7535a6` updates those markers to `static int`. Switch CLI #45249619357 is in progress; do not claim PASS until its exact conclusion is recorded.
- Exact VPN/Process/docs and XY-C1 verification for this combined correction remain pending. No local MIPS/uClibc compile or physical router L4 is claimed.

---

## 2026-11-23 — Remove duplicate WAN route path filters

- WAN route #45243513566 on source SHA `d3772d6` failed at the existing `check_wan_vlan_setup.py` contract because `trunk/user/shared/src/netutils.c` appeared twice under both push and pull_request. The original workflow already had the necessary single entry for this shared source.
- Restored the original workflow path file (one netutils entry per event); kept the new EINVAL / ioctl-close source contracts in `check_wan_detect_wan_status.py`. This change only repairs CI filters and does not revert the production errno fix.
- Parallel commits `36861ae` (bridge radio selector fail-closed policy) and `eb92a7a` (exact-source CI gates) landed on main after `d3772d6`; they are preserved unchanged. Exact-head WAN route rerun pending.

---

## 2026-11-23 — Preserve netutils interface-address errno

- Follow-up inspection found a gap in the WAN local-address error separation: `copy_ifname()` returned `-1` for null/empty/overlong interface names without setting errno, so a caller that clears errno could still classify a failed interface-name copy as an ordinary unassigned address.
- `get_interface_addr4()` now marks invalid interface names as `EINVAL`, preserves `SIOCGIFADDR` errors across `close()`, and surfaces a close error when the address lookup itself returned no usable address. A valid IP address is retained even if closing the temporary socket later fails.
- Extended the WAN errno fixture with EINVAL and added an exact source contract for the shared address helper; the WAN route workflow now triggers on changes to `netutils.c`. XY-C1 already tracks this source. Exact-head WAN route, L2, Process, docs and XY-C1 Build/TRX/artifact verification pending. No local compile is claimed.

---

## 2026-11-23 — Repair exact-source WAN/Process integration assertions

- Current source SHA `bd56c02f7f8d10374c6857f8aac1d56b1f7a4b6e`: WAN ARP L2 #45241824790 PASS, WAN DHCP callbacks #45241824847 PASS, Process inventory #45241824430 PASS, and AI handoff #45241824655 PASS.
- Two umbrella checks exposed stale contracts rather than source/test failures. WAN route #45241824836 reached `check_wan_detect_wan_status.py` and demanded a retired `inventory_error ? inventory_error : EIO` expression; the policy correctly rejects non-positive errno values with `inventory_error > 0 ? inventory_error : EIO`, already covered by its negative-value regression. Process fail-closed #45241824549 reached the legacy `get_wan_ip4()` assertion and rejected the new explicit `if (status != 0) { errno = status; ... }` failure path.
- Updated both source contracts to assert the current fail-closed semantics rather than restore older code. These are checker-only changes; exact-head WAN route and Process reruns pending.
- XY-C1 #45241824748 was still building at last poll; Build/TRX/artifact result must be confirmed before claiming L3. NVRAM #45241824746 was still running. No local test or firmware build is claimed.

---

## 2026-11-23 — Preserve WAN interface-address lookup failures

- Found a real health-monitor error collapse in `detect_wan.c`: both `chk_udhcpc()` and `arpping_gateway()` treated `INADDR_ANY` from `get_wan_ip4()/get_lan_ip4()` as a normal “no address” state. But `get_interface_addr4()` also returns `INADDR_ANY` when socket creation, interface-name validation or `SIOCGIFADDR` fails. That could make a local lookup failure look like a healthy gateway response and suppress DHCP recovery without reporting the probe as a local error.
- Added an errno-aware policy that keeps a genuinely absent address (including Linux `EADDRNOTAVAIL`) distinct from other lookup errors; clear/capture errno immediately around both local address reads before later NVRAM calls can overwrite it. `get_wan_ip4()` now stores the AP-mode lookup status in errno rather than silently folding it into the no-address result.
- Added GNU89 regression coverage for missing address versus EACCES/EIO/invalid status and an exact call-order/source contract in the WAN workflow. Exact-head sanitizer, Process, AI docs and XY-C1 Build/TRX/artifact checks are pending. No local compiler run is claimed.

---

## 2026-11-23 — Restore Process workflow path-set symmetry

- Exact Process umbrella #45239229205 on `79f4d535af499995fc1f2be1d24b52b93a35d2f4` completed SUCCESS. The shared LLDT parser integration contract, process scanner, PID identity, lifecycle, DHCP, firewall, storage and other regression steps all passed.
- AI handoff validation #45239229201 on the same SHA failed because the Process workflow's push paths included `lldt_config_parse_policy.h`, but its pull-request paths did not. The parser header is now included in both path sets; this is a workflow filter mismatch, not a source or test failure. Exact-head AI docs rerun pending.
- LLDT parser L2 #45238664960 and AI docs #45238664839 passed on `38d3732`. XY-C1 run #45237411963 on the production source SHA `eef4417` completed successfully: Build XY-C1, TRX verification and artifact upload all PASS.

---

## 2026-11-23 — Align process integration contract with the shared LLDT parser

- Exact LLDT parser regression #45238183801 on `d0e68ad` passed GNU89/Werror/ASan/UBSan compile and all runtime fixture assertions, then exposed only an over-strict LLDT checker expectation about the XY-C1 workflow's one push-only path filter. That expectation was corrected in `38d3732`.
- The same `d0e68ad` Process integration run #45238182167 then exposed a stale pre-refactor LLDT assertion requiring the old inline `sscanf`/`memchr` parser. Updated the integration contract to verify the shared bounded parser, embedded-NUL rejection, token-capacity checks, checked icon path copy and new workflow header triggers. Process regression and checker must pass on a fresh exact SHA.

---

## 2026-11-23 — Scope XY-C1 source-trigger assertion to its push filter

- The corrected parser fixture on `d0e68ad` compiled and all GNU89/Werror/ASan/UBSan regression cases passed (#45238183801, compile/run step PASS). The source checker then failed because it required the production header to appear twice in XY-C1 workflow filters, while this firmware-only workflow intentionally has a single `push.paths` filter and no pull-request trigger.
- Relaxed that source contract to require the production header at least once in the XY-C1 build-trigger filter. The dedicated LLDT parser workflow still independently requires both push and pull-request path entries. Fresh exact-head LLDT and Process Actions are the acceptance gate.

---

## 2026-11-23 — Correct LLDT test fixture line-ending escapes

- The first fixture correction on `14be2b0` changed double-escaped NUL character constants, so GNU89 compilation completed. The rerun then exposed the same quoting mistake in seven C string fixtures: `\\\\n` / `\\\\r` produced literal backslash characters, and the first parsed value assertion failed (#45237898307, job #137914412295).
- Converted those string tokens to actual C newline/CR escapes. The source parser has not failed its logic checks yet; rerun the complete ASan/UBSan fixture and source contract before acceptance.

---

## 2026-11-23 — Fix LLDT regression fixture NUL character constants

- Exact LLDT parser run #45237411685 and Process umbrella run #45237411877 failed at the GNU89/Werror compile step. The new C regression fixture emitted the two-character source spelling `'\\0'` (multi-character constant) instead of the single NUL escape `'\0'`; the compiler identified lines 56, 86, 92 and 98. The production parser implementation was not implicated.
- Corrected all seven affected character literals to standard C NUL character constants. Exact-head LLDT, Process and XY-C1 checks must be rerun before claiming L2 or L3.

---

## 2026-11-23 — Bound LLDT conformance-tool configuration parsing

- The shipped LLDT daemon's configuration parser had already been bounded, but the optional `lld2test` conformance utility retained an unbounded `sscanf("%s")`, a non-terminated `strncpy` icon path, and overflow-prone `atoi` option parsing.
- Added a shared length-aware assignment parser with embedded-NUL and token-capacity rejection, plus strict bounded decimal parsing. Both `lld2d` and `lld2test` now share the checked icon-path copy; the utility's pause/response options retain safe legacy fallback values on malformed or out-of-range input.
- Added GNU89 ASan/UBSan boundary regressions, an exact-source checker, a dedicated LLDT config workflow, and the production header's XY-C1 input trigger. Exact-head L2 and firmware validation are pending; `ctmain.c` belongs to optional `lld2test`, not the ROMFS-installed daemon.

---

## 2026-11-23 — Correct handoff headings rejected by exact documentation contract

- AI handoff run #45235046184 on `5a43b187` failed because `SESSION_HANDOFF.md` used `### Repository invariants`, `### Active task — ...` and `### Verification boundary` instead of the required top-level `## Current repository state`, `## Active task` and `## Verification boundary`. It also used “the document edit”, while the validator requires the exact clause “a document edit”.
- Restored the literal required headings and checkpoint wording, refreshed the roadmap with the confirmed RStats #45234873403 PASS, Shadowsocks #45235046114 PASS and Process #45235046275 PASS statuses, and retained the outstanding same-SHA XY-C1 limitation. The exact documentation rerun for this commit is pending.
- Parent main: 5a43b18722b9d2f3f864ef6bd1025084ff1edb85. This is a correction of an observed CI contract failure, not routine documentation synchronization.

---
## 2026-11-23 — Cover missing DNS proxy executable without tearing down the current service

- The DNS preflight checks `command -v` before stop, but the host regression suite had no missing-binary case. Added an isolated empty `PATH` fixture: a valid backend/upstream with no `dnsproxy` executable must return 127 and leave the stop/start stubs untouched. The test restores PATH before using external assertion helpers.
- The Shadowsocks Python source contract now requires this negative case. Production code is unchanged; exact-head Shadowsocks and AI handoff Actions are the acceptance gate.

---
## 2026-11-23 — Preserve RStats history view on selector read failure

- `save_history_json()` still used `nvram_safe_get(RSTATS_NVKEY_DM)` after the speed selector had moved to checked NVRAM access. A `/dev/nvram` open/ioctl/close failure could collapse to an empty string and silently publish history index 0 under the wrong interface label.
- The history exporter now uses `nvram_get_checked()`, immediately copies the shared-cache value into a bounded local buffer, preserves the historical empty/default selection when the key is absent, and returns before opening an output stream on backend, NULL-present or overlong values. The last published JSON is retained on UNKNOWN selector state.
- Added GNU89 policy coverage for present, missing, backend `EACCES`, NULL, overlong and invalid destination inputs; extended the RStats workflow to require checked read → copy → UNKNOWN gate → atomic output open. Parent main: f3acf67d58203ae7d18b6d2755014840ba10cccc. Exact-head RStats/Process/AI/XY-C1 Actions pending; no local compilation or firmware result is claimed.

---
## 2026-11-23 — Reject whitespace-corrupted DNS upstreams before proxy teardown

- The DNS preflight accepted whitespace-only values (for example `tunnel_forward='   #53'`) and values containing embedded whitespace. They passed the old non-empty/`nil` check, so `ss_restart_dns_proxy()` could stop the healthy local DNS daemon before the replacement was guaranteed to be usable.
- `ss_dns_proxy_config_read()` now rejects any whitespace in the upstream token before publishing the cached snapshot. The production-linked shell test covers whitespace-only and embedded-whitespace values and asserts that neither the stop nor start function is called. The Python source contract requires the whitespace gate and both negative cases.
- Parent `7fdf985` exact Shadowsocks run #45233294870 passed the complete DNS preflight, startup-status and process-lifecycle suites after the previous dispatcher-contract fix. This new validation slice is not yet verified; exact-head Actions are pending.

---
## 2026-11-23 — Update DNS readiness contract for preflight/launcher split

- Exact 7a48 Shadowsocks #45232308218: shell syntax and all host regressions passed, including the repaired direct process-lifecycle fixtures. The Python source checker then failed because its existing readiness assertion sliced only `start_dns_proxy()` but still expected the executable `command -v` check there; that check now correctly lives in the preceding `ss_dns_proxy_config_read()` preflight.
- Corrected the source-contract slice to cover the ordered preflight → launcher → restart wrapper block. The existing ordered assertions still require executable validation before the backend launch, bounded readiness observation, cleanup on launch failure, and propagation through the stop/start wrapper. No production code changed in this correction.
- Exact 7a48 AI handoff and Process inventory workflows passed. XY-C1 validation remained a separate pending run; rerun the exact-head Shadowsocks and Process Actions for this checker correction.

---
## 2026-11-23 — Align direct DNS process lifecycle fixtures with preflight snapshots

- Exact DC8 Shadowsocks run #45231946878: shell syntax, startup status regression, DNSMasq cleanup regression and the new DNS proxy preflight regression all PASS. The subsequent existing process-lifecycle test FAILED because it extracted `start_dns_proxy()` and invoked it directly without first sourcing/calling the new `ss_dns_proxy_config_read()` helper; under `set -u`, the missing snapshot correctly failed closed (`SS_DNS_PROXY_PROCESS: parameter not set`).
- Updated that production-linked lifecycle harness to extract the preflight helper and validate both dnsproxy/dns2tcp fixtures before launch. Empty-upstream and unsupported-backend negative cases now assert the preflight function rejects configuration before any launcher invocation. Added a source-contract check to prevent direct-launch tests bypassing the snapshot.
- Rerun the complete exact-head Shadowsocks workflow after this correction; the initial DC8 AI handoff contract and Process inventory workflows passed. XY-C1 #45231946608 remained in progress during the failure diagnosis.

---
## 2026-11-23 — Preflight DNS proxy replacement before stopping the live service

- `start_dns()` stopped `dnsproxy`/`dns2tcp` before `start_dns_proxy()` read `pdnsd_enable`, `tunnel_forward` and checked the selected executable. A transient NVRAM read failure, empty upstream, invalid selector or missing binary could therefore take down a working local DNS upstream before the replacement was rejected.
- Added a validated configuration snapshot for selector, upstream and executable, then routed router/gfw restarts through preflight → stop → start. The launcher uses the validated snapshot, so validation failures return without touching the current daemon and a stale NVRAM re-read cannot change the launch target after teardown.
- Added production-linked `/bin/sh` regression cases for selector/upstream read errors, nil upstream, invalid selector, successful dns2tcp selection, and stop/start error propagation, plus Python source-order assertions. Exact-head Shadowsocks and XY-C1 checks remain the acceptance gate; no live-router behavior is claimed.

---
## 2026-11-23 — Bind AliyunDrive process state to its executable inode

- BusyBox 1.24 pidof accepts a process when its comm or basename of argv[0] matches the requested name. A different executable can therefore spoof aliyundrive-webdav; the previous tri-state wrapper treated that result as RUNNING and could send TERM/KILL to the wrong process during stop.
- ald_process_state() now resolves the configured daemon and every /proc/PID/exe candidate with stat -L -c '%d:%i'. A missing, malformed or mismatched identity returns UNKNOWN (2), which blocks start/stop signalling; only candidates whose device/inode matches the packaged executable are RUNNING. The existing no-cached-PID and bounded stop policy is unchanged.
- The lifecycle fixture mocks only these executable-identity stat calls and retains real stat behavior for lock/workdir checks. Added mismatch, inaccessible and malformed identity cases, including no-signal and no-duplicate-start assertions; the Python source contract pins the inode gate. Parent main: 53c6f6623e5531deeb889762e81bf1943c1abf50. Exact-head AliyunDrive lifecycle and AI handoff Actions are pending for this source slice. No local shell execution or firmware validation is claimed.

---
## 2026-11-23 — Repair stale DNSMasq split-DNS test contract

- Historical exact DNSMasq run #45110039566 on `d36ba24` compiled and executed the GNU89/ASan/UBSan NVRAM policy regression successfully, then failed in `check_dnsmasq_integer_options.py` because it required the helper `dnsmasq_split_dns_snapshot_is_zero` to also appear as a standalone `helper();` invocation. The test suite intentionally defines this helper to validate public snapshot fields without relying on struct padding, and its error-path tests call it with `&snapshot`.
- Corrected the checker to extract the helper body and require the field-zero predicate plus an actual `assert(dnsmasq_split_dns_snapshot_is_zero(&snapshot));` call. This removes the false negative without weakening the failure-path test contract or changing production code. Exact-head DNSMasq and XY-C1 Actions are required for this current checkpoint.

---
## 2026-11-23 — Repair the backup-server (`reserver`) startup path

- `ressp()` called `start_redir`, which is not defined in `shadowsocks.sh`; the actual TCP proxy was therefore not launched. Its next `start_rules $BACKUP_SERVER` passed an ignored argument to a helper that reads `GLOBAL_SERVER`, leaving firewall/config generation on the primary node. Stage errors were ignored and the function always printed “backup server started successfully”.
- The backup path now rejects unreadable/empty/`nil` selectors, switches process-local `GLOBAL_SERVER` to the selected backup, and follows the primary startup sequence: firewall, cgroup setup, TCP, optional UDP, DNS, configured local SOCKS5 and watchcat. It preserves the first required error while keeping cron best-effort; the `reserver` CLI propagates failure.
- Extended the production-linked startup fixture and Python source contract for backup node identity, `same` UDP/local mapping, invalid/unreadable configuration, TCP/UDP dependency gates and optional cron failure. Exact-head Actions are required before calling this slice verified; no router failover is claimed.

---
## 2026-11-23 — Revalidate process identity across PID reuse

- `proc_read_identity()` previously read `/proc/PID/stat` and `cmdline` once each. If the process exited and its PID was recycled between those reads, the scanner could combine the old process's matching `comm` with the replacement's unrelated command line, incorrectly report RUNNING and suppress a daemon restart.
- Candidate matches now re-read and parse stat before returning RUNNING. Disappearance/zombie is ABSENT; malformed or inaccessible stat remains UNKNOWN; changed start time/comm/truncation is fail-closed UNKNOWN/EAGAIN. The extra read occurs only after a candidate match.
- Added `test_pid_reuse_between_stat_and_cmdline_is_unknown()`: the fake procfs reader replaces PID 123 between reads, and the test requires UNKNOWN/EAGAIN plus an empty snapshot. The Process regression path already tracks both source and test files; XY-C1 tracks the shared source.
- Parent main: `8aa5ef84403044d2f81160d3fc259384c67c4c60`. Exact-SHA GNU89 host regression, process source audit and XY-C1 build/TRX/artifact checks are pending. No local build is claimed.

---

## 2026-11-23 — Anchor p910nd launch failure contract and restore handoff rule

- Exact e52 USB modem #45225619186 PASS, including the p910nd GNU89/Werror/ASan/UBSan mapping fixture and USB NVRAM/source contract. Exact e52 Storage #45225619727 PASS.
- Exact e52 Process fail-closed #45225619509 FAIL: `launch_error_gate` was found with an unanchored `index()`, so the assertion selected the earlier invalid-device input error branch, not the launcher failure branch. The Process workflow now searches for the error gate starting at the `eval()` launch.
- Exact e52 AI handoff #45225619139 FAIL because `SESSION_HANDOFF.md` did not include the exact required phrase `one atomic main commit`; restored it while retaining one full Git tree and expected-SHA non-forced update.
- A documentation-only continuation commit `0e9eb13c56c6f83fe75fbd8261eedaaef6ff154c` added the previous cross-session task summary but left those two exact failures unresolved. This atomic checkpoint corrects them. XY-C1 #45225109549 on e6a6 was still building; TRX/artifact pending. No L3 or hardware result claimed.
## 2026-11-23 — Normalize p910nd rollback contracts and test invocation

- Exact USB modem run #45225109809 on source SHA `e6a6a3868a0e409a009ede9b01d01c355c372b15` passed the WAN/modem/firewall contracts, then failed in `check_usb_printer_nvram.py` because the checker expected `marker();` while the GNU89 test invokes each case through `CHECK(test_name() == 0)`. The production test was correctly wired; the checker now verifies that actual invocation form.
- The same checker and Process/Storage workflows previously encoded callback formatting or broad character-window assumptions. They now compact whitespace and explicitly assert `usb_printer_daemon_start_failure_recover(process_name,start_status)`, ensuring the port-specific identity is passed without relying on indentation.
- The e6a6 source commit fixes the substantive nonzero-port rollback bug by passing the derived `process_name` rather than literal `p9100d`. USB modem #45225109809 is the checker failure described above; Process #45225109772, Storage #45225109612, process inventory #45225109604, NVRAM #45225109574 and XY-C1 #45225109549 were in progress at the last poll. AI docs #45225109551 PASS. Exact reruns of these checker corrections are pending.
- No local test, MIPS firmware build, TRX, artifact or hardware printer result is claimed.

---

## 2026-11-23 — Keep p910nd readiness rollback scoped to the selected printer

- The first exact-SHA USB workflow caught a source-contract ordering assertion error; review of the same source also found that the new per-port readiness path still passed the old literal `p9100d` to the rollback helper. That could stop the wrong daemon when a non-zero port failed readiness.
- Changed rollback to use the same validated `process_name` used by readiness. Corrected the checker ordering to reflect that port parsing/name construction precedes the port-specific launch. Process workflow now explicitly guards the per-port rollback target.
- Initial p910nd test compile and full exact-head XY-C1/Process/Storage results are being checked on successor SHA.

## 2026-11-23 — Correct p910nd per-printer port and process identity

- Fixed a concrete multi-printer defect: the hotplug loop scans `/dev/usb/lp0` through `lp9`, but every p910nd launch passed port `0`. p910nd uses that digit for its TCP port, lock/PID filename, and rewritten process identity (`p9100d` through `p9109d`), so multiple printers collided and readiness for later devices could be falsely satisfied by the first daemon.
- Startup now strictly parses the canonical `/dev/usb/lpN` path, passes that same digit to p910nd, derives the matching rewritten process name, and performs readiness plus failure rollback against that specific instance. Stop now targets the launcher name and all ten rewritten daemon identities.
- Added a GNU89 ASan/UBSan policy regression for the complete 0–9 map, malformed paths, invalid ports, and non-mutating error paths. USB, Process and Storage workflow source contracts and path triggers are updated. Exact-head Actions pending; hardware multi-printer runtime remains unverified.

## 2026-11-23 — Roll back late p910nd children after readiness failure

- `start_p910nd()` previously returned a readiness timeout or process-inventory error after `p910nd` had already detached its child. The caller observed only the launcher parent, so an incompletely initialized listener could remain running and be retried later.
- The readiness failure path now invokes the shared identity-validated rollback with process name `p9100d` (the child’s rewritten argv[0]), then re-scans process state while preserving the original failure status. This is limited to post-launch/readiness failures; a confirmed launcher failure still returns directly.
- Extended the USB modem, Process and Storage source contracts to require the `p9100d` rollback after the readiness gate. The existing GNU89 detached-daemon policy regression covers cleanup outcomes and primary-error preservation; exact-head Actions are required before treating this slice as verified.

## 2026-11-23 — Align Storage workflow with USB printer rollback

- Storage hotplug run #45221949175 exposed a stale source assertion left over from the pre-rollback launcher: it required a direct `return EIO;` after a partial exec-status write. The implementation now invokes the shared failed-start rollback policy for that uncertain state.
- Updated the Storage source contract to check the rollback hand-off and the dedicated stop/re-scan callbacks rather than the removed direct return. This was a test-contract mismatch, not a runtime storage failure.

## 2026-11-23 — Correct Process workflow rollback callback scope

- Process run #45221949004 failed only because the new source-contract snippet searched for `kill_services()` inside the recovery function, although the call intentionally lives in its separate stop callback. The implementation and USB modem workflow checker both passed.
- Updated the workflow to inspect the stop callback and recovery function independently. No production code changed; rerun on the successor SHA is required.

## 2026-11-23 — Roll back late USB printer daemons after failed readiness

- Follow-up reliability defect: when U2EC/LPRng `execv()` succeeded but the bounded RUNNING postcondition timed out, was interrupted, or encountered an unknown process inventory, the caller returned failure without stopping a daemon that could appear late. A pipe read/partial-status error also left launch state uncertain.
- The printer launcher now uses the shared `detached_daemon_start_rollback_with_ops()` policy on readiness and post-fork status-channel failures. It sends an identity-validated stop through `kill_services()`, takes a fresh process snapshot, logs confirmed absence versus incomplete rollback, and preserves the primary startup errno. A definitive full exec-error record still returns directly because no daemon was started.
- The existing GNU89 rollback-policy regression already covers successful no-op, stop, final ABSENT/RUNNING/UNKNOWN and primary-error preservation; USB-printer and Process source contracts now pin this integration. Exact-head Actions pending.

## 2026-11-24 01:00 CST (UTC+8) — Restore mandatory handoff accuracy checkpoint after AI docs failure

- Exact AI documentation run #45221089290 on `e4d34fd7f13baa203bbbae90130b0782a9b22e5c` failed with: “current handoff does not require an end-of-session accuracy checkpoint”. The handoff had been compacted but omitted the validator-required `Mandatory handoff checkpoint` section and exact conditional-edit sentence, along with its required cross-session heading and section names.
- THIS commit restores the mandatory checkpoint contract, including “The checkpoint is mandatory; a document edit is conditional.”, live-state inspection, materiality/no-op guidance, cross-session atomic-main rules, and the four required handoff section headings. No product source changed.
- USB modem checker PASS remains #45220509149 on f947; Process, NVRAM and Storage PASS remain #45220191260/#45220191315/#45220191366 on ecff. Exact AI docs rerun on THIS handoff correction is pending; XY-C1 runs #45220033426 (e6dc), #45220191233 (ecff), and #45221089272 (e4d34) were still in progress at last observation.

---
