## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- Current parent for this CI-contract repair: `82160c242b345e6decc00e2b4f8ba2217419ae87`. The DHCPACK lease-time production change is intact. The GNU89/Werror/ASan/UBSan host regression passed on both the parent and the immediately preceding checker commit.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store feature is integrated.

## Active task

- Align the two remaining CI source-contract guards with the current DHCPACK implementation. The udhcpc checker must normalize whitespace before asserting the lease-time policy expression; the storage hotplug workflow counts production calls to the shared retry helper and must include the additional malformed-lease-time retry call.
- These are test-contract-only corrections. Do not alter the DHCPACK parser or retry behavior: only absent option 51/length 0 gets the legacy one-hour fallback; present option 51 is valid only at exactly four bytes, and all malformed lengths/statuses defer the ACK.

## Exact-SHA CI checkpoint

- Parent SHA `82160c242b345e6decc00e2b4f8ba2217419ae87`:
  - udhcpc event-script workflow #46152682534 FAIL in Python source contract at line 270 because whitespace was stripped from the policy source but not from the expected assertion string. GNU89/Werror/ASan/UBSan host regression PASS.
  - AI handoff contract #46152682536 PASS.
- Prior source SHA `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce`:
  - WAN DHCP callback #46152329744 PASS; DHCPv6 Reply retry #46152329810 PASS.
  - NVRAM transaction boundary #46152329844 PASS; Process inventory #46152329739 PASS; Process fail-closed regression #46152329718 PASS; AI handoff contract #46152329727 PASS.
  - USB storage hotplug #46152329716 FAIL at embedded source-contract line 1407: the workflow expected three calls to `udhcpc_script_failure_prepare_retry()`, but the correct production call graph now has four (subnet mask, lease-time option, ARP probe, bound/renew hook). All earlier compiled and shell regression steps passed.
  - XY-C1 firmware workflow #46152329717 was IN_PROGRESS when last checked; it must be rechecked against its exact SHA. No Build/TRX/artifact completion was recorded.
- Earlier Shadowsocks identity slice SHA `942f53201f14dcf7130cdae3c356fc87d53d1f2a`: Shadowsocks regression #46152116157 PASS, Process inventory #46152116314 PASS, AI handoff #46152116150 PASS; XY-C1 #46152116153 was IN_PROGRESS at last check.
- DHCP timeout normalization SHA `1e30efeb62d96cd2fe64d1fa41e03d97d34cde81`: udhcpc #46151496786, AI docs #46151496809, WAN DHCP #46151496793, storage #46151496821, DHCPv6 #46151496828, NVRAM #46151496810, Process #46151496829 and WAN monitor #46151496897 all PASS; XY-C1 #46151496815 still needs a fresh status query.
- Networkmap ProductID source SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d` has exact Build/TRX/artifact PASS (XY-C1 #46149948587; artifact #15265976441). This is L3 only for that SHA, not newer DHCP changes. Physical-router L4 remains NOT VERIFIED.

## Verification boundary

- L1: DHCP policy host regression compiled and passed GNU89, `-Wall -Wextra -Werror`, ASan and UBSan on `82160c242b345e6decc00e2b4f8ba2217419ae87`.
- L2: exact-head Python checker failed on a whitespace-normalization mismatch; storage contract independently had a stale retry-call count. This commit fixes both contracts without changing production source.
- L3: XY-C1 for the current production SHA remains unverified until Build, TRX verification and artifact upload all complete on one exact SHA.
- L4: physical-router runtime/network behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head udhcpc, storage hotplug and AI handoff Actions for this commit; repair any new causal failure on a new atomic commit.
2. Recheck XY-C1 runs #46152329717 and #46152682534 ancestry/status, then inspect fresh exact-SHA Build/TRX/artifact steps for the current main head.
3. Continue the next highest-value userland stability/security defect while independent Actions are pending. Preserve DHCPACK strict-length validation and the zero-timeout discovery throttle.
4. Keep the frozen backup unchanged; use complete-tree atomic commits and a non-forced expected-SHA update.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
