# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

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

1. Inspect exact-head Process lifecycle and XY-C1 firmware Actions for THIS firmware-update notification status fix. Require the GNU89 policy regression, source contract and full XY-C1 Build/TRX/artifact to pass on this exact source SHA.
2. Resolve the first causal CI failure; while Actions are pending, continue an independent P1 RC lifecycle/status defect rather than waiting.
3. Do not repeat DDNSTO, GPIO shutdown, HTTPD stop_misc, watchdog, modem, IPv6, firewall, DNS Forwarder, RWFS, IAPP, USB storage or VPN slices already verified above.

## Verification boundary

- DDNSTO L2: exact DDNSTO `44015005394` on `4c499369` PASS. DDNSTO/GPIO L3: exact XY-C1 `44013905031` on product source `e5d3cdfd` PASS: Build, TRX and artifact upload.
- HTTPD stop-count source contract: exact HTTPD `44015242107` on `c5fc9136` PASS.
- Current parent baseline `c5fc9136`: Process inventory `44015242052`, AI docs `44015241932`, infosvr `44015241959`, AliyunDrive `44015241928`, AliDDNS `44015242005` and AdGuardHome `44015241955` PASS. NVRAM `44015242074` and XY-C1 `44015241908` remained pending at last poll.
- Earlier watchdog, modem, IPv6, firewall, DNS Forwarder, RWFS, IAPP and USB storage results remain source-specific as recorded above. Local tests NOT RUN. Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect exact-head Process and XY-C1 Actions for THIS commit. Confirm the notification policy binary/source contract and the same-SHA firmware Build XY-C1, TRX verification and artifact upload independently. Fix the first causal failure; if pending, continue a separate P1 RC lifecycle slice. Keep the backup frozen and use complete-tree atomic commits with non-forced expected-SHA compare-and-swap.
