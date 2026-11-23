## 2026-11-23 — Roll back late USB printer daemons after failed readiness

- Follow-up reliability defect: when U2EC/LPRng `execv()` succeeded but the bounded RUNNING postcondition timed out, was interrupted, or encountered an unknown process inventory, the caller returned failure without stopping a daemon that could appear late. A pipe read/partial-status error also left launch state uncertain.
- The printer launcher now uses the shared `detached_daemon_start_rollback_with_ops()` policy on readiness and post-fork status-channel failures. It sends an identity-validated stop through `kill_services()`, takes a fresh process snapshot, logs confirmed absence versus incomplete rollback, and preserves the primary startup errno. A definitive full exec-error record still returns directly because no daemon was started.
- The existing GNU89 rollback-policy regression already covers successful no-op, stop, final ABSENT/RUNNING/UNKNOWN and primary-error preservation; USB-printer and Process source contracts now pin this integration. Exact-head Actions pending.

## 2026-11-24 01:00 CST (UTC+8) — Restore mandatory handoff accuracy checkpoint after AI docs failure

- Exact AI documentation run #45221089290 on `e4d34fd7f13baa203bbbae90130b0782a9b22e5c` failed with: “current handoff does not require an end-of-session accuracy checkpoint”. The handoff had been compacted but omitted the validator-required `Mandatory handoff checkpoint` section and exact conditional-edit sentence, along with its required cross-session heading and section names.
- THIS commit restores the mandatory checkpoint contract, including “The checkpoint is mandatory; a document edit is conditional.”, live-state inspection, materiality/no-op guidance, cross-session atomic-main rules, and the four required handoff section headings. No product source changed.
- USB modem checker PASS remains #45220509149 on f947; Process, NVRAM and Storage PASS remain #45220191260/#45220191315/#45220191366 on ecff. Exact AI docs rerun on THIS handoff correction is pending; XY-C1 runs #45220033426 (e6dc), #45220191233 (ecff), and #45221089272 (e4d34) were still in progress at last observation.

---

## 2026-11-24 00:53 CST (UTC+8) — Trigger XY-C1 build on WAN ARP policy header changes

- `detect_wan.c` directly includes `detect_wan_arp_socket_policy.h`, but the XY-C1 push path allowlist listed only the C file. A header-only production change could bypass firmware cross-build/TRX validation.
- Added the production policy header beside `detect_wan.c` in `.github/workflows/xy-c1-firmware-validation.yml`. The workflow remains main-only/push-only; its event model was not changed.
- USB printer NVRAM checker correction landed in `f9475bf081db66e795df8c25332253ef7c6c80a2`; exact USB modem run #45220509149 PASS, including the previously failing source-contract step. Process fail-closed #45220191260, NVRAM audit #45220191315 and USB storage #45220191366 PASS on `ecff475abb782b32e0a8f4f02026acc709ea6844`.
- XY-C1 #45220033426 (`e6dc19c`) and #45220191233 (`ecff475`) were still building at last observation, with TRX and artifact steps pending. No L3 claim is made. This workflow-only path correction does not retroactively retrigger those source builds.

---

## 2026-11-23 — Align USB printer NVRAM checker with idempotent launcher

- The first Process/Storage source review caught one stale checker assumption: `check_usb_printer_nvram.py` still expected U2EC/LPD to unlink PID files in the individual wrappers. Cleanup now belongs to the shared launcher after a tri-state ABSENT preflight, so the checker has been updated to verify the checked-setting gates, three-argument paths, fail-closed UNKNOWN handling, RUNNING no-op and cleanup-before-fork ordering.
- This is a regression-check correction, not a production-code change. The initial USB modem workflow failed only this stale assertion; exact successor Actions are pending.

## 2026-11-23 — Normalize invalid process-inventory error values

- Tightened the shared process-presence start policy: an UNKNOWN scan now maps zero or negative/non-errno error values to `EIO`, preserving only positive errno values. This prevents a negative internal result from escaping as a negative startup status or being confused with a launcher convention.
- Added a GNU89 regression for negative inventory status and a source-contract assertion. Exact-SHA Process, Storage and XY-C1 Actions are tracked on the successor commit.

## 2026-11-23 — Make USB printer daemon launch idempotent across hotplug retries

- Fixed a retry-path duplication defect in U2EC/LPRng startup. A previous launch can leave the daemon running even when the subsequent NVRAM/hotplug acknowledgement fails; the retry path previously unlinked that live daemon's PID file and forked another instance.
- `start_usb_printer_daemon()` now takes ownership of the daemon PID-file path, performs a tri-state process inventory before any cleanup or fork, treats RUNNING as an idempotent success, and fails closed on UNKNOWN. A stale PID file is removed only after a complete ABSENT scan, and unlink errors are propagated instead of ignored.
- U2EC and LPRng no longer unlink PID files before the shared preflight. Existing process-presence policy regression and the Process/Storage source contracts are extended to assert RUNNING/UNKNOWN gates precede PID cleanup and fork. Exact-head Process/Storage/XY-C1 validation pending; hardware USB printer behavior remains unverified.

## 2026-11-23 UTC — Snapshot AP-client SSID presence before the next NVRAM read

- `wireless_nvram_get_text_status()` returns a pointer to a cache-owned NVRAM value. The APCLI helper previously inspected that pointer, made a second checked NVRAM read for `sta_wisp`, then passed the original pointer to the policy helper. A subsequent NVRAM operation could invalidate or replace the cache value.
- The helper now snapshots `ssid_present` immediately after the checked text read, before fetching the WISP flag. It passes only this stable boolean to the pure policy function; unexpected values fail with EINVAL.
- Updated GNU89 policy tests and the shared source checker to enforce snapshot-before-WISP-read ordering. Existing Wireless and XY-C1 path filters include the production source and policy test. Exact-SHA Actions, target Build/TRX/artifact and hardware tests remain pending; no local target build claimed.

---

## 2026-11-23 — Fail closed on unavailable or truncated WAN ARP egress interface

- `detect_wan` previously obtained the router-mode probe interface through `get_man_ifname()`, which wraps `nvram_safe_get()`; a transient NVRAM error could be reduced to an empty name. The code also copied names into the legacy 14-byte `sockaddr.sa_data` without checking whether the selected interface name fit.
- Linux 4.4 `SOCK_PACKET` send handling uses that 14-byte `spkt_device` field and forcibly NUL-terminates its last byte before device lookup. A truncated egress name could therefore fail lookup or select a different interface sharing its prefix, while the socket was independently bound using the full `ifreq` name.
- The probe now reads `wan0_ifname` through the checked copy API, snapshots the AP bridge name explicitly, and requires one non-empty interface name to fit both `ifreq.ifr_name` and the 14-byte packet address before socket creation. The socket helper independently rejects empty or unterminated bind names.
- Added GNU89 policy regressions for empty, overlong, non-terminated and valid device names; the test also proves invalid inputs do not partially mutate either output buffer. Exact-head ARP/XY-C1 Actions pending.

## 2026-11-23 UTC — Bind wireless APCLI policy test in shared source checker

- Exact-SHA runs on `3dad528f978904976210ce7c7dcbeda6e81b53fc` exposed a checker bug: the new final assertion referenced `WIRELESS_TEST`, but `check_modem_wan_start_status.py` had not loaded `tests/wireless_radio_policy.c`.
- The production APCLI policy test itself passed in Wireless radio state #45217541665. USB modem #45217541737, Process #45217541804, Firewall #45217546022 and XY-C1 #45217542897 were stopped by the shared Python NameError; AI docs #45217543243, watchdog #45217542656, iNIC #45217542047, HTTPD #45217542023 and process inventory #45217541649 passed. XY-C1 Build/TRX/artifact steps were skipped.
- Add the missing file binding and rerun exact-SHA checks; do not treat the earlier XY-C1 job as a firmware build.

---

## 2026-11-23 UTC — Fail closed on unknown AP-client SSID/WISP startup settings

- Replaced lossy `nvram_wlan_get(..., "sta_ssid")` and `is_apcli_wisp_*` decisions in 2.4/5 GHz AP-client startup. SSID and WISP state now come from the checked NVRAM reader; the WISP flag is only fetched after confirming a nonempty SSID.
- If either required setting cannot be read, APCLI startup is deferred, the interface is best-effort shut down, its LAN bridge membership is removed where applicable, and the error is returned so the restart can retry. This prevents an unknown WAN-side AP-client from being treated as a LAN interface.
- Added a GNU89-tested policy for the SSID/WISP decision, with cases for backend failures, absent SSID, WISP/non-WISP and invalid output pointers. Strengthened the existing modem/WISP source-contract checker and XY-C1 path filters.
- Exact-SHA Wireless radio, USB modem, Firewall, Process, AI docs and XY-C1 workflows are pending. No local target build or hardware result is claimed.

---

## 2026-11-23 — WAN modem reconnect delivery retry

- Source parent: `7a31ac98ebcf9af3762975606364c167687e6060`; implementation commit and exact-SHA Actions are recorded in the active handoff.
- Fixed a dropped-recovery defect: `detect_link` previously cleared its single 20-second modem-check deadline before calling a `void` notifier. Checked NVRAM/WISP/modem identity failures and `notify_rc` queue errors were therefore silently terminal until another physical link transition.
- `notify_modem_on_wan_ether_link_changed()` now returns checked failure status, and requires `notify_rc_status()` to accept the secure `auto_wan_reconnect` queue markers before reporting success. No-op cases remain successful.
- Added a dedicated GNU89 policy/test for two bounded retries at four-second intervals. `detect_link` now retries failed notifications and clears retry state on success, exhaustion, reset, or a new WAN link transition.
- Local ASan/UBSan policy regression: PASS. Exact-SHA GitHub Actions and XY-C1 Build/TRX/artifact status: pending.

## 2026-11-23 UTC — Fix WISP contract expected whitespace

- The first checker-only correction moved the detector extraction to `tight()`, which removes all whitespace, but the expected literals still contained spaces after commas and around `errno = status` / `goto read_failed`. This caused the exact-SHA workflows to fail at the same added assertion.
- Updated the expected source fragments to the same whitespace-free representation. This is a static test-only correction; the WISP production implementation is unchanged.
- Run USB modem, Process, Firewall and XY-C1 again on the next exact source SHA. The failed XY-C1 run had Build/TRX/artifact steps skipped before the source checker was fixed.

---

## 2026-11-23 UTC — Correct AP-client WISP static-check whitespace normalization

- Exact-SHA USB modem workflow #45213624031 on `7c16ec5c8738e3b8b698c75f5b01b1d9ab41aca7` completed all compiled C regression steps and all earlier checker groups successfully, then failed on the new WISP text-reader assertion.
- Root cause was in the new test only: the shared `compact()` function collapses whitespace to one space, while the added assertions compared strings with all spaces removed. Changed the new WISP assertions to use an explicit whitespace-stripping helper; no production source behavior changed.
- Require the rerun of USB modem identity and the exact-SHA XY-C1 Build/TRX/artifact steps before calling this WISP slice verified. The first failure is not evidence of a firmware build result.

---

## 2026-11-23 UTC — Preserve unknown AP-client WISP state during WAN/modem selection

- Found that `get_apcli_wisp_ifname()` derived WAN policy from lossy `nvram_wlan_get_int()` and `nvram_wlan_get()` accessors. A backend failure could be converted to zero/empty and reported as no WISP, which could allow priority-2 USB modem startup or WAN identity changes while the actual AP-client configuration was unknown.
- Added status-aware reads for each supported band: mode, radio-enable, `sta_wisp` and SSID. Missing settings preserve legacy defaults; checked-backend errors and malformed integer values are returned rather than hidden. Kept the old pointer API as a compatibility wrapper.
- Migrated modem auto-start, initial WAN modem device selection, AP-client WAN migration, manual WISP reassociation and WAN recovery after radio changes to the checked API.
- Extended the USB modem identity workflow path filters and static source-order/propagation contract to cover `net_wifi.c`; exact-head USB modem and XY-C1 actions pending. No local firmware build or hardware result is claimed.

---

## 2026-11-23 UTC — Preserve unknown wired-WAN link state during USB modem auto-start

- In `try_start_usb_modem_to_wan()`, priority 2 means the USB modem should start only when the wired WAN link is confirmed down and no AP-client WISP is active. The direct PHY helper returns -1 and sets errno when the checked `wan_src_phy` read or PHY status probe fails, but the caller changed that -1 to zero and proceeded as if the cable were disconnected.
- The WAN device-selection path in `net_wan.c` already records the probe errno and fails closed through `modem_wan_select_device_status()`; this USB hotplug path had diverged.
- Added a shared policy normalizer: only 0/1 are valid link results, negative results preserve errno (or EIO), and unexpected positive results are rejected. USB modem auto-start now returns the status and preserves the deferred event instead of initiating a competing WAN reconnect.
- Added GNU89 policy tests for confirmed up/down, EACCES, errno-less failure and invalid positive state, plus source-order/negative-fallback assertions. Exact-head USB modem, Process, AI and XY-C1 Actions pending; no local compile, target build or hardware result is claimed.

---

## 2026-11-23 UTC — Validate forked p910nd readiness slice

- On `bea2a85c96271c494076510ea06b96065d7c0b75`, Process fail-closed #45210393886 PASS, USB storage #45210396834 PASS, Process inventory #45210393909 PASS, AI docs #45210393826 PASS, USB modem identity #45210393790 PASS and NVRAM boundary #45210393769 PASS.
- XY-C1 #45210393797 was still running Build; TRX verification and artifact upload had not started at the last poll. Do not claim firmware Build/TRX/artifact or L3 for p910nd until all three product steps complete on this exact source SHA.

---

## 2026-11-23 UTC — Fail closed when QMI WDS client-ID publication fails

- `uqmi`'s `--get-client-id wds` path previously called `fopen("/tmp/qmi-client-id", "w+")`, then ignored `fprintf()` and `fclose()` failures while returning success. `qmi_service_get_client_id()` marks the client ID persistent, so the daemon could keep a QMI client allocation even when its only recovery record was never published.
- Added `uqmi_status_file_write()`: writes into a private 0600 `mkstemp()` staging file, checks owner/type/link/mode, handles short writes and EINTR, checks fsync/close, and atomically renames only after success. Failure leaves an existing destination untouched and removes the staging file. If client ID publication fails, uqmi calls `qmi_service_release_client_id()` before exiting so `qmi_device_close()` releases the newly allocated CID.
- `qmi_control_network_start()` now checks the uqmi process exit status and strictly refuses to continue with `--keep-client-id` unless the newly published client ID is readable. This prevents a transient file write failure from silently proceeding with an untracked persistent WDS CID.
- Added GNU89/ASan/UBSan tests for atomic replacement, 0600 ownership/mode, symlink-target preservation, failed-rename cleanup and invalid paths; local test PASS. Added exact USB modem workflow compilation/source-contract steps. Exact-head USB modem, Process, NVRAM/AI and XY-C1 checks are pending on the resulting SHA; no L3/L4 claim.
- Parent main SHA: `bea2a85c96271c494076510ea06b96065d7c0b75`; source, test, workflow, roadmap and handoff changes are one atomic commit.

---

## 2026-11-23 UTC — Require confirmed readiness from the forked p910nd printer daemon

- Found that standalone `p910nd` forks in its server path and exits the parent with status zero before the child acquires the per-port lock or binds the printer socket. `start_p910nd()` therefore treated a successful launcher parent as proof of a live spooler; child startup failures were silently acknowledged.
- After successful launch, `start_p910nd()` now uses the existing bounded process-inventory postcondition for the daemon's renamed `p9100d` argv[0]. A launch failure is returned immediately; a missing child, UNKNOWN inventory or readiness timeout propagates through the USB printer event result.
- Extended the Process workflow to assert the p910nd fork/parent-exit and argv[0]-rename contract, the readiness gate after both launcher forms, and future p910nd source path triggers. Reuses the detached-daemon GNU89/ASan/UBSan policy regression for delayed RUNNING, timeout, UNKNOWN and interrupted-wait behavior.
- Exact-head Process, USB storage, AI handoff and XY-C1 Build/TRX/artifact checks are pending at commit creation. No local compile, MIPS/uClibc build or hardware result is claimed.

---

## 2026-11-23 UTC — Keep Samba snapshot regression GNU89-compatible

- Moved the new snapshot-source policy test invocation below all local declarations in `main()`. The regression helper is now invoked without introducing a declaration-after-statement extension into the repository's GNU89 test compilation.

---

## 2026-11-23 UTC — Keep Samba ACL generation on the passdb credential snapshot

- `run_samba()` now loads passdb credentials from one NVRAM transaction, but `write_smb_conf_unlocked()` independently fetched `get_account_list()` for `valid users`, `invalid users`, `read list` and `write list`. An intervening UI account edit could leave ACLs and passdb describing different generations.
- The run_samba path now passes its stack-owned account pointers/count into smb.conf generation. Standalone public writers retain their own checked account-list read. Cleanup frees only the heap-owned fallback list, never caller-owned snapshot pointers.
- Added policy tests for empty/non-empty snapshot selection and bounds, plus Samba/HTTPD source contracts requiring passdb/ACL generation parity.

---

## 2026-11-23 UTC — Update USB storage caller contract for printer readiness gate

- Process fail-closed workflow exact rerun #45208311957 passed on `c8920caabc88c7f107c2c5240dab0bfaab38783b`, confirming the earlier header-scope correction.
- USB storage regression #45207830753 on production source `8fa162ae5df3fa865bdeedfac2dafc2f21de1ff8` had passed all storage host, sanitizer and BusyBox-ash steps before failing at its older inline source lookup for `start_usb_printer_daemon(char *const argv[])`; its U2EC/LPRng assertions also still expected the old one-argument calls. THIS checker-only change updates the signature and both explicit process-name callsites and asserts the EOF postcondition/process-inventory gate. No production code or behavior assertion was relaxed. Exact storage rerun pending.

---

## 2026-11-23 UTC — Correct USB printer readiness contract header scope

- Exact Process regression #45207830677 on `8fa162ae5df3fa865bdeedfac2dafc2f21de1ff8` passed every compile, sanitizer and pre-existing source check, then the new USB printer source contract failed because it searched for `PROCESS_PRESENCE_START_ABORT` and the RUNNING postcondition expression in `detached_daemon_start_policy.h`; those declarations correctly live in its included `process_presence_start_policy.h`.
- Corrected the contract to inspect the shared detached-daemon helper and presence policy separately. This is checker-only; no production source or assertions were weakened. Exact Process rerun pending.

---

## 2026-11-23 UTC — Require U2EC/LPRng process readiness after exec

- `start_usb_printer_daemon()` used EOF on a close-on-exec pipe as its success condition. That confirms only that `execv()` succeeded; a daemon that immediately exits after configuration parsing, pidfile setup or socket binding was still reported as successfully started.
- U2EC and LPRng startup now wait for and independently verify their named process through `process_inventory_scan()`. The shared detached-daemon policy retries for a bounded interval, treats UNKNOWN inventory as an error, and returns ETIMEDOUT if the process never appears. Those errors propagate through printer spooler startup, so the deferred USB event remains retryable rather than being acknowledged as successful.
- Reused the existing GNU89/ASan/UBSan detached-daemon policy regression (running child, delayed appearance, absent timeout, UNKNOWN and interrupted wait) and added a Process workflow source contract proving both U2EC and LPRng callsites require the postcondition. Exact-head Process, USB readiness and XY-C1 Build/TRX/artifact checks are PENDING at commit creation; no L3/L4 claim.
- Parent main SHA: `93e2d812a050da0029a339bee86b194968226992`; source, regression contract, roadmap and handoff are committed atomically.

---

## 2026-11-23 23:30 Beijing — Preserve the RStats interface selector on NVRAM read failure

- `save_speed_json()` previously used `nvram_safe_get()` and then persisted its first-active-interface fallback. A failed NVRAM read/close was indistinguishable from a missing selector, so a transient backend error could replace the user's configured interface. The fallback `nvram_set()` result was also ignored.
- This slice uses `nvram_get_checked()`, preserves the 0=missing / 1=present / negative=error contract, copies the cache-backed value into a bounded interface buffer, and refuses to persist any fallback when the selector is unknown or overlong. The chart export may still use a temporary live-interface fallback; a known missing/empty/stale selector is normalized only through a checked write whose failure is logged without stopping the exporter.
- Added GNU89 policy cases for missing/present/error/invalid read results and success/errno-less/error-code writes. The RStats workflow source contract now requires checked read, bounded copy, no lossy accessor and the write-status gate. Local GNU89/ASan/UBSan selector-policy test PASS; exact-head RStats, Process inventory, AI docs and XY-C1 Build/TRX/artifact are PENDING on the resulting SHA. No L3/L4 claim.
- Base main SHA: `60ccb0e47651e27ab0409c8fab1805116c7e2fae`. Source/test/contract/docs are included atomically in THIS COMMIT.

---

## 2026-11-23 UTC — Correct second HTTPD Samba identity contract for the snapshot API

- The next exact-head HTTPD rerun passed the Samba share/ACL policy stage but exposed a second source contract in the Unix-account identity stage that still expected the old independent `storage_read_string_status("http_username",...)` call.
- Updated that contract to require the transaction-backed account count and credential reads, mandatory transaction end before process inventory, and the snapshot close-status policy. No production code or security assertion was relaxed.

---

## 2026-11-23 UTC — Align HTTPD trust-boundary checker with coherent Samba NVRAM snapshots

- The first exact-SHA Samba workflow passed after the transactional credential change, but the adjacent HTTPD trust-boundary workflow failed because its source contract still expected three separately checked NVRAM getters.
- Updated the HTTPD checker to require the same single transaction for account count and credentials, transaction closure before process inventory, no legacy credential getter path, and retained password-policy checks. This is a checker-only adjustment; it does not relax the security contract.

---

## 2026-11-23 UTC — Snapshot Samba passdb credentials from one NVRAM generation

- `run_samba()` read `acc_num`, router username, account usernames and passwords through separate checked NVRAM API calls. Each read was individually safe, but concurrent UI writes could interleave and create a mixed-generation credential set.
- Added a single NVRAM transaction around the complete account-count/credential preload. Every cache-backed string is copied before the next transaction read; the transaction closes before process inventory, file cleanup, passdb staging or configuration publication.
- Read/validation failures always close the transaction and block publication; close errors also block publication, with the first read failure taking precedence over close failure.
- Added GNU89/Werror regression cases for successful close, errno-bearing and errno-less close failures, and first-error precedence, plus Samba workflow source assertions. Exact-SHA Samba/Process/AI/XY-C1 runs pending.

---

## 2026-11-23 UTC — Defer logger startup when the remote destination selector is unknown

- `init_router()` used `nvram_invmatch("log_ipaddr", "")` to choose whether syslogd starts before or after LAN setup. That legacy matcher collapses NVRAM read failures into an empty value, causing a configured remote logger to take the early/local-only startup path; the later phase was then skipped because the captured boolean was false.
- The boot path now checks the selector read, resolves missing keys to the legacy empty value, and conservatively defers logger start until LAN is ready whenever the value is unknown. The read error is preserved in the boot service status; `start_syslogd()` then takes a fresh checked snapshot and either configures the remote sink or keeps local logging active.
- Added policy cases for empty/configured values, read failures, malformed present values and invalid output pointers; the Process checker pins the checked selector and early-vs-late startup ordering.

---

## 2026-11-23 UTC — Fail closed on unknown iNIC guest AP schedule

- `check_inic_mii_rebooted()` previously used the value-only guest schedule helper. Backend/close read failures could collapse into the legacy unrestricted default and trigger a guest AP restart when schedule state was unknown.
- Added checked reads for `rt_guest_enable`, `rt_guest_date_x`, `rt_guest_time_x` and `rt_guest_time2_x`; cache-backed strings are copied before subsequent reads. Backend/time conversion failures now defer the recovery action. The normal scheduler and iNIC recovery gate share one weekly/overnight evaluator; absent/malformed legacy values retain the historical unrestricted default.
- Added GNU89 ASan/UBSan policy tests for weekday/weekend windows, overnight and week-boundary windows, and fail-closed NVRAM status resolution.
- Commit `054bf4966997b5a92d27f15ef8174447af0fced0`; exact iNIC #45201720346, Process #45201720149 and AI handoff #45201720172 PASS.

---

## 2026-11-23 UTC — Stop NFS daemons using verified process identities

- Replaced NFS stop-path name-wide `killall` signals with complete exact-name PID/start-time snapshots from `/proc/PID/stat`, identity revalidation before each signal, and separate snapshots before forced termination. Unreadable/malformed matching identities refuse signalling; the Linux 4.4 check-to-kill PID-reuse race is documented.
- Kept the original graceful signal semantics (rpc.mountd TERM, kernel nfsd SIGINT) and delayed SIGKILL until the bounded grace period expires. Tests cover graceful/forced signals and malformed/missing stat identities refusing all signals and unmounts.
- Production shell fix `02c858a1301718f48d83697c18d3669cdd333ae4`, test repair `2106f081a22fd222505eb849074ef53ad59e4fb9`, workflow contract `06ada702ed4f12715b9e9bbcb143b15c87b74678`. NFS #45197560702, Process #45197561398 and storage #45198161869 PASS.

---

## 2026-11-23 UTC — Align Infosvr regression checker with unaligned-input test

- The updated dispatch regression prints `infosvr accepts unaligned input and clears cached replies: PASS`, while the static checker still required the old duplicate-opcode message. Updated only the stale expected marker; source and test were already exercising the unaligned packet fix.
- Commit `6a7ed2a158230196937b161674ade9eaf760267c`; exact Infosvr #45201353587 PASS and latest descendant #45201803859 PASS.

---

## 2026-11-23 UTC — Preserve local syslog while reporting checked remote-destination failures

- `start_syslogd()` previously used `nvram_safe_get("log_ipaddr")` and `nvram_safe_get_int("log_port", ...)`, which collapse backend/close faults into empty/zero values and can silently disable a configured remote log sink.
- The destination is now read in one checked NVRAM transaction, with the address copied before the cache-backed port read. Missing values and invalid legacy port values retain their previous local/default behavior; backend, close and malformed API-result errors remain distinguishable.
- Local syslogd still launches in local-only mode when remote configuration cannot be verified. The degraded status is returned after the daemon startup postcondition succeeds, and klogd is still attempted.
- Added GNU89/Werror/ASan/UBSan regression coverage for checked reads, missing values, copy bounds, port defaults/ranges/overflow and first-failure aggregation. Local policy binary PASS; exact-SHA Process, AI handoff and XY-C1 checks pending.

---

## 2026-11-23 UTC — Mirror XUPNPD checked-port contract in Process source checker

- Added the XUPNPD checked-port text buffer declaration to `scripts/check_process_lifecycle_integration.py` source assertions. The inline workflow now has the matching expectation; editing this checker also re-triggers the Process inventory/fail-closed workflow rather than depending on a workflow-file-only push.
- This only tightens the source contract for the already-committed checked selector path. No product C source changed.
- Exact Process workflow on this commit is pending.
---
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

