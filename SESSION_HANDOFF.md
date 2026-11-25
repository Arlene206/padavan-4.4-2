## Current repository state — 2026-11-25

- Live main parent before this IAPP diagnostic-format correction: 5ff68edb06869f3f58d7f69f5db1a96f9788ef2e; source, development log and handoff are one atomic descendant.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- IAPP firewall identity fix `defc1fe850158dd8c2bb179ad39a650037c11f5a` exact IAPP workflow #45686985107 PASS; AI handoff #45686985129 PASS; inventory #45686985036 PASS; Process #45686985030 PASS. Its XY-C1 #45686985116 was still in Build at last poll.
- IAPP TCP listen fail-closed fix `5ff68edb06869f3f58d7f69f5db1a96f9788ef2e` is committed. Its exact IAPP #45687310341, inventory #45687310391, NVRAM #45687310476, Process #45687310401, AI handoff #45687310376 and XY-C1 #45687310380 were queued/in progress at last inspection. No exact-head result inferred.
- This correction normalizes the TCP listen diagnostic's C newline escape; no behavior change.
- VSFTPD ACL gates remain verified on 8b9b63401dd85232b257120111bb18475db6b5b6 (VSFTPD, AI, Process, NVRAM PASS); its XY-C1 run #45685416720 remained in Build at last poll, so L3 remains unverified.
- PR #1 inventory complete: valid HMAC/CA payload already integrated or superseded; no port required. Backup untouched.
- Physical-router L4 NOT VERIFIED.

## Active task

Validate the IAPP TCP listen fix on exact SHA 5ff68edb06869f3f58d7f69f5db1a96f9788ef2e, including source contract, firmware build/TRX/artifact; then continue targeted audit for unchecked fatal startup resource calls.

## Exact first actions for next GPT

1. Inspect `5ff68ed` exact-head IAPP and AI handoff workflow results; fix any source-contract/test failure.
2. Poll defc1fe XY-C1 #45686985116 and 5ff68ed XY-C1 #45687310380. L3 requires Build + TRX + artifact upload PASS on the same production SHA.
3. Continue IAPP socket/startup audit only for concrete fatal setup statuses; then select another module with a demonstrated defect.
4. Main only; preserve the frozen backup and do not cherry-pick the closed PR.

## Execution contract

One complete Git tree and one commit per logical slice; non-force main update with exact expected parent SHA. No development branch, PR, or force push. PENDING Actions do not block independent work.
## 2026-11-25 autonomous handoff checkpoint

This checkpoint supersedes older active task pointers below; retain historical completed work without re-auditing completed areas.

## Current repository state

- Main base before RStats change: `195bc627d2d809da1877bc7c119a5a5e73342799`; this source fix is committed as one atomic non-forced descendant.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Dropbear checker update exact runs #45650274526 and #45650274515 were in progress at last poll; earlier #45649964742 failed at a stale assertion and skipped its new fixture.
- RStats persisted-history ring-cursor validation and GNU89 boundary regression are included in this checkpoint; exact RStats workflow pending.
- Physical-router L4 NOT VERIFIED.

## Active task

Reject raw RStats history records with negative or capacity-equal daily/monthly ring cursors before copying them to live state. Invalid persisted records are logged and discarded.

## Verification boundary

- RStats GNU89/ASan/UBSan regression and source-order contract: added; exact-head Actions pending.
- Dropbear corrected checker run has not yet demonstrated the dedicated behavioral fixture passes.
- No current same-SHA XY-C1 Build/TRX/artifact result is assumed. Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-SHA RStats workflow and repair any compiler, sanitizer, or source-contract failure.
2. Resolve Dropbear exact-SHA runs #45650274526 and #45650274515; verify its dedicated shell fixture ran and passed.
3. Continue an independent stability/security source fix while pending Actions run.

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update main with a non-forced expected-SHA compare-and-swap. Never force-push, use feature branches or PRs, split one logical change into per-file commits, or modify the frozen backup. PENDING never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Completed source slices

## 2026-11-25 — Additional FTP, DHCP, HTTPD, U2EC and PPP hardening

- VSFTPD ACL metadata entry race: `a4959513d54f9242dca864eaf5b2dae2d571ad7b` adds a post-read directory-entry identity check (dev/inode/size/link-count) so rename-and-replacement cannot make a pinned old inode masquerade as the current ACL file. A deterministic rename+replacement regression was added. VSFTPD #45571067464 PASS.
- DHCP client ACK server identity: `88eaeb8bfe67bf38714c171f94bb0aaa6d86bc91` shares a selected-server/REBINDING policy between ACK and NAK validation. REQUESTING/RENEWING remain bound to the selected server; REBINDING accepts another server only with a server-ID option. Regression #45572615762 PASS.
- HTTPD firmware CRC: `bf8eb40458b6accdb2e10c62f5889881cf099f38` pins firmware input as a no-follow, nonblocking, single-link regular-file descriptor before CRC reads; regression covers symlink/FIFO/hardlink rejection. `b59901afc7c02d809efa84bb76799225630ef87b` corrects the workflow's source-contract newline delimiter. HTTPD #45573299714 PASS on b59901a.
- U2EC build integration: `147a76bc637753a1f23b5aec74a59581512cd4d6` sets `.DEFAULT_GOAL := all`, preventing package.mk's bare make from stopping at the first object dependency and omitting the daemon binary. U2EC #45573521621 and Process inventory #45573521659 PASS. XY-C1 #45573521580 on this exact SHA passed Build, TRX and artifact upload.
- PPP callback lock descriptor: `97871e6c42862b38cf152f31171ea0b631f14118` adds `O_CLOEXEC` to both create and reopen paths for the WAN callback sidecar lock, preventing privileged child commands/daemons from inheriting the lock descriptor. Regression checks `FD_CLOEXEC`. Workflow variable/assertion corrections are `28383a3cb38f2b85127c257fc3973eecac143879` and `c78487d2b564b43f7a7f249618a8d5b4d340744b`; final WAN #45574283546 PASS on c78487d.


### 2026-11-25 — Cross-userland network, resolver and printer hardening

- WAN gateway ARP: detect_wan_arp_parse_gateway_address() distinguishes intentionally absent/0.0.0.0 from malformed non-empty NVRAM. Invalid configured IPv4 fails closed before ARP socket creation; empty/zero preserve legacy no-gateway behavior. Regression #45565621317 PASS on 583576a.
- MiniUPnP: strict bounded Content-Length parsing (including duplicate rejection), unsupported Transfer-Encoding rejection, 8 KiB header / 64 KiB body limits, bounded realloc growth and valid 431 fallback. GNU89 ASan/UBSan regression plus source contract #45566912855 PASS on 019f005.
- SmartDNS: Start_AD no longer truncates the last known-good adblock file before download/validation; invalid/empty/network-failed refresh preserves it, fresh install gets an empty default, and symlink targets are rejected. Shell regression and DNSMasq lifecycle/source contract #45568556249 PASS on 1817dd6.
- U2EC: bounded exact TCP receive/send, MSG_NOSIGNAL, strict inbound/reply lengths, IRP BufferSize backed by bytes actually received, and URB transfer-buffer bounds. GNU89 ASan/UBSan/source-contract #45569202180 PASS on e5d039b; no physical USB-device result claimed.
- p910nd: bidirectional copy_stream rejects descriptors outside [0, FD_SETSIZE) before FD_SET; this prevents out-of-bounds writes into fd_set. Makefile rebuilds when either policy header changes. GNU89 ASan/UBSan socketpair/source-contract #45569803535 PASS on e487703.
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

- HTTPD multipart helper: focused standalone GNU89 ASan/UBSan harness PASS. Repository fixture bytes were corrected in this test-only follow-up; exact `httpd-security` source/test workflow has not yet been observed for the new SHA.
- VSFTPD exact run #45645904216 PASS on `bed104b`; AI handoff #45645904067 PASS on that SHA.
- WAN regression #45642479262 PASS on `8835ec2`; XY-C1 #45642479259 on the same product SHA passed Build/TRX/artifact. No claim for later product SHAs.
- Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; inspect HTTPD security, AI docs and XY-C1 exact-SHA runs triggered by the fixture byte correction.
2. Confirm CR/LF/tab fixtures compile and run under the repository's GNU89 ASan/UBSan step, then validate the upload source contract.
3. Continue another independent userland stability/security source fix; PENDING Actions do not block it.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Compare live refs, exact-head Actions, source baseline and first action against this file. Record resolved material FAIL/PENDING gates. If facts and first action still match, conclude the checkpoint with no documentation change or commit. Never create docs churn solely to synchronize a stored SHA.
