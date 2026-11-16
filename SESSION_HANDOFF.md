# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Main immediately before this log-integrity and CI-contract repair: `7b82bc0826ebf93c66b6ed42811990222f2b98e5`.
- Latest product-source commit: `b2d3288b6282c16cf23859eb9e7f8fdce083cd62` (full LAN restart first-error aggregation). Commits `f66dc26`, `7ac9582`, and `7b82bc0` after it changed workflow/test contracts and handoff records, not firmware production source.
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, move, rewrite or delete.
- Bootstrap, development protocol, handoff, roadmap, archive index, branch inventory, backup marker and legacy PR #1 have already been recovered. PR #1 is closed; valid AliDDNS changes are on main. Do not repeat that audit.
- The current root log had been truncated to three records by the LAN restart checkpoint sequence. The parent `2c75de8133751e8063ded244faa225011f2891b6` retained 61 complete records; THIS COMMIT restores those records verbatim after the three newer records, then adds this repair entry. The resulting root window remains within 20–119 records, without touching archived shards or indexes.

## Active task

The product-source slice `b2d3288` changed `full_restart_lan()` to retain the first nonzero result from status-bearing stop, restoration, restart, reload, start and HTTPD logout operations. It reuses `rc_first_service_start_status()`, preserves the DNSMasq lock/start gates and VPN rollback order, extends the GNU89/Werror/ASan/UBSan policy regression, and merges the aggregate into PID 1's LAN notification result.

Validation and CI repair status:
- The LAN policy regression passed in the Process regression and XY-C1 pre-build jobs. Process path-filter YAML was repaired at `f66dc26260ba426245ad403b6703b5ba6d52336a`; the source-order checker was aligned at `7ac958281702b5d3b39284a3666a90b17e01f75f`, and its exact-head Process run `43815025586` passed.
- Exact product-source XY-C1 run `43814395678` on `b2d3288` completed successfully through Build XY-C1, TRX verification and artifact upload. This is the current L3 evidence; the later commits are test/documentation-only.
- Exact b2 DNSMasq lifecycle `43814395657` and HTTPD security `43814395687` both failed because their embedded source contracts still demanded the previous `full_restart_lan()` returns (`return dnsmasq_status;`, `return dnsmasq_lock_errno;`) after production was correctly changed to aggregate and return `restart_status`. THIS COMMIT updates both contracts to assert the new failure-recording behavior, including the VPN restore result after lock acquisition failure.
- Exact 7b AI handoff run `43815228477` failed because the root `dev_log.md` contained only three records, below the validator's required minimum of 20. THIS COMMIT restores the 61-record parent log verbatim, preserving the three newer records and all archive shards/indexes.
- Exact 7b infosvr `43815228434`, Process inventory `43815228506`, AliDDNS `43815228432`, AliyunDrive `43815228496`, and AdGuardHome `43815228480` passed. NVRAM run `43815228491` was still in progress at the last check.

## Verification boundary

- Exact `e2a43d79ca372a70af9262e1b0463945d64d644c`: AI handoff `43815932791`, Process lifecycle `43815932847`, HTTPD security `43815932882`, AliyunDrive `43815932706`, AliDDNS `43815932894`, AdGuardHome `43815932884`, infosvr `43815932852`, and NVRAM `43815932797` PASS. DNSMasq `43815932909` reached its embedded source contract but failed while finding the old lock-return statement. Follow-up `78af5220da96994440003d1e794ea948457dbd5f` corrected the assertion target but accidentally removed YAML block indentation: its exact workflow `43816311414` failed before creating a job. THIS COMMIT restores the Python indentation. XY-C1 `43815932794` was still IN_PROGRESS at last e2 observation; exact b2 XY-C1 `43814395678` later completed Build, TRX verification and artifact upload SUCCESS.


- L1/L2 source and policy regression: PASS on the exact b2/7ac evidence above.
- L3 firmware: PASS on product-source SHA `b2d3288b6282c16cf23859eb9e7f8fdce083cd62`, XY-C1 run `43814395678` — Build XY-C1, TRX verification and artifact upload all succeeded. The exact source is unchanged by later CI/documentation commits.
- Exact 7b AI handoff `43815228477` is FAIL solely at the root log-retention check (3 records found); the retained-record restoration is in THIS COMMIT and must be checked on its new exact SHA.
- Exact b2 DNSMasq `43814395657` and HTTPD `43814395687` failed at stale source assertions; their corrected workflow contracts are in THIS COMMIT and must be rerun.
- NVRAM `43815228491` remained IN_PROGRESS at last observation; do not claim a result before checking it.
- No physical-router L4 validation has been performed.

## Exact first action for the next GPT

1. Check the exact-head DNSMasq lifecycle, AI handoff, and XY-C1 Actions for THIS COMMIT. Confirm the embedded Python source checker executes with the corrected YAML indentation and validates the real lock-failure rollback source; diagnose first causal failure if any. AI log-retention should remain PASS after restoration.
2. Recheck the pending NVRAM run `43815228491` only as a useful checkpoint; do not let unrelated pending checks block independent work.
3. Continue the P1 repository-wide RC service-result audit outside `full_restart_lan()`: trace enabled notification/startup callers through helper return values and cleanup/rollback paths; implement only confirmed dropped results with caller-aware regression coverage.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
