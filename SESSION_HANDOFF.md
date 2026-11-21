# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `1770074742e98f13155fab91703fc74ca975c248`. The product-source changes now on main include the deferred WAN reconnect fail-closed NVRAM preflight (`136db90e2f84513d9bf3da297802b708b0811b44`) and the checked DHCP WINS address snapshot committed in `1770074742e98f13155fab91703fc74ca975c248`.
- WAN route status and AI handoff documentation passed on `0097f377a1155be4ec09c2fe38334aa985c39b2b`; DNSMasq lifecycle passed on `1770074742e98f13155fab91703fc74ca975c248`. The XY-C1 / USB modem / Firewall / Process checks subsequently exposed one test-only mismatch in `check_modem_wan_start_status.py`: `compact()` preserves single spaces while its new assertion used no-space tokens. This correction strips whitespace from the extracted function before the order checks.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Current source slice

- Main at the start of this slice: `af16c58c166b915e8847d1e30325ccea5bc78460`.
- Deferred WAN reconnect delay read: `136db90e2f84513d9bf3da297802b708b0811b44`; checked `di_recon_pause` errors abort before WAN stop/start and the absent-key zero default is preserved.
- DHCP WINS snapshot: `1770074742e98f13155fab91703fc74ca975c248`; the checked bounded `dhcp_wins_x` value is captured before config staging and reused for option 44; its exact-head DNSMasq regression passed.
- WAN physical-link DHCP renew retry: `939e06459c5cb83b563758c30fc7371141e673f1`; exact-head detect-link signal regression and source contract passed.
- USB modem selector: `af16c58c166b915e8847d1e30325ccea5bc78460`; `modem_prio` and `wan_src_phy` now use checked integer reads, and priority-2 modem selection aborts on an unknown Ethernet link rather than treating probe failure as link absence. Its GNU89/ASAN/UBSAN policy binary passed, but the first exact-head USB workflow run failed in the new source assertion because its expected checked-reader call did not normalize a multiline C expression. This commit corrects only that assertion; production source and test behavior are unchanged.


## Active task

1. Inspect exact-head USB modem identity, WAN route, Process and XY-C1 results after the whitespace-normalized selector assertion; resolve any remaining failures from their logs without weakening the contracts.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on one exact product-source SHA containing the WAN reconnect, DHCP WINS, link-restoration retry and USB modem-selection fixes.
3. Continue independent whole-userland stability/security work while CI is pending; do not re-audit completed FTP, storage or DNSMasq slices.


## Verification boundary

- L1: FTP startup/restart and delayed automatic WAN reconnect fail closed on relevant NVRAM read errors; DHCP WINS is checked before config staging; failed physical-link DHCP renew signals receive two bounded retries; modem selection aborts on unknown priority/Ethernet state without publishing a speculative transport.
- L2: WAN route, DNSMasq and detect-link regressions passed on prior exact heads. On `af16c58c`, the new modem selector GNU89/ASAN/UBSAN policy binary passed; the USB workflow then failed at a whitespace-sensitive source assertion before its source contract completed. The assertion is corrected in this commit; exact-head rerun is pending.
- L3: NOT VERIFIED until XY-C1 Build, TRX verification and artifact upload all PASS on the same exact product-source SHA. The XY-C1 Build associated with `af16c58c` was still in progress at this handoff update.
- L4: Physical WAN reconnect, DHCP lease renewal, modem failover timing and router runtime remain NOT VERIFIED.


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

Fetch live main; inspect the exact-head USB modem workflow and WAN route / Process / XY-C1 checks for the normalized multiline selector assertion. Confirm the modem policy tests and source contract pass, then require Build + TRX + artifact on the same product-source SHA.
