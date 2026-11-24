## Current repository state

- Active parent main SHA: b0d3012e1144f7edd3c6f16658da4d047c5ba8ae. Re-fetch live main before any write because concurrent sessions may advance it.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC work is on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### DHCP ARP preflight and removable-storage descriptor hardening

- be691b89477fefaa2e0fc42606b9bf8ba4650528 made arpping() return -1 for socket-option/send/poll/read errors, 0 for conflict and 1 only after a complete no-conflict probe. DHCPACK rejects negative results and preserves the current lease/state and retry deadline.
- 57db1f707f65c8f753d964ebf69d9cacfb26ec28 removed the accidental nested old arpping() call so the new tri-state probe is used exactly once.
- f148b0881302f62da93e5e81edb53a31bd17f998 updated the USB storage integration assertion to require two udhcpc_script_failure_prepare_retry call sites (event-script failure and ARP-probe failure) and moved resume_state before statements for C89.
- fe2e86b3a6f3d36537bc3c6f9c6b64f04e37c86d corrected the libdisk checker matcher for disk_share_remove_folder_path() after making its extractor declaration-aware. Its libdisk run #45465306855 passed both GNU89 ASan/UBSan host binaries but then exposed a separate stale contract: it searched the rename-only policy body for mkdirat/unlinkat, which reside in create/remove-entry helpers. This commit aligns those checks with the correct helper definitions.
- b0d3012e1144f7edd3c6f16658da4d047c5ba8ae corrected the staging helper assertions. Its exact libdisk run #45465855322 passed both GNU89 ASan/UBSan binaries but exposed one remaining stale contract: the ACL updater now accepts a caller-pinned `directory_fd`, while the checker still expected the removed mount-path formatter/reader/writer. THIS COMMIT aligns the checker to bounded leaf construction, no-follow read, permission rewrite and atomic replacement through that same descriptor.
- Earlier production: 552699f... pinned add_folder to mount fd; f22e429... restored the truncated destructor and pinned full metadata rebuild; 8e3c3ba... added static C89 prototypes after MIPS build errors.

## Verification boundary

- 57db udhcpc #45465005678 PASS (GNU89 ASan/UBSan and Python checker).
- Exact f148 udhcpc #45465557325, USB storage #45465557087, NVRAM #45465557210, Process inventory #45465557323, Process regression #45465557058 and AI handoff #45465557053 all PASS. XY-C1 #45465557176 remains IN_PROGRESS; verify Build/TRX/artifact separately.
- 57db USB hotplug #45465005107 compiled BusyBox, then failed the obsolete one-call integration assertion (workflow corrected in f148).
- fe2e86 libdisk #45465306855 passed both sanitizer binaries, then failed the mis-targeted staging syscall assertion.
- b0d3012 AI handoff #45465855318 PASS; exact-head libdisk #45465855322 passed both sanitizer binaries and failed the remaining stale ACL-updater API assertion. THIS COMMIT corrects that assertion; exact-head libdisk rerun required.
- f148 USB hotplug #45465557087, XY-C1 #45465557176, udhcpc #45465557325, NVRAM #45465557210, Process #45465557323, Process regression #45465557058 and AI handoff #45465557053 were QUEUED at last poll.
- 57db XY-C1 #45465005302 was IN_PROGRESS; verify Build/TRX/artifact steps. No same-SHA L3 Build/TRX/artifact all-PASS verified yet; L3 NOT VERIFIED; L4 NOT VERIFIED.
- Earlier 572ec XY-C1 failed due missing libdisk prototypes, added in 8e3c3ba...; subsequent cross-build remains to be verified. No local MIPS build claimed.

## Exact first action for the next GPT

1. Inspect the fresh exact-head libdisk run after the ACL updater contract was aligned with the pinned-directory API; require both GNU89 ASan/UBSan binaries and every Python assertion to pass.
2. Inspect f148 XY-C1 #45465557176 Build, TRX verification and artifact upload separately; confirm all three complete on the same SHA.
3. Continue the P1 removable-storage metadata/path/symlink/stale-inode audit from the pinned-directory design, implementing a concrete independent defect with a host regression. Pending CI does not block that work.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, tests, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test, commit and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA ref updates. Never force-push or overstate firmware/hardware validation. Complete this checkpoint Before context exhaustion.
