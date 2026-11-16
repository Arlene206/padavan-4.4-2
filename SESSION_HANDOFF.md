# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Source status checkpoint: `4cdf85227650738b7a9c105322fab12e5b5d06cc`.
- LAN IPv6 address reload status propagation is implemented and its test passed. Checker normalization fix is verified: exact Process `43983013930` PASS, Firewall Startup `43983014009` PASS, AI handoff `43983014049` PASS. Exact XY-C1 `43983014153` was still building firmware at the last poll; TRX/artifact steps remained pending. Product-source L3 is NOT VERIFIED until Build/TRX/artifact pass on the same SHA.
- The follow-on audit found three WAN IPv6 address-add commands using shell-formatted `doSystem()` with constructed/config-derived strings: SIT WAN address, derived SIT LAN address, and native static WAN address. They are now argv-based `eval()` calls. SIT addresses are formatted from IPv6 structs; native static address text comes through `format_ipv6_literal()`. Runtime errors continue through existing fail-fast WAN macros.
- Added shared checker and WAN workflow contracts ensuring all three dynamic address-adds remain argv-based and address validation/generation remains in place.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Verify the WAN IPv6 address argv-safety slice in this commit. Confirm the shared checker, WAN inline source contract, Process, Firewall, AI handoff and XY-C1 Actions on exact SHA. Then continue reviewing adjacent dynamic `doSystem()` arguments only where validation provenance is unclear.

## Verification boundary

- Exact `4cdf85227650738b7a9c105322fab12e5b5d06cc`: Process `43983013930`, Firewall Startup `43983014009` and AI handoff `43983014049` PASS.
- XY-C1 `43983014153` remained in `Build XY-C1`; TRX verification and artifact upload had not yet run. WAN route was not triggered by the checker-only follow-up commit but passed on the preceding source SHA `176e93e`.
- The new WAN IPv6 argv-safety edit requires its own exact-SHA WAN and XY-C1 checks. Product-source L3 remains NOT VERIFIED; physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Inspect this commit's exact-head WAN route, Process, Firewall, AI docs and XY-C1 Actions; diagnose any first failing assertion.
2. Confirm XY-C1 Build, TRX verification and artifact upload on one SHA. Do not claim L3 until all product steps pass.
3. Continue audit in `net_wan6.c` and related IPv6 paths, checking whether remaining formatted shell commands receive only fixed/generated values or whether argv-based calls are warranted. Preserve backup immutability.
