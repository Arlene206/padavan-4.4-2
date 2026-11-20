## 2026-11-20 — Include MiniUPnP netfilter production changes in XY-C1 build triggers

- Parent main SHA: `01f56e97c65d45fe998be26c69569208a92f7670`.
- Exact-head MiniUPnP allocation regression #44789878471 passed all steps on `8dbb0963cb6b816bf13cf13726f5de0ce3c867fb`, including the new ASan/UBSan cleanup binary and source contract. AI handoff #44789878451 also passed. Process lifecycle #44789544079 and NVRAM transaction #44789543972 passed on the previous production-source SHA `2ffb847`.
- The XY-C1 workflow's push path list did not include `netfilter/iptcrdr.c` or the new allocation-policy header, so no XY-C1 run had been scheduled for this firmware source change.
- Added both production paths to the XY-C1 trigger list. The workflow already watches its own YAML, so the current push triggers a fresh exact-head firmware build; Build, TRX verification and artifact upload remain the L3 acceptance criteria.

## 2026-11-20 — Correct boot NTP-reset local scope and call-path contract

- Parent main SHA: `8dbb0963cb6b816bf13cf13726f5de0ce3c867fb`.
- Exact-head watchdog run `44789384547` exposed that the new reset assignments are in `nvram_convert_misc_values()` (the NVRAM setup phase called by `init_router()`), while the local status variables had mistakenly been declared in the later `init_router()` function. This was a real GNU89 compile defect; the XY-C1 pre-build regression stopped before firmware compilation.
- Moved both status locals into `nvram_convert_misc_values()`, preserving the early dual reset and its fatal status propagation. Corrected the source contract to inspect the function that actually owns the writes and separately prove `init_router()` aborts before system-ready/autostart when NVRAM setup fails.
- Physical-router L4 remains NOT VERIFIED; exact watchdog and XY-C1 Build/TRX/artifact reruns required.

## 2026-11-20 — Normalize MiniUPnP allocation checker whitespace

- Parent main SHA: `2ffb8479a33ad55a4138b3384a31510f5b7c534e`.
- Exact-head MiniUPnP run `44789544201` on `2ffb8479a33ad55a4138b3384a31510f5b7c534e` passed the GNU89 ASan/UBSan allocation-cleanup binary and the existing configuration, daemon, PID-file and redirect-cleanup regressions. The source-contract checker alone failed at the entry-cleanup assertion because the C call spans multiple lines.
- Normalized extracted function bodies before matching entry, match, target and realloc cleanup calls. Assertions still enforce each guard before first dereference and verify exact cleanup arguments.
- No production C changed. Exact-head MiniUPnP rerun required; XY-C1 L3 not yet confirmed.

## 2026-11-20 — Fail safely when MiniUPnP iptables rule components cannot be allocated

- Parent main SHA: `f06fb00373149abe1afc6fcb06c9a0b6f9bd5510`.
- Confirmed an OOM crash path in `netfilter/iptcrdr.c`: TCP/UDP match factories and DNAT/SNAT/DSCP/MASQUERADE/ACCEPT target factories dereferenced unchecked `calloc()` results. Five rule builders also consumed potentially NULL components and leaked earlier allocations on partial failure.
- Added guards to all seven factories and all five builders. Entry, match, target and final rule-buffer failures release all previously allocated components, preserve allocator errno (defaulting to ENOMEM), and return before iptc append/commit.
- Added GNU89 ASan/UBSan cleanup/errno regression coverage and a Python source contract for every factory and builder. Both production and check/test paths are included in MiniUPnP push/PR workflow filters.
- No full local firmware checkout is available in this session; the exact-head workflow is the sanitizer/source-contract gate. XY-C1 L3 and physical-router L4 remain pending/not verified.
## 2026-11-20 — Fail closed when boot cannot invalidate stale NTP readiness

- Parent main SHA: `c9b931f0a17bb6673dd94bf7ad51c4c8ba6d7a75`.
- `init_router()` cleared temporary `ntpc_counter` without checking it, then reset persistent `ntp_ready=0` much later and also ignored that write. If either write failed, an old `ntp_ready=1` (and possibly stale counter) could make watchdog time-based scheduling trust the previous boot's clock state.
- Both freshness signals are now reset and checked together before router services, watchdog work, or user/autostart scripts proceed. Any failure returns the existing fatal router-init result. The persistent-vs-temporary semantics of the two keys are preserved.
- Fixed the misplaced include-guard terminator in `init_router_result_policy.h`, added GNU89 policy tests, and extended the watchdog lifecycle source contract/workflow. Exact watchdog/XY-C1 Build/TRX/artifact/Process Actions required; physical L4 NOT VERIFIED.

## 2026-11-20 — Align HTTPD marker helper contract with its checked NVRAM wrapper

- Parent main SHA: `1356dfd010bfc851336e2c5ab1f5b03abfd8119a`.
- Exact-head HTTPD Action `44788533266` on `1356dfd010bfc851336e2c5ab1f5b03abfd8119a` again passed the GNU89 daemon-policy binary but failed the integration contract at its assertion for `httpd_nvram_write_status` inside `httpd_started_state_publish_status()`. The latter correctly delegates through `httpd_nvram_set_int_status("httpd_started", started, 1)`; only the source assertion was looking at the wrong abstraction layer.
- Updated the contract to assert that exact delegation. The lower-level writer contract still verifies persistent and temporary backend writes plus errno normalization.
- Exact-head HTTPD rerun required; no production code or firmware source changed.

## 2026-11-20 — Correct HTTPD port-normalization source-check ordering

- Parent SHA: `18b14734be53ff7e53a974a6216d4a4294282cf0`. The HTTPD daemon workflow exposed one stale assertion still looking for a direct `nvram_set_int("http_lanport", ...)` call after the new checked wrapper was introduced; the source had correctly moved that write into `httpd_nvram_set_int_status()`.
- Updated the source contract to verify that process-presence scanning precedes the checked normalization writer. The same workflow commit also already asserts setter persistence semantics and lifecycle marker rollback.

## 2026-11-20 — Align HTTPD lifecycle source checks with checked NVRAM publication

- Parent main SHA: `c90407da0280365bdd41a17061b5b4d960bd4bce`.
- The HTTPD production slice now uses `httpd_nvram_set_int_status()` and `httpd_started_state_publish_status()` instead of direct unchecked NVRAM setters. Updated the lifecycle Action's embedded source contract to check persistent and temporary setter semantics, errno propagation, fail-closed port normalization, pre-launch stale-state clearing, post-start rollback, and checked stop/firewall marker publication.
- Exact HTTPD daemon lifecycle, XY-C1 Build/TRX/artifact and Process lifecycle runs on this workflow/source SHA remain required; physical L4 NOT VERIFIED.

## 2026-11-20 — Make HTTPD configuration and lifecycle-state NVRAM writes fail closed

- Parent main SHA: `12d3351f3deb024aa1a835aec9a30883225066f2`.
- `start_httpd_locked()` previously ignored failures while persisting normalized HTTP/HTTPS ports and while clearing/publishing the `httpd_started` runtime marker. A daemon could bind a fallback port while persistent configuration/firewall state remained stale, or a confirmed running/stopped process could be represented incorrectly.
- Added one status-preserving NVRAM integer writer for persistent configuration and temporary state. Port normalization now aborts before launch if its persistent write fails; marker writes before start and after confirmed start/stop are checked. If running-state publication fails after the listener is confirmed live, startup attempts a stop rollback and returns the publication error. Stop now propagates marker-publication failures after confirmed process exit.
- Firewall-error recovery uses the same checked marker publication and no longer performs a redundant ignored stopped-state write. Added backend errno normalization regression and strengthened the HTTPD source-contract workflow. Exact HTTPD, XY-C1 Build/TRX/artifact and Process lifecycle Actions required; physical L4 NOT VERIFIED.

## 2026-11-20 — Propagate LAN bridge reload-marker NVRAM failures

- Parent main SHA: `b9be4009afd514e2d56c160d777b9d7ad1e487c1`.
- `init_bridge()` recorded failures from switch, bridge, wireless and guest-isolation operations, but silently ignored both final `reload_svc_wl` and `reload_svc_rt` NVRAM writes. A device-node/driver error could leave stale reload state while bridge initialization falsely returned success.
- Both temporary NVRAM writes now feed the existing first-error/failure-count accumulator. The function continues the independent second write and returns the first failure, matching its existing aggregate-status contract. Added a GNU89 ASan/UBSan bridge-policy regression, a callsite source contract, and workflow trigger/compile coverage.
- Exact-head WAN-DHCP, XY-C1 Build/TRX/artifact and Process lifecycle Actions are required. Physical router L4 remains NOT VERIFIED.

## 2026-11-20 — Preserve modem-priority NVRAM persistence semantics while checking the write

- Parent SHA: `d6241fe7630a8df996127c7b295e69a675656641`. Review after the first passing source-contract run found that the initial checked implementation had used the volatile `nvram_set_int_temp()` helper, changing the prior call’s `nvram_set_int()` persistence semantics.
- Added a status-preserving persistent integer helper and switched the modem reconnect priority publication back to `nvram_set_int()` semantics. The reconnect still fails closed when the write fails; only the previously ignored status is changed. The WAN source contract now enforces the persistent setter and errno propagation.
- Exact WAN-route/Process and XY-C1 Build/TRX/artifact reruns required.

## 2026-11-20 — Match split-line modem-priority source assertion

- Parent SHA: `34235bef7131e72d1d006df5678a4729138501e8`. Static source assertion was checked against the actual C body before commit and exposed that the function call’s parameter is on the following line; the prior Python checker expected a space-normalized one-line prefix and would fail before reaching the runtime regression.
- Changed the checker to find the function-call prefix and the quoted `modem_prio` argument independently, preserving explicit ordering through failure gate/log and reconnect enqueue. No production code change in this follow-up; rerun the exact WAN-route source workflow.

## 2026-11-20 — Gate automatic USB-WAN reconnect on modem-priority publication

- Parent main SHA: `9a5506133875a00f31de8a96a4948b9f0befd93b`.
- `notify_on_internet_state_changed()` previously ignored the return from `nvram_set_int("modem_prio", ...)` and enqueued automatic WAN reconnect even if the priority write failed. That could reconnect the wrong modem/WAN using stale selection state.
- Replaced the unchecked persistent setter with the existing status-preserving volatile NVRAM helper `wan_nvram_int_temp_status()`. The reconnect notification is now skipped and the failure logged unless the priority was published successfully; unrelated user internet-state hook execution remains independent.
- Added a source contract for write → error gate/log → enqueue ordering and for preserving API errno. Exact WAN-route/Process and XY-C1 Build/TRX/artifact validation required; physical L4 NOT VERIFIED.

## 2026-11-20 — Scope the BusyBox DHCP daemonization ordering assertion

- Parent source SHA: `2150854923895688840e00a74d9510b07c2e65e5`; exact-head WAN-DHCP run `44785778326` failed only because the checker chose the first textual `write_udhcpc_pidfile()` occurrence inside the earlier `client_background()` definition instead of the main startup write after the `-d` fork branch.
- The assertion now searches for the PID publication after the actual branch, checks that `client_background()` daemonizes before rewriting the PID identity, and confirms the WAN launcher exercises both `-d` and `-b` options. This is a checker-only correction; exact-head callback/Process/XY-C1 verification remains required.

## 2026-11-20 — Correct DHCP readiness source-check formatting assertion

- Parent source SHA: `dc0e09f7924f415a4bdcfbae4fda07d888994a4b`; exact-head WAN-DHCP run `44785621612` FAILED only at the newly-added Python source assertion: `process_presence_start_postcondition()` is split across two C lines, and the checker’s whitespace-compacted expected text omitted the remaining space before `launch_status`.
- All earlier GNU89 `-Wall -Wextra -Werror` builds and the DHCP lease policy sanitizer binary passed in that run. This is a checker expectation correction, not a production C change; exact-head WAN-DHCP, Process, and XY-C1 reruns required.

## 2026-11-20 — Share detached DHCP child-readiness verification across LAN, WAN and IPTV

- Parent main SHA: `881f42a4d5173ac4de45299f6361cf65d00c6df8`.
- The LAN-specific postcondition is consolidated into `udhcpc_start_readiness.h` and applied to all three RC DHCP launchers: LAN `-d`, WAN `-d/-b`, and IPTV `-d`. BusyBox backgrounds both paths before publishing the PID/start_time record, so launcher exit alone cannot establish that the DHCP client survived.
- The shared helper waits within the existing four-probe policy and validates the exact `udhcpc` PID/start_time identity recorded at each interface-specific PID file. UNKNOWN inventory, wait errors, launcher failures, and missing-child timeout propagate to callers. The source contract asserts all three call sites and the BusyBox fork-before-PIDfile ordering.
- Exact-head WAN-DHCP, Process lifecycle and XY-C1 Build/TRX/artifact validation required. Physical L4 remains NOT VERIFIED.

## 2026-11-20 — Verify the detached LAN udhcpc child before accepting startup

- Parent main SHA: `e91c3a18c0dd676c4d4d7cd7b552b30792aaf655`.
- BusyBox `udhcpc -d` calls `bb_daemonize()` before writing `/var/run/udhcpc_lan.pid` with its PID/start_time identity. Consequently `_eval()` waits only for the short-lived daemonizing parent; a post-fork PID-file or signal-setup failure could make RC report success even though the AP DHCP child exited.
- `start_udhcpc_lan()` now performs a bounded readiness probe using `process_pidfile_read_identity()` and the shared RUNNING/ABSENT/UNKNOWN policy. It accepts only the exact live PID/start_time identity, retries ABSENT for the shared four-probe window, and propagates UNKNOWN, interrupted-wait, timeout and launcher failures. No broad process-name-only match can satisfy the gate.
- The WAN-DHCP source contract now asserts this startup postcondition. The shared process-presence policy binary already covers launcher failure, RUNNING success, ABSENT rejection and UNKNOWN rejection. Exact-head Process, WAN-DHCP and XY-C1 validation required; physical L4 remains NOT VERIFIED.

## 2026-11-20 — Log LAN DHCP acquisition failures without disturbing lease state

- Parent main SHA: `8e3c5143e5bcfd24e5b9886e7aa35df013adf463`.
- BusyBox calls the LAN `leasefail` hook when initial discovery/request retries exhaust their current attempt window; there is no active lease to deconfigure, and the client itself schedules another acquisition attempt. The hook was a silent no-op, leaving AP/router logs without any indication why DHCP was not obtaining an address.
- Added an explicit retry-status log while preserving its successful callback status. The source contract asserts that `leasefail` is observational only and must not run deconfig. The NAK callback remains log-only because BusyBox invokes the separate deconfig event after a valid NAK when an existing lease needs withdrawal; its contract now protects against duplicate teardown.
- This is an observability fix, not a change to DHCP retry or lease semantics. Exact-head WAN-DHCP callback and XY-C1 validation are required. Physical L4 remains NOT VERIFIED.

## 2026-11-20 — Refresh networkmap after an address-changing LAN DHCP renewal

- Parent main SHA: `9d8a9036cdd350a3a45971d8cd5d766d5ec93762`.
- `udhcpc_lan_bound()` applies a changed IP/netmask on a DHCP RENEW, but the post-commit `restart_networkmap()` gate only ran on initial BOUND. A changed subnet could therefore leave the networkmap inventory using the previous LAN range.
- Added a tested policy that refreshes on initial BOUND and on address-changing RENEW, while avoiding a restart for an unchanged RENEW. Updated the production callback and source contract; the address tuple is already committed before this ancillary refresh, so its failure remains logged and does not roll back a valid lease.
- Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy regression: PASS. Exact-head WAN-DHCP callback, XY-C1 and related Actions must validate THIS COMMIT.
- Rolling-window rotation: oldest 100 complete records are preserved in `docs/dev-log/2026/11/part-020.md`; root retains the newest 20.

---

## 2026-11-20 — Default MiniUPnP pinhole counters when the kernel rule is missing

- Parent main SHA: `a6a0175208fcbd073d1bbdb424de501a5885cf4b`.
- A live registry entry whose ip6tables rule had disappeared made `get_pinhole_info()` return success without assigning the requested packet/byte outputs. The public wrapper then copied an uninitialized packet count to SOAP callers; `CheckPinholeWorking` could report a false positive and `GetPinholePackets` could expose an indeterminate value.
- Added a shared null-safe counter initializer and call it before the netfilter lookup. A found rule still overwrites the zero defaults; a missing rule deterministically reports zero counters. Added regressions for both outputs and each optional NULL pointer, and pinned the production ordering in MiniUPnP CI.
- Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy regression: PASS. Exact-head MiniUPnP and XY-C1 Actions must be checked on THIS COMMIT; L3/L4 for this source change are not yet verified.

---

## 2026-11-20 — Make the OpenVPN domain source assertion Python-safe

- Parent main SHA: `30dec22ecde555e6d6617a15e1820d13ea15bb68`.
- VPN server lifecycle Action #44766269853 was exercising the corrected DNS branch checks when a later OpenVPN source assertion exposed another escaping defect in the checker: nested backslashes before double quotes made its Python expression invalid/incorrect.
- Rewrote the expected domain-publication fragment using `chr(34)` for the two C string-literal quotes. This keeps the required literal `'\\0'` intact without nested quote escaping; no production source changed.
- Exact-head VPN and AI Actions will validate this follow-up.

---

## 2026-11-20 — Match the quoted OpenVPN domain empty-string literal in CI

- Parent main SHA: `3dc44ff3378c9b29f6ec73076bc85015877f6b33`.
- VPN server lifecycle Action #44765846007 passed all GNU89 C policy binaries, then the source contract reached the OpenVPN client-up check and failed because its expected token omitted the C single quotes around `'\\0'`.
- Production `on_client_ifup()` correctly publishes the domain snapshot when policy permits it, otherwise publishes an empty value before the state-up marker. Updated the whitespace-normalized checker to assert the exact quoted C literal; production code and runtime behavior are unchanged.
- Exact-head VPN and AI Actions required for this checker-only repair.

---

## 2026-11-20 — Correct VPN DNS cleanup checker branch matching

- Parent main SHA: `d83eb501bd09fd56f2662e08ded2c434069f8bf1`.
- The first checker update corrected the stale NVRAM getter condition, and its exact-head VPN run #44765563839 still failed because the newly added resolver-branch assertion matched a literal backslash-n instead of the actual newline/tab sequence in the Python-extracted C body.
- Reworked that check to construct the expected branch text with `chr(10)` and tab characters; it still requires resolver refresh to be the `else` branch after the DNS snapshot clear status check.
- This is a second checker-only correction. The VPN C policy regression and other compiled steps passed in both runs; no product source changed.

---

## 2026-11-20 — Align VPN DNS cleanup source contract with checked getter semantics

- Parent main SHA: `acc1a3d1025f8826af2ea4ebfe524fb09b5609c4`.
- VPN server lifecycle Action #44763108307 passed all seven compiled C regression steps, then failed in `scripts/check_vpn_client_nvram_state.py`: the checker still expected the old getter condition `api_result == 0 || vpnc_dns[0] == '\\0'`.
- The checked-getter policy defines return 1 plus a present value as “present”, and return 0 plus an absent value as “not present”. Production cleanup correctly clears the DNS snapshot only for a successful present, non-empty value, then rebuilds resolver state only after the clear succeeds.
- Updated the source contract to assert both helper semantics and the production non-empty/present guard, including that resolver refresh is only on the successful-clear branch. This is a checker-only correction; no production source changed.

---

## 2026-11-20 — Normalize IPv6 pinhole wildcards and stage firewall registry updates

- Parent main SHA: `71cfd2831e0c6d959638d82243306bb680d23053`.
- Defect: the netfilter backend treated RemoteHost="*" as an IPv6 parse failure but still installed an all-ones source mask, creating ::/128 instead of a wildcard. Duplicate lookup used the same invalid token against an uninitialized address. Invalid IPv6 arguments were not rejected at this backend boundary.
- Added a shared IPv6 policy: NULL/empty/"*" consistently normalize to ::/0; malformed remote/internal values fail closed. UID allocation checks at most 65,535 values and returns ENOSPC instead of looping forever.
- Allocate UID/bookkeeping before installing the kernel rule, then publish the preallocated registry record only after a successful firewall commit. Added NULL checks for match/target allocation and release the libip6tc handle on successful commit.
- Added GNU89/Werror/ASan/UBSan host regression coverage and MiniUPnP source contracts; added production backend/header to XY-C1 firmware input filters and the Makefile dependency. Exact-head MiniUPnP and XY-C1 runs required after commit.

---

## 2026-11-20 — Restore mandatory end-of-session handoff checkpoint contract

- Parent main SHA: `2cb3ee3d27446aca096b14f9ac0324110c0afa8b`.
- Exact-head AI handoff run #44763259666 failed because `SESSION_HANDOFF.md` no longer contained the required end-of-session checkpoint heading and the rule that the checkpoint is mandatory while a document edit is conditional.
- Restored that explicit contract and synchronized the live checkpoint, exact-SHA Actions state, active MiniUPnP backend task and verification boundary. This is a documentation/CI repair only; no firmware L3 or hardware L4 claim.

---

## 2026-11-20 — Clear stale VPN domain snapshots across client sessions

- Parent main SHA: `84199a2fe17d774b8bcdba0a1918e7fa10a33160`.
- Defect: `restore_dns_from_vpnc()` cleared the runtime DNS snapshot but left `vpnc_dom_t`; OpenVPN client-up wrote that key only when a non-empty DOMAIN option was supplied. A disconnect/reconnect without DOMAIN could retain a stale domain from the previous session.
- Added checked domain-only cleanup and a regression. VPN cleanup attempts domain clearing even after DNS read/clear failure, preserves the first error, and rebuilds resolver state only after successfully clearing a non-empty DNS snapshot. OpenVPN client-up now publishes the current domain or an explicit empty value before the up-state marker.
- GNU89/Werror/ASan/UBSan policy regression and `scripts/check_vpn_client_nvram_state.py` are wired into VPN stability CI. Exact-head VPN and XY-C1 results are required; no firmware L3/hardware L4 is claimed.

---

## 2026-11-20 — Align WAN route callback contract with MTU retry state

- Parent main SHA: `125c64c1b38f9290500904bc5de73fcfb0ed3e83`.
- WAN route exact-head run `#44761848737` failed because the embedded source contract still expected the pre-MTU expression `callback_status = route_setup_status;`. The DHCP callback now aggregates route and MTU status before publishing retry metadata.
- Updated that assertion to the production aggregation expression. This is a checker-only repair; no product source changed.

---

## 2026-11-20 — Align WAN renewal source contracts with lease/route/MTU retries

- Parent main SHA: `533de8ffee96d62ac4c2a82fe7a31bd7cdaa0ccc`.
- The new retry policy now triggers a full DHCP bound re-entry for any retained lease/address, route, or advertised-MTU setup error. The WAN-route and HTTPD source contracts still searched for the older route-only helper, so they failed despite the production retry path being present.
- Updated both checkers to require the three-state helper and verify that WAN/IPTV renewal reads each corresponding NVRAM retry marker before parsing the changed lease tuple. No production source changed in this CI repair.
- Exact-head WAN-route, HTTPD and XY-C1 evidence must be checked separately; previous failures on `5c2cbb39` are not transferable.

---

## 2026-11-20 — Reject zero internal ports in MiniUPnP mapping requests

- Parent main SHA: cd6734bf52ef0bcf7429ee0b7f8978fc361660f3.
- SOAP AddPortMapping accepted NewInternalPort=0 because it checked only the external port after decimal conversion. AddAnyPortMapping had a parallel parse-plus-zero check. Port zero is not a valid internal TCP/UDP endpoint.
- Added `miniupnpd_soap_parse_nonzero_port()`, which validates into a temporary so failed parsing does not clobber the caller's output. Both mapping creation handlers use it; external-port zero semantics are unchanged.
- Added boundary tests for 1, 65535, zero, NULL and 65536, plus source contracts proving validation occurs before redirect creation. Exact-head MiniUPnP and XY-C1 checks pending.

---

## 2026-11-20 — Strictly parse numeric arguments in RC switch front-ends

- Parent main SHA: `9f2b19c2dac08dc1fb03d0da1e282a549d3c8594` (its parent source SHA is `bc0d9316777baa125e2ba22ef776d9ea812e3dd4`; intervening commit refreshed documentation only).
- Defect: `rtl8367_main()` and `mtk_esw_main()` used `atoi()` for command IDs and `get_param_int_hex()` for operands without checking complete-token consumption or overflow. Malformed numeric input could silently become a different ioctl command/argument, and extra operands were ignored.
- Added `switch_cli_parse_policy.h`: strict unsigned decimal for command selectors, decimal or explicit `0x` hexadecimal operands; rejects signs, whitespace, trailing data and overflow without changing the output value. Both front-ends now reject argc above four and parse every provided operand before ioctl dispatch.
- Added `tests/switch_cli_parse_policy.c`; local `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` run passed. The Switch CLI workflow compiles/runs the test and checks source wiring; XY-C1 production-input filters include both front-ends and the policy header/test.
- New exact-SHA Switch CLI/XY-C1 Actions pending; no MIPS firmware L3 or physical switch/router L4 claim.

---

## 2026-11-20 — Exact-head checkpoint bc0d9316777baa125e2ba22ef776d9ea812e3dd4

- Wireless: completed/success (#44746584209)
- AI handoff docs: completed/success (#44746584183)
- XY-C1: in_progress (#44746584073)
- NVRAM: completed/success (#44746584217)
- Process regression: no exact-SHA run listed
- Process inventory: completed/success (#44746584108)

- Source tasks: required RT3352 MII TxPower failure propagation, rollback error reporting and single-shot MLME publication. No L4 claim.

---

## 2026-11-20 — Treat RT3352 MII TxPower failure as an enable-gate failure

- Parent main SHA: `31de47640faed47555e830d86cd69f7228887d7e`.
- Review found the MII enable transaction still ignored the `iwpriv ... TxPower` result. It could unisolate and publish an enabled radio even when the configured transmit-power value was rejected, leaving the driver at an unintended default.
- The preparation callback now returns its command status. The policy keeps the port isolated, attempts RadioOn=0 and returns the original TxPower failure instead of unisolating/publishing. Added GNU89 regression coverage for rollback order and no publication.
- Wireless #44746039419, AI handoff #44746039183 and Process inventory #44746039325 passed on `31de4764`; NVRAM #44746039252 was still running. On production source `d250aff4`, NVRAM #44745706021 passed and XY-C1 #44745705732 remained in progress. This changes production code and requires fresh same-SHA XY-C1 validation; no L3/L4 claim.

---

## 2026-11-20 — Correct RT3352 MII unisolation-rollback test event count

- Parent main SHA: `d250aff4e337f5c919e11c44cf5a1e8e5c1cb5a8`.
- Wireless run #44745705779 compiled the policy test successfully, then the C test binary returned FAIL. Inspection of the expected sequence showed the unisolation-failure rollback records six events (initial isolate, radio-on, TxPower preparation, unisolate attempt, rollback isolate, radio-off), while the assertion compared only five and therefore never checked the final radio-off rollback.
- Corrected the expected event count to six. No production source change in this test-only correction.
- On `d250aff4`, AI handoff #44745705703 and Process inventory #44745705762 passed; NVRAM #44745706021 and XY-C1 #44745705732 were still running. New exact-head Wireless/AI reruns pending; firmware L3 and physical L4 not claimed.

---

## 2026-11-20 — Establish iNIC isolation before enabling RT3352 MII radio

- Parent main SHA: `9075dc6385307f5df83ce9d13039462648395583`.
- Follow-up review found a boot/out-of-band state gap: enable assumed the RTL8367 iNIC port was already isolated and turned on radio power before checking switch state. A stale/unexpected unisolated port could expose the newly enabled radio during the transition.
- Enable now explicitly asserts switch isolation first; any initial-isolation error aborts enable, performs best-effort RadioOn=0, preserves the isolation error and does not publish state. Only then does it enable the driver, apply TxPower and unisolate. Existing power-on/unisolation rollback remains.
- Extended GNU89 regression coverage with initial-isolation failure and event-order assertions. Strengthened the Wireless source contract to require isolation-before-power-before-unisolation.
- Parent exact-SHA Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149 passed on `9075dc6385307f5df83ce9d13039462648395583`; NVRAM #44744845007 was pending. Product-source XY-C1 #44744529158 on `9e9d643d` was still running. New exact-head regression pending; no MIPS L3 or hardware L4 claim.

---

## 2026-11-20 — Move RT3352 MII policy regressions to file scope and restore handoff checkpoint contract

- Parent main SHA: `9e9d643dd8c3a78d135c8dae908857f8b90c0276`.
- Wireless radio-state run #44744529117 on `9e9d643dd8c3a78d135c8dae908857f8b90c0276` failed at GNU89 compilation: the added MII mock callbacks and test functions had been inserted inside `main()`, producing invalid storage-class errors. Moved the complete helper/test block to file scope while retaining the seven regression calls inside `main()`.
- AI handoff check #44744529015 failed because the shortened SESSION_HANDOFF omitted the mandatory checkpoint language and four required state/task/verification/next-action sections. Restored the exact governance phrases and required headings without changing bootstrap/protocol rules.
- On the source SHA, Process inventory #44744529131, Process regression #44744528956, HTTPD security #44744528974, watchdog #44744529024 and iNIC signal #44744528955 passed. XY-C1 #44744529158 and NVRAM #44744528984 were pending. Corrected Wireless/AI exact-head Actions and same-source L3 remain pending.

---

## 2026-11-20 — Make RT3352 MII radio/isolation transitions transactional

- Parent main SHA: `cb4cb1debdfc2b5ba3b773e2e5fbbb6b58449f96`.
- `mlme_radio_rt_apply()` ignored the `phy_isolate_inic()` switch ioctl result and published MLME/NVRAM plus LED state even when the RTL8367 iNIC isolation boundary had not transitioned successfully.
- Added an RT3352 MII-only injectable transition policy. Enable keeps the port isolated until radio power is on; failed power/unisolation triggers best-effort isolate/off rollback. Disable attempts isolation before radio-off but still powers the radio down if isolation fails. MLME/LED publication is gated on successful required hardware operations.
- Added GNU89 regressions for event ordering, failed power-on rollback, unisolation/isolation failure, power-off failure and no-publication behavior. Updated Wireless source-contract checks and the XY-C1 production-input trigger for the helper header.
- Local validation: standalone transition-policy harness passed `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`. Exact-head Wireless/Process/AI/NVRAM/XY-C1 checks are pending; no target MIPS L3 or physical L4 claim.

---

## 2026-11-20 — Fix MiniUPnP lease write errno source contract

- Parent main SHA: `3bdb1d9f68767aeea29ce14bc93959426ee382e4`.
- Exact-head MiniUPnP regression #44743068543 on `ee9428dcaedd7530695ddfeedcac7a326448a705` compiled and passed every C policy/sanitizer binary, then failed its Python source contract at the lease writer. The checker compared the first `errno = 0;` (inside the mutually exclusive record-format error branch) against `fwrite()`, so it did not test the reset on the normal write path.
- Corrected the contract to select the nearest errno reset before the write call; the final-close ordering assertion was already corrected by parent commit `3bdb1d9f`. Verified the write and close reset positions against current `upnpredirect.c`.
- Parent exact-SHA MiniUPnP regression #44743367478 and AI handoff #44743367373 PASS. The failed #44743068543 run is on the preceding SHA and is not a current-head failure. New exact-head Actions are pending; no L3 claim.

---

