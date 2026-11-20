# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only engineering across conversations. Use one atomic main commit per logical source slice; create all blobs and one complete tree, then update main using non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regressions and diagnose failed exact-SHA checks from their causal logs.
- Before context exhaustion, finish the current safe slice, inspect exact-head Actions, and leave an accurate first action for the next GPT.

## Current repository state

- Current main before this follow-up slice: `9075dc6385307f5df83ce9d13039462648395583`; its RT3352 source parent is `9e9d643dd8c3a78d135c8dae908857f8b90c0276`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Source implementation commit `9e9d643dd8c3a78d135c8dae908857f8b90c0276` adds RT3352 MII radio/isolation transaction handling. On follow-up test/documentation SHA `9075dc6385307f5df83ce9d13039462648395583`, Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149 passed; NVRAM #44744845007 was still running.
- On `9e9d643d`, Process inventory/regression, HTTPD security, watchdog and iNIC signal checks passed; Wireless #44744529117 and AI #44744529015 failures were fixed by moving MII helpers to file scope and restoring mandatory handoff structure. XY-C1 #44744529158 remained in progress at last query.

## Active task

- Follow-up review found enable assumed the RTL8367 iNIC port was already isolated before powering the radio. This is unsafe after boot or an out-of-band switch-state change. This commit explicitly isolates first, only then enables the driver, and unisolates only after startup/TxPower setup; a failed initial isolation sends best-effort radio-off and publishes no state.
- After this commit, inspect Wireless and AI exact-head checks, plus NVRAM #44744845007 and XY-C1 #44744529158. The latter is for product-source SHA `9e9d643d`; do not transfer L3 to later production source changes.
- Continue an independent P1 slice while any exact-head jobs remain pending.

## Verification boundary

- The original host transition-policy harness passed GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. The actual repository Wireless source/policy regression and AI documentation contract both pass on `9075dc6385307f5df83ce9d13039462648395583`.
- This follow-up adds an additional initial-isolation-failure scenario and strengthens the enable sequence; its exact-head compile/regression is pending.
- XY-C1 #44744529158 was still running for product-source SHA `9e9d643d`; no L3 claim until detector/build/TRX/artifact pass on that exact source SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect Wireless and AI docs exact-head Actions for this follow-up; confirm the new initial-isolation test compiles and executes.
2. Inspect still-pending NVRAM #44744845007 on `9075dc` and XY-C1 #44744529158 on product source `9e9d643d`; capture detector/build/TRX/artifact results without SHA transfer.
3. Continue the next independent userland stability/security fix; do not repeat closed audits.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect the live repository, exact commit SHA, relevant exact-head Actions and the frozen backup status. Update these files only when a fact, active task, CI result or next action materially changes; if still accurate, do not create a no-op commit.

## Operating constraints

- Host unit tests, exact-SHA Actions, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
- Keep the newest 20–119 complete records in `dev_log.md`; batch archive oldest 100 only at the protocol threshold and keep the index/hash exact.
- Do not re-audit closed work without new evidence. Do not report a test, commit, build or device check that was not performed.
