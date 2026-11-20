# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Parent `main` SHA immediately before this atomic LAN DHCP slice: `9d8a9036cdd350a3a45971d8cd5d766d5ec93762`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; unchanged).
- Previous exact-head push checks on `9d8a9036`: MiniUPnP #44768112680 PASS; Process inventory #44768112861 PASS; NVRAM boundary #44768112614 PASS; AI handoff docs #44768112993 PASS. XY-C1 #44768112587 was IN_PROGRESS at last check; do not claim L3 until its Build/TRX/artifact gates finish.
- The preceding counter-default slice is implemented and its MiniUPnP regression passed on `9d8a9036`; it is separate from this LAN DHCP change.
- This slice changes the post-commit networkmap refresh condition for address-changing LAN DHCP renewals. Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy regression passed; exact-head WAN-DHCP and XY-C1 checks are pending.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Fix the stale networkmap inventory on LAN DHCP address-changing RENEW:

- `udhcpc_lan_bound()` already detects IP/netmask changes and applies/commits the new lease, but it previously restarted networkmap only when `is_renew` was false. A changed RENEW can move the LAN subnet without triggering the scanner refresh.
- The new `lan_dhcp_networkmap_refresh_required(address_changed, is_renew)` policy refreshes on initial BOUND and when the address/netmask changes during RENEW, while keeping unchanged RENEW quiet. The networkmap operation stays ancillary after the lease commit; its failure is logged without undoing a valid lease.
- The source-contract checker and GNU89/ASan/UBSan unit test cover the condition. Root `dev_log.md` has been rotated to its 20-record window; oldest 100 records are archived in part-020.

After verification, continue the LAN DHCP transition audit (leasefail/NAK/release and AP restore paths) or move to the next P1 WAN/service-lifecycle defect. Do not repeat the completed pinhole counter or wildcard fixes.

## Verification boundary

- L2 policy regression: GNU89, `-Wall -Wextra -Werror`, ASan and UBSan PASS locally.
- Static LAN DHCP callback contract is updated in `scripts/check_lan_dhcp_deconfig.py`; exact-head WAN-DHCP Action pending after push.
- XY-C1 workflow watches the modified production `net_lan.c` and policy header. Build/TRX/artifact on the new source SHA must be checked; prior exact-source results do not transfer.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check WAN DHCP callback, XY-C1, NVRAM, Process and AI Actions whose `head_sha` exactly equals the new main SHA. Diagnose real failures; do not wait on pending runs before starting independent work.
3. If the new callback and firmware checks pass, continue the remaining LAN DHCP leasefail/NAK/release and AP restore transition audit, then choose the next P1 WAN/service-lifecycle issue.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
