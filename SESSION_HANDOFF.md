## Current repository state

- Active branch: main. This checkpoint is the next atomic main commit on parent 52e34a54f58930247d5ec5c5df0f1a99b6de52fb; resolve the resulting live SHA from refs/heads/main before the next write because the repository is being updated concurrently across sessions.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; its warning file explicitly prohibits mutation. Legacy PR #1 remains closed and unmerged.
- Main-only development. Each logical slice must be one atomic main commit: create all blobs, one complete tree and one commit, then update main with a non-forced expected-SHA compare-and-swap. Never create side branches, PRs or force-push.
- RC firewall startup readiness production fix 7ca46fc7049f3380755f6b2fe5073d86727c6382 has same-source XY-C1 Build/TRX/artifact PASS (#45434352284; L3 VERIFIED). Hardware L4 remains NOT VERIFIED.

## Active task

### Complete exact-SHA validation of Clause 45 write-payload bounds

- Source SHA 7ae410c1ff14d974e659b8c29bbcd83d18269f0c adds raeth_mdio_cl45_write_data_status() and rejects values above 0xffff in mii_mgr_write_cl45() before the MDIO lock or register access. The GNU89 policy test covers 0, 0xffff, 0x10000 and 0xffffffff.
- Switch CLI #45438347998 on 7ae passed the standalone policy binary but failed its source-contract script because it referenced policy and test without loading them. The variable-loading correction landed at 52e34a5, but Switch CLI #45438676877 on 52e failed on an incorrect expected invocation count (the test correctly calls the helper four times). THIS commit changes the assertion to exactly four, pins the named boundary test and asserts mii_mgr.c directly includes the policy header.
- AI handoff #45438348166 on 7ae and #45438677094 on 52e failed because this handoff lacked the required literal phrase "one atomic main commit". THIS commit restores the required cross-session governance phrase.
- Source behavior remains fail-closed: mii_mgr_write_cl45() returns its existing 0/failure convention for a payload over 16 bits; the host policy helper returns -EINVAL.

## Verification boundary

- Exact source SHA 7ae410c1ff14d974e659b8c29bbcd83d18269f0c: Clause 45 GNU89/ASan/UBSan binary PASS; Switch CLI #45438347998 FAIL after that binary because policy/test paths were not loaded. The source checker correction at 52e loaded both, but exact Switch CLI #45438676877 then failed only because the source test has four helper calls while the assertion required at least five. THIS commit corrects that count and requires the regression function to remain present.
- AI handoff #45438348166 (7ae) and #45438677094 (52e) FAIL on the missing phrase one atomic main commit; corrected in THIS commit. Inspect exact new-head Action.
- NVRAM audit #45438348160 and XY-C1 #45438348129 on source SHA 7ae were IN_PROGRESS at last poll. XY Build was in progress; TRX/artifact pending. They are source-SHA evidence only for 7ae, not for this follow-up.
- Earlier shared Clause 45 address commit a08b86e XY-C1 #45436447371 failed from a missing policy-header prototype; include fixed in a650ed8. Switch CLI #45437706525 on checker-only descendant 472064b PASS. AI docs #45437414952 on a650ed8 PASS. The earlier XY-C1 runs #45437415030 (a650ed8) and #45438125350 (bc17758) were still IN_PROGRESS at last inspection; close/inspect separately, but do not transfer results.
- Firewall startup status propagation in RC source SHA 7ca46fc7049f3380755f6b2fe5073d86727c6382 is L3 VERIFIED: same-SHA XY-C1 #45434352284 Build, TRX and artifact all PASS. Physical-device L4 remains NOT VERIFIED.
- Current Clause45 source SHA still needs exact same-SHA XY-C1 Build, TRX and artifact PASS for L3. A successful host test or checker-only firmware workflow with skipped build is not L3.

## Exact first action for the next GPT

1. Inspect exact-head Switch CLI, AI handoff and NVRAM actions triggered by THIS commit; correct only the first demonstrated defect if either checker remains red.
2. Close XY-C1 #45438348129 on source SHA 7ae and NVRAM #45438348160; inspect Build, TRX and artifact steps individually. If Build failed, diagnose the first compiler error.
3. Confirm whether the prior XY runs #45437415030 (a650ed8) and #45438125350 (bc17758) completed, keeping their evidence scoped to their own SHAs.
4. Continue the next high-risk unfinished P1 userland/kernel reliability issue across RC/WAN/HTTPD/USB; do not repeat completed SmartDNS work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At each session ending, inspect live main, the frozen backup, exact source SHA, exact-head Actions, L2/L3/L4 evidence and the next concrete task. If the handoff remains accurate, make no no-op documentation commit.

# Cross-session execution mandate

Use one atomic main commit per logical slice. Before context exhaustion, record the exact current main SHA, source changes, regression results, exact-SHA Actions and first next action. Recovery is read-only by default; do not recreate completed audits or documentation.
