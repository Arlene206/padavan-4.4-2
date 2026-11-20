# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent main before this follow-up: `4e618ff3f5467c691edc4aeb7fea46fea46caebd`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- On `4e618ff`, DNSMasq lifecycle `44661329441`, WAN route `44661329346`, Process inventory `44661329295`, WAN DHCP callback `44661329340`, HTTPD auth/security `44661329359`, miniupnpd config `44661329261`, AI handoff docs `44661329167`, Samba `44661329573`, Inadyn lifecycle `44661329414`, NVRAM transaction audit `44661329493`, and Process umbrella `44661329176` PASS.
- XY-C1 `44661329180` remains IN PROGRESS at last poll. Current-source L3 is NOT VERIFIED until its Build, TRX and artifact all pass. Physical-router L4 remains NOT VERIFIED.

## Active task

- The prior commit `4e618ff` made the two corrected DHCP pool NVRAM writes rollback-aware and fail-closed; exact-head DNSMasq regression `44661329441` PASSed.
- This follow-up addresses the remaining unchecked `nvram_set()` call in `write_inadyn_conf()`: when an invalid DDNS provider alias is repaired to the default Inadyn provider, its persistence result was ignored. Configuration generation could still succeed while the bad value remained persistent. The writer now aborts and preserves errno when fallback-provider normalization fails.

## Verification boundary

- L1: `nvram_set()` is an integer-returning shared API; the remaining unchecked call in `services_ex.c` was the fallback-provider write in `write_inadyn_conf()`.
- This commit changes production source and the DNSMasq workflow's existing inline Inadyn source contract. Exact-head workflow results are pending.
- L3 remains unverified until Build XY-C1, TRX verification and artifact upload all pass for one matching SHA. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check this commit's DNSMasq lifecycle, Inadyn lifecycle, AI handoff, Process umbrella and XY-C1 Actions by exact `head_sha`. Diagnose FAIL; require same-SHA Build XY-C1, TRX and artifact upload before claiming L3.
2. Continue surveying the Padavan userland for concrete dropped service/cleanup statuses beyond `services_ex.c`, with a reproducer or source contract for each real defect.
3. Keep the frozen backup untouched and maintain separate L1/L2/L3/L4 evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
