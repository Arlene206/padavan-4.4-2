# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Conntrack flush implementation parent: `cffc1497fda9f65d9a0b163bad5acc1723c23f81`; checker correction parent: `cffc1497fda9f65d9a0b163bad5acc1723c23f81`.
- Exact-head WAN route `43973861975` compiled/passed the injected conntrack flush policy test, all pre-existing C policy tests and WAN VLAN source check. It then failed one newly added source assertion at Python line 56 because the assertion expected a compact nested first-error return but used only whitespace-collapsing, not whitespace-removing, normalization. This commit fixes only that assertion.
- Exact-head Process `43973862332`, Firewall Startup `43973861431` and AI handoff `43973861519` passed. XY-C1 `43973861873` was queued at last poll.
- Previous exact `1f829937` XY-C1 run `43971856235` passed Build XY-C1, TRX verification and artifact upload. This L3 evidence predates cffc's conntrack API/source changes and is not reused.
- The implementation now returns errno-style status from `flush_conntrack_table()` and propagates it through PID 1 netfilter notifications, LAN teardown, WAN teardown and MAN DHCP deconfig. The injected test verifies target IP, full flush, EACCES propagation and invalid writer handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify it. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Verify the conntrack flush status API and all four callsites after this test-contract correction. Require WAN route source checks, Process, Firewall, AI handoff and XY-C1 exact-head results. Do not declare current firmware L3 until cffc-descendant same-SHA Build/TRX/artifact succeeds.

## Verification boundary

- Exact cffc WAN route run passed the new GNU89/ASan/UBSan conntrack policy regression; its only failure was the whitespace-sensitive Python source assertion.
- Process, Firewall and AI handoff passed on cffc. XY-C1 was queued at last poll; exact result must be fetched.
- Exact 1f829 XY-C1 L3 success does not validate cffc source changes. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head WAN route, XY-C1 and all other cffc-triggered workflows after the assertion correction. Diagnose real failures from their job logs and confirm same-SHA firmware Build/TRX/artifact.
2. Continue reviewing RC/network teardown for discarded operational status, tracing each callee/caller before changing an API.
3. Keep each logical task in one complete-tree atomic main commit using non-forced expected-SHA CAS; leave backup read-only and do not claim router L4 without physical hardware evidence.
