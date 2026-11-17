# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current main: `0c786821fd1bb8b571c51c1c7abf32780f481157`, parent `01bc69f31c7a40591587e095f8be8d634b43ef23`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Parent `9116f521` had 11 exact-head push workflows completed SUCCESS. XY-C1 run `44020469451` reached and passed cross-build, Build XY-C1, TRX verification and artifact upload. Parent Process lifecycle `44020469301`, WAN route `44020469586`, and Firewall Startup `44020469423` also passed.
- Current source commit `01bc69f3` fixes a P1 WAN-up status defect: `wan_up_internal()` previously discarded `notify_ddns_update()`. Configured Inadyn inventory/start/configuration/HUP failures now flow into WAN-up's first-error status. DDNS-disabled and AP-mode configurations are explicit successful no-ops before process probing or start.
- Coverage adds four enable/AP configuration cases to `inadyn_daemon_policy.c`, a WAN caller contract checker, and exact workflow gates.
- Exact-head run batch for `01bc69f3`: 23 workflows; 13 SUCCESS, 5 FAILURE and 5 still pending/in-progress at that poll. The descendant `0c786821` re-ran 11 relevant workflows: WAN route, MiniUPnPd, HTTPD security, infosvr packets, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS completed SUCCESS; AI handoff failed only because this checkpoint lacked the literal `Before context exhaustion` phrase; XY-C1 remained in progress at the last poll. No XY-C1 firmware success is claimed until Build, TRX and artifact upload all complete on one exact SHA.
- Local checkout/toolchain is unavailable; local compilation was NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Complete the five exact-SHA integration fixes identified below: add the shared include path to the Inadyn regression compile; restore the mandatory handoff checkpoint and verification boundary; update the PPP rollback source contracts to the current status-returning implementations; and preserve those changes as one logical test/handoff commit.
2. Inspect every run for the descendant's exact `head_sha`. Resolve any FAIL on that SHA and require the WAN route, Process lifecycle and XY-C1 workflows to pass. For XY-C1, require the actual Build XY-C1, TRX verification and artifact-upload steps.
3. When checks are pending, continue independent P1 source work rather than waiting. The next source area is the remaining WAN-up optional launches (ZeroTier and PPPoE-MWAN `detect.sh`), but first verify their enabled/disabled return semantics.
4. Keep the backup unchanged and use one complete Git tree, one commit and non-forced expected-SHA main update per logical slice.

## Verification boundary

- Exact main SHA inspected: `01bc69f31c7a40591587e095f8be8d634b43ef23`; live main matched this SHA when the 23-run Actions batch was retrieved.
- Exact-head run outcomes:
  - SUCCESS: detect_link `44025185383`; AliyunDrive `44025185263`; AdGuardHome `44025185238`; detect_internet `44025185168`; Samba `44025185163`; AliDDNS `44025185138`; WAN ARP `44025185116`; WAN DHCP callback `44025185110`; VPN server `44025185078`; USB modem identity `44025184984`; WAN teardown `44025184941`; Firewall Startup `44025184934`; Inadyn lifecycle `44025184921`.
  - FAILURE: WAN route `44025184940` — new Inadyn test compile omitted `-I trunk/user/shared/src`, so `process_inventory.h` was not found. This is a CI command defect, not a C source failure.
  - FAILURE: AI handoff `44025185056` and infosvr packet `44025184943` — the new handoff lacked the mandated checkpoint text and `## Verification boundary` heading; restore those required governance sections.
  - FAILURE: HTTPD security `44025185053` — PPP rollback contract still expected an unchecked direct `start_firewall_ex();` and void-style stop branch. Current `net_wan.c` correctly captures and returns `firewall_status` / `upnp_status`; update the stale source assertion.
  - FAILURE: MiniUPnPd config `44025185094` — test still expected direct `if (check_upnp_wanif_changed(...) != 0)` and bare `return;`. Current code captures `upnp_status`, returns that error before changing active WAN, and separately propagates `firewall_status`; update checker to match this stronger contract.
  - PENDING at last poll: XY-C1 `44025185181`; Process lifecycle `44025185227`; Process lifecycle inventory `44025185079`; DNSMasq `44025185052`; NVRAM transaction boundary `44025185036`. Job detail: Process lifecycle and DNSMasq jobs completed SUCCESS while workflow runs were still finishing; XY-C1 cross-build and NVRAM job were still in progress. Do not infer the full XY-C1 build or artifact outcome from the in-progress job.
- Corrective test/handoff commit `0c786821fd1bb8b571c51c1c7abf32780f481157` was created atomically on main. Exact-head WAN route `44026236589`, MiniUPnPd `44026236528`, HTTPD `44026236478`, infosvr `44026236468`, Process lifecycle `44026236421`, NVRAM `44026236411`, AliyunDrive `44026236509`, AdGuardHome `44026236508`, and AliDDNS `44026236477` all passed. AI docs `44026236466` failed solely for the missing literal checkpoint phrase, now being corrected. XY-C1 `44026236613` was still building when last polled; its cross-build job had passed every source regression gate and Build XY-C1 was running, with TRX verification and artifact upload still pending.
- No local compile/test was run in this environment. No new L3 firmware success is claimed for `01bc69f3`. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main, then filter Actions by the exact descendant SHA and event `push`. Resolve the AI handoff gate after adding the exact checkpoint phrase; confirm the new AI docs run passes. Recheck XY-C1 and require Build, TRX verification and artifact upload. If all are green, continue the WAN-up optional-launch result investigation (ZeroTier and PPPoE-MWAN detection); if pending, work on an independent source slice. Keep this handoff current and preserve the frozen backup.