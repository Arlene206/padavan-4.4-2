# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; continue on `main` only. The parent of this atomic test-coverage update was `15349a22f71f3b32ae1d375a4854374feef6c3ad`; resolve the current commit from the live `main` ref on recovery.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and must not be modified/deleted. PR #1 remains closed/unmerged.
- Startup/protocol recovery was completed once. Do not repeat bootstrap or backup audits.

## Active task

- Close a real CI coverage gap in PPP callback generation state: `trunk/user/rc/src/tests/ppp_callback_state_file.c` existed and the WAN DHCP workflow's inline Python read it, but the workflow never compiled or executed it.
- THIS commit adds a GNU89/`-Wall -Wextra -Werror` ASan/UBSan run for that test and adds source-contract assertions that keep the compile/run step from disappearing. The existing push and pull_request filters already watch the state-file header, generation policy header and test file.
- The production implementation remains in `net_ppp.c`, `ppp_callback_generation_policy.h` and `ppp_callback_state_file.h`: stable sidecar fcntl lock, callback generation fencing, same-directory staged write, fsync, close and atomic rename. The test covers cross-process lock contention, last-good generation preservation on short write, embedded-NUL rejection, symlink refusal and file permissions.

## Verification boundary

- VPN route source baseline `a18496e1b8e5ca06a705f7268d19fdab278873dc`: VPN server #45777843194, AI docs #45777843207, DNSMasq #45777843397, Firewall #45777843212, Process inventory #45777843265/#45777843579, NVRAM audit #45777843380 and HTTPD security #45777843239 PASS. WAN route #45777843318 failed only because its workflow assertion expected the old unchecked selector; the corrected contract on `15349a22f71f3b32ae1d375a4854374feef6c3ad` passed run #45778158629.
- XY-C1 #45777843184 on that same production-source SHA `a18496e1b8e5ca06a705f7268d19fdab278873dc` completed PASS for Build XY-C1, TRX verification and artifact upload. This establishes L3 for that production source baseline; it does not establish hardware L4.
- Exact-head `15349a22f71f3b32ae1d375a4854374feef6c3ad`: WAN route #45778158629 and AI handoff #45778158644 PASS. XY-C1 #45778158665 passed its job but Build/TRX/artifact were SKIPPED because that commit changed no firmware production inputs.
- Firewall endpoint source SHA `fe68776c6f7421a6442b770fea4b8f449bb38651` also has XY-C1 #45777134107 Build/TRX/artifact PASS. Do not re-audit already-completed work.
- THIS PPP regression integration has not yet run on its new exact SHA. Check the exact-head WAN DHCP callback workflow and AI handoff workflow after publication. No local checkout was available and no local host test is claimed. No physical-router L4 claim.

## Exact first action for the next GPT

1. Fetch live `main`, record its exact SHA, and inspect that SHA's `WAN DHCP callback stability` run: confirm the new PPP state-file test compiles and prints its PASS marker. Inspect the AI handoff documentation contract on the same SHA; check XY-C1 only for honest source-input detection (a docs/workflow/test-only change may skip firmware Build/TRX/artifact).
2. If the PPP test or source contract fails, inspect the first causal job log and fix it in one atomic commit. If it passes, continue the next unfinished P1 reliability item from `REFACTOR_ROADMAP.md`, prioritizing concrete VSFTPD stop/restart cleanup or another unresolved whole-userland defect rather than repeating the finished PPP/VPN, IAPP or firewall checks.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live repository state and relevant exact-head Actions before ending. Record material PASS/FAIL/PENDING changes and confirm the first next action remains actionable. If all facts and the first action remain accurate, explicitly conclude with no file change and no commit.

# Cross-session execution mandate

Continue one autonomous engineering stream across GPT conversations. Every logical source/test slice must create all blobs, one complete Git tree and one atomic main commit, then publish with a non-forced compare-and-swap update. Before context exhaustion, finish the current safe slice and leave the next GPT an accurate exact-SHA state plus a concrete first task. Do not ask the user what to do next.

## Execution invariants

- Main only; one logical slice = all blobs, one complete tree, one atomic commit, then non-forced expected-parent SHA update. On CAS conflict, refresh and rebuild; never force-push.
- Never modify the frozen backup or reopen PR #1.
- Host Actions do not prove MIPS/uClibc runtime. XY-C1 L3 requires same-SHA Build, TRX verification and artifact upload PASS. Hardware L4 remains unverified absent explicit test.
- Update handoff only when engineering baseline, verification boundary or first concrete action materially changes; no poll-only or SHA-only commit.
