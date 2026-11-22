# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `034ddac2a662f27510e6ce4735efdbd9b6a07e80`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

The checked SIT 6rd snapshot helper and its GNU89 regressions passed the WAN DHCP workflow on `b4c5967`. The WAN route source checker then failed twice on stale assertions: first it expected removed split 6rd keys/getters; after that was updated, it failed because the newly-added test assertions referenced `sit_policy_test` before its assignment. This checker-only commit moves the assignment above those assertions. Re-run WAN route on the new SHA and inspect XY-C1 Build/TRX/artifact on the exact production source SHA `b4c5967`.

## Verification boundary

- Exact source SHA `b4c59675fa4d34edfe4bb5a28a590aff5b7f2313`: WAN DHCP callback #45046472587 PASS, including GNU89 compile/execute of the two-field SIT 6rd snapshot tests; IPv6 service type #45046472568 PASS; Samba #45046472427 PASS; WAN ARP #45046472504 PASS; watchdog #45046472395 PASS; Process inventory #45046472496 PASS; Process lifecycle #45046472508 PASS; HTTPD #45046472439 PASS; USB modem #45046472548 PASS; AI handoff #45046472552 PASS.
- WAN route #45046472456 on `b4c5967` first failed at `scripts/check_wan_dns_read_status.py:1191` due to the removed `"6rd_size"`/`"6rd_relay"` split-read source assertions. The rerun #45046635467 on `034ddac` got past that check and failed with `NameError: sit_policy_test is not defined`; the present commit fixes the assignment ordering. Policy binaries and all earlier route assertions passed in both runs.
- XY-C1 #45046472404 is the actual production-input validation triggered by `net_wan6.c` on `b4c5967`; Build was in progress at the last check, with TRX/artifact still pending. L3 remains NOT VERIFIED until exact-source Build, TRX and artifact upload all pass.
- No local test/build or hardware validation was performed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route # (including the DNS-read script contract) and HTTPD # jobs; verify all updated source-contract steps pass.
2. Inspect XY-C1 host validation and actual Build, TRX and artifact-upload steps on the exact same source SHA. Do not reuse ancestor artifacts.
3. Continue an independent stability/security defect while Actions are pending; keep the frozen backup read-only and work only on main.
