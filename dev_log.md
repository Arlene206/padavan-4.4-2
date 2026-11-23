## 2026-11-23 UTC — Bound WSD accepted-client processing time

- Found two related TCP availability defects in wsdd2: a negative recv on an accepted client propagated as a service failure and restarted the WSD listener; also, the per-recv SO_RCVTIMEO was reset after each read, allowing a slow-trickle peer to block the single-threaded event loop indefinitely.
- Commit fc515297323886e9d7955e46f2a605f7396b0592 isolates accepted-client read failures, uses one five-second request deadline across the initial read, request line, headers and body with poll/nonblocking recv, and sends 408 on timeout. Added idle-client and slow-trickle socket regressions and production-source assertions.
- Exact fc51529 Actions: WSD TCP send #45141178054 PASS, WSD XML #45141178151 PASS, Process inventory #45141178220 PASS. WSD HTTP #45141178185 failed only the Python source checker’s whitespace-sensitive assertion after the sanitizer binary had completed. Checker correction commit 940c53921c210898199bfc78b1b1b0cb33c206d6; exact-descendant WSD HTTP #45141306447 PASS (GNU99 ASan/UBSan socket test and source contract).
- NVRAM audit #45141178137 and XY-C1 #45141178162 were still in progress at last poll; XY had reached Build XY-C1 with preceding source checks successful, TRX/artifact pending. No L3 claim and no local test/build claim.

---

## 2026-11-23 UTC — Align the RC public SSID encoder prototype

- Exact-head Process run #45139858176 on SHA 80824697be5358d7fc38ca258ec0c1fd4beb252b passed the LLDT PID parser, LLTD config-path parser and WLAN SSID encoder ASan/UBSan test steps, then failed at the final source integration assertion: rc.h still declared void char_to_ascii(char *output, char *input).
- The implementation and both RC callers already used the bounded three-argument integer-returning API. This stale public declaration was a real compile-time integration defect, not merely a checker variable issue. Updated rc.h to the actual API and strengthened the contract to prohibit the old declaration and require stddef.h.
- Parent main 0774f960873402a81c2ca1ae3dec62dabdfc688b also contains an independent wsdd2 HTTP header initialization fix. On that SHA, inventory #45140001262 PASS; XY-C1 #45140001410 and NVRAM audit #45140001652 were still in progress. Exact-head Process/XY Actions for this follow-up pending; no local firmware-build claim.

---

## 2026-11-23 UTC — Bound RC WLAN SSID percent encoding

- Found an RC memory-safety defect in common_ex.c::char_to_ascii(): it encoded arbitrary input without an output capacity, formatted signed char values with %X, copied a temporary via strcpy(), then advanced the destination by only three bytes. High-bit bytes could expand beyond the assumed %HH width; worst-case valid 32-octet SSID escaping also needs 97 bytes while rc.c supplied only 64.
- Replaced the implementation with an unsigned-byte %HH encoder that preflights the complete output size, rejects inputs above the 32-octet SSID limit, and leaves output untouched on any failure. Changed both 2.4/5 GHz callers to a 97-byte buffer and to stop before publishing ssid2 when encoding fails.
- Added GNU89 ASan/UBSan regression for safe characters, space/percent escaping, UTF-8 bytes, maximum expansion, oversized SSID rejection, insufficient capacity and invalid pointers. Process workflow now runs the test and source integration checks; XY-C1 path trigger includes the policy header.
- Parent main 67e3c7a457ef9fbdd4684bcf0bf6e0a4609ee731 Actions: Process #45138662121 PASS, inventory #45138661934 PASS, AI docs #45138662283 PASS, NVRAM audit #45138662025 PASS, LLDT lock #45138661939 PASS; XY-C1 #45138662159 still running Build XY-C1 (TRX/artifact pending). No local test/build claim.

---

## 2026-11-23 UTC — Serialize LLDT PID-file check and publish

- Review found a check-then-act race in `osl_write_pidfile()`: it reads and validates the current PID file, closes that descriptor, then separately opens/truncates/writes the new PID. Two LLTD starts can both observe ENOENT or the same stale PID and both continue, overwriting the PID file while both daemons launch.
- Added a validated sidecar `<pidfile>.lock` using O_NOFOLLOW/O_NONBLOCK and a nonblocking exclusive `flock(LOCK_EX)`. The lock file must be a regular single-link file owned by the effective UID and not group/world writable. The descriptor remains open across both PID-file opens and is closed only after the full write path.
- Added a host regression using two independent open file descriptions, a source-order checker and a dedicated exact-SHA workflow. The sidecar is intentionally not unlinked; removing it could split lock ownership between concurrent openers.
- Parent SHA 880487bf61154786218115e6fd8448c07d6be6eb exact-head Actions observed: Process inventory and lifecycle fail-closed regression 45138112617: success; AI handoff documentation contract 45138112370: success. This slice awaits its own exact-head workflow; no local test/build claim.

---

## 2026-11-23 UTC — Match LLTD PID probe contract to saved errno

- Exact-head Process run #45137561990 on SHA 64c795f passed all earlier checks and LLTD PID regression, but the final source contract still expected the literal condition "if (errno != ESRCH)".
- Production saves errno immediately after kill(pid, 0), then tests "if (saved_errno != ESRCH)" so diagnostics and later operations cannot accidentally observe a clobbered errno. The source implementation is safer; the checker was stale. Updated the assertion to pin the saved_errno guard.
- Parent SHA b30898d Process #45138006556, AI #45138006583, NVRAM #45138006560, XY-C1 #45138006606 and inventory #45138006856 were queued/running at last poll; no local tests/build claim.

---

## 2026-11-23 UTC — Bound LLTD config tokens and terminate icon paths

- Source review found init_from_conf_file() used unbounded %s conversions into 256-byte var and val arrays. Both icon and jumbo-icon builders allocated enough space for /etc/ plus the value but copied only strlen(val) bytes with strncpy(), leaving each path without a NUL terminator.
- Replaced the scans with 255-byte token widths and explicit detection of a truncated value token; reject embedded NUL input. Both icon paths now use a shared capacity-aware helper that preserves the legacy relative /etc/ prefix and copies the NUL terminator.
- Added GNU89 ASan/UBSan tests for relative/absolute path compatibility, exact/short capacities, empty/NULL inputs and maximum-length values. Process source-contract workflow includes the new policy/test; XY-C1 path filters now include LLTD main.c and its policy header.
- Parent main SHA 64c795f904f42daa738598274d648e0c8c2d1a07 has exact-head Process/AI/NVRAM/XY Actions pending at this checkpoint; no local test/build claim.

---

## 2026-11-23 UTC — Make the LLTD PID pathname contract whitespace-independent

- Exact-head Process workflow #45137197616 on `20c664edf48d919600670a0f7893504c5dcc6a6a` passed the new LLTD PID-file GNU89 ASan/UBSan regression (step 17), then failed at the final source checker. The existing assertion matched the PID-file format string, comma and `g_Progname` argument as one line; the production writer had intentionally wrapped the function call across lines.
- Corrected the checker to first extract `osl_write_pidfile()`, then assert the pathname literal inside that function body. This keeps the actual path contract while removing formatting sensitivity; no production source changes.
- Same SHA AI docs #45137197736 and Process inventory #45137197649 PASS. XY-C1 #45137197654 and NVRAM audit #45137197675 remained in progress at last poll. No local test/build claim; exact-head reruns pending.

---

## 2026-11-23 UTC — Clear LLTD parser output on every invalid input

- Exact-head Process workflow #45136954916 on `b1a86530f54bd1d12b844e0198c46eb1b00e1b1f` compiled the GNU89 ASan/UBSan parser test successfully, then failed its NULL-text regression: the test requires a provided output PID to be reset to zero on parse failure, but the parser performed a combined `!text || !pid_out` guard before initializing the output.
- Fixed the contract by validating the output pointer first, clearing a valid output pointer immediately, then rejecting NULL input text. The test's invalid-input path now exercises the intended output-reset behavior.
- Same parent SHA Process inventory #45136955217 and AI handoff #45136955195 PASS. XY-C1 #45136955095 and NVRAM transaction audit #45136955054 were still in progress at last poll. No local test/build claim; exact-head reruns pending.

---

## 2026-11-23 UTC — Bound LLTD PID-file parsing and fail closed on uncertain PID probes

- `osl_write_pidfile()` previously read into a fixed buffer without NUL termination and passed it to `strtol()`; a full/partial record could trigger an out-of-bounds read or accept malformed trailing bytes.
- The old liveness check treated any failed `kill(pid, 0)` as stale. `EPERM` and transient errors therefore could overwrite a PID file for a process that had not been proven absent.
- Added a strict byte-count parser for positive decimal PIDs, checked overflow/zero/format, bounded read and EINTR handling, no-follow regular-file checks, safe pre-truncate validation, robust partial writes, and a fail-closed probe where only ESRCH means stale.
- Added a GNU89 ASan/UBSan policy test and Process workflow source contracts/path filters; XY-C1 filters now include the LLTD product source/header. Exact-head Process and AI Actions plus XY-C1 Build/TRX/artifact are pending; no local tests/builds claimed.
- Parent main SHA 96bfc395bf84bae9870ffa562cd90432cf7315f1: Watchdog signal #45136454686, AI docs #45136454674 and Process inventory #45136454600 PASS; XY-C1 #45136454673 and NVRAM audit #45136454583 were in progress.

---

## 2026-11-23 UTC — Make the Watchdog NTP policy header self-contained

- Exact-SHA Watchdog run 45136255417 on 4b1880a1a8ded23efa86641138e9ec65c2e6a5c1 failed during the GNU89 policy compile: the new inline helpers use size_t, strlen(), memcpy() and memmove(), but the header only included errno.h. With -Werror this correctly stopped the regression before executing the policy tests.
- Added the required standard <stddef.h> and <string.h> includes directly to watchdog_ntp_policy.h, so both production and standalone test translation units receive their declarations from the header contract.
- Same SHA AI docs 45136255104, Process inventory 45136255039, DNSMasq 45136255161 and HTTPD daemon lifecycle 45136255105 PASS. XY-C1 45136255075, NVRAM transaction audit 45136255131, Process lifecycle 45136255116 and HTTPD security 45136255046 were still IN_PROGRESS at last poll. No local test/build claim.

---

## 2026-11-23 UTC — Snapshot Watchdog NTP peers before stopping the current daemon

- Source review found refresh_ntp() killed the current ntpd before reading ntp_server0/ntp_server1 via nvram_safe_get(). Backend read/close errors were therefore indistinguishable from two absent values, causing a fallback to pool.ntp.org and replacement of the last-good daemon using an unverified configuration.
- Added a checked pair reader that copies each cache-backed NVRAM value before the next read, under one exclusive nvram_transaction lease. Missing keys preserve the legacy mirror/default behavior; backend errors, close errors and overlong values abort before kill_services().
- Server rotation now advances only when the one-shot ntpd launch succeeds. Added GNU89 regression cases for cache invalidation, second-key read failure, missing-key fallback and no-truncation handling; existing watchdog workflow already compiles/runs this test and its call-path checker.
- CA77 exact-SHA ZeroTier 45135591928, AI docs 45135591868, WAN route 45135591880 and Process 45135591888 all PASS. XY-C1 45135591873 remained IN_PROGRESS at the last poll (Build running, TRX/artifact pending). This NTP source slice awaits new exact-head checks; no local test/build claimed.

---

## 2026-11-23 UTC — Prevent Moon-management errors from leaving ZeroTier running

- start_instance() previously ran creat_moon()/remove_moon() after starting zerotier-one and publishing firewall/routes. If Moon identity creation/removal failed, the function returned failure while the background daemon and network state remained active.
- Moved the checked Moon operation after planet validation/import but before add_join(), daemon launch and rules(). Moon management only needs the prepared config directory and NVRAM, so failures now abort before a service process or firewall state is published.
- Added a source-order contract requiring Moon management before daemon launch and firewall setup. Exact-head ZeroTier/AI checks pending; no local shell test/build claim.
- Parent SHA 151cec5076e366fa17b6afef34c821a75f83e8d7: ZeroTier 45135317755 PASS; AI docs 45135317794 PASS; WAN route 45135317918 PASS; Process inventory 45135317776 PASS. XY-C1 45135317783 remained in progress at last poll.

---

## 2026-11-23 UTC — Fix ZeroTier planet-import hard-link rollback and source contract

- On exact SHA 5ef6145222006cac339a3053f0ff010217ca22ae, the bounded ZeroTier shell regression passed but the Python source contract failed because its initmoon assertion omitted the staged JSON redirection used by production code. Corrected the contract to check the actual invocation.
- The same shell run exposed another same-inode rollback warning in planet import: when the first publish rename is rejected, target planet and planet.previous are still hard links to the same inode, so mv fails and the cleanup trap unnecessarily preserves the .planet-import.* directory.
- Changed planet-import rollback to restore the hard-link snapshot with ln -f and added a regression requiring the prior planet/NVRAM/upload state with no leaked recovery directory after injected publish failure. The deliberately retained recovery directory after an uncertain NVRAM commit remains unchanged.
- Exact SHA 5ef6145: AI handoff 45135034495 PASS; WAN route 45135034518 PASS; Process inventory 45135034450 PASS; ZeroTier 45135034609 failed only the source checker after bounded shell regression PASS. XY-C1 45135034599 remained in progress at the latest poll (Build running; TRX/artifact pending). No firmware/L3 claim.

---

## 2026-11-23 UTC — Correct same-inode hard-link rollback and restore the dev log

- Exact-head ZeroTier run 45134470465 on 380a9a9f6eb42ceab7e57dcc4ea5068e49ba135e failed in second-file publication rollback. GNU mv refuses to move a hard-link backup over a target that already references the same inode; it left the private .moon-create.* directory behind.
- Exact-head AI docs run 45134470452 failed because the prior change replaced the 646-line development log with one entry. THIS commit restores the complete historical dev_log.md from parent 53811e3... and prepends the creator/fix entries instead of truncating the archive.
- Both Moon rollback helpers now restore snapshots with ln -f, which safely reattaches the saved inode when the target still refers to that inode. Added regression assertions for cleanup after creator publication failure and remover NVRAM set/commit/partial-delete failures.
- On 380a9a9..., WAN route 45134470540 and Process inventory 45134470473 passed. XY-C1 45134470507 remained in progress at last poll. No local test or firmware build claimed.

---

## 2026-11-23 UTC — Make ZeroTier Moon creation transactional

- Source review found `creat_moon()` ran `sed -i` inside command substitution rather than checking status, did not check `initmoon`, continued after endpoint-edit failure, ignored missing `genmoon` output, and did not check file/NVRAM publication results.
- Moon endpoint input is validated as IPv4 before interpolation. JSON and signature are produced in a private same-filesystem stage; endpoint insertion, generated 10-hex ID/signature, file modes and every publication operation are checked.
- Existing JSON/signature are hard-linked before replacement. Failed second-file publication or NVRAM set/commit triggers restoration of both previous files and the old `zerotiermoon_id`; incomplete rollback preserves the private recovery directory.
- Added shell regressions for invalid endpoint, initmoon/sed/genmoon failures, setter that writes then reports failure, second-file rename failure and successful generation. Exact-head ZeroTier/AI Actions pending; no local test/build claim.

---
## 2026-11-23 08:55 UTC — Make ZeroTier Moon removal rollback-safe

- Source review found `remove_moon()` deleted the signed file and JSON before clearing/committing `zerotiermoon_id`; NVRAM set/commit failure could leave a stale ID pointing at missing files.
- The remover now checks safe regular-file targets, snapshots existing file state with hard links in a private directory, clears and commits the ID before deleting files, and restores original ID/files if NVRAM set/commit or partial deletion fails.
- Added one-shot NVRAM set, commit and partial-rm failures; each regression checks that the prior Moon ID, signed file and JSON survive the failed operation.
- This slice is L1 source-reviewed; exact-head ZeroTier/AI/XY-C1 Actions pending. No local test/build claims.

---
## 2026-11-23 UTC — Rebind IPTV DHCP when mandatory address options are absent

- Review of `udhcpc_viptv_renew()` found an asymmetry with physical WAN: absent `subnet` or `ip` was treated as unchanged because comparisons used `value &&`. This could skip applying the bound-path default subnet mask after option withdrawal and could let a renewal missing its address return success without validation.
- Added a shared mandatory-option comparator and used it in both physical WAN and IPTV fast-renew paths. Missing `ip`/`subnet` now triggers full bound processing; optional router/route/DNS options retain their existing withdrawal behavior.
- Added GNU89 unit tests for equal, changed, missing and empty values, plus workflow source-contract assertions for helper semantics and both call sites.
- Exact-head WAN DHCP callback, AI handoff, and XY-C1 Actions are pending; no local tests/builds claimed.

---

## 2026-11-23 08:26 UTC — Bound Moon checker extraction around shell backticks

- Exact ZeroTier run `45132438035` on `d3f3aedd1b5a3d83dd6b178506ef59df0fff8ce9`: the new Moon lifecycle shell regression passed; the Python contract failed before semantic assertions because generic `function_body()` could not parse legacy backtick command substitutions in `creat_moon()`.
- Changed the checker to extract `creat_moon()` and `remove_moon()` using their explicit neighboring function boundaries. Production source and shell tests are unchanged.
- AI docs `45132437976`, Process inventory `45132437950`, WAN route `45132437933` and XY-C1 `45132438010` were still in progress at the last poll; XY Build running, TRX/artifact pending. Previous XY `45131246305` passed Build/TRX/artifact on the older `4025152...` source SHA, not the newer NAT/Moon changes.
- Exact-head source-contract rerun pending; no local test/build claims.

---

## 2026-11-23 08:18 UTC — Harden ZeroTier Moon creation and removal

- Remaining direct Moon NVRAM reads were `zerotiermoon_ip` and `zerotiermoon_id`. A legacy getter failure could be mistaken for an empty address (triggering WAN-IP fallback) or empty identity (skipping cleanup).
- `creat_moon()` now uses `nvram get_checked` and aborts before WAN-interface fallback on a read error.
- `remove_moon()` uses a checked ID read, treats absent ID as an idempotent no-op, validates the 10-hex ZeroTier address before constructing the signed-file path, checks file deletion and NVRAM set/commit results, and fixes the malformed condition that blocked removal of present IDs.
- `start_instance()` propagates Moon create/remove failures. Added shell regressions for failed IP/ID reads, no WAN fallback, unsafe ID, valid deletion and empty-ID no-op; Python source contracts pin read/path/return rules.
- L1 source review only; exact-head ZeroTier/AI Actions pending. No local tests/builds claimed.

---

## 2026-11-23 08:07 UTC — Check NAT validation in its helper, not rules()

- Exact ZeroTier run `45131839631` on `1b29ed2fe3ed96a310ac9e83cfda6b455b25e901`: shell lifecycle regression PASS; Python contract failed only because it still expected `case "$nat_enable"` validation inside `rules()`. Validation now correctly resides in `zerotier_read_nat_enabled()`.
- Updated the checker to assert rules calls the checked NAT helper and the extracted helper body contains the case validation, retaining the quoted NAT policy comparison. No production source changes.
- AI docs `45131839596` PASS on the same SHA. XY-C1 Build remained in progress on `4927303...` and `4025152...`; no L3 claim.
- Exact-head checker rerun pending; no local tests/builds claimed.

---

## 2026-11-23 08:03 UTC — Align ZeroTier NAT checker with checked-read implementation

- Exact ZeroTier run `45131686076` on production SHA `492730323b6361baa3a9351e487a9ea0f3e82eaa`: the bounded shell lifecycle regression passed; Python source contract then failed only because one assertion still expected inline `nvram get zerotier_nat`. The implementation now uses checked `zerotier_read_nat_enabled()` and checks route capture before daemon termination.
- Updated the checker to require the checked NAT helper, the NAT-enabled missing-route guard and their order before `kill_z()`. No production source or runtime behavior changed.
- Same-SHA Process inventory `45131685896`, WAN route `45131685948` and AI docs `45131685890` passed. XY-C1 `45131685922` was still in progress; Build was running, TRX/artifact pending. Prior XY run `45131246305` was also still in progress.
- Current checker/doc correction is committed as one complete tree; exact-head reruns pending. No local test/build claim.

---

## 2026-11-23 07:52 UTC — Check ZeroTier NAT, static routes and rollback snapshots

- Source review found additional legacy NVRAM reads in firewall publication, shutdown and static-route reconciliation. A failed `zerotier_nat` read looked like NAT disabled; failed route count/row reads looked like no configured route, potentially leaving an installed route while `stop_zero()` removed the persistent identity. Planet import's rollback snapshot could similarly collapse a read error to an empty old value.
- Added a checked NAT reader used by firewall publication/start/stop, a complete enabled-route preflight before daemon stop/restart, and checked route reads in add/delete (delete failures aggregate rather than being treated as absent configuration). Planet-import rollback now snapshots with `nvram get_checked`.
- Added injected checked-read failure coverage for NAT in rules/stop and for all static-route keys in add/start/stop. The Python contract pins checked reads and preflight-before-signal order.
- Exact-head ZeroTier, XY-C1, WAN route, Process and AI Actions pending at commit creation. No local test/build claim; L3 requires same-SHA Build/TRX/artifact.

---

## 2026-11-23 07:45 UTC — Run ZeroTier settings preflight before stopping the daemon

- Review of the checked-read gate found it was first called from `start_instance()`, but `start_zero()` stops the old daemon before calling that function. A read error would therefore preserve identity yet still take down a working ZeroTier service.
- Moved the five-key settings preflight to `start_zero()`, before `kill_z()` and firewall replay. `start_instance()` no longer performs a second potentially failing read after the old daemon is stopped.
- Added a regression that injects a `zerotier_secret` checked-read error and asserts the existing daemon remains running, no signal is sent, no network cleanup runs and no new instance launches. The source contract pins this ordering.
- Exact-head ZeroTier, XY-C1, AI docs, WAN route and Process runs from the prior commit were pending at the last poll; follow-up Actions are checked on this new source SHA. No local test/build claim.

---

## 2026-11-23 07:40 UTC — Fail closed on ZeroTier startup NVRAM read errors

- Source review found that `start_instance()` used legacy `nvram get` for `zerotier_secret` and four related startup selectors. The NVRAM CLI's legacy getter can return success with no output on a read error. A transient identity-secret read failure could therefore be interpreted as an empty key and trigger generation/publication of a different permanent ZeroTier identity.
- Added a five-key `nvram get_checked` preflight before config-directory, identity, planet, network-join or daemon side effects. Missing keys retain empty defaults, and no global selector is updated unless all five checked reads succeed.
- Extended `zerotier_lifecycle.sh` with each read-failure position, missing-key compatibility, successful snapshot publication and no partial global update; extended `check_zerotier_lifecycle.py` to pin read/publish and startup-side-effect ordering.
- Exact-head ZeroTier lifecycle, XY-C1 and AI docs runs are pending at commit creation. No local test/build claim; L3 requires same-SHA firmware build, TRX and artifact upload, and L4 remains NOT VERIFIED.

---

## 2026-11-23 07:32 UTC — Bind wireless radio checker to the loaded policy header

- Exact Wireless run `45129983626` for source `2d5f218...` failed at inline Python line 134 with `NameError: name 'policy' is not defined`. Earlier wireless lifecycle, SKU, bridge and guest-LAN source contracts printed PASS; the traceback was caused by the new snapshot assertions referencing an undeclared variable.
- The workflow loads `wireless_radio_policy.h` as `notification_policy`. Updated all three assertions to use that variable. This is a source-contract-only correction; the actual C transaction and tests are unchanged.
- On `2d5f218...`, AI docs, Process inventory/fail-closed, HTTPD security, iNIC signal, watchdog signal and NVRAM transaction audit were PASS; XY-C1 was still running. The checker correction requires a fresh exact-head Wireless run. No local host tests or firmware build are claimed.
- Parent `main`: `b2577fe95d0783d6a7168ab94237d582ea43bc6c`; result: THIS FOLLOW-UP.

---
## 2026-11-23 — Handoff checkpoint

- Live main SHA before this documentation commit: `2d5f218f7bbfcfb0f140be9c04f072173fe328c8`; backup branch SHA: `72f4f157a084f8184b9597ca5bf334b8080e5807` (untouched).
- Exact-head Actions lookup: `iNIC signal lifecycle stability regression` run `45129983697` = `completed/success`; `watchdog signal lifecycle stability regression` run `45129983726` = `completed/success`; `NVRAM transaction boundary audit` run `45129983737` = `completed/success`; `XY-C1 Firmware Validation` run `45129983650` = `in_progress`; `AI handoff documentation contract` run `45129983606` = `completed/success`; `Wireless radio state regression` run `45129983626` = `completed/failure`; `Process lifecycle inventory - 2d5f218f7bbfcfb0f140be9c04f072173fe328c8` run `45129983638` = `completed/success`; `Process inventory and lifecycle fail-closed regression` run `45129983589` = `completed/success`; `HTTPD Authentication and Security Regression` run `45129983625` = `completed/success`.
- Verified source work carried forward: checked timezone propagation in RC/PID1, console loglevel failure preservation, atomic Ralink SKU link publication, checked SYN-cookie WAN gate, transactional NAT helper selector read, transactional HNAT/SFE selector read.
- The next verification gate is a real XY-C1 Build + TRX + artifact on the integrated NAT/offload source tree; a green workflow with those steps skipped is not a firmware-build pass.
- First next task: Re-run/verify XY-C1 on the latest integrated source tree (the HNAT/SFE transactional selector and NAT-helper snapshot changes in net.c), and require Build XY-C1 + TRX verification + artifact upload to execute successfully; the previous checker-only exact-SHA run may report workflow success while skipping Build/TRX/artifact steps. Then move to the next independent P1 stability defect outside net.c.
- Handoff/documentation commit: pending.

---

## 2026-11-23 07:25 UTC — Restore exact previous wireless radio state on rollback

- Reviewing the new MLME power/publish helper exposed a state-restoration bug: it compensated with `!target_on`, which is not necessarily the previous state. If the radio was already ON and an ON-to-ON request failed while publishing NVRAM state, the rollback would turn the radio OFF.
- Added a `get_power` callback that reads the existing `mlme_radio_wl/rt` state using `nvram_get_checked()` and the strict wireless flag parser before any hardware change. A read/parse failure aborts the transition with no driver command. A later power or publication failure restores the exact cached prior state, not the inverse of the target.
- Added callback regressions for preflight read ordering, prior-ON restoration after a failed OFF transition, already-ON same-state publication failure, prior-OFF restoration after a failed power command, and read failure with zero hardware side effects. Extended the existing Wireless workflow's source contract; path filters already include the affected files.
- Parent `main`: `6e4c7f27cfad65fe437e88055e4f774d068ba437`. Its Firewall Startup/HTTPD/Process/USB modem/XY-C1 jobs were queued or running at last poll after the HW NAT contract correction. No local checkout, local test or firmware build is claimed; this source slice needs its own exact-head Actions.
- Resulting source/test/workflow/governance change: THIS COMMIT.

---
## 2026-11-23 07:20 UTC — Fix stale HW NAT selector assertions in shared CI contracts

- Exact `6deab04...` Actions showed several unrelated-looking workflow failures sharing one cause: `scripts/check_modem_wan_start_status.py` still expected `nvram_get_int("hw_nat_mode")` in `reload_nat_modules()`, although the source had moved to the checked `rc_nat_offload_settings_read()` snapshot and a local `hw_nat_mode` value. Firewall Startup, XY-C1, Process fail-closed and USB modem identity all stopped at that assertion.
- HTTPD security had a duplicate inline assertion in `.github/workflows/httpd-security.yml`; its job failed on the same obsolete expression. Wireless radio's larger inline checker reached its last group of checks and then failed at line 162 for the same HW NAT expectation after prior wireless, SKU, bridge and guest-LAN checks passed.
- Updated the shared Python checker to assert the checked snapshot call, local mode assignment and absence of the old unchecked getter. Updated the HTTPD inline contract identically. These are checker-only fixes; no production code behavior changed.
- `93bbadd...` exact follow-up runs Wireless `45128993444`, XY-C1 `45128993406` and AI docs `45128993523` were IN_PROGRESS at the last lookup. The checker fixes in THIS COMMIT need their own exact-head Actions. No local checkout, compile/test or firmware build is claimed.
- Parent SHA: `93bbadd21165a9458e97b166a47d4e0cfc1ab447`. Resulting atomic checker/governance commit: THIS COMMIT.

---
## 2026-11-23 07:15 UTC — Correct wireless MLME source-contract assertion

- A post-commit diff review found one stale assertion in `.github/workflows/wireless-radio-state.yml`: the MII publish callback now calls the status-returning `mlme_state_rt(!!is_on)`, but the checker expected the removed two-argument direct `nvram_set_int` call.
- Corrected the assertion to match the actual helper invocation. This follow-up changes no production source; it prevents a false-negative source-contract test and ensures CI checks the intended checked writer.
- Source implementation remains in `6deab04c8fe625abd785447a7a558d45eebf7766`. Exact-head Actions had not appeared at the last query; rerun on THIS COMMIT is required. No local test or firmware build is claimed.
- Resulting checker/handoff/roadmap update: THIS COMMIT.

---

## 2026-11-23 07:13 UTC — Propagate MLME radio-state publication failures

- The classic 5GHz and non-MII 2.4GHz MLME radio paths issued the driver `RadioOn` command, then called `mlme_state_*()` which discarded `nvram_set_int()` failure. LEDs and the reported success state could therefore advance while the cached runtime radio state remained stale.
- Both MLME state setters now return the NVRAM write status. A common callback-tested power/publish transition publishes state only after the driver command succeeds and best-effort restores the previous power state if either the power command or state publication fails. It preserves the primary error and reports rollback failure separately; LEDs update only after successful publication.
- RT3352 MII keeps its dedicated isolation-aware transaction, but the publish callback uses the same status-returning MLME state setter. Added GNU89 policy regressions for successful ordering, hardware failure, publication failure, rollback error precedence and incomplete callback tables; extended the wireless source-contract workflow.
- Parent `main`: `849edf14a54ed2b5f7b38b404b9efcbe34725ea0`. Parent AI docs and Process lifecycle inventory passed; WAN route, NVRAM audit, storage hotplug, Process umbrella and XY-C1 were still running, while Firewall Startup and HTTPD security had failed and require first-log triage. No local checkout/tests/build are claimed for THIS COMMIT.
- Resulting source/test/docs change: THIS COMMIT.

---

## 2026-11-23 07:10 UTC — Preserve HW NAT/SFE state when acceleration selectors cannot be read

- A second unsafe module-state decision remained in `reload_nat_modules()`: `is_hwnat_allow()` used unchecked `nvram_get_int("sw_mode")`/`nvram_get_int("hw_nat_mode")`; the function also read `ip6_lan_auto`, `hw_nat_mode` again for radio registration, and `sfe_enable` using unchecked getters. A transient backend/read/close error could be mistaken for a configured disable and unload active `hw_nat` or `fast_classifier`, or turn off IPv6 acceleration registration.
- Added a checked, transactionally copied acceleration-settings tuple. It preserves the old predicates (HW NAT allowed only in switch mode 1 or 4 and nonzero HW NAT mode; radio registration only when mode equals 1; IPv6 NAT and SFE enabled on integer value 1/nonzero as before), and retains the legacy zero defaults for missing keys. Any transaction/read/parse failure returns before any helper, HNAT or SFE module state is modified.
- Added GNU89/Werror/ASan/UBSan tests for all selector combinations, missing values, every member read failure, transaction-close failure, malformed/oversized values, AP/SFE-only selection and unchanged output on errors. Extended WAN source contract and WAN/XY-C1 path filters. First exact-head WAN run `45128521884` had no jobs because the workflow file lost its top-level `name:` and `on:` preamble while inserting new path filters. The same review caught that the shared offload policy result variables were declared only under `USE_HW_NAT`, although the reader is also compiled for `USE_SFE`-only targets. This follow-up restores the original workflow preamble and moves shared declarations under `USE_HW_NAT || USE_SFE`; exact-head WAN and XY-C1 checks rerun after this commit. No local tests or firmware build claimed.
- Parent `main`: `9e8e8ae58a0c7f0745ec0460da79c7a1296eb541`. Source/test commit: `c3894b095a0496e2fa73e0ced039c848d9f1e836`; workflow/build-guard correction: pending.

---

## 2026-11-23 07:04 UTC — Preserve NAT helper modules when NVRAM selector reads fail

- `reload_nat_modules()` read `wan_nat_x`, FTP helper ports and PPTP/H.323/SIP ALG flags using unchecked `nvram_get_int()`/`nvram_match()`. A transient NVRAM read/close error collapsed into disabled values; the reload could unload live NAT helpers even though persisted configuration still required them.
- The selector tuple is now copied under one NVRAM transaction and parsed only after the transaction closes. Any read, close, truncation or malformed integer error aborts before module state changes; the current helper set remains intact. Missing keys and valid settings retain legacy disabled defaults, exact string matching, FTP port-range fallback and AP-mode limited reads.
- Added GNU89/Werror/ASan/UBSan tests for coherent snapshots, missing defaults, every field read failure, close failure, malformed/oversized inputs and AP-mode behavior. A pre-run review caught that the fake NVRAM cache itself was the same width as the destination, which could truncate the oversized fixture before the production snapshot helper saw it; the test adapter now uses a wider cache so the boundary assertion exercises the real copy limit. Added source-contract CI and XY-C1 path filters. WAN route run `45127938295` compiled and passed all NAT sanitizer tests, then failed in the new Python checker because it parsed beyond the workflow filter header into the embedded job script (there is no `workflow_dispatch` boundary); the self-path was therefore counted again from the embedded path tuple. The checker now bounds parsing at `jobs:` before splitting push/PR filter lists. Exact-head rerun required. No local firmware build claimed.
- Parent `main`: `3a802b83e861018e020f3298ec7de9d41ef9f1bf`. Source/test commit: `0f1fc4ac7af800c219a480853803e6b43c86268f`; test fixture correction: pending.

---

## 2026-11-23 07:00 UTC — Reject out-of-range SYN-cookie policy values

- The checked-read fix for `fw_syn_cook` still passed the returned string to `atoi()`. An out-of-range persisted value can invoke undefined behavior; an implementation-dependent zero could bypass the explicit SYN-cookie WAN admission gate.
- The policy helper now uses base-10 `strtol()` and rejects `ERANGE` plus values outside `INT_MIN..INT_MAX`, leaving the secure-required output unchanged on failure. Representable values keep legacy `atoi()` prefix behavior; missing keys and nonnumeric strings remain zero.
- Added INT boundary, positive/negative overflow and very-long numeric regressions, and pinned the bounded conversion in `check_network_sysctl_status.py`. Existing WAN route workflow compiles and runs the GNU89/Werror/ASan/UBSan policy test, so no workflow filter change was needed.
- Parent `main`: `2e2af029329e422135bbbe0f376924beb2f7f251`. No local checkout, compile, host test or firmware build is claimed. Exact-head WAN/Process/NVRAM/XY-C1 checks are pending.
- Resulting commit: THIS COMMIT.

---
## 2026-11-23 06:55 UTC — Fail closed on SYN-cookie policy read errors during sysctl reload

- `restart_all_sysctl()` used `nvram_get_int("fw_syn_cook")`; a transient NVRAM read/close failure became integer zero. The reload then successfully wrote `tcp_syncookies=0`, and the existing WAN-admission gate treated the value as an explicit user disable, allowing the active WAN to remain up with SYN-cookie protection off.
- Reload now uses `nvram_get_checked()` and an explicit parse policy. A missing key and present values preserve legacy `atoi()`/boolean behavior. A backend error keeps the initialized secure-required state, records the exact read error as SYN-cookie status, skips the sysctl write, and reaches the existing WAN-stop gate before PPPoE passthrough can be restarted.
- Added GNU89/Werror/ASan/UBSan tests for present/missing/legacy numeric parsing, backend errors, invalid API results and unchanged secure-required state. Extended the source integration contract and XY-C1 path filters so Ralink SKU publication changes are also firmware-build inputs. Exact-head regression and XY-C1 Build/TRX/artifact checks pending; no local build claimed.
- Resulting commit: pending.

---

## 2026-11-23 06:50 UTC — Extend checked timezone read to PID 1 recovery shell

- Found a second use of `nvram_safe_get("time_zone_x")` in `init.c:run_shell()`. Unlike the RC process environment update, this path builds the rescue shell's explicit `envp`; a transient NVRAM fault could pass a false empty TZ value into PID 1's recovery console.
- Reused `rc_timezone_env_format_status()` after `time_zone_x_mapping()`. Missing keys keep legacy `TZ=`; backend/close errors, invalid results, null-present data or truncation omit the TZ entry from the rescue shell environment rather than publishing fabricated data.
- Extended the existing policy integration checker/workflow to pin both RC and PID 1 recovery-shell call paths. This production change touches `init.c`, so exact-head XY-C1 Build/TRX/artifact and timezone Actions will validate the combined tree. Pending; no local build claimed.
- Resulting commit: pending.

---

## 2026-11-23 06:45 UTC — Make Ralink regulatory SKU-link publication atomic and checked

- `symlink_sku_file()` previously unlinked the live driver SKU path before selecting a replacement, ignored `symlink()` failures, and returned `void`. A filesystem/link error could therefore remove the existing regulatory profile while allowing wireless configuration generation to report success. The NVRAM `regspec_code` was also interpolated into a path without validation against the regulatory table.
- The helper now distinguishes a missing candidate from real `stat()` failures, validates NVRAM region codes before path construction, and publishes a selected candidate through a same-filesystem private staging directory plus atomic `rename()`. When no candidate exists, it retains the existing intentional remove behavior but propagates non-ENOENT unlink errors. Both band generators discard their staged DAT config and fail when SKU link selection/publication fails.
- Added GNU89/Werror/ASan/UBSan filesystem regressions for live-link replacement, failed publication preserving the destination, staging cleanup and optional-unlink errors. The wireless workflow compiles/runs the regression and checks source callsites. Exact-head Wireless/Process/XY-C1 validation pending; no local checkout or firmware build claimed.
- Resulting commit: pending.

---

## 2026-11-23 06:37 UTC — Retain kernel console logging on NVRAM read failure

- `nvram_restore_defaults()` previously passed `nvram_get_int("console_loglevel")` directly to `klogctl(8)`. A transient NVRAM backend/read/close failure was collapsed to zero, suppressing all kernel console output just before later boot checks could report faults.
- Boot now distinguishes present, missing and failed reads. Present values keep legacy `atoi()` conversion; a missing key keeps the legacy zero default; a backend error skips `klogctl` and retains the kernel's current default level. The syscall result is also logged if applying the requested setting fails.
- Added GNU89/Werror/ASan/UBSan policy coverage and an exact source/workflow contract. Host GNU89/ASan/UBSan test passed on initial commit. Console integration checker run `45125348511` failed because the checker retained spaces inside a multiline function call while matching a normalized string. First follow-up `71ae21e6777d0382c23ec14d54ad60e6564ec645` removed whitespace from source, then review caught one stale spaced `klogctl` assertion before Actions could complete. This follow-up normalizes that last assertion; exact checker rerun pending. No local checkout or firmware build claimed.
- Source commit under validation: `a18a2e0148ec861719a5836fab454ee9a0ed785c`; checker-only corrections: `71ae21e6777d0382c23ec14d54ad60e6564ec645` and current follow-up pending.

---

## 2026-11-23 06:32 UTC — Preserve timezone environment when NVRAM reads fail

- `setenv_tz()` previously used `nvram_safe_get("time_zone_x")`, which collapses NVRAM backend/read/close failures into an empty value and unconditionally replaces the process TZ. A transient fault could make PID 1 and standalone RC applets lose the configured local timezone. The PID 1 recovery-shell path in `init.c` had the same error-collapsing read and could fabricate `TZ=` in the rescue shell.
- The RC path now uses `nvram_get_checked()` and a bounded GNU89 policy helper. Missing keys retain the legacy empty-TZ behavior; backend errors, unexpected API results, null present values and overlong strings leave the existing process environment unchanged. `setenv()` copies the value, avoiding mutation of the prior environment through a shared `putenv()` buffer.
- Added GNU89/Werror/ASan/UBSan regression coverage for present/absent keys, backend failures, invalid results, exact buffer limits and output preservation, plus a source/workflow integration contract.
- Exact-head timezone regression, AI handoff documentation and XY-C1 Build/TRX/artifact checks had passed for the RC-only slice; recovery-shell extension validation pending. No local checkout, shell test or firmware build claimed.
- Resulting commit: pending.

---

## 2026-11-23 06:31 UTC — Scope deferred storage startup initialization check to USE_STORAGE branch

- Exact USB storage hotplug run `45123777839` on `5d8a220b7e16375d0308704c061e066d4304b513` passed the whole-caller-graph step, then failed in `scripts/check_storage_safe_remove.py` at the deferred-worker assertion. The checker counted `storage_should_start = 0` across both preprocessor branches; the function intentionally declares that local once in the `USE_STORAGE` branch and once in the non-integrated-storage branch, so the global count of one was invalid.
- Corrected the source contract to count the initialization inside the `#if defined (USE_STORAGE)` declaration branch only. Retained the ordering assertion before the first checked unplug read, the dispatch-error gate, retry alarm, and start gate; no production behavior or assertion coverage was removed.
- The first correction run `45124098843` still failed this assertion because the extracted declaration branch had not yet been whitespace-normalized. Commit `3d048175aff0e01c0dc27b67401aed051da89494` normalizes the branch before counting; exact USB storage run `45124463284` and AI handoff run `45124463323` both PASS.
- XY-C1 run `45124463354` returned workflow success but skipped Build XY-C1, TRX verification and artifact upload because this checker-only commit did not change a production firmware input. It is not L3 evidence.
- No local checkout or shell test claimed. Resulting checker-only commit: `3d048175aff0e01c0dc27b67401aed051da89494`.

---

## 2026-11-23 06:26 UTC — Move MiniDLNA assertions outside the Process launch loop

- Commit `1d90559f4ca90a168942b1334763022ce40f30b9` added checked `apps_dms` assertions to the Process integration checker but inserted them inside the generic storage-launch loop before its remaining indented checks, breaking the Python block structure.
- Relocated MiniDLNA-only assertions after the generic loop while preserving checks for the checked string read, nonzero-status return before capability checks, `strcmp()` disabled gate, and no legacy getter. Generic launch result/process-presence assertions remain within the loop for all four services.
- The exact 1d Process run `45121324507` was still running at last query; no validation claim. The prior 1b run `45121083737` failed at the stale guard. New exact-head Process rerun is required.
- Exact 1b MiniDLNA `45121083876`, Firewall Startup `45121083736`, Process inventory `45121083732`, HTTPD `45121083771`, Firefly `45121083818`, FTP `45121083826`, Samba `45121083776`, IPv6 `45121083819`, NFS `45121083745`, libdisk `45121083812`, and AI handoff `45121083832` PASS. XY-C1 `45121083755` was in Build with TRX/artifact pending.
- No local Python/test/build claimed. Resulting commit: THIS COMMIT.

---
## 2026-11-23 06:10 UTC — Check MiniDLNA enable and identity reads before startup

- After hardening dlna_rescan, reviewed the remaining run_dms() launch NVRAM inputs. nvram_match("apps_dms", "1") swallowed backend errors as 'disabled', dropping an enabled media-server start; nvram_safe_get("il0macaddr") swallowed identity-read errors and could start with a zero serial/MAC.
- Replaced both with storage_read_string_status() before DB-link/config side effects. Missing apps_dms remains a false/disabled default, missing il0macaddr remains empty and preserves old formatting, while read/close/truncation errors now abort with the exact status.
- Preserved conditional dlna_rescan read for !force_rescan. Extended CI source contract to require all status checks before side effects and prohibit the old lossy getters.
- Commit 507d80d8901131f75259a3b1f8d8f7837605fa3c exact MiniDLNA workflow 45120721550 PASS and AI handoff 45120721566 PASS; XY-C1 45120721414 was building with TRX/artifact pending. This new production change requires exact-head reruns.
- No local compile/build claimed. Resulting commit: THIS COMMIT.

---
## 2026-11-23 06:02 UTC — Fail closed when MiniDLNA cannot read database rescan mode

- run_dms() used nvram_get_int("dlna_rescan") after publishing configuration. Transient backend/read/parse/close errors collapsed to zero and silently skipped a configured full (-R) or update-only (-U) scan while starting the daemon.
- Replaced that lossy getter with storage_read_integer_status() before any DB-link unlink/recreation or config publication. The exact error is logged/returned with no startup side effects. force_rescan bypasses the read because the explicit caller mode supersedes the setting.
- Extended MiniDLNA workflow source contract to pin the checked read, forced-scan bypass, failure gate before link mutation, preservation of -R/-U behavior and removal of the lossy getter.
- MiniDLNA fixture correction 511f1eed74144fb270181dbb53de2ea9dc09be02 exact run 45120292969 PASS. Checker/handoff commit 9597283bfa55eaa16d2b64212e073935e421dedf exact MiniDLNA run 45120463448 PASS and AI handoff contract 45120463437 PASS.
- eebc production tree Firewall Startup 45120141334 PASS. Its XY-C1 run 45120141302 had passed prerequisites and was at Build XY-C1; TRX/artifact pending. This production source change requires its own exact-SHA firmware build validation.
- No local compile/build claimed. Resulting commit: THIS COMMIT.

---
## 2026-11-23 05:55 UTC — Pin MiniDLNA newline-injection fixture semantics

- Exact MiniDLNA run `45120141433` failed the ASan/UBSan host test at line 124 because all malicious inputs encoded two backslashes before `n`; they were literal text, not control bytes, so the expected EINVAL did not occur.
- Concurrent commit `511f1eed74144fb270181dbb53de2ea9dc09be02` replaced the doubled escapes with one C newline escape, including the last-good configuration fixture. Exact rerun `45120292969` completed SUCCESS: both the GNU89 sanitizer test and transactional source contract passed.
- Added source-contract checks to require the exact four malicious C literals, so each has one C LF escape and can't silently regress to backslash+n. Test run 45120292969 verifies the corrected fixture content; a fresh source-contract run on THIS commit is required.
- Exact eebc production-source Firewall Startup `45120141334` PASS. eebc XY-C1 `45120141302` had passed prerequisites and was building; TRX/artifact remained pending. Other eebc process/IPv6/Firefly/vsftpd/libdisk/USB/HTTPD/Samba/NVRAM jobs were pending/in progress.
- No local checkout or local build claimed. Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:45 UTC+8 — Validate VPN server subnet text before deriving NAT network

- Audited the new checked `vpns_vnet` snapshot in `ipt_nat_rules()`. Although backend errors, transaction-close errors and overlong strings already fail closed, any short malformed IPv4 token still reached legacy `ip2class()`, which calls `inet_addr()` without checking its parse result and can derive a bogus network from `INADDR_NONE`.
- Added `firewall_nvram_ipv4_address_is_valid()` using strict `inet_pton(AF_INET)`; invalid/missing VPN server pool text now returns EINVAL before IPv4 NAT staging. Existing bounded checked copy remains first.
- Extended the existing GNU89/ASan/UBSan firewall NVRAM policy test for valid dotted quads and NULL/empty/short/CIDR/out-of-range/leading-zero/trailing-junk inputs. Extended the Firewall Startup source contract to pin validation before both staged-file open and `ip2class()`.
- Prior Firewall mangle transaction regression exact run `45119408526` PASS, including all source and sanitizer tests. FTP mode-gating exact run `45119601925` PASS. Firefly credential safety `45119820496` PASS. The current pre-change d02 XY-C1 `45119820478` was still running; this production firewall source change requires an exact-head XY-C1 build/TRX/artifact and fresh Firewall Startup.
- No local checkout or local build claimed. Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:35 UTC+8 — Avoid reading anonymous FTP rate when the active mode does not use it

- Reviewed concurrent vsftpd hardening from `b330a2f96cc15730b591c8fd5e49a7a63b4bb5a4`. The checked `st_ftp_anmr` read correctly prevented backend errors from becoming `anon_max_rate=0` (unlimited), but it was unconditional: modes 1/2 do not emit or consume `anon_max_rate`, yet any rate-key read fault prevented configuration publication.
- Added `ftp_transfer_rate_setting_required()` and moved normalized FTP-mode selection before the anonymous-rate snapshot. The checked rate read and clamp now run only for modes 3/4, which actually output `anon_max_rate`. Read faults in relevant modes still preserve the active config and fail closed.
- Extended the existing GNU89 ASan/UBSan policy regression to assert modes 3/4 require the setting while modes 1/2 and unsupported modes do not. Updated the workflow source contract to require mode normalization before the guarded rate read and registration of predicate tests.
- Firewall transaction fix `2c911df08141a3810f7d53cf8d0eaf1536e96a88` achieved exact L2 PASS on `1a99a9c543d237398294ef5738622f151c8d7f8e`, Firewall Startup run `45119408526`: source checker plus all compiled GNU89 sanitizer and integration checks succeeded. XY-C1 run `45119331092` on b330 had passed detection and entered Build; no TRX/artifact result was yet available. Exact new-SHA firmware proof still required.
- Concurrent FTP checked-rate fix's original vsftpd action `45119331064` PASS; this mode-gated refinement needs a new exact-SHA vsftpd rerun. No local checkout/test/build claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:26 UTC+8 — Fail closed when IPv4 mangle NVRAM snapshots cannot be read

- Source review found that `ipt_mangle_rules()` returned on errors reading `wan_ttl_fix`, `wan_ttl_value`, conditional `mr_enable_x` and conditional `viptv_ifname` without setting `firewall_core_restore_transaction.failed`. The caller could then continue to NAT/filter and finish with stale mangle rules mixed into the newly restored firewall.
- Added `firewall_mangle_snapshot_failure_status()` to set the transaction failure flag and normalize negative return values to EIO. A source helper logs the failed setting, preserves errno and gates all four preflight reads before side effects; the caller's existing transaction-abort path rolls prior table restores back.
- Added GNU89 sanitizer-policy regressions for success preserving transaction state, positive backend errors, negative status and invalid transaction pointers. The source contract checks all four gates and caller abort-before-NAT.
- Exact Firewall Startup run `45118876044` on 5191 failed only at a literal-whitespace assertion for the new VPN subnet checked-read gate; checker-only commit `4278a6905bc7c30c53690892ed0cb6f2a603bcbf` repaired that assertion and exact run `45119079720` passed. This commit carries that checker correction forward and adds the independent mangle transaction fix.
- Concurrent VPN server multicast failure propagation from `1c5bf5f02bc006484692c592ce5bdc6fd587f06c` is retained. Exact new-SHA Firewall Startup and XY-C1 Build/TRX/artifact are required.
- No local checkout; no local compile/build is claimed. Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:12 UTC+8 — Add missing DHCPv6 source paths to XY-C1 firmware validation

- Exact WAN callback run `45118077270` on contract-correction commit `84110056136496c2f7bcce4034b8450dc4c121a1` completed SUCCESS. Dedicated DHCPv6 sanitizer/source-contract run `45117871967` on product source `80811fc53de0257bf44973fa0d703014981fcaf5` also completed SUCCESS.
- The XY-C1 workflow already included its own workflow file in push paths, but omitted production `dhcp6c.c`, `dhcp6c.h`, `dhcp6c_script.c` and the new retry policy header. Its only related existing path was `dhcp6c_common.c`; thus the DHCPv6 product change did not itself launch firmware validation.
- Added those four DHCPv6 client/source/header paths to the XY-C1 push filter. This commit changes CI trigger coverage only, not product C source.
- While preparing this change, main advanced with commit `3eba7fc820f0c3f9dc2dd7a80db313b39f63fff1` (`fix(firewall): check IPTV interface before TTL rules`). Preserved it as the new parent. Its exact XY-C1 run `45118311710` was in progress at the last lookup after prerequisites; this tree includes DHCPv6 source commit 80811, but no L3 claim is made until exact same-tree Build/TRX/artifact completion.
- Earlier e757, d2c and USB-printer slices have their own exact-SHA XY-C1 Build/TRX/artifact SUCCESS; those do not replace this DHCPv6 target validation. No local checkout or build.
- Resulting commit: THIS COMMIT.

---
## 2026-11-23 14:06 UTC+8 — Align WAN callback source contract with DHCPv6 event retry

- Exact DHCPv6 retry workflow run `45117871967` on `80811fc53de0257bf44973fa0d703014981fcaf5` completed SUCCESS: GNU89 ASan/UBSan policy test and the DHCPv6 source integration checker both passed.
- The same-SHA WAN DHCP callback workflow `45117872004` completed FAILURE only at its final inline Python source contract. Its legacy assertions still required the old DHCPv6 log strings `configuration script failed to execute` / `exited with status %d`; the source fix deliberately replaces them with explicit `retrying event` log messages and a retained-event retry path. All preceding WAN/IPv6/LAN policy binaries and source contracts in that job passed.
- Updated the WAN workflow's stale DHCPv6 callback assertions to require the retry-policy gate, both new diagnostics, retransmission reset and five-second event timer. This is a test-contract migration only; no product C source changed.
- No exact-head XY-C1 run appeared in the returned push-run inventory for SHA 80811 at the latest lookup. The dedicated DHCPv6 host workflow passed, but L3 is still NOT VERIFIED until the same production-source SHA gets XY-C1 MIPS/uClibc build, TRX and artifact upload.
- No local checkout; no local test/build claim. Resulting commit: THIS COMMIT.

---
## 2026-11-23 14:00 UTC+8 — Retry DHCPv6 Reply events when configuration scripts fail

- Confirmed active production path: `start_dhcp6c()` in `trunk/user/rc/src/net_wan6.c` launches `/sbin/dhcp6c`; BusyBox Kbuild includes `dhcp6c.o`, `dhcp6c_script.o` and `dhcp6c_ia.o` under CONFIG_DHCP6C. The legacy `d6_dhcpc.c` is not this router's DHCPv6 client and was not modified.
- Defect: `dhcp6_script()` in `dhcp6c_script.c` returns the real `spawn_and_wait()` result, but `client6_recvreply()` in `dhcp6c.c` only logged a failed callback and unconditionally removed the DHCP event. It also armed Information Refresh Time before the script callback completed. A transient callback failure could discard the DHCP retry event and delay restoration of IPv6 DNS/NVRAM state until a later exchange.
- Fix: initialize script status to success when no callback is configured; on a nonzero status retain the current DHCP event, reset its retransmission parameters using `dhcp6_set_timeoparam()`, schedule the event timer for 5 seconds and return before `dhcp6_remove_event()`. The Information Refresh Time timer is now armed only after callback success. The existing child environment cleanup path remains unchanged.
- Added a policy header and GNU89 host regression for success, positive child exit status, spawn failure and retry interval. Added a Python source contract verifying the callback result, event retention, retry timer and Information Refresh Time ordering. A dedicated workflow compiles/runs the policy under ASan/UBSan and executes the source contract.
- Previous source slices now have complete verification: USB printer unplug commit `20012786541a3789f0391793345e403eb96c77d8` passed storage-hotplug host/sanitizer/source checks in run `45116268933` and exact-SHA XY-C1 build/TRX/artifact in run `45116268960`; FRP source `d2c388a04f6125666043bf94672a8f4222655d4b` passed exact-SHA XY-C1 run `45115500761`; FRP checker correction `41285d396d7bd1f984f13c49699ed6dacbd27e92` passed run `45116127095`.
- No local checkout is available; no local compile or test is claimed. The dedicated DHCPv6 host workflow and exact-new-SHA XY-C1 MIPS/uClibc build, TRX and artifact upload are required before L2/L3 closure. L4 router validation remains unavailable.
- Source, policy, test, checker, workflow and this handoff/log update form one atomic slice. Resulting commit: THIS COMMIT.

---
## 2026-11-23 13:43 UTC+8 — Preserve USB printer unplug retries when spooler shutdown fails

- Source review found a concrete event-loss defect in on_deferred_hotplug_dev(): the usb_unplug_lp branch called stop_usb_printer_spoolers() but discarded its result, then passed action_status=0 to usb_hotplug_event_ack_status() and acknowledged the NVRAM event. services_usb.c confirms that stop_usb_printer_spoolers() aggregates u2ec/lpd/p910nd stop errors and returns a nonzero errno-style status; therefore an un-stopped daemon could be left behind with no unplug retry queued.
- The caller now initializes printer_unplug_status, captures the stop result, skips marker clearing when the stop failed, and passes the real action/marker statuses through usb_hotplug_event_ack_status(). Failed stop remains action-retryable; successful stop plus failed marker clear latches completion so retries only clear the marker and do not repeat the side effect. Existing usblp-in-use policy remains a successful no-op.
- Added a positive EBUSY regression to usb_hotplug_event_policy.c. Extended check_storage_safe_remove.py to require stop-status propagation, marker-clear gating, the action-latch call and retry scheduling. The existing storage-hotplug workflow already compiles the policy test under GNU89 ASan/UBSan and runs the source checker.
- At the latest check, e757 XY-C1 run 45114634020 completed SUCCESS (L3 build/TRX/artifact on that e757 production-source SHA). d2c XY-C1 run 45115500761 was still building on d2c; it does not validate the newer printer source. f932 Process lifecycle 45115896497 passed; f932 FRP 45115896479 exposed a second stale source assertion, which is fixed in parent commit 41285d396d7bd1f984f13c49699ed6dacbd27e92. Its exact-head FRP rerun 45116127095 was in progress at the last lookup; f932 handoff validator 45115896495 and the other previously noted jobs passed.
- No local checkout, so no local host test was run. Require exact new-SHA storage-hotplug regression and XY-C1 target build/TRX/artifact for this source slice. No L4 claim.
- Source, regression and source contract are one logical change with the handoff/log update. Resulting commit: THIS COMMIT.

---
## 2026-11-23 13:34 UTC+8 — Remove the remaining stale FRP inline-copy source assertion

- Exact-head Process lifecycle run 45115896497 on f9320f888b7ac284a4b657b1ae58feca40f0428f completed SUCCESS. This validates the updated scripts/check_dev_init_boot.py and all process lifecycle regression steps, including the FRP credential seeding shell cases.
- Exact-head FRP lifecycle run 45115896479 on the same SHA completed FAILURE in a second, later assertion in scripts/check_frp_lifecycle.py at line 285. The initial checker block had been fixed, but this second assertion still required the removed inline FRP chmod 700 and symlink/regular-file tests rather than the shared seed_storage_file() contract. Every FRP shell regression before that source contract passed.
- Updated the remaining assertion to require the FRP helper invocation, forbid the retired direct cp -f, and assert the shared optional private-mode enforcement branch. This is checker-only; production source is unchanged.
- f932 exact AliyunDrive WebDAV, HTTPD authentication/security and handoff validator runs 45115896556, 45115896513 and 45115896495 completed SUCCESS. The only known f932 failure is this stale FRP checker block; its exact-head rerun is required.
- d2c XY-C1 run 45115500761 and e757 XY-C1 run 45114634020 remain to be inspected for final Build/TRX/artifact conclusions. No local checkout; no local execution claim.
- Corrective checker commit: THIS COMMIT.

---
## 2026-11-23 13:27 UTC+8 — Update FRP source contracts for shared private seeding helper

- Exact-head FRP lifecycle run 45115500757 on product/source SHA d2c388a04f6125666043bf94672a8f4222655d4b completed FAILURE only in scripts/check_frp_lifecycle.py:130: it still required the removed direct cp -f/chmod/-L/-f source block. All prior FRP shell regressions, including private runtime config, exact process stop, managed crontab and lifecycle lock, passed.
- Exact-head Process lifecycle run 45115500771 on the same SHA completed FAILURE only in scripts/check_dev_init_boot.py:315: it still required an inline copyscripts_seed_status=1 assignment inside the FRP branch. The actual shared helper sets the status for every failure; copyscripts_ca_status.sh passed, including the new FRP mode-0700 creation, existing-file mode repair, injected chmod failure and no-old-cp source contract.
- Updated both source contracts to require the common seed_storage_file() call with the enforce-existing-mode flag, two guarded chmod paths and the absence of the old direct FRP cp. This is a checker-only correction; production source remains unchanged.
- Exact XY-C1 run 45115500761 on d2c388a04f6125666043bf94672a8f4222655d4b had passed the firmware input detector and all pre-build host checks; Build XY-C1 was still in progress at the last lookup; TRX and artifact upload were pending. Exact handoff validator 45115500743, inventory 45115500720, Adbyby host config 45115500738 and AliDDNS API transport 45115500805 all passed on d2c.
- No local checkout. No local checker execution. Exact new-SHA FRP and Process Actions must pass before the checker migration is closed. No new L3/L4 claim.
- Corrective checker commit: THIS COMMIT.

---
## 2026-11-23 13:14 UTC+8 — Route FRP credential seeding through exclusive private publication

- Source review found that copyscripts.sh already used seed_storage_file() for ordinary defaults, which stages beside the destination and publishes with an exclusive hard link to avoid following a leaf or parent symlink. The credential-bearing FRP script was a security-sensitive exception: a separate symlink check followed by cp -f had a check/use gap and duplicated path/type/mode handling.
- Extended the shared helper with an optional enforce-existing-mode flag. FRP now uses the same staged/exclusive publication path at mode 0700; an existing operator-managed regular file keeps its content but has its private mode restored on each boot. A concurrently published regular file retains its content while still receiving the requested private mode. Invalid mode policy and chmod failures propagate through the existing boot-seed status.
- Expanded trunk/user/scripts/tests/copyscripts_ca_status.sh for private creation, existing-file content preservation plus mode repair, mode-repair failure, and source wiring that forbids the old direct FRP cp path. The Process lifecycle workflow already watches both changed paths and runs this test; no workflow filter change is needed.
- The preceding libdisk ACL lookup source SHA e757f86309b19f890797b2f36fb15f44f87a7e95 passed its descendant source-contract rerun 45114769360 on 5378fccc2201f4359ea3b0b37f1043d2b813a883. Its exact XY-C1 run 45114634020 is IN_PROGRESS; the NVRAM boundary run 45114633929 is COMPLETED.
- No local checkout is available, so the shell regression was not run locally. The exact-head Process lifecycle Action must validate it. No L3/L4 claim.
- Product source and shell regression changed together. Resulting commit: THIS COMMIT.

---
## 2026-11-23 12:07 UTC+8 — Repair Python syntax in PID1 mask-failure source checker

- Exact-head BusyBox job 137478257344 / run 45109466477 on SHA 81be0de7b68467468789d7a67c590b5eb737a449 compiled and ran both ASan/UBSan C binaries successfully, then failed in scripts/check_busybox_init_reap_queue.py at line 127: JavaScript const/indexOf syntax had been inserted in a Python assertion block.
- Replaced the malformed block with Python index/find/slicing assertions that require failed signal-mask restoration to requeue the batch and invoke pause_and_low_level_reboot(RB_HALT_SYSTEM), with no return to the main loop.
- AI handoff 45109466503 and Process inventory 45109466511 PASS on 81be. USB and NVRAM were still running; XY-C1 45109466474 was in Build XY-C1 with TRX/artifact pending. Exact XY 9de run 45108415640 and ca609 run 45109162782 also remained in Build at their last matching polls; neither is recorded as failed. The earlier curl error 56 log was from a different/unmatched job and is not attributed to them.
- Exact-child wait/reap L3 remains verified on 7c556be. Delayed-signal source L3 remains pending until a same-SHA Build/TRX/artifact result passes.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:02 UTC+8 — Fail closed if PID 1 cannot restore delayed signal mask; correct CI status attribution

- Source review found that check_delayed_sigs() requeued a snapshot when sigprocmask(SIG_SETMASK, &old_set, NULL) failed, but then returned to the main loop. The delayed signal set may still be blocked, leaving PID 1 deaf to reboot/shutdown requests. The failure branch now requeues the captured events and calls the existing NORETURN pause_and_low_level_reboot(RB_HALT_SYSTEM) rather than continue.
- Extended scripts/check_busybox_init_reap_queue.py to require the mask-restore failure branch to retain the pending events and invoke the fatal halt path, and to require the NORETURN declaration. The focused BusyBox source-contract workflow will exercise it on this commit.
- Corrected an earlier CI attribution error in SESSION_HANDOFF.md and REFACTOR_ROADMAP.md: exact XY run 45108415640 on 9de was still IN_PROGRESS at its last matching job lookup (job 137476531958 was at Build XY-C1); it was not confirmed failed. The curl error 56 log came from another unmatched job and should not be attributed to that run.
- Exact ca609 run 45109162782: detector, new atomic queue-header regression and production-input detection PASS; Build XY-C1 IN_PROGRESS, TRX/artifact PENDING at the last poll. Exact AI docs 45109162785 and Process inventory 45109162805 PASS. No L3 claim for delayed-signal source.
- The queue-header helper remains a defensive retry/atomic-publication improvement. Its regression PASS on the exact ca609 workflow; this change is not represented as a repair for a proven 9de failure.
- Resulting commit: THIS COMMIT.
---

## 2026-11-23 11:53 UTC+8 — Make XY-C1 sys/queue.h fetch retryable and atomic

- Exact XY-C1 run 45108415640 on main SHA 9de5f37d550244590158b302b6ac9935bff765b3 failed before firmware source compilation: the root Makefile sys/queue.h curl returned error 56, "Recv failure: Connection reset by peer", and Makefile exited from prepare-headers. Toolchain download succeeded; this is a transient prerequisite transport failure, not a compiler diagnosis.
- Added scripts/install_musl_queue_header.sh and routed the root Makefile through it. Curl retries all errors (five retries with bounded timeouts); output goes to a unique temporary file in the same directory, empty responses are rejected, mode is normalized to 0644, and a complete file is atomically renamed into place. Failed/truncated transfers cannot replace an existing valid header; -s makes an empty stale file trigger a fresh fetch.
- Added scripts/tests/test_install_musl_queue_header.py. It starts a local HTTP server that returns a truncated first response and a valid second response, verifies retry and exact final content, tests partial/empty failed downloads preserve an existing header and leave no temporary files, and checks Makefile/workflow wiring.
- Added root Makefile, helper and regression paths to XY-C1 push filters and a dedicated test step. The root Makefile change is a firmware production input, so this exact commit should retry Build XY-C1, TRX and artifact upload for the delayed-signal PID1 source.
- Exact 9de checks: BusyBox PID1 45108415639 PASS; AI docs 45108415662 PASS; Process inventory 45108415598 PASS; USB 45108415607 PASS; NVRAM 45108415605 PASS. XY-C1 45108415640 is the sole known failure and failed at header download, before compilation.
- Local installer regression passed with curl 8.10.1 using a real truncated HTTP response and subsequent retry. New exact-head Actions and L3 for the signal accumulator remain pending.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:49 UTC+8 — Correct PID1 delayed-signal local declaration scope

- Immediate source review after commit 352c47fef554d64ad8c536d1ed8227a3af261d96 found signal_error declared inside the conditional DEBUG_SEGV_HANDLER block, while the ordinary init signal setup uses it. This would leave the default production build with an undeclared identifier.
- Moved the declaration into the ordinary signal setup block and removed it from the debug-only block. The code is checked for same-block declaration/use before creating this commit.
- The standalone new signal accumulator tests passed under GNU89 with -Wall -Wextra -Werror and ASan/UBSan. This policy test does not compile the full BusyBox init.c; exact-head BusyBox and XY-C1 Actions must verify that translation unit and target build.
- The exact-child wait/reap baseline already has L3 evidence: XY-C1 run 45106983817 on 7c556be passed Build, TRX and artifact upload. The signal accumulator change itself is not yet L2/L3 verified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:47 UTC+8 — Preserve concurrent PID1 delayed-signal requests

- Source review confirmed a lost-event race in check_delayed_sigs(): the main loop snapshots the single bb_got_signal value and then clears it, so a different delayed handler can overwrite that global between those operations. The source comment already acknowledged SIGHUP/Ctrl-Alt-Del collision; a shutdown request could also be overwritten by another delayed event.
- Added init_signal_pending.h with independent event bits and a retained shutdown signal; init.c installs PID1-only delayed handlers with a full signal mask, snapshots and clears pending events while the delayed signal set is blocked, processes shutdown before restart/service actions, and requeues a taken batch if restoring the prior mask fails. The general libbb bb_got_signal path is retained for SIGCONT in stop_handler().
- Added GNU89 ASan/UBSan tests for distinct events, coalescing, shutdown mode, snapshot restore and invalid inputs; extended the source-contract script and BusyBox workflow to compile/run them. A local GNU89 -Wall -Wextra -Werror ASan/UBSan compile and execution of the standalone policy test passed.
- The earlier exact-child source change already has L3 evidence: XY-C1 run 45106983817 on 7c556be passed Build XY-C1, TRX verification and artifact upload; compare 1f9d39bc..7c556be contains only workflow, test, script and log paths after the production source.
- Handoff CI is being repaired alongside this source slice: run 45107256439 first found the missing mandatory checkpoint sentence; exact-head run 45107711807 on 056edb68 then found the additional required phrases one atomic main commit and Before context exhaustion. Added the exact rules without changing the validator. Exact Actions for this source/test commit remain pending.
- Resulting commit: THIS COMMIT. New signal-fix L2/L3 and hardware L4 are not yet claimed.

---

## 2026-11-23 11:42 UTC+8 — Restore the explicit end-of-session handoff checkpoint contract

- The exact-head AI handoff documentation job 45107256439 on 8331df0c690aafa00a7d46263dacea8ab8bfbb01 failed at its validator with: current handoff does not require an end-of-session accuracy checkpoint. The log and roadmap were not the cause.
- Root cause: the refreshed SESSION_HANDOFF.md explained the checkpoint process but omitted the exact required sentence, “The checkpoint is mandatory; a document edit is conditional.” Added that statement and the explicit end-of-session live-state comparison rule. The validator is unchanged; this is a genuine handoff contract correction.
- On the same 8331 SHA, BusyBox PID1 regression 45107256450, Process inventory 45107256374, USB hotplug 45107256428 and NVRAM audit 45107256356 all PASS. XY-C1 host/detector run 45107256442 PASS and deliberately skipped Build/TRX/artifact because the commit changed only tests/docs.
- Exact XY-C1 workflow-only run 45106983817 on 7c556be was still in Build XY-C1 at the latest check; TRX and artifact upload remained pending. L3 is not claimed; L4 is not verified.
- This standalone docs-only governance fix repairs the exact-head mandatory documentation contract failure. Product source is unchanged. Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:32 UTC+8 — Run XY-C1 build after repairing firmware input detector tests

- The workflow-trigger change at `11090e80b126fa1665758026d2163fd9e2f09e6a` correctly queued XY-C1 validation, but its detector unit test failed because the first fixture version used double-escaped Git delimiters. The corrected fixture commit `a71253cc463befcab865afb8a90407afa7975e33` passed the full XY-C1 workflow, including the detector tests; Build XY-C1/TRX/artifact steps were skipped because that commit changed only host tests and dev log.
- Trigger the corrected validation again by clarifying the workflow's own comment that BusyBox init is production code and requires XY-C1 rebuilds. The path filter already includes `trunk/user/busybox/busybox-1.24.x/init/**`, and the detector now treats edits to this workflow as build inputs. This next exact-SHA run should therefore pass tests, detect the workflow change, and run the full build/TRX/artifact stages against the latest BusyBox PID1 source.
- Parent main SHA: `a71253cc463befcab865afb8a90407afa7975e33`. BusyBox PID1 regression, USB storage hotplug and process-lifecycle inventory have each passed exact-head host validation on the current source tree; NVRAM audit passed on the previous exact head `84060ba4f52823b396487b24cdfe66254baff0d0`.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:22 UTC+8 — Correct NUL and newline fixtures in XY-C1 workflow-only detector test

- The exact-head XY-C1 workflow trigger correction at parent `11090e80b126fa1665758026d2163fd9e2f09e6a` started run `45106741184`; code review of the new detector regression found its mocked Git output encoded backslash characters rather than actual newline/NUL delimiters. That would have exercised the fail-safe invalid-SHA path instead of proving the targeted workflow-only diff behavior.
- Corrected the test fixture literals so `rev-parse` returns a normal newline-terminated SHA and `git diff -z` returns a NUL-terminated path. Production detector and workflow configuration are unchanged; the current workflow run is still used to validate its full firmware build path, while this successor will assert the unit-test fixture accurately.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:18 UTC+8 — Trigger XY-C1 firmware validation for BusyBox PID1 production changes

- Exact-head BusyBox PID1 regression run `45106412054` on parent `84060ba4f52823b396487b24cdfe66254baff0d0` passed the GNU89 ASan/UBSan test binary and full source-contract checker. The USB storage hotplug suite `45106411983` also passed on the same SHA; NVRAM audit `45106412065` was still in progress at this checkpoint.
- The successful BusyBox fix had not started an XY-C1 build: the firmware workflow trigger list omitted `trunk/user/busybox/busybox-1.24.x/init/**`, even though it already tracked BusyBox UDHCP sources. Added the init subtree to the path filter. Also classified edits to the XY-C1 workflow itself as build-triggering inputs; otherwise workflow edits are discarded by the detector's blanket `.github/**` host-only rule. Added tests that distinguish production `init.c`/policy header from host-only init tests and assert a workflow-only push triggers validation. Kept ordinary documentation-only pushes build-free.
- Parent BusyBox change SHA `1f9d39bc59f9882b9f0d0efae2f09e908110a1ac` is covered by exact-head regression PASS on follow-up `84060ba4f52823b396487b24cdfe66254baff0d0`; this workflow correction is intended to cause the next exact-head commit to run Build XY-C1, TRX verification and artifact upload. The current commit's L3 state remains pending until those steps complete.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:06 UTC+8 — Match PID1 wait regression declarations to the source-contract style

- Exact-head BusyBox PID1 run `45106301790` on `1f9d39bc59f9882b9f0d0efae2f09e908110a1ac` compiled and ran the GNU89 ASan/UBSan test binary successfully, then the Python contract failed because its test-presence assertion expects the repository's existing single-line `static void test_name(void)` declaration form. The five new functions were formatted with the return type and name split across lines.
- Normalized those five test declarations to the existing test-file style. This is a checker-compatibility-only correction; helper and production behavior are unchanged. Re-running all exact-head checks on the successor commit.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:02 UTC+8 — Wait for the exact BusyBox PID1 action child

- In BusyBox `init.c`, `waitfor(pid)` used `kill(pid, 0)` followed by unqualified `wait(NULL)`. The PID existence probe is vulnerable to PID reuse between probe and wait; if that reused PID belongs to an unrelated live process, init can block on that process instead of its completed action child. The unqualified wait also consumed arbitrary children while waiting for the target.
- Added `init_wait_for_child()` to target `waitpid(pid, NULL, 0)` specifically, retry EINTR, treat ECHILD as the signal-handler-already-reaped case (the queued PID is drained immediately afterward), and preserve other failures. `waitfor()` now marks the exact reaped child, opportunistically drains only already-exited peers with `waitpid(-1, ..., WNOHANG)`, and never blocks on an unrelated child. Added five GNU89 callback regressions for exact PID/options, EINTR retry, queued ECHILD, error propagation and invalid arguments; source contracts reject the old kill/wait sequence.
- Parent main SHA: `e3d9935d58d255d089392a1c8861779a2b6b1be9`. No local full BusyBox toolchain or target init binary was available; host regression and source contract are pending the exact-head Actions run. This change does not claim router PID1 boot validation or hardware behavior.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:39 UTC+8 — Update the final delete-folder contract for extracted transaction policy

- Exact-head libdisk run `45105379349` on `f7ab8fb2f2ccce29e11687f8b5cd5d8134c499e0` passed both GNU89 ASan/UBSan test binaries, including all five newly-added function-level delete transaction tests, then failed a final legacy assertion that still required `disk_share_permission_batch_apply()`, rollback, and `rmdir(staging_path)` to appear directly inside `del_folder()`.
- The new implementation intentionally delegates these steps to `disk_share_folder_delete_batch()`; the stronger source contract already pins its move → refresh → ACL apply → staged remove → ACL rollback → directory restore → snapshot refresh ordering and all tests. Replaced the obsolete wrapper-level assertion with a check for preflight, staging-path creation, injected callbacks, and safe staging-parent cleanup. No product C or regression behavior changed.
- Resulting commit: THIS COMMIT. Root `dev_log.md` now has 119 dated records (within the 20–119 protocol window); do not append another record without rotating/archive-preserving first.

---

## 2026-11-23 12:34 UTC+8 — Exercise the complete folder deletion transaction with fault injection

- Parent main SHA: `b08863f2dc42c06abf1b9765b8ca42f69025fd7b`. The rename path already had injected callback tests, but `del_folder()` still embedded directory staging, snapshot refresh, ACL publication, staged-directory removal and rollback directly in its public wrapper. That left the delete path's rollback combinations untested at policy level.
- Extracted `disk_share_folder_delete_batch()` with injected move/remove/refresh/file-writer callbacks, then routed `del_folder()` through it. The helper preserves the staging directory when a failed rename-back leaves data there, always attempts to refresh the visible snapshot after compensation, and returns the original operation errno unless compensation is incomplete (then EIO). Added five GNU89 state-machine tests: successful delete; initial snapshot failure; partial ACL write failure; staged-directory removal failure; and failed directory compensation with explicit staged-data retention. Updated source contracts to pin operation/rollback ordering.
- Parent exact-head libdisk run `45105209634` is in progress at this update; earlier `e16fc458` exact-head run `45105008310` passed both sanitizer binaries but failed the first source contract because its normalizer preserved spaces. The next exact-head run will validate the corrected contract and these new tests. No L3/L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:25 UTC+8 — Update mod_folder source contract for no-follow directory checks

- Exact-head libdisk run `45105123378` on `e94bdc2ddb2defee0fe5e5edb3d2d298a9b7f7bb` passed both GNU89 ASan/UBSan regression binaries, then failed the source contract at a second stale assertion: it still expected `test_if_dir(full_path)` in `mod_folder()`. The symlink hardening commit correctly replaced that check with `lstat()` plus `disk_share_folder_entry_status()`.
- Updated the ordering contract to require the no-follow check before the cached folder lookup and stale-snapshot refresh. The earlier fresh-enumeration contract also now matches the checker's space-normalized source representation. This is test-only; no product C code changed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:22 UTC+8 — Correct the lstat source-contract whitespace locator

- Exact-head libdisk run `45105008310` on `e16fc458f434cdb097d27e17bda7daaf9f245dec` passed both GNU89 ASan/UBSan C test binaries, including the new temporary-directory symlink fixture, then failed the Python source contract. The source code itself used `lstat(testdir, &entry_status)`; the checker compared against `lstat(testdir,&entry_status)` even though `compact_all_folders` joins tokens with spaces.
- Corrected the source-contract token to match its normalized representation. No product source or test behavior changed in this checker-only correction; the full exact-head libdisk workflow must pass on the successor SHA before this slice is considered L2-verified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:19 UTC+8 — Reject symlinked directories from the libdisk share inventory

- Audited Samba's private-share builder: it consumes `get_folder_list_in_mount_path()` and constructs `path = mount_point/folder_name`. Both the fresh scanner and persisted folder-list cache previously accepted directory symlinks (the scanner used `opendir()`, and the cache trusted stored names), which could expose a path outside the intended removable-volume subtree. `mod_folder()` also used a symlink-following directory check before rename.
- Added a shared `lstat()` status policy. Fresh scans now accept only real directories; cached names are revalidated and compacted before publication, skipping missing/stale/symlink/non-directory entries while failing closed on other stat errors. `mod_folder()` now applies the same no-follow gate before renaming. The GNU89 regression builds a real directory and symlink and checks real-dir accept, symlink reject, ENOENT skip, and EACCES/unknown-error fail-closed semantics. Source contracts pin all three call paths.
- Parent main SHA `a835089b13114527c9acafe5edab3fddaa046725`. Parent libdisk run `45104677566` on `cdc137f84a2d5c704198184d5b014d18b3ffe937` passed both ASan/UBSan C regression binaries but failed its final checker on a stale direct-rename locator; checker-only fix `a835089b13114527c9acafe5edab3fddaa046725` updated the transaction helper assertion. This code change triggers a fresh exact-head libdisk run. NVRAM audit `45104677474` and XY-C1 `45104677571` on `cdc137f` were still in progress at last poll. No L3/L4 result claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:14 UTC+8 — Align libdisk source contract with folder rename transaction helper

- Exact-head libdisk run `45104677566` on parent `cdc137f84a2d5c704198184d5b014d18b3ffe937` passed both GNU89 ASan/UBSan regression binaries, including the new four-case function-level folder-rename state-machine tests. Its final Python source contract failed with `ValueError: substring not found` because an older assertion still searched `mod_folder()` for the direct `rename(full_path,new_full_path)` call after commit `cdc137f84a2d5c704198184d5b014d18b3ffe937` delegated that operation to `disk_share_folder_rename_batch()`.
- Updated the stale ordering assertion to locate the transaction-helper call after stale-cache refresh and reject the removed direct-rename form. This is a checker-only correction; no product C source changed. Parent NVRAM and XY-C1 exact-head runs remain in progress at this update; no L3/L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:05 UTC+8 — Restore required handoff section heading after documentation CI failure

- Current parent main SHA: `768fc390acf8ada74c935a9096c67308b15937b0`; backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head watchdog run `45104441871` completed PASS after the C whitespace-normalized source dispatch assertions were introduced. All runtime policy sanitizer tests and source wiring checks passed.
- The documentation contract run `45104441878` failed because the prior handoff rewrite renamed a required heading from `## Active task` to `## Active work`. The section content is intact; this commit restores the protocol-required heading without changing the active engineering record.
- XY-C1 run `45104213876`: all host checks PASS, Build XY-C1 still IN_PROGRESS, TRX and artifact upload PENDING. NVRAM audit `45104213886`: all transaction/locking regressions PASS, raw ioctl/kernel caller audit IN_PROGRESS. Process inventory `45104213913` PASS. HTTPD authentication `45104213964` PASS. DNSMasq lifecycle `45104213901` PASS; HTTPD daemon lifecycle `45104213888` PASS.
- No firmware L3 or hardware L4 claim. dev_log count after this entry must remain below 120.

---

## 2026-11-23 11:45 UTC+8 — Fix watchdog signal lifecycle source checker whitespace matching

- Exact-head watchdog run `45104213963` on `5892a4229e9683979bc1e97fca71b2e8528d2413` passed GNU89 ASan/UBSan signal deferral, NTP/service lifecycle, and CPU-watchdog postcondition regressions, then failed the final Python source-wiring step with `ValueError: substring not found`.
- Root cause: the embedded Python assertions searched for literal escaped newline/tab sequences in `watchdog_main()`, although the C source has ordinary formatting whitespace. The same ordering assertions now operate on `"".join(main.split())` and compare all offsets in that normalized view.
- The adjacent libdisk follow-up `1ccfaed9ba04feaa5c1d6802e6708d788e16f953` fixed a live-directory rollback bug in `add_folder()`; exact-head libdisk run `45104177783` passed.
- Parent SHA `5892a4229e9683979bc1e97fca71b2e8528d2413`: XY-C1 run `45104213876` host stages PASS / Build IN_PROGRESS / TRX and artifact upload PENDING; NVRAM audit `45104213886` policy and lock regressions PASS / raw caller audit IN_PROGRESS; HTTPD auth `45104213964` PASS; Process inventory `45104213913` IN_PROGRESS; DNSMasq lifecycle `45104213901`, HTTPD daemon lifecycle `45104213888`, and handoff contract `45104213904` PASS.
- This correction changes only the workflow's source-contract assertion and the handoff/log status, not watchdog runtime code. No L3 or hardware L4 validation claimed.
- dev_log now has ${(log.match(/^## \d{4}-\d{2}-\d{2}/gm)||[]).length+1} dated records; it remains below the 120-record rotation threshold.

---

## 2026-11-23 11:35 UTC+8 — Verify PID1 exact-child waiting against real child reaping

- Parent main SHA: 7c556be9eb1caa343d720bcecd3df74c5461262a. The existing PID1 callback tests exercised injected waitpid results but did not prove the OS behavior that motivated the fix: a blocking wait for one action child must not consume an unrelated exited child.
- Added test_wait_for_exact_child_does_not_reap_peer() to trunk/user/busybox/busybox-1.24.x/init/tests/init_reap_queue.c. It creates a peer child and waits for its exited-but-unreaped state with waitid(WNOWAIT), then starts a delayed target child and calls init_wait_for_child(target_pid, waitpid, ...). The test confirms the peer remains waitable and the target has been reaped. Registered the case in the source contract.
- An isolated GNU89 -Wall -Wextra -Werror ASan/UBSan harness for the real fork/waitid scenario passed 10/10 locally. The full repository binary and source contract await this commit's exact-head BusyBox Actions.
- Exact predecessor evidence: BusyBox regression 45106412054, Process inventory 45106412151, AI docs 45106411973, USB hotplug 45106411983, and NVRAM audit 45106412065 PASS on 84060ba4f52823b396487b24cdfe66254baff0d0. XY workflow unit regression 45106798326 and AI docs 45106798279 PASS on a71253cc; its target build was correctly skipped because that SHA only changed tests/docs.
- The workflow-only XY-C1 run 45106983817 on 7c556be is still building; its terminal result is required before the PID1 L3 state can be claimed. No L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:08 UTC+8 — Preserve ACLs when failed folder creation leaves a live directory

- Parent main SHA: `256d736d8483227b5df3e26c074394ae4071786d`. In `add_folder()`, failure cleanup called `rmdir(full_path)`; if another writer populated the directory before rollback, removal failed but the old `else if (permission_init_started)` branch still deleted that folder's permission records from all accounts. The directory remained live while its ACL metadata was removed.
- Added `disk_share_folder_rollback_can_remove_permissions()`: ACL-row compensation is allowed only when the directory was removed or already absent (`ENOENT`). If it survives (for example `ENOTEMPTY`, `EBUSY` or `EACCES`), preserve its ACL rows, report incomplete rollback, and refresh the visible folder snapshot. Added policy regressions for success, ENOENT, ENOTEMPTY, EBUSY and EACCES, and pinned guard ordering in the source contract.
- Parent exact-head libdisk run `45103728364` on `7bd561845032ed68084e53c609da8c0a22655a6a`: PASS (GNU89 ASan/UBSan host regressions and source contract). NVRAM audit `45103585419` on product SHA `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e`: PASS. XY-C1 run `45103585439` on that product SHA remains IN_PROGRESS at Build XY-C1; TRX and artifact upload are PENDING. This commit's exact-head Actions are pending. No local compile, L3 build or L4 hardware result is claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:20 UTC+8 — Make folder permission changes compensating and staging creation exclusive

- `0e83450aa686d745f80547604bd4f08479a43538` added a reusable per-file permission batch with original-content snapshots, reverse-order rollback and retryable compensation. `del_folder()` now stages its directory and only removes it after all permission updates succeed; failures restore the directory and folder snapshot. Added success, later-write failure and rollback-failure/retry policy regressions.
- `f00ed9b4ba735b1943887ccd95b4dcda75255e88` reused the batch for `mod_folder()`, precomputing old-to-new ACL contents before the directory rename and restoring exact original file images on failure.
- `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e` removed the staging-path check/use race by reserving a hidden parent through exclusive `mkdir()` semantics before moving the folder. It retries only `EEXIST`; tests cover collision retry, hard creation errors and path truncation.
- Checker-only descendants `44e3a626bebf255b8a08eb212a62c5fb07ebe74b` and `7bd561845032ed68084e53c609da8c0a22655a6a` repaired stale ACL-pruner/source locators. Exact current-head libdisk run `45103728364`: PASS (both GNU89 sanitizer binaries and source contracts).
- Product-source XY-C1 run `45103585439`: host checks PASS; Build XY-C1 IN_PROGRESS; TRX and artifact upload PENDING. NVRAM boundary audit `45103585419`: policy/locking regressions PASS, raw ioctl/kernel caller audit IN_PROGRESS. HTTPD `45103585429` and Process inventory `45103585424`: PASS.
- The root log had 109 dated records before this entry and now has 110, below the 120-record rotation threshold. No shard rotation performed.
- Next: finish exact product-source build/TRX/upload and NVRAM audit status; then add function-level filesystem failure injection for folder delete/rename compensation.

---

## 2026-11-23 09:13 UTC+8 — Make PID 1 recovery-shell respawn throttling independent of pre-exec latency

- Confirmed the old timer in `init_main_loop()` started before `run_shell(0, 1)` and stopped only after the child exited. This measured fork + signal reset + console attach + timezone setup + shell runtime, not shell runtime itself. Slow pre-exec setup could incorrectly make a repeatedly failing shell appear to have lived at least two seconds, removing the throttle and allowing a rapid restart loop.
- Source commit `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c` removes the misleading uptime timestamp/threshold. Every reaped recovery-shell child now gets a one-second throttle; the existing wait predicate still cancels the delay for a pending shutdown. Updated the policy regression, exact source contract, and its file list for the console policy header/test.
- Exact same-SHA PID 1 mdev/security `45093300454` PASS (including Build/run PID 1 console recovery retry regression and source contract); Process lifecycle `45093300665` PASS; AI docs `45093300644` PASS. PID 1 notification queue `45093300520`, DNSMasq `45093300505`, DNS forwarder `45093300535`, HTTPD `45093300612`, process inventory `45093300456` PASS.
- XY-C1 `45093300583` host checks PASS and Build XY-C1 IN_PROGRESS; TRX verification and artifact upload not yet run. NVRAM audit `45093300474` and USB hotplug `45093300506` still in progress at the last poll. Do not claim L3/L4.
- Resulting source commit: `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`.

---

## 2026-11-23 09:04 UTC+8 — Restore the 20–119 record development-log root window

- Parent main SHA `1b4d7138730431ecfc7d317adc4d8f4efa027ddd` had only one dated record in root `dev_log.md`, violating the validator's rolling-window contract. The prior 100-record root snapshot remains at parent `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`; restored it verbatim and added eight subsequent dated records. Root now contains 108 complete records and stays below the 120-record rotation threshold. No archived shard was changed.
- Parent `a4e7170d` MiniUPnP regression `45092211012` PASS and Process lifecycle regression `45092211001` PASS. Parent `1b4d7138` AI docs run `45092323908` failed because root log count was one; re-run exact docs contract on this repaired window.
- XY-C1 `45091972678` remains IN_PROGRESS at Build for product SHA `4ecc2877`; TRX/artifact not yet verified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 09:03 UTC+8 — Restore the cross-session execution mandate

- Parent `a4e7170d` AI docs run `45092211047` exposed the missing literal `# Cross-session execution mandate` section. Restored the required continuous-work loop, one atomic main commit per logical slice, non-forced compare-and-swap, no repeated user authorization and a first next action.
- The new AI handoff run `45092323908` passed the cross-session section check and exposed the independent root log retention defect (one entry vs. minimum 20). The log-window repair is in the next record.
- Parent MiniUPnP `45092211012` and Process lifecycle `45092211001` both PASS.
- Resulting commit: `1b4d7138730431ecfc7d317adc4d8f4efa027ddd`.

---

## 2026-11-23 09:01 UTC+8 — Normalize C call whitespace in MiniUPnP and Process contracts

- Parent `70d487002e68cb169b4faae75e9ff902235c2699` still failed the MiniUPnP inline contract because whitespace-joining left a space after the multiline call opening parenthesis; Process failed the equivalent check in `check_process_lifecycle_integration.py`.
- Removed all whitespace before comparing the LAN address/netmask/MAC checked-copy calls and their argument order. Exact-head MiniUPnP `45092211012` PASS and Process lifecycle `45092211001` PASS on `a4e7170d826cf6b77cea4393122dcdfa1d7efdd9`.
- No product source changed in this checker-only commit; the earlier failures were source-contract string matching, not GNU89/ASan regression failures.
- Resulting commit: `a4e7170d826cf6b77cea4393122dcdfa1d7efdd9`.

---

## 2026-11-23 09:00 UTC+8 — Align MiniUPnP source contract with checked LAN snapshots

- Parent product SHA `4ecc28778b3605743262514dbce53b7ffbebda2d` changed `start_upnp_locked()` to use checked owned LAN snapshots. The inline MiniUPnP workflow still searched for removed direct `nvram_safe_get()` assignments, producing `ValueError: substring not found`.
- Replaced the stale source-order assertions with checked-get/copy helper checks, ordered address/netmask/MAC snapshots, validation, subnet derivation and config publication ordering, plus explicit rejection of unsafe raw getters. A first run then exposed a remaining whitespace mismatch, corrected by the following `a4e7170d` commit.
- Resulting commit: `70d487002e68cb169b4faae75e9ff902235c2699`.

---

## 2026-11-23 09:00 UTC+8 — Restore the mandatory handoff accuracy checkpoint

- Parent `4ecc28778b3605743262514dbce53b7ffbebda2d` required an accurate end-of-session checkpoint; added the mandatory heading and exact conditional-edit rule to `SESSION_HANDOFF.md`.
- Exact AI docs run `45092102706` then failed at the next required literal section, `# Cross-session execution mandate`; the next docs correction restores it.
- Resulting commit: `030c39e578e83ee690f156f6549c3cf77618c791`.

---

## 2026-11-23 08:59 UTC+8 — Snapshot checked LAN identity before MiniUPnP config publication

- Commit `4ecc28778b3605743262514dbce53b7ffbebda2d` replaces rotating-cache pointers for `lan_ipaddr` and `lan_netmask`, plus an unchecked `lan_hwaddr` read, with checked owned copies before IP validation, subnet derivation and config publication. Any read/close/overflow error aborts startup before publishing configuration.
- Added WAN NVRAM policy regression cases for cache-slot overwrite and failure at each of the three LAN identity reads. Exact Samba, WAN, DNSMasq, HTTPD and NVRAM host checks on this source SHA passed; initial MiniUPnP/Process failures were stale/whitespace-sensitive checker contracts, repaired later. XY-C1 remained in progress at the last poll.
- Resulting source commit: `4ecc28778b3605743262514dbce53b7ffbebda2d`.

---

## 2026-11-23 08:56 UTC+8 — Watch MTK ESW kernel inputs in the XY-C1 workflow

- Commit `7dd05bffdb09a6aecb68480131303d00e857bee2` adds the MTK ESW ioctl driver, policy header and test paths to the XY-C1 workflow push filter and source contract. Switch CLI host regression `45091738973` PASS; XY-C1 `45091739058` completed with workflow SUCCESS but Build/TRX/artifact steps SKIPPED because this commit only changed workflow/docs. Same-SHA kernel Build/TRX/artifact remains NOT VERIFIED.
- Exact AI handoff `45091738990` exposed missing mandatory handoff checkpoint wording; restored in `030c39e5`.
- Resulting commit: `7dd05bffdb09a6aecb68480131303d00e857bee2`.

---

## 2026-11-23 08:55 UTC+8 — Preserve checked Samba workgroup and WINS values

- Commit `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` changes `write_smb_conf_header()` to copy `st_samba_workgroup`, hostname and conditional `dhcp_wins_x` into owned buffers through checked NVRAM reads. This prevents later NVRAM lookups from overwriting cache-backed string pointers and aborts staged smb.conf publication on read/close/length failures.
- Added regression coverage for sequential workgroup/WINS reads and preservation on WINS read failure. Exact Samba host regression `45091634196` PASS; Process umbrella `45091634176` initially failed only on a multiline whitespace-sensitive Samba source assertion, fixed in `b803d7a5`.
- Resulting source commit: `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`.

---

