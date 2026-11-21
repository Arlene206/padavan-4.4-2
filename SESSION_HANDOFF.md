# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD at this handoff refresh: `335fddd5b61d20e22987da948136c412cf4bbf81`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- Latest product-source commit 335fddd5b61d20e22987da948136c412cf4bbf81 hardens WAN-down gateway /32 ownership: checked reads are insufficient unless the prior IP passes dhcp_lease_ipv4_is_valid() and the mask passes dhcp_lease_netmask_is_valid() before subnet classification. Invalid values accumulate EINVAL and cannot authorize /32 deletion.
- 335f Actions already PASS: WAN gateway ARP #44927981584, wireless radio #44927981647, USB modem identity #44927981598, ZeroTier #44927981613, detect_link #44927981597, router-mode NVRAM #44927981587, firewall #44927981595, VPN server #44927981510, detect_internet #44927981557, watchdog #44927981486, AI handoff #44927981573 and WAN interface teardown #44927981495.
- 335f Actions still running at the last poll: XY-C1 Build #44927981534; Process integration #44927981517 and Process inventory #44927981473; WAN DHCP callback #44927981545; NVRAM audit #44927981607; DNSMasq #44927981567; HTTPD security #44927981580; miniupnpd config #44927981612.
- Exact-head 285f WAN route #44927593606 failed because its inline contract called require() without defining it. Process inventory #44927593544 failed because its contract still required the retired telnetd raw nvram_match gate. This slice defines require() in the inline script and validates telnetd's checked service_nvram_match_status() error/disabled branches.
- Next: inspect exact-head WAN route and Process integration/inventory runs plus XY-C1 Build/TRX/artifact. Fix failures from logs; then assess whether gateway/address/netmask should be fetched in one transaction to avoid mixed tuples during concurrent NVRAM writes.

## Verification boundary

- L1: gateway route-selection policy covers NVRAM read errors and invalid prior address/netmask; the WAN route workflow runs the GNU89 policy test under ASan/UBSan. Exact-head WAN interface teardown passed on 335f.
- L2: WAN ARP, wireless, USB modem, ZeroTier, detect_link, router NVRAM, firewall, VPN, detect_internet, watchdog and AI handoff suites passed on 335f. Process, WAN DHCP, NVRAM audit and XY-C1 remained in progress at last poll. Exact corrected WAN route/Process source contracts are pending.
- L3: NOT VERIFIED until XY-C1 Build, TRX verification and artifact upload all PASS on one exact product-source SHA.
- L4: hardware route ownership and concurrent teardown/callback behavior remain NOT VERIFIED.

## Workflow and preservation rules

- Main only. One logical slice, one atomic main commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected_sha. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
- Re-fetch main immediately before writing. On a moved ref, discard the stale prepared tree and rebuild from the new parent.
- Inspect push Actions only when head_sha exactly matches the commit. Diagnose failures from their logs. PENDING does not block independent work.
- Preserve security, architecture, ABI, GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Never weaken useful tests or focused workflow path coverage.
- Do not touch GPT Session Rotator workflow/state or replay SHA 9d5a5cb. The backup branch is permanent immutable recovery data.

## Cross-conversation documentation governance

- GitHub is the durable cross-conversation record; the one-line user entrypoint remains sufficient.
- dev_log.md stays newest-first with 20–119 complete records. Before record 120, archive the oldest 100 complete records verbatim into docs/dev-log/YYYY/MM/part-NNN.md in one batch, leaving 20 newest entries.
- Rotate early only above the 384 KiB target and never exceed 512 KiB. Preserve archive/index bytes. Git history is the version archive for handoff and roadmap; routine snapshots and SHA-chasing commits are prohibited.
- Update dev_log.md for real milestones, this handoff when first-next-action facts materially change, and REFACTOR_ROADMAP.md only when repository-wide module status/priorities materially change.
- Before context exhaustion, finish the safe slice, commit atomically, inspect exact-head Actions and leave the next precise task.

## Exact first action for the next GPT

Check exact-head WAN route, infosvr, crond, Process integration, Process inventory and XY-C1 Build/TRX/artifact Actions for this regression-alignment commit. Repair any remaining failures from exact logs. Then harden WAN teardown host-route classification against missing/invalid address or netmask values while retaining independent cleanup, and continue the WAN control-plane audit.

