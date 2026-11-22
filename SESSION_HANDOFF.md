# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `efb830bce5028536826989c8b5070b68a49e4a4e`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- EAPoL compile-breaking orphan tail removed; exact reruns for XY-C1 and Process pending.
- WSDD2 strict UTF-8/XML 1.0 validation and ttyd detached-child readiness are implemented; their corresponding CI checks passed on exact checker descendants e2 and 30a1.

## Active task

p910nd circular-buffer I/O now retries EINTR, defers EAGAIN/EWOULDBLOCK until the next select pass, treats zero-progress writes with pending data as EIO, and retries select after EINTR. This prevents transient system-call interruptions from aborting jobs and prevents a tight infinite loop when a write returns zero.

## Verification boundary

- Parent main SHA efb830bce5028536826989c8b5070b68a49e4a4e: EAPoL source orphan-tail correction and Process source contract were committed; exact XY-C1, Process umbrella, Process inventory, EAPoL, NVRAM and AI Actions were queued at the last check.
- WSDD2 XML regression #45040760516 PASS and AI docs #45040760489 PASS on checker-only descendant e2a842d08bae9e4f0e64a519e10c13c0444888f7. Product source d2 XY-C1 #45040593342 FAILED at RC compile due orphan EAPoL tail; fixed in parent efb. No L3 on d2.
- The new p910nd EINTR/EAGAIN regression and source contract have not yet run. No local checkout/tests or current firmware build were performed.
- L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. Physical printer/router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head p910nd SIGPIPE/I/O regression, Process/EAPoL checks and XY-C1 build after this commit. Fix actual test/source failures.
2. Confirm XY-C1 Build, TRX verification and artifact upload all PASS on the same exact product-source SHA; the earlier d2 failure is not evidence for efb or a later source SHA.
3. Continue another userland stability/security slice beyond completed WAN/PPP, Inadyn, DNSMasq, EAPoL, ttyd and WSDD2 work. Keep backup read-only and work only on main.
