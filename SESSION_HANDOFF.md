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

- The exported OpenVPN client profile requires `remote-cert-tls server`, matching the generated certificate's `serverAuth` EKU. Dual-stack modes 4/5 use validated WAN IPv4 only as a final fallback after DDNS and IPv6 sources fail; IPv6-only modes remain unchanged.
- MiniUPnP redirect-description output is bounded and always NUL-terminated. MiniUPnP config regression #44940897575 and AI handoff docs #44940897569 passed on `1a13b60672917c55b0075d8c015522b87d37fea6`. XY-C1 #44940897544 passed host checks but explicitly SKIPPED Build, TRX verification and artifact upload; it does not establish firmware L3.
- `start_upnp_locked()` previously read `upnp_proto`, `upnp_secure` and `http_lanport` through error-collapsing getters. A backend failure could silently choose the less restrictive zero-value policy and publish a replacement daemon configuration.
- This slice adds strict checked-integer parsing and a three-key NVRAM preflight. Any backend, malformed or out-of-range value now aborts before configuration publication; missing keys retain their legacy defaults.

## Verification boundary

- On exact main parent `1a13b60672917c55b0075d8c015522b87d37fea6`, MiniUPnP regression #44940897575 PASS and AI handoff docs #44940897569 PASS.
- XY-C1 #44940897544 completed host regression successfully, but Build XY-C1, TRX verification and artifact upload were explicitly SKIPPED because firmware production inputs were unchanged. This is not L3 validation.
- The checked `upnp_proto` / `upnp_secure` / `http_lanport` source change has not yet been validated by exact-head Actions. No local test, cross-build or hardware run is claimed.
- L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one exact SHA. Hardware L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Fetch live main and inspect exact-head MiniUPnP regression and AI handoff Actions for the checked `upnp_proto`, `upnp_secure` and `http_lanport` preflight. Fix test/source-contract failures from their first causal log error.
2. Inspect exact-head XY-C1 and NVRAM audit. Do not claim L3 unless Build, TRX verification and artifact upload all pass on one SHA; the prior 1a13 host-only run skipped those three steps.
3. Continue the next whole-userland reliability/security defect after this MiniUPnP security preflight, without reopening passed OpenVPN profile, redirect-copy or Inadyn parser slices absent same-SHA regressions.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

