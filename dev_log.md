## 2026-11-25 — Restore static-route selectors in the legacy /32 cleanup fixture

- Exact ZeroTier run #45736394732 at SHA ffba20a stopped at the older host-route normalization test after the new manifest cases intentionally reset TEST_STATIC_ROUTE_COUNT and TEST_STATIC_ROUTE_ENABLE to zero. The /32 fixture changed only the destination string, so it no longer configured a route for cleanup and never exercised the intended parser.
- Restored the enabled/count selectors specifically for that /32 case. This is a test-fixture correction; production route-manifest logic is unchanged. Exact-head ZeroTier rerun required.

---

## 2026-11-25 — Persist the exact ZeroTier static routes for later cleanup

- `zero_route add` installed routes from current NVRAM, but `zero_route del` reread current values at stop/restart time. If a user edited or removed a static route while ZeroTier was running, the old destination/gateway was no longer enumerated and could remain in the kernel after firewall cleanup and configuration removal.
- Added a private, same-directory atomically published `.static-routes-state` manifest. It records the interface plus every validated route tuple before any `ip route add`. Stop/restart cleanup loads and validates the exact manifest, not mutable NVRAM; the snapshot is cleared only after all routes are confirmed absent. A malformed or unterminated manifest fails closed. Legacy installs without a manifest retain the checked-NVRAM fallback.
- Extended lifecycle tests for NVRAM route drift, exact old-tuple deletion, truncated manifest preservation and replacement-daemon ordering; source checker enforces snapshot creation/load/clear and stop/replay integration. Exact-head CI pending; no live router route test claimed.

---

## 2026-11-25 — Revalidate VSFTPD LIST symlinks after directory classification

- The listing path authorized a symlink's resolved target, then performed a later followed fstatat() to decide whether the entry was a directory. A link replaced between those operations could inherit the prior target's read ACL and expose an unauthorized share name.
- Preserve the original no-follow entry identity. The directory probe now rechecks that identity and re-resolves symlinks to require the same canonical target before making the entry visible. A deterministic atomic-rename regression proves stale authorization is rejected and a freshly inspected replacement remains valid.
- The exact-head VSFTPD workflow compiles/runs the regression and enforces the source contract; XY-C1 already watches the affected production/header/test paths. Exact-head runs will be recorded after the commit; router runtime remains unverified.

---

## 2026-11-25 — Bound NVRAM rehash to the CRC-validated image length

- `_nvram_init()` checked and CRC-validated only `header->len` bytes, while `nvram_rehash()` put its terminator at `NVRAM_SPACE - 2` and scanned to the physical partition end. Stale/non-FF bytes after the declared image could therefore become settings despite not being covered by the validated CRC.
- Added an overflow-safe helper to compute the final double-NUL offset inside the declared image. Initialization now rejects lengths too short for that terminator, rejects over-capacity images, validates CRC, then passes the bounded offset to `nvram_rehash()`.
- Added GNU89 ASan/UBSan regression cases for short header/data, valid minimum and padded images, exact capacity, over-capacity and `size_t` overflow. NVRAM source checker pins validation/CRC/rehash order; XY-C1 filters now include the kernel driver/helper/test paths. Exact-SHA workflows pending; no hardware flash test claimed.

---

## 2026-11-25 — Repair the DHCP lease-clock integration checker after read-loop hardening

- Exact-SHA DHCP clock workflow on `d5a89c6` compiled and passed its GNU89 ASan/UBSan regression but its source checker failed. The checker still searched for the old `while (full_read(...))` record loop, which had intentionally been replaced by a `for (;;)` loop with explicit COMPLETE/EOF/ERROR classification in the preceding read-framing fix.
- Updated the source checker to assert the new order: timestamp validation, explicit record loop, full-record read, status classification, then clean EOF handling. No production source behavior changed.
- Parent `d5a89c6` read framing, atomic lease writer, AI docs, process regression, USB storage, process inventory and NVRAM workflows PASS. Parent clock C regression PASS/source checker FAIL; XY-C1 firmware build still running at last query. Exact-head result for the repaired checker pending.

---

## 2026-11-25 — Report malformed and truncated DHCP lease-file reads

- `read_leases()` previously treated every non-full `full_read()` result as ordinary loop termination. An empty/partial header, I/O error, or truncated final lease record was silently ignored, making persistent lease corruption indistinguishable from normal EOF.
- Added an explicit read-status classifier and diagnostics. Header EOF/short reads now abort loading with a warning; record EOF ends normally; negative read errors and positive partial/oversized records are reported. Previously completed records remain available when only the trailing record is incomplete.
- Added GNU89 boundary tests for complete records, EOF, short/oversized records and read errors; added a source-order checker and XY-C1 path filters. Exact-head workflow pending at commit creation; no live router test claimed.

---

## 2026-11-25 — Sync DHCP lease directory after atomic replacement

- File fsync plus rename made the new lease data visible atomically but did not guarantee that the directory entry update itself was durable across power loss.
- The atomic writer now opens the destination's parent directory before creating the temp file and fsyncs that directory after successful rename. Parent-directory open failure aborts before changing the live file. A post-rename directory-fsync failure returns a distinct positive status because the new file is already published; `write_leases()` logs the durability warning while still running the post-commit notification.
- Fault-injection coverage adds directory-open failure (old snapshot preserved) and directory-fsync failure (new snapshot visible, durability warning state reported). Local GNU89 test and exact-SHA Actions pending at commit creation; no power-cut hardware test claimed.

---

## 2026-11-25 — Guard atomic lease cleanup after temporary-file creation failure

- Review found that the atomic writer's failure label attempted to unlink the unexpanded `.tmp.XXXXXX` template even when `mkstemp()` itself failed. A pre-existing file with that literal name could be removed.
- Cleanup now unlinks only after `mkstemp()` returned a valid descriptor. Fault-injection regression creates a sentinel at the literal template path and verifies it survives the injected create failure; old destination preservation remains asserted for all pre-rename failures.
- Local GNU89 ASan/UBSan fault-injection test PASS. Exact-head CI pending; no device/storage power-cut result claimed.

---

## 2026-11-25 — Make DHCP lease-file replacement atomic

- `write_leases()` opened the live lease database with `O_TRUNC` and ignored the results of its header/record `full_write()` calls. An interrupted write or storage-full condition could destroy the last valid lease snapshot and cause lease loss after restart.
- It now serializes the current lease snapshot without mutating in-memory entries, writes through a same-directory `mkstemp()` file, loops over partial writes/EINTR, checks `fsync()` and close, then atomically renames only after all preconditions succeed. Failure cleanup closes/unlinks the temp file and preserves the old destination. `notify_file` runs only after replacement succeeds.
- Fault-injection regression covers temp creation, partial write, fsync, close and rename failures and checks the old file remains intact. Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` test PASS; exact-head CI pending. No device/storage power-cut test claimed.

---

## 2026-11-25 — Preserve DHCP leases across bounded clock rollback

- `read_leases()` computed `time(NULL) - written_at` then cast a negative result to `uint64_t` for the 12-hour age test. A small NTP/backward wall-clock correction therefore looked like an enormous age and discarded every saved lease; the same codebase's `dumpleases` already treats a future write stamp as zero elapsed time.
- Added a bounded time-delta policy: accept nonnegative timestamps within the existing 12-hour window in either direction, clamp a future write stamp to zero elapsed seconds, and continue rejecting larger deltas/invalid negative values. Production lease parsing uses the helper before loading any records.
- Added GNU89 ASan/UBSan boundary tests and an integration checker; XY-C1 path filters include the lease parser, policy header and test. Local policy regression PASS. Exact-head CI pending; no runtime router test claimed.

---

## 2026-11-25 — Keep INIT-REBOOT silent for unknown DHCP clients

- DHCPREQUEST's RFC 2131 text in `dhcpd.c` says an INIT-REBOOT client unknown to the server MUST receive no response. The prior fallback sent DHCPNAK whenever option 50 was present, including when `find_lease_by_mac()` found no local lease.
- The response decision now sends NAK for SELECTING, and for INIT-REBOOT only when a local client lease/static record exists. Unknown INIT-REBOOT and RENEWING/REBINDING paths remain silent. Added policy cases for SELECTING with/without records, INIT-REBOOT with/without records, and renewal/rebinding; the source checker pins the decision before NAK send.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy regression PASS. Exact-head DHCP policy, AI docs and XY-C1 checks pending at commit creation; no client/server/hardware result claimed.

---

## 2026-11-25 — Validate DHCPREQUEST state and field combinations

- `udhcpd` selected DHCPREQUEST lease lookup input from option 50 when present and `ciaddr` otherwise, but did not enforce the RFC 2131 option/`ciaddr` combinations. A malformed mixture could match a lease and receive DHCPACK, extending the lease despite an invalid client state encoding.
- Added a fail-closed check before lease selection and ACK: SELECTING requires server-id plus requested IP and zero `ciaddr`; INIT-REBOOT requires requested IP only and zero `ciaddr`; RENEWING/REBINDING require nonzero `ciaddr` and neither option. The GNU89 regression exercises accepted and rejected combinations, and the source checker pins the guard before lease lookup/ACK.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy regression PASS. Exact-head DHCP policy, AI docs, Process, NVRAM, USB and XY-C1 checks pending at commit creation; no live client/server or L3/L4 result claimed.

---

## 2026-11-25 — Reject DHCPINFORM without a client address

- `send_inform()` constructs an ACK using `packet.ciaddr`. When an INFORM has zero `ciaddr`, the shared sender broadcasts because its destination-selection rule falls back to broadcast when `ciaddr == 0`, exposing configured DHCP options to every client on the segment.
- Added a nonzero-`ciaddr` predicate before `send_inform()`. Tests cover zero, 1 and all-ones values; the integration contract checks that the guard precedes the ACK path. This does not compare `ciaddr` to the IP source address; the existing receive helper discards source-address metadata, and relayed INFORM packets have a relay source distinct from the client's `ciaddr`.
- GNU89 ASan/UBSan test and exact-SHA workflow pending.

---

## 2026-11-25 — Validate DHCPDECLINE and DHCPRELEASE request fields

- BusyBox `udhcpd` documented two unimplemented DHCP server checks in its handlers: DECLINE requires `ciaddr == 0`, and RELEASE must not contain option 50 (Requested IP). The server previously ignored these constraints and could mutate lease state for malformed messages.
- Added small policy helpers and tests for zero/nonzero DECLINE `ciaddr` values and absent/present RELEASE requested-IP option. The handlers now include those predicates before their lease mutation. An integration checker verifies guard ordering and that workflow and XY-C1 path filters include the production source/header/test.
- GNU89 ASan/UBSan regression prepared; exact-head CI pending. No full firmware result for this slice claimed.

---

## 2026-11-25 — Correct BOOTREQUEST checker Python quoting

- Exact `cfde353234ea855d259ab991a4dd310ff81e1d36` BOOTREQUEST workflow #45731440093 compiled and ran the GNU89 ASan/UBSan policy regression successfully, then failed the Python source-contract step with a SyntaxError in a double-escaped assertion matching the log call.
- Replaced the brittle embedded-quote assertion with a check for the distinctive message text `Ignoring non-BOOTREQUEST packet on relay client socket`. Production source, helper and C regression are unchanged. Exact-head rerun pending.

---

## 2026-11-25 — Reject non-BOOTREQUEST opcodes on DHCP relay client ingress

- The client-facing relay loop validated UDP datagram length and DHCP magic but did not validate BOOTP opcode. It proceeded to increment hops, set giaddr and call `pass_to_server()`, whose DHCP option parser accepts both BOOTREQUEST and BOOTREPLY. The downstream BusyBox DHCP server drops non-BOOTREQUEST packets, so bogus client-side BOOTREPLY packets could still reach the relay's XID reservation/forwarding path and consume scarce transaction state without a possible reply.
- Added a strict RFC 1542 BOOTREQUEST opcode check immediately after datagram/cookie validation and before hop mutation, interface IP lookup, option parsing or XID reservation. BOOTREPLY remains valid only on the relay's server-facing reply path.
- Added opcode policy regression cases for 0, 1, 2 and 255; expanded the source contract to require the guard before hop and giaddr logic. Exact-SHA Actions pending.

---

## 2026-11-25 — Avoid XID reservations for one-way DHCP messages

- `pass_to_server()` retained an XID slot for every forwarded message type. However, the DHCP server implementation handles DECLINE and RELEASE without transmitting a reply; relay reply forwarding accepts only OFFER/ACK/NAK. Reserving these transactions therefore consumed one of the relay's 1,024 bounded slots for up to 120 seconds without a possible completion event.
- DECLINE and RELEASE are still forwarded to the server, but bypass XID allocation. DISCOVER, REQUEST and INFORM continue through the existing transaction map because the server replies with OFFER/ACK/NAK/ACK respectively.
- Added a GNU89 policy regression for message types 1/3/8 versus one-way types 4/7, and expanded the Python source contract to require forwarding and early return before `xid_add()`. Exact-SHA XID and XY-C1 checks pending.

---

## 2026-11-25 — Reject oversized datagrams on DHCP relay server ingress; preserve concurrent VPN fix

- The DHCP fix commit `6899af46e9907efc0c40d6f8badc204aba0cf8d9` was created from `d4a965e`. Before its expected-SHA ref update completed, `main` advanced independently to `18cc546ac0eece109a230eb4e81fc019cd52bb59` for the VPN DNS replace-mode fix. The compare-and-swap did not move the ref; after rereading it, the DHCP tree was rebuilt on top of current `main`. The integration commit uses `18cc546` as first parent and `6899af4` as second parent; no force push or concurrent change was overwritten.
- The DHCP server-socket path had used `udhcp_recv_kernel_packet()`/ `safe_read()`, which cannot distinguish a datagram larger than the supplied buffer from one exactly equal to the buffer. It now uses `recvfrom(..., MSG_TRUNC)`, zeroes the packet, checks the full reported length, minimum DHCP option bytes and cookie, and drops invalid datagrams before option parsing.
- Added a GNU89 ASan/UBSan AF_UNIX datagram regression for original-length reporting under `MSG_TRUNC`; expanded the Python receive-path contract to require the server ingress checks while preserving client-ingress assertions. Local test PASS.
- Parent `d4a965e` had all 12 Actions PASS, including XY-C1 Build/TRX/artifact. The concurrent `18cc546` update had 9 PASS workflows and three still IN_PROGRESS at last poll: Process lifecycle #45730461987, NVRAM #45730461818, XY-C1 #45730461930. The new integrated SHA's exact Actions remain to be checked.

---

## 2026-11-26 — Accept the documented VPN DNS replace policy

- The VPN UI exposes `vpnc_pdns=0/1/2` (disabled / add VPN DNS to current list / replace current list). `update_resolvconf()` already preserves this distinction: mode 2 omits WAN DNS fallbacks.
- Both PPTP/L2TP `ipup_vpnc_main()` and OpenVPN `on_client_ifup()` incorrectly used `vpn_client_nvram_read_bool_status()`. Valid mode 2 was rejected as `EINVAL`, preventing the VPN DNS snapshot from being published and leaving stale resolver state.
- Added a strict tri-state policy helper and GNU89 regression for values 0/1/2, missing-key default, malformed/out-of-range values, and backend-error preservation. Both callbacks now use it; source contract requires that integration. Exact-head CI pending.

---

## 2026-11-26 — Align DHCP relay XID source contract with client destination

- Exact `6bfef12` XID workflow #45728716637 failed at `check_udhcp_relay_xid_policy.py`: its integration assertion still required `sendto(..., &item->ip)`, but `pass_to_client()` now constructs a protocol-derived `client_dest` and sends to `&client_dest`. The standalone GNU89 ASan/UBSan XID executable passed.
- Updated the checker to require `client_dest` initialization, client port 68, policy-based address selection and `sendto(..., &client_dest)`. Removed the obsolete BOOTP broadcast TODO and in-place mutation of the saved source sockaddr, preserving the captured source tuple for fallback/retry. Exact-head rerun pending.

---

