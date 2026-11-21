## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main at last checkpoint: `1a13b60672917c55b0075d8c015522b87d37fea6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Completed source slices

- Shared VPN boolean policy: both PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`. The follow-up commit `53e9c553994afe3917c3451dee9d73f116be1024` removes an accidentally retained old argument tail from `on_client_ifup()`, updates the checker to the shared helper, and corrects the part-023 archive index byte count from 109434 to the verified 109696 UTF-8 bytes (archive blob SHA `d896a53f5bd56915dbabb5ec4987a2dc069d8388` remains unchanged).
- OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hook cleanup. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- This commit hardens exported client profiles: activate `remote-cert-tls server` to match the generated server certificate's `serverAuth` EKU, and use a validated WAN IPv4 endpoint only as a final fallback for dual-stack modes 4/5 after DDNS and both IPv6 sources fail. IPv6-only modes remain unchanged. The VPN lifecycle checker cross-validates the directive against `openvpn-cert.sh` and checks profile option mapping/fallback order.

## Active task

- The MiniUPnP protocol/secure-mode checked NVRAM preflight is committed in `27c02048027d50435f2d18665bbe74aaddc90fc9`. Missing keys retain legacy defaults; backend errors, malformed and out-of-range values fail before publishing a new daemon configuration.
- Exact-head MiniUPnP run #44941367108 failed only because the inline Python source contract referenced undefined names `config_file_header` and `config_file_test`. The workflow already loads the same files as `helper` and `test`; this correction points the two assertions at those existing variables without changing production code or weakening checks.
- XY-C1 #44941367009 was in progress before Build XY-C1 at last poll; same-SHA Build/TRX/artifact still required for L3. Hardware L4 remains NOT VERIFIED.
## Verification boundary

- Parent `27c02048027d50435f2d18665bbe74aaddc90fc9`: MiniUPnP configuration publication/lifecycle run #44941367108 failed at the final inline-Python source contract with `NameError: config_file_header is not defined`; all preceding policy and sanitizer steps passed. The cause is a checker variable-name mismatch: the workflow reads the header into `helper` and test into `test`.
- XY-C1 #44941367009 was still preparing the build (Build XY-C1 pending, TRX and artifact upload pending). Process inventory #44941366957 passed; DNSMasq #44941367068, Samba #44941367014, WAN route #44941367033, Inadyn #44941366953, WAN DHCP #44941366980 and AI docs #44941366962 passed on the same parent. HTTPD #44941366939 and NVRAM #44941367101 were still in progress.
- No same-SHA L3 Build/TRX/artifact pass is established for `27c02048027d50435f2d18665bbe74aaddc90fc9`. No local tests/build or hardware L4 are claimed.
## Exact first action for the next GPT

1. Inspect exact-head MiniUPnP config-stability rerun for this workflow-variable correction; the checked integer parser regression and config/daemon gates must pass, and the source contract must complete without NameError.
2. Poll XY-C1 #44941367009 for the `27c02048027d50435f2d18665bbe74aaddc90fc9` source. Require Build XY-C1, TRX verification and artifact upload all PASS on the same SHA before marking L3.
3. After these gates, continue the highest-risk unfinished userland reliability/security task outside MiniUPnP; PENDING workflows do not block independent work.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

