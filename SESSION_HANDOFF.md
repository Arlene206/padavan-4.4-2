## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- Current parent for the remaining DHCPACK source-check correction: `773ffae54bcd5e7e934fba86f57031f0e4e62399`. Production code is unchanged across the checker follow-ups; the GNU89/Werror/ASan/UBSan C policy regression has passed on every exact-head retry.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store feature is integrated.

## Active task

- The udhcpc checker has two lease-time contract sections. The first registration assertion was normalized, but the duplicate later assertion at line 407 still expected the whitespace-bearing `!= 0` fragment inside a whitespace-stripped source. Align the duplicate assertion with the actual C `test_lease_time_option_requires_exactly_one_uint32() != 0 ||` registration in `main()`.
- The separate USB storage source-contract call count is now corrected to four. Do not change DHCP production logic: malformed present option 51 must defer the ACK; only true absence retains the one-hour default.

## Exact-SHA CI checkpoint

- Parent SHA `773ffae54bcd5e7e934fba86f57031f0e4e62399`:
  - udhcpc workflow #46153023184 FAIL at duplicate source-contract assertion line 407. GNU89/Werror/ASan/UBSan policy binary PASS.
  - The preceding SHA `1d74edb7b08fe3460109f351dd14c067caab8ba7` has USB storage hotplug workflow #46152870433 PASS and AI handoff contract #46152870482 PASS; udhcpc workflow #46152870458 failed only in the same Python contract (first stale registration assertion).
- Source SHA `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce`: WAN DHCP callback #46152329744 PASS; DHCPv6 Reply retry #46152329810 PASS; NVRAM #46152329844 PASS; Process inventory #46152329739 PASS; Process fail-closed #46152329718 PASS; AI handoff #46152329727 PASS. USB storage #46152329716's stale retry count is fixed and confirmed by #46152870433 on a descendant.
- XY-C1 workflow #46152329717 remains IN_PROGRESS on production SHA `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce` at last status. No exact Build/TRX/artifact completion recorded.
- Shadowsocks identity slice `942f53201f14dcf7130cdae3c356fc87d53d1f2a`: Shadowsocks #46152116157 PASS, Process inventory #46152116314 PASS, AI docs #46152116150 PASS; XY-C1 #46152116153 still IN_PROGRESS at last check.
- Networkmap ProductID source SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d` has Build/TRX/artifact PASS (XY-C1 #46149948587; artifact #15265976441). This is L3 only for that SHA. Physical-router L4 remains NOT VERIFIED.

## Verification boundary

- L1: policy C regression GNU89, `-Wall -Wextra -Werror`, ASan and UBSan PASS.
- L2: storage hotplug exact-head regression PASS on descendant `1d74edb`; udhcpc's Python contract still needs a clean PASS with the duplicate assertion corrected.
- L3: latest firmware Build, TRX and artifact must all complete on one exact production SHA. Earlier networkmap L3 is not transferable to this DHCP build.
- L4: physical-router runtime/network behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head udhcpc and AI handoff Actions for this checker correction. Require the complete Python source contract and host regression to PASS; diagnose any remaining assertion by checking all duplicate blocks, not just the first match.
2. Recheck XY-C1 #46152329717 and current main ancestry/status; require Build, TRX verification and artifact upload from one exact production SHA.
3. Continue independent userland stability/security work while firmware validation is pending. Preserve malformed DHCPACK rejection and zero-timeout discovery throttling.
4. Keep the backup immutable and use complete-tree atomic commits with non-forced expected-SHA updates.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
