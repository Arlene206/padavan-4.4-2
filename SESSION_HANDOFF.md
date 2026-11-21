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

- Deferred WAN reconnect fix: `136db90e2f84513d9bf3da297802b708b0811b44`. `auto_wan_reconnect_pause()` now uses checked `di_recon_pause` reading; backend/parse failure returns before any WAN stop/start while absent key retains zero-second compatibility.
- DHCP WINS fix: `1770074742e98f13155fab91703fc74ca975c248`. `prepare_dns_dhcpd()` snapshots `dhcp_wins_x` using `nvram_get_checked()` before DHCP-pool repair/generated-file changes, aborts on backend/inconsistent/oversized results, and reuses the private bounded value for option 44. Exact-head DNSMasq regression passed on this SHA.
- The WAN route workflow passed on `0097f377a1155be4ec09c2fe38334aa985c39b2b`; DNSMasq lifecycle passed on `1770074742e98f13155fab91703fc74ca975c248`; AI handoff docs passed on `b40c3ec707896cd119d513775ef6d224049ea0d3`. XY-C1, USB modem, Firewall and Process checks on `b40c3ec...` failed at the same source-check assertion: `pause_error` still contained spaces even though the function body had been stripped of whitespace. This commit normalizes that expected text too.


## Active task

1. Inspect exact-head XY-C1, USB modem identity, Firewall and Process lifecycle results after the checker whitespace fix; diagnose any new failures from their logs.
2. Require Build XY-C1, TRX verification and artifact upload to PASS on one exact product-source SHA covering the WAN reconnect and DHCP WINS fixes. Do not mark L3 while any step is skipped or failed.
3. Continue an independent whole-userland stability/security fix while unrelated CI is pending; don't re-audit completed FTP, storage, WAN source or DNSMasq changes.


## Verification boundary

- L1: FTP startup/restart and delayed automatic WAN reconnect fail closed on enable/delay NVRAM read errors; DNSMasq explicit DHCP WINS reads are checked before staging and use a stable bounded snapshot.
- L2: WAN route status and DNSMasq lifecycle passed on their exact heads; AI handoff documentation passed on `b40c3ec707896cd119d513775ef6d224049ea0d3`. The new WAN caller source-check still failed on `pause_error` whitespace in dependent suites; the check is corrected in this commit and exact-head reruns are pending.
- L3: NOT VERIFIED. XY-C1 on `b40c3ec` stopped before firmware compilation in the source checker; Build, TRX and artifact steps have not passed on one exact product-source SHA.
- L4: Physical router reconnect timing, DHCP/WINS lease behavior and runtime failover remain NOT VERIFIED.


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

Fetch live main; inspect exact-head XY-C1, USB modem identity, Firewall and Process lifecycle reruns for `check_modem_wan_start_status.py`. Then require Build + TRX + artifact on the same product-source SHA, and continue an independent reliability/security issue.
