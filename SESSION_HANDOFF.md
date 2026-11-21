# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main: `53e9c553994afe3917c3451dee9d73f116be1024`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Completed source slices

- Shared VPN boolean policy: both PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`. The follow-up commit `53e9c553994afe3917c3451dee9d73f116be1024` removes an accidentally retained old argument tail from `on_client_ifup()`, updates the checker to the shared helper, and corrects the part-023 archive index byte count from 109434 to the verified 109696 UTF-8 bytes (archive blob SHA `d896a53f5bd56915dbabb5ec4987a2dc069d8388` remains unchanged).
- OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hook cleanup. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- This commit hardens exported client profiles: activate `remote-cert-tls server` to match the generated server certificate's `serverAuth` EKU, and use a validated WAN IPv4 endpoint only as a final fallback for dual-stack modes 4/5 after DDNS and both IPv6 sources fail. IPv6-only modes remain unchanged. The VPN lifecycle checker cross-validates the directive against `openvpn-cert.sh` and checks profile option mapping/fallback order.

## Verification boundary

- Exact-head 53e9 Actions were pending at last poll: VPN lifecycle #44939430753, XY-C1 #44939430749, AI docs #44939430857, NVRAM #44939430717, Process inventory #44939430869, OpenVPN credential staging #44939430805. Those runs are stale relative to the current source commit.
- Parent 418 workflow findings: shared VPN policy C regression passed, but VPN lifecycle #44939242958 failed at a stale checker expecting the former parser; AI docs #44939242988 failed on part-023 byte count. Both were corrected in 53e9; exact-head re-runs are required.
- Parent 53e9 exact-head L3 not established. The current source commit must pass same-SHA XY-C1 Build, TRX verification and artifact upload before L3 is claimed. No local build/tests are claimed. Hardware L4 remains NOT VERIFIED.

## Exact first actions for the next GPT

1. Fetch live main and inspect exact-head VPN lifecycle, AI docs, NVRAM, Process and XY-C1 Actions for the current profile commit. The VPN checker must validate server EKU and dual-stack fallback; XY-C1 Build, TRX and artifact upload must all PASS on the same SHA.
2. Diagnose any first causal failure without weakening checks; PENDING may be followed by independent work.
3. Continue the highest-risk remaining stability/security defect in a different Padavan userland module after this bounded OpenVPN export slice; do not stay in VPN indefinitely.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

