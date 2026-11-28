## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once in this session; do not repeat the initial audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. Only `backup/...` and `main` exist.
- Current main before this WAN/VIPTV DHCP teardown fix: `eb9b6a54b7116cd88d28fbfd18e237f7de03946e`. Exact NVRAM transaction-boundary #46148136209 and handoff validation #46148136319 both PASS; CLI max-assignment, CLI secret erasure, kernel value-slot erasure and full caller-boundary regressions are green.
- PR #1 remains closed and unmerged; the CA trust-store feature is already integrated into main.

## Active task

- Fix WAN/VIPTV DHCP client teardown: `start_udhcpc_wan()` and `start_udhcpc_viptv()` do not pass BusyBox `-R`, while the shared `stop_udhcpc_pidfile()` sent only SIGTERM. A BOUND BusyBox client exits on SIGTERM without running `deconfig`; LAN shutdown already documents this and requests SIGUSR2 plus a 300 ms grace period first.
- The shared WAN/VIPTV helper now validates the pidfile identity, requests SIGUSR2/RELEASE, waits for the deconfig callback, then sends graceful termination. ENOENT/ESRCH mean the client is already gone; other identity errors and grace-wait failures stop teardown.
- Added a GNU89 host regression for successful release/wait, stale or exited clients, unknown PID identity and wait failures. The WAN teardown workflow now checks release → wait → terminate ordering and confirms both callers omit `-R`.

## Exact-SHA CI checkpoint

- Previous main `eb9b6a54b7116cd88d28fbfd18e237f7de03946e`: NVRAM transaction-boundary #46148136209 PASS; AI handoff #46148136319 PASS.
- XY-C1 run #46147869371 for production SHA `13339bed554e334b2e9c0176533d6e4a97c7907e` was IN_PROGRESS at Build XY-C1 at last observation; TRX and artifact steps were pending.
- New WAN/VIPTV DHCP release-before-stop change: THIS COMMIT; exact-head WAN teardown, AI handoff and XY-C1 checks pending.

## Verification boundary

- Exact `eb9b6a54b7116cd88d28fbfd18e237f7de03946e`: NVRAM workflow #46148136209 PASS. Log confirms `NVRAM CLI maximum assignment boundary: PASS`, `NVRAM CLI sensitive-buffer erasure: PASS`, `NVRAM sensitive value-slot replacement and release erasure: PASS`, and `NVRAM raw-userland boundary, wrapper-lock contracts and kernel caller inventory: PASS`. Handoff #46148136319 PASS.
- XY-C1 #46147869371 on `13339bed554e334b2e9c0176533d6e4a97c7907e` was still at Build XY-C1 when last checked; TRX/artifact had not started. This result predates the WAN source change.
- This WAN production change has no exact-head host/workflow or firmware result yet. No L3 or router hardware L4 claim.

## Exact first action for the next GPT

1. Inspect exact-head `wan-teardown-stability`. Require the new release-before-stop policy test and source-order assertions to pass alongside the existing WAN teardown suite.
2. Inspect exact-head XY-C1. Require Build XY-C1, TRX verification and artifact upload on one production-source SHA before recording L3.
3. Continue a distinct source-backed stability/security task while any independent build is pending. Do not repeat NVRAM inventories; its exact-head checks are green.
4. Keep the frozen backup untouched and use complete-tree, non-forced expected-SHA main commits. Physical hardware L4 remains NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
