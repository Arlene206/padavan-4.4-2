# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `8c79215790b8e56300d0c07201d62c90dfcd76e2`. FTP enable-state reads now fail closed in startup/restart; its exact-head vsftpd, Process inventory, NFS, firewall, Samba, HTTPD, USB modem and handoff checks passed. Process lifecycle passed; storage hotplug and the repository-wide NVRAM audit were still running, and XY-C1 was in Build.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Current source slice

- Parent main SHA: `ae4ae4a57b2c34d6dccf2541161a51606170047c`; current source contains Samba checked global-enable/mode/WINS state and guarded WAN DHCP publication/rollback.
- FTP enablement now uses a checked NVRAM read, strict integer parsing and explicit 0/1 validation while preserving the historical missing-key default of enabled. Both `run_ftp()` and `restart_ftpd()` refuse side effects after an unreadable enable flag; restart checks before inventory and stop. This commit adds GNU89 policy tests and source-contract coverage.
- `auto_wan_reconnect_pause()` previously passed `nvram_get_int("di_recon_pause")` directly into `try_wan_reconnect()`. A transient read/parse failure therefore became zero and immediately stopped/restarted WAN services instead of honoring the configured delay. It now performs the shared checked integer read first, preserves the missing-key zero default, and aborts before any reconnect side effect on read failure; the WAN watchdog source contract pins this ordering.
- Recovery commit `954d1dc4d11e5518e6e69e86a167fcfd0e5efd33` restored the full repository tree after a transient concurrent partial-tree update. Main remains the only development branch; backup is immutable.
- This source commit requires same-SHA XY-C1 Build + TRX verification + artifact upload. L3/L4 are NOT VERIFIED until their full gates pass.


## Active task

1. Inspect the exact-head WAN route status workflow for the checked `di_recon_pause` preflight; resolve any checker failure without relaxing coverage.
2. Inspect exact-SHA XY-C1 Build, TRX verification and artifact upload for source SHA `8c79215790b8e56300d0c07201d62c90dfcd76e2`; L3 is not established until all three pass on the same source SHA.
3. Continue the next independent whole-userland stability/security defect; do not repeat the FTP or captured-mode storage fixes.


## Verification boundary

- L1: FTP startup/restart now fail closed on enable-state NVRAM errors; delayed automatic WAN reconnect now also fails closed on delay-read errors and preserves the absent-key zero default.
- L2: FTP exact-head regression and Process inventory passed on `8c79215790b8e56300d0c07201d62c90dfcd76e2`. Deferred reconnect GNU89 policy regression and exact-head WAN route check are pending on the next source commit.
- L3: NOT VERIFIED for the delayed reconnect change until Build XY-C1, TRX verification and artifact upload pass on one exact product-source SHA.
- L4: Physical router reconnect timing, DHCP recovery and WAN failover remain NOT VERIFIED.


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

Fetch live main; inspect the exact-head WAN route status result for the checked delayed-reconnect NVRAM preflight and inspect XY-C1 Build/TRX/artifact for the FTP source SHA. Then continue an independent reliability/security defect outside the completed storage and FTP slices.
