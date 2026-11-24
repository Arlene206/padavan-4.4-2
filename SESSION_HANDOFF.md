## Current repository state

- Active development branch: `main`. Source parent for this DNS Forwarder procfs correction: `fd81fc8c6863f2ca59d3610dfead53db076173ee`; this atomic commit updates daemon discovery, regression, checker, workflow filters and handoff together. Resolve live `main` before every later write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; keep it read-only. Legacy PR #1 is closed and unmerged.
- Use one complete Git tree and one commit per logical task, then a non-forced expected-SHA update of `main`.

## Active task

### DNS Forwarder process-state confirmation

- `dns_forwarder_process_state()` accepted `pidof dns-forwarder` status 1 with empty output as confirmed ABSENT, although BusyBox may also return 1 after incomplete procfs enumeration. `func_start()` could launch a duplicate daemon when process discovery was unreliable.
- The fix sources `/usr/lib/process_lifecycle.sh` and cross-checks ambiguous pidof absence with shared `pl_process_state dns-forwarder`. Only shared-helper ABSENT is accepted; a procfs-visible matching task without a usable pidof PID list, or an incomplete/unavailable procfs scan, returns UNKNOWN. Start and stop fail closed.
- Regression injects a hidden `4242 (dns-forwarder)` procfs stat entry and a missing-procfs-root case; both must block start and stop without changing launch or signal counts. The Python checker pins helper inclusion/installation; workflow filters include the helper and Makefile.

### Previously completed RAETH lock-balance fix

- Commit `522210d812578bb0d64b546a7ea8b3fb1b7dfe15` restores `page_lock` before the common epilogue when `RAETH_SET_LAN_IP` rejects malformed input. Its exact-head Switch CLI #45421759200, AI handoff #45421759120 and NVRAM #45421759308 passed. Do not infer firmware L3 from host checks.

## Verification boundary

- Parent `fd81fc8c6863f2ca59d3610dfead53db076173ee` includes the latest SmartDNS source/test follow-up. This DNS Forwarder regression and source checker have not yet run on the new commit; inspect exact-head DNS Forwarder launch-path, Process inventory, AI handoff and XY-C1 Actions.
- L3 requires Build, TRX verification and artifact upload all PASS on the same product-source SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head DNS Forwarder launch-path, Process inventory, AI handoff and XY-C1 Actions for the procfs confirmation fix; diagnose any failure.
2. If XY-C1 remains pending, continue an independent stability/security slice rather than waiting. Do not claim L3 until Build/TRX/artifact all pass on one source SHA.
3. Continue outside DNS Forwarder after this fix is verified; do not repeat completed RAETH lock-balance or SmartDNS write-failure work without new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live `main`, exact-head relevant push Actions, validation boundary, blockers and the precise next task. Update only when resumable facts materially change.

# Cross-session execution mandate

Recover once, then implement. Keep work source-led: a bounded source fix, meaningful regression, necessary validation, one complete Git tree and one atomic main commit, followed by a non-forced compare-and-swap. Pending CI does not block independent work. Before context exhaustion, finish the safe slice and leave the next GPT one precise first action. Never force-push, use work branches/PRs, or overstate host, firmware or hardware evidence.
