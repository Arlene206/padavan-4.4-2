## 2026-11-25 — Preserve DHCPv6 callback tokens when client stop fails

- `stop_dhcp6c()` advanced the shared callback generation before calling `kill_services("dhcp6c")`. If the daemon stop failed, the still-running client retained its old environment token while queued ip-up/ip-down children were released from the lock with a newer generation; those callbacks rejected themselves and silently skipped lease, address, prefix and resolver updates even though WAN6 teardown correctly aborted.
- The stop path now calls the daemon stop while holding the callback lock and advances the generation only after a successful stop. A failed stop leaves the active generation unchanged, so queued callbacks remain valid; successful stop still fences them before unlocking. Generation-write and lock-release failures retain first-error handling.
- Replaced the old one-sided race fixture with forked failed-stop and successful-stop cases; the DHCPv6 IA workflow compiles the GNU89 ASan/UBSan regression and checks exact production ordering. XY-C1 path filters include the RC implementation, callback policy header and regression. Exact-SHA Actions pending; no MIPS firmware or live-router result is claimed.

---

## 2026-11-26 — Align remaining PPP lifecycle source contracts with post-stop fencing

- Exact source SHA `6c0ea137` passed the WAN DHCP callback and other independent lifecycle checks, but VPN server, Process inventory and HTTPD umbrella checks still encoded the former rule that invalidation preceded `kill_services()`. The VPN checker also required generation advancement before the multi-WAN PPP stop.
- Updated those contracts to require: VPN stop → PPP daemon stop → checked failure/abort branch → callback-generation invalidation; launch generation now follows the successful multi-WAN stop and remains before helper/config publication. The same order is enforced in the VPN, Process, HTTPD and WAN-focused checkers.
- Checker-only follow-up. The production change is unchanged; exact-SHA reruns pending.

---

## 2026-11-26 — Correct post-stop PPP callback fencing source contracts

- Exact SHA `6c0ea137a5e94ab7bbd09c542498c5de52a43bf0`: WAN teardown workflow completed its GNU89 regression binaries but the inline Python contract raised `NameError` because deferred-reconnect marker variables were defined below their first use; its older full-WAN assertion also still demanded the previous pre-kill invalidation order.
- WAN route workflow passed its PPP option policy binaries and failed only because the checker compared the interior `kill_services(...)` token offset against the enclosing `if (...)` offset, which necessarily precedes it. The contract now compares the stop gate and failure return directly.
- Test-contract-only correction; production ordering from `6c0ea13` is unchanged. Exact-SHA reruns pending.

---

## 2026-11-25 — Keep PPP callback generations valid across aborted stop attempts

- Both WAN PPP stop paths advanced the callback generation before deferred-reconnect cancellation, VPN-client shutdown and PPP process termination. If any later gate failed, the old daemon could remain running while its existing ipparam token had already become stale; future ip-up/ip-down callbacks then silently skipped state and route updates.
- Moved callback invalidation after successful PPP callback/daemon shutdown in both teardown paths. The L2TP and multi-WAN launcher paths likewise advance the generation only after the previous transport/pppd stop succeeds, still before shared option-file publication. A failed stop therefore leaves a surviving session's callback identity valid.
- Added source-order regressions for all three paths; PPP option and WAN teardown workflows verify generation advancement follows successful process-stop gates. Exact-SHA Actions pending; no target firmware or live-router result is claimed.

---

## 2026-11-25 — Correct ZeroTier pre-existing-route regression marker

- Exact run #45738322534 passed the complete shell lifecycle regression, including the new pre-existing-route preservation and conflict cases. The Python checker then failed because its required marker said “accepted” while the regression correctly reports “rejected an exact route which was already present.”
- Corrected the expected diagnostic wording only; production and regression logic are unchanged.

---
## 2026-11-25 — Align ZeroTier stop preflight contract with route manifests

- Exact run #45738207061 passed the shell lifecycle regression but the Python source contract still required unconditional NVRAM route preflight before stop. The implementation correctly validates a persisted static-route manifest when one exists and uses checked NVRAM preflight only for legacy no-manifest installations.
- Updated the contract to require the legacy-only guard and preserve its ordering before daemon signalling. No production behavior changed.

---
## 2026-11-25 — Align ZeroTier route ownership source contracts

- The route-ownership fix moved checked selector enumeration and tuple validation into zerotier_static_route_state_save(); the older checker still expected these operations inside zero_route() and expected that rules() rollback used the legacy delete mode.
- Updated the contract to validate the new-start snapshot path, legacy cleanup compatibility, exact-route ownership checks and dedicated rollback operation; added required markers for the pre-existing/conflicting route regressions.

---
## 2026-11-25 — Preserve pre-existing routes during ZeroTier ownership cleanup

- New-start route manifests previously copied every enabled NVRAM tuple before ip route add. If an identical route already existed, a later stop could mistake the user's pre-existing route for a ZeroTier-owned route and delete it.
- The add snapshot now inspects each destination first: exact pre-existing routes are omitted from the owned manifest, absent destinations are recorded, and conflicting/ECMP/unknown route state fails closed before publication. Legacy no-manifest cleanup keeps its compatibility snapshot behavior.
- The rules() rollback path no longer invokes legacy NVRAM cleanup when add preflight failed before a manifest was published. Regression cases cover preservation of exact pre-existing routes, conflict rejection and idempotent no-manifest rollback. Exact-head CI pending; live router runtime remains unverified.

---
## 2026-11-25 — Match ZeroTier route-count checker to the persisted executor

- Exact ZeroTier run #45737539198 completed the shell lifecycle regression successfully, then the Python source contract failed because it still required the pre-manifest NVRAM loop and its local rulesnum variable inside zero_route(). The current executor consumes a persisted manifest; the shared save helper validates and bounds the checked NVRAM count before its seq loop, while manifest loading caps records at 64.
- Updated the source contract to follow the current executor-to-save-helper call chain and assert that shared bound. No production behavior changed.

---
## 2026-11-25 — Reset ZeroTier route-count manifests between compatibility cases

- Exact ZeroTier run on d9bb597 completed the bounded lifecycle tests and stopped in the later route-count table: the empty-string case correctly wrote a header-only route manifest, but the next zero-count case reused that same file and the production add path correctly refused to overwrite an unreplayed checkpoint.
- The count table now removes the known-empty manifest after each successful add, modeling an independent add/stop cycle for every accepted selector. Production checkpoint refusal remains unchanged.

---

## 2026-11-25 — Clear the empty ZeroTier route manifest between independent rules fixtures

- Exact ZeroTier run on 3d586 passed the /32 and NAT-readiness cases but the later partial-route-add case could not reach the route-add stub: the preceding successful NAT-only rules() fixture correctly left a header-only .static-routes-state checkpoint (zero configured static routes), and the next independent fixture reused it.
- Both duplicated test blocks now remove this known-empty manifest before modeling a fresh service start with an enabled static route. Production checkpoint behavior remains unchanged; this is fixture lifecycle isolation.

---

## 2026-11-25 — Isolate ZeroTier rule-publication tests from preserved stop checkpoints

- Exact ZeroTier run on e37ac5d passed the corrected legacy /32 setup but then failed when the later direct rules() fixture reused .static-routes-state and .firewall-state deliberately preserved by preceding stop-failure cases.
- Both copies of the direct rules() fixture now reset the private test configuration directory before their independent checks. This preserves production fail-closed checkpoint assertions and prevents expected leftovers from contaminating NAT readiness coverage; no production source changed.

---

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

