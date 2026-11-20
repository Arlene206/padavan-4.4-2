# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent main before the DHCP pool result-propagation change: `6415d3efde51db293a1d8afd8ca8cbd1eec9bc1c`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- Exact-head ZeroTier lifecycle `44660670390` and AI handoff docs `44660670309` PASS on `6415d3e`. Process inventory `44659893624` and WAN route `44659893564` PASS on parent `272c516`.
- XY-C1 for prior SHA `272c516` was run `44659893579`; current main's L3 status must be checked again. Earlier L3 on `bffe074d853e9083d299d3ce07dd96d827a6d681` predates the uploaded-planet source work. Physical-router L4 remains NOT VERIFIED.

## Active task

- Fixed ignored NVRAM results in `prepare_dns_dhcpd()`: the old code wrote corrected `dhcp_start` and `dhcp_end` independently and ignored both return values. A partial write could leave persistent values inconsistent while DNSMasq continued with the corrected local pool.
- The new policy helper writes both keys, restores both original values on either failure, attempts both rollback operations even if one rollback fails, preserves the original errno after a successful rollback, and reports EIO plus a rollback-failed flag if recovery is incomplete. The production caller now aborts DNSMasq preparation on any NVRAM update failure. Regression covers success, first/second write failure and incomplete rollback.

## Verification boundary

- L1: `nvram_set()` is declared as returning `int` in `trunk/user/shared/src/nvram_linux.h`; both results were previously discarded in the DHCP auto-correction branch.
- The policy header/test, `services_ex.c` call site and DNSMasq workflow source contract change together. Exact-head DNSMasq host regression and XY-C1 firmware validation are pending.
- Current-source L2/L3 are NOT VERIFIED until Actions complete; physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check DNSMasq lifecycle and XY-C1 Actions for THIS COMMIT by exact `head_sha`. Require the GNU89 host regression, inline production source contract, Build XY-C1, TRX verification and artifact upload to pass before claiming L3.
2. Continue `services_ex.c` review for additional concrete service-launch/cleanup return statuses discarded by direct callers; add a reproducer before each source correction, avoiding generic signature-only changes.
3. Keep the frozen backup untouched and maintain separate L1/L2/L3/L4 evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
