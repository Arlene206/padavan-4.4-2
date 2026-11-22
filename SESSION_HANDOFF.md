# Cross-session execution mandate

## Mandatory handoff checkpoint

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; parent main SHA: \`6dfdf5c86fbbee4a29538401b9702b74a473ad6a\`.
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- EAPoL credential config hardening and rstats retry-on-failed-periodic-MTD-save are in main. The rstats workflow still needs its new policy header added to both push and pull path filters.

## Active task

The Process lifecycle regression #45039420819 on SHA \`7c445837a2eb9ffa747e5c2b5d94b88d58276903\` failed its source contract because \`notify_reset_detect_link()\` did not name the \`PROCESS_PRESENCE_START_NOTIFY\` branch explicitly; it used a generic \`else\`. The integration contract also requires the explicit UNKNOWN-inventory diagnostic. This source change makes NEW, NOTIFY and ABORT states explicit, logs UNKNOWN inventory before returning the normalized error, and keeps signal errors visible to callers.

## Verification boundary

- On SHA \`7c445837a2eb9ffa747e5c2b5d94b88d58276903\`, process lifecycle #45039420819 failed only at the source assertion \`PROCESS_PRESENCE_START_NOTIFY in body\`, after all preceding GNU89 C/ASan/UBSan tests and other integration checks passed. Exact traceback was line 1241 of \`scripts/check_process_lifecycle_integration.py\`.
- AI handoff run #45039420870 on the same SHA failed because the handoff omitted the required literal \`Before context exhaustion\`; commit \`6dfdf5c86fbbee4a29538401b9702b74a473ad6a\` restored that required rule. Its exact-SHA validation remains pending.
- Rstats run #45039333829 on product-source SHA \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\` failed after the C policy test and source checks because the new \`rstats_persistence_policy.h\` was missing from push/pull path filters. Its watched-path tuple was already updated. The filter fix is the next small task.
- XY-C1 #45039333875 on product-source SHA \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\` was still building at last inspection; TRX and artifact upload were not confirmed. L3 is NOT VERIFIED. No local tests were run; hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect this commit's exact-SHA Process lifecycle and AI handoff runs; diagnose any further failures.
2. Add \`trunk/user/rc/src/rstats_persistence_policy.h\` to the rstats workflow push and pull path filters, then inspect its exact-SHA test.
3. Continue a different repository-wide userland stability/security slice. Keep the backup immutable and work directly on \`main\`.
