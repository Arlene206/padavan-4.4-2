# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`.
- Last source/test baseline: `913c512d2b18079113c521508bb446a7d534100c` (checked temporary NVRAM write for Internet monitor resume delay).
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio checked-read fixes remain in main; do not repeat those completed slices without new evidence.

## Active task

The Internet connectivity monitor's `di_connect_to_host()` obtains a socket descriptor and then passes it to `FD_SET()` for `select()` without checking `FD_SETSIZE`. Socket creation may return a valid descriptor larger than the fixed `fd_set` capacity when the process descriptor table is highly populated; `FD_SET` can then write beyond the stack-allocated descriptor set.

This milestone adds a small GNU89-compatible policy guard, validates the descriptor immediately after socket creation, closes it and returns `EOVERFLOW` before any `FD_SET` access when out of range. Regression cases cover the first and last valid descriptors, the exact upper bound, above-bound values, and invalid arguments. The detect-internet workflow checks the guard/close/error ordering before `FD_SET`.

## Verification boundary

- Exact source SHA `913c512d2b18079113c521508bb446a7d534100c`: detect-internet signal lifecycle regression #45037029639 PASS; AI handoff contract #45037029575 PASS. NVRAM audit #45037029633, Process inventory #45037029588 and Process lifecycle inventory #45037029643 were still in progress at last check.
- XY-C1 run #45037029719 is a push run on that exact SHA; dependency installation was in progress and Build/TRX/artifact were not yet complete. No L3 result is claimed for `913c512`.
- No local checkout or local C/Python test execution was available in the connector-only session. This FD_SET change requires exact-head detect-internet and XY-C1 verification. L4 hardware remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head detect-internet workflow for the FD_SETSIZE regression and source-order contract, then inspect XY-C1 Build, TRX verification and artifact upload on the same SHA. Fix any demonstrated failure.
2. Continue repository-wide stability/security implementation outside the completed WAN/PPP, USB modem, Inadyn, DNSMasq and manual-radio work; rotate to another module after this network-monitor safety slice.
3. Keep the backup immutable and continue all development directly on `main`.
