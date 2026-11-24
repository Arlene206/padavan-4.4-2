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
- Keep expanding across Padavan userland; the next concrete slice is the WAN DHCP client (udhcp/dhcpc.c): add bounded regression coverage for leasefail/NAK and end-to-end state transitions, including REQUESTING vs active-lease NAK behavior, failed deconfig retry, lease-expiry retry, and SIGUSR2/-R release while DECONFIG_PENDING. Only change behavior when a reproducible contract defect is demonstrated; retain the existing fail-closed deconfig retry semantics.
- Current main is 5ec726d5123bbfde442f15b1afad114e64747511; the frozen backup is 72f4f157a084f8184b9597ca5bf334b8080e5807 and remains untouched. No PR/branch work.
## Verification boundary

- Exact source SHA 80f1bfccd255536a0a2682c7d046f0216038a922: libdisk share-list #45471885325 SUCCESS (GNU89 ASan/UBSan regression and source contracts); Process inventory #45471885205 SUCCESS; HTTPD auth #45471889467 SUCCESS; NVRAM audit #45471885464 SUCCESS; XY-C1 #45471885211 SUCCESS, including firmware Build, TRX verification and artifact upload. This establishes L3 for the production source tree at 80f1bf; it does not establish hardware L4.
- AI handoff validator #45471885677 FAILED on 80f1 because the mandatory checkpoint wording was absent, then the docs-only correction 5ec726d5123bbfde442f15b1afad114e64747511 passed AI handoff #45472190508. The 5ec726d commit changes only SESSION_HANDOFF.md and dev_log.md; all product source inputs are identical to the 80f1 L3-verified tree.
- Current exact main SHA 5ec726d5123bbfde442f15b1afad114e64747511: the exact-head AI handoff workflow #45472190508 SUCCESS. It was a documentation-only change, so other product workflows were not triggered on 5ec726d; the product-source firmware proof remains the exact 80f1 run above.
- Backup backup/pre-plugin-overhaul-2026-11-14 remains frozen at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- L3 VERIFIED for production source SHA 80f1bfccd255536a0a2682c7d046f0216038a922 (Build + TRX + artifact all PASS); physical-router L4 NOT VERIFIED. Do not imply the docs-only 5ec commit itself triggered a firmware build.

## Exact first action for the next GPT

1. Continue the WAN DHCP client transition slice in trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c. Inspect its exact callers and workflow contracts; add host tests for REQUESTING/active-lease NAK, leasefail after exhausted acquisition, deconfig-pending retry and SIGUSR2/-R. Do not invent new state behavior unless tests expose a concrete defect.
2. Keep host tests GNU89/Werror/ASan/UBSan and use the existing WAN DHCP callback workflow. Commit source/test/checker changes atomically, then inspect exact-SHA Actions; PENDING means work on an independent userland task.
3. Preserve the frozen backup and no-force expected-SHA main update protocol. Update this handoff only when source/test/CI state or the next concrete task changes.
