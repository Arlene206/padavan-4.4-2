# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main before this follow-up: `7627a0840d64b18be01b93c6cecbd8597f7f64ec`. AI handoff #46121754074 PASS and process inventory #46121754112 PASS. Process lifecycle #46121754078 FAIL in `check_optional_app_start_status.py:828` because the aggregate checker still required the retired `IPERF3_PROCESS_HELPER` interface and its old pidof/killall test model. Iperf3 regression #46121754059 failed because the proc stat fixture contained 19 rather than 20 tokens for fields 3–22. This follow-up corrects both regressions; exact-head validation required.
- Product source `ef122034b4524ec1ce6768dd169ebee62a7da2ee` passed XY-C1 #46120731426: MIPS Build step 38 PASS, TRX step 39 PASS and artifact upload step 40 PASS (SHA-256 `c9e4fff453dd08361893447a1aa985e5699ad4ee0c13919b0a1e0f168373c9f5`). VSFTPD #46120731445, process inventory #46120731490, AI docs #46120731486 and NVRAM #46120731399 also PASS on `ef122`; hardware L4 remains NOT VERIFIED.
- A prior real XY-C1 MIPS build failure remains uncorrected at this parent: run #46119302438 reported `postlogin.c:1271: 'chown_succeeded' undeclared`. The source still declared that local inside `handle_retr()` while `handle_upload_common()` used it. This source fix moves the declaration to the upload function and adds a scope contract; fresh exact-head VSFTPD and XY-C1 runs are required.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, recorded HEAD `72f4f157a084f8184b9597ca5bf334b8080e5807`. Its warning file forbids mutation/deletion. PR #1 remains CLOSED/unmerged; its substantive unmerged work was already compared, with no missing production/test/build change identified.
- Startup recovery was completed once against the bootstrap, development protocol, handoff, roadmap and log indexes. Do not repeat completed audits.
- Root `dev_log.md` remains below the documented 384 KiB target and 120-record rotation threshold. Keep 20–119 records. The roadmap is close to its 128 KiB hard limit; avoid routine expansion.
- No local source checkout/build environment is available. Host regressions and firmware outcomes below are GitHub Actions results, not local claims.

## Active task

1. **XY-C1 L3 verified for product SHA `ef122034b4524ec1ce6768dd169ebee62a7da2ee`:** Build, TRX and artifact upload PASS in #46120731426; artifact SHA-256 `c9e4fff453dd08361893447a1aa985e5699ad4ee0c13919b0a1e0f168373c9f5`. VSFTPD, Process inventory, AI docs and NVRAM audits also PASS on `ef122`. L4 physical hardware remains NOT VERIFIED.
2. **USB printer executable-bound lifecycle closed at CI level:** Process lifecycle #46120862607 PASS, including USB readiness, failed-exec child reaping and executable-bound stop filter steps 94–96 on `ce8e38ec928c8798b4785d58e6ddffee01cccbe6`.
3. **Current iperf3 slice:** source #0b7db introduced exact binary/argv/start_time scanning and PID-scoped BusyBox stop; #7627 fixed the NUL fixture and changed to BusyBox. Latest #46121754059 then found proc stat fixture supplied 19 rather than required 20 tokens from state through start_time. Process lifecycle #46121754078 also failed because aggregate `check_optional_app_start_status.py` still asserted the old shared helper and pidof/killall tests. Current follow-up fixes fixture count and replaces those obsolete aggregate assertions with the dedicated regression. New exact-head tests pending.


## Watchdog scheduled-action NVRAM snapshot baseline

- `04f55a0098f9328c7bd563fccbbafd21013e03b3` added checked single-transaction reads for reboot/SS selectors, `ntp_ready` and schedules. Exact watchdog, Process, NVRAM and AI docs checks passed; XY-C1 #46115631593 passed Build, TRX and artifact upload (artifact ID `15251593325`, 12,838,666 bytes). L3 applies to that exact SHA only.
- `f5bcffd73428bab5b4818e44465f3d25f8b7563a` optimized inactive schedule reads; exact watchdog #46115995003 FAILED because selector fields were evaluated before assignment.
- `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` fixed selector assignment order. Watchdog #46116069843, Process #46116070166, NVRAM #46116069885 and AI docs #46116069828 PASS. XY-C1 #46116069855 remained in progress at Build; later 58700 changes production source again, so that result cannot validate the IPTV code.
- `6599ea954a69c70b4f811f66519e01b00671c634` added test-only pre-NTP schedule-read coverage. Watchdog #46116463184, Process #46116463183, NVRAM #46116463221 and AI docs #46116463180 PASS.

## Exact validation state

| Source SHA | Workflow / run | Result |
|---|---|---|
| `4b0fe352c64fc2aebdbca526ce573a62d0c739b8` | AI handoff #46120573872; USB modem identity #46120573957 | PASS |
| `4b0fe352c64fc2aebdbca526ce573a62d0c739b8` | Process lifecycle #46120573849; USB storage #46120573847 | IN PROGRESS at last inspection; see Active task |
| `4b0fe352c64fc2aebdbca526ce573a62d0c739b8` | XY-C1 #46120573889 | IN PROGRESS before Build; superseded by the current VSFTPD source change |
| `9edd01a13d7c861a1c2e246220f2096134679ca1` | XY-C1 #46119302438 | FAIL — MIPS compile: `postlogin.c:1271: 'chown_succeeded' undeclared`; this commit fixes the local scope |
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | XY-C1 #46115631593 | PASS — Build, TRX, artifact upload |
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | Watchdog #46115631687; Process #46115631548/#46115631613; NVRAM #46115631554; AI docs #46115631633 | PASS |
| `f5bcffd73428bab5b4818e44465f3d25f8b7563a` | Watchdog #46115995003 | FAIL — selector ordering; corrected by 9e154 |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | Watchdog #46116069843; Process #46116070166; NVRAM #46116069885; AI docs #46116069828 | PASS |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | XY-C1 #46116069855 | IN PROGRESS at Build; TRX/artifact pending at last poll |
| `6599ea954a69c70b4f811f66519e01b00671c634` | Watchdog #46116463184; Process #46116463183; NVRAM #46116463221; AI docs #46116463180 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | Process inventory #46116630819; HTTPD #46116630813; DNSMasq #46116630771; WAN DHCP #46116630786; WAN route #46116630800; firewall #46116630795; AI docs #46116630784 | PASS |
| `173fae1685922a5d94d90cb32f370e212eea137e` | Process fail-closed #46116838163 | FAIL — test include fixed, then stale xupnpd port-gate assertion; no product source failure |
| `6a1b28a152bed4341c912ae797945ee83f852d1c` | Process fail-closed #46117034795 | PASS — IPTV port GNU89/Werror/ASan/UBSan regression and xupnpd/udpxy LAN-startup source contracts |
| `6a1b28a152bed4341c912ae797945ee83f852d1c` | Process fail-closed #46117034795; AI docs #46117034802 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | USB storage #46116630890; NVRAM #46116630796 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | XY-C1 #46116630792 | PASS — Build, TRX and artifact upload; artifact ID `15252233106` (12,838,305 bytes) |
| `a747994f8892363f94938655b6db68195d44c9dd` | AI docs #46117399122 | PASS |
| `88695c8189774f5108ff82d0b7c621000c2d76d6` | IAPP host and optional-branch regression #46118049024 | PASS — lifecycle, dependency, FD, datagram, event-log and deadline checks |
| `88695c8189774f5108ff82d0b7c621000c2d76d6` | XY-C1 MIPS optional IAPP compile #46118049024 | PASS — pinned toolchain, both optional translation units; two macro redefinition warnings fixed here |
| `88695c8189774f5108ff82d0b7c621000c2d76d6` | AI docs #46118048938 | PASS |

- The last full XY-C1 L3 result remains exact product SHA `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` (#46116630792: Build, TRX and artifact upload). Since then this session has changed IAPP headers, so do not carry that exact L3 claim onto current main until a fresh full image validation passes. Physical-router L4 remains NOT VERIFIED.
- L4 physical-router flash/boot, wireless/network and WAN behavior are NOT VERIFIED.
- Do not conflate host ASan/UBSan with MIPS build or hardware validation.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head `iperf3 process lifecycle` and `Process inventory and lifecycle fail-closed regression` Actions triggered by this follow-up. Both must run the standalone executable identity regression; inspect logs on any failure.
2. Confirm the source contract no longer tests the old `IPERF3_PROCESS_HELPER`/pidof/killall implementation and still enforces exact executable, server argv, start_time, UNKNOWN fail-closed, PID-scoped BusyBox TERM/KILL and package installation path.
3. XY-C1 L3 is verified only on `ef122034b4524ec1ce6768dd169ebee62a7da2ee` (#46120731426 Build/TRX/artifact PASS). The new source SHA from this iperf work must get its own Build, TRX and artifact PASS before updating L3; hardware L4 remains NOT VERIFIED.
4. After iperf CI passes, continue a separate userland stability/security issue without repeating USB printer or VSFTPD scope reviews.

## Verification boundary

- Source content and Git state are from GitHub; tests/firmware outcomes are the exact Actions runs above.
- No local checkout/build/test is claimed.
- XY-C1 Build/TRX/artifact is L3, not physical-router L4.
- Hardware flashing, boot, radio, WAN and live network validation are NOT VERIFIED.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and compare it with this handoff before ending. The checkpoint is mandatory; a document edit is conditional. Confirm meaningful product-source baselines, active task, exact-head Actions, validation boundary, blockers and exact first action. If all facts remain accurate, leave this file untouched and do not create a no-op commit.

# Cross-session execution mandate

- Only `main` is authorized. Keep the frozen backup immutable; create no feature/checkpoint branches or PRs.
- For each multi-file slice, create all blobs and one complete Git tree, then publish one atomic main commit with the latest main as sole parent; advance with `force:false` and `expected_sha` equal to that parent. Never use per-file commits, force-push, amend or rewrite published history.
- Re-fetch main immediately before every write. If compare-and-swap rejects, inspect the new head, rebuild/retest and retry without force.
- Inspect push-triggered Actions by exact `head_sha`. PASS permits the next slice; FAIL must be diagnosed/fixed; PENDING does not block independent work.
- Keep root `dev_log.md` append-only and inside the 20–119 record / 384 KiB limits; batch-rotate only when required. Do not create routine handoff/roadmap archives.
- Before context exhaustion, finish a safe bounded slice, record exact source/test/CI state and leave the next GPT a precise first action. Do not ask the user what to do next.
