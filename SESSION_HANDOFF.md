## Current repository state

- Active branch: `main`; source parent for this handoff checkpoint: `0b532570d102824d27fd3bb3ab4d6683f956d94f`. Re-fetch live main before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it.
- No open PRs were found. Legacy PR #1 is closed; its valid CA trust-store bootstrap and AliDDNS HMAC work were ported to main.
- LLDT PID descriptor hardening is in `529cf073`; fixture and source-contract fixes `8796b356`, `2fdf8e7a`, `9b97f743`; Process contract alignment `be370293`. Latest LLDT interface-name/path identity fix is `0b532570d102824d27fd3bb3ab4d6683f956d94f`.
- MTK AP 802.11r Boolean ioctl hardening is in `72e1e72dd494e6d930bdfef46f349f20da7065be`.

## Active task — Close LLDT Process path-identity contract, then continue kernel hardening

- Source `0b532570` adds `lldt_validate_interface_name()` (nonempty, slash-free, maximum 15 bytes) and `lldt_pidfile_format_path()`; PID filenames retain the complete interface identity instead of truncating it to ten characters. Tests cover overlong, empty, slash-containing names, path capacity and distinct names sharing a 10-character prefix.
- Dedicated LLDT regression #45376256149 on exact SHA `0b532570d102824d27fd3bb3ab4d6683f956d94f` PASS. Process run #45376256143 compiled and passed the LLTD PID parser, then failed only at a stale assertion expecting the removed `/var/run/%s-%.10s.pid` formatter. This commit updates that contract to require the checked formatter and interface identity tests.
- Process inventory #45376256035 PASS and NVRAM audit #45376256079 PASS on `0b532570`. XY-C1 run #45376256112 was still building; TRX and artifact steps were pending.
- MTK AP 802.11r SUPPORT/RIC/OTD across MT76x2, MT76x3, MT7615 and MT7915 now use exact-size BOOLEAN copies and fail closed on copy faults. Focused regression #45375797791 PASS on source SHA `72e1e72`; NVRAM #45375798550 PASS. Process #45375797796 was cancelled before relevant LLDT checks; XY-C1 #45375799312 remained in progress.
- R0KHID ioctl path is under review: `FT_ROKH_ID_LEN` is 48 and `FT_CFG.FtR0khId` is 49 bytes (48 characters plus terminator). All four variants currently reject lengths <=48, then copy caller length directly into this fixed buffer. Correct handling must accept only 1–48 bytes, copy into a bounded temporary, preserve state on `copy_from_user()` failure, and NUL-terminate before publication. Implement with regression and exact build checks.

## Verification boundary

- LLDT descriptor parser/metadata and dedicated source contract PASS #45375259525 on `9b97f743`; full-interface-name/path fixture and dedicated contract PASS #45376256149 on `0b532570`.
- Process integration #45376256143 FAIL only on obsolete pathname assertion after the LLTD parser step passed. This correction has not yet run on an exact SHA.
- Process inventory #45376256035 PASS and NVRAM #45376256079 PASS on `0b532570`.
- XY-C1 #45376256112 on exact source SHA `0b532570d102824d27fd3bb3ab4d6683f956d94f` was **IN_PROGRESS** at Build XY-C1; TRX and artifact pending.
- L4 physical-router validation: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-SHA Process lifecycle and AI handoff Actions triggered by this checker/docs correction; verify corrected LLDT path identity integration passes. Resolve any new assertion without weakening production behavior.
2. Refresh XY-C1 #45376256112; require Build, TRX and artifact success on exact source SHA `0b532570d102824d27fd3bb3ab4d6683f956d94f`.
3. Then implement the reviewed R0KHID input-boundary fix across all four MTK AP drivers with regression for lengths 0, 1, 48, 49, copy fault and unchanged state; continue source → regression → atomic commit → exact-SHA Actions.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state, then execute on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For every multi-file logical slice, use Git data APIs to create all blobs, one complete tree based on the latest main tree, and one commit with the exact main parent. Update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit; never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc, router configuration, security and compatibility.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
