# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (repair Switch CLI classifier source-contract locator); parent main SHA: `60992c3f496f316baa8dc2377c829ce67a3986be`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The prior source slice `60992c3f496f316baa8dc2377c829ce67a3986be` applies command-specific packed-operand validation to MTK/RTL8367 switch CLIs. Its production C policy binary passed under ASan/UBSan, but the exact-head Switch CLI workflow failed in its Python source-contract step: the contract attempted to locate helper declarations using the literal `static int rtl8367_command_uses_packed_argument`, while the actual C formatting places `static int` and the function name on separate lines. The same stale locator existed for MTK. This follow-up corrects the test's source locator to match the actual declaration signature; no production logic is changed.

## Verification boundary

Exact-head parent SHA `60992c3f496f316baa8dc2377c829ce67a3986be`:
- Switch CLI run `45090786761`: FAIL only in the final Python embedded source contract at `source.split("static int rtl8367_command_uses_packed_argument", 1)`, which raised `IndexError`; policy compile and ASan/UBSan execution printed `RC switch CLI strict unsigned and packed ioctl parsing: PASS`, and prior switch workflow regression steps all passed.
- Process lifecycle inventory `45090786764`: PASS.
- AI handoff docs `45090786813`: PASS.
- HTTPD security `45090786771`: PASS.
- NVRAM audit `45090786770`: IN_PROGRESS at last poll.
- XY-C1 `45090786847`: Build XY-C1 IN_PROGRESS; TRX verification and artifact upload PENDING.

The correction in THIS COMMIT changes only the Python source-locator assertions, but its own exact-head Switch CLI rerun is required. The parent XY-C1 build does not validate this follow-up; inspect same-SHA Build/TRX/artifact. L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; confirm frozen backup `backup/pre-plugin-overhaul-2026-11-14` is still `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head Switch CLI: confirm the corrected MTK/RTL helper locators pass, along with packed-operand and full-width RTL register-write contracts.
3. Inspect exact-head XY-C1 on the same SHA for Build XY-C1, TRX verification and artifact upload; no host-only result substitutes for L3.
4. While checks run, continue a separate concrete stability review. If a check fails, inspect its first causal step/log and repair it.
5. Keep complete-tree atomic commits, fresh main parent, `force:false`, exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `60992c3f496f316baa8dc2377c829ce67a3986be`.
- This follow-up repairs only the embedded Python test locator for the already-added MTK/RTL command-specific packed-operand classifiers.
- Parent exact-head Switch CLI `45090786761` failed only on that locator after the GNU89/ASan/UBSan policy test had printed PASS; Process inventory `45090786764`, AI docs `45090786813`, HTTPD `45090786771` PASS; NVRAM `45090786770` in progress; XY-C1 `45090786847` Build in progress / TRX and artifact pending.
- THIS COMMIT needs its own exact-head Switch CLI/AI/XY-C1 validation. Parent results do not validate descendants; no L3/L4 claim.
