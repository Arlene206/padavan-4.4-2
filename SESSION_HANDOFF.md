## 2026-11-28 continuation checkpoint

- Repository: `Arlene206/padavan-4.4-2`. Continue on `main`; startup recovery was completed once. Do not repeat the completed bootstrap/inventory audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No force-push, reset, rename, CI, or development work on this branch.
- Latest validated parent at this checkpoint: `41a9d0f0f87ce11e737ce8b7a404bca39467049b`; this follow-up makes passwd/passdb/config consume one copied account snapshot.
- PR #1 remains closed and unmerged. Its AliDDNS CA trust-store postcondition source, shell regression and workflow integration are present on main; do not resume development on the closed PR branch or assume its legacy branch is current.

## Exact-SHA validation already established

- DDNSTO production SHA `3f1cab20589e858b2db951b18821194249424957`: XY-C1 run #46141285573 completed SUCCESS; firmware Build, TRX verification and artifact upload all SUCCESS; artifact ID `15261971334`.
- Aria2 production SHA `76646cffc52d09fcb9b7a67052d5ae64b1b14462`: XY-C1 run #46141026720 completed SUCCESS; firmware Build, TRX verification and artifact upload all SUCCESS; artifact ID `15261729486`.
- PPPoE-MWAN detector commit SHA `300e78b2a36ceea51d508d18bc3cf09add6d633d`: WAN route-status run #46141847656 SUCCESS; XY-C1 run #46141847644 SUCCESS, including both PPPoE-MWAN shell tests, static checker, firmware Build, TRX and artifact upload; artifact ID `15262232177`.
- Firmware first-errno policy commit SHA `19d86adc5f57d17800a687435050b294affb6ade`: PID 1 notification run #46142248483 SUCCESS; Process lifecycle regression #46142248545 SUCCESS (including the first-errno policy test and firmware notification checker); Process inventory #46142248530 SUCCESS. XY-C1 #46142248550 remained IN_PROGRESS at last poll; its first-errno test and firmware notification checker completed SUCCESS; Build XY-C1 was still running.
- No local MIPS/kernel build or router hardware is available in this session. Physical L4 remains NOT VERIFIED. Do not claim L3 for a SHA until its exact-SHA Build, TRX check and artifact upload all succeed.

## Current repository state

- Main-only development. The frozen backup remains unchanged.
- `flash_firmware()` now routes all ten independent pre-flash stop operations through `firmware_update_stop_first_errno()`: VPN client, VPN server, residual L2TP/PPP process cleanup, VPN lifecycle-lock release, MiniUPnPd, checked service stop, EAPOL, LAN authentication, 802.1X and PPPoE relay.
- Samba account generation now uses one copied NVRAM generation across `/etc/passwd`, passdb and `smb.conf`; boot-time passwd generation itself reads all account keys under one transaction.
- Each operation clears errno before it runs; the first nonzero failure errno is retained, defaulting to EIO when a failing operation leaves errno unset. Independent stop attempts continue; failure is logged and errno restored before returning. Persistence/MTD write remains unreachable after any stop failure.
- The GNU89 policy regression covers successful operations with stale errno, operation errno capture, EIO fallback and sticky first error. The Process lifecycle and XY-C1 workflows compile/run it with sanitizers and execute the extended firmware-update source checker.
- Root `dev_log.md` has 98 records before this commit and is below archive thresholds; roadmap is near its size cap and was not edited.

## Active task

- Current source task: unify the NVRAM account generation used by Samba and `/etc/passwd`; validate exact-head Process/Samba workflows and XY-C1 build.

- Validate the ten-stop first-errno follow-up and its test-contract maintenance on the new exact main SHA. Require AI handoff docs, detect_internet focused contract, GNU89 policy test, source checker, Process lifecycle regression, PID 1 notification, and XY-C1 pre-build checks to pass.
- Complete exact-SHA Build, TRX and artifact upload before claiming L3 for any firmware-related SHA. The PPPoE-MWAN detector SHA 300e is already fully validated.

## Exact first action for the next GPT

1. Fetch live `main`, identify this test-contract follow-up's exact SHA, and inspect all exact-head Actions.
2. Confirm AI handoff contract, detect_internet signal workflow, Process lifecycle regression, PID 1 notification workflow and Process inventory pass; inspect failures by exact job log.
3. Inspect XY-C1 Build, TRX and artifact steps for SHA `805624195e6826f723c854afd2dabd7df891b50e` and the new follow-up. A pending build does not block independent work.
4. Poll #46142248550 (19d86 firmware first errno) and #46142520458 (805624 firmware build) independently; 300e PPPoE detector and prior DDNSTO/Aria2 builds are already fully validated.
5. Continue main-only atomic changes. Do not modify the frozen backup; do not force-push or publish per-file partial commits.

## Verification boundary

- DDNSTO SHA 3f and Aria2 SHA 766: exact-SHA XY-C1 Build, TRX and artifact upload PASS.
- PPPoE-MWAN detector SHA 300e: source/host checks PASS; exact-SHA firmware build/TRX/artifact not complete at last poll.
- Firmware-stop first-errno SHA 19d86: PID 1 notification, Process lifecycle, Process inventory and pre-build policy checks PASS; exact-SHA XY-C1 firmware Build/TRX/artifact still pending at last poll.
- Follow-up on SHA `805624195e6826f723c854afd2dabd7df891b50e`: Process inventory and PID 1 notification succeeded; XY-C1 first-errno test and source checker passed, Build was in progress. AI handoff contract failed because a required checkpoint sentence was omitted; detect_internet focused regression failed because its old source assertion expected the previous `saved_errno == 0` branch, which was superseded by the first-error helper. This commit repairs both test contracts.
- Physical router/hardware validation L4: NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
