## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- Current parent: `942f53201f14dcf7130cdae3c356fc87d53d1f2a`. Latest parent change binds Shadowsocks watchdog health to executable+argv identity; exact-head Shadowsocks regression #46152116157, AI handoff #46152116150 and Process inventory #46152116314 PASS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store feature is integrated.

## Active task

- Reject malformed DHCPACK lease-time option 51 while preserving the compatibility fallback for a truly absent option. Previous `udhcp_get_option32()` maps absent and present-but-wrong-length (including zero-length) to the same NULL result; the old ACK path incorrectly accepted malformed presence as a fabricated one-hour lease.
- New ACK parsing uses `udhcp_get_option_with_length()`: accept only status=0/length=0 (absent) or status=1/length=4 (valid 32-bit lease time). Parser errors and any present non-four-byte length defer ACK and follow the existing acquisition/renewal retry policy. The four-byte value remains unaligned-safe and network-order decoded; absent still uses the legacy one-hour fallback.
- Added GNU89 policy tests for absent, valid 4-byte, malformed parser status, zero/one/three/five-byte and contradictory status/length pairs. Checker ensures validation/deferral precede fallback/decode. Isolated GNU89/Werror/ASan/UBSan test PASS. Exact-head source workflow, AI documentation and XY-C1 checks pending.

## Exact-SHA CI checkpoint before this DHCPACK source slice

- DHCP discovery zero-timeout guard SHA `1e30efeb62d96cd2fe64d1fa41e03d97d34cde81`: udhcpc workflow #46151496786 PASS; AI docs #46151496809 PASS; WAN DHCP #46151496793 PASS; storage hotplug #46151496821 PASS; DHCPv6 reply retry #46151496828 PASS; NVRAM #46151496810 PASS; process inventory #46151496829 PASS; WAN monitor #46151496897 PASS. XY-C1 #46151496815 remains in progress.
- Latest pre-change SHA `942f53201f14dcf7130cdae3c356fc87d53d1f2a`: Shadowsocks lifecycle regression #46152116157 PASS; AI docs #46152116150 PASS; Process inventory #46152116314 PASS. XY-C1 #46152116153 remains in progress. The firmware build result must be checked for an exact production SHA and includes Build/TRX/artifact before claiming L3.
- Earlier networkmap ProductID fix SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`: exact L1-L3 pass, including networkmap regression and XY-C1 Build/TRX/artifact. This is not the newer DHCP/Shadowsocks production tree.
- Physical-router L4 remains NOT VERIFIED.

## Verification boundary

- L1: isolated GNU89/Werror/ASan/UBSan DHCP lease option status/length test PASS.
- L2: exact-head udhcpc checker and host regression for this DHCPACK change: PENDING.
- L3: exact-head XY-C1 Build/TRX/artifact for the current production tree: PENDING.
- L4: physical-router runtime/network behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Actions for this DHCPACK lease-time commit: udhcpc source-contract/host tests, AI handoff documentation and XY-C1 Build/TRX/artifact; also recheck whether XY-C1 run #46152116153 at the parent has finished.
2. On any checker/unit failure, diagnose its exact assertion and make the smallest causal source/test correction in one complete-tree atomic commit. If firmware is pending, keep progressing another independent stability/security slice.
3. Continue the DHCP client/AP-mode investigation only where source evidence supports another concrete defect. Preserve the separate zero-timeout discovery guard and do not repeat completed networkmap/NVRAM audits without new evidence.
4. Keep the frozen backup unchanged. Distinguish CI L1-L3 from physical-router L4.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
