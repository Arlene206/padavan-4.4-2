# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent main before this CI correction: `272c51644cdecef17d0a6b0891ce56014e93daa1`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- On `272c516`: Process inventory `44659893624` and WAN route `44659893564` PASS. ZeroTier lifecycle `44659893584` FAILed only in the Python source checker after the shell lifecycle test PASSed; AI docs `44659893698` FAILed because required handoff headings were not canonical. XY-C1 `44659893579` was IN PROGRESS at last poll.
- Current-source L3 is NOT VERIFIED. Earlier production SHA `bffe074d853e9083d299d3ce07dd96d827a6d681` has separate L3 evidence from XY-C1 `44656543236`, but predates uploaded-planet import and rollback-recovery updates. Physical-router L4 remains NOT VERIFIED.

## Active task

- The uploaded ZeroTier planet import stages private data, preserves a hard-link rollback point for the previous planet file, and attempts coordinated disk/NVRAM publication. Its rollback hardening now stores the previous NVRAM string in mode-0600 `planet.nvram.previous` and retains the private transaction directory if the compensating NVRAM commit fails. The shell regression asserts the recovery snapshot and permissions.
- This commit corrects two exact-head validation defects on parent `272c516`: the Python source checker had a quoted string where a boolean expression was required, causing `TypeError: require() missing 1 required positional argument: 'message'`; and the handoff used headings rejected by the documentation validator. No production logic or regression assertions are weakened.

## Verification boundary

- ZeroTier `44659893584` on `272c516`: shell lifecycle regression PASS; Python source contract FAILed due the malformed checker argument.
- AI handoff docs `44659893698` on `272c516`: FAILed for missing exact sections `## Active task`, `## Verification boundary`, and `## Exact first action for the next GPT`.
- Process inventory `44659893624` and WAN route `44659893564` on `272c516`: PASS.
- XY-C1 `44659893579` on `272c516`: IN PROGRESS at last poll. Current-source L3 and physical-router L4 are NOT VERIFIED.

## Exact first action for the next GPT

1. Check this commit's exact-head ZeroTier, AI docs, Process inventory, WAN route and XY-C1 Actions. Diagnose any failure. L3 requires Build XY-C1, TRX verification and artifact upload to pass on the same SHA.
2. Continue P1 RC result propagation in `trunk/user/rc/src/services_ex.c`: the paired `nvram_set("dhcp_start", ...)` / `nvram_set("dhcp_end", ...)` inside `prepare_dns_dhcpd()` discards integer return values. Preserve both old values, update them as a rollback-capable pair, and abort DNSMasq preparation if either write or rollback fails. Extend the existing DHCP-range host regression and the inline DNSMasq source contract before changing the call site.
3. Keep the backup branch untouched and maintain separate L1/L2/L3/L4 evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
