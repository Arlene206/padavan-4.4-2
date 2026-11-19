# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for the shared service-lock retry slice: `a9f3c2030c917043642aad02574da2ada180b473` (`docs(ai): restore cross-session checkpoint rule`).
- MiniUPnP bounded EINTR source commit `e736dbfcff830e6ac6baa32026fff7a6334787d1`: Process lifecycle `44628661363`, MiniUPnP config `44628661018`, Process inventory `44628661358`, NVRAM audit `44628661528`, and XY-C1 `44628661156` all PASS. XY-C1 completed Build XY-C1, TRX verification and artifact upload on the exact e736 source SHA.
- AI handoff documentation run `44628661127` failed on e736 because the mandatory literal phrase `Before context exhaustion` was absent; docs-only follow-up `a9f3c20` restored it and AI docs run `44629051861` PASS.
- Shared helper source commit `213a077af64819acf40fd44ca5a6df91f7472fb4`: MiniUPnP config `44631039048`, VPN `44631039170`, AI docs `44631039225`, DNSMasq `44631039014`, Inadyn `44631039151`, Process inventory `44631039149`, Process regression `44631039075`, HTTPD security `44631039061` and NVRAM audit `44631039111` PASS. Exact XY-C1 run `44631039226` was still in progress at last query; its job had completed source builds/tests and was at Build post-run/upload cleanup, but the run conclusion had not yet been recorded. Do not claim L3 until run conclusion success and TRX/artifact steps are SUCCESS.
- Newly confirmed defect: Inadyn, DNSMasq and VPN-server lock acquisition backoff could be shortened by EINTR. Inadyn skipped the one-second wait entirely after EINTR; DNSMasq/VPN ignored sleep()'s unslept remainder. This follow-up corrects the waits with service_wait_seconds(), whose unit regression already pins remaining-time behavior.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged.
- No physical-router result is available. Host Actions are L2; XY-C1 Build/TRX/artifact on one exact production SHA is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed a contention-timing defect in three production lifecycle lock acquire paths. Inadyn skipped the one-second wait when F_SETLK returned EINTR; DNSMasq and VPN server used sleep(1) but ignored a nonzero unslept remainder. Repeated signals could therefore exhaust retry attempts much faster than the configured wait.
- This slice uses the existing service_wait_policy.h/service_wait_seconds(1) in all three paths, preserving the complete wait interval across EINTR. It does not change the lock's EACCES/EAGAIN retry count or the shared eight-retry EINTR cap on unlock.
- The existing service_wait_policy regression exercises interrupted waits and remaining-time retry. The MiniUPnP semantic checker is extended to pin all three acquisition callsites; XY-C1 path filtering includes DNSMasq lock header so all changed production headers trigger firmware validation.
- No local compilation or physical-router result is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Inadyn, DNSMasq, VPN-server, MiniUPnP config, Process lifecycle, AI docs, HTTPD/security and XY-C1 workflows for this signal-safe acquisition-wait fix.
2. On FAIL, inspect the exact failing job log and correct the source/check contract without weakening lock identity checks or retry bounds. XY-C1 L3 requires Build, TRX verification and artifact upload all SUCCESS on the exact same SHA.
3. Continue the next independently evidenced cross-userland stability/security slice; do not repeat the already-corrected PPP, APCLI, SD/MMC notification or lock EINTR loops.

## Verification boundary

- L1: Inadyn, DNSMasq and VPN acquisition paths preserve the one-second lock contention interval after EINTR; shared eight-retry unlock policy remains active.
- L2: The MiniUPnP semantic checker now pins these three acquire paths; exact-head service workflows will provide GNU89/ASan/UBSan and source-contract verification.
- L3: Exact shared-helper commit `213a077` XY-C1 Build/TRX/artifact run `44631039226` remains IN_PROGRESS at this checkpoint. This new acquisition-wait source SHA requires its own L3 build.
- L4: target MIPS/uClibc timing under signal load and physical-router WAN/service transitions remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
