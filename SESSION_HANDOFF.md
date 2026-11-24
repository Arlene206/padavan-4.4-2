## Current repository state

- Parent main SHA: be691b89477fefaa2e0fc42606b9bf8ba4650528. Re-fetch live main before every write; concurrent sessions may advance it.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged. Core AliDDNS CA-bootstrap/HMAC changes are on main with later evolution; do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA ref update. Never force-push or create development branches.

## Active task

### DHCP ARP fail-closed and declaration-aware regression contracts

- Commit be691b89477fefaa2e0fc42606b9bf8ba4650528 fixed DHCP ACK safety: arpping() used to treat socket-option, send, poll and read errors as no conflict, allowing an unverified address to be bound. It now returns -1 on probe I/O failure, and dhcpc rejects that ACK path while preserving the current lease/state and retry deadline. A GNU89 policy regression covers the tri-state.
- The exact be691 udhcpc run #45464184832 passed its GNU89 ASan/UBSan policy binary but failed a Python assertion because the new test is invoked as test_name() != 0, not as an isolated test_name(); call. This correction aligns the assertion.
- The exact 8e3c3ba... libdisk run #45463559390 passed both host sanitizer binaries but the source checker extracted a forward declaration for disk_share_account_metadata_status_at() instead of its actual definition. The generic extractor now skips prototypes and has a synthetic regression fixture.

## Verification boundary

- be691 AI handoff run #45464184981 PASS.
- be691 exact-head XY-C1 run #45464184736 was IN_PROGRESS at last poll; verify Build, TRX and artifact upload separately. This includes the C89 prototypes added in 8e3c3ba... for two MIPS implicit-declaration failures.
- 8e3c3 XY-C1 #45463559354 was also IN_PROGRESS at last poll. The earlier 572ec XY-C1 #45461770986 failed MIPS build on the two missing prototypes; TRX/artifact skipped. L3 remains NOT VERIFIED; physical L4 NOT VERIFIED.
- The new declaration-aware checker correction requires fresh exact-head libdisk and udhcpc sanitizer/source Actions; no local MIPS compile is claimed.

## Exact first action for the next GPT

1. Inspect fresh exact-head libdisk and udhcpc runs. Confirm both GNU89 ASan/UBSan binaries and all source-contract assertions pass.
2. Inspect XY-C1 Build, TRX verification and artifact upload for the source SHA including the C89 declaration fix and DHCP ARP safety fix.
3. Continue removable-storage P1 mount/path/symlink/stale-inode audit, then cross-userland stability/security fixes; PENDING checks do not block independent work.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, regression outcomes, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test, commit and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA ref updates. Never force-push or overstate firmware or hardware validation. Complete this checkpoint Before context exhaustion.
