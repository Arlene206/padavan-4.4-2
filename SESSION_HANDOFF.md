# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record the exact main source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a freshly fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; main source SHA before the current rstats slice: \`661924b3b5cb309988758820c285f6bedeab3c23\`.
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- EAPoL config hardening remains in main; dedicated test and process-lifecycle signature contract were corrected in prior commits.

## Active task

\`rstats.c::save_history_raw()\` previously reset \`g_store_cntr\` to zero before calling \`write_storage_to_mtd()\`, then ignored the flash script result. A failed automatic MTD save could therefore defer retry for another complete configured interval (up to 30 days). The current source change captures status/errno; a successful save clears elapsed time while a failed save saturates the counter at the due threshold, logs, and retries on the next normal rstats tick. A GNU89 host policy test and the existing rstats workflow source/path contract cover retry semantics.

## Verification boundary

- Parent SHA \`661924b3b5cb309988758820c285f6bedeab3c23\`: Router mode NVRAM #45038940605, detect-link #45038940465, process inventory #45038940527, rstats #45038940393 and DNS Forwarder #45038940423 completed SUCCESS.
- Process lifecycle regression #45038940342 completed FAILURE in its “Verify shared-library and RC integration boundaries” step. Its failing assertion was not yet retrieved; inspect its job log and correct only the demonstrated contract/source issue.
- XY-C1 #45038940508 was still in Build XY-C1; TRX verification and artifact upload were not yet confirmed. Do not claim L3 without Build, TRX and artifact upload PASS on the same exact commit.
- No local checkout or C/Python tests were run in the connector-only session. Router hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head rstats policy/source contract, process-lifecycle regression failure, and XY-C1 Build/TRX/artifact state for the current main SHA; diagnose and correct failures.
2. Continue the repository-wide stability/security rotation in another userland subsystem after those checks.
3. Keep the backup immutable and continue all development directly on \`main\`.
