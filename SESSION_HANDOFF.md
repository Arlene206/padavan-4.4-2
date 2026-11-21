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

- The MiniUPnP description-copy implementation is committed in `4a5139f484943abe3a5ac9cf0f6d56bc70067a56`: both stored and fallback metadata copies use the bounded helper. The GNU89 ASan/UBSan test passes exact, truncated, 1-byte, empty and invalid argument cases.
- Exact-head MiniUPnP run #44940761661 failed only in the source-contract assertion: C formatting split the helper call after its opening parenthesis, while the checker expected no space after whitespace normalization. This checker-only child compacts that call-site spacing before matching.
- XY-C1 #44940761694 and NVRAM #44940761781 were pending at the last poll; Process inventory #44940761713 and AI handoff #44940761657 passed. Hardware L4 remains NOT VERIFIED.
## Verification boundary

- Source commit `4a5139f484943abe3a5ac9cf0f6d56bc70067a56`: AI handoff #44940761657 PASS; Process inventory #44940761713 PASS; MiniUPnP workflow #44940761661 FAIL only at checker whitespace assertion. Its preceding GNU89/ASan/UBSan binaries, including the new bounded-description regression, all passed.
- XY-C1 #44940761694: in progress at last poll; require Build XY-C1, TRX verification and artifact upload all PASS on the same source SHA. NVRAM #44940761781 also in progress. No L3 claim yet.
- The checker-only correction must pass a new exact-head MiniUPnP run. No local tests/build or hardware L4 are claimed.
## Exact first action for the next GPT

1. Inspect the exact-head MiniUPnP rerun for this checker correction. Confirm the bounded-description policy binary and SOAP consumer source contract both pass. Poll XY-C1 and require Build, TRX and artifact upload all PASS on one exact source SHA before L3 is recorded.
2. If a contract fails, inspect the normalized source and first causal log error; fix the assertion or source without weakening coverage.
3. After the MiniUPnP security slice validates, continue the next highest-risk unfinished Padavan userland defect.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

