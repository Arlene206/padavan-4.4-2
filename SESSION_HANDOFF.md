## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update `main` with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Parent main SHA for this handoff update: `7d589c2ea0a3f46356e8bc361d212f8a9b81776f`; refresh live refs before future writes.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- `main` is the only development ref. PR #1 is closed/unmerged and must not be ported.
- Bootstrap, protocol and handoff were recovered once; do not repeat startup audits without a new reason.

## Active task

### Make FTP credential lookup transactionally consistent

`asus_getpwnam()` previously read `acc_num`, each `acc_usernameN` and `acc_passwordN` through separate `nvram_get_int()` / `nvram_safe_get()` calls. Each call released the shared NVRAM API lock after one key, while account writers publish count, username and password changes under a multi-key exclusive `nvram_transaction`. Login could therefore observe mixed credential generations during account add, remove, rename or password change.

The reader now holds one `nvram_transaction_begin()` lease across the count, all active usernames and the matching password. Missing active usernames, duplicate matches, malformed/overflowing counts, missing/empty passwords and transaction begin/end failures all fail closed; the output password is cleared on every non-success path. A GNU89 host regression models a concurrent writer attempt and checks that it remains blocked while the credential pair is read, plus error and malformed-inventory cases. Parent main SHA: `7d589c2ea0a3f46356e8bc361d212f8a9b81776f`; inspect exact-head VSFTPD and AI docs checks after this atomic commit.

## Completed source slices

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

1. Inspect exact-head VSFTPD Stability and AI handoff documentation runs for the FTP credential snapshot change. Confirm the GNU89 ASan/UBSan regression and exact-source transaction contract pass; repair the first real failing step.
2. Recheck XY-C1 #45505371723 for its original source SHA `7d589c2ea0a3f46356e8bc361d212f8a9b81776f`; claim L3 only if Build XY-C1, TRX verification and artifact upload all pass on that identical SHA. Do not transfer it to newer auth source.
3. Continue an uncovered stability/security defect across Padavan userland. Do not reopen the verified SSH mode validation or FTP listing ACL, VTS port reads, WAN baseline, USB modem retry, OpenVPN certificate parsing, QMI identity, Wireless, HTTPD, DNSMasq, WAN route/DHCP, process inventory or USB storage work absent a new failing case.
4. Before any write, refresh live main and the frozen backup ref; before ending, refresh exact-SHA Actions and leave the first concrete next action.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Compare live refs, exact-head Actions, source baseline and first action against this file. Record resolved material FAIL/PENDING gates. If facts and first action still match, conclude the checkpoint with no documentation change or commit. Never create docs churn solely to synchronize a stored SHA.
