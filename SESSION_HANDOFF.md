# Cross-session execution mandate

Continue the same repository work across GPT conversations. Recover once, then execute; do not repeatedly audit completed work or ask the user what to do next.

- One logical slice must be completed as source changes, appropriate regression tests, one atomic main commit, and exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree, one commit. Update main only with a non-forced compare-and-swap against the expected parent SHA. Never force-push or expose a multi-file half-state.
- Before context exhaustion, finish the current safe slice, verify the live main SHA and exact-head tests/Actions, and update SESSION_HANDOFF.md with the first next task and truthful status.
- Before context exhaustion, record only facts actually checked. A pending run is not a stop condition for independent work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: current main SHA, backup branch SHA, exact-head relevant Actions, local/host regression results, and whether the next task is still valid.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Current product/source commit: `68b4958d3b6bc6ee3636e8ff1f479fbabdecc679` (`fix(libdisk): continue folder permission cleanup after errors`).
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not modify it.
- The latest source slice centralizes per-account folder permission updates with bounded pathname construction, checked reads, line-preserving rewrite and atomic replacement. `del_folder()` and `mod_folder()` now continue across independent accounts and anonymous FTP after a per-file failure, preserve the first error, and report it after cleanup. `get_permission()` fails closed on a truncated account metadata path.
- Existing libdisk enumeration, folder snapshot, account-default, add-folder initialization, bounded reader, and permission write checks remain in place.

## Active task

- Repair the current libdisk source-contract false negative: its whitespace compaction currently preserves one space after `(`, so it cannot match the exact account-updater call even though source behavior is correct. Normalize by removing all whitespace, then verify the checker.
- Repair the handoff-format regression by retaining the required checkpoint and cross-session execution sections in this file.
- After exact-head CI passes, inspect `add_account()` / `del_account()` account lifecycle. `add_account()` ignores `initial_one_var_file_in_mount_path()` errors on mounted volumes; `del_account()` truncates unchecked per-volume metadata paths, ignores unlink errors, and shortens `acc_num` before freeing the original account list (leaking its final element). Preserve NVRAM semantics while adding bounded paths, error aggregation and regressions in a separate logical slice.

## Verification boundary

- On parent SHA `68b4958d3b6bc6ee3636e8ff1f479fbabdecc679`, libdisk run #45019373684: both GNU89 `-Wall -Wextra -Werror` ASan/UBSan executables passed; Python source contract failed on the whitespace-normalization false negative. No C test failure was observed.
- On the same SHA, AI handoff run #45019373672 failed because the previous handoff rewrite omitted this mandatory checkpoint statement. The current file restores the required exact phrases and cross-session sections.
- On the same SHA, Process lifecycle inventory #45019373666 PASS and HTTPD Authentication/Security #45019373705 PASS. XY-C1 #45019373712 and NVRAM transaction audit #45019373665 were in progress at the last completed inspection; fetch current live results before using them.
- XY-C1 firmware L3 remains NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head libdisk, AI handoff, Process inventory, HTTPD, NVRAM and XY-C1 Actions for the latest commit. Fix real failures from logs; do not infer PASS from an ancestor SHA.
2. Once libdisk and handoff checks pass, implement the `add_account()` / `del_account()` per-volume metadata lifecycle defects with bounded paths, per-volume error handling, correct account-list cleanup, and source/host regression coverage. Keep account add and delete changes independently reviewable.
3. Preserve the frozen backup branch. Claim XY-C1 L3 only after same-SHA Build, TRX and artifact upload pass.

## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched. Update it only when a material source/CI fact or the first next task changes.

