# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for the CI-contract repair: `342631b4efb09b4877dd944d74f03d20da7b4164` (`fix(firewall): validate LAN IPv4 address before NAT redirect`).
- The firewall-startup-policy run `44635384398` PASS on exact SHA `342631b4efb09b4877dd944d74f03d20da7b4164`.
- HTTPD security run `44635384245` failed only at the inline assertion expecting 9 `firewall_ipv4_address_is_safe()` calls; the new NAT path adds one necessary call, and source count is 10. The checker now expects 10.
- WAN DHCP callback run `44635384375` failed only because it expected the earlier `restore_wan_after_ppp_start_failure()` call. The source now uses stronger `restore_wan_after_firewall_failure()`, which delegates to the former and takes the physical interface down if restoring the firewall fails; the checker now pins those safeguards.
- NVRAM audit `44635384340` and XY-C1 `44635384278` were IN_PROGRESS at last poll; exact Build/TRX/artifact not yet confirmed. No L3 claim on this source SHA.
- Prior `ba47b5f` Process umbrella, AI docs, GPIO, detect_link, HTTPD, Process inventory, VPN, crond, watchdog and NVRAM PASS; XY-C1 `44634242726` still in progress at last poll.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. No physical-router result.

## Active task

- This commit repairs two now-stale workflow assertions only; no production code changed.
- HTTPD security expects ten explicit IPv4 address-validation callsites after the firewall path added a second LAN-IP guard for NAT redirect input.
- WAN callback tests now expect the stronger WAN restore wrapper, and pin that it delegates the physical-WAN restoration helper and takes the interface down if the fallback firewall recovery also fails.

## Exact first action for the next GPT

1. Check exact-head HTTPD security, WAN DHCP callback, AI-doc, NVRAM and XY-C1 Actions for this checker repair.
2. If an assertion still fails, inspect the job log and align the checker with actual source behavior only after verifying the production safety invariant.
3. Continue the independently evidenced shared MTD read/write EINTR issue already found in `flash_mtd.c`; do not repeat completed process/service lock and procfs audits.

## Verification boundary

- Firewall source/contract: HTTPD count failure is an assertion drift, corrected from 9 to 10; WAN restoration contract now expects the stronger fallback wrapper and verifies fail-closed interface shutdown.
- Exact SHA `342631b`: Firewall startup policy PASS; HTTPD and WAN callbacks failed stale checker assertions; NVRAM and XY-C1 remained pending.
- No local compile or physical-router result is claimed. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on the same SHA.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
