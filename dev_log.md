## 2026-11-22 — Repair checked modem identity workflow compiler command

- Parent main SHA: `f1327a7988b9552c2ce3294c641d27838efd9664`; source/API commit under test: `f1327a7988b9552c2ce3294c641d27838efd9664`.
- Exact USB modem identity run #44969481834 failed in the added parser compile step because over-escaped line continuations passed backslash arguments to the linker. Earlier host policy steps passed. Flattened the GNU89/ASan compiler invocation to one shell line; production source and parser tests are unchanged.
- XY-C1 run #44969481753 remained in progress on f1327 at last poll. Exact rerun and Build/TRX/artifact results remain required.

---

## 2026-11-22 — Add checked USB modem WAN identity reader

- Parent main SHA: `c6bda35a8e872f040534ef67069e3f30af467928`.
- Added `get_usb_modem_wan_status(unit, &is_modem)` backed by `nvram_get_checked()`; missing/empty/non-numeric values preserve the legacy zero default, numeric prefixes preserve `atoi()` behavior, while backend errors and integer overflow return errno-style failures. The old boolean `get_usb_modem_wan()` remains a compatibility wrapper.
- Extended the shared policy header and host regression for missing key, zero/nonzero and negative values, whitespace/numeric prefixes, malformed input, backend errors, unexpected API results, null output and integer overflow.
- Extended USB modem identity Actions to compile/run the parser regression under GNU89 + ASan/UBSan, include shared paths in push/PR filters and verify the new API contract. Existing lifecycle callsites remain for the next atomic migration slice.
- Exact-head USB modem identity and XY-C1 checks are pending.

---

## 2026-11-22 — Correct the part-024 archive byte count

- Parent main SHA: `8dd57dffb63cfc11dc0459e7afb3b6d02c1ed9aa`.
- Exact-head WAN route regression #44968740776 passed on `09232ef0e8677c59bfa49a538fb908ce31259d0f`. The AI handoff check first caught the 120-entry root log; the subsequent rotation commit `8dd57dff` archived 100 records into part-024 but its index recorded JavaScript character count (129192) rather than UTF-8 byte count (129424).
- Re-read the immutable archive: 100 records, first `Snapshot VPN remote-LAN route ACL before route mutation`, last `Fail closed on WAN startup protocol reads`, blob SHA unchanged `359eae06c5b550cf68dc48710a91ea546551ec8f`. Corrected only the index byte metadata; no archived/root log records changed.
- Exact-head AI docs run #44968969967 failed solely on that byte-count mismatch; rerun required on this correction SHA. The prior product SHA `02857320194ebdda3b451ea3d6de07fa212995c0` has XY-C1 Build, TRX verification and artifact upload steps 37/38/39 all PASS in run #44968037242.
- Next source slice remains the checked USB-modem WAN selector: `wanN_modem_dev` currently has a lossy `nvram_get_int()` reader.

---

## 2026-11-22 — Preserve WAN protocol error in DHCP deconfig regression contract

- Parent main SHA: `8998530e9609b5e401c15a3a63f23b20ebd27d60`.
- Exact-head WAN route run #44968619661 failed after all compiled policy regressions and earlier source contracts passed. Embedded Python line 654 still expected the old `interface_status → conntrack_status → cleanup_status` return chain and omitted the already-implemented `proto_status` first-error preservation in `udhcpc_deconfig()`.
- Verified the current `trunk/user/rc/src/net_wan.c` implementation reads the MAN protocol with `get_wan_unit_proto_checked_status()`, preserves the underlay address on read failure, continues independent teardown, and returns the accumulated protocol/interface/conntrack/cleanup status. Updated the WAN route contract to require that complete aggregation; no production source changed.
- AI handoff documentation check #44968619684 passed on the parent SHA. Exact-head WAN route rerun and this commit's documentation check are pending; XY-C1 validation remains tracked against product-source SHA `02857320194ebdda3b451ea3d6de07fa212995c0`.
- Next source slice: add a status-returning USB modem identity reader and migrate lifecycle-critical consumers without treating NVRAM backend errors as “not a modem”.

---

## 2026-11-22 — Update WAN route contract for checked protocol lookup

- Parent main SHA: `8ed832e39248e1ac0011dccfd247fba0414bac30`.
- Exact `8ed832e39248e1ac0011dccfd247fba0414bac30` WAN route run #44968296991 failed at embedded checker Python line 465. The checker still located `wan_proto = get_wan_proto(unit);` after the VPN-stop notification, but WAN teardown now uses `proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);`. Updated the contract to require the checked call and explicitly forbid the legacy getter in `wan_down_internal()`.
- On that same SHA, HTTPD authentication/disclosure policy and AI handoff checks passed. This change is checker-only.
- The next source task is an unchecked modem identity read: legacy `get_usb_modem_wan()` collapses an NVRAM backend read error into zero, potentially treating a modem as non-modem in lifecycle paths. A status-returning migration is queued as the next source slice.

---

## 2026-11-22 — Align WAN route and HTTPD source contracts with retained status

- Parent main SHA: `02857320194ebdda3b451ea3d6de07fa212995c0`; source under test: `02857320194ebdda3b451ea3d6de07fa212995c0`.
- Exact WAN route #44968037254 reached its cleanup contract and failed because the workflow expected the old `udhcpc_deconfig()` return expression that omitted `proto_status`. Production source deliberately accumulates protocol-read failure, interface reset, conntrack flush and WAN/MAN cleanup; checker now validates the complete nested first-error accumulation.
- Exact HTTPD security #44968037240 failed because its lifecycle check expected the pre-change combined stop/final interface cleanup gate. It now checks that callback cleanup precedes `wan_teardown_first_error_status()`, and that the resulting failure gate precedes clearing WAN state.
- On SHA `0285732...`, WAN teardown, DHCP callback, USB modem identity and handoff checks all passed. XY-C1 build remained in progress at the last poll; the previous build failure had been the four illegal assignments removed in `0285732...`.
- This is a test-contract alignment slice; no production source changes in this commit.

---

## 2026-11-22 — Fix XY-C1 WAN reset compile blocker and refresh source contracts

- Parent main SHA: `6cdaaccda83823f252dc537e4d9bfd358bfe845e`; product-source target was `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact XY-C1 run #44967290950 completed FAILURE. The production build reached `net_wan.c` and reported four errors in `reset_wan_vars_status()`: assigning string literals to fixed-size arrays `man_addr`, `man_mask`, `man_gate`, and `man_mtu`. Those buffers already have valid initializers; removing the redundant assignments fixes the C type error without changing defaults or the manual endpoint snapshot.
- Exact WAN route #44967576615 failed at the stale static-IPoE teardown parser in `check_wan_detect_wan_status.py`; its anchor is updated to match the checked-protocol condition and an array-initialization regression was added.
- Teardown run #44967575277 on `6cdaacc...` had zero jobs; inspection found two Python lines de-indented outside the YAML `run: |` block. Their indentation is repaired.
- Handoff validator #44967576593 passed. XY-C1 TRX and artifact steps were skipped after the compile failure; no L3 claim is made. A fresh exact-head firmware run remains mandatory.

---

## 2026-11-22 — Fix WAN IPv4 and teardown source-contract locators; refresh handoff

- Parent main SHA: `4c1f1be93e4c0436d0b20a3b4ddfbb314d07914c`; product source remains `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact checker SHA `4c1f1be93e4c0436d0b20a3b4ddfbb314d07914c`: WAN route #44967410609 failed only because the IPv4 lookup contract expected a brace-less error return; the source uses a braced checked-protocol error path. WAN teardown #44967410572 reached its final ordering assertion but located an earlier generic stop-status gate. Both source-contract locators are corrected without relaxing production behavior.
- AI handoff #44967410603 failed because a prior docs checkpoint renamed the required first-action heading. Restored the exact protocol heading and updated the handoff with product SHA, checker SHA, actual run states, the frozen backup and the first next action.
- WAN DHCP #44967410597 PASS on checker SHA `4c1f1be...`. The previous firmware run for product SHA `4a6df662...` remained in progress at last poll; L3 is not claimed.
- Next: verify exact-head route/teardown/handoff reruns, inspect XY-C1 Build/TRX/artifact, then continue checked modem identity audit.

---

## 2026-11-22 — Repair exact-head checked-protocol regression contracts

- Parent main SHA: `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact-head `4a6df662b6c34d2282e81b3ef8054e80e0894393` completed with three actionable workflow issues: WAN route checker still expected the removed `rc_first_service_start_status(stop_status, proto_status)`; WAN teardown workflow still located the old combined error gate rather than the new first-error aggregator; the DHCP callback embedded Python used an undefined `require()` helper.
- Updated those source contracts to verify protocol-error aggregation after callback cleanup and switched the inline DHCP check to a standard assertion. These are CI/test harness corrections; the production source slice remains unchanged in this commit.
- The WAN route, WAN DHCP callback and WAN teardown Actions on SHA 4a6df66 failed at the above contract/assertion issues. The host C policy builds preceding the WAN teardown Python contract succeeded. XY-C1 Build/TRX/artifact and the independent repository inventory remain under exact-head verification at this checkpoint; no unobserved result is treated as PASS.
- Next: inspect exact-SHA reruns for these three contracts and the WAN DHCP source gate; then review any compiler/build failure or other independent regression.

---

## 2026-11-22 — Fail closed on remaining unchecked WAN protocol consumers

- Parent main SHA: `d1108616599c54ff5268c780b9119d276b1e82fb`.
- The live source audit found four remaining unchecked `get_wan_proto(unit)` consumers in `add_man_gateway_routes()`, `man_down_internal()`, `wan_up_internal()` and `udhcpc_bound()`. They controlled whether MAN DNS routes were added/removed, which WAN route/address class to initialize, and which DHCP lease namespace was mutated.
- THIS source slice replaces those protocol guesses with `get_wan_unit_proto_checked_status()`: MAN-up continues essential gateway routes but skips optional DNS routes on an unknown protocol; MAN-down still attempts idempotent DNS cleanup while retaining the read error; WAN-up and non-modem DHCP bound refuse protocol-dependent changes before runtime-state/interface mutation. Confirmed modem callbacks retain their transport-specific path.
- A second teardown defect was fixed: `stop_wan_locked()` previously stored a checked protocol error in `stop_status`, then overwrote it with successful VPN/PPP stop results. Final teardown now aggregates the protocol status after independent interface/callback cleanup and refuses final WAN-state clear if it remains nonzero.
- Added executable C policy regressions for conservative MAN DNS route add/remove and preservation of the first protocol teardown error, plus source-contract checks for callback ordering and fail-closed behavior. Exact-SHA Actions are required for validation; no local firmware toolchain is available here.
- Next: inspect exact-head WAN route, DHCP callback, teardown and XY-C1 runs; address any true source/test failures before expanding the audit.

---

## 2026-11-22 08:32 CST — Exact-head WAN protocol hardening checkpoint

- Product source head at inspection: `6a7639cc946d9277a4a54db6c695bd816c472761`; `net_wan.c` blob: `d1315536f781f632aa2c1839eefb01619614b86d`.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head Actions for this SHA: WAN route operation status regression: #44966882221 completed/failure; WAN DHCP callback state stability regression: #44966882242 completed/success; XY-C1 Firmware Validation: #44966882256 completed/success; AI handoff documentation contract: #44966882224 completed/success. A missing workflow is not a PASS; in-progress jobs remain pending.
- WAN-protocol call-site scan still finds 5 direct `get_wan_proto()` occurrences in `net_wan.c`; next inspect `udhcpc_bound()` before changing lease publication, ensuring old lease-route cleanup is not skipped on read failure.
- Firmware L3 remains unverified unless Build XY-C1, TRX verification and artifact upload all pass on one product source SHA. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Align WAN source contracts with checked protocol reads

- Parent main SHA: `d004901f403430f32fd73ea950ed947bc287a6f1`.
- Exact-head WAN route #44966559531 reached the WAN DNS/interface source contract after its host regressions passed, then failed because the checker expected a brace-less error block while `get_wan_ifname()` correctly uses a braced checked-read gate. WAN DHCP callback #44966559464 and the WAN teardown contract #44966559612 likewise reached stale assertions expecting the old unchecked `get_wan_proto()` assignment after `wan_down_internal()` and `stop_wan_locked()` had migrated to the checked reader. Their preceding host regression binaries passed.
- XY-C1 #44966559594 reached the PPPoE-MWAN lifecycle contract and stopped at the same stale WAN-down getter anchor; the preceding detector and multi-WAN host tests passed. Firmware input detection/Build/TRX/artifact were not reached on that SHA.
- THIS checker-only slice updates the four affected contracts to require the actual checked protocol read, its error gate/status aggregation, and ordering before independent IPv4 cleanup. No product source changed.
- Exact-head reruns are required before claiming these checks pass. The same-source WAN-route/DHCP/teardown and XY-C1 results remain pending.
- Next: inspect exact-head reruns; continue the live audit of `udhcpc_bound()`, `add_man_gateway_routes()`, `man_down_internal()` and other remaining direct protocol consumers, preserving best-effort teardown.

---

## 2026-11-22 08:25 CST — Preserve cleanup while failing closed on WAN teardown protocol errors

- Parent main SHA: `14791ee073da672bb0a66c2ac618b786e9b57890`.
- Audit found two remaining unchecked protocol consumers in destructive teardown. `stop_wan_locked()` could skip its WAN-down route cleanup when `get_wan_proto()` failed. `wan_down_internal()` could skip lease-derived DHCP route removal and defensive Kabinet auth shutdown, while also risking a wrong DNS or static-address decision.
- THIS source change uses the checked per-unit protocol reader in both functions. Stop retains the protocol error as its first teardown status and invokes conservative `wan_down()` cleanup for an unknown protocol; subsequent independent interface and service teardown continues.
- `wan_down_internal()` now attempts lease-route prefix removal and defensive auth shutdown when non-modem protocol state is unreadable, propagates that protocol error, and refuses to clear indexed WAN DNS or a possible MAN underlay IPv4 address based on unknown protocol. Default/gateway routes, static routes, resolver, conntrack, statistics and post-down cleanup remain independent of this read.
- Added source-contract assertions to `check_wan_dns_read_status.py` for both checked reads, fail-safe lease/auth cleanup, DNS/IP preservation, and route/resolver/conntrack ordering. The WAN route workflow already filters both modified paths for push and PR.
- No local checkout or firmware toolchain is available; this source commit requires exact-SHA GNU89/ASan/UBSan Actions and XY-C1 firmware validation. The preceding checker-only descendant `14791ee073da672bb0a66c2ac618b786e9b57890` still needs its exact WAN route result recorded.
- Next: inspect exact-head WAN route and DHCP callback/XY-C1 runs; audit remaining direct `get_wan_proto()` in `udhcpc_bound()` and `add_man_gateway_routes()` for stale lease/gateway route decisions on unknown protocol.

---

## 2026-11-22 08:22 CST — Narrow WAN interface protocol contract to the unsafe getter

- Parent main SHA: `0ae742a0e277c75dd5b335f4a62098da5ab6c030`.
- Exact WAN route #44966179307 failed at the newly added `get_wan_ifname()` source assertion. The implementation contains the checked protocol read and returns its status before publishing the interface name; the assertion's broad substring check for `get_wan_proto(unit)` also matches unrelated text in the extracted function body and is redundant with the narrower obsolete assignment check.
- THIS checker-only fix narrows the stale-getter prohibition to `wan_proto=get_wan_proto(unit)`, while retaining requirements for the checked reader and immediate error return. No product source changed.
- On the same SHA, WAN DHCP callback #44966179303 and AI handoff contract #44966179398 completed successfully. WAN route host regressions before this one source-contract assertion all passed.
- Next: rerun WAN route on this checker descendant, then resume `stop_wan_locked()` / `wan_down_internal()` teardown audit. Firmware L3/L4 remains unverified until same-SHA target Build/TRX/artifact and hardware validation exist.

---

## 2026-11-22 08:20 CST — Repair WAN protocol source-check syntax and DHCP deconfig contract

- Parent main SHA: `c2fcb3bb5375637cf0ff9a48cc5cd47eb7b62514`.
- Exact WAN route #44965962990 failed in `check_wan_dns_read_status.py` because a newly added source-contract assertion used an over-escaped quote sequence in a Python string literal. THIS fix shortens the ordering anchor to `ifconfig(wan_ifname,IFUP,`, which still proves the checked protocol read precedes address clearing.
- Exact WAN DHCP callback run #44965963007 failed its previous return-contract assertion: `udhcpc_deconfig()` now correctly folds `proto_status` into the first-error accumulator, but the older workflow assertion still expected the pre-change two-status expression. THIS workflow-only adjustment now requires the nested `proto_status` → interface → conntrack → route-cleanup aggregation.
- All other WAN callback test stages before these assertions passed. XY-C1 #44965962988 was still in progress at this checkpoint; target Build/TRX/artifact outcomes must be verified separately on the exact source SHA.
- Next: check exact-head WAN route, DHCP callback and XY-C1 reruns; then continue the `stop_wan_locked()` / `wan_down_internal()` cleanup audit. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 08:18 CST — Preserve WAN identity and MAN underlay on protocol-read errors

- Parent main SHA: `e29f9cd724cd52d1f501ec48425e4f96fd080587`.
- `get_wan_dns_static_status()`, `get_wan_ifname()`, `get_wan_ip4()` and Ethernet link-restored DHCP notification still consumed unchecked `get_wan_proto()`. Read failures were silently interpreted as non-static DNS, a MAN interface, MAN IPv4 or a DHCP-capable WAN.
- THIS source change uses `get_wan_unit_proto_checked_status()` before each protocol-dependent decision: DNS mode returns the protocol error rather than entering the legacy `dnsenable_x` fallback; interface lookup returns the error rather than guessing a MAN/PPP identity; WAN-IP lookup returns `INADDR_ANY` with errno preserved; link restoration refuses to signal DHCP renewal when protocol state is unknown.
- `udhcpc_deconfig()` previously cleared a MAN interface to `0.0.0.0` whenever its protocol read failed, even though `is_ifunit_man()` conservatively classifies unknown protocol as MAN. It now retains the MAN underlay address on protocol error, while still running `man_down_internal()` route teardown and aggregating protocol, interface, conntrack and cleanup status.
- Extended `check_wan_dns_read_status.py` to pin the checked DNS/interface/IP/link-restored/deconfig paths and cleanup ordering. Both changed paths are already included in WAN route workflow push/PR filters.
- The preceding checker-only fix `e29f9cd724cd52d1f501ec48425e4f96fd080587` has exact-head WAN route #44965890151 and XY-C1 #44965890114 in progress at the time of this commit. No local full checkout/toolchain is available; executable host tests and target-build results must come from exact-SHA Actions.
- Next: inspect exact-head WAN route and XY-C1; audit `stop_wan_locked()` / `wan_down_internal()` for protocol-error static-address/DHCP-route/auth cleanup gaps without bypassing mandatory teardown.

---

## 2026-11-22 08:17 CST — Scope MAN classifier source contract to its definition

- Parent main SHA: `dc5df775b1ec1043b5f4c1ce42f472533f886682`.
- Exact WAN-route #44965681798 failed after earlier WAN tests passed because the final manual-PPP assertion called `body(wan, "is_ifunit_man(")`. A comment in the source contains that token before the definition, so the generic extractor selected the following unrelated function body instead of `is_ifunit_man()`.
- Exact XY-C1 #44965681825 also failed at this WAN checker. Its job log additionally reported an independent Samba contract extraction error (`unterminated function: run_samba`); this checker-only fix does not alter Samba code and that failure remains to diagnose separately.
- THIS checker-only change scopes the extraction to the exact definition signature `static int\nis_ifunit_man(char *wan_ifname, int unit)`, matching the already-correct earlier assertion. The negative protocol sentinel and explicit true classification remain required.
- Next: rerun exact-head WAN route/XY-C1; inspect the Samba checker failure independently. Then proceed with status-aware interface/DHCP protocol changes; hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 08:14 CST — Align manual-PPP checker with explicit error sentinel

- Parent main SHA: `76ddd91f39790262b8a373977d1a3b44f119a638`.
- Exact-head WAN route #44965482671 failed at `scripts/check_wan_firewall_status.py`: an earlier assertion still required `get_wan_unit_proto_for_manual_check()` to return `IPV4_WAN_PROTO_PPTP`, while the newer implementation deliberately returns `-1` and makes `is_ifunit_man()` explicitly return true for any negative protocol sentinel.
- The same checker later correctly required `return -1;` and `if (wan_proto < 0) return 1;`; the duplicate stale expectation made it internally contradictory. THIS checker-only commit changes the earlier assertion to require the explicit error sentinel while retaining the boolean underlay-protection assertion.
- All WAN route host regressions before the firewall contract passed. XY-C1 #44965482665 stopped at the same checker failure, so firmware-input detection, Build XY-C1, TRX and artifact upload were skipped. This is not a compiler failure.
- Next: inspect exact-head WAN route rerun after this checker repair, then continue status-bearing WAN interface selection and DHCP callback protocol reads; hardware L4 remains NOT VERIFIED.

---

## 2026-11-21 — Make manual-PPP protocol failure sentinel explicit

- Parent main SHA: `58903be39199a36dce04f1f87d43561c26e04446`.
- The previous checked wrapper returned PPTP as a conservative stand-in on read failure. That relies on downstream classifier branches treating PPTP as manual without consulting a PPTP-specific selector, which is less explicit than the intended safety property.
- THIS commit makes the wrapper return `-1` on read failure and makes the boolean `is_ifunit_man()` immediately return `true` for that sentinel. Thus an unavailable protocol never gets mistaken for a known protocol while the legacy bool ABI still protects the underlay.
- Added source-contract assertions for the negative sentinel and explicit `return 1` behavior. Exact-head Actions pending.
- Next: inspect the WAN route/NVRAM/Process/XY-C1 results and continue remaining route-gateway/teardown/DHCP reads without bypassing unconditional cleanup.

---

## 2026-11-22 08:10 CST — Fail closed on WAN protocol errors in disconnect and xl2tpd setup

- Parent main SHA: `bbc0dad3aa568ed4885e96cd6022b501fad6f609`.
- Continued the shared `get_wan_proto()` audit. Two concrete error fallthroughs remained: `manual_wan_disconnect()` mapped a failed non-modem protocol read to generic `stop_wan()`; `safe_start_xl2tpd_locked()` mapped an unreadable protocol to “not L2TP” and could rewrite `xl2tpd.conf` without the active WAN LAC section.
- THIS source commit exports the existing bounded checked protocol mapper through `rc.h`; manual disconnect checks protocol only for non-USB WANs and retains USB modem shutdown selected by checked transport type; xl2tpd refuses to rewrite its config on missing, unknown or backend-failed WAN protocol.
- Extended existing WAN DNS and PPP source contracts to pin error propagation, USB/non-USB selection and the protocol gate before `fopen()` truncates the live config. Workflow push and pull_request filters already include all changed sources/checkers.
- The preceding MTU-checker repair `bbc0dad3aa568ed4885e96cd6022b501fad6f609` passed WAN route #44965084237 and AI handoff #44965084235. No local full checkout/toolchain is available here; targeted source preflight confirmed branches, prototypes, checker assertions and workflow path coverage. XY-C1 firmware Build/TRX/artifact was not triggered by the checker-only repair.
- Next: inspect exact-SHA WAN route GNU89/ASan/UBSan and XY-C1 Build/TRX/artifact for THIS source change; continue `get_wan_proto()` audit in `get_wan_ifname()`, `get_wan_ip4()` and `udhcpc_deconfig()`, preserving unconditional lease/route/DNS/conntrack cleanup.

---

## 2026-11-22 08:07 CST — Update initial-MTU contract for checked WAN protocol acquisition

- Parent main SHA: `f1fc3d9d03ed18b6f432a5b1591b8ce2d4a031ab`.
- Exact WAN route run #44964927034 failed after the firewall status contract passed. The WAN initial-MTU checker raised `ValueError: substring not found` because it still located `wan_proto = get_wan_proto(unit);` inside `start_wan()`; the startup protocol read had already migrated to the checked status-bearing helper.
- THIS checker-only fix anchors MTU-order validation to `nvram_status = get_wan_unit_proto_checked_status(unit, &wan_proto);`, then retains all existing assertions that the checked MTU read/failure gate precede default MTU configuration and interface bring-up.
- Exact-source preflight confirmed the checked protocol assignment occurs before `get_wan_unit_value_int_status()`. No product source changed in this repair.
- Exact WAN route/XY-C1 runs for the previous source SHA were still underway at inspection; this descendant's exact-head reruns remain pending.
- Next: inspect exact-head WAN route and XY-C1; continue direct protocol-reader auditing for WAN interface selection, manual disconnect and xl2tpd. L3/L4 remain unverified until same-SHA Build/TRX/artifact and hardware evidence exist.

---

## 2026-11-21 — Preserve manual PPP underlay on protocol-read errors

- Parent main SHA: `506a91e599331c58caf6582b9de6075c472455da`.
- `is_ifunit_man()` is a boolean helper used by WAN callbacks. Its old `get_wan_proto()` call collapsed backend/missing/unknown errors to `-1`, which could be interpreted as “not manual” and route DHCP cleanup through an unknown PPP underlay.
- THIS source slice adds `get_wan_unit_proto_for_manual_check()`, using the checked protocol reader. On error it logs the status and conservatively returns PPTP classification so legacy boolean callers preserve the PPP underlay; valid protocols retain their exact mapping.
- It also includes the checked `start_wan()` protocol/first-error fix if that was not already present in the parent.
- `check_wan_firewall_status.py` statically pins conservative behavior. Source-level preflight passed; exact-head WAN and XY-C1 Actions pending.
- Next: examine remaining direct `get_wan_proto()` uses in teardown, gateway routes and DHCP callbacks; keep mandatory cleanup unconditional on protocol read errors.

---

## 2026-11-22 08:04 CST — Repair WAN startup checker to scope the matching error gate

- Parent main SHA: `33258a680e052c9334e64758b70f7dce3754becb`.
- Exact push WAN-route run #44964696301 and XY-C1 run #44964696249 both stopped at the dependent `scripts/check_wan_firewall_status.py` contract. All 27 preceding WAN-route host regressions and the WAN VLAN/bridge contract passed; the checker then failed its combined startup assertion. XY-C1 likewise stopped at the same contract before firmware-input detection, Build, TRX or artifact upload. This was not a compiler result.
- The defect was in the static contract: `start_wan()` also contains an earlier `nvram_status != 0` branch for the SYN-cookie security preflight. The checker used the first such branch while validating the later WAN-protocol read and compared unrelated gates.
- THIS checker-only commit scopes the error-gate search to the position immediately after the checked protocol read. It retains the first-error aggregation, diagnostic, and interface-lookup ordering requirements.
- Targeted preflight against the exact fetched `start_wan()` source confirmed the protocol read, following error gate, first-error aggregation statement, diagnostic, and gate-before-interface-lookup ordering. No product source changed in this repair.
- Next: inspect exact-head WAN route and XY-C1 reruns. Require WAN route PASS and all firmware stages on one exact source SHA before claiming L3; hardware L4 remains NOT VERIFIED.

---

