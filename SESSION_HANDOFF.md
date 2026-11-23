# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (RC switch packed-ioctl range validation); parent main SHA: `73961add1641c5c23ed0d07c4f74c215e9cb8d1b`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The current task fixes a packed ioctl input-alias defect in both RC switch front-ends: `rtl8367_main()` and `mtk_esw_main()` accepted arbitrary-width numeric command/parameter values, then their ioctl wrappers masked the command into the low `*_IOCTL_CMD_LENGTH_BITS` and shifted the parameter into the remaining high bits. For MTK's 8-bit command field, e.g. decimal command `297` is silently reduced to command `41` (port power); an oversized third argument is truncated by the left shift and can become a different parameter. This defeats the strict lexical parser and can dispatch an unintended switch hardware operation.

This commit:
- Adds bounded helpers that derive the command maximum and packed parameter maximum from each actual UAPI `*_IOCTL_CMD_LENGTH_BITS` macro; rejects out-of-range values before the wrappers mask/shift.
- Applies those helpers to both RTL8367 and MTK front-ends. The ordinary second argument retains its legacy full unsigned range and optional hex syntax.
- Adds GNU89/ASan/UBSan boundary regressions for the highest valid command/parameter, command 256/266/297 alias attempts, parameter truncation, unsupported hex command and invalid bit widths. The Switch CLI workflow pins both frontend calls and the new policy tests.

Parent `73961add1641c5c23ed0d07c4f74c215e9cb8d1b` adds PID 1's terminal quiescence after SIGQUIT shutdown returns. Its exact-head PID1 mdev, AI docs, Process inventory/lifecycle, USB storage, DNSMasq, DNS Forwarder, notification queue and HTTPD checks passed. NVRAM and XY-C1 remained in progress at the last poll; XY-C1 had reached Build XY-C1, with TRX/artifact pending.

## Verification boundary

Parent source SHA `73961add1641c5c23ed0d07c4f74c215e9cb8d1b` exact-head Actions at last poll:
- PID 1 mdev/security `45089808790`: PASS.
- AI handoff docs `45089808802`: PASS.
- PID 1 notification `45089808842`, DNSMasq `45089808852`, DNS Forwarder `45089808746`, Process inventory `45089808793`, Process lifecycle regression `45089808796`, USB storage `45089808856`, and HTTPD security `45089808957`: PASS.
- NVRAM transaction boundary `45089808828`: IN_PROGRESS.
- XY-C1 firmware `45089808798`: Build XY-C1 IN_PROGRESS; TRX verification and artifact upload PENDING. No L3 claim.

For THIS COMMIT, inspect exact-head Switch CLI and XY-C1 after publication. The switch workflow compiles/runs the GNU89/ASan/UBSan policy binary; the same-source firmware workflow must show Build, TRX and artifact success before L3 can be claimed. Local full router checkout/build and physical hardware are unavailable; L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head Switch CLI run for the current source SHA. Confirm the GNU89/ASan/UBSan packed-command boundary tests and both MTK/RTL8367 source contracts pass.
3. Inspect exact-head XY-C1 on the same SHA; confirm Build XY-C1, TRX verification and artifact upload actually execute and pass. Host-only checks are not L3.
4. If checks are pending, continue another concrete module defect. If any exact-head job fails, inspect its first causal log and repair it before claiming the slice verified.
5. Keep one complete-tree atomic commit per logical task; fresh main parent, `force:false`, exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `73961add1641c5c23ed0d07c4f74c215e9cb8d1b`.
- RC MTK/RTL8367 switch ioctl front-ends now validate numeric command and packed parameter widths against each actual UAPI bit count before the wrapper masks/shifts them. This prevents oversized values from aliasing/truncating into another hardware request.
- Parent `73961add`: PID1 mdev `45089808790`, AI docs `45089808802`, notification `45089808842`, DNSMasq `45089808852`, DNS Forwarder `45089808746`, Process inventory `45089808793`, Process regression `45089808796`, Storage `45089808856`, HTTPD `45089808957` PASS; NVRAM `45089808828` pending; XY-C1 `45089808798` Build in progress / TRX and artifact pending.
- This commit requires exact-head Switch CLI and XY-C1 checks. Local full checkout/test not run; same-SHA L3 and hardware L4 not yet verified.
- Parent-SHA PASS results never validate descendant commits.
