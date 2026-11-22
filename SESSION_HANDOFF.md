# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a fetched expected SHA.
- Check exact-SHA Actions. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; branch: \`main\`.
- Parent source SHA: \`972fbea175b0fbbc4c2959178e8f25e2fd7a9446\` (EAPoL configuration security and lifecycle source-contract compatibility).
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- EAPoL hardening is in main: checked credential snapshots, quoted-value escaping/control-byte rejection, mode-0600 temp file, fsync/close checks, atomic publication, and wpa_cli launcher failure cleanup.

## Active task

\`detect_link.c::notify_reset_detect_link()\` previously returned void and discarded both \`start_detect_link()\` and \`process_pidfile_signal()\` results. A missing/unstartable monitor or failed/stale PID identity signal was therefore silently ignored. The function now returns an error status normalized from launcher status or errno, logs failures, and exposes an int declaration in \`rc.h\`. A GNU89 policy regression covers success despite stale errno, preserving positive launcher status, ESRCH propagation, and errno-less failure to EIO. The detect-link workflow now compiles the policy test and verifies result normalization/return.

## Verification boundary

- Exact parent SHA \`972fbea175b0fbbc4c2959178e8f25e2fd7a9446\`: AI handoff #45038528658, EAPoL config #45038528661, process inventory #45038528662, process lifecycle inventory #45038528655, XY-C1 #45038528646 and NVRAM audit #45038528641 were queued at last inspection.
- Older exact SHA \`3af9facfab4e28109916b47ed9bd671f983c4e1b\`: EAPoL regression #45038345083 PASS and AI handoff #45038345180 PASS. Process lifecycle #45038262447 failed at stale EAPoL source assumptions; follow-up #972 updated the function signature and source contract.
- XY-C1 #45037505021 on ancestor \`aeff51b\` completed success, but is not an L3 validation of newer source commits. The #45038262295 run on \`29131f7\` was still in progress at last check. No same-SHA Build/TRX/artifact pass is claimed.
- No local checkout or local C/Python tests were run. L3 requires Build, TRX verification and artifact upload PASS on one identical SHA; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head detect-link notification policy/source contract, process-lifecycle integration, EAPoL regression and XY-C1 Build/TRX/artifact runs. Fix any demonstrated regression.
2. Continue repository-wide stability/security implementation in a different subsystem; avoid repeating the completed WAN/PPP, USB modem, Inadyn, DNSMasq, Wi-Fi and EAPoL slices without new evidence.
3. Keep backup immutable and work directly on \`main\`.
