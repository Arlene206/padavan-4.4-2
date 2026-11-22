## 2026-11-22 — Rotate the rolling development log and preserve full-restart verification

- Root `dev_log.md` had 121 complete records. Archived the 101 oldest records byte-for-byte in chronological order in `docs/dev-log/2026/11/part-025.md`; retained the 20 newest original records at the root, then prepended this rotation checkpoint. Archive SHA: `daf3875420e99758082b0d43ab2e4b4b2cd57ab2`; pre-rotation root blob SHA: `418727893b781c626ffa7995bc378ac8b5cff66f`.
- Exact `a3b7e95b71776808fbb6e6bddf273db8cf0c98f4` WAN route #44997377580, WAN DHCP #44997377452, WAN interface teardown #44997377521, wireless radio #44997377474, Process lifecycle #44997377507 and Process inventory #44997377633 PASS. AI handoff #44997377383 failed only because the root log reached 121 entries; this lossless archive rotation restores the required rolling window.
- Exact a3 XY-C1 #44997377413 was still executing Build XY-C1 at last poll; TRX verification and artifact upload had not completed. NVRAM transaction audit #44997377584 and several independent suites were still running. No L3 claim.
- The same a3 source fix validates and preserves `wan0_ifname` before `full_restart_wan()` changes router mode or stops WAN, then reuses the checked local name for route flush. Hardware L4 remains unverified.

---
## 2026-11-22 — Validate full-restart route interface before WAN teardown

- Parent main SHA: `c69d200c788aafeb76c6905bb18a70d22f9d49d4`. In the prior 86d run, wireless radio #44997181767 and Process lifecycle #44997181794 failed only because the source checker called undefined `require()` at line 323; the wireless policy executables passed. Follow-up c69 changed that call to Python `assert`.
- Found another concrete state-transition read-loss path: `full_restart_wan()` fetched `route_ifnames[0]` via `get_man_ifname(0)` only after `stop_wan()` completed. A transient NVRAM read error could appear as an empty string after services had already been stopped, then route flush would operate on the wrong interface set.
- Moved checked `get_wan_unit_value_copy_status(0, "ifname", ...)` and `wan_ifname_copy()` validation before `update_router_mode()` and `stop_wan()`. The validated bounded local name is reused for route flush; real errors are logged, copied to `errno`, and returned before WAN side effects. Empty legacy configuration remains an empty string.
- Extended the WAN DNS/read checker to pin checked read/error/validation before mode update and service stop, then route use of the same validated buffer, with no `get_man_ifname(0)` left in full restart.
- Exact c69 wireless radio #44997296073, Process inventory #44997296036 and AI docs #44997296148 were in progress at last poll; no same-SHA XY-C1 build claim. Exact-head validation for this product change remains pending.

---
## 2026-11-22 — Fix AP-client radio source-checker assertion helper

- Parent main SHA: `86d18d524ca522dc4d85e5abf7c28376ac74a394`. Exact wireless-radio #44997181767 failed at line 323 with `NameError: name 'require' is not defined`; the newly added checker used a helper absent from this script, which consistently uses Python `assert`.
- Exact Process lifecycle #44997181794 also failed at the same imported guest-LAN checker assertion after its independent account bootstrap and bridge policy tests passed. The wireless-radio, bridge-membership and guest-isolation C policy executables passed before the Python checker.
- Replaced only the undefined `require(condition, message)` call with Python `assert (condition), (message)`, retaining all ordering, empty-configuration and errno assertions. No product source changed.
- Exact 86d AI handoff #44997181801, process inventory #44997181843, HTTPD Auth/Security #44997181802 and iNIC lifecycle #44997181809 PASS. XY-C1 #44997181783 and NVRAM transaction #44997181875 were still in progress at last poll.

---
## 2026-11-22 — Check AP-client radio recovery inputs before WAN restart decisions

- Parent main SHA: `0292b0a44ece35cac3e0331ab04aa15028a671a3`. Exact WAN route #44996976228 PASS, DHCP #44996976338 PASS, WAN teardown #44996976388 PASS and AI handoff #44996976327 PASS. Exact XY-C1 #44996976282 was still building.
- `check_apcli_wan()` previously used `get_ap_mode()` and `get_man_ifname()` (safe_get-backed) to decide whether an AP-client radio restart should trigger full WAN restart or reconnect. A transient NVRAM error could masquerade as AP mode false/empty manual interface and produce an incorrect recovery decision.
- Switched to `get_ap_mode_status()` and checked `get_wan_unit_value_copy_status()`; nonempty manual interface is validated by `wan_ifname_copy()`. Any read/token failure returns before restart/reconnect decision, with errno preserved. Empty legacy manual config continues to map to no APCLI MAN interface.
- Extended `check_guest_lan_isolation_status.py` to pin the checked mode/manual input order and `wan_ifname_policy.h` dependency. Exact-head wireless/WAN/XY Actions remain pending for this source change.

---
## 2026-11-22 — Guard AP-client WISP migration with checked prior WAN identity

- Parent main SHA: `2149789b0636dd794fcb7eeb5343e314045cdc9a`. Exact 214 WAN route #44996754354 PASS, WAN DHCP #44996754350 PASS, WAN interface teardown #44996754355 PASS and AI handoff #44996754406 PASS; XY-C1 #44996754358 remained in Build XY-C1.
- `config_apcli_wisp()` previously fetched old manual WAN interface via `get_man_ifname()` (safe_get-backed); a transient NVRAM error could look empty, skip clearing the prior WAN address/interface, then continue enabling/publishing the AP-client WISP interface.
- Added checked bounded `ifname` read and `wan_ifname_copy()` validation before old-interface cleanup, IPv6 enablement and NVRAM publication. Actual errors are returned with `errno`; empty legacy configuration remains an empty string.
- Extended the WAN source contract to assert read/error/validation ordering before ifconfig/IPv6/publication and forbid the lossy getter. Exact-head Actions for this additional source change remain pending.

---
## 2026-11-22 — Fail closed before WAN teardown when manual interface is unreadable

- Parent main SHA: `59cdab3123a8d6cce5dae6c5f5fe347d9d9bee67`. Exact WAN route #44996570973 PASS and AI handoff #44996570985 PASS; exact d018 WAN DHCP #44996488482 PASS.
- Reviewed `stop_wan_locked()`: it previously read `man_ifname = get_man_ifname(unit)` only after pausing Internet monitoring, invalidating PPP callback sessions and cancelling deferred reconnect. Since the getter is `nvram_safe_get()`-backed, a NVRAM read/backend error could be silently converted to an empty name while teardown proceeded.
- Moved a checked `get_wan_unit_value_copy_status(..., "ifname", ...)` and `wan_ifname_copy()` validation ahead of monitor pause and all teardown side effects. The real read/validation error is logged, copied to `errno`, and returned. An actually empty legacy/unconfigured value is preserved as an empty string rather than rejected; removed the later lossy getter.
- Added a source contract to pin checked read → read-error gate → nonempty token validation/error gate → empty-key preservation → monitor pause, and forbid the lossy getter in `stop_wan_locked()`.
- Exact-head route/DHCP and XY-C1 Actions are pending for this product source change. No MIPS build/TRX/artifact or hardware test is claimed.

---
## 2026-11-22 — Keep modem identity error contract errno-aware

- Parent main SHA: `d018080e3ef848417c1969ba17adfd0b8a775624`. Exact WAN DHCP #44996488482 PASS. Exact WAN route #44996488531 FAILED at `scripts/check_wan_dns_read_status.py:515`: after the `get_wan_ip4()` checked manual fallback change, the old modem identity assertion still required `if(status!=0)returnINADDR_ANY`.
- The actual code correctly sets `errno=status` before returning `INADDR_ANY` for modem identity failures. The stale assertion had not been updated for this improved error preservation; the checked manual-fallback source contract immediately above it passed.
- Replaced the errno-less pattern with an ordered contract requiring modem status read → `errno=status` failure gate → transport selection. This is test-only; no product behavior change in this follow-up.
- Exact d018 XY-C1 #44996488349 was still building/validating at last poll. L3 remains unverified until Build XY-C1, TRX and artifact all pass on one identical product-source SHA.

---
## 2026-11-22 — Check manual interface only when get_wan_ip4 selects it

- Parent main SHA: `616d5a710acdf727a8f110052289c1d63d966a9d`. Exact WAN DHCP #44996346231 PASS and AI handoff #44996346193 PASS; prior WAN route #44996213814 PASS.
- Found the remaining `nvram_safe_get()` loss path in `get_wan_ip4()`: it read `get_man_ifname()` before modem/PPP overrides. Backend/read/close errors could be mistaken for an empty interface; moreover manual `ifname` was fetched even when a modem/PPP interface won.
- Changed interface selection to check `ifname_t`, modem identity and protocol first, retaining modem/PPP overrides. Only when no override is selected does it now perform checked `get_wan_unit_value_copy_status(..., "ifname", ...)`, validate via `wan_ifname_copy()`, and preserve read/validation failure in `errno` before returning the existing `INADDR_ANY` sentinel.
- Updated `scripts/check_wan_dns_read_status.py` to require manual fallback read/validation after modem/protocol selection and before address lookup, while preserving the cache-overwrite regression.
- Exact-head WAN route/DHCP for this product-source SHA are pending. No MIPS build, TRX, artifact or hardware validation is claimed.

---
## 2026-11-22 — Normalize WAN manual-fallback checker ordering

- Parent main SHA: `ecfdafb0e348bcec466d4372400f5a2c87f02006`. Exact ecfd WAN route #44996213814 PASS; AI handoff docs #44996213622 PASS. WAN DHCP callback #44996213685 failed after its GNU89/ASan/UBSan `wan_ifname_policy.c` executable and basic bounds regression both passed.
- The failing assertion at `scripts/check_wan_ifname_bounds.py:51` compared `manual_ifname_gate_at` from the raw source string with later positions from the whitespace-compacted source. Those offsets are not comparable, so a valid checked manual fallback failed the checker.
- All ordering positions are now measured in the same compacted function. The contract also verifies modem and protocol selection happen before the fallback, and that read error and token-validation error branches both precede the final output copy.
- The exact dc6 product-source XY-C1 run #44996098229 remained in progress at last poll with Build XY-C1 running; TRX and artifact upload were pending. The checker-only ecfd commit did not alter product firmware inputs. L3 remains unverified.

---

## 2026-11-22 — Align WAN DNS checker with checked manual-interface getter

- Parent main SHA: `dc6f6b4efb8b0ac088e82e18f9bd42168efdd9d0`. Exact WAN DHCP callback #44996098158 PASS, including the expanded `check_wan_ifname_bounds.py`; HTTPD Auth/Security #44996098097 and AI handoff #44996098288 PASS.
- Exact WAN route #44996098107 FAILED at `scripts/check_wan_dns_read_status.py:493`. Its contract still required `get_man_ifname()` followed by `rc_wan_nvram_copy_text()` in `get_wan_ifname()`, while the current implementation now selects modem/PPP overrides first and performs checked `get_wan_unit_value_copy_status()` plus `wan_ifname_copy()` only when using the manual interface. The C path itself was not the failing step.
- Replaced that stale assertion with the checked fallback read/validation contract and strengthened `check_wan_ifname_bounds.py` to verify the exact manual `ifname` read occurs inside the fallback branch before its copy and the final public output copy; this avoids accidentally matching the separate earlier `ifname_t` checked read.
- Exact dc6 XY-C1 #44996098229 was still IN_PROGRESS before Build/TRX/artifact at last poll. This follow-up's exact-head checks are still pending; no L3 claim.

---
## 2026-11-22 — Preserve checked interface identity in get_wan_ifname

- Parent main SHA: `2b2c2e6c1ef1ca9fc41dd39ebb88858e33cf1bcb`. Exact 2b WAN DHCP callback #44995928887 PASS, WAN route status #44995928825 PASS and AI handoff #44995928838 PASS. Exact XY-C1 #44995928841 remained IN_PROGRESS at last poll; no L3 claim.
- Continued the same concrete NVRAM failure path beyond `start_wan()`: `get_wan_ifname()` also used `get_man_ifname()` backed by `nvram_safe_get()`, then copied the lossy value through `rc_wan_nvram_copy_text()`. A backend/read/close error could be rewritten as an empty-interface `EINVAL` instead of preserving the actual NVRAM error.
- Removed the lossy getter from `get_wan_ifname()`. The function now performs checked bounded read and token validation only if its selected interface still uses the configured manual name; modem/PPP overrides do not gain an unrelated read dependency. Checked status is returned with `errno` preserved.
- Extended the existing WAN interface source contract to pin that conditional checked read, validation, error propagation and lack of the lossy getter. Exact-head WAN DHCP/route/XY verification is pending for this commit.

---
## 2026-11-22 — Align WAN startup source contracts with checked interface identity

- Parent main SHA: `e34444ae55651085a157996f4c6b5699b20066c9`. Exact 6ec WAN DHCP #44995722211 first failed because the first version of the new checker searched for literal `\\n`; the e344 follow-up corrected this, but WAN DHCP #44995792863 then exposed two additional whitespace-sensitive expectations that omitted the space after `(`. The underlying WAN interface-name GNU89/ASan/UBSan executable passed on both source SHAs before the Python checker stopped the job.
- Exact 6ec WAN route #44995722139 and exact 6ec XY-C1 #44995722192, repeated on e344 as #44995792838 and #44995792759, failed at `scripts/check_wan_firewall_status.py:1170`: it still required `wan_ifname = get_man_ifname(unit)` and was stale after the checked source fix.
- Updated the interface-name checker to match normalized actual C call formatting. Updated the WAN firewall source contract to require checked `ifname` read → explicit read-error status handling → empty legacy no-op → validated interface token → explicit validation-error status handling → first interface side effect, and to forbid the removed lossy getter.
- Exact e344 AI handoff #44995792742 and Process inventory #44995792750 PASS; no XY-C1 L3 build is claimed. Recheck all exact-head Actions on the current follow-up SHA.

---
## 2026-11-22 — Correct WAN interface diagnostic and checker newline anchors

- Parent main SHA: `6ec3e3d6bf3576864dee986aaf1e7075a4fbc1fa`. Immediate source review caught two defects before treating CI as validated: the invalid-interface log format accepted only one of its two integer arguments, and the newly added Python checker delimited C functions using a double-escaped newline (literal backslash-n).
- Supplied both unit and ifname_status to the diagnostic and corrected the source-checker delimiters to resolve actual newline boundaries. Added an exact source assertion for the diagnostic argument order.
- The exact 6ec push triggered WAN DHCP callback/route, Process, NVRAM and XY-C1 plus the broader source-filtered suites; their first poll showed pending/in-progress only. Verify the follow-up SHA rather than inferring results from the intermediate head.

---
## 2026-11-22 — Fail closed when WAN startup cannot read its configured interface

- Parent main SHA: `7bd0c10e867a429dfbc7dc2573e211c1963aa580`. Exact 7bd storage hotplug #44995310447 and AI handoff #44995310444 passed. Exact XY-C1 #44995310496 completed host-only validation successfully, but Build XY-C1, TRX verification and artifact upload were skipped because 7bd changed only tests/checkers/handoff; no L3 claim.
- Confirmed a P1 startup-status loss in `start_wan()`: it called `get_man_ifname()`, which uses `nvram_safe_get()`. A transient NVRAM backend/read/close error was therefore indistinguishable from an empty configured interface and silently skipped WAN startup.
- Replaced the pointer lookup with `get_wan_unit_value_copy_status()` into a bounded local buffer, retained the established empty/unconfigured no-op, validated non-empty names through `wan_ifname_copy()`, and merged both read/validation failures into the first WAN startup status before any interface setup.
- Extended `scripts/check_wan_ifname_bounds.py` to pin checked read → empty legacy no-op → validated interface token → network side effects, plus status propagation and removal of the lossy getter from `start_wan()`. Existing WAN interface-name GNU89/ASan/UBSan policy cases and checked NVRAM-copy regression are reused by the existing WAN DHCP workflow.
- Exact-head WAN DHCP/route and XY-C1 Actions are pending for the new product-source SHA. No local MIPS build or hardware test is claimed. L3/L4 remain unverified.

---

## 2026-11-22 — Align printer hotplug checker with start-before-ack semantics

- Parent main SHA: `886f9cf54a6abafad251fb444de199e0930446f8`. Exact 1160 storage run #44995018412 passed the storage policy regressions and whole caller graph, then failed in `scripts/check_storage_safe_remove.py:1512`. Its assertion required the printer failure branch to precede `storage_ack_event_marker_status("usb_hotplug_lp")`.
- The product callback correctly calls `try_start_usb_printer_spoolers()`, acknowledges `usb_hotplug_lp` only when start status is zero, and only then enters the failure branch for start/ack failures. The branch logs that the event remains queued and calls `alarm(5)`. Updated the checker to assert this exact order and retry contract.
- Exact 886 notification queue Actions #44995118883 and AI handoff docs #44995119002 passed. Exact 886 XY-C1 validation #44995118906 completed SUCCESS, but Build XY-C1, TRX verification and artifact upload were SKIPPED because 886 changed only a test expectation and docs. This is not an L3 build result.
- Next: validate the checker on a fresh storage workflow, then push a real firmware-relevant product change so XY-C1 Build/TRX/artifact execute on one SHA.

---

## 2026-11-22 — Include storage-unplug bit in retry-retention regression

- Parent main SHA: `1160bf68ae35438c2ae5acbef8252b7faaea2c77`. Exact PID-1 notification workflow #44995018361 failed in its first GNU89 policy test. After adding the storage-unplug retry bit, the test correctly retained that bit when a different event was recorded with marker status 0, but one intermediate expected mask still omitted the new bit.
- Updated that expectation to include `RC_NOTIFY_HOTPLUG_RETRY_STORAGE_UNPLUG`. The PID-1 retry implementation itself was not the failure; the assertion was stale after expanding the mask.
- Local policy test will be rerun with GNU89, Werror, ASan and UBSan; exact-head notification/storage/XY-C1 Actions must verify the corrected test and product code.

---

## 2026-11-22 — Retain storage-unplug retry when the NVRAM marker write fails

- Parent main SHA: `7db3ff63d8c65c99163b59027fc00feb80e0171a`. The preceding storage run #44994403806 passed the storage inventory/marker/lifecycle regressions and complete caller graph, then failed in the Python checker because it searched for literal backslash-n instead of a real newline at the end of `storage_ack_event_marker_status()`. The parser correction is in parent `7db3ff63d8c65c99163b59027fc00feb80e0171a`.
- `storage_schedule_ejected_cleanup_retry()` previously ignored `nvram_set_int_temp("usb_unplug_ms", 1)`. If ejected cleanup and the NVRAM marker write both failed, the five-second alarm had no durable event to process, so cleanup could be abandoned.
- The scheduler now checks the marker result and calls `rc_notify_schedule_deferred_hotplug_retry("on_unplug_mass_storage")`. PID 1 records a timed retry bit and requeues the event on its next alarm; short-lived applets enqueue it through the existing secure notification queue. Original `errno` is restored.
- Added a storage-unplug retry bit and event mapping, extended GNU89 queue-policy regression for mapping/record/requeue/failure retention, and strengthened storage and PID-1 notification source contracts.
- Exact 7db3 storage #44994641468, XY-C1 #44994641391 and AI handoff docs #44994641406 were in progress at last poll. No local MIPS firmware build or hardware test is claimed.

---

## 2026-11-22 — Fix storage checker function-boundary escape

- Parent main SHA: `8f54faaaf0e0cbd2461ebd35f595612032521987`. Exact-head USB storage run #44994403806 passed the whole storage caller graph and all preceding C/shell regressions, then failed in `scripts/check_storage_safe_remove.py:1446` with `ValueError: substring not found`. The checker searched for literal backslash-`n` sequences (`"\\nstatic int\\nstorage_read_nvram_string"`) while the C source contains real newline characters.
- Corrected the Python string to use actual newline escapes so the checked marker-writer function ends at `storage_read_nvram_string`; product code and the asserted error-return contract are unchanged.
- Exact-head AI handoff documentation contract #44994403647 passed. XY-C1 validation #44994403623 completed SUCCESS on `8f54faa`, but its Build XY-C1, TRX and artifact-upload steps were SKIPPED due to the workflow's path/condition logic; this is not an L3 firmware-build result.
- Next: check exact-head storage workflow after this checker fix and establish why XY-C1's build steps were skipped; do not claim L3 until all three firmware gates actually execute and pass.

---

## 2026-11-22 — Update safe-remove retry checker for PID 1 queue handoff

- Parent main SHA: `9110090e4886d5a183dfe21dfbd26fde08698aea`. The corrected whole-storage caller graph passed on exact SHA `9110090e4886d5a183dfe21dfbd26fde08698aea`; the workflow then reached `scripts/check_storage_safe_remove.py` and failed at its stale line 478, which still demanded `nvram_set_int_temp("usb_hotplug_ms", 1)` and `alarm(5)` inside `safe_remove_stor_device()`.
- The product fix at `89a6e740e24a63142a56e3406136e86a514ce1a2` intentionally replaced that short-lived applet's local alarm with `notify_rc_status("on_hotplug_mass_storage")` after releasing `storage_lock`. PID 1 then owns the deferred event/alarm lifecycle; queue failure is logged and propagated rather than falsely reported as scheduled.
- Updated the source contract to require the PID 1 event name, verify no direct NVRAM marker/alarm remains in the short-lived retry block, require both success/failure logging and preservation of the notification errno. No product source changed in this checker-only slice.
- Exact 9110090e storage run #44994229595: the embedded whole-storage caller graph passed; the workflow failed in this stale safe-remove checker. The AI handoff contract #44994229636 passed. XY-C1 #44994003623 and Process lifecycle #44994003659 on product source ancestor `89a6e740` remained in progress; Process fail-closed #44994003779 and NVRAM boundary #44994003791 passed.
- Next: rerun storage checker and downstream firmware persistence gates on the corrected SHA; inspect XY-C1 Build/TRX/artifact on the same product-source content. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Match USB printer acknowledgement checker to the retry-safe source order

- Parent main SHA: `89a6e740e24a63142a56e3406136e86a514ce1a2`. Exact-head USB storage run #44994003668 passed the BusyBox ash Optware lifecycle checks, MTD save/restore/erase/backup transaction checks, persistence gate, eject inventory regression, deferred storage cleanup/ack regression, checked event-marker and Optware child-result tests, FTP daemon and storage-stop regressions, RC service-launch test, filesystem module-load lock checks and automount ownership checks.
- The final embedded whole-storage caller-graph checker failed at line 239 because its old assertion expected `printer_start_pos < printer_failure_pos < printer_ack_pos`. The actual retry-safe implementation acknowledges the marker only inside the successful-start branch, then checks the combined start/ack status and schedules `alarm(5)` on either failure; the correct order is start → conditional acknowledgement → failure/retry branch.
- Updated that source contract to require the actual order and to scope the retry assertions through the next modem section. No product source changed in this checker-only slice.
- Exact-head XY-C1 #44994003623, Process inventory #44994003659, NVRAM #44994003791 and Process fail-closed #44994003779 were IN_PROGRESS at the last poll. Firmware Build/TRX/artifact and those source validations must be checked again after this correction; no L3 claim.
- Next: rerun the storage source contract on the new SHA; fix only the first remaining stale assertion or real regression. Frozen backup remains read-only; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align the storage caller-graph regression with durable marker acknowledgement

- Parent main SHA: `d414a788abc66e0bf5d8711b4d0584de505964c9`. The exact-head USB storage run #44993693581 failed in the embedded “whole storage caller graph” source contract at the old assertion that required `usb_unplug_ms` to be cleared before `umount_ejected()`. This is now deliberately forbidden by the product fix in the parent.
- Updated the embedded workflow contract to require checked marker read → action-completion gate → eject cleanup → marker acknowledgement → ack-latch handling, and to reject any premature marker clear. This retains the existing caller-graph/lock-order assertions; no product source changed in this checker-only slice.
- The standalone deferred cleanup/ack policy regression had already passed locally with GNU89, Werror, ASan and UBSan. The failed workflow had reached the embedded source contract; the full storage checker and remaining gates were skipped afterward.
- Next: push this workflow-only correction atomically and inspect its exact-SHA USB storage and XY-C1 runs. No firmware/L3 or hardware/L4 claim.

---

