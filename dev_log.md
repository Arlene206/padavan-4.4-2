## 2026-11-22 — Copy fallback WAN DHCP netmask before previous-lease reads

- Parent main SHA: `afb7b19f7d91b61912abfbc89da4032dcb24e59b`. In `udhcpc_bound()`, when the DHCP environment omitted `subnet`, the callback retained `nvram_safe_get("<wan>_netmask")` storage across its subsequent transactional snapshot of the previous lease. That snapshot reads many NVRAM keys and can overwrite the shared cache before the fallback mask is passed to subnet comparison/interface configuration.
- Replaced the borrowed pointer with a bounded `INET_ADDRSTRLEN` buffer and checked `rc_wan_nvram_copy_value()`. Backend errors and truncation now return before lease validation or interface/route changes; a missing optional key still maps to empty text and is rejected by the existing netmask validator.
- Added a shared-cache overwrite regression and WAN DHCP source-order assertions. Exact-head WAN DHCP/route/XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot LAN gateway and router domain before resolver/IPv6 reads

- Parent main SHA: `bccd3fc915e963d4cb6ddeab223f02edf3fef57b`. Two remaining cache-lifetime defects were found in the LAN resolver/hostname path: `lan_up_manual()` kept `lan_gateway` borrowed while later reading both DNS server keys and then used it as fallback; `update_hosts_router()` kept `lan_domain` borrowed across IPv6 service-state probing before writing hostname files.
- Both now use `rc_wan_nvram_copy_value()` into bounded owned buffers and abort before resolver/hostname side effects on read or truncation errors. Missing NVRAM keys still become empty strings, preserving the existing fallback behavior.
- Added cache-overwrite regressions; updated Process and HTTPD source contracts. Exact-head Process/HTTPD and XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align HTTPD security cross-check with AP host snapshot

- Parent main SHA: `2f6231349eaf3fafe0c1885a95afa887d82c11c5`. HTTPD Security run #45008874461 on `56137665cea32ba5d5befce235889f996e4c9fa3` passed the DHCP lease tests, then stopped at the stale AP hostname source assertion in `.github/workflows/httpd-security.yml` (embedded Python line 154). It still required the two direct `nvram_safe_get()` arguments removed in `update_hosts_ap()`.
- Updated the contract to require ordered checked copies into owned `lan_ipaddr` / `lan_dname` buffers before `create_hosts_lan`, and to forbid direct borrowed NVRAM pointers. Exact-head HTTPD rerun pending.

---

## 2026-11-22 — Update XUPnPD integration check for owned LAN address

- Parent main SHA: `3664d438d7b2ed36957b45b91b8e647f1a592deb`. The Process integration checker still required the old direct `xupnpd_config_valid_ipv4(nvram_safe_get(...))` expression after `start_xupnpd()` was correctly changed to a bounded checked copy. Exact Process run #45008874486 on `56137665cea32ba5d5befce235889f996e4c9fa3` completed the preceding regression and source checks, then failed specifically at `scripts/check_process_lifecycle_integration.py:406`.
- Updated that assertion to require the checked NVRAM copy, the owned `lan_addr` validation, and absence of the borrowed-pointer pattern. This is a checker update, not a production-code rollback. Exact rerun pending.

---

## 2026-11-22 — Copy udpxy argv options before the NVRAM cache can wrap

- Parent main SHA: `56137665cea32ba5d5befce235889f996e4c9fa3`. `start_udpxy()` passed three `nvram_safe_get()` pointers directly through the variadic `eval()` call. These pointers are backed by the shared NVRAM cache ring and can be invalidated by later reads before exec constructs argv.
- The enable port is now serialized from its already-parsed integer; client limit and renew interval are copied with checked bounded reads into owned buffers before launch. Read/truncation errors abort startup instead of passing stale or mixed arguments.
- Added cache-overwrite regression coverage and Process source-contract assertions; removed two accidental dead-negative source assertions from the preceding XUPnPD checker. Exact-head Process and XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Own hostname and XUPnPD address across NVRAM reads

- Parent main SHA: `c4bc1f86973e9f45fd53f74c7bd613edc48da6d7`. `get_our_hostname()` kept the shared-cache pointer for `computer_name` while falling back to `productid`, and returned a borrowed cache pointer to callers. `start_xupnpd()` retained `lan_ipaddr_t` across later port/enable NVRAM reads before serializing its URL.
- Replaced both with bounded checked copies into owned buffers. Hostname read errors now return NULL (the caller already rejects a NULL hostname); XUPnPD configuration returns a checked error before filesystem or daemon side effects. The existing URL formatter now consumes the stable preflight address.
- Added GNU89 cache-overwrite regressions and Process source-contract assertions. Exact-head Process / XY-C1 validation pending; Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Normalize LAN NVRAM source-contract call whitespace

- Parent main SHA: `77fcbc9cc84a55906c1bed2456e87eab1c70bf86`. Exact Process run #45008185010 on `864e6c651675f3d22f1a23ebc9b0936f06d6833a` compiled and ran the bridge-init GNU89 ASan/UBSan regression successfully, then failed in the later embedded source checker at Python line 213. The checker collapsed line breaks but expected no space after the helper's opening parenthesis; the C call was intentionally split after `(`, so the normalized source contained one space.
- Corrected the LAN MAC, startup identity and AP-host snapshot matchers to account for the normalized space. This is checker-only; no production behavior changed. Exact rerun is pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot LAN address, mask and AP host identity before reuse

- Parent main SHA: `864e6c651675f3d22f1a23ebc9b0936f06d6833a`. `start_lan()` retained borrowed pointers for `lan_ipaddr` and `lan_netmask`, then fetched `lan_domain` before using all three; `update_hosts_ap()` also passed two cache-backed NVRAM pointers directly as function arguments, whose evaluation order is unspecified in C.
- Replaced these reads with checked copies into bounded owned buffers before interface activation or host-file publication. Backend errors/truncation now abort the operation before its corresponding side effects; absent keys preserve the prior empty-string behavior.
- Extended the bridge-init GNU89 cache-overwrite regressions and Process source-order contract. Exact-head Process and XY-C1 validation pending; Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Copy LAN bridge MAC before subsequent NVRAM reads

- Parent main SHA: `0f4dcd5b0ed934726680577e7d830513e5ba4a5f`. `init_bridge()` retained the borrowed `nvram_safe_get("lan_hwaddr")` pointer across switch/VLAN setup, which performs additional NVRAM reads before applying the MAC to the physical and bridge interfaces. The NVRAM getter's backing storage is a shared cache and must not be retained across reads.
- Replaced the borrowed pointer with `rc_wan_nvram_copy_value()` into a bounded owned buffer. Backend errors and truncation now abort bridge initialization before switch/bridge side effects; a genuinely absent key still maps to the legacy empty value.
- Added GNU89 regression coverage for cache-slot overwrite and checked-read failure, plus Process source-order assertions. Process and XY-C1 exact-head validation pending; XY-C1 must run Build, TRX verification and artifact upload on this production-source SHA before L3 can be claimed. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Keep WAN route test assertion out of trigger-path count

- Parent main SHA: `4c0906ac9c8c79eb5e6f4119a002578366fd539d`. Exact WAN route workflow #45007745240 compiled and passed the shared WAN NVRAM policy regression after the missing-route expectation correction. It then failed at the DDNS status checker because the inline assertion added a third exact quoted occurrence of `trunk/user/rc/src/tests/wan_nvram_status_policy.c`; the checker intentionally requires exactly two occurrences for push and pull_request.
- The workflow already reads the test into `wan_nvram_test`. Changed the assertion to reuse that variable, preserving both trigger filters and reducing the exact path count back to two. No production behavior changed; exact-head WAN rerun pending.

---

## 2026-11-22 — Correct missing-route snapshot regression expectation

- Parent main SHA: `d1cd34322e3ce8058cf7fc363fbabaa8b4696247`. Pre-commit review found one incorrect assertion in the new DHCP route snapshot regression: the fixture marks only `wan0_routes_rfc` absent, so the first and third route strings must remain present.
- Corrected the missing-key case to retain `wan0_routes` and `wan0_routes_ms` while asserting only the middle `routes_rfc` destination is empty. No production source change; exact-head Actions for the route snapshot remain pending.

---

## 2026-11-22 — Snapshot DHCP route options transactionally

- Parent main SHA: `789ca3e5000e677f3e3c9e46609f64e9e771b469`. The DHCP route wrappers retained three pointers returned by `nvram_safe_get()` for `routes`, `routes_rfc`, and `routes_ms` while retrieving the remaining values. Those pointers refer to reusable NVRAM cache-ring storage and are not safe to retain across a wrap or concurrent NVRAM activity.
- Replaced the three unchecked reads with a single `rc_wan_nvram_copy_snapshot()` under the existing exclusive NVRAM transaction callbacks. Each value is copied to a bounded `NVRAM_MAX_VALUE_LEN + 1` buffer before the next read; key construction is length-checked, and failed reads/transaction close abort add/delete instead of silently applying an incomplete tuple.
- Added GNU89 regression for three route option values, reusable-cache overwrite, optional missing fields, every individual read failure and transaction-end failure; extended WAN route source-order contract. Exact-head route regression pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align IPv6 workflow with pre-mount NFS ACL snapshot

- Parent main SHA: `01ccfa38a77c2d54327a7c96dfd9c8e4c10cfe26`. Exact IPv6 service-type run #45006965726 compiled and passed the checked service-type C policy tests, then failed only in `scripts/check_ipv6_service_type_status.py` line 202. That assertion expected NFS to read its IPv6 ACL after opening `/proc/mounts`.
- The NFS ACL fix `67514d707a91ac3e34bd513032a6a85a29e0f2d3` intentionally moves LAN override/fallback, IPv6 and VPN ACL copies and validation before opening /proc/mounts and before generated-file staging. Updated the checker to assert all seven checked ACL copies precede mount enumeration/staging, preserving the security and fail-closed ordering.
- WAN DHCP callback fix `01ccfa38a77c2d54327a7c96dfd9c8e4c10cfe26` exact run #45006965680 PASS. Its broad fan-out mostly PASS; exact XY-C1 #45006965654 and NVRAM audit #45006965617 remained IN_PROGRESS at last poll. Current L3 unverified; hardware L4 NOT VERIFIED. Exact IPv6 rerun pending.

---

## 2026-11-22 — Preserve WAN DHCP hostname and vendor class across NVRAM reads

- Parent main SHA: `67514d707a91ac3e34bd513032a6a85a29e0f2d3`. `start_udhcpc_wan()` used two `get_wan_unit_value()` cache-backed pointers for `hostname` and `vci`; reading the latter can overwrite the former before DHCP client launch.
- Replaced both pointers with checked, bounded copies into owned buffers. A read error or truncation now aborts DHCP startup before spawning udhcpc; absent/empty values remain optional as before.
- Added a fake shared-cache regression verifying the hostname survives the vendor-class read and extended the WAN DHCP source-order contract. Exact-head WAN validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot NFS export ACL NVRAM inputs

- Parent main SHA: `1c61749ba6887639a7430b1d7d7df0b6bc661189`. The active handoff identified shared-cache lifetime hazards in `write_nfsd_exports()`: successive `nvram_safe_get()` calls returned pointers reused before IPv4 LAN fallback, IPv6 LAN and VPN ACL formatting.
- Added a checked, bounded NFS ACL string-copy policy and switched LAN override/fallback, IPv6 address/prefix and VPN subnet reads to owned buffers. All required copies and ACL validation now complete before /proc/mounts is opened or an exports staging file is created; missing keys remain empty and backend/truncation failures abort publication.
- Added GNU89 test coverage for shared-cache overwrite, fallback ACL formatting, IPv6/VPN ACL formatting, missing settings, backend errors and truncation. Extended the NFS workflow source-order contract. Exact-head NFS run pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Avoid matching comments in the FTP firewall source contract

- Parent main SHA: 2333039ea9c33c9b02f941621928f64659b249fc. Exact storage hotplug #45005813291 passed the embedded whole-caller-graph check, then failed scripts/check_storage_safe_remove.py line 654 because its negative substring assertion matched the comment mentioning nvram_match() in control_ftp_fw.
- THIS correction narrows the check to the executable call token nvram_match(" while continuing to reject actual lossy NVRAM match calls. No product-source changes.
- Exact rerun pending; NFS export ACL input-pointer lifetime audit is next. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Update storage firewall-refresh presence contract

- Parent main SHA: 9553ea5c4331d15f85db2842765dd82cfd428e4d. Exact 595 Storage hotplug #45005582512 failed at the embedded whole-caller-graph checker: lines 473–474 still expected Transmission and Aria start-presence to equal RUNNING, although the new source intentionally requests firewall reconciliation for every state except confirmed ABSENT.
- THIS CI-only follow-up updates both assertions to the UNKNOWN-inclusive fail-closed condition. The actual 595 caller graph had passed preceding storage lock, mount inventory, eject and Optware assertions; the dedicated source checker for the new firewall-refresh gates had not yet been reached.
- HTTPD firewall snapshot checker fix at 9553 exact HTTPD Authentication #45005694009 and AI docs #45005694083 PASS. Corrected storage workflow rerun pending; no current product L3 claim.

---

## 2026-11-22 — Update HTTPD firewall snapshot source contract

- Parent main SHA: `595b2dcb7a927a369d3e29a0aef75a5431fc7e74`. Storage firewall-refresh source commit #595 was under exact-head validation; its storage hotplug, Process, AI docs, NVRAM, Firewall Startup and XY-C1 checks were IN_PROGRESS at last inspection.
- HTTPD Authentication and Security Regression #45005080682 on firewall source SHA `cea789c171d0698f32cbe1d188ce8dda79c43353` failed at the first inline checker because it still searched for the removed `nvram_safe_get("lan_ipaddr")` assignment. The unchanged HTTPD workflow on source parent 595 was also IN_PROGRESS as run #45005582489.
- THIS workflow-only change replaces the stale locator with status-bearing copy/error-order assertions for LAN IP, manual WAN IP, WAN-unit IP and LAN netmask. It ensures all four inputs are copied and errors returned before network side effects, while retaining address validation/fallback and netmask-before-subnet behavior. Exact-head HTTPD/AI reruns pending; product behavior unchanged; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Always reconcile firewall after storage daemon presence changes

- Parent main SHA: 005d1e340cfc7a68b0686833c8ab96d30122fe73. Exact 005 Firewall Startup Stability #45005183052 PASS and AI docs #45005183110 PASS; exact cea NVRAM audit #45005080676 PASS. Exact cea XY-C1 #45005080673 remained in progress; no current product L3 claim.
- FTP, Transmission and Aria lifecycle paths used unchecked nvram_match("fw_enable_x","1") gates before firewall refresh. FTP additionally gated on ftpd_wopen, and its restart helper did not cover ordinary daemon disappearance. Read errors could silently skip the firewall rebuild and leave stale mappings.
- THIS FIX invokes restart_firewall() on each service-presence transition without lossy enable/exposure gates; the firewall itself consumes checked configuration and handles rule publication/cleanup. Storage start/stop app groups now include FTP regardless of its exposure toggle and treat UNKNOWN process presence conservatively.
- Added integration assertions to scripts/check_storage_safe_remove.py; its existing storage-hotplug workflow path filters cover both changed files. Exact-head validation pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Repair stale firewall netmask source-contract locator

- Parent main SHA: `cea789c171d0698f32cbe1d188ce8dda79c43353`. Exact Firewall Startup Stability #45005080730 failed after the new checked-IP source-contract script and all firewall C ASan/UBSan tests passed. The final inline NAT/netmask contract still searched for `lan_netmask = nvram_safe_get(...)`, which this source fix intentionally removed; Python raised `ValueError: substring not found`.
- THIS correction anchors the existing contiguity-validation ordering assertion on the checked `"lan_netmask"` read instead. No product-source behavior changes in this CI-only follow-up.
- Exact cea HTTPD lifecycle #45005080894 and IPv6 service-type #45005080692 PASS. NVRAM, XY-C1, WAN route, FULLCONENAT, Process inventory/lifecycle, HTTPD auth and WAN DHCP were still running at last poll; active L3 remains unverified.

---

## 2026-11-22 — Use checked snapshots for firewall IP inputs

- Parent main SHA: `a5777a906c785d296e8612cf4954fc2a5dbb6749`. Exact a577 USB printer tests/checker #45004808849 PASS; Process umbrella #45004808803 PASS; Process inventory #45004808838 PASS; AI docs #45004808939 PASS. NVRAM audit #45004808813, USB storage #45004808805 and XY-C1 #45004808822 were pending at last poll.
- `start_firewall_ex_locked()` used `nvram_safe_get()` for LAN IP, management WAN IP and LAN netmask, plus the legacy WAN unit IP getter. Backend errors could be collapsed to empty strings; LAN IP then fell back to `0.0.0.0`, and firewall generation could continue with unknown network inputs.
- THIS FIX reads all four settings with checked APIs and copies shared-cache values immediately. Read/close failures and truncation abort before rp_filter writes and any firewall table generation; missing keys preserve empty-string behavior, including the established invalid LAN address fallback.
- Added strict checked-string policy cases and a dedicated source-order checker integrated into the firewall startup Actions. Exact-head workflow validation pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed on unreadable USB printer service configuration

- Parent main SHA: `e29431f47e51c48bcbe0e8c4cf1adec51b55f36b`. Exact e294 IAPP #45004292701, Process umbrella #45004292730, Process inventory #45004292778 and AI docs #45004292741 PASS; XY-C1 #45004292784 was building, so no same-SHA L3 claim.
- `start_u2ec()`, `start_lpd()` and `start_p910nd()` used lossy `nvram_match()` reads. A backend failure could look like a missing/default setting and start a daemon despite unknown enable/raw mode.
- This fix reads each value with `nvram_get_checked()` and copies the shared-cache value immediately. Missing keys retain the empty legacy default; backend errors, invalid result states, NULL values and overlong data abort before PID-file unlink or daemon launch. The raw spooler reads `rawd_enable` once.
- Added GNU89/ASan/UBSan tests, source-order checks, dedicated USB workflow coverage and XY-C1 path filters. Exact-head verification pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Correct IAPP firewall reconciliation command and verify ACCEPT target

- Parent main SHA: `117ae6e88894827d7340cd2f53d10b5b569c86b9`. Exact 117 AI docs #45004170577 and Process inventory #45004170607 PASS; IAPP lifecycle #45004170579 failed because the appended iptables command was split after a double-backslash, causing `-m` to execute as a command.
- THIS CORRECTION places the append on one line, fixes printf's newline format and requires the saved rule to include `-j ACCEPT`, not merely an owner tag/protocol/port. A new test rejects a tagged DROP rule without adding a duplicate. The pre-existing-daemon recovery and fail-reporting tests remain.
- Exact 117 XY-C1 #45004170597 and Process umbrella #45004170630 were running at last poll. Corrected-head IAPP/Process/XY checks pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Reconcile IAPP firewall rules on duplicate start

- Parent main SHA: `46a32f2eb17d2bbca8b840ea5fbea30bc76a7731`. Exact-head Process lifecycle #45003857329, Process inventory #45003857277, NVRAM audit #45003857261 and AI docs #45003857267 PASS.
- `iappd.sh start` treated a live PID as sufficient proof of healthy startup and returned before checking the owned TCP/UDP 3517 firewall rules. If a partial rollback or external rule deletion left one rule absent, subsequent starts falsely reported success and never restored connectivity.
- THIS FIX reconciles the two tagged rules on an already-running daemon, appends only a missing rule, verifies the resulting filter inventory, and returns an error when rule inspection or repair fails. Added tests for missing-UDP recovery without daemon relaunch and failed repair without false success.
- The XY-C1 push path filter omitted `trunk/user/shared/src/router_account_nvram_policy.h`; adding it ensures future changes to this linked production header trigger firmware validation. IAPP's source path triggers the exact build on this commit. Current-head firmware validation pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Parse oversized account counts without temporary-buffer truncation

- Parent main SHA: `7c4121b3c2d54e82931374015488d54401b4e784`. Exact 7c Process lifecycle #45003679784 failed at the oversized count case: `"999999999999999999"` exceeded the helper's 16-byte copy buffer, so it returned `ENAMETOOLONG` before the numeric parser could return the expected `ERANGE`.
- THIS FIX consumes the `nvram_get_checked()` shared-cache pointer immediately and parses decimal digits in place. The configured maximum is checked before each multiply/add, so arbitrarily long numeric input fails with `ERANGE` without integer overflow or truncation; malformed characters fail with `EINVAL`, missing keys remain zero.
- Exact 7c AI docs #45003679754, Process inventory #45003679750, HTTPD #45003679789, WAN DHCP #45003679775, DNS forwarder #45003679778 and GPIO #45003679755 PASS. NVRAM audit #45003679838 and XY-C1 #45003679777 were in progress; exact corrected-head Process regression and XY-C1 remain pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Correct account snapshot regression and restore handoff headings

- Parent main SHA: `1ba086afc059a4188884475532011bc27581c5bc`. Exact 1ba Process lifecycle #45003414996 failed in the new unit test because a 5-byte test buffer cannot hold the five-character string `admin` plus its NUL terminator; the helper correctly returned `ENAMETOOLONG`. This follow-up enlarges the test buffer to 6 bytes.
- Static source review also removed the old `sh_num` local left behind by replacing the `nvram_safe_get_int()` loop, and initializes the password snapshot buffer for the non-`force_create` path.
- Exact 1ba AI docs #45003415008 found that the handoff rewrite had removed required section headings. Restored `Current repository state`, `Active task`, `Verification boundary` and `Exact first action` sections; no project-state claims changed except recording the two exact-head failures above.
- Exact 1ba Process inventory #45003414964, HTTPD #45003415121, NVRAM audit #45003414955, WAN DHCP #45003415094, DNS forwarder #45003414947 and GPIO #45003414983 PASS; XY-C1 #45003414994 was still running. Exact correction-SHA CI pending.

---

## 2026-11-22 — Snapshot account NVRAM values before Unix database staging

- Parent main SHA: `229ee26729a5364b7c81ca3b7a4218e72ed000fe`. Exact 229 WAN route #45002519393 and AI handoff #45002519355 PASS. Earlier a5 WAN DHCP #45002087932, Process inventory #45002087966 and AI #45002088016 PASS; XY-C1 #45002087905 completed Build/TRX/artifact on a5, so L3 is VERIFIED only for that exact SHA.
- `nvram_linux.h` documents `nvram_get_checked()` values point into a shared cache and must be copied before another read. `recreate_passwd_unix()` retained root/account pointers across subsequent reads and used `nvram_safe_get*()`, which collapses backend errors into empty/default data and could silently publish an incomplete account list or use an empty password after read failure.
- THIS COMMIT adds an injected checked-copy policy and strict bounded-count parser. It snapshots root username/password and all account entries before opening staged files; missing keys preserve defaults, while backend, overlong and malformed-count failures abort before publication. Duplicate scans use owned snapshots. Added GNU89 sanitizer tests and Process workflow path-filter/source-contract coverage. Exact new-head CI pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Initialize the DHCP policy header binding in the MTU checker

- Parent main SHA: `0a21e8adab28787b58fc795c58d4a75f0c46c3cf`. Exact 874 HTTPD #45002311801 and AI docs #45002311684 PASS; 874 WAN route #45002311669 exposed a checker header-binding issue.
- Commit 0a switched the helper extractor to `DHCP_POLICY` but omitted loading that variable. Exact 0a WAN route #45002417816 was queued at last poll; the first 874 route run had already shown the owning header is `dhcp_lease_policy.h`.
- THIS COMMIT adds the missing `DHCP_POLICY = (RC / "dhcp_lease_policy.h").read_text()` binding. No production source change; exact-head route/AI reruns pending. a5 XY-C1 #45002087905 remained in Build at last detailed poll; no L3/L4 claim.

---

## 2026-11-22 — Load the DHCP option parser contract from its owning header

- Parent main SHA: `874278785995f29628f39cad26d03e6d0054758c`. Exact 874 HTTPD #45002311801 and AI handoff #45002311684 PASS.
- Exact 874 WAN route #45002311669 failed after the prior source-contract update: `check_wan_initial_mtu_status.py` called its function extractor on `wan_mtu_setup_policy.h`, but `dhcp_lease_mtu_option_value()` is defined in `dhcp_lease_policy.h`. The production parser and callback source were unchanged; this was a test-checker file-binding error.
- THIS COMMIT loads the owning DHCP policy header and extracts the parser there. No product-source change. Exact WAN route rerun pending; a5 XY-C1 #45002087905 was still building at last poll. No L3/L4 claim.

---

## 2026-11-22 — Align DHCP MTU source contracts with the shared parser

- Parent main SHA: `a5d5bb332a7b18db1676f8e182a7ddaae92bd2ff`. Exact a5 WAN DHCP callback #45002087932, Process inventory #45002087966, AI docs #45002088016, detect-link #45002088042, router-mode NVRAM #45002087993, ZeroTier #45002087815, VPN server #45002087979, miniupnpd #45002087848, DNSMasq #45002087957, watchdog #45002087992, WAN teardown #45002087970, Wireless #45002087961, USB modem #45002087840, IPv6 service type #45002087857, WAN ARP #45002087845 and detect-internet #45002087952 PASS.
- Exact a5 HTTPD #45002087808 failed only at embedded checker line 214; WAN route #45002087933 failed in the initial-MTU checker. Both still expected the 576..1500 parser call inline in `udhcpc_bound()`, while a5 had deliberately centralized it in `dhcp_lease_mtu_option_value()`. The policy regression and the WAN option-withdrawal checker both passed.
- THIS COMMIT updates these source contracts to verify the shared parser and its 576..1500 bounds; no production C behavior changes. Exact-head HTTPD/WAN route reruns pending; XY-C1 #45002087905, Firewall #45002087962 and NVRAM audit #45002087864 were still running at last poll. No L3/L4 claim.

---

## 2026-11-22 — Restore default MTU when DHCP option 26 is withdrawn

- Parent main SHA: `12f62705798d1c639c4c0d6b542f82a33fc8ed64`. Exact e72 PID1 mdev, HTTPD, Process lifecycle/inventory, NVRAM, USB storage, DNS forwarder, PID1 notification, DNSMasq and AI runs PASS; XY-C1 #45001383211 was still building at last detailed poll.
- BusyBox `fill_envp()` exports a known DHCP option only when present. Physical WAN and IPTV callbacks previously applied and compared MTU only when `getenv("mtu")` was non-NULL, so withdrawing option 26 retained the previous reduced interface MTU and stale NVRAM lease state.
- THIS COMMIT maps an absent option to 1500, always includes physical/IPTV MTU in the accepted lease snapshot, and makes both renew fast paths re-enter bound when the effective MTU differs. Empty previous snapshots mean the interface's established default 1500; malformed present options remain unsupported. Added GNU89 policy coverage and a dedicated source-order workflow contract. Exact new-head CI pending; no L3/L4 claim.

---

## 2026-11-21 23:56 PST — Harden errno-less service waits and execute the existing regression in CI

- Parent main 0170c3948ac0178250ad8508485159b2adc7e9d8; PID1 inherited-signal-reset production source is in e72 `e72a377025dbd4e42cbbfe0186689b5e6e57720b`. The later 0170 commit updated only handoff/dev-log and did not change source.
- Exact e72 L2 PASS: Process lifecycle #45001383698, USB #45001383155, PID1 notification #45001383190, DNS forwarder #45001383158, Process inventory #45001383203, NVRAM #45001383196, DNSMasq #45001383159, HTTPD #45001383163, PID1 mdev #45001383227 and AI #45001383224.
- Exact e72 XY-C1 #45001383211: production-input detector PASS; Build XY-C1 IN_PROGRESS; TRX and artifact PENDING. Not yet L3.
- `service_wait_timespec_with()` used to return -1 with errno=0 when an injected nanosleep provider failed without setting errno; it could also treat an invalid positive return as an EINTR if errno matched. Unexpected status/errno-less failure now returns EIO while legitimate errno is preserved. Added fake-provider regression.
- `service_wait_policy.c` was present in Process lifecycle workflow path filters but had no compile/run step. Added explicit GNU89 `-Wall -Wextra -Werror` + ASan/UBSan compile/run to the exact-head process lifecycle job. L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed if the recovery shell cannot reset inherited PID 1 signals

- Parent main e72a377025dbd4e42cbbfe0186689b5e6e57720b. Exact e72 XY-C1 #45001383211, Process lifecycle #45001383698, USB #45001383155, PID1 notification #45001383190, DNS forwarder #45001383158, Process inventory #45001383203, NVRAM #45001383196, DNSMasq #45001383159, HTTPD #45001383163 and PID1 mdev #45001383227 were in progress at last poll; AI docs #45001383224 PASS.
- Parent 1a5 HTTPD #45001003234 had failed at an embedded-checker NameError because pid1_test was not loaded; parent 0b22 fixed the missing test-source binding.
- Exact 1a5 XY-C1 #45001003215 was host-only: Build, TRX and artifact steps were skipped because no production build inputs changed. Not L3.
- reset_signals() previously ignored disposition-reset sigaction and signal-unblock failures, and looped over invalid signal numbers. It now resets only PID1's fatal/shutdown/event groups via the checked sigaction helper, retains the SIGCHLD reaper, returns the first error, checks sigfillset and unblocks through the checked mask helper.
- Forked console child now logs and _exit(126)s before console initialization/exec if reset fails. Fatal emergency shutdown reports reset failure and continues best-effort. mdev and HTTPD source contracts verify the signal groups, mask unblocking and child fail-closed behavior. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed if the recovery shell cannot reset inherited PID 1 signals

- Parent main 0b22c9729e6abce02181e96c7a60560342db2b29. Exact 1a5 Process inventory #45001003203, PID1 mdev #45001003224, AI #45001003376 and NVRAM #45001003186 PASS. HTTPD #45001003234 failed only because its embedded source contract referenced pid1_test without loading the C test file; the binding fix is in parent 0b22. Parent 0b22 AI #45001261547 PASS; HTTPD #45001261606 was in progress.
- Exact XY-C1 #45001003215 was host-only: production input detector found no build input changes; Build, TRX and artifact were skipped. Not L3.
- reset_signals() previously ignored disposition-reset sigaction and signal-unblock failures, and looped over invalid signal numbers. It now resets only PID1's fatal/shutdown/event groups via the checked sigaction helper, retains the SIGCHLD reaper, returns the first error, checks sigfillset, and unblocks through the checked mask helper.
- Forked console child now logs and _exit(126)s before console initialization/exec if reset fails. Fatal emergency shutdown reports reset failure and continues best-effort. mdev and HTTPD source contracts verify all three groups, mask unblocking and child fail-closed behavior. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 16:04 CST — Load PID 1 test source in HTTPD contract

- Parent main SHA: `1a5bb1367ba92b39f46a89b7172b9f816cc19f56`. Exact PID1 mdev #45001003224, Process lifecycle inventory #45001003203 and AI handoff #45001003376 PASS.
- Exact HTTPD #45001003234 passed its GNU89/Werror PID1 policy executable and all prior checks, then failed in the embedded Python source contract with `NameError: name 'pid1_test' is not defined`. The assertion referenced the test file but this HTTPD step loaded only `init.c` and `pid1_signal_policy.h`; the parallel mdev contract already loaded the test file explicitly.
- THIS COMMIT adds the missing `Path("trunk/user/rc/src/tests/pid1_signal_policy.c").read_text()` binding. No product C behavior changed. NVRAM #45001003186 and XY-C1 #45001003215 were IN_PROGRESS at last poll; exact new-head Actions required. No L3/L4 claim.

---

## 2026-11-21 23:51 PST — Correct errno matcher in PID 1 mask contracts

- Parent main `80ad0e36e6c2ebca9dd2347f2dee484ee50915f6`. Exact 80ad Process lifecycle #45000794677 and AI docs #45000794731 PASS; NVRAM #45000794652 was still running. PID1 mdev #45000794651 and HTTPD #45000794708 failed in their newly added source contracts, not in the checked mask logic: the raw helper line is `errno = 0;`, while our matcher expected `errno=0;`. HTTPD's GNU89 C policy regression compiled and executed before the Python contract failed.
- Corrected the whitespace-sensitive matcher in both workflows. Strengthened the test to assert the adapter forwards a SIGALRM-only set on the optional-old-mask SIG_UNBLOCK path, while clearing stale EINTR.
- No production runtime change in this correction. Exact reruns are triggered by the C test path change; no L3/L4 claim.

---

## 2026-11-21 23:45 PST — Cover optional old-mask output in PID 1 policy regression

- Parent main `c9c47f065a86645dabb8155d133b07e215d3b771`. Exact 34d Process inventory #45000638051, Process lifecycle #45000637980, DNSMasq #45000637989, DNS Forwarder #45000637987, AI #45000637929 and PID1 notification #45000638026 PASS. NVRAM #45000638001, XY-C1 #45000637923 and USB #45000637930 were still running.
- 34d HTTPD #45000637963 and PID1 mdev #45000637950 failed in their new signal-mask matcher: control_signal() calls init_signal_mask_update(), which delegates to the policy helper. c9 corrects both matchers to the actual call chain.
- Extended the C regression with a successful SIG_UNBLOCK call passing old_set == NULL, asserting the helper accepts the optional output, records the requested operation, and clears caller errno. Updating the watched C test path ensures corrected mdev/HTTPD contracts rerun on one exact SHA. No L3/L4 claim.

---

## 2026-11-21 23:41 PST — Correct PID 1 signal-mask workflow matcher

- Parent main `34d7b3cb77426c41e077e47c85da81b0441560a4`. The signal-mask change was committed with mdev and HTTPD inline contracts. Review found one test-only mismatch: control_signal() calls `init_signal_mask_update()`, while that wrapper calls `pid1_signal_update_mask()`; the new assertion incorrectly expected the lower-level call in control_signal().
- Corrected both inline workflows to verify the actual wrapper call chain. No production source changes.
- Exact deaca PID1 mdev #45000188630, HTTPD #45000188641, Process inventory #45000188627, AI #45000188637 and NVRAM #45000188665 PASS. deaca XY-C1 #45000188681 was host-only (production Build/TRX/artifact skipped), not L3. The 34d exact-head workflows were queued at first poll; this test correction now triggers fresh runs.

---

## 2026-11-21 23:37 PST — Check PID 1 signal masks and avoid invalid sigsuspend state

- Parent main `deaca535575f3032b02c53a52667f8eb74494fe2`. Exact deaca PID1 mdev #45000188630, HTTPD #45000188641, Process lifecycle inventory #45000188627 and AI handoff #45000188637 PASS. Exact deaca XY-C1 #45000188681 is HOST-ONLY success: production-input detector found no firmware build inputs changed, so Build, TRX and artifact steps were SKIPPED; not L3. Exact f6a NVRAM #44999987551 PASS; deaca NVRAM #45000188665 was running at last poll.
- PID 1 main loop ignored queued-work sigemptyset/sigaddset/sigprocmask failures, could pass an uninitialized original_mask to sigsuspend, and ignored console-shell mask restore/reblock failures. The two early control_signal mask changes were also unchecked.
- Added an injectable pid1_signal_update_mask() returning errno or EIO, checked control-signal masks and router-readiness gates, mask-construction status, original-mask validity, checked block/restore/reblock transitions and a bounded polling fallback that never suspends with invalid state. Failed SIGUSR1 unblock disables router notifications. Added GNU89 regression coverage for null args, stale errno clearing on success, old-mask publication, EBUSY and errno-less EIO; expanded mdev and HTTPD source contracts. Exact-head L2 reruns pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 18:31 CST — Cover successful signal installer errno reset

- Parent main `52e6692cd806abdb4c52ae2fd42271a3f085aed9`. The 8a66 HTTPD run #44999904303 passed all preceding tests and failed on the old direct-`sigaction` expectation. The checker update in 52e has not yet been exercised because its commit touched only the workflow and documentation.
- Extended `pid1_signal_policy.c` to seed `errno=EINTR` before a successful fake sigaction and assert that the checked wrapper clears it. Updated the mdev source contract to require the new regression. Updating the watched C test forces both mdev and HTTPD exact-head tests to rerun.
- Exact 8a66 PID1 mdev #44999904281, Process inventory #44999904395, Process lifecycle #44999904325 and AI handoff #44999904265 PASS. f6a NVRAM #44999987551 remained in progress. No L3/L4 claim.

---

## 2026-11-22 18:23 CST — Align HTTPD's PID 1 signal contract with checked installation

- Parent main `f6a3c1ca6bfa0ee3a8b4b76c37d1d4dc4eefa9c4`. Exact 8a66 HTTPD #44999904303 passed NVRAM rollback, wireless atomic publication/preflight, route access, Samba/Unix identity, and other earlier contracts; it failed only in the final PID1 signal section because it still expected direct `sigaction(signals_shutdown[i], ...)` instead of the checked installer introduced in c74.
- Replaced the legacy assertion with checks that fatal, shutdown, SIGCHLD and event handlers all flow through `init_signal_install_status()`, and that the checked wrapper calls `pid1_signal_install_action()`.
- Exact 8a66 PID1 mdev #44999904281, Process inventory #44999904395, Process lifecycle #44999904325 and AI docs #44999904265 PASS. NVRAM on 8a66 #44999904244 was still running; f6a exact NVRAM #44999987551 was queued at first poll. L3/L4 unverified.

---

## 2026-11-22 18:11 CST — Update NVRAM audit for checked PID 1 signal gate

- Parent main `8a66f03a5c0e1d7a73e1ee038c1d8f92be15ad79`. Exact c74 NVRAM audit #44999683896 passed ioctl/close, transaction lock, compaction, snapshot and NVRAM device-node regression steps, then failed only at the final PID 1 source assertion that still expected router startup/readiness without `signal_status`.
- Updated `scripts/check_nvram_transaction_boundary.py` to require both signal-aware router predicates, every checked fatal/shutdown/SIGCHLD/event installation, the first-error helper and suppression of deferred mdev retries on incomplete signal setup. Production sources and C tests unchanged.
- Exact 8a66 PID1 mdev #44999904281 and AI docs #44999904265 PASS. HTTPD #44999904303, Process inventory #44999904395, Process lifecycle #44999904325 and NVRAM #44999904244 were still running at first poll. No L3/L4 claim.

---

## 2026-11-22 18:02 CST — Correct PID 1 signal regression and shared boot checks

- Parent main: `c74ba42e7bab3eba8d89b4931873bce37a83cba5`. Exact PID1 mdev workflow #44999683905 failed because policy functions were used before `pid1_signal_policy.h` appeared in the test translation unit. Exact HTTPD #44999683880 failed its inline router-start/readiness contract because it did not include the new signal-status prerequisite. Exact Process inventory #44999683934 failed the same old conditions in `scripts/check_dev_init_boot.py`.
- Moved the policy header above all test references. Updated the shared Process boot checker and HTTPD's inline contract to require signal setup success before mdev deferred retry or router startup; both now pin all four signal handler groups, first-error propagation and the signal gate. The HTTPD workflow also watches the shared checker path explicitly.
- Exact c74 Process lifecycle #44999683951, USB storage #44999683921, DNSMasq #44999683891, DNS Forwarder #44999684058 and AI docs #44999683893 PASS. XY-C1 #44999684046 and NVRAM #44999683896 were still running at first poll; PID1, HTTPD and Process inventory require exact-head reruns. No L3/L4 claim.

---

## 2026-11-22 17:41 CST — Fail closed when PID 1 cannot install signal handlers

- Parent main SHA: `9a3b2a3f60ae9da088e5de17cda252172b418067`. Networkmap ARP receive source parent 2cd6cbb has three GNU89 sanitizer-backed C regressions PASS; exact checker correction on 9a3 is awaiting rerun #44999591254 with AI docs #44999591228. Exact 2cd Process inventory #44999477626, NVRAM #44999477587 and AI docs #44999477625 PASS; no same-SHA XY-C1 proof for 2cd.
- `init_signals()` previously ignored every `sigaction()` result for fatal, shutdown, SIGCHLD-reaping and event handlers. A failed installation could leave PID 1 running router services without required signal-driven shutdown, notification or child-reaping behavior.
- Added an injectable checked sigaction wrapper that returns the actual errno (or EIO if libc fails without errno), records first-error status while still attempting independent handlers, and logs each failed signal. PID 1 now suppresses deferred mdev alarms and skips router startup when handler installation is incomplete, while keeping the recovery event loop available.
- Added GNU89 policy coverage for successful installation, invalid inputs, EACCES, errno-less EIO and sticky first-error behavior; expanded the existing mdev/boot-device source contract. Exact-head CI and XY-C1 Build/TRX/artifact are pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 17:24 CST — Update networkmap main-loop contract for propagated status

- Parent main SHA: `2cd6cbb65590408bd14f2843f3187d3ed2d3010f`. Exact networkmap workflow #44999477629 compiled and passed the signal policy, daemon policy and new ARP receive error policy C executables; it failed only at `scripts/check_networkmap_signals.py:134` because an older source assertion still searched for the former standalone `nmap_iterate();` statement.
- Main now invokes `nmap_iterate() < 0` in its checked error gate so fatal receive status reaches cleanup and exit; corrected the source matcher to track that expression. Product source and C tests are unchanged.
- Exact parent Process lifecycle #44999477626 and AI handoff #44999477625 were still running at first poll; new WAN-related NVRAM audit #44999477587 was also running. Rerun the complete networkmap checker on this new SHA.

---

## 2026-11-22 17:12 CST — Stop networkmap ARP receive errors from spinning the daemon loop

- Parent main SHA: `ef3b22d28619427c3e5823b0888c2ea390405b76`. Exact networkmap signal lifecycle #44999104161 PASS and AI handoff #44999104151 PASS. Checked WAN WISP/manual-DHCP/transport flow passed exact WAN route #44998743083 on b809 and AI handoff #44998743195 PASS. XY-C1 #44998650971 was still IN_PROGRESS at last observed poll; it is not same-SHA L3 evidence.
- Found a networkmap runtime fault: `recvfrom()` failures were treated like short packets, so persistent errors such as `ENODEV`/`ENETDOWN` could make the outer loop call `recvfrom()` continuously. Malformed short frames also terminated the receive batch.
- Added `networkmap_arp_receive_error_is_retryable()` to distinguish expected `EINTR` and `SO_RCVTIMEO` `EAGAIN/EWOULDBLOCK` from fatal socket errors. The receive loop now preserves fatal errno and returns it after flushing already-collected client updates; short frames are discarded while draining, with every datagram counted against the 1024-frame storm cap.
- Propagated receive status through `nmap_iterate()`; main now logs and exits through `clear_resources()` on fatal receive errors rather than hot-looping. Added GNU89/Werror/ASan/UBSan unit coverage and source-order contracts in the existing networkmap workflow. Exact-head Actions pending; no target build claimed.

---

## 2026-11-22 16:55 CST — Correct networkmap signal setup source-order assertion

- Parent main SHA: 44a2a851ac6732dd59ee171a99328a0d5cc7299f. Exact Networkmap signal lifecycle #44999008557 FAILED at scripts/check_networkmap_signals.py:121; both the GNU89 signal-policy executable and networkmap daemon-policy regression passed. The new checker compared the installer-call offset (inside the if expression) as if it preceded the if keyword; the order was reversed.
- Production installer, five checked sigaction calls, and cleanup/return path compiled and passed the C tests. Corrected the source-order assertion to check if-condition → installer call → cleanup → return → daemonize order. No product source change.
- Exact 44a2 NVRAM audit #44999008558, process lifecycle #44999008622 and AI handoff #44999008560 were still in progress at last poll. Exact b809 WAN route #44998743083 and AI handoff #44998743195 PASS.

---

## 2026-11-22 16:43 CST — Make networkmap signal disposition setup fail closed

- Parent main SHA: `b809d95f35efebeb7e653feb2f96e1a0c9c8fb69`. Exact WAN route #44998743083 and AI handoff #44998743195 were in progress at last poll; previous exact WAN route #44998472638 and AI handoff #44998472600 PASS. The WISP and checked DHCP selector/transport changes await the combined WAN rerun; no L3 claim.
- Cross-userland stability review found networkmap's five `signal()` registrations (SIGPIPE/HUP/USR2 ignore and SIGUSR1/TERM event handlers) ignored all setup return values. If registration fails, the daemon can run without required shutdown/refresh/pipe protections; a later SIGTERM can terminate it without `clear_resources()`.
- Added `networkmap_signal_install_handlers()` using checked `sigaction()` for all dispositions. Main checks the installer before daemonization and on failure logs, releases socket/client resources and returns the errno status. Signal handlers remain async-signal-safe flag writers.
- Extended the GNU89 policy regression to install/query all five dispositions and reject null handlers, plus source contracts for installer error returns and main's cleanup-before-daemonize failure path. Exact workflow run pending; no local target or firmware build claimed.

---

## 2026-11-22 16:27 CST — Match compacted WAN callback diagnostics in source checker

- Parent main SHA: `fe1533a05beed4d2a9f3a40b7945a2cc25cc6d20`. Exact WAN route #44998650856 FAILED in `scripts/check_wan_dns_read_status.py:1039`. All C policy and earlier WAN source tests passed; the final assertion searched for spaced diagnostic strings after compacting the entire C function with `"".join(...split())`. The source literals were therefore correctly compacted too, so only the Python matcher was wrong.
- The checked `ifname_t` implementation itself is present and returns errors before USB/WISP classification or DHCP renew. Corrected both new diagnostic expectations to their compact literal forms; no product source change.
- Exact parent HTTPD #44998651042 PASS, firewall #44998650885 PASS, WAN DHCP callback #44998650941 PASS, AI handoff #44998650835 PASS. XY-C1 #44998650971 remained IN_PROGRESS at last poll; no same-SHA L3 claim. WAN route rerun pending.

---

## 2026-11-22 16:18 CST — Fail closed on unreadable WAN transport identity before DHCP renew

- Parent main SHA: `103281fb037a380dfa177edb6cf3b7c5acd7582f`. Exact parent WAN route #44998542768, AI handoff #44998542701 and XY-C1 #44998542772 were IN_PROGRESS at last poll; NVRAM audit #44998542802 and HTTPD #44998542654 were also running. Previous exact 59fa source/checker parent: WAN route #44998472638 PASS and AI handoff #44998472600 PASS, confirming WISP identity and matcher correction.
- Continued the link-restored DHCP state transition review and found `get_wan_unit_value(unit, "ifname_t")` was still a lossy safe_get read. A backend/close error could be treated as a non-USB transport and let the callback proceed without knowing the configured transport identity.
- Replaced it with bounded `get_wan_unit_value_copy_status()`; backend/close/size errors are returned before USB-net classification, WISP classification and DHCP signal. A genuinely missing/empty legacy key remains a non-USB interface, as before.
- Extended the WAN source contract to require checked transport read → error gate → USB classification → WISP read → DHCP renew, and forbid the old lossy getter. This is a source/checker slice only; exact-head Actions and firmware build remain pending.

---

## 2026-11-22 16:07 CST — Check the manual DHCP-client selector before link-restored renew

- Parent main SHA: `59fa3e95dc9cb7228837afda278023c3825aa0b4`. The prior WAN route checker variable-binding fix is now being validated by exact run #44998472638; its AI handoff run #44998472600 is also in progress. Exact b436 AI handoff #44998355100 and HTTPD Authentication/Security #44998355033 passed; b436 WAN route #44998355043 had failed only on the subsequently corrected undefined test variable.
- Found another real fail-open control read in `notify_on_wan_ether_link_restored()`: `nvram_match("x_DHCPClient", "0")` hides NVRAM backend/close errors. That allows an Ethernet link-restored event to reach DHCP renewal without knowing whether the manual DHCP-client mode is enabled.
- Replaced the boolean lookup with `get_nvram_match_checked_status()`, preserving missing-key false compatibility. Static-IP protocol still short-circuits first. A read error is logged and returned before USB/WISP checks or DHCP signalling; a confirmed disabled manual client still suppresses renew.
- Added a source-order regression ensuring the checked selector read/error gate precedes DHCP renewal and forbidding the old lossy `nvram_match()` predicate. The shared WAN NVRAM policy executable already covers checked string-match error, missing-key and output semantics. Exact-head route run pending; no target firmware build or L3 claim.

---

## 2026-11-22 15:55 CST — Fix WAN checker regression-test variable binding

- Parent main SHA: `b4366cb27be501905e3c5c7a3a73e8e2e5f9302d`. Exact b436 WAN route #44998355043 failed at `scripts/check_wan_dns_read_status.py:1030` with `NameError: nvram_policy_test is not defined`. The test source is already loaded as `test`; WISP policy compile and execution, existing WAN DNS checks and earlier route source contracts all passed. This is a checker-only variable-name defect, not a WISP product-source failure.
- Exact b436 HTTPD Authentication/Security #44998355033 PASS; AI handoff #44998355100 PASS. The earlier HTTPD stale firewall assertion is resolved.
- Rebind the new WISP unit-test presence/registration assertions to the existing `test` variable. The WISP C implementation is unchanged. Exact rerun must pass before the WISP slice is called host-verified; no same-SHA L3 claim.

---

## 2026-11-22 15:47 CST — Align WISP and HTTPD firewall source contracts

- Parent main SHA: `a41fdc4d1bf6aef69e5242aa26712c56fe2b56f7`. Exact WAN route #44998118160 FAILED in `scripts/check_wan_dns_read_status.py:988`: the new WISP source matcher collapsed runs of whitespace but retained a single separator after a wrapped function-call opening parenthesis. The WISP policy C executable compiled and ran successfully before the Python checker step.
- Exact HTTPD Authentication/Security #44998118167 FAILED in its route-access checker at embedded Python line 863 (`.github/workflows/httpd-security.yml`): it still required the removed `man_if_value`/`get_man_ifname()` firewall code after the checked MAN-interface fix landed on parent `5a86cca`. All earlier HTTPD steps in that job passed.
- Corrected both contracts without changing product behavior: WISP matching now strips all source whitespace before order assertions; HTTPD firewall assertions now require checked MAN read, error gate, safe-token validation and validation-before-raw-rules, and reject the lossy getter.
- The exact-head WISP source change itself is not yet host-verified because the route checker stopped on this assertion. Exact parent XY-C1 #44998117945 and AI docs #44998118765 were still in progress at last poll; no L3 claim.

---

## 2026-11-22 15:31 CST — Preserve WAN WISP identity read failures

- Parent main SHA: `5a86cca27cb9634a1a0c13375a58518c848b309e`. Its firewall checked-MAN-interface fix is validated by exact Firewall Startup run #44997818203 PASS; AI handoff #44997818219 and Process inventory #44997818261 PASS. WAN route #44997818209, NVRAM audit #44997818217, FULLCONENAT #44997818224, HTTPD security #44997818264 and XY-C1 #44997818248 were still in progress at the last check; no L3 claim.
- Found two consumers of `get_wan_wisp_active()` that could misclassify an unreadable `wan0_ifname` as non-WISP because the getter calls `get_man_ifname()`/safe_get: WAN link-restored DHCP renewal could run when the WISP interface is active, and WAN LED logic could fall back to cached Ethernet link state.
- Added `get_wan_wisp_active_status()`: it preserves checked NVRAM read/close status, validates a nonempty interface token before classification, distinguishes a genuinely empty legacy interface, and only queries AP-client link state after successful classification. The old boolean ABI remains as a conservative wrapper (unknown identity returns active).
- Updated the WAN LED path to turn the link indication off on unknown MAN identity without consulting cached Ethernet link state; updated link-restored handling to propagate the read error and suppress DHCP renewal. Added errno/validation precedence unit coverage and source-order contracts in the existing WAN route regression.
- Tests are wired into the WAN route workflow (GNU89/Werror/ASan/UBSan C policy test and the WAN DNS/source checker); no local target build is claimed. Exact-head Actions for THIS COMMIT are pending at creation time. Firmware/TRX and hardware verification remain NOT VERIFIED.

---

## 2026-11-22 — Fail closed when firewall cannot read its management interface

- Parent main SHA: `b9964bf12272c2f18d5fbc92f1fd9a155c6f4fe5`. Exact b996 AI handoff #44997591604 PASS after lossless `dev_log.md` rotation. Exact a3 WAN route #44997377580, DHCP #44997377452, teardown #44997377521, wireless #44997377474, Process lifecycle #44997377507, Process inventory #44997377633 and NVRAM transaction #44997377584 PASS; XY-C1 #44997377413 remains in Build XY-C1.
- Confirmed a P1 firewall fail-open input-read path in `start_firewall_ex_locked()`: `get_man_ifname()` uses `nvram_safe_get()`, which collapses NVRAM read/backend/close errors into an empty interface. The empty value was accepted, then used to build interface-dependent firewall/rp_filter policy.
- Replaced that getter with checked bounded `get_wan_unit_value_copy_status(unit, "ifname", man_if, sizeof(man_if))`. Real errors now set `errno` and return before firewall/rp_filter rules are applied; nonempty values keep the existing safe-token check; an actually empty legacy value remains compatible.
- Extended `check_wan_firewall_status.py` to pin checked read/error/validation before the first per-interface rp_filter write and forbid the lossy getter in `start_firewall_ex_locked()`. Exact-head firewall Actions are pending.

---
## 2026-11-22 — Rotate the rolling development log and preserve full-restart verification

- Root `dev_log.md` had 121 complete records. Archived the 101 oldest records byte-for-byte in chronological order in `docs/dev-log/2026/11/part-025.md`; retained the 20 newest original records at the root, then prepended this rotation checkpoint. Archive SHA: `daf3875420e99758082b0d43ab2e4b4b2cd57ab2`; pre-rotation root blob SHA: `418727893b781c626ffa7995bc378ac8b5cff66f`.
- Exact `a3b7e95b71776808fbb6e6bddf273db8cf0c98f4` WAN route #44997377580, WAN DHCP #44997377452, WAN interface teardown #44997377521, wireless radio #44997377474, Process lifecycle #44997377507 and Process inventory #44997377633 PASS. AI handoff #44997377383 failed only because the root log reached 121 entries; this lossless archive rotation restores the required rolling window.
- Exact a3 XY-C1 #44997377413 was still executing Build XY-C1 at last poll; TRX verification and artifact upload had not completed. NVRAM transaction audit #44997377584 and several independent suites were still running. No L3 claim.
- The same a3 source fix validates and preserves `wan0_ifname` before `full_restart_wan()` changes router mode or stops WAN, then reuses the checked local name for route flush. Hardware L4 remains unverified.

---
## 2026-11-22 — Validate full-restart route interface before WAN teardown

- Parent main SHA: `c69d200c788aafeb76c6905bb18a70d22f9d49d4`. In the prior 86d run, wireless radio #44997181767 and Process lifecycle #44997181794 failed only because the source checker called undefined `require()` at line 323; the wireless policy executables passed. Follow-up c69 changed that call to Python `assert`.
- Found another concrete state-transition read-loss path: `full_restart_wan()` fetched `route_ifnames[0]` via `get_man_ifname(0)` only after `stop_wan()` completed. A transient NVRAM read error could appear as an empty string after services had already been stopped, then route flush would operate on the wrong interface set.
- Moved checked `get_wan_unit_value_copy_status(0, "ifname", ...)` and `wan_ifname_copy()` validation before `update_router_mode()` and `stop_wan()`. The validated bounded local name is reused for route flush; real errors are logged, copied to `errno`, and returned before WAN side effects. Empty legacy configuration remains an empty string.
- Extended the WAN DNS/read checker to pin checked read/error/validation before mode update and service stop, then route use of the same validated buffer, with no `get_man_ifname(0)` left in full restart.
- Exact c69 wireless radio #44997296073, Process inventory #44997296036 and AI docs #44997296148 were in progress at last poll; no same-SHA XY-C1 build claim. Exact-head validation for this product change remains pending.

---
## 2026-11-22 — Fix AP-client radio source-checker assertion helper

- Parent main SHA: `86d18d524ca522dc4d85e5abf7c28376ac74a394`. Exact wireless-radio #44997181767 failed at line 323 with `NameError: name 'require' is not defined`; the newly added checker used a helper absent from this script, which consistently uses Python `assert`.
- Exact Process lifecycle #44997181794 also failed at the same imported guest-LAN checker assertion after its independent account bootstrap and bridge policy tests passed. The wireless-radio, bridge-membership and guest-isolation C policy executables passed before the Python checker.
- Replaced only the undefined `require(condition, message)` call with Python `assert (condition), (message)`, retaining all ordering, empty-configuration and errno assertions. No product source changed.
- Exact 86d AI handoff #44997181801, process inventory #44997181843, HTTPD Auth/Security #44997181802 and iNIC lifecycle #44997181809 PASS. XY-C1 #44997181783 and NVRAM transaction #44997181875 were still in progress at last poll.

---
## 2026-11-22 — Check AP-client radio recovery inputs before WAN restart decisions

- Parent main SHA: `0292b0a44ece35cac3e0331ab04aa15028a671a3`. Exact WAN route #44996976228 PASS, DHCP #44996976338 PASS, WAN teardown #44996976388 PASS and AI handoff #44996976327 PASS. Exact XY-C1 #44996976282 was still building.
- `check_apcli_wan()` previously used `get_ap_mode()` and `get_man_ifname()` (safe_get-backed) to decide whether an AP-client radio restart should trigger full WAN restart or reconnect. A transient NVRAM error could masquerade as AP mode false/empty manual interface and produce an incorrect recovery decision.
- Switched to `get_ap_mode_status()` and checked `get_wan_unit_value_copy_status()`; nonempty manual interface is validated by `wan_ifname_copy()`. Any read/token failure returns before restart/reconnect decision, with errno preserved. Empty legacy manual config continues to map to no APCLI MAN interface.
- Extended `check_guest_lan_isolation_status.py` to pin the checked mode/manual input order and `wan_ifname_policy.h` dependency. Exact-head wireless/WAN/XY Actions remain pending for this source change.

---
## 2026-11-22 — Guard AP-client WISP migration with checked prior WAN identity

- Parent main SHA: `2149789b0636dd794fcb7eeb5343e314045cdc9a`. Exact 214 WAN route #44996754354 PASS, WAN DHCP #44996754350 PASS, WAN interface teardown #44996754355 PASS and AI handoff #44996754406 PASS; XY-C1 #44996754358 remained in Build XY-C1.
- `config_apcli_wisp()` previously fetched old manual WAN interface via `get_man_ifname()` (safe_get-backed); a transient NVRAM error could look empty, skip clearing the prior WAN address/interface, then continue enabling/publishing the AP-client WISP interface.
- Added checked bounded `ifname` read and `wan_ifname_copy()` validation before old-interface cleanup, IPv6 enablement and NVRAM publication. Actual errors are returned with `errno`; empty legacy configuration remains an empty string.
- Extended the WAN source contract to assert read/error/validation ordering before ifconfig/IPv6/publication and forbid the lossy getter. Exact-head Actions for this additional source change remain pending.

---
## 2026-11-22 — Fail closed before WAN teardown when manual interface is unreadable

- Parent main SHA: `59cdab3123a8d6cce5dae6c5f5fe347d9d9bee67`. Exact WAN route #44996570973 PASS and AI handoff #44996570985 PASS; exact d018 WAN DHCP #44996488482 PASS.
- Reviewed `stop_wan_locked()`: it previously read `man_ifname = get_man_ifname(unit)` only after pausing Internet monitoring, invalidating PPP callback sessions and cancelling deferred reconnect. Since the getter is `nvram_safe_get()`-backed, a NVRAM read/backend error could be silently converted to an empty name while teardown proceeded.
- Moved a checked `get_wan_unit_value_copy_status(..., "ifname", ...)` and `wan_ifname_copy()` validation ahead of monitor pause and all teardown side effects. The real read/validation error is logged, copied to `errno`, and returned. An actually empty legacy/unconfigured value is preserved as an empty string rather than rejected; removed the later lossy getter.
- Added a source contract to pin checked read → read-error gate → nonempty token validation/error gate → empty-key preservation → monitor pause, and forbid the lossy getter in `stop_wan_locked()`.
- Exact-head route/DHCP and XY-C1 Actions are pending for this product source change. No MIPS build/TRX/artifact or hardware test is claimed.

---
## 2026-11-22 — Keep modem identity error contract errno-aware

- Parent main SHA: `d018080e3ef848417c1969ba17adfd0b8a775624`. Exact WAN DHCP #44996488482 PASS. Exact WAN route #44996488531 FAILED at `scripts/check_wan_dns_read_status.py:515`: after the `get_wan_ip4()` checked manual fallback change, the old modem identity assertion still required `if(status!=0)returnINADDR_ANY`.
- The actual code correctly sets `errno=status` before returning `INADDR_ANY` for modem identity failures. The stale assertion had not been updated for this improved error preservation; the checked manual-fallback source contract immediately above it passed.
- Replaced the errno-less pattern with an ordered contract requiring modem status read → `errno=status` failure gate → transport selection. This is test-only; no product behavior change in this follow-up.
- Exact d018 XY-C1 #44996488349 was still building/validating at last poll. L3 remains unverified until Build XY-C1, TRX and artifact all pass on one identical product-source SHA.

---
## 2026-11-22 — Check manual interface only when get_wan_ip4 selects it

- Parent main SHA: `616d5a710acdf727a8f110052289c1d63d966a9d`. Exact WAN DHCP #44996346231 PASS and AI handoff #44996346193 PASS; prior WAN route #44996213814 PASS.
- Found the remaining `nvram_safe_get()` loss path in `get_wan_ip4()`: it read `get_man_ifname()` before modem/PPP overrides. Backend/read/close errors could be mistaken for an empty interface; moreover manual `ifname` was fetched even when a modem/PPP interface won.
- Changed interface selection to check `ifname_t`, modem identity and protocol first, retaining modem/PPP overrides. Only when no override is selected does it now perform checked `get_wan_unit_value_copy_status(..., "ifname", ...)`, validate via `wan_ifname_copy()`, and preserve read/validation failure in `errno` before returning the existing `INADDR_ANY` sentinel.
- Updated `scripts/check_wan_dns_read_status.py` to require manual fallback read/validation after modem/protocol selection and before address lookup, while preserving the cache-overwrite regression.
- Exact-head WAN route/DHCP for this product-source SHA are pending. No MIPS build, TRX, artifact or hardware validation is claimed.

---
## 2026-11-22 — Normalize WAN manual-fallback checker ordering

- Parent main SHA: `ecfdafb0e348bcec466d4372400f5a2c87f02006`. Exact ecfd WAN route #44996213814 PASS; AI handoff docs #44996213622 PASS. WAN DHCP callback #44996213685 failed after its GNU89/ASan/UBSan `wan_ifname_policy.c` executable and basic bounds regression both passed.
- The failing assertion at `scripts/check_wan_ifname_bounds.py:51` compared `manual_ifname_gate_at` from the raw source string with later positions from the whitespace-compacted source. Those offsets are not comparable, so a valid checked manual fallback failed the checker.
- All ordering positions are now measured in the same compacted function. The contract also verifies modem and protocol selection happen before the fallback, and that read error and token-validation error branches both precede the final output copy.
- The exact dc6 product-source XY-C1 run #44996098229 remained in progress at last poll with Build XY-C1 running; TRX and artifact upload were pending. The checker-only ecfd commit did not alter product firmware inputs. L3 remains unverified.

---

## 2026-11-22 — Align WAN DNS checker with checked manual-interface getter

- Parent main SHA: `dc6f6b4efb8b0ac088e82e18f9bd42168efdd9d0`. Exact WAN DHCP callback #44996098158 PASS, including the expanded `check_wan_ifname_bounds.py`; HTTPD Auth/Security #44996098097 and AI handoff #44996098288 PASS.
- Exact WAN route #44996098107 FAILED at `scripts/check_wan_dns_read_status.py:493`. Its contract still required `get_man_ifname()` followed by `rc_wan_nvram_copy_text()` in `get_wan_ifname()`, while the current implementation now selects modem/PPP overrides first and performs checked `get_wan_unit_value_copy_status()` plus `wan_ifname_copy()` only when using the manual interface. The C path itself was not the failing step.
- Replaced that stale assertion with the checked fallback read/validation contract and strengthened `check_wan_ifname_bounds.py` to verify the exact manual `ifname` read occurs inside the fallback branch before its copy and the final public output copy; this avoids accidentally matching the separate earlier `ifname_t` checked read.
- Exact dc6 XY-C1 #44996098229 was still IN_PROGRESS before Build/TRX/artifact at last poll. This follow-up's exact-head checks are still pending; no L3 claim.

---
## 2026-11-22 — Preserve checked interface identity in get_wan_ifname

- Parent main SHA: `2b2c2e6c1ef1ca9fc41dd39ebb88858e33cf1bcb`. Exact 2b WAN DHCP callback #44995928887 PASS, WAN route status #44995928825 PASS and AI handoff #44995928838 PASS. Exact XY-C1 #44995928841 remained IN_PROGRESS at last poll; no L3 claim.
- Continued the same concrete NVRAM failure path beyond `start_wan()`: `get_wan_ifname()` also used `get_man_ifname()` backed by `nvram_safe_get()`, then copied the lossy value through `rc_wan_nvram_copy_text()`. A backend/read/close error could be rewritten as an empty-interface `EINVAL` instead of preserving the actual NVRAM error.
- Removed the lossy getter from `get_wan_ifname()`. The function now performs checked bounded read and token validation only if its selected interface still uses the configured manual name; modem/PPP overrides do not gain an unrelated read dependency. Checked status is returned with `errno` preserved.
- Extended the existing WAN interface source contract to pin that conditional checked read, validation, error propagation and lack of the lossy getter. Exact-head WAN DHCP/route/XY verification is pending for this commit.

---
## 2026-11-22 — Align WAN startup source contracts with checked interface identity

- Parent main SHA: `e34444ae55651085a157996f4c6b5699b20066c9`. Exact 6ec WAN DHCP #44995722211 first failed because the first version of the new checker searched for literal `\\n`; the e344 follow-up corrected this, but WAN DHCP #44995792863 then exposed two additional whitespace-sensitive expectations that omitted the space after `(`. The underlying WAN interface-name GNU89/ASan/UBSan executable passed on both source SHAs before the Python checker stopped the job.
- Exact 6ec WAN route #44995722139 and exact 6ec XY-C1 #44995722192, repeated on e344 as #44995792838 and #44995792759, failed at `scripts/check_wan_firewall_status.py:1170`: it still required `wan_ifname = get_man_ifname(unit)` and was stale after the checked source fix.
- Updated the interface-name checker to match normalized actual C call formatting. Updated the WAN firewall source contract to require checked `ifname` read → explicit read-error status handling → empty legacy no-op → validated interface token → explicit validation-error status handling → first interface side effect, and to forbid the removed lossy getter.
- Exact e344 AI handoff #44995792742 and Process inventory #44995792750 PASS; no XY-C1 L3 build is claimed. Recheck all exact-head Actions on the current follow-up SHA.

---
## 2026-11-22 — Correct WAN interface diagnostic and checker newline anchors

- Parent main SHA: `6ec3e3d6bf3576864dee986aaf1e7075a4fbc1fa`. Immediate source review caught two defects before treating CI as validated: the invalid-interface log format accepted only one of its two integer arguments, and the newly added Python checker delimited C functions using a double-escaped newline (literal backslash-n).
- Supplied both unit and ifname_status to the diagnostic and corrected the source-checker delimiters to resolve actual newline boundaries. Added an exact source assertion for the diagnostic argument order.
- The exact 6ec push triggered WAN DHCP callback/route, Process, NVRAM and XY-C1 plus the broader source-filtered suites; their first poll showed pending/in-progress only. Verify the follow-up SHA rather than inferring results from the intermediate head.

---
## 2026-11-22 — Fail closed when WAN startup cannot read its configured interface

- Parent main SHA: `7bd0c10e867a429dfbc7dc2573e211c1963aa580`. Exact 7bd storage hotplug #44995310447 and AI handoff #44995310444 passed. Exact XY-C1 #44995310496 completed host-only validation successfully, but Build XY-C1, TRX verification and artifact upload were skipped because 7bd changed only tests/checkers/handoff; no L3 claim.
- Confirmed a P1 startup-status loss in `start_wan()`: it called `get_man_ifname()`, which uses `nvram_safe_get()`. A transient NVRAM backend/read/close error was therefore indistinguishable from an empty configured interface and silently skipped WAN startup.
- Replaced the pointer lookup with `get_wan_unit_value_copy_status()` into a bounded local buffer, retained the established empty/unconfigured no-op, validated non-empty names through `wan_ifname_copy()`, and merged both read/validation failures into the first WAN startup status before any interface setup.
- Extended `scripts/check_wan_ifname_bounds.py` to pin checked read → empty legacy no-op → validated interface token → network side effects, plus status propagation and removal of the lossy getter from `start_wan()`. Existing WAN interface-name GNU89/ASan/UBSan policy cases and checked NVRAM-copy regression are reused by the existing WAN DHCP workflow.
- Exact-head WAN DHCP/route and XY-C1 Actions are pending for the new product-source SHA. No local MIPS build or hardware test is claimed. L3/L4 remain unverified.

---

## 2026-11-22 — Align printer hotplug checker with start-before-ack semantics

- Parent main SHA: `886f9cf54a6abafad251fb444de199e0930446f8`. Exact 1160 storage run #44995018412 passed the storage policy regressions and whole caller graph, then failed in `scripts/check_storage_safe_remove.py:1512`. Its assertion required the printer failure branch to precede `storage_ack_event_marker_status("usb_hotplug_lp")`.
- The product callback correctly calls `try_start_usb_printer_spoolers()`, acknowledges `usb_hotplug_lp` only when start status is zero, and only then enters the failure branch for start/ack failures. The branch logs that the event remains queued and calls `alarm(5)`. Updated the checker to assert this exact order and retry contract.
- Exact 886 notification queue Actions #44995118883 and AI handoff docs #44995119002 passed. Exact 886 XY-C1 validation #44995118906 completed SUCCESS, but Build XY-C1, TRX verification and artifact upload were SKIPPED because 886 changed only a test expectation and docs. This is not an L3 build result.
- Next: validate the checker on a fresh storage workflow, then push a real firmware-relevant product change so XY-C1 Build/TRX/artifact execute on one SHA.

---

## 2026-11-22 — Include storage-unplug bit in retry-retention regression

- Parent main SHA: `1160bf68ae35438c2ae5acbef8252b7faaea2c77`. Exact PID-1 notification workflow #44995018361 failed in its first GNU89 policy test. After adding the storage-unplug retry bit, the test correctly retained that bit when a different event was recorded with marker status 0, but one intermediate expected mask still omitted the new bit.
- Updated that expectation to include `RC_NOTIFY_HOTPLUG_RETRY_STORAGE_UNPLUG`. The PID-1 retry implementation itself was not the failure; the assertion was stale after expanding the mask.
- Local policy test will be rerun with GNU89, Werror, ASan and UBSan; exact-head notification/storage/XY-C1 Actions must verify the corrected test and product code.

---

## 2026-11-22 — Retain storage-unplug retry when the NVRAM marker write fails

- Parent main SHA: `7db3ff63d8c65c99163b59027fc00feb80e0171a`. The preceding storage run #44994403806 passed the storage inventory/marker/lifecycle regressions and complete caller graph, then failed in the Python checker because it searched for literal backslash-n instead of a real newline at the end of `storage_ack_event_marker_status()`. The parser correction is in parent `7db3ff63d8c65c99163b59027fc00feb80e0171a`.
- `storage_schedule_ejected_cleanup_retry()` previously ignored `nvram_set_int_temp("usb_unplug_ms", 1)`. If ejected cleanup and the NVRAM marker write both failed, the five-second alarm had no durable event to process, so cleanup could be abandoned.
- The scheduler now checks the marker result and calls `rc_notify_schedule_deferred_hotplug_retry("on_unplug_mass_storage")`. PID 1 records a timed retry bit and requeues the event on its next alarm; short-lived applets enqueue it through the existing secure notification queue. Original `errno` is restored.
- Added a storage-unplug retry bit and event mapping, extended GNU89 queue-policy regression for mapping/record/requeue/failure retention, and strengthened storage and PID-1 notification source contracts.
- Exact 7db3 storage #44994641468, XY-C1 #44994641391 and AI handoff docs #44994641406 were in progress at last poll. No local MIPS firmware build or hardware test is claimed.

---

## 2026-11-22 — Fix storage checker function-boundary escape

- Parent main SHA: `8f54faaaf0e0cbd2461ebd35f595612032521987`. Exact-head USB storage run #44994403806 passed the whole storage caller graph and all preceding C/shell regressions, then failed in `scripts/check_storage_safe_remove.py:1446` with `ValueError: substring not found`. The checker searched for literal backslash-`n` sequences (`"\\nstatic int\\nstorage_read_nvram_string"`) while the C source contains real newline characters.
- Corrected the Python string to use actual newline escapes so the checked marker-writer function ends at `storage_read_nvram_string`; product code and the asserted error-return contract are unchanged.
- Exact-head AI handoff documentation contract #44994403647 passed. XY-C1 validation #44994403623 completed SUCCESS on `8f54faa`, but its Build XY-C1, TRX and artifact-upload steps were SKIPPED due to the workflow's path/condition logic; this is not an L3 firmware-build result.
- Next: check exact-head storage workflow after this checker fix and establish why XY-C1's build steps were skipped; do not claim L3 until all three firmware gates actually execute and pass.

---

## 2026-11-22 — Update safe-remove retry checker for PID 1 queue handoff

- Parent main SHA: `9110090e4886d5a183dfe21dfbd26fde08698aea`. The corrected whole-storage caller graph passed on exact SHA `9110090e4886d5a183dfe21dfbd26fde08698aea`; the workflow then reached `scripts/check_storage_safe_remove.py` and failed at its stale line 478, which still demanded `nvram_set_int_temp("usb_hotplug_ms", 1)` and `alarm(5)` inside `safe_remove_stor_device()`.
- The product fix at `89a6e740e24a63142a56e3406136e86a514ce1a2` intentionally replaced that short-lived applet's local alarm with `notify_rc_status("on_hotplug_mass_storage")` after releasing `storage_lock`. PID 1 then owns the deferred event/alarm lifecycle; queue failure is logged and propagated rather than falsely reported as scheduled.
- Updated the source contract to require the PID 1 event name, verify no direct NVRAM marker/alarm remains in the short-lived retry block, require both success/failure logging and preservation of the notification errno. No product source changed in this checker-only slice.
- Exact 9110090e storage run #44994229595: the embedded whole-storage caller graph passed; the workflow failed in this stale safe-remove checker. The AI handoff contract #44994229636 passed. XY-C1 #44994003623 and Process lifecycle #44994003659 on product source ancestor `89a6e740` remained in progress; Process fail-closed #44994003779 and NVRAM boundary #44994003791 passed.
- Next: rerun storage checker and downstream firmware persistence gates on the corrected SHA; inspect XY-C1 Build/TRX/artifact on the same product-source content. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Match USB printer acknowledgement checker to the retry-safe source order

- Parent main SHA: `89a6e740e24a63142a56e3406136e86a514ce1a2`. Exact-head USB storage run #44994003668 passed the BusyBox ash Optware lifecycle checks, MTD save/restore/erase/backup transaction checks, persistence gate, eject inventory regression, deferred storage cleanup/ack regression, checked event-marker and Optware child-result tests, FTP daemon and storage-stop regressions, RC service-launch test, filesystem module-load lock checks and automount ownership checks.
- The final embedded whole-storage caller-graph checker failed at line 239 because its old assertion expected `printer_start_pos < printer_failure_pos < printer_ack_pos`. The actual retry-safe implementation acknowledges the marker only inside the successful-start branch, then checks the combined start/ack status and schedules `alarm(5)` on either failure; the correct order is start → conditional acknowledgement → failure/retry branch.
- Updated that source contract to require the actual order and to scope the retry assertions through the next modem section. No product source changed in this checker-only slice.
- Exact-head XY-C1 #44994003623, Process inventory #44994003659, NVRAM #44994003791 and Process fail-closed #44994003779 were IN_PROGRESS at the last poll. Firmware Build/TRX/artifact and those source validations must be checked again after this correction; no L3 claim.
- Next: rerun the storage source contract on the new SHA; fix only the first remaining stale assertion or real regression. Frozen backup remains read-only; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align the storage caller-graph regression with durable marker acknowledgement

- Parent main SHA: `d414a788abc66e0bf5d8711b4d0584de505964c9`. The exact-head USB storage run #44993693581 failed in the embedded “whole storage caller graph” source contract at the old assertion that required `usb_unplug_ms` to be cleared before `umount_ejected()`. This is now deliberately forbidden by the product fix in the parent.
- Updated the embedded workflow contract to require checked marker read → action-completion gate → eject cleanup → marker acknowledgement → ack-latch handling, and to reject any premature marker clear. This retains the existing caller-graph/lock-order assertions; no product source changed in this checker-only slice.
- The standalone deferred cleanup/ack policy regression had already passed locally with GNU89, Werror, ASan and UBSan. The failed workflow had reached the embedded source contract; the full storage checker and remaining gates were skipped afterward.
- Next: push this workflow-only correction atomically and inspect its exact-SHA USB storage and XY-C1 runs. No firmware/L3 or hardware/L4 claim.

---

