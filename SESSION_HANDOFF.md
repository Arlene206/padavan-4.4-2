## 2026-11-28 continuation checkpoint

- Repository: `Arlene206/padavan-4.4-2`. Continue on `main`; startup recovery was completed once. Do not repeat the completed bootstrap/inventory audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No force-push, reset, rename, CI, or development work on this branch.
- Latest source-bearing commit: `a78e02ff07c7ea0686d666cd783b754b3b36c27f`; current workflow-only follow-up fixes indentation in the Samba static regression.
- PR #1 remains closed and unmerged. Its AliDDNS CA trust-store postcondition source, shell regression and workflow integration are present on main; do not resume development on the closed PR branch or assume its legacy branch is current.

## Exact-SHA validation already established

- DDNSTO production SHA `3f1cab20589e858b2db951b18821194249424957`: XY-C1 run #46141285573 completed SUCCESS; firmware Build, TRX verification and artifact upload all SUCCESS; artifact ID `15261971334`.
- Aria2 production SHA `76646cffc52d09fcb9b7a67052d5ae64b1b14462`: XY-C1 run #46141026720 completed SUCCESS; firmware Build, TRX verification and artifact upload all SUCCESS; artifact ID `15261729486`.
- PPPoE-MWAN detector commit SHA `300e78b2a36ceea51d508d18bc3cf09add6d633d`: WAN route-status run #46141847656 SUCCESS; XY-C1 run #46141847644 SUCCESS, including both PPPoE-MWAN shell tests, static checker, firmware Build, TRX and artifact upload; artifact ID `15262232177`.
- Firmware first-errno policy commit SHA `19d86adc5f57d17800a687435050b294affb6ade`: PID 1 notification run #46142248483 SUCCESS; Process lifecycle regression #46142248545 SUCCESS (including first-errno policy test and source checker); Process inventory #46142248530 SUCCESS; XY-C1 #46142248550 SUCCESS.
- No local MIPS/kernel build or router hardware is available in this session. Physical L4 remains NOT VERIFIED. Do not claim L3 for a SHA until its exact-SHA Build, TRX check and artifact upload all succeed.

## Current repository state

- Main-only development. The frozen backup remains unchanged.
- `flash_firmware()` now routes all ten independent pre-flash stop operations through `firmware_update_stop_first_errno()`: VPN client, VPN server, residual L2TP/PPP process cleanup, VPN lifecycle-lock release, MiniUPnPd, checked service stop, EAPOL, LAN authentication, 802.1X and PPPoE relay.
- Samba account generation now uses one copied NVRAM generation across `/etc/passwd`, passdb and `smb.conf`; boot-time passwd generation itself reads all account keys under one transaction.
- Each operation clears errno before it runs; the first nonzero failure errno is retained, defaulting to EIO when a failing operation leaves errno unset. Independent stop attempts continue; failure is logged and errno restored before returning. Persistence/MTD write remains unreachable after any stop failure.
- The GNU89 policy regression covers successful operations with stale errno, operation errno capture, EIO fallback and sticky first error. The Process lifecycle and XY-C1 workflows compile/run it with sanitizers and execute the extended firmware-update source checker.
- Root `dev_log.md` has 98 records before this commit and is below archive thresholds; roadmap is near its size cap and was not edited.

## Active task

- Validate the single-account-generation Samba fix plus remaining CI source-contract repairs.
- Complete exact-SHA XY-C1 Build, TRX and artifact checks for the Samba snapshot SHA and its CI follow-ups.

## Exact first action for the next GPT

1. Fetch live main and Actions for this workflow-syntax repair SHA; confirm Samba embedded Python runs and its snapshot assertion passes.
2. Confirm Process lifecycle/optional-app source checks pass with the first-error helper; confirm Process inventory and AI handoff docs pass.
3. Inspect source SHA 9ffd and a78 exact-head XY-C1 runs for firmware Build, TRX and artifact upload; both were IN_PROGRESS at last poll.
4. Continue only after checking exact job logs. Do not touch frozen backup; do not force-push.

## Verification boundary

- DDNSTO SHA 3f, Aria2 SHA 766 and PPPoE-MWAN SHA 300e: exact-SHA XY-C1 Build, TRX and artifact upload PASS.
- First-errno SHA 19d86 and all-stop follow-up SHA 805624: exact-SHA XY-C1 completed SUCCESS; first-errno test/source checker and relevant Process/PID1 contracts passed.
- Samba snapshot source SHA 9ffd: Process inventory and AI handoff PASS; first Samba/Process failures were stale source assertions; XY-C1 #46143284111 remained IN_PROGRESS at last poll.
- CI contract commit a78e02ff: Samba embedded contract and Process optional-app checker were repaired; exact-head Samba/Process regressions still exposed old expectations. Commit 0a423aad updated those assertions, but its Samba workflow run had no jobs because four embedded Python lines were misindented. This commit corrects the YAML indentation.
- Validate corrected exact-head Samba/Process regressions and source SHA XY-C1 run before making any L3 claim.
- Physical router/hardware validation L4: NOT VERIFIED.
## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
