# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice lands as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch `main`.
- Parent main: `bdb0ecf22351f910d1185bf2434a27417e5ecf99`. Recent source fixes include checked xl2tpd VPN-pool snapshot use, fail-closed OpenVPN pushed-DNS policy and checked OpenVPN server callback device fallback.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged. Do not create branches/PRs, force-push, amend history, touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Active task

- VPN lifecycle #44938233419 on `040b3da86f81007605c80a5687b7a516f41d19ce` failed because `scripts/check_vpn_client_nvram_state.py` double-escaped its new function signature. The 88b8124 exact-head VPN rerun #44938409760 failed at the same first source-contract locator; all compiled policy/sanitizer steps passed before it.
- Commit `bdb0ecf22351f910d1185bf2434a27417e5ecf99` corrects that client checker, but inspection found the newly added server callback assertions in `scripts/check_vpn_server_lifecycle.py` also have double-escaped function locator strings. This slice corrects those locators before their first execution. No production changes are made in this checker-only correction.
- The callback production change still trusts OpenVPN `dev`, reads `vpns_ov_mode` checked only when absent, does not invent TAP after failure, permits address-based disconnect lease removal with an unknown interface, and preserves configured hooks.

## Verification boundary

- On exact parent `23520100e3cc74bb7f77a0d023afa361cded2abe`, VPN lifecycle #44937972537, AI docs #44937972518, WAN route #44937972672, WAN DHCP #44937972690, HTTPD #44937972661, Process #44937972749 and NVRAM audit #44937972629 passed; XY-C1 #44937972691 was still in progress at last observation.
- On `040b3da86f81007605c80a5687b7a516f41d19ce`, AI docs #44938233374, OpenVPN client credential staging #44938233509, HTTPD #44938233476, Process #44938233425 and NVRAM #44938233440 passed. VPN lifecycle #44938233419 failed only at the double-escaped client source-contract locator; XY-C1 #44938233497 remained in progress at last poll.
- On `88b8124bfa1363b8f6f066e66b4e2c6db80e26f6`, AI docs #44938409877, OpenVPN client credential staging #44938409762, Process #44938409769 and HTTPD #44938409751 passed; VPN #44938409760 failed at the same client checker locator, while NVRAM #44938409754 and XY-C1 #44938409776 were in progress.
- On `bdb0ecf22351f910d1185bf2434a27417e5ecf99`, VPN #44938509538 and AI docs #44938509917 were in progress at the last poll.
- XY-C1 L3 is NOT VERIFIED until Build, TRX and artifact upload all pass on one exact SHA. L4 hardware flash/boot/runtime validation remains NOT VERIFIED. No local cross-build is claimed.

## Workflow and preservation rules

- Main only. Each logical slice is one complete Git tree, one commit and one non-forced ref update with the exact expected SHA. No partial commits, branches, PRs, force push, amend or history rewrite.
- Re-fetch main before writes. Verify only exact-head main push Actions. PENDING does not block independent work; diagnose failures from the first causal log error.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, NVRAM defaults, security and device compatibility. Do not weaken source contracts or workflow paths.
- `dev_log.md` has 118 root records before this slice; this entry brings it to 119. Before adding another record, archive the oldest 100 complete records to the prescribed dated archive and leave 20 root entries. Rotate early above 384 KiB, never exceed 512 KiB; preserve archive/index bytes. Roadmap changes only for material project-wide status changes.

## Exact first action for the next GPT

1. Inspect the current exact-head VPN lifecycle and AI docs runs after this server-checker locator correction. It must pass both OpenVPN source contracts and the lifecycle contract; diagnose any failure from the first causal log error.
2. Inspect exact-head XY-C1 runs #44938233497 and #44938409776, and the latest head's own build/TRX/artifact result. Require all three product steps on the same SHA for L3.
3. Once these callbacks validate, continue the remaining OpenVPN exported-profile parity checks against the checked network/options snapshots; do not reopen completed credential/CCD/client-option slices absent same-SHA regressions.
