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

- Main at the start of this slice: `b846d1035e0702815b360c1caa8c83a1b5a6980c`.
- Deferred WAN reconnect NVRAM preflight: `136db90e2f84513d9bf3da297802b708b0811b44`. `auto_wan_reconnect_pause()` uses the checked `di_recon_pause` read and aborts before WAN stop/start on backend/parse failure; absent key retains the zero-second default.
- Explicit DHCP WINS fix: `1770074742e98f13155fab91703fc74ca975c248`. `prepare_dns_dhcpd()` reads a bounded checked `dhcp_wins_x` snapshot before generated-config side effects and reuses it for option 44; exact-head DNSMasq lifecycle passed.
- WAN DHCP link-restoration fix in this slice: return normalized `renew_udhcpc_wan()` errors to `detect_link`, retry twice at four-second intervals, reset on down/reset/success, and log exhaustion. GNU89 ASAN/UBSAN policy coverage and workflow path/source contracts are added; CI and target-router behavior are pending.


## Active task

1. Inspect exact-head detect_link signal/retry policy, XY-C1, USB modem identity, Firewall and Process results; fix failures from logs without weakening source contracts.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same exact product-source SHA covering the WAN reconnect NVRAM, DHCP WINS and link-restoration retry fixes.
3. Continue an independent whole-userland reliability/security fix while CI is pending; do not re-audit completed FTP, storage or DNSMasq slices.


## Verification boundary

- L1: FTP startup/restart and delayed automatic WAN reconnect fail closed on relevant NVRAM read errors; explicit DHCP WINS reads are checked before staging and use a bounded snapshot; physical-link DHCP renew signal errors now receive two bounded retries.
- L2: WAN route status and DNSMasq lifecycle passed on exact heads. The reconnect caller source-check was corrected in `b846d103`; dependent exact-head jobs and the new detect_link GNU89/ASAN/UBSAN tests/source contract are pending.
- L3: NOT VERIFIED until XY-C1 Build, TRX verification and artifact upload PASS on one exact product-source SHA.
- L4: Physical WAN reconnect timing, DHCP/WINS lease behavior and DHCP renew delivery remain NOT VERIFIED.


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

Fetch live main; inspect exact-head detect_link retry-policy, XY-C1, USB modem, Firewall and Process checks. Confirm GNU89 ASAN/UBSAN and source contracts pass, then require Build + TRX + artifact on one product-source SHA.
