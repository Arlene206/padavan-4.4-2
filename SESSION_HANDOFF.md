# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; all development remains on `main`.
- Parent `main` for this validation-integration slice: `7a64510bff3db5b1d223b0be70ad04898ce784fb`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not update/delete. PR #1 remains closed and unmerged.
- Startup/protocol recovery was completed once; do not repeat it or re-audit completed slices.

## Completed work

- The previous FTP checker correction is complete on `7a64510bff3db5b1d223b0be70ad04898ce784fb`: USB storage hotplug #45780898833 PASS and AI handoff #45780898695 PASS on that exact push SHA.
- Process lifecycle inventory/fail-closed, DDNSTO, DNS Forwarder and AI checks on `dd2885fc9941d3a57fd9c81cf01ed86488b0183d` passed. Its XY-C1 run #45779633237 also passed Build XY-C1, TRX verification and artifact upload; artifact #15098382009 exists (12,823,167 bytes). This is the current verified product-source L3 baseline; later `7a64510b` changed checker/docs only. Physical router L4 remains NOT VERIFIED.
- Current slice closes the P1 IPTV proxy altnet validation-integration gap. The source already parses strict dotted-decimal IPv4 and bounds CIDR values to 0–32; one-digit `/9` is valid (not malformed). Added an explicit positive case, wired both existing C regressions and `scripts/check_iptv_nvram_status.py` into the WAN DHCP callback workflow, and added all direct file dependencies to matching push/pull_request filters. No production source changed.

## Verification boundary

- A standalone GNU89 `-Wall -Wextra -Werror` ASan/UBSan harness built from the current altnet policy logic passed valid /0, /8, /9, /24 and /32 values, malformed/out-of-range inputs, octal aliases, the broadcast sentinel, and empty-value default normalization. This is not the full repository C suite.
- No repository checkout is available in this environment, so the original C tests and Python source contract have not been run locally. The new exact-head WAN DHCP workflow, AI handoff workflow and resulting XY-C1 behavior are pending after this commit.
- Test/workflow-only changes do not create a new product-source L3 baseline. Do not treat skipped Build/TRX/artifact steps as firmware verification.

## Exact first action for the next GPT

1. Fetch the live `main` SHA and inspect exact-head WAN DHCP callback and AI handoff Actions. Confirm the new GNU89/ASan/UBSan NVRAM test, IGMP atomic-config test and `check_iptv_nvram_status.py` all execute and pass; diagnose the first causal failure if not.
2. Inspect exact-head XY-C1 behavior. If Build/TRX/artifact are skipped because this commit changes only tests/workflow/docs, retain `dd2885fc9941d3a57fd9c81cf01ed86488b0183d` as the product-source L3 baseline; do not claim a new L3 result.
3. Then continue the P1 WAN/IPv6 stability backlog: trace route install/withdrawal and first-error cleanup in `trunk/user/rc/src/net_wan6.c` against `wan6_teardown_policy.h` and its regression; implement only a confirmed defect, with a focused regression and exact-head Actions.

## Execution invariants

- Main only. Each logical slice creates every blob, one complete tree and one atomic commit, then updates `main` non-forced against the expected parent SHA. Re-fetch `main` immediately before every write; on CAS conflict, rebuild against the new parent. Never force-push.
- Preserve the frozen backup; do not reopen PR #1 or create other branches/PRs.
- L3 requires same-product-source-SHA Build XY-C1, TRX verification and artifact upload all PASS. Physical-router L4 requires actual device testing.
- Update this handoff only when the active task, baseline, validation boundary or first next action materially changes; no poll-only/docs-only commits.
