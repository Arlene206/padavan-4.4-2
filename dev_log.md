## 2026-11-27 — Treat PID-less procfs snapshots as UNKNOWN

- Shared process lifecycle fallback treated an empty or non-PID-only procfs listing as a complete negative scan in `pl_process_state()`; the package-client command-line fallback had the same flaw when the root contained nonnumeric entries such as only `sys`. This could authorize teardown/unmount on an incomplete process inventory.
- Both scanners now require at least one numeric PID directory to be observed before returning ABSENT. Invalid zero-prefixed PID entries fail UNKNOWN; normal disappearing PID directories remain tolerated. Regression covers empty and non-PID-only roots for both process-name and full-command-line scans.
- Exact-SHA Process lifecycle regression is required. Local shell execution and hardware L3/L4 are not claimed.

## 2026-11-27 — Keep ZeroTier shell route pattern compatible with source-contract parser

- Exact-SHA ZeroTier run #46054412381 on `c8c3256f339b70bf3e66a1526275e0e94ccfe508`: the lifecycle shell regression passed, including duplicate destination and implicit/explicit `/32` scenarios. The Python source checker then failed before evaluating route contracts because its C-style comment scanner interpreted the POSIX shell case pattern `*/*)` as the start of a `/* ... */` block comment; WAN-up was skipped.
- Rewrote the slash test as `*'/'*)`, which has the same POSIX shell semantics but avoids the scanner's false C-comment token. Added an explicit source-contract assertion for the compatible form. No production routing semantics changed.
- Re-run the exact-SHA ZeroTier workflow after this checker-compatibility correction; the prior shell test PASS does not count as a full workflow PASS. No local target build or hardware L3/L4 result is claimed.

## 2026-11-27 — Reject duplicate ZeroTier static-route destinations before service stop

- Static-route preflight validated each entry independently but did not detect two enabled rows targeting the same network with different gateways. Such rows can create an ECMP/multipath alias; cleanup intentionally refuses to delete one leg because Linux 4.4 gateway/device deletion may remove the entire alias. The duplicate was otherwise detected only during later route installation, after the old daemon had already been stopped.
- Preflight now allows only one managed next hop per destination and canonicalizes an unqualified host route to `/32` for duplicate detection. Regression covers duplicate `/24` entries with different gateways and the equivalent host-IP/explicit-`/32` spelling; it verifies start fails before signaling the running daemon or changing kernel state.
- Exact-SHA ZeroTier lifecycle validation is required for this slice. No local shell execution or router L3/L4 validation is claimed.

## 2026-11-27 — Align signal-source checker with failure-injection seam

- Exact-push DNS Forwarder run #46053530493 on `9e4616c4dc7093a8fc590c18c394cddf8dc80a3c` failed in the Python contract checker at the legacy assertion requiring a direct `hev_event_source_add_fd()` call. The implementation now routes that call through `DNS_EVENT_SOURCE_SIGNAL_ADD_FD` for regression injection; earlier launch-path and service/installer assertions passed, but every C regression step was skipped.
- Updated the checker to assert the production seam used by the new constructor. No C test, sanitizer test, or firmware result is claimed from the failed run; re-run the exact-SHA workflow on this correction.
 
## 2026-11-27 — Roll back signal masks when signalfd setup fails

- `hev_event_source_signal_new()` previously ignored signal-set/blocking errors and left the requested signal blocked if `signalfd()` or event-source attachment failed. The caller received no source capable of consuming that signal, so later delivery could remain pending indefinitely.
- Validate signal-set and `sigprocmask()` operations; remember whether the caller already blocked the signal; on every post-block construction failure, undo only the block introduced by this attempt while preserving the original setup errno.
- Added test-only failure injection for signalfd creation and fd attachment. The event-loop regression checks invalid signal rejection, mask rollback, preservation of a pre-existing mask, descriptor cleanup, and successful SIGTERM delivery. The DNS Forwarder workflow compiles it with GNU89, `-Werror`, ASan and UBSan. Local execution was unavailable in the connector-only session; exact-SHA Actions remains the executable verification gate.

## 2026-11-26 — Remove duplicate SIGTERM source path entries

- DNS Forwarder #45854983436 (`84ad788...`) and #45855217820 (`5fc01c7...`) failed in the Python workflow path-filter contract before any C test ran. The `hev-main.c` path had been inserted twice in each push/pull_request filter; the checker path inventory also listed it twice and loaded the event-loop lifecycle fixture three times.
- Removed the duplicate `hev-main.c` entries from both trigger filters and the checker inventory, and normalized the event-loop fixture to one read. No production signal-handling code was changed.
- On `84ad788`: AI handoff #45854983388 PASS and Process inventory #45854983397 PASS; XY-C1 #45854983214 and NVRAM #45854983349 were in progress at last poll. SIGTERM and remaining C regressions await a clean exact-SHA rerun.

## 2026-11-26 — Detach event sources when post-dispatch preparation fails

- The event loop checked a source's `prepare()` result during initial registration, but ignored it after dispatch. A timeout or other source that failed to re-arm remained attached with an invalid readiness state and could silently stop servicing work.
- The post-dispatch path now routes a false `prepare()` result through the existing invalid-source cleanup: clear the current fd's stale event bits and detach/unref the source. A source removed by its own dispatch is still not prepared again.
- Added a real epoll/pipe regression: initial prepare succeeds, dispatch fires, the next prepare fails, and the test asserts that the source has been detached. The DNS Forwarder checker now protects both implementation and regression presence.
- Parent `a05bb8e170058c41ff88b671b99b11451f1f0a3c`: DNS Forwarder #45854126331 PASS; Process inventory #45854126325, XY-C1 #45854126393 and NVRAM #45854126370 were in progress at last poll. The parent also added a 4098-byte EDNS-capable TCP buffer and an explicit oversized-response-frame close policy; this new event-loop regression is untested until its exact-SHA workflow runs.

## 2026-11-26 20:26 CST (UTC+8) — Inject DNS Forwarder constructor setup failures

- Added a test-only dependency seam around the production constructor's resolver, socket setup, event-source creation/attachment and event-loop registration calls. Production builds retain the normal libc and event-loop implementations.
- Added a GNU99 ASan/UBSan regression that injects failures at getaddrinfo, socket, FIONBIO, SO_REUSEADDR, bind, listener-source allocation/fd attachment/registration, timeout-source allocation and timeout registration. It checks errno preservation, resolver release, listener close, source detach/unref counts and open-FD balance; it also exercises successful initialization and teardown.
- Extended the DNS Forwarder checker and identical push/pull_request source filters. Local compilation is unavailable in this connector-only environment; exact-head Actions provide executable validation. No target-build or hardware result is claimed.

## 2026-11-26 — Propagate DNS Forwarder CLI startup failures

- The DNS Forwarder CLI ignored the return from `hev_event_loop_add_source()`, continued after failed signal-source construction, and returned success when the forwarder could not initialize. It also dereferenced `dns_servers` after an unchecked `strdup()`, leaked replaced option values, and mixed an interior pointer to the DNS server string with independently allocated port ownership.
- CLI startup now checks every required allocation/constructor/registration, exits nonzero on initialization failure, and unwinds the source, forwarder, loop and owned strings through one cleanup path. The upstream port substring remains owned by `dns_servers`; only a separately duplicated default port is freed.
- The DNS Forwarder source checker and push/PR filters now include `hev-main.c`. Firmware/hardware runtime verification remains outstanding.

## 2026-11-26 — Make DNS Forwarder listener initialization transactional

- The constructor validated the upstream endpoint but still ignored `ioctl(FIONBIO)`, `setsockopt(SO_REUSEADDR)`, event-source allocation, fd attachment and event-loop registration failures. It also acquired the socket before resolving the bind endpoint and leaked the listener, addrinfo or source references on several failure branches. An unchecked nonblocking setup could leave an edge-triggered event loop blocked in `recvfrom()`.
- Reworked construction to validate inputs and resolve the bind endpoint before opening the socket, require nonblocking/reuse-address setup and each event-source registration to succeed, and use one cleanup path that detaches registered sources, unreferences unregistered sources, frees addrinfo, closes the listener and preserves the original error.
- Extended the DNS Forwarder source-contract checker and push/PR path filters to cover the production constructor. Host parser and event-loop regressions remain the executable CI checks; router runtime verification is still outstanding.

## 2026-11-26 — Align WSD HTTP framing checker with bounded XML tag scanner

- The WSD XML scanner/reflection workflow #45847368757 passed on `c4a627632235f2e240797a41294e5dc062848f06`, including GNU99 ASan/UBSan compile and execution. The independent HTTP framing workflow #45847368988 failed only at an obsolete assertion expecting `const char *nested = strchr(p, '<');` inside the prior tag finder.
- Updated `check_wsdd2_http_framing.py` to require the new length-carrying wrapper, bounded helper delegation and no legacy `strstr()` tag scan, preserving the HTTP framing assertions. Rerun is pending on the correction SHA.
- XY-C1 run #45847369057 had passed input detection and host checks and was compiling Build XY-C1 at last poll; TRX and artifact status remained pending.

## 2026-11-26 — Parse WSD SOAP action fields as bounded XML text elements

- `wsd_tag_find()` searched raw network XML with `strstr()`, so tag-like bytes inside comments/CDATA/processing instructions or quoted attributes could be treated as actual `wsa:Action` / `wsa:MessageID` elements. It ignored the supplied body length and silently selected the first duplicate.
- Added a bounded lexical scanner that recognizes only supported start/end tag tokens, skips comments/CDATA/PIs, respects quoted attributes, rejects DTD/declarations, embedded NUL, duplicate targets and nested target markup, and never reads beyond the supplied body span. HTTP rejects embedded-NUL SOAP bodies and passes the exact declared body length.
- Added GNU99 ASan/UBSan tests for comment/CDATA/PI/attribute spoofing, duplicate elements, malformed markup, exact body bounds and normal extraction; expanded the existing WSD regression workflow's source paths and checker. Parent main: `a039b68d37cb3eb5f2acdd4ae7d448c6851d5223`; exact-SHA CI and XY-C1 firmware build pending.

## 2026-11-26 — Reject malformed DNS Forwarder upstream ports before resource acquisition

- The DNS Forwarder converted its configured upstream port with `atoi()`, silently accepting `0`, negative input, trailing junk, and overflow/truncation. Its upstream address was validated only after opening the listener and event resources, so invalid addresses returned early with resources still live.
- Added a strict ASCII-decimal parser for ports 1–65535 and validate upstream IP/port before allocating the forwarder or acquiring sockets. The validated address and port are then copied into the upstream sockaddr. GNU89 ASan/UBSan regression covers bounds, signs, whitespace, trailing data, overflow, and unchanged output on failure.

## 2026-11-26 — Correct DNS Forwarder event-loop test source list

- Exact run `#45846000054` passed the Python source/path contract, then the new host-test compile failed because the workflow referenced a nonexistent `hev-event-source-fd.c`; this repository implements the fd helpers in `hev-event-source.c`. Removed the stale compilation unit from the test command; no production code change.

## 2026-11-26 — Make DNS Forwarder event-source registration transactional

- Found several interacting lifetime defects in the DNS Forwarder event core: `epoll_create()` failure was accepted as a valid loop; `hev_event_loop_add_source()` ignored fd registration and `prepare()` errors, leaving partially registered sources while returning success; detach did not clear `source->loop`; and a failed attached `hev_event_source_add_fd()` freed its node without unlinking it from the source list.
- Event-loop source registration now fails closed and rolls back prior `epoll_ctl(ADD)` operations, source detach clears its owner loop, and fd-add allocation/registration failures leave no dangling list entry. Timer/signalfd constructors now unwind if fd attachment fails, and timer activation errors are reported to the loop.
- Added a GNU89 ASan/UBSan regression that forces partial epoll registration failure, retries after rollback, exercises failed fd attachment on a live loop, and reattaches a detached source to another loop. The existing DNS Forwarder checker and workflow now cover the new C sources/tests on both push and pull_request. Router runtime/L4 verification remains outstanding.

## 2026-11-26 — Add the timeout policy to MiniUPnP PR path filters

- On correction SHA `037a94b`, the GNU89 ASan/UBSan policy binary PASSed and the AI handoff documentation workflow PASSed (#45845054818). The embedded source checker then found that the newly added timeout helper was present in the MiniUPnP workflow's push path list but absent from its separate pull_request list.
- Added the header to the PR filter as well; this is workflow wiring only, not a production-code change. XY-C1 on the production source SHA `b81ac781` had passed detector and host-policy steps and was still compiling Build XY-C1; TRX/artifact remained pending.

## 2026-11-26 — Correct MiniUPnP timeout checker fixture and roadmap size

- The exact-SHA event-subscriber workflow #45844543377 compiled and passed the GNU89 ASan/UBSan capacity/timeout binary, then failed only because the source checker expected `Second-InFiNiTe` while the test used the valid mixed-case `sEcOnD-InFiNiTe`; align the literal, not production behavior.
- AI handoff workflow #45844543967 found the roadmap at 131153 bytes, 81 bytes above its 131072-byte cap. Condensed the MiniUPnP dashboard row while preserving relevant lifecycle/security status and new timeout work.
- HTTP framing #45844543368 and event callback #45844543183 PASSed on `b81ac781`; XY-C1 #45844543988 remained in progress. Parent main: `b81ac7815d7fbe3b4573f742b00cbdba19c2e0d7`; rerun both failed host contracts on the corrected exact SHA.

## 2026-11-26 — Parse MiniUPnP event timeouts without integer or time_t overflow

- The network-facing MiniUPnP `Timeout:` parser used `atoi()`, accepting malformed signed/trailing-junk values and invoking undefined behavior for out-of-range input; event subscribe/renew then added the untrusted duration directly to signed `time_t`.
- Added bounded exact parsing for `Second-nnnn` and `Second-infinite`, rejects duplicate/malformed header values before dispatch, and uses an overflow-safe saturating deadline for both subscriber creation and renewal. Existing zero/infinite and strict-build default semantics are preserved.
- Added GNU89 ASan/UBSan boundary tests for grammar, integer overflow, invalid inputs and simulated 32-bit deadline saturation; updated subscriber source contract, dependencies and XY-C1 production path filters. Parent main: `349a3ad2ba842a7eeb2f38b2e4dd307bfa1a715a`; exact-SHA Actions pending.

## 2026-11-26 — Include networkmap production in XY-C1 firmware validation

- The XY-C1 workflow path filter covered core RC sources and multiple userland packages but omitted trunk/user/networkmap/**, so networkmap production changes did not trigger the firmware build even though the package is part of the firmware.
- Added the networkmap subtree to push triggers. The existing changed-tree detector still skips tests-only changes while treating networkmap.c and production policy headers as build inputs; unit coverage now verifies both decisions.

## 2026-11-26 — Preserve networkmap ARP socket across daemon descriptor redirection

- `networkmap` opened its packet socket before `daemon(0, 0)` but did not ensure the returned descriptor was above `stderr`. If any standard descriptor was closed, `socket()` could return 0, 1 or 2; daemonization redirects those descriptors to `/dev/null`, replacing the live ARP socket. Cleanup also used `arp_sockfd > 0` and leaked descriptor 0.
- Duplicate low descriptors with `F_DUPFD` starting at 3 before daemonization, preserve the duplication error on failure, and treat every nonnegative descriptor as open during cleanup. GNU89 policy tests cover descriptors -1 through 3; the source contract enforces duplication before return and the cleanup guard.

## 2026-11-26 — Correct networkmap timeout checker signature literal

- Exact run #45841079284 passed all three GNU89 ASan/UBSan policy binaries, including the new timeout error cases, then failed only because the embedded checker encoded the expected source newline as a literal backslash sequence. Corrected the checker to interpret the newline; production timeout propagation remains unchanged.

## 2026-11-26 — Fail closed when networkmap cannot bound ARP receive waits

- `nmap_init()` and the full-scan completion path ignored `setsockopt(SO_RCVTIMEO)` results. If the initial timeout could not be installed, `recvfrom()` could block indefinitely, preventing periodic discovery and delaying signal-driven shutdown; failure to restore the normal timeout after a full scan could also leave a stale 60 ms cadence.
- Added one checked timeout publisher, propagate initial setup errors before entering the daemon loop, and abort the current iteration if timeout restoration fails. GNU89 regression covers successful option setup, preserved errno, and `EIO` fallback; the source contract enforces both call-site error paths.

## 2026-11-26 — Align detect_internet interrupted-probe source contract

- Exact run #45840372153 compiled and passed the GNU89 ASan/UBSan signal, host-policy and NVRAM-policy binaries; its embedded checker then failed because a new order assertion matched the comment text "Linux select()" before the wait loop, rather than the actual call. Tightened the checker to order `FD_ZERO()` before `ret = select()`; production code and regression results were unchanged.

## 2026-11-26 — Do not count interrupted Internet probes as host failures

- `di_connect_to_host()` treated every `select()` return `-1` as an unreachable endpoint. `SIGHUP` or periodic `SIGALRM` can legitimately interrupt the nonblocking-connect wait without pausing the monitor, so one control signal could incorrectly mark the current probe host down and feed a false offline state.
- Retry only `EINTR` when the monitor is not paused/terminating, rebuilding `fd_set` each time and using Linux `select()`’s remaining `timeval` to retain the original bounded wait. Pause/terminate still unwind immediately. GNU89 regression covers retry/stop decisions; workflow source contract requires the retry fence inside the wait loop.

## 2026-11-26 — Align HTTPD security DHCP lease contract with fail-closed Router parser

- Exact HTTPD security run #45839116268 on `825282da1530323e591acc5dc0124a1bb3343413` passed the DHCP lease GNU89 policy binary (`DHCP lease input policy regression: OK`) then failed in the embedded checker at obsolete assertion line 62: it expected `dhcp_lease_ipv4_list_first()` in `udhcpc_lan_bound()`.
- LAN callback now uses `dhcp_lease_router_option_parse()`; malformed/oversized Router values are rejected before `dhcp_lease_bound_apply()` and `lan_dhcp_nvram_state_publish()`. Updated the cross-workflow contract to require parser error return ordering, reject the old permissive list helper, and assert invalid trailing-address / meta-character regressions. Production C unchanged.
- Other exact-SHA results on `825282d`: WAN DHCP callback #45839116459 PASS; watchdog #45839116450 PASS; IPv6 service-type #45839116290 PASS; detect_internet #45839116334 PASS; wireless radio #45839116279 PASS; detect_link #45839116257 PASS; firewall startup #45839116413 PASS; AI handoff #45839116416 PASS. Process inventory #45839116212, WAN route #45839116267, DNSMasq lifecycle #45839116300, NVRAM audit #45839116359, and XY-C1 #45839116241 were in progress at last poll.
- Parent `main`: `825282da1530323e591acc5dc0124a1bb3343413`; this is a checker-only correction.

## 2026-11-26 — Reject malformed LAN DHCP Router options before changing the live lease

- Found a fail-open classification bug in `udhcpc_lan_bound()`: `dhcp_lease_ipv4_list_first()` returned `-1` for malformed Router input, but the callback treated it as an empty option. The BOUND/RENEW transaction then interpreted malformed input as an intentional withdrawal, deleted the last-known-good default route and published an empty gateway.
- The shared `dhcp_lease_router_option_parse()` distinguishes a valid absent/empty option from malformed/oversized input (fatal `EINVAL`) and clears partial output on failure. LAN now returns the parser error before address/route transaction or NVRAM publication. A genuinely absent/empty Router option still withdraws the route as before.
- Added GNU89 parser regressions for invalid trailing addresses and shell/meta-character injection, and an exact source-order contract proving the callback returns before route/address apply and lease-state publication. The existing WAN DHCP callback workflow runs the policy test under ASan/UBSan; exact-SHA Actions are pending.

## 2026-11-26 — Separate SmartDNS managed markers from unterminated DNSMasq files

- The managed-block publisher copied the active DNSMasq file and appended its begin marker without checking the final byte. A user-owned file with no terminating LF could absorb the marker into the last directive; subsequent stop cleanup rejected the malformed block and left SmartDNS directives behind.
- Added a checked last-byte probe against the staged copy (not the concurrently mutable source) and a separator only when required. Probe or staging-write failure aborts before rename and removes the stage. Regression covers no-final-LF publish/cleanup and exact probe-error preservation.

## 2026-11-26 — Reject malformed and spoofable networkmap ARP frames

- Confirmed `nmap_receive_arp()` accepted any payload at least `sizeof(ARP_HEADER)` before interpreting sender/target data. Unsupported ARP hardware/protocol types, address lengths or opcodes, plus zero/multicast/broadcast sender MACs, could influence client discovery and MAC updates.
- Added a strict on-wire Ethernet/IPv4 ARP predicate: hardware type Ethernet, protocol IPv4, lengths 6/4, and operation REQUEST or REPLY. Added sender-MAC validation requiring nonzero unicast addresses. Invalid frames are discarded before debug output, source-IP processing, or client-table mutation.
- Extended the existing GNU89 ARP policy binary with valid request/reply and adversarial wrong hardware type, IPv6 protocol, address lengths, operation, zero/multicast/broadcast MAC cases. Updated source-contract ordering to require rejection before interpreting sender/target data. Existing workflow path filters already cover the modified source, policy header, test and checker.
- Parent `main`: `b4758abe4d81752f604013d6819ce4ba9d29e9c6`. Exact validation is pending for this commit; no production build or hardware claim follows from this host-policy change.

## 2026-11-26 — Align WAN DHCP contract with DHCPv6 config-token validation

- WAN DHCP callback run #45835965906 on exact SHA `3cb9d6b` failed because its inline source contract still required the former length-only `strlen(wan_ifname) >= IFNAMSIZ` guard. The DHCPv6 writer now calls the shared `wan_ifname_copy()` policy and binds the validated local name before emitting `interface %s {`.
- Replaced the stale assertion with include and copy → bind → config-emission ordering checks plus adversarial line-break/comment token fixtures. This follow-up changes no production code. Dedicated DHCPv6 IA workflow #45835968592 already passed the GNU89 ASan/UBSan token regression and source contract on exact SHA `3cb9d6b`.

## 2026-11-26 — Validate DHCPv6 interface tokens at config publication boundary

- `start_dhcp6c()` wrote the supplied interface name directly into a DHCPv6 configuration-language statement (`interface %s {`). Its local precondition checked only nonempty/length, leaving the privileged config writer dependent on every caller having already enforced the safe token alphabet.
- Reused `wan_ifname_copy()` at the writer boundary. The validated bounded copy replaces the incoming pointer before configuration flags are read, a session lock is acquired, or the staged config is written. Invalid whitespace, quotes, comment/directive punctuation and line breaks are rejected without touching the daemon/session.
- Added source-order assertions and wired the existing GNU89 `wan_ifname_policy.c` adversarial token regression into the DHCPv6 IA workflow under ASan/UBSan. Exact-SHA workflow and XY-C1 Build/TRX/artifact checks are pending; no hardware validation is claimed.

## 2026-11-26 — Align detect_internet checker with WAN-up core wrapper split

- Exact run #45831162388 on `80db7072a08e98b80a7e8290040d8d2e910aff65` passed the GNU89 ASan/UBSan signal, endpoint and NVRAM policy regressions, then failed in the final inline source contract at `body(wan, "wan_up_internal(")`.
- Verified the monitor wakeup and status accumulation are inside `wan_up_internal_with_route_status()`; the legacy `wan_up_internal()` wrapper delegates to that core with a NULL DHCP-status output. The `net_wan.c` blob at `80db707` is identical to current main, so the failure is a stale test selector rather than a production regression.
- Updated the workflow contract to inspect the scoped core and separately assert the compatibility wrapper delegation. Its push/PR filters already include the workflow path exactly once. Production source unchanged.
- Previous related exact checks: WAN route #45832732293 PASS and AI handoff #45832732309 PASS on `14a0ec858738597e43febc08c10f2c7679d920c4`; USB modem #45832891378 PASS on `4e9b81adb99cc6ba32428d6be5672a6c288b066a`. XY-C1 #45832262776 on `144b109` finished host checks but skipped Build/TRX/artifact, so no L3 claim.
- Parent main: `4e9b81adb99cc6ba32428d6be5672a6c288b066a`; resulting commit: THIS COMMIT.

## 2026-11-26 — Align initial WAN MTU checker with DHCP retry-marker semantics

- Exact WAN route run #45832262803 on main `144b109` passed DHCP route ownership and WAN VLAN/bridge checks, then failed because `scripts/check_wan_initial_mtu_status.py` still expected the old inline `route_err` / `manv_route_err` setters.
- Updated both normal WAN and IPTV MTU source contracts to check the shared fail-safe marker publisher, route/MTU error propagation and marker-latch regression. Missing MTU remains optional; a failed MTU application is retained as retry state rather than silently accepted on an unchanged renewal.

## 2026-11-26 — Align DHCP retry-marker source contracts across WAN route and HTTPD

- Exact SHA `80db7072` exposed stale static contracts: the WAN VLAN checker and HTTPD DHCP reconciliation check still required direct inline writes to `route_err` / `manv_route_err` and `err` / `manv_err`. Those expectations were updated to validate the shared marker publisher, checked writer and persistent all-marker fallback.
- Added `scripts/check_wan_vlan_setup.py` to both WAN-route workflow path filters so future standalone contract edits always receive an exact push/PR validation.
- WAN route #45831162683 and HTTPD #45831162519 failed at those obsolete source assertions; their remaining assertions had passed up to the marker checks. The new source-contract commit requires fresh exact-SHA tests.

## 2026-11-26 — Keep WAN/IPTV DHCP retry state latched when an NVRAM marker write fails

- Found a fail-open retry gap after candidate DHCP lease publication: if a single `err`, `route_err` or `mtu_err` marker write failed while the other two writes succeeded with zero, the callback correctly returned an error to udhcpc, but the next unchanged renewal could read an all-zero marker tuple and skip the required full address/route/MTU re-application.
- Added a shared marker publisher for normal WAN and IPTV. It attempts all diagnostic marker writes; on any write failure it best-effort writes a nonzero retry latch to all three markers, while preserving and returning the original publication error. Added GNU89 regression coverage that injects a persistent failure at each marker position and verifies that the successfully stored portion always still requests a retry.
- Updated the WAN DHCP source contract to require the shared fail-safe publisher in both callback paths. The host test and exact-SHA Actions result are pending until this commit is checked.

## 2026-11-26 — Make VSFTPD configuration publication crash-durable

- `ftp_config_file_publish()` flushes and fsyncs staged data, renames it, then fsyncs the parent directory with EINTR retry. A post-rename parent-sync error propagates so `run_ftp()` refuses startup; parent lookup follows configured `/etc` resolution.
- GNU89/Werror regression covers publication, /dev/full failure, missing-parent errors and directory-sync behavior; the source contract checks file-sync → rename → parent-sync order.
- Commits: `63972673ad970fb439a175b21499078d6433c09f`, `65d6e0030edcebdf568dd4dba3ba281a143654aa`, `7f9a288dd74d7cbfdecf03f3b7ea4709bccfd740`, checker fix `0eaada3d45cc8c627a902a962927ad40a3ec5330`, XY trigger filter `f0fbe839174fb6b569c036f21c34a55903ed5965`.
- VSFTPD #45828730341 PASS on `0eaada3`; XY-C1 #45829100040 was building on `f0fbe839`.

## 2026-11-26 — Retain Inadyn last-known-good config through SIGHUP

- DDNS update snapshots the prior config before replacement, restores on write failure, retains recovery state after uncertain SIGHUP delivery, and commits the snapshot only after successful signal delivery and directory sync.
- GNU89 rollback regression added. Commits `4c0b2a02b021c813845e3a5356cdfd0cf987c4ab`, fixture correction `916fbec0fcc6739548870888fdbb24216683955c`, checker fixes `2b2b6ec122e5b4e9c7f94c3599ffd0c4da60c526` and `ece28b515b546cd8539089975322cb79ecbe6395`. Inadyn #45826971290 and DNSMasq #45826971254 PASS.

## 2026-11-26 — Cover repeated failed udhcpc REBINDING callbacks

- Added a GNU89 regression proving repeated failed callbacks cannot commit a new server/requested IP/BOUND state or extend the previous lease deadline; source checker locks down failure/success commit order.
- Exact BusyBox test #45825922148 PASS on `7be435059c1f8ffbd757ea95ba41fe466557960f`. No production `dhcpc.c` change was justified.

---

## 2026-11-26 — Scope DHCP BOUND callback failures to lease-critical work

- Confirmed that normal WAN and IPTV DHCP BOUND callbacks consumed aggregate WAN/MAN up status. Failures from post-route auxiliary work such as firewall and IGMP/multicast startup could therefore be returned to BusyBox as DHCP script failure after an otherwise successful lease address/route application.
- Added route-status-scoped implementations `wan_up_internal_with_route_status()` and `man_up_internal_with_route_status()`; route-critical status is captured before ancillary service startup while existing aggregate wrappers remain for other callers. Normal WAN and IPTV callback status now uses `dhcp_bound_callback_status(route_setup_status, mtu_status)`. IPTV multicast startup remains separately managed and no longer rejects the lease ACK; route, MTU and required NVRAM retry-state publication failures still fail closed.
- Added/extended GNU89 policy and source-contract coverage for the route-only status boundary and ancillary-service exclusion.
- Production source commits: `c36ab9c49ebd68fde9b07454da65e19212c182fd`, `e49e86262257c592ad1e0be1b68cccd49c476517`, `15274181953dc258d6cec4e47d9038d2e3187b47`. Checker/workflow follow-ups: `45cb6f12843035d47ce61dd2d4fa6dd58cd39e79`, `3f54cf2ba737835e9ea1a028fbe0fd229fe113e9`, `cbb470d0054854b3d8469a699ce78601f537c881`, `2377c6094e9acc5358a8abc6873f039eb057438f`, `f14f2b4d4d450f8dec403ed17ca74533ba31d204`, `e41af1199ab8279421fbed098557181767b92f01`, `405bb48aad75b5ea89aaec668a1b1f79fb140f97`, and `21822988b5b299e4f931d6659c175515952bdc57`.
- WAN DHCP callback exact run #45824870841 PASS on SHA `405bb48`. WAN route #45824870995 FAIL on that older checker because assertions still expected the previous callback shape; source-contract assertions were corrected at `21822988` and exact rerun #45825386107 was QUEUED. AI handoff docs #45825386055 and XY-C1 #45825386117 were also QUEUED on `21822988`.
- XY-C1 #45824870811 was overall SUCCESS but Build XY-C1, TRX verification and artifact upload were SKIPPED; this is not L3. No current-source L3 or hardware/L4 claim.
- Next: trace BusyBox handling of nonzero BOUND/RENEW script results after candidate NVRAM lease publication, including T1/T2/lease-deadline and repeated ACK/rebind behavior. Add host GNU89 regression before any further production edit.

---

## 2026-11-26 — Make automatic LAN DHCP resolver publication part of the rollback transaction

- Confirmed a concrete transaction-boundary defect: the automatic LAN DHCP resolver writer used a single-file publisher that could return failure after rename had already exposed the candidate `/etc/resolv.conf`. The outer DHCP transaction would then restore the previous kernel address/routes but leave the new resolver file in place.
- Added a rename-boundary-aware internal publisher and a rollback-aware wrapper using the existing hard-link snapshot/restore primitives. The public single-file helper remains compatible for unrelated callers. The automatic LAN DHCP resolver publisher now restores the prior resolver on a post-rename failure, or removes the new target when no prior target existed.
- Added deterministic GNU89 C regression coverage for an injected post-rename error with both previously existing and absent resolver destinations. Updated the DNSMasq/WAN DHCP/HTTPD static source contracts and the rollback diagnostics.
- Production commit `0c897b7ee4fd80bddad936b7d5bc3d8433917676`; diagnostic correction `72225a52c467a09dcdd5a4e252451d8d9df7eaf1`. Checker/workflow follow-ups: `57bd20e60f73f20d41fed7f08fee5fc5bdde7448`, `24ec9ce37430eaadcdfd3edc3f325ed4737f25b4`, `e428e722c2fbabde120a832bf93316bcf513d7ea`, `8f96eb6ecee967d448667072393aa99758079e7e`, `8477a7777fbd49717ae89a93011671500ce1a9c4`, and `b25835d921e2f3d9b3ae8fec39cb93549aa26e5e`.
- Exact source SHA `72225a52c467a09dcdd5a4e252451d8d9df7eaf1`: DNSMasq #45819215235, WAN DHCP #45819215341, HTTPD #45819215135, NVRAM #45819215141, WAN route #45819215171 and AI handoff docs #45819215253 PASS. Other non-product checks in the same exact-head batch also PASS. XY-C1 #45819215001 remains IN_PROGRESS: all pre-build host checks passed; Build XY-C1 was running and TRX/artifact steps were pending at last poll. No L3 or hardware/L4 claim.
- Re-reviewed `leasefail`, NAK, failed-deconfig retry, SIGUSR2/release and stop/wait semantics; no new correctness defect established, so no speculative change. Next investigation is the WAN DHCP BOUND/RENEW route/MTU-failure return contract and BusyBox retry/deadline interaction, with a focused host regression before any source change.

---

## 2026-11-26 — Keep DNSMasq rollback generation until daemon readiness is confirmed

- Confirmed a transaction-boundary defect in `start_dns_dhcpd_locked()` and `restart_dhcpd_locked()`: static-host output rollback snapshots were committed before the detached dnsmasq daemon passed its RUNNING postcondition. Successful staging/ARP apply did not prove the child remained alive; early commit could discard the only recovery state for an unready generation.
- Production fix `f7b61ba09b9750b3e85c3884b6101a511a12c6e1` keeps the static-host file and ARP snapshots through launch and `dnsmasq_confirm_started()`. Failure invokes a shared rollback helper: it stops/rechecks the candidate; prior files/ARP are restored only on a complete ABSENT process snapshot. RUNNING/UNKNOWN preserves the matched generation and private rollback links. The original launch/postcondition error remains authoritative.
- Checker repairs: `f6de10e6...` updated compacted helper-call matching; `9166a3d4...` corrected helper and success-commit ordering; `3147c02...` corrected inline workflow assertions to require rollback before success commit. DNSMasq #45814219448 PASS and AI handoff #45814219362 PASS on `3147c02f2bddba93a6034e529c5d8935b2768a05`. Current main later received comment-only manual-recovery logging clarification `992bdf6...`; its exact DNSMasq #45814745710 and XY-C1 #45814745724 were pending at this checkpoint.
- Same product-source SHA `f7b61ba...`: NVRAM #45813611887, WAN DHCP #45813611721, Process inventory #45813611693, miniupnpd #45813611534, WAN route #45813611530, Process #45813611502, HTTPD #45813611375, Samba #45813611309 and Inadyn #45813611245 PASS. No L3 claim; hardware L4 NOT VERIFIED.

---

## 2026-11-26 — Stop QMI WDS using the persisted session client ID

- QMI teardown without an explicit client ID may allocate/use a fresh WDS CID, which does not identify the persistent packet-data session established under the saved CID. Treating that command as success could discard the only marker for the still-active session.
- Production fix `4bfccc2e7c46df3495544ae0bb46f473775352f0` reads the persisted WDS CID and PDH marker, rejects unreadable/corrupt/inconsistent state before side effects, and supplies `--set-client-id wds,<saved-CID>` before `--stop-network`. After stop it checks PDH marker removal, releases that exact CID, then removes the CID marker. Each failure preserves recoverable state.
- Follow-up checker/workflow fixes culminated in `bdc8992f6e9a7a6884422c9af3c88177abc700c5`; exact USB modem #45813115174 PASS and AI handoff #45813115104 PASS. Host/CI L2 only; live modem hardware and WAN tests remain unverified.

---

## 2026-11-26 — Require truncated process comm prefix to agree with argv1 fallback

- Found a false positive in `proc_read_identity()`: for any process with a truncated `/proc/PID/stat` comm, an unrelated executable could be counted as a target service solely because its second argument (`argv1`) matched the requested process name. For example, a long-named worker invoked as `worker dnsmasq` could make a dnsmasq absence check report RUNNING.
- Tighten the fallback: `argv1` may disambiguate only when the first 15 bytes of the truncated comm match the requested process-name prefix, in addition to exact basename equality of `argv1`. Normal `argv0` and untruncated comm matching remain unchanged.
- Add GNU89 regressions for an unrelated long worker with `argv1=dnsmasq` (must be ABSENT) and for a matching long process name whose truncated comm prefix plus exact `argv1` identify it (must be RUNNING). The existing Process workflow compiles the scanner/test with GNU89, Werror, ASan and UBSan.
- Parent main: `4f688607c0ef60f829be7454c9c6ca891c9bf77f`; resulting commit: THIS COMMIT. No local compiler run or MIPS/firmware result is claimed; exact-head Process Actions and XY-C1/L3 remain pending.
- Beijing time at preparation: 26/11/2026, 19:17:48 (UTC+8).

---

## 2026-11-26 — Align ZeroTier source contracts with cached Moon preflight

- Exact-head ZeroTier lifecycle #45807554002 on `15bced4fa9c4b24b461ee663f062a49207f036ba`: the shell regression completed successfully, including the new Moon address/identity read-failure, invalid-selector, old-daemon-preservation and PPP fallback cases. The Python source checker failed on two obsolete expectations only: the marker list expected expanded shell loop text, and the removal function was required to perform a direct NVRAM read despite the read being intentionally moved into the shared preflight.
- Update the checker to assert the shell loop covers both `zerotiermoon_ip` and `zerotiermoon_id`, retain the running-daemon preservation assertions, and require `remove_moon()` to consume its preflight snapshot without a late read.
- Parent main: `15bced4fa9c4b24b461ee663f062a49207f036ba`; resulting checker-only follow-up commit: THIS COMMIT. Recheck ZeroTier lifecycle and AI handoff on the exact resulting SHA.
- Beijing time at preparation: 26/11/2026, 19:07:46 (UTC+8).

---

## 2026-11-26 — Preflight ZeroTier Moon inputs before stopping the current daemon

- Found a lifecycle gap in `start_zero()`: it read the five general selectors and route/NAT settings before `kill_z()`, but the Moon branch read `zerotiermoon_ip` and `zerotiermoon_id` later inside `start_instance()`, after the old ZeroTier daemon was already terminated. A read failure, invalid configured/PPP fallback IPv4 endpoint, or malformed Moon ID could therefore abort replacement startup while unnecessarily leaving the service down.
- Added a bounded Moon preflight for create mode and remove mode. It reads the relevant checked NVRAM values, resolves the PPP WAN fallback when the configured address is empty, validates IPv4 or the empty/10-hex Moon identity selector, and stores the values before any process/network side effect. `start_zero()` only exposes the snapshot to the one replacement `start_instance()` invocation; direct `creat_moon()` / `remove_moon()` calls still perform their own fresh preflight and do not consume stale cache values.
- Extended lifecycle regressions for Moon address and ID read failures, invalid Moon IPv4 and removal ID, preservation of a running daemon/no signals/no network mutation, and a valid PPP WAN address fallback. Updated source contracts to require all Moon reads and validation before `kill_z()` and prohibit late NVRAM reads in the mutating Moon handlers.
- The previous exact-head checks on `6fd35a9b57fc6aa85b95c52c62a1961a6c1ac373` are VPN lifecycle #45805581756 PASS and AI handoff #45805581869 PASS. Product L3 was verified only on earlier production SHA `f96dce30881937a686e6bc7e9407a41d47f27c2e` (#45805039346: Build + TRX + artifact PASS); this new ZeroTier source SHA requires its own tests and is not claimed L3-verified. Hardware L4 remains NOT VERIFIED.
- Beijing time at preparation: 26/11/2026, 19:06:27 (UTC+8).
- Parent main: `6fd35a9b57fc6aa85b95c52c62a1961a6c1ac373`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Fix the VPN marker checker by using one normalized source view

- Exact SHA `6edf19ecd1a31332ebc3e23465861ab9399099e8`: AI handoff #45805256973 PASS; VPN lifecycle #45805256951 compiled and passed all C policy regressions, then failed at the marker parser source assertion. The prior checker-only correction used a raw regular expression with double-escaped metacharacters, so it searched for literal backslashes rather than accepting the C function's wrapped formatting.
- Replace the regex with a whitespace-free view of the extracted launcher and compare the parser, strict 0/1 validation, process scan, atomic config publication, ownership policy, marker write and ownership assignment in that single coordinate space. This also makes the checked side-effect order explicit.
- No production C or policy semantics changed. Rerun exact-head VPN lifecycle and AI handoff; inspect the remaining f96dce workflow results and the current product-build boundary before further source work.
- Parent main: `6edf19ecd1a31332ebc3e23465861ab9399099e8`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Make the VPN marker source contract accept wrapped parser calls

- Exact-head VPN lifecycle #45805039088 on `f96dce30881937a686e6bc7e9407a41d47f27c2e` compiled and passed all C policy regressions, including the newly added existing-owner/no-owner marker cases. The final Python contract alone failed because the checked `rc_wan_nvram_parse_int_value()` call was intentionally formatted across two C lines while the checker matched a single-line substring.
- Replace that brittle substring lookup with a whitespace-tolerant regular expression; retain the later order assertions for successful parsing, strict 0/1 validation, ownership policy, marker write and ownership flag.
- This is a checker-only correction. VPN lifecycle and AI handoff must be checked again on this exact new SHA. The same production SHA's HTTPD #45805038950, WAN route #45805039104, AI #45805039195, WAN DHCP #45805039154 and Process inventory #45805038990 PASS; Process fail-closed #45805039322, NVRAM #45805039114 and XY-C1 #45805039346 were still running at last poll.
- Beijing time at preparation: 26/11/2026, 18:51:35 (UTC+8).
- Parent main: `f96dce30881937a686e6bc7e9407a41d47f27c2e`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Preserve pre-existing VPN-client ownership across shared xl2tpd start failures

- Audited every caller of `safe_start_xl2tpd_locked()` after the previous rollback fix. The helper unconditionally wrote `l2tp_cli_t=1` whenever the configured VPN client used L2TP, then marked the value as newly published. If an active VPN-client dependency had already published that marker, a later WAN PPP or VPN-server start failure would clear the existing marker as if this call owned it. Subsequent shared-daemon stop/restore logic could then miss a live client dependency.
- Extend the checked xl2tpd NVRAM transaction snapshot with `l2tp_cli_t`; parse and validate the prior state before process/config side effects. A pure policy gate now publishes only for `use_vpnc && marker_active == 0`; rollback ownership is recorded only after that transition succeeds. A pre-existing marker remains untouched on daemon failure. A newly acquired marker still rolls back, and the direct VPN-client caller retains cleanup for its pre-published marker.
- Add GNU89 policy cases for inactive/new/existing marker ownership and strengthen the VPN source contract to require the ninth snapshot field, strict marker validation, and snapshot → policy gate → write → owned-state ordering. Exact-head VPN lifecycle #45805039088 ran the full C regression set successfully, including the new marker policy cases, then failed only because the source checker expected the multi-line parse call on one line. The checker correction is committed next; the same SHA's HTTPD #45805038950, WAN route #45805039104, AI #45805039195, WAN DHCP #45805039154 and Process inventory #45805038990 PASS. Process fail-closed #45805039322, NVRAM #45805039114 and XY-C1 #45805039346 were pending at last poll; no L3 or hardware result is yet claimed.
- Beijing time at preparation: 26/11/2026, 18:50:09 (UTC+8).
- Parent main: `ce917d69b0436033f4c8c83384dcd0a9b61deb5c`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Rebind WAN PPP and HTTPD contracts to atomic xl2tpd publication

- Exact SHA `1d33cb41aca31eab3d58621a243272dc9cbd0000` exposed two more stale contracts after shared `/etc/xl2tpd.conf` publication moved to `l2tp_config_file_publish()`: WAN route run #45803174983 still compared protocol-read failure against the removed `fopen(l2tp_conf, "w")` position; HTTPD run #45803174928 raised `ValueError: substring not found` for the same obsolete anchor and still expected direct `fprintf` calls inside the launcher. The C policy regressions and the preceding independent HTTPD checks passed; these are checker-location failures, not evidence of a production regression.
- Update `scripts/check_ppp_wan_options.py` to require protocol read failure to return before the atomic publisher, reject the old truncate/unlink writer, and require the publisher helper. Update the HTTPD inline PPP confidentiality contract to inspect `xl2tpd_config_write()` for the preserved LNS/LAC sections and credentials, while checking launcher preflight order and atomic publication. Preserve all existing behavior assertions.
- The pool arithmetic contract has already been corrected in `aa1bd8367768ab66f62a22834861d05df41af0e6`; exact-head VPN lifecycle #45803534463 and AI handoff #45803534499 PASS on that SHA. This commit's WAN route, HTTPD and AI exact-head results must be checked after push. XY-C1 #45803175068 on product source SHA `1d33cb41aca31eab3d58621a243272dc9cbd0000` remains IN PROGRESS at Build; TRX and artifact upload pending. Older product SHA `3c36c0a0f780657b42e7b9d804064fadd9658c3c` has a successful same-SHA Build/TRX/artifact run #45801858571, but does not verify the later source SHA. Hardware L4 NOT VERIFIED.
- Beijing time at preparation: 26/11/2026, 18:44:11 (UTC+8).
- Parent main: `aa1bd8367768ab66f62a22834861d05df41af0e6`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Bind VPN pool address source assertions to the extracted atomic writer

- Exact-head VPN lifecycle run #45803175004 on `1d33cb41aca31eab3d58621a243272dc9cbd0000` passed every preceding GNU89/`-Werror`/ASan/UBSan policy binary, including the xl2tpd process-presence and rp-L2TP atomic config regressions, then failed in the final Python source contract. The old checker still searched `safe_start_xl2tpd_locked()` for the VPN pool address arithmetic after that arithmetic had intentionally moved into the new `xl2tpd_config_write()` callback. This was a stale source-contract location, not a test or production failure.
- Normalize the callback's whitespace and verify that both start/end addresses are derived from its validated `config->vpn_server_address`, `config->vpn_server_mask`, and pool offsets. No production source or policy semantics changed.
- Exact-head AI handoff #45803174976 PASS on the failed-checker SHA. The checker-only follow-up `aa1bd8367768ab66f62a22834861d05df41af0e6` then passed VPN lifecycle #45803534463 and AI handoff #45803534499. On the atomic publisher follow-up `ce917d69b0436033f4c8c83384dcd0a9b61deb5c`, WAN route #45804117385, HTTPD security #45804117258 and AI handoff #45804117195 all PASS. XY-C1 #45803175068 was still in progress before Build XY-C1 on product source 1d33, with TRX and artifact upload pending. Older product SHA 3c36 subsequently passed Build, TRX and artifact upload in #45801858571; this does not validate the later source SHA.
- Parent main: `1d33cb41aca31eab3d58621a243272dc9cbd0000`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Publish the shared xl2tpd configuration atomically

- Found that the shared xl2tpd writer still used `fopen("/etc/xl2tpd.conf", "w")`, unchecked individual `fprintf` calls, and then unlinked the live config if flush, close, or chmod failed. A full filesystem or interrupted write could therefore destroy the last-known-good config after truncating it; an existing symlink would also be followed.
- Reused the existing no-follow, same-directory `l2tp_config_file_publish()` helper for the shared daemon config as well as rp-L2TP. A dedicated writer callback now propagates formatting/write errors; the helper stages a private regular file, flushes/fsyncs it, applies the established 0644 mode, and atomically renames it over the target only after success. Failed staging leaves the previous live config intact and removes the temporary file. Checked NVRAM/process/pool preflights remain before publication.
- Extended the VPN lifecycle source contract to require all three existing config sections and inputs in the callback, reject the old truncating writer/unlink path, and enforce checked-snapshot/process/pool order before atomic publication. The existing GNU89 ASan/UBSan atomic-config regression already covers symlink-target protection and ENOSPC rollback. Exact-head VPN and AI workflows are required; no local firmware build or hardware result is claimed.
- Parent main: `cce2c18a0ba3a90570a733c526d900b47e5eec3e`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Fix the VPN-client marker regression helper return type

- Exact-head VPN lifecycle run #45801858401 on `3c36c0a0f780657b42e7b9d804064fadd9658c3c` exposed a test-only compile failure before the source checker: the new named regression helper used the suite's `CHECK` macro (which returns `1` on failure) but was declared `void`, triggering `-Werror=return-mismatch`. The production marker rollback patch was not reached by this run.
- Change the helper to return `int`, return zero after all cases pass, and assert its result from `main`. No production source or policy semantics changed.
- The corrected VPN workflow and other exact-head Actions must run on this commit; do not carry forward the failed parent result. The 3c36 parent test stopped at the xl2tpd policy compile step, so later workflow steps were skipped.
- Parent main: `3c36c0a0f780657b42e7b9d804064fadd9658c3c`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Roll back the VPN-client marker when shared xl2tpd launch fails

- Continued the shared L2TP lifecycle audit and found an ownership leak in `safe_start_xl2tpd_locked()`: when the coherent config says the VPN client should use xl2tpd, it publishes `l2tp_cli_t=1` before launching the daemon. If process inventory confirmed absence but `eval("/usr/sbin/xl2tpd", ...)` then failed, the helper returned without clearing the marker. The direct VPN-client caller had its own rollback, but WAN PPP and VPN-server callers of the same helper did not; they could leave a phantom VPN-client dependency that affected later shared-daemon stop/restore decisions.
- Track whether this invocation successfully published the VPN-client marker. On unsuccessful completion, clear only that marker, log rollback failure, and restore the original daemon-start errno. An already-running daemon remains a successful outcome and retains the marker. Added a pure policy helper, GNU89 regression cases, and source-order assertions for publication → completion failure → rollback → errno restoration.
- Exact-head VPN lifecycle run #45801196538 on prior checker-fix SHA `7be0f3b1bdc3d5c4fdc07dcd52990cc86e356d9a` was queued/in progress at last poll; this new production source slice requires its own exact-SHA Actions. No local build or hardware result is claimed.
- Parent main: `7be0f3b1bdc3d5c4fdc07dcd52990cc86e356d9a`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Normalize all VPN WAN marker rollback source-order offsets

- The exact-head VPN lifecycle run #45800802057 on `1a0ff27d90b1d6cb396fcc45d979bbde0ebeab50` reran all 14 preceding GNU89 policy regressions successfully, then failed in the Python source contract. The previous whitespace correction compared offsets from the whitespace-stripped source for the rollback gate/call against offsets from the original source for the marker, launch and failure branch. Those index spaces are not comparable, so a correctly ordered implementation was falsely rejected.
- Normalize every WAN marker/launch/failure/rollback anchor and the related errno-preservation checks against the same compact source string. This is a checker-only correction; the C rollback implementation and policy cases are unchanged.
- AI handoff #45800802088 PASS on the failed-checker SHA. Exact-head VPN lifecycle and AI handoff reruns are required for this correction; no firmware L3 or hardware L4 claim.
- Parent main: `1a0ff27d90b1d6cb396fcc45d979bbde0ebeab50`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Roll back the WAN xl2tpd dependency marker after failed launch

- Found that `launch_wan_pppd_locked()` publishes `l2tp_wan_t=1` before calling `safe_start_xl2tpd_locked()` so concurrent shared-transport lifecycle decisions preserve the intended WAN dependency. If checked configuration, process inventory, live config publication, or the daemon launch then fails, the common launch-error path invalidates the PPP callback generation and returns without clearing the marker. Later VPN server stop/restart can incorrectly conclude that a WAN L2TP dependency still owns the shared xl2tpd daemon and may stop or restore it based on a phantom client.
- Track only a successfully published WAN marker and roll it back on failed L2TP/xl2tpd launch. Preserve the original launch errno if marker cleanup also fails, and log the cleanup error. RPL2TP and already-running xl2tpd paths remain unchanged.
- Added a pure policy helper and GNU89 policy regression for published/unpublished markers across success and failure statuses; extended the VPN source contract to pin publication → launch → failure rollback ordering. Exact SHA `3c53eb5a5fd339d77bc650fc2fefc7d8b60e81b4`: the xl2tpd policy test and the other twelve preceding GNU89/Werror (including ASan/UBSan) policy regressions passed; VPN lifecycle run #45800460199 then FAILED in the Python source contract because it searched a multiline marker-set call with a single-line literal. AI handoff #45800460259 PASS; the source-checker whitespace correction is in THIS COMMIT. Process lifecycle #45800460310, WAN route #45800460260, WAN DHCP callback #45800460321 and HTTPD #45800460223 PASS; NVRAM #45800460212 and XY-C1 #45800460210 were still in progress at last poll. No L3/L4 claim.
- Parent main: `cd3d1a6ec469937f3488d073bf11c445c0aa4414`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Preserve the NoBinding watchdog when initial Request setup fails

- Follow-up review found an uncovered branch in the new NoBinding watchdog: `reestablish_ia()` still removed the existing T2 timer before constructing the first Request and returned immediately on preparation failure. The later `IAS_REQUEST` watchdog only existed after successful event creation, so a non-fatal allocation/authentication failure could still leave the old Renew/Rebind event as the sole owner; if that event exhausted retransmissions, the IA was orphaned.
- Enter `IAS_REQUEST` before preparing the event, reuse the existing IA timer or create one when absent, then arm the same watchdog for 5 seconds on setup failure or 4 minutes after successful Request creation. The 4-minute deadline still waits beyond the finite Request retransmission window; successful reply handling and duplicate-NoBinding gates remain unchanged.
- Extended the Python source contract to pin state-before-prepare and watchdog-before-return ordering. Existing GNU89 policy tests already cover the 5-second setup retry and 4-minute exchange watchdog. Exact-SHA Actions are the validation gate; no local checkout/build or hardware test is claimed.
- Parent main: `cfa384c7fe1ba6fdd87ca478a920012880d835c6`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Keep DHCPv6 IA recovery alive after a NoBinding Request exhausts

- Found that `reestablish_ia()` removed the IA timer and launched a DHCP6S_REQUEST event. Request has MRC=10 and MRT=30 seconds; if the server stayed silent, `client6_timo()` removed the finite Request event with no remaining IA timer to start another exchange. The IA stayed in Renew/Rebind recovery until prefix expiry, and infinite valid lifetimes could leave it stuck indefinitely.
- Added `IAS_REQUEST` plus a four-minute IA watchdog. This exceeds the worst-case ten jittered Request retransmission intervals; when it fires, it starts a fresh Request and is rearmed. Transient local event-construction failures retry after 5 seconds. A failed multi-IA Reply can be replayed while another IA retries: duplicate NoBinding for an IA already in Request state is ignored, and its watchdog is deferred after each local application failure to avoid duplicate active Requests.
- Added policy tests for the four-minute/5-second watchdog and duplicate-NoBinding gate, plus Python source contracts for state transition, watchdog ownership and ordering. Exact SHA `cfa384c7fe1ba6fdd87ca478a920012880d835c6`: DHCPv6 IA #45799187926 and AI handoff #45799187883 PASS. Initial-Request setup retry on `cd3d1a6ec469937f3488d073bf11c445c0aa4414`: DHCPv6 IA #45799579046 and AI handoff #45799579038 PASS; XY-C1 #45799579141 remained in progress, no L3/L4 claim.
- Parent main: `fa6281866dfedd5de66302770e6ca133c8190d90`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Retain DHCPv6 IA timer ownership until Renew/Rebind events are prepared

- Found a recovery hole in `ia_timo()`: at T2 it deleted `ia->timer` before building the Rebind event. If prefix-list or authentication allocation failed in `prepare_iaevent()`, the event was discarded and no timer remained to retry Rebind. Invalid IA states also left a live expired timer behind.
- Keep the IA timer until the Renew/Rebind event is successfully prepared. Failed Renew preparation retains the existing T2 deadline; failed Rebind preparation reschedules a bounded 5-second retry; successful Rebind preparation transfers retry responsibility to the DHCP event and removes the IA timer. Invalid-state cleanup now removes any remaining timer.
- Added policy-level GNU89 regression coverage and Python source-contract checks. The existing DHCPv6 IA workflow compiles the policy test with `-Werror`, ASan and UBSan. Exact SHA `1e34fd20be38043504f9c8afba5df2c6d79e10db`: all seven C policy binaries PASS, including timer traversal and IA ownership tests; DHCPv6 IA #45797533696 then FAILED in the Python checker because the new assertion used undefined name `policy_test` instead of existing input variable `test`. AI handoff #45797533572 PASS after log rotation. The checker binding is corrected in THIS COMMIT; XY-C1 #45797533586 was still in progress. Full firmware/hardware not claimed. Exact checker/source rerun SHA `fa6281866dfedd5de66302770e6ca133c8190d90`: DHCPv6 IA #45797751509 PASS and AI handoff #45797751700 PASS; the policy C binaries and complete Python source contract now pass. XY-C1 #45797533586 on product SHA `1e34fd20be38043504f9c8afba5df2c6d79e10db` remained in Build XY-C1, so L3 is not verified.
- Recovered the log governance contract after a concurrent milestone pushed the root log to 120 records: archived the oldest 100 complete records verbatim in `docs/dev-log/2026/11/part-038.md`, retained the newest 20 prior records, and updated the indexed blob SHA/byte count. Root now contains 21 records.
- Parent main: `8a484891c96894a5452710101a05145fab0fc7e6`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Validate one RFC 5908 NTP FQDN suboption

- The generic DHCPv6 domain-list decoder can expand multiple concatenated domain names. The RFC 5908 NTP FQDN suboption is a single FQDN, so its payload is now checked before being passed to that generic decoder.
- The wire validator requires one terminal root label, no compression or extended labels, ASCII LDH labels, no IDNA A-labels, label length <=63 octets and total wire length <=255 octets.
- Regression covers valid FQDNs and malformed/multiple/compressed/IDN/oversized inputs; the Python source contract pins the NTP parser guard.
- Parent main: `26842e306af329188b8aefd999cbbd9d94024ecd`; exact-SHA Actions pending.

---

## 2026-11-26 — Make DHCPv6 timer traversal safe against callback-side deletion

- Found a concrete iterator use-after-free: `dhcp6_timer_check()` saved `tm_next` before invoking a timer callback, but a delegated-prefix retry callback can complete cleanup and call `remove_ia()`, whose `pref_cleanup()` removes sibling prefix timers. The next loop iteration could dereference a timer node already freed by `dhcp6_timer_remove()`.
- Added per-timer check generations and a timer-list mutation generation. A callback that frees its timer or changes the timer list causes traversal to restart at the live list head; already visited timers and timers newly added during the callback are skipped until the next sweep. Recompute the next-expiry sentinel after all callbacks so rescheduling of previously visited timers is reflected.
- Added a GNU89 policy regression for saved-next removal, head insertion, generation wrap and callback invalidation; wired the source contract and sanitizer regression into the DHCPv6 IA workflow for both push and pull_request.
- Local host compile/run: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` regression PASS. Full-source and target checks await exact-SHA Actions; no L3/L4 claim.
- Exact SHA `a856ba60a4695276b5c484f7e8388fdaeaf1ced0`: DHCPv6 IA run #45795811813 FAILED only at the newly added compile invocation; all six pre-existing policy binaries passed. The workflow command contained doubled trailing backslashes, causing the linker to treat a literal backslash as an input file. This follow-up changes the compile invocation to one shell command line and needs exact-SHA rerun.
- Workflow correction parent main: `a856ba60a4695276b5c484f7e8388fdaeaf1ced0`; resulting commit: THIS COMMIT.
- Exact-SHA follow-up: DHCPv6 IA workflow #45796127157 on `c241c6a150f2c0bf8b6f2a89177780cb9766ad9f` PASSed the new ASan/UBSan timer traversal regression, all existing C policy binaries and the full Python source contract; AI handoff #45796127128 PASSed. The prior failure was limited to YAML shell escaping, not production C or regression logic.
- Parent main for the source/test implementation: `5b03226e811dbba4cba5d9219c43086b7962d6e7`.

---

## 2026-11-26 — Correct literal unlock counting in NVRAM source contract

- Corrected a double-escaped regex in the new NVRAM commit-order source check. A literal-token scan now counts both unlock sites and enforces mutex-before-snapshot plus mutex-through-flash-I/O ordering.
- No production source or regression behavior changed in this follow-up.
- Parent main: `02c27d988c9c3cdb7be180bb468da6a465633990`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Serialize NVRAM snapshot generation with flash commits

- Found a persistence lost-update race: `nvram_commit()` built its in-memory image before acquiring `nvram_sem`, so concurrent commits could write the newer snapshot first and then overwrite it with an older snapshot.
- Acquire `nvram_sem` before the snapshot spinlock/generation and hold it through flash compare/write. On image-generation failure, release the semaphore before freeing buffers.
- Added a deterministic pthread regression for the stale-snapshot and corrected orderings, plus Python source-contract assertions. The NVRAM workflow compiles/runs the regression under GNU89 with warnings as errors.
- Parent main: `d5ef589529bf19971f996265cffb92b9906c5635`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Fence DHCPv6 SIGHUP reload against pending IA cleanup

- Confirmed P1 lifetime defect: SIGHUP called `free_resources(0)` and immediately reloaded configuration. If `pref_cleanup()` failed, `remove_ia()` retained an IA and retry timer in `iac->iadata`, while `read_config6()` previously called `clear_ifconf(0)`/`clear_iaconf()` and freed objects still referenced by the timer callback.
- Added `restart_pending` fencing, blocked new client exchanges, deferred reload until both IA cleanup and outstanding RELEASE transactions drain, and handled SIGTERM during a pending restart without queuing duplicate Release events. Added a defense-in-depth `read_config6()` `EBUSY` guard before `cleanup_cflist()` and `clear_ifconf()`.
- Extended GNU89 policy regressions and the Python source contract. Corrected the DHCPv6 IA workflow pull-request filter to include modified `dhcp6c_config.h`; push filtering already covered it. No local C/Python test performed without checkout; exact-SHA Actions are the validation gate.
- Parent main: `6dbcf822bed1213baabef7639aa3f1ae68288f3f`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Enforce exactly one DHCPv6 NTP time-source suboption

- RFC 5908 requires one and only one time-source suboption within each OPTION_NTP_SERVER. The parser previously set its `source_seen` flag only for suboption codes 1–3. Because unknown future registry values were skipped without claiming the source slot, an empty container or a container with multiple unknown/known suboptions could pass framing checks without satisfying the one-source rule.
- The parser now counts every suboption code in the NTP Time Source Suboption namespace as the single source slot. Unknown future codes remain forward-compatible and their payload is skipped, but they cannot be combined with another source suboption. The decoder rejects an empty container and rejects a second known or unknown source.
- Added policy regressions for no source, one unknown future source, unknown+known, one known source and two known sources. Python source contract pins the final mandatory-source gate.
- Parent main: `734e9a86960d5f9158904a3e73280fbbae3b3842`; exact-SHA validation pending.

---

## 2026-11-26 — Enforce RFC 5908 NTP address suboption classes

- Exact-SHA DHCPv6 IA run #45792959729 on `c713dbc59c566254f736d21ff1e8b06de6f2ab43` passed all six GNU89/`-Werror`/ASan/UBSan policy binaries and the complete Python source contract; AI handoff #45792959822 also PASSed.
- Added a follow-up contract: RFC 5908 suboption 1 carries an IPv6 unicast server address, while suboption 2 carries an IPv6 multicast group address. The decoder now rejects an address whose multicast class does not match the suboption identifier. Tests cover both valid classes, both mismatches, and forward-compatible unknown suboption handling.
- Python source checks pin the `IN6_IS_ADDR_MULTICAST()` classification at the actual parser path. Parent main: `c713dbc59c566254f736d21ff1e8b06de6f2ab43`; exact-SHA validation pending.

---

## 2026-11-26 — Bound NTP parser source assertion by option case

- Exact-SHA DHCPv6 IA run #45792727216 on `5ed671f2247989d48b9420f445130aae01280c94` compiled and passed all six GNU89/`-Werror`/ASan/UBSan C policy binaries. The Python source checker alone failed because its 240-character substring ended before the continuation line containing `&optinfo->ad_list`; the production decoder and C tests were not failing.
- Replaced the brittle fixed-length window with the complete `D6_OPT_NTP_SERVER` case (up to the next option case), normalized whitespace, and pinned the parser call before the malformed-option branch.
- Parent main: `5ed671f2247989d48b9420f445130aae01280c94`; exact-SHA DHCPv6 IA rerun pending.

---

## 2026-11-26 — Correct DHCPv6 option-bounds test output assertion

- Exact-SHA IA run #45792454948 for `74a0dbccda2028b87f4fe7cd57dd0513608f8f39` compiled and passed all six GNU89/`-Werror`/ASan/UBSan policy binaries, including the newly added NTP suboption length/source tests.
- The Python checker alone failed because it still expected the pre-NTP test banner. Updated the assertion to the actual intentional output, `DHCPv6 option bounds and NTP suboption policy: PASS`. No production C or test behavior changed in this corrective slice.
- Same SHA: AI handoff #45792454964 PASS, DHCPv6 script retry #45792455152 PASS, Process inventory #45792454913 PASS. HTTPD/NVRAM/USB and XY-C1 #45792454987 were still running at last poll.
- Parent main: `74a0dbccda2028b87f4fe7cd57dd0513608f8f39`; rerun Actions required.

---

## 2026-11-26 — Parse RFC 5908 DHCPv6 NTP Server suboptions

- Confirmed standards mismatch: `D6_OPT_NTP_SERVER` (option 56) was parsed in the same branch as flat IPv6-address arrays, requiring its payload length to be a multiple of 16 and treating each 16-byte chunk as an address. RFC 5908 defines option 56 as a container of 4-byte (code/length) suboption headers, with time-source forms for a unicast address, multicast address, or FQDN; each option instance carries exactly one time source. Consequently, a normal address suboption uses a 20-byte payload (4-byte header + 16-byte address) and was incorrectly rejected, while FQDN and multicast suboptions were not decoded.
- Added a bounds-safe suboption decoder. Address and multicast-address suboptions require exactly 16 payload bytes; FQDN requires nonzero payload and uses the existing RFC 3397 domain decoder when enabled. Duplicate known time-source suboptions are rejected; unknown future suboptions are skipped. The old RFC 4075 `D6_OPT_SNTP_SERVERS` remains on its flat IPv6-list path. The option typelen entry is now variable-length and dispatches to this dedicated parser.
- Added unit coverage for truncated headers, length overruns, valid and invalid address/FQDN lengths, unknown suboptions and duplicate source rejection; extended Python source contracts to pin the production decoder and legacy SNTP separation. The existing DHCPv6 IA workflow already compiles the extended policy test and checks `dhcp6c_common.c`.
- Parent main: `fb01f7aa71477bc1dfc895bb81d368e6c69031e7`; Actions validation pending. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-26 — Normalize shutdown source contract whitespace

- Exact-SHA DHCPv6 IA run #45791468143 on `aa7181cdfdf98d283e0126db0726d0daa51fdc9d` compiled and passed all six GNU89/`-Werror`/ASan/UBSan policy tests, including IA-only, event-only, combined pending-work and drained shutdown cases.
- The Python source contract failed only because it compared a wrapped two-line C call against a single-line string. Changed the check to strip whitespace from the extracted function before matching the call. No production source or C test changed in this correction.
- Parent main: `aa7181cdfdf98d283e0126db0726d0daa51fdc9d`; exact-SHA rerun pending.

---

## 2026-11-26 — Preserve DHCPv6 RELEASE exchanges during graceful shutdown

- Confirmed follow-on bug in the SIGTERM drain: `release_all_ia()` calls `prepare_iaevent(ia, DHCP6S_RELEASE)` before removing each IA. `free_resources(0)` intentionally keeps Release events so the client can transmit and wait for Reply/timeout. The shutdown readiness check only examined `iac->iadata`; after successful local cleanup that queue became empty while Release events were still outstanding, causing `check_exit(1)` to discard the Release exchanges.
- Shutdown readiness now requires both all configured IA data queues and the global event list to be empty. This waits for each preserved Release transaction to complete by Reply or retransmission timeout, while still keeping failed delegated-prefix cleanup IAs pending through retry timers.
- Expanded the GNU89 policy test for IA-only, event-only, combined pending work and fully drained state. Python source contract pins Release event creation, `free_resources(0)` preservation, and the two-part exit gate.
- Parent main: `b3cfaab82ee2dfbe1b5982f9cc127b778ddb3c4e`; exact-SHA Actions pending after commit. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-26 — Reject malformed DHCPv6 Reconfigure Accept payload

- DHCPv6 Reconfigure Accept (option 20) is defined with no payload. The outer option parser previously silently ignored this known option after applying only the option table minimum length (zero), so it accepted malformed non-empty instances.
- Added a production zero-payload check before ignoring the unsupported feature. The existing GNU89/ASan/UBSan option policy regression now covers zero, one and maximum-size payload lengths; the Python source contract pins the actual `D6_OPT_RECONF_ACCEPT` parser branch.
- The existing DHCPv6 IA workflow path filters and test step already cover every changed source/policy/test/checker file. Exact-head validation pending after commit; no local repo checkout or full build is available.
- Parent main: `d739b7f409e57e9b7bbdfa85b7abfacbd8e5804d`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Exact-SHA DHCPv6 shutdown policy and source contract PASS

- DHCPv6 IA workflow #45790398561 on exact source/checker SHA `78205c10fb71cfb962917e65261fa499c0548573` PASSed: all GNU89/`-Werror`/ASan/UBSan C policy regressions and the complete Python source contract passed. AI handoff workflow #45790398455 also PASSed.
- WAN DHCP callback workflow #45790261953 on exact workflow/source SHA `48bec7b6ac28ea93394467d6814023884eadfa1c` PASSed after inline lock-order assertions were updated for the intentional 16-second non-forced client stop.
- XY-C1 firmware run #45789833788 remains in progress on product-source SHA `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5`: Build is running; TRX verification and artifact upload are pending. Hardware L4 is not tested.
- Main parent for this handoff-only update: `78205c10fb71cfb962917e65261fa499c0548573`.

---

## 2026-11-26 — Update remaining DHCPv6 stop-fencing source assertion

- The #8d2bf19 DHCPv6 IA rerun passed all C policy binaries but the Python checker still contained an older callback-stop assertion searching for `stop_status = kill_services(svcs, 3, 1);`. The new non-forced 16-second stop made that substring stale; it failed with `ValueError: substring not found` before subsequent source checks.
- Updated the existing callback-generation fencing assertion to the new stop contract. The WAN callback inline checks were separately updated in #48bec7b to expect 16 seconds without SIGKILL escalation. Re-run the exact-SHA IA checker before claiming the contract passes.
- Parent main: `48bec7b6ac28ea93394467d6814023884eadfa1c`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Align WAN DHCP source contract with graceful DHCPv6 stop

- Exact-SHA WAN DHCP callback run #45789833759 for `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5` passed all compiled policy tests but failed its final inline Python source contract with `ValueError: substring not found`. Two assertions still searched for `kill_services(svcs, 3, 1)` after the DHCPv6 stop was deliberately changed to `kill_services(svcs, 16, 0)`.
- Updated both the lock-order and stop-generation inline assertions to recognize the 16-second non-forced stop. This tests the new fail-closed contract while retaining the assertion that callback generation advances only after successful process exit.
- Parent main: `8d2bf194cdbc065eae75318ba01cbe36c04e2356`; resulting commit: THIS COMMIT. Exact-SHA WAN DHCP callback run pending.

---

## 2026-11-26 — Fix DHCPv6 shutdown source-check ordering

- Exact-SHA DHCPv6 IA workflow #45789833785 for `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5` compiled and passed all six GNU89/`-Werror`/ASan/UBSan policy binaries, including the new shutdown-state regression. The Python source checker then failed because the newly added stop assertion referenced `wan6` before the existing `wan6 = read(...)` assignment later in the script.
- Moved the two stop-timeout assertions below that existing assignment. No firmware/runtime source changed in this corrective slice. Exact-SHA checker result is pending; do not claim this source contract has passed until the next workflow run succeeds.
- Parent main: `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Drain delegated-prefix cleanup before DHCPv6 client exit

- Confirmed shutdown defect: `remove_iaprefix()` retains failed downstream prefix removals and schedules a 5-second retry, but SIGTERM entered `check_exit(1)`; `free_resources(1)` removed those timers and exited. The WAN stop caller used `kill_services(svcs, 3, 1)`, which could SIGKILL the process before the first cleanup retry. A transient kernel deletion failure could strand a delegated prefix without a retry owner.
- SIGTERM/SIGQUIT now set a shutdown flag and call `free_resources(0)`. The main loop services pending IA timers and exits only when every configured IA data queue is empty. `client6_start()` suppresses new Solicit/Information-Request events after shutdown, and SIGHUP no longer restarts the client once shutdown begins.
- Normal stop and failed-start cleanup now use a 16-second non-forced wait. Persistent cleanup errors time out instead of escalating to SIGKILL; normal stop therefore leaves callback generation unchanged and WAN teardown fail-closed. The timeout covers multiple 5-second retry intervals.
- Added policy unit cases and source contracts for IA-drain readiness, restart suppression, timer-drain ordering and the non-forced stop contract. Exact-SHA CI is required; hardware behavior is not verified.
- Parent main: `f55521e7bc74691d18d4a6f62a6f9e326730b970`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Reject duplicate DHCPv6 singleton vbuf options

- The outer DHCPv6 option decoder stores CLIENTID, SERVERID, RELAY_MSG and INTERFACE_ID in one owned `dhcp6_vbuf` field per option type. Repeated instances previously overwrote the prior allocation and silently kept the last value, leaking memory until the packet cleanup path.
- Added a shared unset-vbuf predicate and reject each duplicate before calling `dhcp6_vbuf_copy()` or `xmemdup()`. The existing malformed-option failure path clears the partially populated `dhcp6_optinfo`, releasing the first allocation. Added four policy boundary cases; the Python source contract pins all four production guards and the cleanup path.
- `dhcp6c-ia-publication-stability.yml` already compiles the policy test and runs the checker, with push/PR path filters for every changed source/test/checker file. Exact-head validation pending after commit.
- No local checkout/full original test suite is available because the container cannot resolve github.com; no local build or device test is claimed.
- Parent main: `9ac2adf871a85e91a2a05518e47aba18c99df9be`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — Preserve first error across DHCPv6 delegated-prefix cleanup

- `remove_iaprefix()` iterates every delegated downstream interface and correctly attempts all removals, retaining failed entries for retry. However, each failed `ifaddrconf(IFADDRCONF_REMOVE)` overwrote `ret`; multiple failures therefore returned the last errno, contrary to the existing first-error aggregation policy used elsewhere in DHCPv6 IA.
- Routed each failed delete through `dhcp6c_ia_first_error()`, without changing retry or per-interface cleanup behavior. The host regression now drives three independent failed downstream deletions (`EBUSY`, `EINVAL`, `EIO`) and proves that all are attempted, all remain pending, and the first error is returned. The Python source contract pins the production call and the new regression.
- The existing DHCPv6 IA workflow compiles this test with GNU89/`-Werror`/ASan/UBSan and runs the source checker; exact new-SHA results are pending until Actions conclude.
- Parent main: `21d71c59e186de568492b0f31e4518396ed63b35`; resulting commit: THIS COMMIT.

---

### 2026-11-26 — autonomous stability continuation

Handoff HEAD: `1e3154270732eef0314244dec117dd325971638a`. Frozen backup ref remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

Implemented source/test slices in Samba paired-transaction durability, Ralink IAPP event-table/TCP deadline safety, DNSMasq staged publication and failed-start rollback, and Inadyn last-known-good config retention through startup confirmation. Inadyn follow-up covers same-inode hard-link cleanup on rollback before publication. See the corresponding commit history and exact-SHA Actions in `SESSION_HANDOFF.md`; this log does not treat a pending check as a pass.

Current first follow-up: verify the exact-SHA Inadyn workflow status for the latest snapshot rollback changes; fix any test or build regression before moving on.
