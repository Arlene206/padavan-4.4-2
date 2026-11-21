# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending and compare it with this handoff: engineering/source baseline, active task, exact-head Actions, validation boundary, blockers and the exact first action. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information when it changes the validation gate or next action.

## Current repository state
- Parent main before this DNSMasq hardening slice: `04b8537ac3810740e15ec901fe14594b7383d3df`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- WAN protocol copy fix is commit `b4bdfa7803678887021284b8876251043e723d85`. A checker-only repair commit `04b8537ac3810740e15ec901fe14594b7383d3df` now normalizes whitespace in the transactional helper assertion, updates XY-C1's protocol reset assertion to the checked transaction helper, and restores the exact AI handoff checkpoint wording. Its exact Actions are not yet polled.
- Previous b4 Actions: WAN route #44901566662 and XY-C1 #44901566573 failed due stale source checker assertions; XY-C1 Build/TRX/artifact were SKIPPED because the contract step failed before build. AI docs #44901566570 failed because the exact conditional checkpoint sentence was omitted. Firewall #44901566561, Process inventory #44901566665, WAN DHCP #44901566634, WAN teardown #44901566648, HTTPD #44901566615, watchdog #44901566575, ZeroTier #44901566677, miniupnpd #44901566667, wireless #44901566613, VPN #44901566558, DNSMasq #44901566564, detect_link #44901566586, detect_internet #44901566699 and USB modem #44901566582 passed on b4. NVRAM audit #44901566629 was in progress.
- Firewall fail-closed NVRAM commits remain `ee01141a` and `4a02532f`; these enforce checked `fw_enable_x`, `fw_dos_x`, `macfilter_enable_x`, `fw_mac_drop`, preserve absent-key defaults, and abort core table staging/rollback on read errors.
- This DNSMasq slice closes a resolver-policy gap: five `nvram_match()` reads could silently remove configured filter-AAAA, all-servers, strict-order, DNS-rebinding protection or DNSSEC proxy directives after an NVRAM backend failure. The new checked reader runs before any staged host/resolver/config writes and preserves the last-known-good configuration.

- On exact SHA `232e31d7eeaa5a6a5e7510ce74a59d9b2f9a7950`, DNSMasq #44902070166 passed the new GNU89/ASan policy binary and preceding lifecycle regressions, then hit a checker `ValueError` caused by an over-escaped newline in the new C signature. WAN route #44902070187 passed WAN DNS-mode, MTU, EAPOL and firewall propagation checks, then failed the new ternary token only because the checker expected extra outer parentheses. This commit corrects both checker expressions; source behavior is unchanged. AI docs #44902070141, miniupnpd #44902070251, HTTPD #44902070259, Samba #44902070353, Inadyn #44902070265, WAN DHCP #44902070191 and Process inventory #44902070152 passed. XY-C1 #44902070311 and NVRAM audit #44902070127 were in progress at last poll.

- On SHA `2a2238af`, the DNSMasq option source checker uses the actual `prepare_dns_dhcpd()` newline signature. A final whitespace-normalization review found the five multiline option calls compact to one space after `(`, so the checker is now aligned to that representation without changing production code.

## Active task
- Verify exact-head WAN route, XY-C1 and AI handoff reruns for `04b8537a`, and DNSMasq regression plus XY-C1 for this new source slice.
- For DNSMasq, confirm all five checked reads happen before DHCP range persistence, hosts writes, resolver file creation and opening staged config. Missing values retain false defaults; API/read errors abort before new config publication.
- Inspect Build XY-C1, TRX validation and artifact upload as separate steps; distinguish source-contract failure from actual firmware compilation.
- Continue wider userland P1 security/stability work once these checks are understood; do not return to repeated audits of completed firewall changes.

## Verification boundary
- WAN protocol source regression on b4 previously failed only because its new checker expected a whitespace-specific helper string; XY-C1 WAN firewall assertion likewise expected the removed `nvram_safe_get("wan_proto")` call. Both checker contracts are corrected in `04b8537a`; exact reruns pending.
- AI handoff docs exact sentence is restored in `04b8537a`; exact rerun pending.
- DNSMasq standalone policy binary: GNU89, `-Wall -Wextra -Werror`, ASan/UBSan: PASS locally. Full `services_ex.c` RC cross-build, MIPS firmware, TRX/artifact and hardware router behavior are pending.
- Backup SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 hardware claim.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head DNSMasq, WAN route, AI docs and XY-C1 Actions for the current main SHA.
2. Confirm checker fixes on `04b8537a` pass, then inspect the new DNSMasq option policy test/source contract.
3. Verify Build XY-C1, TRX output and uploaded artifact independently; no build success claim while earlier checker steps are skipped.
4. Continue one new P1 userland reliability/security defect outside the just-fixed DNSMasq/WAN/firewall areas.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first next action materially changes; no no-op commit.
