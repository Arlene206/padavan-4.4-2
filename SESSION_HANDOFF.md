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
- Parent source SHA for the EAPoL lifecycle-contract correction: \`3af9facfab4e28109916b47ed9bd671f983c4e1b\`.
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio checked-read fixes remain in main; do not repeat those completed slices without new evidence.

## Active task

EAPoL hardening is in main: checked credential snapshots, quoted-value escaping/control-byte rejection, a same-directory mode-0600 temporary file, fsync/close checking, atomic publication after old daemons stop, and propagation/cleanup when \`wpa_cli\` fails.

Exact SHA \`3af9facfab4e28109916b47ed9bd671f983c4e1b\` EAPoL regression #45038345083 PASS (GNU89/ASan/UBSan policy test and source contract); AI handoff #45038345180 PASS. The broader process-lifecycle integration then exposed an obsolete source contract: its parser expected \`int start_auth_eapol(\` on one line and specifically required the old insecure \`fopen(wpa_conf, "w")\` after daemon stop. The function definition is restored to the established one-line signature, and the contract now checks checked credential snapshots, private temp-file creation, stop-before-rename, atomic publication and checked wpa_cli launch.

## Verification boundary

- Parent SHA \`3af9facfab4e28109916b47ed9bd671f983c4e1b\`: EAPoL #45038345083 PASS and AI handoff #45038345180 PASS.
- Process inventory/lifecycle regression #45038262447 failed only in its integration contract at auth.c line 1407 parsing; preceding compiled policy tests and earlier contracts passed. The exact failing assertion expected the old signature/body ordering and legacy \`fopen\`. This commit updates that contract; the exact-head rerun is pending.
- XY-C1 build #45038262295 on source SHA \`29131f7707e151ef62b1f9ce9611901fd277ce66\` was still in progress at last check. Do not claim same-SHA L3. Parent XY-C1 #45037505021 on \`aeff51b\` completed success, but it is not evidence for descendant source SHAs.
- No local checkout or local C/Python execution was performed. L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head process-lifecycle regression after the auth parser correction and EAPoL security workflow; diagnose/fix any newly exposed assertion.
2. Inspect XY-C1 Build/TRX/artifact on the same source SHA; do not conflate older successful firmware builds with newer commits.
3. Continue repository-wide stability/security work in another subsystem; keep backup immutable and work only on \`main\`.
