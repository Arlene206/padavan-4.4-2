## 2026-11-24 — Fix Adbyby process inventory source-checker scope

- The Adbyby stop test was repaired in two sequential test-only commits: `c0afda5` moved the per-PID TERM mock out of the shell builtin namespace, and `3462bca` exported `ADBYBY_TEST_PID_OUTPUT` before invoking the external mocked `pidof`, so its `123 456` multi-PID case actually reaches both validated PIDs.
- On exact SHA `3462bcacba47114b93d3d9da0366513064c13dbb`, all Adbyby shell regressions, including the multi-PID TERM case, passed. The workflow then failed only in `scripts/check_adbyby_hosts_config.py`: the pidof assignment/status contract was being searched in `adbyby_process_state()`, while the checked command substitution correctly lives in `adbyby_process_snapshot()`.
- Updated the checker to validate the actual function which owns the checked read. Production Adbyby code is unchanged in this slice. Exact-head Adbyby and Process workflow reruns are required.
- Parent main: `3462bcacba47114b93d3d9da0366513064c13dbb`; no firmware, MIPS build or hardware result claimed for this checker-only correction.

---

## 2026-11-24 — Keep SmartDNS shutdown independent of configuration NVRAM reads

- `smartdns.sh` performed five checked configuration reads (`sdns_enable`, `sdns_port`, `sdns_redirect`, `sdnse_enable`, `sdnse_port`) before dispatching every action. A transient NVRAM backend error therefore exited before an explicit `stop` could acquire the lifecycle lock, load its durable `smartdns_conf.ini` resolver snapshot, remove DNSMasq's managed override, reload DNSMasq or stop the daemon.
- Added `smartdns_load_lifecycle_nvram_state()`: explicit `stop` makes no configuration-NVRAM calls, selects the local disabled cleanup path and relies on `Read_ini`'s saved ownership/port/redirect snapshot. Start/restart/reset retain checked, ordered reads and strict selector rejection with original error-status propagation.
- Added a shell regression for all five independent read failures, stop's zero-read path, valid snapshot loading and malformed-selector rejection; wired it into both push and pull-request DNSMasq lifecycle triggers and the source checker.
- Parent main: `08d59c49ef1080079830e66302516747e06cf82f`. This source/test milestone is THIS COMMIT; exact-head DNSMasq/Process/XY-C1 Actions must be checked after publication. No local MIPS build, live DNS test or hardware L4 result is claimed.

---

## 2026-11-24 — Align the shared stop regression with Transmission's per-PID SIGTERM

- Exact-head USB storage workflow #45336245210 failed only in the shared Transmission/Aria launcher lifecycle regression; the preceding storage C and HDD/spindown checks passed.
- Root cause was a stale test double, not a production-source failure: `Transmission.func_stop()` now sends `kill -TERM <validated-pid>`, but `storage_app_start_status.sh`'s shared `kill()` stub accepted only `-9`. The graceful fixture therefore left PIDs alive and incorrectly drove the stop path into SIGKILL.
- Updated the fixture to record/clear validated per-PID SIGTERM separately from SIGKILL, while preserving Aria2's existing killall graceful-stop contract and post-signal PID revalidation. The first exact-head correction run must confirm the shared storage workflow.
- Parent main: `2d87291cd72a25f90d58d12baea545e01609e4c7`. No firmware source changed in this test-only slice; no new firmware build or hardware result is claimed.

---

## 2026-11-24 — Validate service selectors across userland and repair exact-head regression contracts

- Fixed the Autostart lifecycle test's missing `fail()` helper, then corrected its concurrent-publish fixture so the shell writes a real newline. Process lifecycle Actions #45332907096 and #45332907032 passed on `6bc7b0665719835a6f45e54c3dfd13e3516f6b73`.
- AliyunDrive WebDAV startup now treats only explicit disabled values as a stop request; corrupt selectors fail without stopping or relaunching a healthy daemon. Its lifecycle and Process regressions passed on `5cfb70929934a800f867a7e21179e11fafb9d86c`.
- LAN DHCP now validates `lan_dns_x` as a binary selector in both automatic resolver publication and lease deconfig, before route/resolver side effects. GNU89 policy coverage includes valid values and integer boundaries; WAN DHCP callback stability #45333806032 passed on `d0c7bb76c70f23b18046719d1d8ede572dfca05b`.
- AliDDNS now rejects corrupt startup/runtime enable values without converting them to a stop or terminating its keeper; exact lifecycle #45334063814 and Process #45334063878 passed on `248e045b1526868c9324485bfb18234ca268c7cf`.
- DDNSTO now validates checked `ddnsto_enable` before start/restart, leaving an active daemon untouched on invalid values. Its first regression run exposed a fixture that left enable=0 before later launch tests; the fixture was restored to 1. Exact lifecycle #45334345903 and Process #45334345858 passed on `10ca6cb384a9f83864216f5302d5f44ea59fd0cf`.
- AdGuardHome restart now preflights checked enable-state and accepts only empty/0/1 before stopping the daemon. Exact config-security #45334530232 and Process #45334530159 passed on `e3d7f4b0f347f01da2cf4a84e58bee511934ea41`.
- Adbyby now rejects malformed enable selectors before lifecycle mutations and preserves the original status from each checked NVRAM read. Its regression failures exposed a multiline awk extraction mismatch, then the lost NVRAM status, then a checker-only IndentationError; each was fixed atomically. Final Adbyby regression #45335321139 passed on `ae615df132c429ee58373bfa248be2df4d1e828d`.
- SmartDNS now validates its binary UI enable selector before start/restart/reset while leaving stop usable for shutdown. DNSMasq lifecycle #45335248129, Process inventory #45335248118 and AI handoff #45335248114 passed on production SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8`; USB storage hotplug #45335248143 also passed there.
- Latest production-source SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8` XY-C1 run #45335248142 was IN_PROGRESS at checkpoint: Build running, TRX validation and artifact upload pending. Do not infer L3 from older firmware runs. No local target compile or physical-router L4 verification is claimed.
- The subsequent `ae615df` commit is checker-only (Adbyby source-contract syntax); no product source changed after `a89ebc`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains untouched.
- Next: poll #45335248142's Build, TRX and artifact steps; then continue an independent, evidence-backed stability/security fix while pending checks run.

---

## 2026-11-24 — Preserve the pre-PM GE2 register-map failure boundary

- Switch CLI #45326822109 on parent 49c37472f8e6c1c630382869ef0bbb4b12d492a7 failed at its existing inline contract: the GE2 temporary sysctl ioremap is required to occur before pm_runtime_enable, and the mapping-failure branch must return -ENOMEM before acquiring PM state. The clock helper now correctly unwinds later failures, but the temporary mapping's location had changed that tested boundary.
- Moved the GE2 register mapping/program/unmap block to the top of ei_clock_enable, before PM-runtime and any clock prepare operations. The mapping failure again returns -ENOMEM immediately; once the map succeeds it is unmapped before any later failure edge. Clock and PM partial-failure rollback remains unchanged.
- Exact parent outcomes: AI docs #45326822074 PASS; Switch CLI #45326822109 FAIL at the described source contract; NVRAM #45326822151 IN_PROGRESS; XY-C1 #45326822049 IN_PROGRESS (IAPP firewall lifecycle validation). The correction requires a new exact-head Switch CLI and XY-C1 build/TRX/artifact run.
- Next: inspect exact-head actions, then advance outside RAETH while pending jobs run.

---

## 2026-11-23 — Make RAETH clock preparation and SGMII teardown transactional

- ei_clock_enable ignored every clk_prepare_enable return value and continued after TRGPLL clk_set_rate failure. Probe only logged devm_clk_get error pointers, so a required missing/deferred clock could reach the clock API unchecked; later failure had no tracked partial unwind.
- ei_clock_disable unconditionally disabled clock handles instead of pairing only successful enables. It also omitted Leopard MTK_CLK_SGMII_TOP and all four GE2 MTK_CLK_SGMII1_* clocks. If GE1 and GE2 SGMII were both selected, the shared SGMII PLL could be prepared twice but disabled once.
- Added a successful-enable bitmap and per-clock helpers. Required error pointers now propagate, NULL handles remain a no-op for MT7621's deliberately skipped clock acquisition, duplicate shared PLL enables are suppressed, and every partial failure unwinds enabled clocks plus runtime-PM usage/enable state. TRGPLL set-rate failure is fatal to setup. Teardown releases both SGMII groups and the top clock exactly once.
- Added source-contract checks for ERR_PTR handling, success-only bit tracking, PM-runtime rollback, failure unwind and SGMII disable coverage. No local target compile is claimed; exact Actions on this commit must validate GNU89/MIPS integration.
- Parent main: 3ea3089230ea2696e518c525ec6cf1132aa1ea83. Exact parent Actions #45326081842 AI docs, #45326081376 Switch CLI and #45326081336 NVRAM PASS. XY-C1 #45326081305 had completed host checks but Build was still in progress; TRX/artifact pending. Previous completed firmware Build/TRX/artifact on f7bd670 predates later source changes.
- Next: inspect exact-head checks, then move to an independent userland stability/security defect outside RAETH.

---

## 2026-11-24 — Close RAETH switch-node and primary-netdev teardown ownership

- `rather_probe()` now attaches the registered primary netdev to the platform device. Its error path releases `switch_np`, shuts down debug proc state only after successful proc initialization, releases RX/TX descriptor resources before putting a separately allocated QDMA helper device, and never puts the platform-owned device itself.
- `ei_uninit()` now drops the parsed switch phandle after close/uninit and clears the pointer. `raether_remove()` retrieves the netdev from drvdata, unregisters it first (thereby running ndo_stop/ndo_uninit), releases descriptor state, puts only the allocated QDMA helper device, disables clocks, unmaps MAC/sysctl regions, clears wakeup/drvdata/global pointers and frees the netdev.
- This closes the platform driver remove leak: the old remove path neither unregistered/freed the primary netdev nor released descriptor/MMIO state, and directly invoked the QDMA helper device's release callback instead of dropping its reference with `platform_device_put()`.
- Parent main: `d3ae666f5e46809610e19919fa16b3eb874e68ac`. The earlier preinit slice `615d6be` had AI docs PASS; its Switch CLI failure was caused by an overly broad test assertion, corrected on `65103b60` where Switch CLI #45325386250 **PASS**. NVRAM #45325334304 **PASS**; XY-C1 #45325334328 remained pending at last poll. Current source verification is required on the new head.
- Next source task: prove and cover reverse-order cleanup for all RAETH `rather_probe()` failure labels, especially clocks, mappings and helper-device stages.

---

## 2026-11-24 — Balance MTK GSW probe resources

- Audited `mtk_gsw_probe()`: `irq_of_parse_and_map()` returns unsigned zero on failure, but the old code compared it with < 0; successfully mapped IRQs were not disposed on later probe errors or remove. The pctl phandle was not released, the code tested the phandle instead of the returned regmap for IS_ERR, and pinctrl_get's ERR_PTR result was tested as a boolean while its successful non-devm handle was never put.
- Probe now validates IRQ zero, checks the pctl node for NULL, drops its reference after resolving the syscon regmap, checks the regmap/pinctrl handles correctly, and sends all post-map failures through a common pinctrl/IRQ cleanup path. Remove releases both retained resources. Added source-contract coverage for each ownership edge.
- Parent main: `65103b60d8267e769efbc7aff5395f2662a7d83e`. Preinit source commit `615d6be0f8db31905bda066be38804490b02e45f` AI docs #45325334351 **PASS**; original Switch CLI #45325334356 exposed a checker-only range assertion error. Checker correction `65103b60d8267e769efbc7aff5395f2662a7d83e` Switch CLI #45325386250 **PASS**. NVRAM #45325334304 and XY-C1 #45325334328 were still PENDING at the last poll.
- Earlier production source `f7bd670c88520298c8c4628b1ff06e0c95251f12` had Switch CLI, NVRAM and XY-C1 Build/TRX/artifact **PASS**. No L4 hardware verification.
- Next source task: trace `switch_np` phandle ownership and primary RAETH netdev teardown across `rather_probe()`, `ei_uninit()`, `raether_remove()` and failed `register_netdev()`.

---

## 2026-11-24 — Pair MT7622 switch regulator state and release lookup references

- MT7622 `fe_sw_preinit()` called `of_find_device_by_node()` without checking the NULL result, dereferenced missing platform drvdata, and never released the device reference. It also ignored voltage/enable failures and continued opening the netdev; `fe_sw_deinit()` unconditionally disabled rails even when the matching enable failed or never ran.
- Made preinit return an error, validate node/device/driver-data, and release the held device reference through one exit. Main and MCM B3V regulator state is recorded only after successful enable; voltage/enable/GPIO failures stop open and roll back enabled rails. A failed rollback remains recorded so deinit can retry. Shutdown disables only rails whose enable succeeded and clears state only after successful disable.
- `ei_open()` now treats preinit failure as an open failure: reset FE, release DMA, retry outstanding rail cleanup, release module reference and return the original error. The RAETH checker asserts pdev-reference, regulator and GPIO failure ordering plus caller unwind.
- Parent main: `8f141576497dd6c04938cf89cd95bcc5b29de333`. Exact previous production-source SHA `f7bd670c88520298c8c4628b1ff06e0c95251f12`: Switch CLI #45323341869, NVRAM #45323341860 and XY-C1 #45323341920 **PASS**; the firmware job completed Build, TRX validation and artifact upload. No hardware L4 claim.
- AI docs #45324509191 **PASS** on `81b978`. The later #45324629954 on `8f14157` failed because a duplicated Roadmap snapshot pushed the file to 132155 bytes; this commit removes the duplicate snapshot append from the Roadmap, handoff and root log and keeps the module status concise. Exact-head AI documentation check is required.
- Next source task: audit `mtk_gsw_probe()` resource ownership for pinctrl references, IRQ mappings and parsed DT nodes.

---

## 2026-11-24 — Rotate the root development log and refresh RAETH handoff

- AI handoff run #45323806827 on `7cd1ffd3` advanced beyond the Roadmap-size check, then failed because root `dev_log.md` contained 122 complete dated records; the validator requires the 20–119 rolling window.
- Archived the 102 oldest complete records verbatim, oldest first, to `docs/dev-log/2026/11/part-031.md`; the root retains its newest 20 pre-existing records plus this rotation checkpoint (21 total). Updated the index with exact size and Git blob SHA; no history was rewritten or dropped.
- Parent main: `7cd1ffd3eecc05786846be684053428f20e8f535`. Process lifecycle #45323579958 PASS on `24d204e`; AI handoff #45323806827 failed only at the log-retention check. RAETH Switch CLI #45323341869 and NVRAM #45323341860 PASS on source `f7bd670c88520298c8c4628b1ff06e0c95251f12`; XY-C1 #45323341920 remains IN_PROGRESS. No same-SHA TRX/artifact or hardware L4 result is claimed.
- Next source task: audit `ra_switch.c::fe_sw_init()` / `fe_sw_deinit()` for platform-device reference balancing, regulator/clock pairing and complete partial-failure unwind while independent CI jobs run.

---

## 2026-11-24 — Reduce Roadmap size and preserve current LAN DHCP state

- AI handoff runs #45323474834 (`5df6be1`) and #45323579867 (`24d204e`) both failed solely because `REFACTOR_ROADMAP.md` was 132159 UTF-8 bytes vs the 131072-byte hard ceiling.
- Condensed the LAN DHCP/AP-mode lease row into the implemented behaviors, remaining protocol gaps and exact verification requirements. The core fail-closed BOUND/RENEW transaction, AP host/alias handling, networkmap retry and DHCP-first shutdown state remain represented; historical per-commit details already live in `dev_log.md`.
- Parent main: `24d204e991e1ea1b2f1d1af6bb1c96d327e37ca6`; compacted Roadmap now measures 128495 UTF-8 bytes. Exact-SHA AI handoff rerun required.

---
## 2026-11-24 — Fix the metadata test fixture selected by lifecycle integration

- Process lifecycle run #45323274944 on `017ffdf4` advanced through the C and ASan/UBSan regressions, then failed at `check_process_lifecycle_integration.py`: the assertion for `metadata_policy_rejects_unsafe_files` inspected `shared_pidfile_test`, which is the parser test in `src/tests/`, not the metadata test.
- Added a separate `shared_pidfile_open_test` binding for `trunk/user/shared/tests/shutils_pidfile_open.c` and moved the marker assertion to the correct file. This preserves parser assertions against the parser test while checking owner/nlink/mode rejection in the metadata fixture.
- The `017ffdf4` AI docs, legacy wrapper and process inventory checks passed; its XY-C1 build was triggered and remained pending at last poll. RAETH `f7bd670` Switch CLI passed, while its XY-C1/NVRAM runs were still pending.
- Parent main: `5df6be1d45a53c76f9a20ffa524dd4480be65f67`; fresh exact-SHA process lifecycle and AI handoff runs are required.

---
## 2026-11-24 — Align PID metadata source contracts and XY-C1 header triggers

<!-- 2026-11-23 RAETH DEV LOG -->
## 2026-11-23 — RAETH lifecycle and allocation hardening

Baseline before this documentation commit: `f7bd670c88520298c8c4628b1ff06e0c95251f12`. Backup `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

Implemented small, separately committed source slices covering PDMA/QDMA RX and TX allocation rollback, RSS/HW-LRO descriptor ring teardown, QDMA FQ/SFQ lifetime, staged DMA initialization rollback, IRQ request tracking and unwind, switch IRQ-source shutdown, open/close worker sequencing, pseudo-netdev registration failure handling, primary netdev `ndo_uninit`/NAPI teardown, MT7621 GP0 clock-enable failure propagation, and removal of cancellation of an uninitialized `reset_task`. Added/expanded `scripts/check_raeth_rx_ring_cleanup.py` and related static assertions.

Exact-SHA Actions for `f7bd670c88520298c8c4628b1ff06e0c95251f12`:
- XY-C1 Firmware Validation: in_progress (run 45323341920)
- Switch CLI ioctl failure-status regression: completed / success (run 45323341869)
- NVRAM transaction boundary audit: in_progress (run 45323341860)

No local MIPS firmware compile is claimed. Next: audit `fe_sw_init()` resource/reference unwind and verify XY-C1 firmware Actions against the latest source SHA.


- Process lifecycle run #45322826402 on `fdc7ac1` passed the strict-GNU89 compile stage but failed later in `check_process_lifecycle_integration.py`: the checker still required inline `S_ISREG(st.st_mode)` even though production now calls `shutils_pidfile_metadata_is_safe(&st, geteuid())`. Replaced the stale assertion with checks for the helper and the complete owner, file-type, link-count and mode policy.
- Made `shutils_pidfile_policy.h` self-contained with `<sys/stat.h>` and added the include contract to the legacy wrapper checker.
- Found that XY-C1 firmware push paths included `shutils.c` but not its newly included policy header. Added the header path and a source-contract assertion to prevent future silent skips of header-only target changes.
- Parent main: `776c84b04a194d0260830eb14f8732df864ee7c3`; this correction requires fresh exact-SHA process lifecycle, legacy-wrapper and XY-C1 runs.

---
## 2026-11-24 — Keep pidfile metadata policy portable to strict GNU89 tests

- Process lifecycle regression #45322580591 caught a test-build incompatibility: under the workflow's `-std=gnu89 -Wall -Wextra -Werror`, glibc did not expose `S_ISREG` because the test did not request miscellaneous feature macros. Production build had `_GNU_SOURCE`, but the header-only policy tests deliberately compile without it.
- Replaced `S_ISREG(st_mode)` with the equivalent `(st_mode & S_IFMT) == S_IFREG`, which is available under the test's declared POSIX/GNU89 environment, and updated the source-contract checker. A fresh exact-SHA process lifecycle run must confirm.
- Parent main: `3e13e241219d08b26aeb57795f553504783c1b06`; no new exact-SHA validation exists yet.

---
## 2026-11-24 — Validate legacy PID-file metadata before privileged signalling

- `kill_pidfile_s()` parsed a PID from a no-follow opened path, but only verified that the opened object was a regular file and below the size limit. It did not constrain file owner, hard-link count or group/world write permission. Since the helper invokes `kill(pid, sig)` with caller privileges, an untrusted PID file can direct privileged signalling at an unrelated process.
- Added metadata policy matching the stricter shared process-pidfile checks: regular file, owner equals effective UID, `st_nlink == 1`, and no group/world write bits. Production `kill_pidfile_s()` now fails closed with `EPERM` before parsing/signalling when metadata is unsafe. Existing no-follow and nonblocking open flags remain.
- Extended the existing host regression to cover safe metadata, wrong owner, hardlink, group-write and FIFO; extended the source checker to require production enforcement. Existing legacy-wrapper PID-file workflow builds this test with GNU89 warnings-as-errors and ASan/UBSan.
- Parent main: `dd5e71103ee4ea1e2b312d358f253f662c0ddc0f`; exact-SHA Actions pending after commit.

---
## 2026-11-24 — Fail closed on malformed LAN DHCP static-DNS selector during deconfig

- `lan_down_auto()` read `lan_dns_x` into its transactional deconfig snapshot but selected static DNS with `atoi(snapshot.static_dns_mode) == 0`. The bound/lease resolver path uses `rc_wan_nvram_parse_int_value()` on the same setting. `atoi("0;reboot")` returns zero and incorrectly enables static-DNS mode, potentially replacing `/etc/resolv.conf` during deconfig.
- Reused the strict integer parser immediately after the snapshot and before route deletion, interface reconfiguration, resolver staging or publication. Parse failures now return the parser status and preserve lease identity for retry. The existing Python source contract verifies the fail-closed path and ordering; its workflow already covers `net_lan.c` and `check_lan_dhcp_deconfig.py`.
- Parent main: `69c15a20e8ce91bf8c31e3ac600547421ba3bfd1`. Exact-SHA WAN DHCP regression and XY-C1 are required for this change; no local target build or hardware result is claimed.

---
## 2026-11-24 — Keep the handoff roadmap within the validator byte limit

- AI handoff run #45321100686 on source SHA `a52ded9e6c0982ff7b838a59b4fb62df3fbd1815` failed only because `REFACTOR_ROADMAP.md` was 131250 UTF-8 bytes, 178 bytes above the 131072-byte hard limit. The new RAETH DMA/RX status row was longer than necessary.
- Compressed that row without removing the source status or verification boundary. The revised file is now below the validator limit; this documentation-only commit needs a fresh exact-SHA AI handoff run.
- Source SHA `a52ded9`: Switch CLI #45321100540 PASS; XY-C1 #45321100612 and NVRAM #45321100619 were pending/in progress at last poll. The docs follow-up does not change firmware inputs and must not be treated as a product-source build.

---
## 2026-11-24 — Add reserved headroom to RSS and hardware-LRO RX buffers

- RSS 4-ring/2-ring initialization and hardware-LRO ring setup allocated skb buffers without `NET_SKB_PAD`, although their DMA mappings start at `buffer + NET_SKB_PAD` and the receive path reserves `NET_SKB_PAD + NET_IP_ALIGN`. This left the initial buffers smaller than the corresponding PDMA/QDMA and runtime replacement allocations.
- Added `NET_SKB_PAD` to all three initialization size calculations. Extended `check_raeth_rx_ring_cleanup.py` to require the correct aligned payload + headroom + shared-info capacity for PDMA, QDMA, RSS 4-ring, RSS 2-ring and HW LRO.
- Parent main: `54f23c27f924b31ad8d3caefc25ed9973a82bd08`. This source/checker slice requires exact-SHA Switch CLI, AI handoff and XY-C1 Build/TRX/artifact verification. No local MIPS build or hardware L4 result is claimed.

---
## 2026-11-24 — Validate RAETH descriptor DT properties and handle allocation failures

- `fe_tx_rx_dec()` ignored every `of_property_read_u32()` result and reused `val`, so missing/malformed descriptor properties could feed uninitialized or stale counts into the driver. Queue×descriptor products and sums were unchecked, and every `kmalloc_array()` result was dereferenced without a NULL check.
- Converted the decoder to return status. GMAC1 queue/descriptor and RX counts are mandatory; GMAC2 queue/descriptor properties must be supplied together and are mandatory when `FE_GE2_SUPPORT` is enabled. Zero/partial counts and 32-bit multiplication/addition overflow are rejected before global descriptor counters are published.
- Checked all TX bookkeeping and RX-ring array allocations. Partial allocation failure releases every pointer already allocated and resets published counters. If a later probe stage fails after descriptor setup succeeds, probe rollback now releases descriptor bookkeeping before freeing the net_device.
- Added a pure GNU89 descriptor policy and ASan/UBSan regression for GE1+GE2 and GE1-only configurations, invalid/missing counts, partial GMAC2 properties, invalid GE2 state, NULL outputs and arithmetic overflow. Wired the header/test into push and pull-request Switch CLI path filters and extended the RAETH source checker.
- Parent `fe690de6011c957bdbb860ff0c792d6236faefde`: rollback-contract correction was just committed; exact-head Actions had not yet been observed. This source/test slice requires fresh exact-SHA Switch CLI and XY-C1 checks.

---
## 2026-11-24 — Fix the RAETH frame-engine rollback source assertion

- The exact Switch CLI run #45314640982 on `2391437add2f2d614accc3264ec3e6d83b13d238` failed in the RAETH probe/MMIO source-contract step, not in compilation or the descriptor work. The code correctly checks `IS_ERR(ethdma_frame_engine_base)`, obtains `PTR_ERR`, logs it, and jumps to `err_free_dev`.
- The checker incorrectly required `goto err_free_dev;` to occur within 120 characters after `PTR_ERR`. The intervening `dev_err()` line made that arbitrary text window too short. Replaced it with an ordered lookup of the rollback jump after the PTR_ERR assignment; this still verifies failure propagation without depending on formatting.
- On parent `2391437...`: AI handoff #45314640970 **PASS**, NVRAM #45314641002 **PASS**, XY-C1 #45314641044 was **IN_PROGRESS** at last poll. New exact-head Switch CLI is required. No L3 claim is made for this parent until XY-C1 completes.

---
## 2026-11-24 — Restore ERR_PTR handling for the RAETH frame-engine mapping

- The exact main commit `f44888c6103f2ff3bf0712b4d1bb38eb683fc331` changed the failure check for `devm_ioremap_resource()` from `IS_ERR()/PTR_ERR()` to a NULL check. Linux 4.4's `lib/devres.c` documents and implements this API as returning an `ERR_PTR()` on failure; an error pointer is non-NULL and would have passed the new guard into later MMIO use.
- Restored the error-pointer check and preserved the original errno for probe rollback. Strengthened the Switch CLI source contract to require the mapping call → `IS_ERR` guard → `PTR_ERR` propagation → `err_free_dev` path, and reject a NULL-only check.
- Parent main: `f44888c6103f2ff3bf0712b4d1bb38eb683fc331`. THIS COMMIT requires exact-head Switch CLI, AI handoff, NVRAM and XY-C1 checks. No local MIPS build, firmware image or hardware validation is claimed.

---

## 2026-11-23 — Make AliyunDrive failed-start cleanup regression race-safe

- The exact-head AliyunDrive lifecycle run `#45312872098` on `3160f80` passed the bundled MIPSel CLI compatibility check, then failed in the new HTTP-503 launched-child fixture's combined assertion. The fixture assumed exactly 11 probes despite the child marker being published asynchronously; the initial process poll can legitimately observe ABSENT before the child becomes visible.
- Reset PID/executable mock modes explicitly and bound the HTTP probe count to 1..`ALD_START_WAIT_SECONDS + 1`, while still requiring the full ten-sleep timeout, exactly one TERM, zero KILL, a launched argv file and no remaining process marker. Failure output reports the observed counters.
- The same CI inspection found the RAETH compile-integrity checker counted only `void` definitions; `fe_chip_name_config()` is deliberately `static int`. Updated the matcher to count exactly one actual function body for either supported return type.
- Both corrections require fresh exact-head Actions. No Aliyun lifecycle PASS or RAETH selector-contract PASS is claimed yet.

---
## 2026-11-24 — Remove a stale checker continuation after matcher correction

- The first follow-up fixed the over-escaped Python regex and source-string literals but left the previous assertion's continuation string behind, which would make the Python checker syntactically invalid. Removed the orphaned line; the selector `all(..., message)` assertion now has one balanced call. No earlier checker run is counted as PASS.

---
## 2026-11-24 — Tighten the RAETH DT-selector regression matcher

- The first source-checker commit accidentally double-escaped its Python regex and C-source newline assertions, so those new assertions would search for literal backslash sequences rather than whitespace/newlines. Corrected the matchers before treating them as validation, and strengthened the selector contract to require `!ret` before every `strcasecmp(pm, ...)` comparison in `raeth_arch_setting()`.
- The source change remains `7f7aee3b5df94c6996743e58834ee2ec4d67eafd`; this checker-only follow-up requires a fresh exact-head Switch CLI run. No test PASS is inferred from the earlier checker version.

---
## 2026-11-24 — Fail closed on missing RAETH device-tree selectors

- `raeth_arch_setting()` reused one `pm` pointer across `of_property_read_string()` calls. When `gmac1-support` was absent, the first `else if (!strcasecmp(pm, ...))` dereferenced an uninitialized/stale pointer; when later selector reads failed, their `else if` branches could interpret the previous property's value as the missing selector.
- Guarded every alternate `gmac1`/`gmac2` and mode comparison with the status of its immediately preceding property read. Missing or malformed optional selector values now skip that mode branch instead of consulting stale data.
- `fe_chip_name_config()` now returns an error on failed or unsupported `compatible`, and probe propagates that error to `err_free_dev`; the failed probe clears `dev_raether` before freeing the unpublished net_device. Extended the existing RAETH compile-integrity checker to pin these error/selector guards.
- Parent `3160f80e4e2c82d2188bddc12f88589898a79327`: AI docs #45312872263 **PASS**, Process inventory #45312872116 **PASS**, AliyunDrive lifecycle #45312872098 **IN_PROGRESS**. This new driver change requires exact-head Switch CLI and XY-C1 validation; no target build or hardware result is claimed.

---
## 2026-11-24 — Clean up a newly launched AliyunDrive daemon that never becomes HTTP-ready

- `ald_start_unlocked()` previously returned failure after a full bounded HTTP readiness timeout but left its newly launched, executable-confirmed daemon running. Subsequent starts then treated it as a pre-existing unhealthy process and refused to launch another copy, requiring manual stop.
- Added `ald_cleanup_failed_start()`, restricted to the post-launch readiness-failure path. It revalidates executable identity, uses the existing bounded TERM/KILL routine only for a confirmed RUNNING child, treats ABSENT as already clean, and never signals UNKNOWN. Existing pre-existing unhealthy-daemon and UNKNOWN-state paths are unchanged.
- The lifecycle shell regression adds a permanently HTTP-503 newly launched child; it requires a failed start, eleven bounded HTTP probes, ten readiness sleeps, one TERM, no KILL, and removal of the process marker. This commit's exact-head lifecycle Action was still running when the next RAETH issue was selected; no PASS is claimed.
- Commit `3160f80e4e2c82d2188bddc12f88589898a79327`: Process inventory #45312872116 **PASS** and AI docs #45312872263 **PASS**; AliyunDrive lifecycle #45312872098 **IN_PROGRESS** at last poll.

---
## 2026-11-24 — Unblock XY-C1 RAETH compilation after exact-SHA build failure

- The exact XY-C1 Action on `edb529c4baf1e60df1fd542a4e8192cf608ffbdc` failed in `drivers/net/ethernet/raeth/raether.o`, before TRX/artifact stages. The compiler reported conflicting `set_ge2_gmii` / `set_ge0_gmii` prototypes (`ra_mac.h` said `void`, while `raether.h` and `ra_mac.c` correctly returned `int`), plus duplicate definitions of `fe_chip_name_config`, `raeth_set_wol`, `raeth_arch_setting` and `fe_tx_rx_dec` in `raether.c`.
- Corrected the stale `ra_mac.h` prototypes to `int`, preserving MMIO setup error values already implemented by `ra_mac.c` and consumed by the driver. Removed the second accidental duplicate helper block in `raether.c`, retaining one complete definition of each helper.
- Added `scripts/check_raeth_compile_integrity.py` and wired it into both push and pull-request Switch CLI workflow path filters. It asserts uniqueness of the four helper definitions and matching `int` declarations/definitions for both GMII helpers.
- On the failed `edb529c` SHA, Switch CLI #45311706551 **PASS**, AI handoff #45311706519 **PASS**, NVRAM audit #45311706453 **PASS**, but XY-C1 #45311706484 **FAIL** at kernel compilation; Build failed before TRX or artifact upload. THIS COMMIT requires fresh exact-head XY-C1 Build/TRX/artifact and Switch CLI checks. No L3 or hardware result is claimed.

---
## 2026-11-24 — Scope the GE2 SGMII switch error gate to its optional feature

- `fe_sw_init()` checked the return status of `set_rtl8367s_sgmii()` outside the `if (architecture & SGMII_SWITCH)` branch. When GE2 forced-2.5G was enabled without the external switch feature, the code could inspect stale/uninitialized `status` and incorrectly abort switch initialization.
- Added braces so the RTL8367 call and its error gate execute only when `SGMII_SWITCH` is enabled. `set_sgmii_force_link(2, 1)` remains outside that inner guard, preserving the non-external-switch GE2 forced-link path.
- Strengthened the Switch CLI source contract to parse the nested brace block and require the call plus `status`/`mdio_error` gate inside it, while proving force-link remains outside. The previous proximity-only assertion passed without establishing lexical control-flow ownership.
- Parent main `f38519f5f92f39148cccdfa6c37eb9126346a982`: Switch CLI #45311619652 **in_progress**; AI handoff #45311620761 **success**; NVRAM audit #45311619611 **in_progress**; XY-C1 #45311619583 **in_progress**. This exact-head Switch CLI pass did not detect the unbraced scope defect. THIS COMMIT requires fresh exact-head Switch CLI/AI checks and same-SHA XY-C1 validation. No local kernel build or hardware result is claimed.

---
## 2026-11-23 — Propagate RStats history read descriptor-close failures

- `f_read()` saved the read errno but ignored `close(fd)`'s return value. A descriptor-close I/O error after a successful or short read could therefore be reported as ordinary data, allowing a questionable persistent-history record to be consumed.
- Added `rstats_file_read_close_result()`: preserves a primary read error, converts close-only failures to `-1` with close errno (or `EIO` if unavailable), and otherwise preserves full/short read counts. `f_read()` now captures read and close errno separately. It deliberately does not retry `close()` after EINTR on Linux.
- Added GNU89 policy cases for full/short success, read-error precedence, close-only failure and missing-errno fallbacks; extended the source contract to require errno capture before checked close and helper return.
- Parent `47100dccc0a7128cbc711b31766fedc82ac787ad`: Switch CLI `#45309913398` and AI handoff `#45309913249` **PASS**; NVRAM audit `#45309913323` and XY-C1 validation `#45309913333` were **IN_PROGRESS** at last poll. This RStats slice awaits its own exact-SHA Actions.

---
