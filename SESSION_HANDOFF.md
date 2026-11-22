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
- Last source/test baseline: `e615c1538ce45af6969a27fd24de50f25565ab7c` (guards Internet probe `FD_SET` against descriptors at or above `FD_SETSIZE`).
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio checked-read fixes remain in main; do not repeat those completed slices without new evidence.

## Active task

`detect_link.c::dl_reset_state()` used unchecked `get_ap_mode()`. A transient NVRAM read failure could collapse to the legacy zero fallback and make an AP-mode router appear to be in router mode while the link monitor decided whether to perform WAN checks. The startup and SIGUSR1 reset paths now use `get_ap_mode_status()` and only update the cached mode after a successful read. Initial read failure removes the PID marker and aborts monitor startup; a later refresh failure preserves the last-known-good topology and continues without resetting the other cached counters.

The existing WAN NVRAM policy header/test now covers successful mode updates, read-error preservation and invalid output pointers. The detect-link workflow asserts checked read, startup fail-closed cleanup and last-known-good runtime behavior.

## Verification boundary

- Exact source SHA `913c512d2b18079113c521508bb446a7d534100c`: detect-internet signal lifecycle regression #45037029639 PASS; AI handoff contract #45037029575 PASS. NVRAM audit #45037029633, Process inventory #45037029588 and Process lifecycle inventory #45037029643 later completed successfully; XY-C1 #45037029719 was still building at last check.
- Exact source SHA `e615c1538ce45af6969a27fd24de50f25565ab7c`: detect-internet #45037219732, XY-C1 #45037219745, AI handoff #45037219756, NVRAM #45037219754, Process inventory #45037219761 and Process lifecycle inventory #45037219775 were in progress at last check. This FD_SET slice has not yet been verified.
- No local checkout or local C/Python execution was available in the connector-only session. Current checked AP-mode work requires exact-head detect-link, process and XY-C1 verification. L3 needs Build, TRX and artifact upload PASS on one SHA; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head detect-link workflow and its GNU89/ASan/UBSan regression for checked AP-mode refresh; fix any demonstrated contract/compiler failure.
2. Inspect the detect-internet FD_SETSIZE workflow and XY-C1 result for SHA `e615c1538ce45af6969a27fd24de50f25565ab7c`; do not conflate that older run with the current commit's L3 status.
3. Continue repository-wide stability/security implementation in a different subsystem after these checks. Keep backup immutable and work only on `main`.
