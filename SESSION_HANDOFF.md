## Current repository state

- Active branch: `main`. Source parent: `9804865fa5c34b5e7e00b57ce3949dca89ab5893`; the next atomic commit fixes the RAETH lock-balance defect, strengthens its Switch CLI source contract, and records this handoff.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is read-only. Legacy PR #1 is closed and unmerged.
- Development uses complete Git trees and one commit per logical slice, then non-forced expected-SHA update of `main`. No force-pushes, work branches or PR-based development.

## Active task

### RAETH `RAETH_SET_LAN_IP` page-lock balance

- In `ei_ioctl()`, this case drops `page_lock` around `copy_from_user()`. The bounded NUL-termination check previously ran after reacquiring the lock. Its invalid-input `break` therefore bypassed a balanced locked state, but the common function epilogue still unconditionally called `spin_unlock_irq()`. Empty or unterminated input could trigger an unbalanced spin unlock / IRQ restore.
- The fix validates the local copied buffer while `page_lock` remains released, then unconditionally reacquires `page_lock` before checking the return status. Both success and rejection now reach the common epilogue with the expected lock held.
- Existing GNU89 regression checks valid, empty, unterminated, NULL and zero-length input. The Switch CLI inline Python contract now enforces unlock → copy → validate → reacquire → error branch → copy state ordering.

## Verification boundary

- Parent SHA `9804865fa5c34b5e7e00b57ce3949dca89ab5893`: DNSMasq lifecycle #45421011367 PASS; Process inventory #45421011328 PASS; USB storage #45421011314 PASS; XY-C1 #45421011312 was IN_PROGRESS at last poll.
- Parent AI handoff #45421011316 FAILED because `check_ai_handoff_docs.py` requires the exact literal `The checkpoint is mandatory; a document edit is conditional.`. This handoff includes that required sentence. New exact-head AI validation is required.
- The SmartDNS procfs fallback in parent `9804865` is covered by exact-head DNSMasq and Process PASS. Do not reopen that issue absent new failure.
- Host policy regression was previously run with GNU89, `-Wall -Wextra -Werror`, ASan/UBSan and passed. The new source-order contract requires fresh exact-head Switch CLI validation. L3 requires Build, TRX verification and artifact upload to PASS on the same source SHA; hardware L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live `main` and check exact-head Switch CLI, AI handoff, Process inventory, NVRAM, DNSMasq and XY-C1 Actions for the lock-balance commit. Diagnose any failure; ignore stale parent results.
2. Confirm XY-C1 Build, TRX verification and artifact upload all PASS on one exact source SHA before claiming L3. Hardware L4 is NOT VERIFIED.
3. Continue the next independent stability/security defect outside RAETH after this bounded correction is verified; do not repeat completed MTK ESW authorization, AliDDNS persistence, VSFTPD ACL, MiniUPnP or SmartDNS procfs checks.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect the live `main` ref, exact-head relevant push Actions, validation boundary, blockers and the next precise task. Update this file only when those resumable facts materially change; if they remain accurate, do not create a no-op commit.

# Cross-session execution mandate

Recover once, then implement. Keep work source-led: one bounded source fix, meaningful regression, necessary validation, one complete Git tree and one atomic main commit, followed by a non-forced compare-and-swap. Pending CI does not block independent work. Before context exhaustion, finish the safe slice and leave the next GPT one precise first action. Never overstate host, firmware or hardware evidence.
