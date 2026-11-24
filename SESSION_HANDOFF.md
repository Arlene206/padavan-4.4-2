## Current repository state

- Active branch: `main`; source parent for this commit: `570be1efbc85fb86e3941de9da4a7d4649f159e8`. Re-fetch live main before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it.
- No open PRs were found. Legacy PR #1 is closed; its valid CA trust-store bootstrap and AliDDNS HMAC work were ported to main.
- LLDT PID descriptor hardening: `529cf073`; fixture/checker fixes `8796b356`, `2fdf8e7a`, `9b97f743`; Process contract `be370293`; full interface identity `0b532570`; Process pathname checker fix `b270bcc` (run #45376899681 PASS on exact SHA).
- MTK AP 802.11r Boolean fix: `72e1e72`; MT76x3 PSK fix: `e32120d`; PSK checker corrections: `f22cdf9` and `570be1e`.

## Active task — Validate R0KH-ID input boundaries and the MTK ioctl series

- The fixed field is confirmed across MT76x2, MT76x3, MT7615 and MT7915: `FT_ROKH_ID_LEN` is 48 and `FT_CFG.FtR0khId` is 49 bytes with a 1–48-character comment. The prior `length <= 48` check rejected valid IDs then copied arbitrary caller lengths into the fixed buffer.
- This commit updates all four `OID_802_11R_R0KHID` cases to accept only 1–48, stage in a zeroed 49-byte temporary, return `-EFAULT` without publishing on usercopy failure, and copy the full NUL-terminated storage only on success.
- New shared policy header and GNU89 ASan/UBSan C test cover 0, 1, 48, 49, UINT_MAX and zero maximum. Python contracts confirm all four macro/storage definitions and validate copy-before-publish ordering. The focused MTK workflow compiles the C test; XY-C1 watches the production helper header.
- MT76x3 `OID_SET_PSK` was fixed in `e32120d` (64-byte maximum, one user copy to staged NUL-terminated storage, checked before AP/APCLI/WSC publication). Its exact MTK run #45377268089 failed because of a regex escape; #45377504033 then exposed a false-positive WSC diagnostic assertion. Both checker-only issues are corrected in `f22cdf9` and `570be1e`; exact MTK run #45377817802 on `570be1e` PASS. AI docs #45377817942 on the same SHA PASS.
- LLDT Process integration checker fix `b270bcc` PASS #45376899681 on exact SHA, including LLTD parser and shared-library/RC integration. Dedicated LLDT #45376256149, Process inventory #45376256035 and NVRAM #45376256079 PASS on `0b532570`.
- NVRAM #45377268111 on `e32120d` PASS. XY-C1 #45377268326 on `e32120d` was still in progress at last check. The R0KH-ID source commit needs its own exact-SHA Build/TRX/artifact run.

## Verification boundary

- R0KH-ID GNU89/ASan/UBSan boundary test and four-source source contract are newly added; not run on this commit yet. Require focused MTK exact-SHA PASS.
- MT76x3 PSK fix: exact MTK run #45377817802 PASS on `570be1e`; AI docs #45377817942 PASS.
- LLDT Process integration #45376899681 PASS on `b270bcc`; LLDT dedicated #45376256149 PASS on `0b532570`.
- XY-C1 #45377268326 on `e32120d` still needs its terminal status checked; no L3 claim without Build, TRX and artifact success on one SHA.
- L4 physical-router validation: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head Actions for this commit. Verify the MTK R0KH-ID C boundary test and four-source contract; fix genuine failures without weakening checks.
2. Refresh XY-C1 #45377268326 for the previous source, then inspect this commit's exact XY-C1 Build/TRX/artifact run. Close NVRAM/Process checks triggered by the newest source if any.
3. After verification is closed or running independently, choose the next concrete P1 stability/security issue in wireless or userland and continue the source → regression → atomic commit → exact-SHA Actions loop.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state, then execute on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For every multi-file logical slice, use Git data APIs to create all blobs, one complete tree based on the latest main tree, and one commit with the exact main parent. Update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit; never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc, router configuration, security and compatibility.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
