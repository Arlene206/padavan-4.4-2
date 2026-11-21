# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `067d41bbfc677e65867a3ce434c688a4a04f29f9`. The WAN DHCP NVRAM state publisher has one forward declaration before the bound callbacks and one implementation after the transaction get/set and guarded restore adapters.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Relevant exact-SHA evidence

- XY-C1 #44914789805 on `2bfb2aa93f525762685d49ad50ff89d498eacdde`: the firmware Build step was IN_PROGRESS at last poll; TRX verification and artifact upload were pending. This source SHA includes the compile fix that removed duplicate WAN publisher definitions. The present retry-semantics patch changes `net_wan.c`, so it requires a new same-SHA firmware run; do not substitute the older run.
- NVRAM transaction-boundary audit #44914790008: SUCCESS on `2bfb2aa`.
- Process lifecycle regression #44914789828 and Process inventory #44914790193: SUCCESS on `2bfb2aa`.
- WAN DHCP callback #44915189957: SUCCESS on exact `067d41bbfc677e65867a3ce434c688a4a04f29f9`.
- AI handoff documentation contract #44915189829: SUCCESS on exact `067d41b`.
- The prior WAN callback #44915003721 on `0c172fa` failed only from a use-before-assignment in its embedded source contract; the reads were moved ahead of first use, and the corrected exact-head callback passed on `067d41b`.
- Multiple independent regression workflows also passed on `2bfb2aa`: HTTPD, Wireless, Firewall, WAN teardown, miniupnpd, VPN, WAN route, gateway ARP, ZeroTier, USB modem, watchdog, detect_link, detect_internet and DNSMasq.

## Active task

1. Verify the current source patch's exact-head WAN callback and XY-C1 firmware jobs.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on one SHA before claiming firmware L3.
3. Inspect the NVRAM audit on the current source SHA, then continue the next independent reliability/security defect.

## Verification boundary

- L1: lease tuple publication is compare-and-publish under the NVRAM transaction; stale tuple conflicts return `EAGAIN`; recovery is fenced against a newer writer. This patch further avoids a redundant fresh restore when in-transaction compensation succeeded.
- L2: Process lifecycle/inventory, NVRAM audit and WAN callback each have PASS evidence on the preceding source/checker SHAs noted above. The current production-source patch still needs exact-head validation.
- L3: NOT VERIFIED for the current source until XY-C1 Build, TRX verification and artifact upload all pass on that same SHA.
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

Fetch live `main`, inspect exact-head WAN callback and XY-C1 Build/TRX/artifact results, then inspect NVRAM audit status. Fix the first causal failure without weakening regression coverage; keep independent source work moving while Actions are pending.
