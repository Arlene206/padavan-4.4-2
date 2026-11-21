# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Every logical change must be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, publish atomically and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, the product-source baseline, verification boundary, blockers and the first next action with this record. Update it only when one of those resumable facts materially changes; do not create a no-op commit.

## Current repository state

- Live `main` before this checker correction: `accf1726b163d54bd269019fd590554fc430fa0e`. The latest product-source baseline remains `8a55d24a4b797c96ca94bc8ece112966d86fa204`; main commits since that source change have affected workflow/checker/handoff files only, not firmware source.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (permanently read-only).
- Watchdog workflow regression `#44885579764` passed on main SHA `9a1175e8`. AI handoff `#44885716352` failed on SHA `accf1726` because the validator also requires the literal phrase `one atomic main commit`; the previous wording described an atomic commit but omitted the word `main`. This checkpoint now uses the exact required phrase.
- On product-source SHA `8a55d24a`, Process inventory `#44885238990`, HTTPD lifecycle `#44885238949`, DNSMasq lifecycle `#44885238979`, HTTPD security `#44885238957` and NVRAM audit `#44885238989` passed. Aggregate Process lifecycle `#44885238958` failed at the final watchdog integration contract because two assertions still required the old direct `return launch_status;` behavior. This commit updates both duplicate checker sections to validate the current six-branch rollback implementation and its adapters.
- Watchdog source commit `8a55d24a` had its first signal workflow fail on an obsolete direct-return assertion (`#44885239087`). After updating and deduplicating the workflow contract, the exact-head Watchdog run `#44885579764` passed on docs-only child `9a1175e8`; no production source changed between those SHAs.
- XY-C1 run `#44885238962` remains IN_PROGRESS on exact product-source SHA `8a55d24a`: all earlier host regression and input-detection steps passed; Build XY-C1 was running at last inspection; TRX output verification and artifact upload were pending.

## Active task

1. Validate this checker/handoff correction with exact-head push Actions for Process lifecycle and AI handoff. The Watchdog regression was green on `9a1175e8`; confirm it remains green if a new relevant run is triggered. Fix the first causal failure without weakening assertions.
2. Recheck XY-C1 `#44885238962` on product-source SHA `8a55d24a`. Do not claim L3 until Build XY-C1, Verify TRX output and artifact upload all succeed on that exact SHA.
3. Once the rollback slice and source contracts are green, resume the shared PID-file audit at `trunk/user/shared/src/shutils.c::kill_pidfile_s()`: map every consumer to its producer format, owner, lifecycle and signal semantics before considering any migration. Preserve numeric-only external PID-file compatibility unless a source-backed safe change is proven.

## Verification boundary

- Product-source baseline: `8a55d24a4b797c96ca94bc8ece112966d86fa204`; this follow-up changes no firmware production source.
- The GNU89/ASan/UBSan watchdog policy binaries and the majority of Process lifecycle host regressions passed in the prior Actions run. The remaining Process failure was a stale source-contract assertion, not a demonstrated production C test failure.
- No local checkout/compiler is available in this session; no local Python, shell or C test is claimed. Exact-head GitHub Actions are the validation source.
- XY-C1 firmware L3 remains pending on `8a55d24a`. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect the exact-head Process lifecycle and AI handoff push Actions for this correction; diagnose the earliest remaining failure without removing coverage. Recheck XY-C1 `#44885238962` for Build, TRX verification and artifact upload on source SHA `8a55d24a`. Once these checks pass, start mapping the readers and producers of `kill_pidfile_s()` in `trunk/user/shared/src/shutils.c`.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
