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

