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
- Parent source SHA for this EAPoL security slice: \`aeff51b643212ff3a95fe1b308b294cc2d6c8fc1\` (status-checked detect-link AP/router mode refresh).
- Frozen backup: \`backup/pre-plugin-overhaul-2026-11-14\` at \`72f4f157a084f8184b9597ca5bf334b8080e5807\`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio checked-read fixes remain in main; do not repeat those completed slices without new evidence.

## Active task

\`auth.c::start_auth_eapol()\` generated \`/etc/wpa_supplicant.conf\` by writing raw NVRAM identity/password values inside quoted configuration strings using \`fopen("w")\`. This did not reject control characters that could escape the quoted value, did not guarantee private permissions for a file containing a plaintext EAP password, and ignored the \`wpa_cli\` event-handler launch result.

The current milestone introduces a quoted-value writer that escapes backslashes/quotes and rejects control bytes; snapshots WAN identity/password using status-returning caller-owned buffers before stopping the current daemons; writes a same-directory temporary config with mode 0600, checks flush/fsync/close, then atomically renames it after the previous processes stop. If \`wpa_cli\` cannot start, it stops the incomplete EAP stack and returns the error.

## Verification boundary

- Parent source SHA \`aeff51b643212ff3a95fe1b308b294cc2d6c8fc1\`: detect_link #45037505035, AI documentation #45037505063, NVRAM #45037505044, process inventory #45037505053, process lifecycle #45037505043, HTTPD #45037505013 and USB modem #45037504998 completed SUCCESS.
- Parent XY-C1 #45037505021 was in progress: all host/policy steps passed; \`Build XY-C1\` still running, TRX verification and artifact upload pending. Do not claim L3.
- No local checkout or local C/Python test execution was available in this connector-only session. The EAPoL GNU89/ASan/UBSan test and exact-SHA XY-C1 build remain pending after publication.
- L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA. Router hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check the exact-SHA EAPoL credential/config regression and XY-C1 Build/TRX/artifact result for the current main commit; diagnose any failure without waiting on unrelated pending runs.
2. Continue the repository-wide stability/security rotation in another subsystem; do not re-audit WAN/PPP, USB modem, Inadyn, DNSMasq or manual-radio fixes already completed.
3. Keep the backup immutable and continue all development directly on \`main\`.
