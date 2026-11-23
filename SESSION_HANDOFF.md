# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (reject invalid MTK ESW PHY link-mode encodings); parent main SHA: `c0a8285cc4b84cf05e0d691211759dc41a44c3f5`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The MTK ESW kernel ioctl `change_port_link_mode_uapi()` validated the port number but not the caller's packed `port_link_mode`. It extracts speed from bits 0..3 and flow control from bits 8..9, then writes MDIO registers. Unsupported speed values 10..14 fell through the speed switch and still programmed the default PHY configuration; flow value 3 fell through as the default flow setting. Reserved bits above the documented fields were silently accepted and cached in `g_port_link_mode[]`.

This commit:
- Adds a pure helper matching the UAPI: speeds 0..9 and 15 (power off), flow modes 0..2, and no reserved bits outside masks `0x0000000f` and `0x00000300`.
- Rejects malformed encodings with `-EINVAL` before cache comparison and before any MDIO reads/writes.
- Adds GNU89/ASan/UBSan host tests covering every valid speed/flow pair and unsupported speed, flow and reserved-bit values.
- Extends Switch CLI workflow filters and production source-order assertions.

The preceding classifier source-contract repair `c0a8285cc4b84cf05e0d691211759dc41a44c3f5` has exact-head Switch CLI `45090987240` PASS and AI docs `45090987221` PASS. This kernel/PHY task requires its own exact-head host and XY-C1 checks.

## Verification boundary

Parent source SHA `c0a8285cc4b84cf05e0d691211759dc41a44c3f5`:
- Switch CLI source-contract repair `45090987240`: PASS.
- AI handoff docs `45090987221`: PASS.
- Prior source SHA `60992c3f496f316baa8dc2377c829ce67a3986be`: Process inventory `45090786764`, AI docs `45090786813`, HTTPD `45090786771`, NVRAM `45090786770` all PASS. Switch CLI `45090786761` failed only at the obsolete helper source locator after the GNU89/ASan/UBSan policy test printed PASS.
- XY-C1 for `60992c3f496f316baa8dc2377c829ce67a3986be` run `45090786847` was still in progress at last poll (Build active, TRX/artifact pending); the docs/workflow-only repair did not trigger another firmware build.

For THIS COMMIT, inspect exact-head Switch CLI and XY-C1. The host policy test must pass under GNU89/ASan/UBSan; same-SHA firmware Build/TRX/artifact must pass before L3 is claimed. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head Switch CLI: GNU89/ASan/UBSan mode policy test, all defined speed/flow pairs, reserved-field rejection, and the source contract proving validation occurs before MDIO.
3. Inspect same-SHA XY-C1 Build, TRX verification and artifact upload; host-only results are not L3.
4. While checks are pending, pursue another independent concrete stability issue; if a job fails, inspect its first causal log and repair it.
5. Keep complete-tree atomic commits on fresh main with `force:false` and exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `c0a8285cc4b84cf05e0d691211759dc41a44c3f5`.
- MTK ESW `change_port_link_mode_uapi()` rejects speed codes 10..14, flow code 3, and reserved link-mode bits before cache lookup or MDIO. Speeds 0..9/15 and flows 0..2 remain valid.
- Parent `c0a8285cc4b84cf05e0d691211759dc41a44c3f5`: exact-head Switch CLI `45090987240` and AI docs `45090987221` PASS. Prior source `60992c3f` Process inventory, AI docs, HTTPD and NVRAM PASS; its Switch CLI source-check failure was fixed by `c0a8285`.
- XY-C1 `45090786847` for `60992c3f` was Build-in-progress at last poll; TRX/artifact not confirmed. This source change requires exact-head firmware checks.
- Full MIPS build and hardware L4 NOT VERIFIED.
