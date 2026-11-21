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

- Main at the start of this slice: `956925458e646947d05930ab6ba00bf064f34482`; develop only on main; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains untouched.
- Deferred WAN reconnect delay: `136db90e2f84513d9bf3da297802b708b0811b44`; checked `di_recon_pause` errors abort before WAN stop/start; absent-key zero default is preserved.
- DHCP WINS snapshot: `1770074742e98f13155fab91703fc74ca975c248`; bounded checked `dhcp_wins_x` snapshot precedes DHCP config side effects.
- Physical-link DHCP renew retry: `939e06459c5cb83b563758c30fc7371141e673f1`; detect-link GNU89/ASAN/UBSAN policy and source contract passed.
- USB modem selector: `af16c58c166b915e8847d1e30325ccea5bc78460`; checked modem priority/Ethernet probe; USB workflow passed on checker descendant `772916f1f377917ba9c6262b7b421f61dc79e6b6`.
- New WAN bridge slice: checked bridge mode/isolation reads gate LAN/WAN bridge topology and WAN VLAN setup; active additional-port VID/priority pairs are checked before port probes or bridge mutations. Readable out-of-range values retain historical fallback; backend/malformed errors abort with original status. Added integer-range policy tests and exact source-order assertions; exact-head CI pending.


## Active task

1. Inspect exact-head WAN VLAN/bridge regression, USB modem, Process and Firewall results for the checked bridge-setting slice; diagnose concrete failures from logs without weakening contracts.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on one exact product-source SHA covering WAN reconnect, DHCP WINS, link-restoration retry, modem selection and bridge-read fixes.
3. Continue a separate whole-userland reliability/security defect while independent Actions are pending; do not repeat completed FTP/storage/DNSMasq work.


## Verification boundary

- L1: FTP startup/restart and delayed automatic WAN reconnect fail closed on relevant NVRAM errors; DHCP WINS uses a checked bounded snapshot; physical-link DHCP renew errors get two bounded retries; modem selection aborts on unknown priority/link state; WAN bridge mode, isolation and active additional-port VLAN settings abort before topology changes on read errors.
- L2: WAN route, DNSMasq, detect-link, USB modem and HTTPD policy checks passed on prior exact heads. This WAN bridge range-policy regression and checked-read source contracts are pending on the current main SHA.
- L3: NOT VERIFIED until XY-C1 Build, TRX verification and artifact upload all PASS on one exact product-source SHA.
- L4: Physical WAN reconnect, DHCP renewal, modem failover, switch VLAN and bridge isolation behavior remain NOT VERIFIED on hardware.


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

Fetch live main; inspect exact-head WAN route regression for checked bridge mode/isolation/port VLAN reads and GNU89/ASAN/UBSAN integer-range cases. Check XY-C1 Build + TRX + artifact on the product-source SHA, then continue one independent reliability/security issue.
