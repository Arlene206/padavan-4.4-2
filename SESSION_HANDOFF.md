# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current main parent for the MTD flash-I/O slice: `89879756a4e2741bbde4aa0460c346039c542e6c` (`ci: align firewall and WAN safety contracts`).
- Exact parent firewall source SHA `342631b4efb09b4877dd944d74f03d20da7b4164`: firewall-startup policy `44635384398` PASS; HTTPD `44635384245` and WAN DHCP callback `44635384375` failed stale checker assertions now repaired in `8987975`. NVRAM `44635384340` and XY-C1 `44635384278` were pending at last poll; exact Build/TRX/artifact not yet confirmed.
- Exact source SHA `ba47b5f`: Process umbrella, AI docs, GPIO, detect_link, HTTPD, Process inventory, VPN, crond, watchdog and NVRAM PASS; XY-C1 `44634242726` remained IN_PROGRESS when last inspected.
- Exact source SHA `39c3db7`: Process umbrella, Process inventory, AI docs, NFS, Samba, WAN route and Wireless PASS; NVRAM `44634616501` was IN_PROGRESS at last poll.
- Shared procfs/PID-file I/O fix on `ba47b5f` reuses the common bounded wrappers. Earlier procfs source SHA `53dda2c` passed XY-C1 Build/TRX/artifact `44632416166`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. No physical-router result.

## Active task

- Confirmed unbounded `EINTR` loops in shared `flash_mtd.c`: `read_safe()` and `write_safe()` retried raw `read(2)`/`write(2)` indefinitely. Repeated signals during flash partition operations could spin instead of returning an error.
- Reused `process_io_retry_policy.h` bounded wrappers for each syscall. Outer full-buffer loops still advance on positive short I/O, reads still return a short count at EOF, zero-progress writes still fail with EIO, and retry exhaustion propagates EINTR.
- Process lifecycle CI now checks both MTD callsites and watches `flash_mtd.c` on push and pull_request. XY-C1 also watches this production file, ensuring source changes trigger firmware validation.
- No local compile or target/router test is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process lifecycle, Process inventory, AI-doc, NVRAM and XY-C1 runs for the MTD retry fix.
2. On FAIL, inspect exact job logs and fix only demonstrated source/build problems; preserve short-I/O semantics and bounded EINTR failure. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on one source SHA.
3. Continue an independently evidenced cross-userland stability/security issue; do not reopen fixed lock, wait, procfs or PID-file retry paths without new evidence.

## Verification boundary

- L1: MTD flash `read_safe()`/`write_safe()` now use the shared eight-consecutive-EINTR-bounded wrappers; short read/EOF and short-write semantics remain intact.
- L2: Process lifecycle source contract pins both MTD callsites; common callback regression is covered by Process PID-file test targets. Exact-head Actions remain pending.
- L3: XY-C1 watches `flash_mtd.c`; Build/TRX/artifact for the new source SHA remains pending.
- L4: MIPS/uClibc MTD behavior under signal storms and physical-router firmware operations remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
