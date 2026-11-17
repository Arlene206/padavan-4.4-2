# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Before context exhaustion, inspect live main, exact-head Actions, validation boundaries, frozen backup and the concrete next task.
- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. Parent main SHA for THIS change: `9e3316f6740a77fd8db26d1f51af4183dd071021`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU-watchdog tri-state source `aeddc157`: GNU89 regression and XY-C1 Build/TRX/artifact PASS. USB modem tri-state `76683274`: dedicated readiness and Process/inventory/HTTPD/infosvr/handoff checks PASS. IPv6 boot-status source `37e414f2`: same-source XY-C1 Build/TRX/artifact PASS in `44003333586`; Process/handoff and dependent checks PASS on `b447358e`.
- Firewall module-table UNKNOWN/invalid rollback source `736ebe86`; Firewall Startup Stability `44005295947` PASS. HTTPD checker correction `49f79883` and firmware-identical XY-C1 `44005782493` passed Build/TRX/artifact.
- DNS Forwarder checked-stop source `8b63b6ea`: Process `44006695435`, launch regression `44006695180`, and same-source XY-C1 `44006695535` passed through Build/TRX/artifact.
- RWFS mount/Optware fix `5f1d87f6` and Ralink IAPP checked-shutdown hook are in descendant `f5a42763`; same-source XY-C1 `44009838585` passed Build/TRX/artifact, storage hotplug `44009838525` passed, and Process `44009960277` passed on descendant `b95bb735`.
- GPIO graceful shutdown registration source `e48aca6b`: GPIO regression `44013229820`, Process inventory `44013229755`, Process lifecycle `44013229793` PASS; same-source XY-C1 `44013905031` on `e5d3cdfd` passed Build XY-C1, TRX verification and artifact upload.
- DDNSTO launcher source `574caf1b`: validates the ROMFS executable and nonempty NVRAM identity before stopping an existing daemon, serializes lifecycle actions and requires validated RUNNING within five bounded polls. Exact DDNSTO run `44015005394` on `4c499369` PASS including shell syntax, lifecycle regression and semantic checker; same-source XY-C1 `44013905031` PASS through Build/TRX/artifact.
- HTTPD stop contract was updated to include the four checked daemons including `gpio_btn`; HTTPD `44015242107` on exact main `c5fc9136` PASS. Current c5 baseline Process inventory `44015242052`, AI docs `44015241932`, infosvr `44015241959`, AliyunDrive `44015241928`, AliDDNS `44015242005`, and AdGuardHome `44015241955` PASS. NVRAM `44015242074` and XY-C1 `44015241908` were pending at last poll.
- THIS COMMIT fixes a P1 PID 1 notification defect: `flash_firmware()` previously returned `void` and its notification handler ignored every early-abort and firmware-write status, allowing a failed update to lose its incomplete marker. It now propagates failures into `action_result`; Process/XY-C1 exact-head verification is pending.
- Local checkout/toolchain is unavailable; local execution is NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Re-run exact-head Process inventory after updating the firmware-write contract in `check_process_lifecycle_integration.py`. Run `44017512634` reached the RC integration boundary and failed only because the checker still expected the pre-change inline `if (eval("/tmp/mtd_write"...))` call; production now stores the command status, restores the watchdog and returns failure.
2. Confirm exact-head XY-C1 completes the notification source contract, Build, TRX verification and artifact upload. At the last poll `44017512514` was still running on `97bd094c`.
3. Continue independent P1 RC lifecycle/status work while Actions run; do not repeat verified DDNSTO, GPIO, HTTPD or completed storage/network slices.

## Verification boundary

- Firmware-update notification source `1d695e75`: all ten preparation aborts and failed `mtd_write` return failure; the notification handler merges the result and preserves the incomplete marker. The GNU89 notification policy binary and dedicated source contract passed inside the Process run `44017512634`; the umbrella failed later at the old `check_process_lifecycle_integration.py:589` source-shape assertion.
- Process workflow path/step integration and modem/WAN checker correction are in `97bd094c`. Process inventory `44017512634` on `97bd094c` failed only because its integration checker still required the obsolete inline eval form; THIS COMMIT updates it to require the status variable, failure branch, errno preservation, watchdog recovery and failure return.
- AI handoff `44017512559` on `97bd094c` PASS. XY-C1 `44017512514` on `97bd094c` remained in progress at the last poll; no Build/TRX/artifact claim yet.
- DDNSTO L2 `44015005394` on `4c499369` PASS; same-source XY-C1 `44013905031` on product source `e5d3cdfd` PASS Build/TRX/artifact. GPIO shutdown and HTTPD stop-count validations remain verified at their exact source boundaries recorded above.
- Local checkout/toolchain unavailable; local execution NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect exact-head Process inventory and XY-C1 Actions for THIS integration-checker correction. Confirm the RC integration checker now accepts the status-capturing `mtd_write` path while retaining failure/errno/watchdog guarantees, and require same-SHA XY-C1 Build, TRX verification and artifact upload. If checks remain pending, continue a separate P1 RC lifecycle slice. Keep the backup frozen and use complete-tree atomic commits with non-forced expected-SHA compare-and-swap.
