# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `e49224f71f90339247917b66d8a9add54228e8a1`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

Prevent resolver DNS leaks when VPN-only DNS policy cannot be read. `update_resolvconf()` previously used lossy `nvram_get_int("vpnc_pdns")` and `nvram_safe_get("vpnc_dns_t")`; backend errors could become policy 0/empty list, after which the code deliberately falls back to WAN or public DNS. This change snapshots both values in one checked NVRAM transaction and aborts before staging `/etc/resolv.conf` on any backend/close/parse/truncation error, preserving the last-known-good resolver file. Missing values retain prior fallback semantics.

## Verification boundary

- Exact `e49224f71f90339247917b66d8a9add54228e8a1` WAN route #45047446525 PASS; AI handoff #45047446469 PASS.
- Exact parent `690dc90846b959d05a5ee51e8f5d9cfa5c960408`: WAN DHCP callback #45047262363 PASS (including SIT 6rd tuple tests); ZeroTier #45047262287 PASS; Samba #45047262278 PASS; WAN signal tests #45047262383/#45047262340 PASS; Wireless #45047262438 PASS; IPv6 service type #45047262463 PASS; AI handoff #45047262377 PASS.
- XY-C1 #45047262320 for source `690dc9` was still in the `Build XY-C1` step, with TRX/artifact pending on last inspection. L3 remains NOT VERIFIED until same-source Build/TRX/artifact all pass.
- No local tests/build or hardware validation was performed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route # (including the DNS-read script contract) and HTTPD # jobs; verify all updated source-contract steps pass.
2. Inspect XY-C1 host validation and actual Build, TRX and artifact-upload steps on the exact same source SHA. Do not reuse ancestor artifacts.
3. Continue an independent stability/security defect while Actions are pending; keep the frozen backup read-only and work only on main.
