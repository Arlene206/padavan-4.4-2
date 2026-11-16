# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main. Current parent for WAN source-contract repairs: c3bcb486aa97e0dbb478f05585e6c24ab33407bc; production conntrack source is in cffc1497fda9f65d9a0b163bad5acc1723c23f81.
- Exact cffc WAN route 43973861975 passed the injected conntrack C regression and existing route tests, then failed a whitespace-sensitive status assertion. Follow-up c3b normalized that first assertion, but exact c3b WAN route 43974249410 exposed a second stale check at Python line 304 that still expected the pre-conntrack return value from udhcpc_deconfig(). Production now correctly aggregates interface, conntrack and cleanup statuses.
- Exact c3b Process 43974249384 and AI handoff 43974249444 passed. cffc Firewall Startup 43973861431 passed. XY-C1 43973861873 was still IN_PROGRESS at last poll; current-source firmware L3 remains NOT VERIFIED until a matching build/TRX/artifact pass.
- The conntrack API now returns fput_string() errno-style status; PID 1, LAN, WAN and MAN teardown preserve/merge it while attempting later independent cleanup. The focused GNU89/ASan/UBSan policy test passed in the WAN route run before its final source-contract assertion failed.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. Physical-router L4 remains NOT VERIFIED.

## Active task

Finish repairing the WAN route integration contract for the conntrack flush API. The current failure is a stale legacy assertion in the later udhcpc_deconfig() check: it still expects route_status_accumulate(interface_status, cleanup_status) and must assert the nested aggregation including conntrack_status. Re-run exact-head WAN route; address any further genuine assertion failures rather than weakening production behavior.

## Verification boundary

- The cffc WAN route run compiled and passed the injected conntrack policy test, existing C policy regressions and WAN VLAN checks; c3b WAN route failed only at the second stale deconfig assertion in the source checker. This commit repairs that contract; exact-head WAN route rerun is required.
- Exact c3b Process 43974249384 and AI handoff 43974249444 passed; cffc Firewall Startup 43973861431 passed. cffc XY-C1 43973861873 remained IN_PROGRESS at last poll. Do not claim current-source L3 without same-product-source Build/TRX/artifact evidence.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main; inspect WAN route regression after this source-contract correction and finish all remaining exact-head checks. Use failed-step logs to distinguish stale assertions from real behavior defects.
2. Continue an independent RC/network stability fix once the conntrack validation gate passes; do not wait on queued firmware CI.
3. Keep complete-tree atomic commits, expected-SHA non-forced main updates and the frozen backup unchanged. Hardware L4 requires device evidence.
