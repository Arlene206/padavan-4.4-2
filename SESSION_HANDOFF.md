# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Current main source baseline: `c34e54b200272f17682cab1c98a2ad922ef43728` (checked Inadyn DDNS-enable startup gate). Modem WAN link-change reconnect NVRAM snapshot is in `33e87d4540698ef87226715fd3fa8e967243cb01`; its USB modem embedded checker correction is in `97c7286aedd02a97e784482981fa2c51d916297e`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses one checked NVRAM transaction for `pppoemwan_enable` and (APP_RPL2TP L2TP only) `wan_l2tpd`; same immutable flags drive lock selection, daemon stop/start, multi-WAN startup and rollback.
- WAN PPP source contracts, VPN lifecycle, WAN route, WAN DHCP, HTTPD, modem, firewall, IPv6 router-hosts and LAN DHCP checks have been reconciled with their checked APIs.
- `SESSION_HANDOFF.md` and `dev_log.md` are the only live root checkpoint/history records. Do not repeat startup audits or create routine history snapshots.

## Active task

`start_ddns_locked()` previously called unchecked `nvram_match("ddns_enable_x", "1")` after stopping the running Inadyn daemon. A transient NVRAM error could therefore be treated as a disabled setting and leave DDNS offline. Commit `c34e54b200272f17682cab1c98a2ad922ef43728` now reads the flag with `nvram_get_checked()` and the existing `inadyn_ddns_enabled_status()` policy before process scan/stop; read failure returns immediately without touching the daemon. Confirmed disabled state still performs the legacy stop. The existing GNU89/ASan/UBSan policy test covers backend error mapping, and `check_inadyn_lifecycle.py` now pins the no-mutation-before-read-error order. Earlier modem work: source `33e87d4540698ef87226715fd3fa8e967243cb01` added the checked link-change recovery policy; `97c7286aedd02a97e784482981fa2c51d916297e` updated the duplicated embedded workflow checker and exact USB modem #45032701038 PASS.

## Verification boundary

- Local tests were not run in this connector-only session.
- Parent `b443b54cf920195df1405e4a172ec0ed7f9b1f09`: USB modem #45031581333 PASS; firewall startup #45031581354 PASS; process inventory/lifecycle #45031581442 PASS; AI handoff #45031581408 PASS.
- Parent XY-C1 #45031581390 workflow SUCCESS: host C/Python regressions and source contracts passed, but Build XY-C1, TRX verification and artifact upload were intentionally skipped because that parent commit contained no firmware production input changes. This is not an L3 build.
- The WAN PPP selector source implementation and downstream contracts are committed in `d3805365ca95a17d5bd54736f29d5627580fa305` and `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`. Their early XY host attempts stopped at stale source contracts before the cross-build. The next source-triggered XY run must complete Build, TRX and artifact upload on the same SHA before L3 can be claimed.
- Earlier exact runs: IPv6 service type #45031159848 PASS; WAN DHCP #45031431084 PASS; VPN lifecycle #45031330323 PASS; WAN route #45031330345 PASS; AI handoff #45031330331 PASS; NVRAM audit #45031084347 PASS. These validate their respective ancestor SHAs only.
- Exact SHA `b3caebcbaba0924abcb83339d49a1968e237f042`: WAN DHCP #45032049974 PASS; VPN #45032050106 PASS; WAN route #45032049989 PASS; HTTPD #45032050007 PASS; process #45032050136 PASS; NVRAM #45032049962 PASS. AI docs #45032049968 was corrected by `b0338e1be6b52f0f43cc644253565675fc243138` (AI handoff #45032198181 PASS). XY-C1 #45032049960 remained in_progress at last observation with production Build started; TRX/artifact have no completed evidence.
- Exact source SHA `33e87d4540698ef87226715fd3fa8e967243cb01`: USB modem #45032550398 initially failed only at the obsolete embedded link-change source assertion; modem policy C tests and earlier C test steps PASS. Checker fix SHA `97c7286aedd02a97e784482981fa2c51d916297e` reran USB modem #45032701038 PASS.
- Current Inadyn source SHA `c34e54b200272f17682cab1c98a2ad922ef43728`: Inadyn #45032859299, DNSMasq #45032859372, HTTPD #45032859349, XY-C1 #45032859323, Process inventory #45032859409, WAN route #45032859347, miniupnpd #45032859331, Process lifecycle #45032859298, NVRAM #45032859363, WAN DHCP #45032859313, Samba #45032859405 and AI docs #45032859440 were all in_progress at last poll. No result is yet claimed for this SHA.
- Hardware L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA Inadyn, DNSMasq, WAN route/DHCP, miniupnpd, process, NVRAM, Samba, HTTPD, AI and XY-C1 Actions for `c34e54b200272f17682cab1c98a2ad922ef43728`. Fix any concrete failure; leave unrelated PENDING jobs running.
2. Recheck XY-C1 #45032049960 for the PPP change and current XY-C1 #45032859323 for the Inadyn change; L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA.
3. Continue the broad userland stability/security rotation after the Inadyn read fix. Keep the backup immutable; hardware L4 remains unverified.
