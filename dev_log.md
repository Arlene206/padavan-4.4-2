## 2026-11-20 — Rotate the root development log and refresh the verification checkpoint

- Parent main SHA: `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49`.
- The root development log contained 120 complete dated records, and exact-head AI handoff run `44693247011` correctly failed its rolling-window guard. Archived the oldest 100 complete records, oldest-to-newest and byte-for-byte, in `docs/dev-log/2026/11/part-019.md`; retained the newest 20 records in the root before adding this checkpoint.
- Archive source root blob: `eb7190fedf832884fdaf9b48045223a91dcdcb22`; archive size: 114230 bytes; archive blob: `bf5f91f6e47631f2f1331a3be3dd46c3515d4a89`. No archived record was rewritten or omitted.
- The WAN callback source contract on `ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1` failed only at a stale workflow assertion requiring a direct ZeroConf interface-clear call after rollback had been centralized. Commit `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49` removes that obsolete assertion and pins the shared restore helper's set/clear branches; exact-head callback rerun `44693740862` was queued at the last poll.
- On `ee8ef67`, WAN route, HTTPD, AI docs, NVRAM boundary and the other completed host checks passed. XY-C1 firmware validation run `44693084076` was still in progress at the last poll; MIPS build/TRX/artifact and L3 remain unverified.
- Next source task: make IPTV MAN route-inventory and IPTV/ZeroConf-IPTV deconfig metadata clears transactional and status-returning. A failed NVRAM publish must remain visible to the deconfig retry path; add injected tests for partial writes, rollback and retry ordering before changing the callers.

---

## 2026-11-20 — Prevent truncated IPv4 rollback values across DHCP and ZeroConf callbacks

- Parent main SHA: ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1; commit under verification: ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1.
- Found a defensive rollback bug in all four bound callbacks: previous IP/netmask/gateway text was captured in larger snapshot storage then copied into INET_ADDRSTRLEN buffers without a fit check. An overlong corrupt string beginning with a valid IPv4 literal could be truncated to that different address and used for interface/route rollback. Added dhcp_lease_text_fits_buffer(); each callback now rejects an oversized old address before interface or route mutation.
- Added helper tests for empty, exact-fit, valid-prefix-overflow, NULL and zero-capacity values. Added an 11-field injected NVRAM regression: a setter that writes then returns ENOSPC must not stop remaining writes, and all old tuple fields must be restored in reverse order.
- Also made WAN callback and WAN route source-body extractors skip C prototypes/call sites so the publisher parser reaches the actual function implementation; this fixes the stale ZeroConf assertion exposed by 44692804497 on 5e2c845.
- Exact-SHA Actions on ee8ef67 were queued/in progress at the last poll: WAN callback 44693084363, WAN route 44693084041, HTTPD 44693084106, AI docs 44693084139, NVRAM 44693084087, XY-C1 44693084076. No result claimed until completion.

## 2026-11-20 — Align WAN/PPP and DHCP-renew source contracts with shared transactions

- Parent main SHA: `2c19a2172400fb3b775168c499f1148bb3c6b5a6`; production implementation under validation: `10b57237ced5487a0d17e0bb09425ea96973a78e`.
- WAN route `44689432208` and XY-C1 `44689432315` on `5e7d8cf`, plus XY-C1 `44689620163` on `b5146272`, failed in `scripts/check_wan_firewall_status.py` because its PPP snapshot marker expected `rc_wan_nvram_set_all_values()`. Production `ppp_wan_ipup_publish_address()` correctly calls `dhcp_lease_nvram_publish()` inside the NVRAM transaction and retries `dhcp_lease_nvram_restore()` under a fresh transaction; checker now matches that code.
- HTTPD `44689024189` on source `10b5723` failed because its embedded contract required removed comment text `dnsmasq domain-specific server routing too`. It now verifies DNS/domain changes set `resolver_changed`, and publication failure is checked before the resolver-refresh gate/call.
- WAN DHCP callback `44689620059` passed on `b5146272`; NVRAM boundary `44689024044` and injected GNU89/Werror/ASan/UBSan attempt-all/full-rollback regression passed on production source `10b5723`. AI docs `44689831389` passed on parent `2c19a2172400fb3b775168c499f1148bb3c6b5a6`.
- Validator/workflow/handoff change only; no production C changed in this slice. The XY-C1 attempts stopped before detector/MIPS build/TRX/artifact due to the stale source-check assertion; L3/L4 remain NOT VERIFIED.

## 2026-11-20 — Restore mandatory handoff status and next-action headings

- Parent main SHA: `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`.
- Exact-head AI handoff validation run `44689620108` failed with a concrete validator message: `SESSION_HANDOFF.md` was missing required sections `## Verification boundary` and `## Exact first action for the next GPT`. This was a handoff-structure defect, not a source/test failure; both sections are restored with current CI evidence and a specific next source task.
- Exact-head WAN DHCP callback run `44689620059` passed on this parent. NVRAM boundary run `44689024044` and the injected GNU89/ASan/UBSan attempt-all/full-rollback policy executable passed on production source SHA `10b5723`.
- XY-C1 run `44689620163` was still in progress at the last poll. Its outcome determines whether the current production slice reaches detector/MIPS build/TRX/artifact verification; no L3 result is claimed.
- The WAN route/XY-C1 stale MAN teardown contract was corrected to assert the checked `rc_wan_nvram_set_all_values()` path. The latest WAN DHCP callback PASS demonstrates that the LAN rollback checker no longer stops this workflow.
- Local tests remain NOT RUN because this execution environment has no local repository checkout. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Remove invalid test-source variable from LAN policy checker

- Parent main: `5e7d8cf074fcaf7bb392dd50373dfd435e518176`. The LAN deconfiguration checker was updated alongside the new shared DHCP rollback semantics, but the added test-name assertion referenced a `test` variable that this standalone checker does not define. Removed that redundant assertion; the WAN DHCP callback workflow independently checks the test function and invocation in `tests/dhcp_lease_policy.c`.
- The checker continues to require validate-all, attempt-all and reverse full-vector restoration (`for (j = count; j > 0; --j)`). This is a validator correction only; no production source change.
- Next exact-SHA Actions: WAN DHCP callback, WAN route, XY-C1, AI docs and NVRAM boundary must be polled. No new PASS is claimed. The production source SHA remains `10b57237ced5487a0d17e0bb09425ea96973a78e`; XY-C1 L3/TRX/artifact and physical L4 remain NOT VERIFIED.

## 2026-11-20 — Align shared DHCP policy and MAN teardown validators with current writers

- Parent main SHA: 9cadbbd9c116db4b8c0cc3d4f8e312800575f173; production implementation under validation: `10b57237ced5487a0d17e0bb09425ea96973a78e`.
- Exact-source WAN DHCP callback run `44689024028`, job `136130112379`, compiled and passed the GNU89/ASan/UBSan DHCP lease policy executable, including the new attempt-all/full-rollback regression. It then failed in `scripts/check_lan_dhcp_deconfig.py` because the checker still required prefix-only rollback loop `for (j = i + 1; j > 0; --j)`. The shared publisher now restores every tuple entry with `for (j = count; j > 0; --j)`; the validator now checks full-vector rollback plus the new regression.
- Exact-source WAN route run `44689024130`, job `136130113076`, and XY-C1 run `44689024255`, job `136130113735`, failed in `scripts/check_wan_firewall_status.py` because its MAN teardown check incorrectly expected `dhcp_lease_nvram_publish()`. MAN teardown correctly uses the checked all-fields `rc_wan_nvram_set_all_values()` helper and its errno-aware writer; this commit fixes the assertion only.
- Exact-doc SHA `9cadbbd9c116db4b8c0cc3d4f8e312800575f173` AI handoff validation passed (`44689130545`).
- NVRAM boundary run `44689024044` was still in progress at checkpoint. XY-C1 had not reached MIPS build/TRX/artifact steps because the shared source checker stopped first. L3/L4 are NOT VERIFIED.

## 2026-11-20 — Publish DHCP renewal options and PPP runtime tuple through shared rollback policy

- Policy/test commit: `6c0ba561b93e9e4c6ee647718a251d7e4173d727`. The shared `dhcp_lease_nvram_publish()` validates the full vector, attempts every candidate setter despite individual failures, preserves the first write error, and restores the complete previous tuple in reverse order. An injected regression covers invalid-tail validation before writes, a first-key error after applying its value, attempt-all behavior and full restoration.
- Product/source-contract commit: `10b57237ced5487a0d17e0bb09425ea96973a78e`. WAN `udhcpc_renew()` now publishes changed DNS/domain/WINS/lease duration as one serialized NVRAM transaction. Withdrawn/invalid DNS/domain/WINS normalize to empty; absent/invalid lease duration preserves the old value. Resolver refresh occurs only after the DNS/domain snapshot succeeds.
- PPP `ipup_main()` reuses the shared policy for `ipaddr/netmask/ifname_t/gateway/dns`, runs setters under one NVRAM transaction, retries full tuple restoration under a fresh lock after write or transaction-end failure, and retains the existing interface rollback/route gate.
- Updated `scripts/check_wan_firewall_status.py` and the WAN DHCP callback workflow to pin the new renewal and PPP transaction semantics. The exact-source Actions on `10b5723` were in progress at handoff: WAN DHCP callback `44689024028`, WAN route `44689024130`, XY-C1 `44689024255`, AI docs `44689024296`; broader lifecycle and NVRAM checks were also still running.
- Previous exact SHA `79d8f0a7cc59be4e96ee827ff09fcad0576b3720`: WAN route PASS (`44687337571`); AI docs FAIL (`44687337698`) only because the handoff used uppercase `One atomic main commit`. This handoff restores the required lowercase phrase.
- No local checkout is available here; local GNU89/ASan/UBSan tests were NOT RUN. Do not claim L3 until same-source-SHA detector, MIPS build, TRX verification and artifact upload all pass. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Correct the WAN resolver refresh assertion and handoff contract

- Parent main SHA: 971372f4a8a662eeda42e3d7b80ec6f9be6e3de2.
- Exact-SHA detect-internet run 44686752941 passed after its checked VPN resolver marker contract was updated. NVRAM boundary run 44686157053 also passed on the preceding product source.
- WAN route run 44686752930 and XY-C1 run 44686753007 still failed at the new WAN resolver-refresh assertion. The previous repair used startswith() on a string slice that begins at the setter assignment; that made the expected closing brace impossible at offset zero. Corrected it to require the close-of-conditional + unconditional refresh sequence as a substring after the checked setter. Production C remains unchanged.
- AI handoff run 44686752989 flagged two exact protocol phrases omitted while rewriting SESSION_HANDOFF.md. Restore the required "one atomic main commit" and "Before context exhaustion" contract wording without changing the execution rules.
- No same-SHA MIPS build/TRX/artifact result exists yet: XY-C1 stopped at the source contract. Current-source L3 and physical-router L4 remain NOT VERIFIED. The next exact-head WAN route, XY-C1 and AI docs runs are the verification gate.

## 2026-11-20 — Repair WAN resolver teardown source-contract boundaries

- Parent main SHA: 21e567affba4e992476aa7dc16c30be7fa6d4500; the product-source change under test is fix(wan): report resolver snapshot clear failures on teardown.
- Exact-head run 44686156823 (XY-C1, job 136120854113) and run 44686157087 (WAN route, job 136120854818) passed the GNU89/ASan/UBSan route and IPv6 policy executables, then failed in the shared scripts/check_wan_firewall_status.py source contract. The assertion built wan_dns_block with an exclusive end at wan_dns_refresh, then incorrectly required the refresh call to be inside that preceding slice. The production wan_down_internal() call is unconditional and correctly follows the checked conditional DNS clear.
- Exact-head detect-internet run 44686156914, job 136120854304, failed because the embedded contract still searched for the removed unchecked nvram_set_temp("vpnc_dns_t", ""); the current source uses set_nvram_value_status() and aggregates the error.
- Corrected the WAN source contract to assert the closing conditional and the unconditional resolver refresh directly after the checked clear. Updated the detect-internet callback-order marker to the checked setter. No production C behavior was changed in this checker repair.
- NVRAM transaction boundary run 44686157053 subsequently completed SUCCESS on the parent SHA. The XY-C1 job stopped at the WAN contract before the complete build/TRX/artifact gate; L3 is NOT VERIFIED. Physical-router L4 remains NOT VERIFIED. All corrected workflows must be checked again on the resulting exact SHA.
- The parent SHA had 17 successful Actions and the three failures above; the AI docs, Process inventory, VPN server, WAN DHCP, watchdog, DNSMasq, HTTPD, miniupnpd, ZeroTier, wireless, firewall, detect-link, gateway-ARP, WAN teardown and USB modem regressions passed.

## 2026-11-20 — Check WAN-down VPN and indexed DNS snapshot clears

- Parent `main` SHA: `b68158c3ee405bcdc3270db0e6adfd9de91828e5`.
- Found two unchecked temporary NVRAM writes in `wan_down_internal()`: the global `vpnc_dns_t` resolver marker was unconditionally cleared before the optional VPN-client stop, and the indexed `wanN_dns` snapshot was conditionally cleared before `refresh_resolvconf_nonfatal()`. Either failed write was invisible to the callback return status.
- Both clears now use the existing errno-aware NVRAM status setters and accumulate the first failure into WAN teardown status while continuing independent VPN stop, route/interface cleanup and resolver refresh. The existing demand-PPP preservation condition for indexed DNS is unchanged; resolver rebuild remains nonfatal.
- Added a route-status regression for two independent resolver-clear failures with later teardown steps still attempted, and extended the WAN source contract to pin both checked writes, their error aggregation/logging, the original conditional and resolver-refresh ordering.
- Exact-head WAN DHCP/route/HTTPD and XY-C1 validation is required; no firmware L3 or physical-router L4 claim is made here.

## 2026-11-20 — Align PPP ip-up Actions with checked five-field snapshot publication

- Parent `main` SHA: `b4ab85764446ab133873f9d931d411ef25e7761e`.
- Exact-head WAN DHCP and HTTPD Actions on `b4ab85764446ab133873f9d931d411ef25e7761e` failed only in embedded source contracts left over from the earlier callback shape. WAN DHCP still expected the publisher to call unchecked per-field setters and expected `ipup_main()` to perform identity/gateway writes after the policy call; HTTPD used the old identity setter as the first callback side effect.
- Updated the WAN DHCP contract to check the five-field temporary NVRAM snapshot, attempt-all publish + rollback, NVRAM return/errno handling, interface rollback, and no `wan_up()` before the checked policy succeeds. The HTTPD callback-lock contract now treats the policy entry as the first guarded mutation. Regression coverage is strengthened, not removed.
- The production PPP source is unchanged in this test-contract repair. Re-run the exact new SHA before marking these checks PASS; XY-C1 and NVRAM transaction Actions from the prior SHA were still pending at the last poll.

## 2026-11-20 — Publish PPP ip-up runtime state as a checked, rollback-capable snapshot

- Parent `main` SHA: `e4ffdf7e5f3fc201355f359325b637532e824cdc`.
- `ipup_main()` applied IPCP addresses, but the address callback was `void` and silently ignored `wanN_ipaddr/netmask` NVRAM failures. The subsequent `ifname_t/gateway/dns` copies were also unchecked, after which WAN route setup proceeded with an incomplete or mixed snapshot.
- Changed the callback contract to return status and expanded the production publication to five volatile fields: `ipaddr/netmask/ifname_t/gateway/dns`. The writer checks NVRAM return+errno, attempts the full tuple, snapshots the previous values and restores that tuple if any write fails. The policy then takes the newly configured PPP interface down and returns the original publication error; `wan_up()` cannot run unless the complete snapshot succeeds.
- Added policy regressions for publish failure, interface rollback and preserving the primary error when rollback also fails. The WAN route source contract pins all five keys, attempt-all/restore semantics, volatile checked writes and the `wan_up()` gate. Exact-SHA WAN, DHCP and XY-C1 Actions are the production verification gate.

## 2026-11-20 — Correct exact MAN teardown comment anchor in all source contracts

- Parent `main` SHA: `da06fe3916f3f643d833f738b5ab8652c4f4913f`.
- Exact-SHA Actions for `da06fe3916f3f643d833f738b5ab8652c4f4913f` reached 16 PASS, 3 FAIL and 1 PENDING. The failed WAN route, WAN DHCP callback and HTTPD embedded source-contract steps all raised `ValueError: substring not found` on the same test-only anchor: the test abbreviated the production comment `/* Attempt every independent clear even if the address write failed. */` to a shorter literal that does not exist in the C source.
- Updated all three workflow assertions to use the exact source comment. The production source and its semantics are unchanged in this repair. Re-run these three exact-head workflow contracts and keep XY-C1 firmware validation pending until its own run completes.

## 2026-11-20 — Clear PPP demand address snapshots before preinstalling routes

- Parent `main` SHA: `dff6f88a90d503332e27a01bea38b5e81140fb9a`.
- `wait_ppp_up()` previously wrote `wanN_ipaddr/netmask/gateway` through unchecked setters and skipped tuple updates when address/peer ioctls failed. Demand PPP may be administratively present before IPCP; stale values from a previous session could survive and later be read by WAN route/NAT/service setup.
- The demand path now stages the complete three-field tuple through the existing checked NVRAM sequence helper. It clears the local address/netmask when no local address exists, clears the peer when no P-t-P endpoint exists, and aborts before route preinstallation on any NVRAM write failure. No public ABI changes.
- The WAN source contract pins tuple ordering, unavailable-address clearing, checked write result propagation and route-preinstallation ordering. Existing injected helper tests cover ordered writes and first-error handling; exact-SHA WAN and XY-C1 Actions remain the production verification gate.

## 2026-11-20 — Repair MAN teardown source contracts after checked-clear refactor

- Parent `main` SHA: `84cc52a9602c769cbdb4bb0519e056841dabb508`.
- Exact-SHA Actions for `64e751a36d1b3cac83c605f390a8aea3b4d77624` reported failures in `wan-route-status`, `wan-dhcp-callback`, `httpd-security` and the XY-C1 workflow. The WAN route checker had a malformed Python `require()` expression; three workflows still asserted that removed direct `nvram_set_temp()` calls were present.
- Corrected the checker syntax and synchronized the existing route, DHCP callback and HTTPD embedded source contracts with the attempt-all temporary NVRAM helper, while retaining route-before-clear ordering and the conditional DHCP address clear. No production source behavior is changed in this CI repair.
- The next exact-SHA Actions run is the required verification; the preceding source task's standalone sanitizer smoke test passed, but its full source workflow had not yet passed.

## 2026-11-20 — Gate static MAN routes on the refreshed gateway snapshot

- Parent `main` SHA: `64e751a36d1b3cac83c605f390a8aea3b4d77624`.
- Traced `wan_up_internal()` into `add_static_man_routes()` and `control_static_routes()`: a route with no valid explicit gateway falls back to global `wanx_gateway`. The WAN-up path previously ignored the result of `nvram_set_temp("wanx_gateway", ...)`, so a failed snapshot write could make an ostensibly new WAN-up path install a route using the previous MAN gateway.
- WAN-up now uses the checked temporary NVRAM writer, preserves the first error in its return status, and skips the dependent static MAN route batch when the refreshed gateway cannot be published. Independent WAN route, DHCP, firewall and service attempts continue.
- Extended the WAN source contract to pin the actual `net.c` fallback dependency, checked volatile write and fail-closed route gate. Exact-head WAN route Actions will execute the contract; this change makes no firmware/hardware claim.

## 2026-11-20 — Attempt every MAN teardown snapshot clear and preserve the first error

- Parent `main` SHA: `885e71f9b3d7eeab9a2b39fc7602c11880450fec`.
- Confirmed `man_down_internal()` cleared `wanx_ipaddr/gateway/dns/domain/routes/routes_ms/routes_rfc` through unchecked `nvram_set_temp()` calls after route removal. A failed clear was invisible to the DHCP deconfig callback, which otherwise already propagates `man_down_internal()`'s status.
- Added an attempt-all NVRAM value policy: validate the complete operation list before any writes, attempt every independent clear, and return the first write failure. MAN teardown uses checked `nvram_set_temp()` writes (not flash-persistent `nvram_set()`) for the global gateway/DNS/domain/route snapshot and conditionally clears the DHCP address exactly as before. A failed clear joins the teardown result; unrelated later cleanup still proceeds.
- Added GNU89-injectable regressions for all writes being attempted, first-error preservation and invalid-input no-side-effects. Extended the WAN source contract to pin the MAN clear inventory, route-teardown ordering, conditional IP clear, unchecked-write removal and DHCP callback status propagation.
- Local standalone policy regression and exact-head WAN Actions are the required verification; no firmware or hardware result is claimed here.

## 2026-11-20 — Fail closed when clearing indexed WAN runtime snapshots

- Parent `main` SHA: `d59dc2bdea3307eac78fba6c6aabd7bdac7caf85`.
- Confirmed reset gap in `reset_wan_vars_status()`: the indexed WAN DNS, WINS, lease, domain, DHCP route lists and error snapshot were cleared through void setters. The non-static branch also zeroed the indexed IP address, netmask, gateway and MTU through unchecked setters. A failed NVRAM write could preserve a previous WAN's resolver/route/address state while reset continued toward protocol selection and activation.
- Added an injected ordered value-write policy and checked indexed setter callback. Every required clear now stops at the first error, latches `wan_vars_reset_status`, logs the affected snapshot class and aborts reset before WAN activation. The public `void reset_wan_vars(void)` ABI remains unchanged.
- Added policy regressions for ordered success, first-error short-circuiting, invalid field names and null values; the WAN source contract now verifies all eight runtime clears and all four non-static address clears are checked and fail closed. Existing WAN workflow path filters already cover every changed source/helper/test/checker file.
- Local isolated policy regression: GNU89, `-Wall -Wextra -Werror`, ASan and UBSan PASS. Full repository source-contract workflow and exact-head XY-C1 build/TRX/artifact status remain pending; no L3 firmware or L4 hardware claim is made.

## 2026-11-20 — Make WAN interface MAC snapshot writes fail closed

- Parent main SHA: `e487695db417478a5879b99da8d18871c3363d62`.
- The reset path selected `wan_hwaddr_x` or the `il1macaddr` fallback, but ignored failures from persistent `nvram_set("wan_hwaddr", ...)` and indexed `set_wan_unit_param(unit, "hwaddr")`. Later WAN VLAN and soft-bridge setup uses the global MAC for VLAN/interface `set_interface_hwaddr` operations.
- Added status-returning checks for the global persistent MAC and indexed per-WAN MAC snapshot. A failed write now latches the first reset error and prevents later interface/VLAN activation. Existing public void setter ABI is untouched.
- Extended the WAN source contract to require both writes and the fail-closed gate. Exact-head verification is pending; no firmware/L3 or hardware result is claimed.

---

## 2026-11-20 — Anchor WAN protocol checker to the protocol publication gate

- Parent main SHA: `eac0f17e12e394ede94e56dc4d432770256f9009`.
- Exact WAN-route job `136105910004` on parent `077a917` reached all policy compile/run steps, including GNU89/`-Werror`/ASan/UBSan, then failed at the final Python source contract: "failed WAN protocol publication must latch and return before protocol selection".
- The checker selected the first `if (proto_status != 0)` in the status-aware reset body. After adding checked MAN/virtual IPTV snapshot reset, this first failure gate occurs before the protocol-copy write and is unrelated to the protocol-copy contract. The checker now finds the protocol failure gate starting at the protocol-write position, preserving its intended order assertion rather than weakening it.
- Production code was not changed in this corrective commit. Exact-head rerun is pending.

---

## 2026-11-20 — Check DHCP client and PPP DNS negotiation snapshots

- Parent main SHA: `077a917ef22e4d5516289264e60b6c4b6e5607f1`.
- Source review showed `dnsenable_x` directly controls MAN DNS host routes and pppd `usepeerdns`; `vci` is passed as udhcpc's DHCP vendor-class option; `hostname` is passed to udhcpc as its client hostname. Previously all three were copied into the indexed WAN snapshot through unchecked setters.
- Added a checked required tuple for these three configuration inputs. Any write failure now latches WAN reset failure and prevents protocol activation, avoiding stale DNS behavior or DHCP identity/option reuse.
- Extended the WAN source contract to require exact field coverage, order and failure context. Exact-head CI is pending; no device/L3 claim.

---

