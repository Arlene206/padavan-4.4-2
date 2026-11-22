# Cross-session execution mandate

## Mandatory handoff checkpoint

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main SHA for this checkpoint: `ff25eacd70dccd78c6416c76ad199ad69aa8f61d` (`docs: refresh session handoff for continued stability work`). The frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it.
- The previous exact-SHA handoff run #45051742269 failed only because root `dev_log.md` had reached 120 complete records. This checkpoint rotates the oldest 101 records into `docs/dev-log/2026/11/part-027.md` and retains the newest 20 in the root; verify the resulting exact-SHA handoff workflow after publication.
- Exact parent SHA Actions: AI handoff run #45051742269 = completed/failure at the 120-entry rolling-log check. No other workflow run was returned for this exact SHA. Process inventory run #45051059927 passed on ancestor `372451d468fd772858635590f1c1c10bccf95dc0`; it is not an exact-parent result.
- XY-C1 L3 is NOT VERIFIED on current main. Historical XY-C1 run #45047957843 fully passed host validation, Build XY-C1, TRX verification and artifact upload on older SHA `c139dd5da2cbce53ee683d303f56e66d6b55dca0`. The later exact-source run #45050810796 on `6418e87add0ffce79fd918077bb1e7dfdded2fc0` passed host checks but skipped Build, TRX and artifact upload because no production input changed. Do not reuse ancestor results as current-SHA verification.
- Earlier XY-C1 run #45050298446 on `b5fbcebd47fa735f70d06064a8a8cb682f4b0229` failed in a stale source checker before firmware production stages; the L2TP checker contract was subsequently corrected and Process passed on `372451d...`. Current-head Build/TRX/artifact remains unverified.
- No local test suite, compiler, firmware build or router hardware validation was run in this session. L4 hardware remains NOT VERIFIED.

## Active engineering task

Fix the two-phase publication gap in `trunk/user/shared/src/shutils.c::recreate_passwd_unix()`.

Current flow stages passwd/group/shadow files, retains hard-link backups through the multi-file rename sequence, but `passwd_file_publish_transaction()` deletes those backups as soon as all renames succeed. Only afterwards does `recreate_passwd_unix()` call `change_passwd_unix(rootnm, root_password)`. If `chpasswd` or the subsequent shadow permission operation fails, the function returns an error while leaving the replacement shadow file (whose staged root entry is locked with `!`) in place; there is no longer a prior account database to restore. This can leave the device inaccessible after a password-provisioning fault.

First implementation slice:
1. Extend `trunk/user/shared/src/passwd_temp_file_policy.h` with a retain-backups publish path plus explicit finalize and reverse-rollback helpers. Preserve the current `passwd_file_publish_transaction()` contract for its callers.
2. Update `recreate_passwd_unix()` to retain old passwd/group/shadow recovery links until `change_passwd_unix()` succeeds. On password-install failure, restore the prior files in reverse order and preserve recovery links if restoration fails. On success, finalize by deleting the backups; report cleanup failures without reverting a successfully installed password.
3. Extend `trunk/user/shared/src/tests/passwd_temp_file_policy.c` with successful post-publish rollback, successful finalize, and rollback-failure/recovery-link tests. Update the process-lifecycle workflow's source contract accordingly.
4. Validate the GNU89 `-Wall -Wextra -Werror` and ASan/UBSan test step on the exact SHA; also check exact-SHA Process and XY-C1 workflows. A source change in shared RC support should trigger production input detection; only mark L3 after Build, TRX and artifact upload all pass on that same SHA.

Do not repeat completed WAN DHCP snapshot, LAN subnet snapshot, L2TP selector or VLAN-ID audits unless a new independent defect is demonstrated. Continue with other userland stability/security work once this slice is committed and its exact-SHA Actions state is known.

## Verification boundary

- Documentation parent: `ff25eacd70dccd78c6416c76ad199ad69aa8f61d`.
- This rotation includes 101 archived records, oldest-first, preserving the complete prior root records verbatim. Root `dev_log.md` is reduced to 20 complete entries.
- Local documentation checker was not run. Exact-SHA Actions results must be queried after the atomic commit is published.
- XY-C1 L3 and hardware L4 are NOT VERIFIED for this checkpoint.

## Exact first action for the next GPT

Implement the two-phase passwd/group/shadow transaction described above. Fetch the current versions of `passwd_temp_file_policy.h`, `shutils.c`, `passwd_temp_file_policy.c` and `.github/workflows/process-lifecycle-regression.yml`, make the helper and caller changes together, then run/check the exact-SHA regressions and firmware workflow. Continue from the freshly fetched `main`; never assume a SHA from an earlier chat is still HEAD.
