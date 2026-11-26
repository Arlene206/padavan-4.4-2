# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development stays on `main`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and must not be modified/deleted. PR #1 remains closed/unmerged.
- Parent main for this VPN route slice: `fe68776c6f7421a6442b770fea4b8f449bb38651`. Startup/protocol recovery was completed once; do not repeat bootstrap or backup audits.

## Active task

- VPN client callback route NVRAM correctness in `trunk/user/rc/src/vpn_client.c`.
- `vpnc_route_to_remote_lan()` previously retained four pointers from consecutive `nvram_safe_get()` calls. The NVRAM policy documents that getter results must be copied before later reads; subsequent reads could reuse the shared cache buffer before comparison/use. Backend/close failures were also mistaken for absent strings, silently skipping a configured remote-LAN route.
- `vpnc_route_dgw()` similarly used unchecked `nvram_match("vpnc_dgw","1")`, which could omit the default route on read errors. This slice adds checked readers, immediate bounded copies and a tested exact-match policy preserving absent=false and legacy exact-string semantics.
- Regression assets: `trunk/user/rc/src/vpn_client_nvram_policy.h`, `trunk/user/rc/src/tests/vpn_client_nvram_policy.c`, and `scripts/check_vpn_client_nvram_state.py`. Existing `.github/workflows/vpn-server-stability.yml` watches modified paths and compiles the GNU89/Werror policy regression plus source checker.

## Verification boundary

- Firewall 6in4 endpoint fix SHA `fe68776c6f7421a6442b770fea4b8f449bb38651`: Firewall startup #45777134071, WAN route #45777134084, IPv6 service-type #45777134082, FULLCONENAT #45777134262, WAN DHCP callback #45777134115, Process inventory #45777134074 and HTTPD security #45777134070 PASS. NVRAM audit #45777134104 was IN_PROGRESS at last check; XY-C1 #45777134107 was at Build with TRX/artifact pending. AI handoff #45777134054 failed because the compact handoff lacked `Mandatory handoff checkpoint` and the exact conditional-edit sentence; this version restores both plus the cross-session execution mandate.
- SmartDNS UI-key correction SHA `8431202d8ce320b6df72b87d183749c44bea7f50`: DNSMasq #45776763963, Process inventory #45776764005, Process fail-closed #45776764177, USB #45776764153 and AI docs #45776764152 PASS. XY-C1 #45776764204 completed successfully, but firmware Build, TRX verification and artifact upload were SKIPPED (test-only change); no L3 claim.
- MiniUPnP SHA `588beb5d574127fd4d0027559d9c8cb97e75af3d`: HTTP framing #45775186800 and AI docs #45775186907 PASS.
- IAPP SHA `3bb7d977b30525c202009630892ea0afa94a0d9a`: IAPP #45773717841, Process inventory #45773717856, NVRAM #45773717775, AI docs #45773717816 and XY-C1 #45773717864 PASS including Build/TRX/artifact.
- VPN route slice exact-SHA Actions start after publication. This environment has no usable local checkout; no local/router test or physical L4 claim.

## Exact first action for the next GPT

1. Fetch live `main`, identify the VPN route SHA, and inspect exact-SHA VPN server stability (policy test and `check_vpn_client_nvram_state.py`), Firewall startup, Process inventory/fail-closed, WAN route/DHCP, NVRAM audit, AI handoff and XY-C1 runs. Diagnose failures from the first causal job log.
2. Close outstanding firewall SHA `fe68776c6f7421a6442b770fea4b8f449bb38651` NVRAM audit #45777134104 and XY-C1 #45777134107, checking TRX and artifact separately. Keep them distinct from the newer VPN SHA.
3. Continue the next concrete high-risk userland reliability/security fix from `REFACTOR_ROADMAP.md`; do not repeat completed SmartDNS, MiniUPnP, IAPP or firewall endpoint audits.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live repository state and relevant exact-head Actions before ending. Record material PASS/FAIL/PENDING changes and confirm the first next action remains actionable. If all facts and the first action remain accurate, explicitly conclude with no file change and no commit.

# Cross-session execution mandate

Continue one autonomous engineering stream across GPT conversations. Every logical source/test slice must create all blobs, one complete Git tree and one atomic main commit, then publish with a non-forced compare-and-swap update. Before context exhaustion, finish the current safe slice and leave the next GPT an accurate exact-SHA state plus a concrete first task. Do not ask the user what to do next.

## Execution invariants

- Main only; one logical slice = all blobs, one complete tree, one atomic commit, then non-forced expected-parent SHA update. On CAS conflict, refresh and rebuild; never force-push.
- Never modify the frozen backup or reopen PR #1.
- Host Actions do not prove MIPS/uClibc runtime. XY-C1 L3 requires same-SHA Build, TRX verification and artifact upload PASS. Hardware L4 remains unverified absent explicit test.
- Update handoff only when engineering baseline, verification boundary or first concrete action materially changes; no poll-only or SHA-only commit.
