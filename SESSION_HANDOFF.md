# 2026-11-18 continuation checkpoint — SmartDNS autostart malformed-state regression

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, development branch `main`. Live main immediately before this checkpoint: `a36da98521b1d2fc3b7641f4cd98f73d103d89e7`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged; its useful AliDDNS CA/HMAC/bootstrap changes are already present in main. Do not mutate the backup, PR #1 or GPT Session Rotator.
- The SmartDNS managed DNSMasq product change is `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`; later commits in this series update tests/checkers/docs, not SmartDNS production source.

## Active task
- Fix the optional autostart regression fixture. Process run `44320600815` on `957017518551a598a0dae1a1e6af10e5750c7d68` first exposed the obsolete expectation that generic autostart itself removes SmartDNS DNSMasq entries. Commit `661d4f3` removed those resolver-cleanup assertions but left a contradictory expectation that a stub SmartDNS launcher deletes malformed saved state even though the test also requires that state to remain unchanged.
- Production ownership is intentional: `autostart.sh` calls the SmartDNS launcher and logs its status; `smartdns.sh` validates saved state in `Read_ini()` and explicitly refuses malformed state while preserving DNSMasq configuration. The regression now simulates launcher status 45, verifies the saved state and DNSMasq file stay byte-identical, checks the diagnostic, confirms custom user rules survive ad-block cleanup, and confirms independent DDNSTO/FRP launchers continue.
- Extend `scripts/check_smartdns_lifecycle.py` to pin the production Read_ini fail-closed validation and the autostart preservation expectations. No production SmartDNS source edit is intended for this slice.

## Verification boundary
- Exact Process run `44321557863` on `661d4f30857fdf97a922a5ee3bfffdebbfc920a8`: SmartDNS process-state source contract and process inventory passed; optional-add-on autostart regression failed solely at the stale saved-state deletion assertion. The corrected test/checker in THIS COMMIT require a new exact-head Process run.
- DNSMasq workflow `44321070200` on `0439e8c83146c2242f6907f1ef3b6888e5ccf6ac`: PASS, including managed-block and user-directive preservation tests and the regex-based POSIX helper parser.
- XY-C1 firmware run `44318683585` on the exact SmartDNS production-source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`: Build XY-C1, TRX verification and artifact upload all PASS; L3 is VERIFIED for that product-source SHA.
- vsftpd stability `44320651576` on `2b732b768a9d840e9123e60dc47f588bce129d33`, Infosvr `44320600854` on `957017518551a598a0dae1a1e6af10e5750c7d68`, Crond `44320518213` and Networkmap `44320518215` on `0fbc1958d5d74cdf13163de5cf965bad41eaa752`: PASS. NVRAM audit `44319662342`, USB storage `44319662199`, Process inventory `44319662359` and Samba `44319662327` on `8223c9b`: PASS.
- AI handoff workflow `44321071282` on `0439e8c` failed because the handoff omitted the exact required phrase `one atomic main commit`; THIS COMMIT adds it. New exact-head CI remains pending.
- No local checkout-based shell test or MIPS build was run in this connector-only session. BusyBox/uClibc live runtime, DNS traffic and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and exact-head Actions. Inspect the new Process and AI handoff workflows first; repair any first causal failure without weakening source or behavioral checks.
2. Confirm the corrected autostart test preserves malformed saved state and DNSMasq bytes while logging the SmartDNS failure and still launching DDNSTO/FRP. The host fixture stubs SmartDNS's exit status; the source checker separately proves the production Read_ini fail-closed path.
3. Continue the next risk-ranked stability defect from `REFACTOR_ROADMAP.md`; do not reopen the closed SmartDNS parser/checker or vsftpd fixes unless new evidence shows regression.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. If the active task, verification boundary and first action remain accurate, leave this handoff untouched.

## Cross-session execution mandate
One logical slice is one atomic main commit. Finish bounded source/test work, create one complete Git tree and commit, then update main with non-forced expected-SHA compare-and-swap. Continue independent work while relevant Actions are pending. Before context exhaustion, leave the next GPT the exact first action, commit SHA, tests and CI state. Never claim unobserved local tests, target builds or physical-device validation.
