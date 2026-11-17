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

1. Verify exact-head Actions for the WAN firewall-status implementation `9a0256f5d4cd97a902e48e32839fde7e1391d370` and this test-contract correction. The first implementation run exposed two checker-only failures; confirm both updated Python contracts pass on the new SHA.
2. Require same-SHA XY-C1 Build, TRX verification and artifact upload after the Python gates. The 9a XY-C1 job stopped before Build because the shared modem/WAN checker still expected three, not four, modem status aggregations.
3. If Actions remain pending, continue independent P1 RC startup-result work; avoid repeating completed lifecycle/network slices.

## Verification boundary

- WAN production-source commit `9a0256f5d4cd97a902e48e32839fde7e1391d370` (parent `a84174f9f5719272c1cc111c07be083790ae5734`) captures all four `start_firewall_ex()` returns and all three PPP-recovery results in `start_wan()`.
- Exact-head `9a0256f5` Actions: WAN-route `44019982102` FAIL at the new checker because its regex was over-escaped; Process `44019982105`, XY-C1 `44019982156` and Firewall Startup `44019982218` FAIL in existing `check_modem_wan_start_status.py:308`, whose expected aggregation count was still 3 after the new fourth RAS firewall result. AI handoff `44019982167` PASS. These are source-contract failures; the XY build step was not reached, so no new product build claim exists.
- Corrective test-only work in THIS COMMIT: replace the fragile regexes in `check_wan_firewall_status.py` with explicit source-position checks, and extend `check_modem_wan_start_status.py` to assert four modem status merges and specifically cover the new RAS firewall result.
- Earlier baseline: exact-head Process `44017924214` and AI docs `44017924265` on `a84174f9` PASS. XY-C1 `44017512514` on `97bd094c` passed Build/TRX/artifact; it predates WAN source commit `9a0256f5`.
- Exact-head WAN-route, Process, firewall, AI docs and XY-C1 reruns for the corrective commit are PENDING until Actions execute. No same-SHA L3 claim yet.
- Local checkout/toolchain unavailable; local execution NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect exact-head WAN-route, Process inventory/lifecycle, Firewall Startup Stability, AI handoff and XY-C1 runs for the checker-correction commit whose parent is `9a0256f5d4cd97a902e48e32839fde7e1391d370`. Confirm the WAN firewall caller contract and the shared modem/WAN first-error integration contract both pass; then require the same SHA to complete XY-C1 Build, TRX verification and artifact upload. If Actions remain pending, continue independent P1 RC startup-result work. Before context exhaustion, update this handoff with the correction commit SHA, test outcomes and current workflow IDs. Keep the frozen backup unchanged and publish multi-file changes as one tree via non-forced expected-SHA update.
