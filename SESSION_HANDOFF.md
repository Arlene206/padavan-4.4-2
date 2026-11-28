## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- Current parent for the checker repair: `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce`. The preceding DHCPACK source change uses the length-reporting option parser and has passing GNU89/Werror/ASan/UBSan host tests; its exact-head Python checker had one stale assertion still looking for the removed `udhcp_get_option32()` call.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store feature is integrated.

## Active task

- Correct the DHCPACK lease-time source contract to require the actual validation sequence: parse option 51 with `udhcp_get_option_with_length()`; accept absent/zero-length or present/four-byte only; malformed presence must retry before default/decode; preserve the one-hour fallback only for a truly absent option; decode present uint32 with unaligned-safe read and network byte order.
- This is a checker-only correction. The failed source workflow already compiled and passed the host regression; its Python assertion expected the intentionally superseded `udhcp_get_option32()` code and raised `ValueError`. Do not revert the production fix.

## Exact-SHA CI checkpoint

- Parent SHA `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce`:
  - BusyBox udhcpc event-script workflow #46152329755 FAIL only in its source-contract step at the stale lease-time reader assertion; GNU89/Werror/ASan/UBSan host regression PASS.
  - WAN DHCP callback #46152329744 PASS; DHCPv6 Reply retry #46152329810 PASS.
  - NVRAM transaction boundary #46152329844 PASS; Process inventory #46152329739 PASS; Process fail-closed regression #46152329718 PASS; AI handoff contract #46152329727 PASS.
  - USB storage hotplug stability #46152329716 FAIL; this is a separate failure and must be diagnosed from its exact job log, not conflated with udhcpc.
  - XY-C1 firmware workflow #46152329717 remains IN_PROGRESS on this exact SHA; Build/TRX/artifact have not yet all completed.
- Previous Shadowsocks slice SHA `942f53201f14dcf7130cdae3c356fc87d53d1f2a`: Shadowsocks regression #46152116157 PASS, Process inventory #46152116314 PASS, AI handoff #46152116150 PASS; XY-C1 #46152116153 was IN_PROGRESS at last check.
- Earlier DHCP timeout normalization SHA `1e30efeb62d96cd2fe64d1fa41e03d97d34cde81`: udhcpc #46151496786, AI docs #46151496809, WAN DHCP #46151496793, storage #46151496821, DHCPv6 #46151496828, NVRAM #46151496810, Process #46151496829 and WAN monitor #46151496897 all PASS; XY-C1 #46151496815 status must be rechecked.
- Networkmap ProductID source SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d` has exact Build/TRX/artifact PASS (XY-C1 #46149948587; artifact #15265976441). This is L3 only for that SHA, not newer DHCP changes. Physical-router L4 remains NOT VERIFIED.

## Verification boundary

- L1: DHCP lease option policy host regression compiled/run under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan: PASS on parent SHA.
- L2: exact-head udhcpc checker failed due to stale assertion; this commit repairs its expected source sequence. Fresh exact-head workflow must pass both the C regression and Python contract.
- L3: XY-C1 on the active production SHA `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce` remains PENDING; require Build, TRX verification and artifact upload on one exact SHA.
- L4: physical-router runtime/network behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Actions for the checker repair and require udhcpc, AI handoff and XY-C1 Build/TRX/artifact statuses to be recorded against their exact SHAs.
2. Diagnose USB storage hotplug workflow #46152329716 from its failed step/log and fix the actual cause in a separate atomic commit; do not bundle unrelated regressions.
3. Continue independent stability/security work while firmware builds are pending. Preserve the DHCPACK malformed-length behavior and the earlier zero-timeout discovery throttle.
4. Keep the frozen backup unchanged; use complete-tree atomic commits and a non-forced expected-SHA update.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
