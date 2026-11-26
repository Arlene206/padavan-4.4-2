
## Current development checkpoint — 2026-11-25

- Live `main` at checkpoint creation: `cc5cde0fcbbb68e62e2a5ad225f9695c9195b3f2`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged; do not modify).
- Startup protocol/handoff recovery was already completed earlier in this work stream. Do not repeat startup audits or re-review completed XY-C1/CI work from older SHAs.
- Current work has concentrated on WAN IPv6 input validation, BusyBox DHCP client socket recovery, DHCPv6 IA timer correctness, and safe DHCP route ownership.

### Source changes since the previous handoff checkpoint

1. `3fcd88e1f699f119eee2735346c6743a4b9f69d7` — reject IPv4-compatible and IPv4-mapped forms as WAN IPv6 interface endpoints; preserve NAT64 prefix as an allowed unicast address.
2. `62c0f13155e63a27e36b1cfba0febd2906bf6b8e` — ensure XY-C1 firmware validation is triggered when the WAN6 address-policy header changes.
3. `6b42794df888984ee29c41965100e726ec91da77` — add persistent DHCP-installed route ownership manifest; never claim an untracked `EEXIST` route; retain failed deletes for retry.
4. `102fa728279593e3911782e910da79ed29f7ed33` and `5596a66d2d4bf5f798e4e27f7c20055b9ffb25f4` — align old DHCP/WAN route source contracts and repair the route-status workflow's duplicate `run:` block and missing checker path filter.
5. `33e219bf1e741355e61da65c1d4212d02916629d` — scope route records by DHCP service prefix so WAN and IPTV owners cannot clean each other's entries.
6. `4bce1db9ca92e5317e9a2b4359bd2b0079fddad7` — strengthen owner-isolation regression and update route/callback source contracts for the internal owner argument.
7. `c91b7af2a654227c227571b265b3fc8401dd0f35` — fix v2 manifest parser field count (six parsed fields including owner).
8. `8caddb355060d19e74ba1732cf8307b6c9033f3d` — clean routes by stable service owner rather than current interface/metric; this also cleans routes left on an old interface after PPP/WAN interface transitions.
9. `cc5cde0fcbbb68e62e2a5ad225f9695c9195b3f2` — update the DHCP route checker to find the IPTV deconfig call in its enclosing function after formatting changed.

### DHCP route ownership contract now implemented

- Manifest format v2 records `owner-prefix + interface + metric + route tuple`.
- Add success is persisted atomically. An untracked `EEXIST` is treated as an idempotent add but is not claimed by DHCP; a tracked route that returns `EEXIST` remains owned.
- Teardown selects the stable service prefix only, not the current interface/metric. This handles WAN PPP/interface/metric transitions while keeping `wanN_`, `wanx_`, and `manv_` owners isolated.
- Manifest and lock files are checked as regular, owner-owned, non-symlink files with restrictive permissions. Writes use a temporary file, fsync, and rename. Failed deletes stay in the manifest; already absent routes are complete.
- GNU89/ASan/UBSan regression covers untracked `EEXIST`, manifest reload, owner isolation, different interface/metric under one owner, retry after delete error, malformed state, and symlink rejection.

### Actions state observed during this checkpoint

- WAN route operation status regression #45756989235 on `cc5cde0fcbbb`: **completed/failure** (2026-11-26T04:43:58Z).
- XY-C1 Firmware Validation #45756858864 on `8caddb355060`: **in_progress** (2026-11-26T04:42:06Z).
- WAN DHCP callback state stability regression #45756858682 on `8caddb355060`: **completed/success** (2026-11-26T04:42:32Z).
- NVRAM transaction boundary audit #45756858789 on `8caddb355060`: **completed/success** (2026-11-26T04:44:31Z).
- WAN interface teardown stability regression #45756858770 on `8caddb355060`: **completed/success** (2026-11-26T04:42:27Z).
- AI handoff documentation contract #45756858736 on `8caddb355060`: **completed/success** (2026-11-26T04:42:24Z).

Do not infer XY-C1 success from an older SHA. Verify that `Build XY-C1`, `Verify TRX output`, and artifact upload all pass on the same source SHA before recording firmware success. The route ownership test step may pass while a later Python source-contract checker fails; inspect exact failing step/logs before declaring a workflow PASS.

## Next concrete task

1. Poll the exact WAN route-status workflow for `cc5cde0fcbbb68e62e2a5ad225f9695c9195b3f2`; it was triggered by the checker-contract correction. Resolve any remaining checker or YAML failures and confirm the DHCP route ownership sanitizer regression is PASS.
2. Verify the XY-C1 build for source SHA `8caddb355060d19e74ba1732cf8307b6c9033f3d` (this source tree matches the production code at `cc5cde0fcbbb68e62e2a5ad225f9695c9195b3f2`; `cc5cde0` only changes the checker script). Confirm Build, TRX validation, and artifact upload separately.
3. Continue cross-userland stability work after these checks; do not repeat the already completed IPv6 and udhcpc regression audits.

### Verification caveats

- Physical-router runtime testing remains NOT VERIFIED.
- No firmware/build pass should be claimed unless the three XY-C1 stages above are successful on one SHA.
- Exact current-SHA CI state is represented by the run statuses above; anything still `in_progress` or `queued` remains pending.

## Current repository state — 2026-11-26 UTC

- Live `main` source head: `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`. This session's changes are on `main`; no feature branch or PR was used.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged. PR #1 remains closed/unmerged. Do not modify the backup or revive the PR.
- Startup protocol, handoff, roadmap and development log were restored once at the start of this work stream. Do not repeat startup recovery or audit previously completed startup/build work.

### Source commits in this work stream

1. `89c92c97853afccb1b2f116df3552b66cd3684f9` — reject unusable IPv6 interface/gateway literals (unspecified, loopback, multicast); add GNU89 address-policy regression and CI step.
2. `0622ba5e15f0bea349d25102f9311f3ca1ee6aac` — reject unspecified/multicast static and DHCPv6 DNS servers. The first exact run exposed an old source-checker expectation; the correction is in the next commit.
3. `c64608d3d1178e581bceca032e43759d769517c5` — retain the documented positive `inet_pton()==1` source contract while enforcing unicast DNS; exact WAN IPv6 DNS regression #45752311446 PASS.
4. `661e5d02e74dcd998628d1905f205430d9ad1683` — recover an active udhcpc receive mode when socket creation left `sockfd < 0`, retry every 3 seconds without extending the lease/renewal deadline, and add GNU89 regression coverage.
5. `f47045abd9890ccb5c4cdc79333d5f3d9aa44228` — update the udhcpc timer checker after the timer helper changed; this checker revision still had a whitespace-sensitive assertion.
6. `9b0b19e7bf3da2f0392f76adb8407d46a73a1a38` — make the udhcpc source assertion whitespace-insensitive. Exact BusyBox udhcpc run #45752972196 PASS on this SHA; the previous checker-only run #45752873093 failed and is superseded.
7. `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e` — age DHCPv6 IA preferred lifetimes from `monotonic_ms()` rather than wall-clock `time(NULL)`; preserve the infinite-lifetime sentinel and clamp expired finite values to zero. Add GNU89 regression cases for fractional seconds, exact/over expiry, largest finite lifetime and infinite lifetime.

### Exact-SHA CI boundary for `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`

- DHCPv6 IA publication / delegated-prefix retry #45753771175: PASS. Its GNU89 ASan/UBSan IA policy executable and source checker both completed successfully.
- WAN DHCP callback #45753771143: PASS.
- Process lifecycle inventory #45753771145: PASS.
- USB storage hotplug #45753771153: PASS.
- NVRAM transaction boundary #45753771126: PASS.
- XY-C1 Firmware Validation #45753771146: IN_PROGRESS at the cross-build job. At the last observed step poll, `Build XY-C1` was IN_PROGRESS, while `Verify TRX output` and artifact upload were PENDING. Do not claim firmware/L3 verification until Build, TRX validation and artifact upload all pass on the same exact SHA.
- The previous source SHA `c64608d3` had XY-C1 run #45752311476 and NVRAM run #45752311488 complete SUCCESS, including Build/TRX/artifact for that older SHA. That result is historical only and does not prove `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`.

## Active task

1. Continue polling exact XY-C1 run #45753771146. Record Build, TRX and artifact-upload outcomes separately; PENDING must not block independent source work.
2. Next investigation: verify DHCP route ownership in `trunk/user/rc/src/net_wan.c`, especially `control_dhcp_routes()` around lines 6953–7048. It treats `EEXIST` as successful add through `dhcp_lease_route_add_is_ok()`, but its corresponding delete path iterates the saved route list without an obvious persisted ownership record. Determine whether an identical pre-existing/static route can be deleted on WAN teardown. Do not simply remove idempotent-add handling; establish callback/restart semantics and add a focused regression before changing production behavior.
3. Keep work cross-userland. Each confirmed defect gets a source fix, focused regression, one atomic commit, CAS update to main, and exact-SHA Actions inspection.

## Verification boundary

- Exact DHCPv6 IA helper/test/source checker: PASS on `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`.
- Exact BusyBox udhcpc regression: PASS on `9b0b19e7bf3da2f0392f76adb8407d46a73a1a38`; do not treat it as an XY-C1 build result for later SHA.
- Exact current SHA `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`: five independent regression/audit workflows PASS; XY-C1 Build/TRX/artifact still unconfirmed at the last poll.
- Physical-router runtime L4: NOT VERIFIED.

## Exact first action for the next GPT

Poll XY-C1 run #45753771146 for `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`; verify the Build, TRX and artifact-upload steps. Then start the DHCP route ownership investigation at `control_dhcp_routes()` as described above. Do not repeat startup recovery, IPv6 DNS/address tests, or the already-passing DHCPv6 IA and udhcpc regression work.

Historical completed-source notes below are archival; their older first-action/checkpoint sections do not override this current state.

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
