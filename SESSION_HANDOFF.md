## Current repository state

- Live branch: `main` at `7efbd10d64f946ce112aab1e70a4206b649bfbf5`. RC production fix `7ca46fc7049f3380755f6b2fe5073d86727c6382` was based on `e1631a606564e95a93802123ceba97478ac3197c`; current main includes RAETH Clause 45 bounds commit `a76f771cd56c1ad623769d32400562f853330e34`, follow-up MDIO PHY address bounds `7efbd10d64f946ce112aab1e70a4206b649bfbf5`, and an earlier firewall checker correction `03ca0dbf9b201bb8752c53afbf0b44632c1818e8`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed and unmerged.
- Every logical task must be one atomic main commit: create a complete Git tree and one commit, then update `main` by non-forced expected-SHA compare-and-swap. Do not create extra branches or force-push.
- SmartDNS checker/runtime correction at `79d912269f2ec021e21d1f543fe879140f4aab6e` passed Process, DNSMasq, USB and AI host checks; XY-C1 skipped Build/TRX/artifact, so it is not L3 evidence.

## Active task

### Preserve firewall startup prerequisite failures in the PID 1 boot result

- `rc.c:init_router()` previously let failed required NAT restore or IPv4/IPv6 filter operations block WAN startup without retaining their error in `boot_services_status`. PID 1 could therefore report READY while WAN was deliberately withheld.
- `firewall_startup_policy_wan_failure_status()` now retains the first applicable selector/VLAN/NAT/IPv6/filter error; `init_router()` merges it into `boot_services_status` before the no-WAN branch. Explicitly disabled features remain compatible.
- Regression tests cover exact errno propagation, precedence, disabled-feature behavior and READY-to-DEGRADED mapping.
- Workflow source assertions now normalize whitespace around the split C assignment and status-merge call, while checking the exact arguments and fail-closed ordering.

## Verification boundary

- On source SHA `7ca46fc7049f3380755f6b2fe5073d86727c6382`, Firewall Startup C policy and checked-NVRAM host regressions passed. Its first static source contract failed on a line-wrapped status assignment.
- Follow-up `03ca0dbf` fixed that assignment assertion but the Firewall Startup job #45435162086 then failed because the status-merge call also had a line break after the opening parenthesis. This commit corrects the assertion by checking the call prefix, exact argument pair and ordering separately.
- AI handoff #45435161894 and #45435366901 failed because the handoff lacked the required literal end-of-session checkpoint sentence and exact required section headings; this handoff restores both.
- Process inventory #45434352443 on `7ca46fc` PASS. XY-C1 #45434352284 on `7ca46fc` had Build in progress and TRX/artifact pending; it is the L3 validation run for the firewall source fix.
- RAETH Clause 45 commit `a76f771`: Switch CLI #45434536973 PASS; XY-C1 #45434537034 had Build in progress and TRX/artifact pending. AI handoff #45434536945 failed for the same missing checkpoint sentence.
- RAETH MDIO address commit `7efbd10`: Switch CLI #45435366858 failed in the new MDIO source-order checker after the Clause 22 address policy test passed; AI handoff #45435366901 failed on the handoff checkpoint. These exact failure details are in their job logs and need separate follow-up.
- L3 requires Build, TRX and artifact PASS on the same exact source SHA. Physical-device L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Query exact-head Firewall Startup and AI handoff Actions created by the current commit; inspect logs and fix any remaining failure atomically.
2. Recheck XY-C1 #45434352284 on `7ca46fc` for final Build/TRX/artifact status; do not claim L3 until all three pass.
3. Inspect Switch CLI #45435366858 and fix the MDIO Clause 22 source-order assertion only after validating the actual source path; preserve the successful GNU89 bounds policy test.
4. Recheck XY-C1 #45434537034 for the RAETH Clause 45 commit. Do not use one commit's run to certify another source SHA.
5. When exact-head checks are green, continue the broad RC startup-result propagation P1 queue or another higher-risk unfinished userland stability issue outside SmartDNS/DNS Forwarder. A pending check is not a stop condition.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At every session ending, inspect the live repository state, frozen backup, active source SHA, exact-head Actions, validation boundary and first action. If all remain accurate, explicitly conclude the checkpoint with no file change and no no-op commit.

# Cross-session execution mandate

Recover once, implement and hand off. Keep one atomic main commit per logical task; create all blobs, one complete tree and one commit, then update main by non-forced compare-and-swap. Before context exhaustion, record exact tests, Actions results and the next precise action. Never force-push or overstate firmware/hardware evidence.
