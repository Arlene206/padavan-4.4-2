# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `cee3941a374c410a208f64aef8fa3f214d5d2be1`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

Audit the SIT 6rd activation tuple: the DHCP lease writer publishes `wan0_6rd_size` and `wan0_6rd_relay` under one NVRAM transaction, but SIT startup previously read them independently and could combine two lease generations. This commit adds an atomic two-field reader and pre-mutation regression; inspect exact-head WAN DHCP and XY-C1 Build/TRX/artifact.

## Verification boundary

- Exact `cee3941a374c410a208f64aef8fa3f214d5d2be1` Actions: WAN route #45045448318 PASS; HTTPD security #45045448442 PASS; firewall #45045448338 PASS; USB modem identity #45045448331 PASS; Process inventory/lifecycle #45045448370 PASS; AI handoff #45045448321 PASS.
- XY-C1 #45045448352 completed host regressions successfully, but Build, TRX verification and artifact upload were all SKIPPED because the parent commit changed only checkers/docs, not firmware production inputs. Do not claim L3 from this host-only workflow.
- The present change modifies `net_wan6.c`, which is included in the XY-C1 push path filter and should trigger a product build on the new exact SHA. Confirm Build, TRX and artifact upload individually.
- No local tests/build were run by this session. L3 is NOT VERIFIED until exact-source Build/TRX/artifact PASS; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route # (including the DNS-read script contract) and HTTPD # jobs; verify all updated source-contract steps pass.
2. Inspect XY-C1 host validation and actual Build, TRX and artifact-upload steps on the exact same source SHA. Do not reuse ancestor artifacts.
3. Continue an independent stability/security defect while Actions are pending; keep the frozen backup read-only and work only on main.
