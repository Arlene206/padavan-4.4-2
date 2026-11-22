## 2026-11-22 — Align USB modem Actions link-change contract with checked recovery logic

- Parent main SHA: `33e87d4540698ef87226715fd3fa8e967243cb01`. Exact source run USB modem identity #45032550398 failed in its embedded Python source contract at the old assertion requiring the former standalone `get_usb_modem_wan_status()` flow. The new checked startup config/WISP snapshot implementation and its policy had already compiled and passed the preceding GNU89/ASan/UBSan test step.
- Inspected the job log: failure was `ValueError: substring not found` in the workflow's duplicated inline checker, not a C compile or runtime regression. Updated its ordering contract to require checked `get_modem_wan_start_config_status()`, checked WISP status, checked modem identity, pure link-change policy and notification in sequence; it also forbids unchecked NVRAM/WISP fallbacks and pins the new regression test.
- On the same exact source SHA, WAN teardown #45032550356, WAN DHCP #45032550362 and AI handoff #45032550424 PASS. Firewall #45032550381, XY-C1 #45032550417, NVRAM #45032550412, Process inventory #45032550504, Process lifecycle #45032550421 and HTTPD #45032550438 were pending at last poll.
- The older PPP source SHA `b3caebcbaba0924abcb83339d49a1968e237f042` passed WAN DHCP, VPN, WAN route, HTTPD, Process and NVRAM; its XY-C1 run #45032049960 remained in_progress after the production Build step began. No TRX/artifact success is claimed.
- No local tests were run in this connector-only session. Result: THIS COMMIT is a workflow-contract correction plus accurate handoff. Next: inspect exact-head reruns and continue independent source work while pending jobs run.

---

## 2026-11-22 — Preserve modem WAN link-change recovery across NVRAM read errors

- Parent main SHA: `b0338e1be6b52f0f43cc644253565675fc243138`. The active WAN PPP source SHA `b3caebcbaba0924abcb83339d49a1968e237f042` had WAN DHCP #45032049974, VPN #45032050106, WAN route #45032049989, HTTPD #45032050007, process lifecycle #45032050136 and NVRAM audit #45032049962 PASS. XY-C1 #45032049960 was still building; AI docs failure #45032049968 was fixed by `b0338e1be6b52f0f43cc644253565675fc243138`, whose exact AI handoff run #45032198181 PASS.
- `notify_modem_on_wan_ether_link_changed()` used unchecked `nvram_get_int("modem_prio")`; a transient NVRAM backend failure could collapse to zero and silently suppress the modem WAN recovery notification. It also used the legacy boolean WISP wrapper, which hides the difference between WISP=true and unknown identity.
- Replaced those reads with one `get_modem_wan_start_config_status()` snapshot (priority + device readiness) and `get_wan_wisp_active_status()`; every read failure now logs and aborts without scheduling a reconnect on unknown state. WAN identity remains status-checked.
- Added a pure link-change reconnect policy and GNU89/ASan/UBSan regression coverage for equal-state transitions, disabled priority, absent device, mismatched identity, legacy truthy link values and invalid state. Extended the modem source checker to require the checked-read/error-gate order and forbid unchecked fallbacks. The existing USB modem workflow already compiles and runs this policy test and includes all changed source/test/checker paths.
- No local repository build or test was run in this connector-only session. Exact-head CI for this new source SHA remains to be checked after commit; no L3/L4 result is claimed.
- Result: THIS COMMIT. Next: inspect exact-SHA USB modem identity, firewall, process-lifecycle and XY-C1 Actions; diagnose any failure, and continue independent work while checks are pending.

---

## 2026-11-22 — Restore exact atomic-main-commit handoff contract

- Parent main SHA: `b3caebcbaba0924abcb83339d49a1968e237f042`. Exact-head VPN lifecycle #45032050106, WAN route #45032049989, WAN DHCP #45032049974, HTTPD #45032050007 and process lifecycle #45032050136 PASS. NVRAM #45032049962 and XY-C1 #45032049960 are still in progress. AI handoff #45032049968 failed because `SESSION_HANDOFF.md` said "one atomic commit" but the validator requires the literal phrase "one atomic main commit".
- Restored the exact required phrase in the handoff commit rule. The workflow remains one complete Git tree, one atomic commit, and a non-forced expected-SHA update to main.
- XY-C1 #45032049960 has started the production Build step for the PPP source-triggered commit; TRX and artifact upload remain pending. No L3 claim until all three succeed on this exact SHA.
- No local tests were run in this connector-only session. Result: THIS COMMIT. Next: inspect exact-head AI/NVRAM/XY-C1 checks and continue an independent userland stability task while pending jobs finish.

---

## 2026-11-22 — Return deterministic errno for invalid PPP WAN units

- Parent main SHA: `b443b54cf920195df1405e4a172ec0ed7f9b1f09`. Exact-head USB modem #45031581333, firewall startup #45031581354, process inventory/lifecycle #45031581442 and AI handoff #45031581408 all PASS. XY-C1 #45031581390 also reports workflow SUCCESS and every host regression/source contract passed, but its production Build, TRX check and artifact upload were intentionally SKIPPED because this parent commit changed only checker/documentation paths. The PPP production source slice in ancestor `52a8c60d1e9aa324bb7f020fdeafda99a03319e9` still needs an actual target cross-build.
- Final review found `launch_wan_pppd()` returned `-1` for a negative WAN unit without setting `errno`. Callers therefore received a stale unrelated error, undermining failure diagnosis and first-error aggregation. The locked worker already rejects negative units with EINVAL; the public wrapper now uses a tiny pure validation policy and sets `errno = EINVAL` before returning, before opening the NVRAM transaction.
- Added GNU89 policy regressions for zero/positive and negative unit indices, and extended the PPP source contract to enforce validation/errno assignment before the checked launch-mode snapshot.
- This modifies `net_ppp.c`, so the exact-head XY-C1 workflow should now detect a production input change and execute Build, TRX verification and artifact upload instead of reporting host-only validation. No target build is claimed until that run completes.
- No local tests were run in this connector-only session. Result: THIS COMMIT. Next: inspect exact-head WAN PPP/VPN/WAN-route/XY-C1/AI checks, prioritize any FAIL, and continue independent userland stability work while jobs are pending.

---

## 2026-11-22 — Make LAN IPv6 reload source contract whitespace-robust

- Parent main SHA: `a624157703da1feabe5838db691b4b28e4d68328`. Exact-head WAN DHCP callback run #45031431084 PASS confirms the LAN DHCP hostname status-snapshot contract correction. AI handoff #45031431144 PASS. NVRAM transaction audit #45031084347 on earlier exact source `52a8c60d1e9aa324bb7f020fdeafda99a03319e9` PASS.
- The first update to `check_modem_wan_start_status.py` still failed on exact-head USB modem #45031431089 and firewall #45031431129: its new contract matched an entire multiline call after the helper `compact()` had collapsed whitespace but retained a single space after the opening parenthesis. This was a checker formatting error, not a source/API mismatch.
- Reworked the assertion to locate the status-read and snapshot-apply calls by function token, then separately assert the key and clear/add callbacks are passed. It still verifies read-before-apply, error-return ordering, and the existing no-mutation-on-read-failure policy test.
- Process inventory #45031431085 and XY-C1 #45031431087 were still running at last observation; the prior XY-C1 exact run #45031330348 failed at the same stale LAN reload check before firmware build. No build/TRX/artifact result is claimed.
- No local tests were run in this connector-only session. This checker correction needs exact-head process/USB/firewall/XY reruns.
- Result: THIS COMMIT. Next: inspect exact-head Actions after the corrected modem checker; continue any remaining concrete CI failure.

---

## 2026-11-22 — Align LAN IPv6 read-failure contracts with snapshot APIs

- Parent main SHA: `592ce6b42c4c1f3ff473cdfb8839959c2fb4b5da`. The latest exact-head IPv6 service-type run #45031159848 and AI handoff run #45031159852 on ancestor `1e6238117d62087ea9eb5e99384b3ad2c65d1511` both PASS.
- Exact PPP contract descendant `592ce6b42c4c1f3ff473cdfb8839959c2fb4b5da` fixes the stale PPPoE-MWAN worker signature and removes a whitespace-sensitive VPN source assertion. Its exact-head Actions are expected to rerun from this source tree.
- The shared modem/USB/firewall/process/XY checker failed because it expected the retired `rc_lan_addr6_reload_apply()` call inside `reload_lan_addr6()`. Production code now first reads `lan_addr6` with `lan6_read_config_value_status()` then calls `rc_lan_addr6_reload_from_snapshot()`; that helper returns a backend read error before invoking clear/add callbacks. The checker now pins this order and the existing no-mutation-on-read-error regression test.
- WAN DHCP checker #45031084413 also expected an unchecked compatibility getter `get_lan_addr6_host()`, but `create_hosts_lan()` correctly uses `get_lan_addr6_host_status()`, aborts on read error and only stages hostname/hosts files after the checked address snapshot. The source contract now asserts the read/error/return path precedes stage-file opening and the emitted rows use the snapshot.
- No product source changes were needed; these were stale source contracts against previously hardened production code. No local tests were run in this connector-only session.
- Result: THIS COMMIT. Next: inspect exact-head WAN DHCP/modem/USB/firewall/process/XY workflows after the checker corrections; then continue any remaining concrete userland stability/security issue.

---

## 2026-11-22 — Normalize PPP lifecycle and multi-WAN source contracts

- Parent main SHA: `1e6238117d62087ea9eb5e99384b3ad2c65d1511`. IPv6 service-type #45031159848 and AI handoff #45031159852 passed on this parent.
- Exact PPP integration descendant `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`: VPN lifecycle #45031084455 failed because the checker compared a literal escaped newline/tab sequence rather than normalizing the C source call; WAN route #45031084451 failed in `check_pppoe_mwan_status.py` because it still located the old two-argument worker signature. Both regressions are corrected.
- The PPPoE-MWAN partial-failure contract now also verifies the cleanup block is gated by the same immutable `launch_mode->multiwan_enabled` selector before stopping partial PPP sessions.
- The same parent had unrelated source-contract failures in LAN DHCP and checked LAN IPv6 reload helpers (WAN DHCP #45031084413; modem/USB/firewall/process/XY checks on #45031084464, #45031084483, #45031084376, #45031084406); those are independent and remain the next cleanup slice. NVRAM audit #45031084347 was still running at last observation. The XY run failed before firmware build.
- No local tests or target firmware build were run in this connector-only session. No XY-C1/TRX/artifact result is claimed.
- Result: THIS COMMIT. Next: reconcile checked LAN IPv6 snapshot source contracts and LAN DHCP hostname publication tests; continue to monitor pending NVRAM audit.

---

## 2026-11-22 — Distinguish router-hosts IPv6 preflight from emission

- Parent main SHA: `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`. The PPP launch-selector integration correction `52a8c60d1e9aa324bb7f020fdeafda99a03319e9` has triggered exact-head WAN route #45031084451, WAN DHCP #45031084413, USB modem #45031084464, VPN lifecycle #45031084455, XY-C1 #45031084406, NVRAM audit #45031084347, AI handoff #45031084508, firewall startup #45031084376 and process-inventory #45031084490/#45031084483; all were queued at the last observation. Do not infer a result from those queued runs.
- Exact-head IPv6 service-type #45030079720 on ancestor `0af768201a45cf28482ffa4e8c42b6846e16081a` passed its GNU89/ASan/UBSan policy binary but failed in the standalone source checker. `update_hosts_router()` intentionally has two separate IPv6 branches: checked LAN address preflight before stage-file creation, then IPv6 hosts-row emission after the files have been staged. The checker used an unscoped `.index()` and selected the first conditional as the emission gate.
- Corrected the source contract to verify the checked IPv6 service/address read and error returns before staging, then locate the later emission conditional after `dnsmasq_file_open(hostname_temp)`. It also asserts that the obsolete unchecked `get_lan_addr6_host()` call is absent. Production source and runtime behavior are unchanged.
- No local tests were run in this connector-only session. The IPv6 service-type workflow should validate this checker correction on this exact commit; the pending PPP integration workflows remain independent.
- Result: THIS COMMIT. Next: inspect exact-head IPv6 service-type and PPP integration Actions; address only real failures before continuing the userland stability/security rotation.

---

## 2026-11-22 — Correct PPP launch selector integration contracts

- Parent main SHA: `d3805365ca95a17d5bd54736f29d5627580fa305`. On that exact source SHA, AI handoff #45030597515 and process lifecycle inventory #45030597384 passed; NVRAM audit #45030597487 was still pending at last check.
- WAN DHCP #45030597508, VPN lifecycle #45030597433, WAN route #45030597395, HTTPD security #45030597411 and XY-C1 #45030597436 failed at source-contract steps. The WAN DHCP, VPN, WAN route and XY-C1 checkers still located the former two-argument `launch_wan_pppd_locked()` signature. The HTTPD inline contract expected the deleted unchecked `nvram_match("wan_l2tpd")` selector. The PPP options checker also asserted a non-pointer `launch_mode.multiwan_enabled` token inside the locked worker although the worker correctly uses `launch_mode->multiwan_enabled`.
- Production inspection uncovered an additional real compile defect: the RPL2TP daemon-stop branch used the new checked `launch_mode->use_rpl2tp`, but the daemon-start branch still referenced the deleted local `use_rpl2tp`. Both paths now use the same immutable snapshot flag.
- Updated the independent source contracts for WAN interface-name bounds, VPN lifecycle, modem WAN startup, WAN route and HTTPD lifecycle to follow the new locked-worker signature and checked selector. The PPP option contract now tests the pointer-member field and preserves the wrapper's separate value-member check.
- The WAN PPP option policy test binary compiled and passed GNU89 with ASan/UBSan on parent `d3805365ca95a17d5bd54736f29d5627580fa305`; no local tests or firmware build were run in this connector-only session. Exact-head reruns of WAN DHCP/VPN/WAN route/HTTPD/XY-C1/AI are required for this correction.
- Result: THIS COMMIT. Next: inspect exact-head Actions for the corrective commit, then resolve the known independent IPv6 router-hosts checker ordering assertion.

---

## 2026-11-22 — Snapshot WAN PPP launch selectors before lifecycle changes

- Parent main SHA: `0af768201a45cf28482ffa4e8c42b6846e16081a`. On this exact SHA, WAN route #45030079767 passed; IPv6 service-type #45030079720 passed its GNU89 sanitizer binary but failed in a stale ordering assertion that selected the early IPv6 address-read branch instead of the later file-emission branch. AI handoff #45030079766 exposed missing mandatory checkpoint wording/section names in the current handoff; this commit restores the canonical required sections.
- `launch_wan_pppd()` previously used lossy `nvram_match()` reads for `pppoemwan_enable` and, on APP_RPL2TP builds, `wan_l2tpd`. A backend error could be mistaken for disabled multi-WAN (bypassing the lifecycle lock and skipping multi-WAN start/rollback) or select the wrong L2TP daemon.
- Added one checked NVRAM transaction snapshot for `pppoemwan_enable` and, only on APP_RPL2TP L2TP launches, `wan_l2tpd`. The wrapper fails before lock/session changes on any read/close/copy failure, then passes one immutable mode to the locked launch path. Multi-WAN stop/start/partial-failure cleanup and RPL2TP daemon selection all consume that same snapshot. Missing/empty values and non-literal values preserve exact `nvram_match(key, "1")` behavior.
- Added GNU89 policy regressions for exact string matching, missing defaults, protocol/build scoping, invalid inputs and checked-read failures. Extended `check_ppp_wan_options.py` to pin transaction adapters, selector scope, ordering and shared flag use.
- Local tests were not run in this connector-only session. Exact-head WAN route/VPN/HTTPD/XY-C1/AI Actions must validate this complete source SHA; no target firmware or hardware result is claimed.
- Result: THIS COMMIT. Next: inspect exact-head Actions for this SHA, fix any concrete failing test/source contract, then resolve the known IPv6 router-hosts checker ordering failure on a separate atomic slice.

---

## 2026-11-22 — Check xl2tpd transaction adapters through their wrapper bodies

- Parent main SHA: `ba5aaa320963ba7a0322effe5388e1e5b09d6166`. Exact-head VPN lifecycle run #45029214663 on `dcda9e5a118711d1b35b8e347e5fde54ccdc8270` passed all ten GNU89 C policy binaries, including the new xl2tpd selector-policy regression, then failed only in the source checker because it searched for `nvram_transaction_begin/get/end` text inside `xl2tpd_read_config_snapshot()`. Those calls correctly live in its three small transaction adapter functions.
- Updated the source contract to inspect the begin/get/end adapter bodies individually and keep the snapshot helper assertion focused on `rc_wan_nvram_copy_snapshot()`. No product source changed.
- The cleanup commit `ba5aaa320963ba7a0322effe5388e1e5b09d6166` started exact-head VPN/AI/WAN/XY workflows; this checker correction requires a fresh same-SHA rerun. No PASS is claimed from a pending run.
- Result: THIS COMMIT. Next: inspect the new VPN lifecycle workflow; if any remaining contract failure is stale, update it precisely, otherwise move to the next independent PPP launch-state read.

---

## 2026-11-22 — Simplify xl2tpd snapshot policy evaluation

- Parent main SHA: `dcda9e5a118711d1b35b8e347e5fde54ccdc8270`. The xl2tpd selector/credential snapshot implementation and GNU89 policy tests are committed in `dcda9e5a118711d1b35b8e347e5fde54ccdc8270`; exact-SHA VPN, AI, WAN, HTTPD, process and XY-C1 workflows are queued/in progress.
- Removed a redundant provisional flag evaluation before the checked WAN protocol read. The complete policy now runs exactly once, with the actual checked protocol, before process discovery or live configuration I/O. Also removed an unnecessary destination-size array and corrected a test comment describing non-RPL2TP WAN LAC behavior.
- No behavioral change beyond removing duplicate calculation. Local tests remain unrun; exact-SHA CI remains authoritative.
- Result: THIS COMMIT. Next: inspect this new SHA's VPN-server regression output and fix any compiler or source-contract failure.

---

## 2026-11-22 — Snapshot xl2tpd VPN selectors and credentials before config rewrite

- Parent main SHA: `47f458e55610dcb91703e72b0e8a236ec0ca75e1`. At that SHA the exact-head source workflows for DNSMasq #45028829756, WAN DHCP #45028829655, Samba #45028829731, WAN route #45028829637, process lifecycle #45028829646/#45028829818, HTTPD security #45028829630, Inadyn #45028829709, miniupnpd #45028829662 and AI handoff #45028829822 PASS. NVRAM transaction audit #45028829724 and XY-C1 firmware #45028829692 were still in progress at last observation.
- `safe_start_xl2tpd_locked()` still used lossy `nvram_match()/nvram_invmatch()` for VPN server/client/L2TP selectors and `nvram_safe_get()` for VPN-client peer/username and default route. A backend read/transaction failure could therefore be interpreted as a disabled optional VPN and cause the shared `xl2tpd.conf` to be rewritten without a required LAC/LNS section.
- Added one bounded NVRAM transaction snapshot for `vpns_enable/type`, `vpnc_enable/type`, `vpnc_dgw`, `wan_l2tpd`, `vpnc_peer` and `vpnc_user`. Backend, copy/truncation and transaction-close failures now abort before process scan or opening/truncating the live config. Empty/missing values retain legacy `nvram_match/invmatch` defaults.
- Added a pure flag policy and GNU89 regression cases for active VPN server/client, disabled/missing selectors, APP_RPL2TP inversion, WAN protocol mismatch and backend error. Extended the existing VPN lifecycle source contract to pin all snapshot fields, transaction adapters and failure ordering. Local tests were not run in this connector-only runtime; exact-head VPN Actions must verify this slice.
- Result: THIS COMMIT. Next: inspect exact-head VPN and AI handoff runs; then review `launch_wan_pppd()` / L2TP protocol selectors for remaining unchecked NVRAM-dependent launch decisions without reopening completed lifecycle-lock work.

---

## 2026-11-22 — Preserve DHCPv6 server mode read failures

- Parent main SHA: `d7b9063ffd43613534909b7707783c7b9f967d12`. The exact-head IPv6 service-type run for `e3cc86e9503b978ddbe2ab49ff498db81076da54` passed the compiled GNU89/ASan/UBSan policy binary and failed only in the source checker due to its local variable typo; `d7b9063ffd43613534909b7707783c7b9f967d12` corrected that typo and started a fresh exact-SHA run. WAN teardown #45028413713 and WAN route #45028413724 on `e3cc86e...` PASS; AI docs #45028413690 PASS. XY-C1 #45028413654 and NVRAM #45028413627 were still running at handoff.
- `get_lan_dhcp6s_mode()` still used `nvram_get_int("ip6_lan_dhcp")`, collapsing backend/transaction errors into a legacy numeric default even though this getter already returns `-1` for other unavailable states. It now uses the checked one-key transaction reader and returns `-1` on read/release failure.
- Refactored the legacy integer parser so missing, empty, malformed, trailing-junk and out-of-range values retain `atoi()` defaults; valid leading numeric values (including negative values) are preserved. Prefix-size normalization reuses the parser and still defaults to /64 outside 48..64.
- Added two parser regressions and source contracts for checked mode read/error propagation. Local tests were not run in this connector-only runtime; exact-SHA Actions are required.
- Result: THIS COMMIT. Next: inspect the new exact-head service/XY workflows, then review `get_lan_addr6_host()` and related cached LAN address readers for stale-pointer/error fallback behavior; do not change cached reads unless a caller-visible defect is proven.

---

## 2026-11-22 — Fix LAN RA source checker policy handle

- Parent main SHA: `e3cc86e9503b978ddbe2ab49ff498db81076da54`. Exact-head IPv6 service-type run #45028413750 compiled and passed its GNU89/ASan/UBSan policy binary, then the source checker stopped at the new RA assertion with `NameError: policy is not defined`. This checker loads the relevant header as a distinct source, so the assertion used the wrong local variable.
- The check now loads `lan_addr6_reload_policy.h` as `lan6_policy` and validates the helper there. Runtime source and policy tests are unchanged.
- Result: THIS COMMIT. Re-run IPv6 service-type and all exact-head Actions on the new SHA.

---

## 2026-11-22 — Preserve router-advertisement NVRAM read errors

- Parent main SHA: `75a61dfe4f48664a6a35caa0e0f1df6243fe48a8`. The LAN reset source-contract corrections are committed in `75a61dfe4f48664a6a35caa0e0f1df6243fe48a8`; its exact-SHA workflows were just enqueued (no result asserted yet). On the prior source SHA `e45027e842af2c42de6760991fbe9d80fa21967c`, the XY-C1 host-only checks had passed through the LAN IPv6 publication contract and the firmware Build step was still in progress. Two unrelated source contracts failed because they expected the removed direct LAN config getters; the checker corrections are in `75a61dfe...`.
- `is_lan_radv_on()` used `nvram_invmatch("ip6_lan_radv","0")`; a failed backend read collapsed to “not equal zero”, incorrectly enabling router advertisements. Added a one-key checked transaction reader and a status-bearing policy. The API now returns `-1` on backend/read/close failure; only a clean read with literal `"0"` disables RA. Missing/empty values preserve the legacy enabled default.
- Added four policy regressions and source contracts for transactional read, missing-key compatibility and error propagation. No other LAN DHCP mode or service semantics changed.
- Local tests were not run in this connector-only runtime. Exact-head XY-C1/IPv6 service/AI workflows are required after this commit.
- Result: THIS COMMIT. Next: audit `get_lan_dhcp6s_mode()` for its unchecked `nvram_get_int("ip6_lan_dhcp")` read and preserve atoi-style defaults while propagating read/transaction failures to existing -1 callers.

---

## 2026-11-22 — Align LAN reset source contracts with transactional configuration snapshot

- Parent main SHA: `cac4f73f3757d1617b2fb34a58fd7bb2f65af589`. Exact-head XY-C1 run #45028146852 on `e45027e842af2c42de6760991fbe9d80fa21967c` remained in progress after the initial host-only test phase; the LAN address reload GNU89 sanitizer binary itself passed and the new LAN reset snapshot source contract was exercised by the XY workflow. The same SHA's WAN route #45028146813 and AI handoff #45028146832 passed.
- Two independent source contracts failed on the e450 commit because they encoded the old implementation: `scripts/check_ipv6_service_type_status.py` required direct `nvram_safe_get("ip6_lan_addr")` in `reset_lan6_vars()`, and WAN teardown's inline contract required the former `lan_size6` local. Both now assert the checked snapshot, transaction begin/get/end adapters, and the failure gate before publication / the copied prefix size.
- This commit changes validation only; the LAN reset runtime code remains in `e45027e842af2c42de6760991fbe9d80fa21967c`. Local tests were not run in this connector-only runtime. Recheck the exact new main SHA actions; the e450 XY build must be checked separately for its later Build/TRX/artifact steps.
- Result: THIS COMMIT. Next: harden `is_lan_radv_on()` so a failed `ip6_lan_radv` backend read is returned as unknown instead of silently treating it as enabled.

---

## 2026-11-22 — Fail closed on LAN IPv6 reset configuration read errors

- Parent main SHA: `16f571b03e7ea5e29e76a1b06934531fc4644f32`. The active DHCPv6 callback fix is validated on its exact SHA `5e0100c059639b323c903351d8f1fab6ff181659`: WAN DHCP #45027565522 PASS and AI handoff #45027565508 PASS. The XY-C1 source-contract correction in `1805fa23a010044d4f82c7e23e91695a6b7f904d` was still being checked when this slice began; exact target build evidence must be rechecked after the new commit.
- `reset_lan6_vars()` previously used `nvram_match("ip6_lan_auto","0")`, `nvram_safe_get("ip6_lan_addr")` and `nvram_get_int("ip6_lan_size")` and then published `lan_addr6` even when a configuration read/transaction failed. A transient unreadable address could therefore clear a valid previously published LAN IPv6 address.
- Added a single checked NVRAM transaction for the relevant auto/address/prefix fields, with mode-specific selection (disabled/6to4/6rd skip static address, native-static/6in4 force it). Backend/read/transaction-close/truncation failures now return before the `lan_addr6` write. `is_lan_addr6_static()` uses the same status-bearing snapshot rather than the unchecked matcher.
- Preserved legacy behavior for missing/malformed values: missing `ip6_lan_auto` remains non-static; missing/empty/non-numeric/out-of-range prefix size still normalizes to 64; numeric strings retain atoi-style leading-number parsing. Invalid IPv6 address text still follows the existing parse/no-address behavior.
- Added five GNU89 policy regressions for service-type overrides, auto-read failure, missing/malformed match defaults, prefix-size compatibility and read-error propagation. XY-C1 source contract pins transaction adapters and verifies the snapshot error gate precedes the NVRAM publication.
- Local tests were not run in this connector-only runtime. The exact-head XY-C1 workflow must compile/run the LAN policy binary and pass the source contract before proceeding to Build XY-C1/TRX/artifact.
- Result: THIS COMMIT. Next: inspect the exact-SHA XY-C1 workflow and then audit `is_lan_radv_on()` for a checked `ip6_lan_radv` read that currently collapses backend errors into “RA enabled”.

---

## 2026-11-22 — Restore validator-required handoff checkpoint sections

- Parent main SHA: `3fd35299892c4499f9efbce8e50617307b71f2e3`. VPN server lifecycle #45027707286 PASS on `3fd35299892c4499f9efbce8e50617307b71f2e3`; AI handoff #45027707271 failed only because the rewritten handoff omitted the required end-of-session checkpoint phrase and canonical section headings.
- Restored the mandatory checkpoint wording and exact headings required by `scripts/check_ai_handoff_docs.py`, while retaining the current main/backup identities, VPN validation state and next concrete action. No source behavior changed.
- The original AI validator log identified the single first failure; the next exact-head run must confirm that all remaining document limits and cross-file governance contracts also pass.
- Result: THIS COMMIT. Next: inspect exact-head AI handoff, then resume the wider userland stability/security rotation.

---

## 2026-11-22 — Require checked WAN IPv6 endpoint handling in VPN export

- Parent main SHA: `1805fa23a010044d4f82c7e23e91695a6b7f904d`. The earlier VPN server lifecycle run #45026956696 on `db11b15bb545dbf9dafff9c4795600735884e0dc` failed in the final source contract because it still required the legacy `get_wan_addr6_host()` call.
- Production OpenVPN export now uses `get_wan_addr6_host_status()`: interface-identity and address-enumeration failures abort the staged export, while a clean miss remains eligible for LAN and dual-stack IPv4 fallback.
- Updated the VPN source contract to pin bounded checked identity retrieval, getifaddrs error preservation, policy clean-miss semantics, and the status-error gate before fallback. Existing fake-backend test cases for read failure, enumeration failure, missing identity and clean miss are now required by the contract. Product code is unchanged.
- Parent exact-head AI handoff #45027565508 and WAN DHCP callback #45027565522 both PASS. The new VPN workflow run must validate this checker-only correction.
- Local tests were not run: this runtime cannot resolve GitHub to create a local checkout. Exact-head VPN Actions is the required regression validation.
- Result: THIS COMMIT. Next: inspect exact new-head VPN/AI Actions; then continue with a separate userland stability/security defect.

---

## 2026-11-22 — Align XY-C1 LAN IPv6 publication contract with checked callback

- Parent main SHA: `5e0100c059639b323c903351d8f1fab6ff181659`. Exact-head WAN DHCP #45027565522 PASS: the source-policy checker, WAN IPv6 DNS tests and all five new LAN-address fake-backend cases compiled and ran under GNU89 with ASan/UBSan; DHCPv6 config publication, callback generation fencing, and all remaining lease/callback contracts also passed. AI handoff #45027565508 PASS.
- XY-C1 firmware validation #45027298483 on `a9aa6ba...` passed its preceding host test binaries and stopped at step 28 only because its inline contract still expected `store_lan_addr6(lan_addr6_new)` directly in `dhcp6c_main()`. The checked lookup/publish adapter now owns that operation.
- Updated the XY-C1 workflow source contract to assert the status-bearing LAN lookup, errno preservation, checked publisher, and callback error aggregation. Build, TRX and artifact upload were dependency-skipped after the stale assertion; no L3 result is claimed. The workflow includes its own path in the trigger filter, so this correction should produce a fresh exact-SHA build run.
- Result: THIS COMMIT. Next: inspect the new XY-C1 host checks and Build/TRX/artifact steps; continue wider userland stability work while independent checks run.

---

## 2026-11-22 — Update DHCP callback workflow assertion for checked LAN lookup

- Parent main SHA: `98b54718077878a93758c327277ce10fc8c6a44f`. Exact-head AI handoff #45027419717 PASS. WAN route #45027298482 PASS on the preceding source/checker state. WAN DHCP #45027419733 passed its first 20 steps, including `check_wan6_dns_policy.py`, the WAN6 identity sanitizer binary (all new fake-backend cases), DHCPv6 DNS, DUID/config publication, callback generation fencing, and lease/rollback binaries. It failed only in the final inline Python assertion at workflow line 956, which still required a direct `store_lan_addr6()` call inside `dhcp6c_main()`.
- Updated that workflow contract to require the `wan6_identity_publish_lan_addr6()` gate, callback error propagation, and absence of the old unchecked direct address lookup/publication. The direct store remains inside its checked adapter.
- NVRAM audit, HTTPD security, Process lifecycle and XY-C1 build runs from `a9aa6ba...` were in progress at last check. Exact new-main SHA Actions must be rechecked after this workflow-only correction. No firmware PASS is claimed; local tests are not run in this connector-only runtime.
- Result: THIS COMMIT. Next: inspect the exact-head callback workflow, then continue the wider userland stability/security rotation.

---

## 2026-11-22 — Update DHCPv6 callback ordering contract for LAN address policy

- Parent main SHA: `a9aa6ba87339b0e21d1deed8464723a4aba50e9d`. Exact-head WAN route #45027298482 PASS, including all route sanitizer and source-contract steps; AI handoff #45027298505 PASS; IPv6 service-type #45027298602 PASS.
- WAN DHCP #45027298503 completed with failure only in `scripts/check_wan6_dns_policy.py`: the existing inline source contract still searched for a direct `store_lan_addr6()` call in `dhcp6c_main()`. The checked publication helper intentionally moved that call into `wan6_publish_lan_addr6_status()`.
- Updated the ordering contract to require the policy gate before DNS processing, callback failure propagation on lookup/publish error, and absence of direct unchecked bridge-address publication in the callback. The new tests and lower-file helper assertions remain in place.
- XY-C1 build #45027298483, NVRAM audit #45027298455, HTTPD security #45027298564 and Process lifecycle #45027298542 were still in progress at last observation. No firmware PASS claimed. Local tests are not run in this connector-only environment.
- Result: THIS COMMIT. Next: inspect exact-head WAN DHCP run; then continue with the next independently evidenced Padavan userland reliability/security defect.

---

## 2026-11-22 — Remove obsolete DHCPv6 callback local

- Parent main SHA: `9e77c6924cc6bce18cb3fcb93d2e0b20ecd27f9c`. The LAN address callback now uses the bounded `addr6s` buffer directly through `wan6_identity_publish_lan_addr6()`; the old `lan_addr6_new` pointer local became unused after that refactor and would fail the project’s `-Werror` C build.
- Removed the dead local only. No runtime behavior changed.
- Result: THIS COMMIT. Exact-head WAN DHCP/route sanitizer workflows will validate the callback policy and production source contracts.

---

## 2026-11-22 — Propagate DHCPv6 LAN address enumeration failures

- Parent main SHA: `72a1ca0a81a3cc7a30183bfb6527337c00bf94f8`. The DHCPv6 lease callback previously called `get_ifaddr6(IFNAME_BR,...)` and passed its nullable result to `store_lan_addr6()`; the latter treats NULL/empty as a successful no-op. Since `get_ifaddr6()` also returns NULL with errno set when `getifaddrs()` fails, the callback silently reported success and left the last published LAN address untouched after an actual lookup failure.
- Added an address-publication policy that preserves the legacy clean-miss behavior (no new address means do not overwrite the last published value) while returning lookup/backend errors. The runtime adapter clears errno before address enumeration; `dhcp6c_main()` records enumeration/publication errors as callback failures while still attempting independent DNS processing.
- Added five GNU89 fake-backend cases for lookup failure, clean miss, success, publish failure and invalid arguments; both WAN DHCP and WAN route source contracts enforce the checked path. Local sanitizer tests were not run here; exact-SHA CI will provide the verification.
- Parent exact-SHA WAN DHCP #45027101788 and WAN route #45027101849 were still in progress; AI handoff #45027101798 was also in progress when this source task began. Their outcomes should be checked separately from this new commit.
- Result: THIS COMMIT. Next: inspect DHCPv6 callback lease ordering after tests; then move on to a separate userland stability/security issue if no further checked-read regression remains.

---

## 2026-11-22 — Correct OpenVPN IPv6 endpoint source-contract signatures

- Parent main SHA: `db11b15bb545dbf9dafff9c4795600735884e0dc`. Exact-head WAN route #45026956811 compiled and passed all earlier sanitizer/policy checks, then the added `scripts/check_wan_dns_read_status.py` contract could not locate the new status helper because its C return type and function name are split across lines. Exact-head WAN DHCP #45026956814 also passed all earlier regression binaries and DNS policy checks, then failed on the old assertion that the lookup remained inside the legacy pointer wrapper.
- Updated the route checker to locate the definition by its function-name substring and migrated the DHCP checker to validate the status-bearing helper, getifaddrs errno handling, fake-backend regressions, OpenVPN fail-closed behavior and LAN fallback ordering. These are test-harness corrections; the product-source commit `db11b15bb545dbf9dafff9c4795600735884e0dc` is unchanged.
- AI handoff validation on `db11b15...` is queued/in progress at the time of this correction; WAN DHCP and route both failed only in the stale/new checker contracts as detailed above. Recheck every relevant exact-SHA Actions run after this correction. Local tests remain not run in this connector-only environment.
- Result: THIS COMMIT. Next: inspect DHCPv6 callback LAN address retrieval; determine whether `get_ifaddr6(IFNAME_BR,...)` can collapse getifaddrs errors or publish an uninitialized address through `store_lan_addr6()`. Continue beyond DHCPv6 if this has no concrete failure.

---

## 2026-11-22 — Keep OpenVPN IPv6 endpoint errors distinct from clean address misses

- Parent main SHA: `52d3648af131fc24adfc607bb9f9f6e228349fff`. Exact-head WAN DHCP #45026555245, WAN route #45026555259, IPv6 service-type #45026555291, AI handoff #45026555208, HTTPD security #45026555260 and Process inventory #45026555340 passed. NVRAM transaction audit #45026555243 and XY-C1 build #45026555307 remained in progress at the last observation.
- A prior overlapping source edit had already replaced the lossy `ifname6` pointer getter in `get_wan_addr6_host()` with a checked copy and cleared errno before `get_ifaddr6()`. The remaining defect was at its only RC caller: `ovpn_server_expcli_main()` treated every NULL result as a clean miss and silently fell back to the LAN address, even when the NVRAM read or `getifaddrs()` failed.
- Added a status-bearing helper that preserves the checked `wan0_ifname6` error and the address-enumeration errno; kept the legacy pointer ABI wrapper while setting errno on failure. OpenVPN export now aborts through its staging cleanup on lookup errors. An empty configured identity or a clean no-address result still allows the legacy LAN/IPv4 fallback.
- Added six fake-backend identity regressions and route source contracts covering NVRAM errors, missing interface, successful prefix removal, getifaddrs errors, clean address misses and OpenVPN fallback ordering. Local tests were not run in this connector-only environment; verify exact-SHA Actions after this atomic commit.
- Result: THIS COMMIT. Next: inspect `start_dhcp6c()` and DHCPv6 lease callbacks for error-collapsing NVRAM reads before interface/session or route/DNS side effects, then continue the wider Padavan userland stability rotation.

---

## 2026-11-22 — Use a checked stable interface name for IPv6 host-address lookup

- Parent main SHA: `5df642aad7357f8817c674cde37ff696071366ac`. `get_wan_addr6_host()` fetched `wan0_ifname6` through the pointer-returning getter and immediately passed that cache-owned pointer to interface-address lookup. The pointer lifetime happened to be short, but a backend read failure was still collapsed into the missing-interface path and could not be distinguished by callers.
- Replaced the borrowed pointer with a bounded `IFNAMSIZ` local snapshot from `get_wan_unit_value_copy_status()`. The function now returns `NULL` with the concrete read error in `errno`, keeps missing/empty interface behavior unchanged, clears stale errno after a successful empty read, and rejects a null output buffer with `EINVAL`.
- Extended the WAN6 source contract to pin stable-buffer, checked-read/error propagation and ordering before `get_ifaddr6()`. Existing generic checked-copy GNU89 tests cover missing keys, backend errors and truncation.
- Next task: inspect the exact-SHA checks on the recent teardown-contract fix, then audit the remaining IPv6 teardown and DNS consumers for lossy NVRAM reads.

---
## 2026-11-22 — Fix escaped newline in WAN route teardown contract

- Parent main SHA: `da151744e963da9ae5af21227fe8bd42a97c6797`. The exact-SHA `wan-route-status` job reached the refreshed `wan6_down()` contract and failed with `ValueError: substring not found`: the inserted Python search needle contained two literal backslashes before `n`/`t`, while the extracted C source contains actual newline/tab characters. The IPv6 service-type checker and handoff validator both passed on this SHA.
- Corrected the workflow needle to use the same single-escaped newline/tab pattern as the already-updated later teardown assertion. Runtime code is unchanged.
- Next task: finish the `get_wan_addr6_host()` checked interface-identity audit and rerun current main SHA checks.

---
## 2026-11-22 — Align WAN6 teardown source contracts with checked identity reads

- Parent main SHA: `475053e6acf945b73d91b630a4fc1f3ff5e14839`. Exact-SHA checks exposed two stale assertions after `wan6_down()` moved from the lossy `get_wan_unit_value(unit, "ifname6")` getter to a bounded status-bearing copy: the IPv6 service-type checker still searched for the old getter, and the earlier WAN route workflow assertion still required the old pointer assignment. The later route-workflow contract had already been migrated, but its earlier sibling had not.
- Updated both contracts to require checked-copy, early error-return and interface-probe ordering, and to forbid the old getter. The production teardown implementation is unchanged.
- Prior exact-SHA run results: `f2765a6` WAN DHCP failed on `dns_policy` NameError; `a4f550a` fixed that and then the WAN DHCP workflow reached the stale `wan6_down` contract; `4258f28` updated one route contract but malformed its YAML indentation; `475053e` repaired the indentation. A fresh exact-SHA run is required for the corrected contracts.
- Next task: finish `get_wan_addr6_host()` checked identity and pointer lifetime audit, then inspect exact-SHA WAN DHCP/route/IPv6 checks.

---
## 2026-11-22 — Repair YAML indentation in WAN DHCP teardown contract

- Parent main SHA: `4258f281fe2f5bc6d5705004d0a1cea33e101b3a`. The workflow-contract change in `4258f281fe2f5bc6d5705004d0a1cea33e101b3a` correctly replaced the stale pointer-getter assertion but omitted the YAML block-scalar indentation on five continuation lines. GitHub consequently registered the push under the workflow path with no job records; this was a malformed workflow definition, not a test result.
- Restored the required indentation for all inserted Python statements while preserving the checked-read/error/probe ordering assertions. No product source changed.
- The previous `a4f550ace8a2bbba3723ea08c0eaae59762e9c2e` WAN DHCP run had passed all C regression jobs and failed only at the stale teardown assertion; commit `4258f281...` did not produce a valid workflow run. Re-run all relevant checks on this commit's exact SHA before claiming validation.
- Result: THIS COMMIT. Next: continue the checked-read/pointer-lifetime audit of `get_wan_addr6_host()` after confirming its callers.

---

## 2026-11-22 — Update WAN DHCP teardown contract for checked interface identity

- Parent main SHA: `a4f550ace8a2bbba3723ea08c0eaae59762e9c2e`. Exact-head WAN DHCP #45025898727 compiled and ran its full preceding GNU89/ASan/UBSan policy suite, including DHCPv6 DNS withdrawal, WAN IPv6 identity, lease retry, route rollback, static IPTV startup, DHCPv6 configuration publication and callback fencing. The final inline Python contract then failed at its stale assertion requiring `previous_ifname = get_wan_unit_value(unit, "ifname6")` in `wan6_down()`.
- The implementation had already been hardened in `5247382388b17f4fdeb0a60fa46599583ce39e17`: it copies `ifname6` through the checked status API and aborts before interface probing when the read fails. Updated the workflow assertion to pin this read/error/probe order and the absence of the old lossy getter. Runtime C code is unchanged.
- Exact `a4f550ace8a2bbba3723ea08c0eaae59762e9c2e` AI handoff #45025898747 PASS; WAN DHCP #45025898727 FAILED only at this stale assertion after all listed compiled C policy jobs passed. The new workflow commit requires a fresh exact-SHA rerun.
- Result: THIS COMMIT. Next: finish the `get_wan_addr6_host()` checked identity/pointer-lifetime audit and fix only verified caller-visible issues.

---

## 2026-11-22 — Fix DNS checker policy-variable reference

- Parent main SHA: `f2765a60ea34a6174fa3e2ad7202cce818c7d683`. The WAN DHCP exact-head rerun #45025732489 passed all preceding compiled policy binaries and contracts, then the corrected DNS assertion itself failed with `NameError: dns_policy is not defined`. The script already loads the policy header as `policy`; the new assertion used the wrong variable name.
- Corrected the checker to inspect the existing `policy` variable. This is a test harness correction only; production C code is unchanged.
- Result: THIS COMMIT. Next: audit `get_wan_addr6_host()` for checked interface identity reads and pointer lifetime; migrate call sites to a status-bearing API only where callers can consume the error.

---

## 2026-11-22 — Align DNS checker with the checked old-value gate

- Parent main SHA: `5247382388b17f4fdeb0a60fa46599583ce39e17`. The exact-SHA `wan-dhcp-callback` run compiled and executed its preceding GNU89/ASan policy programs successfully; the final Python contract failed because the existing checker expected `wan6_dns_store_if_changed()` to be called directly in `store_wan_dns6()`, while the new fail-closed wrapper now calls that lower-level helper after verifying the old-value read status.
- Updated the source contract to require the integration wrapper in `store_wan_dns6()` and the delegated compare/write policy in `wan6_dns_policy.h`. No runtime code changed.
- Next task: audit `get_wan_addr6_host()`'s interface-name read for error and pointer-lifetime handling.

---
## 2026-11-22 — Fail closed when previous IPv6 interface identity cannot be read

- Parent main SHA: `273db47f11e6d8f5f71d387456677d47141282d3`. `wan6_down()` used the pointer-returning `get_wan_unit_value(unit, "ifname6")`. A failed/missing cached read was indistinguishable from an absent identity, so teardown could silently choose the current WAN/SIT fallback and leave addresses/routes on a previous PPP/device interface.
- Replaced it with a bounded `get_wan_unit_value_copy_status()` read into a stable interface-name buffer. Read errors now return before any interface existence probe or address/route mutation; an actually missing key remains an empty string and keeps the existing fallback policy.
- Both WAN source contracts enforce the checked read/error gate before fallback selection. Existing checked-copy regression coverage exercises missing, backend-error and truncation outcomes.
- Next task: audit `get_wan_addr6_host()` and `store_wan_dns6()` callers for pointer lifetime and error-propagation after the checked DNS fix, then continue the IPv6 teardown side-effect order.

---
## 2026-11-22 — Preserve DHCPv6 DNS withdrawal when the old NVRAM value is unreadable

- Parent main SHA: `ced9b3e9ff811d6ed140e4ebad7a288dd907e8b2`. `store_wan_dns6()` fetched the previous `wan0_dns6` with the lossy pointer getter. If the NVRAM backend failed while the DHCPv6 lease withdrew its DNS option (new value empty), the read became an empty fallback and `wan6_dns_store_if_changed()` skipped the write, leaving stale DNS servers published.
- Replaced the pointer getter with `get_wan_unit_value_copy_status()` and a policy gate that converts read errors into negative errno results before deciding whether a value changed. A genuinely missing key still maps to empty; unchanged values still avoid redundant writes.
- Added GNU89 policy regressions proving an old-value read failure never calls the writer, while a missing old key still publishes a non-empty DNS value.
- Next task: make `wan6_down()` preserve the old interface identity if its `wanN_ifname6` read fails, rather than treating the failure as an absent interface and selecting a fallback teardown target.

---
## 2026-11-22 — Snapshot WAN IPv6 unit address settings before runtime changes

- Parent main SHA: `ce01fb84ca28cf0b4faca46c6d28a8b7557c134d`. `wan6_up()` used four pointer-returning `get_wan_unit_value()` reads for SIT IPv4 address/gateway/IPv6 prefix and native-static IPv6 address/gateway after it had already stopped DHCPv6, published DNS and changed bridge DAD. Cache reuse could invalidate earlier pointers, and a failed NVRAM read was indistinguishable from an empty setting.
- Added `wan6_read_unit_address_snapshot()` to copy each mode's required `wanN_*` fields through the existing transaction callbacks and bounded copy-snapshot policy. SIT 6in4/6rd captures the three-value tuple; 6to4 captures only IPv4 address/gateway; native-static captures address/gateway. Missing values remain empty and checked read/close/truncation errors abort before stopping the old DHCPv6 client.
- Native-static IPv6 literals and the SIT WAN IPv4/prefix inputs are now validated from the stable copies before any stop/DNS/DAD/route/interface side effect. Removed the late pointer getters.
- Added GNU89 regression coverage for one-generation three-field copies and each backend read failure discarding the entire partial tuple. Both WAN source contracts require the transaction and pre-stop order.
- Next task: audit `wan6_up()` global native DHCP6 selectors and `start_dhcp6c()` callback identity for duplicate reads versus transaction coherence; then continue to route/neighbor data selectors.

---
## 2026-11-22 — Correct SIT source-contract assertion for macro scope

- Parent main SHA: `129697133e29a7757adeff9c61e590b825aba915`. The exact-SHA `wan-dhcp-callback` and `wan-route-status` runs both compiled and ran the GNU89 ASan/UBSan `wan_nvram_status_policy` binary successfully, and all preceding source contracts passed. Their final checker assertions failed because `errno = status;` is in the file-scope `WAN6_SIT_READ_CHECK` macro, outside the extracted `start_sit_tunnel()` function body.
- Corrected both source contracts to inspect the function for the macro invocation and inspect the macro definition at file scope for explicit errno preservation. This is a checker-only correction; the SIT implementation is unchanged.
- Exact main-SHA Actions currently show `validate-handoff-records`, `ipv6-service-type`, `inventory`, `HTTPD security`, `nvram-transaction-boundary` and XY-C1 build as successful or still running; the two WAN jobs failed only at the macro-scope assertion just described. No overall PASS/build PASS is claimed.
- Next task: transactionally snapshot the per-unit WAN IPv4/IPv6 address and gateway tuple in `wan6_up()` before stopping DHCPv6 or changing routes/interface state.

---
## 2026-11-22 — Check every SIT tunnel configuration input before replacing the tunnel

- Parent main SHA: `ada0d247ede7df03a9bdba5ab3de20da6dfe2d0b`. `start_sit_tunnel()` still used lossy `nvram_safe_get()`, `nvram_get_int()`, `get_wan_unit_value()` and `get_wan_unit_value_int()` for 6in4 remote/gateway, 6to4 relay, 6rd size/relay, MTU and TTL. A backend read fault could be mistaken for an absent/default value and trigger tunnel replacement with incomplete configuration.
- Added checked NVRAM string/integer snapshot helpers and switched all applicable global and per-unit selectors to status-returning reads. Missing optional keys preserve the existing empty/zero defaults; read, malformed-value and truncation errors are returned before deleting or creating the SIT tunnel. Existing MTU/TTL clamp behavior and hard-coded WAN unit 0 semantics are unchanged.
- Both WAN source contracts now reject the lossy getters in `start_sit_tunnel()`, enforce that all configuration reads precede the first tunnel mutation, and require the existing checked-copy/integer missing-key/backend-error/truncation regressions to be registered.
- Historical exact-SHA Actions inspection found the validator failure on `5874011`, `c57d750` and `54a8ea0` was the same root issue: the rolling root `dev_log.md` exceeded its 120-record threshold. Rotated the 104 oldest records losslessly into `docs/dev-log/2026/11/part-026.md` and retained the newest 20; corrected the previous premature statements that push check runs were inaccessible.
- Local host C binaries were not run in this network-isolated session. Current exact-SHA push Actions are inspected through the checks/runs endpoints; no build or CI PASS is claimed until their conclusions are available.
- Next task: replace the unchecked per-unit WAN IPv4/IPv6 tuple reads in `wan6_up()` with a checked snapshot completed before stopping the active DHCPv6 client or changing interface state.

---
## 2026-11-22 — Propagate DHCPv6 PPP interface selector read errors

- Parent main SHA: 54a8ea07bc4e7ca77750392863c774d566e74675. `is_wan_ipv6_if_ppp()` used `nvram_get_int("ip6_wan_if")`; an NVRAM backend fault collapsed to the legacy zero/PPP result. The MAN WAN startup callsite then skipped physical `wan6_up()`, silently losing native/SIT IPv6 activation.
- Added `is_wan_ipv6_if_ppp_status()` backed by `nvram_get_checked()` and a testable policy helper. Missing `ip6_wan_if` still maps to zero/PPP; malformed and backend errors remain distinct. The old boolean compatibility wrapper conservatively returns PPP on unknown state.
- `launch_wanx()` now consumes the status API and aggregates/logs read errors before deciding whether to skip PPP IPv6 or activate physical IPv6. Added and registered GNU89 regressions for missing key, 0/1 values, backend errors, malformed values and invalid output.
- Extended both WAN IPv6 source contracts to pin checked reads, compatibility fallback and first-error propagation. Direct exact-SHA inspection of `ada0d247ede7df03a9bdba5ab3de20da6dfe2d0b` found 39 push workflow runs / 43 check-runs, queued or in progress at the snapshot; no overall PASS was available yet.
- Next task: audit `start_sit_tunnel()` endpoint/relay/6rd size/MTU/TTL reads for error-collapsing getters before SIT tunnel creation.

---
## 2026-11-22 — Check native DHCPv6 RA/privacy selectors before startup

- Parent main SHA: c57d7501d031dba065e01bc3084733971d606368. `wan6_up()` used `nvram_invmatch("ip6_wan_dhcp", "1")` and `nvram_match("ip6_wan_priv", "1")` after stopping the previous DHCPv6 client and beginning runtime setup. An NVRAM fault could be treated as a normal non-`1` value and incorrectly enable RA/autoconfiguration or alter privacy behavior.
- It now uses the checked exact-string matcher before stopping the existing client or mutating DNS/interface state, rejects required read errors, and preserves the original short-circuit: the privacy key is not read when DHCP is enabled. Missing and malformed values retain the legacy exact-match mapping.
- Both WAN source contracts now enforce checked-read/error-gate ordering and require the existing generic exact-value/backend-error regressions.
- Local C/host tests were not run. Exact-SHA Actions on `54a8ea07bc4e7ca77750392863c774d566e74675` showed six successes, one `validate-handoff-records` failure because the root log had 122 records, and one check still in progress; no overall PASS.
- Next task: replace `is_wan_ipv6_if_ppp()`'s unchecked `ip6_wan_if` read with a status-bearing API and propagate failure through WAN IPv6 activation.

---
## 2026-11-22 — Fail closed on unreadable DHCPv6 6rd mode selector

- Parent main SHA: 58740111204bdfa6d8d457a87c8b338fb1bf0eed. `store_ip6rd_from_dhcp_status()` still used `nvram_match("ip6_6rd_dhcp", "0")`; a backend read failure was collapsed to “not disabled,” allowing a received DHCP 6rd option to be published despite an unknown local mode.
- The function now preserves no-op handling for absent/malformed option data, then reads the mode through the existing checked NVRAM matcher. Exact `0` still disables DHCP 6rd; a missing key and non-matching values preserve legacy automatic behavior; read errors return before `wan6_publish_6rd_snapshot()` so the DHCP lease callback can roll back the tuple.
- Added a GNU89 policy regression for exact-disabled, missing-key, malformed-value and backend-error behavior; both WAN source contracts require the checked selector gate and test registration.
- Local host tests were not run. Exact-SHA Actions on `c57d7501d031dba065e01bc3084733971d606368` later showed ten successes, one `validate-handoff-records` failure from the oversized rolling log, and one cancelled check-run; no overall PASS.
- Next task: replace `wan6_up()`'s unchecked `ip6_wan_dhcp` / `ip6_wan_priv` reads before native DHCPv6 RA/autoconfiguration/privacy changes.

---
## 2026-11-22 — Preserve the DHCPv6 no-op path before DUID lookup

- Parent main SHA: ef63749dad3c140c36fc1ea2afc951f6faaf17ea. The DUID MAC snapshot had been moved ahead of interface validation and the all-disabled early return. A transient `wan_hwaddr` read error could therefore make a no-op DHCPv6 invocation fail even when no DHCP/DNS/LAN client work was configured.
- Restored the order: validate the interface, read the active client flags, return successfully when all work is disabled, then read `wan_hwaddr` through the checked snapshot before callback-session lock, file creation, or process launch.
- Both WAN source contracts now pin this ordering. The existing missing-key, backend-read-failure and transaction-close-failure GNU89 regressions remain registered; no new runtime test body was needed for this call-order correction.
- Local host binaries were not run. Exact-SHA Actions on `58740111204bdfa6d8d457a87c8b338fb1bf0eed` later showed six successes, one `validate-handoff-records` failure because the root log had reached 120 records, and one cancelled check-run; no overall PASS.
- Next task: replace the remaining error-collapsing `ip6_6rd_dhcp` selector read in `store_ip6rd_from_dhcp_status()` while retaining malformed/absent option no-op behavior.

---
## 2026-11-22 — Read DHCPv6 DUID hardware address before resource acquisition

- Parent main SHA: df210641d1dcaaba795dde2c672e753643547f04. start_dhcp6c() used nvram_safe_get("wan_hwaddr") directly as the DUID/IAID hardware-address source, collapsing NVRAM backend failures to an empty value. A transient store error could thus be treated like a missing MAC while the client proceeded.
- It now reads the key through rc_wan_nvram_copy_snapshot() and the existing begin/get/end transaction callbacks before the first callback-session/file resource side effect. Missing or invalid MAC remains a legacy no-DUID case; backend get or transaction-close errors return before starting the client.
- Added three fake NVRAM tests: missing value compatibility, backend read failure, and transaction-close failure. Both WAN source-check scripts now require the checked one-field snapshot and reject nvram_safe_get("wan_hwaddr").
- Reset-input snapshot is present on this branch: confirmed in the live net_wan6.c source.
- Exact-head WAN DHCP, WAN route and AI handoff runs for this commit require live inspection. Do not infer a current-SHA PASS from a parent.
- XY-C1 Build/TRX/artifact L3 and hardware L4 remain NOT VERIFIED.
- Next: audit remaining DHCPv6 startup/lease configuration inputs for backend errors collapsed to defaults.

---
## 2026-11-22 — Fail closed on DHCPv6 DUID hardware-address NVRAM errors

- Parent main SHA: c338dc336ac798e6d319fb50fd0d3ec801fbc434. start_dhcp6c() used nvram_safe_get("wan_hwaddr") directly in ether_atoe(), so backend read failure was indistinguishable from a missing/invalid MAC and could be silently treated as normal while the DHCPv6 client started.
- Added a one-field checked rc_wan_nvram_copy_snapshot() before DUID file and callback-session side effects. A missing/invalid MAC still follows legacy behavior and skips DUID file generation; a real read or transaction-close error now aborts startup before acquiring resources.
- Added three fake-backend tests for missing-key compatibility, backend read failure, and transaction-close failure; both WAN source contracts require the checked snapshot and reject the legacy getter.
- Exact-head verification of the reset snapshot parent must be reviewed from live Actions; do not treat an ancestor PASS as proof of this combined source state. This commit also requires fresh exact-head WAN DHCP/WAN route/AI handoff runs.
- XY-C1 Build/TRX/artifact L3 and hardware L4 remain NOT VERIFIED.
- Next task: inspect remaining DHCPv6 lease/service startup inputs for error-collapsing NVRAM getters affecting route, DNS, DUID or client lifetime.

---
## 2026-11-22 — Snapshot IPv6 WAN reset settings before runtime mutation

- Parent main SHA: bbff528ccfe5f1cc45e9cbc87d699004867eb541. reset_wan6_vars_status() still consumed static address/prefix/gateway and 6rd relay/size with separate error-collapsing nvram_safe_get()/nvram_get_int() reads. Backend errors could masquerade as empty or zero configuration while reset logic proceeded.
- The active settings are now copied in one checked rc_wan_nvram_copy_snapshot() transaction: static mode reads address/prefix/gateway plus the 6rd pair; non-static mode reads only the 6rd pair. Prefix parsing uses rc_wan_nvram_parse_int_snapshot(), retaining empty-to-zero compatibility but rejecting malformed values. Transaction/read/close errors return before runtime NVRAM writes.
- Added three GNU89 fake-backend regressions covering coherent static tuple/prefix parse, failure injection at each of five fields with all outputs cleared, and the two-field dynamic mode. Updated both WAN source contracts to require the snapshot and reject legacy getters.
- Parent b7185685f1868b72ad48e87fadf434bdf7f7f0ee: WAN DHCP #45021657373 PASS, WAN route #45021657378 PASS, AI handoff #45021657238 PASS. These runs validate the previous DNS change, not this snapshot.
- Exact-head WAN DHCP, WAN route and AI handoff Actions are required for this new commit. XY-C1 L3 and hardware L4 remain NOT VERIFIED.
- Next task: inspect the `start_dhcp6c()` `wan_hwaddr` DUID/IAID read; preserve missing/invalid MAC behavior but stop startup on real backend read/close failure.

---
## 2026-11-22 — Align WAN DHCP source contract with compensated IPTV startup
- Parent main SHA: fe22b9fae01db2bb9fe57f2fffe67cee067ea364. WAN DHCP callback run #45022142584 passed every preceding host C regression, including the five new dynamic IPTV start/stop/rollback tests, then failed in its inline Python source contract at line 1242.
- The stale contract required literal assignments calling start_zcip_viptv(viptv_ifname) and start_udhcpc_viptv(viptv_ifname) inside launch_viptv_wan(). Those calls now sit behind wan_viptv_dynamic_start_callback() so failed launch can invoke stop-before-VLAN-rollback compensation.
- Updated the inline workflow contract to validate both real client start callbacks, the udhcpc/zcip stop callbacks and the compensated launcher call inside launch_viptv_wan(). Checker-only; no production behavior changed.
- Exact parent WAN route #45022142699, AI handoff #45022142702, WAN interface teardown #45022142730, HTTPD #45022142700 and IPv6 service-type #45022142701 PASS. Parent WAN DHCP failed only at the outdated assertion; NVRAM #45022142711 and XY-C1 #45022142628 remained in progress. New-SHA Actions must be checked independently.

---
## 2026-11-22 — Roll back IPTV VLAN after failed dynamic client start
- Parent main SHA: b7185685f1868b72ad48e87fadf434bdf7f7f0ee. launch_viptv_wan() published viptv_ifname as a recovery handle and tracked VLAN/bridge ownership, but DHCP/ZeroConf client launch errors returned directly. A failed detached udhcpc readiness check could leave the client/process uncertain while the newly-owned VLAN remained unreconciled.
- Added a dynamic IPTV start policy: start failure invokes the client-specific stop (udhcpc PID-file no-force stop, or the zcip process stop). Only a confirmed stop permits rollback_viptv_vlan_setup(); if stop is uncertain, the VLAN and recovery identity remain intact for later teardown. The primary launch status/errno remains authoritative; cleanup statuses are logged separately.
- Added five GNU89 ASan/UBSan policy regressions and expanded scripts/check_wan_vlan_setup.py to pin stop-before-rollback ordering, callback wiring, recovery identity retention and main() registration. The existing WAN workflow already watches source, policy and test and runs the Python checker.
- Parent exact-head AI handoff #45021657238, WAN DHCP callback #45021657373 and WAN route #45021657378 PASS. New-SHA results must be queried separately; XY-C1 will be triggered by net_wan.c.
- XY-C1 L3 remains unverified until same-SHA Build, TRX and artifact upload all pass; hardware L4 remains unverified.

---
## 2026-11-22 — Update WAN DNS source contracts for transactional static configuration reads

- Parent main SHA: 2d98a256c0703d02688de4ab88bb470bd1d5ee3d. Exact-head WAN DHCP callback run #45021524741 and WAN route status run #45021524732 exposed two stale Python source contracts after the DNS getter was correctly replaced. Both failures were in checkers, not C sanitizer tests.
- `check_wan_dns_read_status.py` still required `rc_wan_nvram_copy_text(nvram_safe_get("ip6_dns1"))` in build_dns6_value(); it now pins the checked `wan6_dns_copy_config_snapshot()` call after service-mode error handling.
- `check_wan6_dns_policy.py` expected a literal `destination[0]` clear in the DNS wrapper, but all-or-nothing clearing is correctly implemented by the shared `rc_wan_nvram_copy_snapshot()` helper as `fields[i].destination[0]`. The checker now verifies that shared implementation instead of a duplicate local clear.
- Parent exact-head HTTPD #45021524756 PASS, IPv6 service-type #45021524735 PASS, AI handoff #45021524758 PASS. WAN DHCP #45021524741 and WAN route #45021524732 failed on stale checker assumptions. NVRAM #45021524817 and XY-C1 #45021524718 were in progress at last inspection.
- The focused DNS policy C regressions and shared NVRAM sanitizer test were reached and their output showed successful behavior up to the Python source contract; re-run exact-head WAN workflows after this checker-only correction.
- Next source task remains static IPv6 WAN address/gateway/6rd reset inputs: inspect the remaining error-collapsing `nvram_safe_get()` calls in reset_wan6_vars_status() and move relevant values into one checked snapshot.

---
## 2026-11-22 — Read static WAN IPv6 DNS configuration as one checked NVRAM snapshot

- Parent main SHA: 1f1a8736e9e4edd0938002ee3a0321344eea3317. build_dns6_value() read ip6_dns1/ip6_dns2/ip6_dns3 through nvram_safe_get(). That API returns empty values when an NVRAM backend read fails, so a transient fault could be mistaken for intentionally empty static DNS and published as a DNS withdrawal.
- Replaced three error-collapsing reads with one rc_wan_nvram_copy_snapshot() transaction through the existing WAN NVRAM callbacks. All three cache-backed strings are copied before the next read; missing individual keys retain the legacy empty-string default; any backend or transaction-close error invalidates the complete tuple and prevents publication.
- Added five focused GNU89 host regressions for coherent cache-copying, missing-key compatibility, read failure, transaction close failure and begin failure. Updated check_wan6_dns_policy.py to reject the old nvram_safe_get path and require the checked transaction helper.
- Parent exact-head Actions on 1f1a8736e9e4edd0938002ee3a0321344eea3317: libdisk #45021103093 PASS, Process inventory #45021103117 PASS, AI handoff #45021103133 PASS and NVRAM transaction audit #45021103142 PASS. No XY-C1 Build/TRX/artifact gate was observed on this parent.
- Exact-head WAN DHCP callback Actions for THIS COMMIT must be checked after publication; no host test or CI result is claimed before that run completes. XY-C1 L3 and hardware L4 remain NOT VERIFIED.
- Next task: audit remaining DHCPv6 static WAN address/gateway and router advertisement settings for the same lossy NVRAM getter pattern, then add a checked snapshot if a caller can publish partial runtime identity.

---
## 2026-11-22 — Correct account-inventory test registration after exact-head failure

- Parent main SHA: a09d94d07031bf597c8b9ba77d65df6a40367dbb. Exact libdisk run #45020980995 failed at the first GNU89 test compile, before either sanitizer binary ran. The regression call list had matched the first generic return statement in the fake begin adapter instead of main(), producing implicit declarations and unused static test functions.
- Moved all ten checked-account-inventory test invocations into the real test main() and removed them from the fake NVRAM begin callback. The policy implementation, test bodies and source checker are unchanged. Re-run the exact-head libdisk workflow before treating this slice as validated.
- No other test result is inferred from the failed run; the exact-head AI handoff workflow on the source SHA passed.

---
## 2026-11-22 — Read libdisk account inventory through a checked NVRAM transaction

- Parent main SHA: 9ab6be48e197346138804d954433ffb956e507cf. get_account_list() relied on nvram_get_int("acc_num") and nvram_safe_get("acc_usernameN"); both collapse a backend/close failure into zero or empty text. During transient NVRAM faults, account management could mistake an unreadable inventory for an empty one.
- Added `disk_share_account_nvram_policy.h` using the existing checked count/copy helpers and a begin/get/end NVRAM transaction. Count parsing is strict and capacity-bounded; the router HTTP username and each account name are copied before the shared NVRAM cache is reused. The policy rejects malformed/sparse/duplicate/reserved inventories and discards partial results when any read or transaction close fails. Missing `acc_num` still maps to an empty inventory.
- `get_account_list()` now delegates to the policy through transaction adapters; the Samba configuration caller does not hold an outer NVRAM transaction, avoiding nested acquisition. The host workflow includes the shared policy dependency path and compiles fake-backend regressions for count/read/end/begin errors, malformed range, sparse and reserved names, duplicate values, and clean snapshot results.
- Parent SHA `3b52653213f3e9e3f0580944d0a59d8024430bec`: Process #45020631231 PASS, HTTPD #45020631208 PASS, AI handoff #45020631277 PASS; NVRAM audit #45020631251 and XY-C1 #45020631232 were in progress. Libdisk #45020631266 failed only in Python source contract because an obsolete duplicate mod_account assertion expected a direct errno expression; both sanitizer binaries passed. That stale block was removed in child checker repair #9ab6be48e197346138804d954433ffb956e507cf; exact-head results must be inspected.
- Exact-head Actions for THIS COMMIT are required before claiming the new fake-backend tests PASS. XY-C1 L3 remains unverified until Build, TRX and artifact upload pass on one exact product SHA; hardware L4 remains NOT VERIFIED.
- Next task: select a bounded WAN/service lifecycle defect outside libdisk and add a focused regression; do not reopen the account inventory slice unless its exact-head test finds a defect.

---
## 2026-11-22 — Remove stale duplicate mod-account checker contract

- Parent main SHA: 3b52653213f3e9e3f0580944d0a59d8024430bec. Exact-head libdisk run #45020631266: both GNU89 ASan/UBSan binaries PASS; Python source-contract step FAIL. The newly hardened mod_account() now checks old/new paths, source/target metadata types, checked disk inventory, and uses disk_share_account_rename_status() to normalize ENOENT/error outcomes. An older duplicate checker block still expected the pre-refactor direct errno!=ENOENT expression and rejected the valid policy-helper implementation.
- Removed only that stale duplicate assertion block. The subsequent comprehensive mod_account source contract remains: it requires read_disk_data_checked(), bounded paths, regular-file source, destination collision rejection, rename-result policy, cleanup, service reload, and error propagation. Existing GNU89 rename-policy regressions remain enabled.
- No production behavior or test assertion was weakened. The failure was a duplicate obsolete structural assumption; exact-head libdisk and handoff Actions must be rechecked on this checker-fix SHA.
- After the checker passes, continue with the previously identified get_account_list() defect: unsafe nvram_get_int()/nvram_safe_get() collapse backend read/close errors into zero/empty account state. Replace with one checked NVRAM transaction and fake-backend host regression.

---
## 2026-11-22 — Fail closed on incomplete mod-account inventory and metadata collisions

- Parent main SHA: `074bc11b3338917d1472e5146b31f9c0c59cca25`.
- `mod_account()` used legacy `read_disk_data()`, which returns NULL both when there are no disks and when the storage inventory cannot be read completely. After the account name was committed to NVRAM, this could silently skip all per-volume metadata migration and return success. It now uses `read_disk_data_checked()` and reports inventory failure after service reload.
- The per-volume rename used plain `rename(old,new)`, which can replace an existing destination metadata file. It now validates the old metadata is regular, treats only ENOENT as an absent source, checks that the new path does not already exist (EEXIST rather than clobbering stale permissions), and preserves the first path/stat/rename error while continuing across volumes.
- NVRAM-before-filesystem ordering is unchanged. A migration failure may leave NVRAM updated while some per-volume metadata remains under the old name; no rollback is claimed. The destination pre-check prevents ordinary stale-file clobbering but is not a kernel-atomic no-replace operation against an external race.
- Added GNU89 policy regressions for rename success, ENOENT idempotency, real errno preservation and EIO normalization, plus source contracts for checked inventory, bounded paths, target collision checks and cleanup/service ordering.
- Parent exact-head #45020129047 AI handoff, #45020129064 libdisk, #45020129138 Process and #45020129049 HTTPD were PASS. #45020129046 NVRAM audit was still scanning ioctl/API callers; #45020129134 XY-C1 Build was running, with TRX and artifact upload pending. These ancestor results do not validate this commit.
- L3 remains NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on this exact product/source SHA. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 20:23 CST (UTC+8) — Bound mod-account metadata renames and propagate failures

- Parent main SHA: 5f00e014e746562927ffe73b75db82333c170300. mod_account() changed the NVRAM username/password first, then formatted old/new per-volume metadata paths with unchecked snprintf() and ignored every rename() result. Long mount paths could silently target truncated paths; failed renames were reported as success.
- Both old and new metadata paths now use disk_share_format_account_var_path(). A missing old metadata file is treated as no volume-specific record; path construction and all other rename failures are aggregated while processing every mounted volume. Disk inventory is freed and ftpsamba is reloaded before the first error is returned.
- Password-only and same-name behavior are preserved, and the existing NVRAM commit ordering remains unchanged. No claim is made that a post-commit volume rename failure rolls NVRAM back.
- Added a source-contract regression for bounded old/new paths, ENOENT-only skip behavior, full-volume processing and cleanup/service/error ordering.
- Exact parent libdisk #45019979218, AI handoff #45019979195, HTTPD #45019979197, Process inventory #45019979224, NVRAM audit #45019979154 and XY-C1 #45019979256 were in progress at the last poll. No result is inferred from pending checks.
- Next bounded task: inspect the remaining libdisk account bootstrap/removal call sites and NVRAM error-return semantics, then select one independent source-level stability defect outside libdisk (WAN/service lifecycle) to broaden coverage.

---
## 2026-11-22 20:18 CST (UTC+8) — Fix del-account inventory leak and per-volume metadata cleanup

- Parent main SHA: 0a7581f2ef8ad2a04d00fa827626ae28fd85a339. del_account() decremented acc_num before passing it to free_2_dimension_list(), leaking the final owned account string. It also built per-volume metadata paths with unchecked snprintf() and ignored unlink() failures.
- Preserve the original account_count for full-list cleanup while using the decremented acc_num for NVRAM compaction. Even the zero-account/error path now releases the returned inventory.
- Per-volume metadata deletion now uses disk_share_format_account_var_path(), ignores only ENOENT, and records the first other path/unlink error while continuing through every mounted partition. The disk inventory is freed and ftpsamba is reloaded before the first cleanup error is returned.
- Added source-contract regressions for original-count freeing, checked path construction, ENOENT-only behavior, all-volume processing, cleanup ordering and first-error reporting. NVRAM mutation ordering is unchanged.
- Exact parent Actions on `0a7581f2ef8ad2a04d00fa827626ae28fd85a339`: libdisk #45019873074 PASS; AI handoff #45019873116 PASS. The earlier 9deb94c source/checker run failed due to multiline source lookup in the checker; that was corrected in 0a7581f and its exact-head libdisk run passed.
- New commit exact-head Actions must be inspected after publication. XY-C1 L3 remains NOT VERIFIED until Build, TRX and artifact upload all pass on a single product-source SHA; hardware L4 NOT VERIFIED.
- Next bounded task: audit mod_account() for unchecked metadata path construction and ignored rename failures while preserving the NVRAM transaction order.

---
## 2026-11-22 20:14 CST (UTC+8) — Normalize the remaining libdisk lifecycle contracts

- Parent main SHA: 9deb94ca8a41f17d782fb3626a41675fd91236e6. The new folder-cleanup contract normalized source calls but still compared whitespace-bearing first-error literals against the whitespace-free source. Corrected both first-error predicates to their compact forms.
- The newly added add_account() contract also needed to normalize its multi-line initializer call and cleanup sequence before checking; it now validates the exact checked call, per-volume error gate, free_disk_data(), Samba reload and final first-error return without assuming line wrapping.
- Exact parent Actions: Process inventory #45019778710, HTTPD #45019778715, XY-C1 #45019778714, libdisk #45019778723, handoff #45019778755 and NVRAM audit #45019778700 were in progress at the last poll. The previous parent checker run #45019713462 failed the `first_error` string formatting contract; both sanitizer binaries passed.
- Exact-head regression validation must be re-run on this checker-only commit. Next product task remains del_account() metadata unlink error propagation and account-list allocation cleanup.

---
## 2026-11-22 20:10 CST (UTC+8) — Propagate add-account volume metadata initialization errors

- Parent main SHA: a6908c5279bee5071759f8b45da3d1a75f27fd6d. add_account() committed the account to NVRAM, iterated mounted volumes, but ignored every initial_one_var_file_in_mount_path() return value and always reported success. A failed disk write could leave the new account without its permission metadata while the UI/API saw a successful add.
- It now records the first initialization errno (falling back to EIO), continues initializing every independent mounted volume, frees the disk inventory, reloads Samba, and then returns failure when any volume could not be initialized. The established NVRAM commit ordering and no-mounted-disk behavior are unchanged; this does not claim an NVRAM rollback.
- Added an exact-source regression contract requiring the initialization result to be checked, all volume attempts to finish, cleanup and Samba reload to occur, and the first error to be returned.
- Previous exact-head handoff Actions: #45019602967 PASS on a6908c5; libdisk #45019603097 failed because the expected call literal retained whitespace after the actual source was compacted. The follow-up checker correction is included in parent a6908c; its new exact-head libdisk run #45019713462 was QUEUED at last inspection.
- Exact-head Actions for THIS COMMIT must be fetched after publication. XY-C1 L3 remains unverified until Build, TRX and artifact upload all pass on one identical product-source SHA; hardware L4 remains NOT VERIFIED.
- Next bounded task: fix del_account()'s account-list length leak and checked per-volume metadata unlink/path handling without silently leaving cleanup failures unreported.

---
