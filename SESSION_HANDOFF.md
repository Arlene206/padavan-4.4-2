# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue main-only development with one complete Git tree/commit per logical task and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement real fixes with focused regressions; do not weaken tests.
- PENDING Actions do not block independent work. Diagnose exact-SHA failures from the first causal log.
- Finish each safe slice, commit it and leave a factual first next action before context exhaustion.

## Mandatory handoff checkpoint

At session end inspect live main, verify the frozen backup hash, and inspect relevant exact-SHA Actions. Update this file only when task, verification evidence or the first next action materially changes. Never make a no-op handoff commit.

## Repository state

- Repository Arlene206/padavan-4.4-2. Parent for this test-fixture correction: 30af417ed8d7665e5da0f630e79b38bbb9f92296.
- Frozen backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product commit 30af417ed8d7665e5da0f630e79b38bbb9f92296 moves nat_scan_completed to after the completed NAT rule loop, updates the source contract to enforce loop-close-before-completion, and keeps missing-chain handling fail-closed.
- Workflow-only commit 72b914095e14c099f60d6ac8f20150b4ca5d29e9: MiniUPnP #44803703734 PASS and AI docs #44803703663 PASS. It fixed the obsolete inline lease-file stdio assertion.
- Injected test scripts/test_miniupnpd_redirect_delete_integration.py compiles the actual _upnp_delete_redir() function with mock firewall/lease operations. Previous cbe06d3 test run #44804109446 failed because the generated C omitted the signature and used literal backslash-n delimiters. This commit fixes those test-fixture construction errors, not production behavior.
- Earlier product source SHA 35e still has XY run #44803257077 pending as of the last poll. Its result cannot validate later product source 30af417ed8d7665e5da0f630e79b38bbb9f92296; this commit and 30af417ed8d7665e5da0f630e79b38bbb9f92296 need their own exact-head CI for the current source.
- AliDDNS PR #1 remains closed/unmerged; HMAC stdin helper and CA trust bootstrap tests were already present on main, so no cherry-pick required.

## Active task

1. Inspect exact-head MiniUPnP and AI Actions after this test correction. Ensure the generated C compiles under GNU89/Werror/ASan/UBSan and all injected cases pass.
2. Inspect exact-head XY-C1 build/TRX/artifact and other source-validation jobs for product commit 30af417ed8d7665e5da0f630e79b38bbb9f92296. Count L3 only if Build XY-C1, TRX verification and artifact upload all ran and passed on that same SHA.
3. If integration passes, extend injected coverage to primary NAT/filter backend stages and rollback ordering, or pursue another concrete userland reliability/security defect.
4. Continue source → regression → atomic commit → exact-SHA Actions without waiting on unrelated pending work.

## Verification boundary

- Parent cbe06d3 MiniUPnP #44804109446 FAIL only at the new fixture-generation step; earlier policy and delete source checks passed. Its AI run #44804109316 was also triggered before the fixture source was corrected.
- Commit 72b MiniUPnP #44803703734 PASS; AI #44803703663 PASS.
- Product commit 30af417ed8d7665e5da0f630e79b38bbb9f92296 has fresh exact-SHA MiniUPnP, XY-C1, process inventory, NVRAM and handoff Actions. Results pending at this checkpoint.
- MIPS/uClibc firmware L3 and physical-router L4 remain NOT VERIFIED until their independent gates pass.

## Exact first action for the next GPT

Fetch live main and poll exact-SHA MiniUPnP, XY-C1, process inventory, NVRAM and AI runs. Inspect the injected test step log if it fails; verify Build XY-C1, TRX and artifact steps individually. Then continue implementation without repeating completed repository recovery.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20 entries.
- Update the roadmap only for a material module-level priority or verification change.
- Host regression, same-SHA firmware L3, and physical hardware L4 are distinct evidence levels.
