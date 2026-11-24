## Current repository state

- Active branch: `main`; source parent for this logical slice: `8f639815fa60bcfa9a44514832ad895d36dd5a68`. Re-fetch live main before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify, develop on, or delete it.
- No open PRs were found. Legacy PR #1 is closed; its valid CA trust-store bootstrap and AliDDNS HMAC work were ported to main.
- Latest concurrent MTK usercopy fault source checkpoint: `8f639815fa60bcfa9a44514832ad895d36dd5a68`. It checks AuthMode/WepStatus/SSID usercopy faults in four AP families and APCLI 802.1X Boolean setter faults in MT76x2/MT7915.
- LLDT descriptor/path hardening: `529cf073`, `8796b356`, `2fdf8e7a`, `9b97f743`, `be370293`, `0b532570`, `b270bcc`. Process #45376899681 PASS on `b270bcc`; dedicated LLDT #45376256149 PASS on `0b532570`.
- MTK 802.11r Boolean fix `72e1e72`; MT76x3 PSK initial bounds fix `e32120d` with checker fixes `f22cdf9`/`570be1e`; R0KH-ID final input policy `b9d55bf` and checker/docs fixes `b046206`/`2789d637`.

## Active task — Validate the cross-family WPA PSK hardening

- MT7615 and MT7915 used a 65-byte security PSK field but also copied the caller's unrestricted length directly into the 64-byte WSC PSK field, even after the ordinary PSK branch rejected lengths of 65 or more. Usercopy errors were ignored and plaintext PSKs were logged.
- This slice caps PSK input at 64 bytes, stages one checked copy into a zeroed 65-byte temporary, and returns `-EINVAL`/`-EFAULT` before modifying either destination. It then clears and publishes the terminated value to the security field and only the bounded payload to WSC storage; WSC length is updated after successful staging. All three relevant families now log only lengths, not credentials.
- MT76x3 also uses the shared length predicate instead of its inline check. Added `mtk_wpa_psk_policy.h` and `tests/wpa_psk_policy.c` for 0, 1, 63, 64, 65, UINT_MAX and invalid zero maximum; tests compile under GNU89 with ASan/UBSan. Python source contracts cover MT76x3, MT7615 and MT7915, verify the 64/65-byte structure contract, checked-copy ordering, no independent WSC usercopy and absence of plaintext diagnostics.
- The prior R0KH-ID fix supports both raw and trailing-NUL Wireless Extensions inputs, stores a NUL-terminated 1–48-byte ID safely in 49 bytes. Exact focused MTK #45379297940 and AI docs #45379297910 PASS on checker/docs SHA `2789d637423205f209fae72461b1163ba45967a4`; source SHA `b9d55bfcbf23c751c2f548e35ba78e9e05e5118b`. NVRAM #45378781951 on b9d PASS; XY-C1 #45378781960 was still in Build XY-C1 at last observation, TRX/artifact pending.
- Concurrent fail-closed selector/Boolean slice on `8f639815`: exact MTK #45380653108 and AI #45380653126 PASS; XY-C1 #45380653170 and NVRAM #45380653441 were still running at last observation.

## Verification boundary

- R0KH-ID GNU89 ASan/UBSan regression plus four-driver source contract PASS #45379297940; AI handoff #45379297910 PASS on SHA `2789d637`.
- Usercopy fault-handling contracts in `8f639815`: MTK #45380653108 PASS; AI #45380653126 PASS. XY-C1 #45380653170 and NVRAM #45380653441 IN_PROGRESS at last observation.
- WPA PSK C test and MT76x3/MT7615/MT7915 source contracts are newly added here; exact-SHA results pending.
- XY-C1 #45378781960 for R0KH-ID source `b9d55bf` was IN_PROGRESS at last observation. This commit changes production sources and requires a fresh same-SHA Build/TRX/artifact PASS.
- L4 physical-router validation: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head MTK and AI handoff Actions for this PSK commit. Verify GNU89 ASan/UBSan boundaries and all three PSK source contracts; fix real failures without weakening checks.
2. Inspect exact-head XY-C1 Build/TRX/artifact and NVRAM/Process runs for the current source SHA. Also refresh earlier XY #45378781960 and #45380653170, but never claim L3 from a different source SHA.
3. Continue with the next concrete P1 stability/security defect beyond the MTK private-ioctl cluster; pending Actions do not block independent work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
