## Current repository state

- Active development branch: `main`; source parent for the WAN source PHY slice: `659b91b497b5cf8073b54feab849a91623092c70`. Always fetch live `main` and its tree before any write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable and must not be modified, used for development, or deleted.
- No open PRs; legacy PR #1 is closed and its valid unmerged work was ported to main.
- HTTPD protocol-selector guard and regression are L3 VERIFIED on exact SHA `659b91b497b5cf8073b54feab849a91623092c70`: HTTPD lifecycle #45388114470 PASS, AI handoff docs #45388114468 PASS, Process lifecycle #45388114507 PASS, and XY-C1 #45388114515 PASS including Build XY-C1, TRX verification and artifact upload. Physical-router L4 remains NOT VERIFIED.
- MTK AP private-ioctl safety has prior L3 verification on exact source SHA `d33fcf06b73fbe2771ab0b559116d3e750dc4918`; physical-router L4 remains NOT VERIFIED.

## Active task — Bound the persisted WAN source PHY selector

- `get_wan_ether_link_direct()` reads `wan_src_phy` through a checked NVRAM accessor but previously sent every nonnegative integer directly to `phy_status_port_link()`. The MTK kernel maps an unsupported UAPI port ID to a successful zero-valued link result; corrupt selectors (including CPU port IDs) could therefore be mistaken for a real cable disconnect and trigger WAN teardown.
- The generic switch UAPI defines WAN=0 and external LAN ports 1..5, while CPU ports 14/15 are not external WAN PHYs. THIS COMMIT preserves legacy negative-value fallback to WAN=0 but rejects positive IDs >5 with `ERANGE` before ioctl. The caller returns `-1` with errno so detect_link retains the last valid sample and retries.
- Regression covers external IDs 0..5, negative compatibility including INT_MIN, and invalid 6/13/14/15/INT_MAX with untouched output. The detect-link workflow runs GNU89/`-Werror`/ASan/UBSan plus source-order contract; the XY-C1 workflow watches the new policy header/test and production caller.
- Historic DNSMasq/Process failures from older stale source contracts are superseded; current Process inventory #45388114507 PASS confirms no need to replay those assertions.

## Verification boundary

- THIS COMMIT contains the source change, policy, test, workflow contracts/path triggers and current handoff in one atomic commit.
- Check exact-head detect-link regression, XY-C1 Build/TRX/artifact, AI docs, Process and NVRAM Actions. Physical router L4 remains unverified.
- HTTPD selector source is L3 verified on parent SHA `659b91b`.

## Exact first action for the next GPT

1. Fetch current `main` and query push Actions by exact head SHA; inspect detect-link WAN source PHY regression and XY-C1 Build/TRX/artifact, plus AI/Process/NVRAM exact-head checks.
2. Fix any first causal exact-head failure atomically; do not claim L3 while a firmware Build/TRX/artifact step is pending or skipped.
3. After this WAN selector slice passes, continue the next highest-priority unresolved stability/security issue outside completed MTK AP ioctl and HTTPD selector work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
