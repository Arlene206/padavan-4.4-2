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

1. Recheck exact-head Process inventory and XY-C1 Actions for the firmware-update notification change. The first run exposed three CI contract/integration issues; this follow-up wires the same regression into Process and repairs the stale WAN-MTU assertion.
2. Require the GNU89 notification policy test and Python source contract to pass in both Process and XY-C1, then require same-SHA XY-C1 Build, TRX verification and artifact upload.
3. While CI is pending, continue an independent P1 RC lifecycle defect; do not repeat the already verified DDNSTO, GPIO shutdown or HTTPD stop-count work.

## Verification boundary

- Firmware-update notification production change `1d695e75`: all ten preparation abort paths and failed `mtd_write` now return failure; caller merges status into `action_result`, preserving the incomplete marker. GNU89 policy test passed in the 1d XY-C1 run before the source checker failed.
- Exact-head Process inventory run `44016786493` on `1d695e75` failed in the pre-existing modem/WAN boot-status checker at its obsolete static-WAN MTU log assertion. Live production source now logs `static WAN interface %s MTU setup failed (status=%d); skipping address and route setup`; THIS COMMIT aligns the checker.
- Exact-head XY-C1 run `44016786510` on `1d695e75` failed because the new source checker correctly required both workflows to include the policy test and Python contract, but the Process workflow hookup was missing. XY had compiled/run the GNU89 policy test; the Process workflow integration is added in THIS COMMIT.
- PID 1 notification queue regression `44016786729` and standalone Process lifecycle `44016786612` PASS on `1d695e75`. Exact-head AI handoff `44016786535` failed only because the required phrase `Before context exhaustion` was omitted; THIS COMMIT restores it.
- No same-SHA XY-C1 Build/TRX/artifact claim for `1d695e75`; firmware validation failed at the source contract. Local checkout/toolchain unavailable; local execution NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect exact-head Process inventory, Process lifecycle, AI handoff and XY-C1 Actions for THIS Process workflow/checker correction. Confirm the modem/WAN source contract and notification policy/source contract both pass. Require XY-C1 Build, TRX verification and artifact upload on the exact same source SHA before claiming L3. If Actions remain pending, continue a separate P1 RC lifecycle slice. Keep the backup frozen and use complete-tree atomic commits with non-forced expected-SHA compare-and-swap.
