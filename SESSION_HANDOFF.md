## Current repository state

# Cross-session execution mandate

Continue this repository as one cross-session development stream. A logical source/test change is one atomic main commit with a complete Git tree, followed by an exact-SHA Actions check; never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow is not a reason to stop independent work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify main and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record any state transitions from PENDING to PASS/FAIL, and name the next concrete implementation task. Before context exhaustion, complete the current safe slice and update this handoff accurately. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not actually performed.


- Parent main SHA: 5ec726d5123bbfde442f15b1afad114e64747511. This checkpoint records completed exact-SHA validation for the descriptor-pinned libdisk deletion slice; fetch live main for its exact SHA before every write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC changes are integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### Cross-userland continuation — WAN DHCP client transitions

- The libdisk removable-share ACL mount identity slice is implemented in production (del_folder() now pins one volume and staging descriptor set) and its GNU89 ASan/UBSan/source-contract tests passed on exact source SHA 80f1bfccd255536a0a2682c7d046f0216038a922.
- WAN DHCP client NAK/leasefail contracts are now explicit: REQUESTING skips deconfig; active-lease NAK must deconfigure and defer INIT_SELECTING on failure; wrong-server NAKs remain filtered; leasefail callback status stays advisory. Added GNU89 coverage for the REQUESTING/active lease gate and source assertions for NAK and leasefail ordering. Next, continue broader exact transition integration coverage for failed deconfig, lease expiry and SIGUSR2/-R while DECONFIG_PENDING.
- Current main is 5ec726d5123bbfde442f15b1afad114e64747511; the frozen backup is 72f4f157a084f8184b9597ca5bf334b8080e5807 and remains untouched. No PR/branch work.
## Verification boundary

- L3 VERIFIED for production source SHA 80f1bfccd255536a0a2682c7d046f0216038a922 (XY-C1 Build + TRX + artifact upload all PASS; libdisk, Process, HTTPD, NVRAM exact-source checks PASS). Physical-router L4 NOT VERIFIED.
- Docs-only main commits 5ec726d5123bbfde442f15b1afad114e64747511 and c203d84ebce9a3d38947731857098aa62c876a4f each passed the AI handoff validator; product-source tree was unchanged by those commits.
- THIS COMMIT changes the DHCP client source/header/test/source checker. Run exact-SHA udhcpc-script-status-stability and XY-C1 firmware Build/TRX/artifact before advancing product L3 beyond the previous source SHA.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Exact first action for the next GPT

1. Inspect exact-SHA udhcpc-script-status-stability on THIS COMMIT. Require GNU89 -Werror ASan/UBSan and the source checker to pass.
2. Continue deeper DHCP transition tests for NAK during REQUESTING vs active lease, leasefail, failed deconfig/expiry, and SIGUSR2/-R while DECONFIG_PENDING. Preserve BusyBox's established wrong-server NAK filter unless stronger evidence demonstrates a defect.
3. Check the matching XY-C1 Build/TRX/artifact workflow for THIS source SHA; PENDING means continue another independent userland stability task. Preserve frozen backup and update handoff only when state/next task changes.
