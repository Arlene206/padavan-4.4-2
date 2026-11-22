# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `b4c59675fa4d34edfe4bb5a28a590aff5b7f2313`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

The SIT 6rd size/relay tuple now reads through one NVRAM transaction and its WAN DHCP regression passed. The exact-head WAN route workflow then failed only because `scripts/check_wan_dns_read_status.py` still required the removed independent `6rd_size`/`6rd_relay` getters inside `start_sit_tunnel()`. This commit updates that second source contract to require the new snapshot helper and pre-mutation ordering. Re-run WAN route on the new SHA, then verify XY-C1 Build/TRX/artifact on the exact source SHA.

## Verification boundary

- Exact parent SHA `cee3941a374c410a208f64aef8fa3f214d5d2be1`: WAN route #45045448318, HTTPD security #45045448442, firewall #45045448338, USB modem identity #45045448331, Process lifecycle #45045448370, AI handoff #45045448321 all PASS. XY-C1 #45045448352 host regressions PASS but Build/TRX/artifact were SKIPPED because the parent commit had no firmware-source changes.
- Exact source SHA `b4c59675fa4d34edfe4bb5a28a590aff5b7f2313`: WAN DHCP callback #45046472587 PASS, including compilation/execution of the new checked NVRAM snapshot regressions; IPv6 service type #45046472568, Samba #45046472427, WAN ARP #45046472504, watchdog #45046472395, Process inventory #45046472496, HTTPD #45046472439, USB modem #45046472548 and AI handoff #45046472552 PASS.
- WAN route #45046472456 FAILED at the final `scripts/check_wan_dns_read_status.py` SIT assertion after all earlier policy/compiler regressions passed. The stale contract required the old split `6rd_size`/`6rd_relay` getters; it did not indicate a production regression. Updated in the present checker-only commit.
- XY-C1 #45046472404 was IN_PROGRESS when handed off; Build dependencies were installing and Build/TRX/artifact had not yet reported. Verify the same-SHA production steps; L3 is NOT VERIFIED until all three pass.
- No local test/build or hardware validation was performed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route # (including the DNS-read script contract) and HTTPD # jobs; verify all updated source-contract steps pass.
2. Inspect XY-C1 host validation and actual Build, TRX and artifact-upload steps on the exact same source SHA. Do not reuse ancestor artifacts.
3. Continue an independent stability/security defect while Actions are pending; keep the frozen backup read-only and work only on main.
