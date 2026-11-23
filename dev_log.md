## 2026-11-23 UTC — Normalize Process workflow inline Python indentation

- The first workflow-only update `8bac506` correctly replaced stale XUPNPD expectations but inserted its first `lua_udpxy` assertion at 20 spaces and left the trailing `snprintf` assertion at column 0. GitHub marked the workflow-file push run #45191666225 failed before it exposed runnable jobs.
- Replaced the entire injected Lua assertion block including its line prefixes so all statements align within the Python heredoc. The corrected block still checks the checked IPTV selectors and absence of legacy unchecked APIs.
- This is another checker-only repair; exact Process run pending. Do not treat #45191666225 as a product-source regression.
---
## 2026-11-23 UTC — Repair stale XUPNPD assertions in Process workflow

- Exact Process lifecycle runs #45189655889 (c8dade9), #45190394210 (a2a5892) and #45190706662 (2a9f7fe) passed preceding C regressions but failed the final LAN/PID1 inline source contract on obsolete XUPNPD expectations: the declaration omitted `xport_text`, and the old test expected `nvram_get_int("xupnpd_enable_x")` after LAN address loading.
- Updated the inline workflow to pin checked selector → disabled gate → LAN snapshot → procfs preflight order, and the checked `udpxy_enable_x`/`xupnpd_udpxy` reads in Lua URL generation. It also asserts the unchecked legacy getter/matcher are absent.
- Checker-only correction; no product source changed. Exact Process rerun pending; historical failures were stale workflow assertions, not C-policy test failures.
---
## 2026-11-23 UTC — Fail closed on LAN STP selector NVRAM read

- `start_services_once()` used `nvram_match("lan_stp", "0")` to decide whether to set bridge STP and forward delay. That API collapses an NVRAM backend read failure into a non-match, so a read fault could enable both bridge mutations while boot status remained successful.
- Replaced the lossy match with `service_nvram_match_status()`. Missing values retain the old non-match behavior; checked-read failures now log and enter the accumulated boot status without changing STP or forward delay.
- Added regression cases for absent/present selectors, EACCES and errno-less EIO; the integration checker pins error handling before either bridge mutation. Firewall Startup #45190708304 passed all tests on source `2a9f7fe`. The Process integration run #45190706662 found only a checker whitespace assertion failure after all earlier checks passed; checker-only correction `a9053a0` now compacts whitespace. Exact rerun and XY-C1 Build/TRX/artifact verification were pending at last poll. No local firmware build or hardware result is claimed.
---
## 2026-11-23 UTC — Preserve checked IPTV NVRAM state in XUPNPD config

- `start_xupnpd()` had a second unchecked read of `udpxy_enable_x` while generating `xupnpd.lua`, and used `nvram_match("xupnpd_udpxy", "1")` without distinguishing a missing/false setting from an NVRAM backend read failure. A backend fault could silently remove the IPTV URL from an otherwise successful XUPNPD configuration.
- Both configuration reads now use the shared checked NVRAM copy helper; the port is strictly parsed/range-checked and the integration flag keeps the exact legacy string-equals-1 behavior. Read/parse errors abort the temporary config transaction before rename, preserving the previous live Lua config.
- The Process integration checker pins the checked reads and parser order. Existing checked-copy/strict-parse GNU89 policy regressions apply. Exact Process and XY-C1 Build/TRX/artifact checks are pending.
---
## 2026-11-23 UTC — Propagate USB printer runtime-marker write failures

- `start_u2ec()` launched the printer daemon, ignored failure from `nvram_set("apps_u2ec_ex", "1")`, and always returned success. A NVRAM write/close failure therefore hid an incomplete printer hotplug action from the retry queue.
- Added an errno-preserving printer NVRAM write policy and GNU89-friendly unit cases. `start_u2ec()` now returns the write failure after logging that the daemon was started but the marker could not be confirmed; the caller keeps the hotplug action retryable rather than falsely completing it.
- The exact USB modem/printer policy workflow and same-SHA XY-C1 firmware checks are pending. No runtime or hardware result is claimed.
---
## 2026-11-23 UTC — Check XUPNPD enable-port reads before service preparation

- `start_xupnpd()` used `nvram_get_int("xupnpd_enable_x")`, so an NVRAM device read failure collapsed to port 0 and was treated as a disabled service. It also loaded and validated the AP LAN address before checking the enable port, even when the service was disabled.
- It now uses the checked NVRAM copy and strict integer parser first, preserves missing/empty as the legacy disabled zero, rejects backend/malformed values and out-of-range enabled ports, then performs interface/LAN/procfs/storage preflight only for an enabled service.
- The Process integration checker pins checked read → disabled/range gate → LAN snapshot → process preflight. Reuses the existing checked-copy/parser regressions. Exact-head Process/XY-C1 checks pending.
---

## 2026-11-23 UTC — Fail closed on IPTV proxy port-selector reads

- `start_udpxy()` used `nvram_get_int("udpxy_enable_x")` and returned success for values below 1024. That API collapses an NVRAM backend read failure into the integer default, so a failed read was silently misreported as an intentionally disabled IPTV relay; malformed values also passed through prefix/overflow-prone conversion.
- The selector is now copied through the shared checked NVRAM reader and parsed as a complete decimal integer before the disable gate or process/config side effects. Missing/empty retains the legacy port-0 disabled default; backend errors and malformed/overflow values abort; configured enabled ports above 65535 are rejected.
- Existing GNU89 policy tests cover checked-copy missing/error and strict integer parsing; the Process integration checker now asserts checked read → parse → disabled gate → process inventory ordering. Exact-head Process/XY-C1 checks pending.
---

## 2026-11-23 UTC — Fail closed on MSD Lite enable-state publication

- `start_udpxy()`'s `APP_MSD_LITE` path ignored the result of `nvram_set("msd_lite_enable", "1")` and launched `msd_lite` anyway. A failed NVRAM write could leave the daemon running while the enable state remained disabled, making restart/UI state inconsistent.
- The branch now normalizes the setter result through the existing `rc_wan_nvram_write_status()` policy and refuses daemon launch if the enable-state write cannot be confirmed. The Process integration checker asserts that the failure gate precedes `eval`; the existing WAN NVRAM policy regression covers errno and errno-less write failures.
- Exact Process lifecycle and same-SHA XY-C1 Build/TRX/artifact verification are pending. No router runtime result is claimed.
---
## 2026-11-23 UTC — Align HTTPD cross-service checker with checked L2TP markers

- Exact HTTPD run #45187074690 on `eb928a2b259f601792bd7b8a55078e5c45e82cfe` failed only at the embedded `Verify bounded service termination and WAN stop failure gates` source contract. The production WAN L2TP path now uses the checked marker setter, but this independent HTTPD workflow still asserted the removed raw `nvram_set_int_temp("l2tp_wan_t", ...)` calls.
- Updated the embedded assertions to require the allow-listed checked setter and the fail-closed RPL2TP/xl2tpd launch messages. No HTTPD or WAN production code changed. Exact-head HTTPD rerun required.
---

## 2026-11-23 UTC — Fix VPN marker checker declaration order

- Exact VPN lifecycle run #45187074365 on `eb928a2b259f601792bd7b8a55078e5c45e82cfe` compiled and passed the new GNU89 shared-xl2tpd marker status regression plus all existing VPN C regressions, but the Python source checker raised `NameError`: the newly inserted ownership-order assertions referenced `safe_start_locked` before the original assignment later in the file.
- Moved the existing function-body extraction above those assertions; no production code or test behavior changed. Exact-head VPN rerun required.
---

## 2026-11-23 UTC — Check WAN and VPN-client shared xl2tpd ownership publication

- A source pass across the shared xl2tpd call graph found three additional unchecked runtime marker writes in `net_ppp.c`: `l2tp_cli_t=1` immediately before daemon launch and `l2tp_wan_t=0/1` before RPL2TP/xl2tpd WAN launch. A backend write/close failure could launch a shared transport without the marker used by VPN teardown/restoration to decide whether an independent WAN or VPN client still depends on it.
- Added one allow-listed status-bearing setter for `l2tp_cli_t` and `l2tp_wan_t`. The VPNC marker must publish successfully before xl2tpd launch; WAN xl2tpd refuses launch until its marker is published; RPL2TP refuses launch if stale xl2tpd ownership cannot be cleared. Error mapping and source-order regressions are wired into the existing VPN workflow.
- Exact-head VPN Actions and XY-C1 build pending.
---

## 2026-11-23 UTC — Propagate VPN-triggered NFS export reload failures

- `restart_vpn_server()` called `reload_nfsd()` under `APP_NFSD` but discarded its status. Because NFS exports include VPN ACL/network policy, a failed export generation or NFS reload could leave stale share access after a VPN configuration change while reporting restart success.
- The restart now records/logs the NFS reload result, preserves the first errno, merges it into the overall restart status, and still attempts shared xl2tpd restoration and lifecycle unlock. The regression source contract enforces ordering; the existing first-error unit policy covers error precedence.
- Exact-head VPN Actions and XY-C1 build pending.
---

## 2026-11-23 UTC — Align VPN stop checker with checked ownership setter

- Exact VPN lifecycle run #45185622799 on `374a3c45664f2f06a84c1b39375d86bb389ca2e1` compiled and passed every C regression, including the new NVRAM write-status cases; the source checker then failed because a pre-existing stop-order assertion still searched for the removed raw `nvram_set_int_temp()` call.
- Updated that assertion to require the checked marker wrapper after OpenVPN stop and before callback cleanup. The new marker-specific source contracts had already passed before the stale assertion was reached. Exact-head VPN/XY-C1 rerun required.
---

## 2026-11-23 UTC — Fail closed on VPN L2TP runtime marker writes

- The VPN server's `nvram_set_int_temp("l2tp_srv_t", ...)` calls discarded all four setter results. An NVRAM device-node open/ioctl/close failure could start xl2tpd or pptpd while leaving PID 1's shared-daemon ownership marker stale, or report a successful stop with stale ownership state.
- Added a checked setter wrapper that normalizes NVRAM API and close failures. L2TP startup now refuses daemon launch if marker publication fails and best-effort rolls the marker back while preserving the first error; PPTP startup refuses launch if it cannot clear a stale L2TP marker. Stop reports marker failure while continuing independent callback/lease cleanup.
- Added GNU89/Werror/ASan/UBSan policy cases and exact source-order contracts. Exact VPN workflow and XY-C1 build pending.
---

## 2026-11-23 UTC — Scope infosvr PID-file O_TRUNC contract to open flags

- Exact Infosvr run #45184200774 on `9fd7bafd5ad6ecb6511d8f6b21b8a0942ff9ee7c` passed all six GNU89/Werror/ASan/UBSan policy binaries, including the new competing-open PID-file test. The Python source contract then failed because it searched for `O_TRUNC` anywhere in `acquire_pidfile()`, matching the explanatory comment that explicitly says not to use it.
- This checker-only correction inspects the actual `open()` flag argument, so the contract still rejects truncating opens without rejecting the safeguard comment. No production source or regression behavior changed. Exact-head Infosvr rerun required.
---

## 2026-11-23 UTC — Serialize infosvr daemon PID ownership

- The RC start path uses a tri-state process preflight, but the check and daemon launch are not one atomic operation. Concurrent start requests can both observe ABSENT; infosvr enables SO_REUSEADDR on its UDP socket, so separate daemon children can bind the same service socket. The prior PID writer opened with truncation and ignored write/close errors, while shutdown unconditionally removed the shared marker.
- Infosvr now opens the PID file without truncation or symlink following, requires a single-link regular file, and obtains a nonblocking exclusive flock before replacing stale PID contents. The child holds that lock for its lifetime. PID publication checks truncate/seek/write and retries EINTR; shutdown unlinks only an exact current-PID record whose path still resolves to the locked inode.
- Added GNU89/Werror/ASan/UBSan regression for competing opens, no-clobber behavior, lock handoff, exact ownership and invalid PID inputs. Wired the regression and source-order/path-filter assertions into the Infosvr workflow. Local helper regression passed; exact-SHA Actions pending.
---

## 2026-11-23 UTC — Update the SCUT workflow's PID cleanup assertion

- Exact-head SCUT run `45181232164` passed shell syntax and lifecycle/PID-race execution but its inline source contract still expected the old literal-`$` cleanup expression. A concurrent source-contract update has already aligned the NVRAM ordering check with `watchcat_read_boolean`; this follow-up changes the remaining PID assertion to require `$$`.
- No production code changed. Exact-head SCUT rerun required.

---

## 2026-11-23 UTC — Align SCUT watchdog source contract with fixed PID comparison

- Exact-head SCUT watchdog run #45181232164 on `ce39930eee413d71e0227c7d59e8e539b18e9f98` passed shell syntax and the full PID/NVRAM/link-state shell regression. Its source-contract step then failed because one legacy checker assertion still expected the old literal-`$` PID comparison; the production code correctly uses `$$`.
- Updated the workflow assertion to require the current-shell PID comparison. This is a checker-only repair; no production or test behavior changed. Exact-head SCUT/AI/Process reruns and XY-C1 Build/TRX/artifact remain the verification gate.

---

## 2026-11-23 UTC — Fail closed on unknown SCUT watchdog WAN link state

- Follow-up review found `mtk_esw 11` was still consumed through command substitution without checking its exit code. An ioctl failure produced empty output that the old comparison interpreted as "WAN link up", allowing a health failure to restart SCUTClient while physical link state was unknown.
- The watchdog now propagates switch command failures through its checked failure path and cleans only its own PID marker. A confirmed WAN-down value still skips health checks. Regression stubs inject the switch error and preserve the WAN-down path.
- Updated the SCUT workflow ordering contract to target the new checked NVRAM gate and extended it to pin WAN ioctl error handling. Exact-head SCUT watchdog, Process inventory, AI handoff and XY-C1 Build/TRX/artifact validation remain required. No local execution or hardware result is claimed.

---

## 2026-11-23 UTC — Correct SCUT watchdog PID cleanup in source

- The first follow-up commit `aabcc161` recorded the intended `$$` correction but a generated Git tree check showed the production blob was still unchanged; the source edit had not been materialized. This commit applies the correction and verifies the resulting source blob content before publication.
- `watchcat_cleanup_pidfile()` now removes the PID file only when its contents equal the current shell PID (`$$`). The exact-head SCUT regression `45180376775` remains the failure to rerun; the fixture export correction is already present in `e522760`.
- The roadmap file is restored from the uncorrupted parent and its SCUT note is applied with a literal-safe replacement. No local shell execution or router validation is claimed.

---

## 2026-11-23 UTC — Repair SCUT watchdog regression harness environment

- Exact-head SCUT watchdog run #45180376775 on `31676e8f363e976c930568041ec09690fb08a60f` passed shell syntax and reached the existing PID-replacement regression, then failed because the test-harness rewrite had accidentally dropped the export of `NVRAM_LOG`, `KILL_LOG` and `RM_BLOCKED`. The NVRAM mock therefore could not append its event log and never performed the injected PID replacement; this was a test fixture defect, not a product-code assertion.
- Restored the three environment exports alongside the new logger/client test paths. The watchdog source and its fail-closed NVRAM behavior are unchanged. Exact-head SCUT watchdog, AI handoff, Process and XY-C1 checks must pass before closing the slice.

---

## 2026-11-23 UTC — SCUT watchdog checks NVRAM read status

- `scutclient_watchcat.sh` used legacy `nvram get` for its enable, auth-flag and force-reauth selectors. An NVRAM backend fault therefore looked like an empty value: the watchdog could silently skip health monitoring, while a failed `scutclient_wdg_force` read before 06:00 looked "not 0" and could trigger repeated unnecessary client restarts.
- The watchdog now uses `nvram get_checked` and preserves the empty legacy defaults (watchcat/client/auth-flag disabled; empty force-reauth remains enabled). Read errors and unsupported UI boolean values abort the watchdog action, remove only its own PID file and preserve the client; no speculative re-auth occurs on a failed policy read.
- Expanded the existing POSIX shell lifecycle regression with injected errors for all four read positions, invalid selector values, the legacy empty-force behavior, and the explicit force-off case. The existing SCUT watchdog Actions workflow runs syntax, PID identity/race tests and the source contract. No local execution or router validation is claimed.

---

## 2026-11-23 UTC — Make the SFE configuration policy reject unknown modes

- Follow-up hardening after the NVRAM snapshot validation: `rc_sfe_configure_policy()` itself treated every value except 1 and 2 as a successful no-op. Although the startup snapshot now rejects values outside 0/1/2, a future direct caller could still pass 3 or -1 and report success while leaving a requested SFE configuration unapplied.
- Mode 0 remains the explicit no-write/disabled state; modes 1 and 2 retain their existing sysfs behavior. All other values now return `EINVAL` before invoking the writer. Regression coverage checks -1, 3 and 99 produce no writes, and the WAN source checker pins the API contract. The existing XY-C1 workflow already tracks the production header and test.
- Exact-head WAN route GNU89/Werror/ASan/UBSan and XY-C1 Build/TRX/artifact Actions are required. No local compile or hardware validation is claimed.

---

## 2026-11-23 UTC — Reject malformed SYN-cookie policy selectors

- The Basic Firewall UI exposes `fw_syn_cook` only as radio values `0` and `1`, but `rc_network_sysctl_parse_syncookies_status()` used `strtol(value, NULL, 10)` and treated any nonnumeric string as 0. A corrupted value such as `not-a-number` therefore looked like an explicit request to disable SYN cookies; `1junk` was also accepted as enabled. This weakened a security setting despite a successful NVRAM read.
- The parser now requires a complete decimal value in the 0/1 domain before publishing the setting. An absent key still preserves the legacy disabled default; malformed, empty, negative and out-of-range values return an error without changing the caller's enabled state. Regressions cover valid 0/1, invalid 2/-1/junk/empty and numeric overflow, and verify the WAN admission gate remains closed on parse failure.
- Exact-head WAN route GNU89/Werror/ASan/UBSan, NVRAM audit and XY-C1 Build/TRX/artifact Actions are required. No local compile or hardware validation is claimed.

---

## 2026-11-23 UTC — Validate the SFE mode domain before module reload

- `rc_sfe_configure_policy()` supports mode 0 (disabled), 1 and 2 (two distinct `skip_to_bridge_ingress` settings). The NAT offload snapshot previously accepted any integer. A corrupt value such as 3 was truthy in `reload_nat_modules()`, so it could load `fast_classifier`, while the SFE configuration policy treated the unknown mode as a successful no-op and left the accelerator unconfigured.
- The snapshot now rejects SFE values below 0 or above 2 before publishing settings. Regression coverage exercises `3`, `-1` and malformed text, verifies transaction closure and preserves the prior settings on failure; a positive mode-2 case confirms that the second supported mode remains compatible. The WAN source checker pins the 0/1/2 domain. The immediately preceding source SHA `eeaefd8` separately added strict 0/1 validation for `ip6_lan_auto`.
- Exact-head WAN route GNU89/Werror/ASan/UBSan and XY-C1 Build/TRX/artifact Actions are required. No local compilation or hardware behavior is claimed.

---

## 2026-11-23 UTC — Reject invalid IPv6 NAT auto selector values

- The `ip6_lan_auto` option is a 0/1 toggle in the IPv6 UI, but the NAT offload snapshot previously accepted any integer and forwarded it as the `ipv6_nat` mode. The HW NAT radio-policy adapter only enables IPv6 NAT for exactly `1`, so an invalid value silently forced the feature off during an otherwise successful module reload.
- The coherent snapshot now rejects any present `ip6_lan_auto` value other than 0 or 1 before publishing HW NAT/SFE settings or touching modules. Regression coverage exercises `2`, `-1` and malformed text, checks the transaction is closed and verifies the previous output remains unchanged. Existing HW NAT mode values remain unmodified (including mode 2), and SFE's distinct 0/1/2 mode is intentionally not treated as a boolean.
- Exact WAN route, NVRAM boundary and XY-C1 Build/TRX/artifact Actions are required. No local test or hardware behavior is claimed.

---

## 2026-11-23 UTC — Reject invalid NAT helper boolean selectors

- `reload_nat_modules()` consumes `wan_nat_x` and the PPTP/H.323/SIP ALG switches before unloading or loading kernel helpers. The snapshot parser accepted every integer for `wan_nat_x`, while ALG values other than exactly `1` silently became disabled; a corrupted selector such as `2` could therefore disable a configured helper during reload.
- Added strict empty/0/1-compatible boolean parsing for all four selectors. Missing keys and empty legacy values remain disabled; any other numeric or malformed value aborts the entire snapshot before publishing settings or changing module state. Added GNU89 regression cases for `2`, `-1` and malformed text in every selector, including preservation of the caller's prior snapshot.
- The existing WAN route workflow already compiles the regression with GNU89, `-Wall -Wextra -Werror`, ASan/UBSan and runs the source-order contract. Exact-head WAN route and XY-C1 Build/TRX/artifact validation are required; no local run or hardware result is claimed.

---

## 2026-11-23 UTC — Fail closed on invalid DNSMasq static-ARP selector

- The static-host NVRAM snapshot parsed `dhcp_static_arp` as an arbitrary integer and copied it to `i_arp_bind`; any nonzero value enabled static ARP binding, although the web UI only exposes values `0` and `1`. A corrupted value such as `2` or `-1` could silently activate ARP bindings.
- The snapshot now accepts exactly `0` or `1` and fails closed before reading the remaining table or staging/replacing DHCP host files. Regression checks cover invalid positive and negative values, transaction closure, zeroed output and no reads beyond the invalid selector. The source contract pins this behavior; the XY-C1 path filter now includes the production header.
- Exact-head DNSMasq, AI handoff and XY-C1 build Actions are required. No local compile or device runtime validation is claimed.

---

## 2026-11-23 UTC — Trigger DNSMasq ARP regression on pull requests

- The DNSMasq lifecycle job reads and statically checks BusyBox `networking/arp.c` in its static-ARP validation step, and its push path filter already included that file; the pull-request filter did not. A PR changing only the ARP command handler therefore skipped the security regression.
- Added the missing source path to `pull_request.paths`, added it to the step's declared dependency contract, and made the workflow assert complete push/pull_request path-set parity so another one-sided trigger omission is caught automatically.
- Exact-head DNSMasq lifecycle and AI handoff Actions are required.

---

## 2026-11-23 UTC — Reject invalid ZeroTier static-route enable values

- The ZeroTier route UI offers only `0` (disabled) and `1` (enabled), but preflight and route execution accepted every non-negative digit string as enabled. An invalid NVRAM value such as `2` could therefore install a route that the UI had no way to express intentionally.
- Changed both preflight and add/delete execution to accept only empty/0 as disabled and 1 as enabled; all other values abort. Added regression coverage for no route side effects and preservation of a running daemon across both start and stop.
- Exact-head ZeroTier lifecycle, WAN route, XY-C1 and AI Actions are required; device runtime is not claimed.

---

## 2026-11-23 UTC — Match ZeroTier startup contract markers to loop-based regression

- The startup-selector source logic and shell regression both ran, but the Python contract expected fully expanded key names while the test intentionally used one loop variable (`$setting`) for the two invalid-selector cases. Corrected the source contract to assert the actual reusable marker and its daemon-preservation assertion; no production source or runtime regression changed.
- Exact-head ZeroTier lifecycle and AI documentation Actions are required.

---

## 2026-11-23 UTC — Validate ZeroTier selectors before daemon replacement

- `zerotier_load_start_settings()` checked all five NVRAM reads but accepted arbitrary values. A corrupted Moon toggle such as `2` or `enabled` fell through to the `remove_moon()` branch, while a malformed network ID was rejected only in `add_join()` after `start_zero()` had stopped the old daemon and cleaned its network state.
- Added a shared empty-or-16-hex network-ID validator used by startup preflight and the join-file writer. The Moon selector now accepts only the legacy empty default or explicit `0`/`1`. Both checks run before snapshot publication and before `kill_z()`; regressions cover invalid values, atomic non-publication and preservation of a running daemon.
- Exact-head ZeroTier lifecycle, WAN route and XY-C1 validation are required. No local runtime or hardware validation is claimed.

---

## 2026-11-23 UTC — Make ZeroTier route-count contract parsing shell-safe

- The bounded-route shell regression passed on `aee9699d`, but the Python source-contract parser treated the apostrophe in a shell comment as the start of a character literal and failed while extracting the new helper. Reused the checker's established explicit function-boundary strategy for this shell helper, avoiding false parsing of shell comments without weakening the count assertions.
- Exact-head ZeroTier lifecycle, AI handoff and XY-C1 validation are required; no local execution is claimed.

---

## 2026-11-23 UTC — Bound ZeroTier static-route count before iteration

- The web UI limits ZeroTier static routes to 64 entries, but both `zerotier_preflight_static_routes()` and `zero_route()` trusted any digit-only NVRAM count and fed it directly to `seq`. A corrupted or externally modified count could create an effectively unbounded startup/stop loop before daemon state changes.
- Added one shared normalizer/validator that preserves empty, zero and leading-zero compatibility, rejects non-digits and values above 64 before arithmetic/iteration, and applies identically to startup/stop preflight and route add/delete. Regression cases cover 65, leading-zero 65, a huge decimal string, empty, zero and the maximum 64; the source checker ties the bound to the UI's existing 64-route limit.
- Exact-head ZeroTier lifecycle and AI documentation Actions are required. XY-C1 has an existing `trunk/user/zerotier/**` path trigger; no local test/build or hardware result is claimed.

---

## 2026-11-23 UTC — Make LLDT startup cleanup regression detect shell commands

- Follow-up review of the new LLDT init-wrapper contract found its Python raw-regex literal had been double-escaped, so the negative assertion could not match an `rm ... .pid` command if it regressed. Replaced it with a direct per-line command check; the production script is unchanged.
- Exact-head LLDT PID-file workflow rerun is required. The initial source-fix commit remains the one that removes destructive PID-file cleanup; this commit only tightens regression detection.

---

## 2026-11-23 UTC — Match the legacy EventTcp_WriteBuf buffer type

- Review against `event_tcp.h` confirmed `EventTcp_WriteBuf()` accepts `char *`, not `const char *`. Made the shutdown acknowledgement buffer writable static storage to avoid a discarded-qualifier warning in the firmware build; the event API copies the bytes into its own state before queuing.

---

## 2026-11-23 UTC — Preserve LLDT live PID markers during startup

- `S75lld2` unconditionally removed `/var/run/lld2d*.pid` before launching the daemon. Re-entering the start wrapper while an LLDT daemon was live therefore erased the PID evidence that `osl_write_pidfile()` uses to reject a second instance; the sidecar lock serializes publication but cannot recover a marker deleted before the next launch.
- Removed the destructive cleanup. The existing locked PID writer remains the sole authority for deciding whether a record is live or stale (only `kill(pid, 0)` returning `ESRCH` permits replacement). Added a startup-wrapper source contract and wired the script into LLDT regression and XY-C1 product-build path filters, with a detector test.
- Exact-head LLDT, XY-C1 and AI documentation Actions are required. No local checkout or local compile is available; hardware runtime behavior is not claimed.

---

## 2026-11-23 UTC — Supply the configured PREFIX to the RP-L2TP host harness

- The exact-head WAN run `45174056209` passed the source contract and reached the new error-message test, but the standalone host compile failed because `utils.c` normally receives `PREFIX` from the configured build and the isolated test compile did not provide it. This was a harness build-configuration omission; no production compile failure was observed.
- The host harness now defines `PREFIX` as `/usr`, matching the installed-prefix string use in the unused loader section, and the source contract requires that compile flag. The test still links only the error-formatting sections under `--gc-sections`.

---

## 2026-11-23 UTC — Flush the RP-L2TP shutdown acknowledgement before exit

- `cmd_exit()` queued `OK Shutting down` through the asynchronous `EventTcp_WriteBuf()` path and then immediately ran cleanup and `exit(0)`. The event loop never got a chance to send the queued acknowledgement; with the controller now validating response tokens, `l2tp-control exit` would correctly see an empty reply and report failure.
- Shutdown now runs from the write-completion callback, which closes the client socket after the complete acknowledgement (or on write failure/timeout) and then stops tunnels, performs cleanup and exits. Event-registration failure takes the same cleanup path synchronously. Added source-contract coverage for the ordering.

---

## 2026-11-23 UTC — Fix RP-L2TP error logging's ended va_list reuse

- `l2tp_set_errmsg()` passed its `va_list` to `vsnprintf()`, called `va_end()`, and then passed that ended list to `vsyslog()`. Reusing a `va_list` after `va_end` is undefined behavior; the log record could contain corrupted arguments or fail while handling an error.
- The formatter and syslog paths now each receive a fresh `va_start()` / `va_end()` pair. Added a host regression that intercepts `vsyslog()` and checks mixed string/integer arguments and successive calls, plus source-contract and WAN/XY-C1 trigger coverage.

---

## 2026-11-23 UTC — Match watchdog signal workflow push and pull-request inputs

- The watchdog workflow's first regression compiles and executes `watchdog_httpd_probe_policy.h` and its test, but both paths were present in `push.paths` and absent from `pull_request.paths`. A pull request changing either dependency alone would not run the regression.
- Added both exact paths to the pull-request filter and extended `check_watchdog_ntp_lifecycle.py` to require both inputs and assert complete push/pull-request path-set parity. Product firmware source is unchanged.
- Exact-head Watchdog and AI handoff Actions on the resulting main SHA are required; no local checkout or local test run is available in this environment.

---

## 2026-11-23 UTC — Close RP-L2TP control clients when event registration fails

- `EventTcp_ReadBuf()` / `EventTcp_WriteBuf()` return `NULL` when allocation, `fcntl()` setup, or event registration fails; in those paths they do not assume ownership of the accepted socket. The `cmd_acceptor()` and `cmd_reply()` callers ignored the return value, leaving accepted control descriptors open and clients stalled during memory/resource pressure.
- Both call sites now close the socket when scheduling the read/write event fails. Extended the WAN source contract to protect the accept and response failure paths. The exact-head request/response parser regressions are already passing on `2904fa3`; a new WAN run verifies these resource cleanup gates.

---

## 2026-11-23 UTC — Bound RP-L2TP command framing before NUL termination

- Review of the actual `EventTcp_ReadBuf()` implementation confirmed it allocates exactly the requested 512 bytes and reports `EVENT_TCP_FLAG_COMPLETE` both on delimiter match and when the receive limit is reached. `cmd_handler()` previously wrote `buf[len+1]` for lengths below 511 and `buf[len]` at length 512, then parsed EOF/timeout-truncated data. This allowed an uninitialized byte to be included in a command and a full-buffer request to write one byte beyond the heap allocation.
- Added a shared request-framing policy: accept only a complete newline-terminated request with `0 < len < capacity`; reject EOF, timeout, empty, unterminated and capacity-sized input before touching the buffer; then terminate at `buf[len]` and replace the newline. Added GNU89 ASan/UBSan boundary regressions, source contracts and WAN/XY-C1 trigger coverage.

---

## 2026-11-23 UTC — Fix AP monitor source-contract newline matching

- Process lifecycle run `45172093429` passed all earlier PID 1, NVRAM and daemon regressions and failed in `scripts/check_process_lifecycle_integration.py` while locating the AP monitor block. The checker searched for literal backslash-`n` sequences (`\\nfi\\n`) although the source contains actual newline characters, so it raised `ValueError` before checking the AP lock contract.
- Corrected the Python source literal to `"\nfi\n"` / `"\nfi"`. The separate AP lock regression already handles indentation and tests both worker lifetimes; this change repairs only the source extraction contract.

---

## 2026-11-23 UTC — Make the L2TP newline source contract literal-safe

- Tightened the RP-L2TP source contract to use a Python raw string for the C `\\n` token. Without the raw-string prefix, Python would interpret the escape as an actual newline and compare against the wrong source representation.
- No production behavior changed; exact-head WAN and XY-C1 validation remain required for the control-socket hardening slice.

---

## 2026-11-23 UTC — Fail closed when the RP-L2TP control socket cannot be secured

- `handlers/cmd.c` ignored the return value from `chmod(sockname, 0600)` and proceeded to `listen()`. If the permission change failed, the daemon could expose its privileged local control socket with the mode inherited from the process umask. Startup now records the chmod error, closes the listener, removes the socket path, restores `errno`, and aborts before `listen()`.
- Preserved the controller's historical trailing newline for both `OK` and `ERR` replies. Extended the PPP/RP-L2TP source contract for the server permission gate and newline ordering, and added `cmd.c` to WAN and XY-C1 production-input triggers.

---

## 2026-11-23 UTC — Fix AP relay lock regression indentation handling

- Process lifecycle run `45171514518` passed the PID 1 and DDNS boot checks but failed the AP relay lock test's line-count assertion. The production `rm -f /tmp/apc.lock` line is intentionally indented inside the generated helper script; the test used `grep -Fxc` against an unindented literal, so it counted zero despite extracting the correct single guarded operation.
- Changed the test to count the command with an anchored whitespace-tolerant expression. The process-presence test still checks that the lock remains while either worker lives and is removed only when both are absent.

---

## 2026-11-23 UTC — Align source contracts with the DDNS publisher and PPP response regression

- The exact-head WAN run `45171234395` exposed a brittle test-name assertion: it looked for `function();`, while the test correctly checks each function's return with `CHECK(function() == 0)`. Relaxed the source contract to require the actual function call.
- The exact-head Process lifecycle run `45171234444` exposed a stale boot contract that still expected inline `copyscripts_seed_status=1` in the DDNS call site. The shared `seed_storage_file()` helper owns that error aggregation now; the checker explicitly requires the DDNS helper invocation and rejects the old direct `cp -f` path. The preceding test step confirms the symlink and external sentinel behavior.
- Both were checker mismatches revealed by CI; no production source change in this follow-up. New exact-head WAN, Process and XY-C1 runs must verify the revised contracts and RP-L2TP host test.

---

## 2026-11-23 UTC — Propagate RP-L2TP control-socket command failures

- `start_rpl2tp()` treats a zero exit from `/usr/sbin/l2tp-control start-session` as a successful session. The bundled controller previously returned zero on every clean socket EOF, even when `cmd.so` had replied `ERR Unknown peer` or another command error; it also ignored partial/failed `writev()` and could block indefinitely waiting for a reply.
- The controller now parses the daemon's `OK`/`ERR` response token before returning success, rejects empty/truncated/malformed replies, writes the command with an EINTR/partial-write loop, and applies five-second send/receive socket timeouts. Added GNU89 ASan/UBSan response-policy regression and checker/workflow path coverage; XY-C1 firmware validation is triggered for the production handler source. Exact-head Actions/build pending.

---

## 2026-11-23 20:02:52 Beijing — Prevent premature AP relay lock release

- Fixed a process-state bug in the generated AP relay monitor: its cleanup gate checked `ap_script_num` twice, so it removed `/tmp/apc.lock` when `ap_script.sh` had exited even if `sh_ezscript.sh` was still performing the relay scan. This allowed a second scan/reconfiguration to overlap the first.
- The gate now requires both independently measured counts to be zero. Added a shell regression that extracts and executes the exact production fragment under mocked process tables for script-only, helper-only, both-running and neither-running cases. Wired the test and AP script into Process CI push/PR filters and XY-C1 firmware input filtering.
- Exact Process/XY-C1 validation for this SHA is pending; no local shell execution or firmware result is claimed.

---
## 2026-11-23 UTC — Correct the DDNS storage regression fixture

- Pre-run review found the new DDNS publication test's `printf` format had a doubled backslash and would write a literal `\\n`, so the preservation assertion would fail for a fixture-format reason.
- Changed the fixture to pass the string separately to `printf '%s\\n'`. Production code is unchanged; exact-head rerun is required.

---

## 2026-11-23 UTC — Publish the DDNS factory script through the safe storage writer

- The boot-time DDNS default path checked for a symlink and then used `cp -f` on the destination. That check/write sequence was not atomic: a concurrent replacement could redirect the privileged copy through a symlink, and the separate chmod also left a partial publication window.
- Reused the existing `seed_storage_file()` transaction, which validates the storage root/parent/leaf, stages beside the destination, applies mode 0755 before publication, and links without replacing a concurrent destination. Existing operator-managed regular files remain untouched; symlink and non-regular targets fail closed and join the boot status.
- Extended the script host regression and AliDDNS source contract to require DDNS default seeding through the shared no-follow publisher. The workflow already triggers for the source, test and checker paths. Exact-head AliDDNS, Process/XY-C1 and firmware checks are required; no local test or target build is claimed.

---

## 2026-11-23 19:48:37 Beijing — Reject malformed Ethernet source MACs in gateway ARP replies

- During a follow-on review of `detect_wan_arp_reply_matches()`, found it verified that Ethernet source and ARP sender-hardware fields matched each other but did not verify the sender MAC was a valid unicast station address. An attacker or malformed bridge frame could use a multicast, all-zero or broadcast sender address and still be treated as a live gateway reply, suppressing DHCP recovery.
- Added an explicit responder-MAC policy: unicast locally administered addresses remain valid, while multicast, all-zero and all-broadcast addresses are rejected before IP tuple matching. Added GNU89 regression cases for all four invalid forms and the missing sender-hardware pointer; the dedicated WAN ARP workflow now pins the new production check and test.
- Exact-head WAN ARP regression and XY-C1 Build/TRX/artifact are pending. No local sanitizer, firmware or physical-router result is claimed.

---
## 2026-11-23 19:44:36 Beijing — Repair OpenSSH lifecycle CI contracts

- Exact-main Process run `45167154533` exposed a stale integration fixture: `check_optional_app_start_status.py:check_sshd()` still expected the wrapper to call raw `killall`, but the new standalone stop correctly refused to proceed without its shared process-lifecycle helper (status 2). Added a test-local helper adapter that delegates to the existing mocked `killall`, and asserted the wrapper passes exactly `sshd` while propagating status 61.
- Exact-main AI documentation run `45167154428` found that the Process regression push allowlist included `trunk/user/openssh/Makefile`, while its pull-request allowlist did not. Added the same packaging input to PR filters; the lists now remain identical.
- These changes repair test/trigger contracts only. The OpenSSH source hardening remains in `e525274ee5a75490627bdc11593a5e3da9b53558`; exact-head Process and AI-doc reruns are pending, as is XY-C1 firmware validation. No local test, MIPS build or hardware result is claimed.

---
## 2026-11-23 19:38:16 Beijing — Fail closed on standalone OpenSSH stop

- Found a lifecycle gap in the standalone `/usr/bin/sshd.sh stop` entry point: `func_stop()` issued `killall -q sshd` without checking the initial process inventory or confirming shutdown. The main RC `stop_sshd()` path already uses the shared lifecycle transaction, but the script's direct CLI bypassed it.
- Replaced the raw signal with the shared `pl_stop_named` transaction (tri-state inventory, bounded TERM wait, fresh recheck before KILL, and confirmed-absence postcondition). The OpenSSH package now installs `/usr/lib/process_lifecycle.sh` itself; a missing, symlinked or unreadable helper fails closed rather than reverting to `killall`.
- Extended the existing OpenSSH shell regression for delegation, missing-helper refusal and helper error propagation; added a source contract and included the OpenSSH Makefile in Process and XY-C1 build triggers. Exact-head Process/XY-C1 validation pending; no local shell run or target image is claimed.

---
## 2026-11-23 UTC — Reject duplicate static VPN peer addresses

- `vpn_server_read_acl_snapshot()` validated every per-client address value individually but `start_vpn_server()` did not validate uniqueness across the snapshot. Two authenticated PPP clients could therefore receive the same configured static remote address in `chap-secrets`, creating a peer-address collision and ambiguous routing.
- Added a bounded policy check for the complete ACL snapshot before lease-inventory cleanup, private CHAP file creation or daemon startup. Repeated nonzero static offsets now fail with `EADDRINUSE`; zero remains the supported dynamic-assignment value and can be shared. GNU89 policy coverage includes distinct, duplicate, dynamic-only, null and count-overflow cases.
- Exact VPN Server regression rerun is required. No local MIPS/uClibc build, sanitizer run or hardware behavior is claimed in this record.

---
## 2026-11-23 UTC — Enforce removal of all unchecked URL MAC fallbacks

- After the IPv6 source fix, a fresh source scan confirms exactly two URL-MAC consumers (IPv4 and IPv6) call `firewall_read_url_filter_mac()` and there are no remaining `mac_conv("url_mac_x")` fallbacks anywhere in `firewall_ex.c`.
- Restored a whole-file negative assertion in the Firewall Startup contract now that the previously missed IPv6 branch has been converted. The per-protocol checked-read → abort → staged-file removal → return ordering assertions remain in place. The exact Firewall Startup rerun on the IPv6-fix SHA #45165224108 passed; this adds a regression guard for future accidental fallback reintroduction.

---
## 2026-11-23 UTC — Close IPv6 URL-filter MAC fail-open gap

- Source review after the IPv4 URL-MAC checker repairs found the matching IPv6 `ip6t_filter_rules()` branch still called `mac_conv("url_mac_x")`. Unlike the updated IPv4 branch, it still collapsed an NVRAM backend failure into an empty qualifier and could publish an IPv6 URL-filter rule without its per-client MAC restriction. This was a real fail-open gap, not a checker-only issue.
- Replaced the IPv6 fallback with the shared presence-aware checked reader. Backend or malformed-value errors now abort core restore, remove the staged rule file, preserve errno and return before appending the URL-filter rule, matching IPv4 behavior. The destination is sized to the exact 18-byte formatted-MAC requirement.
- Existing Firewall Startup source contract iterates both `ipt_filter_rules()` and `ip6t_filter_rules()` and verifies checked-reader → abort → staged-file removal → error return precede the MAC rule. Exact-head Firewall Startup rerun required; no local build or sanitizer result is claimed.

---
## 2026-11-23 UTC — Scope URL MAC legacy-call assertion to URL builders

- Firewall Startup #45164776940 on `d9796188e245b2cd2ae752b865b140b4f0959a8b` passed the checked-helper and policy fixture assertions but failed a global source assertion that prohibited every `mac_conv("url_mac_x")` call in `firewall_ex.c`. The remaining legacy call is in an unrelated filter-generation path outside the IPv4/IPv6 URL-rule builders; the URL builders themselves already have explicit scoped assertions forbidding that fallback.
- Removed the over-broad global assertion while retaining both per-builder checks that each URL filter uses the checked helper and cannot emit a rule after a read failure. No runtime source was changed. Exact Firewall Startup rerun required.

---
## 2026-11-23 UTC — Repair Firewall Startup checker variable reference

- The exact-head Firewall Startup rerun #45164580872 progressed past the prior multiline-source assertion and then raised `NameError`: the checker referred to `firewall_policy`, but its existing fixture variable is named `filter_policy`. HTTPD policy regression #45164581007 passed, including the corrected 17-byte undersized-buffer test.
- Updated the Firewall Startup contract to use the already-loaded `filter_policy` variable. This is a checker-only fix; rerun Firewall Startup on the next main SHA.

---
## 2026-11-23 UTC — Correct URL MAC regression test boundaries

- Exact-head run #45164372502 stopped in the Firewall Startup source checker because the new source assertion expected a checked-getter call on one line while production intentionally wrapped its arguments across lines. HTTPD regression #45164372525 compiled the new firewall policy test and exposed a bad test boundary: a 23-byte buffer was expected to fail even though the formatter contract requires only 18 bytes.
- Fixed the source contract to validate the normalized multiline call and changed the negative buffer test to pass 17 bytes. Production firewall code is unchanged; rerun exact-SHA Firewall Startup and HTTPD checks before accepting the fix.

---
## 2026-11-23 UTC — Fail closed on URL-filter MAC qualifier reads

- Found that both IPv4 and IPv6 URL-filter builders called the generic `mac_conv("url_mac_x")`, which uses error-collapsing `nvram_safe_get()` and only checked stored string length. An NVRAM backend error could silently remove a configured MAC qualifier, and a 12-byte non-hex value could become an untrusted bare `--mac-source` token in the staged iptables-restore file.
- Added a checked, presence-aware URL MAC reader. Confirmed missing/empty/zero/broadcast values retain the historical optional-no-qualifier behavior; valid 12-hex-digit values are formatted through the existing strict MAC policy; backend errors and malformed present values abort and remove the unpublished IPv4/IPv6 rule file before core restore publication.
- Extended the GNU89 firewall policy test with missing/present/read-error/invalid-input cases and the Firewall Startup source contract with both-family abort-before-rule assertions. Exact-SHA Firewall Startup, HTTPD and XY-C1 runs are required; no local compile, MIPS build or hardware result is claimed.

---
## 2026-11-23 UTC — Match watchdog timezone source filters for pull requests

- AI handoff check #45163084187 on `da6c721111852b8ca888d2797717a9b29b4713fb` exposed two path-filter asymmetries in `.github/workflows/process-lifecycle-regression.yml`: the watchdog-timezone policy header and its GNU89 regression test were added to `push.paths` by the timezone scheduler fix but omitted from `pull_request.paths`.
- Added both exact paths to the pull-request filter so PR validation now runs for the same source/test changes as main pushes. This repairs the governance failure without relaxing the parity check. Exact-head AI docs rerun required.

---
## 2026-11-23 UTC — Update HTTPD umbrella contract for checked firewall schedule calls

- HTTPD authentication/security run #45162166214 on code SHA 444674117db122af04538cff330777f5421dd0e4 passed all HTTPD auth/upload/route contracts, then failed in its shared firewall source audit at inline Python line 831. That umbrella checker still required two direct `if (timematch_conv(...))` calls which were deliberately replaced in ad562 by explicit `url_schedule_status = timematch_conv(...)` handling so NVRAM errors can abort the staged IPv4/IPv6 restore.
- Updated the umbrella contract to require both checked call sites and both negative-error gates. The dedicated Firewall Startup source-order contract remains stronger and still verifies abort/cleanup/return before rule publication. No production code changed in this checker-only repair; exact HTTPD rerun required.

---
## 2026-11-23 UTC — Keep scheduled actions on the last valid timezone

- Found an unchecked NVRAM access in `watchdog.c:timecheck_reboot()`: unlike the checked timezone setup used by PID 1, this scheduler still called `nvram_safe_get("time_zone_x")` and overwrote process `TZ` with an empty value on a backend error. The resulting local-time comparison could trigger a configured reboot/update at the wrong wall-clock time. The same function also ignored `time()` failure and could evaluate an uninitialized timestamp.
- Reused the existing `timezone_env_policy.h` checked-read formatter. The scheduler now validates/copies the timezone before changing `TZ`, preserves the prior environment and skips the scheduled action on read/validation/setenv errors, rate-limits repeated error logs, and checks the clock result before calling `localtime()`.
- Added the existing timezone GNU89/Werror/ASan/UBSan policy test to the Process lifecycle workflow and source-order assertions for checked read → validate → apply → clock. Process and XY-C1 exact-head Actions are pending; no local compile or MIPS result is claimed.

---
## 2026-11-23 UTC — Retry LAN networkmap when completion state cannot be inspected

- Found a tri-state recovery bug in `udhcpc_lan_bound()`: a missing or inaccessible PID 1 incomplete-action directory made `lan_dhcp_networkmap_retry_pending()` return a negative error, but the caller converted that UNKNOWN state to zero. An unchanged DHCP RENEW then skipped retrying a previous failed networkmap refresh.
- Added `lan_dhcp_networkmap_retry_required()`: only a confirmed absent marker (0) may suppress a retry; a present marker (1) and any inspection error (negative) both conservatively requeue the idempotent networkmap request. The DHCP lease remains independent of this ancillary refresh.
- Added GNU89 policy regression cases for absent, present, EIO and ENOENT probe results, and strengthened the production source-order contract to prevent UNKNOWN from being normalized to “no retry”. Exact-head WAN DHCP callback and XY-C1 Actions are pending; no local sanitizer, MIPS build or hardware result is claimed.

---
## 2026-11-23 UTC — Fail closed on URL-filter schedule NVRAM reads

- `timematch_conv()` previously used `nvram_get()` and substituted the all-days/all-day legacy defaults whenever a lookup returned NULL. Because that path could not distinguish an absent key from an NVRAM backend failure, a read fault could silently broaden the URL-filter activation schedule. It now uses `nvram_get_checked()` via the presence-aware string reader, copies each value before the next shared-cache read, preserves defaults only for genuinely missing keys, and returns an error for backend failures.
- Both IPv4 and IPv6 URL-filter rule builders now abort the staged filter restore, remove the unpublished rule file and preserve the read errno when either schedule lookup fails. Present-empty and oversized values remain inactive, matching existing schedule validation semantics. The policy helper and GNU89 ASan/UBSan test cover missing defaults, present-empty, backend EIO, oversized values and invalid destinations.
- Product-source commit `ad56220d066e771be499426708b6c2f744937467` (`fix(firewall): fail closed on URL schedule read errors`) had Firewall Startup run #45161165818 fail only at a stale test-name assertion after the preceding checked-NVRAM, lifecycle, startup, enable and static-route checks passed. Checker-only follow-up `db60eb4deb95f7702c63f50563b8cc9b23d350e9` corrected the assertion; exact Firewall Startup #45161387113 PASS and AI docs #45161387099 PASS on db60. WAN DHCP callback #45161165802 also PASS on ad562. XY-C1 #45161165974 on the product-source SHA ad562 remained in progress at Build XY-C1, with TRX and artifact pending at last poll.
- The prior LAN networkmap-retry product slice `edeb1e8354a97b312d8f3f5dd8787af030abfc6d` is L2-verified by WAN DHCP callback #45160239807 on descendant checker SHA cbec. Exact XY-C1 #45160010405 on edeb completed Build, TRX verification and artifact upload; artifact `XY-C1-firmware-edeb1e8354a97b312d8f3f5dd8787af030abfc6d` (12,759,150 bytes, ID 14811585463). This confirms L3 through edeb, not the later firewall URL-schedule source.
- No local MIPS/uClibc build, local sanitizer run or physical-router test is claimed; sanitizer and policy results above are GitHub Actions results only.

---
## 2026-11-23 UTC — Fix firewall URL schedule regression contract

- Firewall Startup run #45161165818 on source SHA ad56220d066e771be499426708b6c2f744937467 passed the firewall IP checked-read contract and lifecycle/startup/enable/static-route GNU89 ASan/UBSan binaries, then failed at inline Python line 634. The checker referred to a nonexistent standalone test function name; the tests are assertions inside the existing `main()` and directly exercise `firewall_schedule_copy_checked_value()`.
- Corrected the contract to assert the helper is exercised by `firewall_filter_policy.c`. No runtime C or test code changed in this follow-up. Exact Firewall Startup rerun is required. The initial exact WAN DHCP callback #45161165802 completed PASS on ad56220. XY-C1 #45161165974, Process inventory #45161166010, AI docs #45161165835 and NVRAM audit #45161165871 were in progress at last poll; no result is inferred.

---
## 2026-11-23 UTC — Normalize notification macro source-contract whitespace

- WAN DHCP callback run #45160010474 on product/checker SHA edeb1e8354a97b312d8f3f5dd8787af030abfc6d compiled and passed the initial USB-modem, WAN interface-name, DHCP lease policy and WAN NVRAM ASan/UBSan binaries, then stopped at the new source-contract assertion for `RCN_RESTART_NETWORKMAP`. The checker expected one literal space while the shared header aligns macro definitions with tabs; this was a formatting-only checker false negative.
- The assertion now removes source whitespace before comparing the macro name/value. Runtime code, queue policy and test semantics are unchanged. Exact-head WAN DHCP callback rerun required. Networkmap signal lifecycle #45160010227, PID 1 notification queue #45160010473, Wireless #45160010276 and AI docs #45160010425 already PASS on edeb1e8. XY-C1 #45160010405, Process umbrella #45160010508 and Process inventory #45160010184 were still in progress at last poll. No local MIPS build or hardware result is claimed.

---
## 2026-11-23 UTC — Reject non-unicast Ethernet source addresses in WAN ARP

- Extended the new ARP source-MAC parser beyond syntax: a sender address must be unicast, not all-zero and not broadcast. A syntactically valid multicast/group MAC (for example 01:00:5e:...) or reserved zero/broadcast value is not a valid station source address for the router's Ethernet/ARP probe, even though every nibble is hexadecimal.
- Kept locally administered unicast addresses accepted (02:...) and added explicit globally administered unicast coverage (00:11:22:...). Added transactional rejection cases for zero, broadcast, IPv4 multicast and IPv6 multicast source MACs; the output buffer remains unchanged on failure.
- The check remains local to ARP frame construction and does not change the historical WAN override formatter or NVRAM values. Exact-head regression and XY-C1 Build/TRX/artifact checks for this source refinement are pending.

---

## 2026-11-23 UTC — Validate WAN ARP source MAC before packet construction

- Confirmed a malformed-input boundary in arpping_gateway(): the WAN/LAN MAC is read through checked NVRAM APIs, but each octet was decoded with strtol(tmp, NULL, 16). The end pointer was ignored, so non-hex legacy override text silently became zero or a partially parsed byte and was transmitted as the Ethernet/ARP source address.
- Added an ARP-boundary-only parser for exactly six colon-separated hexadecimal octets. It accepts uppercase/lowercase hex and locally administered addresses, rejects malformed lengths/separators/nibbles/null pointers, and leaves output untouched on failure. The legacy WAN override formatter is deliberately unchanged for configuration compatibility.
- Replaced the permissive strtol loop in the live ARP probe. An invalid source identity now produces EINVAL/probe error before socket creation or packet construction, so it cannot be mistaken for a gateway no-reply or trigger DHCP renewal.
- Added GNU89 -Wall -Wextra -Werror ASan/UBSan regression cases for valid mixed-case parsing and transactional rejection of malformed/missing values. Extended the existing ARP source contract to require strict parser use, removal of the permissive decoder, and the parse/error gate before socket creation. Exact-head WAN ARP and XY-C1 Build/TRX/artifact checks are pending; no local or hardware result is claimed.

---

## 2026-11-23 UTC — Complete compact DBDC return-order assertions

- Exact Wireless #45157966807 on SHA 280e959c passed the wireless policy executable, Ralink config ASan/UBSan, bridge membership, guest isolation and the first DBDC token-presence checks, then failed with `ValueError: substring not found` at inline Python line 656. The second DBDC ordering lookup still searched for `return -<peer_status>;` in a whitespace-stripped function string. The prior correction fixed the token-presence assertion but missed this second occurrence; this checkpoint aligns the remaining ordering lookup with the same compact representation. Production wireless code is unchanged.
- AI handoff documentation #45157966954 passed on 280e959c after restoring the validator-required heading `## Exact first action for the next GPT`.
- XY-C1 #45157966804 on 280e959c was IN_PROGRESS at last poll, with the build step not yet completed; TRX and artifact outcomes remain unverified. The source product baseline remains 0e4976cf, whose #45156939135 build was also still in progress at last poll. Checker-only descendant runs with skipped product steps do not establish L3.

---

## 2026-11-23 UTC — Normalize DBDC error-return contract and restore handoff heading

- Exact Wireless run #45157423895 on source SHA d9a5f32 passed the GNU89 policy binaries, Ralink config/SKU ASan/UBSan, bridge membership and guest-isolation tests, then failed at the DBDC peer-band source contract (inline Python line 649). The checker compacts all whitespace from the function body but expected `return -<status>;` with a space; the production source correctly returns `-rt_radio_status` / `-wl_radio_status`, which becomes `return-<status>;` in the compact representation. Updated the assertion to use the compact form; no runtime source change.
- Exact AI handoff run #45157423846 failed because the handoff used heading `## Exact first action for the next session`, while the validator requires `## Exact first action for the next GPT`. Restored the protocol/validator-required heading and refreshed the checkpoint facts.
- XY-C1 run #45157423889 on d9a5f32 completed workflow SUCCESS but Build XY-C1, TRX verification and artifact upload were all SKIPPED because this was checker/docs-only. Product-source XY-C1 #45156939135 on 0e4976cf remained IN_PROGRESS at Build; TRX/artifact pending. The latest confirmed full Build/TRX/artifact PASS remains #45154946558 on descendant product SHA eac5eeee. No L3 is claimed for 0e4976cf.

---

