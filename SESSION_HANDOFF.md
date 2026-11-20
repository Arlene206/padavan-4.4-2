# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent `main` before this slice: `6f59f340868cd43c458f7a6b7fc676e7d6b0db04`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- All 12 push Actions on exact parent `6f59f34` passed, including Process lifecycle/inventory, AI handoff docs, Inadyn, DNSMasq, WAN route, HTTPD, Samba and NVRAM checks.
- XY-C1 `44661978626` on `6f59f340868cd43c458f7a6b7fc676e7d6b0db04`: Build XY-C1, TRX verification and artifact upload all SUCCESS. Artifact `14596327119` is present and not expired. This L3 evidence applies to the parent SHA only.
- Historical PR #1 is CLOSED/UNMERGED. Its CA trust-store bootstrap and AliDDNS HMAC helper/tests/workflow integration are already present on current `main`; no remaining core PR implementation needs porting.
- Physical-router L4 remains NOT VERIFIED.

## Active task

- This slice fixes PID 1 early-boot status loss in `init.c::init_sysctl()`. Previously five integer kernel writes, IPv4 `rp_filter`, and the IPv6 global-disable operation discarded their results.
- The new code logs every failed operation while continuing independent settings, preserves the first failure, and merges it into router readiness as DEGRADED. Fatal router prerequisites remain FATAL; PID 1 recovery/event handling is not disabled by an optional sysctl failure.
- The result-policy regression covers successful, positive/negative failure, already-degraded, fatal and unknown states. `scripts/check_dev_init_boot.py` pins production call ordering; the existing Process workflow already runs the GNU89/ASan/UBSan test and source contract.

## Verification boundary

- L1 source review: PASS for the seven integer-returning operations captured by this slice. The legacy `set_tcp_tweaks()` and `set_libc_gai()` helpers are void and still hide internal write failures; they are explicitly the next task, not claimed fixed here.
- Exact-head Process lifecycle/inventory, AI handoff docs and XY-C1 Actions for THIS COMMIT must be fetched by its exact `head_sha`; parent SHA results must not be reused.
- Current-source L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all pass on one matching SHA. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Resolve live `main` HEAD and inspect Actions runs whose `head_sha` exactly matches it; diagnose any FAIL from job logs.
2. Require same-SHA Process GNU89/ASan/UBSan test, `check_dev_init_boot.py`, AI docs contract, XY-C1 Build, TRX verification and artifact upload before closing L2/L3.
3. Continue the next concrete PID 1/network sysctl slice: inspect `net.c::set_tcp_tweaks()` and `set_libc_gai()`, both currently void and discarding `fput_int`/stdio/unlink errors; trace all callers in `init.c` and `net6.c` before changing signatures. Preserve best-effort startup and merge failures into first-status accumulators.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
