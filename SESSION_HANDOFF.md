## 2026-11-24 autonomous handoff snapshot

This addendum supersedes older task pointers below where they conflict with the live Git state.

- Main at snapshot: `904d7b489b96a99c72cedb022667c67d4e3b2eb3`.
- Backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not move it).
- Recent main history:
- `904d7b489b96a99c72cedb022667c67d4e3b2eb3` — test(zerotier): ignore NAT fixture seeding in cleanup assertion
- `74946cc36589c72b0582fa5dc3132b246e805da7` — test(zerotier): encode empty checkpoint lines correctly
- `f2724cd666c7265759b23a72b7dd634041fa7975` — fix(zerotier): honor empty NAT cleanup checkpoints
- `33d0f4c2edaa3f74b004abf2939bad9ca530763d` — test(vsftpd): assert hard-link rename leaves no alias
- `453ef6f30e0ad21b474602eab8d37a6b4d32ecbf` — fix(vsftpd): block cross-share hard-link renames
- `fe0a700656aa90b3f2fcf1c0929c3bd129d80521` — test(zerotier): write checkpoint fixture as two lines
- `6a5b74fdb75537599055151b01eea7c20ef6141e` — fix(zerotier): preserve published NAT cleanup checkpoint
- `0b68b5f86479c766e0d5204cb1ee6fd76f78a2ee` — test(vsftpd): remove unused hard-link fixture path
- `7d9efe339fb102c366bf15fda490f4aff49f2233` — fix(vsftpd): retain hard-linked uploads during rollback
- `a247cd34ac841b12f57f692209eebd01ce9d5611` — test(udhcpc): match multiline static route guard
- Exact-head Actions currently known:
- Process lifecycle inventory - 904d7b489b96a99c72cedb022667c67d4e3b2eb3 (#45530983779, SHA 904d7b489b96): in_progress.
- WAN route operation status regression (#45530983759, SHA 904d7b489b96): in_progress.
- XY-C1 Firmware Validation (#45530983731, SHA 904d7b489b96): in_progress.
- ZeroTier lifecycle stability regression (#45530983712, SHA 904d7b489b96): in_progress.
- Latest relevant workflow observations (may refer to older SHAs and are not proof for current main):
- vsftpd configuration publication stability regression (#45530467033, SHA 33d0f4c2edaa): completed/success.
- ZeroTier lifecycle stability regression (#45530983712, SHA 904d7b489b96): in_progress.
- XY-C1 Firmware Validation (#45530983731, SHA 904d7b489b96): in_progress.
- WAN DHCP callback state stability regression (#45528021031, SHA f72030990d75): completed/success.
- DNSMasq lifecycle stability regression (#45528020810, SHA f72030990d75): completed/success.
- Wireless radio state regression (#45528020753, SHA f72030990d75): completed/success.
- AI handoff documentation contract (#45530467034, SHA 33d0f4c2edaa): completed/success.
- Process lifecycle inventory - 904d7b489b96a99c72cedb022667c67d4e3b2eb3 (#45530983779, SHA 904d7b489b96): in_progress.

Completed implementation slices in the current development line include: ZeroTier stop now fails closed on malformed firewall recovery checkpoints; WAN DHCP Router and legacy/classless route lists are validated before lease/interface/route teardown; DNSMasq static ARP parser requires Ethernet hardware type; Ralink radio config checker covers all observed atomic-discard paths; VSFTPD denies multiply-linked non-directory entries on open/stat/upload, refuses rollback deletion of a multiply-linked upload, and blocks cross-share hard-link renames. Their regression/source-contract changes are committed in the recent history above. Treat each workflow's exact SHA and conclusion as authoritative; do not infer a product firmware build from a host-only regression workflow.

First tasks for the next session:
1. Inspect any exact-head FAIL above, beginning with the current ZeroTier firewall lifecycle run if it is failed; read the failing job log, distinguish product defect from stale test/source contract, fix and add regression tests.
2. Confirm the current-head VSFTPD run includes the hard-link rename and rollback regressions. Check the corresponding job, not only a workflow with a similar name on an older SHA.
3. Check current-head WAN DHCP callback and route-operation Actions; resolve compiler/test/checker failures before moving to another unrelated module.
4. Check XY-C1 Build, TRX verification and artifact upload on a production-source SHA. L3 remains NOT VERIFIED until those exact steps pass; L4 remains hardware NOT VERIFIED. No local compile is claimed by this handoff.
5. Continue with the next independent userland stability/security defect after the above, preserving small atomic commits and expected-SHA updates. Do not wait on unrelated PENDING workflows.

No force-push. The backup branch remains unchanged.

## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Last verified PPP source baseline before embedded-NUL hardening: 0af78992a772036818dc4fd71b48ff74b066abeb; refresh live main and tree before every future write.
- Frozen backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; verified unchanged and read-only.
- main is the only development ref. PR #1 is closed/unmerged and is not a development target.
- Bootstrap, protocol, handoff, roadmap, dev-log window/index and live refs were recovered once. The recorded RC whole-line parser gate has been superseded by subsequent main commits; do not repeat it.

## Active task

### AI handoff documentation size — restore the roadmap hard limit

The DHCPv6 timer source helper and regression are committed in `c009462148d00369007edaf3e5b87d31899b07c2`; the follow-up checker marker is fixed in `5b54971422023d9e1dd0d3e7f2f21f2d73816ffb`. The exact-main DHCPv6 GNU89 regression #45524153756 passes. AI handoff validation #45524153765 failed only because `REFACTOR_ROADMAP.md` measured 131,167 bytes, 95 bytes above the 131,072-byte hard limit. Condense redundant historical status text without dropping active findings, then confirm the exact-head docs workflow.

## Current exact-head Actions on 5b54971422023d9e1dd0d3e7f2f21f2d73816ffb

- DHCPv6 Reply script retry regression #45524153756: PASS; the new expired-timer boundary coverage and current output marker both pass.
- AI handoff documentation #45524153765: FAIL solely on the roadmap 131,167-byte hard limit (limit 131,072); this slice trims the redundant top summary and must be rechecked on its own SHA.
- Product-source baseline `c009462148d00369007edaf3e5b87d31899b07c2`: Process inventory #45523977446, HTTPD security #45523977367, USB storage #45523977447 and NVRAM boundary #45523977503: PASS.
- XY-C1 #45523977414 is still IN_PROGRESS for that unchanged product-source baseline; Build/TRX/artifact are not confirmed. L3 NOT VERIFIED; L4 hardware NOT VERIFIED.
- No local compile is claimed.

## Completed source slices

### RC optional user-config long-line handling

The generic load_user_config() reader previously discarded a full 255-byte fgets() chunk when it had no newline, then resumed at the remainder of the same physical line. A suffix such as injected=payload could therefore be interpreted as a fresh configuration directive in hosts or wireless driver overlays. The reader now drains and rejects the entire overlong physical record; an exactly 255-byte record is accepted, and a final unterminated short record is normalized with a newline. Embedded-NUL records are also rejected. The API returns read/write/flush/close errors (while a missing optional file remains success), and the LAN hosts, WAN hosts and Ralink config publishers abort their staged publication on copy failure. Host regression covers overlong suffix injection, binary suffix injection, exact-length records, comments, forbidden records and EOF without a newline. Exact-head CI pending.
### LAN authentication password argument exposure

This main slice stages the legacy maximum 20-byte password in a random mode-0600 file and launches lanauth with only its pathname through new `-f` input. The daemon checks regular-file type, owner, exact permissions, single-link count and inode identity, uses `O_NOFOLLOW`, then consumes the file before daemonizing. The legacy `-p` CLI remains for manual compatibility, but managed RC starts no longer pass the secret in argv. A GNU89 ASan/UBSan test covers valid consumption, empty/oversized/insecure/missing files and symlink rejection. Exact Process and XY-C1 checks pending.

### LAN authentication restart checked-read gate

Commit `3e31c744db7c4fd27665ccadc7325058c3c60a83` fixes `start_auth_kabinet()` previously using `nvram_safe_get()` for `wan_auth_host` / `wan_auth_pass` and stopping the active daemon before it could distinguish a valid empty password from a backend read failure. Both settings are now copied through `nvram_get_checked()` and `rc_wan_nvram_copy_value()` before stop; read error/truncation preserves the running daemon, while a readable empty password retains legacy stop/no-start behavior. The policy clears temporary buffers on exit. GNU89 ASan/UBSan tests cover both read failures, empty password, stop/start failures and read→stop→start ordering. The Process workflow initially exposed an outdated aggregate assertion still requiring the old inline stop call; the checker is corrected in the follow-up commit. Exact Process rerun pending.

### FTP credential lookup transaction snapshot

Commit `b2afc079f262a13aa05ef0d926455b328c480dd6` moves `asus_getpwnam()` to one coherent `nvram_transaction` snapshot across account count, active usernames and the matching password. It fails closed on missing/incomplete/duplicate credentials and transaction errors. Exact-head VSFTPD #45506421995, AI docs #45506421990 and Process inventory #45506422053 PASS. NVRAM audit #45506422062 was cancelled; XY-C1 #45506422126 was still in progress at last poll, so no L3 claim.

### FTP share listing read-ACL consistency

Commit `7d589c2ea0a3f46356e8bc361d212f8a9b81776f` made LIST visibility use the same documented read-right predicate as CWD/read authorization. Write-only (2) and unknown rights are no longer treated as readable merely because they are positive. VSFTPD Stability #45505371869 PASS on this exact SHA, including the GNU89 ACL policy regression and exact-source checks. AI handoff docs #45505371734, process inventory #45505371860 and NVRAM audit #45505371707 PASS. XY-C1 #45505371723 was still running at last poll.

### SSH daemon supported-mode validation

Commit `07e7c3453ab02d25704c2d6770a1c30d06d771f3` rejects `sshd_enable` outside 0..2 with `EINVAL`, preserving the caller's output. GNU89 tests cover -1, 3, INT_MIN and INT_MAX. Exact-head Process Lifecycle #45504233875 and PID 1 notification #45504233986 both PASS, including the SSH policy regression compile/run; AI docs #45504233800, process inventory #45504234314 and NVRAM boundary audit #45504234041 PASS. No XY-C1 run was triggered for this SSH-only source SHA.

### Firewall virtual-server optional translation-port reads

Commit `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1` fixed `include_vts_filter()` and `include_vts_nat()` conflating failed `vts_lport_x` reads with absent/empty optional values, which could publish the wrong internal-port mapping. Missing/empty settings keep the legacy external-port fallback; backend and malformed results abort incomplete staging to retain the previously published ruleset. Firewall Startup Stability #45503784466 PASS on checker-fix SHA `2be0e5125bb29f01fc3a9a15fe8938a554387452`, including the VTS policy test compile/run and exact-source contract.

### OpenVPN client certificate lifetime arguments

Commit `d096f168bfe4188f5223bf76806b4ea012742580` replaces privileged `atoi(argv[3])` parsing with strict positive decimal days (1–36,500); optional/empty input keeps the 365-day default and legacy whitespace/leading-plus support. Invalid syntax, negative/zero, suffix, fractions, range and overflow fail before side effects. VPN lifecycle #45502092649 and AI docs #45502092664 PASS on d096.

### USB modem reload retry state

Commit `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` publishes modem rule/type caches only after driver readiness; failure retains the previous baseline and blocks WAN restart so a queued retry can retry. USB caller contract #45499733421 PASS on d2fa; PID 1 queue #45500981047 and AI docs #45500980887 PASS on 8ebf.

### WAN traffic persisted-baseline parsing

Commit `786c504c8247fe42d5d1d5f2dc3ad28757fafc80` strictly parses persisted RX/TX byte baselines instead of accepting numeric-prefix junk. WAN regression #45498624341 PASS.

## Verification boundary

### Exact-head status after LAN authentication checked-read commit 3e31c744db7c4fd27665ccadc7325058c3c60a83

- AI handoff documentation #45506666102: FAIL because `REFACTOR_ROADMAP.md` exceeded its 131072-byte hard limit (131503 bytes); this correction removes the redundant roadmap bullet and restores the full handoff records.
- Process lifecycle inventory #45506666210: PASS.
- EAPoL configuration confidentiality and credential escaping #45506666147: PASS.
- Process lifecycle fail-closed regression #45506666241: FAIL at the aggregate integration assertion requiring `ret = stop_auth_kabinet();`; the checked-read refactor moved stop into the policy callback. Earlier host regression steps passed; this follow-up updates the assertion to validate checked read-before-stop and ordered service start.
- NVRAM transaction boundary audit #45506666281: IN_PROGRESS at raw userland ioctl/kernel caller audit.
- XY-C1 Firmware Validation #45506666559: IN_PROGRESS; Build active, TRX and artifact pending. Do not claim L3.

### FTP account credential snapshot SHA b2afc079f262a13aa05ef0d926455b328c480dd6

- VSFTPD stability #45506421995: PASS.
- AI handoff documentation #45506421990: PASS.
- Process lifecycle inventory #45506422053: PASS.
- NVRAM transaction boundary audit #45506422062: CANCELLED (not a pass).
- XY-C1 Firmware Validation #45506422126: IN_PROGRESS at Build XY-C1; TRX/artifact not confirmed.

### XY-C1 exact-source results

- SHA `786c504c8247fe42d5d1d5f2dc3ad28757fafc80`: #45498624157 Build, TRX and artifact upload PASS.
- SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a`: #45499228265 Build, TRX and artifact upload PASS.
- SHA `d096f168bfe4188f5223bf76806b4ea012742580`: XY-C1 #45502092732 PASS; Build XY-C1, TRX verification and artifact upload all completed SUCCESS.
- SHA `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1`: XY-C1 #45503440356 PASS; Build XY-C1, TRX verification and artifact upload all completed SUCCESS.

### d096f168bfe4188f5223bf76806b4ea012742580 exact-head Actions

- VPN lifecycle #45502092649: PASS.
- AI handoff docs #45502092664: PASS.
- NVRAM boundary audit #45502092721: PASS; zero raw userland bypasses, kernel snapshot/caller contract passed.
- Process lifecycle inventory #45502093101: PASS.
- IPv6 service checked-read #45502092750: PASS.
- HTTPD authentication/security #45502092764: PASS.
- OpenVPN client credential staging #45502092820: PASS.

### SSH policy SHA 07e7c3453ab02d25704c2d6770a1c30d06d771f3

- Process Lifecycle regression #45504233875: PASS, including `Compile and run SSH firewall fail-closed policy regression`.
- PID 1 notification queue #45504233986: PASS, including `Compile and run GNU89 SSH daemon restart policy regression`.
- AI handoff docs #45504233800: PASS.
- Process lifecycle inventory #45504234314: PASS.
- NVRAM transaction boundary audit #45504234041: PASS; raw userland NVRAM caller audit completed with no bypass finding.
- No XY-C1 build was triggered for this SSH-only commit. The closest firmware validations are d096 #45502092732 and 6d #45503440356, both full Build/TRX/artifact PASS, but those results are tied to their exact source SHAs.

### VSFTPD share-listing ACL SHA 7d589c2ea0a3f46356e8bc361d212f8a9b81776f

- VSFTPD Stability #45505371869: PASS, including GNU89 ACL policy regression and exact-source read-right callsite checks.
- AI handoff docs #45505371734: PASS.
- Process lifecycle inventory #45505371860: PASS.
- NVRAM transaction boundary audit #45505371707: PASS.
- XY-C1 Firmware Validation #45505371723 was IN PROGRESS at last poll. Build, TRX verification and artifact upload must all pass on this same SHA before claiming L3.
- No physical-router L4 validation is claimed.

### Firewall / checker SHA 2be0e5125bb29f01fc3a9a15fe8938a554387452

- Firewall Startup Stability #45503784466: PASS. The exact-source checker and GNU89 ASan/UBSan VTS policy test passed, together with the other firewall policy checks.
- AI handoff documentation #45503784498: FAIL because the handoff omitted the validator's exact mandatory-checkpoint sentence. Commit #05356ed622d1762e5e251c6adeb5fcd7be7713b6 restores it; rerun #45504014822 PASS.
- XY-C1 #45503440356 PASS on older product-source SHA 6d5d5376e425587e6f116d09f88cbcc7ac6a05c1; this does not validate later SSH source.
- NVRAM audit #45503440350, WAN route #45503440493, IPv6 service #45503440453, AI docs #45503440345, WAN DHCP #45503440341, process inventory #45503440565, HTTPD #45503440466 and FULLCONENAT #45503440566: PASS on SHA 6d5d5376e425587e6f116d09f88cbcc7ac6a05c1.
- No target MIPS/uClibc runtime or physical-router L4 validation is claimed.

## Exact first action for the next GPT

1. Confirm the AI handoff documentation workflow passes on the current main SHA after shrinking REFACTOR_ROADMAP.md below 131,072 bytes; repair any exact-SHA failure.
2. Recheck XY-C1 Build/TRX/artifact on the unchanged product-source baseline `c009462148d00369007edaf3e5b87d31899b07c2`; do not claim L3 while its run remains pending.
3. Resume the next distinct userland stability/security fix, starting with the ZeroTier stop path's corrupt firewall-checkpoint recovery gap; keep source/test/checker changes atomic and do not wait on unrelated PENDING runs.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Compare live refs, exact-head Actions, source baseline and first action against this file. Record resolved material FAIL/PENDING gates. If facts and first action still match, conclude the checkpoint with no documentation change or commit. Never create docs churn solely to synchronize a stored SHA.
