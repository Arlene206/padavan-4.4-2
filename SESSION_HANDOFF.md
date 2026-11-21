# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending and compare it with this handoff: engineering/source baseline, active task, exact-head Actions, validation boundary, blockers and the exact first action. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information when it changes the validation gate or next action.

## Current repository state
- Parent main before the URL-filter fail-closed slice: `dffca184e64dc44d8cfae11a43bbcebe21d9e0b6`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Firewall global enable, DoS and MAC policy fail-closed changes are committed in `ee01141a` and `4a02532f`. Exact Firewall runs #44900939770 and #44901320517 passed respectively; AI/Process checks also passed.
- WAN protocol reset now uses checked transactional `set_wan_unit_param_status(unit, "proto")` (source commit `b4bdfa78`). WAN source-contract whitespace issues were fixed in the script; exact WAN route run #44902460823 and AI handoff #44902460812 both PASS on `dffca184e64dc44d8cfae11a43bbcebe21d9e0b6`.
- DNSMasq now checks `dhcp_filter_aaaa`, `dhcp_all_servers`, `dhcp_strict_order`, `dhcp_stop_dns_rebind`, and `dhcp_proxy_dnssec` before generating/publishing config. Exact DNSMasq #44902327424 PASS and AI docs #44902327438 PASS on `5a4366981557ddef27f4bb0717543f84b353b36c`.
- XY-C1 #44902070311 on the production input SHA `232e31d7eeaa5a6a5e7510ce74a59d9b2f9a7950` was still building when last checked: Build XY-C1 IN_PROGRESS, TRX and artifact steps PENDING. This exact run has already passed the full pre-build source/test suite, including WAN reset, firewall and detector contracts. No firmware/TRX/artifact result is claimed yet.
- New firewall review found three additional unchecked filtering switches: `url_enable_x` enabled URL/webstr filtering, `fw_lw_enable_x` enabled LWF chain generation, and `url_inv_x` selected the inverse MAC match used by URL filtering. Their `nvram_match()` calls collapsed backend errors to false, silently removing configured URL/LWF policy from a regenerated firewall ruleset.
- Frozen backup remains immutable; legacy PR #1 remains closed/unmerged.

## Active task
- Verify exact-head Firewall Startup Stability and XY-C1 actions for the URL/LWF fail-closed change.
- Ensure IPv4 checks URL enable, LWF enable and (when URL filtering is active) URL inverse mode before opening the staged filter file. Ensure IPv6 checks URL enable and inverse mode likewise. Backend/invalid read errors must abort the existing core restore transaction; missing keys retain false defaults.
- Confirm both `url_inv_x` call sites consume the checked local state and no unchecked match remains for these three settings.
- After exact tests, continue independent P1 stability/security work in another Padavan userland subsystem, not another broad audit of already-fixed DHCP/DNS/WAN paths.

## Verification boundary
- Latest standalone firewall boolean policy fixture (GNU89, `-Wall -Wextra -Werror`, ASan/UBSan) passed on prior firewall changes; this slice reuses that unchanged helper.
- WAN route run #44902460823 PASS and AI docs #44902460812 PASS on `dffca184`.
- DNSMasq run #44902327424 PASS and AI docs #44902327438 PASS on `5a436698`.
- XY-C1 run #44902070311 on `232e31d7`: all pre-build source/test steps PASS; Build XY-C1 still IN_PROGRESS at last poll, TRX/artifact PENDING. No L3 claim.
- Current URL/LWF source test and firmware build are pending exact-head Actions; hardware/router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Firewall Startup Stability and XY-C1 results for this URL/LWF slice.
2. Confirm the five URL/LWF checked gates, no remaining direct `nvram_match()` for those settings, and GNU89 regression results.
3. Inspect Build XY-C1, TRX verification and artifact upload separately; do not conflate pending build with completed tests.
4. Continue one confirmed P1 defect in a separate userland subsystem, preserving the atomic main workflow.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first next action materially changes; no no-op commit.
