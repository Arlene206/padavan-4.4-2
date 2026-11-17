# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current main: THIS COMMIT; parent `8b950c04a80b88e475b670f02510bf67aac70db4`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- P1 WAN-up DDNS fix `01bc69f3`: configured Inadyn inventory/start/config-publication/HUP failures flow into WAN-up's first-error status; DDNS-disabled and AP-mode remain successful no-ops.
- Integration repair `0c786821` fixed WAN test include paths and updated MiniUPnD/HTTPD PPP rollback contracts to the checked status-returning implementation. Its WAN route, MiniUPnD, HTTPD, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS tests passed. The AI handoff contract passed on descendant `06ed4321`.
- XY-C1 for source baseline `0c786821` completed exact-head Build XY-C1, TRX output verification and artifact upload successfully in run `44026236613`; do not treat this artifact as verification of later source changes.
- P1 ZeroTier slice `d2a41360`: WAN-up now uses the existing enabled/disabled-aware `start_zerotier()` wrapper rather than dropping the result of `doSystem()`. It merges/logs the launch failure. The shell regression injects a launcher failure; ZeroTier lifecycle `44027996550` and Process lifecycle `44027996499` passed. Related DNSMasq `44027996482`, VPN server `44027996498`, WAN DHCP `44027996570`, WAN ARP `44027996951`, MiniUPnD `44027996455`, HTTPD `44027996404`, infosvr `44027996430`, AI handoff `44027996394`, teardown `44027996402`, firewall `44027996280`, detect_link `44027996330`, detect_internet `44027996257`, AdGuardHome `44027996230`, AliDDNS `44027996464`, AliyunDrive `44027996377`, and process inventory `44027996381` passed on exact SHA `d2a41360`.
- The WAN-route run `44027994137` on `d2a41360` failed before creating a job because pull_request path patterns were placed outside `on.pull_request.paths`; commit `42097d6c` repaired the filter, and exact-head WAN run `44028469558` passed.
- Exact-head `42097d6c` push Actions all completed SUCCESS: WAN route `44028469558`, AI handoff docs `44028469593`, Process lifecycle `44028469544`, NVRAM `44028469453`, AdGuardHome `44028469448`, AliDDNS `44028469580`, AliyunDrive `44028469536`, infosvr `44028469559`. The WAN workflow now creates and runs its job; no zero-job YAML failure remains.
- THIS COMMIT also fixes the symmetric WAN-down defect: `wan_down_internal()` now captures `eval("/usr/bin/detect.sh")`, accumulates/logs its status like WAN-up, and returns the accumulated result through `wan_down()`. `scripts/check_pppoe_mwan_status.py` asserts both up/down gates, call ordering, diagnostics and wrapper returns; WAN CI runs it. Current commit's exact-head Actions and XY-C1 workflows are pending.
- P1 PPPoE-MWAN detector slice `4f32e40f`: `detect.sh` preserves the first command failure, safely parses `iptables-save` without `eval`, validates NVRAM/IPv4 inputs, bounds loop counts and owns atomic lock cleanup. Its local Bash syntax/mock regression passed; exact-head WAN run `44037585283` also passed, including the production test step. Same-SHA XY-C1 run `44037585221` was still building; TRX and artifact were pending at last poll.
- XY-C1 run `44027996318` for source SHA `d2a41360` completed SUCCESS through Build XY-C1, TRX output verification and artifact upload (artifact `14304605616`, 12.64 MB; SHA-256 `05e374b72ce3f88eb6a019f50c2b1a85c924e28faaae772103bdd102a1485e8f`). This does not verify the newer PPPoE-MWAN source in THIS COMMIT.
- No full local repository checkout or MIPS toolchain is available. The PPPoE-MWAN host Bash syntax and mock-command regression were run locally and passed. Physical-router L4 remains NOT VERIFIED.
- Parent main `5525430a2ee5ff16f3d5b84912cd183e195808df` had 22/22 exact-head `push` Actions successful before its descendant source slices. WAN route run `44039076856` passed; XY-C1 run `44039076948` completed Build XY-C1, TRX output verification and artifact upload successfully.
- Gateway ARP checker launch failure propagation landed in parent `ac9b8936`: after a clean ABSENT process scan, `eval("detect_wan")` is captured, aggregated and logged. A later poll found 21/22 exact-head push runs SUCCESS with no failures; XY-C1 `44041394404` was still building.
- Parent `9722f273` implemented UNKNOWN inventory error propagation; its WAN route semantic-check ordering issue was repaired by `d20696fc`, whose exact-head selected push Actions then completed 8/8 SUCCESS. Physical WAN initial-MTU failure propagation landed in `bb427148`; its WAN route `44043948927`, Process lifecycle `44043948999`, AI handoff `44043949051`, WAN teardown `44043949040`, ZeroTier `44043949034`, WAN ARP probe `44043949028`, and several other stability/security jobs passed. WAN DHCP callback run `44043948977` on source `bb427148` still failed after two checker-only descendants (`038b2aa9`, `8b950c04`). The actual line 779 failure was a stale physical-setup marker/log assertion from before the MTU change, not the static-branch slice. THIS COMMIT updates those assertions to the new MTU policy/status flow. XY-C1 `44043949016` for source `bb427148` was still building at last poll; TRX/artifact pending.

## Active task

1. Verify WAN DHCP callback exact-head run with the corrected physical-setup contract; verify Process inventory and inspect XY-C1 `44043949016` for source `bb427148` through Build/TRX/artifact.
2. Continue a bounded WISP migration audit; the down-only fallback intentionally prevents traffic but does not remove an IPv4 address if `SIOCSIFADDR` fails. Trace whether the old interface can later be reused before deciding whether this warrants a fail-closed source change.
3. Work only on `main`, preserve the frozen backup, and publish each logical multi-file slice as one tree/commit with a non-forced expected-SHA update.

## Verification boundary

- Production-source commit `bb427148c3367bdb69139ca493caf6e2ffe193ed`: WAN route `44043948927` PASS including initial-MTU contract; Process lifecycle `44043948999` PASS; AI handoff `44043949051` PASS. XY-C1 `44043949016` was still in progress at last poll; product Build/TRX/artifact not yet confirmed.
- Checker-only descendants `038b2aa9` and `8b950c04` attempted to simplify a later static branch boundary but exposed the actual failure still at inline source-checker line 779: its physical setup marker and old MTU diagnostic expectations were stale. THIS COMMIT aligns them to the new code path.
- The current WAN DHCP callback rerun, Process inventory, and other current-head jobs must be checked against THIS COMMIT's exact SHA.
- No full local repository checkout or MIPS toolchain is available. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, then filter push Actions by its exact `head_sha`. Verify the repaired WAN DHCP callback job and exact-head WAN route/Process/AI jobs; inspect XY-C1 `44043949016` for Build, TRX and artifact completion. Then trace `config_apcli_wisp()` old-interface IPv4-clear fallback before any source change.
