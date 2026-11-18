# 2026-11-18 continuation checkpoint — SSH lifecycle contract and cross-module CI repair

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`, development branch `main`. Live main immediately before this checkpoint: `5e5c3c0515e0f42513711703e637cd7bab312c88`. Last product-source milestone relevant to this recovery: SmartDNS managed DNSMasq configuration preservation at `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`; subsequent `e896587`, `8223c9b`, and `5e5c3c0` commits updated checker/test contracts or fixtures, not firmware production source.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged; its valid AliDDNS CA/HMAC/bootstrap work is already present in the evolved main files, so no further port is outstanding.
- All development remains main-only. Do not touch the frozen backup, PR #1, or GPT Session Rotator.

## Active task
- Correct the SSH detached-child startup checker after the exact Process workflow on `8223c9b9023f48fbeb7c6cb2a114cde7ba2d46db` (run `44319662328`) passed the compiled SSH lifecycle policy tests but failed the Python source contract. The checker expected a direct `sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS)`; current `start_sshd()` correctly uses the common status-returning `service_start_probe_wait("SSH", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS)`. THIS COMMIT changes that checker assertion only.
- SmartDNS transactional DNSMasq integration at source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c` preserves user-owned directives using a marked managed block, validates legacy saved state, stages config in the same directory and atomically renames it. Runtime regressions cover idempotent cleanup, exact legacy directives, malformed markers, symlinks, failed rename and unchanged user rules. No SmartDNS runtime product change is part of THIS COMMIT.
- Next independent failed contract: vsftpd workflow run `44319662317` on `8223c9b`; its GNU89 C regressions passed, but the embedded Python source contract raised an unlabelled AssertionError at stdin line 89. Inspect the numbered assertion against current source before adjusting the contract.

## Verification boundary
- Exact-head DNSMasq workflow `44319742816` on `5e5c3c0515e0f42513711703e637cd7bab312c88`: PASS, including DNSMasq configuration preservation shell tests and the corrected Python helper extractor.
- Process workflow `44319662328` on `8223c9b`: FAIL only at the SSH source-contract assertion after the SSH GNU89 policy test and firewall shutdown checker printed PASS; the corrected assertion in THIS COMMIT requires a new exact-head Process run.
- Process lifecycle inventory `44319662359` on `8223c9b`: PASS. Samba share stability `44319662327` on `8223c9b`: PASS. NVRAM transaction audit `44319662342` and USB storage hotplug `44319662199` were IN_PROGRESS at the last observation.
- vsftpd run `44319662317` on `8223c9b`: FAIL at its embedded Python source contract after GNU89 C regressions passed; investigate next.
- The SmartDNS product source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c` had XY-C1 run `44318683585` IN_PROGRESS at the last observation; no current-SHA Build/TRX/artifact PASS is claimed. Host/router L4 and target BusyBox/uClibc runtime remain NOT VERIFIED. Local checkout and local test execution were unavailable in this connector-only session.

## Exact first action for the next GPT
1. Fetch current live main and exact-head Actions. Verify the Process run created by THIS COMMIT; inspect whether the SSH lifecycle checker now passes and repair any first causal failure.
2. Continue the vsftpd repair from run `44319662317`: number the embedded Python here-doc and map stdin line 89 to the exact assertion, then compare the expected contract with the current source. Preserve the successful C tests and do not weaken real ACL, path identity or configuration publication checks.
3. Revisit the NVRAM and USB-storage pending runs only if their results affect the next action; confirm the XY-C1 build for the actual SmartDNS production-source SHA before claiming L3.
4. Continue the repo-wide stability/security queue with bounded source fixes, regressions, atomic main commits and exact-head Actions.

## Mandatory handoff checkpoint
Every session MUST compare live repository state and exact-head Actions with this checkpoint before ending. Before context exhaustion, update this file only if the active task, verification boundary, blocker or precise next action materially changes.

## Cross-session execution mandate
Continue without asking the user to select the next task. Finish safe slices with source review, regression tests, complete-tree atomic commits and non-forced expected-SHA main updates. Never claim local tests, MIPS builds, firmware artifacts or physical-device validation that were not actually observed.
