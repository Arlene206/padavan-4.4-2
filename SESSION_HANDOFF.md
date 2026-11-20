# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only engineering across conversations. Use one atomic main commit per logical source slice; create all blobs and one complete tree, then update main using non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regressions and diagnose failed exact-SHA checks from their causal logs.
- Before context exhaustion, finish the current safe slice, inspect exact-head Actions, and leave an accurate first action for the next GPT.

## Current repository state

- Current main before this test correction: `d250aff4e337f5c919e11c44cf5a1e8e5c1cb5a8`; the preceding RT3352 runtime source parent is `9e9d643dd8c3a78d135c8dae908857f8b90c0276`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Source implementation commit `9e9d643d` and follow-up test/docs SHA `9075dc63` passed Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149; NVRAM #44744845007 passed on `9075dc63`.
- Boot-state isolation follow-up `d250aff4` changed the policy to assert isolation before RadioOn. Its Wireless run #44745705779 compiled successfully but failed the C regression because the unisolation-rollback case expected five recorded events although its expected sequence contains six. This commit corrects that test count. AI handoff #44745705703 and Process inventory #44745705762 passed; NVRAM #44745706021 and XY-C1 #44745705732 were in progress at last observation.

## Active task

- Follow-up review found enable assumed the RTL8367 iNIC port was already isolated before powering the radio. This is unsafe after boot or an out-of-band switch-state change. This commit explicitly isolates first, only then enables the driver, and unisolates only after startup/TxPower setup; a failed initial isolation sends best-effort radio-off and publishes no state.
- After this test correction, inspect Wireless and AI exact-head reruns; check NVRAM #44745706021 and XY-C1 #44745705732 on `d250aff4`. The XY-C1 run is required to establish L3 for the isolation-before-power production source; do not transfer results across later product-source SHAs.
- Continue an independent P1 slice while any exact-head jobs remain pending.

## Verification boundary

- Wireless source/policy tests and AI documentation contract passed on `9075dc63`. On `d250aff4`, the test program compiled, but its execution failed only because the unisolation-failure event count was set to 5 instead of 6; the six expected events are isolate, radio-on, TxPower preparation, unisolate attempt, rollback isolate and radio-off. This commit corrects the count; exact-head rerun pending.
- XY-C1 #44745705732 and NVRAM #44745706021 were still running for `d250aff4`; no L3 claim until detector/build/TRX/artifact all pass on the exact production-source SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect Wireless and AI docs exact-head Actions for this correction; ensure the full six-event rollback expectation passes.
2. Inspect XY-C1 #44745705732 and NVRAM #44745706021 for `d250aff4`; record detector/build/TRX/artifact results only on that production-source SHA.
3. Continue the next independent stability/security task; do not repeat closed audits.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect the live repository, exact commit SHA, relevant exact-head Actions and the frozen backup status. Update these files only when a fact, active task, CI result or next action materially changes; if still accurate, do not create a no-op commit.

## Operating constraints

- Host unit tests, exact-SHA Actions, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
- Keep the newest 20–119 complete records in `dev_log.md`; batch archive oldest 100 only at the protocol threshold and keep the index/hash exact.
- Do not re-audit closed work without new evidence. Do not report a test, commit, build or device check that was not performed.
