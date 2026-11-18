# 2026-11-17 continuation checkpoint — Adbyby detached startup readiness

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; source parent before THIS Adbyby readiness slice: `f50583d26acad4904271fcd0d968425c2cd906fc`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains the preserved archival ref; head: `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not switch to or modify it.
- The previous XY-C1 full-Git-diff detector task is complete; do not repeat that audit.
- The ttyd detached-start source fix and exact handoff-heading correction are already committed. This slice moves to the next concrete async add-on gap found in the handoff queue.
- No physical-router L4 validation is established.

## Active task
- trunk/user/adbyby/adbyby.sh previously forked /tmp/adbyby/adbyby and immediately continued to DNS/firewall/hosts publication. adbyby_start() and adbyby_uprules() now share a bounded process-presence startup postcondition, and the lifecycle restart path preserves a failed start status.
- The Process workflow now includes trunk/user/adbyby/tests/adbyby_start_postcondition.sh to cover RUNNING, delayed appearance, ABSENT timeout, UNKNOWN/malformed inventory, sleep failure, both publication gates and restart status propagation.
- The production Adbyby script is already in the XY-C1 firmware workflow path filters.

## Verification boundary
- The new shell regression is wired into the Process lifecycle workflow but was not run locally in this connector-only editing environment.
- After the commit lands, check exact-head push Actions for the Process lifecycle regression, AI handoff documentation and XY-C1 firmware validation. For L3, confirm that the same source SHA reaches Build XY-C1, TRX verification and firmware artifact upload; do not infer L3 from host tests.
- If an exact-head check fails, repair the first causal failure on latest main and rerun. If pending, continue the independent AdGuardHome async-start audit instead of waiting idle.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch the live main SHA; inspect push-triggered Actions whose head_sha exactly matches it for Process lifecycle, AI handoff and XY-C1. Verify each outcome, and inspect the first failed job/step rather than repeating the whole previous audit.
2. Confirm the XY-C1 run reached Build XY-C1, TRX verification and artifact upload on that exact product-source SHA.
3. If checks pass, inspect trunk/user/adguardhome/adguardhome.sh from start_adg() through its process-state and stop helpers. Verify that successful background fork is not mistaken for a surviving AdGuardHome daemon and that DNS/firewall state is rolled back or withheld on startup failure; add a deterministic regression before editing production code.
4. Keep the frozen backup unchanged. Do not repeat the already completed XY-C1 detector, WAN restart or ttyd source reviews without a new failing exact-head result.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state before ending. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, update this file only when the active task, verification gate or next first action materially changes.

# Cross-session execution mandate
Continue as the same maintainer: one logical slice per one atomic main commit, regression and build/workflow integration, exact-SHA push validation, then the next ranked defect. Before context exhaustion, checkpoint exact Actions and hardware evidence. Never force-push, use a development branch/PR, alter the frozen backup, or claim an unperformed test/build/hardware result.
