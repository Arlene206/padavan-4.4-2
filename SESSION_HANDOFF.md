## Current repository state

- Active branch: `main`; follow-up parent SHA: `d3733ecbec2e0cf652c00eadf1db4645ae5ff13e`. Re-fetch live main before any write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable, never develop on or modify it.
- No open PRs.
- HTTPD protocol-selector guard is L3 VERIFIED on exact SHA `659b91b497b5cf8073b54feab849a91623092c70`: HTTPD #45388114470, AI #45388114468, Process #45388114507 and XY-C1 #45388114515 PASS, including Build/TRX/artifact. Physical L4 unverified.
- MTK AP private-ioctl safety remains previously L3 verified on `d33fcf06b73fbe2771ab0b559116d3e750dc4918`; physical L4 unverified.

## Active task — WAN source PHY selector range guard

- Source change in `net_wan.c` and policy `wan_src_phy_policy.h` reject positive `wan_src_phy` IDs >5 before switch ioctl; negative values retain historical WAN=0 fallback. Reason: MTK's unsupported UAPI port mapping reports success with link=0, which otherwise looks like a real cable disconnect.
- Host regression in detect-link run #45390205127 PASS (GNU89/Werror/ASan/UBSan); the pre-existing detect_link integration contract also PASS. The new final source-contract step FAILED because it expected direct `test_name();` while the test uses repository convention `CHECK(test_name() == 0)`. THIS COMMIT corrects the assertion and C diagnostic newline.
- AI docs run #45390205125 FAILED only because `REFACTOR_ROADMAP.md` reached 131111 bytes, 39 bytes over its 131072-byte hard limit. THIS COMMIT shortens the two new priority entries; encoded roadmap size is checked below the limit before publication.
- XY-C1 run #45390205170 was still building when last checked; no Build/TRX/artifact conclusion yet. WAN route status #45390205087 PASS. Process #45390205058 and NVRAM #45390205138 were still running at last poll.
- Parent SHA: `d3733ecbec2e0cf652c00eadf1db4645ae5ff13e`; HTTPD selector L3 remains proven on its earlier exact SHA, not this WAN change.

## Verification boundary

- THIS FOLLOW-UP contains the corrected regression contract, valid C diagnostic string, reduced roadmap and handoff/log status.
- Recheck detect-link, AI docs, XY-C1 Build/TRX/artifact, Process and NVRAM by this exact SHA. L3 for the WAN selector requires same-SHA XY-C1 Build, TRX and artifact PASS.
- Physical router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch current `main` and poll exact-head push Actions for detect-link, AI docs, XY-C1, Process and NVRAM. Confirm the two known checker corrections now PASS.
2. Fix any new first-causal failure atomically; keep L3 open until same-SHA XY-C1 Build/TRX/artifact PASS.
3. Continue the next highest-priority independent stability/security defect; do not re-audit the verified HTTPD/MTK slices without new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
