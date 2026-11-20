# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only engineering across conversations. Use one atomic main commit per logical source slice; create all blobs and one complete tree, then update main using non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regressions and diagnose failed exact-SHA checks from their causal logs.
- Before context exhaustion, finish the current safe slice, inspect exact-head Actions, and leave an accurate first action for the next GPT.

## Current repository state

- Current main before this RT3352 TxPower status slice: `31de47640faed47555e830d86cd69f7228887d7e`; the boot-state isolation production source was introduced in `d250aff4e337f5c919e11c44cf5a1e8e5c1cb5a8`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Source implementation commit `9e9d643d` and follow-up test/docs SHA `9075dc63` passed Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149; NVRAM #44744845007 passed on `9075dc63`.
- Boot-state isolation follow-up `d250aff4` changed the policy to assert isolation before RadioOn; its six-event rollback test count was corrected on `31de4764`. Wireless #44746039419, AI #44746039183 and Process inventory #44746039325 passed on `31de4764`; NVRAM #44746039252 was in progress. Product-source XY-C1 #44745705732 on `d250aff4` remained in progress; NVRAM #44745706021 passed.

## Active task

- Follow-up review found enable assumed the RTL8367 iNIC port was already isolated before powering the radio. This is unsafe after boot or an out-of-band switch-state change. This commit explicitly isolates first, only then enables the driver, and unisolates only after startup/TxPower setup; a failed initial isolation sends best-effort radio-off and publishes no state.
- After this test correction, inspect Wireless and AI exact-head reruns; check NVRAM #44745706021 and XY-C1 #44745705732 on `d250aff4`. The XY-C1 run is required to establish L3 for the isolation-before-power production source; do not transfer results across later product-source SHAs.
- Continue an independent P1 slice while any exact-head jobs remain pending.

## Verification boundary

- Wireless regression passed on `31de4764` after the six-event expectation correction; AI handoff and Process inventory also passed. This slice adds a required TxPower-success gate and regression; its exact-head compile/test is pending.
- XY-C1 #44745705732 for product source `d250aff4` remained in progress at last check; NVRAM #44745706021 passed there. This production-source change requires its own exact-SHA XY-C1 run. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect Wireless and AI docs exact-head Actions for this TxPower failure gate; confirm rollback does not unisolate or publish state.
2. Inspect NVRAM #44746039252 and the new same-SHA XY-C1 detector/build/TRX/artifact; do not transfer L3 from `d250aff4`.
3. Continue an independent stability/security fix while pending checks run.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect the live repository, exact commit SHA, relevant exact-head Actions and the frozen backup status. Update these files only when a fact, active task, CI result or next action materially changes; if still accurate, do not create a no-op commit.

## Operating constraints

- Host unit tests, exact-SHA Actions, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
- Keep the newest 20–119 complete records in `dev_log.md`; batch archive oldest 100 only at the protocol threshold and keep the index/hash exact.
- Do not re-audit closed work without new evidence. Do not report a test, commit, build or device check that was not performed.
