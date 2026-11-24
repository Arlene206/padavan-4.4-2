## Current repository state

- Active parent main SHA: fe2e86b3a6f3d36537bc3c6f9c6b64f04e37c86d. Re-fetch live main before every write because concurrent sessions may advance it.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 is closed/unmerged; core AliDDNS CA-bootstrap/HMAC work is integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### DHCP ARP preflight and integrated BusyBox verification

- be691b89477fefaa2e0fc42606b9bf8ba4650528 made arpping() return -1 on socket-option/send/poll/read errors, 0 for a conflict and 1 only after a completed no-conflict probe. DHCPACK rejects negative results without committing the lease, preserves current state/deadline and retries.
- 57db1f707f65c8f753d964ebf69d9cacfb26ec28 removed the accidentally nested legacy arpping call so the tri-state result is the sole probe and the DHCPACK branch braces balance.
- USB hotplug run #45465005107 on 57db successfully compiled BusyBox after that brace correction but failed its embedded Python source check because it still expected a single call to udhcpc_script_failure_prepare_retry(). The new ARP error gate adds a second legitimate caller. This commit changes expected count to two and makes the nearby deconfig block's resume_state declaration C89-compliant.
- eb75f63ecd7ac2779fdbf53b989c94531ff3cd13 aligned the udhcpc source checker; exact udhcpc run #45464697251 PASS.
- fe2e86b3a6f3d36537bc3c6f9c6b64f04e37c86d repaired the libdisk checker staging-helper signature after its earlier declaration-aware extractor fix.

## Verification boundary

- 57db udhcpc #45465005678 PASS (GNU89 ASan/UBSan and Python source contract).
- 57db USB hotplug #45465005107 compiled BusyBox, then failed only the integration contract's outdated call count; this commit fixes it. Fresh exact-head USB check required.
- 57db XY-C1 #45465005302 was IN_PROGRESS at last poll. Confirm MIPS Build, TRX verification and artifact upload separately.
- fe2e86 libdisk #45465306855 and AI handoff #45465306892 were QUEUED at last poll. Prior eb75 libdisk #45464697321 passed sanitizer binaries but checker failed on the escaped staging-helper matcher; corrected in fe2e86. Fresh exact-head test required.
- Earlier production parent 572ec XY-C1 #45461770986 failed MIPS due missing prototypes; fixed in 8e3c3ba... No exact-SHA L3 Build/TRX/artifact PASS yet; L3 NOT VERIFIED; L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect fresh exact-head storage-hotplug BusyBox build and caller-graph checker; confirm the two retry-policy call sites and sanitizer/source contracts pass.
2. Inspect XY-C1 Build, TRX verification and artifact upload for the source that includes both the C89 libdisk prototypes and DHCP ARP fail-closed change.
3. Inspect fresh exact-head libdisk run after the declaration-aware extractor and staging-helper signature correction.
4. Continue P1 removable-storage path/symlink/stale-inode audit and broader userland stability/security while PENDING workflows proceed.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, tests, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test, commit and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware/hardware validation. Complete this checkpoint Before context exhaustion.
