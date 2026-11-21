## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main: `53e9c553994afe3917c3451dee9d73f116be1024`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Completed source slices

- Shared VPN boolean policy: both PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`. The follow-up commit `53e9c553994afe3917c3451dee9d73f116be1024` removes an accidentally retained old argument tail from `on_client_ifup()`, updates the checker to the shared helper, and corrects the part-023 archive index byte count from 109434 to the verified 109696 UTF-8 bytes (archive blob SHA `d896a53f5bd56915dbabb5ec4987a2dc069d8388` remains unchanged).
- OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hook cleanup. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- This commit hardens exported client profiles: activate `remote-cert-tls server` to match the generated server certificate's `serverAuth` EKU, and use a validated WAN IPv4 endpoint only as a final fallback for dual-stack modes 4/5 after DDNS and both IPv6 sources fail. IPv6-only modes remain unchanged. The VPN lifecycle checker cross-validates the directive against `openvpn-cert.sh` and checks profile option mapping/fallback order.

## Active task

- The exported OpenVPN client profile now actively requires `remote-cert-tls server`, matching the server certificate's `serverAuth` EKU. Dual-stack modes 4/5 fall back to a validated WAN IPv4 endpoint only after DDNS and IPv6 WAN/LAN lookup fail; IPv6-only modes remain unchanged.
- The follow-up exact-head VPN lifecycle run #44939760147 on `69b61e88ef094496f3c7b4e7f7ac69669aa7a3b1` passed the corrected profile and certificate-role contracts. OpenVPN client credential staging #44939588500 and Process inventory #44939588492 also passed on the profile's source parent `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a`.
- The d3b7 XY-C1 run #44939588534 was still building at the last poll; same-SHA Build XY-C1, TRX and artifact upload remain required for L3. Hardware L4 is NOT VERIFIED.

## Verification boundary

- On parent `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a`, OpenVPN client credential staging #44939588500 PASS. VPN lifecycle #44939588479 failed in the new exported-profile source contract because the ordered assertion searched for nested `options.auth_digest` before the containing `openvpn_add_auth()` call; this follow-up separates field-presence checks from ordered helper calls. Production source is unchanged by this checker correction.
- AI handoff docs #44939588455 failed because the previous handoff rewrite omitted the mandatory end-of-session accuracy checkpoint; this commit restores that required section.
- XY-C1 #44939588534 remained in progress before Build XY-C1 at last poll; Build, TRX verification and artifact upload had not yet run. No L3 result is established. Process #44939588492, NVRAM #44939588487 and HTTPD #44939588574 were also in progress.
- The parent source SHA `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a` still requires same-SHA XY-C1 Build, TRX and artifact upload. No local tests/build are claimed; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head VPN lifecycle, AI docs, NVRAM, Process and XY-C1 Actions for the current profile commit. The VPN checker must validate server EKU and dual-stack fallback; XY-C1 Build, TRX and artifact upload must all PASS on the same SHA.
2. Diagnose any first causal failure without weakening checks; PENDING may be followed by independent work.
3. Continue the highest-risk remaining stability/security defect in a different Padavan userland module after this bounded OpenVPN export slice; do not stay in VPN indefinitely.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

