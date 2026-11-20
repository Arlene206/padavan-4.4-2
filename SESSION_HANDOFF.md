# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only engineering across conversations. Use one atomic main commit per logical source slice; create all blobs and one complete tree, then update main using non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regressions and diagnose failed exact-SHA checks from their causal logs.
- Before context exhaustion, finish the current safe slice, inspect exact-head Actions, and leave an accurate first action for the next GPT.

### Latest exact-head implementation checkpoint — bc0d9316777baa125e2ba22ef776d9ea812e3dd4

The source slice on this commit makes RT3352 MII TxPower a required radio-enable step, captures rollback errors separately, and prevents duplicate MLME publication. Exact-head Action snapshot:
- Wireless: completed/success (#44746584209)
- AI handoff docs: completed/success (#44746584183)
- XY-C1: in_progress (#44746584073)
- NVRAM: completed/success (#44746584217)
- Process regression: no exact-SHA run listed
- Process inventory: completed/success (#44746584108)
Physical router verification remains NOT VERIFIED. Firmware L3 is claimed only if this same product-source SHA has detector, Build XY-C1, TRX and artifact-upload success.

## Current continuation task — strict RC switch CLI numeric parsing

- Current main before this source slice: `9f2b19c2dac08dc1fb03d0da1e282a549d3c8594` (this parent commit only refreshed the RT3352 verification checkpoint; source SHA remains `bc0d9316777baa125e2ba22ef776d9ea812e3dd4`).
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not move or modify it.
- At `bc0d9316`, Wireless #44746584209, AI docs #44746584183, NVRAM #44746584217, Process regression #44746584355, Process inventory #44746584108, watchdog #44746584223, iNIC signal #44746584178 and HTTPD security #44746584119 passed. XY-C1 #44746584073 was still in progress. AI docs #44747215425 passed on docs-only SHA `9f2b19c2`.
- Active source defect: RC RTL8367 and MTK switch front-ends accepted malformed/overflowing CLI numeric values through `atoi()` and `get_param_int_hex()`. This slice adds strict checked parsing, GNU89 regression and Switch CLI/XY-C1 path coverage.
- Local GNU89/Werror/ASan/UBSan parser regression passed. New exact-SHA Actions are pending until the atomic source commit is pushed.

## Current repository state

- Current main before this RT3352 TxPower status slice: `31de47640faed47555e830d86cd69f7228887d7e`; the boot-state isolation production source was introduced in `d250aff4e337f5c919e11c44cf5a1e8e5c1cb5a8`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Source implementation commit `9e9d643d` and follow-up test/docs SHA `9075dc63` passed Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149; NVRAM #44744845007 passed on `9075dc63`.
- Boot-state isolation follow-up `d250aff4` changed the policy to assert isolation before RadioOn; its six-event rollback test count was corrected on `31de4764`. Wireless #44746039419, AI #44746039183 and Process inventory #44746039325 passed on `31de4764`; NVRAM #44746039252 was in progress. Product-source XY-C1 #44745705732 on `d250aff4` remained in progress; NVRAM #44745706021 passed.

## Previous MII follow-up context

## Active task

- `rtl8367_main()` and `mtk_esw_main()` used `isdigit()+atoi()` for command selectors and `get_param_int_hex()` for operands. Their conversions omitted end-pointer and overflow checks, allowing malformed or partially valid tokens (for example `10junk` and `0x1z`) to be coerced; extra operands were ignored.
- This slice adds a shared unsigned parser: decimal-only command selector, decimal or explicit `0x` operands, rejection of signs/whitespace/trailing junk/overflow, and an argc upper bound. It adds a standalone GNU89 regression plus exact-head Switch CLI and XY-C1 path filters for both production front-ends.
- Local test passed with `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`. Push this complete nine-file tree as one atomic main commit, then inspect exact-SHA Switch CLI, XY-C1, AI, Process and NVRAM runs. Never overwrite later concurrent main commits; use expected-SHA CAS.

## Verification boundary

- Host parser regression PASS; no target MIPS build, XY-C1 L3 for the new production-source SHA, or physical switch/router L4 is claimed yet.
- XY-C1 L3 requires detector, Build XY-C1, TRX verification and artifact upload all successful on this exact source SHA. Results on `9e9d643d`, `d250aff4`, or `bc0d9316` are not transferable to a later source SHA.

## Exact first action for the next GPT

1. Inspect live main and latest exact-SHA Switch CLI, XY-C1, AI docs, Process and NVRAM Actions after the parser commit.
2. Diagnose failures from their logs. If checks are pending, move to another independent P1 source fix.
3. Confirm detector/build/TRX/artifact all pass on the exact parser production SHA before claiming firmware L3. Physical router/switch L4 remains NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect the live repository, exact commit SHA, relevant exact-head Actions and the frozen backup status. Update these files only when a fact, active task, CI result or next action materially changes; if still accurate, do not create a no-op commit.

## Operating constraints

- Host unit tests, exact-SHA Actions, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
- Keep the newest 20–119 complete records in `dev_log.md`; batch archive oldest 100 only at the protocol threshold and keep the index/hash exact.
- Do not re-audit closed work without new evidence. Do not report a test, commit, build or device check that was not performed.
