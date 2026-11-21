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

