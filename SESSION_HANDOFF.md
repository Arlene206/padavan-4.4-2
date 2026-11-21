# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD immediately before the PPP-only WAN address checker correction: `4dc1e9db6dd81451530593170d0679c79d4aab91`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- Latest product-source commit `335fddd5b61d20e22987da948136c412cf4bbf81` hardens WAN-down gateway /32 ownership: checked reads are insufficient unless the previous IP passes `dhcp_lease_ipv4_is_valid()` and the mask passes `dhcp_lease_netmask_is_valid()` before subnet classification. Invalid values accumulate EINVAL and cannot authorize /32 deletion.
- On product-source SHA 335f, WAN teardown #44927981495, Process inventory #44927981473, WAN DHCP callback #44927981545, NVRAM transaction audit #44927981607 and AI handoff #44927981573 passed. XY-C1 firmware validation #44927981534 completed successfully: XY-C1 Build, TRX verification and artifact upload all PASS. L3 is VERIFIED for this exact product SHA; L4 remains NOT VERIFIED.
- The WAN gateway/address/netmask tuple is now read under one NVRAM transaction in `wan_unit_route_snapshot()` (production commit `74329b3f1e1ae376930dbeb1143d305fb3f6d646`); d0 (`d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192`) restores the PPP-only checked IP read used by conntrack. On d0: WAN DHCP callback #44929756539 PASS, WAN teardown #44929756599 PASS, Process inventory #44929756463 PASS, AI docs #44929756531 PASS, NVRAM audit #44929756500 PASS; WAN route #44929756615 failed at inline assertion 366 (multiline gateway fallback formatting). Exact route rerun on 4dc reached assertion 370 (PPP-only address getter was line-wrapped); the current workflow correction counts that call in the compact function body. XY-C1 #44929756345 remains in progress.
- After these checker fixes, inspect exact-head Process and WAN route logs. The next production review is whether the WAN gateway/address/netmask reads can race across independent NVRAM updates and produce a mixed-generation route tuple. Use the NVRAM transaction audit result and actual lock/transaction API semantics before changing production code.

## Verification boundary

- L1: WAN host-route ownership requires a validated coherent gateway/address/netmask tuple from one NVRAM transaction; tuple-read failure denies /32 deletion while a checked gateway fallback still permits default-route cleanup. PPP gets a separate checked IP copy for conntrack. GNU89 policy tests run under ASan/UBSan. Exact d0 WAN teardown and WAN DHCP callback passed; fresh WAN route source-contract rerun is pending.
- L2: Process regression #44929876370 and AI handoff #44929876383 PASS on exact main `a760c37dd793f0497907fb5b30a871be6497415b`. WAN route #44929756615 on d0 failed at inline assertion 366 for the multiline gateway fallback; rerun #44930083634 on exact SHA 4dc fixed that assertion but advanced to the next formatting-sensitive PPP IP getter assertion 370. The current correction normalizes both gateway and PPP address checks; exact rerun required.
- L3: VERIFIED on exact product-source SHA `335fddd5b61d20e22987da948136c412cf4bbf81`: XY-C1 Build, TRX output verification and artifact upload all PASS. This validation does not cover later product-source commits unless rerun.
- L4: hardware route ownership and concurrent teardown/callback behavior remain NOT VERIFIED.

## Workflow and preservation rules

- Main only. One logical slice, one atomic main commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected SHA. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
- Re-fetch main immediately before writing. On a moved ref, discard the stale prepared tree and rebuild from the new parent.
- Inspect push Actions only when head_sha exactly matches the commit. Diagnose failures from logs. PENDING does not block independent work.
- Preserve security, architecture, ABI, GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Never weaken useful tests or focused workflow path coverage.
- Do not touch GPT Session Rotator workflow/state or replay SHA 9d5a5cb. The backup branch is permanent immutable recovery data.

## Cross-conversation documentation governance

- GitHub is the durable cross-conversation record; the one-line user entrypoint remains sufficient.
- dev_log.md stays newest-first with 20–119 complete records. Before record 120, archive the oldest 100 complete records verbatim into docs/dev-log/YYYY/MM/part-NNN.md in one batch, leaving 20 newest entries.
- Rotate early only above the 384 KiB target and never exceed 512 KiB. Preserve archive/index bytes. Git history is the version archive for handoff and roadmap; routine snapshots and SHA-chasing commits are prohibited.
- Update dev_log.md for real milestones, this handoff when first-next-action facts materially change, and REFACTOR_ROADMAP.md only when repository-wide module status/priorities materially change.
- Before context exhaustion, finish the safe slice, commit atomically, inspect exact-head Actions and leave the next precise task.

## Exact first action for the next GPT

undefined
