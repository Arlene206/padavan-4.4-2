# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository Arlene206/padavan-4.4-2; develop on main only.
- Current main at checkpoint preparation: 0c172fa0e5c7454af1052aa2fee84c2803d6e6b8. It includes the WAN publisher forward-declaration fix and the YAML/handoff corrections; the remaining WAN checker use-before-assignment is fixed in THIS COMMIT.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 is closed and unmerged. Do not reopen or develop through its branch.
- The previous product-source baseline was 9a25b71fb74932dc27063432b2f342d9724d60e2. Recent protected functionality includes checked optional-daemon settings, ttyd/SSH mode handling, WAN DHCP compare-and-publish and stale-callback rollback fencing.

### Relevant exact-SHA evidence

- Latest production-source SHA: `2bfb2aa93f525762685d49ad50ff89d498eacdde`. It restores the WAN DHCP state-publisher forward declaration after a prior accidental duplicate body caused MIPS compiler errors; the original single lower implementation retains compare-and-publish and guarded stale-writer restoration.
- XY-C1 run #44914789805 on product-source SHA `2bfb2aa` remained IN_PROGRESS at the last poll. Its Build XY-C1 step was running and TRX verification/artifact upload were pending. Do not claim L3 until all three pass on that same SHA.
- NVRAM transaction-boundary audit #44914790008: SUCCESS on product-source SHA `2bfb2aa`.
- Process lifecycle regression #44914789828 and Process inventory #44914790193: SUCCESS on `2bfb2aa`. HTTPD, Wireless, Firewall, WAN interface teardown, miniupnpd, VPN server, WAN route, WAN gateway ARP, ZeroTier, USB modem, watchdog, detect_link, detect_internet and DNSMasq workflows also PASS on the same SHA.
- AI handoff docs #44915003659: SUCCESS on exact main SHA `0c172fa`.
- WAN DHCP callback #44915003721: FAILURE on `0c172fa`; all earlier compile/policy checks passed, then the inline contract referenced `lease_policy` before its later initialization. This is a test ordering bug, not a production source failure. The policy-header/test reads are moved before first use in THIS COMMIT.
- L3 still depends on completion of XY-C1 Build, TRX verification and artifact upload on `2bfb2aa`; L4 physical router validation remains NOT VERIFIED.


## Active task

1. Inspect exact-head WAN callback, XY-C1, AI handoff and Process lifecycle Actions after THIS COMMIT. Fix the first causal failure without weakening checks.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same new product-source SHA before claiming L3.
3. After the build correction is validated, continue the next confirmed reliability issue. While Actions are pending, keep progressing on independent source/test work.

## Verification boundary

- L1: source review confirms the first WAN publisher is now a forward declaration and only the later implementation remains. The guarded stale-writer rollback logic is retained.
- L2: Process regression and process inventory PASS on 2bfb2aa; NVRAM audit PASS on the same SHA. AI handoff docs PASS on 0c172fa. WAN callback failed due to checker use-before-initialization; fixed in THIS COMMIT and requires exact-head rerun.
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

Fetch live main and inspect its exact-head WAN callback and AI handoff push runs. Recheck XY-C1 #44914789805 on product-source SHA 2bfb2aa, including Build/TRX/artifact; NVRAM #44914790008 already PASSed. Diagnose failures from the first causal job step and continue independent reliability work while the build is pending.
