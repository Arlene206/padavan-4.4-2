# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this checker correction: edeaf28ec35ad24aebaf6b1a82272cfe72939fd5. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807 and read-only.
- Exact-head edeaf Firewall Startup 43939885812 passed firewall startup policy and core firewall restore transaction fault-injection tests, then failed in the canonical checker because an older assertion still expected restart_firewall() to return upnp_status directly. XY-C1 43939886057 hit the same checker at step 16 and skipped Build/TRX/artifact; all earlier XY host regressions passed.
- Exact-head edeaf AI handoff 43939886029 and NVRAM 43939885658, Process, HTTPD and other independent jobs were in progress at last observation. Many other service and security jobs had passed.
- Prior 10ab Firewall Startup 43936882423, Process 43936882692, Storage 43936881877, AI docs 43936882570, WAN route 43936881887, USB modem identity 43936881865, HTTPD security 43936882544, AliDDNS 43936881919 and DNSMasq 43936881899 passed.
- Historical PR #1 remains closed/unmerged; do not resume.

## Active task

start_firewall_ex() returns EBUSY/EINVAL/EIO for unresolved recovery, unsafe interface tokens and core/extension restore or cleanup failures. Failed rollback still disables forwarding and retains snapshots. restart_firewall() always attempts UPnP refresh and returns the first nonzero firewall/UPnP result while restoring the core errno.

The transaction fault-injection test is wired into Firewall Startup CI and passed on edeaf. The remaining exact-head failure is a stale checker assertion expecting return upnp_status; that assertion is being aligned to first-error aggregation. After green validation, continue inspecting direct firewall callers and extension-hook result boundaries.

## Verification boundary

- c3 source commit changed core firewall status propagation. c3 exact-head Firewall Startup 43938857743 and XY-C1 43938857763 were blocked by the first stale checker signature; that was corrected in edeaf.
- On edeaf, Firewall Startup 43939885812 passed the new transaction failure-injection regression and firewall policy regression, then hit a second stale checker assumption about the old UPnP-only return value. XY-C1 43939886057 stopped at the same source checker before firmware Build/TRX/artifact; all prior host checks passed.
- This commit updates that checker to require firewall and UPnP first-error aggregation. No full firmware result or physical-router validation is claimed. L4 remains NOT VERIFIED; frozen backup unchanged.

## Exact first action for the next GPT

1. Inspect exact-SHA Firewall Startup, XY-C1 Build/TRX/artifact, NVRAM, Process, Storage and AI handoff runs after the UPnP checker correction.
2. If the canonical checker or the transaction regression still fails, fix the first causal log. Once green, audit unchecked firewall extension-hook results and retain fail-closed rollback behavior.
3. Main-only complete-tree atomic commits with non-forced expected-SHA CAS. Frozen backup read-only; physical-router L4 NOT VERIFIED.
