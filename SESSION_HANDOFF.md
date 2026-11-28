## 2026-11-28 — Read IAPP timer helper contract from its header

- IAPP lifecycle #46095191121 on SHA 3962b8955c0699548627bfe58c67bbad47e87858 passed the converted legacy ioctl test and all daemon/shell regressions, then failed in `scripts/check_iappd_select_fd.py`: a new assertion searched the helper declaration inside `rtmpiapp.c` rather than the separate `iapp_event_log_poll_policy.h`.
- THIS checker-only correction reads the dedicated policy header and verifies its timespec/nanosecond validation there. No product source changed.
- AI Docs #46095191125 and Process inventory #46095191145 PASS on 3962b895; NVRAM audit #46095191123 remained IN_PROGRESS. Exact IAPP, XY-C1 and AI reruns required for this commit.

---

## 2026-11-28 — Update IAPP ioctl regression for timespec poll contract

- IAPP lifecycle #46095024005 on SHA 78ff00d25b0957ecc2c148d606aee12615fbaa1d failed during its pre-existing interface regression compile: `iapp_ioctl_ifname_policy.c` still called `iapp_event_log_poll_due(int)` six times. The API is now intentionally timespec-based, and the new monotonic test itself was not reached.
- THIS commit converts that legacy test to sub-second/exact-second/overdue/backwards and invalid-pointer cases. No runtime source changed.
- The separate IAPP timer test's GNU89 declaration order and diagnostic escape fixes are in 7040bb66f7d0c236a7a65c56b2d840dbd3ce8153. Exact IAPP and XY-C1 reruns required; AI docs and Process inventory on 78ff00d2 PASS.

---

## 2026-11-28 — Tighten IAPP monotonic timer diagnostics and GNU89 test ordering

- Follow-up review of 78ff00d25b0957ecc2c148d606aee12615fbaa1d caught two small integration issues before CI completion: the new DBGPRINT strings contained doubled backslashes (printing literal "\\n" instead of a line break), and the GNU89 test called its new helper before declaring local constants in main.
- THIS commit corrects the log escapes and moves the test call after C89 declarations. Runtime scheduling logic is unchanged.
- Exact 78ff00d Actions #46095024005 (IAPP lifecycle), #46095023997 (XY-C1), #46095024014 (AI docs) and #46095024034 (Process lifecycle inventory) were IN_PROGRESS at last poll; exact reruns required.

---

## 2026-11-28 — Make optional IAPP event-log polling monotonic

- Fixed a reliability defect in the optional Ralink IAPP event logger: `IAPP_Start()` used `gettimeofday()`/wall-clock subtraction for its one-second poll. NTP/manual clock adjustments could stall snapshots or trigger them at the wrong rate.
- The poll now uses `clock_gettime(CLOCK_MONOTONIC)`, a validated timespec interval helper, and retries after transient clock-read errors (first successful sample polls immediately). Wall-clock APIs are removed from this timer path.
- Added GNU89/Werror/ASan/UBSan tests for sub-second, exact-one-second, overdue, backwards and invalid timespec inputs; added the test to the IAPP lifecycle workflow and updated source contract.
- DHCP source SHA c94fb8058b32b46b9a3b98c10605772799f82739: udhcpc sanitizer unit binary PASS, but its integration checker failed due a stale ordering assertion (fixed in parent checker-only SHA 89607a2a52ef77680926ebb385ff2815351942b9). Process inventory/lifecycle, WAN DHCP, DHCPv6 and AI docs PASS; XY-C1, storage and NVRAM checks were pending. Exact-head reruns required.
- No local firmware cross-build or hardware/L4 test performed.

---

## 2026-11-28 — Correct DHCP lease planner integration assertion

- Exact SHA c94fb8058b32b46b9a3b98c10605772799f82739: GNU89/Werror/ASan/UBSan C policy test PASS; Process inventory/lifecycle, AI docs, WAN DHCP callback and DHCPv6 reply regressions PASS. udhcpc integration checker #46094795076 failed at an obsolete assertion requiring `already_waited_sec = script_elapsed` and a fixed `timeout = lease_seconds / 2`; the new planner correctly computes both values in `udhcpc_lease_script_plan()` and performs expiry cleanup before committing the server/lease state.
- THIS commit updates only the checker ordering contract to assert planner → expiry/deconfig/DECONFIG_PENDING → server/rebind/IP/BOUND commit. No production source changed.
- XY-C1 #46094795007 and USB storage #46094794990 were IN_PROGRESS; NVRAM audit #46094795027 was IN_PROGRESS at last poll. Exact-head reruns required.
- Next independent work: switch optional IAPP event-log polling from wall-clock gettimeofday/timersub to CLOCK_MONOTONIC and add timespec-based regressions.

---

## 2026-11-28 — Start DHCP ACK lease accounting before ARP/script work

- Follow-up to 2705ffb259efc0e26af1a692641c2ed6e5dd5a6a: found that the new slow-callback lease planner still started its monotonic clock immediately before `bound`/`renew`, excluding the optional ARP conflict probe and preceding ACK processing. A long ARP probe could therefore consume the lease before the clock began.
- Moved the timestamp to the beginning of the accepted DHCPACK branch, before ARP probing and event callbacks. The same elapsed duration now constrains both ACK script success/lease commit and failed-renew retry timing. Added an integration assertion for ACK receipt → ARP probe → script order.
- Exact-head udhcpc/XY-C1/Process/AI Actions for the parent 2705ffb are pending; this follow-up supersedes them for source verification. No local MIPS build or hardware test claimed.

---

## 2026-11-28 — Preserve valid DHCP leases when ACK callbacks run past T1

- Found a live lease-timing defect in BusyBox udhcpc: after a successful `bound`/`renew` callback, it treated any callback runtime greater than T1 (`lease_seconds / 2`) as if the entire lease had expired by resetting both `timeout` and `already_waited_sec` to zero. The next BOUND tick therefore entered RENEWING → REBINDING with no retry budget and deconfigured the address, even when the full lease still had substantial time left.
- Added `udhcpc_lease_script_plan()`: before T1 it charges callback time against the T1 wait; at/after T1 it immediately starts renewal but caps future retry budget to the actual `lease_seconds - script_elapsed`; at/after full lease expiry it refuses to commit the ACK and runs `deconfig` before returning to discovery. If cleanup fails, the existing DECONFIG_PENDING retry path is used.
- Added GNU89 regression for before/at/after T1, partial remaining leases, full expiration, zero leases and invalid outputs. Updated the live source checker; existing workflow already watches the touched policy header/test paths.
- Production source SHA: THIS COMMIT. Exact-head udhcpc regression + XY-C1 compile/TRX/artifact and AI docs checks are required. No local cross-build or hardware test is claimed.
- Prior checkpoint: watchdog signal #46094212414 PASS on 3d6d2d7; AI handoff docs failed only on oversized handoff, compacted in 82ef9ed. XY-C1 run #46093910770 on source SHA 5592891 was still pending at last poll; poll it independently.

---

# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`.
- Parent main SHA before this documentation checkpoint: `3d6d2d712a3c5368f327810dd5adb73c8af70932`. Read the live branch ref before the next write.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it.
- Historical per-slice details remain in `dev_log.md` and the indexed `docs/handoffs/` archive. This current handoff intentionally contains only active work and verification state.

## Active task

- Fixed a watchdog readiness false-positive: the shared process inventory intentionally accepts argv0/comm aliases for legacy BusyBox daemons, so a process using `exec -a watchdog` could suppress the PID 1 watchdog start/recovery loop.
- Watchdog-specific scanning now requires both argv0 basename `watchdog` and `/proc/PID/exe` resolving to the ROMFS executable alias `/sbin/rc` or `/sbin/watchdog`; it revalidates the PID/start-time/argv snapshot after procfs reads. The generic shared inventory semantics remain unchanged.
- Failed-start rollback now sends TERM/KILL only to individually verified watchdog identities, with bounded waits and a final absence check; it no longer delegates to generic `kill_services()`.
- Added `watchdog_process_executable_policy.h` and GNU89/Werror/ASan/UBSan tests for accepted executable aliases, deleted suffix, exact argv0 matching and invalid inputs. Related Process, watchdog-signal and XY-C1 path filters/source contracts were updated.
- Storage checker follow-up is complete: `scripts/check_storage_safe_remove.py` now checks the existing `lstat()`/media-link policy and the shared publisher's validated `force_first_valid` forwarding rather than obsolete source forms.

## Verification boundary

- Process lifecycle regression #46094087141 PASS on SHA `44f801f34ef7e8d908b9a5d7f5bac5e56d0ac4d2`; its GNU89/Werror/ASan/UBSan step `Compile and run watchdog executable identity regression` PASSed.
- Watchdog signal lifecycle regression #46094212414 PASS on SHA `3d6d2d712a3c5368f327810dd5adb73c8af70932`.
- AI handoff docs #46094212400 FAIL on that same SHA only because `SESSION_HANDOFF.md` exceeded the 64 KiB limit. THIS checkpoint compacts the current handoff; exact-head AI docs rerun is required.
- XY-C1 firmware validation #46093910770 on production source SHA `559289198b598cc2531ce7f37d14e1737a1e51ee` was IN_PROGRESS at last poll: host regressions passed, Build XY-C1 was running, TRX and artifact upload were pending. Do not claim L3 until Build, TRX verification and artifact upload all PASS on that same SHA.
- Storage hotplug #46093641272 PASS, AI docs #46093641318 PASS, and XY-C1 workflow #46093641344 completed successfully on SHA `363fd7fba6f90352d70bb581ea6d0690a573441c`; however, its Build/TRX/artifact steps were SKIPPED by the firmware-input detector, so it is not L3 evidence.
- No local cross-build or physical-router test was run. Hardware/L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Poll XY-C1 run #46093910770 for SHA `559289198b598cc2531ce7f37d14e1737a1e51ee`. If Build fails, inspect its job log and repair the actual compiler/source failure. If Build passes, require TRX and artifact upload to finish PASS before making any L3 claim.
2. Check the exact-head AI docs rerun triggered by this compact handoff, plus Process lifecycle and watchdog-signal runs for the latest main SHA. Fix only concrete failures; do not repeat the completed generic inventory audit.
3. Once those checks are closed or pending independently, continue the next open P1 stability/security slice in another userland module. Do not wait on pending XY-C1 work before doing independent work.

## Mandatory handoff checkpoint

Every session MUST perform an end-of-session accuracy checkpoint. Check the live `main` SHA, frozen backup SHA, exact-head Actions, completed/failed/pending tests, unresolved blockers, and the precise first action for the next session. Earlier-SHA PASS results do not validate a later SHA. **The checkpoint is mandatory; a document edit is conditional.** When all checkpoint facts and the first action are already accurate, conclude the check without a no-op file change or commit.

# Cross-session execution mandate

- Treat the repository as one continuous engineering effort across GPT conversations. Recover once, resume the exact unfinished task, and do not ask the user what to do next.
- Complete each logical source/test/workflow slice with **one atomic main commit**: create every required blob, one complete tree and one commit; update `main` only through a non-forced expected-SHA compare-and-swap. Never create partial-file commits or alter the frozen backup.
- After every commit, inspect Actions for that exact SHA. PASS permits the next slice; FAIL requires diagnosis and repair; PENDING is not a stop condition while independent work remains.
- Before context exhaustion, finish and commit a safe bounded slice, verify its exact-SHA tests/build boundary, and update this handoff with the committed baseline and precise first action. Keep production-source and documentation-only SHAs distinct.
- Preserve hardware/L4 as NOT VERIFIED unless actual hardware validation has been performed.
