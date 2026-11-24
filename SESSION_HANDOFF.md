## Current repository state

- Active development branch: `main`. Source/test commit for the completed VSFTPD EPSV fix: `974315bb0113d5fbc294a716579edceeb7b5d68f` (parent `36d712d5a7899a95510950e43289e9b68491cff2`). This handoff checkpoint is committed directly on top of that source SHA; resolve the live main head before making further changes.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- RAETH forced-mode indirect MMD page/data transaction is complete: source change `e721cb0e2f42f22a8189937ac2311d246ea9912c`, followed by test include-path correction `542b8f7e38a30dca616db877a1757d22b1b19060`; exact Switch CLI regression #45363906601 and AI handoff #45363906553 PASS. The initial Switch run failed on the compile include path; the correction was followed by the passing exact-SHA run. No repeat audit is needed.
- VSFTPD SITE CHMOD/UMASK strict-octal validation remains verified: source `ea7d7e28be9988175fbcc423f9378e5d765de764`, checker correction `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5`; exact VSFTPD stability #45359061300 PASS.
- FRP procfs-read and argv newline-spoofing fixes `0104186915ee6132d99871bede241ecf1835c279` and `5ca9761969591766c583d3146cf5f32155ab7891`; exact FRP lifecycle #45361194679, AI handoff #45361194624, Process inventory #45361194576 PASS.
- AliDNS POSIX byte encoder `ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6`; exact AliDNS #45357686604 PASS.
- Samba per-share ACL snapshot source `d8f6092c6af34f1c8ec45c746ce9411e40522577`; HTTPD legacy source-contract correction `3026ffe460fe9d01ad5b72312978ff957b400479` exact HTTPD job #45363047352 PASS.
- QMI client-ID workflow heredoc correction `4985f33bda8360f4dfb92581b7b7d47d7850a03c`; exact USB modem run #45363094446 PASS.
- XY-C1 L3 firmware validation was previously completed for the production-source baseline `ea7d7e28be9988175fbcc423f9378e5d765de764`: exact Build/TRX/artifact #45358712652, Process #45358712716 and NVRAM #45358712663 PASS. This is not a build claim for later source commits. No physical L4 router validation is claimed.

## Completed task — Strict VSFTPD EPSV protocol selector parsing

- `handle_pasv()` previously parsed optional `EPSV <protocol>` with `atoi()`. Prefix forms such as `1junk`, signs, and whitespace were accepted as protocol 1 instead of rejected.
- Replaced that conversion with `vsf_sysutil_parse_uint()` bounded to 2 and the existing address-family check. `EPSV ALL` remains unchanged; protocol 1 is accepted on IPv4/IPv6 and protocol 2 only on IPv6.
- Extended the existing production-parser host regression in `trunk/user/vsftpd/src/tests/ipaddrparse_policy.c` with IPv4/IPv6 protocol cases and malformed-selector cases. The VSFTPD workflow already compiles/runs that test; its source contract now pins strict parser use and forbids an `atoi()` regression in `handle_pasv()`.
- Source/test/workflow/dev-log changes are one atomic commit: `974315bb0113d5fbc294a716579edceeb7b5d68f`.

## Verification boundary

- Exact source SHA `974315bb0113d5fbc294a716579edceeb7b5d68f`: VSFTPD stability #45372034160 PASS; AI handoff #45372034407 PASS; Process inventory #45372034153 PASS.
- At the last check, NVRAM transaction audit #45372034333 and XY-C1 firmware validation #45372034143 were still IN_PROGRESS on that exact SHA. Recheck before making any L3 claim. Do not claim a current-SHA firmware build until Build, TRX and artifact upload all finish successfully.
- Exact parent SHA `36d712d5a7899a95510950e43289e9b68491cff2`: AI handoff #45371307227, Process inventory #45371307371, DNSMasq lifecycle #45371307256 and NVRAM audit #45371307355 PASS; XY-C1 #45371307318 was still IN_PROGRESS at the last recorded poll.
- The VSFTPD regression workflow on `974315bb` compiled and ran all C policy regressions, including the extended `ipaddrparse_policy.c`, and its inline source-contract job passed.
- No local repository clone was available in this runtime (network DNS resolution failed); no local compile result is claimed. The exact-SHA GitHub Actions run is the test evidence.

## Exact first action for the next GPT

1. Inspect exact-SHA NVRAM #45372034333 and XY-C1 #45372034143 for `974315bb0113d5fbc294a716579edceeb7b5d68f`. Both were pending at the last check; resolve an actual failure, but do not wait on a pending job before moving to independent work.
2. The VSFTPD EPSV fix and its exact regression are complete; do not repeat this audit. Continue with a fresh concrete P1 stability/security defect elsewhere in the userland, preserving Linux 4.4/GNU89/MIPS-uClibc constraints and small atomic commits.
3. Keep `backup/pre-plugin-overhaul-2026-11-14` immutable. Use complete Git trees/commits and non-forced expected-SHA updates to `main`.


# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change must land as one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task.
