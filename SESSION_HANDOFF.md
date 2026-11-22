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

Audit and harden NFS export ACL generation. `write_nfsd_exports()` previously read LAN IPv4 address/netmask, fallback pair, IPv6 address/prefix and VPN mode/address selectors through independent NVRAM getters. A configuration update between reads could mix generations; an old LAN address combined with a newer, broader netmask could widen the generated NFS client CIDR. The current slice replaces those reads with one all-or-nothing 12-key NVRAM transaction snapshot, parses VPN selectors strictly, and adds cache-reuse/read-failure/close-failure regressions. Verify exact-SHA NFS and XY-C1 workflows.

## Verification boundary

- Exact parent main SHA `cd92a56405e96a08ab9c64e45cc345828ba5a37d`: WAN route #45047446525 PASS and AI handoff #45047446469 PASS. These checks did not build firmware.
- Exact source SHA `690dc90846b959d05a5ee51e8f5d9cfa5c960408`: WAN DHCP callback #45047262363 PASS, including checked SIT 6rd tuple tests; ZeroTier #45047262287 PASS; Samba #45047262278 PASS; link/detect Internet signals #45047262383/#45047262340 PASS; Wireless #45047262438 PASS; IPv6 service type #45047262463 PASS; AI handoff #45047262377 PASS. The WAN route failure on this SHA was checker-only and corrected by parent `e49224f`.
- XY-C1 #45047262320 on `690dc9` had completed host validation and reached `Build XY-C1`; TRX and artifact upload were still pending at the last check. This NFS source change modifies `services_stor.c`, included in the XY-C1 path filter; inspect the exact new SHA's Build/TRX/artifact run rather than reusing ancestor output.
- The new NFS slice is not yet verified: its exact-head NFS GNU89 regression, sanitizer/source-contract workflow and XY-C1 production stages must be checked. No local C tests, firmware build or hardware validation were performed. L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect NFS daemon workflow on the exact new main SHA: GNU89 `-Wall -Wextra -Werror` policy test, ASan/UBSan regressions and Python source contracts.
2. Inspect XY-C1 on that same SHA. Require Build, TRX validation and artifact upload all to succeed before claiming firmware verification.
3. If either workflow is pending, continue an independent stability/security fix; if one fails, diagnose the failing step without repeating completed audits.
