## Current repository state

- Active parent main SHA: 57db1f707f65c8f753d964ebf69d9cacfb26ec28. Re-fetch live main before every write because concurrent sessions may advance it.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 is closed/unmerged; core AliDDNS CA-bootstrap/HMAC work is on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### DHCP ARP preflight fail-closed and libdisk descriptor hardening

- Commit be691b89477fefaa2e0fc42606b9bf8ba4650528 made arpping() return -1 on socket-option/send/poll/read errors, 0 for conflict and 1 only after a complete no-conflict probe. DHCPACK rejects a negative probe result and preserves current lease/state/retry deadline.
- Commit 57db1f707f65c8f753d964ebf69d9cacfb26ec28 removed the nested legacy arpping() call still present under the new status==0 branch, closing dhcpc.c correctly and ensuring one already-completed probe drives the conflict path.
- Commit eb75f63ecd7ac2779fdbf53b989c94531ff3cd13 made the libdisk source-contract extractor declaration-aware and corrected the udhcpc test invocation assertion. Its udhcpc run #45464697251 PASS. Its libdisk #45464697321 passed host sanitizer binaries but failed because a staging-helper matcher included literal backslash+n; this commit fixes the matcher and makes the fixture use actual newline escapes.
- Earlier production fixes: 552699f8003dbd1e48c2c109e526a77b4b15da8d pinned add_folder operations to mount fd; f22e42974a86bb499cdc30f0b4537bcfaaa0626d restored the truncated list destructor and pinned full ACL rebuild; 8e3c3ba6702c2fe7a1a28ca3016a22611aad0b7a added C89 prototypes for descriptor helpers after MIPS compile errors.

## Verification boundary

- be691 udhcpc #45464184832: GNU89 ASan/UBSan binary PASS; Python checker failed the test invocation assertion. eb75 udhcpc #45464697251 PASS.
- be691 USB hotplug #45464184906 FAILED compiling BusyBox because of an unmatched block from the nested old ARP probe. Fixed in 57db1; exact 57db USB hotplug #45465005107 was IN_PROGRESS at last poll.
- 57db1 XY-C1 #45465005302, udhcpc #45465005678, USB hotplug #45465005107, Process #45465005138 and NVRAM #45465005570 were IN_PROGRESS at last poll. be691 AI handoff #45464184981, Process #45464184974, process regression #45464184684 and NVRAM #45464184541 PASS.
- 8e3c3 XY-C1 #45463559354 was IN_PROGRESS at last poll. 8e3c3 libdisk #45463559390 passed host binaries but failed checker; eb75 libdisk #45464697321 passed host binaries but failed the escaped signature now fixed. Fresh exact-head checks required.
- Earlier 572ec XY-C1 #45461770986 failed MIPS on missing prototypes; added in 8e3c3. No same-SHA Build/TRX/artifact PASS confirmed; L3 NOT VERIFIED; physical L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect fresh exact-head libdisk run after the staging-helper signature correction; confirm both GNU89 ASan/UBSan binaries and the full Python contract pass.
2. Inspect fresh exact-head udhcpc and USB hotplug runs after the single-ARP-probe brace fix; confirm BusyBox compiles and sanitizer/source checks pass.
3. Inspect 57db XY-C1 MIPS Build, TRX and artifact upload individually; only all three PASS on the same production SHA can claim L3.
4. Continue P1 removable-storage mount/path/symlink/stale-inode audit and cross-userland stability/security fixes while PENDING checks continue.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, regressions, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test, commit and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA ref updates. Never force-push or overstate firmware/hardware validation. Complete this checkpoint Before context exhaustion.
