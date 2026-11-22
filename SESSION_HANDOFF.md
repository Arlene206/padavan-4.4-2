# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; parent main SHA: \`9263cbc5583a57dd435eacd2f72a5858351d2537\`.
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- EAPoL configuration hardening and status-bearing detect-link notification are in main. Rstats periodic MTD save now remains due and retries after a reported failure.

## Active task

Process lifecycle run #45039606446 on SHA \`e7d0cc1cb95e5b93387bd8362d5af77447105838\` reached the EAPoL stop integration assertion and failed because \`auth.c::stop_auth_eapol()\` had an opening brace but its original body had been lost while inserting the secure config writer. This was a real source defect, not a stale test: the helper definition was placed inside the unclosed function. The current source restores the intended \`wpa_cli\`/\`wpa_supplicant\` stop list and \`kill_services(...)\` return before the secure config writer.

## Verification boundary

- Exact SHA \`9263cbc5583a57dd435eacd2f72a5858351d2537\`: rstats stability #45039712265 PASS, including GNU89/ASan/UBSan policy tests and source/path contracts.
- AI handoff #45039712273 failed because the current handoff did not contain the checker-required exact sentence \`The checkpoint is mandatory; a document edit is conditional.\`; this commit restores it and preserves the required \`Mandatory handoff checkpoint\` heading and live repository inspection requirement.
- Process lifecycle #45039606446 on \`e7d0cc1\` failed at \`check_process_lifecycle_integration.py:1411\`: the source lacked \`return kill_services(svcs, 3, 1);\` inside \`stop_auth_eapol\`. The restored function body addresses that exact failure; exact-head rerun is pending.
- XY-C1 #45039606450 on product-source SHA \`e7d0cc1\` was still in Build XY-C1 at last check; TRX verification and artifact upload were not confirmed. No L3 result is claimed.
- No local checkout or C/Python tests were run. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect this commit's AI handoff and process-lifecycle Actions; diagnose any additional source or contract failure.
2. Inspect XY-C1 Build/TRX/artifact on product SHA \`e7d0cc1\` and run the firmware validation workflow on the latest source when appropriate.
3. Continue a separate userland stability/security slice. Keep backup immutable and work directly on \`main\`.
