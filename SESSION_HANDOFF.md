# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent main before this follow-up: `ab6c532b1211ac5ed676537002edef6a231ebcc7`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- On parent `ab6c532`: ZeroTier lifecycle `44659276399`, Process inventory `44659276281`, WAN route `44659276230`, and AI handoff docs `44659276308` PASS. XY-C1 `44659276259` is IN PROGRESS; do not claim L3 until Build, TRX and artifact upload pass on one exact production-source SHA.
- Earlier production SHA `bffe074d853e9083d299d3ce07dd96d827a6d681` had L3 via XY-C1 `44656543236`, but that run predates the uploaded-planet import and staging cleanup changes. It does not establish L3 for current source. Physical-router L4 remains NOT VERIFIED.

## Active engineering work

- Uploaded ZeroTier planet import privately stages the upload and attempts coordinated disk/NVRAM rollback. Review found that a failed forward `nvram commit` followed by a failed compensating commit left the old NVRAM value only in shell memory. The EXIT trap could restore the file and delete its transaction directory while persistent NVRAM state remained uncertain. THIS COMMIT stores the old value in mode-0600 `planet.nvram.previous` and retains the transaction directory on NVRAM rollback failure. The fault-injection regression verifies the snapshot and retained directory.
- The parent `ab6c532` also fixed the EXIT-trap registration gap after staging-directory creation and added failed-NVRAM-read coverage; exact-head ZeroTier lifecycle `44659276399` PASS.

## Exact first actions for the next GPT

1. Check ZeroTier lifecycle, Process inventory, WAN route, AI docs and XY-C1 for THIS COMMIT by exact `head_sha`. Diagnose any failure; claim L3 only when the matching SHA's Build XY-C1, TRX and artifact upload all pass.
2. Resume P1 RC result propagation in `trunk/user/rc/src/services_ex.c`. The direct `nvram_set("dhcp_start", ...)` / `nvram_set("dhcp_end", ...)` pair in `prepare_dns_dhcpd()` ignores both integer return values declared in `trunk/user/shared/src/nvram_linux.h`. Preserve both original values, update them as one rollback-capable pair and abort DNSMasq preparation if either write or rollback fails. Extend the existing DHCP-range host regression and inline DNSMasq source contract before changing the call site.
3. Preserve the frozen backup and maintain separate L1/L2/L3/L4 evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
