## 2026-11-28 — Verify IAPP monotonic hardening and repair the handoff checkpoint gate

- Last product-source baseline: `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00` (IAPP negative-`tv_sec` validation and regression).
- Exact-main push Actions on that SHA: IAPP lifecycle `#46096900784` PASS; Process inventory `#46096900769` PASS; NVRAM transaction audit `#46096900816` PASS; XY-C1 Firmware Validation `#46096900847` PASS. The XY-C1 run completed Cross-build, TRX verification and artifact upload; artifact `15243698792`, 12,837,390 bytes, SHA-256 `c77d7d4117396efe6e0f560c72b3f9f557a5397493955a2636e99b8a40fc0d63`.
- AI handoff documentation contract `#46096900817` FAILED on the same SHA because this handoff omitted the explicit mandatory end-of-session checkpoint rule. This documentation-only commit adds the exact required rule, records the live validation boundary and updates the IAPP module's current evidence. Recheck the documentation workflow on the resulting main SHA.
- The GitHub Actions firmware build is L3 evidence for the default XY-C1 configuration. No physical-router/L4 test was run; the default package does not define `IAPP_EVENT_LOG`, so a macro-enabled MIPS build remains unverified.

---

# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; all development and validation are on `main` only.
- Live main before this documentation checkpoint: `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`. The product-source baseline remains this SHA; the commit containing this handoff refresh is documentation-only. At the next takeover, fetch live main and do not treat a docs-only tip as a new source baseline.
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

## Verification boundary

For product-source SHA `7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`:

| Workflow | Run | Result |
|---|---:|---|
| Ralink IAPP daemon firewall lifecycle regression | `46096900784` | PASS |
| Process lifecycle source inventory | `46096900769` | PASS |
| NVRAM transaction boundary audit | `46096900816` | PASS |
| XY-C1 Firmware Validation | `46096900847` | PASS — Cross-build, TRX verify, artifact upload |
| AI handoff documentation contract | `46096900817` | FAIL — handoff omitted mandatory checkpoint wording; correction is in this docs commit, exact-head rerun pending |

- Firmware artifact: `XY-C1-firmware-7daa20cd2f1bd7abca4499fb9f9a6e05ce5d3d00`, artifact ID `15243698792`, 12,837,390 bytes, SHA-256 `c77d7d4117396efe6e0f560c72b3f9f557a5397493955a2636e99b8a40fc0d63`.
- L3: PASS for default XY-C1 MIPS firmware build, TRX validation and artifact upload on the exact source SHA above.
- L4: NOT VERIFIED — no physical router flash/boot or live radio/network validation was performed.
- No local source build/test result is claimed. Do not conflate host sanitizer tests with target firmware validation.

## Closed legacy PR preservation check

- PR #1 is CLOSED (not merged), head SHA `ba48e22609e0c81ad757b06756fe91d05f39cbf6`; compare against current main reported 17 PR-only commits and 5,474 main-only commits from the merge base.
- Reviewed all substantive changed source/test/workflow paths. The HMAC helper C source, its Makefile, package Makefile and HMAC regression are byte-identical between the PR head and main. Current main's expanded AliDDNS script uses the helper; current CA bootstrap publisher, checker, regression and workflow contain the PR's trust-store behavior plus later hardening.
- No missing production/test/build change was identified for porting. PR #1 is already closed; do not resume work on its deleted/legacy head or reopen it just to continue development.

## Active task

The next engineering slice is the P1 VSFTPD stop/restart cleanup defect described below: firewall/UPnP reconciliation and successful-stop reporting must wait until a complete process inventory confirms the daemon is ABSENT; preserve the original stop error and fail closed on UNKNOWN.

## Exact first action for the next GPT

1. Re-fetch live `main`, then inspect the AI handoff documentation workflow for the new documentation commit. The previous exact-head run `#46096900817` failed at `scripts/check_ai_handoff_docs.py` because the handoff lacked the two required checkpoint strings. This commit adds both. If the new exact-head run fails, inspect its first causal log line and correct the actual contract; do not weaken the checker.
2. After the handoff workflow is green, continue the next P1 user-space stability item in the roadmap: VSFTPD stop/restart cleanup. The stated defect is that firewall/UPnP reconciliation and stop success must occur only after procfs confirms the daemon is ABSENT; preserve the original stop error and fail closed on UNKNOWN. Start by tracing the current VSFTPD launch/stop functions, their RC callers, and the dedicated regression/workflow inputs. Add a regression for each missing outcome before changing production code.
3. Do not wait for unrelated pending Actions. Do not re-audit completed IAPP, DHCP, generic process inventory, watchdog or storage slices without a new failing case.

# Cross-session execution mandate

- Only `main` is an authorized development branch. Create no feature/checkpoint branches and no pull requests. Keep the frozen backup immutable.
- For each multi-file engineering slice, create all blobs and one complete tree, then publish one atomic main commit with the latest main as sole parent; advance main with `force:false` and `expected_sha` equal to that parent. Never use sequential file commits, force-push, amend or rewrite published history.
- Re-fetch main immediately before each write. If compare-and-swap rejects because main moved, inspect the new head, rebuild/retest on top of it, and retry without force.
- Inspect push-triggered Actions by exact `head_sha`. PASS permits the next slice; FAIL must be diagnosed/fixed; PENDING does not block independent engineering.

Before context exhaustion, stop starting new modules, finish and publish a safe bounded slice, then leave this handoff with the exact first action, validation boundary and committed baseline.
- Run the local handoff-doc validator when a checkout is available. This session had no local checkout; the exact-head GitHub Actions workflow is the validation gate for the docs correction.
- Keep the root dev log at 20–119 complete records; it had 72 records before this entry, so no archive rotation is due. Do not add handoff/roadmap archive snapshots.
- Do not change GPT Session Rotator or replay its existing state/SHA. Do not claim hardware validation or a successful build when steps were skipped.
