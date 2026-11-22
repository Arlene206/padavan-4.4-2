# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record the exact main source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on \`main\`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update \`main\` only with \`force:false\` and a freshly fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; product-source baseline: \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\` (rstats retry-on-failed-periodic-storage-save fix).
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq, manual Wi-Fi radio and EAPoL hardening remain in main; do not repeat without new evidence.

## Active task

Repair the process-lifecycle integration checker after the detect-link API was correctly changed from void to int. Exact-head Process lifecycle regression #45038940342 on product-source SHA \`661924b3b5cb309988758820c285f6bedeab3c23\` passed all preceding GNU89 C/ASan/UBSan tests and integration assertions, then failed at \`scripts/check_process_lifecycle_integration.py:1224\` because the parser still split the old \`void\\nnotify_reset_detect_link(void)\` signature. The checker now parses the int-returning definition and requires the normalized status to be returned. This follow-up changes only the checker and handoff/log records; production source is unchanged.

## Verification boundary

- Exact SHA \`661924b3b5cb309988758820c285f6bedeab3c23\`: detect-link regression #45038940465 PASS; process-lifecycle inventory #45038940527 PASS; AI handoff docs #45038940232 PASS; DNS Forwarder launch path #45038940423 PASS; router-mode NVRAM #45038940605 PASS.
- Process lifecycle regression #45038940342 FAILED only at the stale void-signature split in \`scripts/check_process_lifecycle_integration.py:1224\`. The checker-only correction is included in this commit; its current-main exact-head rerun is pending.
- Parent product-source SHA \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\`: Process lifecycle regression #45039333810, process inventory #45039333858, rstats regression #45039333829, AI handoff docs #45039333912, NVRAM audit #45039333836 and XY-C1 #45039333875 were in progress at the last inspection. Their results do not validate the checker-only descendant.
- L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. Hardware L4 remains NOT VERIFIED.
- No local checkout or local C/Python tests were performed. Exact-SHA GitHub Actions remain authoritative.

## Exact first action for the next GPT

1. Inspect the exact-head Process lifecycle regression triggered by this checker correction; fix only a demonstrated remaining assertion or production defect.
2. Inspect XY-C1 run #45039333875 and confirm whether Build, TRX verification and artifact upload all completed on product-source SHA \`5be7fe1c4dd9ec16c3546d871346ca09d7060541\`. Do not treat this checker-only descendant as new product-source L3 evidence.
3. Continue a repository-wide stability/security slice in a different user-space subsystem. Keep the frozen backup read-only and develop only on \`main\`.
