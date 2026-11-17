# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current main: THIS COMMIT; parent `d2a41360466574898f7fc9fe224236d5a51517a7`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- P1 WAN-up DDNS fix `01bc69f3`: configured Inadyn inventory/start/config-publication/HUP failures flow into WAN-up's first-error status; DDNS-disabled and AP-mode remain successful no-ops.
- Integration repair `0c786821` fixed WAN test include paths and updated MiniUPnD/HTTPD PPP rollback contracts to the checked status-returning implementation. Its WAN route, MiniUPnD, HTTPD, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS tests passed. The AI handoff contract passed on descendant `06ed4321`.
- XY-C1 for source baseline `0c786821` completed exact-head Build XY-C1, TRX output verification and artifact upload successfully in run `44026236613`; do not treat this artifact as verification of later source changes.
- P1 ZeroTier slice `d2a41360`: WAN-up now uses the existing enabled/disabled-aware `start_zerotier()` wrapper rather than dropping the result of `doSystem()`. It merges/logs the launch failure. The shell regression injects a launcher failure; ZeroTier lifecycle `44027996550` and Process lifecycle `44027996499` passed. Related DNSMasq `44027996482`, VPN server `44027996498`, WAN DHCP `44027996570`, WAN ARP `44027996951`, MiniUPnD `44027996455`, HTTPD `44027996404`, infosvr `44027996430`, AI handoff `44027996394`, teardown `44027996402`, firewall `44027996280`, detect_link `44027996330`, detect_internet `44027996257`, AdGuardHome `44027996230`, AliDDNS `44027996464`, AliyunDrive `44027996377`, and process inventory `44027996381` passed on exact SHA `d2a41360`.
- The WAN-route run `44027994137` on `d2a41360` failed before creating any job (GitHub reported `total_count=0`): the newly appended pull_request path patterns landed after the workflow's YAML body instead of inside `on.pull_request.paths`. THIS COMMIT moves those four paths into the correct filter block; it does not change source behavior.
- XY-C1 `44027996318` was still in progress with Build XY-C1 running at last poll. NVRAM `44027996968` was in progress. Exact-head Actions for THIS COMMIT are pending at creation.
- Local checkout/toolchain is unavailable; local compile/test NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Verify every exact-head `push` workflow for THIS COMMIT. Confirm WAN route now creates a job and passes the DDNS + ZeroTier caller checkers. Confirm ZeroTier lifecycle and Process checks remain green. For XY-C1, require same-SHA Build, TRX verification and artifact upload.
2. With CI pending, continue the next P1 WAN-up caller: `pppoemwan_enable` invokes `/usr/bin/detect.sh` via `doSystem()` and discards status. Do not directly aggregate its present exit code: `trunk/user/pppoe/detect.sh` lacks `set -e`, ignores many `iptables`, `ip` and route failures, and ends with lock cleanup. First make script status/cleanup truthful with mock-command failure-path tests; then propagate status.
3. Preserve the frozen backup and use one complete Git tree, one commit and non-forced expected-SHA main update per logical slice.

## Verification boundary

- Exact parent main before this repair: `d2a41360466574898f7fc9fe224236d5a51517a7`; live main matched.
- Exact-head `d2a41360`: ZeroTier lifecycle `44027996550` PASS (shell suite plus source integration checker); Process lifecycle `44027996499` PASS; Process inventory `44027996381` PASS; WAN DHCP callback `44027996570` PASS; WAN ARP `44027996951` PASS; VPN server `44027996498` PASS; DNSMasq `44027996482` PASS; MiniUPnD `44027996455` PASS; HTTPD `44027996404` PASS; infosvr `44027996430` PASS; AI handoff `44027996394` PASS; teardown `44027996402` PASS; firewall startup `44027996280` PASS; detect_link `44027996330` PASS; detect_internet `44027996257` PASS; AdGuardHome `44027996230` PASS; AliDDNS `44027996464` PASS; AliyunDrive `44027996377` PASS.
- Exact-head WAN route `44027994137` had zero GitHub jobs and failed because malformed workflow YAML resulted from the source path insertions; fixed in THIS COMMIT. It did not execute a source test.
- XY-C1 run `44027996318` was still building with Build XY-C1 in progress at last poll; TRX and artifact steps not yet verified. NVRAM run `44027996968` still in progress at last poll.
- No local compile/test was run. No L3 pass is claimed for `d2a41360` or THIS COMMIT until same-SHA XY-C1 Build/TRX/artifact steps pass. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch the live main SHA and filter Actions on that exact `head_sha`/push event. Verify WAN route, ZeroTier lifecycle, Process lifecycle, NVRAM and XY-C1. Confirm XY-C1 reaches Build, TRX output verification and artifact upload. If those runs are pending, begin a proper failure-propagating refactor of PPPoE-MWAN `detect.sh` with shell mocks and regression tests; do not mask the errors by blindly capturing the current script status. Keep the backup unchanged.