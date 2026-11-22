# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `690dc90846b959d05a5ee51e8f5d9cfa5c960408`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

`notify_on_internet_state_changed()` now reads `di_lost_action` with `get_nvram_int_checked_status()` instead of an error-collapsing `nvram_safe_get_int()`. Its policy preserves the old 0..3/default-zero bounds and logs NVRAM failures instead of silently suppressing configured automatic recovery. The new GNU89 regression and WAN DNS source contract passed on the 690dc host run; WAN route still stopped later at a stale watchdog checker log literal, corrected in this commit.

## Verification boundary

- Exact parent `88f33bc3d23fd4b1c4cd909abfcd55d6991f7ab2`: WAN route #45046759582 PASS and AI handoff #45046759590 PASS.
- Exact source SHA `690dc90846b959d05a5ee51e8f5d9cfa5c960408`: WAN DHCP callback #45047262363 PASS (includes checked 6rd tuple tests); ZeroTier #45047262287 PASS; Samba #45047262278 PASS; link/detect Internet signals #45047262383/#45047262340 PASS; Wireless #45047262438 PASS; IPv6 service type #45047262463 PASS; AI handoff #45047262377 PASS. Other workflows were pending at last observation.
- WAN route #45047262283 reached `check_wan_deferred_watchdog_status.py` after WAN DNS contract passed; failed because it expected the phrase `cannot publish modem priority before automatic WAN reconnect`, while the actual checked source log has `cannot publish modem priority before automatic reconnect`. This checker-only correction aligns it with the source; re-run route on the new SHA.
- XY-C1 #45047262320 on `690dc9` was running with Build XY-C1 pending/starting; TRX and artifact upload were not yet complete. L3 remains NOT VERIFIED until all same-SHA steps pass.
- No local tests/build or hardware validation was performed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route # (including the DNS-read script contract) and HTTPD # jobs; verify all updated source-contract steps pass.
2. Inspect XY-C1 host validation and actual Build, TRX and artifact-upload steps on the exact same source SHA. Do not reuse ancestor artifacts.
3. Continue an independent stability/security defect while Actions are pending; keep the frozen backup read-only and work only on main.
