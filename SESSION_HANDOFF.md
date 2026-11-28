# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at this source-fix batch's parent: `4b0fe352c64fc2aebdbca526ce573a62d0c739b8`. AI handoff #46120573872 PASS and USB modem identity #46120573957 PASS. Process lifecycle #46120573849 was in progress at step 35 with the USB printer executable-bound filter test at step 96 still pending; USB storage #46120573847 was in progress at its BusyBox/ash stage. XY-C1 #46120573889 was still before Build XY-C1 (step 38); it does not provide an L3 result for this parent. The prior USB stop checker, shared include, storage assertion and 120-record log rotation failures have been corrected in 4b0fe.
- A prior real XY-C1 MIPS build failure remains uncorrected at this parent: run #46119302438 reported `postlogin.c:1271: 'chown_succeeded' undeclared`. The source still declared that local inside `handle_retr()` while `handle_upload_common()` used it. This source fix moves the declaration to the upload function and adds a scope contract; fresh exact-head VSFTPD and XY-C1 runs are required.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, recorded HEAD `72f4f157a084f8184b9597ca5bf334b8080e5807`. Its warning file forbids mutation/deletion. PR #1 remains CLOSED/unmerged; its substantive unmerged work was already compared, with no missing production/test/build change identified.
- Startup recovery was completed once against the bootstrap, development protocol, handoff, roadmap and log indexes. Do not repeat completed audits.
- Root `dev_log.md` remains below the documented 384 KiB target and 120-record rotation threshold. Keep 20–119 records. The roadmap is close to its 128 KiB hard limit; avoid routine expansion.
- No local source checkout/build environment is available. Host regressions and firmware outcomes below are GitHub Actions results, not local claims.

## Active task

Fix the concrete MIPS firmware compilation defect found by XY-C1. `trunk/user/vsftpd/src/postlogin.c` declared `chown_succeeded` inside `handle_retr()`, but assigns and reads it in `handle_upload_common()`. The previous host source contract did not assert the variable scope; XY-C1 #46119302438 exposed the issue during the real MIPS build. This commit moves the declaration to `handle_upload_common()` and adds explicit VSFTPD source-contract assertions requiring exactly one upload-local declaration and none in RETR.

USB printer executable-bound stop hardening and its CI-only repairs are already on main through `4b0fe352c64fc2aebdbca526ce573a62d0c739b8`: AI docs #46120573872 PASS, USB modem #46120573957 PASS; Process lifecycle #46120573849 and USB storage #46120573847 were still pending when this fix was prepared. The 4b0fe XY-C1 #46120573889 had not started its Build step and is superseded by this source change. No local compilation is possible in this session. Do not claim L3 until the exact new SHA completes Build, TRX and artifact upload; physical L4 remains NOT VERIFIED.


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

1. Fetch live `main` and enumerate only push Actions whose `head_sha` exactly matches it. Inspect the VSFTPD workflow and XY-C1 firmware validation triggered by this source change; require the scope contract, MIPS compile, full image build, TRX check and artifact upload to pass.
2. Check whether Process lifecycle #46120573849 (step 96) and USB storage #46120573847 (BusyBox/ash stage) completed on ancestor `4b0fe352c64fc2aebdbca526ce573a62d0c739b8`. Those results validate the USB-printer fix/CI repair only, not this postlogin source change.
3. If the new exact-head MIPS build finds further compile failures, inspect the first causal compiler diagnostic and repair it with a bounded regression. Never infer L3 from host-only or ancestor runs. Physical L4 remains unverified.
4. After this compile slice is green, resume a separate repository-wide stability/security issue; do not repeat the USB printer source audit or earlier watchdog, HTTPD, IPTV, IAPP and anonymous-upload review.

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
