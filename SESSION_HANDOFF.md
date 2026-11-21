# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Every logical change is one atomic main commit: create every blob, one complete Git tree, one commit, then update main without force using the exact expected parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regression coverage for source fixes; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, publish atomically and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, the product-source baseline, verification boundary, blockers and the first next action with this record. Update it only when one of those resumable facts materially changes; do not create a no-op commit.

## Current repository state

- Live `main` before this legacy PID-file hardening slice: `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`. Production-source baseline: `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact-head Process lifecycle `#44887442755` and AI handoff `#44887442739` both passed on `1ab5e535`. Watchdog signal regression `#44885579764` passed on `9a1175e8`.
- XY-C1 `#44885238962` completed SUCCESS on exact production-source SHA `8a55d24a4b797c96ca94bc8ece112966d86fa204`: Build XY-C1, Verify TRX output and artifact upload all passed. Firmware build L3 is VERIFIED for this source baseline; physical-router L4 remains NOT VERIFIED.
- PID-file audit: legacy `kill_pidfile_s()` in `trunk/user/shared/src/shutils.c` used `fgets` then `strtoul(..., NULL, 0)` and cast before range checking. This accepted numeric prefixes with trailing garbage and could wrap out-of-range values on wider hosts. The helper remains exported for PID-only compatibility. RC/HTTPD/shared callers examined use identity-bound `process_pidfile_signal()`; LPD writes identity records and permits a separately validated PID-only legacy fallback. No direct call was found in the 383 selected active-package C files plus scanned RC/shared/HTTPD files; the broader package scan remains incomplete.
- This patch hardens the legacy helper without changing its PID-only disk format: nonblocking open avoids FIFO hangs; fstat requires a regular file; bounded full-record parsing rejects embedded NUL, trailing junk and overflow; historical decimal/octal/hex values and surrounding whitespace remain accepted.

## Active task

1. Validate this legacy PID-only parser hardening with exact-head Process lifecycle Actions. Check the GNU89/ASan/UBSan parser regression, source integration assertions and workflow path filters; fix the earliest causal failure without weakening coverage.
2. Continue the remaining higher-risk PID-file caller/producer scan in unreviewed packages (including LPRng, u2ec, DNS and user daemons); map record format, path, owner and signal semantics before considering migration. Preserve numeric-only external records.
3. Retain XY-C1 L3 as verified only for exact SHA `8a55d24a`; physical-router L4 is still not verified.

## Verification boundary

- This slice hardens the exported shared PID-only compatibility helper; it does not migrate service PID-file formats or change identity-bound process PID-file APIs.
- The standalone parser regression was independently compiled and run locally with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan: PASS. Exact-head Process Actions remain required to validate integration.
- XY-C1 firmware L3 is VERIFIED on `8a55d24a` (build, TRX verification and artifact upload all PASS). Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and verify the frozen backup SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect exact-head Process lifecycle and AI handoff Actions for this hardening; correct the first causal failure without weakening checks, then resume the remaining package scan for legacy PID-only consumers/producers. Preserve the external numeric-only record contract.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep frozen backup permanently unchanged.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
