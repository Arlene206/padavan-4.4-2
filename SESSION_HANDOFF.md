# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice lands as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch `main`.
- Parent main: `88b8124bfa1363b8f6f066e66b4e2c6db80e26f6`. Recent slices repair the checked VPN pool caller, guard pushed-DNS policy reads and make OpenVPN server callback device fallback depend on an actual `dev` or a checked mode value.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no branch/PR development. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Active task

- The exact-head VPN lifecycle run #44938233419 for `040b3da86f81007605c80a5687b7a516f41d19ce` failed in the Python source contract, not a compiled C regression: the new checker embedded a double-escaped function signature (`static int\\non_client_ifup`) and failed locating the function. All compiled policy/sanitizer steps passed before the source-contract step.
- This corrective slice reduces the escaped newline/tab sequences in the new checker block so its function signatures and source tokens are actual Python newline/tab escapes. It does not alter production source or weaken assertions. Exact-head VPN workflow must rerun.
- The `vpnc_pdns` production change remains: checked read and 0/1 validation; on read error skip optional DNS/domain/resolver publication while still attempting link-state publication, the up hook and environment cleanup.

## Verification boundary

- On parent `23520100e3cc74bb7f77a0d023afa361cded2abe`, exact-head VPN #44937972537, AI docs #44937972518, WAN route #44937972672, WAN DHCP #44937972690, HTTPD #44937972661, Process #44937972749 and NVRAM audit #44937972629 passed. XY-C1 #44937972691 remained in progress at its last observation.
- On `040b3da86f81007605c80a5687b7a516f41d19ce`, AI docs #44938233374, OpenVPN client credential staging #44938233509, HTTPD #44938233476, Process #44938233425 and NVRAM #44938233440 passed; VPN lifecycle #44938233419 failed only at the checker locator above; XY-C1 #44938233497 was still in progress.
- On `88b8124bfa1363b8f6f066e66b4e2c6db80e26f6`, the latest poll had only Process #44938409769 and HTTPD #44938409751 queued; no VPN/XY result was yet visible. Do not infer PASS from a missing run.
- L3 is NOT VERIFIED until same-SHA Build XY-C1, TRX and artifact upload all pass. L4 hardware flash/boot/runtime validation remains NOT VERIFIED. No local cross-build is claimed.

## Workflow and preservation rules

- Main only. Each multi-file logical task is one complete Git tree, one commit and one non-forced ref update using the exact expected SHA. No per-file partial commits, force push, amend, rewrite, branches or PRs.
- Re-fetch main before writes. Verify only exact-head main push Actions. PENDING does not block independent work; diagnose failures from the first causal error.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, NVRAM defaults, security and device compatibility. Do not weaken regression assertions or workflow path filters.
- `dev_log.md` retains 20–119 newest-first entries; archive oldest 100 before record 120, leaving 20. Rotate early above 384 KiB; hard limit 512 KiB. Preserve archive/index bytes. Update logs for real milestones; roadmap only for material project-wide changes; no routine handoff snapshots.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head VPN lifecycle / AI docs results for the checker correction, then inspect XY-C1 #44938233497 and the latest head's own firmware run. Diagnose any failure from its first causal error.
2. Confirm the checked-`vpnc_pdns` and server callback source-contract slices pass on one exact SHA; then continue with any remaining OpenVPN export-profile parity issue against network/options snapshots.
3. Require Build XY-C1, TRX and artifact upload on the same SHA for L3; hardware L4 remains not verified.
