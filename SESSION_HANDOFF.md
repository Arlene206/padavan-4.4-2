## Current repository state

- Live `main` baseline before this SmartDNS slice: `2ad18b19ccb069746ee07f5a3a0efeef13cf2444`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and unchanged. PR #1 remains closed/unmerged.
- Bootstrap/protocol/handoff recovery was completed once in this continuation. Do not repeat startup audits or already-completed MiniDLNA, ZeroTier and IPv6/DHCPv6 work.
- Source commits are complete-tree Git commits published to main only by expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Active task

- VPN resolver: `8d19ad0` enforces strict `vpnc_pdns` values 0/1/2 before staging; `42eb80d` corrected mode 2 to exclude both WAN IPv4 and IPv6 upstreams only when at least one valid VPN DNS value exists; `19bce65` is a comment-only change; `9cf650e` relaxed formatting-sensitive checker matching.
- The checker at `9cf650e` still used the obsolete `/* DNSv6 servers */` marker. Exact WAN #45762094181 failed at this source assertion; all preceding compiled GNU89/sanitizer checks passed. This was fixed in checker-only commit `2ad18b19ccb069746ee07f5a3a0efeef13cf2444` by locating the guarded `wan_dns6_snapshot` consumer. Exact WAN rerun for `2ad18b1` pending.
- THIS SmartDNS source slice hardens the unchecked `nvram set sdns_redirect=0` used when port 53 is selected. Failure now preserves persistent/local selector and live DNSMasq config; the shell test injects a nonzero NVRAM write result and then validates the successful path.

## Verification boundary

- WAN route #45761724079 PASS on VPN selector commit `8d19ad0`; its DNS-mode error propagation source check passed.
- The follow-up mode-2 implementation commit `42eb80d` caused a new stale checker failure; exact WAN #45762008586 and #45762094181 failed only on locating the old IPv6 comment marker. All GNU89/ASan/UBSan steps before that checker passed. Checker fix is `2ad18b1`; exact run needs recheck.
- On `8d19ad0`, AI handoff #45761724234 PASS, ZeroTier #45761724083 PASS, Process inventory #45761724209 PASS, WAN DHCP callback #45761724217 PASS, Firewall #45761724163 PASS, DNSMasq #45761724316 PASS, USB modem #45761724259 PASS, IPv6 service-type #45761724142 PASS, WAN teardown #45761724250 PASS, Samba #45761724229 PASS and HTTPD #45761724104 PASS.
- XY-C1 on `8d19ad0` #45761724207 was IN_PROGRESS at Build; TRX/artifact pending. On predecessor `19bce653`, XY-C1 #45762008543 was also IN_PROGRESS at Build; these do NOT verify subsequent production source commit `42eb80d`. Latest-source L3 is NOT VERIFIED.
- Physical-router L4 remains NOT VERIFIED. This continuation used GitHub API operations only; local shell/Python execution was NOT RUN.
- Keep required handoff headings: `## Current repository state`, `## Active task`, `## Verification boundary`, `## Exact first action for the next GPT`.

## Exact first action for the next GPT

1. Inspect exact-main-SHA Actions from this commit. SmartDNS/DNSMasq workflow must pass its shell regression (including NVRAM status 37), shell syntax and `check_smartdns_dnsmasq_config.py`. Inspect first failing step/log rather than weakening the contract.
2. Check exact WAN route result from checker-only parent `2ad18b1`. Confirm the IPv6 replace-mode check is green. Reconcile latest-source XY-C1 Build/TRX/artifact; no L3 claim unless all three pass on the same production SHA.
3. Continue the next distinct stability defect across Padavan userland. Avoid repeat audits of VPN selector semantics, WAN replace mode, MiniDLNA and ZeroTier. Use source → test → atomic commit → exact-SHA Actions; PENDING does not block independent work.

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update main with a non-forced expected-SHA compare-and-swap. Never force-push, use feature branches or PRs, split one logical change into per-file commits, or modify the frozen backup. PENDING never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Re-check exact refs and relevant exact-SHA Actions before ending. Record material PASS/FAIL/PENDING changes and the next concrete action. If the current handoff remains accurate, leave it untouched; do not create a no-op commit.


## Completed source slices

### 2026-11-25 — Guard WSDD2 select registration against out-of-range descriptors

- `wsdd2.c` routes both endpoint registrations through `wsd_select_fd_add()`; fd >= `FD_SETSIZE` is rejected with `EMFILE` before `FD_SET`, preventing an out-of-bounds write into libc's fixed `fd_set`.
- GNU89 ASan/UBSan boundary regression covers fd 0, `FD_SETSIZE - 1`, `FD_SETSIZE`, `INT_MAX`, and canary preservation. Exact workflow: #45725275770 completed/success @ 6028a1387733f0f45d8b96f169dbc34061e9b8fc.


### 2026-11-25 — Harden WSDD2 select descriptor registration and netlink notification handling

- `258d673` adds a checked `wsd_select_fd_add()` gate at both endpoint registration sites. The previous raw `FD_SET(ep->sock, &fds)` would write past libc's fixed `fd_set` when an opened socket descriptor reached `FD_SETSIZE` or higher. Out-of-range descriptors now close the endpoint and follow the daemon's normal fatal startup path. GNU89 ASan/UBSan boundary workflow #45725275770 passed on this source SHA.
- `6028a13` checks `msghdr.msg_flags` after netlink `recvmsg()`. A truncated address-notification datagram now returns `EMSGSIZE` so the existing event loop restarts and re-enumerates interfaces, rather than silently accepting only the first 4096 bytes and potentially missing a tail notification.
- The netlink host test compiled and ran; its first two exact-SHA runs exposed checker-only bugs (a forward declaration match, then a double-escaped newline in the checker signature), not C test failures. `5d983a2` and `4de82b5` corrected the checker in two small test-only commits; latest netlink workflow status is recorded above.

### 2026-11-25 — QMI, rstats and UDPXY regression follow-ups

- `6631da6` hardens rstats raw-history open with `O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC` and requires a single-link regular file via `fstat()`; exact rstats workflow #45722628854 passed. Tests cover regular files, symlinks, FIFOs, directories and hardlinks.
- `4efe6e6` fails closed when `qmi_wwan` lacks its same-device `cdc-wdmN` control endpoint, preventing DHCP from starting on an unestablished QMI session; exact USB modem workflow #45723458320 passed.
- `61e7058` corrects the UDPXY source checker to ignore comments when checking for banned calls; the implementation had removed `strchr()`, but the explanatory safety comment itself triggered the previous substring assertion. The updated exact UDPXY parser workflow #45723868583 passed.
- UDPXY multicast membership retry work `b320dd2` passed exact request-parser and multicast-refresh workflows (#45724141327 and #45724141340).


### 2026-11-25 — DHCPv6 IA lifecycle and packet-input hardening

- `e7cf7e7` releases an unqueued IA event-data wrapper when `iapref_renew()` returns a recoverable preparation error; ordinary fatal `xzalloc()` OOM behavior is unchanged.
- `7b2c912` scans the complete IA option list for NoBinding before applying any address/prefix state during Renew/Rebind.
- `7614f9a` validates option header and payload bounds in both top-level and nested DHCPv6 decoders before pointer formation; unknown nested options are not skipped until their lengths are proven in-bounds.
- `6975c9b` rejects oversized UDP datagrams using Linux `MSG_TRUNC`; `90b82d0` separates socket errors from short datagrams; `4daa654` wires the helper into the receive translation unit.
- `cc401362` corrects the checker to expect the explicit signed-size comparison. All five DHCPv6 host policy executables passed in the preceding run, and the post-correction DHCPv6 IA workflow passed on `cc401362`.
- XY-C1 Build/TRX/artifact verification remains pending on product-source SHA `4daa654`; no L3/L4 success is claimed.


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
