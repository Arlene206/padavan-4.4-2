# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending and compare it with this handoff: engineering/source baseline, active task, exact-head Actions, validation boundary, blockers and the exact first action. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information when it changes the validation gate or next action.

## Current repository state
- Parent main before the WAN protocol checker contract repair: `b4bdfa7803678887021284b8876251043e723d85`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- WAN source commit `b4bdfa7803678887021284b8876251043e723d85` replaced `nvram_safe_get("wan_proto")` plus a separate write with `set_wan_unit_param_status(unit, "proto")`, which copies the configured protocol under the existing NVRAM transaction and propagates read/write/close errors before protocol selection.
- Exact WAN route run #44901566662 FAILED at checker step "Check WAN DNS-mode error propagation contract": its new helper assertion compared raw whitespace while source layout is multiline; the C policy binary and earlier checker sections passed. XY-C1 #44901566573 FAILED at "Verify WAN firewall result propagation" because that separate checker still expected the old unsafe `nvram_safe_get` source marker. Build XY-C1, TRX verification and artifact upload were skipped because the pre-build source contract failed; this is not a firmware compiler failure.
- AI handoff docs #44901566570 FAILED because this handoff omitted the validator's exact sentence `The checkpoint is mandatory; a document edit is conditional.`; this repair restores its wording. Firewall #44901566561, process inventory #44901566665, WAN DHCP #44901566634, teardown #44901566648, HTTPD #44901566615, watchdog #44901566575, ZeroTier #44901566677, miniupnpd #44901566667, wireless #44901566613, VPN server #44901566558, DNSMasq #44901566564, detect_link #44901566586, detect_internet #44901566699, USB modem #44901566582 and process regression #44901566595 PASS on `b4bdfa78`. NVRAM transaction audit #44901566629 remained IN_PROGRESS at last poll.
- Frozen backup remains unchanged; legacy PR #1 closed/unmerged.

## Active task
- Rerun the exact-head WAN route, XY-C1 and AI handoff checks after repairing only their stale literal assertions / exact checkpoint wording.
- For WAN route checker, retain the source proof that checked NVRAM transaction begins, reads, validates status, and stops before writing `wan0_proto`; whitespace normalization must not weaken semantic or call-order checks.
- For XY-C1, verify protocol copy now uses the checked transaction before `get_wan_proto(unit)`. Once source contracts pass, inspect actual Build XY-C1, TRX and artifact steps independently.
- Then continue the P1 DNSMasq hardening task: checked-read security-sensitive `dhcp_stop_dns_rebind` and `dhcp_proxy_dnssec` (and associated resolver options) before staging a replacement config, preserving the last-known-good config on read failure.

## Verification boundary
- WAN protocol host policy binaries in #44901566662 passed; the new script's prior WAN DNS, PPPoE, MTU, authentication, route and related checks passed before its whitespace-only assertion failed.
- Exact WAN route #44901566662 and XY-C1 #44901566573 did not reach a PASS. In the XY-C1 run Build/TRX/artifact steps were SKIPPED because the stale source checker failed first.
- AI docs #44901566570 failed only on missing exact checkpoint sentence, now restored by this handoff repair.
- Full MIPS image, TRX artifact and hardware/router L4 remain NOT VERIFIED for `b4bdfa78`.

## Exact first action for the next GPT
1. Fetch current main and inspect exact-head WAN route, XY-C1 and AI handoff Actions for the checker/handoff repair.
2. Confirm WAN transactional-copy source checks, XY-C1 firewall reset preconditions, and AI handoff checkpoint validation all pass.
3. Verify Build XY-C1, TRX output verification and artifact upload separately; do not infer any from a skipped job.
4. Continue checked DNSMasq option reads before config staging, with GNU89 host coverage and source-order regression tests.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first next action materially changes; no no-op commit.
