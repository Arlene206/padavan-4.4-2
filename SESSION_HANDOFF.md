# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent `main` before the current source-checker repair: `24dfe60f26ebdc89dd98ddbd62728acc853d1d20`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- The TCP policy slice on `e387b92` passed Process `44672214134` and AI docs `44672214038`; its XY-C1 run was still in progress at last check.
- The libc address-selection change on `24dfe60` has a focused local GNU89/Werror/ASan/UBSan policy test PASS and AI docs run `44673236138` PASS. Its Process run `44673235684` FAILed in `scripts/check_dev_init_boot.py` because the checker searched for a no-argument callback signature while the production callback correctly takes `void *context`. The GAI test step was skipped after this checker failure; XY-C1 `44673236140` was still in progress at last check. This is a checker-signature defect, not evidence of a product C failure.
- Historical PR #1 is CLOSED/UNMERGED. CA trust-store bootstrap and AliDDNS HMAC implementation/tests are already present on `main`.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Repair the PID 1 GAI source-checker signature. The production callback signatures include `void *context`; make the source checker match both callback declarations without weakening any test. Then re-run exact-head Process and docs checks. The GAI source code itself returns checked `fput_string()`/unlink status, treats unlink ENOENT as idempotent, and propagates status through PID 1 plus IPv6 init/restart.

## Verification boundary

- The GAI focused host policy test passed locally, but the exact Process checker failed before reaching the GAI test on SHA `24dfe60`; no exact-head Process PASS can be claimed for that slice.
- No same-SHA GAI L3 Build/TRX/artifact completion was confirmed at last poll. L4 remains NOT VERIFIED.
- This checker correction has not yet been validated by Actions. No full firmware build was performed locally.

## Exact first action for the next GPT

1. Verify live `main` and exact-head Actions for the checker correction. Confirm Process step `Verify PID 1 required boot environment failure gates` and the GAI policy regression both PASS; inspect any first failure log rather than assuming.
2. Confirm the GAI source commit's XY-C1 production-input detector, Build, TRX and artifact upload are complete on an exact source SHA before claiming L3.
3. Continue the newly confirmed WAN fail-closed state-publication defect: `restore_wan_after_ppp_start_failure()` ignores failure from `set_wan_unit_value(unit, "ifname_t", wan_ifname)` before rebuilding the firewall. Propagate the NVRAM errno and abort firewall rebuild if the physical interface cannot be published; the existing caller must keep the interface down. Add a GNU89 injected status regression and extend the WAN route workflow/checker. Legacy setter ABI/callers should remain unchanged.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
