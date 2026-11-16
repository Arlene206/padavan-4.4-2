# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this validation repair: 56da21749779b8de17d29acec7666808f0828269. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807 and read-only.
- Exact-head 56da Firewall Startup 43937616937 passed firewall policy/NAT WAN-gate tests and the new required-module C regression, then failed at the final canonical source checker. Cause: the SFE assertion expected the former local variable name sfe_module_ops; the shared module ops variable is module_ops. This commit corrects the assertion and adds module source-contract coverage.
- Exact-head 56da XY-C1 43937617363 and NVRAM 43937617096 remained IN_PROGRESS at last poll. Same-SHA XY-C1 Build/TRX/artifact is not yet verified.
- Parent 10ab Firewall Startup 43936882423, Process 43936882692, Storage 43936881877, AI docs 43936882570, WAN route 43936881887, USB modem identity 43936881865, HTTPD security 43936882544, AliDDNS 43936881919 and DNSMasq 43936881899 passed.
- Historical PR #1 remains closed/unmerged; do not resume.

## Active task

Configured PPTP/H.323/SIP/FTP conntrack/NAT helpers are post-load verified. The required-module policy attempts every independent requirement and preserves the first failure; FTP parameter-read failures and unload races return EIO/EBUSY rather than claiming the requested state was applied. Both RC NETFILTER/FIREWALL notification branches merge module reload status while still proceeding with firewall refresh.

Next P1 task: convert start_firewall_ex() to return actionable status for input validation, core table restore/rollback and snapshot cleanup. Preserve fail-closed forwarding when rollback is incomplete. Then aggregate core firewall status and UPnP refresh status in restart_firewall() without skipping either operation.

## Verification boundary

- Exact-head 56da Firewall Startup 43937616937 passed the firewall policy and required-NAT gate tests and the new module-policy regression, then failed in the source checker on the stale SFE local-variable name. This commit corrects the contract and adds coverage for configured ALG/FTP status propagation.
- Exact-head 56da XY-C1 43937617363 and NVRAM audit 43937617096 were in progress at last poll. New source SHA requires fresh exact-SHA Build/TRX/artifact verification.
- No full local firmware build or physical-router validation is claimed. Physical-router L4 remains NOT VERIFIED; frozen backup unchanged.

## Exact first action for the next GPT

1. Inspect exact-SHA Firewall Startup, XY-C1 Build/TRX/artifact, Process, Storage, and AI handoff runs for the validation repair.
2. Continue the start_firewall_ex() status-return slice, including failure injection for validation, core restore, rollback, and snapshot cleanup; then merge the first error with UPnP refresh in restart_firewall().
3. Use fresh main SHA, one complete Git tree/commit, and non-forced expected-SHA CAS. Keep the frozen backup read-only and router L4 NOT VERIFIED.
