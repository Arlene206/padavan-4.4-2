## 2026-11-28 — Make firmware stop-status checker match scoped C statements (THIS COMMIT)

- Base main SHA: `4bc110b517f8f1f49e5b0d6f64ea613a633577b0`. The production MiniUPnPd firmware-preparation fix is committed in parent `4bc110b517f8f1f49e5b0d6f64ea613a633577b0`. Review caught a checker-only issue before the queued Process run started: the shared `block()` parser expects a braced block, but the service-stop first-error assignment is intentionally a single statement. The checker incorrectly asked that parser to extract it and would inspect the following aggregate block instead.
- THIS COMMIT uses a whitespace-tolerant source assertion for that single-statement condition. No production source or runtime behavior changed. Parent Process/XY-C1/PID 1 runs were still pending at the last poll; exact-head reruns are required.


---

# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; all development and validation are on `main` only.
- Live main before this source-contract correction: `4bc110b517f8f1f49e5b0d6f64ea613a633577b0`. It contains the firmware-preparation change that explicitly stops MiniUPnPd before persistence/MTD write. The host checker was strengthened to cover that gate, then this follow-up corrected its single-statement parsing without modifying production source. Parent exact-head Process #46114250080 / Process umbrella #46114249925, PID 1 notification queue #46114249923 and XY-C1 #46114250159 were queued or in progress at last poll; no result was claimed.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14`; do not mutate. Its warning file was verified during startup restore and handoff-recorded frozen HEAD is `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains CLOSED/unmerged.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` (handoff-recorded HEAD `72f4f157a084f8184b9597ca5bf334b8080e5807`); its warning file explicitly forbids mutation. PR #1 remains CLOSED/unmerged with no identified missing change.
- Remote branch search shows only the preserved `backup/pre-plugin-overhaul-2026-11-14` archive beyond `main`. Its warning file was read from that ref and explicitly forbids any modification/deletion; the handoff-recorded frozen HEAD is `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains CLOSED and unmerged; its unmerged work was previously compared and no valid missing source/test/build change was identified.
- Pre-commit exact-main AI docs #46110148275 and HTTPD #46110148273 both PASS on `6b897adfbe68c7b3ab9d0eb040181b806662e3ef`. No local source checkout is available; source/test evidence is from GitHub and Actions, not a claimed local build.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and compare it with this handoff before ending. The checkpoint is mandatory; a document edit is conditional. Confirm the meaningful product-source baseline, active task, exact-head Actions, validation boundary, blockers and exact first action. If all checkpoint facts remain accurate, leave this file untouched and do not create a no-op commit.

## Current source and regression baselines

### Ralink IAPP daemon — latest source baseline `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`

- `iapp_event_log_poll_policy.h` rejects negative monotonic seconds and invalid nanoseconds before signed elapsed-time subtraction. This prevents malformed synthetic timespecs from overflowing while preserving all valid CLOCK_MONOTONIC samples.
- Regression `iapp_event_log_policy.c` covers negative `now.tv_sec`, negative `last.tv_sec`, invalid nanoseconds, backwards samples and interval boundaries. The source checker asserts both seconds fields are validated.
- Exact IAPP lifecycle run `#46096900784` PASS on `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`, including the GNU89/Werror/ASan/UBSan monotonic event-log regression and optional event-log branch host compilation.
- The previous IAPP safety slice also retains executable/start-time-bound lifecycle checks, deadline-bounded TCP framing, PIPE_BUF-bounded length-framed driver notifications, FD_SETSIZE guards, MSG_TRUNC datagram bounds and checked event-log append/close handling.
- The XY-C1 default firmware image built and passed TRX verification and artifact upload on the same source SHA (run `#46096900847`). The default package does not define `IAPP_EVENT_LOG`; its macro-enabled MIPS compile and real radio/bridge behavior still require validation.

### BusyBox udhcpc — strict server-ID option parsing (THIS COMMIT)

- Prior lease-accounting and failed-initial-bound rollback work remains in production history: ACK-time lease accounting charges script time, initial bound-hook failure deconfigures partial state, and failed cleanup stays in `DECONFIG_PENDING`.
- Parent `c2445cf` added explicit-zero rejection, `7cc7bef` added limited-broadcast rejection, and `c21c9f0` restored the documented address-less DHCP behavior (absent or explicit 0.0.0.0 before any server is selected) while rejecting zero after pinning/during REBINDING and all-ones in every state.
- Review of the c21 code found `udhcp_get_option32()` returned NULL both for absent option 54 and for a present wrong-length option; plain `udhcp_get_option()` also skips zero-length options. That meant malformed lengths could be treated as absence in address-less mode.
- THIS COMMIT adds `udhcp_get_option_with_length()` with -1 malformed/0 absent/1 present (including zero length) plus the actual payload length. The legacy wrapper retains its previous behavior for unrelated callers. DHCPOFFER, ACK and NAK now reject present server-ID lengths other than four bytes and malformed option streams.
- GNU89/Werror/ASan/UBSan policy tests cover absent/found/invalid status combinations and lengths 0/3/5; checker assertions pin scanner behavior and OFFER/ACK/NAK validation order. THIS commit's exact-head regression and docs Actions are pending. No local compilation is claimed.
- Parent c21 exact-head udhcpc #46111075543, AI docs #46111075486, DHCPv6 Reply #46111075672, WAN DHCP callback #46111075595, Process inventory #46111075501, Process fail-closed #46111075531 and USB storage #46111075489 all PASS. Parent NVRAM #46111075506 was still running its final ioctl/caller audit, and XY-C1 #46111075565 was still in Build.
- XY-C1 #46109982320 for HTTPD source SHA `856fe202e000de0b751c3f1ace304a208e7c0b37` completed Build, TRX and artifact upload (L3 for that earlier exact SHA only). The current DHCP SHA requires its own firmware result; no physical-router L4 has been run.

### PID 1 watchdog and shared process lifecycle

- Watchdog production source commit: `c9f5d0e1abd9df58576b18db4e8d10db0e448066`. Watchdog start/readiness/rollback is scoped to both argv0 and the resolved `/sbin/rc` executable identity, with PID/start-time revalidation. Failed-start rollback signals only verified watchdog instances, not all generic same-name services.
- Earlier exact regressions passed on their recorded SHAs: Process lifecycle on `44f801f34ef7e8d908b9a5d7f5bac5e56d0ac4d2` and watchdog signal on `3d6d2d712a3c5368f327810dd5adb73c8af70932`. These are historical SHA-specific results, not fresh watchdog regression results for `7daa20cd`.
- Do not repeat the broad process-inventory or watchdog audit without a new concrete failure. The exact current-head Process inventory workflow `#46096900769` passed.

### Storage / media publisher

- The storage checker was corrected to match lstat-based target validation and validated `force_first_valid` forwarding. The earlier run `#46095697039` passed on `db92b3164321618e559051b471f560c7a863d25d`; that run's firmware build steps were not a substitute for the later full L3 result on `7daa20cd`.
- Do not repeat the completed media-publisher audit without new evidence.

- AI handoff documentation contract #46106089042 PASS on docs SHA `badf4b0a7b0b94f2c774134da72978474bd859d9`; the new source/test commit has its own required exact-head rerun.

### VSFTPD stop/restart cleanup — THIS COMMIT

- Stop status now preserves the exact nonzero `kill_services()` result; if the command reports success but final procfs state is RUNNING, UNKNOWN or malformed, the stop still fails and records a meaningful errno.
- Every stop-path firewall/UPnP reconciliation requires final `PROCESS_PRESENCE_ABSENT`. The storage-stop caller no longer lets a pre-stop RUNNING/UNKNOWN observation authorize a refresh when the final inventory remains uncertain; it aborts storage removal and retains the first stop failure.
- GNU89 policy tests and workflow source-order contracts cover detailed status propagation, final ABSENT/RUNNING/UNKNOWN gating and group-stop first-error preservation. Dedicated VSFTPD regression #46106551633 PASS (L2); the umbrella storage-hotplug checker is being corrected for the new helper. XY-C1 and NVRAM audit were still running at last poll.

### Shadowsocks — complete procfs confirmation for ABSENT lookups

- Product and fake-procfs regression commit `f23be0cff7d7e0d1e942ebcf10094497b73be676` added complete procfs confirmation after BusyBox `pidof`/`pgrep` status 1. The dedicated workflow's shell syntax, startup-status, DNSMasq cleanup, process lifecycle runtime regression and adblock publication steps ALL PASSED; Process inventory #46079286495 PASS and XY-C1 #46079286427 PASS on the exact SHA.
- The dedicated workflow's syntax and all four shell/runtime regressions passed on product SHA `f23be0cff7d7e0d1e942ebcf10094497b73be676`; only its Python source-contract step failed. The follow-up in `2baf290f67eb662699b9edf6ae1eec97f51f5806` removed one over-escaping layer but left the Python assertion without a raw-string prefix, so Python interpreted `\000` and `\n` as a NUL and newline rather than the literal bytes present in the shell source. THIS checker-only commit makes the expectation a raw string, preserving literal backslash sequences. Exact-main rerun pending.
- The old AI-docs run #46079286331 failed because the roadmap exceeded the 128 KiB documentation hard limit. Later main checkpoint #46106904624 and latest #46107190369 passed the handoff-doc contract, so this is historical and does not need redoing.

### Storage hotplug caller-graph checker — remaining stale FTP contracts

- Run #46107190352 passed the whole storage caller graph (including the previous two workflow-embedded FTP assertions) and then failed in `scripts/check_storage_safe_remove.py` at the FTP stop contract. That external checker still expected `storage_service_stop_complete(status, final_state)` in two duplicated sections and the retired `ftp_was_present` override. THIS commit updates all stale assertions to the tested `ftp_stop_status_result`, first-error aggregation, and final-ABSENT-only firewall gate. Host storage policy and workflow tests before this last checker passed.

## Verification boundary

For product-source SHA `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`:

| Workflow | Run | Result |
|---|---:|---|
| Ralink IAPP daemon firewall lifecycle regression | `46096900784` | PASS |
| Process lifecycle source inventory | `46096900769` | PASS |
| NVRAM transaction boundary audit | `46096900816` | PASS |
| XY-C1 Firmware Validation | `46096900847` | PASS — Cross-build, TRX verify, artifact upload |
| AI handoff documentation contract | `46096900817` | FAIL — handoff omitted mandatory checkpoint wording; correction is in this docs commit, exact-head rerun pending |
| AI handoff documentation contract #46106551579 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | PASS |
| VSFTPD stability regression #46106551633 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | PASS — GNU89/Werror/ASan/UBSan and source-order contract |
| Process lifecycle inventory #46106551548 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | PASS |
| Process inventory fail-closed #46106551859 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | PASS |
| Storage hotplug #46106551612 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | FAIL — caller-graph checker still expected `storage_service_stop_complete(status, final_state)`; corrected in THIS workflow-only follow-up, exact rerun pending |
| XY-C1 Firmware Validation #46106551564 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | IN PROGRESS at last poll — Build running; TRX/artifact pending |
| NVRAM transaction boundary audit #46106551639 on `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` | IN PROGRESS at last poll |

- Firmware artifact: `XY-C1-firmware-7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`, artifact ID `15243698792`, 12,837,390 bytes, SHA-256 `c77d7d4117396efe6e0f560c72b3f9f557a5397493955a2636e99b8a40fc0d63`.
- L3: PASS for default XY-C1 MIPS firmware build, TRX validation and artifact upload on the exact source SHA above.
- L4: NOT VERIFIED — no physical router flash/boot or live radio/network validation was performed.
- No local source build/test result is claimed. Do not conflate host sanitizer tests with target firmware validation.
- iperf3 fail-closed discovery: dedicated shell regression #46079981839 and Process inventory #46079981918 PASS on `6d659d767e8ac236527ca5cf02f2daedaf386507`. Commit comparison against `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` found no changes in `trunk/user/iperf3/**` or `trunk/user/shared/scripts/process_lifecycle.sh`; L2 applies to current source.

## Closed legacy PR preservation check

- PR #1 is CLOSED (not merged), head SHA `ba48e22609e0c81ad757b06756fe91d05f39cbf6`; compare against current main reported 17 PR-only commits and 5,474 main-only commits from the merge base.
- Reviewed all substantive changed source/test/workflow paths. The HMAC helper C source, its Makefile, package Makefile and HMAC regression are byte-identical between the PR head and main. Current main's expanded AliDDNS script uses the helper; current CA bootstrap publisher, checker, regression and workflow contain the PR's trust-store behavior plus later hardening.
- No missing production/test/build change was identified for porting. PR #1 is already closed; do not resume work on its deleted/legacy head or reopen it just to continue development.

## Active task

Production fix in parent `4bc110b517f8f1f49e5b0d6f64ea613a633577b0`: `flash_firmware()` explicitly stops MiniUPnPd (not included in `stop_services(0)`), then still attempts the independent generic stop pass, preserves the first errno and aborts before persistence/flash if either stop fails. `scripts/check_firmware_update_notification.py` now locks this order and abort path.

This follow-up is checker/docs-only. It replaces an invalid use of the shared braced-block parser on a one-line `if` with a whitespace-tolerant regex assertion. At the last poll, parent runs Process inventory #46114250080, Process umbrella #46114249925, PID 1 notification #46114249923 and XY-C1 #46114250159 were queued/in progress; no result is claimed. VPN FIFO production SHA `78afe64833e88851484b28c28d62ec482cff5ecd` has completed XY Build/TRX/artifact PASS; L4 remains unverified.

## Exact first action for the next GPT

1. Poll parent source SHA `4bc110b517f8f1f49e5b0d6f64ea613a633577b0` run #46114250159 and Process/PID 1 runs #46114250080, #46114249925 and #46114249923; distinguish any failure in the original checker from the corrected checker below.
2. Inspect current-main exact-head Actions from this checker correction. Require the firmware update notification C policy test and `check_firmware_update_notification.py` to PASS; inspect logs and fix real failures without weakening assertions.
3. Require the XY-C1 Build, TRX verification and artifact upload on the exact production-source SHA `4bc110b517f8f1f49e5b0d6f64ea613a633577b0` to PASS before claiming L3 for the MiniUPnPd flash-preparation change. The current descendant changes only checker/docs.
4. Continue an independent P1 stability/security fix while pending work runs. Physical firmware flash and router L4 have not been performed.

# Cross-session execution mandate

- Only `main` is an authorized development branch. Create no feature/checkpoint branches and no pull requests. Keep the frozen backup immutable.
- For each multi-file engineering slice, create all blobs and one complete tree, then publish one atomic main commit with the latest main as sole parent; advance main with `force:false` and `expected_sha` equal to that parent. Never use sequential file commits, force-push, amend or rewrite published history.
- Re-fetch main immediately before each write. If compare-and-swap rejects because main moved, inspect the new head, rebuild/retest on top of it, and retry without force.
- Inspect push-triggered Actions by exact `head_sha`. PASS permits the next slice; FAIL must be diagnosed/fixed; PENDING does not block independent engineering.

Before context exhaustion, stop starting new modules, finish and publish a safe bounded slice, then leave this handoff with the exact first action, validation boundary and committed baseline.
- Run the local handoff-doc validator when a checkout is available. This session had no local checkout; the exact-head GitHub Actions workflow is the validation gate for the docs correction.
- Keep the root dev log at 20–119 complete records; it had 91 records before this entry and now has 92, so no archive rotation is due. Do not add handoff/roadmap archive snapshots.
- Do not change GPT Session Rotator or replay its existing state/SHA. Do not claim hardware validation or a successful build when steps were skipped.
