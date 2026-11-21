# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `8c79215790b8e56300d0c07201d62c90dfcd76e2`. FTP enable-state reads now fail closed in startup/restart; its exact-head vsftpd, Process inventory, NFS, firewall, Samba, HTTPD, USB modem and handoff checks passed. Process lifecycle passed; storage hotplug and the repository-wide NVRAM audit were still running, and XY-C1 was in Build.
- The deferred WAN reconnect change is committed in `136db90e2f84513d9bf3da297802b708b0811b44`. Its first WAN-route run passed the GNU89/ASan/UBSan policy binaries and earlier source contracts but exposed one line-wrap-sensitive assertion in the new source checker; this checker-only commit normalizes whitespace. XY-C1 Build/TRX/artifact and the remaining independent checks for the source SHA are still pending.
- The first XY-C1 pre-build pass on the same product-source SHA then exposed an older assertion in `check_modem_wan_start_status.py` that still expected the removed unchecked `nvram_get_int("di_recon_pause")` expression. This checker follow-up now pins the checked read, failure return and reconnect ordering; no production change is made in this correction.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Current source slice

- Main at the start of this slice: 9341a1ce8e47c74787c4db5426ecdd49eab0392f.
- The prior exact-head FTP, Samba, Process, DNSMasq and storage checker work is recorded in the previous dev_log entries. A deferred WAN reconnect NVRAM preflight was added in commit `136db90e2f84513d9bf3da297802b708b0811b44`; its checker corrections are also on main.
- This slice addresses the remaining unchecked `dhcp_wins_x` NVRAM reads in `prepare_dns_dhcpd()`: explicit WINS address read errors could be mistaken for no explicit address and silently choose router-IP fallback.
- New checked string parser, output-preservation tests and source-order contract are committed atomically here; exact-head DNSMasq and XY-C1 actions pending.


## Active task

1. Inspect exact-head DNSMasq policy/source-contract Action for the checked `dhcp_wins_x` snapshot.
2. Require same-source-SHA XY-C1 Build, TRX verification and artifact upload to PASS before marking L3.
3. Continue the next independent whole-userland stability/security issue; do not re-audit completed FTP/Samba/storage fixes.


## Verification boundary

- L1: `dhcp_wins_x` is read through `nvram_get_checked()` before DHCP-pool or generated-file side effects; backend/inconsistent/oversized values abort preparation, absent key remains empty, and a private bounded snapshot is reused when writing option 44.
- L2: New GNU89 string policy regressions and exact source-order contracts are part of this atomic commit; exact-head result pending.
- L3: NOT VERIFIED until Build XY-C1, TRX verification and artifact upload PASS on one exact product-source SHA.
- L4: Physical DHCP client lease/WINS behavior and live router operation NOT VERIFIED.


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

Fetch live main; inspect the exact-head DNSMasq string-policy/source-order regression and XY-C1 Build/TRX/artifact for this DHCP WINS snapshot. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect the exact-head WAN route status result for the checked delayed-reconnect NVRAM preflight and inspect XY-C1 Build/TRX/artifact for the FTP source SHA. Then continue an independent reliability/security defect outside the completed storage and FTP slices.
