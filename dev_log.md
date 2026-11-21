## 2026-11-22 09:03 CST — Check switch base feature settings before hardware writes

- Parent main SHA: `991bc8cb3cc9be8a223f8d6646df3f8384a4f590`.
- `switch_config_base()` previously passed unchecked `nvram_get_int()` results directly into jumbo-frame, green-Ethernet and EEE PHY operations. An NVRAM read failure silently disabled features or applied a zero-valued configuration.
- THIS COMMIT reads all three settings through the checked integer parser before the first PHY call. Backend/close/malformed-value failures return before any switch base hardware write; absent-key and valid integer semantics remain unchanged. Best-effort LED indication now follows the checked snapshot.
- Extended the source contract checker to require all three checked reads before `phy_jumbo_frames()` and reject the former getters. Existing sanitizer policy test covers the shared integer backend.
- Exact current product SHA `991bc8cb3cc9be8a223f8d6646df3f8384a4f590` had WAN route, XY-C1, NVRAM and subsystem workflows queued/in progress at last poll. This new production source commit triggers exact-head checks again.
- Next: inspect exact-head WAN route and XY-C1 Build/TRX/artifact; fix any current-SHA source/checker/compiler regression and continue another independent userland issue.

---

## 2026-11-22 08:48 CST — Preserve switch storm-control rates on NVRAM read errors

- Parent main SHA: `ebc799f8bb49b9258ae9e0206ec48c69b0772e7a`.
- `switch_config_storm()` used four unchecked `nvram_get_int()` calls; read failure became zero and then normalized to rate 1024, the most permissive fallback. This could silently weaken unknown-unicast, unknown-multicast, multicast and broadcast storm controls when NVRAM was unavailable.
- THIS COMMIT checks all four values through `rc_wan_nvram_int_range_status()`: valid 1..1024 values are retained, readable missing/out-of-range values preserve the historical 1024 fallback, but backend/malformed-value errors abort before the first `phy_storm_*` operation.
- Extended `scripts/check_wan_vlan_setup.py` to pin the four checked reads and range normalization before hardware operations. Existing `wan_nvram_status_policy.c` covers range bounds, fallback and EACCES/EIO propagation without altering outputs on errors.
- Exact route results: checker-normalization SHA `cfc41b84b12004bc43d1a0a136072a8ef53c5727` WAN route + AI docs PASS. `7f66b9d41d7244265bf7292e89d5b8400da4080a` WAN route PASS and other completed checks PASS; XY-C1/NVRAM were still running. `f20473f17034ca1188abb3226533e2eb0e6564d3` WAN route PASS; XY-C1/NVRAM still running. Latest source `ebc799f8bb49b9258ae9e0206ec48c69b0772e7a` WAN route was running its regression matrix at last check.
- Next: inspect exact-head WAN route/XY-C1 Build/TRX/artifact for this change; fix any same-SHA failure before advancing.

---

## 2026-11-22 08:34 CST — Fail closed on LAN switch VLAN snapshot errors

- Parent main SHA: `f20473f17034ca1188abb3226533e2eb0e6564d3`.
- `switch_config_vlan()` still used fallback `get_wan_bridge_mode()/get_wan_bridge_iso_mode()`, `nvram_match("vlan_filter")`, and 16 unchecked VLAN VID/priority/tag integer reads. Backend or malformed-value errors could be converted to disabled filtering/zero fields, then the function would program all hardware VLAN rules and bridge mode from an incomplete snapshot.
- THIS COMMIT routes bridge mode/isolation through checked range policies, reads `vlan_filter` through checked exact-match, and checks every enabled VID/priority/tag setting before the first `phy_vlan_rule_set()`. A failure returns with errno before any hardware VLAN/bridge operation. Missing-key defaults, valid-value behavior, low-three-bit priority masking and readable out-of-range bridge-mode fallbacks remain unchanged.
- Extended `scripts/check_wan_vlan_setup.py` with ordered source assertions for all 19 checked reads and ensures lossy getters are absent. Existing NVRAM status policy tests cover missing keys, exact matches, malformed/overflow integers and backend errors.
- Previous LAN core `config_bridge()` checked-read slice is `f20473f17034ca1188abb3226533e2eb0e6564d3`; exact actions for it had not yet appeared at last poll. WAN VLAN/soft-bridge source slices are earlier in the same main history. This commit is the next product-source trigger for WAN route checks and XY-C1 Build/TRX/artifact.
- Next: inspect exact-head WAN route and XY-C1 build jobs; repair any current-SHA issue before continuing another subsystem.

---

## 2026-11-22 08:16 CST — Check LAN bridge multicast and port settings before applying bridge state

- Parent main SHA: `7f66b9d41d7244265bf7292e89d5b8400da4080a`.
- `config_bridge()` was another critical bridge path still using unchecked `nvram_get_int()` for `ether_igmp`, `ether_m2u`, and AP-mode `ether_uport`, plus unchecked `nvram_match("mr_enable_x")` and a fallback `get_wan_bridge_mode()` on GMAC2. A transient NVRAM read error could silently change multicast-router, snooping, multicast-to-unicast, static-port, or WAN-port selection before bridge sysfs changes.
- THIS COMMIT adds local checked NVRAM integer/exact-match wrappers over the shared `rc_wan_nvram_status_policy.h`; reads every mode-specific setting before `rc_bridge_config_apply()`, and fails closed with concrete status. GMAC2 WAN bridge mode now uses a checked integer/range policy with the same 0..7 legacy fallback for readable out-of-range values.
- Extended `scripts/check_wan_vlan_setup.py` to inspect `net_lan.c`, enforce all read/error gates before bridge apply, and reject the former lossy readers. Existing WAN NVRAM sanitizer tests cover the shared parser and exact-match backend error policies.
- Current main WAN VLAN contract-normalization source chain: `b4c2a6010c765631a5d6d93e4e84521f54fcf72b`, checker correction `cfc41b84b12004bc43d1a0a136072a8ef53c5727`; cfc exact WAN route and AI docs both PASS. WAN route for prior LAN-soft-bridge slice `7f66b9d41d7244265bf7292e89d5b8400da4080a` is queued/running at last poll. XY-C1 Build/TRX/artifact and NVRAM exact results remain pending.
- Next: validate this AP/router bridge preflight and resolve exact current-source XY-C1 Build/TRX/artifact; then continue independent stability work.

---

## 2026-11-22 08:02 CST — Check LAN bridge multicast settings before topology changes

- Parent main SHA: `cfc41b84b12004bc43d1a0a136072a8ef53c5727`.
- `config_soft_bridge_lan()` read `ether_igmp` and `ether_m2u` through `nvram_get_int()` in declaration initializers. An NVRAM read/close/backend error could therefore silently disable IGMP snooping or multicast-to-unicast and still proceed to VLAN/bridge membership mutations.
- THIS COMMIT reads both settings with `get_nvram_int_checked_status()` and returns the preserved error before bridge mode reads, interface probes, VLAN creation or bridge membership changes. Valid integer semantics remain unchanged; the existing WAN NVRAM sanitizer test exercises the checked integer parser/error policy.
- Extended `scripts/check_wan_vlan_setup.py` to pin both reads, error returns and ordering before `get_interface_flags()`, and forbid the old lossy getters.
- Exact WAN route checker normalization on `cfc41b84b12004bc43d1a0a136072a8ef53c5727` is now queued/running; exact b4 source Build/TRX and NVRAM audit were pending last poll. This second source slice should independently trigger the WAN and XY-C1 checks; no build/device result claimed.
- Next: inspect exact-head WAN route and XY-C1 checks for the combined LAN/WAN NVRAM preflight changes; continue only after failures are fixed.

---

## 2026-11-22 07:49 CST — Normalize full WAN VLAN source contract before ordering checks

- Parent main SHA: `85a34af0483c4c16a26e5a25281c3c18afa78c5f`; WAN source `b4c2a6010c765631a5d6d93e4e84521f54fcf72b`.
- Exact WAN route run #44960876481 failed at `scripts/check_wan_vlan_setup.py:183`: checked getter parameters span lines, so comparing a single-line token against the raw C function body raised `ValueError: substring not found`. The earlier GNU89/Werror/ASan/UBSan WAN helper test and preceding WAN route regressions passed. THIS COMMIT normalizes the complete extracted function, including bridge-mode and VLAN selector/read ordering positions, so positions are compared in one coordinate system.
- The e853 USB storage checker failure was the same class of mismatch: compact C uses `}elseif(...)`; the corrected token is already present in `85a34af0483c4c16a26e5a25281c3c18afa78c5f` and retained here for exact rerun.
- Exact b4 Actions already PASS: HTTPD, Process umbrella/inventory, DNSMasq, MiniUPnP, ZeroTier, router-mode NVRAM, WAN teardown, USB modem, WAN ARP, WAN DHCP callback, detect_link, AI docs, wireless, Firewall Startup, watchdog, detect_internet, VPN server. WAN route failed at the checker formatting mismatch. XY-C1 #44960876531 and NVRAM #44960876561 were in progress at last poll.
- Next: verify exact-head WAN route and USB storage checker jobs, then confirm b4 XY-C1 Build/TRX/artifact and NVRAM audit. No L3 claim before all firmware steps pass.

---

## 2026-11-22 07:34 CST — Correct normalized storage marker branch token

- Parent main SHA: `b4c2a6010c765631a5d6d93e4e84521f54fcf72b`.
- Exact e853 checker run USB storage #44960744369 failed only in `scripts/check_storage_safe_remove.py`: after removing C whitespace, the source token is `}elseif(storage_unplug_value==1)`, not `else if(...)`. The checked dispatch checker otherwise reached this assertion after the USB storage caller-graph PASS. No storage-source defect was indicated.
- THIS COMMIT corrects the normalized token expectation only. The product WAN VLAN source commit `b4c2a6010c765631a5d6d93e4e84521f54fcf72b` is concurrently running exact-head WAN/Process/firewall/XY-C1 checks; those results must be resolved separately.
- Next: verify exact-head USB storage rerun and complete the WAN VLAN exact-SHA checks including Build/TRX/artifact. Do not infer L3 from checker-only jobs.

---

## 2026-11-22 07:26 CST — Fail closed on WAN VLAN configuration read errors

- Parent main SHA: `e8537853a78498db6c0b7fdc1c5fae020dc6675b`.
- `config_vinet_wan()` still read `vlan_filter` with `nvram_match()` and, when enabled, read CPU/IPTV VLAN IDs and CPU priority with `nvram_get_int()`. Transient NVRAM ioctl/read/close errors could masquerade as disabled filtering or zero values, then mutate VLAN TX mappings and interface topology from an incomplete configuration.
- THIS COMMIT reads the selector through `get_nvram_match_checked_status()` and enabled VLAN ID/priority through `get_nvram_int_checked_status()`. Any backend or malformed-integer failure returns before `hw_vlan_tx_map()`, interface changes or WAN VLAN publication. Missing-key defaults and disabled-filter behavior remain compatible; valid CPU priority still uses the low three bits.
- Extended `scripts/check_wan_vlan_setup.py` to pin checked selector/value ordering, fail-before-mutation gates, and removal of the old lossy getters. The existing `wan_nvram_status_policy.c` GNU89/ASan/UBSan regression already covers exact match, missing defaults, integer parsing, malformed/overflow values and backend errors.
- Exact-head WAN route and XY-C1 results for THIS source commit are PENDING. No MIPS Build/TRX/artifact or hardware result is claimed yet.
- Next: inspect exact-head WAN route and XY-C1 jobs, confirm Build/TRX/artifact actually ran, and repair any same-SHA failure. Then continue another independent userland stability/security issue.

---

## 2026-11-22 07:19 CST — Align safe-eject checker with checked storage marker dispatch

- Parent main SHA: `dadb757d3bd255e8a1b49b1b50aa31ef4084db36`.
- Exact parent Actions: Firewall Startup #44960542648 PASS; Process fail-closed #44960542623 PASS; USB modem #44960542688 PASS; AI handoff #44960542624 PASS. XY-C1 #44960542641 completed the full detector/host checks successfully, but Build XY-C1, TRX and artifact upload were SKIPPED because this checker-only SHA contained no production build-input change; it is not an L3 verification. NVRAM transaction audit on source SHA 7e2e #44960208335 PASS.
- USB storage #44960542709 failed only in `scripts/check_storage_safe_remove.py:310`, which still located the now-removed `nvram_match("usb_unplug_ms")` branch. The full storage caller graph passed; the checked deferred dispatch policy binary passed.
- THIS COMMIT replaces that stale assertion with source-order checks for checked unplug/hotplug/Optware reads, pending-on-error, successful-read-only acknowledgement, failed-unmount requeue, checked dispatch and startup gating. Production code unchanged.
- Next: confirm exact-head USB storage rerun PASS. Then perform a distinct source hardening slice and require a same-source XY-C1 Build/TRX/artifact run; do not claim hardware L4.

---

## 2026-11-22 07:12 CST — Align USB marker regression contracts with checked pair dispatch

- Parent main SHA: `7e2e7d4efcb3391b61d7bbe03397f83a57029ae8`; source/test slice `7e2e7d4efcb3391b61d7bbe03397f83a57029ae8` replaces four deferred printer/modem marker `nvram_match()` gates with checked pair reads, failure preservation and five-second retry.
- Exact 7e2e Actions exposed stale contracts, not a confirmed production-source failure: USB modem #44960208387 reached only its old `nvram_match("usb_hotplug_md")` source lookup after all compiled host regressions (including the new pair policy) passed; Firewall Startup #44960208310 and Process fail-closed #44960208466 stopped in `check_modem_wan_start_status.py` at the removed `usb_unplug_md` match; XY-C1 #44960208271 likewise stopped at that global checker before firmware build. USB storage #44960208338 hit a stale newline-sensitive checked-read slice in its whole-caller-graph checker.
- This checker-only slice removes those obsolete assumptions: global WAN-start and USB modem workflow assert checked printer/modem pair reads, read-before-dispatch, no unchecked marker matches, checked modem-rule acknowledgement and requeue/retry order; storage checker slices already-normalized source tokens instead of escaped whitespace.
- Exact 7e2e Actions: HTTPD #44960208345, NFS #44960208262, VSFTPD #44960208366, Samba #44960208251, Process lifecycle inventory #44960208452 and AI handoff #44960209215 PASS. USB modem #44960208387, Firewall Startup #44960208310, Process fail-closed #44960208466, XY-C1 #44960208271 and USB storage #44960208338 FAIL at the stale checker assertions above. NVRAM audit #44960208335 was still IN_PROGRESS at last poll; recheck. No firmware/TRX/artifact L3 claim.
- Next: verify exact-head checker reruns for this commit; inspect the remaining NVRAM and XY-C1 results. Once this regression slice passes, choose a separate unaddressed whole-userland stability/security issue; do not re-audit printer/modem marker reads.

---

## 2026-11-21 — Fail closed on deferred printer/modem USB event-marker reads

- Parent main SHA: `b69c759d94c2091996b78efe196a21eecaa5f3f7`.
- `on_deferred_hotplug_dev()` still used unchecked `nvram_match()` for `usb_unplug_lp`, `usb_hotplug_lp`, `usb_unplug_md` and `usb_hotplug_md`. A transient NVRAM/backend error could look like an absent event and silently skip printer teardown/startup or modem WAN transition without scheduling a retry. The previously checked `modem_rule` read did not protect the marker reads that determine whether an event exists.
- THIS COMMIT reads each unplug/plug marker pair with the checked storage reader and a pure pair-status policy. Both output flags stay clear unless both reads succeed; on error neither marker is acknowledged and no transition runs for that pair, with an explicit five-second retry. Successful values preserve the prior exact numeric value 1 behavior. Printer and modem pairs fail independently.
- Added GNU89/ASan/UBSan regression for no events, coalesced events, non-1 values, each backend error, negative-status normalization and null outputs. USB modem and storage workflows compile/run it and assert checked reads plus absence of all four unchecked marker matches.
- Exact a618 storage/HTTPD failures were stale safe-eject caller-graph assertions; corrected in `b69c759d94c2091996b78efe196a21eecaa5f3f7`. That checker-only SHA's USB storage/HTTPD reruns were pending at last poll. No L3/L4 claim yet.
- Next: validate this USB marker commit together with pending safe-eject and XY-C1 checks; then continue another independent reliability defect.

---

## 2026-11-21 — Update storage/HTTPD safe-eject contracts for checked event dispatch

- Parent main SHA: `a6188035614e198c9983d7dea0bb86d1499b1629`.
- Exact a618 SHA `a6188035614e198c9983d7dea0bb86d1499b1629`: USB storage L2 failed only in its whole-caller-graph checker because it still searched for removed `nvram_match("usb_unplug_ms")` conditions; HTTPD Auth similarly failed its cross-subsystem safe-eject/spindown checker on the same retired dispatch pattern. The new callback deliberately checks unplug/hotplug/Optware markers before acknowledging/dispatching and schedules retry on read error.
- THIS COMMIT updates both independent regression contracts to assert checked marker reads, exact ordering, no unchecked matches in the `USE_STORAGE` branch, unplug cleanup sequencing, and the fail-closed five-second retry. Production source is unchanged in this checker-only commit.
- On exact a618 SHA, VSFTPD, Samba, Firewall Startup, USB modem, NFS, Process inventory and NVRAM audit passed; USB storage and HTTPD failed only at these stale legacy assertions. XY-C1 Firmware Validation was still building; Build XY-C1 active, TRX and artifact upload pending at last poll.
- Next: inspect this commit's exact USB storage/HTTPD and XY-C1 results; repair any source- or contract-level failure before further subsystem work. No L3/L4 claim until this SHA completes firmware/TRX/artifact verification.

---

## 2026-11-21 — Preserve storage hotplug wakeups on NVRAM marker read failures

- Parent main SHA: `711269a797bc8fb06da5cf8e48b6bf449451eb46`.
- The prior checked `usb_opt_start` change still had an unchecked outer dispatch gate: `on_deferred_hotplug_dev()` used `nvram_match("usb_hotplug_ms")` / `nvram_match("usb_opt_start")`. A transient read error could look like no event, so it skipped `try_start_stor_apps()`, left the marker pending without arming retry, and never reached its checked read. The locked inner startup path also used unchecked `nvram_match("usb_hotplug_ms")`.
- THIS COMMIT checks unplug, hotplug and Optware markers before dispatch; read errors keep pending state and arm the five-second retry without consuming the child-result pipe. The inner path independently checks hotplug and Optware flags before reading the worker result; failure unlocks and returns error. Existing value-1 event matching, missing-key default 0, completed-unplug and async-child semantics are retained.
- Added GNU89/ASan/UBSan policy regression for no event, each start trigger, pending child result, unplug suppression, all three marker read failures, negative-status normalization and null output. Storage workflow compiles/runs it and asserts read-before-dispatch/child-result ordering.
- Exact 711 SHA: USB modem #44959196316 PASS and Firewall Startup #44959196327 PASS; Storage #44959059095 PASS, VSFTPD #44959059259 PASS, Samba #44959059221 PASS, HTTPD #44959059070 PASS, NFS #44959059105 PASS, NVRAM #44959059216 PASS. XY-C1 #44959196330 and Process #44959196360 were still running at last poll.
- Next: inspect exact-head Storage/USB/Firewall/XY-C1 and Process Actions for this dispatch change; no L3/L4 claim until the exact source SHA passes XY-C1 Build, TRX and artifact upload.

---

## 2026-11-21 — Normalize source whitespace before modem retry assertions

- Parent main SHA: `b0696dce08423d7467b3fca461a0ef8d35230567`.
- Exact USB modem run #44959059134 reached the shared WAN-start source checker after the new modem policy host test compiled and executed. It failed because `check_modem_wan_start_status.py` applies `compact()` (collapsing whitespace to single spaces) but this new assertion compared against a no-whitespace token string. Firewall Startup #44959059154 failed at the same shared checker line; neither failure indicated a production-source or policy-test defect.
- THIS COMMIT strips whitespace from the extracted deferred-handler body before the new token/order assertions and independently checks unplug-event → replug-event → reconnect-attempt → replug-attempt → retry-mask ordering. Production source and policy behavior are unchanged.
- Exact b069 SHA: VSFTPD #44959059259 PASS, Samba #44959059221 PASS, HTTPD #44959059070 PASS, AI handoff #44959059168 PASS, Process inventory #44959059199 PASS, NFS #44959059105 PASS. USB modem #44959059134 and Firewall Startup #44959059154 failed only at the shared checker; exact XY-C1 #44959059138 and USB storage #44959059095 were still running at last poll.
- Next: inspect corrected exact-head USB modem/Firewall/Storage and XY-C1 jobs; no L3/L4 claim until current source SHA's complete firmware validation passes.

---

## 2026-11-21 — Align modem unplug retry gate with shared source checker

- Parent main SHA: `f29b64cccedf3fddbf8c0e4f04f3c28aefe2d5e0`.
- Exact SHA `0d37d2c1049788176bdbf4b95562a027f5436afc` exposed two defects in the first modem-unplug patch: its static host test was initially nested inside `main()` (moved to file scope by `f29b64cccedf3fddbf8c0e4f04f3c28aefe2d5e0`), and the shared WAN-start checker expected the established `if (unplug_modem)` guard.
- THIS COMMIT keeps that legacy outer guard and nests the checked `modem_rule_status == 0` condition inside it, then extends the shared WAN-start checker to pin checked-read → policy decision → event acknowledgement order, reject the unchecked getter and test the retry state.
- The earlier compile failure is corrected by the separate file-scope test move. Exact current-head USB modem/storage and XY-C1 reruns are pending; no L3/L4 claim until current source SHA completes.
- Next: inspect exact-head USB modem, USB storage, firewall/process and XY-C1 jobs; fix only failures tied to current source/checks.

---

## 2026-11-21 — Preserve Optware startup requests when NVRAM reads fail

- Parent main SHA: `5be0434769a37b94703676e99ed797095626368f`.
- `try_start_stor_apps()` used unchecked `nvram_get_int("usb_opt_start")`. A transient NVRAM backend/read/close error collapsed to zero, so a requested Optware startup could be silently skipped and the deferred callback would see success rather than arm its retry.
- THIS COMMIT checks the value with the shared storage NVRAM reader while holding the storage operation lock and before consuming the asynchronous child's result pipe. On read failure it preserves the pending request, does not consume a completed child status, restores errno and returns failure so the existing five-second deferred retry is armed. Empty storage inventory retains the historical stale-request cleanup; missing key still defaults to zero.
- Extended the storage-hotplug workflow source contract to require checked read → failure/lock release → child-result read ordering and reject the old unchecked getter.
- Exact-head USB storage, USB modem and XY-C1 Build/TRX/artifact checks are pending for this source SHA; no current L3/L4 claim.
- Next: inspect exact-SHA storage/USB modem/XY-C1 results and correct any causal failure.

---

## 2026-11-21 — Keep deferred modem unplug queued when modem_rule cannot be read

- Parent main SHA: `6f8be6b66bd9974390b29556ff9a671d67d9e0b6`.
- Confirmed defect in `on_deferred_hotplug_dev()`: it cleared `usb_unplug_md` before an unchecked `nvram_get_int("modem_rule")`. A transient NVRAM backend/read/close error collapsed to zero, permanently acknowledged the detach and skipped WAN reconnection. If detach and replug were coalesced, the replug could then start against stale WAN transport state.
- THIS COMMIT reads `modem_rule` through the shared checked NVRAM reader and a pure policy helper. A missing key retains the historical zero/no-configured-modem behavior. On a real read failure, the code does not acknowledge the unplug event, records the read error in the existing retry state, blocks a coalesced replug, and uses the existing hotplug retry mask/alarm to retry.
- Added GNU89 modem policy regression cases for missing/positive/negative values, backend error, negative status normalization and null output. The USB-modem Actions source contract now verifies checked-read → decision → event acknowledgement ordering, retry preservation, and the absence of the unchecked getter.
- Current main before this slice: `6f8be6b66bd9974390b29556ff9a671d67d9e0b6`. Exact-head USB modem L2 and XY-C1 Build/TRX/artifact checks are pending for this source change; no L3 or L4 claim.
- Next: inspect exact-head USB modem and XY-C1 results; then continue a separate concrete RC/storage reliability defect.

---

## 2026-11-21 — Align VSFTPD source contract with combined NVRAM failure gate

- Parent main SHA: `21124596ab9442c2c1bb00669c351e66313aea0c`.
- The VSFTPD regression on `a5be39915259794a7a7f17447c401ca9f6f52801` compiled and ran the shared policy test, then failed only because the older connection-limit checker still expected the superseded log text `cannot read max-connection limit`. The follow-up source gate now combines session-limit, mode and syslog read failures under `cannot read FTP configuration`; the checker now validates that actual common abort message before config staging.
- The Samba failure on the same SHA was a separate checker mistake (asserting the new helper in the old Samba-mode header); corrected on parent `21124596ab9442c2c1bb00669c351e66313aea0c`. Both relevant exact-SHA reruns are pending.
- XY-C1 exact run #44958245688, VSFTPD #44958245789 and Samba #44958245707 were previously checked; the new push will launch corrected exact-head checks. No L3/L4 claim yet.
- Next: inspect corrected VSFTPD and Samba jobs, then review their result alongside the XY-C1 build.

---

## 2026-11-21 — Point Samba contract at the shared connection-limit policy

- Parent main SHA: `a5be39915259794a7a7f17447c401ca9f6f52801`.
- Exact Samba source-contract run #44958170852 advanced past the compacted-emission locator but failed at line 99: the new helper belongs to `service_connection_limit_policy.h`, not the pre-existing `samba_config_mode_policy.h`. The checker now reads and asserts against the correct shared header; production source and the GNU89 policy test were unchanged by this workflow-only correction.
- Exact current SHA `a5be39915259794a7a7f17447c401ca9f6f52801` runs #44958245707 (Samba), #44958245789 (VSFTPD), and #44958245688 (XY-C1) were pending at last poll. The next pushed SHA will re-run relevant checks.
- Next: inspect exact-head Samba/VSFTPD/XY-C1 results, then continue the next independent service-hardening slice.

---

## 2026-11-21 — Fail closed on FTP mode and audit-setting NVRAM errors

- Parent main SHA: `88f545440c24761c046eb03d3954287db3088ca4`.
- Follow-up review of `write_vsftpd_conf()` found two remaining unchecked security/observability controls: `st_ftp_mode` (anonymous versus local-auth mode) and `st_ftp_log` (syslog export). On a transient backend failure, the old getters silently normalized the mode to local-auth-only or disabled FTP logs, then replaced the last-known-good config.
- THIS COMMIT reads both settings through the shared checked NVRAM reader before staging `vsftpd.conf`. Missing-key behavior is preserved (mode normalizes to authenticated-local mode; logging remains disabled). A real read error now preserves the existing config. The connection-limit checked-read gate remains in place.
- Extended the VSFTPD source-contract regression to require each read before staging and reject the old unchecked getters. Shared policy GNU89/Werror test and the initial FTP source contract passed exact run #44957924406. Samba checker follow-up #88f5454 is now in exact run #44958170852; pending at this checkpoint.
- XY-C1 run #44957924444 was still executing `Build XY-C1` at last poll. This follow-up has no L3/L4 claim until its own exact-SHA validation.
- Next: inspect current exact-head VSFTPD/Samba/XY-C1 runs and repair any causal failure.

---

## 2026-11-21 — Correct Samba connection-limit workflow source contract

- Parent main SHA: `d2890e3fa3930e64d909df8feeeaff3ad87439a4`.
- Exact VSFTPD run #44957924406 passed, including GNU89/Werror compile+execution of the shared connection-limit policy test and the updated FTP source contract.
- Samba run #44957924306 compiled and ran the new GNU89 policy test successfully, then failed only in the Python source contract: its token included the C string's escaped newline, so the checker could not locate the max-connections emission with `str.index()`. Simplified the locator to the stable `fprintf(fp,"max connections = %d` prefix without weakening the read-before-write order assertion.
- Exact AI handoff docs #44957924341 passed. XY-C1 run #44957924444 was in progress at last poll; no L3 or L4 claim.
- Next: inspect the corrected Samba exact-SHA run and current XY-C1 build, then continue an independent storage defect.

---

## 2026-11-21 — Fail closed on shared FTP/Samba session-limit NVRAM reads

- Parent main SHA: `119e323e545c243b7a0edfe0aa20b2c8483db77d`.
- Both `write_vsftpd_conf()` and `write_smb_conf_unlocked()` used unchecked `nvram_get_int("st_max_user")`. A transient NVRAM/backend/close failure could be read as zero, silently clamp both daemons to one connection, and publish an unintended service configuration.
- Added one shared checked reader and a pure policy preserving the historical 1..`MAX_CLIENTS_NUM` clamp for successful/missing values. Actual read errors now abort FTP config generation before opening its staged file and abort Samba staged publication through its discard path. Existing active configs remain untouched on failed session-limit reads.
- Added GNU89/Werror policy tests for valid, missing/low/high, API/close-error, invalid maximum and null-output cases. Both VSFTPD and Samba workflows compile/run the test and enforce source-order/fail-closed contracts; both workflow path filters include the new files.
- Local MIPS/uClibc firmware build is unavailable in this GitHub-only workspace. Exact-head VSFTPD/Samba Actions and XY-C1 Build/TRX/artifact results are pending for this source commit; no L3/L4 claim.
- Next: inspect those exact-SHA runs and fix any real failure. Then continue an independent storage/network stability slice.

---

## 2026-11-21 — Close MAC-filter and static-route hardening; stabilize contracts

- MAC filter mode and per-entry checked-read hardening landed in `8dd8a5bcb515b752dc9c4fd07e11440e929d5c49`; exact XY-C1 #44955441811 passed Build/TRX/artifact.
- Static routes now enforce the actual 64-row UI limit, read `sr_enable_x` and entry fields through checked APIs, validate metrics/gateways/interfaces, preserve invalid-gateway fallback, and safely retain up to 128 persisted entries. Max snapshot 7,552 bytes fits the 8,192-byte buffer. Latest product-source commit `182cfee52473b9f072816c6e5f67ef727945143e`.
- Fixed the escaped Delete-button checker (`37d0b7b`), storage IPTV dispatch whitespace checker (`f3a1cdda`), and WAN-route workflow source-contract check (`923f8c64`). Exact checker-head WAN route #44956642473 and AI docs #44956642509 passed.
- Exact product-source `182cfee` Firewall Startup #44956596799, WAN DHCP #44956596822, AI docs #44956596834 and HTTPD check-run passed at last poll. XY-C1 #44956596804 was still in progress; no current-head L3 claim. Hardware L4 remains NOT VERIFIED.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-21 — Stabilize exact-head Firewall/HTTPD source contracts

- Parent main SHA: `ef324cc77a34420655ab63cb856120c5fad59dc7`.
- Firewall Startup #44953373884 passed the first three compiled regression steps, then failed at the LWF checked-reader source contract. The generic C function-body parser yielded a brittle slice/count; changed the contract to take the reader's actual source range and count both checked reads directly.
- HTTPD #44953373906 passed the URL sanitizer, firewall token-validation and VPN lease checks, then failed on an exact total count of `firewall_rule_file_remove(ipt_file)`. The checked URL/LWF abort paths legitimately add cleanup sites. Changed the test to require at least the existing baseline count, so future safe abort paths do not break an unrelated invariant.
- Parent `ef324cc...` other exact-head results: WAN route #44953373909 PASS; Process #44953373841 PASS; AI docs #44953373949 PASS; FULLCONENAT #44953373928 PASS; WAN DHCP #44953373881 PASS. NVRAM #44953373840 and XY-C1 #44953373845 were in progress.
- No local test/build or L3/L4 evidence.

---

## 2026-11-21 — Harden LWF schedule reads and IPv6 URL rule framing

- Parent main SHA: `ee47e7f79e400d79361a62d2582816fd54652ad1`.
- Lightweight Filter activation still used unchecked `nvram_get()` for date/time keys. Backend/API/close failures could be confused with missing keys, selecting historical all-days/all-day defaults. Added a presence-aware checked reader and LWF-specific formatter: only absent keys receive defaults; present-empty/invalid/overlong values remain inactive; actual read/close errors abort the staged IPv4 restore. Each NVRAM value is copied before the next lookup.
- Extracted a pure schedule formatter while preserving the legacy URL/MAC wrapper. Added C policy tests for missing versus present-empty schedule values and Firewall Startup contracts for checked date/time reads and abort ordering.
- Corrected the IPv6 URL rule emitter's double-escaped newline so the generated iptables-restore rule terminates normally.
- Fixed `ee47e7f` workflow defects: Firewall Startup used undefined `re` in its new symbol assertion; HTTPD still expected the removed unchecked LWF expression. Both now test the current production API.
- Parent `ee47e7f` Actions: AI docs #44953071474 PASS; Firewall Startup #44953071424 FAIL (`NameError: re is not defined`); HTTPD #44953071413 FAIL (stale LWF schedule assertion). No local build/test or L3/L4 claim.

---

## 2026-11-21 — Correct URL-filter source-contract assumptions

- Parent main SHA: a16cd7b95d19ccd9057fdf117d794071ead768c4.
- Firewall Startup #44952833879 failed at Python stdin line 500. The contract used a Python string containing a double-escaped newline, so it searched for a literal backslash+n between static int and ip6t_filter_rules rather than the real source newline. Replaced with whitespace-tolerant regex.
- HTTPD #44952833897 failed at Python stdin line 793 with ValueError: substring not found because its URL-filter source contract still searched for the old include_webstr_filter(FILE *fp) signature after the checked bounded-list change added the num_entries parameter. Updated the contract to the actual signature.
- These are checker-only corrections; no production source behavior changed. On parent a16cd7..., WAN route #44952833895, FULLCONENAT #44952833882, WAN DHCP #44952833872, Process #44952833889 and AI docs #44952833861 PASS; NVRAM #44952833900 and XY-C1 #44952833916 were in progress. Firewall Startup #44952833879 and HTTPD #44952833897 failed only at the stale assertions above.
- No local test/build or L3/L4 claim.

---

## 2026-11-21 — Restore checked IPv6 URL-filter staging

- Parent main SHA: `4f59f6c72e1d35b323ee4c622bfdbf8f8be23da9`.
- URL keyword loading had been converted to a bounded two-argument checked helper, but the IPv6 filter call site still used the old one-argument call and the IPv6 function declaration was accidentally joined as `static intip6t_filter_rules`. That left the IPv6 URL path outside the new fail-closed contract and broke the function symbol/compile contract.
- Restored the `static int` / `ip6t_filter_rules()` declaration. The IPv6 builder now passes its preflighted 0..128 count, and on keyword read failure closes/removes its staged file, aborts the core restore transaction, preserves errno and returns failure before publication.
- Strengthened Firewall Startup source contracts for the exact IPv6 symbol, bounded helper call and abort-before-finish ordering.
- Parent 4f59f6c exact push runs: Firewall Startup #44952467207 and HTTPD #44952467237 failed in source contracts; NVRAM #44952467177, Process #44952467224, AI docs #44952467200, WAN DHCP #44952467197, FULLCONENAT #44952467198 and WAN route #44952467210 passed. XY-C1 #44952467208 remained in progress before Build/TRX/artifact. HTTPD's VPN source-order contract is corrected to scope the abort after the VPN include call.
- No local compile/test or same-SHA L3 evidence; hardware L4 remains unverified.

---

## 2026-11-21 — Bound URL filter entries and check keyword reads

- Parent main SHA: `3566f46ad6d8521fb7283617c5044bb38521e3dc`.
- The URL filter WebUI permits at most 128 entries, but `include_webstr_filter()` used the generic `foreach_x("url_num_x")` loop and repeatedly evaluated an unchecked persistent count. It also read each `url_keyword_xN` with `nvram_safe_get()`, so a backend/close failure became an empty keyword and silently omitted a deny rule.
- Added the checked `firewall_read_url_entry_count()` and a 0..128 policy predicate, with unit tests for 0,128,-1,129. Both IPv4 and IPv6 filter builders preflight/cache the count before opening their rule files and pass it to the URL-rule emitter.
- URL keyword values now use the checked indexed-string reader. Missing keys retain empty/skip behavior; a failed checked read returns an error. Both builders close/remove the staged rule file, abort the core restore transaction and return without publishing a partial URL policy.
- Firewall Startup source contracts cover the entry bound, preflight ordering, checked keyword reads, bounded loop and IPv6 error propagation. Exact-head CI is pending.
- Parent `3566f46...`: all nine triggered runs (Firewall #44952192432, NVRAM #44952192527, AI docs #44952192530, FULLCONENAT #44952192469, XY-C1 #44952192509, Process #44952192541, WAN DHCP #44952192483, HTTPD #44952192648, WAN route #44952192537) were IN_PROGRESS at last poll. No local test/build or L3/L4 claim.

---

## 2026-11-21 — Fail closed on Lightweight Filter field-read errors

- Parent main SHA: `7494b0860d16d6cc9d4e9a812a22db29bd7aa1ac`.
- Follow-up inspection found LWF per-entry protocol, TCP flag, port and IP-range converters still called `nvram_safe_get()`; a NVRAM backend/close error therefore looked like an empty or default value. For a blacklist, a failed port/IP read could omit a configured DROP rule or broaden an empty wildcard; a protocol failure could default to TCP. The ICMP list was also read unchecked, allowing failed reads to omit blacklist drops.
- Added checked string and indexed-string readers using `nvram_get_checked()` plus `firewall_nvram_read_status()`. Migrated LWF protocol flag, port-range and IP-range conversions, and the ICMP list. Missing keys retain legacy empty-string/default behavior; successfully read malformed values still take the prior validation/skip path.
- LWF rule generation now returns an error immediately on checked-read failure. `ipt_filter_rules()` detects the negative result, closes the staged file, aborts the firewall restore transaction, removes the temporary file and returns without publishing a partial or broadened ruleset.
- Extended Firewall Startup source contracts to require checked indexed reads, checked ICMP read and propagation into transaction abort. Existing policy/NVRAM regression tests cover the shared read-status and missing-key semantics; exact-head workflow must compile and run them.
- Parent `7494b0860d16d6cc9d4e9a812a22db29bd7aa1ac`: Firewall Startup #44951908711 PASS; Process #44951908748 PASS; AI docs #44951908732 PASS; NVRAM #44951908715 still in progress. No local tests/build or L3/L4 claim.

---

## 2026-11-21 — Correct LWF NVRAM test sentinel setup

- Parent main SHA: `02389c723fbe0aa18474da899f85f483a6135064`.
- Firewall Startup #44951788974 failed in `test_lightweight_filter_drop_mode_match_uses_legacy_defaults`. The new assertion intended to test that a backend failure leaves the output parameter unchanged at 23, but the same test had already set it to 0 with a successful mismatch and a missing-key default. The final expectation was therefore wrong.
- Reset the sentinel to 23 immediately before the backend-error call. This changes only the regression test, not runtime code or legacy semantics. Exact-head rerun is required.
- Parent Actions: AI docs #44951789062 PASS; Firewall Startup #44951788974 FAIL on the test expectation above; NVRAM #44951789060, HTTPD #44951788996, Process #44951789016, FULLCONENAT #44951789081, WAN route #44951788962, WAN DHCP #44951789018 and XY-C1 #44951788955 were still running. XY-C1 had not yet reached Build. No local test/build or L3/L4 claim.

---

## 2026-11-21 — Snapshot Lightweight Filter mode and bounded rule count

- Parent main SHA: `fc208c6baf1956855799c3d3320b5d88ae440d7a`.
- The LWF UI caps `markGroupLWF(..., 64, " Add ")`, but `include_lw_filter()` used `foreach_x("filter_lw_num_x")`, which repeatedly evaluated an unchecked NVRAM count. More seriously, `nvram_match("filter_lw_default_x", "DROP")` collapsed backend/close failures into false, silently changing a configured whitelist (per-rule ACCEPT plus terminal DROP) into blacklist behavior and potentially removing the terminal drop.
- Added `firewall_read_match_setting()` to retain exact-match/missing-key semantics while propagating NVRAM errors. Added checked `firewall_read_lw_entry_count()` and the policy predicate `firewall_lw_entry_count_is_safe()` (0..64).
- `ipt_filter_rules()` snapshots the mode and count before opening its filter rules file. `include_lw_filter()` now receives cached values and uses a bounded loop; it no longer re-reads the default mode or count during rule generation.
- Added filter-policy unit tests for counts 0,64,-1,65 and NVRAM status tests for exact DROP match, ACCEPT mismatch, missing-key blacklist default and backend-error output preservation. Added source-contract assertions for checked preflight before rule-file staging and for the bounded loop.
- Parent exact-head SHA `fc208c6...`: Firewall Startup #44951481093 PASS; HTTPD #44951481068 PASS; HTTPD lifecycle #44951481171 PASS; FULLCONENAT #44951481035 PASS; WAN DHCP #44951481067 PASS; WAN route #44951481066 PASS; Process lifecycle #44951481091 PASS; Process inventory #44951481191 PASS; AI docs #44951481072 PASS. NVRAM #44951481074 and XY-C1 #44951481069 remained IN_PROGRESS; Build XY-C1 was running. No local tests/build or L3/L4 claim.

---

## 2026-11-21 — Restore VTS protocol mapping semantics and correct count contract

- Parent main SHA: `b60f5d95776c8f7cc78d30edc7c6c2c6bfe032a3`.
- Firewall Startup run #44951308314 on parent b60 failed at Python stdin line 405 with `AssertionError: VTS filter`. The newly added `body()` source parser returns a function body without its signature, but the contract incorrectly looked for `int num_entries` inside that body. Updated the workflow to assert the complete signatures against `firewall_source` and the bounded loop inside each parsed body.
- Source review of parent b60 also found a production behavior regression: `proto_conv()` returned `tcpudp` for BOTH (where filter/NAT emitters require `both`) and omitted OTHER, making an OTHER rule fall back to TCP. Added a tested protocol normalizer that preserves legacy `tcp`, `udp`, `both`, `other` tokens, as well as TCP fallback for absent/unrecognized values, while checked NVRAM backend errors still return NULL and skip only that mapping.
- Hardened `firewall_nvram_read_status()` to reject the invalid API combination “missing key with non-NULL value”. Regression now covers the malformed combination and all legacy protocol mappings.
- Parent b60 exact-head checks: Firewall Startup #44951308314 FAIL as above; HTTPD #44951308243, HTTPD lifecycle #44951308276, FULLCONENAT #44951308225, WAN route #44951308283, WAN DHCP #44951308315, Process #44951308244 and AI docs #44951308326 were IN_PROGRESS; NVRAM #44951308249 QUEUED; XY-C1 #44951308274 IN_PROGRESS. No local test/build or L3/L4 claim.

---

## 2026-11-21 — Fail closed on VTS protocol NVRAM backend errors

- Parent main SHA: `79cfb79099f40e3b67b6c23a520c623c422c2d16`.
- The shared VTS `proto_conv()` path used the unchecked legacy getter and defaulted an empty/unknown protocol to TCP. Because legacy NVRAM APIs also collapse transient backend/close failures to an empty value, a failed read could silently convert a configured UDP rule into a TCP mapping/ACCEPT exception.
- Added `firewall_nvram_read_status()` to distinguish present, absent and failed `nvram_get_checked()` results. Its regression covers valid/missing, explicit backend errno, errno-less backend failure, invalid API return and NULL-on-present cases.
- `proto_conv()` now builds the dynamic indexed key and reads it through `nvram_get_checked()`. Absent and unrecognized values retain historical TCP fallback; backend failure returns NULL. `include_vts_filter()` and `include_vts_nat()` log and skip the single affected mapping, failing closed without discarding unrelated firewall rules.
- Extended Firewall Startup's source checker to require the checked status path and both skip-on-error call sites. No local build/test execution claimed; exact-head Firewall Startup and XY-C1 checks pending.

---

## 2026-11-21 — Bound virtual-server rule count before firewall generation

- Parent main SHA: `aea6b95138171a165edec978680d6fb14913a640`.
- The UI's `Advanced_VirtualServer_Content.asp` limits additions using `markGroupVS(..., 64, " Add ")`, but both `include_vts_filter()` and `include_vts_nat()` used `foreach_x("vts_num_x")`, which repeatedly read an unchecked persistent count as the loop condition. A corrupted or oversized count could make firewall generation iterate far beyond the 64 supported entries; read failures were also not distinguished from a legitimate empty list.
- Added `firewall_vts_entry_count_is_safe()` (0..64), a checked `firewall_read_vts_entry_count()`, and a cached count parameter to the VTS filter/NAT builders. Both IPv4 builders now read `vts_enable_x` and, when enabled, validate the count before opening rule files; all three VTS rule-generation call sites use the same bounded snapshot.
- Added policy unit cases for counts 0, 64, -1 and 65. The workflow source contract checks preflight order, cached count use and removal of the unchecked `foreach_x` loops. Existing filter-policy CI compiles with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan.
- Parent exact-head `aea6b951...`: Firewall Startup #44950937394, AI docs #44950937391, HTTPD #44950937384, FULLCONENAT #44950937401, WAN DHCP #44950937407, WAN route #44950937440 and Process #44950937421 PASS; NVRAM #44950937375 and XY-C1 #44950937424 remained IN_PROGRESS. No local test/build or L3/L4 claim.

---

## 2026-11-21 — Bound UDPXY listener ports to valid unprivileged range

- Parent main SHA: `e1e48877d521ef9b4559ade13568ff4d97de59ad`.
- Source review found UDPXY's checked integer setting was still only tested as `> 1023` in the IPv4 filter, IPv6 filter and IPv4 NAT builders. A corrupted value such as `65536` could therefore be emitted as an invalid iptables `--dport` token (and in NAT as a destination port).
- Added `firewall_unprivileged_port_is_safe()` to the shared filter policy and applied it to all three builders' preflight and rule-emission branches. Range is 1024..65535 inclusive, retaining the intended privileged-port exclusion.
- Added GNU89 policy regression cases for 0, 1023, 1024, 65535, 65536 and negative values; the Firewall Startup workflow source contract requires the predicate in IPv4 filter, IPv6 filter and IPv4 NAT rule builders and confirms the unit cases remain present. The existing workflow compile step uses GNU89/Werror/ASan/UBSan.
- Parent exact-head SHA `e1e48877...`: Firewall Startup #44950737943, Process #44950737903, WAN DHCP #44950737926, AI docs #44950737855, HTTPD #44950738016, WAN route #44950737937 and FULLCONENAT #44950737844 PASS; NVRAM #44950737906 and XY-C1 #44950737940 were IN_PROGRESS. No local tests/build or L3/L4 claim.

---

## 2026-11-21 — Restore IPv6 HTTP listener block boundary

- Parent main SHA: `4cd3b21733ad373d2cb0f9adb418bef2df277458`.
- Firewall Startup run #44950535060 failed at Python stdin line 314. The workflow's brace-aware `body()` parser found `ip6t_filter_rules()` unterminated. The IPv6 HTTP listener conditional opened a block but did not close it before the HTTPS compile section, unintentionally nesting subsequent WAN listener and VPN exceptions under the HTTP-enable predicate.
- Closed the HTTP listener block, corrected its indentation and added a source-contract assertion that it ends before the HTTPS section. This ensures the workflow rejects the same accidental control-flow nesting.
- Parent exact-head checks: HTTPD #44950534996, AI docs #44950535073, Process #44950535022, WAN route #44950534986, WAN DHCP #44950534973 and FULLCONENAT #44950534983 PASS. Firewall Startup #44950535060 FAIL as above; NVRAM #44950535055 and XY-C1 #44950534974 were IN_PROGRESS. No local C/build test or L3/L4 claim.

---

## 2026-11-21 — Guard optional filter listener caches by feature macro

- Parent main SHA: `8a7cf932879931c7df25708a9290dcc8eba2ed89`.
- The listener-preflight slice declared HTTPS/SSH/FTP cached enable flags unconditionally although their reads/uses are compile-guarded by `SUPPORT_HTTPS`, `APP_SSHD` and `APP_FTPD`. This could generate unused-local warnings on board configurations that omit one of those optional services.
- Moved each cache declaration under its matching feature macro in both IPv4 and IPv6 filter functions. The source behavior and guarded read/use paths are unchanged.
- Exact-head Firewall Startup #44950485651, XY-C1 #44950485679 and the parallel HTTPD/NVRAM/Process/AI checks were in progress at last poll; the follow-up needs fresh exact-head validation.

---

## 2026-11-21 — Check IPv4/IPv6 remote-listener filter selectors

- Parent main SHA: `13a229e14833708e70f0da7d541fb4aee8396459`.
- Both filter builders still used unchecked port reads after firewall rules were staged: HTTP/HTTPS listener pairs, SSH/FTP WAN ports, UDPXY internal/external ports and Transmission/Aria primary/remote ports. Their service flags were also read through unchecked exact-match helpers. This could desynchronize filter exceptions from the already-checked NAT mapping and enabled service configuration.
- Added preflight reads before `firewall_open_rule_file()` in both `ipt_filter_rules()` and `ip6t_filter_rules()`. Exact service-enable and remote-open flags now use `firewall_read_enabled_setting()`; listener ports and HTTP protocol use strict `firewall_read_integer_setting()`. Reads are conditional on protocol, compiled feature and active service/remote-open state. Existing Transmission/Aria port range fallback (1024..65535 to built-in defaults) is preserved.
- Rule bodies now use cached settings rather than re-reading NVRAM. Extended Firewall Startup CI source contracts to require every checked flag/port in both functions before file-open and ban the previous unchecked getters. Existing helper tests run under GNU89/Werror/ASan/UBSan. No local test execution claimed.
- Parent `6e67...` Firewall Startup #44950120695, AI #44950120745, Process #44950120902, WAN route #44950120777, FULLCONENAT #44950120775 and WAN DHCP #44950120688 PASS; HTTPD #44950120693 failed on the old DMZ literal corrected by `13a229...`; NVRAM #44950120698 and XY-C1 #44950120665 were pending at Build XY-C1. Current exact-head checks pending; no L3/L4 claim.

---

## 2026-11-22 — Correct stale HTTPD firewall source assertion

- Parent main SHA: `6e67a918cf560634fa597fcdfd2ea35df96a78c3`.
- Exact-head HTTPD Security #44949890929 on `da1695156cebf82ac7e8c4e4052c44a4a370a5f1` failed at Python stdin line 874. The failing check required unnecessary parentheses around the already-safe DMZ IPv4 predicate assignment; the actual production statement is equivalent and had been introduced in the preceding source commit. The earlier compiled and HTTPD security checks had passed.
- Updated `.github/workflows/httpd-security.yml` to assert the actual assignment with whitespace-tolerant matching. No production source changed. Added this exact failure diagnosis and the next IPv6 listener/port-forward task to the handoff.
- On parent `6e67a918cf560634fa597fcdfd2ea35df96a78c3`, all nine push Actions (Firewall, XY-C1, NVRAM, WAN route, FULLCONENAT, WAN DHCP, HTTPD, Process inventory and AI docs) were IN_PROGRESS at the last live query. Fresh exact-head checks are required after this checker commit. No local workflow/test or L3/L4 claim.

---

## 2026-11-21 — Use checked OpenVPN protocol cache in both filter families

- Parent main SHA: `da1695156cebf82ac7e8c4e4052c44a4a370a5f1`.
- Source review found four stale `i_ov_prot` references remaining across `ipt_filter_rules()` and `ip6t_filter_rules()`. Their old local getter declarations had already been removed when the checked `i_vpns_ov_prot` value was introduced, so APP_OPENVPN builds could fail on an undeclared identifier.
- Replaced all four references with the checked function-scope `i_vpns_ov_prot` cache, preserving the original protocol-value sets. Added a Firewall Startup source-contract assertion prohibiting `i_ov_prot` in the complete source file.
- Parent `da169...` Firewall Startup #44949891033, AI docs #44949890928, Process inventory #44949890931, WAN route #44949890950, FULLCONENAT #44949890994 and WAN DHCP #44949890952 PASS. HTTPD #44949890929 failed its final Python source contract at line 874 (investigate separately); NVRAM #44949890937 and XY-C1 #44949891023 were in progress. Current exact-head validation pending. No local build/test claim.

---

## 2026-11-21 — Check Transmission/Aria2 DMZ exception selectors

- Parent main SHA: `f35638a262b7d0ce797aa41576161713933fb687`.
- When the IPv4 DMZ catch-all is active, unchecked `trmd_enable` / `aria_enable` failures could suppress the local-router RETURN exception and allow the DMZ host DNAT to receive packets destined for a router daemon. Unchecked primary/remote port reads could also use fallback values inconsistent with the actual listener.
- Moved DMZ target detection before NAT rule-file opening. If a valid DMZ target and daemon binary exist, exact service enable / remote-open flags and primary / remote ports are checked via the shared helpers; the old default port-range fallback is retained. The rule body uses cached values only. Backend/parser/close failures now abort before a DMZ catch-all can publish an incomplete exclusion set.
- Extended the Firewall Startup source checker to require all eight checked settings before rule-file creation and reject their unchecked legacy accessors. No local tests claimed.
- Parent `f356...` Firewall Startup #44949762834, Process inventory #44949762882, WAN route #44949762889, HTTPD #44949762946, FULLCONENAT #44949762933 and WAN DHCP #44949762871 PASS; AI docs #44949762939, NVRAM #44949762990 and XY-C1 #44949762845 were pending. Current exact-head checks pending; no L3/L4 claim.

---

## 2026-11-21 — Restore SSH/FTP enabled gates in NAT listener preflight

- Parent main SHA: `29a7c9a5e41bf8444a1a782b26ad0ecb2b8f1e79`.
- Exact-head Firewall Startup #44949619993 failed in `Verify required NAT restore gates WAN startup`: the source checker expected `firewall_read_not_zero_setting("sshd_enable", &is_sshd_enabled)` and the FTP equivalent inside `ipt_nat_rules()`. The previous listener-port change had replaced the earlier service-enable reads and omitted them in the new block, leaving the locals at legacy-enabled initial values.
- Restored both checked inverted enable reads before the HTTP/HTTPS/SSH/FTP/UDPXY port checks. The same workflow assertion remains in place and now guards against this regression.
- Parent 29a AI docs #44949620120 PASS; Firewall Startup #44949619993 FAIL (fixed here); NVRAM #44949620067 and XY-C1 #44949620017 were running. New exact-head validation pending; no L3/L4 claim.

---

## 2026-11-21 — Fail closed on IPv4 remote-DNAT listener port reads

- Parent main SHA: `70c0d4c097d68177d698bc88031f829d2a6ecd9a`.
- `ipt_nat_rules()` used unchecked `nvram_get_int()` for HTTP/HTTPS, SSH, FTP and UDPXY listener ports. A backend error could be collapsed to zero and then interpreted as an invalid configured port, invoking legacy fallback values (notably SSH 10022, HTTP 8080 and HTTPS 8443) and creating a new WAN DNAT mapping. UDPXY could fall back to its internal port.
- Added checked integer preflight for the HTTP protocol and only the active/externally-open listener pairs before `firewall_open_rule_file()`. Existing missing-key zero defaults and later port-range normalization remain intact; failed backend/parser/close checks now abort before a different WAN-facing port is published.
- Extended the firewall workflow source contract to require each of the nine listener-setting reads before NAT rule staging and forbid the former direct integer getters. Existing GNU89/Werror/ASan/UBSan helper coverage verifies strict integer/default/backend semantics. No local test execution claimed.
- Parent `70c...` Firewall Startup #44949461661 and AI #44949461610 passed; XY-C1 #44949461605 and NVRAM #44949461623 were running. Current exact-head checks pending, no L3/L4 claim.

---

## 2026-11-21 — Check IPv6 VPN exception policy reads

- Parent main SHA: `f30820395919fb0650a4f0e4ad3889115b1206cc`.
- `ip6t_filter_rules()` still used unchecked exact `vpns_enable` matching, `nvram_get_int("vpns_type")`, and unchecked OpenVPN protocol/port values when emitting WAN ACCEPT exceptions. If the server was configured for another VPN type but the type read failed to zero, the firewall could instead emit the legacy PPTP TCP/47 holes.
- Added checked exact enable and strict integer/type reads before ruleset staging. For enabled type-2 OpenVPN servers, protocol and port are also checked and cached; invalid-range ports retain the 1194 default. The rules builder now reuses the validated settings rather than silently selecting PPTP or a mismatched OpenVPN exception on NVRAM failure.
- Extended the firewall workflow source contract to require all checked gates before `firewall_open_rule_file()` and reject the direct getters. Shared host helper regression covers integer parsing, missing defaults and backend errors under GNU89/Werror/ASan/UBSan.
- Parent `f308...` Firewall Startup #44949377270, AI #44949377241 and XY-C1 #44949377343 were in progress; NVRAM audit #44949377281 was pending. New exact-head checks pending; no L3/L4 claim.

---

## 2026-11-21 — Fail closed on IPv6 filter inverted enable selectors

- Parent main SHA: `3447846b0f81fb252f313cd6c7fe6c429645e673`.
- `ip6t_filter_rules()` still used unchecked inverted NVRAM reads for ping/ICMPv6, SSH enable, FTP enable and UPnP. A backend failure could be interpreted as enabled due to legacy empty-string comparison. The ping selector was read three times across INPUT, FORWARD and traceroute rules.
- Added checked preflight reads before opening the IPv6 rules file. One `misc_ping_x` result is reused for all three rules; SSH/FTP remain behind their APP feature macros; UPnP is checked before pinhole-chain publication. Missing keys preserve the old `nvram_invmatch(..., "0")` semantics; backend failures abort staged rule generation.
- Extended `firewall-startup-stability.yml` to require all checked selectors before `firewall_open_rule_file()` and reject the old inverted getters. Existing GNU89/Werror/ASan/UBSan helper regression covers inverted defaults and backend errors. No local test execution claimed.
- Parent NAT commit `3447846b...` exact-head Firewall Startup #44949286687, AI #44949286648, Process #44949286668, NVRAM #44949286631 and XY-C1 #44949291903 were in progress at last poll. Current exact-head checks pending; no L3/L4 claim.

---

## 2026-11-21 — Check IPv4 NAT builder VPN and exposure selectors

- Parent main SHA: `27cee43d08d3a66c741b0ca61e1496d243e7bd80`.
- `ipt_nat_rules()` still had a separate set of unchecked reads for `wan_nat_x`, VPN enable/type and `vpnc_sfw`, plus inverted UPnP/SSH/FTP gates. The DMZ VPN exceptions also reread OpenVPN mode/protocol/port values through unchecked accessors. These values select NAT, VPN masquerade/PREROUTING, UPnP chain jumps and WAN remote-management DNAT.
- Replaced the NAT and VPN selectors with checked readers before `firewall_open_rule_file()`. Conditional server `vpns_vuse/actl` and enabled type-2 OpenVPN server/client mode/protocol/port fields are checked and reused in the DMZ policy. Out-of-range OpenVPN ports retain the 1194 default.
- Replaced inverted UPnP, SSH and FTP enable reads with the checked not-zero wrapper, retaining missing-key defaults but propagating backend errors. Extended the firewall workflow with NAT-builder exact source-order assertions and checks forbidding the old getters. Existing helper test runs under GNU89/Werror/ASan/UBSan in CI. No local test execution is claimed.
- Parent `27cee...` Firewall Startup #44948947193, Process inventory #44948947321, AI docs #44948947163, HTTPD #44948947166, FULLCONENAT #44948947177, WAN route #44948947181 and WAN DHCP #44948947214 all PASS. Parent XY-C1 #44948947161 and NVRAM audit #44948947164 were still running at last poll. Current exact-head checks pending; no L3/L4 claim.

---

## 2026-11-21 — Reuse checked ping selector in IPv4 FORWARD policy

- Parent main SHA: `fb8b82f4af18601e5745929fc0bc10d9b4bd8919`.
- Final scan of `ipt_filter_rules()` found the INPUT-chain `misc_ping_x` inverted read was converted, but a second FORWARD-chain occurrence remained because the original replacement changed only its first occurrence.
- Replaced the remaining FORWARD-chain `nvram_invmatch("misc_ping_x", "0")` with the same pre-checked `is_ping_enabled` value used by INPUT. Both rule sections now share one checked read before staging; backend errors abort ruleset construction.
- Existing firewall workflow source contract rejects any direct inverted getter for `misc_ping_x` in the complete function; current exact-head Firewall Startup and XY-C1 validation pending.

---

## 2026-11-21 — Fail closed on inverted firewall enable selectors

- Parent main SHA: `bbd199ab0458296efacd741b473d0ef6d3853f93`.
- `ipt_filter_rules()` still used unchecked `nvram_invmatch(key, "0")` for `misc_ping_x`, `udpxy_enable_x`, optional `xupnpd_enable_x`, `sshd_enable`, `enable_ftp` and `upnp_enable_x`. Legacy NVRAM access returned an empty string on read failure, and empty string compares unequal to `"0"`, so a transient backend fault could be mistaken for an enabled option.
- Added a strict checked not-zero policy wrapper. Absent keys retain the legacy inverted-match default (enabled); backend/API/close failures preserve output, abort ruleset staging and invoke rollback. IPv4 filter generation now resolves each applicable inverted selector before opening its rule file; optional feature flags remain compile-guarded, and UPnP is read only when NAT is enabled.
- Extended the header policy tests for explicit zero/nonzero/unexpected values, missing-key defaults and backend errors. Extended the firewall workflow source contract to pin each read before rule-file creation and forbid the old direct inverted match. The workflow's existing GNU89/Werror/ASan/UBSan compile/run step covers this C regression. No local checkout/test execution is claimed.
- Parent Firewall Startup #44948667798, Process inventory #44948667779 and AI handoff #44948667780 all PASS. Parent NVRAM audit #44948667802 and XY-C1 #44948667816 were pending at last poll. Current exact-head checks pending; no L3/L4 claim.

---

## 2026-11-21 — Fail closed on firewall VPN/NAT NVRAM policy reads

- Parent main SHA: `a727f2e78a42fac20d2558dee63ac78673551c79`.
- `ipt_filter_rules()` still selected NAT and VPN policy from unchecked `nvram_match()` / `nvram_get_int()` calls. A backend failure for `vpnc_sfw` became `0`, which emits ACCEPT for the VPN-client interface; `vpns_actl` failures also became the broad legacy mode `0`. This could publish a changed and potentially more permissive firewall ruleset during transient NVRAM failure.
- Replaced unchecked `wan_nat_x` with `firewall_read_enabled_setting()` and all six VPN server/client enable/type/traffic-policy integers with `firewall_read_integer_setting()`, each checked before rule-file creation. Conditional OpenVPN server mode/protocol/port and client TUN/TAP mode flags now use the same checked reader only when an enabled type-2 endpoint needs them. The OpenVPN server port still defaults to 1194 when outside 1..65535, matching its prior safe_get_int range behavior.
- Extended `firewall-startup-stability.yml` source-contract assertions to require checked readers before `firewall_open_rule_file()` and reject the old unchecked key reads. The existing `firewall_nvram_match_policy.c` host regression covers absent defaults, strict decimal behavior, malformed/overflow input and backend errors; that workflow compiles/runs it under GNU89/Werror/ASan/UBSan. No local checkout/test execution is claimed.
- Exact-head Firewall Startup and XY-C1 validation pending; no L3 or hardware L4 claim.

---

## 2026-11-21 — Normalize USB/XHCI source-checker whitespace

- Parent main SHA: `692c260ecf9d49ab44b2baefcb4895d1a4190d7f`.
- Exact Process lifecycle integration #44948258619 reached the final integration checker after all previous regression cases passed, then failed at `scripts/check_process_lifecycle_integration.py:1932`: the checker expected the checked `usb3_disable` call as a single-line literal even though `rc.c` wraps its arguments.
- Correct the checker by compacting whitespace in the extracted `load_usb_modules()` body before comparing the read call, error block, zero fallback, module-parameter formatting and XHCI/MTK load order. This is a checker-only correction; no production source is changed.
- Source-parent Process inventory #44948258854, USB modem #44948258515, Firewall Startup #44948258642, WAN route #44948258490 and AI handoff #44948258596 passed. Parent XY-C1 #44948258645 and NVRAM transaction audit #44948258687 were in progress at the last poll. Exact-head Process rerun required; no L3 claim.

---

## 2026-11-22 — Check MT7621 XHCI USB3 NVRAM module parameter

- Parent main SHA: `5f61f0345e3bdddf7fa21219eb16a316801920ea`.
- `load_usb_modules()` already handled `modem_rule` and `modem_type` via `nvram_get_checked()` and the shared strict decimal parser, but the `USE_USB_XHCI` path still used `nvram_get_int("usb3_disable")`. Backend errors, malformed text and overflow could silently become zero, changing the XHCI module configuration while reporting a successful read.
- Replaced the unchecked read with `rc_boot_nvram_read_int_status("usb3_disable", 0, ...)`. Missing keys preserve the legacy zero default; valid integer values retain their prior behavior. On backend/parser failure the code logs and aggregates the status, explicitly falls back to zero, and still attempts both `xhci_hcd` and `xhci_mtk` to preserve independent USB-host availability.
- Extended `check_process_lifecycle_integration.py` to require the checked read, failure aggregation, legacy fallback, checked-value parameter formatting and XHCI-before-MTK load order. The existing GNU89/ASan/UBSan `wan_nvram_status_policy.c` regression already covers missing values, valid signed decimal input, malformed/overflow values and backend errors. No local checkout/test execution is claimed; exact-head Process, AI docs and XY-C1 Actions are required.
- Exact-head CI status: pending for THIS COMMIT; XY-C1 L3 is not claimed. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-21 — Check DNSMasq DHCP lease and verbosity integers

- Parent main SHA: `4b9f9c2dd2d1a8d736ab8d78a30cd007a39a3c42`.
- `prepare_dns_dhcpd()` used unchecked `nvram_get_int("dhcp_lease")` and `nvram_get_int("dhcp_verbose")`. Backend errors could silently become zero and change generated lease/logging behavior; malformed present values could create unsupported configuration.
- Added checked NVRAM integer/range policy: present lease values must satisfy the UI's 120..604800 seconds and verbosity must be 0..3. Missing keys preserve legacy integer-getter zero defaults. Both are read before any generated hosts, resolver or DNSMasq config file is modified.
- Added ASan/UBSan range, boundary, malformed, overflow, backend, inconsistent-result and missing-key tests, plus an exact workflow source-order checker.
- Exact-head DNSMasq, Process, NVRAM and XY-C1 Build/TRX/artifact results are pending for this source SHA. Hardware DHCP L4 NOT VERIFIED.

---

## 2026-11-21 — Fail closed on Samba local-master NVRAM errors

- Parent main SHA: `9a0ed45f2be0f67807b53c0b8e6d33723436b0ac`.
- `write_smb_conf_header()` used `nvram_get_int("st_samba_lmb")`; transient NVRAM backend/close errors collapsed to zero and could publish `local master = no`. Invalid values outside 0..2 could select domain-master configuration.
- Added checked `nvram_get_checked()` plus the common strict decimal reader, retained missing-key default 0, and reject backend errors or values outside 0..2 before the first smb.conf write.
- Added local-master policy boundary/error tests and exact Samba workflow assertions for checked-read/error ordering and removal of the raw getter.
- Local GNU89/Werror/ASan/UBSan regression and exact-head Samba, Process, XY-C1 Build/TRX/artifact remain to be verified after this source commit. Hardware L4 NOT VERIFIED.

---

## 2026-11-21 — Complete WSD TCP response writes

- Parent main SHA: `a293665472f8f5cef0ce80a928730b6c9ab66980`.
- `wsd_send_msg()` used one `send()` call for TCP responses and treated a positive short write as fatal, dropping the unsent suffix. It also failed immediately on `EINTR`.
- Added `wsd_stream_send_all()` to advance over partial writes, retry `EINTR`, reject zero progress as `EPIPE` and impossible over-count as `EIO`; the UDP/datagram `sendto()` path is unchanged.
- Added GNU89 `-Werror` ASan/UBSan tests for short writes, EINTR, errors after a sent prefix, zero progress, impossible byte counts and invalid arguments, plus source-contract coverage and an exact-path workflow. The XY-C1 workflow already covers `trunk/user/wsdd2/**`.
- Local isolated C regression PASS. No complete repository checkout or MIPS cross-build was available locally; this checkpoint's exact-head source workflow and XY-C1 Build/TRX/artifact gates remain to be checked. Hardware L4 is NOT VERIFIED.

---

## 2026-11-21 — Escape configured metadata in wsdd2 WSD GetResponse XML

- Parent main: `202266c59eafb6c1afff9205ef793f5818ce5ea6`; source commit: `edc9d59456fe40d5c991b3a07507515ddf2c5eaf`.
- WSD GetResponse reflected seven configured values raw into XML text nodes: vendor, vendor URL, model, model URL, presentation URL, NetBIOS name and workgroup. Characters such as ampersand/angle brackets could break the SOAP/XML response and permit markup injection.
- Reuse `wsd_xml_escape_text()` for every value before formatting; fail closed on invalid/control input or allocation failure and release every escaped string on all exits. Added metadata escaping unit cases and source-contract assertions.
- Exact-head XML regression state: #44945383982 wsdd2 WSD XML reflection regression: completed/success; #44945383956 wsdd2 bounded HTTP framing regression: completed/success; #44945383886 Process lifecycle inventory - edc9d59456fe40d5c991b3a07507515ddf2c5eaf: completed/success; #44945383870 XY-C1 Firmware Validation: in_progress; #44945383856 NVRAM transaction boundary audit: completed/success. No new L3/hardware claim.

---

## 2026-11-21 — Fix HTTPD security workflow source-offset assertion

- Parent main: `b5d63a69a644e7edd92ec407a1964d71290e95df`; checker-only commit: `c9005f49bb7edf6ac9efad6bfa51b82f6b902cc4`.
- #44945277626 failed with `ValueError: substring not found` in the inline Python route-flush source checker. It calculated the starting offset with `full_restart_compact.index(...)` then used that offset to search the original `full_restart_routes` string; whitespace compaction changes offsets. Compute the offset using `full_restart_routes.index(...)` instead.
- This change only repairs test/checker logic; it does not modify router runtime source. Exact-head rerun state: #44945615079 AI handoff documentation contract: completed/success; #44945615000 HTTPD Authentication and Security Regression: completed/cancelled. The previous run is a confirmed FAIL; do not report this fix as PASS until rerun.

---

## 2026-11-21 — Harden wsdd2 HTTP POST request framing

- Parent source SHA: `be4952e90111ddea8f91684f6c3fe5ecf25be785`; source commit: `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e`.
- A malformed/fragmented HTTP POST could crash wsdd2: `wsd_parse_http_header()` dereferenced `*eol` after `strstr()` returned NULL when the first TCP read did not include CRLF. The header refill path compacted bytes but kept the old `p` offset; the body path assumed one `recv()` filled the requested bytes; `atoi(Content-Length)` also accepted trailing garbage and had overflow behavior.
- Added bounded CRLF search/read helpers, complete-body reads that retry short reads/EINTR, strict overflow-checked nonzero decimal Content-Length parsing, duplicate Content-Length rejection and the missing `p = buf` after compaction. Requests exceeding the in-buffer body cap are rejected before body access.
- Local GNU99 `-Wall -Wextra -Werror` ASan/UBSan socketpair regression PASS. Exact-head WSD run #44942356801 PASS, covering bounded CRLF search, split request line, split body, malformed/overflow lengths, overlong line and EOF. AI handoff #44942356848 PASS. XY-C1 #44942356809 Build pending at checkpoint; TRX/artifact pending. L3 and hardware L4 NOT VERIFIED.

---

## 2026-11-21 — Prevent p910nd bidirectional SIGPIPE daemon termination

- Parent main SHA: `6c48528d8ec5c746de31f9d25b2016504e9aa831`; source commit: `23248f7df1575885e991fe518629bb974fc05c35`.
- In bidirectional `-b` mode, `copy_stream()` writes printer-return bytes to the remote TCP socket. A disconnected client could trigger default SIGPIPE and terminate the daemon before the existing EPIPE recovery path ran.
- Install SIGPIPE ignore only for bidirectional mode and fail startup if policy setup fails. Existing socket write-error recovery handles the resulting EPIPE. Added GNU89 socketpair/source-contract regression and dedicated Actions coverage; XY-C1 path filter includes p910nd.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` socketpair regression PASS. Exact-head p910nd #44941685747, Process inventory #44941685697, Process regression #44941685794 and AI docs #44941685744 PASS. XY-C1 #44941685839 still building (TRX/artifact pending); NVRAM #44941685864 cancelled. L3 and hardware L4 NOT VERIFIED.

---

## 2026-11-21 — Fail closed on MiniUPnP port-range and cleanup policy reads

- Parent main SHA: `23248f7df1575885e991fe518629bb974fc05c35`.
- `start_upnp_locked()` already used checked NVRAM reads for protocol, secure-mode and HTTP presentation port, but still loaded six related configuration values with `nvram_safe_get_int()`: cleanup minimum/interval and the internal/external mapping port-range bounds. A transient NVRAM backend failure or malformed value could silently replace a configured restrictive port range with the permissive legacy defaults (21–65535) and publish a daemon config with different exposure policy.
- Migrated all six values to the existing checked integer reader in the same preflight gate. Missing/empty keys retain their previous defaults; read errors, malformed values and out-of-range values abort before config publication. Existing min>max normalization remains unchanged.
- Extended the existing MiniUPnP workflow source contract to require all nine checked settings in order and reject any remaining `nvram_safe_get_int("upnp_...")` read in this start path. The generic GNU89 parser regression already covers defaults, backend errors, malformed values and range rejection; no duplicate test harness added.
- Parent 6c485 MiniUPnP workflow #44941570192 and AI docs #44941570201 passed. Parent 23248 XY-C1 #44941685839 had not reached Build XY-C1 at last poll; no L3 or hardware L4 claim is made.

---

## 2026-11-21 — Fix MiniUPnP checked-config workflow variable references

- Parent main SHA: `27c02048027d50435f2d18665bbe74aaddc90fc9`.
- Exact-head MiniUPnP run #44941367108 reached the final inline-Python source contract after the C policy/build regressions had passed, then failed with `NameError: config_file_header is not defined`. The same block had already loaded the config header and test source as `helper` and `test`; corrected the two assertions to use those existing variables. No source implementation or assertion coverage was weakened.
- On the parent, Process inventory #44941366957, DNSMasq #44941367068, Samba #44941367014, WAN route #44941367033, Inadyn #44941366953, WAN DHCP #44941366980 and AI handoff #44941366962 passed. XY-C1 #44941367009, HTTPD #44941366939 and NVRAM #44941367101 were still in progress at last poll. No L3 or hardware L4 claim is made.

---

## 2026-11-21 — Fail closed on MiniUPnP protocol and secure-mode reads

- Parent main SHA: `1a13b60672917c55b0075d8c015522b87d37fea6`.
- `start_upnp_locked()` used error-collapsing `nvram_get_int()` calls for `upnp_proto`, `upnp_secure` and `http_lanport`. A transient backend failure could silently choose a less restrictive zero-value daemon policy.
- Added `miniupnpd_config_file_read_integer_status()`: missing/blank values retain legacy defaults; backend errors, malformed text, overflow and out-of-range values return errno-style failures without changing the output.
- Added `miniupnpd_read_nvram_integer()` and preflighted protocol (0–2), secure mode (0–1) and HTTP LAN port (0–65535) before publishing the replacement config. Any failed read aborts startup while leaving the last-good daemon config untouched.
- Added GNU89 parser regression coverage for defaults, valid/whitespace values, malformed/signed/range/overflow values, backend errors and output preservation. Extended the existing MiniUPnP workflow source contract to verify checked reads, use of validated values and preflight-before-publication ordering. No local test or cross-build is claimed.
- Parent MiniUPnP regression #44940897575 and AI docs #44940897569 passed; XY-C1 #44940897544 skipped Build/TRX/artifact. New exact-head CI is required; L3 is NOT VERIFIED, and L4 remains NOT VERIFIED.

---

## 2026-11-22 — Normalize bounded-description source contract spacing

- Parent main SHA: `4a5139f484943abe3a5ac9cf0f6d56bc70067a56`.
- Exact-head MiniUPnP run #44940761661 passed all GNU89/ASan/UBSan binaries, including the new bounded-description copy regression, then failed in `check_miniupnpd_redirect_desc.py` because the source normalizer collapses newlines to a space after the function's opening parenthesis. The C call is correctly `miniupnpd_redirect_desc_copy_bounded( desc, ...)`; the assertion expected no space. Normalize only the call's post-parenthesis whitespace before checking both stored and fallback call tokens.
- AI handoff #44940761657 and Process inventory #44940761713 passed on the source SHA. XY-C1 #44940761694 and NVRAM audit #44940761781 were still pending. The source sanitizer regression passes, but the lifecycle workflow remains FAIL until the corrected source contract passes; same-SHA Build/TRX/artifact and L3 are not yet claimed.

---

## 2026-11-22 — Bound MiniUPnP redirect-description reads

- Parent main SHA: `1af2d134760bef0ade9054dfb88f67f641759ced`.
- Security defect: `get_redirect_desc()` copied a dynamically stored, SOAP-controlled mapping description with `strncpy(desc, p->str, desclen)` without reserving or forcing a trailing NUL. `GetSpecificPortMappingEntry()` supplies a fixed 64-byte stack buffer and immediately passes it to XML escaping and `%s` logging. A description of 64 bytes or longer therefore caused an out-of-bounds stack read.
- Added a bounded-copy helper to the redirect-description policy and used it for both stored descriptions and the default fallback. It reserves one byte for NUL and safely handles 1-byte, zero-capacity and null-argument boundaries.
- Extended the existing GNU89 `-Werror` ASan/UBSan redirect-metadata regression for exact fit, truncation, 1-byte, empty and invalid arguments. Extended `check_miniupnpd_redirect_desc.py` to bind the helper to `get_redirect_desc()` and the fixed-size SOAP consumer. Existing workflow push/PR filters already cover the source, helper, test, checker and SOAP consumer.
- Local test execution: NOT RUN (no local checkout). Exact-head MiniUPnP config-stability and XY-C1 Build/TRX/artifact must be checked after this source commit. L3 and hardware L4 are not claimed.

---

## 2026-11-21 — Replace undefined Inadyn checker ordering helper

- Parent main SHA: `f27f59a23141112124ef874a1d4a0406dac898f5`.
- Exact-head Inadyn lifecycle run #44940446835 compiled and passed the existing tri-state policy, the new checked-integer parser, and the exclusive lifecycle lock regression binaries under GNU89/ASan/UBSan.
- The final production source-contract step failed with `NameError: ordered is not defined`; the checker has no such helper. Replaced the call with explicit ordered-token positions plus assertions that all six checked reads, the fail-closed return, and the preflight occur before opening the staged config. The checks remain in place.
- AI docs #44940446730, WAN route #44940446824, WAN DHCP #44940446779, HTTPD #44940446885, DNSMasq #44940446712, Samba #44940446725, miniupnpd #44940446973, Process #44940446796 and Process inventory #44940446907 passed on f27f59a. NVRAM audit #44940446992 was pending.
- XY-C1 #44940446898 was in its build step at last poll. Require Build + TRX + artifact on one SHA for L3; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Fail closed on Inadyn TLS and endpoint-option NVRAM reads

- Parent main SHA: `96de0aedf60c0605cbead65e50f69aa31483e82b`.
- `write_inadyn_conf()` used error-collapsing NVRAM reads for `ddns_ssl` / `ddns2_ssl` and wildcard flags; `ddns_ipv6` and `ddns_source` also collapsed failures into zero. A transient SSL-policy read failure could publish `ssl=false` and expose provider credentials over plaintext.
- Added `inadyn_config_file_read_integer_status()`. Missing/empty keys retain legacy defaults; backend errors, malformed text, overflow and out-of-range values return errno-style failures without changing the output.
- Added `inadyn_read_nvram_integer()` and moved six endpoint/security settings into a checked preflight before opening the staged config. Any failure preserves the last-known-good Inadyn configuration. Both providers' SSL and wildcard directives now use validated local values; the duplicated unchecked secondary SSL read is removed.
- Added GNU89/ASan/UBSan regression coverage for missing/default, valid and whitespace values, malformed/range/overflow, backend errors and output preservation. The Inadyn integration checker now enforces checked reads and preflight-before-stage ordering. No local test or target cross-build is claimed.
- Before this slice: AI docs #44940022312 PASS on `96de0aedf60c0605cbead65e50f69aa31483e82b`; VPN lifecycle #44939760147 PASS on `69b61e88ef094496f3c7b4e7f7ac69669aa7a3b1`. d3b7 XY-C1 #44939588534 was in progress at last poll. New Inadyn CI and same-SHA Build/TRX/artifact are pending; L3 is not verified for this change and L4 remains NOT VERIFIED.

---

## 2026-11-22 — Align handoff heading with required recovery contract

- Parent main SHA: `dde4c69fc39bff9a9521de16aab9eee995d75cbe`.
- Exact-head AI handoff run #44939899882 still failed after the missing `## Active task` section was restored, because the validator requires the singular heading `## Exact first action for the next GPT`; the handoff used the plural form. Corrected the heading without changing its actionable content.
- Exact-head VPN lifecycle #44939760147 on `69b61e88ef094496f3c7b4e7f7ac69669aa7a3b1` passed. The d3b7 source SHA's XY-C1 #44939588534 was still building at the previous poll; no L3 result is claimed.
- This is a docs-contract correction only; the next exact-head AI handoff workflow is the validation gate.

---

## 2026-11-22 — Restore mandatory active-task handoff section

- Parent main SHA: `69b61e88ef094496f3c7b4e7f7ac69669aa7a3b1`.
- Exact-head AI handoff docs #44939760108 failed after the mandatory checkpoint was restored, because its validator separately requires the `## Active task` heading. Restored that required section with current OpenVPN profile scope and exact-head validation status.
- Exact-head VPN lifecycle #44939760147 passed after the export contract ordering correction. On the profile source parent `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a`, credential staging #44939588500 and Process inventory #44939588492 passed; XY-C1 #44939588534 remained building, with Build/TRX/artifact not yet all confirmed. No L3 result is claimed.

---

## 2026-11-22 — Correct OpenVPN export contract ordering and restore mandatory handoff gate

- Parent main SHA: `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a`.
- VPN lifecycle #44939588479 passed the client DNS shared-reader contract and earlier policy regressions, then failed because `ordered()` matched nested `options.auth_digest` before the containing `openvpn_add_auth()` call. Split snapshot-field presence from the ordered helper-call sequence; production source is unchanged by this checker correction.
- AI handoff docs #44939588455 found that the rewritten handoff omitted the mandatory end-of-session accuracy checkpoint. Restored the required checkpoint section.
- OpenVPN credential staging #44939588500 passed on the parent. XY-C1 #44939588534 was still before Build XY-C1 at last poll; no TRX/artifact/L3 result was available. Exact-head VPN and AI reruns plus same-SHA XY-C1 Build/TRX/artifact verification remain required; L4 is NOT VERIFIED.

---

## 2026-11-22 — Harden OpenVPN exported server identity and dual-stack reachability

- Parent main SHA: `53e9c553994afe3917c3451dee9d73f116be1024`.
- Exported client profiles previously emitted a commented `;remote-cert-tls server` directive even though `openvpn-cert.sh` signs the server certificate with `extendedKeyUsage=serverAuth` and client certificates with `clientAuth`. The exporter now activates `remote-cert-tls server`, requiring the peer's server certificate purpose.
- For dual-stack server protocol modes 4/5, export previously stopped after DDNS and IPv6 WAN/LAN lookup. It now falls back to the WAN IPv4 address only when both IPv6 sources fail and only if the IPv4 address validates. IPv6-only protocol modes 2/3 remain IPv6-only.
- Extended the VPN lifecycle source contract to tie the profile directive to the certificate generator's EKU/signing definitions, verify shared auth/cipher/compression mapping, and enforce the dual-stack fallback ordering. Added `openvpn-cert.sh` to VPN lifecycle push/PR triggers; XY-C1 already rebuilds for both OpenVPN source and certificate-generator paths.
- Exact-head Actions on parent 53e9 were still pending at last poll (VPN lifecycle #44939430753, XY-C1 #44939430749, AI docs #44939430857, NVRAM #44939430717, Process #44939430869 and OpenVPN credential staging #44939430805). No local tests/build or L3/L4 result is claimed; current source commit requires fresh same-SHA checks.

---

## 2026-11-22 — Repair shared VPN DNS policy integration and archive byte index

- Parent main SHA: `418373c53d35051fc316c67f119d372126a08f89`.
- The shared VPN DNS policy migration changed `on_client_ifup()` to `vpn_client_nvram_read_bool_status()`, but a trailing fragment from the previous six-argument parser call remained after the new five-argument call. Removed the invalid fragment so the C function has one complete, correctly typed helper invocation.
- The exact-head VPN lifecycle checker also still expected `vpn_server_config_int_status()` in the OpenVPN callback. Updated it to require the shared helper and explicitly reject the stale argument tail. The shared GNU89 policy unit test had already passed on parent 418.
- Exact-head AI handoff docs #44939242988 found the part-023 index byte count was 109434 while the archived file was 109696 bytes. Corrected the index metadata without modifying the immutable archive blob or its SHA.
- Parent #44939242958 failed at the stale OpenVPN source contract before the new source correction; #44939242892 XY-C1 was still in progress before Build XY-C1. No target build or L3 result is claimed for parent 418. Verify current exact-SHA Actions after this repair; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Share checked boolean reads across both VPN DNS callbacks

- Parent main SHA: `7ad1cb24f67b9c9dffafeb1a492b2a41343a8413`.
- `ipup_vpnc_main()` still used `nvram_get_int("vpnc_pdns")`. As with the OpenVPN callback, a transient NVRAM backend error was indistinguishable from DNS push being disabled and could overwrite `vpnc_dns_t` with an empty snapshot followed by a resolver refresh.
- Added `vpn_client_nvram_read_bool_status()` in the shared client NVRAM policy layer. It preserves missing/empty legacy defaults, accepts bounded decimal boolean values, rejects malformed/out-of-range settings, propagates backend errno and leaves caller output untouched on error.
- Migrated both PPP VPN and OpenVPN up callbacks to the shared reader. On policy read failure they preserve DNS/domain snapshots and do not refresh resolvconf, but still attempt link-state publication, retain route/balancing work, run configured up hooks and return the error. Expanded GNU89 policy tests and both source-contract checks.
- Rotated the root development log at 120 records: the 100 oldest complete records are preserved oldest-first in `docs/dev-log/2026/11/part-023.md`; the newest 20 (including this entry) remain at root. Archive source root blob before rotation: `7fc5db49fc36bc93efc1ee25e707d828304115de`.
- On `23520100e3cc74bb7f77a0d023afa361cded2abe`, XY-C1 Build, TRX verification and artifact upload all passed on one SHA. Later production callback changes on 040b/88b still had XY-C1 builds in progress at last poll; do not treat those results as validating the later commits. L4 hardware flash/boot/runtime remains NOT VERIFIED.


---

## 2026-11-22 — Correct server callback checker function locators

- Parent main SHA: `bdb0ecf22351f910d1185bf2434a27417e5ecf99`.
- The client callback source-contract locator fix was committed in bdb0ecf, but inspection found the new server callback assertions in `scripts/check_vpn_server_lifecycle.py` also used double-escaped newlines in `body()` function signatures.
- Corrected those locators to actual Python newline escapes before the assertions execute. The production callback change and assertions are unchanged.
- Exact-head VPN lifecycle #44938509538 on bdb0ecf and AI docs #44938509917 were in progress at the last poll. Earlier 040b/88b VPN failures were confined to the client checker locator; compiled policy steps passed. Exact-head rerun is required for both source-contract layers.
- `dev_log.md` now reaches 119 root entries; archive the oldest 100 before appending another record. XY-C1 L3 and hardware L4 remain NOT VERIFIED.

---

## 2026-11-22 — Correct escaped function locators in OpenVPN callback contract

- Parent main SHA: `88b8124bfa1363b8f6f066e66b4e2c6db80e26f6`.
- Exact-head VPN lifecycle #44938233419 on `040b3da86f81007605c80a5687b7a516f41d19ce` passed all compiled policy/sanitizer steps but failed in `scripts/check_vpn_client_nvram_state.py`: the new source contract had double-escaped newline/tab sequences and could not locate `on_client_ifup()`.
- Corrected the Python string escapes in the new contract block. Assertions remain intact; production code is unchanged. Exact-head VPN lifecycle rerun is required.
- The 040b XY-C1 run #44938233497 and 2352 XY-C1 run #44937972691 were still in progress at the last observation. The latest `88b8124` Actions listing showed only Process and HTTPD queued; do not infer a firmware result from absent runs.
- L3 requires same-SHA Build, TRX and artifact upload; L4 remains NOT VERIFIED.

---

## 2026-11-22 — Check OpenVPN callback device mode without losing disconnect cleanup

- Parent main SHA: `040b3da86f81007605c80a5687b7a516f41d19ce`.
- `ovpn_server_script_main()` unconditionally used `nvram_get_int("vpns_ov_mode")`, even though OpenVPN supplies the actual `dev` environment value. On a transient backend error, the compatibility getter silently selected TAP for callbacks that needed the NVRAM fallback.
- The callback now trusts OpenVPN's `dev` when present and only reads the mode through checked NVRAM + strict 0/1 parsing when `dev` is absent. An unreadable fallback stays unknown instead of synthesizing TAP; connect/disconnect dispatch and configured hooks still run. Lease additions require a known interface, while disconnect cleanup remains address-based and accepts an absent interface so stale lease entries can still be removed.
- Updated the VPN lifecycle source contract to protect checked mode fallback, no fabricated interface, disconnect lease removal and hook dispatch. Exact-head CI for this slice and the preceding pushed-DNS policy fix must be inspected; no local build is claimed.
- XY-C1 L3 and hardware L4 remain NOT VERIFIED.

---

## 2026-11-22 — Fail closed on OpenVPN pushed-DNS policy read errors

- Parent main SHA: `23520100e3cc74bb7f77a0d023afa361cded2abe`.
- `on_client_ifup()` used `nvram_get_int("vpnc_pdns")`; a backend failure collapsed into disabled and could overwrite resolver/domain snapshots with empty values and trigger an unintended resolver refresh.
- Replaced the compatibility getter with `nvram_get_checked()` plus bounded 0/1 parsing. On a read/parse error, optional DNS/domain publication and resolver refresh are skipped, preserving existing resolver state. The callback still attempts `vpnc_state_t=1`, executes the configured up hook, and clears temporary environment variables; the error remains in its return status. The down path still independently restores DNS, clears domain state and executes the down hook.
- Updated `scripts/check_vpn_client_nvram_state.py` to enforce checked-policy ordering and mandatory state/hook behavior. Existing `vpn_server_config_int_status()` regression tests cover backend errors and malformed/range values; the VPN workflow compiles them and runs the source contract.
- Exact-head CI for this commit must be checked on live main; no local regression build is claimed. XY-C1 L3 and hardware L4 remain NOT VERIFIED.

---

## 2026-11-22 — Fix xl2tpd caller for the checked VPN address-pool API

- Parent main SHA: `5365b10a5136e479e9db91079eb525634c3fcfa8`.
- Exact-head XY-C1 run #44936762599 failed in `trunk/user/rc/src/net_ppp.c`: `safe_start_xl2tpd_locked()` still passed `nvram_get_int("vpns_vuse")` plus four outputs to `get_vpns_pool()`, while the transactional API now takes four output pointers and reads the mode itself.
- Updated the xl2tpd caller to use the checked four-output API and one immutable pool snapshot for local/range address generation. Pool read/validation/transaction-end failures now preserve errno and abort before `/etc/xl2tpd.conf` is opened or truncated.
- Extended `scripts/check_vpn_server_lifecycle.py` to assert the four-argument API, snapshot-before-open ordering, fail-closed error return, and use of the checked snapshot outputs. Existing workflow already covers `net_ppp.c` and the checker.
- This commit's exact-head VPN lifecycle, NVRAM and XY-C1 Actions must be read from live main; no local cross-build or test run is claimed. L3 remains NOT VERIFIED until same-SHA Build, TRX and artifact upload pass; L4 remains NOT VERIFIED.

---

## 2026-11-21 — Snapshot OpenVPN server settings for exported client profiles

- Parent main SHA: 2e99cd420a33c4e23bbfe866fba502c3cd59ca80.
- ovpn_server_expcli_main() remained a bypass around the checked server configuration path: it fetched vpns_ov_mode, protocol, port, TLS auth/crypt flags, digest, cipher/NCP list and compression through compatibility getters, so an exported client profile could disagree with the running server and silently select defaults after read failures.
- The exporter now obtains both the validated server network snapshot and complete server options snapshot before checking server keys, unlinking any previous export, creating the temporary certificate directory or generating a client certificate. Profile device, protocol, port, digest/cipher/compression and TLS flags all come from those immutable snapshots.
- All direct compatibility NVRAM getters/matchers were removed from the export function. The lifecycle source contract now asserts snapshot-before-side-effects and prohibits getter bypasses. Existing server option and network policy tests cover the values and defaults.
- On exact 2e99, VPN #44936433999, OpenVPN client staging #44936433953, Process #44936433958, AI #44936433911 and HTTPD #44936433967 PASS; NVRAM #44936433895 and XY-C1 #44936434017 were pending at last poll.

---

## 2026-11-21 — Snapshot OpenVPN client settings before config and interface side effects

- Parent main SHA: 48c23d6cad5313bd1d62416097d12b2169c64b8b.
- The client config writer still read authentication mode, TLS mode, transport protocol, peer, port, digest, cipher/NCP list, compression and redirect-gateway independently through compatibility getters. start_openvpn_client() also selected TUN/TAP mode and TAP bridge/NAT behavior via unchecked getters after creating the client directory. Backend failures could silently produce mixed settings or route to an unintended peer/interface.
- Added one checked NVRAM transaction snapshot for all client options, including a bounded copied peer host validated by openvpn_safe_remote_host(). UI values are range-checked (auth 0..1, TLS 0..3, protocol 0..5, digest 0..5, cipher 0..15, compression 0..4, port 1..65535 with legacy default 1194, mode and flags 0..1). NCP cipher string is copied into a bounded buffer.
- Client startup now validates the entire snapshot before creating directories, staging credentials/config or creating TUN/TAP interfaces; config writer and interface selection consume the same snapshot. IPv4/IPv6 peer-family adjustment remains local to emitted config rather than persisting a silent NVRAM mutation. Existing private secret staging and atomic config publication are unchanged.
- Added UI-range/default regression tests and source contracts requiring complete snapshot before side effects and no compatibility getter/writes in the config writer.
- Exact 48c23d6 VPN lifecycle #44936185717, AI docs #44936185688, OpenVPN credential staging #44936185574, Process #44936185621 and HTTPD #44936185598 PASS. NVRAM audit #44936185600 and XY-C1 #44936185654 were pending at last poll.

---

## 2026-11-21 — Snapshot OpenVPN server options before staged config publication

- Parent main SHA: 6d1019d4b8da56ab545be67038934e57a65b631a.
- The server config writer still fetched protocol, port, TLS flags, auth/cipher/compression, NCP cipher list, redirect-gateway, DHCP DNS/WINS and Samba WINS enablement through compatibility NVRAM getters. Backend failures could silently select defaults or leave pointer-backed values stale while writing server.conf.
- Added a checked DHCP enable status read and one NVRAM transaction for OpenVPN server option scalars and bounded copied DNS/WINS/NCP strings. Scalar ranges match the UI enumerations; port defaults to 1194 and is constrained to 1..65535. Missing optional strings remain empty. Read/parse/transaction-end errors abort before opening the staged server configuration.
- Config generation now consumes the immutable options snapshot and contains no compatibility NVRAM reads or writes. The IPv6-unavailable protocol fallback remains local to emitted config instead of mutating persistent vpns_ov_prot.
- Added numeric boundary/default policy tests and source contracts requiring DHCP status + transaction completion before staged-file publication. Existing topology and CCD snapshots remain intact.
- Exact 6d1019d4b8da56ab545be67038934e57a65b631a VPN #44935854519 PASS, AI #44935854512 PASS, HTTPD #44935854556 PASS, OpenVPN client staging #44935854570 PASS, Process #44935854615 PASS; NVRAM #44935854517 and XY-C1 #44935854504 were pending at last poll.

---

## 2026-11-21 — Snapshot OpenVPN server network topology before side effects

- Parent main SHA: cb06dfd4d1135414c231726f19ab635a91f3af85.
- The previous mode-only transaction left a race: LAN address/netmask, TUN virtual network or TAP client range were fetched later through compatibility getters while server.conf was being written. Failures silently became 0.0.0.0/empty strings/default pool values, and config/interface behavior could combine different NVRAM generations.
- Replaced the mode-only reader with one transaction covering mode, LAN IPv4/netmask and the mode-specific TUN virtual network or TAP client offsets. Required IPv4 text is strictly parsed; LAN netmask and host address are validated; TUN networks must be private and non-overlapping with LAN; TAP offsets use the previous defaults and numeric bounds. Transaction-end errors abort the snapshot.
- start_openvpn_server() now resolves the complete topology before creating directories or config; config generation and TUN/TAP interface setup consume the same snapshot. Removed compatibility reads of the mode, LAN, virtual network and TAP offsets from the server config writer. Existing CCD ACL snapshot stays independent and unchanged.
- Added source contracts requiring the complete read/validation transaction before filesystem/config/interface side effects and prohibiting topology rereads. Existing policy tests cover strict LAN validation, private TUN networks and overlap; mode boundaries remain covered.
- Prior dfd2 VPN lifecycle #44935459202 failed only because its regression wrongly expected leading-zero decimal `01` to be rejected. cb06dfd corrects that test; exact rerun and same-head product verification pending. dfd2 NVRAM/HTTPD/OpenVPN-client/Process/AI passed; XY-C1 was pending.

---

## 2026-11-21 — Correct OpenVPN server mode decimal regression

- Parent main SHA: dfd2f275ecc1b56c7cfd251790ea609db38d8ad7.
- Exact VPN lifecycle #44935459202 failed before later lifecycle tests ran: `test_openvpn_server_mode_bounds()` assumed the shared strict integer parser rejected `01`, but that parser intentionally accepts a fully consumed decimal string with leading zeroes. No production bug was demonstrated; the assertion contradicted established parser behavior.
- Updated the test to assert that `01` is accepted as integer mode 1. Unsupported values such as 2, out-of-bounds parse results and backend errors remain rejected. Production code is unchanged.
- Same-SHA NVRAM #44935459206, HTTPD #44935459183, OpenVPN client staging #44935459254, Process #44935459317 and AI docs #44935459291 PASS. XY-C1 #44935459269 was still in progress at last poll. Exact VPN rerun required.

---

## 2026-11-21 — Fail closed on OpenVPN server TUN/TAP mode reads

- Parent main SHA: 29c9c56318ae8770da8bb5fc23b77456b0bccf79.
- start_openvpn_server() previously collapsed vpns_ov_mode read/parse failures to integer zero, silently selecting TAP mode even when NVRAM backend or configuration state was unknown. The selected mode controls both emitted server.conf device/network directives and whether a TAP bridge or TUN interface is created.
- Added a checked NVRAM transaction read with strict integer bounds 0..1. Missing key retains legacy TAP default; backend errors, malformed values and unsupported mode values abort before creating directories, publishing configuration, or mutating interfaces. The same checked value is passed to config generation and interface setup.
- Added boundary/backend-error regressions and lifecycle source contracts requiring the transaction read before config and interface side effects.
- Parent caafec82 OpenVPN CCD source compiled and its VPN lifecycle failure was only the checker NameError, corrected by 29c9c563. The 29c9 lifecycle check was PASS at last poll; full run was finishing. Parent d93dc94 VPN, HTTPD, Firewall, OpenVPN client staging, AI docs, Process and NVRAM PASS; XY-C1 #44934938330 still pending.

---

## 2026-11-21 — Snapshot OpenVPN server CCD ACL policy before file publication

- Parent main SHA: d93dc94dd989bf81049cd041b0b7bf4d3876a326.
- openvpn_create_server_acl() previously read the ACL row count, usernames, remote networks/masks and per-client address offsets with compatibility NVRAM getters while creating and publishing CCD files row-by-row. Backend failures collapsed to empty strings or zero, cached pointers were exposed to later getters, and an invalid/partial network pair could silently omit a client route.
- Added one checked NVRAM transaction snapshot for the complete bounded ACL table. Each username and address is copied before another read; names must remain safe single path components; remote network/mask must be both empty or both present, parse as IPv4, and use a nonzero contiguous netmask; optional address offsets are bounded 0..255. Backend, parse, overlong-field and transaction-end errors abort before CCD directory creation or file publication.
- CCD file publication now consumes only the completed snapshot, and no longer performs direct NVRAM reads. Existing route/iroute and static-address output rules remain unchanged for valid rows. Existing policy tests cover strict IPv4, bounded integer and contiguous netmask behavior; the lifecycle checker now asserts transaction-before-filesystem ordering and absence of compatibility getters.
- Exact d93dc94 VPN lifecycle #44934938301, Firewall #44934938310, OpenVPN client credential #44934938321, AI docs #44934938340, Process #44934938302/#44934938299 and HTTPD #44934938324 PASS. NVRAM #44934938318 and XY-C1 #44934938330 were pending at last poll.

---

## 2026-11-21 — Fail closed before VPN teardown when xl2tpd ownership is unreadable

- Parent main SHA: b1c2a8743ac7232ba4ad5fe3cc53a5e6eca03110.
- stop_vpn_server_locked() selected xl2tpd for the stop target set through get_xl2tpd_vpns_active(), which chained nvram_match() compatibility reads. Backend read failures collapsed to false. A failed dependency read could therefore omit a shared xl2tpd daemon from the identity-preflighted stop set while the rest of VPN teardown continued and cleared l2tp_srv_t, leaving ownership ambiguous.
- Replaced the helper with one checked NVRAM transaction snapshot of vpns_enable, vpns_type and l2tp_srv_t. Missing keys preserve legacy defaults; backend errors, malformed enable/type values, or transaction-end failure return before any daemon stop. The exact historical membership rule is now an explicit pure policy helper and has combination regression tests.
- Extended the lifecycle source contract to require transaction reads and ensure stop_vpn_server_locked() resolves xl2tpd ownership before adding targets, recording stop attempts or invoking the multi-process stop routine.
- Exact b1c2 VPN lifecycle #44934740188 and AI docs #44934740177 PASS. Exact 6569 NVRAM #44934446421 and Process #44934446571/#44934446418 PASS; HTTPD #44934446395, Firewall #44934446425, OpenVPN #44934446352 and AI #44934446376 PASS. 6569 XY-C1 #44934446423 remained pending at last poll; L3 remains verified only on d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192.

---

## 2026-11-21 — Correct the remaining shared-xl2tpd success-sentinel contract

- Parent main SHA: 75351d845a23d351c3d6e44a682257693be90d8a.
- Exact VPN lifecycle #44934584827 passed all compiled regression steps but then failed at Python line 498: a second pre-existing source assertion still required the removed vpn_server_restart_should_restore_shared_xl2tpd() call. That predicate was superseded by the fail-safe checked dependency snapshot and vpn_server_restart_should_attempt_restore_shared_xl2tpd() decision.
- Updated the assertion to require the checked dependency reader, conservative restore decision, safe_start_xl2tpd_locked() attempt, dependency-error propagation and the existing nonnegative-start success convention. This verifies that a positive already-running sentinel remains success while read errors remain visible.
- Exact AI handoff #44934584859 passed. Exact 6569 VPN lifecycle failed only in source contracts after all compiled policy/sanitizer tests passed; 6569 OpenVPN #44934446352, Firewall #44934446425, HTTPD #44934446395, Process #44934446571/#44934446418 passed; NVRAM #44934446421 and XY-C1 #44934446423 were still pending.
- L3 remains verified only on d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192; L4 remains not verified.

---

## 2026-11-21 — Align shared-xl2tpd lifecycle contract with fail-safe restoration

- Parent main SHA: 6569fef7b8420d1d8b6adf51a3a3b6d069f9af86.
- Exact VPN lifecycle #44934446434 passed all compiled VPN config, lifecycle-lock, xl2tpd, L2TP config, lease, restart, client NVRAM and PPP scalar regression steps, then failed in the existing Python source-contract assertion: it expected the former `vpn_server_restart_should_restore_shared_xl2tpd()` call and returned-success path. The current implementation uses the checked dependency snapshot and `vpn_server_restart_should_attempt_restore_shared_xl2tpd()`, and returns failure after a conservative restore attempt if the dependency read failed.
- Updated the contract to assert dependency snapshot -> conservative restore decision -> start attempt -> read-error propagation, and explicitly require the corresponding policy helper and unit test. This is checker-only; production behavior and test strength are unchanged.
- On exact 6569, OpenVPN #44934446352, Firewall #44934446425 and AI docs #44934446376 passed. HTTPD #44934446395, Process #44934446418/#44934446571, NVRAM #44934446421 and XY-C1 #44934446423 were pending at last poll. Exact VPN lifecycle rerun required.
- L3 remains verified only on d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192; L4 hardware testing remains not verified.

---

## 2026-11-21 — Snapshot VPN callback modes before connection side effects

- Parent main SHA: 4037e24d6f7201988ec73c74a5cc6cc06b38684c.
- ipup_vpns_main() previously read vpns_vuse before multicast setup and vpns_cast after route installation through compatibility getters. A failed read silently became zero, potentially applying the wrong multicast and bcrelay policy after firewall/lease mutation. ipdown_vpns_main() also read vpns_cast unchecked when deciding optional relay teardown.
- Added one checked NVRAM transaction snapshot for the two bounded modes (vpns_vuse 0..2, vpns_cast 0..3). Connection setup now aborts before firewall/lease/route changes if the snapshot or transaction end fails. Disconnect still performs mandatory firewall, lease, route and custom-hook cleanup after a mode-read failure, but skips the optional cast=0 relay branch unless the independent last-client condition requires it.
- Added mode-bound regression coverage and lifecycle assertions for snapshot-before-side-effects and absence of compatibility mode getters. The new source commit triggers exact-head VPN, NVRAM audit and XY-C1 checks.
- Exact 4037 shared-xl2tpd restoration checks #44934344633 VPN, #44934344659 NVRAM, #44934344688 AI docs and #44934344755 XY-C1 were still in progress at last poll.

---

## 2026-11-21 — Restore possibly shared xl2tpd when dependency state is unreadable

- Parent main SHA: `d784825ba2ded79d11f50bdbef08ebc1a71fd83f`.
- `restore_shared_xl2tpd_locked()` used two `nvram_match()` calls to decide whether WAN-L2TP or VPN-client-L2TP still depended on the shared daemon. Those compatibility checks collapse backend failure into false; if the VPN stop path had already stopped xl2tpd, a transient NVRAM error could therefore skip restoration and leave an independent L2TP service offline.
- Added a checked transaction snapshot for both dependency flags. When the snapshot succeeds, behavior is unchanged. When dependency state is unreadable after a stop attempt, restoration is attempted conservatively and the original read error is still logged/returned so the caller does not mistake the restart for fully verified success.
- Added policy tests for restore-on-read-error and source contracts enforcing transaction read, restore attempt and failure propagation.
- Exact d784 VPN lifecycle #44934107678 and AI handoff #44934107735 passed. Exact f08 HTTPD, Process, Firewall, OpenVPN and Process-lifecycle workflows passed; NVRAM #44933990196 and XY-C1 #44933990161 were pending at last poll. Exact checks for this new source slice are pending.

---

## 2026-11-21 — Update VPN route source contract after transactional ACL snapshot

- Parent main SHA: `f08c69de6f8a44c6bc623b62da09fe06b6723ba5`.
- Exact VPN lifecycle #44933990171 failed at the existing pre-remote-LAN contract at Python line 117. It still demanded the prior compatibility getter guard and literal `route_status = add ?` / unformatted route-add statements, while the production callback now obtains a checked transaction snapshot, then performs a formatted route_add/route_del call. The new appended contract covers transaction ordering and no compatibility getters; the legacy contract was not updated in the same source slice.
- Updated the old contract to require the new fail-closed input guard, completed snapshot before route changes, idempotent status mapping and status propagation. All preceding C policy and sanitizer steps passed in the failed run; rerun exact-head VPN stability after this checker correction.
- Exact f08 XY-C1 #44933990161, NVRAM #44933990196, AI docs #44933990188, OpenVPN #44933990213, Process checks #44933990242/#44933990166, HTTPD #44933990290 and Firewall #44933990132 were in progress at last poll.

---

## 2026-11-21 — Snapshot VPN remote-LAN route ACL before route mutation

- Parent main SHA: `47f339f6c51b64f75963c0e743db51285e8be897`.
- `vpns_route_to_remote_lan()` previously read ACL count, usernames, remote networks/masks and LAN address/netmask with compatibility getters. Backend errors collapsed into empty strings or integer zero, so an ACL could be partially observed and the callback could silently skip or mis-evaluate a route on connection/disconnection.
- Added one exclusive NVRAM transaction snapshot of all ACL rows and LAN addressing. Each shared-cache value is copied before the next read; count is bounded, LAN address/netmask are validated, remote rows are normalized to their masked network, and remote masks must be contiguous/non-zero. Missing/partial/invalid rows retain the legacy skip behavior; transaction/backend errors abort before any route add/delete call.
- Added unit coverage for zero, contiguous and non-contiguous masks, plus lifecycle source contracts enforcing transaction-complete-before-route-mutation and no compatibility getters in the callback.
- Exact 67274 VPN policy/C regression steps passed; its lifecycle source contract failed only because the checker selected the PPP snapshot forward declaration. The correction commit 47f339f6c51b64f75963c0e743db51285e8be897 passed exact VPN lifecycle #44933768307 and AI docs #44933768276. XY-C1 for 67274 was still building at last check; the current route ACL slice needs its own exact-head checks.

---

