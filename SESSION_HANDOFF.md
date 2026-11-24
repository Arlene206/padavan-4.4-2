## Current repository state

- Parent main SHA: 53ef9feca9ef9a031fae2782f0f83144be832d18. The prototype/build correction is THIS COMMIT; fetch live main for its exact SHA before every write.
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
- Exact main `53ef9feca9ef9a031fae2782f0f83144be832d18`: libdisk host regression #45468585750 PASS; AI handoff #45468585944, Process inventory #45468585831, HTTPD auth #45468585722 and NVRAM audit #45468585988 PASS. XY-C1 #45468585947 FAILED in the MIPS `libdisk` compile because `set_permission()` used `disk_share_folder_exists_in_directory_fd()` before its later static definition. The missing file-scope prototype and a source-contract guard are fixed in THIS COMMIT; exact-head reruns are required.
- f148 USB hotplug #45465557087 PASS, udhcpc #45465557325 PASS, NVRAM #45465557210 PASS, Process inventory #45465557323 PASS, Process regression #45465557058 PASS and AI handoff #45465557053 PASS. XY-C1 #45465557176 had been PENDING at last check; inspect newest result.
- No same-SHA XY-C1 Build/TRX/artifact all-PASS is confirmed here. L3 NOT VERIFIED; physical-router L4 NOT VERIFIED. No local MIPS build claimed.
- Backup branch remains immutable.

## Exact first action for the next GPT

1. Inspect exact-head libdisk GNU89 ASan/UBSan/source-contract and XY-C1 push Actions for THIS COMMIT. Require Build, TRX verification and artifact upload to all PASS on the same SHA before claiming L3.
2. Then pin `del_folder()` staging, ACL preparation/publication, snapshot and compensation to one directory descriptor; add mount-replacement and failed-cleanup regression tests.
3. Continue independent cross-userland stability/security fixes while unrelated PENDING Actions run.


## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, test outcomes, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Continue until context exhaustion. Use source fix → tests → one atomic main commit → exact-SHA Actions. Keep one atomic main commit per logical task and non-forced expected-SHA ref updates. Never force-push or overstate firmware/hardware validation. Complete this checkpoint Before context exhaustion.
