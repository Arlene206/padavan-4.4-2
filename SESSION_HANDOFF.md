# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository Arlene206/padavan-4.4-2; develop on main only.
- Current main at checkpoint preparation: 2bfb2aa93f525762685d49ad50ff89d498eacdde. This source commit restores the WAN DHCP state-publisher forward declaration and keeps one implementation; it is awaiting exact-SHA CI and XY-C1 build validation.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 is closed and unmerged. Do not reopen or develop through its branch.
- The previous product-source baseline was 9a25b71fb74932dc27063432b2f342d9724d60e2. Recent protected functionality includes checked optional-daemon settings, ttyd/SSH mode handling, WAN DHCP compare-and-publish and stale-callback rollback fencing.

### Relevant exact-SHA evidence

- Current product-source SHA: `2bfb2aa93f525762685d49ad50ff89d498eacdde`. It removes the accidental duplicate `wan_dhcp_nvram_state_publish` definition at the earlier forward-declaration location and preserves the single later compare-and-publish/guarded-restore implementation.
- The previous XY-C1 run #44913834107 on SHA `9a25b71` failed at MIPS compilation because that earlier declaration had been replaced by a second full definition. TRX verification and artifact upload were skipped. The new exact-head XY-C1 run #44914789805 is IN_PROGRESS on `2bfb2aa`; Build was running and TRX/artifact remained pending at the last inspection.
- NVRAM transaction audit #44914790008 is IN_PROGRESS on exact SHA `2bfb2aa`.
- Process lifecycle regression #44914789828 and process inventory #44914790193 both PASS on exact SHA `2bfb2aa`.
- HTTPD authentication #44914789741, Wireless radio #44914789770, Firewall #44914789954, WAN interface teardown #44914789922, miniupnpd #44914789910, VPN server #44914789743, WAN route #44914789727, WAN gateway ARP #44914789915, ZeroTier #44914789950, USB modem #44914790028, watchdog #44914789925, detect_link #44914789773, detect_internet #44914789710 and DNSMasq #44914789700 all PASS on exact SHA `2bfb2aa`.
- AI handoff documentation run #44914790056 FAILED on `2bfb2aa` because this file lacked the exact cross-session rule phrase `one atomic main commit`; corrected in THIS COMMIT.
- WAN callback workflow run #44914788251 failed to load any job because the inline Python block contained one unindented assertion at YAML line 494; this broke the workflow's step block. Corrected by restoring the ten-space YAML script indentation in THIS COMMIT. A fresh WAN callback push run is required.
- Hardware L4 remains NOT VERIFIED. Do not claim L3 until Build XY-C1, TRX verification and artifact upload all PASS on the same exact product-source SHA.


## Active task

1. Inspect exact-head WAN callback, XY-C1, AI handoff and Process lifecycle Actions after THIS COMMIT. Fix the first causal failure without weakening checks.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same new product-source SHA before claiming L3.
3. After the build correction is validated, continue the next confirmed reliability issue. While Actions are pending, keep progressing on independent source/test work.

## Verification boundary

- L1: source review confirms the first WAN publisher is now a forward declaration and only the later implementation remains. The guarded stale-writer rollback logic is retained.
- L2: Process regression and process inventory PASS on 2bfb2aa. The WAN workflow and AI handoff check failed due to checker/document text issues fixed in THIS COMMIT; exact-head reruns are required.
- L3: NOT VERIFIED. XY-C1 run #44914789805 is still running on 2bfb2aa; require Build + TRX + artifact upload all PASS on that same SHA.
- L4: physical router flash, boot and runtime validation NOT VERIFIED.

## Workflow and preservation rules

- Main only. One logical slice, one atomic main commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected_sha. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
- Re-fetch main immediately before writing. On a moved ref, discard the stale prepared tree and rebuild from the new parent.
- Inspect push Actions only when head_sha exactly matches the commit. Diagnose failures from their logs. PENDING does not block independent work.
- Preserve security, architecture, ABI, GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Never weaken useful tests or focused workflow path coverage.
- Do not touch GPT Session Rotator workflow/state or replay SHA 9d5a5cb. The backup branch is permanent immutable recovery data.

## Cross-conversation documentation governance

- GitHub is the durable cross-conversation record; the one-line user entrypoint remains sufficient.
- dev_log.md stays newest-first with 20–119 complete records. Before record 120, archive the oldest 100 complete records verbatim into docs/dev-log/YYYY/MM/part-NNN.md in one batch, leaving 20 newest entries.
- Rotate early only above the 384 KiB target and never exceed 512 KiB. Preserve archive/index bytes. Git history is the version archive for handoff and roadmap; routine snapshots and SHA-chasing commits are prohibited.
- Update dev_log.md for real milestones, this handoff when first-next-action facts materially change, and REFACTOR_ROADMAP.md only when repository-wide module status/priorities materially change.
- Before context exhaustion, finish the safe slice, commit atomically, inspect exact-head Actions and leave the next precise task.

## Exact first action for the next GPT

Fetch live main and inspect the new exact-head WAN callback and AI handoff push runs after THIS COMMIT; recheck XY-C1 #44914789805 and NVRAM #44914790008 on 2bfb2aa. Diagnose first failures from their job logs and continue independent reliability work while builds are pending.
