## 2026-11-22 08:04 CST — Repair WAN startup checker to scope the matching error gate

- Parent main SHA: `33258a680e052c9334e64758b70f7dce3754becb`.
- Exact push WAN-route run #44964696301 and XY-C1 run #44964696249 both stopped at the dependent `scripts/check_wan_firewall_status.py` contract. All 27 preceding WAN-route host regressions and the WAN VLAN/bridge contract passed; the checker then failed its combined startup assertion. XY-C1 likewise stopped at the same contract before firmware-input detection, Build, TRX or artifact upload. This was not a compiler result.
- The defect was in the static contract: `start_wan()` also contains an earlier `nvram_status != 0` branch for the SYN-cookie security preflight. The checker used the first such branch while validating the later WAN-protocol read and compared unrelated gates.
- THIS checker-only commit scopes the error-gate search to the position immediately after the checked protocol read. It retains the first-error aggregation, diagnostic, and interface-lookup ordering requirements.
- Targeted preflight against the exact fetched `start_wan()` source confirmed the protocol read, following error gate, first-error aggregation statement, diagnostic, and gate-before-interface-lookup ordering. No product source changed in this repair.
- Next: inspect exact-head WAN route and XY-C1 reruns. Require WAN route PASS and all firmware stages on one exact source SHA before claiming L3; hardware L4 remains NOT VERIFIED.

---

## 2026-11-21 — Preserve manual PPP underlay on protocol-read errors

- Parent main SHA: `506a91e599331c58caf6582b9de6075c472455da`.
- `is_ifunit_man()` is a boolean helper used by WAN callbacks. Its old `get_wan_proto()` call collapsed backend/missing/unknown errors to `-1`, which could be interpreted as “not manual” and route DHCP cleanup through an unknown PPP underlay.
- THIS source slice adds `get_wan_unit_proto_for_manual_check()`, using the checked protocol reader. On error it logs the status and conservatively returns PPTP classification so legacy boolean callers preserve the PPP underlay; valid protocols retain their exact mapping.
- It also includes the checked `start_wan()` protocol/first-error fix if that was not already present in the parent.
- `check_wan_firewall_status.py` statically pins conservative behavior. Source-level preflight passed; exact-head WAN and XY-C1 Actions pending.
- Next: examine remaining direct `get_wan_proto()` uses in teardown, gateway routes and DHCP callbacks; keep mandatory cleanup unconditional on protocol read errors.

---

## 2026-11-22 08:07 CST — Update initial-MTU contract for checked WAN protocol acquisition

- Parent main SHA: `f1fc3d9d03ed18b6f432a5b1591b8ce2d4a031ab`.
- Exact WAN route run #44964927034 failed after the firewall status contract passed. The WAN initial-MTU checker raised `ValueError: substring not found` because it still located `wan_proto = get_wan_proto(unit);` inside `start_wan()`; the startup protocol read had already migrated to the checked status-bearing helper.
- THIS checker-only fix anchors MTU-order validation to `nvram_status = get_wan_unit_proto_checked_status(unit, &wan_proto);`, then retains all existing assertions that the checked MTU read/failure gate precede default MTU configuration and interface bring-up.
- Exact-source preflight confirmed the checked protocol assignment occurs before `get_wan_unit_value_int_status()`. No product source changed in this repair.
- Exact WAN route/XY-C1 runs for the previous source SHA were still underway at inspection; this descendant's exact-head reruns remain pending.
- Next: inspect exact-head WAN route and XY-C1; continue direct protocol-reader auditing for WAN interface selection, manual disconnect and xl2tpd. L3/L4 remain unverified until same-SHA Build/TRX/artifact and hardware evidence exist.

---

## 2026-11-22 08:10 CST — Fail closed on WAN protocol errors in disconnect and xl2tpd setup

- Parent main SHA: `bbc0dad3aa568ed4885e96cd6022b501fad6f609`.
- Continued the shared `get_wan_proto()` audit. Two concrete error fallthroughs remained: `manual_wan_disconnect()` mapped a failed non-modem protocol read to generic `stop_wan()`; `safe_start_xl2tpd_locked()` mapped an unreadable protocol to “not L2TP” and could rewrite `xl2tpd.conf` without the active WAN LAC section.
- THIS source commit exports the existing bounded checked protocol mapper through `rc.h`; manual disconnect checks protocol only for non-USB WANs and retains USB modem shutdown selected by checked transport type; xl2tpd refuses to rewrite its config on missing, unknown or backend-failed WAN protocol.
- Extended existing WAN DNS and PPP source contracts to pin error propagation, USB/non-USB selection and the protocol gate before `fopen()` truncates the live config. Workflow push and pull_request filters already include all changed sources/checkers.
- The preceding MTU-checker repair `bbc0dad3aa568ed4885e96cd6022b501fad6f609` passed WAN route #44965084237 and AI handoff #44965084235. No local full checkout/toolchain is available here; targeted source preflight confirmed branches, prototypes, checker assertions and workflow path coverage. XY-C1 firmware Build/TRX/artifact was not triggered by the checker-only repair.
- Next: inspect exact-SHA WAN route GNU89/ASan/UBSan and XY-C1 Build/TRX/artifact for THIS source change; continue `get_wan_proto()` audit in `get_wan_ifname()`, `get_wan_ip4()` and `udhcpc_deconfig()`, preserving unconditional lease/route/DNS/conntrack cleanup.

---

## 2026-11-21 — Make manual-PPP protocol failure sentinel explicit

- Parent main SHA: `58903be39199a36dce04f1f87d43561c26e04446`.
- The previous checked wrapper returned PPTP as a conservative stand-in on read failure. That relies on downstream classifier branches treating PPTP as manual without consulting a PPTP-specific selector, which is less explicit than the intended safety property.
- THIS commit makes the wrapper return `-1` on read failure and makes the boolean `is_ifunit_man()` immediately return `true` for that sentinel. Thus an unavailable protocol never gets mistaken for a known protocol while the legacy bool ABI still protects the underlay.
- Added source-contract assertions for the negative sentinel and explicit `return 1` behavior. Exact-head Actions pending.
- Next: inspect the WAN route/NVRAM/Process/XY-C1 results and continue remaining route-gateway/teardown/DHCP reads without bypassing unconditional cleanup.

---

## 2026-11-22 08:14 CST — Align manual-PPP checker with explicit error sentinel

- Parent main SHA: `76ddd91f39790262b8a373977d1a3b44f119a638`.
- Exact-head WAN route #44965482671 failed at `scripts/check_wan_firewall_status.py`: an earlier assertion still required `get_wan_unit_proto_for_manual_check()` to return `IPV4_WAN_PROTO_PPTP`, while the newer implementation deliberately returns `-1` and makes `is_ifunit_man()` explicitly return true for any negative protocol sentinel.
- The same checker later correctly required `return -1;` and `if (wan_proto < 0) return 1;`; the duplicate stale expectation made it internally contradictory. THIS checker-only commit changes the earlier assertion to require the explicit error sentinel while retaining the boolean underlay-protection assertion.
- All WAN route host regressions before the firewall contract passed. XY-C1 #44965482665 stopped at the same checker failure, so firmware-input detection, Build XY-C1, TRX and artifact upload were skipped. This is not a compiler failure.
- Next: inspect exact-head WAN route rerun after this checker repair, then continue status-bearing WAN interface selection and DHCP callback protocol reads; hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 08:17 CST — Scope MAN classifier source contract to its definition

- Parent main SHA: `dc5df775b1ec1043b5f4c1ce42f472533f886682`.
- Exact WAN-route #44965681798 failed after earlier WAN tests passed because the final manual-PPP assertion called `body(wan, "is_ifunit_man(")`. A comment in the source contains that token before the definition, so the generic extractor selected the following unrelated function body instead of `is_ifunit_man()`.
- Exact XY-C1 #44965681825 also failed at this WAN checker. Its job log additionally reported an independent Samba contract extraction error (`unterminated function: run_samba`); this checker-only fix does not alter Samba code and that failure remains to diagnose separately.
- THIS checker-only change scopes the extraction to the exact definition signature `static int\nis_ifunit_man(char *wan_ifname, int unit)`, matching the already-correct earlier assertion. The negative protocol sentinel and explicit true classification remain required.
- Next: rerun exact-head WAN route/XY-C1; inspect the Samba checker failure independently. Then proceed with status-aware interface/DHCP protocol changes; hardware L4 remains NOT VERIFIED.

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

## 2026-11-22 08:20 CST — Repair WAN protocol source-check syntax and DHCP deconfig contract

- Parent main SHA: `c2fcb3bb5375637cf0ff9a48cc5cd47eb7b62514`.
- Exact WAN route #44965962990 failed in `check_wan_dns_read_status.py` because a newly added source-contract assertion used an over-escaped quote sequence in a Python string literal. THIS fix shortens the ordering anchor to `ifconfig(wan_ifname,IFUP,`, which still proves the checked protocol read precedes address clearing.
- Exact WAN DHCP callback run #44965963007 failed its previous return-contract assertion: `udhcpc_deconfig()` now correctly folds `proto_status` into the first-error accumulator, but the older workflow assertion still expected the pre-change two-status expression. THIS workflow-only adjustment now requires the nested `proto_status` → interface → conntrack → route-cleanup aggregation.
- All other WAN callback test stages before these assertions passed. XY-C1 #44965962988 was still in progress at this checkpoint; target Build/TRX/artifact outcomes must be verified separately on the exact source SHA.
- Next: check exact-head WAN route, DHCP callback and XY-C1 reruns; then continue the `stop_wan_locked()` / `wan_down_internal()` cleanup audit. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 08:22 CST — Narrow WAN interface protocol contract to the unsafe getter

- Parent main SHA: `0ae742a0e277c75dd5b335f4a62098da5ab6c030`.
- Exact WAN route #44966179307 failed at the newly added `get_wan_ifname()` source assertion. The implementation contains the checked protocol read and returns its status before publishing the interface name; the assertion's broad substring check for `get_wan_proto(unit)` also matches unrelated text in the extracted function body and is redundant with the narrower obsolete assignment check.
- THIS checker-only fix narrows the stale-getter prohibition to `wan_proto=get_wan_proto(unit)`, while retaining requirements for the checked reader and immediate error return. No product source changed.
- On the same SHA, WAN DHCP callback #44966179303 and AI handoff contract #44966179398 completed successfully. WAN route host regressions before this one source-contract assertion all passed.
- Next: rerun WAN route on this checker descendant, then resume `stop_wan_locked()` / `wan_down_internal()` teardown audit. Firmware L3/L4 remains unverified until same-SHA target Build/TRX/artifact and hardware validation exist.

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

## 2026-11-22 — Align WAN source contracts with checked protocol reads

- Parent main SHA: `d004901f403430f32fd73ea950ed947bc287a6f1`.
- Exact-head WAN route #44966559531 reached the WAN DNS/interface source contract after its host regressions passed, then failed because the checker expected a brace-less error block while `get_wan_ifname()` correctly uses a braced checked-read gate. WAN DHCP callback #44966559464 and the WAN teardown contract #44966559612 likewise reached stale assertions expecting the old unchecked `get_wan_proto()` assignment after `wan_down_internal()` and `stop_wan_locked()` had migrated to the checked reader. Their preceding host regression binaries passed.
- XY-C1 #44966559594 reached the PPPoE-MWAN lifecycle contract and stopped at the same stale WAN-down getter anchor; the preceding detector and multi-WAN host tests passed. Firmware input detection/Build/TRX/artifact were not reached on that SHA.
- THIS checker-only slice updates the four affected contracts to require the actual checked protocol read, its error gate/status aggregation, and ordering before independent IPv4 cleanup. No product source changed.
- Exact-head reruns are required before claiming these checks pass. The same-source WAN-route/DHCP/teardown and XY-C1 results remain pending.
- Next: inspect exact-head reruns; continue the live audit of `udhcpc_bound()`, `add_man_gateway_routes()`, `man_down_internal()` and other remaining direct protocol consumers, preserving best-effort teardown.

---

## 2026-11-22 08:32 CST — Exact-head WAN protocol hardening checkpoint

- Product source head at inspection: `6a7639cc946d9277a4a54db6c695bd816c472761`; `net_wan.c` blob: `d1315536f781f632aa2c1839eefb01619614b86d`.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head Actions for this SHA: WAN route operation status regression: #44966882221 completed/failure; WAN DHCP callback state stability regression: #44966882242 completed/success; XY-C1 Firmware Validation: #44966882256 completed/success; AI handoff documentation contract: #44966882224 completed/success. A missing workflow is not a PASS; in-progress jobs remain pending.
- WAN-protocol call-site scan still finds 5 direct `get_wan_proto()` occurrences in `net_wan.c`; next inspect `udhcpc_bound()` before changing lease publication, ensuring old lease-route cleanup is not skipped on read failure.
- Firmware L3 remains unverified unless Build XY-C1, TRX verification and artifact upload all pass on one product source SHA. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Fail closed on remaining unchecked WAN protocol consumers

- Parent main SHA: `d1108616599c54ff5268c780b9119d276b1e82fb`.
- The live source audit found four remaining unchecked `get_wan_proto(unit)` consumers in `add_man_gateway_routes()`, `man_down_internal()`, `wan_up_internal()` and `udhcpc_bound()`. They controlled whether MAN DNS routes were added/removed, which WAN route/address class to initialize, and which DHCP lease namespace was mutated.
- THIS source slice replaces those protocol guesses with `get_wan_unit_proto_checked_status()`: MAN-up continues essential gateway routes but skips optional DNS routes on an unknown protocol; MAN-down still attempts idempotent DNS cleanup while retaining the read error; WAN-up and non-modem DHCP bound refuse protocol-dependent changes before runtime-state/interface mutation. Confirmed modem callbacks retain their transport-specific path.
- A second teardown defect was fixed: `stop_wan_locked()` previously stored a checked protocol error in `stop_status`, then overwrote it with successful VPN/PPP stop results. Final teardown now aggregates the protocol status after independent interface/callback cleanup and refuses final WAN-state clear if it remains nonzero.
- Added executable C policy regressions for conservative MAN DNS route add/remove and preservation of the first protocol teardown error, plus source-contract checks for callback ordering and fail-closed behavior. Exact-SHA Actions are required for validation; no local firmware toolchain is available here.
- Next: inspect exact-head WAN route, DHCP callback, teardown and XY-C1 runs; address any true source/test failures before expanding the audit.

---

## 2026-11-22 — Repair exact-head checked-protocol regression contracts

- Parent main SHA: `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact-head `4a6df662b6c34d2282e81b3ef8054e80e0894393` completed with three actionable workflow issues: WAN route checker still expected the removed `rc_first_service_start_status(stop_status, proto_status)`; WAN teardown workflow still located the old combined error gate rather than the new first-error aggregator; the DHCP callback embedded Python used an undefined `require()` helper.
- Updated those source contracts to verify protocol-error aggregation after callback cleanup and switched the inline DHCP check to a standard assertion. These are CI/test harness corrections; the production source slice remains unchanged in this commit.
- The WAN route, WAN DHCP callback and WAN teardown Actions on SHA 4a6df66 failed at the above contract/assertion issues. The host C policy builds preceding the WAN teardown Python contract succeeded. XY-C1 Build/TRX/artifact and the independent repository inventory remain under exact-head verification at this checkpoint; no unobserved result is treated as PASS.
- Next: inspect exact-SHA reruns for these three contracts and the WAN DHCP source gate; then review any compiler/build failure or other independent regression.

---

## 2026-11-22 — Fix WAN IPv4 and teardown source-contract locators; refresh handoff

- Parent main SHA: `4c1f1be93e4c0436d0b20a3b4ddfbb314d07914c`; product source remains `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact checker SHA `4c1f1be93e4c0436d0b20a3b4ddfbb314d07914c`: WAN route #44967410609 failed only because the IPv4 lookup contract expected a brace-less error return; the source uses a braced checked-protocol error path. WAN teardown #44967410572 reached its final ordering assertion but located an earlier generic stop-status gate. Both source-contract locators are corrected without relaxing production behavior.
- AI handoff #44967410603 failed because a prior docs checkpoint renamed the required first-action heading. Restored the exact protocol heading and updated the handoff with product SHA, checker SHA, actual run states, the frozen backup and the first next action.
- WAN DHCP #44967410597 PASS on checker SHA `4c1f1be...`. The previous firmware run for product SHA `4a6df662...` remained in progress at last poll; L3 is not claimed.
- Next: verify exact-head route/teardown/handoff reruns, inspect XY-C1 Build/TRX/artifact, then continue checked modem identity audit.

---

## 2026-11-22 — Fix XY-C1 WAN reset compile blocker and refresh source contracts

- Parent main SHA: `6cdaaccda83823f252dc537e4d9bfd358bfe845e`; product-source target was `4a6df662b6c34d2282e81b3ef8054e80e0894393`.
- Exact XY-C1 run #44967290950 completed FAILURE. The production build reached `net_wan.c` and reported four errors in `reset_wan_vars_status()`: assigning string literals to fixed-size arrays `man_addr`, `man_mask`, `man_gate`, and `man_mtu`. Those buffers already have valid initializers; removing the redundant assignments fixes the C type error without changing defaults or the manual endpoint snapshot.
- Exact WAN route #44967576615 failed at the stale static-IPoE teardown parser in `check_wan_detect_wan_status.py`; its anchor is updated to match the checked-protocol condition and an array-initialization regression was added.
- Teardown run #44967575277 on `6cdaacc...` had zero jobs; inspection found two Python lines de-indented outside the YAML `run: |` block. Their indentation is repaired.
- Handoff validator #44967576593 passed. XY-C1 TRX and artifact steps were skipped after the compile failure; no L3 claim is made. A fresh exact-head firmware run remains mandatory.

---

## 2026-11-22 — Align WAN route and HTTPD source contracts with retained status

- Parent main SHA: `02857320194ebdda3b451ea3d6de07fa212995c0`; source under test: `02857320194ebdda3b451ea3d6de07fa212995c0`.
- Exact WAN route #44968037254 reached its cleanup contract and failed because the workflow expected the old `udhcpc_deconfig()` return expression that omitted `proto_status`. Production source deliberately accumulates protocol-read failure, interface reset, conntrack flush and WAN/MAN cleanup; checker now validates the complete nested first-error accumulation.
- Exact HTTPD security #44968037240 failed because its lifecycle check expected the pre-change combined stop/final interface cleanup gate. It now checks that callback cleanup precedes `wan_teardown_first_error_status()`, and that the resulting failure gate precedes clearing WAN state.
- On SHA `0285732...`, WAN teardown, DHCP callback, USB modem identity and handoff checks all passed. XY-C1 build remained in progress at the last poll; the previous build failure had been the four illegal assignments removed in `0285732...`.
- This is a test-contract alignment slice; no production source changes in this commit.

---

## 2026-11-22 — Update WAN route contract for checked protocol lookup

- Parent main SHA: `8ed832e39248e1ac0011dccfd247fba0414bac30`.
- Exact `8ed832e39248e1ac0011dccfd247fba0414bac30` WAN route run #44968296991 failed at embedded checker Python line 465. The checker still located `wan_proto = get_wan_proto(unit);` after the VPN-stop notification, but WAN teardown now uses `proto_status = get_wan_unit_proto_checked_status(unit, &wan_proto);`. Updated the contract to require the checked call and explicitly forbid the legacy getter in `wan_down_internal()`.
- On that same SHA, HTTPD authentication/disclosure policy and AI handoff checks passed. This change is checker-only.
- The next source task is an unchecked modem identity read: legacy `get_usb_modem_wan()` collapses an NVRAM backend read error into zero, potentially treating a modem as non-modem in lifecycle paths. A status-returning migration is queued as the next source slice.

---

## 2026-11-22 — Preserve WAN protocol error in DHCP deconfig regression contract

- Parent main SHA: `8998530e9609b5e401c15a3a63f23b20ebd27d60`.
- Exact-head WAN route run #44968619661 failed after all compiled policy regressions and earlier source contracts passed. Embedded Python line 654 still expected the old `interface_status → conntrack_status → cleanup_status` return chain and omitted the already-implemented `proto_status` first-error preservation in `udhcpc_deconfig()`.
- Verified the current `trunk/user/rc/src/net_wan.c` implementation reads the MAN protocol with `get_wan_unit_proto_checked_status()`, preserves the underlay address on read failure, continues independent teardown, and returns the accumulated protocol/interface/conntrack/cleanup status. Updated the WAN route contract to require that complete aggregation; no production source changed.
- AI handoff documentation check #44968619684 passed on the parent SHA. Exact-head WAN route rerun and this commit's documentation check are pending; XY-C1 validation remains tracked against product-source SHA `02857320194ebdda3b451ea3d6de07fa212995c0`.
- Next source slice: add a status-returning USB modem identity reader and migrate lifecycle-critical consumers without treating NVRAM backend errors as “not a modem”.

---

## 2026-11-22 — Correct the part-024 archive byte count

- Parent main SHA: `8dd57dffb63cfc11dc0459e7afb3b6d02c1ed9aa`.
- Exact-head WAN route regression #44968740776 passed on `09232ef0e8677c59bfa49a538fb908ce31259d0f`. The AI handoff check first caught the 120-entry root log; the subsequent rotation commit `8dd57dff` archived 100 records into part-024 but its index recorded JavaScript character count (129192) rather than UTF-8 byte count (129424).
- Re-read the immutable archive: 100 records, first `Snapshot VPN remote-LAN route ACL before route mutation`, last `Fail closed on WAN startup protocol reads`, blob SHA unchanged `359eae06c5b550cf68dc48710a91ea546551ec8f`. Corrected only the index byte metadata; no archived/root log records changed.
- Exact-head AI docs run #44968969967 failed solely on that byte-count mismatch; rerun required on this correction SHA. The prior product SHA `02857320194ebdda3b451ea3d6de07fa212995c0` has XY-C1 Build, TRX verification and artifact upload steps 37/38/39 all PASS in run #44968037242.
- Next source slice remains the checked USB-modem WAN selector: `wanN_modem_dev` currently has a lossy `nvram_get_int()` reader.

---

## 2026-11-22 — Add checked USB modem WAN identity reader

- Parent main SHA: `c6bda35a8e872f040534ef67069e3f30af467928`.
- Added `get_usb_modem_wan_status(unit, &is_modem)` backed by `nvram_get_checked()`; missing/empty/non-numeric values preserve the legacy zero default, numeric prefixes preserve `atoi()` behavior, while backend errors and integer overflow return errno-style failures. The old boolean `get_usb_modem_wan()` remains a compatibility wrapper.
- Extended the shared policy header and host regression for missing key, zero/nonzero and negative values, whitespace/numeric prefixes, malformed input, backend errors, unexpected API results, null output and integer overflow.
- Extended USB modem identity Actions to compile/run the parser regression under GNU89 + ASan/UBSan, include shared paths in push/PR filters and verify the new API contract. Existing lifecycle callsites remain for the next atomic migration slice.
- Exact-head USB modem identity and XY-C1 checks are pending.

---

## 2026-11-22 — Repair checked modem identity workflow compiler command

- Parent main SHA: `f1327a7988b9552c2ce3294c641d27838efd9664`; source/API commit under test: `f1327a7988b9552c2ce3294c641d27838efd9664`.
- Exact USB modem identity run #44969481834 failed in the added parser compile step because over-escaped line continuations passed backslash arguments to the linker. Earlier host policy steps passed. Flattened the GNU89/ASan compiler invocation to one shell line; production source and parser tests are unchanged.
- XY-C1 run #44969481753 remained in progress on f1327 at last poll. Exact rerun and Build/TRX/artifact results remain required.

---

## 2026-11-22 — Propagate checked USB modem identity through WAN lifecycle

- Parent main SHA: 7f44877976cd0c2399975adf09eb97c24a366aee. The checked selector API landed in f1327a7988b9552c2ce3294c641d27838efd9664; its identity workflow command escaping was corrected separately in 7f44877976cd0c2399975adf09eb97c24a366aee.
- Replaced is_ifunit_modem() with is_ifunit_modem_status(), which surfaces get_usb_modem_wan_status() failures and uses a bounded checked ifname_t copy. WAN-up and DHCP-bound now abort before interface/lease/route/NVRAM side effects when identity is unknown.
- WAN-down accumulates the selector error, still removes DHCP lease routes conservatively, retains WAN DNS snapshots and avoids static-IPoE address clearing when modem identity is unknown. Independent auth teardown remains best-effort.
- Extended the USB modem identity workflow source contract to guard the classifier, both early-abort paths and conservative teardown behavior.
- Exact-head USB identity, WAN DHCP, WAN route, AI docs and XY-C1 Build/TRX/artifact checks are pending for this source SHA; no target-side execution is claimed.
- Next source audit: remaining direct get_usb_modem_wan() consumers in WAN start/stop, MAN route setup/teardown, manual disconnect and resolver notification.

---

## 2026-11-22 — Align WAN lifecycle regression contracts with checked modem identity

- Parent main SHA: 4a353d031429acdb49c6fe8a6d8aebedda9a6846. The committed lifecycle change 4a353d031429acdb49c6fe8a6d8aebedda9a6846 passed USB modem identity #44969756150, WAN interface teardown #44969756159 and AI handoff validation #44969756161.
- WAN DHCP run #44969756169 failed only in its final source-order contract, which still named the retired boolean is_ifunit_modem() helper. Earlier compile/policy checks 3–19 passed. Updated the expected classifier symbol to is_ifunit_modem_status() while retaining the ordering assertions that protocol validation precedes lease snapshot and interface mutation.
- WAN route run #44969756230 failed only in the DNS lifecycle contract, whose auth/DNS/static-IP assertions still encoded pre-migration boolean identity semantics. All compiled route/policy checks 3–10 passed. Updated it to assert accumulated modem identity errors, conservative lease/auth cleanup and preservation of DNS/static-IP state.
- XY-C1 run #44969756217 was still in progress when recorded; exact Build/TRX/artifact results must be checked. This commit changes checkers and handoff only; no production source changes are claimed.
- Next source slice migrates remaining direct get_usb_modem_wan() callsites beyond WAN-up/down/DHCP-bound and adds fail-closed source coverage.

---

## 2026-11-22 — Fail closed on USB modem identity in WAN start/stop and MAN routes

- Parent main SHA: a103c9ac29a3c353592119bbb3e458529378c1f1. The previous checker-only correction commit a103c9ac29a3c353592119bbb3e458529378c1f1 passed AI handoff documentation check #44969951549. Exact reruns of the updated DHCP and WAN-route contracts had not yet appeared at the last poll.
- WAN startup now checks modem identity after the configured interface is resolved but before MTU/interface-up changes; selector errors are retained and the unit is skipped instead of falling through to ordinary Ethernet/PPP startup.
- stop_wan_locked() checks the selector before pausing the monitor or stopping services, and refuses teardown when identity is unknown.
- MAN route setup records identity errors but still adds required gateway/default routes while suppressing optional DNS route additions; MAN teardown preserves the error and still attempts scoped idempotent DNS route cleanup when modem identity is unknown.
- Manual WAN disconnect refuses to stop services until modem identity is verified. Added USB identity workflow source contracts covering these orderings and cleanup semantics.
- Exact-head USB identity, WAN DHCP, WAN route, WAN teardown and XY-C1 Build/TRX/artifact checks are pending; no runtime hardware result is claimed.
- Next: migrate remaining boolean selectors in automatic reconnect, resolver notification, DNS-mode selection and WAN interface/address getters, then search the wider RC userland for lifecycle-critical reads.

---

## 2026-11-22 — Refresh WAN-up and interface-teardown contracts for status API

- Parent main SHA: 7e8aee2ce10e3ad893015516395d82317d06a77d. The code commit 7e8aee2ce10e3ad893015516395d82317d06a77d passed USB identity #44970128339, WAN DHCP #44970128247 and AI handoff #44970128213.
- WAN route #44970128170 passed all compiled route, teardown-policy, PPP and interface-copy regressions, then failed at scripts/check_wan_dns_read_status.py line 320 because its WAN-up assertion still named the removed is_ifunit_modem() helper. Updated it to require is_ifunit_modem_status(), its explicit error return and ordering before checked protocol selection. The existing WAN-down status assertions remain in place.
- WAN interface teardown #44970128270 failed at an embedded workflow assertion requiring is_usb_modem = get_usb_modem_wan(unit) in stop_wan_locked(); production code now uses get_usb_modem_wan_status() and returns before transport/teardown side effects on read failure. Updated the workflow assertion to verify this checked order.
- XY-C1 run #44970128330 was still in progress at last poll. No target build or artifact outcome is claimed.
- Next source slice migrates reconnect-priority, resolver-notification, DNS-mode and WAN interface/address selectors.

---

## 2026-11-22 — Align WAN identity lifecycle source contracts

- Parent main SHA: `05aebf7d3bf2242fd967cb825a928693f120a545`.
- Updated WAN-up and DHCP-bound contracts for the checked modem identity helper, requiring a status-returning helper call and error return before protocol/runtime or DHCP snapshot/interface mutation; also check that modem_dev and ifname_t are fetched through status APIs.
- Added the `re` import inside the DHCP workflow's embedded Python heredoc (not at YAML root) for the identity-call regular expression.
- This commit changes regression contracts/handoff only; exact-head workflow and XY-C1 validation remain required.

---

## 2026-11-22 — Remove final lossy modem selectors from WAN resolver and address paths

- Parent main SHA: 2ea191e751da5bbf4c6cdfb76a76e7324314168a. On exact source SHA 2ea191e751da5bbf4c6cdfb76a76e7324314168a, AI handoff #44970372450 passed. WAN route #44970372464 failed in scripts/check_wan_dns_read_status.py line 334 due a duplicated print() statement; all compiled route/policy steps 3–10 passed. WAN DHCP #44970372463 failed at embedded Python line 207 due a duplicated viptv_bound assignment; all preceding DHCP policy/compile/order checks 3–19 passed.
- Replaced the last five get_usb_modem_wan() calls in net_wan.c: automatic reconnect priority now refuses to queue on unknown identity; resolver publication forces DNSMasq refresh if identity is unknown; static DNS mode, WAN interface selection and WAN IPv4 lookup all use the status API and propagate failures.
- Removed direct legacy selector consumers from net_wan.c entirely while keeping the shared legacy boolean ABI wrapper for compatibility. Updated source contracts for identity/error ordering, fixed the two duplicated Python lines, and changed the stale static-IPoE teardown assertion in check_wan_detect_wan_status.py.
- New exact-head USB identity, WAN route, WAN DHCP and XY-C1 Build/TRX/artifact results are pending; no firmware pass is claimed.
- Next: search the wider RC userland for direct legacy selector calls and migrate only lifecycle-sensitive consumers with focused tests.

---

## 2026-11-21 — Fail closed in legacy USB modem identity wrapper

- Parent main SHA: `95bfb65ed32a6c507fb93bd442086496fccacb9d`.
- Changed `get_usb_modem_wan()` compatibility behavior so failed `get_usb_modem_wan_status()` reads map to boolean true (conservative modem classification), not false. This prevents legacy callers from misclassifying an unknown WAN as non-modem while they are migrated to explicit status handling.
- Added pure policy regression for absent/present identities and EIO/ENOSPC read failures; extended USB modem workflow source assertions to forbid the old zero-on-error wrapper.
- Exact-head USB modem parser and XY-C1 firmware checks are pending for this source change.

---

## 2026-11-22 — Correct exact-SHA WAN route, modem identity and process lifecycle contracts

- Parent main SHA: `c4a27498de42e68bf21a358e4938008ff43ba0e7`. AI handoff #44970629909 PASS, Wireless #44970629880 PASS, WAN DHCP #44970629930 PASS, NVRAM transaction #44970629921 PASS and process lifecycle inventory scan #44970629864 PASS. XY-C1 #44970629875 remained IN_PROGRESS at last poll.
- WAN route #44970629902 failed in its resolver source contract because it searched for the deleted comment `/* force notify dnsmasq for modem */`. The production code now checks the modem identity status and forces DNSMasq reload on unknown identity. Updated the source assertion to bind to that behavior.
- USB identity #44970629920 failed because the checker body helper matched a forward declaration for `get_wan_dns_static_status()`; it therefore extracted the wrong subsequent brace body. Anchored DNS, WAN interface, WAN IP and reconnect checks to actual function definitions.
- Process lifecycle #44970629903 failed because its script expected a stale compound condition. Production currently aggregates protocol, interface and callback failures through `wan_teardown_first_error_status()`, then gates final WAN state clear; updated the assertion to check that actual ordering and argument set.
- This commit is checker/handoff/log only. No production source changes or firmware-build result are claimed.
- Next production slice: migrate RC-wide legacy USB modem selector consumers, with special care not to skip physical-unplug cleanup when the selector read itself fails.

---

## 2026-11-22 — Preserve modem unplug cleanup and reject ambiguous auto-start

- Parent main SHA: `6444ce9468a81e8f52b7636990b93ba994b7235a`.
- Exact parent-head Actions: USB modem identity #44971100959 failed only in the final source contract because it searched for the checked modem identity call on one line; production wraps the call across two lines. AI handoff #44971100969, WAN route #44971100970 and Process lifecycle #44971100963 passed. The eight preceding USB host compile/regression steps passed.
- `hotplug_usb.c`: both NDIS and TTY physical-unplug handlers now use `get_usb_modem_wan_status()`. A confirmed modem or an unreadable identity queues `on_unplug_usb_modem`; known non-modem state does not. This preserves mandatory WAN teardown when NVRAM identity reads fail.
- `services_usb.c`: automatic modem selection now requires a successful identity read proving that the current WAN is not already a modem. Read errors are logged and returned before any reconnect request.
- Added policy regressions for unknown-identity unplug cleanup and auto-start rejection, and source contracts for both physical unplug paths and the modem start gate. Repaired the multiline reconnect contract in the existing USB identity workflow. Existing GNU89/ASan/UBSan test integration is retained.
- THIS COMMIT includes source, tests, workflow contract, handoff and this log entry atomically. Exact-head Actions and XY-C1 target Build/TRX/artifact are required; no local target build or hardware result is claimed.
- Next: verify exact-head Actions, then migrate the remaining selectors in `usb_modem.c`, `rc.c` and `detect_link.c`.

---

## 2026-11-22 — Include modem WAN start policy in USB service translation unit

- Parent main SHA: `ff6e5b90051317d81b14427829ccc1999c2f572b`.
- Post-commit source integration review found that `services_usb.c` called the newly added `modem_wan_auto_start_allowed()` static-inline policy without including its defining header. Added the explicit `modem_wan_start_policy.h` include to prevent an implicit declaration/unresolved external at compile/link time.
- This is a build-integration correction to the preceding modem identity source slice; no behavior or configuration format changed.
- Exact Actions for preceding SHA `ff6e5b90051317d81b14427829ccc1999c2f572b` were still in progress at review: AI docs #44971644605, XY-C1 #44971644588, NVRAM audit #44971644611, Process inventory #44971644652, USB storage #44971644630, HTTPD #44971644660, process fail-closed #44971644640 and USB identity #44971644620. The new SHA must independently pass.
- THIS COMMIT changes the include and updates the handoff/log as one atomic source-integration correction. No local target build or hardware result is claimed.
- Next: verify exact-head checks, then migrate the remaining checked modem selectors in `usb_modem.c`, `rc.c` and `detect_link.c`.

---

## 2026-11-22 — Align unplug identity source contract with conservative cleanup

- Parent main SHA: `be7b6c73d92af9de68a365156d69ad84e1eb81ab`.
- Exact USB modem identity run #44971644620 failed at the pre-existing comment assertion `Always notify the WAN owner`. The new production behavior intentionally notifies only for known modem identity or an unreadable identity; a known non-modem unplug is not a WAN modem event.
- Updated the contract to require the accurate `Preserve mandatory WAN cleanup on physical unplug` statement. This is a checker-only correction; no production behavior changed.
- All eight preceding GNU89/ASan/UBSan compile-and-run regression steps passed on behavior SHA `ff6e5b90051317d81b14427829ccc1999c2f572b`, including the added unknown-identity policy tests. AI handoff docs, HTTPD auth/security and Process lifecycle inventory also passed on that SHA; XY-C1 was still in progress at last poll.
- THIS COMMIT updates the workflow assertion, handoff and log atomically. Fresh exact-head USB identity/AI docs and target Build/TRX/artifact results remain required.
- Next: confirm current-head CI, then migrate the remaining selectors in `usb_modem.c`, `rc.c` and `detect_link.c`.

---

## 2026-11-22 — Gate modem link, module restart and WAN LED decisions on checked identity

- Parent main SHA: `4949f8f41ccd23df2e657477ede2e667410f2e19`.
- `usb_modem.c::notify_modem_on_wan_ether_link_changed()` previously used the legacy boolean selector and could queue an automatic reconnect from ambiguous state. It now uses `get_usb_modem_wan_status()`, logs and returns on failure. The disabled legacy internet-state helper was migrated as well.
- `rc.c::RCN_RESTART_MODEM` previously used the conservative legacy boolean as a destructive restart decision. It now gates WAN stop, modem module load/unload/reload, persistent runtime rule/type checkpoints and full WAN restart on a successful identity read, preserving a read error in the RC action result.
- Both WAN LED paths in `detect_link.c` now use the checked selector plus `usb_modem_wan_legacy_boolean()`; errors conservatively select the modem display branch. The USB identity workflow reads these sources, asserts their ordering/gates, and includes `detect_link.c` symmetrically in push/pull_request triggers and its inline source allowlist.
- Added a pure regression for the modem restart identity gate to the existing GNU89/ASan/UBSan policy test.
- THIS COMMIT atomically includes three source migrations, policy/test, workflow contracts and handoff/log. Current exact-head CI must confirm the full regression suite and XY-C1 target stages; no local target or hardware result is claimed.
- Next: reconcile exact-head CI, then select a distinct whole-userland P1/P2 stability/security defect from the roadmap ledger.

---

## 2026-11-22 — Retry failed deferred USB event marker writes

- Parent main SHA: `e26890edfef8dd47aa1bbb1d514a315933de2571`.
- `rc.c::handle_notifications()` removed queued RC events before writing temporary NVRAM markers for printer add/remove, modem add/remove and mass-storage add. The ignored setter result could discard the only wakeup when NVRAM write failed; the existing alarm did not recreate the notification.
- Added a fixed-event retry mask and queue recovery worker. Marker failures now propagate to `action_result`, preserving the incomplete marker. PID 1 retries safe queue marker creation on SIGALRM and sends SIGUSR1; retry bits clear only after queue marker creation and wakeup succeed.
- Added GNU89 policy tests for all five mappings, unknown names, failure recording and wakeup completion. Extended the RC notification source contract.
- Corrected `check_storage_safe_remove.py`: it incorrectly prohibited the single initial `storage_should_start = 0` output initializer. The check now requires exactly one initializer before checked marker reads and dispatch.
- Parent exact-SHA Actions: USB modem identity #44972041142 PASS; PID 1 notification #44972041292 PASS; USB storage #44972041244 failed only on the stale checker assertion; AI handoff #44972041355 PASS; XY-C1 #44972041318 still IN_PROGRESS. Fresh source-SHA tests/build/TRX/artifact are required; no target or hardware pass is claimed.
- THIS COMMIT atomically includes RC source/API, policy and regression, notification contracts, storage checker correction, handoff and log.

---

## 2026-11-22 — Register deferred USB retry regressions and align exact-head CI contracts

- Parent main SHA: `2f93266c59b9a10fec33589ada3e2e9c7b803b84`.
- PID 1 notification #44972905893 failed because the regression functions had not been included/registered in `rc_notify_batch_policy.c`. This commit adds both GNU89 regression bodies before `main()` and executes them: fixed event mapping and rejection of unknown names; marker-write error accumulation; and retry-bit retention/clearing based on queue wakeup completion.
- HTTPD auth/security #44972905619 exposed a legacy source-shape contract in `init.c`. Retry remains conditional on `router_ready` and precedes deferred USB handling, but the existing standalone `if (router_ready) on_deferred_hotplug_dev();` is preserved.
- HTTPD daemon lifecycle #44972905858 had a stale assertion demanding an explicit `presence == PROCESS_PRESENCE_UNKNOWN` branch. Production delegates that check to `httpd_start_postcondition_requires_recovery(presence)`; the workflow now asserts the delegation and the helper's non-ABSENT condition.
- USB storage #44972905435 and XY-C1 #44972905721 were in progress at the prior poll. No same-SHA target build/TRX/artifact result or hardware verification is claimed.
- THIS COMMIT atomically updates the regression, init source shape, HTTPD daemon source contract, handoff and historic dev log.

---

## 2026-11-22 — Fix missing PID 1 main-loop binding in retry workflow contract

- Parent main SHA: `3e99d89dd56e70d1c59230d42d33da166b9d2cc2`.
- PID 1 notification #44973128571 compiled and executed every GNU89/ASan/UBSan regression successfully, including the newly registered deferred hotplug marker mapping and retry lifecycle tests. Its final Python source contract failed with `NameError: main_loop is not defined` because the checker used that local in the new SIGALRM ordering assertion without first extracting `init_main_loop()`.
- Added the missing `main_loop = function_body(init, "init_main_loop(void)")` binding. The new assertion continues to require queue marker retry before deferred-device dispatch inside the PID 1 alarm path.
- Exact parent-SHA HTTPD auth/security #44973128500 PASS, HTTPD daemon lifecycle #44973128575 PASS, PID 1 mdev security #44973128472 PASS, AI handoff #44973128460 PASS, DNS Forwarder #44973128486 PASS, DNSMasq #44973128493 PASS and Process inventory #44973128482 PASS. USB storage #44973128576 and XY-C1 #44973128574 were still running at last poll.
- THIS COMMIT atomically updates the workflow contract, handoff and log; fresh exact-SHA CI and target Build/TRX/artifact results remain required.

---

## 2026-11-22 — Firewall boot enable-state hardening and exact-head CI reconciliation

- Production source slice: `e9ef41bd5a103b1693d5f0044d3ec5e5532c1661`. PID 1 no longer maps failed reads of `fw_enable_x` or `wan_nat_x` to false; checked status gates `start_wan()`. `ipt_nat_default()` now preflights all applicable NVRAM inputs before creating the restore file. Added GNU89 policy regressions and source-order assertions.
- Initial Firewall Startup run #44974210810 exposed a stale caller signature in `scripts/check_forwarding_control_status.py`; all preceding firewall policy/NVRAM and forwarding regression binaries passed. Updated the checker in `ef20a36bf970ec3c3c487cb71c2e0bdc615bf073` without changing production code.
- Exact descendant Firewall Startup rerun #44974378538 PASS on `ef20a36`. PID 1 notifications #44974210641, Process inventory #44974210706, AI docs #44974210724, WAN DHCP callback #44974210669 and infosvr lifecycle #44974210764 PASS on production source `e9ef41`.
- XY-C1 validation #44974378528 is tied to exact code SHA `ef20a36` and remained IN_PROGRESS at this checkpoint. No L3 Build/TRX/artifact completion or hardware L4 result is claimed.

---

## 2026-11-22 10:07 CST — Align HTTPD firewall boot contract and correct L3 checkpoint

- Parent main SHA: `ac8436355e5081e81737ffa758bd78fc2d73112e`; firewall product-source SHA: `e9ef41bd5a103b1693d5f0044d3ec5e5532c1661`.
- Exact HTTPD auth/security run #44974210716 failed at the embedded firewall boot assertion still requiring the removed unchecked `nvram_match("wan_nat_x", "1")` assignment. The workflow now verifies both checked boot-enable readers, read-before-NAT-restore order, the unreadable-selector regression, and checked WAN-start policy order.
- Firewall Startup #44974378538 PASS on checker-only descendant `ef20a36`; NVRAM #44974210645, Process inventory #44974210694, AI docs #44974210724 and WAN route #44974210718 PASS on product SHA `e9ef41`. AI handoff #44974465784 PASS on documentation checkpoint `ac843635`.
- Important L3 correction: XY-C1 #44974378528 on `ef20a36` completed SUCCESS but Build/TRX/artifact were SKIPPED because that push changed only the checker. Product-source run #44974210522 on `e9ef41` remains IN_PROGRESS at Build XY-C1; TRX/artifact are not yet verified.
- No local checkout/toolchain; local tests were NOT RUN. No L3/L4 claim. THIS commit atomically updates HTTPD workflow contract, handoff, roadmap and this log; exact HTTPD/AI reruns remain pending.

---

## 2026-11-22 — Transactional LAN DHCP bound-state snapshot

- Parent main SHA: `6d03a4b1d63ab5cf058e8c270a76c50740de6c2b`; production source commit: `6ca55ecdaca95858aadbf77c4fd1af49b0e1644c`.
- `udhcpc_lan_bound()` previously read the seven old DHCP tuple fields individually with `nvram_safe_get()`. A transient NVRAM read/close error was indistinguishable from a missing/empty key, so rollback and changed-field detection could consume an incomplete previous tuple. The no-`subnet` path also read the lease netmask and configured fallback separately without status.
- Added one transaction snapshot for `lan_ipaddr_t`, `lan_netmask_t`, `lan_gateway_t`, `lan_dns_t`, `lan_domain_t`, `lan_wins_t`, `lan_lease_t` and `lan_netmask`. The callback rejects the event on any backend, close or destination-capacity error before kernel, file or NVRAM publication. Both rollback comparisons and netmask fallback use the copied snapshot.
- Updated DNSMasq source contract to pin the eight-key list, begin/get/end transaction callbacks, fail-closed ordering and removal of unchecked getters. DNSMasq #44974691080 PASS, WAN DHCP callback #44974691075 PASS and NVRAM transaction boundary #44974691126 PASS on this exact source SHA. XY-C1 #44974691162 was still building; TRX and artifact upload pending.

---

## 2026-11-22 — Reconcile HTTPD firewall validator source contract

- HTTPD #44974691100 on source SHA `6ca55ecdaca95858aadbf77c4fd1af49b0e1644c` failed because the workflow hard-coded ten occurrences of `firewall_ipv4_address_is_safe()`, while the committed production source has nine. This was a stale checker expectation, not a C source or regression-test failure.
- Checker-only commit `544ef61b244c13b8a2220f3e21d91a257f065fd3` changes the brittle exact count to a minimum floor and directly asserts the default NAT builder validates its copied LAN IP and contains no unchecked NVRAM reads. Follow-up `a05cb570eea8deb76d67fece4994ee415924c67e` replaces escaped-newline function markers with stable source slicing.
- Exact rerun HTTPD #44974951724 PASS and AI docs #44974951739 PASS on `a05cb57`; no production HTTPD code changed.

---

## 2026-11-22 10:16 CST — Propagate OpenVPN certificate generation failures

- Parent main SHA: `3118578664b49741a149c40fdd509b9a0bd807ef`. Audit found `make_cert()` ignored EC/RSA/Ed keygen and CSR return codes, `sign_cert()` removed the CSR even when signing failed and returned success, `make_dh()` ignored DH generation errors, and server/client wrappers continued after a failed prerequisite.
- Production script now checks OpenSSL keygen/CSR/sign/DH command results and non-empty outputs, removes incomplete newly generated key/DH outputs, protects private keys before request generation, propagates chmod errors, stops server/client workflows at the first failure, and preserves the CSR when signing fails.
- The mocked crypto regression now injects partial files plus failure/empty-output paths across RSA/EC/Ed keygen, CSR, signer, DH and chmod, and verifies client orchestration stops. Existing TLS-key permission repair and TLS-crypt-v2 error checks remain.
- Exact OpenVPN private-key workflow is triggered by the changed script/test paths; results pending. No local test run or XY-C1 result is claimed.

---

## 2026-11-22 10:17 CST — Close OpenVPN key-cleanup follow-up

- Parent main SHA: `ddef1929c2c2799a464d8d397597f1e7d9f64b51`. Review of the first implementation found the function-slice generator had joined adjacent shell function declarations without a newline; this follow-up restores explicit shell delimiters before validation.
- Also fail closed on TLS-Auth/TLS-Crypt v2 chmod errors by removing the unprotected key path, and extend the fake OpenVPN generator to leave a partial file on failure so cleanup is actually exercised. Regression now injects chmod failure for CA and both TLS private keys.
- The exact-head OpenVPN workflow for `ddef192` was started as #44975313356 while the delimiter defect was being closed; do not use it as pass evidence. The follow-up commit triggers a clean exact-head syntax/test run.

---

## 2026-11-22 10:18 CST — Remove certificate private keys after mode-protection failure

- Parent main SHA: `456c8176cc81209f0b75c2bc5f7d156589810870`; exact OpenVPN certificate private-key workflow #44975381015 PASS on that SHA, including `bash -n` and the expanded crypto failure/permissions regression.
- Follow-up review found `make_cert()` returned on a failed chmod but left the key pathname behind. It now removes the key before returning; regression injects CA-key and server-key chmod failures and verifies no key/CSR is left. TLS-Auth/TLS-Crypt v2 cleanup cases are also covered.
- VPN server lifecycle #44975380856 and XY-C1 #44975380833 were still running against `456c817` at last poll; do not claim firmware or target verification from them until exact-stage results complete.

---

## 2026-11-22 10:20 CST — Fix OpenVPN signer digest option and lifecycle contract

- Parent main SHA: `9f98dbe48d0f4bf6ece4cf025741cbca68c35d23`.
- VPN lifecycle #44975466566 exposed a stale script checker requiring the old unquoted signer call. The checker now verifies the quoted server signer invocation, server/client fail-fast propagation, and that the signer passes through its configured digest option exactly once.
- A source audit found a real crypto defect: `DGST_ALG` already contains a leading dash, while `sign_cert()` prepended another, passing OpenSSL `--sha256`/`--sha1`. Corrected to pass `"$6"` verbatim; fake OpenSSL now rejects the doubled-dash option to guard the success path.
- Repaired the malformed active/verification/next-action handoff block and updated VPN roadmap status atomically with source/test/checker changes. Exact-head Actions pending; no local toolchain/router validation claimed.

---

## 2026-11-22 10:24 CST — Fail closed on invalid vsftpd passive-port range

- Parent main SHA: `4188434603fb3a6dc55ebe8b5e6c9a1636964c8b`.
- `write_vsftpd_conf()` checked FTP mode, logging and connection-limit NVRAM reads but still used unchecked `nvram_safe_get_int()` for `st_ftp_pmin` and `st_ftp_pmax`. Backend/malformed-value failures silently became defaults; an inverted readable pair could be published as an unusable vsftpd configuration.
- Added `ftp_passive_port_policy.h`: propagates checked read errors, keeps missing-key defaults and historical 1..65535 clamp, and rejects inverted ranges. `services_stor.c` reads both values through the checked reader and publishes only validated ports.
- Added a GNU89/Werror regression; extended vsftpd Actions build, source-contract and changed-path checks. Updated roadmap and handoff. Exact-head CI pending; no local test result claimed.

---

## 2026-11-22 CST — Add passive-port regression paths to PR filters

- Parent main SHA: `1a922e08b90f3cc82211b1d81ffb926d78441eff`.
- Exact VSFTPD #44975976664 ran the new GNU89/Werror passive-port policy test and then failed the workflow's final changed-path contract: the new policy header/test were included in push paths but absent from pull_request paths.
- Added the same two file paths to the PR filter and recorded the precise CI boundary in the handoff. Workflow-only correction; production source remains in parent commit.

---

## 2026-11-22 10:30 CST — Reject unknown Samba account-count state before passdb publication

- Parent main SHA: `128089bbad8b39111eb5449f41ccfd6884c20bf5`.
- Audit of `run_samba()` found `sh_num = nvram_safe_get_int("acc_num", 0, 0, MAX_ACCOUNT_NUM)`: NVRAM backend/parse failures were converted to a plausible zero-user count. On SMBD36, that could build and publish an empty staged passdb, replacing valid SMB credentials; it also hid config corruption on other builds.
- Added `samba_passdb_account_count_status()` with checked-status propagation and the existing 0..MAX_ACCOUNT_NUM clamp. run_samba now validates `acc_num` immediately after acquiring the config lock, before process/cleanup/passdb mutations, and preserves the old passdb/config on any read failure.
- Added policy regressions for default zero, valid values, lower/upper clamping, EIO/EACCES/negative failures and invalid arguments; extended Samba workflow source-order contract and roadmap/handoff. Exact-head CI pending.

---

## 2026-11-22 CST — Normalize Samba account-count source-order contract

- Parent main SHA: `45d052982bd51efcbdc47e5c818544f95d4d5689`.
- Samba #44976440925 failed at Python `ValueError: substring not found` in the new source-order assertion. The checker verified compact strings in the whitespace-preserving function body, then called `.index()` without normalization. It now evaluates all ordering assertions against one whitespace-normalized `run_samba_compact` string.
- No product-source change. The preceding account-count guard remains pending an exact-head Samba workflow rerun.

---

## 2026-11-22 10:38 CST — Snapshot checked Samba account identities and secrets before publication

- Parent main SHA: `909a0095de86a88f7dfb8343923e641f466ac923`; exact Samba #44976705529 and AI docs #44976705516 PASS on this parent, validating the prior account-count guard.
- Audit found `run_samba()` still used `nvram_safe_get()` for `http_username`, `acc_usernameN` and `acc_passwordN`. Those reads did not distinguish NVRAM backend errors from empty values, and cache-backed pointers could be invalidated by the next NVRAM read.
- Added a copied status-bearing string reader around `rc_wan_nvram_copy_value()`. `run_samba()` now snapshots router identity, each configured username and each accepted password into bounded buffers before process inventory, cleanup, config or passdb staging; it validates duplicate identities and password policy in the preflight. Read error, overlong value or invalid password aborts without replacing the last-good passdb/config. The passdb loop consumes only validated snapshots and clears password buffers on every locked exit.
- Updated Samba workflow source/order assertions, roadmap and handoff. Exact-head Actions pending; no local target build or L3/L4 claim.

---

## 2026-11-22 10:52 CST — Trigger Samba stability when its NVRAM regression input changes

- Parent main SHA: `feb5dfe55704a253cea979454cdad439d715596a`.
- The Samba workflow's source-contract step reads `trunk/user/rc/src/tests/wan_nvram_status_policy.c` and checks the copy-helper EIO/ENAMETOOLONG regression coverage, but that test path was absent from both `push.paths` and `pull_request.paths`. A change to a tested source-contract input could therefore bypass this workflow.
- Added the test path to both event filters; their path sets remain equivalent. Product source and test contents are unchanged.
- Connector inspection did not expose push-triggered exact-head runs (combined commit status returned no rows; the workflow-run helper is PR-filtered). Exact SHA CI must be checked in GitHub Actions; no PASS is claimed. No local checkout/host execution or target build was available; L3/L4 NOT VERIFIED.

---

## 2026-11-22 10:57 CST — Fix Samba ACL writer after exact XY-C1 compile failure

- Parent main SHA: `4f5fa861c2702ead3c3fcbe6a871fad6639f6089`; product-source SHA that exposed the defect: `feb5dfe55704a253cea979454cdad439d715596a`.
- Exact XY-C1 run #44977071471 reached the RC build and failed compilation in `services_stor.c`: four private-share ACL loops passed only `account_list[i]` to `samba_service_account_is_safe()` after its signature changed to require the router-account name. Build failed; TRX and artifact upload were skipped. All preceding host/policy stages in that run passed.
- Exact HTTPD security #44977071470 passed its earlier compiled and static checks, then failed because its Samba source contract still searched for the removed `nvram_safe_get(tmpuser/tmp2)` account reads. Updated the contract to pin the new copied username/password snapshots and the two-argument ACL helper.
- Production fix: `write_smb_conf_unlocked()` now accepts the preflight router-username snapshot from `run_samba()`; all four share ACL lists use it. The standalone `write_smb_conf()` entry passes a null sentinel and causes a checked `http_username` copy only in private-user modes, before the staged config file is opened. Samba and HTTPD workflows now assert the read-before-open ordering and all four helper calls.
- On the product parent `feb5dfe`, Samba #44977071563, Process inventory #44977071505, Process fail-closed #44977071916, NVRAM #44977071500, USB storage #44977071530, NFS #44977071428, Firewall #44977071404, USB modem #44977071499, vsftpd #44977071617 and AI docs #44977071668 passed. Workflow-only parent `4f5fa861` had Samba #44978305350 and AI docs #44978305586 PASS. Fresh exact-head source/HTTPD/Samba/XY-C1 reruns are pending for THIS commit. No local checkout test or L3/L4 claim.

---

## 2026-11-22 11:02 CST — Reconcile Samba and HTTPD exact-head source contracts

- Parent main SHA: `5b93812bd53fff8a0a4bdb71e9e85b8a23543b9c`.
- Exact Samba #44978563585 failed in its source contract at remaining old function-body lookups `write_smb_conf_unlocked(void)` and zero-argument call assumptions. The production writer now has a router-identity parameter; the workflow now locates that signature and distinguishes public `write_smb_conf_unlocked(NULL)` from `run_samba()`'s checked `router_username`.
- Exact HTTPD #44978563500 reached the Samba/Unix account identity boundary check and failed because it prohibited every `nvram_safe_get("http_username")` occurrence in the whole source file. One separate, unrelated service path legitimately still uses that key; unsafe Samba credential helper/run-body checks are now scoped to the actual Samba functions.
- These are contract-only corrections; no product source changed. Exact-head HTTPD/Samba reruns and Build XY-C1/TRX/artifact validation remain pending. The same `5b93812` VSFTPD #44978563499, AI docs #44978563541, NFS #44978563620, USB modem #44978563518, Firewall #44978563485 and Process inventory #44978563472 passed; USB storage, NVRAM, Process fail-closed and XY-C1 were still running. No L3/L4 claim.

---

## 2026-11-22 11:08 CST — Correct newline-sensitive Samba public-writer contract

- Parent main SHA: `32394e6581f566d1cd4a65a33749f319c002f9f3`.
- Exact Samba run #44978784709 (head `32394e6`) compiled and ran all GNU89/Werror policy regressions, then failed in the source-contract Python block at `function_body(stor, "int write_smb_conf(void)")`. Production declares the public wrapper as `int\\nwrite_smb_conf(void)`; the helper uses literal substring matching, so the checker locator did not match.
- Changed only the locator to search for `"int" + chr(10) + "write_smb_conf(void)"`. No production source or functional assertion was relaxed.
- Exact HTTPD #44978784728 and AI docs #44978784711 passed on `32394e6`. XY-C1 #44978563487 on product-source parent `5b93812` was still in progress at last poll; recheck terminal status. No L3/L4 claim.

---

## 2026-11-21 19:18 PST — Use coherent checked WAN tuple during route-up

- Parent main SHA: `9dfdae39490d7486f5dd33ff2b3ad68f23f1a0b0`; Samba workflow contract locator correction `9dfdae39490d7486f5dd33ff2b3ad68f23f1a0b0` has Samba #44979109639 PASS and AI docs #44979110109 PASS on its exact SHA.
- P1 audit found `wan_up_internal()` reading gateway, IPv4 address and netmask independently via `get_wan_unit_value()` (`nvram_safe_get`), then only copying the rotating-cache strings. Unlike teardown's existing `wan_unit_route_snapshot()`, this hid NVRAM backend errors as empty values and could mix different DHCP-renew generations when deciding whether to add an off-subnet gateway host route.
- WAN-up now reads the tuple through the same checked NVRAM transaction used by teardown. If that tuple read/close fails, the code preserves the failure, invalidates address/netmask ownership data, and permits only a separate checked gateway read for the existing default-route attempt; it does not infer a gateway /32 from mixed generations.
- Extended `test_transactional_wan_route_snapshot_reads_one_generation` to cover transaction-close failure and whole-tuple invalidation. Updated `wan-route-status.yml` to pin snapshot-before-route ordering, checked fallback, and absence of unchecked tuple reads. This session has no local repository checkout/toolchain; exact GNU89/Werror/ASan/UBSan and source-contract execution is delegated to same-SHA Actions. No L3/L4 claim.

---

## 2026-11-21 19:27 PST — Align WAN firewall umbrella contract with checked route snapshot

- Parent main SHA: `06604d0e6261efbf264fbe16c1eca5a6dc90cb13`.
- Exact WAN route run #44979351874 and XY-C1 run #44979351755 both failed at step 28/30, `Verify WAN firewall result propagation`, before later checks/build. The job log confirms GNU89/Werror/ASan/UBSan route-policy, teardown, PPP callback, and WAN NVRAM regressions had already passed, including the new `wan_nvram_status_policy.c` test.
- Root cause is a stale shared checker, `scripts/check_wan_firewall_status.py` lines 912–927, which still required WAN-up to independently copy three rotating-cache values through `rc_wan_nvram_copy_text(get_wan_unit_value(...))`. The product fix intentionally replaced that pattern with a coherent checked `wan_unit_route_snapshot()` plus checked gateway-only fallback after tuple failure.
- Updated the shared source contract to pin the checked snapshot, failure propagation, invalid address/netmask ownership, checked gateway fallback ordering, and absence of all three unchecked tuple getters. Both WAN-route and XY-C1 workflows explicitly include this script in their path filters.
- No product source changed in THIS checker correction. Exact reruns must pass the source contract; current-source XY-C1 Build/TRX/artifact remains pending. Earlier product source `5b93812` separately has L3 Build/TRX/artifact PASS; that does not validate WAN change SHA `06604d0`.

---

## 2026-11-21 19:35 PST — Match combined WAN route status declaration in inline contract

- Parent main SHA: `de582a224e9bb20c3e6dbfe311d926f5096de19d`.
- Exact WAN-route #44979536359 passed the updated shared `check_wan_firewall_status.py` and all preceding GNU89/Werror/ASan/UBSan route and NVRAM policy binaries, then failed at inline Python line 491. The checker expected standalone `int route_snapshot_status;`; `wan_up_internal()` actually declares `int route_snapshot_status, wan_addr_status, wan_mask_status, wan_gate_status;`.
- Updated the assertion to match the exact combined production declaration. No product source or behavioral assertion changed; the current XY-C1 run #44979536417 was pending at this check. Exact WAN/XY reruns required, no L3 claim for the WAN source SHA.

---

## 2026-11-21 19:48 PST — Track the exact PID1 recovery-shell child

- Parent main SHA: `b1c2d3fde5caa330f36715c11f270cff728bd6ec`.
- Source audit found a concrete PID 1 recovery-console liveness defect. `init_main_loop()` kept the shell's numeric PID after `catch_sig_reap()` reaped it, then used `kill(shell_pid, 0)` to decide whether to launch another shell. If a long-running daemon reused that PID, the probe succeeded for the unrelated process and PID 1 could leave recovery console unavailable indefinitely.
- Added `console_shell_pid` as volatile `sig_atomic_t` state. `run_shell()` blocks SIGCHLD before fork, publishes the exact child PID, then restores the previous mask; this closes the race where a fast-exiting shell could be reaped before its PID was tracked. SIGCHLD reaping clears only the exact tracked shell PID. The main loop now consults this tracked state, not a recyclable PID.
- Added policy regressions showing unrelated-child reap preserves the console and exact-child reap clears the tracked state; wired the test/header into both init workflow path filters, GNU89/Werror/ASan/UBSan, and Python source-order checks. Fork/mask failures are logged and retried with a one-second backoff rather than spinning.
- Exact init-mdev security and Process lifecycle runs for THIS source are pending; exact XY-C1 must build, verify TRX and upload artifact on the same SHA before L3. Hardware L4 not verified.

---

## 2026-11-21 19:52 PST — Add source include path for PID1 child-reap regression

- Parent main SHA: `b76d4764438ed923f3f20ccc272914d8f79cf96c`.
- Exact init-mdev security #44980178644 on `b76d476` passed the four pre-existing GNU89/Werror/ASan/UBSan policy binaries; the new `pid1_signal_policy.c` compile failed because its quoted header `pid1_signal_policy.h` is in parent `src/`, not adjacent `src/tests/`. The Python source contract was skipped.
- Added `-I trunk/user/rc/src` to the new test compiler command and pinned that include path in the workflow's source contract. No production source change.
- Process lifecycle #44980178618 PASS on `b76d476`; XY-C1 #44980178569 was still pending at the last poll. New exact-head init test and product Build/TRX/artifact results remain required; no L3/L4 claim.

---

## 2026-11-21 — Restore static LAN gateway after AP DHCP deconfig

- Parent main SHA: `d0bfd9407f50cad01cc683a7d7b8069bc15eb5d6`.
- Source audit found `lan_down_auto()` deleted the active DHCP default route and restored the configured static LAN address, but never re-added the configured static `lan_gateway`. This left AP-mode LAN without its configured fallback default route after a lease deconfig. The manual/static bring-up path already installs that same route.
- Added a checked/idempotent static gateway route restore after successful interface-address restoration. `EEXIST` is accepted for DECONFIG_PENDING retries; other route-add failures are returned before clearing the last-known-good lease metadata, allowing BusyBox to retry. Resolver/hosts cleanup still proceeds and preserves first-error semantics.
- Expanded the GNU89 policy regression to prove route-restore failure blocks deconfig commit and resolver errors remain lower priority; expanded the source checker to pin route-add ordering and fail-closed metadata publication.
- Exact WAN DHCP callback run #44980800615 on `3cbf081` FAILED at compile time due to a three-argument IPTV caller after the shared helper expanded to four parameters. The caller was corrected in `3078a1f`. Exact rerun #44980980058 on `3078a1f` compiled and PASSED the GNU89/Werror/ASan/UBSan policy binary, then FAILED in the Python source contract at line 108: the check expected a multiline `route_add()` invocation as one literal line. This follow-up normalizes the matcher by anchoring the route call and checking its `snapshot.static_gateway` argument before resolver publication. HTTPD #44980979934 failed on the same checker; XY-C1 #44980980036 and NVRAM #44980979967 were pending at last poll. No L2/L3/L4 claim until exact reruns and same-SHA Build/TRX/artifact pass.

---

## 2026-11-21 — Restore configured static LAN resolver after AP DHCP deconfig

- Parent main SHA: `3520dd2e2aca184d62e180ee5f331990cbb12035`; exact WAN DHCP regression #44981079170 and AI handoff contract #44981079127 passed on this parent. XY-C1 #44981079156 was host-only and skipped Build/TRX/artifact because the parent changed only checker/docs.
- Source review found `lan_down_auto()` atomically replaced `/etc/resolv.conf` with an empty file after DHCP deconfig. Unlike `lan_up_manual()`, it did not restore configured static `lan_domain`, `lan_dns1`, `lan_dns2`, or the configured gateway as fallback DNS when no static nameserver was usable.
- Extended the same transactional NVRAM snapshot with both static DNS fields. Deconfig now validates the static domain, writes each valid configured nameserver, falls back to a valid static gateway only when neither configured nameserver is usable, and refuses publication on invalid domain or staged-write failure. Existing resolver lock/atomic publish and fail-closed lease metadata semantics remain.
- Added GNU89 policy assertions for DNS gateway fallback selection and expanded the Python source contract for the 13-field snapshot and ordered resolver writes. Exact-head WAN DHCP regression and same-SHA XY-C1 Build/TRX/artifact are pending; no L2/L3/L4 claim yet.

---

## 2026-11-21 — Correct DHCP deconfig source-contract arity and resolver matchers

- Parent main SHA: `1483e35a5321692372faf317851d1a09c59c7eb7`.
- Exact WAN DHCP callback #44981685847 compiled and passed the GNU89/Werror/ASan/UBSan policy binary, then failed the Python contract: the new resolver matcher asserted a semicolon immediately after `fprintf(...)`, but production correctly checks the return value with `< 0`. Updated the checker to match each checked expression and assert domain/DNS/gateway-fallback ordering through atomic publish.
- Exact HTTPD security run #44981685758 failed its embedded LAN deconfig contract because the route-delete gate still expected the old three-argument status helper; updated it to the current four-argument call. No production source changed in this follow-up.
- On parent SHA `1483e35`, DNSMasq #44981685679, AI docs #44981685798, Process umbrella #44981685824 and Process lifecycle #44981685762 passed. XY-C1 #44981685710 and NVRAM audit #44981685780 were pending at last poll. Exact reruns required; no L2/L3 claim yet.

---

## 2026-11-21 — Preserve LAN automatic DNS mode on DHCP deconfig

- Parent main SHA: `51ff4552ae4b66dc6df736a9a61836fec8637a94`. Exact WAN DHCP #44981845607 and HTTPD security #44981845582 PASS. XY-C1 #44981845610 completed with Build/TRX/artifact skipped because the parent changed only test contracts/docs. Production-source XY-C1 #44981685710 on `1483e35` was still building; NVRAM audit #44981685780 PASS.
- Found a configuration-semantic regression in the previous static resolver restore: the manual LAN resolver path uses configured `lan_dns1`/`lan_dns2` only when `nvram_get_int("lan_dns_x") == 0`; nonzero selects lease-supplied DNS. The deconfig path unconditionally wrote DNS1/DNS2, so AP automatic-DNS mode silently switched to manual servers after lease withdrawal.
- Added `lan_dns_x` to the same transactional NVRAM snapshot (14 fields total). Deconfig now mirrors manual/automatic mode selection: static DNS1/DNS2 are emitted only when the mode parses as zero; otherwise the configured static gateway is used as fallback DNS. The fallback helper receives DNS mode, usable static DNS count and gateway validity.
- Extended GNU89 policy tests for both manual and automatic DNS mode and updated the Python source checker for the mode snapshot, guarded DNS writes and fallback selection. Exact-head WAN DHCP/HTTPD and same-source XY-C1 Build/TRX/artifact pending; no L2/L3/L4 claim yet.

---

## 2026-11-21 — Preserve static host entries and IPv4/IPv6 aliases on AP DHCP updates

- Parent main SHA: `e8b4de14f3355bef5754c780bb6ff96ea88609b8`. Exact WAN DHCP regression #44982248186 and HTTPD security #44982248126 PASS on that parent. XY-C1 #44982248169 was still in `Cross-build XY-C1 firmware`; NVRAM audit #44982248197 was still running. No current-parent L3 claim.
- Comparing AP-mode `create_hosts_lan()` with router-mode `update_hosts_router()` exposed a real compatibility regression: AP BOUND/RENEW/DECONFIG atomically replaces `/etc/hosts` but omitted the administrator-managed `/tmp/hosts.static`, the `my.router` alias, and IPv6 localhost/LAN aliases. Each AP lease refresh could therefore discard custom host mappings and IPv6 host identity.
- AP host generation now retains the same safe-regular-file check and static-host import used by router mode, adds the IPv4 `my.router` alias, and restores `::1`, the active LAN IPv6 hostname and IPv6 `my.router` alias when IPv6 is enabled. Lease-supplied domain handling, paired atomic publish, lifecycle lock and kernel hostname update remain unchanged.
- Extended `check_lan_dhcp_deconfig.py` to pin the static-host safety gate, import-before-publish ordering and all aliases. The existing WAN DHCP workflow already includes both changed paths in push and pull_request filters and compiles the GNU89/Werror/ASan/UBSan DHCP regressions. Exact-head Actions required; no local checkout/toolchain or L3/L4 result is claimed.

---

## 2026-11-22 — Correct AP DHCP host-file checker source range

- Parent main SHA: `cbc53740bfbcc292f691879f7574f70eae6d5cb4`. Exact WAN DHCP callback run #44982591441 failed in `scripts/check_lan_dhcp_deconfig.py:434`: the new assertion sliced source text only up to the `load_user_config()` call and therefore excluded the adjacent `else if (errno != ENOENT)` safety branch it intended to verify. The DHCP lease policy GNU89/Wall/Wextra/Werror/ASan/UBSan binary passed before the Python source contract failed.
- Widened only the assertion range from the static-host import call through lifecycle-lock acquisition, so the missing-file exception branch and import-before-publish ordering are both checked. This is a checker-only corrective slice; exact-head WAN DHCP rerun pending. No product compile/L3 claim is made.

---

## 2026-11-21 — Fail closed on SSH brute-force policy NVRAM reads

- Parent main SHA: `893c65a7ae1fb04a61d8aa4965c476fc50072c85`. Checker-only LAN source-range correction `893c65a7ae1fb04a61d8aa4965c476fc50072c85` has WAN DHCP #44982805519, AI handoff #44982805486 and XY-C1 #44982805453 in progress at the last poll; prior NVRAM audit #44982591433 on parent `cbc5374` completed SUCCESS.
- Firewall source review found a fail-open security defect in both `ipt_filter_rules()` and `ip6t_filter_rules()`: each read `sshd_wbfp` with `nvram_get_int()` while emitting the BFP chain. Backend read failures collapse to zero and silently omit SSH brute-force `recent` lockout rules while SSH is exposed.
- Both builders now pre-read the mode through `firewall_read_integer_setting("sshd_wbfp", 0, &i_bfp_mode)` before opening the temporary rule file, then reuse the checked snapshot during rule emission. Missing keys retain legacy mode 0; backend/malformed results abort staged publication through the existing restore-transaction failure gate.
- Added an explicit missing-default/configured-value/backend-error unit test and workflow source-order contracts pinning both checked reads before staging and forbidding the unchecked getter. Exact-head Firewall Startup, XY-C1 Build/TRX/artifact and other triggered checks remain pending; no L3/L4 claim.

---

## 2026-11-21 — Check WAN TTL policy before firewall side effects

- Parent main SHA: `7fec09ca534f339fd7ac14bf036d83c12fd66587`. Exact Firewall Startup #44982894139, WAN DHCP #44982894189, HTTPD #44982894126, WAN route #44982894201, FULLCONENAT #44982894180, process inventory #44982894127 and AI handoff #44982894167 PASS. NVRAM #44982894159 and XY-C1 #44982894183 were still running at last poll.
- A second firewall fail-open reliability path used unchecked `nvram_get_int("wan_ttl_fix")` and `nvram_get_int("wan_ttl_value")`; optional mode 2 also used unchecked `nvram_invmatch("mr_enable_x", "1")`. NVRAM backend failures therefore appeared as disabled TTL settings or altered multicast policy.
- `ipt_mangle_rules()` now checks both integers and conditionally checks exact multicast enablement through existing checked readers before writing `ip_default_ttl`, loading kernel modules or opening/staging the mangle table. Missing values preserve old zero/false semantics; failed reads abort the active firewall restore transaction.
- Added explicit helper-policy sanitizer coverage and workflow source-order contracts. Exact-head Firewall Startup and XY-C1 Build/TRX/artifact pending; no L3/L4 claim.

---

## 2026-11-21 — Fail closed when reading the NAT DNS redirect toggle

- Parent main SHA: `99f3ac8c8aa921dd958b3434ccfa708290b8b50b`. Exact Firewall Startup #44982990213 and AI handoff #44982990231 PASS. WAN DHCP #44982990226, XY-C1 #44982990270 and NVRAM #44982990240 were IN_PROGRESS at the last poll. The WAN TTL checked-read change has an exact-head Firewall Startup PASS; the target firmware build remains unverified.
- The IPv4 NAT builder still used unchecked `nvram_match("redirect_all_dns", "1")` to decide whether to DNAT LAN TCP/UDP DNS queries to the router. If this NVRAM lookup failed, the builder treated the setting as disabled and silently omitted the redirect, allowing LAN clients to bypass an enabled router DNS policy.
- `ipt_nat_rules()` now obtains the toggle via `firewall_read_enabled_setting()` before opening/staging the NAT ruleset. An absent setting remains disabled; backend or malformed-result errors abort the active firewall restore transaction instead of publishing a partial DNS policy.
- Added a named NVRAM status regression and workflow assertions for the checked pre-stage gate and generated condition. Exact-head Firewall Startup pending; no L3/L4 claim.

---

## 2026-11-21 — Move DNS-redirection preflight into the NAT builder

- Parent main SHA: `4e10fb7c8c186c12140d3d76a8a698e6c35714d9`. Exact Firewall Startup #44983057576 failed at the new DNS-redirection source contract: the checked read was inserted into the IPv4 default-filter builder, while the PREROUTING redirect is emitted by `ipt_nat_rules()`. The checker correctly caught the mismatch before rule publication; no successful Firewall result is claimed for this SHA.
- Removed the misplaced preflight and scoped the insertion to `ipt_nat_rules()`, immediately after that function's own checked firewall-enabled read and before NAT rule-file staging. The existing workflow assertion now examines the correct function body.
- Exact replacement Firewall Startup rerun required. XY-C1 #44983057625 and NVRAM #44983057663 were IN_PROGRESS at last poll; no L3 claim.

---

## 2026-11-21 — Check rp_filter feature flags before relaxing anti-spoofing

- Parent main SHA: `6fda27da8b7834297a96e0e363b2df8fa9608d4d`. DNS redirect correction `6fda27da8b7834297a96e0e363b2df8fa9608d4d` has Firewall Startup #44983258834, WAN DHCP #44983258898, WAN route #44983258810, FULLCONENAT #44983258808, process inventory #44983258806 and AI handoff #44983258870 PASS. HTTPD #44983258852 and XY-C1 #44983258842 were IN_PROGRESS; NVRAM #44983258826 was pending at the last poll.
- `start_firewall_ex_locked()` used unchecked `nvram_match("mr_enable_x", "1")` and inverse-match reads of `udpxy_enable_x` (plus optional `xupnpd_enable_x`) to decide whether WAN-side `rp_filter` could be disabled. Backend read errors could be interpreted as enabled services and set `rp_filter=0`, weakening source-address anti-spoofing.
- These feature switches are now read through checked match helpers before the sysctl update. Missing values keep legacy semantics: absent multicast is false, while absent nonzero-service fields remain unequal to `"0"` like the old inverse-match API. Backend errors return before sysctl changes.
- Added GNU89 sanitizer coverage for missing defaults, inverse-match compatibility and backend-error output preservation; workflow contracts pin the checked reads before the `rp_filter` write. Exact-head Firewall Startup pending; no L3/L4 claim.

---

## 2026-11-21 — Check NAT remote-listener gates before staging DNAT rules

- Parent main SHA: `7f88931cf6c433eab982878aa4c9a8037481e28d`. Exact Firewall Startup #44983346287, WAN DHCP #44983346264, WAN route #44983346267, HTTPD #44983346285, FULLCONENAT #44983346271, Process inventory #44983346353 and AI handoff #44983346275 all PASS. The firewall log includes the rp_filter source contracts and `Firewall rp_filter sysctl status policy: PASS`. XY-C1 #44983346281 and NVRAM #44983346304 were still running at the last poll.
- Review of `ipt_nat_rules()` found the remote-listener gates `misc_http_x`, `https_wopen`, `sshd_wopen`, `ftpd_wopen` and `udpxy_wopen` still used unchecked `nvram_match()` both to decide which external port values to pre-read and again while emitting DNAT rules. An NVRAM error could silently omit a configured port mapping or desynchronize the gate used by snapshot and emission.
- Each applicable listener toggle is now read once with `firewall_read_enabled_setting()` before NAT rule-file staging; the same local boolean controls port snapshot reads and rule generation. Missing keys preserve the exact-match legacy disabled default; backend errors abort before NAT publication.
- Added GNU89 sanitizer coverage for all five absent/default and backend-error cases, plus exact workflow assertions that checked gates precede staging and no unchecked legacy gate remains. Exact-head Firewall Startup / XY-C1 verification pending; no L3/L4 claim.

---

## 2026-11-21 — Fail closed on IPv4/IPv6 filter multicast toggle reads

- Parent main SHA: `e9a5b1fbdda849d74deb59a89a6653b25f26c26e`. Firewall Startup #44983346287, WAN DHCP #44983346264, WAN route #44983346267, HTTPD #44983346285, FULLCONENAT #44983346271, process inventory #44983346353 and AI handoff #44983346275 all PASS. The firewall log includes `Firewall rp_filter sysctl status policy: PASS`. XY-C1 #44983346281 (job #137088453878) and NVRAM #44983346304 (job #137088514305) were still running at last poll.
- Four IPv4/IPv6 filter rule sites still used unchecked `nvram_match("mr_enable_x", "1")` to decide multicast ACCEPT rules. A transient NVRAM failure could silently omit multicast handling while other filter policy was assembled from checked snapshots.
- Both `ipt_filter_rules()` and `ip6t_filter_rules()` now read the multicast toggle once through `firewall_read_enabled_setting()` before rule-file staging and reuse that value for their input/forward multicast rules. Missing-key behavior remains disabled; backend errors abort before publication.
- Added GNU89/Werror/ASan/UBSan policy coverage for missing false and backend output preservation, and workflow assertions for both filter builders' checked pre-stage read and absence of unchecked matches. Exact-head Firewall Startup pending; no L3/L4 claim.

---

## 2026-11-21 — Check firewall log-mode NVRAM reads before changing sysctls

- Parent main SHA: `1c2511a27ffe701c30ee473b630e13fcd78a63a9`. Exact `e9a5b1fb` Firewall Startup #44983577555 completed SUCCESS. Its XY-C1 #44983577660 and NVRAM #44983577624 were still IN_PROGRESS at last poll. Source SHA `1c2511a27ffe701c30ee473b630e13fcd78a63a9` had Firewall Startup #44983656044, XY-C1 #44983656163 and other triggered Actions QUEUED/PENDING; no exact-head PASS was claimed for multicast source changes.
- `start_firewall_ex_locked()` previously called unchecked `nvram_match("fw_log_x", "accept"/"drop"/"both")` after it had already written WAN `rp_filter`. A backend/close error could not be distinguished from a mode mismatch and might leave the sysctl changed while the new firewall policy failed later.
- It now validates the three exact mode matches before the `rp_filter` write, then selects ACCEPT/DROP log chains solely from the checked snapshot. Missing/unknown values still disable extra logging, preserving legacy behavior.
- Added sanitizer regression for accept/drop/both/missing/backend-error outputs and workflow source contracts requiring all checks before the first sysctl update. Exact-head Firewall Startup pending; no L3/L4 claim.

---
## 2026-11-21 — Check NAT auxiliary policy toggles before rule staging

- Parent main SHA: `d0dda5dc4dc5feada52ecfdf8a83c99a40b3e169`. Exact Firewall Startup #44983656044 PASS; its log confirms NAT remote-listener source contracts, IPv4/IPv6 multicast preflight assertions, `Required NAT restore, filter safety and push/PR workflow contracts: PASS`, and rp_filter policy checks. WAN DHCP #44983656013, HTTPD #44983656061, FULLCONENAT #44983656051, WAN route #44983656027, process inventory #44983656019 and AI #44983656035 PASS. XY-C1 #44983656163 and NVRAM #44983656068 were still IN_PROGRESS at last poll.
- NAT rule generation still used unchecked exact-match getters for `sp_battle_ips`, `nf_nat_type` and `nf_nat_loop`. These could omit BattleNET filter/NETMAP policy, select classical instead of configured fullcone NAT, or silently drop LAN hairpin SNAT after an NVRAM backend error.
- `ipt_nat_rules()` now reads all three through checked enabled-setting APIs before NAT staging and uses the same booleans for emitted rules. `ipt_filter_rules()` also pre-reads BattleNET enablement before filter staging to avoid a separate unchecked filter-side gate. Missing values remain false as before; backend errors return before ruleset publication.
- Added one named GNU89/Werror/ASan/UBSan policy regression for all missing/default and backend-error outcomes, with workflow source contracts requiring reads before rule-file open and forbidding the old unchecked calls. Exact-head Firewall Startup pending.

---
## 2026-11-21 — Check PPPoE-MWAN hook gate before firewall restore

- Parent main SHA: `82bc8c209786e5f7d412ba6e40daeab8b9c027e7`. Exact parent Firewall Startup #44983883141 and XY-C1 #44983883157 were queued; NVRAM #44983883172 pending; HTTPD #44983883282, FULLCONENAT #44983883272, WAN DHCP #44983883201, WAN route #44983883250, process #44983883289 and AI #44983883217 queued. Previous `d0dda5d` Firewall Startup #44983793304 and other completed regressions passed.
- The last direct `nvram_match()` in `firewall_ex.c` was the `pppoemwan_enable` selector for `/usr/bin/detect.sh`, evaluated after built-in firewall restore. A backend failure was silently treated as disabled.
- The toggle is now checked with `firewall_read_enabled_setting()` before `rp_filter` or table changes; the post-restore hook reuses that validated boolean. Missing remains false; backend error prevents the new firewall apply from starting.
- Added GNU89 sanitizer coverage and exact workflow source ordering/no-unchecked-getter assertions. Exact-head Firewall Startup pending.

---
## 2026-11-22 — Align PPPoE-MWAN firewall contracts with checked preflight

- Parent main SHA: b39d69486ffffa4dc4d370921524678a49c3aac8. Exact Actions: HTTPD #44983973130, process inventory #44983973215, AI handoff #44983973254, WAN DHCP #44983973157 and FULLCONENAT #44983973138 PASS. XY-C1 #44983973136 and NVRAM #44983973152 were IN_PROGRESS. Firewall Startup #44983973377 and WAN route #44983973320 failed at stale source contracts that still searched for `if (nvram_match("pppoemwan_enable", "1"))`.
- The production change intentionally replaced this unchecked getter with `firewall_read_enabled_setting()` before sysctl/table changes and `if (is_pppoemwan_enabled)` at the detector call. The command result remains logged and merged; both suites still required the obsolete spelling.
- Updated Firewall Startup and WAN route source contracts to verify checked preflight before rp_filter sysctl, the checked boolean gate, absence of the direct unchecked getter, and detector-result aggregation. No production source change in this corrective commit.
- Exact-head Firewall Startup/WAN route reruns pending. Parent XY-C1/NVRAM were not yet final; no L3 claim.

---
## 2026-11-21 — Make USB modem hotplug and startup configuration reads retry-safe

- Parent main SHA: `40f639ad8d647dc1239e0041796c03af8eaed876`. Its exact-head AI handoff #44984119963, WAN route #44984119975, Firewall Startup #44984119979 and XY-C1 host-contract run #44984119951 completed SUCCESS. The XY-C1 run skipped Build XY-C1, TRX verification and artifact upload because the parent was checker-only; it is not L3 evidence.
- Confirmed two fail-open paths: NDIS/TTY hotplug selectors used unchecked `nvram_match()/nvram_invmatch()` and silently dropped PID 1 notifications on backend errors; deferred modem startup and `get_modem_devnum()` used `nvram_get_int()`, collapsing configuration read failures to the disabled/no-device default.
- Added one transaction snapshot for `modem_prio`, `modem_rule` and `modem_type`, with strict integer parsing for readiness and explicit key-presence bits so present-empty values retain legacy `invmatch` semantics. NDIS/TTY hotplug conservatively queues the event on snapshot failure; deferred startup returns the read error so the existing PID 1 retry marker is restored.
- Added GNU89 policy regressions for NDIS/RAS exact matching, missing keys, present-empty RAS type and invalid inputs, plus source-order/no-unchecked-getter contracts. Exact-head USB workflow and same-SHA XY-C1 Build/TRX/artifact are pending; no L3 or hardware L4 claim.

---
## 2026-11-21 — Clear policy output before rejecting invalid modem match flags

- Parent main SHA: `06556f943016bbb0a8dd988d89e3892dc487402e`. Exact USB workflow #44985455596 failed at the new GNU89 policy regression: the invalid-presence case returned `EINVAL` before zeroing the caller's output flag (assertion at `modem_wan_start_policy.c:48`). Earlier USB policy executables in that job passed. XY-C1 #44985455464 was still building at the last poll.
- Move output initialization ahead of all input-flag validation whenever the output pointer itself is valid. The regression already asserts this fail-safe output contract; no rule-selection semantics are relaxed.
- Exact-head USB rerun and same-SHA XY-C1 Build/TRX/artifact are required; no L3 claim until all target steps pass.

---
## 2026-11-22 — Align USB hotplug and NVRAM snapshot source contracts with checked implementation

- Parent main SHA: `7744265e95743f832e51d6b0b041fc14167d175b` (includes the independent firewall NAT/DMZ transactional snapshot commit `7744265e95743f832e51d6b0b041fc14167d175b`).
- On USB commit `06556f943016bbb0a8dd988d89e3892dc487402e`, SCSI, modem-node identity and module-readiness host regressions passed, then the new policy test found that an invalid flag returned `EINVAL` before clearing a valid output. Commit `b6e8d07b51569502afe10a5a1ea8d0f211b09415` moved the output initialization before flag validation; the GNU89/ASan/UBSan policy regression passed on the exact b6 SHA.
- Exact b6 USB run #44985580053 then failed in the workflow's inline WDM/NDIS checker because it still searched for the removed raw `nvram_invmatch("modem_prio", "0")` gate. Exact b6 XY-C1 run #44985580063 reached later host tests but stopped at the existing LAN restart first-error regression; Build XY-C1, TRX verification and artifact upload were skipped. No L3.
- Updated the inline NDIS/RAS checks to require the checked `get_modem_hotplug_notification_status()` call, conservative queue-on-read-failure path, event-result propagation and absence of the old raw match gates. Corrected `check_modem_wan_start_status.py` to test `nvram_transaction_get` within the field-reader helper rather than incorrectly requiring it in the outer snapshot function. Exact-head USB rerun pending.
- The LAN restart failure is outside this USB change; inspect its failed test output before changing that code. No L3/L4 claim.

---
## 2026-11-22 — Align firewall NAT preflight checker with single-snapshot API

- Parent main SHA: `ba4db5fa752074d89e99413642327fba5985aae1`. Exact-head USB workflow #44985821796 PASS; exact-head handoff validation #44985821715 PASS; process lifecycle #44985821691 PASS. Firewall workflow #44985821682 failed its inline source contract at `Verify required NAT restore gates WAN startup`, Python line 239: it still required `wan_nat_x` to be read inside `ipt_filter_rules()`.
- The current firewall implementation intentionally reads the NAT toggle once in the outer restore/preflight path and passes `is_nat_enabled` into `ipt_filter_rules()`. The workflow already verifies checked preflight ordering in `start_firewall_ex_locked()`; requiring a second read inside the builder was stale and contradicted the single-snapshot design. Updated the assertion to require the passed boolean gate and explicitly reject a second raw NAT toggle read. The remaining VPN integer preflight checks are unchanged.
- The ba4 XY-C1 check-run completed SUCCESS but `Build XY-C1`, `Verify TRX output` and `upload-artifact` were SKIPPED because ba4 changed only workflow/checker/documentation; therefore no L3 evidence. Next source slice is the manual USB modem control-node pin read fail-closed fix. Frozen backup unchanged.

---
## 2026-11-22 — Fail closed on USB modem manual control-node selection read errors

- Parent main SHA: 1705d60b42615f370ead7575b2545c082b336fdf. Exact ba4 USB workflow #44985821796 PASS; handoff #44985821715 and process lifecycle #44985821691 PASS. Firewall #44985821682 exposed a stale NAT builder read expectation; #1705 corrected the filter-builder check but its exact-head rerun still failed on the same stale expectation in the NAT builder. That separate workflow contract needs a second correction. ba4 XY-C1 ran host-only and skipped Build/TRX/artifact because product source was unchanged; no L3.
- Found a manual control-node fail-open: get_modem_node_by_usb_identity() and get_modem_node() used unchecked nvram_get_int("modem_node"). A backend error could discard a configured hard index pin and silently auto-select another tty on a multi-modem hub.
- Added a checked one-key NVRAM transaction for modem_node. Missing/non-positive retains legacy automatic selection; read and close failures propagate instead of permitting fallback. RAS readiness and PPP launch carry the error; PPP launch removes stale credential-bearing peer data. NDIS blocks only TTY-control drivers after preserving WDM/QMI/MBIM branches.
- Added GNU89 identity-policy regressions for missing/default values, positive pins, INT_MAX, backend errors and invalid output, plus inline/Python source contracts. Exact-head USB, firewall and XY-C1 checks are pending; no L3/L4 claim.

---
## 2026-11-22 — Complete the firewall NAT single-snapshot source contract

- Parent main SHA: 675466a45ee1d6cef1f121c22d3914225c2c1d90. Exact #1705 Firewall Startup failed again in the NAT-builder contract: the prior correction removed the stale raw read expectation from ipt_filter_rules(), but left the same assertion in the ipt_nat_rules() builder. Source ipt_nat_rules() already receives is_nat_enabled as an argument and gates NAT work with that snapshot; the toggle is checked once in the outer firewall startup preflight.
- Updated the NAT builder contract to require the passed is_nat_enabled guard and reject a duplicate firewall_read_enabled_setting("wan_nat_x", ...) inside the builder. VPN integer settings remain independently checked and ordered before rule-file access.
- USB manual modem-node implementation commit 675466a45ee1d6cef1f121c22d3914225c2c1d90 is under exact-head validation: USB identity, firewall, XY-C1 firmware build, HTTPD, WAN DHCP and NVRAM checks were still running at last poll; handoff and WAN teardown had passed. No L3 claim until same-SHA Build, TRX and artifact steps all pass.

---
## 2026-11-22 12:53 UTC+8 — Make USB manual-node source contracts whitespace-insensitive

- Parent main SHA: 5f443c15bc6dca23769ecf14b00a4213b99bc594. USB manual modem_node fail-closed product source baseline: 675466a45ee1d6cef1f121c22d3914225c2c1d90.
- Exact 675466a USB #44986534796, Process #44986534724 and XY-C1 #44986534716 failed in scripts/check_modem_wan_start_status.py at line 892. The latest exact-head Firewall run #44986594353 on 5f443c1 failed at the same assertion. The production C call uses conventional spaces after commas; the checker applied compact() (which collapses whitespace to one space) but incorrectly compared it with a no-space token. The following RAS/NDIS assertions in the same new block made the same assumption.
- Normalize only the newly added manual-node C function bodies by removing whitespace before comparing their tokens. This keeps the checks semantic/formatting-insensitive without weakening their function-scope, error-propagation, transaction and regression-coverage assertions. Production firmware source is unchanged.
- Validation before commit: full checker not run locally (no local checkout/toolchain); source and assertion patterns cross-checked against fetched usb_modem.c. Exact-head GitHub Actions are pending for the new commit. L3/L4 NOT VERIFIED.
- Backup branch remains frozen at 72f4f157a084f8184b9597ca5bf334b8080e5807; PR #1 remains closed/unmerged.

---

## 2026-11-22 13:18 UTC+8 — Fail closed on DHCPv6 client flag read errors

- Parent main SHA: `b47e570c4001c00962bcb93dd4d1dd8a503f59ab`. Whole-source review of `trunk/user/rc/src/net_wan6.c` found `start_dhcp6c()` reading `ip6_wan_dhcp`, `ip6_dns_auto` and `ip6_lan_auto` with `nvram_get_int()`, and `dhcp6c_main()` independently reading the two automatic flags the same way. The lossy getter maps NVRAM backend read/close errors to integer zero; startup could silently skip an enabled DHCPv6 client, and a valid Reply callback could silently skip automatic LAN-prefix or DNS publication.
- Added `wan6_read_dhcp_integer_snapshot()`: the requested flags are copied under one checked NVRAM transaction, then decoded all-or-nothing with strict integer parsing. Absent keys remain empty strings and parse as zero, preserving the old default; read/close, malformed or overflow values now prevent config generation or lease-state side effects. Startup reads the three-key config tuple; callback reads only its two relevant flags. Failure in callback records the primary error and still releases the callback session lock.
- Added `rc_wan_nvram_parse_int_snapshot()` and GNU89/ASan/UBSan regression coverage for valid values, missing-key defaults, each read position failing, cache/partial-snapshot invalidation, malformed values and overflow. `scripts/check_wan6_dns_policy.py` now pins both caller orderings and rejects the old unchecked getters. The existing WAN DHCP workflow already path-filters all changed code/test/checker files.
- Validation before commit: source transformations and static contracts were assembled against fetched live `main`; no local firmware checkout/toolchain is available, so no local C/Python execution is claimed. Exact-head WAN DHCP and XY-C1 checks are pending. b47e570 USB/Process/Firewall/AI runs passed; b47e570 XY-C1 host gates passed with Build/TRX/artifact skipped because the parent was checker/docs-only. L3/L4 are NOT VERIFIED for this DHCPv6 slice.
- Backup branch remains frozen at `backup/pre-plugin-overhaul-2026-11-14` / `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

---

## 2026-11-22 13:40 UTC+8 — Preserve IPv6 service-type NVRAM read failures

- Parent main SHA: `445e03f5b18aa185934ad145fc41a7953f82cdc0`. Shared `get_ipv6_type()` in `trunk/user/shared/src/netutils.c` read `ip6_service` via `nvram_safe_get()`. That API converts backend/read/close errors into an empty string; the parser then reports `IPV6_DISABLED`, which can disable host-wide IPv6 at boot or make WAN IPv6 transitions skip required state while configuration is merely unreadable.
- Added `get_ipv6_type_status(int *)` on `nvram_get_checked()` and a pure mapping policy. Missing, empty and unknown legacy service values still map to disabled; backend failures and malformed API results remain explicit errno statuses. Kept the legacy wrapper for compatibility; state-changing callers in this slice use the checked API.
- Migrated `init_ipv6()` and `full_restart_ipv6()` to refuse global sysctl/service changes before side effects on read failure. Router boot validates/caches the service type; IPv6 restart notifications update that cache only after a successful read. WAN6 up/down and DHCPv6 callback gates now use checked reads; callback lock is released before reporting read failure.
- Added GNU89 ASan/UBSan policy cases for all five service types, absent/empty/unknown legacy values, backend EACCES/fallback EIO, malformed API results and NULL output; added exact-source ordering checker and dedicated Actions workflow. No local firmware checkout/toolchain is available; new-SHA source and sanitizer CI pending. Parent `445e03f` exact-SHA WAN DHCP #44987846226, WAN route #44987846238, USB #44987846271, AI docs #44987846250, Process #44987846220/#44987846198, Samba #44987846169, watchdog #44987846136, HTTPD #44987846117 and NVRAM #44987846132 PASS. XY-C1 #44987846160 was still at Build, TRX/artifact pending; L3/L4 not verified for this slice.
- Remaining legacy `get_ipv6_type()` callers in LAN policy, firewall, WAN interface setup, storage and OpenVPN are the next migration slice. Backup remains frozen at `backup/pre-plugin-overhaul-2026-11-14` / `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged.

---

## 2026-11-22 13:52 UTC+8 — Align exact-source gates with checked IPv6 service cache

- Parent main SHA: `915456f9e919badaa489713ce62f6017c5787808`. The new IPv6 service-type regression #44988532661 and WAN DHCP callback regression #44988532640 PASS on that exact SHA.
- Firewall #44988532674 and HTTPD #44988532650 source contracts still asserted that boot derived `ipv6_disable_required` by calling the legacy `get_ipv6_type()` wrapper. WAN route #44988532603 asserted the RCN_RESTART_IPV6 handler refreshed its runtime `nvram_ipv6_type` cache directly through that wrapper. Product code now validates and cache-loads service type using `get_ipv6_type_status()` and publishes runtime cache only after a successful checked read. Updated these three exact-source contracts to enforce the new checked behavior rather than revert product code to the lossy API.
- No production C source changed in this contract-only slice. Parent XY-C1 #44988532602 remained at Build XY-C1 with TRX/artifact pending; NVRAM audit #44988532630 remained in progress. No L3/L4 claim; exact new-SHA workflows pending.
- Remaining legacy service-type callsite migration is active; frozen backup branch and closed PR #1 remain untouched.

---

## 2026-11-22 13:23 UTC+8 — Fix Firewall Actions source-contract NameError

- Parent main SHA: `250ebfd2babb558957924e18033db434976fdbfe`. Exact Firewall Startup run #44988923723 reached the inline “Verify required NAT restore gates WAN startup” checker after four earlier host regression steps passed, then failed with Python `NameError: name 'rc_source' is not defined`.
- The workflow already loads `trunk/user/rc/src/rc.c` into `rc`; corrected the assertion to inspect `rc`. The assertion still verifies the checked IPv6 cache read and is not weakened. No product C source changed.
- SESSION_HANDOFF.md now records the exact failing run, corrected immediate validation gate, outstanding exact-head checks and next IPv6 caller migration. The whole-repository roadmap is unchanged.
- No local checkout or local workflow execution is available. Exact-head Firewall Startup and the other relevant Actions are pending on THIS COMMIT. No L3/L4 claim.

---

## 2026-11-22 13:25 UTC+8 — Preserve LAN IPv6 state on service-type read failure

- Parent main SHA: `7e8fa44420f186bea85ed9dc5cdef5990665913a`; product-source parent: `250ebfd2babb558957924e18033db434976fdbfe`.
- `net_lan6.c` used the lossy `get_ipv6_type()` in its Router Advertisement/static-prefix/DHCPv6 mode predicates. More critically, `reset_lan6_vars()` treated a failed service-type read as disabled/not-static and then published an empty `lan_addr6`, clearing last-known-good LAN IPv6 state during a transient NVRAM backend/close error.
- Added a checked LAN service-type reader, reused one already-checked type for static-prefix classification, and made `reset_lan6_vars()` return before its address read/write on service-type error. The three mode predicates retain the legacy `-1` sentinel but preserve errno/log the read failure.
- Extended `scripts/check_ipv6_service_type_status.py` to require checked reader ordering and prove the reset error return precedes NVRAM address publication. Added `net_lan6.c` to aligned push/pull_request triggers for the GNU89 ASan/UBSan-backed IPv6 service regression.
- Firewall Startup #44989237566 and AI handoff docs #44989237640 PASS on prior exact push SHA `7e8fa44420f186bea85ed9dc5cdef5990665913a`. NVRAM audit #44988923766 and USB #44988923719 PASS on product parent `250ebfd`; XY-C1 #44988923611 was still IN_PROGRESS there. This new source SHA's CI/build is pending. No local execution; no L3/L4 claim.

---

## 2026-11-22 14:06 UTC+8 — Fail closed in firewall IPv6-dependent rule builders

- Main parent at start of slice: `e85eaf114556844668acaaea9fe4d9d7afd60406`. Recovery record: commit `0608bf0` had been assembled against a stale pre-`915456f` tree, reverting nine checked-IPv6 files; `250ebfd2` restored those exact blobs on top of the correct tree.
- Migrated the three remaining `firewall_ex.c` IPv6-service reads (IPv4 filter, IPv6 filter, and IPv4 NAT/DMZ) to `get_ipv6_type_status()`. Each read fails closed before its rule file is opened/staged; NAT only reads the service value when DMZ is enabled. The protocol-41 exception uses the checked type rather than a second lossy read.
- Extended the dedicated IPv6 source checker to enforce function-local ordering and no legacy getters; included `firewall_ex.c` in both workflow path filters. Fixed Firewall workflow's prior `rc_source` NameError by using its actual `rc` source variable.
- Validation: no local firmware toolchain is available, so no local C compile claimed. Parent exact-SHA IPv6-service #44988923829, HTTPD #44988923791 and WAN-route #44988923630 passed; Firewall #44988923723 failed only at the checker NameError. New SHA checks pending; L3/L4 unverified.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` / `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged.

---

## 2026-11-22 13:27 UTC+8 — Repair IPv6 checker LAN and firewall function locators

- Parent main SHA: `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`. Exact IPv6 service-type run #44989410008 passed the GNU89 ASan/UBSan policy test and failed in the source checker at line 125.
- The LAN guard assertion required exact `if (status != 0)`; the product correctly combines the checked-read error with the disabled-mode test. The checker now matches the condition prefix and still requires the subsequent `return -1;`.
- Inspection also found three firewall function locators encoded as literal backslash-n text, which would fail after the LAN assertion was repaired. Replaced them with explicit function-signature prefixes for IPv4 filter, IPv6 filter and NAT builders. Their product-source checks remain intact.
- Product C is unchanged in THIS correction. Exact-head IPv6 workflow is pending; no local test/build, L3 or L4 claim.

---

## 2026-11-22 13:30 UTC+8 — Preserve LAN hosts and IPv6 state on service read errors

- Parent main SHA: `c29cb02e1536627abb824379ba92c8dc3d9cd8af`. The IPv6 checker correction #44989518974 and AI docs #44989518977 PASS on this exact SHA; the prior GNU89 ASan/UBSan policy and LAN6/firewall contracts are green.
- `create_hosts_lan()` previously turned an unreadable IPv6 service key into an IPv4-only `/etc/hosts` replacement. It now checks the service type before opening staged output, so backend/read/close failure preserves the last-known-good hostname/hosts pair.
- `start_lan()` now propagates a checked service read error instead of silently skipping LAN IPv6 address reload. It continues independent bridge setup through the existing first-error aggregator; a valid disabled configuration retains the old skip behavior.
- Extended the source checker with publication/reload ordering assertions and added `net_lan.c` to push/PR triggers. No local compile is available; exact-head tests and XY-C1 are pending. No L3/L4 claim.

---

## 2026-11-22 13:32 UTC+8 — Fail closed in FTP and NFS IPv6 config generation

- Parent main SHA: `0673248bc999e9e06f4f69c8d1b0bb2d05c6bea3`. Exact IPv6 service-type #44989677353 and AI docs #44989677501 PASS; DNSMasq #44989677293, Firewall #44989677298, WAN DHCP #44989677276, watchdog #44989677335, detect-internet #44989677332, detect-link #44989677304 and wireless #44989677260 PASS on that SHA. XY-C1 #44989677285 and several long jobs were still in progress at last poll.
- `write_vsftpd_conf()` previously treated an unreadable IPv6 service key as disabled and staged IPv4-only listener config. It now checks service state before opening its stage file and preserves the active config on read/close failure.
- Generated NFS exports now check service type before opening procfs/staging resources, so transient errors cannot produce/publish IPv4-only ACL output. Valid operator-provided custom exports keep the existing override path.
- Added order/no-legacy assertions to the IPv6 source checker. `services_stor.c` already triggers storage-hotplug; it is also added to the IPv6 workflow push/PR filters. New SHA Actions pending; no local compile or L3/L4 claim.

---

## 2026-11-22 13:34 UTC+8 — Check IPv6 service policy before WAN VLAN/WISP mutation

- Parent main SHA: `573ccf369c58088ca1576b43b0b9d8e75dd9c7b1`. The previous storage commit introduced checked FTP/NFS reads; its exact-head jobs were queued/in progress at last poll, so this commit does not claim their results.
- `config_vinet_wan()` now checks `ip6_service` once before the first VLAN hardware/interface/NVRAM side effect and reuses that type for WAN parent IPv6 disable and final interface IPv6 enable. Backend read/close failure aborts before topology mutation.
- `config_apcli_wisp()` now checks service state before clearing/shutting down the current WAN interface; failure refuses migration rather than continuing with unknown IPv6 policy.
- Added read-before-side-effect source assertions and `net_wan.c` to IPv6 workflow push/PR path filters. No local C compile; exact-head Actions pending, L3/L4 not claimed.

---

## 2026-11-22 13:34 UTC+8 — Preserve router hosts across IPv6 service-read failure

- Parent main SHA: `80f478f76dfb6caf2c73ba774358201757a065e8`. Previous storage commit's exact IPv6 service-type regression #44989861751 PASS; the generic run list showed Process inventory #44989861714, AI docs #44989861727, USB modem #44989861748, HTTPD #44989861752, vsftpd #44989861707, Samba #44989861773, NFS lifecycle #44989861848, Firewall Startup #44989861713 and USB storage #44989861711 PASS. NVRAM #44989861835 and XY-C1 #44989861708 were still IN_PROGRESS at last poll.
- `update_hosts_router()` previously treated an unreadable IPv6 service setting as disabled and could replace the last-good dual-stack hosts file with IPv4-only output. It now checks service type before opening staged files and returns on read/close error; enabled/disabled valid modes retain their prior output.
- Added source-order regression assertions; no local compile. New exact-head CI pending; no L3/L4 claim.

---

## 2026-11-22 13:36 UTC+8 — Align WAN startup source test with checked IPv6 reader

- Parent main SHA: `66944d56ea2da8a129f557f1d385a875f74e0333`. Source commit `80f478f76dfb6caf2c73ba774358201757a065e8` passed IPv6 service #44989981542 and multiple network/security regressions, but Firewall Startup #44989981636 and USB modem WDM/NDIS #44989981642 failed in `scripts/check_modem_wan_start_status.py`.
- Both first causes were stale source-contract checks: checker lines 292/303/318 expected old `get_ipv6_type()` guards after `config_vinet_wan()` and `config_apcli_wisp()` had been migrated to checked service reads. All earlier C compile/runtime steps in those jobs passed.
- This correction verifies the checked read and fail-closed return before VLAN hardware changes or WISP WAN mutation, while preserving required IPv6-control failure and interface-publication assertions. Product C is unchanged in this correction.
- Main `66944d56ea2da8a129f557f1d385a875f74e0333` still had in-progress Actions at last poll; this correction must be checked by exact SHA. No local C build or L3/L4 claim.

---

## 2026-11-22 13:36 UTC+8 — Fail closed when WAN DHCP 6rd policy cannot be read

- Parent main SHA: `a753477f00b5748533d4d01e2a1d862908c9ba53`. Prior main `66944d56ea2da8a129f557f1d385a875f74e0333` had the WAN checker correction pending. IPv6 service #44990023843 passed; Firewall Startup #44990023829, USB modem WDM/NDIS #44990024050 and XY-C1 #44990024029/Process #44990024080 failed at the stale WAN source-guard checker (first cause documented in the preceding entry), while multiple unrelated suites passed.
- `start_udhcpc_wan()` no longer treats an unreadable `ip6_service` as “not 6rd”. It now returns the checked-read error before starting udhcpc; valid 6rd retains both DHCP options `-O212` and `-O150`.
- Added explicit source-order checks for the read/error return, both DHCP options and process launch. Product source has no remaining legacy service-type getter in this function. No local compile; exact-head CI pending, L3/L4 not claimed.

---

## 2026-11-22 13:42 UTC+8 — Fail closed in all OpenVPN IPv6 protocol selection paths

- Parent main SHA: `db34350f72614bb891d62f512beced599cd7c5d2`. Its exact IPv6 service workflow #44990186172 and VPN lifecycle workflow #44990186063 PASS; WAN DHCP callback #44990186358, WAN teardown #44990186114, Firewall Startup #44990186002, HTTPD #44990185966, USB modem #44990185945, router NVRAM #44990186271, miniupnpd #44990186168, DNSMasq #44990186281, watchdog #44990186297, detect-internet #44990186254, detect-link #44990186417, wireless #44990186268, ZeroTier #44990185935, ARP #44990186025 and AI docs #44990186377 also PASS.
- Remaining work was in `vpn_openvpn.c`: server config, client config and downloadable client-profile export all treated an unreadable `ip6_service` as disabled IPv6 and silently downgraded requested IPv6 transports. They now share a checked resolver; errors abort before staged config/secret publication and, for export, before unlinking the prior profile, creating temp files/dirs or invoking certificate generation.
- Valid disabled IPv6 retains the existing IPv4 fallback. Added pure protocol policy plus GNU89 ASan/UBSan regression for EIO/EACCES, disabled UDP/TCP fallback, enabled transport, IPv4 no-read and NULL handling. Extended source checker and both IPv6/VPN workflow path filters; VPN workflow runs the new test.
- Previous XY-C1 #44990186127, NVRAM #44990186139, Process #44990186133/#44990186174 and WAN route #44990186395 were still in progress at last poll. No local compile. This commit's CI pending; no L3/L4 claim.

---

## 2026-11-22 13:47 UTC+8 — Align OpenVPN checks with checked IPv6 protocol selection

- Parent main SHA: `71c4a9e9111d70caa62dd45213acb82844eee717`. IPv6 service run #44990566402 failed only because three Python source locators expected a single-line resolver call; VPN lifecycle #44990566379 failed only because its exported-profile contract still demanded the removed `get_ipv6_type()` fallback. The GNU89 ASan/UBSan policy executable and earlier C lifecycle/credential tests passed in both jobs.
- Updated `scripts/check_ipv6_service_type_status.py` to locate the multiline resolver call by its function token, preserving per-function error/order assertions.
- Updated `scripts/check_vpn_server_lifecycle.py` to require the checked resolver before dual-stack endpoint selection and assert checked-read → pure policy → error-return, plus EIO/EACCES preservation tests. Existing VPN workflow already triggers on this checker, so no redundant workflow edit was needed.
- Exact parent-SHA XY-C1 #44990566341 was IN_PROGRESS at last poll; NVRAM #44990566362, Process inventory #44990566399, credential staging #44990566378, AI docs #44990566350 and HTTPD #44990566325 PASS. This checker-only commit needs its own exact-head actions; it is not a firmware/L3 claim.

---

## 2026-11-22 13:49 UTC+8 — Repair duplicate OpenVPN endpoint checker anchor

- Parent main SHA: `b87a4a09377e7efad534ed129cbd0ccc3ca719de`. On this exact push, IPv6 service-type #44990936117 and AI handoff docs #44990936053 passed; VPN lifecycle #44990936035 failed only at the final `check_vpn_server_lifecycle.py` source contract. Every preceding GNU89 policy/lifecycle/lease/PPP regression passed.
- The first causal assertion failure was an accidental duplicate `wan_addr = get_wan_addr6_host(addr6s);` token in the ordered endpoint list. The production function has one such lookup, followed by LAN IPv6 and then the validated IPv4 fallback for dual-stack protocols.
- Removed the duplicated expectation without changing the source or weakening endpoint-order checks. The existing VPN workflow already covers the checker on push and pull_request, so no workflow change is needed.
- Exact product-source XY-C1 #44990566341 on `71c4a9e9111d70caa62dd45213acb82844eee717` was still in progress at last inspection; recheck it separately. THIS checker-only descendant requires its own exact-head VPN/IPv6 Actions and is not an L3 claim.

---

## 2026-11-22 — Keep physical storage-unplug cleanup durable until marker acknowledgement

- Parent main SHA: `289c6fcb28553e78b3aa7c3f8a4e251d2544450c`. On this exact SHA, USB storage hotplug #44993522407 was still running at the last check; XY-C1 #44993522386, process inventory/lifecycle #44993522352/#44993522416, HTTPD #44993522353, USB modem #44993522402, NVRAM #44993522356 and other triggered suites were in progress. Do not infer a PASS from an in-progress result.
- `on_deferred_hotplug_dev()` cleared `usb_unplug_ms` before calling `umount_ejected()`. The wrapper attempts to re-create the marker on failure, but that write is unchecked; a transient NVRAM write failure could lose the only durable retry event. A successful eject followed by a failed marker clear could also cause the expensive cleanup action to run again on each alarm.
- The callback now performs eject cleanup before clearing the marker. It keeps the marker set on cleanup/ack errors and schedules `alarm(5)`; a process-local completion latch prevents repeating successful cleanup while only marker acknowledgement is retried. The latch resets after the marker is observed absent.
- Added pure GNU89 policy helpers and sanitizer regression for acknowledgement failure/no replay, cleanup failure/retry, absent marker/latch reset, negative status normalization and NULL output. Corrected the storage checker to match the acknowledgement writer’s actual ternary EIO fallback and expanded its source-order/retry assertions. Wired the policy binary into the storage Actions workflow.
- Local host unit test: GNU89 + `-Wall -Wextra -Werror -fsanitize=address,undefined` compile/run PASS. Only the standalone policy unit was run locally; no full MIPS firmware build is claimed.
- Next: verify exact-head USB storage Actions and XY-C1 Build/TRX/artifact on the same source SHA. Hardware L4 remains NOT VERIFIED.

---

