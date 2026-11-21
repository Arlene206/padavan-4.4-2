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

- The exported OpenVPN client profile actively requires `remote-cert-tls server`, matching the generated certificate's `serverAuth` EKU. Dual-stack modes 4/5 use validated WAN IPv4 only as a final fallback after DDNS and IPv6 sources fail; IPv6-only modes remain unchanged.
- VPN lifecycle #44939760147 passed on `69b61e88ef094496f3c7b4e7f7ac69669aa7a3b1`; AI handoff docs #44940022312 passed on `96de0aedf60c0605cbead65e50f69aa31483e82b`. d3b7 XY-C1 run #44939588534 was still in progress at last poll; inspect its final result.
- Inadyn `write_inadyn_conf()` used error-collapsing reads for `ddns_ssl`, `ddns2_ssl`, wildcard flags, `ddns_ipv6` and `ddns_source`. A backend failure could turn TLS off and publish a configuration that transmits credentials without encryption.
- This slice adds a strict checked integer parser and preflights six security/endpoint options via `nvram_get_checked()` before staging a replacement. Missing/empty values preserve legacy defaults; read/parse/range/overflow errors leave the last-known-good config intact. The existing Inadyn workflow gains GNU89/ASan/UBSan tests and source-contract assertions.

## Verification boundary

- On parent `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a`, OpenVPN client credential staging #44939588500 PASS. VPN lifecycle #44939588479 failed in the new exported-profile source contract because the ordered assertion searched for nested `options.auth_digest` before the containing `openvpn_add_auth()` call; this follow-up separates field-presence checks from ordered helper calls. Production source is unchanged by this checker correction.
- AI handoff docs #44939588455 failed because the previous handoff rewrite omitted the mandatory end-of-session accuracy checkpoint; this commit restores that required section.
- XY-C1 #44939588534 remained in progress before Build XY-C1 at last poll; Build, TRX verification and artifact upload had not yet run. No L3 result is established. Process #44939588492, NVRAM #44939588487 and HTTPD #44939588574 were also in progress.
- The parent source SHA `d3b7304c83e962c15d25f4be7c2dae8ce4f8484a` still requires same-SHA XY-C1 Build, TRX and artifact upload. No local tests/build are claimed; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Inadyn lifecycle, AI docs, NVRAM/Process and XY-C1 Actions for the checked TLS-option preflight. Diagnose any failure from its first causal log error; preserve the last-good DDNS configuration.
2. Require Build XY-C1, TRX verification and artifact upload on one exact SHA before recording L3. Independently resolve d3b7 XY-C1 run #44939588534; it was pending at last observation.
3. Continue the next highest-risk stability/security defect in a different Padavan userland module after this Inadyn slice.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

