# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: Arlene206/padavan-4.4-2. The authorized development branch is main. The parent for the current FRP credential seeding source/test slice is 5378fccc2201f4359ea3b0b37f1043d2b813a883. This commit is intended to include source, regression, log and handoff as one atomic commit; resolve THIS COMMIT from Git history and do not create a follow-up commit just to replace its placeholder SHA.
- The immutable archival ref backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Never modify, delete, rename, merge, rebase, cherry-pick, reset, prune, or develop from it.
- Branch inventory at this checkpoint contains main and the frozen backup only. Do not create feature, fix, temporary or checkpoint branches. No pull request development.
- There is no local repository checkout available in this session. Source changes are prepared by GitHub Git Data API; do not claim local shell tests, compiler runs or firmware builds.
- Legacy PR #1 is closed and unmerged, head ba48e22609e0c81ad757b06756fe91d05f39cbf6 and base 9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74. The inspected AliDDNS HMAC helper, OpenSSL 3 EVP_MAC code, CA trust-store bootstrap, tests and package/workflow integration are already present in main. Do not reopen or develop its old head.

## Active task

### Replace the FRP credential direct-copy exception with tested exclusive publication

- Files in this logical slice: trunk/user/scripts/files/usr/bin/copyscripts.sh; trunk/user/scripts/tests/copyscripts_ca_status.sh; dev_log.md; SESSION_HANDOFF.md.
- The ordinary seed_storage_file() helper validates that /etc/storage and its parent are real directories, rejects a symlink or non-regular leaf, creates a same-directory mktemp file, copies and chmods it, then publishes with ln (exclusive hard-link creation). Existing normal three-argument callers preserve operator-managed files without changing their permissions.
- The FRP credential-bearing script used a separate symlink test followed by cp -f /etc_ro/frp_script.sh. That check/use split allowed a leaf symlink to be introduced between check and write, and duplicated the common helper policy.
- The current fix extends seed_storage_file() with optional fourth argument 1, meaning enforce the requested file mode for an existing regular file or a concurrently published regular file. Omitted fourth argument defaults to 0, preserving every existing three-argument caller. The FRP config is seeded with the existing helper, source /etc_ro/frp_script.sh, destination /etc/storage/frp_script.sh, mode 700, enforce-existing-mode flag 1; it shares no-follow path checks, staged copy and exclusive link publication.
- If FRP mode repair fails, helper sets copyscripts_seed_status=1 and returns failure, so the existing final boot-preparation status gate reports the problem. Operator-managed file content is preserved while its sensitive mode is restored. The fresh target is never overwritten by a concurrent destination.
- Regression additions test new FRP file creation at mode 0700, existing operator content preservation plus mode repair, injected chmod failure with status propagation, and static source assertions preventing the old direct cp path. Existing Process lifecycle workflow path filters already include both changed source and test paths and runs this regression.
- Limitation: this shell helper follows the existing repository trust model for /etc/storage ownership. Do not describe the change as a universal kernel-level elimination of all path races; it replaces the known direct cp -f gap with the same exclusive no-follow staged publication policy already used for other storage defaults.

### Exact parallel baseline and Actions

- The libdisk ambiguous ACL source fix on e757f86309b19f890797b2f36fb15f44f87a7e95 was followed by checker-only commit 5378fccc2201f4359ea3b0b37f1043d2b813a883, which defined the missing get_permission_body and set_permission_body source-checker regions.
- Exact descendant libdisk workflow 45114769360, push, head 5378fccc2201f4359ea3b0b37f1043d2b813a883, completed SUCCESS. Both GNU89 ASan/UBSan regressions and the Python source contract passed.
- e757 exact XY-C1 Firmware Validation run 45114634020 remains IN_PROGRESS at this checkpoint; inspect Build XY-C1, Verify TRX output and artifact upload. Only those three successful stages on the same product-source SHA close L3.
- e757 NVRAM transaction boundary audit 45114633929 completed SUCCESS.
- e757 HTTPD Authentication and Security Regression 45114633840 completed SUCCESS. e757 Process lifecycle inventory 45114634024 completed SUCCESS. These do not validate the FRP source change.

## Verification boundary

- L1: source-level issue and intended fix reviewed. The old FRP cp -f path is gone from the changed source; ordinary three-argument helper use remains unchanged and FRP gets the fourth private-mode flag.
- L2: NOT YET VERIFIED for this FRP slice. There is no local checkout; the expanded copyscripts_ca_status.sh was not executed locally. Require the exact-head Process lifecycle regression push Action to run and pass the shell regression and full source contracts.
- L3: NOT YET VERIFIED for the e757 ACL source until run 45114634020 completes Build XY-C1, TRX verification and artifact upload on e757. The FRP source change also needs its own exact new-SHA XY-C1 production-input detector, target Build, TRX and artifact upload. No different SHA's green check substitutes.
- L4: NOT VERIFIED; no physical router test is evidenced in this session.

## Exact first action for the next GPT

1. Fetch live main before acting and verify the frozen backup still equals 72f4f157a084f8184b9597ca5bf334b8080e5807. Resolve THIS COMMIT from history and inspect its complete diff; do not assume this handoff's parent is still main.
2. Inspect push Actions whose head_sha exactly equals the FRP commit. Start with Process lifecycle regression and verify the copyscripts shell regression, including the injected chmod failure and direct-cp source contract. On failure, inspect the first causal step/log and fix it without repeating already-passed CA bootstrap tests.
3. Inspect same-SHA XY-C1 detector, Build XY-C1, TRX output and artifact upload. Claim L3 only if all target stages pass for this exact production-source SHA.
4. Revisit e757 XY-C1 45114634020 if still open. NVRAM audit 45114633929 is already SUCCESS. Do not wait idly: continue the highest-risk independent userland defect after the FRP regression is committed and exact-head checks are recorded.
5. Do not re-audit completed libdisk duplicate ACL lookup or BusyBox PID1 exact-child work without new evidence. Rotate to another concrete source-backed stability/security defect across the userland.

# Cross-session execution mandate

- The same maintainer continues the same repository mission. Recover live state once, resume the recorded task and implement; do not ask the user what to do next or repeat completed audits.
- Stability bug discovery and repair is primary; security hardening continues in parallel. Critical vulnerabilities, data corruption, boot failures and severe service faults take immediate priority. Cover the integrated Padavan userland rather than remaining in one subsystem.
- Each logical source/test slice is one atomic main commit: fetch the latest main parent, create every blob, one full tree and one commit, then update main through non-forced compare-and-swap using expected_sha. Do not create sequential partial per-file commits. Never force-push, amend, rewrite history, branch for development or use PRs.
- Verify the landed main SHA and complete diff. Inspect relevant push Actions only when head_sha exactly equals that commit. Diagnose the first causal failure. PASS closes that check; FAIL requires a fix; PENDING does not block independent work and is not a reason to poll idly.
- Preserve security, architecture, compatibility and necessary regression tests. Review callers/callees before source edits. Do not weaken checks for a green result.
- Distinguish L1 source review, L2 host test/build, L3 MIPS/uClibc target build + TRX + artifact on the same production-source SHA, and L4 physical router validation. A static checker or skipped build is not L3/L4.
- Keep GPT Session Rotator unchanged. Do not reset or replay SHA 9d5a5cb or alter its quiet-period/de-duplication behavior.
- Before context exhaustion, finish the current safe slice, inspect its diff, publish and check exact-head Actions. Leave an accurate first action for the next GPT.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live GitHub before ending and compare the meaningful source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup identity and exact first action with this file. Update this handoff only if next-session-critical facts materially change. If it remains accurate, explicitly conclude the check with no file change and no commit. Do not chase documentation-only SHA changes or create a no-op commit.
