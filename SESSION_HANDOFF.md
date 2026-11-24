## Current repository state

- Active branch: `main`; source parent for this checkpoint: `2789d637423205f209fae72461b1163ba45967a4`. Re-fetch live main before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it.
- No open PRs were found. Legacy PR #1 is closed; its valid CA trust-store bootstrap and AliDDNS HMAC work were ported to main.
- LLDT PID descriptor hardening: `529cf073`; fixture/checker fixes `8796b356`, `2fdf8e7a`, `9b97f743`; Process contract `be370293`; full interface identity `0b532570`; pathname checker fix `b270bcc` (Process #45376899681 PASS).
- MTK AP 802.11r Boolean fix: `72e1e72`; MT76x3 PSK fix: `e32120d`; PSK checker fixes: `f22cdf9`, `570be1e`; R0KH-ID bounded-copy source commit: `0517ce5e9e67d94a82e546b67a7876e5f110691b`.

## Active task — Fail closed on MTK AP usercopy faults and finish R0KH-ID validation

- All four driver families have `FT_ROKH_ID_LEN == 48` and a 49-byte `FtR0khId` field. The original `length <= 48` check rejected valid identifiers and then copied unbounded user length into fixed storage.
- A wider pass over the same AP ioctl dispatchers found exact-size `AuthMode`, `WepStatus` and `Ssid` stack copies in all four families that ignored nonzero `copy_from_user()` results and continued consuming partially initialized data. MT76x2 and MT7915 APCLI 802.1X/required-key Boolean setters had the same publish-after-fault defect. This slice changes each to return `-EFAULT` and break before using or publishing the input; the source checker now enforces the length/copy/failure/stop ordering for all ten paths.
- Commit `0517ce5` introduced a 1–48 byte payload guard, zeroed 49-byte staging buffer, checked `copy_from_user()`, and publish-after-success behavior. Its focused C boundary test passed in #45378091419; the source checker failed because it decoded a vendor header as UTF-8. The next commit changes the source contract to inspect vendor headers as bytes.
- Follow-up also handles Wireless Extensions character arguments that include the terminating NUL in `wrq->u.data.length`: raw 1–48-byte payload and 2–49-byte payload-plus-NUL forms are normalized to a 1–48-byte identifier, with the maximum 49-byte input checked before copy. A 49-byte non-NUL input is rejected after copy to the bounded staging buffer; no stored state changes on either copy or normalization failure.
- New shared helper/test covers invalid 0/50/UINT_MAX lengths, valid payload sizes 1 and 48, both raw and NUL-inclusive maximum-length encodings, missing terminator on the 49-byte form, empty payload and NULL arguments. Four driver contracts enforce the usercopy/normalization/publish ordering.
- MT76x3 `OID_SET_PSK` fix in `e32120d` caps at 64 bytes and stages one checked user copy before AP/APCLI/WSC publication. Focused MTK #45377817802 and AI docs #45377817942 PASS on `570be1e`.
- LLDT integration checker correction `b270bcc` PASS #45376899681 on exact SHA; dedicated LLDT #45376256149, Process inventory #45376256035 and NVRAM #45376256079 PASS on `0b532570`.

## Verification boundary

- R0KH-ID GNU89 ASan/UBSan boundary fixture and four-driver source contract PASS on exact SHA `2789d637423205f209fae72461b1163ba45967a4` (MTK #45379297940); AI handoff contract #45379297910 PASS. The exact-source XY-C1 run #45378781960 on production-source SHA `b9d55bfcbf23c751c2f548e35ba78e9e05e5118b` was in Build at last poll; TRX and artifact were pending.
- This slice's fail-closed AuthMode/WepStatus/SSID and APCLI Boolean copy-fault contracts are added but have not yet run in Actions.
- MT76x3 PSK focused regression #45377817802 PASS on `570be1e`; AI handoff #45377817942 PASS.
- LLDT Process integration #45376899681 PASS on `b270bcc`; dedicated LLDT #45376256149 PASS on `0b532570`.
- XY-C1 #45378091249 on `0517ce5` was still in progress at last check; this follow-up must receive its own Build/TRX/artifact validation.
- L4 physical-router validation: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head MTK and AI handoff Actions for this usercopy-fault slice. Confirm GNU89 ASan/UBSan R0KH-ID boundaries plus all ten source-contract cases pass; resolve any real failure.
2. Fetch exact-head XY-C1 Build/TRX/artifact Actions. The previous R0KH-ID source baseline #45378781960 was building on `b9d55bf`; require a result for a SHA containing this production-source slice before claiming L3.
3. After checks are closed or running independently, select the next concrete P1 security/stability issue outside this R0KH-ID slice and continue the source → regression → atomic commit → exact-SHA Actions loop.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state, then execute on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For every multi-file logical slice, use Git data APIs to create all blobs, one complete tree based on the latest main tree, and one commit with the exact main parent. Update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit; never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc, router configuration, security and compatibility.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
