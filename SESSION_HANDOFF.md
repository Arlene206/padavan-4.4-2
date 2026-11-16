# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for the current validation work: c3cf016298907f704ef1724ca317a5fc61d329e2. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807 and read-only.
- Exact-head c3 Firewall Startup 43938857743 failed in the canonical checker because its body() signature for reload_nat_modules still included the former return type; source regressions preceding it passed. Exact-head c3 XY-C1 43938857763 hit the same checker and skipped firmware Build/TRX/artifact.
- Exact-head c3 AI handoff contract 43938858001 passed. NVRAM 43938857987, USB storage 43938857733, HTTPD 43938857658, Process inventory 43938857476, Process regression 43938857455 and several other jobs were in progress at last poll; other network/service/security checks passed.
- Parent 10ab Firewall Startup 43936882423, Process 43936882692, Storage 43936881877, AI docs 43936882570, WAN route 43936881887, USB modem identity 43936881865, HTTPD security 43936882544, AliDDNS 43936881919 and DNSMasq 43936881899 passed.
- Historical PR #1 remains closed/unmerged; do not resume.

## Active task

start_firewall_ex() now returns errno-style failures for unsafe interface tokens, retained transaction recovery, failed core table restore/rollback, snapshot cleanup, and Adbyby restore errors. Incomplete rollback disables forwarding and preserves recovery snapshots. restart_firewall() always attempts the independent UPnP refresh and merges its status after the core ruleset result.

Next, verify this return-status contract with the transaction fault-injection test and exact-SHA CI, then inspect restart_firewall() callers and the firewall extension-hook error boundaries. Never enable IPv4 forwarding on an unapplied core ruleset.

## Verification boundary

- On c3, the core firewall status change is committed as c3cf016298907f704ef1724ca317a5fc61d329e2. Exact-head Firewall Startup 43938857743 and XY-C1 43938857763 both failed in the stale checker signature for reload_nat_modules(); preceding host regressions passed, but firmware build/TRX/artifact were skipped.
- This correction matches the stable symbol name, adds source contracts for firewall transaction/restart status, and wires tests/firewall_restore_transaction.c into a GNU89/-Werror/ASan/UBSan CI step.
- No full local firmware build or physical-router validation is claimed. Physical-router L4 remains NOT VERIFIED; frozen backup unchanged.

## Exact first action for the next GPT

1. Inspect exact-SHA Firewall Startup, XY-C1 Build/TRX/artifact, Process, Storage, NVRAM and AI handoff runs after the checker/test wiring correction.
2. If the firewall transaction test or source checker fails, repair the first causal issue. Once green, continue auditing restart_firewall() callers and privileged extension-hook result boundaries.
3. Keep commits main-only, complete-tree atomic with non-forced expected-SHA CAS. Frozen backup stays read-only; physical-router L4 stays NOT VERIFIED.
