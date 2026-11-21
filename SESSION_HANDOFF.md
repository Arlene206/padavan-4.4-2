# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD immediately before the latest checker correction: `fac45a05fa27110c2301e72829666e2fbe0d32fc`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- Latest product-source commit `335fddd5b61d20e22987da948136c412cf4bbf81` hardens WAN-down gateway /32 ownership: checked reads are insufficient unless the previous IP passes `dhcp_lease_ipv4_is_valid()` and the mask passes `dhcp_lease_netmask_is_valid()` before subnet classification. Invalid values accumulate EINVAL and cannot authorize /32 deletion.
- On product-source SHA 335f, WAN teardown #44927981495, Process inventory #44927981473, WAN DHCP callback #44927981545, NVRAM transaction audit #44927981607 and AI handoff #44927981573 passed. XY-C1 firmware validation #44927981534 completed successfully: XY-C1 Build, TRX verification and artifact upload all PASS. L3 is VERIFIED for this exact product SHA; L4 remains NOT VERIFIED.
- On exact checker/workflow SHA `a77ed612c19ac0b4c29d88f4b547fb7e077bd0d9`, AI handoff #44928210228 passed. Process regression #44928210224 failed at a stale ttyd declaration/rollback assertion. WAN route workflow #44928208859 failed before creating a job because six assertions below `jobs:` were outdented to YAML root level. `fac45a05fa27110c2301e72829666e2fbe0d32fc` corrected the ttyd checks and YAML indentation; WAN route #44928653174 and AI handoff #44928653213 pass on that SHA. Process regression #44928653157 reached its final integration checker and found two stale crond enable-guard expectations; those are corrected in the current checker update.
- After these checker fixes, inspect exact-head Process and WAN route logs. The next production review is whether the WAN gateway/address/netmask reads can race across independent NVRAM updates and produce a mixed-generation route tuple. Use the NVRAM transaction audit result and actual lock/transaction API semantics before changing production code.

## Verification boundary

- L1: WAN host-route policy covers checked read errors and invalid IPv4/netmask values; GNU89 policy tests run under ASan/UBSan. WAN interface teardown, WAN DHCP callback, process inventory and NVRAM transaction audit passed on product-source SHA 335f.
- L2: Exact checker/workflow head `a77ed612c19ac0b4c29d88f4b547fb7e077bd0d9` had AI handoff PASS, Process regression FAIL (stale ttyd checker), and WAN route workflow failure before job creation (YAML indentation). The current atomic correction requires fresh exact-head Actions.
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

Inspect the new exact-head Process regression after the crond checker correction; repair any remaining assertions from its exact log. Then implement a coherent NVRAM transaction snapshot for WAN gateway/address/netmask ownership classification in `wan_down_internal()`, while preserving the independent gateway read path needed to delete the default route when an address/mask snapshot fails. Add regression coverage and update the WAN route checker. L3 remains VERIFIED only for product-source SHA 335f; any subsequent product-source edit requires a new XY-C1 Build/TRX/artifact run for that exact SHA.
