# 2026-11-19 continuation checkpoint — WAN first-error contract normalization

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`, development on `main`.
- Main source follow-up commit: `18a73adeea90de2935a07cad347e1f214a76966c`; its checker/handoff correction commit: `1c368975b354e4b8e7277146f86891cf6cb34f55`. Always fetch the live ref before changes.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. PR #1 remains closed/unmerged.
- Parent `18a73adeea90de2935a07cad347e1f214a76966c` exact WAN route `44443755126` passed. USB modem `44443755296`, Process lifecycle `44443755214`, AI docs `44443755298`, and XY-C1 `44443755078` failed at exact source checker/handoff contract (AI docs separately failed because of omitted governance text). Do not transfer those statuses to newer SHAs.
- On `1c368975b354e4b8e7277146f86891cf6cb34f55`, AI docs `44444344544` passed; USB modem `44444344542`, Process lifecycle `44444344676`, and Firewall `44444344609` failed on one exact checker assertion. WAN route and Process inventory were not failing.

## Active task

The source checker calls `compact()`, which normalizes every whitespace run to exactly one ordinary space; it does not remove whitespace. The WAN source has a line break immediately after the `rc_first_service_start_status(` opening parenthesis. The checker expected a zero-space compact form, which can never match. Correct the callback merge, generic setup merge, and cleanup expressions to match the actual compact representation. This is a test/checker defect only; do not change production code to satisfy it.

## Verification boundary

- L1: source implementation for callback directory, VLAN and APCLI/WISP fail-fast preflights now merges into the chronological accumulator and returns first status.
- L2: GNU89/ASan/UBSan modem-WAN policy and Process policy binaries passed before the source contract assertion; USB modem, Process lifecycle and Firewall exact-head Actions failed only at that assertion on `1c368`. New checker normalization must pass on the next SHA.
- AI docs passed on `1c368` run `44444344544`.
- L3: XY-C1 exact run `44443755078` on `18a73` failed late because of the same checker; inspect whether Build, TRX and artifact upload finished before the checker. No L3 claim without all three successful stages on one exact source SHA.
- L4: router hardware/real WAN and modem bring-up remains NOT VERIFIED. No local C tests or MIPS build were run in this connector-only session.

## Exact first action for the next GPT

1. Fetch live `main` and backup refs, and inspect exact-head USB modem, Process lifecycle/inventory, Firewall, WAN route and AI docs Actions for the checker normalization commit.
2. Fix only evidence-based remaining failures. Do not loosen source contracts or change runtime code for a checker whitespace mismatch.
3. Once L2 passes, continue with a separate P1 defect in Padavan userland; do not re-audit already closed WAN paths.
4. For XY-C1, verify Build, TRX and artifact upload individually on the same exact SHA before recording L3 PASS.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Use the Git data APIs to create every blob, one complete tree and one commit from the exact live parent; update main only with non-forced expected-SHA compare-and-swap. Keep working across GPT conversations; pending checks do not block independent tasks, and the backup is immutable.

## Before context exhaustion

Record the exact source commit, regression results, exact-SHA Action IDs and conclusions, and unverified boundaries. Never claim local tests, builds, TRX, artifacts or hardware checks that did not pass.
