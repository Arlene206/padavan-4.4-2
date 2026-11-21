# Cross-session execution mandate

## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 dated entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Parent main before this checker repair: 917415b910965a663b0d0869dec75639e93b277d; production source includes the RT3352 MII TxPower normalization fix from its parent chain. Frozen archival branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable. Legacy PR #1 is closed/unmerged.
- RT3352 MII TxPower fix is live in 917415b: shared 0..100/default-100 normalization is used by both ralink.c AP config generation and net_wifi.c runtime iwpriv command; GNU89 boundary test and source/path CI contract included. Wireless radio #44906405015 PASS, iNIC signal #44906405082 PASS, watchdog signal #44906405010 PASS, process inventory #44906404949 PASS, AI docs #44906404948 PASS and HTTPD #44906404924 PASS, all exact SHA 917415b.
- Process lifecycle regression #44906404890 failed on exact 917415b at its final shared-library/RC integration checker line 3348. All preceding service and shell regressions passed, including fput_string, MiniDLNA, OpenSSH PID-file, Transmission, WireGuard, ZeroTier, RWFS, RC notification, WAN/LAN, VPN, UPnP, IPv6, and daemon startup checks. The failure is a stale test mapping: loop expected settling waits in public start_sshd(void), but SSH checked-start now correctly delegates to start_sshd_with_mode(int sshd_mode); public wrapper only performs checked mode read/delegation. This commit updates the source contract to validate wait probes in the helper and separately assert checked-read/delegation in the wrapper.
- XY-C1 #44906404894 is still in progress on exact production-source SHA 917415b, currently at Build XY-C1; TRX and artifact upload are pending. Do not claim L3 until all three steps pass on the same SHA.
- NVRAM transaction audit #44906405138 remains in progress at raw ioctl/kernel caller audit. No conclusion yet.
- Router-mode transactional fix, checked NFS start/restart, HTTPD fail-closed wrapper, USB storage NFS policy, MiniUPnP internal-port nonzero parser and SSH lifecycle fix are already implemented; avoid repeating completed work. Physical router L4 remains NOT VERIFIED.

## Active task
1. Inspect exact-head Process lifecycle and AI docs reruns from this checker repair. If there is another failure, read the first causal assertion/log and correct the exact contract without weakening regression coverage.
2. Inspect XY-C1 #44906404894 Build XY-C1, Verify TRX output and artifact upload separately on 917415b; claim L3 only if the entire same-SHA chain passes.
3. Check NVRAM audit #44906405138 result and continue with any genuinely open P1 security/reliability defect from the broader userland.

## Verification boundary
- RT3352 TxPower GNU89/-Werror wireless regression and source contract PASS; iNIC/watchdog and other named Actions PASS on exact SHA 917415b.
- Process lifecycle failed only in late source integration assertion: it expected startup waits in the SSH public wrapper. Prior test steps in that job passed; updated contract awaits exact-SHA rerun.
- XY-C1 current product-source build is pending at Build; TRX/artifact pending. No L3 result for 917415b yet. NVRAM audit pending at caller audit. No physical hardware L4.
- No local RC cross-compile or hardware test is claimed.

## Exact first action for the next GPT
Fetch live main and inspect exact-head Process lifecycle and AI docs reruns from this checker correction. Poll XY-C1 Build/TRX/artifact and NVRAM caller audit independently; continue an unrelated confirmed production-source fix while jobs are pending.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.