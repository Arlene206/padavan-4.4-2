# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Before context exhaustion, inspect live main, exact-head Actions, validation boundaries, frozen backup and the concrete next task.
- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. The firmware-notification source slice was based on `9e3316f6740a77fd8db26d1f51af4183dd071021`; the current WAN firewall-status slice is based on parent main `a84174f9f5719272c1cc111c07be083790ae5734`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU-watchdog tri-state source `aeddc157`: GNU89 regression and XY-C1 Build/TRX/artifact PASS. USB modem tri-state `76683274`: dedicated readiness and Process/inventory/HTTPD/infosvr/handoff checks PASS. IPv6 boot-status source `37e414f2`: same-source XY-C1 Build/TRX/artifact PASS in `44003333586`; Process/handoff and dependent checks PASS on `b447358e`.
- Firewall module-table UNKNOWN/invalid rollback source `736ebe86`; Firewall Startup Stability `44005295947` PASS. HTTPD checker correction `49f79883` and firmware-identical XY-C1 `44005782493` passed Build/TRX/artifact.
- DNS Forwarder checked-stop source `8b63b6ea`: Process `44006695435`, launch regression `44006695180`, and same-source XY-C1 `44006695535` passed through Build/TRX/artifact.
- RWFS mount/Optware fix `5f1d87f6` and Ralink IAPP checked-shutdown hook are in descendant `f5a42763`; same-source XY-C1 `44009838585` passed Build/TRX/artifact, storage hotplug `44009838525` passed, and Process `44009960277` passed on descendant `b95bb735`.
- GPIO graceful shutdown registration source `e48aca6b`: GPIO regression `44013229820`, Process inventory `44013229755`, Process lifecycle `44013229793` PASS; same-source XY-C1 `44013905031` on `e5d3cdfd` passed Build XY-C1, TRX verification and artifact upload.
- DDNSTO launcher source `574caf1b`: validates the ROMFS executable and nonempty NVRAM identity before stopping an existing daemon, serializes lifecycle actions and requires validated RUNNING within five bounded polls. Exact DDNSTO run `44015005394` on `4c499369` PASS including shell syntax, lifecycle regression and semantic checker; same-source XY-C1 `44013905031` PASS through Build/TRX/artifact.
- HTTPD stop contract was updated to include the four checked daemons including `gpio_btn`; HTTPD `44015242107` on exact main `c5fc9136` PASS. Current c5 baseline Process inventory `44015242052`, AI docs `44015241932`, infosvr `44015241959`, AliyunDrive `44015241928`, AliDDNS `44015242005`, and AdGuardHome `44015241955` PASS. NVRAM `44015242074` and XY-C1 `44015241908` were pending at last poll.
- THIS COMMIT fixes a P1 PID 1 notification defect: `flash_firmware()` previously returned `void` and its notification handler ignored every early-abort and firmware-write status, allowing a failed update to lose its incomplete marker. It now propagates failures into `action_result`; Process/XY-C1 exact-head verification is pending.
- P1 WAN firewall caller-status defect fixed in THIS COMMIT: three direct `start_firewall_ex()` results and one PPP-recovery rebuild result previously disappeared; `net_wan.c` now captures and merges all four paths and returns PPP recovery UPnP/firewall errors.
- Local checkout/toolchain is unavailable; local execution is NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Verify exact-head WAN-route, Process lifecycle, AI handoff and XY-C1 Actions for the WAN firewall caller-status change in THIS COMMIT. Confirm the new contract passes and all four `start_firewall_ex()` results plus all PPP recovery failures reach `start_wan()`.
2. Require same-SHA XY-C1 Build, TRX verification and artifact upload before claiming L3. The older XY-C1 run `44017512514` on `97bd094c` passed Build/TRX/artifact, but predates this WAN source change.
3. If Actions are pending, continue another independent P1 RC startup/status defect; do not repeat already verified DDNSTO, GPIO, HTTPD, storage or completed network slices.

## Verification boundary

- Parent main SHA for THIS WAN firewall-status source change: `a84174f9f5719272c1cc111c07be083790ae5734`. The implementation captures the three primary WAN firewall rebuild statuses in `start_wan()`, converts `restore_wan_after_ppp_start_failure()` to a status-returning helper, and aggregates all three recovery callers.
- New source-contract checker `scripts/check_wan_firewall_status.py` runs in both WAN-route status and XY-C1 firmware workflows; both path filters include it and `net_wan.c`.
- Exact-head Process inventory `44017924214` on `a84174f9` PASS, including RC integration and LAN startup status checks. AI handoff contract `44017924265` on the same SHA PASS.
- XY-C1 run `44017512514` on `97bd094c` completed SUCCESS through Build XY-C1, TRX verification and artifact upload. Commit `a84174f9` changed only checker/roadmap/handoff/log files, not firmware sources; nevertheless, that is not an exact-head a841 run and does not verify THIS new WAN source change.
- Current WAN-route, Process, handoff and XY-C1 checks for THIS COMMIT are PENDING until their exact-head runs complete. No same-SHA L3 claim yet.
- Local checkout/toolchain unavailable; local execution NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect exact-head WAN-route, Process inventory/lifecycle, AI handoff and XY-C1 runs for the WAN firewall-status commit whose parent is `a84174f9f5719272c1cc111c07be083790ae5734`. Resolve any failed contract; require the same resulting source SHA to pass XY-C1 Build, TRX verification and artifact upload. If checks remain pending, continue independent P1 RC startup-result work. Before context exhaustion, update this handoff with the exact source commit SHA, test outcomes and current workflow IDs. Keep the frozen backup unchanged; use one complete Git tree and a non-forced expected-SHA update for every atomic multi-file commit.
