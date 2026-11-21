# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Every logical task is one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish it atomically, and preserve the exact next action here. Do not start work that cannot be safely completed.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live `main`, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the source baseline, test evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Parent `main` SHA for this continuation slice: `5a5981318b1b15c6d1d16d4ddf9725206c0f5c3d`; the latest product-source commit touching MiniUPnP `iptcrdr.c` is `0b5eef8bd00fd6e732449bcf241949df84b4f40c`; the latest errno-runner path commit is `c492d33b10baf96d8a971c4adc6773b3df5808f3`.
- The frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. The branch inventory contains only `main` and this preserved backup.
- Exact-head AI docs run `#44810641058` on `5a5981318b1b15c6d1d16d4ddf9725206c0f5c3d` failed because this file omitted the required phrase `Before context exhaustion`; this commit restores that rule and the required `Current repository state` heading.
- MiniUPnP run `#44810270387` was on parent `bb4a1910acd73c2ae9433d66971bdcd4313d4f23`, not the current parent SHA. Its GNU89/Werror/ASan/UBSan policy steps and both production IPTC errno-helper API variants passed; the delete source-contract step failed only because it still expected the pre-refactor direct `primary_failure_errno = ENOENT` and `secondary_failure_errno = errno` assignments. Production now correctly records both through `miniupnpd_record_first_failure_errno()`; the stale assertions are corrected in this commit.
- XY-C1 run `#44809783977` passed Build XY-C1, TRX verification and artifact upload for product-source SHA `0b5eef8bd00fd6e732449bcf241949df84b4f40c`. Later commits through this parent changed tests/checkers/docs, not product source; L3 is therefore verified for that exact current product tree. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Validate this checker/handoff correction on its exact pushed `main` SHA. Confirm the MiniUPnP delete-compensation source contract now accepts the first-error helper assignments while continuing to reject direct bypass assignments.
2. Confirm the AI handoff documentation contract passes with the required current-state heading and context-exhaustion rule.
3. If either Action fails, inspect the earliest causal log and repair that specific source/test/document contract without weakening coverage. If both pass, close this MiniUPnP validation slice and select the next high-priority user-space reliability defect from the whole-repository roadmap, avoiding repeated audits.

## Verification boundary

- The last observed MiniUPnP C policy and errno-injection binaries passed on parent `bb4a1910acd73c2ae9433d66971bdcd4313d4f23`; its only observed failure was the stale delete source-contract assertion described above. This commit requires a fresh exact-head run.
- The handoff checker failed on parent `5a5981318b1b15c6d1d16d4ddf9725206c0f5c3d` only for the missing `Before context exhaustion` phrase; this commit requires a fresh exact-head AI docs run.
- Exact-source XY-C1 L3 is verified for product SHA `0b5eef8bd00fd6e732449bcf241949df84b4f40c` as all three target steps passed on run `#44809783977`. No later product source change has been made.
- Physical-router L4 is NOT VERIFIED. No local checkout or local test execution was available in this session.

## Exact first action for the next GPT

Fetch live `main` and inspect the exact-head MiniUPnP and AI handoff Actions created by this commit. Read the MiniUPnP delete source-contract result and AI documentation result. Repair the first causal failure if either is red; otherwise continue with the next ranked user-space reliability issue.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
