# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current main: THIS COMMIT; parent `42097d6c2ac28c37960bbb5b4940f2db22bf30f2`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- P1 WAN-up DDNS fix `01bc69f3`: configured Inadyn inventory/start/config-publication/HUP failures flow into WAN-up's first-error status; DDNS-disabled and AP-mode remain successful no-ops.
- Integration repair `0c786821` fixed WAN test include paths and updated MiniUPnD/HTTPD PPP rollback contracts to the checked status-returning implementation. Its WAN route, MiniUPnD, HTTPD, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS tests passed. The AI handoff contract passed on descendant `06ed4321`.
- XY-C1 for source baseline `0c786821` completed exact-head Build XY-C1, TRX output verification and artifact upload successfully in run `44026236613`; do not treat this artifact as verification of later source changes.
- P1 ZeroTier slice `d2a41360`: WAN-up now uses the existing enabled/disabled-aware `start_zerotier()` wrapper rather than dropping the result of `doSystem()`. It merges/logs the launch failure. The shell regression injects a launcher failure; ZeroTier lifecycle `44027996550` and Process lifecycle `44027996499` passed. Related DNSMasq `44027996482`, VPN server `44027996498`, WAN DHCP `44027996570`, WAN ARP `44027996951`, MiniUPnD `44027996455`, HTTPD `44027996404`, infosvr `44027996430`, AI handoff `44027996394`, teardown `44027996402`, firewall `44027996280`, detect_link `44027996330`, detect_internet `44027996257`, AdGuardHome `44027996230`, AliDDNS `44027996464`, AliyunDrive `44027996377`, and process inventory `44027996381` passed on exact SHA `d2a41360`.
- The WAN-route run `44027994137` on `d2a41360` failed before creating any job (GitHub reported `total_count=0`): the newly appended pull_request path patterns landed after the workflow's YAML body instead of inside `on.pull_request.paths`. Commit `42097d6c` moved those four paths into the correct `on.pull_request.paths` filter block; it did not change source behavior.
- Exact-head `42097d6c` push Actions all completed SUCCESS: WAN route `44028469558`, AI handoff docs `44028469593`, Process lifecycle `44028469544`, NVRAM `44028469453`, AdGuardHome `44028469448`, AliDDNS `44028469580`, AliyunDrive `44028469536`, infosvr `44028469559`. The WAN workflow now creates and runs its job; no zero-job YAML failure remains.
- Exact-head PPPoE-MWAN source slice in THIS COMMIT: `detect.sh` now tracks first command failure, safely tokenizes `iptables-save` without `eval`, validates NVRAM/IPv4 values, uses bounded counts and an owned atomic lock with checked cleanup. Host mock regression covers successful multi-WAN route replacement, deletion/insertion/save failures, route failure with cleanup continuation, absent PPP interface, malformed/oversized values, lock cleanup and inert shell metacharacters. Local Bash syntax checks and regression PASS; this source SHA still requires exact-head CI and a fresh XY-C1 build.
- XY-C1 run `44027996318` for source SHA `d2a41360` completed SUCCESS through Build XY-C1, TRX output verification and artifact upload (artifact `14304605616`, 12.64 MB; SHA-256 `05e374b72ce3f88eb6a019f50c2b1a85c924e28faaae772103bdd102a1485e8f`). This does not verify the newer PPPoE-MWAN source in THIS COMMIT.
- No full local repository checkout or MIPS toolchain is available; host Bash syntax and mock-command tests for the PPPoE-MWAN script were run locally and passed. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Verify exact-head Actions for THIS COMMIT, especially WAN route script/mock regression and XY-C1 Build, TRX check and artifact upload; inspect job steps, not only run conclusions.
2. After same-SHA CI is launched, continue the separate `wan_up_internal()` caller slice: capture `eval("/usr/bin/detect.sh")`, aggregate it via `route_status_accumulate()` and log failures, with a caller source-contract regression. Do not re-audit the already-tested script semantics.
3. Preserve the frozen backup and use one complete Git tree, one commit and non-forced expected-SHA main update per logical slice.

## Verification boundary

- Exact parent main before this PPPoE-MWAN source slice: `42097d6c2ac28c37960bbb5b4940f2db22bf30f2`; live main matched.
- Exact-head `d2a41360`: ZeroTier lifecycle `44027996550` PASS (shell suite plus source integration checker); Process lifecycle `44027996499` PASS; Process inventory `44027996381` PASS; WAN DHCP callback `44027996570` PASS; WAN ARP `44027996951` PASS; VPN server `44027996498` PASS; DNSMasq `44027996482` PASS; MiniUPnD `44027996455` PASS; HTTPD `44027996404` PASS; infosvr `44027996430` PASS; AI handoff `44027996394` PASS; teardown `44027996402` PASS; firewall startup `44027996280` PASS; detect_link `44027996330` PASS; detect_internet `44027996257` PASS; AdGuardHome `44027996230` PASS; AliDDNS `44027996464` PASS; AliyunDrive `44027996377` PASS.
- Historical WAN route run `44027994137` on `d2a41360` had zero jobs due to malformed path-filter placement; the repair commit `42097d6` passed the exact-head WAN workflow `44028469558`.
- `42097d6` push Actions: eight selected workflow runs all SUCCESS, including WAN route, Process lifecycle, AI handoff docs and NVRAM. Its source did not change from `d2a41360`; the `d2a41360` XY-C1 run completed Build/TRX/artifact successfully.
- Current PPPoE-MWAN host Bash syntax and mock regression PASS locally. This commit's MIPS XY-C1 and all same-SHA Actions are not yet verified. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch the live main SHA and filter Actions on that exact `head_sha`/push event. Verify the PPPoE-MWAN WAN workflow step, DDNS/ZeroTier checks and XY-C1 Build → TRX verification → artifact upload. Then implement the separate WAN-up caller return-value aggregation for `/usr/bin/detect.sh`; the script's internal status/cleanup regression is already covered by THIS COMMIT. Keep the backup unchanged.