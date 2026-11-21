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

- Deferred WAN reconnect NVRAM preflight: `136db90e2f84513d9bf3da297802b708b0811b44`. The function uses the checked `di_recon_pause` reader and aborts before WAN stop/start on backend/parse failure; absent key retains the zero-second default.
- Explicit DHCP WINS fix: `1770074742e98f13155fab91703fc74ca975c248`. The checked `dhcp_wins_x` snapshot is validated before generated DHCP config side effects and reused when emitting option 44; exact-head DNSMasq workflow passed.
- WAN caller source-checker failures are test-only. The normalized-body assertion successively exposed spaces in the diagnostic, return and reconnect-call tokens; this patch normalizes the last token to `returntry_wan_reconnect(1,pause_seconds);`. Prior C policy binaries and earlier source contracts passed before reaching this final assertion. Exact-head dependent reruns are pending.


## Active task

1. Inspect exact-head XY-C1, USB modem identity, Firewall and Process checks after the normalized reconnect-call token fix; investigate any remaining failures from logs.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same exact product-source SHA, including the WAN reconnect and DHCP WINS commits.
3. Continue independent whole-userland reliability/security work while CI is pending; do not repeat the completed DNSMasq/FTP/storage slices.


## Verification boundary

- L1: FTP startup/restart and delayed automatic WAN reconnect fail closed on relevant NVRAM read errors; explicit DHCP WINS reads are checked before staging and use a stable bounded snapshot.
- L2: WAN route status and DNSMasq lifecycle passed on exact heads. On `9b7eecf7`, AI handoff docs passed but XY-C1, USB modem, Firewall and Process stopped at the remaining whitespace-sensitive `pause_start` token. This checker correction is queued for exact-head reruns.
- L3: NOT VERIFIED until Build XY-C1, TRX verification and artifact upload PASS on one exact product-source SHA; the prior XY-C1 jobs stopped in source checks before firmware compilation.
- L4: Physical WAN reconnect timing, DHCP/WINS lease behavior and router runtime remain NOT VERIFIED.


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

Fetch live main; inspect exact-head XY-C1 / USB modem / Firewall / Process checks for the normalized reconnect-call token. Confirm the checker passes end-to-end, then require Build + TRX + artifact on one product-source SHA and continue a separate reliability/security issue.
