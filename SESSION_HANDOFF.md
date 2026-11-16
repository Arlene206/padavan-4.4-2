# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Source parent before THIS COMMIT: `b61ca225bc996bbace4000a7aa5aa6972f9cf5db`. It includes NDIS control-result propagation: QMI, Huawei NCM, CDC-NCM and Sierra control commands are marked required; failures stop WAN startup before DHCP. Optional no-control paths preserve their existing behavior.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Exact-head USB modem identity regression `43921625498` completed SUCCESS, including GNU89 modem tests and source checks.
- XY-C1 run `43921624845` failed only at checker execution line 48: literal full NDIS function signature did not match its line-wrapped C definition. The earlier GNU89 modem WAN policy regression passed. THIS COMMIT makes the checker use a stable prefix; XY-C1 must be rerun.
- Process inventory/fail-closed run `43921624682` and WAN teardown run `43921624637` also reported failure on the parent SHA and need independent log diagnosis. NVRAM run `43921624686` was IN_PROGRESS at last poll. Other same-SHA successful runs include HTTPD `43921624642`, AI docs `43921624718`, infosvr `43921624706`, AliDDNS `43921624768`, AliyunDrive `43921624837`, AdGuardHome `43921624773`, and Process lifecycle inventory `43921624681`.

## Active task

THIS COMMIT is a narrow test infrastructure repair: the semantic source checker now identifies the multiline `ndis_control_network()` definition using its stable function-name prefix, instead of a full prototype string that cannot match a line break. No production code changed.

## Verification boundary

- Parent source SHA `b61ca225bc996bbace4000a7aa5aa6972f9cf5db`: USB modem identity regression `43921625498` PASS; XY-C1 `43921624845` FAILED at the checker signature extraction after the modem policy C test passed. The XY build/TRX/artifact stages were not reached.
- THIS COMMIT triggers another exact-head XY-C1 validation. Do not claim firmware L3 until the semantic checker, Build XY-C1, TRX verification and artifact upload all succeed on the same source SHA.
- Physical-router L4 remains NOT VERIFIED. Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect this commit's exact-head XY-C1 job; confirm `check_modem_wan_start_status.py` advances past the NDIS function extraction and all modem policy/source-contract tests pass, then inspect build/TRX/artifact steps.
2. Diagnose exact failed steps and logs for WAN teardown `43921624637` and Process inventory/fail-closed `43921624682`; apply only confirmed independent defects.
3. Recheck NVRAM audit `43921624686`; if pending, continue independent implementation rather than waiting.
4. Keep the frozen backup read-only and physical-router L4 NOT VERIFIED.
