## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- This WAN-monitor readiness slice starts from parent `3bd150afbea9a910407002a9ba933dd9257ab5cc`. The exact-head udhcpc checker correction is clean: run #46153182409 PASS; AI handoff docs #46153182404 PASS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store bootstrap is already integrated.

## Active task in this commit

- Fix detached WAN gateway monitor startup status. `detect_wan_main()` calls `daemon()` before publishing its PID file; the daemonizing parent can exit successfully even if the child subsequently fails PID-file publication and exits.
- The previous WAN-up path treated `eval("detect_wan") == 0` as proof of readiness. This commit moves launch into `start_detect_wan_with_readiness()`, then requires a bounded tri-state `process_inventory_scan("detect_wan")` postcondition through the shared `detached_daemon_start_postcondition_with_ops()` policy. UNKNOWN and timeout errors propagate; sleep resumes the remaining interval after EINTR.
- The existing `detached_daemon_start_policy.c` GNU89/ASan regression already covers delayed appearance, child-absent timeout, UNKNOWN fail-closed, launcher failure and interrupted waits. The WAN integration checker requires the production call order and those regression cases.

## Exact-SHA verification checkpoint before this commit

- Main SHA `3bd150afbea9a910407002a9ba933dd9257ab5cc`: udhcpc event-script status stability #46153182409 PASS; AI handoff docs #46153182404 PASS.
- USB storage hotplug #46152870433 PASS on `1d74edb7b08fe3460109f351dd14c067caab8ba7`; subsequent commits in this chain changed the udhcpc checker/docs only.
- XY-C1 firmware validation #46152329717 completed SUCCESS on `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce`; the Build XY-C1, TRX verification and artifact upload steps were all successful. This is the earlier production-source L3 baseline, not verification of the present `net_wan.c` change.
- Earlier udhcpc checker failures #46153023184/#46152870458 were corrected by exact descendant commits and the final udhcpc run above PASS. Physical-router L4 remains NOT VERIFIED.

## Verification boundary

- L1: the shared detached-daemon policy host regression is wired into `.github/workflows/wan-route-status.yml`; exact-head rerun for this commit pending.
- L2: WAN route status and WAN ARP socket workflows must pass on this exact source SHA; exact-head run pending.
- L3: this `net_wan.c` change requires a fresh XY-C1 Build, TRX verification and artifact upload from one exact SHA; no transfer from `4652b68`.
- L4: physical WAN/gateway-loss and router runtime behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Read the live `main` head and inspect exact-SHA WAN route status, WAN ARP socket, AI handoff and XY-C1 Actions for this change. Diagnose any failure from its exact job log; do not treat a pending firmware run as a reason to wait.
2. If these checks pass, move to an independent high-priority userland stability/security defect; do not reopen the already-completed udhcpc/storage checker work.
3. Keep the backup branch immutable and use complete-tree single-commit writes with a non-forced expected-SHA update.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
