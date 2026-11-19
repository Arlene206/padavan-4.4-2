# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current main before this focused regression correction: `ba47b5f7425cfef4dab3f191ce971cefce329006` (`fix(shared): bound pidfile and legacy procps I/O retries`), parent `437c7b289b77cf601544715525a1488dcf14575e`.
- Exact-head Process umbrella `44634242619` and AI handoff docs `44634242529` PASS on `ba47b5f`; NVRAM audit `44634242550` and XY-C1 `44634242726` were still IN_PROGRESS at last check. The Process job includes the new PID-file read/write policy regression.
- On parent `437c7b2`, Process umbrella `44633888470`, AI docs `44633888315`, watchdog `44633888607`, Inadyn `44633888240`, crond `44633888255`, vsftpd `44633888215`, networkmap `44633888756`, infosvr `44633888277`, wireless `44633888761`, WAN route `44633888640`, MiniUPnP `44633888430` and NVRAM `44633888682` PASS. NFS `44633888395` and Samba `44633888489` failed because their shared service-wait test could not compile: `fixture_sleep` was undeclared. This commit adds the missing fixture and explicit seconds-argument assertions.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. No physical-router result is available.

## Active task

- Fixed an exact-head CI defect in `trunk/user/rc/src/tests/service_wait_policy.c`: `test_repeated_sleep_eintr_is_bounded()` referenced missing `fixture_sleep` and treated a nanosleep fixture as a seconds-sleep fixture. Added a dedicated deterministic sleep fixture/callback; its nine calls all request the same unslept one-second remainder, asserting the eighth retry ceiling returns `EINTR` on the ninth call. Existing nanosleep cases remain separate and unchanged.
- The prior shared process-I/O slice `ba47b5f` adds bounded wrappers for PID-file reads/writes, inline proc-stat reads and legacy procps; Process umbrella run `44634242619` passed, including its read/write callback regression.
- No local compiler, firmware build or physical-router result is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and verify this commit's AI docs and Process regression, plus NFS daemon and Samba workflows rerun after the fixture correction. Inspect exact job logs for any new failures.
2. Check XY-C1 run `44634242726` for source SHA `ba47b5f` through Build XY-C1, TRX verification and artifact upload; L3 requires all stages PASS on the same source SHA.
3. Continue another independently evidenced cross-userland stability/security issue; do not spend the next session re-auditing completed retry loops.

## Verification boundary

- L1: service-wait seconds/nanosleep wrappers in `437c7b2` use an eight-consecutive-EINTR ceiling and preserve remaining-time semantics.
- L2: PID-file/legacy procps bounded I/O policy regression passes in Process umbrella run `44634242619` on `ba47b5f`. The missing seconds-sleep fixture found by NFS run `44633888395` and Samba run `44633888489` has been corrected here; exact reruns remain pending.
- L3: XY-C1 run `44634242726` on `ba47b5f` was in progress; verify Build/TRX/artifact before claiming L3.
- L4: real MIPS/uClibc signal-storm timing and physical-router behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
