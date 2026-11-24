## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream. For every logical source/test slice, create all blobs, one complete Git tree and one commit, then update `main` with a non-forced expected-SHA compare-and-swap. Never make partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow never blocks independent work.

## Branch and checkpoint

- Current main: `7e700e86e191bfbf11d563fce7f9cfbbcd539516`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- `main` is the only development ref.
- PR #1 remains closed/unmerged and deeply behind main; do not port it wholesale.
- Do not repeat completed audits. Re-read the live main ref and exact-SHA Actions before each new write.

## Active stability fix — VSFTPD failed-upload cleanup and quarantine confinement

The prior public-leaf check/unlink pair had a TOCTOU race: another writer could replace the upload leaf between inode comparison and unlink. Cleanup now atomically moves the leaf with `renameat2(RENAME_NOREPLACE)` into an effective-UID-owned 0700 quarantine directory, verifies the moved inode through the pinned quarantine FD, and only unlinks a matching regular file there. Mismatches/unlink failures attempt a no-replace restore; a concurrent replacement is not deleted. No racy fallback is used when renameat2 is unavailable.

A second review found that lexical reserved-name rejection did not block a missing new leaf under a symlinked parent into the quarantine. Canonical reserved-path checks now protect write-parent, rename-entry and directory resolution. The GNU89 regression creates an alias to the quarantine directory and requires each of those entry points to reject it with EACCES.

## Exact verification boundary at main 7e700e8

- VSFTPD regression #45486633987: PASS. Both GNU89 `-Wall -Wextra -Werror` policy binaries and embedded source-contract checks passed, including quarantine inode verification and all three canonical-path alias fences.
- AI handoff contract #45486633983: PASS.
- Process lifecycle inventory #45486634249: PASS.
- NVRAM transaction boundary audit #45486634206: PENDING at raw userland ioctl / kernel API caller audit (job step 13). Earlier compile/exercise stages, including device-node permissions and lock inode/permissions/contention/crash release (steps 3–12), were PASS at last poll.
- XY-C1 firmware validation #45486634245: PENDING at `Build XY-C1` (step 38). Steps 1–37 were PASS at last poll. TRX verification (step 39) and artifact upload (step 40) have not completed. Do not claim L3 until Build, TRX and artifact upload are all PASS on this exact source SHA.
- The earlier failures are preserved in `dev_log.md`: d01 compile duplicate declarations; 4ee workflow formatting assertion; 77 rename-alias regression. The later commits corrected each cause; exact 7e VSFTPD run is now PASS.
- No physical-router run has occurred. Hardware L4 remains NOT VERIFIED.

## Exact first actions for the next GPT

1. Re-poll XY-C1 #45486634245 on exact SHA `7e700e86e191bfbf11d563fce7f9cfbbcd539516`. If Build passes, require the same run's TRX verification and artifact upload to pass before recording L3. If a step fails, inspect its log and fix the first cause in a new atomic slice.
2. Re-poll NVRAM audit #45486634206. If step 13 reports a concrete raw-ioctl/API bypass or unsafe caller, reproduce it, implement the narrow fix plus regression and exact source contract; otherwise preserve the auditor's bounded scope and proceed.
3. Continue with the next highest-risk unfinished userland item from `REFACTOR_ROADMAP.md`; avoid reopening the completed VSFTPD alias work or repeating already-passing inventory/NVRAM checks. Keep backup immutable and main writes expected-SHA/non-force only.
