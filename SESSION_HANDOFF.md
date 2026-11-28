## 2026-11-28 — Verify IAPP monotonic hardening and repair the handoff checkpoint gate

- Last product-source baseline: `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00` (IAPP negative-`tv_sec` validation and regression).
- Exact-main push Actions on that SHA: IAPP lifecycle `#46096900784` PASS; Process inventory `#46096900769` PASS; NVRAM transaction audit `#46096900816` PASS; XY-C1 Firmware Validation `#46096900847` PASS. The XY-C1 run completed Cross-build, TRX verification and artifact upload; artifact `15243698792`, 12,837,390 bytes, SHA-256 `c77d7d4117396efe6e0f560c72b3f9f557a5397493955a2636e99b8a40fc0d63`.
- AI handoff documentation contract `#46096900817` FAILED on the same SHA because this handoff omitted the explicit mandatory end-of-session checkpoint rule. This documentation-only commit adds the exact required rule, records the live validation boundary and updates the IAPP module's current evidence. Recheck the documentation workflow on the resulting main SHA.
- The GitHub Actions firmware build is L3 evidence for the default XY-C1 configuration. No physical-router/L4 test was run; the default package does not define `IAPP_EVENT_LOG`, so a macro-enabled MIPS build remains unverified.

---

# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; all development and validation are on `main` only.
- IAPP product-source baseline: `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`; latest product-source commit: `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` (VSFTPD final-absence stop gate). This follow-up only corrects the storage-hotplug workflow source contract and handoff evidence.
- Remote branch inventory: `main` and the preserved `backup/pre-plugin-overhaul-2026-11-14` only. Frozen backup HEAD: `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, delete, rename, or use it for development.
- No local source checkout was available to this session. Source/test evidence below comes from current GitHub files and exact-head Actions, not a claimed local build.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and compare it with this handoff before ending. The checkpoint is mandatory; a document edit is conditional. Confirm the meaningful product-source baseline, active task, exact-head Actions, validation boundary, blockers and exact first action. If all checkpoint facts remain accurate, leave this file untouched and do not create a no-op commit.

## Current source and regression baselines

### Ralink IAPP daemon — latest source baseline `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`

- `iapp_event_log_poll_policy.h` rejects negative monotonic seconds and invalid nanoseconds before signed elapsed-time subtraction. This prevents malformed synthetic timespecs from overflowing while preserving all valid CLOCK_MONOTONIC samples.
- Regression `iapp_event_log_policy.c` covers negative `now.tv_sec`, negative `last.tv_sec`, invalid nanoseconds, backwards samples and interval boundaries. The source checker asserts both seconds fields are validated.
- Exact IAPP lifecycle run `#46096900784` PASS on `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`, including the GNU89/Werror/ASan/UBSan monotonic event-log regression and optional event-log branch host compilation.
- The previous IAPP safety slice also retains executable/start-time-bound lifecycle checks, deadline-bounded TCP framing, PIPE_BUF-bounded length-framed driver notifications, FD_SETSIZE guards, MSG_TRUNC datagram bounds and checked event-log append/close handling.
- The XY-C1 default firmware image built and passed TRX verification and artifact upload on the same source SHA (run `#46096900847`). The default package does not define `IAPP_EVENT_LOG`; its macro-enabled MIPS compile and real radio/bridge behavior still require validation.

### BusyBox udhcpc — latest production source baseline `db92b3164321618e559051b471f560c7a863d25d`

- Successful DHCPACK lease accounting begins at ACK receipt, before optional ARP work and event callbacks. Callback runtime crossing T1 starts renewal immediately with the actual remaining lease budget; expiry triggers deconfiguration before rediscovery.
- A failed initial `bound` callback is rolled back with `deconfig` before returning to discovery. Failed cleanup remains in `DECONFIG_PENDING`; failed `renew`/`rebind` callbacks preserve the active lease and retry deadline.
- The dedicated GNU89/Werror/ASan/UBSan policy regression and integration checker passed on the subsequent checker/source baseline `a6faece21549c01ad0161c7c18d9a26725c3a6fd` (run `#46096136358`). The exact IAPP commit changes no DHCP source or DHCP checker paths.

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

The P1 VSFTPD final-absence stop gate is implemented and its dedicated GNU89/ASan/UBSan workflow passes. XY-C1 #46106551564 also completed Build, TRX and artifact upload successfully (L3 default XY-C1). Storage-hotplug is being brought into sync with the revised FTP stop contract; the current checker correction is #437b6ffd and its exact-head workflow was pending at last poll. Shadowsocks' procfs runtime regression passed, but its Python source-contract escape assertion needs the raw-string fix in THIS follow-up.

## Exact first action for the next GPT

1. Recheck Storage-hotplug run #46107555462 on base source commit `437b6ffd7fc1deb784ff86257a1271c22fa54874`; it was still in progress after whole storage caller-graph tests, with the eject/persistence gate pending. If it fails, inspect its exact job log and correct only the stale checker or actual failing behavior.
2. Inspect Shadowsocks lifecycle run #46107441785 on checker commit `2baf290f67eb662699b9edf6ae1eec97f51f5806`: shell syntax, startup-status, DNSMasq cleanup, fake-procfs runtime and adblock regression all passed; its Python source-contract step still failed because the expected `tr` string was parsed as NUL/newline. THIS follow-up changes it to a raw string. Verify exact-main Shadowsocks Actions on this commit and any associated docs check.
3. VSFTPD product change at `4dd43a97904e7bec0c8ae094ff7c25ff82312b07` has L2 regression #46106551633 PASS and exact XY-C1 #46106551564 PASS: Build, TRX verification and artifact upload all succeeded; artifact #15247998322, 12,837,319 bytes, SHA-256 `9d387a4418292dfbd642f2d0271ec00a4e0227a65ac02e50ef7ce83e3e6f56c9`. This is L3 default XY-C1 evidence; L4 physical-router behavior remains unverified.
4. iperf3 fail-closed discovery is already L2 verified: regression #46079981839 and Process inventory #46079981918 PASS on `6d659d767e8ac236527ca5cf02f2daedaf386507`; compare confirms the iperf3 source and shared process helper did not change through `4dd43a9`. Proceed to the next open reliability/security item from the current roadmap.
# Cross-session execution mandate

- Only `main` is an authorized development branch. Create no feature/checkpoint branches and no pull requests. Keep the frozen backup immutable.
- For each multi-file engineering slice, create all blobs and one complete tree, then publish one atomic main commit with the latest main as sole parent; advance main with `force:false` and `expected_sha` equal to that parent. Never use sequential file commits, force-push, amend or rewrite published history.
- Re-fetch main immediately before each write. If compare-and-swap rejects because main moved, inspect the new head, rebuild/retest on top of it, and retry without force.
- Inspect push-triggered Actions by exact `head_sha`. PASS permits the next slice; FAIL must be diagnosed/fixed; PENDING does not block independent engineering.

Before context exhaustion, stop starting new modules, finish and publish a safe bounded slice, then leave this handoff with the exact first action, validation boundary and committed baseline.
- Run the local handoff-doc validator when a checkout is available. This session had no local checkout; the exact-head GitHub Actions workflow is the validation gate for the docs correction.
- Keep the root dev log at 20–119 complete records; it had 74 records before this entry, so no archive rotation is due. Do not add handoff/roadmap archive snapshots.
- Do not change GPT Session Rotator or replay its existing state/SHA. Do not claim hardware validation or a successful build when steps were skipped.
