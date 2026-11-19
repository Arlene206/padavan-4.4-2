## 2026-11-19 — Propagate SCUT crontab update failures into boot status

- `check_crontab.sh` uses exit 1 for a successful new entry and 0 for an existing entry, but append failure silently fell through to exit 0 and grep I/O errors were treated as “not found”. `init_crontab()` ORed the raw encoded `system()` wait status; `init_router()` consequently treated every nonzero error as “changed”. It also logged but discarded MTD persistence failure.
- Added the explicit 0=unchanged, 1=added, 2+=error contract, symlink/non-regular path rejection, checked grep/append/directory results and restrictive creation umask. RC now decodes wait status, preserves `system()` errno, persists only after a real append, and aggregates script/MTD failures while still attempting crond activation.
- Added GNU89/Werror/ASan/UBSan policy tests and isolated shell regressions for changed/no-op, bad arguments, missing storage, grep I/O, append target errors and symlink paths. Local C policy binary, shell syntax and shell regression all PASS. Exact-head Process lifecycle, AI docs and XY-C1 Build/TRX/artifact remain pending.
- Parent main: `930dd7ed850ef59275cc340bc7c5a36bc7ebedc9`; its AI handoff documentation check `44617795116` PASS. No local MIPS build or physical-router L4 result is claimed.

---

## 2026-11-20 05:09 CST (UTC+8) — Restore the AI handoff contract and catch up root log rotation

- Parent main SHA: `f4fbd093e934fda5304aa796cba30d59eb29b3a4`. Exact-head OpenVPN client credential staging run `44613650285` PASS; VPN server lifecycle run `44613173654` PASS; XY-C1 run `44613173721` completed Build, TRX verification and artifact upload on the OpenVPN source SHA `e01f70a766c4e628593e272192723f6add59ffca` (L3).
- AI handoff documentation run `44613650462` failed because SESSION_HANDOFF.md lacked the required literal `## Active task` section. Restored the active-task section without changing product source or weakening the validator.
- The root log contained 121 dated records. Archived the 101 oldest complete records verbatim, oldest-first, in `docs/dev-log/2026/11/part-018.md`; retained the newest 20 and prepended this milestone (21 root records). Updated the archive index with exact count, byte size and Git blob SHA.
- No product source changed. No local validator was run in the connector-only environment. Exact-head AI documentation validation for THIS COMMIT is pending; L4 physical-router/client-import behavior remains NOT VERIFIED.

---

## 2026-11-19 — Correct OpenVPN export contract formatting check

- The OpenVPN credential workflow at exact SHA e01f70a766c4e628593e272192723f6add59ffca completed its four GNU89/ASan/UBSan policy tests successfully, then failed the new source contract because the TLS-Crypt-v2 key call spans two source lines while the assertion expected a single line. The VPN server lifecycle workflow passed. This is a test-checker formatting mismatch; no source failure was indicated.
- Corrected the contract to check the second-line key argument in order after chmod and before cleanup. Kept all source security checks unchanged.
- The exact-head AI docs, XY-C1, HTTPD, process and NVRAM runs were still in progress at the last query.

---

## 2026-11-19 — Harden OpenVPN exported client credentials

- ovpn_server_expcli_main() previously wrote embedded client private-key material to predictable /tmp/client.ovpn via fopen("w+"), ignored fclose() and chmod(0600) results, and returned success even if certificate/key generation or PEM input reads failed. A pre-existing symlink could be followed and a failed permission change could leave private credentials exposed.
- Switched the output to the existing private staged/atomic OpenVPN config publisher (fsync, checked close, atomic rename, final 0600). Generated keys reside in a unique mkdtemp() 0700 directory. Key input now uses no-follow/nonblocking descriptors, regular-file ownership/link checks, restrictive modes for embedded secret sections, PEM BEGIN/END validation and read/close error checks. Certificate and TLS-Crypt-v2 key generation and temporary cleanup are checked. Removed the post-publish unix2dos/chmod rewrite; OpenVPN accepts LF.
- Expanded scripts/check_openvpn_client_secret.py with exact source contracts. Existing openvpn-client-secret CI runs this checker and the ASan/UBSan atomic config publication regression; VPN server lifecycle and XY-C1 workflows also trigger from vpn_openvpn.c.
- Parent main SHA 4148ac7315d4835b9818c555956d99b195bf4947 had all 13 Actions PASS, including XY-C1 44608598285 and AI docs 44608598296. New exact-head OpenVPN, VPN lifecycle, AI docs and XY-C1 runs remain pending until checked.
- No physical-router or VPN client import result is claimed.

---

## 2026-11-19 — Propagate create_file close failures and satisfy exact handoff contract

- Exact-head DNSMasq lifecycle regression `44606578028`, HTTPD security `44606578001`, WAN DHCP `44606577590`, Process lifecycle inventory `44606578189`, WAN route `44606578101`, Inadyn `44606577502`, Samba `44606577607`, miniupnpd `44606578008` and Process inventory `44606578137` passed on `b1835ef0936517df7a39ef06dc242168251dd4fe`. XY-C1 `44606578070` and NVRAM `44606578018` were still running.
- AI handoff check `44606578020` failed because SESSION_HANDOFF omitted the explicit conditional-document-edit checkpoint statement and the Verification boundary heading. Restore both exact governance requirements; no source failure was reported by that job.
- Continued the resolver-file reliability slice after reviewing shared `create_file()`: it returned success even when `close(fd)` failed. The new injectable policy returns failure with the real close errno (or EIO fallback), and GNU89 ASan/UBSan tests cover open success/failure, close failure, errno preservation and invalid inputs. `common_ex.c` now routes through the tested helper.
- Exact-head DNSMasq, AI docs, and XY-C1 for the next commit are pending; no physical-router L4 result is claimed.

---

## 2026-11-19 — Fail closed when DNSMasq resolver-file creation fails

- Confirmed a concrete RC service-preparation gap in `prepare_dns_dhcpd()`: router mode called `create_file(DNS_RESOLV_CONF)` but discarded its nonzero result, then continued to publish a DNSMasq configuration that references the file and launch the daemon. A permission/filesystem failure could therefore leave resolver forwarding unavailable while the launch path treated the service as started.
- This slice preserves the create errno (falling back to EIO), logs the failure, and aborts before DNSMasq config publication/launch. AP mode is unchanged. Added a GNU89/Werror/ASan/UBSan policy test for success, errno preservation and missing-errno failure, plus exact source-order checks and XY-C1 header path coverage.
- Parent main SHA: `dec2079fcbf9d45731dff1ce03b2765a23bd3a31`; preceding exact-head detect-internet `44603838266`, WAN route `44603838310` and AI docs `44603838305` PASS. New DNSMasq regression, AI docs and XY-C1 Build/TRX/artifact are pending for this source commit until its exact-head runs complete.
- No local firmware/toolchain result or physical-router L4 result is claimed.

---

## 2026-11-19 — Align monitor-pause teardown source contracts

- Exact-source detect-internet Action 44600896372 failed in its inline Python contract at line 134; WAN route Action 44600896744 failed at inline line 253. Both expected obsolete `(void)notify_pause_detect_internet()` calls and advisory-only behavior.
- Source commit a7eae5a115dae74685e9130ff6d339b8202e5725 deliberately changed MAN/WAN teardown to aggregate the monitor pause result into the first error, emit a diagnostic, continue route/interface cleanup, and return the accumulated status. MAN only clears lease state when all cleanup succeeds. The workflow contracts now assert this ordering and status preservation; production code is unchanged.
- Infosvr correction commit 2cdd33f556b925f130d5a05348bb6774fc88dea0 passed exact-head Infosvr packet/signal 44602137230 and AI handoff docs 44602137197. XY-C1, SkipDBv2 and NVRAM passed on product source 2e61e6b; Samba regression, detector, Build XY-C1, TRX and artifact all passed in 44600896324.
- Exact reruns for detect-internet and WAN route are pending. No physical-router L4 result claimed.

---
## 2026-11-19 — Repair the Infosvr checker after launcher-status normalization

- Exact-head Infosvr Action 44600896016 on production SHA 2e61e6b4608d30127692f5d26e939bbce92f52fc passed packet, signal and daemon policy binaries under GNU89/Werror/ASan/UBSan and passed the packet/signal source checker.
- The lifecycle checker failed only at line 132 because its marker tuple still named test_start_preserves_launcher_failure, intentionally replaced by test_start_normalizes_launcher_failure_for_pid1. Updated the expected marker; no product source changes.
- On the same SHA, Process umbrella 44600895951, Process inventory 44600896622, PID 1 notification queue 44600896480 and AI docs 44600896036 passed. XY-C1 44600896324, SkipDBv2 44600896667 and NVRAM audit 44600896559 were still running.
- Also tracked two separate checker failures: detect_internet 44600896372 ended in Python ValueError at line 134; WAN route 44600896744 ended in Python ValueError at inline line 253. Preserve the DHCP/monitor cleanup assertions when correcting them.
- No physical-router L4 result claimed.

---
## 2026-11-19 — Normalize infosvr launch wait status for PID 1 notifications

- PID 1's `handle_notifications()` retains an incomplete notification marker only when `action_result < 0`. `start_infosvr()` returned a positive `eval("/usr/sbin/infosvr", IFNAME_BR)` wait status unchanged, and `infosvr_start_postcondition()` preserved it; a failed detached child could therefore be acknowledged as success and its restart marker cleared.
- The launch error path now preserves the original status in its diagnostic log, maps positive encoded wait statuses to `errno=EIO`, and returns through `infosvr_start_postcondition()`. The helper maps every nonzero launch status to `-1`; disabled optional-service success `0` and the guard-before-inventory order remain unchanged.
- Regression tests cover positive wait statuses 7/256, negative launcher failure, and RUNNING/ABSENT/UNKNOWN postconditions. Dedicated Infosvr and broader Process source checkers now enforce the negative-only PID 1 failure convention. Existing workflows compile the GNU89/Werror/ASan/UBSan policy test.
- Parent `5c43773c08d50540c481bc8e0ca4b6204c63a7f5` inherited exact-head Infosvr `44598421247`, Process umbrella `44598421270`, Crond `44598421431`, Process inventory `44598421241`, and NVRAM `44598421188` PASS on `9e51fc0...`; XY-C1 `44598421480` was still IN_PROGRESS. Samba L2 `44597410202` PASS on `0e137...`; HTTPD security/AI docs `44598733635`/`44598733483` PASS after descriptor-check correction. New exact-head L2/L3 pending. No physical-router L4 result claimed.

---

## 2026-11-19 — Secure every Samba transaction entry point and enforce private 0700 state

- Review after `536f6f0e` found two additional ways into the same state journal/passdb: `run_samba()` and public `write_smb_conf()` in `services_stor.c`. The former still created `/etc/samba` with mode 0777 before a path-based chmod; the latter recovered the journal without securing its parent. Guarding only `start_wins()` was incomplete.
- The parent-directory helper now enforces exact effective-UID-owned 0700 (not merely no group/other write), using an opened `O_DIRECTORY|O_NOFOLLOW` fd and post-`fchmod` verification. All three transaction entry points — `start_wins()`, `run_samba()`, and `write_smb_conf()` — prepare the parent before recovery. Removed the redundant path-based `samba_secure_private_samba_dir()` check and the remaining permissive `mkdir_if_none(..., "777")`.
- The first commit's XY-C1 run `44593848957` failed only because its regression compile omitted `-I trunk/user/rc/src`; workflow-only commit `287aa1cb8105de24dc6683485087ccdbcb12f062` corrected it. Its exact XY-C1 run `44595726202` passed source/input detection; Build/TRX/artifact were skipped because the SHA contained workflow-only changes. Initial dedicated Samba run `44593849014` passed. New all-entrypoint checks and firmware build for this production-source change are pending.
- No local build or physical-router result is claimed.

---

## 2026-11-19 — Harden Samba passdb and recovery parent-directory permissions

- Source review found `start_wins()` used `mkdir_if_none("/etc/samba", "777")`. PID 1 runs with a permissive umask, so a missing directory could become world-writable. The helper also accepts an existing path without validating its type, owner or mode. This directory parents `smbpasswd` and the root-only `.config-transaction` journal; group/other write access lets local users replace or remove passdb/recovery entries despite the child transaction directory's 0700 mode.
- Replaced the blind mkdir with descriptor-based parent preparation: create as 0755 even under umask 0000; open with `O_DIRECTORY|O_NOFOLLOW`; require effective-UID ownership; remove group/other write bits while preserving safe read/execute access; revalidate the opened directory after permission repair. Symlinks, non-directories and foreign-owned paths fail closed before WINS recovery/publication.
- Added GNU89 ASan/UBSan regression for permissive umask, legacy 0777 repair, preservation of restrictive 0711 mode, and file/symlink rejection. XY-C1 workflow watches the header/test and runs this regression plus a production caller-order contract. Exact-head regression and XY-C1 Build/TRX/artifact are pending; no local test or device result is claimed.

---

## 2026-11-19 — Correct the UNKNOWN guest rollback source contract

- Exact Wireless radio run `44592415803` on production source `ef421978331ade679241c5bc1c10dbb6350d4004` compiled and passed the radio-state policy, bridge-membership policy, and guest-isolation policy binary with GNU89/Werror/ASan/UBSan. The Python checker then failed at line 161 because it required the old early-return-on-UNKNOWN pattern in both rollback callbacks.
- The new fail-closed down callback intentionally requests interface-down after an UNKNOWN flags probe, then relies on a separate probe in `wireless_guest_rollback_verify_down()`; only that verifier should keep the old UNKNOWN early-return assertion. Updated the source checker to distinguish the two contracts. Exact-head rerun pending.
- On the same source SHA, AI docs `44592415781`, watchdog `44592415729`, iNIC signal `44592415628`, and Process inventory `44592415914` passed; Process, NVRAM, HTTPD and XY-C1 remained in progress at the last poll.
- This commit changes only checker and handoff/roadmap/log documentation; no production C. No L4 router verification is claimed.

---

## 2026-11-19 — Fail closed when guest ebtables isolation setup is incomplete

- Source review found that `restart_guest_lan_isolation()` propagated module, flush and rule-add failures but left pre-existing guest AP interfaces up and bridged. The guest control callers only rolled back interfaces started by the current request, so toggling/rebuilding isolation could leave a configured guest AP connected without its required LAN isolation.
- AP-mode ebtables policy bits are now retained even when the interface-flags probe is UNKNOWN; the code still records the probe error rather than treating the interface as up/down. A failure-count checkpoint isolates errors from module confirmation, `ebtables -F/-X`, and individual rule additions. Any new setup error now triggers down, bridge detach and independent DOWN verification for every configured guest AP using ebtables.
- Guest rollback no longer treats an UNKNOWN initial flags probe as a reason to skip the down request: it attempts down and lets the independent postcondition probe determine completion. RT3352 MII iNIC interface/VLAN mapping is preserved.
- Added GNU89 policy cases for UNKNOWN-triggered rollback requests and ebtables setup failure/mask semantics; extended the existing Wireless production-source contract. Exact-head Wireless L2, AI docs and XY-C1 Build/TRX/artifact for this product-source change are pending.
- The parent `981857e60def0d4237253a3c0b061b85111ebbf5` WAN route and AI handoff Actions `44586115401` / `44586115612` passed; prior product SHA `29fa...` XY-C1 run `44585450325` passed detector, Build XY-C1, TRX and artifact, but does not validate this new wireless source. No local compile or physical-router L4 is claimed.

---

## 2026-11-19 — Fix WAN route source-contract whitespace matching

- Exact-head WAN route run `44585450279` on production source `29fa24741613095593a03f4e602f1d77636a21e0` compiled and ran `route_status_policy.c` with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan successfully. The subsequent Python contract failed at line 275 because its joined string retained a space after the multiline helper's opening parenthesis while the expected token did not.
- Updated both WAN-up and WAN-down helper-call checks to remove all source whitespace before matching. This preserves the semantic assertion while allowing normal multiline formatting. Exact-head rerun pending.
- XY-C1 run `44585450325` was still in progress at checkpoint; all other listed 29fa stability and security Actions completed PASS. No product build result is inferred from the route-policy host test.

---
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

