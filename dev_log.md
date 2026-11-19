## 2026-11-19 — Remove the orphaned off-subnet WAN gateway host route

- `wan_up_internal()` installed a metric-1 `/32` gateway host route for valid off-subnet gateways on non-PPP interfaces. `wan_down_internal()` deleted the default route plus DHCP/static route inventories, but did not delete that independently-created host route. It could survive WAN DHCP lease loss or replacement.
- A shared route-policy predicate now guards both setup and teardown using valid-gateway, non-PPP and off-subnet conditions. Teardown attempts the host-route delete after the default-route delete, treats only ESRCH as idempotent absence, accumulates other errors, and continues ordinary route cleanup.
- Added GNU89 policy regression for PPP/on-subnet/invalid exclusions, absent route handling and first-error preservation; extended WAN route workflow contracts for setup/delete symmetry and ordering. Exact-head WAN route L2 and XY-C1 L3 are pending. No local cross-build or physical WAN test is claimed.

---
## 2026-11-19 — Correct NDIS local scope and check all USB queue notifications

- Follow-up exact-head XY-C1 runs 44578459810 and 44580744748 failed at Build XY-C1 because interface_down_status, interface_down_errno, interface_flags, interface_error and interface_cleanup_status had been declared in mdev_wdm_main() rather than mdev_net_main(). Moved the locals to the NDIS handler; the first compiler diagnostics are recorded in SESSION_HANDOFF.md.
- The USB workflow checker-only repair 4c331818077d8b9ddfdb5998e35e9c8e0b0add6f passed exact-head USB run 44582391137 and AI docs 44582391240. This commit carries that corrected contract forward and adds the missing printer-handler checks.
- Replaced the two remaining void notify_rc() printer calls with the same errno-logging checked queue helper used by modem NDIS/TTY. The printer hotplug command now returns nonzero if the queue markers were not accepted. All USB hotplug events are checked; exact-head USB and XY-C1 reruns pending.
- No local build or physical USB device validation is claimed.

---

## 2026-11-20 — Align USB hotplug checker with checked PID-1 notifier

- Base main SHA: `0f719780b4d1b1d38b0300855b8424d74d033e20`. Exact-head USB push run [44580744698](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44580744698) failed in the WDM/NDIS source-contract step after all five GNU89, -Wall/-Wextra/-Werror, ASan/UBSan policy binaries passed. The assertion still required the removed `notify_rc("on_unplug_usb_modem")` call even though the production handler now uses the checked `notify_modem_hotplug_event(...)` helper.
- Updated the cleanup-before-notification contract to check the checked unplug notifier call. The existing assertions continue to require checked NDIS/TTY plug/unplug events, preserve status precedence, and reject legacy void notifier use. No production source changed in this checker correction.
- On the same source SHA, Process inventory `44580744598`, Process fail-closed `44580744806`, AI docs `44580744704`, and HTTPD security `44580744667` passed; NVRAM `44580744629` and XY-C1 `44580744748` were in progress. THIS COMMIT's exact-head USB, NVRAM, and XY-C1 results must be checked after push. No local compile, MIPS build, or physical-router validation is claimed; L4 remains NOT VERIFIED.

---

## 2026-11-19 — Propagate modem hotplug queue publication errors

- notify_rc.h exposes notify_rc_status(), whose contract returns failure when secure incomplete/action queue markers cannot be accepted; legacy notify_rc() intentionally discards the result. NDIS and serial TTY hotplug handlers used the void wrapper for modem plug/unplug events, so they could report success while PID 1 had no queued transition to process.
- Replaced the four modem NDIS/TTY plug/unplug notifier calls with a checked helper that preserves/logs errno and merges queue failure into each handler's returned metadata status. An unplug notification is still attempted after interface-down or metadata cleanup failure; first nonzero status remains authoritative.
- Added GNU89 policy coverage for queue success/failure and status precedence. Extended the USB modem identity workflow contract to pin NDIS and TTY plug/unplug notification wiring and reject legacy void calls in those handlers. Exact-head USB L2/XY-C1 L3 pending; no local compile or physical modem test is claimed.
- The preceding NDIS unplug status commit 8fea3095d59b3278a9318d401eadad0606c193b7 passed USB workflow 44578459451, Process inventory 44578460016, Process fail-closed 44578459789, AI docs 44578459995, NVRAM 44578460023, and HTTPD 44578459942; XY-C1 44578459810 remained in progress at checkpoint.

---

## 2026-11-19 — Preserve NDIS ifconfig-down failures on USB removal

- `mdev_net_main()` discarded the result of `ifconfig(device_name, 0, "0.0.0.0", NULL)` on NDIS removal. A vanished USB interface is an expected ENODEV race, but a failed down operation on an interface that still exists was being reported as success; state-probe failures were also hidden.
- The remove path now captures ifconfig status and probes interface flags only on failure. Confirmed ENODEV is treated as already removed; an extant interface preserves the original ifconfig failure, while UNKNOWN preserves the probe error and fails closed. WAN-owner notification is still attempted after cleanup failure.
- Added GNU89 policy cases for successful down, extant-interface failure, physical-removal ENODEV, and UNKNOWN probes. Extended the USB modem identity workflow contract to verify status handling precedes but does not suppress WAN notification. Exact-head USB L2/XY-C1 L3 pending; no local compile or hardware test is claimed.
- The preceding guest ebtables source SHA `f0406397927cc648c7fd70c1961fb3aba1fd1243` passed Wireless `44576171024`, AI docs `44576170956`, NVRAM `44576171017`, Process fail-closed `44576171294`, inventory `44576170689`, HTTPD `44576170606`, iNIC signal `44576171003`, and watchdog signal `44576171120`; XY-C1 `44576170960` remained in progress at checkpoint.

---

## 2026-11-19 — Preserve guest ebtables state when module probes are unknown

- Selected a separate Wireless stability defect after the iNIC force-disable fix. `is_module_loaded()` converts the tri-state module probe's UNKNOWN/error result to false, so guest-isolation disable could skip stale ebtables cleanup yet return success. The enable path also trusted `module_smart_load()` even though its zero return means either already-loaded or load failure.
- Guest-isolation enable now probes `ebtable_filter` after the load attempt and refuses to flush/install rules unless the module is confirmed present. Disable maps module state to three explicit actions: loaded = cleanup then unload; absent = no-op; unknown = record the probe error and attempt `-F`/`-X` cleanup without unloading modules.
- Added GNU89 unit coverage for loaded/absent/negative-errno enable states and all three disable actions. Extended the production source checker to pin tri-state probe, error-recording, cleanup, and unload ordering. Exact-head Wireless/AI docs and XY-C1 validation are pending; no local compile or physical-router result is claimed.
- Preceding iNIC force-disable exact-head Wireless radio run `44574370283` and AI docs run `44574368349` passed; Process lifecycle regression `44573883051` and NVRAM transaction audit `44573883023` passed. XY-C1 `44573883318` was still in progress on iNIC product-source SHA `3d86aebbef26086a4383489efc4470e35e91b9df` at the last poll.

---

## 2026-11-19 — Correct iNIC disable source-contract matcher

- Exact-head Wireless Action `44573883138` for source commit `3d86aebbef26086a4383489efc4470e35e91b9df` passed the GNU89 `-Wall -Wextra -Werror` policy regression, bridge policy regression, and guest isolation policy regression. Its Python production-source contract then failed because the newly added expected multiline call literal was over-escaped and therefore did not match the actual C statement.
- Fixed the checker to assert the stable call prefix and argument token independently, retaining the separate assertion that the call precedes bridge detach, PHY isolation/RGMII disable, GPIO reset, and the final status return. No production code or test behavior was weakened.
- Exact-head AI docs, Process inventory, iNIC signal, watchdog signal and HTTPD checks passed on `3d86aeb`; Process regression, NVRAM and XY-C1 were still pending at the last poll. This checker follow-up's exact-head Wireless and XY-C1 runs must be checked after commit.

---

## 2026-11-19 — Preserve iNIC force-disable interface failure in returned status

- Selected a separate RC wireless stability defect after closing WAN gateway ARP identity. In `start_inic_mii()`, the explicit `inic_disable` branch discarded the result of `wif_control_success(ifname_inic, 0)`; it still continued bridge detach, PHY isolation, RGMII disable and reset, but the returned status could be successful when the initial interface-down failed and later cleanup succeeded.
- The force-disable branch now merges the interface-down boolean into the sticky `all_started` result before all later safety operations. It does not short-circuit or skip bridge/hardware/reset cleanup on failure.
- Added `test_inic_force_disable_interface_failure_is_sticky` to model the failed ifconfig operation followed by successful detach/PHY cleanup, and added an exact production/source regression assertion in `.github/workflows/wireless-radio-state.yml`.
- Prior WAN ARP product SHA `fe286b8cabd4334664c8270c5212d26a15ce6393` reached exact-head L2 and XY-C1 L3 PASS; the corrected AI handoff contract `44573162136` passed on `f3f39d7d6e7acde293982a51e9dcab76994b3c8e`. This new wireless slice is pending exact-head validation. No local toolchain or physical iNIC test is available.

---

## 2026-11-19 09:20 EST — Correct the committed AI handoff checkpoint contract

- Exact-head AI docs run `44571309978` failed because the handoff omitted the literal `The checkpoint is mandatory; a document edit is conditional.`. A first follow-up commit (`76161729272bdfeb553725a43afdf4b142232e10`) did not include that exact sentence or the required root heading/active-task section; its own run `44572344939` therefore failed with the same validator error. The live handoff is now corrected directly and the next commit must be verified before the documentation contract is called closed.
- Product source commit `fe286b8cabd4334664c8270c5212d26a15ce6393`: exact-head WAN ARP regression/source contract, Process inventory, Process regression, and NVRAM audit passed. XY-C1 input detection and pre-build host gates passed; Build/TRX/artifact were still pending at last poll.
- The corrected handoff restores the validator-required root heading, active-task section and exact mandatory-checkpoint sentence, explicitly separating mandatory accuracy review from conditional document edits. This repairs a real CI failure rather than creating a routine snapshot.

---

## 2026-11-20 00:54:01 CST (UTC+8) — Validate complete WAN gateway ARP reply identity

- Base main SHA: `f50387e29be43e9de4a1b55344390742a4252b53`. Product source changed in `trunk/user/rc/src/detect_wan.c` and `detect_wan_arp_result_policy.h`; regression changed in `trunk/user/rc/src/tests/detect_wan_arp_result_policy.c`; the focused `.github/workflows/detect-wan-arp-stability.yml` source contract now pins the complete matcher and tuple wiring.
- `arpping_gateway()` previously treated a minimum-length frame as a gateway reply after checking only Ethernet EtherType, ARP opcode, target hardware address and sender IPv4 address. It did not verify Ethernet/IPv4 ARP types and address lengths, Ethernet source/destination, consistency of Ethernet-source and ARP-sender MACs, or the reply target IPv4 address. An unrelated or malformed frame could therefore be mistaken for a live gateway and suppress the intended DHCP recovery decision.
- The new policy accepts only an Ethernet/IPv4 ARP reply addressed to the current local MAC/IP, whose Ethernet and ARP sender MACs agree and whose sender IPv4 address is the configured gateway. Sender and target IPv4 values are copied from potentially unaligned packed fields before comparison, avoiding unaligned integer access on MIPS.
- Regression cases cover a valid unaligned reply and reject wrong EtherTypes/types/lengths/opcode, inconsistent or unrelated L2 addresses, a wrong gateway sender IP, a stale target IP, and missing pointers. Existing local socket-error/no-reply/DHCP-renewal policy coverage is retained.
- No local checkout/toolchain is available; no local compiler or hardware result is claimed. Exact-head WAN ARP sanitizer/source-contract, Process source inventory, AI handoff docs, and XY-C1 Build/TRX/artifact Actions are pending inspection on THIS COMMIT. XY-C1 L3 and physical-router L4 are not assumed from prior SHAs; L4 remains NOT VERIFIED.

---

## 2026-11-19 — Preserve the actual DHCPv6 startup-cleanup errno

- Main parent: `3ae692ae9da8fe013c283b294ab6a66ee22a72f8`. The exact checker-only follow-up WAN DHCP callback `44561987026` and AI handoff docs `44561986982` both PASS, clearing the stale direct-`unsetenv()` assertion from the previous source-contract failure.
- Source review found a distinct errno-reporting defect in `start_dhcp6c()`: it saved `errno` immediately after `eval("/sbin/dhcp6c", ...)`, but a successful `eval()` does not reset errno. If the later environment cleanup failed, startup was correctly turned into failure and logged the cleanup errno, yet the caller could receive the stale pre-existing errno because cleanup replaced it only when the saved value happened to be zero.
- Added `dhcp6c_callback_start_errno()` to ignore errno after successful launch, return the actual cleanup errno on cleanup failure, preserve a real launch error as primary, and use EIO when a failed launch supplied no errno. Regression cases cover stale errno after successful launch, success with clean cleanup, primary launch-error precedence, and EIO fallback. The WAN DHCP source contract pins helper usage and ordering.
- The new source SHA requires exact-head WAN DHCP and XY-C1 Build/TRX/artifact checks; parent `3ae692ae9da8fe013c283b294ab6a66ee22a72f8` is checker-only and did not change firmware inputs. No local checkout/toolchain or physical-router L4 is available; no local tests are claimed.

---
## 2026-11-19 — Repair stale DHCPv6 environment-cleanup source contract

- Main parent: `0b3449bc31ceda006408409dacd31b81785908b7`. Exact push WAN DHCP callback run [44558399735](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558399735) compiled and passed all host policy regressions, including the DHCPv6 callback-generation test, then failed in its final Python integration contract with `ValueError: substring not found` at script line 333.
- The failing assertion still searched for a direct `unsetenv(DHCP6C_CALLBACK_IFNAME_ENV)` call after `eval()`. The production implementation had correctly moved both cleanup operations behind `dhcp6c_callback_environment_clear()`; the newer contract already verified that helper, but a second older startup-order assertion was stale.
- Updated the startup-order contract to require the helper call after the client launch and confirm both environment names and `unsetenv` are passed to it. No product source or runtime behavior changed; push and pull_request path filters remain unchanged.
- On parent SHA `0b3449bc31ceda006408409dacd31b81785908b7`, Process inventory [44558400105](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558400105), HTTPD [44558399718](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558399718), WAN route [44558399992](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558399992), AI docs [44558399754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558399754), and NVRAM audit [44558399692](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558399692) passed. XY-C1 [44558399503](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44558399503) passed Build, TRX verification and artifact upload (L3 on the parent product-source SHA). Only the WAN DHCP callback source-contract job failed.
- This commit's new exact-head WAN DHCP, AI docs and relevant Process/XY checks must be inspected. No local checkout/toolchain or physical-router L4 is available here; no local test is claimed.

---
## 2026-11-19 — Fix short-circuit DHCPv6 startup environment cleanup

- Main parent: `7c0481f8f390db2e1f26351679fb2cb98bdfb021`. `start_dhcp6c()` used `unsetenv(IFNAME) != 0 || unsetenv(GENERATION) != 0`, so a failed first unset short-circuited and never attempted to remove the generation variable. The later startup rollback path already made two independent attempts.
- Introduced an injectable helper that attempts both removals, preserves the first errno, and returns failure. Both post-launch and startup rollback cleanup use it. C tests simulate first-unset failure, assert the second was attempted, verify first errno, and test successful removal of both variables. WAN DHCP workflow checks callsites and regression markers.
- Exact `ef12b80` WAN DHCP `44555817409`, Process `44555817648` and AI docs `44555817614` PASS. NVRAM `44555817580` and XY-C1 `44555817271` pending at last poll. The DHCPv6 lock-cleanup commit `7c0481f8` Actions were still running.
- No local test or physical L4 claimed.

---

## 2026-11-19 — Report DHCPv6 callback-lock cleanup failures

- Main parent: `ef12b8003ef5d1a856d61ff4c7b2eb2a6bcfa3be`. In `stop_dhcp6c()`, the generation-advance failure path discarded `dhcp6c_callback_session_unlock()` status with `(void)`, suppressing possible unlock/close failure diagnostics.
- Captures the cleanup result, logs its errno, and restores the generation failure errno as the primary result. Added `dhcp6c_callback_primary_errno()` plus unit coverage for primary/secondary/fallback precedence; WAN DHCP workflow checks that the cleanup status is captured and logged before errno restoration.
- Exact `ef12b80` WAN DHCP callback `44555817409` PASS, AI docs `44555817614` PASS, Process fail-closed `44555817648` PASS. NVRAM `44555817580` and XY-C1 `44555817271` pending at last poll.
- No local compile or physical-router L4 is claimed.

---

## 2026-11-19 — Log post-commit LAN DHCP watchdog/monitor failures

- Main parent: `a1bbc3f87e892882abde51688f88b1cfe982f9b6`. In `udhcpc_lan_bound()`, watchdog-time and detect_internet notifications were cast to void after the LAN lease transaction and NVRAM publication had committed. Unlike host-file, IAPP and networkmap ancillary work, these failures had no diagnostic.
- This change captures/logs both notification statuses, while leaving them out of the BusyBox callback result: once the router's address/route/resolver/NVRAM tuple is committed, a failed optional notification must not cause BusyBox to keep an old internal lease. Added explicit C policy cases for watchdog/monitor errors with successful and failed transaction status; WAN DHCP workflow now asserts order, logging and non-propagation.
- Exact `a1bbc3f` AI docs `44554950244` and Process fail-closed `44554950348` PASS. AP LAN source L3 is separately verified on `908fee5` / XY-C1 `44552799621`; ZeroTier L3 on `a834edb` / `44544667515`.
- No local test/compile or physical-router L4 is claimed.

---

## 2026-11-19 — Update duplicate Process LAN teardown contract

- Main parent: `82153dbc2fb46c48e85f7f88f2466c5845a4c152`. Process fail-closed run `44554200782` failed at line 3059 of `scripts/check_process_lifecycle_integration.py`: a second copy of the old LAN teardown return assertion still omitted `monitor_resume_status`. The first copy had already been updated and passed. This commit fixes only the stale duplicate.
- WAN route `44554200970` and AI docs `44554201629` PASS on `82153dbc`.
- AP LAN production slice `b735ea8` is L3 VERIFIED by exact XY-C1 run `44552799621` on `908fee5`: Build XY-C1, TRX verification and artifact upload all PASS.
- No physical-router L4 or local compile is claimed.

---

## 2026-11-19 — Align remaining AP monitor-resume source contracts

- Main parent: `908fee5e350717c881ffda0c69d81f005f7e04c7`. Exact WAN route `44552799897` failed because its new source assertion incorrectly expected the DHCP-preflight abort branch to return the monitor-resume error; that branch already has a nonzero primary stop status and must preserve `return status;`. The checker now verifies that primary return follows the resume-failure log.
- Exact Process fail-closed `44552799701` failed because its expected compact LAN teardown aggregate predated the new final `monitor_resume_status` addition. The checker now requires that final status to be preserved, and also checks the AP DHCP-resume log.
- Exact `908fee5` PASS: HTTPD `44552800096`, DNSMasq `44552799756`, NVRAM `44552799727`, Wireless `44552799518`, WAN DHCP callbacks `44552799472`, AI docs `44552799400`, detect_internet `44552799384`, watchdog `44552799369`, Process inventory `44552799345`, Firewall `44552799340`, detect_link `44552799320`. XY-C1 `44552799621` remained pending at last poll.
- Source behavior and regression tests are unchanged in this correction; no assertion was removed. No local execution/physical L4 is claimed.

---

## 2026-11-19 — Report AP Internet-monitor resume failures after aborted LAN teardown

- Main parent: `722606bf0af021382871a58ba71f9071706a40b4`. In AP mode, `stop_lan()` paused `detect_internet`, then discarded the result of resuming it when DHCP-client identity shutdown failed or when bridge-down failed. The monitor could remain paused without a diagnostic for this secondary recovery failure.
- This source slice captures both resume results, logs each failed recovery, preserves the original DHCP/bridge teardown error as the first status, and includes the bridge-abort resume status after existing cleanup statuses. Regression adds `test_ap_lan_monitor_resume_failure_preserves_primary_teardown_status`.
- Exact `b735ea8` runs: WAN route `44551953519` failed the first new assertion because the checker expected one capture rather than two and referenced its C test variable too late. Process fail-closed `44551953591` and detect_internet `44551953522` flagged their old expectation of `return status;` after the resumed monitor; keep this primary return unchanged in the next commit, while retaining the bridge-path aggregate. Their full earlier checks and shell/host tests reached that final integration checker.
- Other exact `b735ea8` actions PASS: AI docs `44551953627`, Firewall `44551953596`, HTTPD `44551953576`, Process inventory `44551953530`, USB storage `44551953524`, DNSMasq `44551953512`, WAN DHCP callbacks `44551953404`, detect_link `44551953518`, watchdog `44551953340`, Wireless `44551953822`. NVRAM `44551953644` and XY-C1 `44551953308` pending. ZeroTier L3 remains on `a834edb` / run `44544667515`.
- This follow-up corrects those source contracts. No local execution or router L4 is claimed.

---

## 2026-11-19 — Restore the required next-session action heading

- Main parent: `a8e8b63353ff5f665b1ec4baa532a8398646bb5d`. Exact AI handoff workflow `44549810797` failed at the documentation validator because the preceding handoff replacement omitted the mandatory `## Exact first action for the next GPT` section.
- This follow-up restores that required heading with an actionable sequence: verify new exact-head CI once, then continue with an independent lifecycle/rollback defect without repeating the completed ZeroTier audit. The active state, exact runs, and L3/L4 boundaries are corrected as well.
- Exact `a8e8b633` ZeroTier `44549810631`, Process inventory `44549810901`, and WAN route `44549810691` PASS. XY-C1 `44549810780` was pending on a test-only change. ZeroTier product SHA `a834edb` has exact L3 Build/TRX/artifact PASS `44544667515`.
- No local tests or physical-router L4 are claimed.

---

## 2026-11-19 — Remove orphaned ZeroTier regression fragment after assertion repair

- Main parent: `5df5d3487b87fffc2c68d6637e5d994df8e724e5`. Exact ZeroTier push run `44545994030` passed the shell assertions and printed the regression PASS marker, then exited with `Syntax error: ")" unexpected` at line 774.
- Inspection found a stale duplicate suffix ` "$TEST_ACTION_LOG")" = 1 ] ...` after the final echo. The intended exact start-count assertion is already present beside the successful stale-state replay test. THIS COMMIT removes only the orphan; no behavior assertion is deleted or weakened.
- The immediately preceding ZeroTier run `44545284526` on `0b1e7877` exposed the replacement-start assertion issue now corrected in `5df5d348`. Process inventory `44545994027` and WAN route `44545994009` PASS on `5df5d348`.
- Product source `a834edbad9ba5a168768a882c766607065fb7708` has exact XY-C1 Build/TRX/artifact PASS in run `44544667515` (L3). The current test-only XY run `44545994024` skipped those expensive stages. No local shell execution or physical-router L4 is claimed.

---

## 2026-11-19 — Expose ZeroTier firewall-delete regression trace

- Main parent: `deed12f04f7bd2030a99acf689295396b505111e`; the current source still includes the ZeroTier firewall cleanup implementation and the corrected `iptables -t nat -C/-D` ordering.
- Exact b1 ZeroTier workflow run `44518868454` failed at the five-delete log-count assertion in `trunk/user/zerotier/tests/zerotier_lifecycle.sh`. The preceding assertion that the fake firewall-state file was empty passed, so the regression output did not yet explain why the log count disagreed with the actual fake state.
- This diagnostic follow-up keeps the strict five-delete assertion and emits the numbered raw iptables log plus remaining fake state only on mismatch. Use the next exact-head Action to identify the exact unmatched command; do not remove the assertion or weaken state-empty/cleanup checks without proving they are redundant and replacing them with equivalent behavioral coverage.
- No production source changed in this diagnostic slice. Current-main XY-C1 run `44541196137` was pending at Build when checked; earlier b1 XY-C1 `44518868802` does not establish L3 for later sources. No local test execution is claimed.
- Backup branch remains immutable; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Propagate USB modem WAN reconnect failures

- Parent main SHA: `92a098912e648e0c00a03db8592c555a3cfb2aeb`. The latest production sources include strict USB eject status/target handling (`6a846b9`, `7fa39c`) and the RPL2TP stop/rollback correction (`cd398c9`).
- Found a caller-level result drop in `safe_remove_usb_device()`: it invoked integer-returning `try_wan_reconnect(0, 0)` after a successful modem removal, ignored any `stop_wan()`/`start_wan()`/L2TP failure and unconditionally returned success. This could report a successful modem eject while WAN recovery failed.
- The production path now checks the reconnect result, clears stale errno before the call, maps positive statuses to errno, preserves a negative-call errno or defaults to `EIO`, logs the failure and returns `-1`. Only a successful reconnect returns zero.
- Added source-contract regression assertions to both the Process lifecycle integration checker and storage safe-remove checker. These pin status capture, fail-closed return ordering, errno normalization, logging, and the underlying `try_wan_reconnect()` status return contract.
- Pre-change Actions: USB storage `44536791653` PASS on 7fa; Process fail-closed `44537131446` PASS on f0a; Process inventory `44539111552` PASS on cd398; WAN route `44539829446` PASS on 92a. XY `44539829469` on 92a was host-only and skipped Build/TRX/artifact because its diff had no firmware input; the RPL2TP production change cd398 has not yet received a successful same-source build.
- No local checkout/toolchain is available, so no local test execution is claimed. New exact-head Process/storage/XY-C1/docs Actions are pending; physical L4 remains NOT VERIFIED.

---

## 2026-11-19 — Align safe-remove errno capture contract with rollback ordering

- Product-source baseline remains `25bc8c15915cbcd617a5cb61bd33fef4a7f524bf`; no production source changes are included in this checker repair. `safe_remove_stor_device()` captures the stop failure's errno before attempting storage-service rollback, then aborts without unmounting if stop/rollback cannot be completed.
- Exact XY-C1 run `44530185559` passed Build XY-C1, TRX verification and artifact upload on the product-source SHA (L3). HTTPD security `44530598102` and AI handoff docs `44530598334` passed on test-only SHA `46b273f46dfc92e1dc2b3229772ae7f8858a90c3`.
- The storage caller-graph Action `44531837841` on `e1712996e052b6f5e161dc510dbe5f9ab77f19e4` passed earlier storage regressions and the corrected cleanup-order assertion, then failed because its errno check looked only after the rollback call; production correctly saves errno before rollback. THIS COMMIT checks that capture occurs before rollback and that the captured status remains within the abort branch.
- Exact-head storage and documentation reruns are pending. No local checkout/runtime tests were available in this connector-only session; storage L2 remains pending until the exact workflow passes. Product L3 is verified at `25bc8c15915cbcd617a5cb61bd33fef4a7f524bf`; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Verify storage swap-file eject fix and restore end-of-session handoff gate

- Exact storage-hotplug Action `44526279751` PASS on `5adde0e864e7ea9db88b7c352fb04d048fc8ce6f`. Every step passed: strict storage-eject GNU89 policy regression, BusyBox 1.24.2 build and ash lifecycle tests, and final source/caller/persistence gates. The new swap-file detection and `swapoff(path)` path has host L2 evidence.
- Exact same-SHA XY-C1 run `44526279408` remained in progress at Build XY-C1 at the last poll; TRX verification and artifact upload remain pending. No storage L3 claim.
- AI handoff docs run `44526279501` failed because a previous handoff edit accidentally removed the mandatory end-of-session checkpoint section. Restored the required checkpoint and main-only cross-session execution mandate in the live handoff; this docs correction will be checked on its own exact SHA.
- Exact-head Firewall Startup, HTTPD security, NFS lifecycle, Process inventory/fail-closed, vsftpd publication, USB modem identity, and Samba configuration Actions all PASS on 5adde. NVRAM transaction audit remained in progress at the last poll.

---

## 2026-11-19 — Disable swap files on physically ejected storage before unmount

- Confirmed a physical-unplug cleanup gap in `umount_ejected_locked()`: `storage_stage_ejected_swaps()` discarded every `/proc/swaps` entry for which `storage_ejected_swap_device_name()` returned 0, including swap files such as `/media/usb/swapfile`. When the backing USB partition disappeared from `/proc/partitions`, the stale mount was detected but the active swap file was never passed to `swapoff()`; unmount remained busy and repeated eject retries could not release the device.
- The production path now stages absolute non-`/dev` swap paths alongside raw removable block swaps, associates them with stale removable mountpoints using strict component-boundary matching, and calls `swapoff(path)` before unmount. Internal swap files and non-removable `/dev` nodes are not selected.
- GNU89 regression adds normal/deep mount matches, root mount, prefix-collision rejection, exact-mount rejection, relative-path rejection, and `..` traversal rejection. The Python source-contract checker links the policy and tests to the physical-eject implementation.
- No local toolchain is available in this connector-only session. Exact-head storage-hotplug GNU89/ASan/UBSan and BusyBox ash checks plus same-SHA XY-C1 Build/TRX/artifact are pending; no storage L2/L3 claim yet.

---

## 2026-11-19 — Close ZeroTier lifecycle L2 verification on d6acda

- Exact-head ZeroTier Action `44522534088` completed PASS on `d6acda494876c87e413939689ed3cd058b7f7fa6`: bounded shell stop/start/readiness/firewall/static-route regression, Python lifecycle and RC integration source contracts, and WAN-up launch-status wiring all passed.
- Exact-head AI handoff documentation Action `44522534108` also PASS. This closes the prior checker-only failures; no pending ZeroTier contract repair remains.
- Treat this as host L2 evidence only. The ZeroTier product slice still lacks same-source XY-C1 Build/TRX/artifact validation and physical-router L4 evidence. RStats is independently L3 verified on source SHA `3523bb9f27ba33065731abcd946ae11726d04470` with host `44519760957` and same-SHA XY-C1 `44519760903` Build, TRX and artifact all PASS.
- Next review slice: storage/USB hotplug safe-remove and raw swap-only cleanup. Existing inventory parsing and fail-closed gates were inspected; no new source defect has yet been confirmed.

---

## 2026-11-19 — Match ZeroTier interface and route cleanup guard contracts to POSIX shell

- Exact ZeroTier run `44522100399` on `500ba4a4f1bd45b19f3c08c3dcaab0333562e0d8` passed the shell lifecycle regression and passed the preceding stop-state ordering contract after the prior correction.
- The next source contract failed because it expected parameter-expansion forms `[ -z "${zt0:-}" ]` and `[ -z "${ip_segment:-}" ]`; actual production shell uses simple initialized variables `[ -z "$zt0" ]` and `[ -z "$ip_segment" ]`. The checker now asserts the exact production guards; it continues to enforce interface/route capture before daemon stop and saved-state reuse during firewall cleanup.
- This is checker-only alignment. Product-source SHA `3523bb9` remains L3 VERIFIED by exact RStats/XY-C1 runs `44519760957` and `44519760903` (Build/TRX/artifact all PASS).

---

## 2026-11-19 — Close the remaining ZeroTier checker string literal

- Exact ZeroTier run `44521351924` on `714a4eaa2f645887ff6a199562c06fbb3d76f73a` passed the shell lifecycle regression but failed when Python parsed `scripts/check_zerotier_lifecycle.py`: line 223 still lacked the closing double quote before `in test`.
- The checker line now uses a complete double-quoted Python literal around the shell grep pattern. No product source or shell regression behavior changes.
- L3 boundary is confirmed: product-source SHA `3523bb9f27ba33065731abcd946ae11726d04470` passed the RStats workflow, other exact-head checks and XY-C1 Build/TRX/artifact run `44519760903`. Physical-router L4 remains unverified.

---

## 2026-11-19 — Align ZeroTier NAT append rollback assertion with table-qualified CLI

- Exact ZeroTier lifecycle run `44520354130` on `4d015c02459d94b6d904cfe7dd0e381bb3f5cfa8` progressed through the retry-state tests and failed in the later NAT append rollback case.
- The failing assertion still expected `-D POSTROUTING -o ztunit`; after the b1 production correction the helper correctly emits `-t nat -D POSTROUTING -o ztunit`. The preceding fake firewall-state check had passed, proving no previously-published NAT rule remained.
- Update the test assertion and add a source-contract requirement for the table-qualified delete. No production behavior or workflow filters change. Exact-head Actions remain required; no L3/L4 claim is made.

---

## 2026-11-19 — Complete ZeroTier NAT cleanup regression after table-order fix

- The source correction at `b1b621253ddb1bd2f3dc3f9d3cd2c5b39f98285d` now issues `iptables -t nat -C/-D` with the table option before the action. The exact-head ZeroTier run `44518868454` progressed past the former NAT-table parsing failure and reached the final all-rules-deleted assertion.
- That assertion counted only log lines beginning `-D`, so it omitted the now-correct table-qualified NAT commands beginning `-t nat -D`. This follow-up counts both filter and NAT deletion forms; the separate retry check still demands the exact saved NAT source rule after daemon, interface and kernel route disappear.
- No production behavior change in this test-only correction. Exact-head ZeroTier and XY-C1 Actions must be checked on its resulting SHA; no L3/L4 claim is carried forward.

---


---


---

## 2026-11-19 — Guard the ZeroConf IPTV interface environment

- Parent main SHA: `780e8e153a96d3732cdace2f2692a1c7c27dc7be`; source commit: `b7cfce1be8c56bb2f82c5e53919898cba3a3a542`.
- `zcip_viptv_main()` called `strlen(man_ifname)` immediately after `safe_getenv("interface")`. The existing DHCP interface policy explicitly accepts that the environment accessor may return NULL, so this path could dereference NULL before callback work began.
- Replaced the raw length call with `dhcp_wan_interface_name_is_valid(man_ifname)` and added `zcip_viptv_main` to the callback workflow's same source-contract loop used for udhcpc/zcip entrypoints. Existing policy regression covers NULL and empty names.
- Exact WAN DHCP callback `44510842900`, Process fail-closed `44510842966`, Process inventory `44510842982`, and AI handoff docs `44510843038` PASS on b7. XY-C1 `44510842879` was still in progress at Build; current b7 L3 remains pending.

---

## 2026-11-19 — Verify Inadyn rollback and private cache directory

- Production sequence: `01e965edf79d548510786dd29c78b61e92da6eb1` restores a previously running Inadyn daemon after pre-launch cache/config failure; `1f6ec97d4b46c674efb6b130c8c5c83970c8d683` hardens the cache directory to an owned 0700 directory; `03dc5a8f3a38561b8a33eb1a530d7a0da6dfc7ae` corrects the missing unistd declaration and misplaced test-fixture assignments.
- The initial Inadyn run on the permission change failed at policy-test compilation/fixtures. The follow-up's exact Inadyn run `44509496651` passed. Process inventory `44509496666`, DNSMasq `44509496752`, and WAN route `44509496774` also passed on 03dc.
- Exact XY-C1 `44509496895` passed Build, TRX verification and artifact upload on 03dc. The later b7 source change requires its own XY check.

---

## 2026-11-19 — Confine WAN sysfs counter paths

- Source commit: `2a490739eb0ef8d9da0a1772bb779fe686f837cd`.
- Removed direct interpolation of NVRAM-derived interface names into sysfs statistics paths. New policy enforces one bounded interface component, a fixed RX/TX counter allowlist, checked snprintf results, and strict unsigned-decimal uint64 parsing.
- Added traversal, invalid token, boundary, path truncation, sign/junk/multiline and overflow regression cases, compiled with GNU89 warnings-as-errors and ASan/UBSan in Process workflow. Exact XY-C1 `44505963750` passed Build, TRX verification and artifact upload.

---

## 2026-11-19 — Verify SkipDBv2 detached daemon lifecycle

- Source commit: `69e590769fbcb44e853820efac16112a3b60212e`.
- Added tri-state preflight and bounded RUNNING postcondition after `skipd` daemonization; added SIGTERM-only stop to shutdown/firmware teardown so the persistent database can commit and close cleanly.
- SkipDBv2 regression `44502462286` passed. Exact XY-C1 `44502462390` passed Build, TRX verification and artifact upload on this source SHA.

---

## 2026-11-19 — Align downstream lifecycle contracts with SmartDNS's locked restart wrapper

- Parent main SHA: `03dc5a8f3a38561b8a33eb1a530d7a0da6dfc7ae`.
- SmartDNS restart intentionally delegates the complete stop/reconfigure/start transaction to `/usr/bin/smartdns.sh restart`, where one shell lifecycle lock spans the operation. Two downstream consumers still required the generic C stop/status/start shape: PID 1 notification's static contract failed at its generic `return start_smartdns();` assertion (run `44507439447`), while the storage safe-remove checker failed at the matching `stop_status = stop_smartdns()` assertion (run `44508947435`). Their preceding C regression/caller-graph steps passed.
- The PID 1 workflow now checks SmartDNS's exact locked-shell delegation separately while retaining generic restart assertions for other services. `scripts/check_storage_safe_remove.py` gets the same narrow SmartDNS case; all other service stop-result gates remain mandatory.
- No production source change is included. A local checkout is unavailable; exact-head PID 1, USB storage and AI docs Actions are required to validate this source-contract milestone. Latest production-source XY-C1 L3 remains NOT VERIFIED.

---

## 2026-11-19 — Validate NVRAM WAN interface names at PPPoE config and process boundaries

- Main sequence: `1ed9992e8af14913fdf2f030de70206b8140197a` (PPP options source/checker), `213546dfb420c350c3754eb9cb9b2b6d706dd350` (PPPoE relay argv and path-filter coverage), and `72fc0604832ff6fa3f3cfc7a152a0895b5e74896` (static checker boundary correction).
- `launch_wan_pppd_locked()` previously formatted raw `get_man_ifname(unit)` into a line-oriented `pppd` options file as `nic-%s`. A newline/control token could create additional privileged PPP options. The launcher now validates and bounds the name before session invalidation or file publication and writes only the local validated token.
- `set_passthrough_pppoe()` now validates the management interface before passing it as one argument to `pppoe-relay`. The WAN interface policy checker covers both process/config callsites; the focused workflow watches `net.c` in push and pull-request events.
- WAN callback `44500974469` passed on `1ed9992`. Run `44501588287` on `213546` failed only because the static checker searched for a literal escaped-newline function boundary; the ASan/UBSan interface-policy binary passed. Commit `72fc060` corrected that delimiter; exact WAN callback `44501725249` passed.
- On `213546`, NVRAM `44501588395`, AI docs `44501588399`, HTTPD `44501588392`, Process inventory `44501588327`, Process fail-closed `44501588361`, Firewall `44501588307`, WAN route `44501588455` and USB storage `44501588302` passed. XY-C1 `44501588862` was still building; no L3 claim for that SHA.

---

## 2026-11-19 — Bound NVRAM WAN interface names before fixed-size copies

- Source commit: `a6cf1fd26584614fe9212cd7be55f4a2f05cb3c0`; checker follow-up: `9f2367b578d376593d4a32d2849cba97708961a7`.
- Replaced `strcpy(wan_ifname, ifname)` in `get_wan_ifname()` with an errno-style bounded token copy and propagated its return through miniupnpd, Inadyn and firewall startup. The separate Inadyn management-interface `strcpy()` path is also removed.
- The helper rejects null/empty, overlong, whitespace/control and config-token metacharacters, clears a valid destination on failure, and copies only after validation. Regression covers maximum `IFNAMSIZ` boundaries, overflow, invalid bytes and invalid destinations; it is compiled under GNU89 `-Wall -Wextra -Werror` plus ASan/UBSan.
- Exact WAN callback `44499846484` passed on `9f2367b`. Exact XY-C1 run `44499578824` on production SHA `a6cf1fd` completed Build, TRX verification and artifact upload.

---

## 2026-11-19 — Restore eventual AP DHCP hostname-file recovery

- Source commit: `aeec47de88127b46a722ee426549349f3e54de73`; stale test-invocation correction: `f9d54df535c24e09ee9d7781003c180d38f896cf`.
- Found that an AP DHCP lease could commit while `create_hosts_lan()` failed, and both the volatile `lan_hosts_retry` marker write and immediate PID 1 notification enqueue could fail. In that case no durable later retry was guaranteed.
- Every AP-mode DHCP RENEW now refreshes host files even when the lease address/domain are unchanged, supplying a periodic recovery path; immediate notification retry is retained. This preserves the intentionally non-fatal post-commit lease behavior.
- Exact WAN callback `44496707114` passed after removing the obsolete retry-status helper invocation from the policy test.

---

## 2026-11-19 — Clean up failed USB RAS PPP launches and restore physical WAN

- Parent main SHA: `eb379476c57a11da1f8c05af92bd60a53b7902cf`.
- `launch_wan_modem_ras()` previously returned pppd `eval()` status directly. When pppd launch failed, that return bypassed the common stale-peer cleanup, leaving a credential-bearing `/tmp/ppp/peers/modem.wanN` file.
- The launch status is now captured; a successful launch retains its peer file, while a failed launch removes it and restores the original errno/status after best-effort cleanup. A GNU89 policy regression covers success, positive failure and negative failure.
- `start_wan()` now restores the physical WAN binding and firewall after RAS PPP startup failure through the established fail-closed rollback helper. The initial pppd failure remains the first error if rollback also fails; incomplete rollback is logged and the physical interface is taken down by the helper.
- `scripts/check_modem_wan_start_status.py` verifies the production cleanup and recovery ordering. Exact USB modem `44490980473` and WAN route `44490980282` workflows passed all steps; Process fail-closed `44490980113`, Process inventory `44490980140`, and NVRAM audit `44490980245` also passed on production SHA `3ae9fd9a7318ef3945c72d3ab313b9c0126927bc`.
- XY-C1 `44490980325` was still in progress at Build at the last poll; TRX and artifact upload were pending. No current-source L3 result is claimed.

---
## 2026-11-19 — Normalize wrapped C parser calls in switch source contract

- Parent main SHA: `3d6a245e86bcccf3af342b391cb420cdbe93fb8e`.
- Switch CLI run `44490029309` on 3d compiled and ran the strict IPv4/MAC/numeric/hex/decimal parser policy tests with GNU89 `-Wall -Wextra -Werror` ASan/UBSan successfully. The existing operational-status, IPv4 and ACL semantic contracts also passed.
- The new PHY/VLAN/raw-register source contract failed because it compared a full parser call against a one-line literal while the production VLAN parser invocation is line-wrapped. Normalize whitespace in the extracted PHY, VLAN and register command blocks; retain all exact argument, bound, mode and pre-ioctl checks.
- No production code changes or semantic assertions are weakened; exact-head checker rerun is pending. XY-C1 `44490029298`, Process `44490029325`, AI docs `44490029437` and NVRAM `44490029411` are tracked separately.

---

## 2026-11-19 — Strictly parse remaining PHY/VLAN/raw-register CLI numbers

- Parent main SHA: `752c880e995333ded4d93b4aa82186a537e40b34`.
- After strict ACL input gates passed Switch CLI workflow `44488416113`, a second scan found remaining `strtoul(..., NULL, base)` use in production `switch_gsw.c`: PHY address, VLAN dump maximum VID, raw register read/write/dump offsets and raw register write value.
- Added a decimal-only bounded parser (preserves leading-zero decimal input); PHY addresses are 0–31, VLAN max preserves the existing internal 1–4095 clamp, register offsets are bounded to signed-int representation, register-write data to 32 bits, and dump start offset leaves room for the final +0xfc read. Exact `r/w/d` command tokens and argument counts are required.
- Parser tests cover leading zeros, valid boundaries, signs, whitespace, partial tokens and decimal overflow. Switch workflow contract checks all command branches reject malformed input before ioctl/MMIO. Exact-head regression and XY-C1 build are pending after this commit.
- ACL source SHA 752 Switch CLI `44488416113`, Process `44488416467`, AI docs `44488415970`, NVRAM audit `44488416220` PASS; XY-C1 `44488415852` still running at the last poll.

---

## 2026-11-19 — Validate MT7621 ACL CLI values before switch MMIO writes

- Parent main SHA: `bc4e2c0ce204af2b3827fcaf6cbed17343139092`.
- Inspection of production `trunk/user/utils/src/switch/switch_gsw.c` found three ACL handlers still using permissive `strtoul(..., NULL, ...)`: EtherType, L4 payload and source port. Each also checked only the first seven bits of an eight-character port map, and silently returned for too few arguments without rejecting extra arguments.
- Require exactly six arguments, use strict bounded hex parsing for EtherType/L4 payload (16-bit), strict base-0 range parsing for the source port (16-bit), and the shared full eight-character port-map parser before the first register write. Keep the hardware register-write sequence unchanged.
- Add parser unit cases for optional 0x prefixes, boundary values, malformed/partial/signed/whitespace inputs, overflow and output stability. Add an exact source contract for all three ACL handlers to the existing GNU89/ASan/UBSan Switch CLI workflow.
- Exact-head Switch CLI and XY-C1 validation are pending on the resulting commit; no local MIPS build or physical PHY/router result is claimed.

---

## 2026-11-19 — Fix HTTPD security workflow storage policy include path

- Parent main SHA: `2a65185fa4cb2910f80429b6b5450677a9fde25d`.
- Exact HTTPD security run `44486117403` on 2a passed its preceding NVRAM, wireless, DHCP, resolver and USB checks, then failed the block-storage whole-disk host regression compile because `storage_partition_probe.h` now correctly includes the staged public header `disk_inventory_policy.h`, but this workflow still compiled with only `-I trunk/user/rc/src`.
- Add `-I trunk/user/libdisk/src` to the standalone GNU89 `-Wall -Wextra -Werror` host command and add embedded assertions for the new include and compile path. The storage-hotplug workflow `44486117414` already passes on 2a; this closes the duplicate HTTPD CI consumer.
- Exact 2a AI docs `44486117156` and Process inventory `44486117196` PASS; NVRAM `44486117346` was still running at last check. No production source changes in this correction.

---

## 2026-11-19 — Fix staged XY-C1 build include for strict storage partition policy

- Parent main SHA: `82e6e9ec3bb6b16d11e218b6fd76d5dc10a90c96`.
- Exact XY-C1 run `44482869903` on d45 failed during Build XY-C1 at `storage_partition_probe.h:6`: `../../libdisk/src/disk_inventory_policy.h` is source-tree-relative, but RC source is compiled from staged `trunk/build/rc`, where the path points outside `trunk/user`. TRX and artifact upload were not reached.
- RC's Makefile already adds `-I$(STAGING_DIR)/include/libdisk` for storage-enabled builds, and libdisk's source Makefile installs its headers there. Change the probe to `#include "disk_inventory_policy.h"`; add `-I trunk/user/libdisk/src` to the standalone ASan/UBSan test compile.
- Extend the embedded workflow contract to verify the include name, host compile path, RC staged include flag, and libdisk header-install rule. No production build result is claimed yet for this fix.
- Exact d82 runs still in progress at the time of correction: ZeroTier `44484902778`, WAN route `44484902793`, XY-C1 `44484902954`, Process inventory `44484902785`, and AI docs `44484902809`.

---
## 2026-11-19 — Preserve ZeroTier NAT source segment across daemon shutdown

- Parent main SHA: `d4f540583704b4cb2299d0fcf7a4fd61acd7f599`.
- Review of `stop_zero()` after the interface-retention fix found that `del_rules()` re-queries the kernel connected route only after `kill_z()`. The ZeroTier daemon removes its `zt0` route with the interface, so the source-NAT POSTROUTING rule could be left behind with an empty `-s` operand.
- Capture the connected route segment before stopping the daemon, reuse it in `del_rules()`, and fail before process termination when NAT is enabled but the source segment cannot be captured. The test fixture exposes the connected route only while the daemon is running and requires firewall cleanup to use the saved segment after shutdown.
- Add static source contracts for capture order, saved-value reuse and the fail-closed NAT gate.
- Exact ZeroTier `44483203855` and AI docs `44483203814` PASS on d4. Process inventory `44482870006` and WAN route `44482869891` PASS on d45. XY-C1 `44482869903` on d45 was still in progress at last check. The new descendant's relevant Actions and firmware validation are pending; no L3/L4 claim for the new source SHA.

---

## 2026-11-19 — Repair ZeroTier source-contract marker and required handoff heading

- Parent main SHA: `d45cb7634cef62c5581ad5364a54ac4ba2b7927d`.
- Exact ZeroTier run `44482869900` passed `sh trunk/user/zerotier/tests/zerotier_lifecycle.sh`, including the new route delete failure/query/postcondition fixtures, then failed only because the source checker required the absent phrase `route cleanup failure` instead of the fixture's actual `failed static route deletion`. Align the checker marker to the fixture.
- Exact AI docs run `44482869908` failed because the handoff used `## Active task`-equivalent wording but changed the required literal heading to `## Active slice`. Restore the mandated section heading.
- Exact Process inventory `44482870006` and WAN route `44482869891` PASS on d45. XY-C1 `44482869903` was still running at this checkpoint; inspect its exact-head Build/TRX/artifact result later.
- This is a checker/handoff correction only; no production source or regression behavior changes. Rerun all relevant Actions on the descendant SHA.

---

## 2026-11-19 — Verify ZeroTier static-route deletion and preserve identity on cleanup failure

- Parent main SHA: `f873c65531a1a20f9a3f4bdce14b4c4df026f86b`.
- `zero_route del` previously suppressed each `ip route del` result with `|| true`; `stop_zero()` then removed the ZeroTier identity/configuration without confirming that configured static routes were gone.
- Match the exact destination, gateway and interface in `ip route show`; treat a verified absent route as an idempotent success, propagate inspection/deletion/parser errors, and query again after a successful delete to verify absence. Continue attempting later configured routes while returning an aggregate failure.
- `stop_zero()` now retains persistent identity/configuration and returns failure when any route state is unknown or cleanup remains incomplete. Failed startup route rollback logs unconfirmed cleanup.
- Add shell regressions for already-absent routes, failed delete, failed route inventory query, delete-success-with-route-still-present and partial route-add rollback; extend the source contract to pin exact-tuple matching and the configuration-removal gate.
- No local shell test ran because this environment could not resolve github.com for a checkout. Exact-head ZeroTier/Process/AI docs and XY-C1 Actions remain pending; no L3 or L4 result is claimed for this slice.

---

## 2026-11-19 — Align storage partition regression push and pull-request filters

- Parent main SHA: `379fd897be61a52ee215d768fbb6c745febb087f`.
- Exact storage hotplug run `44478957633` on `379fd897be61a52ee215d768fbb6c745febb087f` passed the strict partition-probe GNU89 `-Wall -Wextra -Werror` build and ASan/UBSan runtime, then failed in the embedded caller-graph checker at its exact assertion that every relevant path appear once in both push and pull-request filters.
- The new `trunk/user/rc/src/tests/storage_partition_probe.c` regression was present in push but missing from pull_request. Add the same explicit path to the pull-request list; do not broaden filters or remove any test.
- The preceding storage test-fix run `44478689416` on `41b6e0c` failed only because two valid fixtures lacked the newly required procfs header; `379fd89` fixed those and the later tests all passed. This follow-up addresses the remaining workflow governance contract failure.
- No local checkout test was run. Exact-head storage, AI docs and other triggered Actions are pending after this commit; no target build or hardware result is claimed.

---

## 2026-11-19 — Fix SD/MMC test fixtures for strict procfs header validation

- Parent main SHA: `41b6e0ca0cd29a4e2fd39036b92106d7670bde09`.
- Exact storage workflow `44478689416` on `41b6e0ca0cd29a4e2fd39036b92106d7670bde09` compiled both GNU89 test binaries with `-Wall -Wextra -Werror` and ASan/UBSan. The new partition-probe test failed at the old SD no-child fixture because it contained data rows without the now-required `/proc/partitions` header; the MMC sibling fixture had the same defect.
- Add the exact procfs header to both valid no-child/prefix-collision fixtures. Keep the separate malformed/missing-header case expecting UNKNOWN (-1), so strict header validation remains covered.
- The storage workflow failed at test runtime (not compile); embedded source-contract and later workflow steps did not execute. No production source changes.

---

## 2026-11-19 — Reject incomplete SD/MMC partition inventories before whole-disk mount

- Parent main SHA: `7cc78f44fc11aa3698ae1231ea3e1399360bcb38`.
- Whole-disk SD/MMC hotplug uses `storage_disk_has_partitions()` to decide whether mounting the base device is safe. Its old `sscanf` loop silently skipped malformed/overlong records and stopped at the first child partition, so malformed tail data could be hidden.
- Reuse `disk_inventory_policy.h` to require a complete newline-terminated procfs record, one exact header, strict unsigned decimal major/minor/block fields, and exactly one device token. Treat malformed/duplicate/missing header, truncated/overlong row, read error or incomplete tail as UNKNOWN (-1), which already blocks whole-disk mount.
- Continue scanning to EOF after a child is found; a malformed later row still invalidates the inventory. Add GNU89 ASan/UBSan regression for malformed, truncated, overlong, duplicate header, and malformed-after-match cases, plus workflow source-contract/filter coverage.
- Exact ZeroTier handoff correction SHA `7cc78f44fc11aa3698ae1231ea3e1399360bcb38`: ZeroTier `44477056786`, Process inventory `44477056791`, WAN route `44477056811`, AI docs `44477056873`, XY-C1 host job `44477057022` PASS. Build/TRX/artifact skipped on 7cc due to no firmware production input changes; exact 255 XY-C1 `44476582849` was still IN_PROGRESS when last checked.
- No local checkout or physical-router test available/performed; exact-head storage Actions are pending for this commit.

---

## 2026-11-19 — Explicitly enable NAT in ZeroTier missing-route regression

- Parent main SHA: `255334d0dbd7e98a1a63dde528b0e3de59aef9c1`.
- Exact ZeroTier run `44476582735` passed interface-preserving stop cleanup and route-add rollback, then failed because the “Missing NAT route” test inherited `TEST_NAT_ENABLE=0` from the previous case. The production `rules()` path correctly skipped kernel-route readiness when NAT was disabled.
- Explicitly enable `TEST_NAT_ENABLE=1` inside the missing-route case while retaining `TEST_ROUTE_MODE=none`; add a scoped Python source-contract assertion to protect the fixture.
- On parent 255: WAN route `44476582757`, AI docs `44476582739` and Process inventory `44476582787` PASS; ZeroTier `44476582735` exposed the fixture omission; XY-C1 `44476582849` remained IN_PROGRESS. Exact rerun pending.

---

## 2026-11-19 — Capture ZeroTier interface before stopping its daemon

- Parent main SHA: `f65813735c6f817338006777e81645b2015bc215`.
- Exact ZeroTier run `44474852372` failed at strict-nounset cleanup, and Process lifecycle run `44475349901` on descendant 83 failed at the same ZeroTier test. Intervening test-only commits `b2d3b94b4414d0dee4ba423791a9ad91ba175893` and `f65813735c6f817338006777e81645b2015bc215` seeded `zt0` and corrected the rule-count grep. The seed masks a production lifecycle defect: `stop_zero()` kills `zerotier-one` before obtaining its interface, even though the virtual interface can disappear with the daemon.
- Capture the interface before `kill_z()`; make `del_rules()` safe when the variable is unset in a fresh shell. The revised regression removes the test-global seed, exposes `ztunit` only while the daemon is running, then checks both firewall and static-route cleanup still target `ztunit` after daemon exit.
- The checked firewall append, static-route add failure and rollback tests remain enabled. Exact-head ZeroTier, Process lifecycle, AI docs and XY-C1 checks are pending for this production correction. No local checkout, MIPS build or physical-router result is claimed.

---

## 2026-11-19 — Count filter and NAT append tokens in ZeroTier rollback regression

- Parent main SHA: `b2d3b94b4414d0dee4ba423791a9ad91ba175893`.
- The exact ZeroTier run `44475876933` passed the earlier `zt0` fixture setup but failed the final five-rule assertion. The grep expression `' -A '` required a leading space and therefore counted only the two NAT commands; INPUT/FORWARD commands start with `-A` in column one.
- Replace it with the POSIX extended-regex boundary `'(^| )-A '`, which matches both a command-leading `-A` and the NAT form after `-t nat`. Add a source-contract assertion so the regression cannot revert to the incomplete count.
- Exact AI docs `44475876921`, process inventory `44475876966`, WAN route `44475876871` and XY-C1 `44475877010` PASS on parent b2; ZeroTier failed only at the count assertion after route failure handling. Exact rerun pending. Production ZeroTier code is unchanged.

---

## 2026-11-19 — Seed the ZeroTier interface in the strict-nounset lifecycle fixture

- Parent main SHA: `67a45de88cb4776cf464164a9d3784e8555e0546`; the fixture-only repair is based on current main `83b726a87e5948d38afee7885e050d82d2d03452`.
- Exact ZeroTier workflow `44474852372` failed at stop cleanup with `zt0: parameter not set`. The fixture sources production functions with `set -u`, but its stubbed `start_instance()` does not run `rules()`/interface discovery to seed the global interface variable.
- Seed `zt0=ztunit` to model the interface state that a successful production start establishes, and add a source-contract assertion so future fixture edits cannot silently remove it. This is a test-only correction; production route/firewall code is unchanged.
- On parent SHA 67a: WAN route `44474852328`, process inventory `44474852420`, and AI docs `44474852357` PASS; ZeroTier `44474852372` FAIL due to this fixture setup issue; XY-C1 `44474852399` was IN_PROGRESS at last poll. Exact rerun pending after this commit.

---

## 2026-11-19 — Propagate ZeroTier static-route installation failures

- Parent main SHA: `3ffb25f6357535965f095eba2bfd45a813c96db8`.
- A second false-success path remained after firewall append checking: `zero_route()` discarded every `ip route add` status, and `rules()` discarded the helper status. NAT-enabled ZeroTier could be reported ready while one or more configured LAN routes were absent.
- Validate the configured route count/enable flag before installation, quote route operands, fail on incomplete entries or an unsuccessful `ip route add`, and make `rules()` remove the published firewall rules plus attempt route cleanup before returning failure.
- Reworked the lifecycle fixture to exercise the production `zero_route()` implementation with mocked NVRAM/ip commands. It now injects a static-route failure after all firewall rules have been installed and verifies both firewall and route rollback, while preserving the existing stop-cleanup route assertion.
- The preceding firewall publication commit `3ffb25f6357535965f095eba2bfd45a813c96db8` has exact ZeroTier `44473915670`, process inventory `44473915585` and AI docs `44473915620` PASS; XY-C1 `44473915674` remained in progress. Exact-head results for this follow-up are pending. No local checkout, MIPS build, or physical-router test is claimed.

---

## 2026-11-19 — Fail closed when ZeroTier firewall rules cannot be installed

- Parent main SHA: `a4f6176bd774ab0ea44ab9515a367e1b8aac41d6`.
- Review found `rules()` ignored the exit status of all three INPUT/FORWARD and both NAT rule appends, then returned success. A router could therefore report ZeroTier as started while required interface firewall rules were missing.
- Added a checked firewall publisher used for all five required appends. Any rejected append logs the failure, attempts removal of the full partially published ZeroTier rule set, and returns nonzero so the existing startup path reports failure and attempts daemon cleanup.
- Extended the shell regression with injected failures on the second filter rule and second NAT rule; both cases require a nonzero result, prove later rules are not added, and verify rollback is attempted. The source-contract checker now requires every one of the five append paths to use the checked publisher.
- Parent baseline ZeroTier workflow `44473205688`, wireless radio `44473205481`, process inventory `44473205503`, AI docs `44473205559` and PID-1 notification queue `44473205689` passed on `a4f6176bd774ab0ea44ab9515a367e1b8aac41d6`. Parent Process lifecycle regression `44473205836`, XY-C1 `44473205687` and NVRAM audit `44473205667` were still in progress at last poll. This ZeroTier source change is in the XY-C1 watched `trunk/user/zerotier/**` path; exact-head CI is pending. No local checkout or device test is claimed.

---

## 2026-11-19 — Propagate radio control results from rc CLI applets

- Parent main SHA: `1d1a839cab45ea4db21d2fec1ba74028baa44338`.
- Ten `radio2_*` and `radio5_*` command branches ignored their manual radio helper return values, leaving main's default exit status zero after failed hardware transitions.
- Added state-aware CLI adapters without changing helper APIs: toggle checks pre-operation physical state; enable/disable checks pre-operation NVRAM configuration, matching its existing no-op condition; negative watchdog-settling errors are preserved. Every affected branch now assigns the adapter result to `ret`.
- Extended the wireless-radio source contract to require all ten CLI paths to return adapter status and reject direct unchecked manual helper calls.
- Parent PID-1 notification slice on 1d1a839 had exact wireless `44472557258`, Process inventory `44472557323`, PID1 queue `44472557326`, AI docs `44472557416`, Firewall `44472557194` PASS. Parent XY-C1 `44472557312` and NVRAM `44472557377` remained in progress at last poll; check them separately.

---

## 2026-11-19 — Retain PID-1 radio notification markers when transitions fail

- Parent main SHA: `ef26e97d5791fcb00c7f3e1af0693ad1bd440d4b`.
- All six PID-1 radio events discarded `control_radio_wl/rt()` results, leaving `action_result` at success. The notification queue therefore deleted the incomplete marker after a failed scheduled or explicit on/off transition.
- Preserved the legacy boolean changed/not-changed API. Added a state-aware adapter: a no-op succeeds only if already at target; an actual transition requires a positive changed result; otherwise `-EIO` is merged into the event result and the incomplete marker remains.
- Added GNU89 policy cases for no-op, successful transitions and failed transitions; wireless Actions now watch `rc.c`.
- RStats validation update: exact RStats host run `44470244091` PASS; exact XY-C1 run `44470078199` passed Build, TRX and artifact upload on production source `5e11aa501a9ae2ad16e2946f0004ebff3535574e`; NVRAM audit `44470244067` on ef26 PASS. The initial RStats run `44470078263` failed only on the double-escaped test NUL; test-only follow-up ef26 corrected it. No L4 hardware claim.

---

## 2026-11-19 — Prevent RStats atomic publishers from bypassing umask

- Parent main SHA: `c4d2080843e9a39d917daf5f03e5ec340d89b039`.
- Review found both RStats new-file publication paths set an explicit `fchmod(0666)` after `mkstemp()`. This is not equivalent to the former `open(..., O_CREAT, 0666)`: fchmod ignores the process umask, so newly created raw history and spool JavaScript files become world-writable.
- Added a shared creation-mode helper that computes `0666 & ~umask` and restores the inherited umask; existing regular-file modes remain preserved. Added GNU89 regression coverage for atomic raw history and JSON stream creation under umask `0027` (expected `0640`) and for umask restoration.
- Added the production policy header to the XY-C1 firmware path filter, and strengthened the RStats workflow source contract. Exact-head RStats and XY-C1 validation will be recorded after these runs complete; no local checkout test or physical-router result is claimed.

---

## 2026-11-19 — Restore exact atomic-main wording required by handoff validator

- Parent main SHA: `fea88d3f18d7aac9c46dda14fd2c0fc2cc2180bd`.
- Exact-head AI handoff run `44467726364` failed only because the mandatory handoff checker requires the literal phrase `one atomic main commit`; the current handoff stated the same intent as “one atomic commit” without the word “main”.
- Restore the validator-required wording in the cross-session mandate. No workflow/test coverage or source behavior changes.
- The exact WAN route checker correction on this parent passed run `44467726365`. On product source SHA `2ff30969d5e326d64fe2ca1857e31b97fbfe90b6`, WAN DHCP, process, firewall, wireless and other relevant checks passed; exact XY-C1 build was still pending at last checkpoint.

---

## 2026-11-19 — Normalize VLAN rollback contract whitespace after first exact-head failure

- Parent main SHA: `2ff30969d5e326d64fe2ca1857e31b97fbfe90b6`.
- Exact WAN route run `44467380635` compiled and ran every host regression successfully, including the new VLAN tri-state/rollback ownership ASan/UBSan binary. Its final `check_wan_vlan_setup.py` source-contract step failed on one checker-only formatting assumption: the production `vlan_interface_cleanup_status()` call wraps its second argument across lines, while the checker searched for a single-line spelling.
- Normalize whitespace for that assertion instead of changing production formatting or removing any coverage. The failed step was a false negative; no C compilation or policy test failed.
- At checkpoint, XY-C1 run `44467380728` for source SHA `2ff30969d5e326d64fe2ca1857e31b97fbfe90b6` and NVRAM audit `44467380654` were still IN_PROGRESS. WAN DHCP `44467380611`, AI docs `44467380609`, process inventory `44467380658`, HTTPD `44467380676`, firewall `44467380601`, wireless `44467380729`, Process regression `44467380725`, and USB storage `44467380839` PASS.
- This checker repair must be revalidated on its new exact SHA. No new firmware source changes in this follow-up.

---

## 2026-11-19 — Fail closed on VLAN interface probe errors and preserve incomplete rollback ownership

- Parent main SHA: `013d6f1ba896981b10299b4130e9d9a17b7c1eb9`.
- Found that `remove_vlan_iface()` and `create_vlan_iface_internal()` used `is_interface_exist()`, which maps every negative `get_interface_flags()` result to “absent”. Socket creation, permission, invalid-name and other inspection failures could therefore be mistaken for a missing VLAN; teardown could return success without deleting the interface.
- Added a tri-state VLAN inspection policy: only ENODEV/ENXIO are accepted as definitely absent; all other probe errors fail closed, with errno preserved across `get_interface_flags()` socket close. Both VLAN creation and removal use the checked inspection helper.
- Failed setup now attempts both down and delete cleanup operations, preserves the original setup failure, logs incomplete cleanup and publishes tracked ownership when a newly created VLAN may remain. This allows the caller's transaction rollback to make another cleanup attempt rather than losing the resource identity.
- Added GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy regression for present, absent, unknown and errno-zero probes, first cleanup error and retained ownership. Local compile/run passed in this environment. Exact-head WAN route and AI handoff Actions are pending; XY-C1 trigger/build status must be checked by exact SHA. No full firmware build or physical-router validation is claimed.
- Handoff and roadmap status remain anchored to this logical slice; re-fetch exact main and update handoff after Actions results materially change the next action.

---

## 2026-11-19 12:51 CST (UTC+8) — Restore the rolling development log and checkpoint IPTV CI

- Parent main SHA: `f5fcdb9321b255acdf3e601c3b25228e6c5c7aac`; this record accompanies the required handoff checkpoint.
- IPTV cross-board production changes are on source SHA `5e9ebe5d5da73d770754808eaf68fbfbedcaa199`: ownership structs are available outside the optional GMAC2 block, the feature block closes with a standalone `#endif`, and the common rollback adapter uses bridge-aware rollback or VLAN-only cleanup according to board features. The static IPTV address/netmask/gateway values are copied into bounded local buffers before temporary NVRAM publication.
- Exact-source WAN route run `44464340419` PASSed. The static NVRAM snapshot refactor exposed a stale inline WAN DHCP checker; after the checker was aligned to validation/copy-before-cleanup ordering, exact run `44464784723` PASSed on workflow tip `f5fcdb9`. XY-C1 run `44464340449` for source SHA `5e9ebe5` was still IN_PROGRESS at checkpoint; L3 is pending.
- AI docs run `44464784761` failed because the current handoff omitted the mandatory checkpoint rule; this commit restores the validator-required wording.
- Log integrity correction: commit `9a394c3a4fe21ffe034fab2b39c1122877647903` truncated the root log from its prior rolling window to one record. Restore all 46 intact dated records from the unchanged parent file at `3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04` verbatim, then prepend this record (47 total). No historic archive shards or index entries are rewritten.
- No local build or physical-device check was performed. This checkpoint does not claim XY-C1 Build/TRX/artifact success.

---

## 2026-11-19 — Scope switch MAC-table source contracts to function bounds

- Parent main SHA: `0b55f61c96fedd9d83b2d2f3a099a26286d89983`.
- Exact Switch CLI run `44453388456` on `42478d29` passed the expanded ASan/UBSan parser executable (`switch IPv4/MAC/numeric/port-map parser policy: PASS`) and failed at the following static assertion. The checker stopped the `table_del()` slice at `void table_dump`, an earlier function, accidentally including unrelated later code. Bound the source slice by the table function's own top-level closing brace; apply the same robust boundary to `table_add()`.
- The exact-parent XY-C1 run `44453387921` stopped in `check_modem_wan_start_status.py` because the firewall reset assertion expects `firewall_restore_transaction_prepare_reset()` within `firewall_rebuild()`. Trace the production reset path before deciding whether the checker or source is wrong.
- No production C change in this checker-only follow-up. XY-C1 L3 remains unverified.

---

## 2026-11-19 — Correct the table_del static checker slice

- Parent main SHA: `3fc43cb557382c5f4e1a8790bf7c88fe753ee049`.
- Exact Switch CLI run `44453388456` on `42478d29` ran the expanded policy binary successfully: `switch IPv4/MAC/numeric/port-map parser policy: PASS`. Its subsequent static check failed at line 55 because `table_del` was sliced until `void table_dump`, a function declared earlier in the file. The slice included later functions (and their unrelated `strtoul` calls). Stop the slice at the actual next function, `void table_clear`.
- The same-parent XY-C1 workflow `44453387921` failed in `check_modem_wan_start_status.py` on a firewall-reset source contract. Investigate the production reset path and intended helper before changing the assertion.
- Checker/handoff-only correction; keep workflow path filters unchanged. XY-C1 L3 remains unverified.

---

## 2026-11-19 — Strict MAC-table parsing and pre-MMIO validation

- Parent main: `f172f15c04d90e79d3135151589f1ae66fdd7f38`.
- `table_add()`/`table_del()` used `strtoul` without end-pointer validation, allowing malformed MAC/numeric suffixes to parse partially. `table_add()` also wrote ATA1/ATA2 before validating later arguments.
- Add strict host-testable MAC, base-0 bounded unsigned and seven/eight-character portmap parsers with success/failure and output-preservation cases. Validate argc and every field before first `reg_write()`; preserve register packing and filter default.
- Update existing Switch CLI ASan/UBSan policy test and static contract; existing path filters already cover touched paths. No local MIPS build/hardware test is claimed.
- XY-C1 `44451550350` was still building its ancestor SHA; require an identical SHA Build + TRX + artifact for L3.

---

## 2026-11-19 — Correct SIP/DIP source-checker symbol lookup

- Parent main SHA: f3d4e793064bd97bcc105e9102406df318f29ec9.
- Exact Switch CLI run 44450415160 failed before the GNU89 parser test because the inline checker looked up dip_add(...) as a substring and matched acl_dip_add(...) first. The production functions already include their local count guards; the checker now uses fully qualified void dip_add/dip_del/sip_add/sip_del signatures.
- AI handoff documentation run 44450415070 and Process inventory 44450414996 passed on f3d4e79; NVRAM boundary 44450415009 passed. XY-C1 run 44450415016 was still in progress at last poll.
- This follow-up changes only the source contract and handoff evidence. The skipped ASan/UBSan switch parser executable must run on the new exact SHA.

---

## 2026-11-19 — Guard SIP/DIP switch inputs before MMIO

- Parent main SHA: 19b125963c38efd5c2dda0e24fb574b40a07bf83. Source review found no local argc checks in dip_add, dip_del, sip_add or sip_del; top-level dispatch only guaranteed argv[2].
- Add local required-count checks (DIP add 5, DIP delete 4, SIP add 6, SIP delete 5) before argv dereferences.
- SIP/DIP add previously wrote switch registers before validating port-map format. Add a reusable host-testable eight-character binary parser (retaining the existing mapping of ports 0–6) and parse the full address and port map before the first reg_write().
- Extend the existing IPv4 parser test executable with valid/invalid port-map cases and output-preservation assertions; extend existing Switch CLI static integration checks. Keep path filters unchanged.
- Exact-head Switch CLI and AI-doc checks are required after this commit; no local MIPS build or hardware result is claimed.

---

## 2026-11-19 — Align HTTPD DHCP contract with transactional LAN lease state and recovery

- Parent main SHA: `7edb5d58b4ba61e1c68deab9aa1601f90c5339d0`; previous HTTPD exact run `44447312302` failed in `Verify DHCP lease input and renewal reconciliation`, after the GNU89 DHCP policy binary printed `DHCP lease input policy regression: OK`.
- The stale checker still required a direct `nvram_set_temp("lan_ipaddr_t", ...)` write; the implementation now publishes seven volatile lease tuples using `lan_dhcp_nvram_state_publish()` under the shared NVRAM transaction lock.
- Production follow-up `7edb5d5` introduces a dedicated one-way `dhcp_lease_nvram_restore()`: unlike the publish compensation helper, it attempts every previous value and never compensates a failed restore by reapplying the candidate lease. Regression covers a failed restore key and a close error after an applied restore.
- Replace the obsolete single-key assertion with transaction ordering, all seven keys, restore helper, close/write failure test and kernel/resolver rollback diagnostics. No workflow trigger expansion.
- Exact-head WAN callback `44447008179` and AI docs `44447008247` PASS on `d1d8473`. The HTTPD and related checks on `7edb5d5` require exact-head completion; XY-C1 L3 and router L4 remain NOT VERIFIED.

---

## 2026-11-19 — Add missing switch mirror and TrTCM argument guards

- Parent main SHA: `bef7889d16ca08912d126c3cf97c2fb8c1d04f2c`.
- Further source review after ACL argc fix found `set_mirror_to()` and `set_mirror_from()` read argv[3]/argv[4] with no minimum count, while the `acl_dip_trtcm()` guard allowed argc 6 even though the implementation reads argv[6] through argv[9].
- Add argc 4, argc 5 and argc 10 fail-closed guards respectively, using the now-nonzero `usage()` routine before any argument dereference.
- Extend existing Switch CLI workflow source assertions for all three helpers; reuse existing workflow and unchanged push/PR path filters.
- Initial ACL/usage fix Switch CLI run `44445815633` passed on prior SHA `4f2ae0b`; this expanded contract needs exact-head verification. XY-C1 L3 and physical switch L4 remain unverified.
- No local tests or MIPS build performed.

---

## 2026-11-19 — Check the final WAN return after fail-fast exits

- Parent main SHA: `4f2ae0b46f16b60230888d6422fc9c3c505b90ef`.
- Exact `cc3f3d9` USB modem `44445491851`, Firewall `44445491599` and Process lifecycle `44445491797` all passed the corrected VLAN aggregate count but reached a further stale checker assumption: `wan_start.index("return wan_start_status;")` selected one of the newly required early abort returns.
- Change only the ordering contract to compare `set_passthrough_pppoe(1)` with the final `rindex("return wan_start_status;")`. This aligns it with the existing final-return check earlier in the same script; no production behavior changes.
- AI handoff docs on parent `cc3f3d9` run `44445491600` passed. Parent WAN source-check runs failed at the described assertion; current exact-head reruns still required.
- No local tests or MIPS build performed. Router L4 NOT VERIFIED.

---

## 2026-11-19 — Harden switch CLI invalid ACL invocation handling

- Parent main SHA: `cc3f3d9d3c9261e394f87a20c93dd9874f870c79`.
- L1 found ACL invocations such as `switch acl dip` (argc 3) passed the old argc guard and dereferenced missing `argv[3]`, risking a NULL-pointer crash. ACL dispatch now requires at least four arguments before subcommand access.
- The shared `usage()` handler returned status 0 for malformed/unknown commands; it now closes the descriptor and exits `EXIT_FAILURE`, so callers can detect invalid syntax.
- Extend the existing Switch CLI workflow source contract to require the nonzero usage exit and guard-before-`argv[3]` ordering. Reuse existing path filters; no workflow or trigger expansion.
- Exact-head Switch CLI Actions and XY-C1 Build/TRX/artifact checks are pending. WAN count correction USB/Firewall/Process/AI docs Actions on its parent are also pending.
- No local C test/build or hardware run performed; switch hardware L4 NOT VERIFIED.

---

## 2026-11-19 — Update WAN accumulator source-contract occurrence count

- Parent main SHA: `cd85e8681b6ae88836a98eaca0e8d7569f8ada78`.
- Exact `cd85` USB modem `44445137405`, Firewall `44445136993` and Process lifecycle `44445136954` all reached the final WAN checker and failed at the same hard-coded count of 19 `vlan_result` first-error merges.
- The callback/VLAN/APCLI-WISP follow-up correctly introduced two additional aggregate merge sites, so the complete `start_wan()` body now contains 21 matching `vlan_result` merges. Update the test contract to 21; production source is unchanged.
- Earlier GNU89/ASan/UBSan policy binaries in these jobs passed before the static checker failed. Exact-head USB/Firewall/Process and AI docs reruns remain required on the correction SHA; XY-C1 build stages must be verified on that same source SHA.
- Hardware L4 NOT VERIFIED; no local build/test performed.

---

## 2026-11-19 — Match WAN checker to compact whitespace representation

- Parent main SHA: `1c368975b354e4b8e7277146f86891cf6cb34f55`.
- Exact-head USB modem `44444344542`, Process lifecycle `44444344676`, and Firewall `44444344609` all failed at the same assertion. Logs confirmed policy binaries passed; the checker applied `compact()` (one space for each whitespace run) but expected expressions with no spaces. The first attempt's correction mistakenly removed spaces rather than matching the compact form.
- Correct callback and setup aggregate/cleanup literals to the actual compacted source representation. No production C behavior changes.
- AI handoff contract on parent `1c368` passed run `44444344544`; exact parent WAN route and Process inventory were successful. These parent results do not validate the new checker commit.
- Exact-head reruns required for USB modem, Process lifecycle, Firewall and AI docs. XY-C1 remains separately subject to same-SHA Build/TRX/artifact inspection.
- No local test, MIPS build or hardware verification performed. L4 NOT VERIFIED.

---

## 2026-11-19 — Correct WAN source-check normalization and handoff checkpoint contract

- Parent main SHA: `18a73adeea90de2935a07cad347e1f214a76966c`.
- The exact-head USB modem, Process and XY-C1 failures traced to the new source checker comparing whitespace-normalized C text against unnormalized expected strings. Host GNU89/ASan/UBSan policy tests themselves passed. Correct the expected normalized expressions; production behavior is unchanged.
- The AI handoff checker found that the refreshed `SESSION_HANDOFF.md` omitted the mandatory checkpoint heading/sentence and required section names. Restore the machine-checked governance contract and retain the exact prior-SHA failure evidence and next action.
- Exact parent WAN route run `44443755126` passed. USB modem `44443755296`, Process lifecycle `44443755214`, and XY-C1 `44443755078` failed on the source-checker assertion; AI docs `44443755298` failed only on the missing handoff governance text. All are stale evidence for the parent SHA and must be rerun on the correction commit.
- No source runtime change or workflow expansion is made in this correction. New exact-head checker and AI docs reruns are required; Build/TRX/artifact status must be checked separately.

---

## 2026-11-19 — Preserve the first WAN error across fail-fast preflights

- Parent main SHA: `8e0763554388cf4dfcbc985c0712a991b3bb3c14`.
- Follow-up review found three remaining `start_wan()` early exits bypassing `wan_start_status`: PPP callback-link preparation returned errno directly; WAN VLAN and AP-client WISP configuration returned their own later status. Any earlier TCP/conntrack/IGMP/monitor failure could be hidden by one of these later aborts.
- Capture callback errno before logging (EIO fallback), merge the callback, VLAN and WISP failures chronologically, and return the accumulated first error while retaining existing rollback and fail-fast behavior.
- Add GNU89 policy regression `test_early_wan_abort_preserves_first_error()` and strengthen `check_modem_wan_start_status.py` to require all three aggregate-before-return branches and prohibit direct-return regressions. Existing USB modem/WAN/Process/XY-C1 paths already cover the changed product/test/checker sources; no workflow or path filter was broadened.
- Prior exact parent USB modem `44442187961`, Process `44442187890`, WAN route `44442187838`, and AI docs `44442187852` passed. Parent XY-C1 `44442187996` was still building (job `135285479772`) at last poll; it does not validate this follow-up SHA.
- Local host tests and MIPS build NOT RUN in this GitHub-connector-only session; exact-head regression/Process/WAN/AI reruns and XY-C1 Build/TRX/artifact remain required. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Preserve chronological first error across all WAN startup paths

- Parent main SHA: `25de09a671c77fb2633f98c4eab34c28cd21bd22`; frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains unchanged.
- L1 found a result-order defect in `start_wan()`: early/global failures were held in `wan_start_status`, USB NDIS/RAS outcomes in `modem_start_status`, and the final merge always put the modem accumulator first. A later modem error could mask an earlier NAT/sysctl/WAN error.
- Remove the split accumulator, merge USB NDIS/RAS failures into `wan_start_status` at their execution points, and return the single chronological result. Existing best-effort startup control flow remains unchanged.
- Add `test_first_wan_error_survives_usb_modem_and_later_wan_failures()` to the GNU89 policy suite. Update modem-WAN, WAN-firewall and initial-MTU source contracts; existing workflow path filters cover these files, so no new workflow or broad trigger is introduced.
- Parent exact-head WAN gateway ARP `44440477965` and AI docs `44440477700` PASS. No local test or MIPS build was run here. USB modem, Process, WAN-route, AI docs and XY-C1 Build/TRX/artifact runs for this new commit remain pending; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Fail closed when detect_wan cannot publish its PID identity

- Parent main SHA: `9e7882b2354e57a4408f099b73d054b3a6635b2c` (includes concurrent storage mount decoding and regression fixture corrections).
- L1 found `detect_wan_main()` ignored `fopen(DW_PID_FILE)`, `fprintf()` and `fclose()` failures, then entered `poll_gateway()`. A permissions, space or I/O error could leave a missing/partial PID file while the long-running monitor continued; the adjacent `detect_link` monitor already uses the shared strict PID/start-time writer.
- Replace the ad-hoc PID-only write with `process_pidfile_write(DW_PID_FILE, "detect_wan")`. Any inventory or atomic publication failure now logs errno (EIO fallback) and aborts before gateway polling. Normal monitor completion still removes the PID file.
- Extend the existing Detect WAN ARP workflow contract to require the shared PID helper, guarded publish-before-poll ordering, errno fallback, controlled error log/return, and absence of unchecked `fopen`. Existing Process lifecycle path filters already include `detect_wan.c`; no workflow or trigger expansion is needed.
- Parent source-storage run `44432765082` had failed only a narrow literal check; correction `16e42d02368b485781c1ce1541dc513f413a7647` is followed by mount decoding/test-fixture changes `66a0a5d` and `9e7882b`. Latest exact-head Storage hotplug `44434391034`, HTTPD security `44434391030`, Process inventory `44434391084`, and NVRAM `44434391143` were still in progress/pending when this slice began; poll their results.
- No local shell test or MIPS build was run in this connector-only session. Exact-head Detect WAN, Process, AI docs, and XY-C1 Build/TRX/artifact checks are required; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Match storage mount checker to explicit errno fallback

- Parent main SHA: `35235da0cb4af66a8dedd96b7698948ec6b15687`.
- Exact storage hotplug run `44432765082` passed the preceding full caller-graph/lock-order checks and then failed its new mount-inventory source contract. The implementation correctly preserves the original parser/read/close errno, substitutes EIO when no errno was set, and returns failure; the checker incorrectly required the narrower literal `errno = saved_errno;`.
- Tighten the test to require the actual stronger implementation expression `errno = saved_errno ? saved_errno : EIO;`. No production storage behavior or failure gate is changed.
- Parent exact checks: AI handoff `44432765070`, Process inventory `44432765047`, NVRAM transaction `44432765261`, and HTTPD auth/security `44432765061` PASS. XY-C1 `44432765098` remained IN_PROGRESS at this checkpoint; Build/TRX/artifact must be checked on the exact source SHA.
- No local test or MIPS build was run. Exact-head storage hotplug and XY-C1 verification remain required.

---

## 2026-11-19 — Preserve VPN server up/down hook failures

- Parent main SHA: `6e7e90efb393af3380f0b8607a53ecb8921ba306` (concurrent NFS daemon wait-status fix).
- The VPN server PPP callbacks already preserve route, lease, relay and lock failures, but silently ignored the return value from the configured `VPN_SERVER_UPDOWN_SCRIPT` on both connect and disconnect. A custom hook could fail to install user routing/filtering or fail teardown while the callback still returned success.
- Capture both `_eval(hook_argv, ...)` statuses, log the peer and exact status, and mark the callback failed without skipping subsequent independent cleanup. Add a route-policy regression for custom hook failure propagation and source contracts for both callbacks.
- Exact parent VPN-server lifecycle `44431781964`, WAN route-status `44431782135`, Process regression `44431782106`, Process inventory `44431782022`, and AI handoff docs `44431782169` all PASS on `521c25a387bc45041f4db1e7ca9b27318e7b0e5e`. Exact XY-C1 `44431782312` was still building: Build in progress; TRX and artifact pending. This new hook change requires fresh exact-head L2 and L3.
- No local tests or MIPS build were run. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Propagate VPN server per-peer route failures

- Parent main SHA: `aa3691d7db9efbdbf0020a5b8c58d5734a579e9f`; includes concurrent Crond/shared-daemon probe-wait normalization and its checker/test corrections.
- L1 found `vpns_route_to_remote_lan()` returned void and discarded `route_add()`/`route_del()` results. PPP up/down callbacks therefore returned success even when a configured per-peer remote-LAN route could not be installed or removed.
- Convert the helper to return route status, reject invalid peer/interface inputs, treat `EEXIST` (route already present) and `ESRCH` (route already absent) as idempotent success, and preserve/log other route errors. Both callbacks mark failure while continuing independent lease/firewall/relay work.
- Extend the existing route-status regression with first-error preservation and the VPN-server lifecycle checker with source contracts. Add route policy header/test to VPN-server Actions push/PR filters and the header to XY-C1 rebuild triggers; no new workflow added.
- On parent, VPN-server lifecycle `44430527476` PASS; Process integration/inventory `44430527367`/`44430527397`, WAN route status `44430527641`, XY-C1 `44430527521`, and NVRAM `44430527357` were queued/in progress at last poll. Inspect final conclusions.
- IAPP `007a38b34c5c7b8127f31a2aeb427e08833cdf67`, iNIC signal `82ac332f84dbe8f454ad63247d4db53acac30c23`, and RT3352 iNIC monitor `41c2382d45e0496fdd336fc09bc2873d9216d553` are L3 VERIFIED by exact-source XY Build/TRX/artifact runs `44427295060`, `44427595460`, `44428277197`.
- No local tests or MIPS build were run. Exact-head VPN-server, WAN route-status, AI docs, Process and XY-C1 Build/TRX/artifact checks are required; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Propagate VPN-client callback route failures

- Parent main SHA: 79422b3cc31fea82dd5905720e1f31b9ea037bec.
- L1 found both VPN-client PPP callback route helpers were void and discarded every route_add()/route_del() result. ip-up therefore returned success and set the connected state even when configured split-default or remote-LAN routes failed; with vpnc_dgw enabled, a missing half of the two /1 routes can cause traffic to use the ordinary WAN route instead of the VPN. ip-down also hid failures to remove stale callback routes.
- Convert both helpers to return status, validate required callback interface/peer values only when the corresponding route is configured, normalize route-add EEXIST and route-delete ESRCH (already absent) as idempotent success, and preserve other errors. Both callback handlers attempt all independent route operations and return the first failure; optional up/down scripts also have their execution result preserved.
- Extend the existing route-status policy tests for idempotent delete and errors; extend the existing WAN route-status workflow with VPN-client source assertions and push/PR path coverage. No new workflow was added.
- Parent wireless iNIC monitor status source SHA 41c2382d45e0496fdd336fc09bc2873d9216d553 is in exact XY-C1 Build; it requires the result to be polled. Current head before this change is test-only correction 79422b3cc31fea82dd5905720e1f31b9ea037bec.
- No local shell/C test or MIPS build was run. Exact-head WAN route status, AI docs, Process and XY-C1 checks required. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Update wireless iNIC source contract for captured startup status

- Parent main SHA: 41c2382d45e0496fdd336fc09bc2873d9216d553.
- The new iNIC launch-result aggregation passed all preceding policy unit binaries in Wireless radio run 44428277432, but its inline source contract stopped at the legacy literal start_inicd() != 0. This assertion became stale after production code correctly captures the launcher result in inicd_start_status; it was the first failure reported by the checker.
- Update the static contract to require the exact captured start_inicd() result and its propagation through wireless_radio_status_success(). Do not weaken the contract or change production code.
- Exact Process inventory run 44428277260 passed on parent SHA. Wireless radio 44428277432 failed at the stale assertion. XY-C1 44428277197 was still running when polled and its host checks/build progression must be inspected separately. Parent iNIC signal SHA 82ac332 host/iNIC/Process/NVRAM/AI checks all passed; XY-C1 44427595460 was still building at last poll.
- No local tests/build run. This is a test contract correction; exact-head Wireless radio/AI/Process checks are pending. It cannot itself establish XY-C1 L3.

---

## 2026-11-19 — Propagate RT3352 iNIC health-monitor startup failures

- Parent main SHA: `82ac332f84dbe8f454ad63247d4db53acac30c23`.
- `trunk/user/rc/src/net_wifi.c::start_wifi_ap_rt()` under `USE_RT3352_MII` logged a failed `start_inicd()` but never merged the failure into `all_started`, so the radio restart could return success with its enabled health monitor missing. The corresponding stop path already treats stopping iNICD as a prerequisite before PHY isolation.
- Capture and report the monitor launch status and merge it into the existing sticky radio startup result. Preserve the deliberate no-op when `inic_disable=1`. Added a status-policy regression and a source contract for the NVRAM gate, launch, log, aggregation and final return order.
- The parent iNIC signal workflow `44427595739`, Process inventory `44427595469`, AI docs `44427595502`, and NVRAM audit `44427595602` passed exact parent SHA. Parent XY-C1 run `44427595460` was IN_PROGRESS at last poll; the IAPP production-source XY run `44427295060` was also IN_PROGRESS. No exact L3 evidence is claimed until final Build/TRX/artifact steps pass.
- Local tests and MIPS build NOT RUN (connector-only environment). Exact-head Wireless radio/Process/AI and XY-C1 Build/TRX/artifact checks are required; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Fail closed when iNIC auxiliary signal dispositions cannot be installed

- Parent main SHA: `007a38b34c5c7b8127f31a2aeb427e08833cdf67`.
- L1 found `inicd_main()` checked installation of SIGALRM/SIGTERM handlers but ignored all four `sigaction()` results when setting SIGPIPE, SIGHUP, SIGUSR1 and SIGUSR2 to SIG_IGN. On a disposition-install failure, the daemon still launched with the affected signal's default behavior; e.g. an unexpected SIGHUP could terminate the monitor while the code believed it was ignored.
- The daemon now checks the complete auxiliary disposition setup before `daemon()`; any failure captures errno (or EIO fallback), restores the caller's signal mask and returns without publishing the PID file or launching the monitor. The pure policy helper has GNU89 regression coverage, and the existing iNIC source contract asserts all four checked calls precede daemonization and that failure restores the mask.
- Existing iNIC signal workflow already watches the production source, policy header, test and checker for both push and pull_request; no workflow was disabled or weakened.
- Parent IAPP SHA `007a38b34c5c7b8127f31a2aeb427e08833cdf67`: standalone IAPP `44427295051`, Process inventory `44427294988`, AI docs `44427294975` PASS; Process lifecycle `44427295029` and XY-C1 `44427295060` were in progress at this checkpoint. Build/TRX/artifact for this parent are not yet confirmed.
- No local C test or MIPS build was run in this connector-only session. Exact iNIC signal, Process, AI docs and XY-C1 Build/TRX/artifact checks for this source SHA are required. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Keep IAPP procfs diagnostics in the controlled service log

- Parent main SHA: `d06c6a31212a42c5ec5afec99124010b80fcc2a4`.
- The initial exact IAPP run `44426426321` emitted raw grep errors to stderr for the two intentional missing-procfs regression fixtures; the Process integration checker requires the lifecycle invocation to return no uncontrolled stderr. The production path already records each inventory error through `log_iapp()` and returns grep's exact status, so the raw grep diagnostic duplicated the controlled service log.
- Redirect only the two `grep -q` stderr streams to `/dev/null`; preserve exit status tri-state and fail-closed returns. The structured `log_iapp()` error remains authoritative, while expected inventory faults no longer leak raw command diagnostics to the caller.
- The fixture-only correction parent `d06c6a3` has IAPP lifecycle `44426901415`, Process regression `44426901495`, Process inventory `44426901507`, and AI docs `44426901440` PASS. Its XY-C1 workflow `44426901490` completed successfully but deliberately SKIPPED Build/TRX/artifact because it contained test/docs only; it is not L3 evidence for the radio inventory production source.
- Exact-head IAPP, Process and XY-C1 production Build/TRX/artifact runs for this source change are pending. No local shell test or local MIPS build was run; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Restore IAPP radio fixtures after fallback regression

- Parent main SHA: `0c7648e03312f2923b343339209ce57cdb58f600`.
- Exact IAPP workflow `44426426321` passed shell syntax and the new module/interrupt inventory-error branches, then failed because the added valid-fallback case mutated the shared fixture used by subsequent pre-existing lifecycle checks.
- Restore the original `mt76x3_ap` module and `rai0` interrupt fixtures immediately after the fallback assertion. This is a test-only correction; no production behavior changed.
- Exact-head IAPP, Process, AI docs and XY-C1 checks for the corrected regression remain pending. The preceding production-source SHA retains its independent Actions results; do not treat the failed harness run as L2 completion.

---

## 2026-11-19 — Fail closed on unreadable Ralink IAPP radio inventory

- Parent main SHA: `d7c6c57e47876b320bd42eb2f95f0542d6cb4cc6`.
- `trunk/user/ralinkiappd/iappd.sh::start()` previously treated every non-match from `grep -q` as proof that a radio marker was absent. A procfs read error (grep status 2) therefore selected the legacy `rax0` fallback and could launch the daemon against the wrong interface.
- The startup path now distinguishes grep status 0 (match), 1 (complete inventory with no match), and >1 (inventory error). An unreadable `/proc/modules` or required `/proc/interrupts` now aborts before launching `ralinkiappd` or publishing firewall rules; the verified no-match fallback is unchanged.
- Extended the existing IAPP lifecycle regression with both procfs read-error paths and a valid no-match fallback check. Existing IAPP lifecycle, Process lifecycle, and XY-C1 workflow path filters already include the changed launcher and regression file; no workflow trigger was broadened or disabled.
- The immediately preceding WAN monitor SIGTERM fix at `d7c6c57e47876b320bd42eb2f95f0542d6cb4cc6` passed exact-head WAN ARP (`44424808719`), Process regression (`44424808712`), Process inventory (`44424808772`), NVRAM (`44424808887`) and AI docs (`44424808821`). XY-C1 run `44424808889` completed Build, TRX verification and artifact upload on that exact preceding SHA.
- Exact IAPP lifecycle run `44426426321` on this source SHA failed at the later pre-existing `wrong radio arguments` assertion. The two new unreadable-procfs cases and valid fallback assertion had passed, but that fallback test left the shared module/interrupt fixtures changed for subsequent tests. This is a test-harness fixture leak, not a production-source failure; this follow-up restores the original fixture before remaining lifecycle cases. Process (`44426426254`, `44426426375`), AI docs (`44426426366`) and XY-C1 (`44426426264`) were still in progress at the correction point.
- Local shell tests were not run in this connector-only session. The follow-up fixture correction must pass exact-head IAPP lifecycle; Process, AI docs and exact-source XY-C1 Build/TRX/artifact results must also be inspected. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 07:10 CST (UTC+8) — Defer detect_wan SIGTERM cleanup to normal context

- Parent main SHA: `d98ef418844b40bac6c6e31df6480a24645dc47b`.
- L1 found `catch_sig_detect_wan()` directly called `remove(DW_PID_FILE)` and `exit(0)` from SIGTERM handler context. Neither function is async-signal-safe; if SIGTERM interrupts libc stdio/allocator state, shutdown can deadlock or corrupt process state.
- THIS COMMIT records SIGTERM in a `volatile sig_atomic_t` flag, checks it around bounded ARP receive/retry and DHCP renewal, replaces long waits with one-second shutdown-aware slices, and removes the PID file after the monitor loop returns in normal process context. Signal-action installation errors are no longer ignored.
- Added `detect_wan_signal_policy.h` and GNU89/Werror/ASan/UBSan coverage for SIGTERM-only sticky flagging; extended WAN ARP source-contract checks and added the new production header to XY-C1 push filters.
- Parent AP-LAN Process checker/inventory and WAN route runs `44422350218`, `44422350427`, `44422787189` PASS; AP-LAN XY-C1 source SHA `747bb165` is L3 PASS on `44421896203`. AI docs archive rotation PASS on `44424090829`. USB storage `44420519515` PASS; MentoHUST L3 `44415470536` PASS.
- Local tests NOT RUN (GitHub connector-only environment); exact-head WAN ARP, XY-C1 Build/TRX/artifact rerun pending. Physical-router L4 NOT VERIFIED.

---

## 2026-11-19 — Update WAN route workflow for the AP LAN PHY gate

- Parent main SHA: `094d488122768b104c54ce371fb92943ceca0689`. WAN route run `44421896213` on product-source SHA `747bb165c61051000dfe6426c0de808cfd96f8b1` failed after every preceding route/IPv6/C regression passed. The failing inline source check still expected the old PHY wait result to be held in `operation_status` and used `if (operation_status == 0)` as the DHCP gate. The production change deliberately moved the PHY result to `phy_wait_status` and uses `rc_lan_ap_dhcp_start_allowed_after_phy_wait()`.
- Update the workflow's source contract to require separate PHY result capture, first-error aggregation, independent monitor notification, and the PHY-only DHCP gate. Preserve ordering assertions and both push/pull_request path coverage; the policy header is already present in both allowlists.
- No production code changed in this test-contract update. Exact-head WAN route, Process and AI docs runs for the resulting commit are pending at commit creation. XY-C1 remains tied to product-source SHA `747bb165c61051000dfe6426c0de808cfd96f8b1` (run `44421896203`); local tests and physical-router L4 remain NOT RUN/NOT VERIFIED.

## 2026-11-19 — Narrow the AP LAN DHCP source contract to the post-notification gate

- Parent main SHA: `747bb165c61051000dfe6426c0de808cfd96f8b1`. Exact Process workflow run `44421896053` compiled and passed the GNU89/ASan/UBSan `service_start_status.c` regression and all earlier integration checks, then failed in the new source checker. The assertion that no `if (operation_status == 0)` existed anywhere in `start_lan()` was too broad: the existing required callback-link preflight uses that condition before the PHY wait. This was a test-contract defect, not a production-source or C-regression failure.
- Correct the contract to reject the stale operation-status gate only after `notify_run_detect_internet(60)`, while retaining the required ordered PHY capture, first-error aggregation, independent monitor notification and PHY-only DHCP gate. The production implementation and regression cases are unchanged. Also normalize the policy-header comment indentation.
- AI handoff docs run `44421896176` is SUCCESS on exact source SHA `747bb165c61051000dfe6426c0de808cfd96f8b1`. XY-C1 run `44421896203` and the corrected exact-head Process run remain pending at the time of this correction.
- No local compile/test was run (GitHub connector-only environment). Hardware L4 remains NOT VERIFIED.

## 2026-11-19 — Keep AP LAN DHCP launch gated by PHY readiness

- Parent main SHA: `a2d0b74112b4002073015dccc5fca88f6ccd43ab`. The preceding storage hotplug CI repair is verified: exact-head USB storage run `44420519515` completed SUCCESS on `a2d0b74112b4002073015dccc5fca88f6ccd43ab`; every step passed, including the shared-header-dependent RC service launch regression compile/run and firmware BusyBox ash Optware regressions.
- Review of `start_lan()` found the mandatory PHY settle wait and the independent `notify_run_detect_internet(60)` result reused `operation_status`. The monitor call overwrote the wait result before the DHCP gate and before first-error aggregation: a successful notification could therefore permit DHCP after a failed PHY wait, while a monitor notification error could suppress DHCP after a successful wait.
- Keep `phy_wait_status` separate, aggregate it before the independent monitor notification, and gate `start_udhcpc_lan()` only on the PHY wait postcondition. Monitor-notification errors remain visible in the aggregate status but do not change DHCP eligibility.
- Extend the GNU89 service-start policy regression with both cross-failure cases, and strengthen the Process integration checker to pin production ordering and Process/XY-C1 regression coverage. Existing workflow path filters already include every changed source/test/checker path; no workflow coverage was removed or broadened.
- No local compile/test was run (GitHub connector-only environment). Exact-head Process lifecycle, AI handoff docs and XY-C1 Actions for the resulting source commit are pending at commit creation. Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 07:24 CST (UTC+8) — Fix storage hotplug service-status test include path

- Parent main SHA: `b26b59b2175492567e2018a93bb41c78e1036bd0`.
- USB storage workflow run `44418388785` failed at `Compile and run RC service launch status regression`; its previous storage daemon stop policy regression passed. The new test includes `optional_daemon_policy.h`, which transitively includes `trunk/user/shared/src/process_inventory.h`, but this workflow passed only `-I trunk/user/rc/src`.
- THIS COMMIT adds the shared include directory and extends the workflow's inline source checker to enforce both include directories and push/pull_request filters for the policy and shared inventory headers.
- Crond/IPTV Process L2 is PASS on exact current parent main: Process umbrella `44419048581`, AI docs `44419048640`; dedicated crond lifecycle `44417198647`, inventory `44417198657` passed. MentoHUST source SHA `4f54d9f0b861091c884459bd178ab4b966439719` is XY-C1 L3 PASS on run `44415470536` (Build/TRX/artifact all successful). Crond/IPTV firmware L3 remains open because run `44418643024` skipped Build/TRX/artifact after host tests; the commit changed only CI.
- Local tests: NOT RUN (GitHub connector-only environment). Storage exact-head regression rerun pending; physical-router L4 NOT VERIFIED.

---

## 2026-11-19 07:14 CST (UTC+8) — Add the missing crond policy-header regression trigger

- Parent main SHA: `aabe9a0c1a2574bde10cb7b3d60025db7a57e3b0`.
- Parent commit `aabe9a0c` fixed the shared include directory for the Process and XY-C1 `service_start_status.c` GNU89/ASan/UBSan compile commands. Before that correction, the exact Process and XY runs failed on `optional_daemon_policy.h` -> `process_inventory.h` resolution, not on production source or MIPS compilation.
- The Process workflow still lacked `optional_daemon_policy.h` in both push and pull_request path filters. THIS COMMIT adds the transitive policy header to both sets and makes `check_process_lifecycle_integration.py` enforce symmetric header coverage and both compile commands' shared include path.
- Corrected parent-main Process `44418643022` and XY-C1 `44418643024` were IN_PROGRESS at the last poll; AI docs `44418643233` passed. This follow-up's new Process trigger should run the strengthened source contract on the exact new SHA.
- Local tests: NOT RUN (GitHub connector-only environment). L1 dependency diagnosis PASS; L2/L3 reruns pending; physical-router L4 NOT VERIFIED.

---

## 2026-11-18 — Make crond startup idempotent under repeated RC start requests

- Parent main SHA: `a9d9594dc1875e5dd7adf2f894667d2e6566d76e`.
- L1 found `trunk/user/rc/src/services.c::start_crond()` had detached-child readiness but no initial process inventory before `_eval()`. A repeated start request could start a second BusyBox scheduler, while an UNKNOWN inventory did not block launch.
- THIS COMMIT calls `optional_daemon_start_status("crond", "crond")` before the launcher: RUNNING is a logged no-op, UNKNOWN returns the inventory error, and ABSENT alone reaches `_eval()`. Existing bounded readiness and stop-before-restart behavior remain unchanged.
- Added ABSENT/RUNNING/UNKNOWN/invalid-state assertions to `trunk/user/rc/src/tests/service_start_status.c`; extended `scripts/check_process_lifecycle_integration.py` with an order/branch contract. Existing Process workflow compiles this GNU89 ASan/UBSan test and executes the checker; `services.c` is already in its and XY-C1 path filters.
- MentoHUST L2 on exact source SHA `4f54d9f0b861091c884459bd178ab4b966439719`: Legacy wrapper `44415470631` PASS, AI docs `44415470654` PASS, Process inventory `44415470822` PASS, lifecycle fail-closed `44415470569` PASS. XY-C1 `44415470536` was IN_PROGRESS at last check, so MentoHUST L3 remains pending.
- Local compile/tests: NOT RUN. Exact-head Process lifecycle, process inventory, AI docs and XY-C1 for the crond commit are pending at commit creation. Physical router L4 remains NOT VERIFIED.

---

## 2026-11-19 06:50 CST (UTC+8) — Fix the MentoHUST production LF/CR sentinels

- Parent main SHA: `6cda7c56e6e69b4b1ea38b0d618ab87d0ee46ecf`.
- After the function-separator correction at `b9c9500aac4489f5d1f3841c1db8e3d19095d6e0`, Process lifecycle integration passed run `44414594807`. Legacy wrapper run `44414594963` reached runtime and failed because LF injection was accepted. A follow-up commit `6cda7c56e6e69b4b1ea38b0d618ab87d0ee46ecf` corrected the CR regression fixture to produce a real carriage return and extended the static checker, but exact legacy run `44415154101` still failed at the LF case.
- Inspection confirmed the production `newline=$(printf ...)` and `carriage_return=$(printf ...)` formats still contained two backslashes. THIS COMMIT reduces each format to one, so shell `printf` creates the actual LF and CR control characters matched by the rejection case.
- Exact-head legacy wrapper, Process lifecycle, Process inventory, AI docs and XY-C1 validation remain required. No local shell test was run; L4 physical router remains NOT VERIFIED.

---

## 2026-11-19 06:07 CST (UTC+8) — Protect MentoHUST credential configuration at creation and reuse

- Parent main SHA: `3561edf47144278a2fa386358930cda901206ccf`.
- L1 review found PID 1 calls `umask(0000)` in `init.c`; MentoHUST previously wrote plaintext username/password to `/var/mentohust.conf` using ordinary shell redirections, so the inherited umask could expose credentials to every local user. Existing `/etc/storage/mentohust.conf` files were reused without permission tightening. The old sed-based value substitution also made CR/LF-bearing NVRAM values capable of adding configuration directives.
- THIS COMMIT hardens `trunk/user/mentohust/scripts/mentohust.sh`: set umask 077; write fields literally from a fixed NVRAM-to-config mapping; reject line breaks; create a same-directory mode-0600 temporary file; atomically rename; verify ownership against the executing UID; refuse symlink/directory config targets; and restrict reused regular files before daemon launch. A failed config publication blocks start.
- Extended `scripts/tests/legacy_wrapper_pidfile_safety.sh` with private-mode, literal special-character value, existing-config mode repair, symlink refusal, newline/carriage-return rejection, and no-launch-on-failure cases. Extended `scripts/check_legacy_wrapper_pidfile_safety.py`, including repair of its over-escaped `kill -9` matcher. The existing workflow already covers all touched source/test/checker paths; the XY-C1 workflow already lists the MentoHUST production script.
- Local tests: NOT RUN (GitHub connector-only environment; no local repository checkout). Exact-head legacy wrapper and AI handoff Actions: PENDING at commit creation. XY-C1 Build/TRX/artifact: PENDING at commit creation. Hardware L4: NOT VERIFIED.
- Remaining compatibility boundary: verify BusyBox `stat -c`, `id -u`, `mktemp`, `readlink`, ash behavior and persistent-storage ownership on target; physical XY-C1 runtime remains unverified.

---

## 2026-11-19 06:05 CST (UTC+8) — Close Redsocks heredoc-aware source contract

- Parent main SHA: `81348ba7fc8cc3affcc2fa5a4b48c87ad48b906d`. Shadowsocks lifecycle push run `44406248171` passed after the oversea profile regression PASS marker was correctly read from `tests/start_status.sh`. Earlier exact `5debbf4` run had shell syntax and every runtime regression pass; its source checker failed only on that stale marker location. The `5debbf4` XY-C1 run `44406037894` remained IN_PROGRESS at checkpoint (Build in progress, TRX/artifact pending).
- Redsocks source/test SHA `e01cb8e8a74ed9a22f2b81374679be460dea6bfb`: shell syntax and runtime tests passed, but the checker parser's generic brace scan stopped at a brace-only line inside the config heredoc and missed both start call paths. Change checker extraction to terminate at the next top-level `func_clean(){` declaration; do not weaken the existing callsite or test assertions.
- Update the handoff and roadmap to reflect live main `81348ba`, the Shadowsocks L2 PASS, and the pending Redsocks checker rerun. The archived backup remains untouched. No Redsocks firmware L3 or physical-router L4 claim.

---

## 2026-11-19 05:15 CST (UTC+8) — Verify Redsocks detached startup readiness

- Parent main SHA: `3dd89aa3d401f82ab32216cafb6f896ce458e451`. Exact-main Process inventory `44402419113` and Process lifecycle regression `44402419118` both passed, including the existing WireGuard lifecycle test path.
- `trunk/user/redsocks/redsocks.sh` configured the daemon with `daemon = on`, but both `func_start()` paths treated a successful launch command as proof that the detached child survived and logged `STARTED.` unconditionally. An immediate bind/config/runtime exit could therefore produce a false success.
- Add a bounded tri-state startup wait and shared launch wrapper: only confirmed RUNNING succeeds; ABSENT gets four one-second retries then times out; UNKNOWN or sleep failure aborts; the original launcher error is preserved. Both existing-config and generated-config paths use the same postcondition.
- Extend `scripts/tests/redsocks_lifecycle.sh` for delayed RUNNING, timeout, UNKNOWN, sleep failure, exact launch-status propagation and generated-config startup. Extend `scripts/check_redsocks_lifecycle.py` to require the production integration and test cases. Existing Redsocks workflow push/pull_request paths already cover all changed source/test/checker files; no CI filters are changed.
- No local checkout/test execution is available in this connector-only environment. Exact-head Redsocks Actions are pending until the atomic main commit is pushed. This shell-only add-on change does not trigger or justify a new firmware L3 claim; hardware L4 remains NOT VERIFIED.

---

## 2026-11-19 — Harden privileged WAN callback symlink ownership

- Parent main SHA: `8cfe4a01f116dc9f08baae9c858b3b98f2242749`. Latest WAN teardown checker SHA `8cfe4a01f116dc9f08baae9c858b3b98f2242749` has exact VPN `44393711494` and Process `44393711289` PASS. It follows full WAN teardown status capture from `2c94a4e`.
- Review found `create_cb_links()` ignored all `symlink()` errors at PPP, DHCP, ZeroConf and WPA callback paths; `remove_cb_links()` unconditionally unlinked fixed names under shared `/tmp`. Foreign files or attacker-owned symlinks could remain as privileged callbacks or be deleted by teardown.
- Added `ppp_callback_link_policy.h`: shared `/tmp` must be owned and sticky when shared-writable; callback links must be effective-UID-owned and target exactly `/sbin/rc`; `lstat/readlink` validates type/owner/target; EEXIST is accepted only after exact-link revalidation; cleanup removes only owned matching symlinks.
- WAN startup fails closed, tracks links created during this attempt, rolls back those links on subsequent publication failure and preserves errno. Early cleanup failures are logged. WAN stop propagates callback cleanup failure and skips final WAN state clear when an unsafe callback path remains.
- Added GNU89/Werror ASan/UBSan regressions for sticky-directory protection, idempotent create/remove, regular-file and wrong-target collisions, non-owner exact target, injected symlink failure and exact-target EEXIST race. Local compile/runtime PASS. Exact WAN route/teardown/Process/XY-C1 Actions pending for this source; no MIPS/router L4 claim.
- Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Align WAN teardown lifecycle contracts with exact status capture

- Parent SHA: `bc90e3b44af85fe5ad18a58f8f39bf36e5de680d`. WAN source SHA `bc90e3b44af85fe5ad18a58f8f39bf36e5de680d` exposed stale tests in exact-head WAN `44390606879`, VPN server `44390606952`, HTTPD security `44390607054` and Process lifecycle `44390606840`. The compile/policy test stages of WAN, VPN and HTTPD and prior Process host regressions passed before each static integration assertion failed.
- All four failures were assertions for old PPP stop predicates/return expressions, not compile errors: WAN expected the removed ternary status return; VPN, HTTPD and Process still looked for `stop_vpn_client_under_lifecycle_lock() != 0` after production code started assigning its exact result to `stop_status` before gating.
- Updated the contracts to verify exact result assignment, its subsequent error gate, non-reentrant under-lock helper usage, and the new primary/resume status policy in both WAN abort paths. No production code and no CI triggers/path filters were changed.
- XY-C1 run `44390607038` was in progress at last inspection. No L3 claim for the WAN source SHA; no local tests or physical-router validation performed.

## 2026-11-19 — Preserve WAN PPP teardown and monitor-resume failure status

- Parent SHA: `d8b5e9206024ba45208d95fbb1ff4dc5d73d0b4d`. Exact-head AliDDNS `44388958868`, FRP `44388958846` and AI handoff `44388958941` passed. Autostart production SHA `51596e1acf6b4842b80e1246911aac59e9dd2baf` completed XY-C1 Build, TRX verification and artifact upload `44386912939` (L3).
- WAN teardown review found `stop_wan_ppp_locked()` initialized `stop_status=-1` but never assigned callback-session fencing, VPN client stop or PPP daemon-stop results; all failure paths discarded the initiating status. PPP and full WAN abort handlers also ignored Internet-monitor resume errors.
- Added `wan_teardown_abort_status()` to preserve the original operation failure and surface a resume error when it is the only failure. PPP teardown now captures and logs the exact return from each operation; both abort paths log a failed monitor resume.
- Extended the existing GNU89/Werror C policy regression for primary-error precedence, resume-only failure and fallback. Extended existing `wan-teardown-stability.yml` source contracts. Existing path filters cover all modified files.
- No local compile/test, full firmware build or hardware-router validation was run. Exact-head WAN Actions pending; physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Align AliDDNS and FRP static contracts with shared NVRAM helper

- Parent SHA: `e0e24637fcb2901bef453812747a5f2e7b74f864`. Static-contract repair SHA `e0e24637fcb2901bef453812747a5f2e7b74f864`: AI docs `44388094912` and Process inventory `44388094850` PASS; AliDDNS `44388094895` and FRP `44388094871` failed at static source-check steps.
- AliDDNS checker passed neither the NVRAM change nor a firmware condition: its generic function extractor was given a signature with an opening brace on the same line, while production declares the function and opening brace on separate lines. It now anchors only to the function declaration while retaining status and CA-order assertions.
- FRP checker still required direct `nvram get` assignments after the common checked helper replaced them. It now verifies both helper calls, exact read-status propagation and the combined client/server enable decision; it also requires the failure-injection regression cases.
- FRP extracted shell regression and its status-9 cases passed before its separate static source-contract step failed. No production source or workflow trigger changed. No local tests executed; exact-head CI is required.

## 2026-11-19 — Repair AliDDNS and FRP contracts for shared autostart NVRAM reader

- Parent SHA: `51596e1acf6b4842b80e1246911aac59e9dd2baf`. Production SHA `51596e1acf6b4842b80e1246911aac59e9dd2baf`: Process lifecycle `44386912755`, DNSMasq `44386912932`, AI docs `44386912783`, Process inventory `44386913112` and HTTPD `44386912691` passed. XY-C1 `44386912939` was IN_PROGRESS; AliDDNS `44386912869` and FRP `44386912971` failed.
- AliDDNS host transport/API suites passed; its static checker failed on a stale literal requirement for the old inline AliDDNS NVRAM expression. Updated it to assert `autostart_nvram_read()` preserves nvram status, the enable wrapper propagates it, and the CA bundle gate remains ordered before launch.
- FRP tests passed lifecycle/watchdog/crontab before the isolated autostart fixture failed with `autostart_nvram_read: not found`: its extractor sourced only FRP functions. The fixture now imports the production reader and injects status 9 for both client/server state reads, asserting no launch and key/status logging.
- No production source or CI trigger changed in this fix; existing workflow filters cover the checker, fixture and common script. No local Python/shell tests were run. Exact-head AliDDNS/FRP and XY-C1 results remain required.

## 2026-11-19 — Report autostart feature-switch NVRAM read failures

- Parent main SHA: `55cf20ecfec45a5b519996c85921bce722e54d99`. DDNSTO source SHA `55cf20ecfec45a5b519996c85921bce722e54d99` passed exact-head DDNSTO `44385101598`, Process inventory `44385101669` and AI docs `44385101578`; XY-C1 `44385101749` was still building. RStats source SHA `1bf9a30d1003b9ad5835c86a6dfaeab1fae15961` passed exact-head RStats/Process/HTTPD/AI/NVRAM and XY-C1, with Build/TRX/artifact complete.
- `autostart.sh` expanded feature NVRAM reads directly inside test expressions. Failed reads were silently treated as disabled switches, hiding why an enabled optional service was not launched. FRP returned a generic failure without identifying the failed key.
- Added status-preserving `autostart_nvram_read()` and boolean `autostart_nvram_enabled()`; migrated every boot feature guard and FRP client/server read. On command failure, the dependent service remains fail-closed and the key/original status are logged; independent launchers continue.
- Extended the existing host shell regression with injected status-9 cases for AdGuardHome, DDNSTO and FRP. It verifies the affected service is skipped, unrelated services still start, and precise NVRAM/FRP failure status is logged. Existing Process and XY-C1 path filters already include both modified files; no workflow was broadened or disabled.
- No local shell test, full repository build or MIPS firmware build was performed. Exact-head Process lifecycle and XY-C1 verification for this source SHA are pending; physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Preserve DDNSTO on NVRAM enable-state read failure

- Parent main SHA: `1bf9a30d1003b9ad5835c86a6dfaeab1fae15961`. The preceding RStats history-read commit passed RStats `44384384582`, Process inventory `44384384359`, HTTPD `44384384054` and AI docs `44384384820`; Process fail-closed `44384384017`, XY-C1 `44384384072` and NVRAM `44384384235` were still in progress at implementation start.
- DDNSTO review found that `[ "$(nvram get ddnsto_enable)" = "1" ]` discarded the command's exit status. A read error was treated as a successful disabled state; restart additionally stopped the existing daemon before making the unchecked read, risking an avoidable service outage.
- Added `ddnsto_read_enable_state()` to preserve the original NVRAM failure status and use one checked result for start/restart under the existing lifecycle lock. Restart now completes the read before TERM/KILL or replacement launch.
- Added host shell regression cases injecting status 9 for both start and restart with an already-running daemon; tests assert preserved status, no signal, no launch, unchanged daemon state and exactly one enable read. The DDNSTO source checker verifies helper use and ordering. Existing DDNSTO and XY-C1 workflow path filters already include the packaged launcher, test and checker; no workflow was broadened or disabled.
- Also corrected the roadmap's stale DDNSTO status and recorded the completed five-wrapper RC call-graph audit (no C callers; dispatch uses separate int restart entry points).
- No local shell test, full repository build or MIPS firmware build was performed. Exact-head DDNSTO lifecycle, Process inventory and XY-C1 results for this packaged-source SHA are pending; prior L3 on `3c7bd2d` does not transfer. Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Harden RStats raw history loading against short reads and EINTR

- Parent main SHA: `1b3117769d52021de09ba09ffbee0118fae159de3fe`. `rstats.c::f_read()` previously performed one `read()` and returned its byte count; legal short reads caused `load_history_raw()` to reject otherwise valid history, and EINTR also aborted the read.
- Add injectable `rstats_file_read_exact_with()` / `rstats_file_read_exact()` helpers: accumulate short reads, retry EINTR, return the actual short count at EOF, reject invalid arguments and oversized lengths, and propagate non-EINTR failures. `f_read()` retries interrupted `open()`, uses the exact-read helper and preserves the read errno across `close()`.
- Add RStats fixture tests for one EINTR followed by short chunks, truncated EOF, EIO after partial progress, zero-length input, invalid arguments and oversized request. Extend `rstats-stability.yml` source-contract checks; existing push/PR filters already watch all modified source/test paths.
- A standalone helper harness compiled and passed under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. This does not replace the full repository test or firmware build. New exact-head RStats/Process/AI/XY-C1 Actions must be inspected; XY-C1 L3 requires Build, TRX and artifact success on the exact source SHA. Physical-router L4 remains NOT VERIFIED.

## 2026-11-19 — Align downstream checks with Inadyn locked helpers

- Inadyn SHA `3c7bd2d28bc6cc285fca3074dee4118fae159f66`: Inadyn regression `44381565885` (lock compile/run + production contract), Process inventory `44381563425`, Process fail-closed `44381563270`, HTTPD `44381563266`, miniupnpd `44381563068` and Samba `44381563561` PASS.
- WAN route `44381563038` failed because `scripts/check_wan_ddns_status.py` still parsed `notify_ddns_update(void)` rather than `notify_ddns_update_locked(void)`. DNSMasq lifecycle `44381563474` failed because its embedded checker expected the former public start/update bodies and nested stop call. This follow-up adjusts only those source contract signatures/calls, retaining all substantive assertions.
- AI docs `44381563154` failed because the prior handoff splice dropped the required `## Verification boundary` section. Restore it and record current exact-SHA evidence.
- XY-C1 `44381563540` remains IN_PROGRESS (Build active; TRX/artifact pending); NVRAM audit `44381563037` was pending. No firmware L3 or physical-router L4 claim.

## 2026-11-19 — Serialize Inadyn stop/start and configuration notifications

- Base main SHA: `b471b7e328b79c9eac412f2af701c51b9a573e19`. AI handoff documentation run `44377784741` PASS after restoring the literal Active task heading.
- Review found public `stop_ddns`, `start_ddns`, `notify_ddns_update` and `notify_ddns_manual_update` had no shared process lock. Concurrent RC processes could race stop/start for the `--no-pidfile` daemon or race config publication/reload/manual refresh against restart.
- Add root-owned `/tmp/.inadyn.lifecycle.lock` via `fcntl`; validate regular-file type, effective UID, single link and restrictive mode; open with `O_NOFOLLOW|O_CLOEXEC`; bound contention to 10 seconds. All public operations hold one lock through their transaction; internal `_locked` helpers avoid recursive acquisition.
- Add fork-based host regression for cross-process exclusion, release/reacquisition, hardlink/symlink rejection and invalid paths; compile/run it with GNU89, ASan and UBSan in the Inadyn workflow. Add explicit lock paths to XY-C1 and Process inventory workflow filters.
- Inadyn source SHA `3c7bd2d28bc6cc285fca3074dee4118fae159f66`: Inadyn `44381565885`, Process inventory `44381563425`, Process fail-closed `44381563270`, HTTPD `44381563266`, miniupnpd `44381563068` and Samba `44381563561` PASS. WAN route `44381563038` and DNSMasq `44381563474` failed due downstream checkers still expecting the pre-lock function bodies; AI docs `44381563154` failed due missing `## Verification boundary`. This follow-up aligns the checkers and restores the heading. XY-C1 `44381563540` remains IN_PROGRESS; NVRAM `44381563037` was also pending. L3/L4 are not claimed.

