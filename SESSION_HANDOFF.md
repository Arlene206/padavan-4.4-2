# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Parent for the PID 1 degraded-readiness slice: `b35b6a7aaa2b5018c8c5dac85b2aa81bb6074c35`. Current-main VPN lifecycle regression `44640556013` and AI handoff documentation contract `44640556098` both PASS on that exact SHA.
- rp-L2TP configuration publication hardening is implemented in `2b646cccdcb24da5d447586893a500fdcf3cfe48`; its GNU89 ASan/UBSan filesystem regression passed. Exact XY-C1 run `44639411601` completed successfully on that exact product SHA: Build XY-C1, TRX verification and artifact upload all PASS; artifact `14584429568` (12,678,233 bytes; SHA-256 `db6fc9199ef0798b75ec8c3d3bc98bc9becee50c07474b6e1fbada14d9df4bc8`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is archival and untouched. Develop on main only.
- The prior VPN checker path-filter regression on `40eb3ad8bde1b728c449d907208886fa751f8f55` is corrected; current-main exact VPN and AI docs checks pass. No physical-router L4 result is available.

## Active task

- P1 boot-result defect: `init_router()` already aggregated hardware/module and service startup failures in `boot_services_status`, but its final `return 0` discarded that result. PID 1 therefore could not distinguish complete startup from a router with degraded services.
- This slice introduces explicit `FATAL`, `READY`, and `DEGRADED` results. The four existing mandatory NVRAM/configuration aborts remain fatal; a nonzero accumulated boot-service status becomes degraded, while zero remains ready.
- PID 1 now treats READY and DEGRADED as router-ready, retaining signal notifications and deferred hotplug handling after partial service startup. Mandatory `dev_init.sh`/NVRAM prerequisites and fatal `init_router()` failures still keep router event handling disabled and preserve console recovery.
- Added GNU89 policy regression coverage and extended the PID 1 source/workflow contract. Local GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy binary passed. Exact-head Process, AI docs and XY-C1 Actions for this new source commit must be checked after publication.

## Exact first action for the next GPT

1. Query exact-head Process lifecycle, AI handoff docs and XY-C1 results for the PID 1 result-policy commit. XY-C1 L3 is complete only when Build, TRX verification and artifact upload all pass on that same source SHA.
2. Repair only demonstrated checker/build failures; keep READY/DEGRADED event handling enabled and mandatory prerequisite failures fatal.
3. Continue the open P1 RC startup-result propagation queue at the next unclosed caller in `start_services_once()`/WAN service dispatch; do not repeat the closed void-wrapper caller audit or completed rp-L2TP/netmask/MTD slices.

## Verification boundary

- L1: router startup result now distinguishes fatal prerequisite failure, fully ready, and degraded boot-service status; the PID 1 readiness classifier admits only READY/DEGRADED.
- L2: local GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy regression passed. Exact-head Process source integration and AI docs Actions are pending for this commit.
- L3: prior rp-L2TP product SHA `2b646cc` passed XY-C1 Build/TRX/artifact, but the current `rc.c`/`init.c` source change requires its own exact-head XY-C1 Build/TRX/artifact pass.
- L4: target MIPS/uClibc runtime and physical-router startup/recovery behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Before context exhaustion

- Finish the current safe source/test slice and commit it atomically before handoff; do not publish related multi-file changes as sequential partial commits.
- Re-fetch live `main`, create one complete Git tree and one commit with that exact parent, then update main only with `force=false` and the exact parent as `expected_sha`. Never force-push.
- Query relevant Actions by exact source SHA and record PASS/FAIL/PENDING with run IDs. Pending validation does not block independent work and must not be reported as complete.
- Record the production-source baseline, test commands/results, exact CI state, unresolved limitations and the next concrete task. Distinguish host L2, firmware L3 and device L4 evidence.
- Confirm all work is committed and remotely present before claiming it is preserved across conversations.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
