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
