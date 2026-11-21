## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main at last checkpoint: `f27f59a23141112124ef874a1d4a0406dac898f5`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Completed source slices

- Shared VPN boolean policy: both PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`. The follow-up commit `53e9c553994afe3917c3451dee9d73f116be1024` removes an accidentally retained old argument tail from `on_client_ifup()`, updates the checker to the shared helper, and corrects the part-023 archive index byte count from 109434 to the verified 109696 UTF-8 bytes (archive blob SHA `d896a53f5bd56915dbabb5ec4987a2dc069d8388` remains unchanged).
- OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hook cleanup. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- This commit hardens exported client profiles: activate `remote-cert-tls server` to match the generated server certificate's `serverAuth` EKU, and use a validated WAN IPv4 endpoint only as a final fallback for dual-stack modes 4/5 after DDNS and both IPv6 sources fail. IPv6-only modes remain unchanged. The VPN lifecycle checker cross-validates the directive against `openvpn-cert.sh` and checks profile option mapping/fallback order.

## Active task

- MiniUPnP redirect-description output previously used unbounded-to-termination `strncpy()` into caller-supplied buffers. A SOAP-controlled description can be at least 64 bytes, while `GetSpecificPortMappingEntry()` copies into `char desc[64]` and passes the result to XML escaping and `%s` logging. This slice adds a NUL-preserving bounded-copy helper for stored and default descriptions, GNU89 sanitizer regression coverage and a source contract tied to the SOAP consumer.
- The recent current-head Inadyn repair `1af2d134760bef0ade9054dfb88f67f641759ced` passed AI docs #44940632808, Inadyn lifecycle #44940632768 and DNSMasq #44940632885. The previous helper-definition failure on parent f27f was corrected without weakening the source checks.
- The MiniUPnP slice must pass its exact-head stability workflow and same-SHA XY-C1 Build/TRX/artifact. Hardware L4 remains NOT VERIFIED.
## Verification boundary

- Parent `1af2d134760bef0ade9054dfb88f67f641759ced`: AI handoff #44940632808 PASS, Inadyn lifecycle #44940632768 PASS, DNSMasq lifecycle #44940632885 PASS. Process inventory, WAN route, WAN DHCP, Samba, HTTPD and MiniUPnP configuration publication also passed on parent f27f `#44940446796`, `#44940446824`, `#44940446779`, `#44940446725`, `#44940446885` and `#44940446973` respectively.
- The earlier Inadyn workflow #44940446835 failed only after all GNU89 sanitizer binaries passed, because its checker called undefined `ordered()`; commit `1af2d134760bef0ade9054dfb88f67f641759ced` corrected the contract and exact-head Inadyn rerun passed.
- The d3b7 OpenVPN source SHA `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a` has VPN lifecycle PASS on checker child `69b61e88ef094496f3c7b4e7f7ac69669aa7a3b1`. Its same-source XY-C1 run #44939588534 had Build/TRX PASS and artifact upload still running at the last poll; do not claim L3 until artifact upload completes.
- No local test/build is run or claimed for this MiniUPnP slice. Its exact-head MiniUPnP workflow and XY-C1 are pending after commit; L3 and hardware L4 are NOT VERIFIED.
## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head MiniUPnP config-stability and XY-C1 Actions for the bounded redirect-description commit. Confirm the GNU89 ASan/UBSan regression and source contract pass; require Build XY-C1, TRX verification and artifact upload all PASS on this same production-source SHA before claiming L3.
2. Diagnose any CI failure from its first causal log error and fix source/tests atomically without weakening checks. PENDING runs do not block independent tasks.
3. After this bounded MiniUPnP security issue validates, continue the highest-risk unfinished defect in another Padavan userland module.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

