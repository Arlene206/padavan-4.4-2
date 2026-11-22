# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; branch: \`main\`.
- Parent source SHA for EAPoL credential hardening: \`29131f7707e151ef62b1f9ce9611901fd277ce66\`.
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio checked-read fixes remain in main; do not repeat those completed slices without new evidence.

## Active task

\`auth.c::start_auth_eapol()\` now snapshots WAN EAP credentials through checked caller-owned buffers, writes an escaped/validated configuration into a same-directory temporary file with mode 0600, checks flush/fsync/close, and atomically renames it after stopping old daemons. Failure to start the wpa_cli action handler is returned and triggers best-effort cleanup of the incomplete EAP stack.

The first EAPoL workflow source-contract draft had a predicate mismatch: test functions are invoked through \`CHECK(test_name() == 0)\`, while the contract expected a standalone \`test_name();\` call. Production code and the compiled policy test are not implicated. The contract now matches the test harness invocation.

## Verification boundary

- Parent source SHA \`29131f7707e151ef62b1f9ce9611901fd277ce66\`: EAPoL workflow run #45038262266 was in progress at its checkout step when inspected; the first draft contract assertion was known to be too strict and is corrected in this follow-up. The exact-head run must still be inspected.
- Parent XY-C1 run #45038262295 was in progress at source checkout; no Build/TRX/artifact completion is claimed.
- No local checkout or local C/Python execution was performed in this connector-only session. EAPoL GNU89/ASan/UBSan policy run and source contract must pass on the new exact SHA.
- L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA. Router hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head EAPoL workflow for both GNU89/ASan/UBSan execution and source-contract PASS, and inspect XY-C1 Build/TRX/artifact on the same SHA.
2. Continue repository-wide stability/security implementation in another subsystem; do not repeat completed WAN/PPP, USB modem, Inadyn, DNSMasq or manual-radio slices.
3. Keep the backup immutable and continue all development directly on \`main\`.
