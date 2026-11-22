# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `45a78362cb1c4a6082514049d309508f80e9e28a`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

Verify the corrected route-source contracts across WAN DNS-read, modem WAN-start, HTTPD security and XY-C1 checks. The first b56e0b5 run passed the new C policy test but failed three contracts that still expected the old two-interface call; close all remaining stale contracts and then inspect the exact source SHA's XY-C1 Build/TRX/artifact stages.

## Verification boundary

- Exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`: HTTPD security #45044509138 PASS; AliyunDrive WebDAV #45044509155 PASS (shell syntax, QEMU MIPSel CLI, lifecycle regression and source/RC contract); AI handoff #45044509168 PASS; Process inventory #45044509233 PASS; NVRAM transaction boundary #45044509202 PASS (including new checked-get regression and all prior boundary tests).
- The XY-C1 push workflow did not trigger on `11576ae` because its path filter omitted the newly changed AliyunDrive and NVRAM CLI locations. The older XY-C1 run #45043730811 on source SHA `3556d297e887ad81b5457e16a33b32eafadafa9a` passed, but is not evidence for `11576ae`. The next `net_wan.c` source change is in the XY-C1 path filter and should trigger a current-source validation.
- No local C/shell test, target build, TRX/artifact or hardware validation was performed for `11576ae`.
- L3 remains NOT VERIFIED until XY-C1 Build, TRX validation and artifact upload all PASS on one identical production source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route # (including the DNS-read script contract) and HTTPD # jobs; verify all updated source-contract steps pass.
2. Inspect XY-C1 host validation and actual Build, TRX and artifact-upload steps on the exact same source SHA. Do not reuse ancestor artifacts.
3. Continue an independent stability/security defect while Actions are pending; keep the frozen backup read-only and work only on main.
