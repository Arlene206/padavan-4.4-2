# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Live main parent for this Samba config-mode slice: `c9eff6c602124abc3d389272d3f0a98bbc6d092d`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Radio product source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e`: Wireless/iNIC L2 plus XY-C1 #44897419694 Build + TRX + artifact all PASS (L3 VERIFIED).
- USB modem boot-selection + fail-closed NDIS/RAS removal source SHA `c58272b954ad175b16bb85879d3913d384035704`: USB #44898901905, Process #44898901784, AI docs #44898901797, WAN DHCP #44898901879, WAN teardown #44898901886 and NVRAM audit #44898901848 PASS. XY-C1 #44898901805 still IN_PROGRESS (Build running, TRX/artifact pending).
- OpenVPN TLS-crypt-v2 server key/umask fix source SHA `c9eff6c602124abc3d389272d3f0a98bbc6d092d`: private-key regression #44899519368 PASS, Process #44899519328/#44899519428 PASS, AI docs #44899519407 PASS. XY-C1 #44899519362 still IN_PROGRESS (Build step started; TRX/artifact pending).
- New distinct Samba defect fixed in current commit: `write_smb_conf_unlocked()` used an error-collapsing `nvram_get_int("st_samba_mode")`; backend failure became zero and invalid values silently published an incomplete header-only configuration. The checked reader distinguishes missing-key legacy zero from I/O/malformed/overflow errors; mode allowlist is exactly 0/1/3/4; writer rejects invalid mode before staging and preserves the last-good config. A GNU89 allowlist fixture and exact source contracts are wired into Samba workflow; new header is a XY-C1 production input.
- Frozen backup is immutable; legacy PR #1 closed/unmerged.

## Active task
- Inspect current exact-head Samba share-stability, Process lifecycle, AI handoff and XY-C1 push Actions for this commit.
- On failure, read the first causal log and fix the source/contract rather than relaxing mode allowlist or treating NVRAM errors as disabled.
- Continue to an independent P1 stability/security issue after this Samba slice; do not loop back to completed HTTPD/WAN/SkipDB work.

## Verification boundary
- Samba checked-mode source and C fixture committed; exact-head L2 pending. No local test run.
- OpenVPN source `c9eff6c6`: host OpenVPN/Process/AI checks pass; same-SHA XY-C1 pending.
- USB source `c58272b9`: host checks pass; same-SHA XY-C1 pending.
- Radio source `60ad278e`: L3 VERIFIED.
- Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Samba, Process, AI docs and XY-C1 Actions.
2. Diagnose the first causal test/contract failure; preserve checked-read and fail-closed mode behavior.
3. Inspect XY-C1 #44899519362 for OpenVPN L3 and #44898901805 for USB L3 separately; do not combine SHA results.
4. Continue a distinct P1 security/reliability issue outside the recently completed radio, USB and OpenVPN slices.

Before context exhaustion, update this handoff only when active task, exact-head state or verification boundary changes; no no-op commit.
