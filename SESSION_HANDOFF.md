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

1. Fetch live main and exact-head Actions for this Samba ACL guard update.
2. Require all three Samba workflow steps (config source guard, ACL/passwd parity, transaction durability) to pass.
3. Confirm Process inventory/lifecycle and AI handoff checks remain green.
4. Inspect XY-C1 firmware Build, TRX and artifact for source SHAs 9ffd and a78; both were in progress at last poll.
5. Continue main-only source fixes while CI builds are pending; preserve immutable backup and never force-push.

## Verification boundary

- DDNSTO SHA 3f, Aria2 SHA 766 and PPPoE-MWAN SHA 300e: exact-SHA XY-C1 Build, TRX and artifact upload PASS.
- First-errno SHA 19d86 and all-stop follow-up SHA 805624: exact-SHA XY-C1 completed SUCCESS; first-errno tests/source checker and Process/PID1 contracts passed.
- Samba snapshot source SHA 9ffd: Process inventory and AI docs PASS; Samba/Process earlier runs failed on stale source assertions; XY-C1 #46143284111 still IN_PROGRESS.
- CI contract follow-up a78e02ff: Process inventory success, but Process lifecycle failed on stale optional-app errno assertion; fixed in 0a423aad, whose Process inventory and AI handoff checks PASS.
- Workflow syntax repair 369afd4a: AI handoff PASS; Samba first source-contract step PASS. Samba ACL parity step failed because one more assertion expected the old recreate_passwd_unix(0) call; this commit updates it to the snapshot API.
- The XY-C1 runs for source SHAs 9ffd and a78 remained IN_PROGRESS at the last poll. Inspect their exact Build/TRX/artifact steps.
- Physical router/hardware validation L4: NOT VERIFIED.
## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
