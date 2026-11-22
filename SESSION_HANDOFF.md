# Cross-session execution mandate

## Mandatory handoff checkpoint

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; parent main SHA: \`e7d0cc1cb95e5b93387bd8362d5af77447105838\`.
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- EAPoL credential config hardening, detect-link status-bearing tri-state notification, and rstats retry-on-failed-periodic-MTD-save remain in main.

## Active task

The rstats policy/source change passed its earlier validation steps, but run #45039333829 on product SHA \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\` failed the final path-filter contract. The new \`rstats_persistence_policy.h\` was in the Python watched-path tuple but missing from the actual push and pull_request paths. This commit adds it to both filters.

## Verification boundary

- Process lifecycle #45039420819 on SHA \`7c445837a2eb9ffa747e5c2b5d94b88d58276903\` failed because the detect-link function used a generic \`else\` rather than naming \`PROCESS_PRESENCE_START_NOTIFY\`; commit \`e7d0cc1cb95e5b93387bd8362d5af77447105838\` made NEW/NOTIFY/ABORT dispatch explicit and logged UNKNOWN inventory before returning an error. Exact-head rerun is queued.
- AI handoff #45039420870 on SHA \`7c44583\` failed because it lacked the required literal \`Before context exhaustion\`; commit \`6dfdf5c86fbbee4a29538401b9702b74a473ad6a\` restored the rule.
- Rstats #45039333829 failed only at push-filter path validation after earlier policy/test/source assertions passed. The trigger paths are now corrected in both push and pull_request sections.
- XY-C1 #45039333875 on product SHA \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\` remained in Build XY-C1 at last inspection; TRX/artifact not confirmed. L3 is NOT VERIFIED.
- Parent SHA \`e7d0cc1\`: Actions #45039606526, #45039606468, #45039606454, #45039606450, #45039606446, #45039606434, #45039606429 and #45039606416 were queued at last inspection.
- No local checkout or C/Python tests were run. Hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head rstats, process lifecycle, detect-link and AI handoff Actions; fix any newly demonstrated failures.
2. Check XY-C1 Build/TRX/artifact for product SHA \`5be7fe1\`; do not conflate checker-only descendants with firmware product-source evidence.
3. Continue an independent userland stability/security slice. Keep the backup immutable and work directly on \`main\`.
