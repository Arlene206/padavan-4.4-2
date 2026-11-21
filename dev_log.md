## 2026-11-21 — Propagate iNIC MII health-recovery failures to periodic monitor

- Parent main SHA: `6dd282fc7112b86ccdb167100e44765fbca97b73`.
- Source review found `check_inic_mii_rebooted()` silently ignored the `doSystem()` result when forcing RT3352 iNIC radio off for a disabled radio. Guest AP recovery errors were only logged; the void callback gave `inicd_main()` no status to report.
- Changed the callback to return int, normalize both command failure paths through the existing tested `wireless_radio_restart_status()`, and have the daemon log an incomplete health poll while continuing to the next timer tick. Updated both iNIC and Wireless source contracts to require returned status and retry semantics. No timer/stop signal behavior changed.
- Existing Wireless C policy regression already exercises positive and negative `system()` wait status normalization/sticky first error; existing iNIC signal GNU89 ASan/UBSan test remains wired.
- No local checkout/runtime available; no local C/Python test, sanitizer or firmware build is claimed. Exact-head iNIC and Wireless Actions are the verification gate.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Align IPTV static-copy bounds contract with shared helper

- Parent main SHA: `8a2f50bedb44fdff0cb8b322372090fc190d978c`.
- Exact-head WAN DHCP callback `#44896283833` passed all earlier policy/renewal and callback checks, then failed because the static IPTV startup checker still required three hand-written `strlen() >= sizeof()` checks. Production now uses `rc_wan_nvram_copy_value()` for IP address, netmask and gateway; that shared helper validates destination size and returns `ENAMETOOLONG` rather than truncating. Updated the embedded contract to require the three checked bounded-copy calls with `sizeof(destination)`, their shared error-return gate, and absence of obsolete manual checks.
- AI docs `#44896283589` and WAN route `#44896283559` passed on the exact parent SHA. This fix is checker + handoff only; no product source changed.
- No local tests/build claimed. Exact-head WAN DHCP and docs Actions on the resulting SHA are required.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Align IPTV renewal and MTU checks with checked NVRAM status

- Parent main SHA: `728468e145d625164bf2a6aaba6883b183cb20c2`.
- Exact-head WAN DHCP callback `#44896094291` passed the C policy regressions and the physical-WAN renewal contract, then failed at the remaining IPTV renewal assertions, which still expected the removed unchecked retry helper and direct NVRAM getters. Updated them to require checked `err`/`route_err`/`mtu_err` reads, decision/error/retry ordering, checked `manv_mtu`, and no legacy getters.
- Exact-head WAN route status `#44896094274` passed the revised VLAN checker, then failed in `scripts/check_wan_initial_mtu_status.py` because its physical and IPTV renewal checks still looked for the removed retry helper. Migrated both contracts to the checked status decision and fail-closed ordering, retaining the post-retry MTU-triggered rebind checks.
- Exact-head AI handoff docs `#44896094292` requires the precise phrase `one atomic main commit` and standard checkpoint section headings. Rebuilt the handoff with the protocol's required sections and first actionable task.
- No product source changed. No local test/build claimed; validate this change only through exact-head Actions.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Align WAN DHCP and IPTV cache-copy source contracts

- Parent main SHA: `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6`.
- Exact-head HTTPD `#44895865685` and WAN DHCP callback `#44895865642` still asserted the pre-migration direct helper and unchecked `nvram_get_int()` status reads. Updated the source contracts to require all three checked status reads, first-error fail-closed ordering, checked stored-MTU reads, and rebind only after the decision helper succeeds. The HTTPD route-option assertion now checks each of the three supported route variables is validated before comparison rather than requiring an obsolete duplicate count.
- Exact-head WAN route `#44895865688` exposed stale `launch_viptv_wan()` assertions for `nvram_get_int()` and borrowed static setting pointers. Updated the contract to require checked `viptv_mode` reads, fail-closed error gates, copied address/netmask/gateway values, validation before local copy, and completion before VLAN teardown.
- Exact-head AI handoff `#44895865686` exposed a missing handoff checkpoint rule; restored the mandatory state-accuracy check and explicit no-op-commit prohibition.
- No production source changed in this commit. Remote exact-head CI on the new SHA is the verification gate; no local test/build is claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Preserve IPTV VLAN settings across NVRAM cache reuse

- Parent main SHA: `87807147e186061a8450680de567edc13311826a`.
- `launch_viptv_wan()` retained pointers returned by `get_wan_unit_value()` / `nvram_safe_get()` across later reads. The shared `nvr_cache` buffer was overwritten between the old WAN VLAN and IPTV interface names, and between static IPTV address and netmask; the old code could remove the wrong VLAN or use the netmask as the static address. The WAN MAC pointer was also retained through VLAN setup.
- Replaced these pointers with checked, bounded copies into local arrays. VLAN filtering, VID, priority and IPTV address mode now use checked reads and fail before any VLAN removal/creation if configuration cannot be read. Static address/netmask/gateway and WAN MAC are separately copied; missing gateway retains the previous empty/default behavior.
- Added a fake backend with a single mutable cache slot and a regression proving the old/new VLAN names, address, mask, gateway and WAN MAC remain distinct after six sequential reads. Extended the WAN route source contract to require checked configuration reads and reject the legacy pointer pattern.

---

## 2026-11-21 — Update HTTPD DHCP renew contract for checked-state retry flow

- Parent main SHA: c14651353b1096db701f560729c0052d0275f560.
- HTTPD security run #44895295713 on 8700539 failed in its embedded Python source contract after the WAN renewal migration. The contract still required dhcp_wan_bound_retry_required() and three nvram_get_int() reads in both the physical WAN and IPTV renew callbacks.
- Updated the workflow contract to require all three get_nvram_int_checked_status() reads, dhcp_wan_bound_retry_decide(), checked stored-MTU reads, and error-return ordering before retry/bind side effects. It still rejects the old error-collapsing getters and preserves the surrounding DNS/resolver/route assertions.
- This is a checker-only change; exact-head HTTPD, WAN DHCP and WAN route Actions on the new SHA are required. No local workflow script execution is claimed.

---
## 2026-11-21 — Tighten RAS credential snapshot source-contract ordering

- Parent main SHA: 72b789bc6c027d1bb9d36fdbc9473a00de9bb13f.
- Firewall startup run #44895295888 on ancestor 8700539 exposed the only failure in scripts/check_modem_wan_start_status.py: the checker searched a whitespace-compacted function but included an extra space in the expected modem-type parser token. Production write_pppd_ras_conf() already snapshots modem_type/user/pass/isp under one transaction and returns on snapshot/strict-parse errors before opening the peer file.
- Corrected the compact token and made the contract enforce snapshot -> strict integer parse -> peer-file open, with both snapshot_status and modem_type_status error branches before file creation. No production code or checks were removed.
- Exact 72b789 main had just retriggered Firewall startup run #44895636746, AI handoff run #44895636818, USB identity #44895636794, Process inventory/lifecycle #44895636775 and XY-C1 #44895636822; those were still in progress when queried. This checker change requires fresh exact-head runs.

---
## 2026-11-21 — Correct RAS modem snapshot source-contract whitespace

- Parent main SHA: c6c1d7a78213f9525560a5fe8602a6ec07957357.
- Firewall startup run #44895295888 on ancestor 8700539 passed the native UPnP/firewall policy binary then failed at scripts/check_modem_wan_start_status.py:162. Production write_pppd_ras_conf() correctly snapshots modem type/user/password/ISP transactionally, parses the copied modem-type string, and refuses to open the peer file on snapshot or parse error. The checker compared a whitespace-compacted C function against an expected token that still contained a space after the comma.
- Corrected the token and strengthened ordering assertions: snapshot -> strict modem-type parse -> PPP peer open, with both snapshot and parse failure gates before file creation. No production code or coverage is weakened.
- Exact-head c6c1d7a actions were in progress at this checkpoint; the new commit must rerun Firewall startup and related suites. HTTPD security has a separate stale DHCP-renew source assertion queued for repair.

---
## 2026-11-21 — Repair shared-cache NVRAM test fixture for interface names

- Parent main SHA: `8700539128d7865df7d4e706fe710603e772c2c9`; exact-head commit `8700539128d7865df7d4e706fe710603e772c2c9`.
- WAN DHCP callback run `#44895295803`, WAN route run `#44895295721`, and USB modem identity run `#44895295781` all reached the GNU89/ASan/UBSan `wan_nvram_status_policy.c` binary and failed in its `fake_nvram_get()` fixture at line 660. The new cache-overwrite test requested `wan0_man_ifname` and `wan0_ifname_t`, but the fake reader still hard-coded the older address/mask key sequence.
- Extended the fixture with optional per-call expected names (retaining the original defaults for existing tests) and initialized the two interface-name keys in the new regression. This restores the intended shared-cache overwrite coverage; it does not alter production code or weaken assertions.
- The same exact-head Actions also exposed independent stale checker failures: Firewall startup `#44895295888` expects an obsolete direct modem-type parse, while HTTPD security `#44895295713` fails a DHCP callback ordering assertion. These are the next diagnosis targets. WAN route and DHCP tests did not reach the new checked-renew tests because the fixture aborted first.

---

## 2026-11-21 — Fail closed on unreadable DHCP renewal status and MTU

- Parent main SHA: `a68bac734fd3f232fb16f095bed9bba83813da24`.
- Both physical-WAN `udhcpc_renew()` and IPTV `udhcpc_viptv_renew()` read prior lease, route and MTU error flags with `nvram_get_int()`. A temporary NVRAM backend error collapsed to zero, so a renew path could classify an incomplete lease transition as healthy and skip the full bound/rebind repair.
- Replaced those reads with `get_nvram_int_checked_status()`, and added `dhcp_wan_bound_retry_decide()` to preserve the first read error and refuse a retry decision from a partial tuple. A read failure is logged and returned before lease comparisons or publication; absent keys retain the default zero.
- Physical and IPTV renewers now also use checked getters for stored MTU comparisons; backend or malformed-value errors abort renewal rather than being treated as zero.
- Added GNU89 policy regression cases for each status-read failure, first-error ordering, complete tuple evaluation and independent retry reasons. Extended the WAN DHCP callback source checker to require checked reads and fail-closed ordering. Exact-head WAN DHCP, WAN route and handoff Actions are the validation gate; no local tests were run because this session has no checkout/runtime.

---

## 2026-11-21 — Preserve WAN interface names across cache-backed NVRAM reads

- Parent main SHA: `f268eab8f685b67a59736ce716a9bf88e031f69c`.
- Inspection of `nvram_linux.c` confirmed all `nvram_get_checked()` / `nvram_safe_get()` values point into a single static `nvr_cache` buffer reused on the next NVRAM read. `get_wan_ifname()` and `get_wan_ip4()` fetched the management interface and `wan0_ifname_t` into pointers, then called `get_usb_modem_wan()`, checked modem-type reads or WAN-protocol lookups before consuming them. The pointers therefore no longer represented their original interface names.
- Both paths now copy the management interface immediately and fetch/copy the WAN temporary interface with a checked, bounded helper before any further NVRAM access. Backend errors / oversized values propagate from `get_wan_ifname()`; `get_wan_ip4()` returns `INADDR_ANY` instead of querying an unverified interface.
- Added a regression proving both management and temporary interface strings remain distinct after the fake NVRAM backend overwrites its shared cache. Extended the WAN route static checker to require both safe-copy sequences.

---

## 2026-11-21 — Snapshot USB modem RAS credentials before shared NVRAM cache reuse

- Parent main SHA: `d8ff11c5c24230e88a7adc9464bb89d34eeb5036`.
- Inspection of `trunk/user/shared/src/nvram_linux.c` confirmed `nvram_get_checked_unlocked()` returns a pointer into one static `nvr_cache` buffer. Each subsequent NVRAM get overwrites that buffer. The RAS peer writer fetched `modem_user`, `modem_pass` and `modem_isp` into pointers before using them, so the earlier values could all resolve to the last setting read (or change under concurrent configuration updates).
- Replaced the independent reads with one bounded `nvram_transaction` snapshot of `modem_type`, username, password and ISP. The existing snapshot helper copies each cache-backed value before the next read, preserves a coherent tuple and invalidates it if any read or transaction close fails. No peer file is opened until the whole snapshot and strict modem-type parse succeed.
- Added a GNU89 policy regression using the fake backend's single mutable cache slot to prove all four RAS settings survive subsequent reads. Extended USB modem static checks; workflow already compiles/runs this policy test with `-Werror`, ASan and UBSan.
- This change prevents stale/misbound RAS credentials and carrier configuration; it does not alter the NVRAM keys or missing-key defaults.

---

## 2026-11-21 — Make RAS modem-type checker whitespace independent

- Parent main SHA: `912fbff13b652c09563159f3357a21732db35f4b`.
- Exact-head USB modem run `#44894556779` and firewall startup run `#44894556782` both reached `scripts/check_modem_wan_start_status.py` and failed at the new `modem_type_parse` assertion. Production code has the intended checked read before peer-file open; the checker first normalized whitespace to single spaces, then expected a call with no space after the opening parenthesis.
- Changed the source contract to compare a whitespace-free compacted function body, including the checked getter, strict parser arguments, saved errno/early return and read-before-open ordering. This is a test-only correction; the WAN NVRAM policy GNU89/ASan/UBSan regression had already passed in the USB workflow before the static step.
- Firewall run's UPnP/firewall restart result policy also passed before this shared checker failed. Rerun USB modem and Firewall workflows on the new exact SHA.

---

## 2026-11-21 — Fail closed on unreadable USB modem type while writing RAS peer config

- Parent main SHA: `b107e60a4c30b838ab963d3dc8ccdabeb4fe62e8`.
- `write_pppd_ras_conf()` still performed a second, unchecked `nvram_get_int("modem_type")` after opening/truncating the RAS peer file. If the NVRAM backend failed at this point, the error collapsed to legacy value zero and could select `Generic_conn.scr`; a partially rewritten peer file could then be published using a guessed transport profile.
- Moved the modem-type lookup ahead of peer-file creation, routed it through `nvram_get_checked()` plus the existing strict `rc_wan_nvram_read_int_status()` parser, and aborts with the preserved status on backend, malformed-value or range errors. Missing-key behavior remains the legacy default zero.
- Extended the USB-modem source contract to require the checked read and reject the unchecked getter. The USB-modem workflow now includes the existing WAN NVRAM policy regression, compiles it as GNU89 with `-Werror`/ASan/UBSan, and runs it on changes to the checked-reader header/tests.

---

## 2026-11-21 — Align deferred reconnect checker with watchdog rollback contract

- Parent main SHA: `2c0f9ce2b316798109289cdabcdcb20c1d22c21d`.
- Exact-head WAN route run `#44894284811` passed the earlier network sysctl and initial physical-MTU checks plus the policy binaries, then stopped at `Verify deferred WAN reconnect watchdog launch status`. Its checker required the literal `return launch_status;`, but production intentionally normalizes launcher failure and invokes `watchdog_rollback_failed_start(launch_status, launch_errno, "launcher")`; readiness timeout also runs through the same rollback path. This was a stale source expectation, not a missing production status return.
- Updated the contract to require explicit unknown-inventory handling and the rollback helper on both launcher failure and readiness timeout. The checked deferred deadline and watchdog call ordering remains required.
- Exact-head AI documentation run `#44894284801` PASS. New WAN/AI push Actions are the validation gate. No production source changed; current-source L3 and hardware L4 are not claimed.

---

## 2026-11-21 — Correct MTU checker position space after checked-read migration

- Parent main SHA: `751cb64ff459a1d5afebc836996dd43f810172f6`.
- Exact-head WAN route run `#44894158533` passed the new network-sysctl assertion and all earlier route policy binaries/contracts, but the MTU source checker failed at its checked getter lookup. The checker mixed an offset taken from the original C function text with a lookup in whitespace-compacted text; those indices are not interchangeable, so it failed to find a real, correctly ordered `get_wan_unit_value_int_status()` call.
- Reworked the initial-MTU contract to use one source coordinate space throughout. It now checks the checked per-unit MTU read, error aggregation/logging and `continue` before physical default MTU application, then verifies the existing default-write error handling still precedes `ifconfig(IFUP)`. Production source is unchanged.
- On this parent, WAN teardown run `#44894158502` PASS and AI handoff docs run `#44894158457` PASS. The new checker commit needs a fresh exact-head WAN/AI run. Current firmware L3 and hardware L4 are not claimed.

---

## 2026-11-21 — Align WAN MTU and teardown checks with checked NVRAM reads

- Parent main SHA: `2811673b34fdf7629dc5bb04d1e488a2a56d4a23`.
- WAN route run `#44893956383` reached `scripts/check_wan_initial_mtu_status.py` after the WAN sysctl and earlier route contracts passed, then failed because this older checker still expected the removed unchecked `get_wan_unit_value_int()` MTU read. Updated it to require `get_wan_unit_value_int_status()`, verify the per-unit error/continue path occurs before default MTU setup, and keep the default-write error aggregation ordering. Its `start_wan()` extraction now handles mutually exclusive preprocessor brace branches with a regression fixture.
- The WAN teardown contract previously expected the removed direct `nvram_get_int("modem_type")` branch. It now requires checked transport selection before monitor pause and the NDIS decision before adding the USBnet interface to teardown.
- No product behavior changed in this commit; this aligns source-contract checks with the checked modem-type and MTU changes already present.

---

## 2026-11-21 — Align network sysctl checker with checked SYN-cookie gate

- Parent main SHA: `7af88e6e0cf704717511efd808a758611d2fca2b`.
- WAN route run `#44893645481` on its exact parent exposed a stale integration assertion: production `start_wan()` had moved `fw_syn_cook` from `nvram_get_int()` to `get_nvram_int_checked_status()`, but `scripts/check_network_sysctl_status.py` still required the old getter. The same checker assertion remained at step 29 of WAN run `#44893837491` on this descendant while that workflow was in progress.
- Updated the contract to require the checked read, preserve the separate `tcp_syncookies_status` write result, and prove the NVRAM error return precedes sysctl application. Production behavior and workflow coverage are unchanged.
- AI handoff run `#44893837451` and XY-C1 run `#44893837554` were in progress on the parent SHA when inspected. New exact-head runs are required; no current-source L3 or hardware L4 claim is made.

---

## 2026-11-21 — Fail closed on unreadable USB modem transport during stop

- Parent main SHA: `19b5cdee226daa53db575fb82b02dd8b5c3c4077`.
- Source review found three remaining unchecked `nvram_get_int("modem_type")` decisions in WAN teardown/interface selection. A backend error was collapsed to the legacy zero/RAS value: manual disconnect could select the PPP-only stop path for an NDIS modem, the full stop could omit the USBnet interface from ifdown, and `get_wan_ifname()` could report the wrong transport interface.
- Added one checked transport-selection helper and a GNU89 policy regression for NDIS, legacy RAS default, invalid output and backend-error preservation. Full WAN stop now reads the type before pausing/stopping services; manual disconnect and interface lookup propagate the read failure instead of choosing a guessed path.
- The WAN route workflow already compiles the checked NVRAM policy regression and runs the expanded source contract. Firmware Build/TRX/artifact verification remains a separate exact-SHA requirement.

---

## 2026-11-21 — Normalize SYN-cookie and modem-type checked-read contracts

- Parent main SHA: `35967d072836add527cbfec20d1865088b5cd8c3`.
- Exact-head WAN route run `#44893536012` passed DNS, PPPoE, MAN/physical MTU, EAPOL/Kabinet and MAN DNS-route contracts, then found the same line-break mismatch in the checked `fw_syn_cook` call. Source inspection also showed the adjacent `modem_type` call and rollback call are split over lines, so their source assertions were normalized in the same pass.
- The checker now matches compacted source for the exact security-setting getter and verifies its fail-closed return precedes `set_tcp_syncookies()`; modem transport failure must still restore the WAN interface and continue before choosing NDIS/RAS.
- AI handoff documentation run `#44893536072` passed on the same parent SHA. No product C behavior changed.

---

## 2026-11-21 — Normalize MAN DNS-route source contracts

- Parent main SHA: `590ae64484500b91695076220961e3baabc929bc`.
- Exact-head WAN route run `#44893438430` passed the GNU89/ASan/UBSan policy binary, DNS, PPPoE, WAN MTU and EAPOL/Kabinet checks, then failed at the MAN route-creation assertion. The checked `dnsenable_x` call is split over lines; the test flattened line breaks but still required the exact unflattened call string.
- Compacted both MAN route setup and teardown slices before matching their checked getters and fail-closed/best-effort cleanup branches. No source behavior changed.
- AI handoff documentation run `#44893438413` passed on the same parent SHA.

---

## 2026-11-21 — Normalize EAPOL checked-read contract

- Parent main SHA: `ef17608b90343776a20ec2e7591b885b8be54359`.
- Exact-head WAN route run `#44893354019` passed the GNU89/ASan/UBSan policy binary, DNS-mode and PPPoE checks, and both MAN/physical MTU checks. It then failed the EAPOL checked-read source assertion because `start_wan()` splits the getter call over multiple lines.
- Updated the checker to compact the extracted function body and compare the exact checked getter, key and output variable without formatting sensitivity. The existing `auth_status` cleanup-path requirements remain.
- AI handoff documentation run `#44893354037` passed on the same parent SHA. No product C behavior changed.

---

## 2026-11-21 — Normalize physical WAN MTU checked-read contract

- Parent main SHA: `35f7f642c0115cbb238190256d498ad75cbb4fc1`.
- Exact-head WAN route run `#44893239999` passed the GNU89/ASan/UBSan policy binary, DNS-mode checks and PPPoE read/rollback contracts, and the MAN MTU contract after its whitespace normalization. It then exposed the same newline-sensitive source assertion for the physical WAN MTU getter.
- The production `start_wan()` call is split across lines. Updated the contract to compact the extracted C body before matching the checked getter, finding the NVRAM failure guard and verifying the unit is skipped before `ifconfig(...IFUP...)`.
- AI handoff documentation run `#44893240012` passed on the same parent SHA. No production C behavior changed.

---

## 2026-11-21 — Normalize multiline MAN MTU checked-read contract

- Parent main SHA: `703ff6abcffa17313482ac2ca772fc0c79afb826`.
- Exact-head WAN route run `#44893094794` passed the GNU89/ASan/UBSan policy binaries and the DNS-mode + PPPoE read contracts, then stopped at the MAN MTU static assertion. The implementation uses `get_wan_unit_value_int_status()` and returns before the MAN address snapshot on read error; its call is split across lines, but the checker first flattened whitespace and then compared against a string with no space after the function name.
- Changed the assertion to compare a whitespace-compacted call expression, preserving the exact getter, `unit`, `man_mtu` key and output pointer.
- AI handoff documentation run `#44893094799` passed on the same parent SHA. No product C behavior changed.

---

## 2026-11-21 — Normalize PPPoE rollback call arguments in source checker

- Parent main SHA: `7f5221f1fc861e1b9191ddec2ba5aca106741bc8`.
- Exact-head WAN route run `#44892863922` reached the idle-time rollback assertion but rejected the source call because the regex was emitted with doubled backslashes. The C branch uses the expected restore helper and arguments; this checker-only issue does not imply missing rollback.
- Replaced the regex with a whitespace-insensitive extraction of the call expression, then compare its compact form to the exact helper, unit and interface arguments and require the subsequent `continue;`.
- WAN DHCP and AI handoff checks passed on the same parent SHA.

---

## 2026-11-21 — Correct WAN rollback regex escaping

- Parent main SHA: `49cde0cd18521a94466b3bac2017d3edab346e88`.
- Exact-head WAN route run `#44892863922` passed the GNU89/ASan/UBSan policy binaries and the preceding WAN source contract, then failed the new PPPoE rollback regex. The source contains the expected multiline `restore_wan_after_firewall_failure(unit, wan_ifname)` call; the checker expression again contained doubled backslashes and matched literal backslashes instead of whitespace.
- Corrected the regex to use one backslash per whitespace escape. The assertion still pins both the restore helper and the exact `unit` / `wan_ifname` arguments.
- WAN DHCP callback run `#44892863908` and AI handoff docs `#44892863965` both passed on the same parent SHA.

---

## 2026-11-21 — Scope WAN failure-path assertions to their own branches

- Parent main SHA: `b1a6dc2eef817674065df02bcb08c8bce95ca8ac`.
- Exact-head WAN route run `#44892718181` reached the PPPoE idle-time rollback assertion. Source inspection showed the checked-read failure branch does restore WAN and continue; the call is split after the opening parenthesis, while the checker required a single exact spacing form. Changed it to a whitespace-tolerant regex that still verifies the unit and interface arguments.
- Exact-head WAN DHCP run `#44892718115` reached physical-interface cleanup ordering. The MTU-read error branch contains an earlier `continue;`, so searching the whole physical-setup slice falsely treated it as the interface-failure branch's continue. Scoped cleanup ordering to the `if (interface_status != 0)` block.
- Both are static-contract corrections; no production code or failure guards were weakened.

---

## 2026-11-21 — Scope DNSMasq resolver-lock assertion to its failure branch

- Parent main SHA: `68e10bad396c35c000a17cd26c6763573b13413e`.
- Re-examined DNSMasq lifecycle run `#44891688020` on c930b7e. The C implementation correctly returns on resolver-lock acquisition failure without unlocking an invalid descriptor. The workflow sliced from that branch through the later staging-file open, inadvertently including a separate, valid `file_unlock(lock)` when WAN DNS-mode lookup fails after lock acquisition.
- Changed the regression to extract only the `if (lock < 0)` block before asserting that it never unlocks. This preserves the lock-failure guarantee while permitting cleanup on subsequent post-acquisition failures.
- No production source changed. The DNSMasq workflow will rerun on this workflow-file update.

---

## 2026-11-21 — Require an actual column-zero C function terminator

- Parent main SHA: `c3fe4cfceb16f9fb140316ad789a45bebe2a60cb`.
- The first preprocessor-aware extractor revision still accepted any whitespace-only line prefix as a function terminator; its own regression fixture exposed that this stopped at an indented inner block close rather than the function close. The same predicate existed in the WAN DHCP workflow helper.
- Tightened both extractors to accept only a literal column-zero closing brace (`line_start == brace_position`), matching the repository's C function layout. The conditional-branch fixtures assert statements after the inner brace remain in the extracted body.
- No production code changed. Both WAN route and WAN DHCP workflows will rerun because their checker files are part of the updated commit.

---

## 2026-11-21 — Use preprocessor-aware extraction in WAN DHCP regression

- Parent main SHA: `a4a60944890edae9dc6821eec906d3c4c6d881b0`.
- Exact-head WAN DHCP callback run `#44892451936` passed the updated checked-MTU read assertions, then failed at the ordering assertion between default MTU setup and physical interface activation. The source order is correct, but the existing C brace counter treated mutually exclusive `#if/#else` branches in `start_wan()` as simultaneously active and terminated the extracted body at the wrong closing brace.
- Added a focused extractor that finds this repository's column-zero function terminator while continuing to skip comments and literals, with an alternate-preprocessor-branch regression fixture. The two WAN DHCP workflow extractions of `start_wan()` now use it; other function-body extraction remains unchanged.
- No production C behavior changed. Rerun the WAN DHCP exact-SHA Action after this checker correction.

---

## 2026-11-21 — Handle conditional-compilation braces in WAN startup checker

- Parent main SHA: `fee0fe11f4a7d12da0238e3f548fcc7b57b93215`.
- Exact-head WAN route run `#44892366354` passed the policy binaries and initial DNS-mode checks, then failed the PPPoE idle-time failure-path assertion. The production function contains the checked read, WAN restoration and `continue`; the generic brace counter counted both mutually exclusive `#if/#else` branches as simultaneous C blocks and truncated the extracted function body.
- Added a regression fixture with alternate preprocessor branches and a function extractor that uses the repository's column-zero C function terminator while still ignoring comments/literals. All `start_wan()` source-contract slices now use this preprocessor-aware boundary; the existing generic extractor remains unchanged for other functions.
- This repairs a false-negative static contract, not WAN runtime behavior. Rerun WAN route Actions on the resulting exact SHA.

---

## 2026-11-21 — Align WAN DHCP MTU regression with checked NVRAM read

- Parent main SHA: `b816880f652adcec58168be3911d38dec75a22b1`.
- Reviewed the earlier WAN DHCP callback run `#44891687966` (on c930b7e). Its inline checker still expected `wan_mtu = get_wan_unit_value_int(unit, "mtu")`, while current `start_wan()` reads MTU through `get_wan_unit_value_int_status()` and aborts the unit before default-MTU/interface setup on read failure.
- Updated the existing physical-WAN setup contract to require the checked getter, verify the `nvram_status != 0` guard and `continue` precede the default-MTU branch, and reject the legacy error-collapsing getter.
- This is a stale test expectation, not a new production change. The current exact SHA's WAN DHCP workflow will verify the updated contract.

---

## 2026-11-21 — Correct PPPoE whitespace regex escape in WAN checker

- Parent main SHA: `d2e41ab61db5607c7d075b65df3de9ae081cc83a`.
- Exact-head WAN route run `#44892123843` passed the GNU89/ASan/UBSan policy binaries and the preceding WAN DNS-mode contract, then failed the PPPoE demand-publication assertion.
- The C source confirms the demand setter is present immediately after the checked idle-time read; inspection found the raw Python regex contained doubled backslashes (`\\s`) and therefore searched for literal backslash characters instead of whitespace.
- Corrected the raw regex to use single regex escapes (`\s`), retaining the requirement that demand publication follows the checked read.
- XY-C1 run `#44892123837` and process lifecycle run `#44892123811` both completed SUCCESS on this parent SHA; XY-C1 Build/TRX/artifact details must be checked before assigning L3.

---

## 2026-11-21 — Align WAN regression contracts with checked EAPOL and PPPoE reads

- Parent main SHA: `cb21893b8651dc975646c9ac0bb08cf923a29030`.
- Exact-head WAN route run `#44891999134` passed its GNU89/ASan/UBSan policy binaries and the DNS-mode contract, then exposed a second formatting-sensitive check: idle-time publication used a multi-line call and was searched as one literal.
- The checked PPPoE demand-publication assertion now tolerates C whitespace while still requiring a call after the checked idle-time read.
- The XY-C1 and process integration workflows also expected the pre-migration direct EAPOL assignment. Updated them to require the current sequence: checked auth-mode read initializes `auth_status`, conditional EAPOL launch overwrites it, and the error branch consumes it. No production behavior or guard was weakened.
- Re-run the WAN route, XY-C1, and process checks on the resulting exact SHA; firmware L3 must only be recorded after build, TRX and artifact upload all pass on one SHA.

---

## 2026-11-21 — Restore regex import in checked WAN source contract

- Parent main SHA: `839922fedd844174ca6553c50901c5a9c942cc44`.
- Exact-head WAN route run `#44891897645` passed the GNU89/ASan/UBSan policy binaries and DNS-mode static contract, then stopped with `NameError: name 're' is not defined` after the PPPoE checked-read assertions were converted to whitespace-tolerant regex matching.
- Added the missing standard-library `re` import. No product C source changed in this checker-only correction.
- The WAN route workflow must pass on the resulting exact SHA; firmware L3 and physical L4 are not claimed.

---

## 2026-11-21 — Enforce zero unchecked WAN integer getter callsites

- Parent main SHA: `b8cb95b34d1362b23c32cf5c58538b57667e65cc`.
- Audited the current `net_wan.c` after migrating DNS mode, PPPoE manual/demand, WAN/MAN MTU, EAPOL/Kabinet auth and MAN DNS route setup/teardown. The legacy `get_wan_unit_value_int()` remains only as a public compatibility definition; there are no remaining internal callsites.
- Added a whole-file source-contract assertion so future WAN code cannot silently reintroduce the error-collapsing getter into an internal network decision.
- This is a static integration guard; exact-head CI is still required for C policy tests and downstream workflow gates.

---
## 2026-11-21 — Make WAN source-contract parser skip prototypes

- Parent main SHA: `de62205e3e02687e5908e863312945ff4b81bc39`.
- Hardened the WAN source-contract helper so function lookups skip forward declarations and select the actual implementation body. This matters for `get_wan_dns_static_status()` and route helpers, where a prototype can precede the definition; the previous naïve first-brace search could inspect the next function body and produce a false failure.
- No production code changed. Exact-head WAN route CI will validate the current DNS, PPPoE, MTU, authentication and MAN route assertions.

---
## 2026-11-21 — Preserve WAN DNS-mode NVRAM read failures

- Parent main SHA: `f2920b4d320be4bbb7340673f1b29a9c7c0d2b62`.
- Narrow WAN DNS slice: added GNU89-compatible strict decimal parsing and a checked integer-read policy around `nvram_get_checked()`. Missing keys preserve the existing numeric default, successful values must be a complete base-10 integer within `int` bounds, malformed prefixes/overflow fail, and backend/read/close errors remain distinct.
- Migrated `get_wan_dns_static()` internals to an errno-style status helper for both USB modem `modem_dnsa` and normal WAN `dnsenable_x`. The exported legacy getter retains its prior conservative boolean fallback and reports the error through `errno`.
- `update_resolvconf()` now aborts before staging/opening `/etc/resolv.conf` if the DNS-mode read failed; it releases the resolver lock and preserves the last-known-good file.
- Added missing/default, valid signed/whitespace/boundary, malformed, overflow, backend-error and null-pointer policy regressions; added a source-contract checker and exact workflow path/step wiring.
- No local compiler/checkout is available. Exact-head WAN-route, Process, AI-docs, Storage and XY-C1 Actions must be inspected on the resulting SHA; host regression PASS is not firmware L3.

---
## 2026-11-21 — Fix archive index path/size and Storage filter assertion

- Parent main SHA: `d397166be2fcd3582492a8004eac5e8dec216929`.
- AI handoff run `#44889732024` on `d397166be2fcd3582492a8004eac5e8dec216929` found that the archive itself was restored, but its index row used only `part-021.md` rather than the repository-relative path and recorded 107,416 rather than the actual 107,650 UTF-8 bytes. Corrected the index path and byte count; the original blob SHA and 100 headings are unchanged.
- Storage run `#44889614290` on `3791719dff38276df830dafe1ffbb2f501f0b13b` passed BusyBox 1.24.2 ash lifecycle tests and every earlier runtime check, then failed because a checker counted three Optware paths across the entire YAML file. Embedded test-script references made the total exceed two even though each path existed exactly once in push and pull_request filters. Corrected the checker to inspect those filter sections separately.
- XY-C1 run `#44889614321` completed SUCCESS on `3791719d`, but Build XY-C1, TRX verification and artifact upload were all SKIPPED by production-input detection for documentation/checker-only changes; this is not L3 verification. The last verified product-source baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Re-run exact-head Storage Hotplug and AI handoff checks on this correction. No production source behavior changed.
- Resulting main commit: THIS COMMIT

---

## 2026-11-21 — Restore the indexed November log archive object

- Parent main SHA: `3791719dff38276df830dafe1ffbb2f501f0b13b`.
- Exact-head AI handoff run `#44889614300` on `3791719dff38276df830dafe1ffbb2f501f0b13b` failed because `docs/dev-log/2026/11/part-021.md` was referenced by `docs/dev-log/INDEX.md` but missing from the Git tree.
- Recovered the original complete 107,416-byte archive from its existing Git blob `f80bf2d087fa1c313cb671f2108e3279092db234` and restored the exact indexed path; no archive content was regenerated or truncated.
- At last poll, Storage Hotplug run `#44889614290` and XY-C1 firmware run `#44889614321` were still in progress on exact SHA `3791719dff38276df830dafe1ffbb2f501f0b13b`. Their results must remain attached only to that source SHA.
- Re-run AI handoff and Storage Hotplug exact-head checks on this archive-restoration commit. Product L3 remains verified only on `20e139d230b73eabc5f25a2b17d56a3a5768af11`; physical-router L4 remains unverified.
- Resulting main commit: THIS COMMIT

---

## 2026-11-21 — Align Optware tri-state checker and end-session handoff contract

- Parent main SHA: `fa888e9c9323a555d63b2d84a9bd4bd82a181764`.
- Exact Storage Hotplug run `#44889271838` on source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17`: the BusyBox 1.24.2 ash lifecycle regression and all shell regression steps passed, including the explicit UNKNOWN/malformed-mount case; only the final static source checker failed because it expected `case _mount_match_result in` while the actual shell correctly uses `case $_mount_match_result in`. Corrected the checker to match the actual variable expansion; no production shell behavior was changed.
- Exact Process inventory `#44889271901` passed on `421042f3`. Exact AI handoff run `#44889426235` on `fa888e9` failed because the handoff was missing the required literal `one atomic main commit` and the required four structured sections. Reorganized the live handoff into those required sections while retaining the mandatory end-of-session accuracy checkpoint.
- Fixed the dev-log archive index to record the actual part-021 blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`.
- Storage Hotplug and AI handoff exact-head Actions must be rechecked on this commit. XY-C1 run `#44889271879` remained PENDING/IN_PROGRESS on exact source SHA `421042f3` at the last poll; no result is inferred for this documentation-only child. Product L3 remains verified only on `20e139d230b73eabc5f25a2b17d56a3a5768af11`; physical-router L4 is not verified.

---
## 2026-11-21 — Preserve UNKNOWN mount inventory status under BusyBox awk

- Parent main SHA: `065837e4dcc383ebe2d5276b8165ffa834f9dd3f`.
- Exact-head Storage Hotplug run `#44888827697` passed all earlier shell/source checks but failed in the BusyBox 1.24.2 ash lifecycle step. The truncated five-field `/proc/mounts` test expected UNKNOWN (2) but observed 1.
- The mount matcher encoded its tri-state contract in AWK process exit values 0/1/2. BusyBox 1.24.2 did not preserve the third value through the ash test path, collapsing malformed-inventory UNKNOWN into ordinary non-match. Replaced the nonzero AWK exits with explicit `match` / `different` / `unknown` output tokens; the shell maps those tokens back to 0/1/2 and still maps AWK execution/read failure to UNKNOWN.
- Extended the standalone source checker to enforce this portable result protocol. Existing regressions continue to cover exact source+mountpoint matching, device-prefix collision, malformed/truncated rows, duplicate rows, empty and unreadable inventories.
- Exact-head Storage Hotplug regression must confirm the correction. Process inventory and AI handoff passed on the parent SHA; XY-C1 run `#44888827678` was still running on parent SHA at last poll. No new firmware source changed.
- Resulting main commit: THIS COMMIT.

---

---

## 2026-11-21 — Match Optware teardown to exact mount ownership

- Parent main SHA: `94c0e2c11f6e73b90366ed6e74cb292858acb6e7`.
- Fixed an unsafe `opt-umount.sh` source check that grepped `/proc/mounts` for an unquoted device substring and ignored the supplied mountpoint. For example, `/dev/sda1` could match `/dev/sda10`, causing Optware teardown during an unrelated partition eject.
- Added `optware_lifecycle_mount_matches()`: require target `/opt`, validate every six-field mount record, compare source and mountpoint as exact fields, reject duplicate matches and return UNKNOWN for malformed/empty/unreadable inventory. Teardown proceeds only on one exact owner row and fails closed on UNKNOWN.
- Added shell regression cases for exact match, device-prefix collision, wrong mountpoint, malformed row, duplicate row, empty/unreadable inventory; mirrored these contracts in the Storage Hotplug workflow and standalone static checker.
- Prior validations: Process lifecycle `#44887824016` and AI handoff `#44887824043` PASS on `94c0e2c`; XY-C1 Build, TRX verification and artifact upload `#44887656164` PASS on product source `20e139d2` (L3 verified on that SHA).
- No physical-router L4 evidence is available. This Optware patch requires exact-head Storage Hotplug Actions after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Repair PID-file regression workflow YAML indentation

- Parent main SHA: `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Exact-head Process workflow run `#44887655190` failed before job creation because the new parser step had one extra leading space, breaking the workflow's step-level YAML indentation. Corrected the step to match adjacent six-space step/eight-space `run` indentation; no test or workflow coverage was removed.
- On the parent SHA, Process inventory `#44887656234` and AI handoff `#44887656223` both passed. The standalone parser regression also passed local GNU89/ASan/UBSan compilation and execution; the corrected exact-head Process workflow must still confirm integration.
- XY-C1 exact-source L3 is now verified: run `#44885238962` passed build, TRX validation and artifact upload on SHA `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Harden exported legacy shared PID-file signalling; confirm XY-C1 L3

- Parent main SHA: `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`.
- The legacy `kill_pidfile_s()` helper used `fgets` followed by `strtoul(..., NULL, 0)` and cast before range checking. This accepted numeric prefixes such as `2junk` and could wrap out-of-range records on wider hosts.
- Hardened the exported compatibility API without changing PID-only record syntax: nonblocking open plus regular-file validation, bounded full-record read, strict trailing/NUL checks, and `INT_MAX` validation before casting. Legacy decimal/octal/hex forms and surrounding whitespace remain accepted. Added GNU89 parser regressions, source integration assertions and Process-workflow push/PR path filters.
- Exact-source XY-C1 `#44885238962` completed SUCCESS on `8a55d24a4b797c96ca94bc8ece112966d86fa204`; Build XY-C1, TRX output verification and artifact upload all passed. Firmware build L3 is verified on that SHA; physical-router L4 remains unverified.
- The new standalone parser regression was compiled and run locally with GNU89, warnings-as-errors, ASan and UBSan: PASS. Exact-head Actions are still required for repository integration.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Remove obsolete watchdog ETIMEDOUT errno checker assertions

- Parent main SHA: `6ce50b1f558a99db1569fa6fd1a03aff4075e012`.
- Exact Process lifecycle run #44885848598 failed in `scripts/check_process_lifecycle_integration.py` at two duplicate assertions requiring `errno = ETIMEDOUT;` inside `start_watchdog()`.
- Production watchdog now supplies `ETIMEDOUT` as the explicit primary error argument to `watchdog_rollback_failed_start(-1, ETIMEDOUT, "readiness timeout")`; the preceding source-contract loop already verifies that exact readiness-timeout rollback call in both duplicated checker sections. Removed only the obsolete errno-in-body assertions, leaving the timeout policy check intact.
- AI handoff #44885848590 passed on the same parent SHA. Exact Process lifecycle Actions must rerun after this checker-only correction; no production code changed.

---

## 2026-11-21 — Correct Process watchdog rollback contract and handoff execution metadata

- Beijing time: 2026-11-21 16:31 (UTC+8). Parent main SHA: `accf1726b163d54bd269019fd590554fc430fa0e`.
- Watchdog signal workflow `44885579764` passed on parent `9a1175e8`. AI handoff `44885716352` then failed because the exact validator also requires the phrase `one atomic main commit`; updated the handoff mandate with that exact branch/commit invariant.
- Aggregate Process lifecycle `44885238958` on product-source SHA `8a55d24a` failed in the final source integration check because two copies of the watchdog contract still required `return launch_status;`. Both copies now require all six failure exits to invoke `watchdog_rollback_failed_start()`, check targeted stop/fresh-inventory adapters, verify errno preservation and assert every failure operation marker.
- No firmware production source changed. No local Python/C tests are claimed; exact-head Process and AI handoff Actions are required after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Match the handoff checkpoint heading to the exact validator contract

- Parent main SHA: `9a1175e8c5332751b68b7861c1ea5b043a67547d`.
- The exact Watchdog workflow passed on parent SHA (`44885579764`). AI handoff `44885579772` still failed: the validator requires the literal heading text `Mandatory handoff checkpoint` plus the exact conditional-document-edit sentence. The prior heading inserted `end-of-session` between those words, so it did not satisfy the contract.
- Changed the heading to `## Mandatory handoff checkpoint — end-of-session accuracy`; the required sentence remains present. This is a material correction to a failed exact-SHA contract, not a no-op documentation rewrite.
- Exact-SHA AI handoff must be rechecked after publication; no production files changed.

---

## 2026-11-21 — Fix duplicate watchdog workflow filters and restore mandatory handoff checkpoint wording

- Parent main SHA: `759eb1181483a74f8f4103ec7a5827fcac375dd4`.
- Exact Watchdog Action #44885340239 failed because the detached-daemon rollback header and regression test were each listed twice in both push and pull_request path filters. Removed only duplicates; the inline source-contract path list remains single-copy.
- Exact AI handoff Action #44885340194 failed because the rewritten handoff omitted the validator's exact end-of-session checkpoint heading and wording. Restored both required phrases.
- This follow-up changes no production behavior. Exact-SHA Watchdog and AI handoff results are required after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align watchdog workflow contract with targeted startup rollback

- Parent main SHA: `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- The first exact-head watchdog run for the detached-start rollback change still contained a stale inline assertion requiring `return launch_status;`. Production correctly routes the launcher failure through `watchdog_rollback_failed_start()` and therefore the old assertion would fail despite correct behavior.
- Updated the workflow to require all six post-launch failure exits to use the rollback helper, check the stop/inventory adapters, and include the shared rollback policy header/test in push and PR path coverage.
- No production source changed in this follow-up. Exact-SHA Watchdog, Process lifecycle and XY-C1 Actions must pass after publication.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Roll back the watchdog daemon when detached startup cannot be verified

- Parent main SHA: `e6b112eb34b7e7fc7f3571e8e823e1ee91a9ea19`.
- `start_watchdog()` launched `/sbin/watchdog` through `eval()`, but `watchdog_main()` daemonizes before publishing its PID. Launcher failure, interrupted initial/retry waits, UNKNOWN/invalid process inventory and readiness timeout all returned directly without stopping a child that could still appear later.
- Added a targeted detached-start rollback adapter: stop only the `watchdog` process family, perform a fresh tri-state inventory scan, log incomplete cleanup, and preserve the original launch/wait/scan/timeout status and errno.
- Existing-running startup remains a no-op, and UNKNOWN preflight still fails closed before launch. The shared rollback policy regression already covers confirmed cleanup, a still-running child, UNKNOWN cleanup inventory and primary error preservation.
- Extended the watchdog source contract to require every post-launch failure branch to use rollback. Added the rollback-policy header and unit test to watchdog workflow path filters; XY-C1 already tracks the header and its input contract now asserts it remains a firmware input.
- Exact-SHA watchdog, Process lifecycle, AI handoff and XY-C1 Actions are required after publication; no local tests or new firmware L3 result are claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Ensure crond timeout always executes rollback

- Parent main SHA: `7085606f16b27e7ebc693cb0c3d3af29eb8e117b`.
- A post-commit source review caught a real status-propagation bug not covered by the current static tests: after a successful retry sleep followed by all-ABSENT probes, `crond` retained `status == 0`. Passing that zero into `detached_daemon_start_rollback_with_ops()` made its documented success fast path skip cleanup even though the function logged a startup timeout.
- The timeout now passes the explicit failure status `-1`; the crond lifecycle contract asserts this exact argument. Other core daemon timeout paths already pass `-1`/ETIMEDOUT.
- This is a production correctness fix discovered during review after the preceding Actions passed; exact-head Process Lifecycle and XY-C1 source validation are required again.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Update daemon lifecycle source contracts for rollback

- Parent main SHA: `1b25077e6e5a9f8750d4b5c76637beec5ca354d1`.
- The core RC daemon rollback change is now present on main. Its first exact-head test run compiled and passed the crond/infosvr C policy unit tests and most Process Lifecycle contracts, then failed three Python source validators that still expected the old direct-return behavior: infosvr, crond and the aggregated Process Lifecycle checker.
- Updated those validators to require the new targeted `optional_daemon_rollback_failed_start()` path, primary errno capture, UNKNOWN rollback handling and bounded timeout cleanup instead of the previous direct postcondition/status returns. The Process Lifecycle checker already verifies the shared adapter uses identity-fenced `kill_services()` and a fresh inventory.
- No production source changed in this commit. Exact-head CI is the verification target; no local checkout was available.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Serialize the SCUT watchdog stale-PID reclaim transaction

- Parent main SHA: `9ff6c64b9731a848ed07c0a08500d2c8608b09e7`.
- The stale-PID branch compared the old PID twice and then unlinked/recreated the PID file without inter-process serialization. Two overlapping cron invocations could both validate the stale record, race at unlink, and allow one invocation to remove the PID record claimed by the other.
- Added a stable adjacent lock file and BusyBox `flock -n 9` around the complete identity-check/reclaim/publish transaction. The persistent lock inode is never unlinked; FD 9 is closed immediately after the PID claim so health-check children and restarted daemons do not inherit the lock.
- Added a deterministic race regression that blocks the first invocation inside PID-file `rm` after the stale comparison, then starts a second invocation. It requires exactly one health check, retains the stable lock inode and rejects a symlink lock path.
- Confirmed BusyBox 1.24.x `CONFIG_FLOCK` defaults to y. Dedicated SCUT workflow guards the Kconfig dependency and host utility; XY-C1 path classification/trigger tests include the config source.
- Exact-head SCUT, AI handoff and XY-C1 Actions are required; no firmware L3 or hardware L4 result is claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Roll back failed detached starts for core RC daemons

- Parent main SHA: `c8923187b3027da02b41dd6d76ae00b2dc0b12cd`.
- Closed the late-child/orphan race in syslogd, klogd, infosvr, crond and networkmap. Each already waited for a detached RUNNING postcondition, but launcher errors, interrupted initial/retry waits, UNKNOWN inventories and readiness timeouts returned without stopping a child that could still have started.
- Added a shared RC adapter using `kill_services()` for only the affected process name, followed by fresh tri-state inventory. The common detached rollback policy keeps the original startup status/errno; incomplete cleanup is logged without masking the primary failure. Existing RUNNING preflight remains a no-op and UNKNOWN preflight still aborts before launch.
- Reused the previously ASan/UBSan-tested detached rollback policy and strengthened the Process Lifecycle source contract to require rollback wiring for all five daemons. Process Lifecycle and XY-C1 already watch `services.c` and the shared helper/test.
- Local build is unavailable; exact-head Process Lifecycle and XY-C1 Actions must validate the C integration. Router startup behavior remains unverified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Fix UDPXY checker NameError after detached-daemon policy validation

- Parent main SHA: `41023c0835fc399ea521e397a8fb5563dd86f02e`.
- Exact WAN route Action #44883618627 compiled and passed the detached-daemon policy regression, then failed in `scripts/check_udpxy_startup.py` because `igmpproxy_start` was used in a rollback assertion before the checker assigned its function body.
- Moved the XUPnPD and IGMPproxy function-body extraction ahead of their rollback assertions. No production behavior or assertions were removed or relaxed.
- Process lifecycle inventory #44883618647, process fail-closed regression #44883618648 and AI handoff contract #44883618619 passed on the parent SHA. Exact-head WAN route, XY-C1 and NVRAM runs remain required.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-21 — Align IPTV rollback regressions with existing checker contracts

- Parent main SHA: `daf8248436c650eb10429d7098e52535c436ae4d`.
- Exact-SHA Actions for the IPTV rollback patch compiled and passed the new GNU89/ASan/UBSan unit test, but two existing source-check contracts needed alignment: the process checker assumed XUPnPD used the generic `iptv_proxy_confirm_started` wrapper (it has its own `xupnpd_confirm_started` wrapper), and the UDpxy checker deliberately expects the original test PASS line. Corrected the per-daemon confirm expectation and preserved the established stdout string.
- Extended the UDpxy checker to verify the new rollback policy, targeted identity-safe stop callback and rollback test cases across UDPXY/MSD Lite, XUPnPD and IGMPproxy.
- Run failures on SHA `734f8116` were checker-contract mismatches after the C unit itself passed; exact-head reruns on this fix and current main are authoritative.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Reset errno before UDPXY/MSD Lite launch

- Parent main SHA: `734f811620f596b9ea72863261d8cc4a7103bee2`.
- Tightened failed-launch diagnostics and rollback context: the UDPXY and MSD Lite launcher paths now clear ambient `errno` immediately before `eval()`, so a nonzero launcher result is not paired with an unrelated errno left by an earlier successful config/procfs operation.
- Added a source-contract guard for the ordering.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Roll back IPTV daemons after failed detached startup

- Parent main SHA: `9df0573782afb3a3e19864ac17250d7358ef26ca`.
- Fixed a lifecycle gap shared by UDPXY/MSD Lite, XUPnPD and IGMPproxy: each launcher required a bounded RUNNING postcondition, but a failed launcher/readiness check returned without reaping a child that appeared late or survived a nonzero launcher result. RC could report startup failure while the proxy remained active.
- Added a generic detached-daemon rollback policy that invokes a targeted stop callback, re-inventories process state, and preserves the primary startup status/errno. Production IPTV callbacks use `kill_services()` with only the failed daemon name; that existing stop path is PID/start-time identity checked. Cleanup logs confirmed absence versus remaining RUNNING/UNKNOWN state.
- Applied rollback independently to each failed IPTV daemon, leaving successfully started sibling services alone. Added GNU89 ASan/UBSan unit coverage for confirmed absence, stubborn RUNNING child, UNKNOWN inventory, successful-start no-op, invalid callbacks and primary-error preservation.
- Extended Process Lifecycle triggers and compilation, plus XY-C1 source-path filters. No local checkout was available; exact-SHA host CI and XY-C1 build are the required verification. Physical-router IPTV behavior remains unverified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Reset preserved UNKNOWN PID fixture before symlink case

- Parent main SHA: `bc72bd5df72543ced814d565177932db253c47a6`.
- The next exact-SHA watchdog run progressed through active-instance, recycled-PID, malformed-PID and UNKNOWN-procfs checks, then found the test reused the intentionally preserved UNKNOWN PID file when creating the symlink fixture. The fixture now explicitly removes that prior file before creating the symlink case.
- The failure was confined to test setup; production source behavior was not implicated.
- Resulting commit: THIS COMMIT.

---

## 2026-11-21 — Correct watchdog PID-file and procfs fixture escaping

- Parent main SHA: `59284736191e9480cdd7fcc77156bd8a1f9925a4`.
- The exact-SHA watchdog test exposed an escaping error in the generated shell text: the PID writer used a literal backslash-`n` instead of a line terminator, and the mock procfs fixture used literal backslash sequences instead of NUL delimiters. Corrected the two PID writes and all mock `printf` formats. The process-command-line reader already used the proper single-backslash `tr '\\000'` syntax.
- The earlier watchdog Actions failed only in the fixture path (first workspace allocation, then active-PID identity); this change addresses the second root cause. No extra production behavior was added beyond proper PID-file line termination.
- Resulting commit: THIS COMMIT.
