# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Parent for the shared miniupnpd netmask validation slice: `5e2c7a3fd8479bd1cdbdf455b34c18d20494e1b7`.
- Exact parent Actions: Process inventory `44637423511` PASS; Process lifecycle fail-closed regression `44637423543` PASS. AI handoff docs `44637423581` failed because the current handoff omitted `The checkpoint is mandatory; a document edit is conditional.`; that sentence is restored here. NVRAM `44637423514` and XY-C1 `44637423396` were IN_PROGRESS at last poll.
- MTD flash I/O uses the shared eight-consecutive-EINTR wrapper without outer restart loops. Process regression verifies bounded wrapper use, no direct syscalls/no outer retries, short-I/O progress, read EOF and zero-write EIO.
- Earlier firewall source SHA `342631b4efb09b4877dd944d74f03d20da7b4164` passed Firewall Startup; HTTPD/WAN checker contracts were subsequently repaired in `8987975` and `5c557db`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged. No physical-router result is available.

## Active task

- Firewall startup and baseline NAT reject malformed/non-contiguous LAN masks, but miniupnpd startup still checked only dotted-IPv4 syntax before `ip2class()`.
- Extracted the nonzero contiguous IPv4 netmask predicate into shared `ipv4_netmask_policy.h`; firewall and miniupnpd both consume it. Miniupnpd now rejects invalid/non-contiguous masks and returns `EINVAL` before subnet derivation.
- Existing firewall regression covers valid prefix masks, zero, non-contiguous, malformed and injection strings. Firewall source-order contract and miniupnpd daemon-start source-order contract cover both callers. The new shared header is included in both affected workflows' push and pull_request path filters.
- This integration awaits exact-head Firewall, miniupnpd, Process inventory, AI docs, NVRAM and XY-C1 Actions.

## Exact first action for the next GPT

1. Check exact-head Firewall startup, miniupnpd config stability, Process inventory, AI docs, NVRAM and XY-C1 results. On FAIL, inspect the exact job log and repair only the demonstrated issue.
2. L3 requires XY-C1 Build, TRX verification and artifact upload all SUCCESS on the same source SHA. No host regression is a substitute.
3. Continue with an independently evidenced stability/security issue outside completed firewall netmask, MTD bounded-I/O and RStats persistence work.

## Verification boundary

- L1: shared contiguous-mask policy cases run through existing GNU89 Firewall regression; integration awaits exact-head CI.
- L2: parent MTD Process inventory and Process lifecycle runs PASS. AI docs required the exact conditional-checkpoint sentence, restored here. Current Firewall/miniupnpd/AI/Process Actions pending.
- L3: parent NVRAM and XY-C1 were IN_PROGRESS; this integration requires exact-head Build/TRX/artifact success.
- L4: non-contiguous LAN-mask behavior on physical router remains NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
