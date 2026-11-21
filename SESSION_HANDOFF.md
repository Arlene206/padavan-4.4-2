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

- `start_upnp_locked()` now reads protocol, secure mode, HTTP presentation port, both cleanup timer settings, and all internal/external mapping port-range bounds through `miniupnpd_read_nvram_integer()`. Missing/empty values preserve defaults; backend, malformed and out-of-range values fail before publishing a replacement daemon configuration. Existing min>max normalization is unchanged.
- The preceding checked config preflight on parent `27c02048027d50435f2d18665bbe74aaddc90fc9` had a workflow-only NameError from two undefined checker identifiers; fixed in child `6c48528d8ec5c746de31f9d25b2016504e9aa831`. Exact-head MiniUPnP and AI docs checks on that child both passed (#44941570192 and #44941570201).
- This expanded preflight is the next source validation target. Require its MiniUPnP host/source-contract run and XY-C1 Build/TRX/artifact on the same source SHA; hardware L4 remains NOT VERIFIED.
## Verification boundary

- Parent `23248f7df1575885e991fe518629bb974fc05c35`: p910nd SIGPIPE regression #44941685747 PASS; AI handoff #44941685744 PASS. XY-C1 #44941685839 remained before Build XY-C1 (Build/TRX/artifact pending) at the last poll. Process inventory #44941685697 and NVRAM audit #44941685864 were still in progress.
- The preceding MiniUPnP checked protocol/security/HTTP preflight was validated on child `6c48528d8ec5c746de31f9d25b2016504e9aa831`: MiniUPnP config regression #44941570192 PASS and AI handoff #44941570201 PASS. The prior failure #44941367108 was a Python NameError, corrected without altering production policy.
- The current full preflight change has not yet run Actions. Same-SHA Build XY-C1, TRX and artifact upload are required for L3. No local build or hardware L4 is claimed.
## Exact first action for the next GPT

1. Inspect exact-head MiniUPnP config regression for this change. Confirm its parser tests pass and the source contract requires all nine checked NVRAM settings, rejecting any unchecked `nvram_safe_get_int("upnp_...")` fallback in `start_upnp_locked()`.
2. Inspect the XY-C1 run for this same production-source SHA; require Build XY-C1, TRX verification and artifact upload all PASS before recording L3.
3. Continue the highest-risk remaining reliability/security defect in another Padavan userland module once these independent checks are running; PENDING does not block work.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

