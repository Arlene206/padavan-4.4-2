# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, exact-head Actions, validation boundary and next concrete task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU-watchdog tri-state source `aeddc157` passed GNU89 regression and XY-C1 Build/TRX/artifact (`44000632827`). USB modem tri-state source `76683274` passed its dedicated readiness and Process/inventory/HTTPD/infosvr/handoff checks. IPv6 boot status propagation source `37e414f2` passed same-source XY-C1 Build/TRX/artifact in run `44003333586`; Process/handoff and dependent checks passed on `b447358e`.
- Firewall module-table UNKNOWN/invalid-state rollback fix is source `736ebe86`; Firewall Startup Stability `44005295947` passed. The stale HTTPD Boolean-probe checker was corrected in `49f79883`; HTTPD `44005782392` and firmware-identical XY-C1 `44005782493` passed, including Build/TRX/artifact.

- This source slice fixes an RC shutdown omission: `start_services_once()` starts DNS Forwarder under `APP_DNSFORWARDER`, but `stop_services()` did not register its existing checked stop wrapper. The omission could leave DNS Forwarder alive during reboot/halt or firmware update. The fix adds it to the common first-error stop accumulator; this feeds both shutdown storage-persistence and firmware-update abort gates. A source integration regression pins the start/stop guards and both caller gates.
- P1 RWFS boot-mount/Optware readiness fix product-source commit `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`: `mount_rwfs_partition()` previously discarded UBI module/attach/mkdir/UBIFS mount failures, while Optware start treated directory existence as mount readiness. The new flow verifies tri-state UBI module readiness and the exact MTD attachment before/after attach, validates the mountpoint, propagates its status into PID1 boot results and gates the Optware worker on successful mount; a newly created empty directory is cleaned up on failure. Dedicated GNU89 regression and Process/XY-C1 integration are wired; exact-head Actions pending.
- DNS Forwarder source `8b63b6ea70536e2677204e6da8741a662145ba5f` and handoff follow-up `db73892db0692050eca7249e5ac55d4eb35bb3b9` are verified; same-source XY-C1 run `44006695535` passed Build/TRX/artifact.
- The RWFS product source is `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`; its GNU89 policy + focused source checker PASS. Process umbrella `44009960277` passed on `b95bb735`. The RWFS + Ralink IAPP production-source descendant `f5a42763` passed storage hotplug `44009838525` and same-source XY-C1 `44009838585` completed SUCCESS including Build XY-C1, TRX verification and artifact upload.
- Process lifecycle `44009960277` on main `b95bb735` PASS, including RWFS and the Ralink IAPP shutdown result wiring. AI handoff `44009960341` PASS. NVRAM transaction audit `44009960343` remains in progress; its result has not been claimed.
- Physical-router L4 remains NOT VERIFIED.

- GPIO button daemon lifecycle: `start_gpio_btn()` had a SIGTERM-aware loop but `stop_misc()` omitted `gpio_btn`. THIS COMMIT adds it to the common graceful stop list and extends the GPIO checker to pin the SIGTERM handler and shutdown persistence gate; exact-head GPIO/Process runs pending.

- DDNSTO production source `574caf1b` validates executable/NVRAM identity before stopping a running instance and requires validated RUNNING within five bounded polls. Exact DDNSTO `44013229590` on `e48aca6b` passed shell syntax and the full production-linked lifecycle regression; the Python checker alone failed because it compared the stop gate with the first `$DDNSTO_BIN` reference (the preflight), not the later launch invocation, and expected the obsolete `echo $` test fixture rather than `echo $$`. THIS COMMIT corrects those checks and adds the packaged launcher to the XY-C1 trigger.
- GPIO button stop registration was added in `e48aca6b`; GPIO regression `44013229820` PASS. Same-SHA Process inventory `44013229755` and Process lifecycle `44013229793` PASS; XY-C1 `44013229689` was pending. NVRAM `44013229734` was still running at last poll; HTTPD umbrella `44013229543` failed pending log inspection.

- DDNSTO exact run `44013905030` on `e5d3cdfd` passed shell syntax and all shell lifecycle regression cases, then failed only in `check_ddnsto_lifecycle.py`: the checker still searched for the obsolete literal `echo $`, but current test fixtures write the numeric mock PID as `echo $$`. THIS COMMIT fixes that test-contract string. XY-C1 run `44013905031` was still running for the same SHA. Other exact-head checks: Process inventory `44013905279`, AI docs `44013904997`, infosvr `44013904956`, AliyunDrive `44013905273`, AdGuardHome `44013905011`, and AliDDNS `44013904971` PASS; NVRAM `44013905187` pending.

- DDNSTO exact run `44013905030` on `e5d3cdfd` completed shell syntax and all production-linked lifecycle shell cases successfully. The only failure was a stale Python checker fixture expectation `echo $`; the actual fake daemon writes a numeric PID with `echo $$`. THIS COMMIT corrects that final assertion. Same-SHA XY-C1 `44013905031` remains in Build XY-C1; NVRAM `44013905187` PASS.

## Active task

1. Inspect exact-head DDNSTO regression after THIS numeric PID checker fix; require shell tests and all semantic source/workflow contracts PASS.
2. Verify XY-C1 run `44013905031` against the same production source through Build XY-C1, TRX verification and artifact upload.
3. Fix HTTPD umbrella checker failure `44013229543`: its old `stop_misc()` assertion expects three processes, but production now correctly stops four, including `gpio_btn`. Re-run HTTPD after updating the source contract.


## Verification boundary

- DDNSTO production source `574caf1b`: complete shell lifecycle tests PASS on `6e997ddb`, `e48aca6b` and `e5d3cdfd`. Prior semantic failures were stale launch-order and PID fixture assertions. Launch-order/XY-C1 trigger correction is in `e5d3cdfd`; THIS COMMIT fixes the last `echo $` expectation. Exact-head DDNSTO rerun pending.
- Same-source XY-C1 run `44013905031` on `e5d3cdfd` was still at Build XY-C1; no Build/TRX/artifact PASS claim until all three steps finish. NVRAM audit `44013905187` PASS.
- On `3c2ee531`: Process inventory `44014408639`, NVRAM audit `44014408877`, AI handoff `44014408656`, infosvr `44014408753`, AliyunDrive `44014408640`, AliDDNS `44014408747`, and AdGuardHome `44014408744` PASS.
- HTTPD umbrella `44013229543` failed a stale assertion expecting `return kill_services(svcs, 3, 1);`; the actual `stop_misc()` array includes `gpio_btn` and correctly calls `kill_services(svcs, 4, 1);`. This is a checker mismatch, not a production shutdown defect.
- Local checkout/toolchain unavailable; local execution NOT RUN. Physical-router L4 remains NOT VERIFIED.


## Exact first action for the next GPT

Fetch live `main` and inspect exact-head DDNSTO result after the PID checker correction. Verify the current same-source XY-C1 Build, TRX verification and artifact upload individually. Then update the HTTPD workflow's stale `stop_misc()` assertion to require the four production daemon entries including `gpio_btn`, rerun HTTPD, and continue independent P1 RC work while Actions are pending. Keep backup frozen and use atomic complete-tree commits with non-forced expected-SHA compare-and-swap.
