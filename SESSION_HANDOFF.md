## Current repository state

- Parent main SHA: a6cbec6ca88598ce1a85daabad9b7febd32e860c. Re-fetch live main before every write because concurrent sessions may advance it.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC changes are integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### Removable-share ACL mount identity

- Earlier production commits 552699f... (add_folder pinned-FD create/rollback), f22e429... (full ACL rebuild and list destructor pinned), 572ec... (rollback ACL writer FD API), and 8e3c... (C89 helper prototypes) hardened folder/metadata operations.
- set_permission() had a remaining mountpoint race: folder validation/cache refresh, ACL read, default file creation and atomic replacement independently reopened mount_path. This source change pins the volume root once and routes no-follow folder validation, enumeration/snapshot, ACL read/default creation and atomic replacement through that descriptor.
- Added a host race regression and source-order contracts. DHCP ARP fail-closed work remains in be691.../57db... with integrated USB validation updates in f148....

## Verification boundary

- Parent 1a62da7d321c4860c0531a289993a54192d67131 libdisk #45466418305 PASS and AI handoff #45466418446 PASS; these are pre-change checks only.
- Exact a6cb libdisk #45468099683 PASS (both GNU89 ASan/UBSan binaries and all Python source contracts); AI handoff #45468099898 PASS. THIS COMMIT updates mod_folder() production logic and adds mountpoint-swap/ACL-rollback host regressions; exact-head verification pending.
- f148 USB hotplug #45465557087 PASS, udhcpc #45465557325 PASS, NVRAM #45465557210 PASS, Process inventory #45465557323 PASS, Process regression #45465557058 PASS and AI handoff #45465557053 PASS. XY-C1 #45465557176 had been PENDING at last check; inspect newest result.
- No same-SHA XY-C1 Build/TRX/artifact all-PASS is confirmed here. L3 NOT VERIFIED; physical-router L4 NOT VERIFIED. No local MIPS build claimed.
- Backup branch remains immutable.

## Exact first action for the next GPT

1. Inspect this commit's exact-head libdisk GNU89 ASan/UBSan binaries and source checker; require the new mountpoint-swap and ACL-write-rollback tests to pass.
2. Inspect dc094 XY-C1 #45467758980 Build, TRX verification and artifact upload individually on that production SHA; only all three PASS support L3.
3. Pin del_folder() staging, ACL preparation/publication, snapshot and compensation to one directory descriptor; add mount replacement and failed-cleanup regression tests.
4. Continue independent cross-userland stability/security fixes while PENDING Actions run.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, test outcomes, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Continue until context exhaustion. Use source fix → tests → one atomic main commit → exact-SHA Actions. Keep one atomic main commit per logical task and non-forced expected-SHA ref updates. Never force-push or overstate firmware/hardware validation. Complete this checkpoint Before context exhaustion.
