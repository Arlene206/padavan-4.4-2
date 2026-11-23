# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (RC switch command-specific packed operand validation); parent main SHA: `ae30039aa9b0a70800d76c1c6d7b273d88c44540`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

A follow-up to `ae30039aa9b0a70800d76c1c6d7b273d88c44540` refines packed-ioctl validation to match each command's actual call path. The first bounded-parser slice correctly prevented wide command IDs from aliasing low-byte ioctl commands, but its global third-argument cap treated every `argv[3]` as a packed request parameter. RTL8367 register writes do not use that shape: `REG_SET` / `PHY_REG_SET` pass `argv[3]` by pointer as the register value, while `argv[2]` is the packed register address. Status/register commands likewise pass `argv[2]` into the packed high bits.

This commit:
- Adds per-front-end command classifiers for commands whose second CLI operand is packed into the ioctl request (MTK link/speed/MIB status; RTL link/speed/MIB and register/PHY register operations).
- Bounds those `argv[2]` operands to the actual UAPI parameter width, while ordinary data operands keep their full unsigned range.
- For RTL register-write commands, parses the third operand as a normal unsigned register value; generic ioctl commands still bound `argv[3]` as their packed parameter.
- Extends the Switch CLI source contract to pin command-specific routing, including preserving raw register-write values.

Parent `ae30039aa9b0a70800d76c1c6d7b273d88c44540` has the initial low-command-bit and packed-parameter helper implementation; its Switch CLI and AI docs checks passed. Its XY-C1 Build was in progress, with TRX/artifact pending; current results are below.

## Verification boundary

Parent source SHA `ae30039aa9b0a70800d76c1c6d7b273d88c44540` exact-head results at last poll:
- Switch CLI ioctl failure-status regression `45090457040`: PASS.
- AI handoff documentation contract `45090457070`: PASS.
- Process lifecycle inventory `45090457046`: PASS.
- HTTPD security `45090457067`: PASS.
- NVRAM transaction boundary `45090457094`: IN_PROGRESS.
- XY-C1 firmware `45090457058`: Build XY-C1 IN_PROGRESS; TRX and artifact upload PENDING.

These are parent results only. Inspect THIS COMMIT's exact-head Switch CLI, AI docs and XY-C1 runs after publication. The source refinement keeps register-write payloads full-width while bounding actual packed operands. No full MIPS build, TRX/artifact or physical-router result is inferred from the parent SHA; L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head Switch CLI for the current source SHA. Confirm GNU89/ASan/UBSan packed-width tests, MTK and RTL command-specific packed-argument routing, and the full-width RTL register-write value contract pass.
3. Inspect exact-head XY-C1 on the same SHA; confirm Build XY-C1, TRX verification and artifact upload actually execute and pass. Host-only checks are not L3.
4. If checks are pending, continue another concrete module defect. If an exact-head job fails, inspect its first causal log and repair it before claiming the slice verified.
5. Use complete-tree atomic commits on fresh main with `force:false` and exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `ae30039aa9b0a70800d76c1c6d7b273d88c44540`.
- Command-width and packed-parameter helpers remain. This follow-up applies parameter-width validation only to operands actually shifted into ioctl high bits; RTL register-write values remain full-width data and register addresses are bounded.
- Parent `ae30039aa9b0a70800d76c1c6d7b273d88c44540`: Switch CLI `45090457040`, AI docs `45090457070`, Process inventory `45090457046`, HTTPD `45090457067` PASS; NVRAM `45090457094` in progress; XY-C1 `45090457058` Build in progress / TRX and artifact pending.
- This commit needs exact-head Switch CLI/AI/XY-C1. Parent PASS results do not validate descendants; L3/L4 remain NOT VERIFIED.
