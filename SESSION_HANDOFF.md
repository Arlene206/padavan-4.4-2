# 2026-11-18 continuation checkpoint — dnsmasq transaction source contract correction

## Current repository state
- Repository: Arlene206/padavan-4.4-2; active branch main. This checker follow-up is based on exact parent 8e2121694004a1fdd79150a6e8107f845f2eea80. Refresh main before any further write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807; PR #1 remains closed/unmerged.
- Exact 8e Actions: Process lifecycle inventory 44291689977 PASS; Process inventory umbrella 44291689958 PASS; AI handoff docs 44291690049 PASS. Shadowsocks 44291689941 passed syntax, required startup, process lifecycle (including stop-timeout fixture) and Adblock runtime tests, but failed the static source checker because one remaining assertion still expected the pre-normalized $ss_chdns argument instead of $use_chinadns. This follow-up fixes that assertion. Exact XY-C1 was not triggered by the test/docs-only 8e commit; source commit cb6d XY-C1 44291015307 was still IN_PROGRESS at last poll.
- Exact cb6d AI docs failed because the handoff omitted a cross-session rule; 3ec corrected that rule but revealed the required heading; 8e restores all mandatory sections and the AI docs Action passed.

## Active task
- ChinaDNS-ng startup validates non-empty upstream and UNKNOWN-safe process inventory, avoids duplicates, and requires a bounded RUNNING postcondition.
- DNSMasq publication stages/backs up configuration, validates with dnsmasq --test -C, confirms process stop before atomic publish, verifies startup with bounded process inventory and rolls back safely when startup fails. The standalone launcher keeps global dnsmasq outside Shadowsocks cgroups.
- The ChinaDNS active flag is published only after the config transaction and service readiness succeed. Runtime shell regressions now pass: successful apply, syntax rejection, stop-timeout fail-closed behavior, restart rollback and cgroup isolation.
- The current exact-head Shadowsocks failure is only a stale static checker assertion; source arguments were intentionally normalized to "$use_chinadns".

## Verification boundary
- Exact 8e runtime shell tests, startup regression and Adblock regression passed. Process inventory/umbrella and AI handoff Actions passed. The exact-head Shadowsocks source checker failed on the stale "$ss_chdns" static assertion; correction pending new exact-head check.
- XY-C1 L3 remains unverified for the cb6d product source until run 44291015307 concludes Build XY-C1, TRX verification and artifact upload. No target BusyBox runtime or physical-router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and exact-head Actions for this checker correction; fix any real regression rather than merely updating text.
2. Confirm the cb6d XY-C1 Build, TRX check and artifact upload result; run a fresh exact-source build if it failed or source has changed.
3. Continue inspecting the oversea and ssp_close DNSMasq configuration paths. The RC restart_dhcpd implementation holds /tmp/.dnsmasq.lifecycle.lock and performs tri-state stop and bounded start; the Shadowsocks helper currently directly stops/starts dnsmasq without that shared lifecycle lock. Decide whether to converge the helper onto the RC service entry point without weakening atomic rollback or cgroup isolation.
4. Add focused regressions for any source change, then broaden P1 reliability across RC/userland. Keep the frozen backup immutable and physical-router L4 unverified.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md, verifying active engineering baseline, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts are accurate, make no file change and do not create a no-op commit.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work during PENDING checks. Before context exhaustion, leave an accurate checkpoint with the exact source SHA, tests/Actions status, validation boundary and next first action. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed validation.
