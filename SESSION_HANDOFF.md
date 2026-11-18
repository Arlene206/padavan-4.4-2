# 2026-11-18 continuation checkpoint — dnsmasq transaction fail-closed ordering

## Current repository state
- Repository: Arlene206/padavan-4.4-2; active branch main. This source correction is based on exact parent 745bf37b2dbaaef5fdce77b11f15e1b77632f411; refresh the live tip before any further write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807; PR #1 remains closed/unmerged.
- Exact 745 checks: Shadowsocks 44290280799 PASS; Process inventory 44290280731 PASS; umbrella Process 44290280781 PASS; AI handoff 44290280778 PASS. XY-C1 44290280891 was IN_PROGRESS at last poll; Build/TRX/artifact not yet confirmed.
- Parent e9 source check ran the shell syntax, required startup, process lifecycle and Adblock regressions successfully; its static checker failed because it retained the old inline ChinaDNS sed expectation. This was fixed in 745 and 745's Shadowsocks source contract now passes.

## Active task
- ChinaDNS-ng start validates non-empty upstream and UNKNOWN-safe process inventory, avoids duplicates, and requires a bounded RUNNING postcondition.
- DNSMasq publication stages and backs up in the config directory, validates with dnsmasq --test -C, and restarts through a dedicated count+1 PID launcher that intentionally leaves dnsmasq outside the Shadowsocks CPU/memory cgroup.
- THIS correction confirms dnsmasq is stopped before the config rename; a stop timeout/UNKNOWN keeps the live config unchanged. It restarts the old config if publication fails, stops a failed candidate before rollback, and withholds a rollback restart if candidate stop cannot be verified.
- chinadnsng_enable_flag is set only after both the DNSMasq config transaction and daemon readiness succeed. Host regression now covers stop failure without file replacement or a new launch.

## Verification boundary
- Exact-head Actions for this correction are newly pending; verify Shadowsocks lifecycle, Process inventory, umbrella Process, AI handoff and XY-C1 by exact SHA.
- On 745, Shadowsocks (44290280799), Process inventory (44290280731), umbrella Process (44290280781) and AI docs (44290280778) PASS. XY-C1 (44290280891) was pending. The new changes need a fresh exact-source firmware build/TRX/artifact before L3 can be claimed.
- No target BusyBox runtime or physical-router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and verify all Actions for this exact correction SHA. Repair any actual failing source/test/checker result.
2. Confirm Build XY-C1, TRX verification and artifact upload on the same current product-source SHA.
3. Inspect remaining DNSMasq integration paths (oversea config edits, ssp_close cleanup, restart_dhcpd service-manager coupling) for ownership and rollback races, and add focused regression coverage.
4. Continue broader RC/userland P1 reliability work while independent CI jobs remain pending; keep backup frozen and L4 unverified.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md, verifying active engineering baseline, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, make no file change and do not create a no-op commit.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work during PENDING checks. Before context exhaustion, leave an accurate checkpoint with the exact source SHA, tests/Actions status, validation boundary and next first action. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed validation.
