## 2026-11-25 — Snapshot VPN client startup settings before side effects

- PPTP/L2TP startup reads enable/type/auth/MPPE/MTU/MRU/default-route/peer/user/password/custom PPP options in one checked NVRAM transaction. Backend or transaction-end errors, malformed values and oversized strings abort before runtime DNS clearing, PPP file creation or protocol selection; absent optional values retain defaults and MTU/MRU retain the 1000–1460 clamp.
- OpenVPN enable/type/selectors and credentials join its checked client-config transaction. Runtime DNS/state clearing follows full config validation, and secret publication uses copied credentials instead of unchecked cache-backed getters.
- Added policy regressions for read failure versus absence, stale-buffer clearing, bounded copy, defaults/clamps, malformed suffix and overflow; static contracts pin snapshot-before-side-effects. Exact VPN/Process/NVRAM/AI/XY-C1 Actions pending.
- DHCPv6 finite-expiry fix `cb8f014` has exact-SHA L3: XY-C1 #45647751940 Build, TRX and artifact steps all PASS on the same SHA; focused DHCPv6/Process/WAN/USB/NVRAM checks PASS. Physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Reject corrupt RStats history ring cursors before restore

- `load_history_raw()` accepted a persisted `history_t` when only its byte count and magic ID matched, then copied `dailyp` / `monthlyp` directly into live state. `bump_history()` indexes the daily/monthly arrays using those cursors, so a corrupt or stale record could cause an out-of-bounds access during the next statistics update.
- Added a capacity-aware cursor validator and discard/log invalid records before `memcpy`. Regression covers first/last valid positions, negative and capacity-equal cursors, and invalid capacities; the existing workflow locks validation-before-copy order.
- Base main SHA: `195bc627d2d809da1877bc7c119a5a5e73342799`. Exact-head RStats Actions pending. No firmware-build or hardware-verification claim.

---

## 2026-11-25 — Align optional-app source checker with Dropbear PID identity

- The first exact Process lifecycle run #45649732824 on `880c339` failed before the new shell fixture because `scripts/check_optional_app_start_status.py::check_dropbear()` still asserted the retired `pidof` output-validation expression and its mock `killall` behavior. Those assertions no longer described the hardened procfs implementation.
- Replaced the stale mock-specific checker with explicit source invariants for procfs identity, stat start time, exact executable identity, targeted TERM/KILL, absence of name-only discovery and presence/syntax of the new regression. The dedicated shell fixture remains the behavioral test. Exact rerun pending.

---

## 2026-11-25 — Correct Dropbear shell parameter expansion in identity checks

- Review of the first published Dropbear identity change found that the generated shell source contained literal backslashes before 13 `${...}` expansions. That would prevent procfs-root, stat-field and PID snapshot variables from expanding as intended.
- Removed the stray backslashes in the source. This is a required correction to the Dropbear scanner/stop implementation; the exact process-lifecycle workflow must pass on the corrected SHA before claiming verification.

---

## 2026-11-25 — Bind Dropbear lifecycle signals to executable identity

- The service controller used BusyBox `pidof dropbear` and `killall -q dropbear`. BusyBox `find_pid_by_name()` accepts a short process-`comm` match without validating the executable, so an unrelated process can spoof `dropbear` via `PR_SET_NAME` and be counted or killed by the root stop command.
- Replaced name-only discovery with a complete numeric procfs scan that validates the stat record, PID, state, start time, exact comm and `/proc/PID/exe` device/inode against the installed Dropbear binary. Incomplete/unknown procfs blocks start and stop; TERM/KILL signal only freshly revalidated PID/start-time/executable identities.
- Added a shell regression for comm spoofing, executable mismatch, PID-reuse mismatch, targeted TERM, bounded KILL, signal failure propagation, missing procfs, malformed stat and unreadable executable identity. Local fixture PASS; exact GitHub Actions pending.

---

## 2026-11-25 — Retain the complete maximum HTTPD closing delimiter across reads

- The parser accepts a 127-byte marker, optional closing `--`, up to 128 bytes of transport padding and a CRLF terminator. The former 257-byte carry window covered a regular delimiter but could truncate the leading marker byte when the maximum 259-byte closing delimiter was split immediately before its final LF; this could leave delimiter bytes in the uploaded file.
- Sized the carry window from explicit marker, padding and suffix maxima (259 bytes). Extended the existing carry regression to split the maximal closing delimiter before its last byte and assert the whole previous chunk fits the carry. The existing workflow compiles and checks this same fixture name; exact HTTPD rerun pending. Earlier fixture-correction run #45647312009 passed.

---

## 2026-11-25 — Restore AI handoff validation after roadmap size overflow

- Exact AI handoff run #45647751963 on `cb8f014893d99e8991c55cd792443e5c2c1f4a37` failed only because REFACTOR_ROADMAP.md reached 131682 bytes against its 131072-byte hard cap. DHCPv6 IA regression #45647751899, Process inventory #45647752064, WAN DHCP callback #45647751951 and USB storage #45647752298 all PASS on that exact source SHA; XY-C1 #45647751940 and NVRAM #45647752146 remained in progress at last poll.
- Removed two obsolete 2026-11-14 run-snapshot sections from the roadmap; their historical outcomes are retained in the development log/archive and the active module dashboard/priority queue remain unchanged. The current DHCPv6 finite-expiry fix summary stays at the top. Exact AI rerun pending.

---

## 2026-11-25 — Arm DHCPv6 IA finite expiry before address publication

- update_prefix6() previously armed a finite valid-lifetime timer only after all downstream IA_PD/IA_NA kernel address operations succeeded. A failure adding a later delegated-prefix interface returned early after earlier interfaces may already have received addresses, leaving partial state without a refreshed expiry timer while the Reply was retried.
- Move zero-lifetime removal/return and finite/infinite timer handling before fallible address publication. Finite lifetimes now arm/reset expiry first; infinite lifetime cancels any old timer; zero lifetime follows the cleanup-retry path without trying additions.
- Add a GNU89 policy fixture for zero, one-second, ordinary finite, maximum finite and infinite values. The Python source contract locks expiry timer setup before both delegated-prefix and IA_NA additions and confirms zero-lifetime removal returns before reconciliation. Exact-head CI pending.

---

## 2026-11-25 — Correct HTTPD multipart fixture escape sequences

- Review of the newly added boundary regression found that the C string literals had two backslashes before `r`, `n`, `t` and `0`. They exercised literal backslash text rather than actual CR/LF/tab/NUL byte semantics.
- Corrected the fixture literals to single C escape sequences. Production helper and parser logic are unchanged. The focused helper harness compiled and passed under GNU89 ASan/UBSan; the exact repository workflow is still pending.

---

## 2026-11-25 — Validate complete HTTPD multipart delimiters

- Storage-restore scanning previously accepted any `\r\n--boundary` prefix without checking the following delimiter-line syntax. A BZ2 payload containing the same prefix followed by an ordinary byte could be mistaken for end-of-part; the truncated upload could be atomically published and flagged ready, replacing the retained retry archive.
- The rolling lookbehind was 64 bytes although the fixed 128-byte boundary buffer permits a 127-byte marker. A marker split across reads could therefore be missed. Recognition now requires a regular/closing delimiter followed by CRLF or the existing LF compatibility ending, with bounded 128-byte transport padding; the carry is 257 bytes.
- Added GNU89 fixture cases for false prefixes, malformed closing suffixes, regular/closing/LF delimiters, transport padding, and a maximum-size marker split across reads. Focused local ASan/UBSan harness PASS; exact-head HTTPD and firmware Actions pending.

---

## 2026-11-25 — Correct VSFTPD MDTM fixture and source contract

- Exact VSFTPD run #45644905866 first exposed a fixture-adapter error: the test passed the full command length instead of the timestamp-token length. The fixture now extracts the first-space offset; a local GNU89 ASan/UBSan compile/run passed.
- Exact run #45645513224 compiled and ran the corrected fixture successfully, then failed only because the source checker searched for the removed `str_stat()` name inside an explanatory comment. Narrowed the checker to the actual raw-argument call expression and asserted the test's separator-based token extraction; exact-head rerun is pending.

---

## 2026-11-25 — Remove pre-ACL filesystem probing from FTP MDTM

- `handle_mdtm()` previously called `str_stat()` on the raw client argument before `vsf_access_check_file()` or the share ACL check. The result controlled whether a date-like token was treated as an MDTM write timestamp or as a filename, giving unauthorised path existence influence over command parsing.
- Replaced that filesystem-dependent ambiguity check with strict RFC-style time-token grammar validation before any filesystem access. Date-only legacy tokens, 14-digit timestamps and fractional seconds remain supported; malformed calendar dates/times and malformed fractions fall back to ordinary filename handling. Actual reads/writes still use descriptor-relative ACL-checked opens and fd-based mtime operations.
- Added GNU89 ASan/UBSan policy coverage for leap years, month/day boundaries, clock ranges, fractional syntax, delimiters and missing paths; exact-head VSFTPD workflow pending.

---

## 2026-11-25 — Reject ambiguous IPv4 notation in WAN gateway ARP configuration

- Confirmed that `inet_aton()` accepts abbreviated, octal, hexadecimal and single-integer IPv4 forms. For example, `0300.0.2.1` is interpreted as `192.0.2.1`, even though the value looks like a decimal dotted address; the WAN gateway monitor could therefore probe a different host than the configured text indicates.
- Changed the parser to `inet_pton(AF_INET, ...)`, requiring an unambiguous four-octet dotted-decimal address. Empty and explicit `0.0.0.0` retain the legacy no-gateway behavior, and parse failures leave the output untouched.
- Extended the GNU89 gateway-address regression with leading-zero, octal, abbreviated, hexadecimal, integer and trailing-whitespace forms. A focused host GNU89 ASan/UBSan harness passed; exact-head WAN ARP and XY-C1 Actions remain the verification gate.

---

## 2026-11-25 — Free IA_NA configuration prefix lists during DHCPv6 teardown

- Confirmed a config-lifetime leak in BusyBox DHCPv6: `configure_ia()` stores IA_NA addresses in `iac->prefix_list`, but `clear_iaconf()` only reached `dhcp6_clear_list()` through the IA_PD-only `clear_pd_pif()` path. IA_NA list nodes and nested values were therefore abandoned on configuration cleanup.
- Moved prefix-list ownership cleanup into a type-aware shared policy used by both IA_NA and IA_PD, retaining the existing IA_PD-specific interface cleanup and avoiding duplicate list release. Added four GNU89 ASan/UBSan cases (IA_NA, IA_PD, unrelated type and invalid callback inputs), plus source-order/adapter contracts.
- Both push/PR filters for the DHCPv6 IA regression and XY-C1 firmware validation now track the new helper and fixture. Parent `e985875` has fresh Actions in progress; exact results on this new source commit are pending.

---

## 2026-11-25 22:24 CST (UTC+8) — Correct vanished-interface route teardown contract

- - Exact SHA `578c8647eec5e0c39bb4910d7757a0de5752468e`: Firewall startup #45632938361 PASS, USB modem #45632938668 PASS, AI docs #45632938331 PASS, Process lifecycle inventory #45632938233 PASS, XY-C1 #45632938514 PASS host validation but Build XY-C1/TRX/artifact were SKIPPED because this SHA did not change product inputs. WAN route #45632938327 failed only at the stale assertion `return clear_status;`: `wan6_down()` now accumulates the earlier SIT-route cleanup and metadata-clear errors into `status` and correctly returns that aggregate. This correction narrows the check to the vanished-interface branch and asserts both aggregation and `return status;`.
- The subsequent exact source SHA `263050a5762527a9ebe20560f2d15cc87cf15378` adds a real SIT route-gateway normalization fix: empty gateway values are now passed as NULL to route_add/route_del instead of a non-NULL empty string, which the BusyBox route API rejects. Two GNU89 tests cover empty and non-empty gateway arguments; source checks cover both add and delete paths.
- On `263050a`, current exact-head suite is queued/in progress: XY-C1 #45633281371, WAN route #45633280992, WAN DHCP #45633281034, NVRAM #45633281147, IPv6 service #45633280943, Process inventory #45633280920/#45633281057, USB modem #45633281299, Firewall #45633281250, HTTPD security #45633281195. XY-C1 must reach Build XY-C1, TRX verification and artifact upload on this exact SHA. No L3 or physical L4 claim yet.

---

## 2026-11-25 22:10 CST (UTC+8) — Fail closed on malformed LAN IPv6 address before NVRAM publication

- Found a concrete state-corruption bug in `reset_lan6_vars()`: `ipv6_from_string()` returns `-1` on invalid input, but the old code skipped address formatting and still published the uninitialized-empty `addr6s` value to NVRAM. A subsequent `reload_lan_addr6()` interprets empty as a request to flush the live global IPv6 address on `br0`; malformed configuration could therefore erase the working address.
- The function now checks the parser result and `inet_ntop()` status explicitly, propagates `EINVAL`/the conversion errno (or `EIO` fallback), and returns before NVRAM write. Prefix-format failures are also fatal and leave the prior configuration untouched.
- Added five GNU89 policy regressions for invalid parse, stale errno, `inet_ntop` failure with/without errno and valid prefix length zero. Extended XY-C1 modem checker and WAN-route source contract to prove conversion guard and early return precede `nvram_set_temp("lan_addr6", ...)`.
- Parent `40a77df4fbb3e41251d135994035ee9876c4d2a8` added SIT endpoint-route cleanup ownership assertions/tests. Exact Actions on that parent included XY-C1 #45631397674, WAN route #45631397687, WAN DHCP #45631397694, Process inventory #456313973? and other userland workflows; most were queued/in progress at last poll. This source-bearing commit should run current XY-C1 Build/TRX/artifact on one exact SHA. Physical router L4 remains NOT VERIFIED.

---

## 2026-11-25 22:02 CST (UTC+8) — Align modem/XY-C1 checker with LAN6 recovery API

- Exact d5 source SHA `d5adff9d7413b391d523d4b2c0d571d75de8b98a`: LAN IPv6 regression compiled and printed PASS; WAN route #45629160990, NVRAM #45629160447, IPv6 service type #45629160900, AI docs #45629161062, Process inventory #45629160410 and DNSMasq #45629160328 PASS. WAN teardown #45629160931 failed only on a stale snapshot API assertion, corrected in `3ea1835`; exact `3ea1835` WAN teardown #45629626346 PASS and AI docs #45629626161 PASS.
- The same stale call remained in `scripts/check_modem_wan_start_status.py:699`. It blocked XY-C1 #45629161139 before Build/TRX/artifact and Process fail-closed #45629160483. Updated the checker to require the recovery-aware apply/snapshot policy, snapshot-before-mutation, both recovery logs, argv-based `ip addr replace`, and all seven recovery fixture cases.
- No source change in this checker-only commit; XY-C1 may run host checks but its production-input detector can skip Build. Current-source L3 remains NOT VERIFIED; physical L4 NOT VERIFIED. The next source-bearing commit must run Build + TRX + artifact on one identical SHA.

---

## 2026-11-25 21:57 CST (UTC+8) — Update WAN teardown LAN IPv6 reload contract

- Exact-head WAN teardown #45629160931 on `d5adff9d7413b391d523d4b2c0d571d75de8b98a` failed in the final Python source-contract block because it still expected `rc_lan_addr6_reload_from_snapshot()`; production now uses the recovery-aware `rc_lan_addr6_reload_from_snapshot_with_recovery()`. The GNU89 LAN IPv6 policy fixture had already compiled and printed PASS in this same job; the failure was a stale assertion, not a C regression.
- Updated the teardown contract to require the recovery-aware snapshot/apply APIs and argv-based BusyBox `ip addr replace` restoration. Added `net_lan6.c`, the policy header and fixture to the workflow's internal push/PR path assertions. No production source changed in this follow-up.
- On `d5adff9`: Process lifecycle inventory #45629160410, IPv6 service type #45629160900, AI docs #45629161062 and DNSMasq #45629160328 PASS; WAN teardown #45629160931 failed as described; WAN route #45629160990, XY-C1 #45629161139, NVRAM #45629160447 and Process fail-closed #45629160483 were still in progress at last poll. XY Build/TRX/artifact were pending. Current-source L3 is NOT VERIFIED; physical L4 is NOT VERIFIED.

---

## 2026-11-25 21:55 CST (UTC+8) — Recover LAN IPv6 address after failed reload

- Found a second destructive-update gap in `reload_lan_addr6()`: it flushes all global addresses on `br0`, then adds the configured NVRAM address. If the flush partially fails or the add fails, it previously returned an error without making a compensating attempt to preserve/recreate the configured address.
- Added a recovery-aware policy that retains the primary clear/add error and reports recovery status separately. Recovery uses BusyBox 1.24's verified `ip -6 addr replace <address> dev br0` operation; an already-present address is replaced idempotently, while a missing address is recreated. A failed NVRAM snapshot still blocks clear/add/recovery side effects.
- Added seven GNU89 regression cases for recovery after clear failure, recovery after add failure, independent recovery-error reporting, empty configuration, callback preflight, NVRAM read failure and normal success ordering. Updated WAN-route source contracts and XY-C1 source checks; added `net_lan6.c` to the XY-C1 product-input filter so this production change triggers Build/TRX/artifact validation.
- Main parent `7a9bdb35a879be1dfae3bbc8277b2a6a845249cf` follows `103e7355`'s fix for duplicate SIT endpoint host routes. On `103e7355`: WAN route #45627761309 PASS, AI docs #45627761272 PASS, Process inventory #45627761497 PASS, IPv6 service #45627761403 PASS and HTTPD security #45627761288 PASS; WAN DHCP #45627761300 failed only on an outdated `WAN6_SIT_CHECK(route_add(...))` assertion, corrected in `7a9bdb3`; WAN DHCP #45628195224 and AI docs #45628195241 PASS on `7a9bdb3`.
- On `103e7355`, NVRAM #45627761287 and XY-C1 #45627761382 were in progress at the last poll (Build active; TRX/artifact pending). Earlier checker-only XY-C1 #45627044645 host validation passed but Build/TRX/artifact were skipped because its change set lacked a production source diff. Historical L3 on `0bed846` does not validate SIT or current LAN reload code; no current-source L3 or physical L4 claim.

---

## 2026-11-25 21:44 CST (UTC+8) — Synchronize duplicate WAN6 command-count checks

- Two additional exact-head validations on `0ae462027968d0d31ddf9c349cd0b5f15ab0cdc7` failed on the same stale pre-SIT source-count assertion: XY-C1 #45626020329 at `scripts/check_modem_wan_start_status.py:744`, and WAN DHCP callback #45626020186 in its embedded source contract (workflow line 1472). All earlier C regression steps in those jobs passed; XY-C1 Build/TRX/artifact steps were skipped because host validation stopped at that contract.
- The source now has four argv-based `ip -6 addr add` sites after adding the rollback adapter. Updated both checks to expect four and explicitly require the saved address be passed as a single argument to `ip` on `br0`. The route workflow check had already been updated to four in `92578d78` and its YAML indentation corrected in `14bbc0a`.
- L3 record: XY-C1 run #45624855182 on exact source SHA `0bed846b6ac67f8f77f0104f06decddcaa62ce1a` completed successfully, including Build XY-C1, TRX verification and artifact upload. This does NOT validate later SIT source commit `0ae4620`; on `0ae4620`, XY-C1 host validation failed before build/TRX/artifact.
- Exact reruns of WAN route, WAN DHCP and XY-C1 are required for the current source; no physical-router L4 claim.

---

## 2026-11-25 21:43 CST (UTC+8) — Fix YAML block indentation in WAN route contract

- Review of the follow-up commit `92578d786df806f9da1f68d0b6ce835b0c397207` caught a formatting error in the newly added embedded Python assertion: it had been inserted at column zero instead of the ten-space indentation required by the YAML `run: |` block. This made the workflow file invalid; corrected the indentation here.
- The earlier exact WAN route failure on `0ae4620` remains attributed to the old hard-coded command count (3 vs the new 4); this commit now contains both the count update and properly nested argv-safety assertion. Require a fresh exact-head workflow validation before claiming PASS.
- XY-C1 run #45626456137 on `92578d7` is IN_PROGRESS with Build pending at last check; TRX and artifact upload are pending. No L3 claim.

---

## 2026-11-25 21:42 CST (UTC+8) — Align IPv6 source contract with SIT LAN restore callback

- Exact-head WAN route run #45626020132 on `0ae462027968d0d31ddf9c349cd0b5f15ab0cdc7` failed only in the final embedded source-contract step. All preceding 52 steps, including the new WAN IPv6 identity test compile and `check_wan_vlan_setup.py`, passed.
- The failure was a stale hard-coded count: the route workflow expected three argv-based `ip -6 addr add` calls in `net_wan6.c`, while the new safe LAN-address restoration callback correctly adds a fourth. Updated the expected count and added an explicit assertion that the restoration callback passes the saved address as one argv argument on `br0`. No production code changed in this follow-up.
- This correction needs fresh exact-head WAN route validation. On source SHA `0bed846b6ac67f8f77f0104f06decddcaa62ce1a`, all listed stability workflows and NVRAM audit #45624855071 PASS; XY-C1 #45624855182 was still building when last inspected, with TRX/artifact pending. L3 not yet claimed.

---

## 2026-11-25 21:39 CST (UTC+8) — Restore prior LAN IPv6 address when SIT activation fails

- Found a concrete rollback hole in `start_sit_tunnel()`: after the tunnel and routes were ready, the function flushed all global addresses from `br0` and attempted to add a derived 6to4/6rd LAN address. If the clear partially failed, the add failed, or publishing the new address to NVRAM failed, the common failure path removed the incomplete SIT tunnel but never restored the previously configured `lan_addr6`. The old address could therefore be lost after a failed WAN IPv6 transition.
- The code now performs a checked snapshot of `lan_addr6` before touching bridge addresses, marks the address transaction before the flush, and restores the previous configured value through argv-based `ip -6 addr add` after clearing partial state on any subsequent failure. Empty prior configuration is preserved as empty; rollback errors are logged separately and do not replace the original SIT startup failure.
- Added a callback-driven GNU89 policy and regression cases for restoration ordering, empty snapshot, clear/add failure aggregation, untouched state and invalid callbacks. Source-order checks are added to `scripts/check_wan_vlan_setup.py`; existing WAN route workflow already includes the touched source, policy, test and checker paths. Exact-head Actions pending.
- On parent `0bed846b6ac67f8f77f0104f06decddcaa62ce1a`: WAN route #45624855225, Wireless #45624855188, WAN DHCP #45624855254, Process inventory #45624855312, Process fail-closed #45624855157, WAN sysfs counters #45624855211, USB modem #45624855066, USB storage #45624855128, AI docs #45624855327 and IPv6 service type #45624855120 PASS. NVRAM #45624855071 and XY-C1 #45624855182 remained IN_PROGRESS at last poll. No same-SHA XY-C1 Build/TRX/artifact completion or physical L4 claim yet.

---

## 2026-11-25 21:31 CST (UTC+8) — Harden bridge M2U errno and snapshot validation

- Reviewed the M2U transaction after its harness corrections. Linux 4.4 exposes both bridge-port flags as boolean 0/1 values; the policy now fails closed with EPROTO if readback violates that kernel contract.
- The production setter adapter normalizes positive status-code errors and negative/-1 failures into -1 with a reliable errno, clearing stale errno before each write. The sysfs reader initializes its output on failure.
- Added GNU89 cases for rollback after the first setter itself fails and for impossible non-boolean snapshots. Existing second-write rollback, reverse-order restore and primary-error preservation cases remain.
- On parent `7192958c21ddce8798b8793c9b34df34717ed15d`, WAN route #45624479165 and AI docs #45624479128 were IN_PROGRESS at last poll. This follow-up requires fresh exact-head checks; no XY-C1 L3 or physical L4 claim is made.

---

## 2026-11-25 22:12 CST (UTC+8) — Correct M2U rollback-order checker symbol

- Exact-head WAN route #45624201337 on `fb86393c` passed the ASan/UBSan policy test binaries and failed only in the Python source checker: the declaration was named `m2u_restore_second`, but the ordering assertion referenced undefined `m2u_second_restore`.
- Corrected the identifier in the assertion. Wireless radio #45624201448 passed on the same `fb86393c` after the missing `<assert.h>` include. Exact-head WAN route and XY-C1 still need a green run on the next code-bearing SHA.

---

## 2026-11-25 22:08 CST (UTC+8) — Add assert declaration to bridge M2U regression

- Exact-head WAN route #45623857230 and wireless radio #45623856807 on `d9d23fe` both failed at compile time because new bridge M2U regression cases used `assert()` without including `<assert.h>`; `-Werror=implicit-function-declaration` stopped compilation. The existing bridge tests use manual expect helpers and did not previously need this header.
- Add the standard assertion header to the bridge membership test. No production logic changed. Both workflows should rerun on the corrected SHA; exact XY-C1 #45623856954 was still running at last poll.

---

## 2026-11-25 22:02 CST (UTC+8) — Make bridge M2U sysfs updates transactional

- Found a partial-update bug in `brport_set_m2u()`: it writes `multicast_fast_leave` then `multicast_to_unicast`; the second sysfs write can fail after the first has changed, leaving a partially applied M2U setting while returning an error.
- Added a checked sysfs reader for both bridge-port flags and a policy transaction that snapshots both values before any write. It tracks attempted writes, rolls them back in reverse order after a failure, preserves the primary errno, and separately reports the first rollback failure. Production logs an incomplete rollback through syslog.
- Added GNU89 regression coverage for snapshot-before-mutation, second-write failure, rollback failure/primary errno preservation, snapshot failure with no writes and idempotent target state. Existing WAN route workflow already compiles/runs the bridge membership policy fixture and invokes the VLAN/bridge source checker.

---

## 2026-11-25 21:51 CST (UTC+8) — Fix VLAN checker local-name collision

- Exact WAN route run #45622867970 on `134638e4` passed all sanitizer test binaries, including the corrected VLAN rollback test. The Python source checker then failed because an older section reused the name `create_vlan` for an integer offset, shadowing the function-body string expected by the new assertion.
- The appended assertions now use a distinct `vlan_create_body` variable from `body(net, ...)`. No product source or runtime test changed; exact-head route rerun required.

---

## 2026-11-25 21:48 CST (UTC+8) — Correct VLAN rollback mock capacity

- Exact WAN route run #45622569505 on `64bd4201` compiled the policy fixtures, but UBSan caught the new VLAN restore mock's pointer array as length 3 while the successful rollback path records 4 operations (down, MTU, MAC, final up). No production code was reached by this fixture failure.
- Corrected the mock array to four slots, recorder bound to four, and final restore-order assertion to index 3. Re-run the complete exact-head WAN route workflow; the source checker follows the sanitizer fixture.
- XY-C1 #45622569603 was still in progress on `64bd4201`; the correction commit triggers a fresh target build.

---

## 2026-11-25 21:43 CST (UTC+8) — Make reused VLAN rollback compatible with target 4.4 VLAN ioctl

- Kernel-source inspection found this repository's Linux 4.4 `vlan_ioctl_handler()` does not implement `GET_VLAN_EGRESS_PRIORITY_CMD` (it returns `-EOPNOTSUPP`). A snapshot design relying on that ioctl would make every existing VLAN with requested QoS fail closed.
- Reworked the transaction to snapshot only supported MTU and hardware address status readers plus the original IFF_UP bit. The egress setter is now deliberately the final configuration step. In this kernel, `vlan_dev_set_egress_priority()` changes an existing mapping and returns success, or for a new mapping allocates first and returns `-ENOBUFS` before publishing on failure; thus a failed setter leaves the previous mapping unchanged and, being last, there is no subsequent failure that needs a GET/restore operation.
- The policy rollback restores down state, requested MTU/MAC, then original admin-up state. Tests and semantic checker are adjusted to verify the target kernel's missing GET API and setter atomicity. Exact-head WAN route sanitizer workflow pending.

---

## 2026-11-25 — Fix VLAN rollback test compile error
- WAN route status #45621857629 failed at the VLAN policy test compile because it used strcmp() without declaring it. Added the missing <string.h> include; source logic unchanged.

---

## 2026-11-25 21:34 CST (UTC+8) — Restore reused VLAN attributes after failed setup

- Found a real rollback defect in `create_vlan_iface_internal()`: a pre-existing VLAN was taken down and reconfigured (egress priority, MTU, MAC), but the failure path only removed newly created VLANs. A reused VLAN could remain down or partially changed.
- Added checked snapshots for IFF_UP, requested MTU/MAC and skb-priority-0 egress QoS before mutation. On failure, rollback takes the reused VLAN down, restores captured attributes, and restores original up/down state. It attempts all rollback operations, logs first rollback error, and preserves primary setup errno. New VLAN deletion/ownership behavior is unchanged.
- Added GNU89 policy regression for restore order, continued attempts after errors and skipped untouched settings; extended WAN VLAN source checker. WAN route workflow already filters the source, shared netutils, VLAN policy/test and checker. Exact-head checks pending; no physical-router L4 claim.

---

## 2026-11-25 21:21 CST (UTC+8) — Fix DHCPv6 startup-cleanup checker assertion

- Exact-head WAN DHCP callback #45620059391 on `28a6bb0` passed the UBSan/ASan policy binaries and all preceding WAN/IPv6 checks, then failed only at the new `check_wan6_dns_policy.py` source assertion. The checker incorrectly searched for the helper's formal parameter names in the call site; production correctly calls it with `start_status, env_cleanup_status`.
- The assertion now inspects the helper body and pins the actual `start_status == 0 && cleanup_status != 0` decision. No production code or test behavior changed. Rerun required on current SHA.
- IA publication #45620275164 and AI docs #45620275432 PASS on parent `94c75e4`. Exact-head callback check remained unverified until this rerun.

---

## 2026-11-25 21:18 CST (UTC+8) — Correct DHCPv6 IA output contract

- Exact-head IA workflow #45619758139 on `6a1faf201e57eab3e9757cc845c15a779b8e8baa` passed the GNU89 ASan/UBSan regression, including failed-prefix-delete retention and retry. The Python source checker then failed only because it still required the old `DHCPv6 IA delegated-prefix retry policy: PASS` banner; the test now correctly emits `DHCPv6 IA delegated-prefix retry and removal policy: PASS`.
- Removed only that obsolete duplicate banner assertion. The checker still pins both new cleanup regressions and the new exact output banner. Exact-head rerun on the current commit is required; no production C changes in this checker-only correction.

---

## 2026-11-25 21:14 CST (UTC+8) — Stop DHCPv6 client after failed environment cleanup

- Base SHA: `6a1faf201e57eab3e9757cc845c15a779b8e8baa`.
- Found a resource/state leak in `start_dhcp6c()`: `eval("/sbin/dhcp6c", ...)` could launch the background client successfully, then `dhcp6c_callback_environment_clear()` could fail. The function converted this to startup failure and advanced the callback generation, but never stopped the already-running `dhcp6c`; callers could roll back IPv6 setup while the client remained alive.
- Added `dhcp6c_callback_start_requires_stop()` plus regression cases to distinguish failed launch from successful launch + cleanup failure. On that latter path, `start_dhcp6c()` now advances the generation then invokes the same bounded `kill_services("dhcp6c")` stop under the held session lock, logs stop failure, and only then unlocks. A failed launch still avoids unnecessary stop.
- Extended the existing DHCPv6 semantic source checker. Workflow `wan-dhcp-callback-stability.yml` already tracks the modified source, policy, test and checker. Exact-head CI required; no live-router test claimed.

---

## 2026-11-25 — Retain failed DHCPv6 address removals for retry
- remove_iaprefix() previously discarded IA_PD target records even when kernel deletion failed; IA_NA failures were ignored by cleanup callers. pref_cleanup() also unlinked the same entry twice. Stale IPv6 addresses could remain installed without retry state.
- Cleanup now retains failed targets, schedules bounded retry, keeps the IA/controller allocated until all deletes succeed, and resumes pending IA cleanup after each prefix timer. Already-absent deletes are idempotent; EEXIST is accepted only for ADD.
- Added host errno-classification/selective-removal regression and production source-contract checks. Exact-head Actions and XY-C1 Build/TRX verification pending.

---

## 2026-11-25 20:59 CST (UTC+8) — Clamp expired udhcpc select waits

- Base SHA: `aa1bdf567d3100f5e773bdd9b41235cc85346bbc`.
- Confirmed a signed/unsigned arithmetic defect in the BusyBox udhcpc main loop: after select() returns EINTR, `already_waited_sec` is increased. If it reaches or exceeds the signed `timeout`, `timeout - already_waited_sec` underwent unsigned subtraction and could produce a huge positive wait instead of processing the expired DHCP transition.
- Added `udhcpc_wait_remaining_seconds()`, which saturates expired/non-positive budgets at zero before subtraction, and routed `timeval.tv_sec` through it. Added GNU89 regression boundaries for elapsed less than/equal to/greater than timeout, UINT_MAX-like elapsed input, negative/zero timeouts and INT_MAX.
- Updated the existing source contract. The existing udhcpc workflow already filters the changed source, policy header, test and checker; the XY-C1 workflow already filters the source and policy header.
- Local isolated GNU89 ASan/UBSan policy smoke test passed. Full repository regression and target build are delegated to exact-head Actions; no complete-repository local test or firmware result is claimed.
- Pre-change current-main evidence: DHCPv6 IA #45618045032 PASS and AI handoff #45618045228 PASS on `aa1bdf5`; XY-C1 #45617826332 remained in progress on `a319ffe`. New exact-head udhcpc and XY-C1 workflows required.
- Resulting source/test/checker/docs commit: THIS COMMIT. Physical-router L4 remains unverified.

---

## 2026-11-25 — Resolve DHCPv6 checker definition lookup
- The latest IA workflow's GNU89 ASan/UBSan regression passed, but its static checker matched the client6_recvreply forward declaration. It now anchors on the full definition signature, consistent with the production callback checker.
- Exact-head AI docs, DHCPv6 script retry and WAN callback workflows passed after the compact roadmap and legacy declaration fixes. XY-C1 Build/TRX and NVRAM audit remain in progress.

---

## 2026-11-25 — Correct DHCPv6 regression contracts
- First exact-head iteration exposed checker-only failures after the GNU89 ASan/UBSan IA test passed: helper-definition lookup and an incorrect Reply-state anchor. The existing DHCPv6 script checker also depends on the original local declaration; retain it and declare IA status separately.
- AI handoff validation caught the 128 KiB roadmap hard limit. Replaced the oversized status paragraph with a compact entry. Re-run exact-head Actions after this correction.

---

## 2026-11-25 — Retry failed DHCPv6 IA kernel publication

- DHCPv6 IA_NA address-add and IA_PD downstream-prefix-add failures were logged and swallowed by update_ia(); the Reply was then considered handled and the configuration callback could run.
- IA_PD installation was gated on first creation and ignored add_ifprefix() status. If one downstream interface failed after another succeeded, later Replies skipped the missing interface permanently.
- update_ia() now returns the first kernel-configuration error. client6_recvreply() keeps the exchange alive and retries the same Reply after the existing bounded 5-second delay before running the callback. IA_PD reconciles every configured downstream interface on each Reply, skipping successful entries and propagating missing-target failures.
- Added a GNU89 ASan/UBSan partial-failure/selective-retry/idempotence regression, production-source checker and dedicated Actions workflow. Local host regression passed; exact-SHA Actions and XY-C1 Build/TRX/artifact verification are pending.
- Added dhcp6c_ia.c and its policy header to the XY-C1 firmware path filter, which previously did not rebuild the IA implementation when changed.

---

## 2026-11-25 — Point U2EC send assertions at bounded implementation

- Exact U2EC run #45610564863 on `e258dbf3` compiled and passed all GNU89 ASan/UBSan socketpair tests, including the new non-reading-peer timeout case. The production source checker then failed three legacy assertions because `send_policy` now correctly represents the compatibility wrapper, while `MSG_NOSIGNAL`, EINTR handling and the sent-byte return are in the new `send_timeout_policy` helper.
- Moved those three assertions to the bounded helper. Runtime code and tests are unchanged.
- Exact-head U2EC, storage, AI handoff and XY-C1 build/TRX/artifact checks required.

---
## 2026-11-25 — Bound U2EC client TCP sends as well as reads

- The main U2EC loop also used blocking `u2ec_send_all_no_sigpipe()` for client greetings and responses. A peer that stopped reading could fill the TCP send buffer and stall the same sole event loop, even after receive-side deadlines were added.
- Replaced the unbounded send loop with select-driven nonblocking sends under a 10-second absolute deadline. The compatibility wrapper retains the existing API and `MSG_NOSIGNAL` behavior; timeout and EPIPE remain ordinary errors handled by the existing disconnect path.
- Added successful socketpair delivery and a non-reading-peer send-buffer exhaustion regression; source-contract checks require all sends through the bounded wrapper. Exact U2EC ASan/UBSan/source-contract and XY-C1 validation pending.

---
## 2026-11-25 — Trigger MTD fill regressions for pull requests

- Exact USB-storage run #45609688415 on `eed1e197` passed the host-shell and BusyBox ash fill regressions and surrounding source contracts, but the workflow checker correctly found that `mtd_storage_fill_safe_paths.sh` was listed under `push.paths` and omitted from `pull_request.paths`.
- Added the regression script to the pull-request path filter beside the other MTD lifecycle tests. PRs changing the test or MTD fill contract will now trigger the same storage stability workflow.
- Exact-head storage and AI handoff workflows must be green before this slice is complete.

---
## 2026-11-25 — Fix storage checker filter scope and rotate log window

- Exact USB-storage run #45608745837 on `fa7d05e5` passed shell syntax, host-shell and BusyBox ash MTD fill regressions, then the source checker raised `NameError` because its early MTD contract referenced `push_filters`/`pull_filters` before the workflow-filter variables were defined. Moved this path assertion into the existing later push/pull filter inventory.
- The AI handoff validator also reported 121 root log entries. Archived the 101 oldest complete records byte-for-byte, chronological oldest-to-newest, and retained the newest 20 before this correction entry. The pre-rotation root blob SHA and archive shard are indexed.
- Exact-head storage and AI handoff reruns required. Product source was unchanged in this checker/governance correction.

---
## 2026-11-25 — Align U2EC framing source contract with bounded reads

- Exact U2EC run #45608574671 on `0c505e0b` compiled the GNU89 policy test with ASan/UBSan and passed both complete-field and partial-field timeout regressions. The final source checker failed only because three pre-existing assertions still required the old unbounded `u2ec_recv_all()` call sites in `usb_connection()`.
- Updated the production-source contract and read-order assertion to require `u2ec_recv_all_timeout()` for both the fixed header and IRP body. No runtime source changed in this follow-up.
- Exact-head U2EC, storage, and XY-C1 checks remain required.

---
## 2026-11-25 — Fix literal backslash count in chnroute source contract

- Exact USB-storage run #45608329431 on `14ec3f27` passed runtime MTD fill regressions but the checker-only chnroute rewrite assertion failed: the test shell source contains one backslash before each escaped quote, while the Python raw-string expectation contained two.
- Corrected the raw-string expectation to one literal backslash. This calibrates only the source contract; MTD runtime source and fixtures are unchanged.
- Exact-head storage rerun required.

---
## 2026-11-25 — Bound U2EC client TCP field reads

- `usb_connection()` runs synchronously in the sole U2EC event loop. Its blocking `u2ec_recv_all()` calls could wait indefinitely after a peer sent only part of the fixed TCP header or advertised IRP body, preventing other printer clients, FIFO events and timeout processing from running.
- Added a 10-second absolute deadline per incoming TCP field. Readiness waits use the remaining deadline and nonblocking recv, so partial progress does not reset the total budget; timeout returns `ETIMEDOUT` and the existing close path removes the stalled connection. Header and IRP-body reads use the bounded helper; discovery worker reads retain their separate one-second socket timeout.
- Added successful complete-field and stalled-after-partial-field socketpair regressions plus source-contract checks for the main event-loop call sites. Exact U2EC host ASan/UBSan/source-contract and XY-C1 Build/TRX/artifact verification pending; no device result claimed.

---
## 2026-11-25 — Match quoted chnroute fixture rewrite literally

- Exact storage run #45607584405 on `0cc1703c` passed shell syntax, host-shell and BusyBox ash MTD fill regressions, checked caller graph and preceding assertions. The final static checker failed because the sed substitution in the shell fixture escapes its inner double quotes, while the Python source contract expected the unescaped form.
- Updated the contract to match the literal escaped shell text. No production source or runtime test behavior changed.
- Exact-head storage workflow rerun required.

---

## 2026-11-25 — Align chnroute source contract with checked directory helper

- Exact USB-storage run #45607146427 on `72f9d8fa` passed shell syntax validation and both host-shell and BusyBox ash MTD fill regressions, including injected write/chmod/rename failures. The static checker then failed on a stale assertion that still expected raw `mkdir -p`; production code now correctly uses `func_fill_mkdir "$dir_chnroute" 755` to validate the path and propagate directory-creation errors.
- Updated the source contract to require the checked helper while retaining assertions for archive extraction failure cleanup. No production source or test behavior changed.
- Exact-head storage workflow rerun required.

---

## 2026-11-25 — Correct MTD fill function source-contract delimiter

- Exact USB-storage run #45606836538 on `da508394` passed the MTD shell syntax and host/BusyBox runtime regressions, then the static checker failed because its Python string searched for the literal characters `\\ncase` rather than the actual function boundary `\n}\n\ncase "$1" in`.
- Corrected the delimiter so the checker slices the complete `func_fill()` body through its closing brace. Production MTD behavior and test fixtures are unchanged.
- Exact-head USB storage rerun is required; the earlier checker failure is a test-contract error, not a firmware/runtime failure.

---

## 2026-11-25 20:40 CST — Include the string API used by SOAPAction tests

- Exact miniupnpd HTTP request framing run #45606537043 on `1f5e34e7` failed before executing tests: the new quoted-value assertion used `memcmp()` without including `<string.h>`, and the workflow treats implicit declarations as errors under GNU89 `-Werror`.
- Added the standard declaration include to the test translation unit. The parser change itself has not yet completed CI validation; exact-head build/test rerun required.

---

## 2026-11-25 20:35 CST — Extract MTD fill source without parsing heredoc bodies as C

- Exact USB-storage run #45605993018 on `1c62feb6` passed both host-shell and BusyBox ash MTD save/restore/load/erase/backup/fill regressions, including all eight injected fill failures and the normal success case. The final static checker failed when the C-style `body()` scanner tried to parse the shell `func_fill()` function: heredoc payloads contain shell braces and quoting that are not C source, so the scanner could not find a balanced closing brace.
- Replaced only the `func_fill()` extraction with its stable shell boundary: the function starts at `func_fill()` and ends at the top-level `case "$1" in` command dispatcher. The smaller helper functions continue using the shared brace parser. Runtime MTD source and tests are unchanged.
- Exact storage rerun required; `1c62feb6` is FAIL solely at the static checker after the runtime regression steps passed. No local or hardware result is claimed.

---

## 2026-11-25 20:30 CST — Reject malformed quoted MiniUPnP SOAPAction values

- `ParseHttpHeaders()` stripped surrounding quotes by testing `p[n-1]` after finding a value length. A one-byte value consisting only of a quote satisfied both ends of the check and reduced the signed SOAPAction length from 1 to -1; downstream `%.*s` logging then treated the negative precision as unbounded. The parser also retained trailing whitespace and scanned the line without an explicit header-bound check.
- Added a length-safe SOAPAction normalizer that trims optional edge whitespace, rejects unbalanced/interior quotes and embedded control bytes, and returns an offset/length pair without modifying outputs on failure. The production parser now bounds the value scan by `req_contentoff`; Content-Length scanning is bounded the same way. Added GNU89-compatible boundary tests for one-byte quotes, mismatched quotes, empty quoted value, whitespace and control characters.
- Exact MiniUPnP framing Actions required. No local build or hardware result is claimed.

---

## 2026-11-25 20:35 CST — Publish MTD default files atomically

- The latest MTD fill guard now detected failed heredoc writes and chmod errors, but wrote directly to live config paths before checking status. A partial file remained at its final name after ENOSPC/I/O failure; subsequent fills would skip it because the pathname already existed.
- Added same-directory temporary staging for generated scripts and configuration, verified the staged file's type/mode, then published with no-clobber rename. Failures remove the stage and propagate to reset/fill callers. DNSMasq base content and all optional overlays publish as one file; existing user config is not replaced.
- Extended host-shell and BusyBox ash regressions to inject content-write, chmod and rename failures and verify no partial target/stage remains. Exact USB-storage and XY-C1 Build/TRX/artifact workflows required; no local hardware result is claimed.

---

## 2026-11-25 20:25 CST — Teach the storage checker to parse shell parameter expansions

- Exact USB-storage run #45605321376 on `6dd7052c` passed the runtime restore/load/erase/backup tests and the pre-existing full-caller-graph contract, then the static checker failed while extracting `func_fill_check_file()`. Its C-style brace scanner interpreted the shell parameter expansion `${target%/*}` as the start of a C block comment and reached EOF without a closing delimiter.
- Extended the shared function-body extractor to skip nested `${...}` shell parameter expansions, and added a focused parser regression containing the slash-star pattern. This is a checker-only correction; MTD production/test behavior is unchanged.
- The exact `6dd7052c` storage run is FAIL due to checker parser incompatibility; exact-head rerun required. No local test or firmware result is claimed.

---

## 2026-11-25 20:20 CST — Fail MTD fill on default-file write or mode errors

- The 23 default-file heredoc writes and their `chmod` operations were unchecked. A full filesystem, read-only transition or write error could leave an empty/partial configuration file or a non-executable hook while `func_fill()` continued and reset started services.
- Added immediate status checks after every default-file `cat >`/`cat >>`, plus a common file-type/symlink/mode helper that propagates each chmod error. Added host/BusyBox ash injection for heredoc write and mode-setting failures; static checker pins all 23 heredoc status gates and representative mode operations.
- Exact USB storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 20:15 CST — Isolate clean fill from the corrupt archive fixture

- Exact USB-storage run #45604771628 on `13f9f431` passed restore/load/erase/backup, the four path-symlink denials, the injected mkdir failure, the reset_ss failure case, and the chnroute partial-extraction failure. The final positive-path assertion failed because the negative chnroute test deliberately left a plain-text invalid archive at the fixture path; the subsequent clean-fill case reused it, so real `tar jxf` correctly rejected it. The production source and mkdir wrapper were not at fault.
- The successful-fill fixture now removes the deliberately corrupt archive before running against a clean storage tree. Production source is unchanged.
- Exact storage run is FAIL due to test-state leakage between scenarios; new exact-head validation required. No local test or hardware result is claimed.

---

## 2026-11-25 20:10 CST — Point the chnroute failure fixture at its test archive

- Exact USB-storage run #45604235661 on `5dd0dcf0` passed restore/load/erase/backup and the preceding fill symlink and mkdir-failure cases, but the injected chnroute failure case unexpectedly succeeded because the test's sed pattern started at column zero while the production `chnroute_file` assignment is indented inside `func_fill()`. The test therefore left the production path `/etc_ro/chnroute.bz2` unchanged and skipped extraction.
- Replaced the fixture with an exact assignment-value substitution that preserves the indentation, and pinned the redirection in the source contract. Production code is unchanged.
- Exact storage and XY-C1 validation required; the 5dd0dcf storage run is FAIL due to fixture path injection, not a successful archive-failure test. No local test result is claimed.

---

## 2026-11-25 20:05 CST — Verify the safe default-fill success path

- The new fill regression already exercised four fail-closed cases; it lacked a clean-tree success case, so an overly strict path preflight or a shell portability issue could have made every fill fail while all negative assertions passed. The reset_ss fixture now publishes a valid start script on success, and the test verifies the core generated scripts and dnsmasq/inadyn/WLAN defaults under both host shell and BusyBox ash.
- Test-only expansion; the storage workflow and source contract are unchanged apart from requiring the positive-path assertion. Exact storage run required; no local test result is claimed.

---

## 2026-11-25 20:00 CST — Require successful reset_ss publication during fill

- Default fill previously invoked `reset_ss.sh -a` without checking its result or confirming that the expected `start_script.sh` was published. A write/rename failure could be hidden while fill returned success and reset proceeded to service startup.
- Fill now stops on a failing generator and independently verifies that the resulting start script is a regular non-symlink file. Added a host/BusyBox ash failure-injection case and static source-contract assertions.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:55 CST — Propagate required MTD fill directory-creation failures

- MTD default fill ignored several `mkdir -p` failures for cron, HTTPS, dnsmasq, inadyn, WLAN, OpenVPN and strongSwan state. The function could continue with an incomplete configuration tree and let reset proceed to application startup. Added a checked mkdir helper that revalidates the path, preserves existing-directory modes, logs failures, and aborts fill; migrated all required default directories to it.
- Extended the host/BusyBox ash fill regression with injected failure at the first required cron directory, asserting nonzero fill status and that reset-script generation never begins. Static source contract covers the directory call sites.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:50 CST — Propagate MTD fill failures to the command exit status

- The new path guard correctly returns failure from `func_fill()`, but the command dispatcher previously invoked it and then exited the script with the unchanged global `result=0`. Direct `mtd_storage.sh fill` callers therefore saw success even when a protected symlink or archive extraction failure aborted fill. The reset path already propagates the function result through its conditional.
- The `fill)` dispatcher now records a nonzero result when `func_fill` fails; the static contract pins this status propagation. The existing production-linked host/BusyBox fill regression checks that unsafe paths and partial chnroute extraction return nonzero.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:45 CST — Preserve shell parameter expansion in fill path guards

- Exact USB-storage run #45603292894 on `dfee708a` passed the earlier MTD restore/load/erase/backup regressions, then failed when the new fill-path test tried to create its sandbox: literal backslashes had been retained before `${TMPDIR}` and `${BUSYBOX_BIN}` in the generated shell fixture. The source helper's `${target}`/parent/component parameter expansions had the same quoting defect and would not walk path components correctly.
- Removed the unintended backslashes from the three production shell expansions and both test expansions; strengthened the static source contract to pin the parameter expansions themselves. Exact storage and XY-C1 validation required.
- The prior exact-head storage run is FAIL due to fixture/bootstrap expansion, not a passing result for the new fill guard. No local test or firmware result is claimed.

---

## 2026-11-25 19:40 CST — Make bundled chnroute extraction fail closed

- The MTD default-fill path used `mkdir -p ... && tar jxf ...` inside a conditional but discarded the compound command's failure. A corrupt bundled chnroute archive could leave a partial `/etc/storage/chinadns` directory; subsequent boots would see the directory and never retry extraction.
- Fill now checks directory creation, checks archive extraction, removes partial output on extraction failure (logs a critical cleanup failure if necessary), and returns failure so reset retains its save barrier and does not restart apps. Extended the host/BusyBox ash fill regression with injected partial extraction and cleanup assertions.
- Exact USB storage, static source contract and XY-C1 build verification required. No local BusyBox/firmware/hardware result is claimed.

---

