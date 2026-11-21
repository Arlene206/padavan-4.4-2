## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main source SHA at this checkpoint: `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e` (the handoff/log/roadmap update is committed as a documentation-only child).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Completed source slices

- Shared VPN boolean policy: both PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`. The follow-up commit `53e9c553994afe3917c3451dee9d73f116be1024` removes an accidentally retained old argument tail from `on_client_ifup()`, updates the checker to the shared helper, and corrects the part-023 archive index byte count from 109434 to the verified 109696 UTF-8 bytes (archive blob SHA `d896a53f5bd56915dbabb5ec4987a2dc069d8388` remains unchanged).
- OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hook cleanup. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- This commit hardens exported client profiles: activate `remote-cert-tls server` to match the generated server certificate's `serverAuth` EKU, and use a validated WAN IPv4 endpoint only as a final fallback for dual-stack modes 4/5 after DDNS and both IPv6 sources fail. IPv6-only modes remain unchanged. The VPN lifecycle checker cross-validates the directive against `openvpn-cert.sh` and checks profile option mapping/fallback order.

## Active task

- WSD/WS-Discovery HTTP POST framing hardening is in source commit `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e`. The old parser dereferenced a NULL `strstr()` result for a POST whose first read did not yet contain CRLF; fragmented header refill also retained the pre-`memmove` pointer, one `recv()` incorrectly assumed it filled the body, and `atoi(Content-Length)` accepted overflow/trailing junk. The new bounded socket helpers read complete CRLF lines and body length, restore the buffer cursor after compaction, strictly parse a non-zero decimal Content-Length, reject duplicates and enforce the buffer cap. GNU99/Werror/ASan/UBSan socket regression and source contract passed in exact-head WSD run #44942356801.
- P910nd bidirectional SIGPIPE daemon-crash fix remains in source commit `23248f7df1575885e991fe518629bb974fc05c35`. Exact-head p910nd workflow #44941685747, Process inventory #44941685697, Process regression #44941685794 and AI docs #44941685744 passed. Its XY-C1 run #44941685839 remains in progress (Build XY-C1 in progress; TRX and artifact pending); NVRAM run #44941685864 was cancelled by a later head. L3 is not verified.
- MiniUPnP config preflight now checks protocol, secure mode, HTTP presentation port, cleanup timers and internal/external mapping range bounds through checked integer reads in source commit `be4952e90111ddea8f91684f6c3fe5ecf25be785`. MiniUPnP run #44941839578, NVRAM #44941839605, Process inventory #44941839608, Samba #44941839654 and AI docs #44941839672 passed; XY-C1 #44941839623 remained Build-in-progress with TRX/artifact pending.
- The older MiniUPnP redirect-description source SHA `4a5139f484943abe3a5ac9cf0f6d56bc70067a56` is the only one of these MiniUPnP slices with exact-source XY-C1 Build, TRX and artifact upload PASS (#44940761694); do not extend L3 to later SHAs.

## Verification boundary

- WSD source SHA `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e`: WSD HTTP framing workflow #44942356801 PASS; AI handoff #44942356848 PASS. Process inventory #44942356815, Process lifecycle regression #44942356893 and NVRAM audit #44942356787 were queued/in progress at last poll. XY-C1 #44942356809 was in progress with Build still pending; TRX and upload pending. L2 host/contract PASS, L3 NOT VERIFIED, hardware L4 NOT VERIFIED.
- P910nd source SHA `23248f7df1575885e991fe518629bb974fc05c35`: p910nd #44941685747, Process inventory #44941685697, Process lifecycle #44941685794 and AI docs #44941685744 PASS. XY-C1 #44941685839 Build was in progress, TRX/artifact pending. NVRAM #44941685864 was cancelled. L3 NOT VERIFIED.
- MiniUPnP checked-range/cleanup source SHA `be4952e90111ddea8f91684f6c3fe5ecf25be785`: MiniUPnP, NVRAM, Process inventory, Samba and AI docs PASS; XY-C1 #44941839623 Build was in progress, TRX/artifact pending. L3 NOT VERIFIED.
- L3 remains established only for the redirect-description source SHA `4a5139f484943abe3a5ac9cf0f6d56bc70067a56` with #44940761694 Build/TRX/artifact PASS. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect WSD XY-C1 run #44942356809 for source SHA `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e`; require Build XY-C1, TRX verification and artifact upload all PASS before recording L3. Resolve any exact-SHA failure at its first causal log.
2. Poll WSD Process inventory #44942356815, Process lifecycle #44942356893 and NVRAM #44942356787; inspect p910nd XY-C1 #44941685839 and MiniUPnP XY-C1 #44941839623. Do not wait on PENDING; continue another independent userland reliability/security slice.
3. Keep the backup branch immutable. Do not reopen the bounded WSD/p910nd/MiniUPnP fixes without an exact-source regression.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

