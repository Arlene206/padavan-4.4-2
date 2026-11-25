## 2026-11-25 — Prevent SIGPIPE from terminating IAPP TCP sender

- `IAPP_TCP_WriteFrame()` now uses `MSG_NOSIGNAL` when the target socket API exposes it. A remote close/reset during `send()` therefore becomes a normal send error handled by the helper instead of delivering SIGPIPE and potentially terminating the daemon. The existing zero/error handling remains fail-closed; platforms without this flag retain their native fallback.
- Added a lifecycle source assertion for the conditional no-SIGPIPE send flag.

---
## 2026-11-25 — Keep receive-length validation active without FT encryption

- Follow-up review found TCP/UDP `ReceivedLength` was recorded unconditionally, but its corresponding decrypted-length guard sat inside `#ifdef FT_KDP_FUNC_PKT_ENCRYPT`. In builds without that feature macro, this left a set-but-unused local and skipped the generic returned-length/capacity invariant. Moved both guards immediately after the optional decrypt block so raw and decrypted paths always require a nonnegative length no larger than the bytes received.

---
## 2026-11-25 — Validate every IAPP encryption ioctl result

- Audit found transmit encryption IOCTLs (except the UDP call's failure branch) ignored failure and trusted the mutable returned length before TCP/UDP/raw-RRB sends. A failed ioctl could leak plaintext; a negative or over-capacity output length could transmit stale bytes beyond the reserved packet tail.
- Added `IAPP_EncryptFrame()`: it holds the driver output in a signed `INT32`, rejects ioctl failure/negative lengths, and requires the encrypted output to fit the caller's reserved `IAPP_SECURITY_EXTEND_LEN` tail before updating the send length. Routed UDP, generic TCP, FT security-block send/ACK, AP-information response, and raw RRB encryption through it.
- Added GNU89 sanitizer policy cases for negative, exact-capacity and over-capacity driver lengths. Lifecycle source contract requires all six transmit encryption paths to use the wrapper.

---
## 2026-11-25 — Bound driver-returned receive and signal-context lengths

- TCP/UDP decrypt ioctls update the in/out `SizeRcvMsg`. The dispatcher then trusted that returned value as if it were still the number of bytes actually received, so a driver-reported length larger than the input frame could bypass all subsequent parser bounds. Both paths now retain the pre-ioctl receive length and reject negative or larger decrypted lengths before reading the IAPP header.
- The non-raw-socket `IAPP_USR2Handle()` allocated exactly one `RT_SIGNAL_STRUC`, ignored ioctl failure, then used the returned `DataLen` to read from that allocation into a pipe. It now rejects ioctl failure and negative/over-capacity returned lengths, and reports short pipe writes.
- Lifecycle source gates pin the TCP/UDP in-place decrypt output limits and signal-context allocation bound.

---
## 2026-11-25 — Guard TCP select descriptors before FD_SET

- The absolute-deadline wait helper passed its socket descriptor to `FD_SET()` without checking `FD_SETSIZE`. `FD_SET` operates on a fixed-size `fd_set`; an unexpectedly high descriptor can write beyond that object. The helper now rejects negative or out-of-range descriptors before touching the sets, and the lifecycle source gate pins the check.

---
## 2026-11-25 — Validate the raw RRB decryptor's returned length

- The raw RRB path now checks the rounded encrypted input span before ioctl, but then trusted the in/out `SizeRcvMsg` returned by the driver decrypt ioctl and added the fixed header before allocating/copying it. A negative or unexpectedly large returned length could make the later copy exceed the received frame buffer.
- After a successful decrypt, reject negative lengths and require the plaintext length to fit the actual received frame bytes after `FT_RRB_HEADER_SIZE` before adding the header or copying into the OID request.
- Lifecycle source contract now pins both pre-decrypt encrypted-span and post-decrypt plaintext-span checks.

---
## 2026-11-25 — Bound TCP frame receive time against slow peers

- Review of the new read-until-EOF loop found an availability regression: a peer could connect, send a prefix, and keep the stream open indefinitely, blocking the IAPP event loop. Added an absolute five-second frame deadline with `select()` before each read (including the overflow probe); EINTR is retried without extending the deadline.
- The frame reader still accumulates fragmented reads, rejects data beyond `IAPP_MAX_RCV_PKT_SIZE`, and treats EOF as the end of this one-frame connection. The lifecycle contract pins the deadline constant and wait-before-read path.

---
## 2026-11-25 — Require exact IAPP protocol frame lengths

- TCP and UDP dispatchers previously checked only the fixed IAPP header minimum and trusted its 16-bit `Length` field independently of actual post-decrypt bytes. This allowed packets with truncated or trailing data to reach command handlers when the command-specific payload check did not reject them.
- Added shared equality policy and require `SWAP_16(RT_IAPP_HEADER.Length) == SizeRcvMsg` in both dispatchers before command dispatch. The optional IAPP_EVENT_LOG security monitor now requires the full `RT_IAPP_SECURITY_MONITOR` struct before reading its event table.
- Added exact/short/long header-length unit cases and lifecycle source contracts for TCP, UDP and security-monitor checks.

---
## 2026-11-25 — Complete partial TCP writes for IAPP frames

- Four IAPP TCP paths used a single `send()` and treated any nonnegative return as a complete frame. A short write can leave the peer with a truncated encrypted IAPP packet, while the sender closes the one-frame connection immediately afterward.
- Added `IAPP_TCP_WriteFrame()` to loop until the entire frame is sent, retry EINTR, reject zero/error writes and bound each append. Replaced single-send behavior in the general TCP packet sender, FT security-block send, FT security-block ACK and AP-information response sender. Raw Ethernet/datagram sends remain single-message sends.
- The shared append-capacity policy test covers partial accumulation/exact-capacity/overflow, and lifecycle source contracts require all four TCP send paths to use the helper.

---
## 2026-11-25 — Read complete IAPP TCP frames across partial reads

- `IAPP_RcvHandlerTcp()` used a single `read()` on a TCP stream and then closed the connection. TCP may return only a prefix even when the peer issued one `send()`, so valid frames could be dropped or parsed incomplete.
- Added `IAPP_TCP_ReadFrame()`: collect until EOF (the repository's TCP sender creates one connection per frame and closes after its send), retry EINTR, enforce receive-buffer capacity and probe one extra byte to reject over-capacity frames rather than silently parse a truncated prefix.
- Added overflow-safe append-capacity policy and GNU89 ASan/UBSan tests for fragmented accumulation, exact capacity, over-capacity and SIZE_MAX. Lifecycle shell pins EOF/retry/capacity behavior.

---
## 2026-11-25 — Reject truncated fixed IAPP ADD/MOVE notifications

- UDP ADD_NOTIFY checked only the protocol-declared IAPP Length before reading AddressLen/MAC/Rsvd; MOVE_NOTIFY did the same and had no actual receive-length input. A short datagram with a forged fixed-length header could trigger reads beyond received bytes.
- TCP and UDP MOVE_NOTIFY dispatches now pass actual post-decrypt receive length. Its handler requires exact `sizeof(RT_IAPP_MOVE_NOTIFY)`, matching IAPP Length, AddressLen == ETH_ALEN, and zero context length before any field read that drives state changes. UDP ADD_NOTIFY applies equivalent actual-size, declared-size and AddressLen checks before reading its MAC/Rsvd.
- IAPP lifecycle source contracts pin both actual-frame-size guards. Exact-head Actions pending.

---
## 2026-11-25 — Bound raw driver signal event payloads

- `IAPP_RcvHandlerRawDrv()` copied the destination MAC from any positive-length raw frame and formed a signal pointer after the 14-byte Ethernet prefix; `IAPP_SIG_Process()` then read `RT_SIGNAL_STRUC` fields and an 8-byte `FT_KDP_EVT_HEADER` without using its `Len` parameter. EventLen subsequently drives key/action/neighbor data access, and debug-trace signals read an additional 32-bit value.
- Raw driver now rejects frames shorter than Ethernet + the fixed target signal prefix + event header before MAC copy/pointer dispatch. It passes only the post-Ethernet signal byte count. `IAPP_SIG_Process()` validates the fixed signal/event prefix, ensures EventLen fits actual received bytes using the subtraction-based policy helper, and requires a full INT32 for debug-level events.
- Added GNU89 ASan/UBSan policy boundary cases for empty/exact/truncated signal event payloads. Exact-head CI pending.

---
## 2026-11-25 — Validate raw IAPP RRB frame and encrypted payload lengths

- `IAPP_RcvHandlerRawRRB()` read the Ethernet DA and `FTActionLength` from any positive-length datagram, then rounded the declared action length for the encryption ioctl without checking that the padded encrypted span existed in the received frame. A truncated packet could therefore cause out-of-bounds reads in daemon/driver decrypt handling.
- The handler now requires the complete fixed `FT_RRB_FRAME` header before any field access, saves the physical receive length, preserves the documented Ethernet minimum-frame zero padding behavior, and verifies the rounded encrypted payload fits in the bytes actually received after the RRB prefix before issuing the decrypt ioctl.
- Added regression boundaries for a 60-byte minimum Ethernet frame (24-byte RRB header + 36-byte available payload) and a 64-byte frame. Exact-head IAPP and XY-C1 Actions pending.

---
## 2026-11-25 — Bound IAPP AP-information payload copies

- `IAPP_RcvHandlerApInfor()` trusted its 16-bit Information Block Length and copied that many bytes from the received packet without knowing the packet's actual size. This could read beyond a short TCP/UDP receive buffer. TCP INFO_RESPONSE and UDP INFO_BROADCAST/INFO_REQUEST now pass the post-decrypt receive length; the handler verifies its fixed prefix and validates the declared payload span before command-buffer sizing or copying.
- The shared helper uses subtraction after checking the fixed prefix, preventing `size_t` addition wrap. The IAPP lifecycle source contract asserts the AP-info length guard precedes its payload copy. Exact-head IAPP/firmware checks pending.

---
## 2026-11-25 — Repair GNU89 receive-length test dependency

- Exact IAPP lifecycle run #45691011094 on d444b0e8c2347bf70f5cafec9b05aee08a6e6ffd FAILED before running the policy tests: the new SIZE_MAX boundary assertions lacked their required <stdint.h> include. This was a test-fixture compile error; production receive guards were not exercised by that job. Add the direct standard header include so the test does not depend on transitive definitions.
- d444 exact Actions at correction time: AI handoff #45691011016 PASS; XY-C1 #45691011111, Process lifecycle #45691011010, process lifecycle regression #45691011103 and NVRAM #45691010984 pending. The previous IAPP failure is corrected here; rerun its workflow on this descendant.

---
## 2026-11-25 — Bound IAPP FT security-block copies by received bytes

- IAPP_RcvHandlerSSB() copied attacker-controlled pSendSB->Length bytes into fixed-size stack object FT_KDP_EVT_KEY_ELM kdp_info; an oversized field could overwrite the stack. The handler now validates post-decrypt received bytes, checks the declared payload span without size_t addition overflow, and requires the exact fixed key-element size before either copy.
- TCP and UDP dispatchers pass their actual receive length to the SSB handler and reject buffers shorter than the fixed IAPP header before reading Version/Command. The TCP ACK-security-block path checks its fixed prefix and payload length before copying.
- Added an overflow-safe receive-span policy and boundary regression (exact, truncated, oversized, zero payload and SIZE_MAX). IAPP lifecycle now compiles the combined policy test as GNU89 with ASan/UBSan; shell source contracts pin both short-header gates and stack-copy guard. Exact-head Actions pending.

---
## 2026-11-25 — Prevent stale FT-client hash links on slot reuse

- Fixed a concrete IAPP FT-client table corruption path. Deletion unlinked an entry from its hash bucket but left its `next` pointer intact; when that fixed-array slot was reused in another MAC-hash bucket, the stale pointer could splice the old bucket's tail into the new chain. Insertion also assumed a free slot existed after checking only the table count, then incremented the count even if no unused slot was found.
- Added a bounded hash-chain policy: free-slot selection clears stale links, append refuses duplicates/full/cyclic chains, and unlink detaches the released entry. Production insertion checks slot/append results before publishing the used flag and table count; deletion unlinks before clearing the slot and preserves state if unlink fails. Lookup, insert and delete reject null table/key pointers.
- Added compiled regressions for middle/head/tail unlink, cross-bucket slot reuse, stale-link clearing, full table, duplicate append and cyclic-chain bounds. IAPP lifecycle source contracts enforce helper use and unlink-before-release order; the workflow now watches the utility C source and policy header. Exact-head IAPP and XY-C1 Actions pending.

---

## 2026-11-25 — Keep the IAPP UDP BSS guard C89-buildable

- Static review of 4cd7ec6a17fd5150ac103495e9a1ace73760e365 found the new BSS guard had repeated all local declarations after the guard statement. This is a same-scope redeclaration; the fix keeps the original declarations at function entry and removes the duplicate block. Corrected the new diagnostic's escaped newline.
- Tightened iapp_wifi_index_valid() to reject a configured interface count larger than the fixed array capacity, not merely an invalid requested index. Added a regression for an over-capacity count.
- Host GNU89/ASan/UBSan helper test is to be rerun on this revision. On 4cd7, IAPP workflow #45689557162 PASS, Process inventory #45689557189 PASS, AI handoff #45689557186 PASS; Process lifecycle #45689557170, XY-C1 #45689557176 and NVRAM #45689557329 were pending. The IAPP shell gate does not compile rtmpiapp.c; the exact-SHA XY-C1 cross-build remains the production compiler gate. No new firmware result is claimed.

---
## 2026-11-25 — Validate IAPP BSS index before UDP MAC lookup

- A second fixed-array access remained after the initial ioctl-index fix: IAPP_UDP_PacketSend() copied `IfNameWlanMAC[if_idx]` into every outgoing broadcast frame without checking `if_idx`. The index comes from STA/interface lookup paths; an unexpected negative or out-of-range result could read outside the two-entry MAC array.
- Added a shared `iapp_wifi_index_valid()` check and reject an invalid BSS index before packet-ID mutation or any MAC-array read. The ioctl interface selector now reuses the same policy, keeping the two-slot bounds consistent.
- Extended the compiled C policy regression for indices -1, 0, 1, 2, 255 and configured counts 0/1/2; the shell source contract asserts the UDP guard precedes the MAC-array access. Exact-head IAPP and XY-C1 Actions pending.

---
## 2026-11-25 — Bound all IAPP command-line interface names

- The exact-head IAPP regression on `c89a47e69fbdec2edc43a36bd01de458f36ec122` FAILED in the newly added C fixture only: `char too_long[2][16]` was initialized with a 24-byte string under `-Werror`. The production index-bound source contract ran before the compiler stopped. The fixture now uses a 32-byte slot while still testing fallback against the 12-byte ioctl destination.
- Extended the same input hardening to the `-e` Ethernet and `-w` wireless parser options, whose fixed IFNAMSIZ buffers were also populated with unchecked `strcpy`. A generic length predicate now rejects empty/overlong names before each copy; the `-wi` path retains count/capacity validation.
- Added source-order checks for all three parser options and tests for the generic name-length boundary. Also normalized the -wi diagnostic to a normal C newline escape. Exact-head IAPP/XY-C1 Actions pending.

---
## 2026-11-25 — Bound Ralink IAPP wireless interface indices

- Fixed a concrete out-of-bounds read/write in the Ralink IAPP daemon. `MAX_WIFI_COUNT` is 2, but `IAPP_IoctlToWLAN()` dereferenced `IfNameWlanIoctl[ApIdx]` before testing the index and used `ApIdx > MAX_WIFI_COUNT`, allowing index 2 (and larger values) to access beyond the two-slot array. The `-wi` parser also incremented `IfNameWlanCount` and wrote into the array without checking capacity or interface-name length.
- Added a bounded interface-name policy: the `-wi` parser validates count and nonempty NUL-terminated argument length before storing; ioctl selection validates configured count and index before pointer arithmetic, bounds the NUL search to the slot, and falls back for invalid/empty/too-long slots.
- Added a compiled C regression for valid slots, index 2/255, negative/unconfigured indices, invalid counts, empty/unterminated/oversized names and parser capacity. The IAPP lifecycle suite enforces parser/selector source ordering and runs the new C test. Exact-head IAPP and XY-C1 Actions pending.

---
## 2026-11-25 — Make the IAPP TCP listener restart-safe after TIME_WAIT

- The Ralink IAPP TCP listener bound port 3517 without SO_REUSEADDR. A graceful stop after accepted TCP peers can leave the server endpoint in TIME_WAIT; an immediate restart then fails bind with EADDRINUSE. A local Linux socket reproduction confirmed the replacement listener must set SO_REUSEADDR and the previous listener must also have had it enabled.
- Set SO_REUSEADDR on SocketTcpRcv before bind(), fail closed through the existing socket cleanup if setsockopt fails, and add a host C regression that creates an accepted connection, actively closes the server endpoint and immediately rebinds the same port. The IAPP shell suite also pins option-before-bind ordering and failure cleanup.
- The IAPP workflow now runs the compiled TIME_WAIT regression and watches its new test source. The XY-C1 workflow already watches the full production src subtree. Exact-head Actions pending; no router result claimed.

---
## 2026-11-25 — Include Ralink IAPP C sources in XY-C1 build triggers

- Exact-SHA review found a gap in xy-c1-firmware-validation.yml: push and pull_request path filters included the IAPP lifecycle test and shell wrapper but omitted trunk/user/ralinkiappd/src/**. The preceding IAPP C listener fix therefore did not itself trigger firmware validation; a host-test path in the same earlier commit happened to trigger a build.
- Added the entire IAPP src subtree to both path filters and a unit regression that pins both entries and verifies the C sources/Makefile are production inputs to the build detector.
- The source-input detector already treats unknown production paths as firmware inputs, so no detector logic change was needed. Changing this workflow itself intentionally forces an exact-head firmware validation run.

---
## 2026-11-25 — Fail IAPP startup when TCP listen() fails

- IAPP_SocketOpen() checked socket creation and bind(), but ignored listen()'s return value. A listener setup failure could leave the daemon loop running and be reported healthy even though incoming TCP security-block connections could not be accepted.
- The TCP listen result is now checked; on failure the function logs the error and follows its existing label_fail cleanup, closing all partially opened sockets and preventing the child from entering IAPP_Start(). The diagnostic uses the normal C newline escape.
- Added an IAPP source-contract regression for the checked call, failure log and cleanup branch; the workflow now triggers when this production C source changes. Exact-head Actions pending; no firmware or hardware result claimed.

---
## 2026-11-25 — Match Ralink IAPP firewall rules by exact xtables identity

- Found a firewall readiness false-positive in iapp_ensure_firewall_rule(): substring filters accepted a TCP rule for port 35170 with comment ralinkiappd-tcp-extra as the required TCP/3517 rule. The previous check also tolerated duplicate owned tags because one matching line was sufficient.
- Replaced substring acceptance with iptables -C over the complete protocol/interface/port/comment/target tuple. Saved-rule tag counts now reject duplicate/conflicting tags, and newly added rules are checked again with xtables plus a fresh inventory. Query errors remain distinct from a genuinely absent rule.
- Added lifecycle regressions for port/tag substring collisions, duplicate exact rules and xtables query failure. The existing IAPP workflow already tracks both changed paths. Exact-head Actions pending; no target-router result claimed.

---
## 2026-11-25 — Gate FTP listing parent opens on READ ACL

- After the first directory-filter probe was gated, the fallback branch still opened the derived parent directory with str_opendir(&s_dir_name_str) before checking its share ACL. A filtered file path under an unreadable share could therefore still reach a pre-authorization parent lookup.
- Added the same READ ACL precheck before the fallback parent open. The descriptor-bound ACL/identity revalidation remains after open. The VSFTPD source contract now independently pins both prechecks before their respective opendir calls and the opened-FD check afterward.
- This is a follow-up to the preceding filter-probe slice; no local test, MIPS build, firmware artifact, or hardware result is claimed. Exact-head VSFTPD and AI handoff Actions must validate the complete current source.

---
## 2026-11-25 — Check FTP directory READ ACL before path probing

- handle_dir_common() previously called str_opendir(&s_filter_str) to distinguish a directory filter from a filename before checking the share READ ACL. The opened directory was denied later, but the pre-authorization path lookup could reveal whether a candidate path resolves as a directory through different FTP outcomes.
- Added asus_check_permission_directory() before that path-based open. The existing descriptor-bound asus_check_permission_open_directory() remains after open, preserving protection against path replacement between precheck and use. The VSFTPD workflow source contract now enforces precheck → path open → opened-FD ACL verification.
- Production source and workflow regression are included in this atomic commit. No local host test, MIPS build, firmware artifact, or hardware result is claimed. Exact-head Actions are the required validation.

---
## 2026-11-25 — Repair VSFTPD and storage workflow source contracts

- Exact VSFTPD run #45683682240 on f356cfad passed its C regression stage but the Python contract then failed at line 679 because five old assertions still required the removed path-based stat()/HIDE_PARTITION_FILE implementation. Replaced them with the descriptor-relative marker wrapper and behavioral fixture tokens for empty file, FIFO, directory, symlink and dangling symlink.
- Exact USB storage run #45683527825 on dc4ad757 passed earlier test/build contracts, then the source checker raised NameError at line 162: `pr_filters` was undefined while the script had defined `pull_filters`. Corrected the identifier; the storage workflow path filter includes this checker.
- XY-C1 run #45683527955 on dc4ad757 ended workflow-success but its job summary explicitly says Build/TRX/artifact were intentionally skipped because production inputs were unchanged. This is not a new L3 validation claim.

---

## 2026-11-25 — Run VSFTPD hidden-marker contract after function helper setup

- The VSFTPD C regression compilation/execution step completed, then the Python source-contract step failed with NameError because the new hidden-marker assertions were placed above the workflow's function_body() helper definition.
- Moved the five assertions below the helper fixtures; no production or test source behavior changed. Re-run the exact-head VSFTPD workflow before closing this change.

---

## 2026-11-25 — Fail closed on malformed VSFTPD hidden-share markers

- Found a fail-open in test_if_hidden_share(): pathname stat() treated the mount as hidden only when the followed target was a regular file. A FIFO, directory, dangling symlink, symlink loop or other lookup error made the protected marker look absent.
- Added a descriptor-relative policy check that pins and revalidates the share directory, inspects the .hidden dentry with fstatat(AT_SYMLINK_NOFOLLOW), treats every present entry as a hide marker, and regards only ENOENT as visible. This avoids following marker symlinks or opening special files.
- Host regression fixtures cover absent marker, empty regular marker, FIFO, directory, symlink to a regular target and dangling symlink. Exact-head VSFTPD/AI/Process/NVRAM checks will be verified after this atomic source/test/checker commit.

---

## 2026-11-25 — Exercise embedded-NUL rejection in Firefly INI input

- Added a binary-input regression for a physical INI record containing an embedded NUL followed by an apparent `admin_pw` directive. The publisher must return EINVAL, preserve every input byte in the active config and remove the staged temporary file.
- The existing overlong physical-record regression continues to cover EOVERFLOW and identical last-good preservation. The CI source contract now requires both behavioral fixtures.
- The parent commit's exact-head Firefly, AI Docs, Process inventory and Process lifecycle Actions passed; NVRAM audit and XY-C1 Build/TRX/artifact workflow were still in progress at the time of checkpoint. This test-only follow-up requires its own exact-SHA validation.

---

## 2026-11-25 — Reject split Firefly INI records before publication

- Reviewed the exact-head Firefly regression failure chain: the C sanitizer binary passed; earlier Python failures came from stale source-function and fixture-string assertions, corrected in two follow-up checker commits. The exact Firefly workflow then passed on 4e8a18f8f8cecc054a37267eea53b965b0cb3cbd.
- Found a separate parser defect in firefly_config_write_existing(): a 512-byte fgets() buffer could split one long physical INI record. If its first chunk matched a rewritten key (for example port =), the continuation was written after the normalized newline and became a separate configuration directive.
- Added a bounded physical-line reader with a 4095-byte maximum; oversized records return EOVERFLOW, embedded NUL returns EINVAL, and read errors abort staged publication. The regression asserts that the prior config remains byte-identical and no temporary file leaks.
- Standalone GNU89 -Wall -Wextra -Werror ASan/UBSan parser regression passed. Full repository Firefly workflow, source contract and exact-SHA Actions are pending on this commit.

---

## 2026-11-25 — Fail closed on modem driver unload postcondition

- Source fix: `f561a44e1332101ea6e8f42f91b3399134b2d138` made the unload callback verify that a module disappeared from `/sys/module`; a legacy unload helper success with a held reference is not treated as proof of removal. `6d75c4f67f39fb95fbe5626ef97a50b2145f2a54` explicitly classifies the remaining-loaded state as `EBUSY` and preserves a concrete unload errno. This blocks NDIS/RAS family load, QMI client-marker deletion and cache publication after an incomplete unload.
- `2f29cde4dde752472cd83dd496a9a44376a9befa` / `76d78219e70a4f211b40089eb28efc58ffcd9e04` add removal accounting so the one-second settle delay occurs only if at least one previously loaded module is confirmed removed. The host policy regression covers no-op absent modules, actual removal counts, busy/unknown/error outcomes and invalid groups.
- Test-only follow-up `87406ad733a35d505823373addf3a30ea7c534b1` first exposed an uninitialized test counter; `ed0c1d9342bbe38469926c30145df389a5bc9076` initializes it for each fixture. Exact USB modem workflow #45677107277 PASS on ed0c1d9. The source remains in the current main lineage.
- Related exact-head checks: ZeroTier #45677056983 PASS on `92ebe37...`; on `cdd7e45...` Process inventory #45677711700, Process fail-closed #45677711853, HTTPD #45677711707, WAN DHCP #45677711661, Firewall #45677711875, DNSMasq #45677712025, USB storage #45677711693 and NVRAM #45677711890 PASS. WAN route #45677711691 failed at a stale checker assertion and was corrected: #45678120958 PASS on `6dbd63d...`; AI docs #45678121059 PASS.
- XY-C1 #45677711638 on cdd7e45 is still in progress at Build XY-C1; the 6dbd63d checker-only descendant #45678120929 passed host checks but skipped Build/TRX/artifact. No current L3 claim; physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Retry STOU after a concurrent exclusive-create collision

- Base SHA: `b7828a9da9ca5f15fc513f7806efcf46aebb4daf`; atomic source commit: `ada6ebecc055d781200fa6621b2a2b60a4777889`.
- The authorized/no-follow candidate preflight and final `openat(O_CREAT | O_EXCL)` prevent symlink and overwrite races, but a concurrent STOU could create the same candidate between those steps. The final open then returned `EEXIST` and the transfer failed immediately instead of selecting the next free suffix.
- Added a bounded retry policy (16 retries) that treats only `EEXIST` as a name collision. On retry the original request is reprocessed, so deny-file checks, ACL authorization, pinned-parent verification, and nofollow candidate probing run again. Permission/path/I/O errors are not retried and the captured errno is restored before reporting failure. Non-STOU upload behavior is unchanged.
- Extended `ftp_acl_path_policy.c` to assert duplicate exclusive open returns `EEXIST`, cleanup ownership remains false for the duplicate, retry is permitted only below the bound, and successful/non-`EEXIST` results do not retry. The VSFTPD workflow tracks the new policy header on push and pull_request; XY-C1 firmware path filtering also tracks it.
- Exact-SHA VSFTPD #45673063824 PASS (C regression and source contract); AI docs #45673063860 PASS; Process inventory #45673063812 PASS; Process lifecycle #45673063825 PASS; NVRAM audit #45673063811 PASS. XY-C1 #45673063864 was still at Build XY-C1 when recorded; TRX/artifact pending, so L3 is not claimed. Standalone collision helper GNU89/Werror/ASan/UBSan smoke test PASS; no local full firmware build claimed.
- Adjacent WAN 6rd host-bit commit `778714965a2013c2e1febb9e884eed4dd422db23` had a checker-only failure because `sixrd_body` was undefined. Follow-up `b7828a9da9ca5f15fc513f7806efcf46aebb4daf` corrected the assertion scope; WAN DHCP callback #45672691417 PASS. Physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Reject impossible DHCPv4 OPTION_6RD prefix mappings

- Base SHA: `7404c9e0b49f7395ad90d07b9f5f2d82cb501d26`; atomic source commit: `4bbbcd391987de3e200770a865b0401e556b4b20`.
- Found that `store_ip6rd_from_dhcp_status()` range-checked each OPTION_6RD field independently, then transactionally published the tuple even when the derived delegated-prefix length exceeded 128 bits. The later tunnel setup rejected the mapping, but the invalid tuple had already replaced the previous valid NVRAM state.
- Added `wan6_sixrd_prefix_mapping_is_valid()` and call it before `wan6_publish_6rd_snapshot()`. It enforces RFC 5969's `(32 - IPv4MaskLen) + 6rdPrefixLen <= 128` bound; invalid optional DHCP options remain a no-op and preserve the previous tuple. Added GNU89 boundary tests for valid /128 and /96 edges, 129-bit overflow and out-of-range fields, plus Python source-order contracts and the CI compile/run step.
- Host C test passed with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. Exact-SHA WAN DHCP callback #45671544919 PASS, including the new C test and source checker. WAN route #45671545155, IPv6 service type #45671544995, HTTPD security #45671544917, AI docs #45671545086 and Process inventory #45671545105 PASS. NVRAM raw ioctl audit #45671544955 and XY-C1 #45671545199 were still IN_PROGRESS at last poll; XY Build/TRX/artifact not yet confirmed. Physical-router L4 NOT VERIFIED.

---

## 2026-11-26 05:02 CST (UTC+8) — Revalidate STOU candidate parent after nofollow stat

- Base SHA: `8f2f34597dd87318be2478ef58f00d3ef0ee6073`; focused VSFTPD source/workflow/handoff update.
- STOU candidate checks already resolve the authorized parent, enforce WRITE ACL and use a nofollow descriptor-relative entry stat. Added a second `ftp_acl_path_verify_directory_identity()` immediately after the candidate stat, before releasing the pinned directory FD, so a path replacement during the probe cannot be accepted as a stable parent result.
- Added a workflow source contract requiring WRITE ACL → first parent identity → nofollow stat → second parent identity ordering.
- Exact-SHA Actions and the FTP ACL C regression for this commit are pending inspection; no result is assumed. Physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Authorize STOU candidate probes before filesystem lookup

- Base SHA: `3ec6ac84a096d953b9ecda469c0c2e13f998ee5f`; source commit: `c683355e539de6e435cf8b25b749960bfcb10653`. Scope: vsftpd ACL helper/API, post-login STOU selection, shared entry-stat contract and VSFTPD workflow.
- Review found `get_unique_filename()` called path-based `str_stat()` before the upload ACL check. Because `stat` follows the final symlink, STOU could reveal whether a dangling or external symlink target existed; errors were also treated as if a name were free. Added `asus_check_upload_candidate_exists()`: deny-file filtering precedes it; it resolves and pins the parent, checks `PERM_WRITE`, performs a nofollow descriptor-relative entry stat, and revalidates parent identity after the probe. Errors fail closed, while an existing symlink (including dangling) occupies the candidate name. A suffix counter overflow returns `EOVERFLOW`; final O_EXCL creation remains the race-safe arbiter.
- Existing path-policy C regression compiles with GNU89/Werror and covers external symlink targets, dangling symlinks, real ENOENT, reserved metadata and malformed leaves. VSFTPD source checker requires ACL → pinned parent → nofollow stat → revalidation and bans `str_stat()` from the unique-name chooser.
- Exact-SHA `c683355e539de6e435cf8b25b749960bfcb10653`: VSFTPD #45666057681 PASS; Process inventory #45666057417 PASS; AI docs #45666057477 PASS. XY-C1 #45666057475 had header self-containment PASS, Build IN_PROGRESS, TRX/artifact PENDING. NVRAM #45666057562 had host regressions PASS and raw-call audit IN_PROGRESS. No L3/L4 claim.

---

## 2026-11-26 04:30 CST (UTC+8) — Close RNFR descriptor and source-contract checks

- Base SHA: `6ba2336de958c44fc214f2d50a00af0fc68adc63`; atomic follow-up to the RNFR nofollow-entry implementation.
- Exact-SHA #45664790224 on `6ba2336de958c44fc214f2d50a00af0fc68adc63` still failed only because the workflow retained one obsolete assertion ordering RNFR through `asus_check_permission_path()`; the helper signature itself was corrected. Removed that assertion and replaced it with the nofollow helper ordering.
- Revalidate the canonical parent identity after the descriptor-relative stat before releasing the pinned parent FD, so a concurrent path replacement is detected even after the entry probe.
- Added regression cases for `.__alice_var.txt`, partition `.hidden`, slash-containing leaf and dot-dot leaf. Existing tests already cover external-target and dangling symlinks and genuine ENOENT.
- Parent #45664790212 AI docs PASS. This repair commit's VSFTPD and any production-triggered XY checks must be polled by exact SHA; no build or router L4 result is assumed.

---

## 2026-11-26 03:36 CST (UTC+8) — Align VPN TAP checker with fail-closed cleanup

- Base SHA: `b8b8b366ee8eee33be0726081e9ff636f7f3644c`; this is a checker-only repair following the TAP setup rollback safety fix.
- Exact-SHA VPN lifecycle #45662893097 compiled and passed all OpenVPN/PPP/VPN C regressions, including ASan/UBSan startup rollback policy. Its final source contract still demanded the old local TAP cleanup sequence (`if (created)`, `--rmtun`) that b8 intentionally removed. Updated the ordering contract to require ownership publication before bridge/ifconfig work and absence of local bridge unlink/`--rmtun`; the outer caller remains responsible for cleanup only after two complete ABSENT process inventories.
- Exact SHA b8 AI handoff docs #45662892982, OpenVPN client credential #45662893530, Process lifecycle #45662893062, HTTPD #45662893005 and IPv6 #45662893357 PASS. VPN lifecycle #45662893097 failed only at this stale TAP expectation. XY-C1 #45662893279 and NVRAM #45662893093 were PENDING at last poll.
- Earlier production-source build #45660655080 (dc6 SHA) completed SUCCESS through Build XY-C1, TRX verification and artifact upload. It predates b8's TAP teardown change; the exact b8 XY-C1 run must complete before validating the new production source. Physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Defer TAP setup rollback until OpenVPN process absence is confirmed

- Base SHA: `7b67475e01d13139d9fb97cc9ada5a5cda9b5361`; result SHA: this commit. Scope: `vpn_openvpn.c`, `check_vpn_server_lifecycle.py`, current handoff/log.
- Review found `openvpn_tapif_start()` performed immediate local bridge removal and `--rmtun` when the subsequent `ifconfig` command failed. The outer server/client caller already routes this failure through `openvpn_start_rollback_failed()`, whose policy requires two complete process-inventory scans confirming ABSENT and only permits cleanup of resources created by the current attempt. The local rollback bypassed that mandatory gate and contradicted the adjacent safety comment.
- Removed the local destructive cleanup. The helper still reports `interface_created` before the fallible `ifconfig` step; on failure it returns the status so the caller can preserve resources for RUNNING/UNKNOWN/error states or safely clean them after confirmed absence. Added a source-contract assertion that the ownership report precedes ifconfig and the TAP helper contains no local bridge unlink or `--rmtun`.
- Exact base `7b67475e01d13139d9fb97cc9ada5a5cda9b5361`: VPN server lifecycle #45662378987 PASS; AI docs #45662378978 PASS. Current fix tests and exact-SHA Actions pending. Earlier d91 XY-C1 Build/TRX/artifact PASS applies only to `d91bc841...`; dc6 exact build #45660655080 was still running at last poll. Physical-router L4 NOT VERIFIED.

---

## 2026-11-26 03:24 CST (UTC+8) — Scope interface ownership checks to declarations

- Base SHA: `615469fc69f100ad6dfce63a27b2a0b3f46f2864`; focused source-check correction, no production-code changes.
- Exact-SHA VPN lifecycle #45661993374 passed all C regressions and the earlier lifecycle source contracts, including the two-snapshot OpenVPN rollback policy test. It then failed because the new ownership checker searched the function body for the parameter declaration `int *interface_created`; the shared `body()` parser intentionally returns only statements inside braces. Moved declaration inspection to the signature span and kept the initialization/ownership assignments asserted inside the function body for both TAP and TUN.
- Exact-SHA OpenVPN credential staging #45661993123 PASS and AI handoff docs #45661992916 PASS. The prior d9 VPN run #45661762483 failed at the double-escaped signatures, now normalized in `615469fc69f100ad6dfce63a27b2a0b3f46f2864`; the 615 run exposed this final checker-scope issue.
- Parent dc6 XY-C1 #45660655080 was still building at last poll; process lifecycle #45660655082, process inventory #45660655142, NVRAM #45660655603 and HTTPD #45660655362 had completed SUCCESS. Same-SHA L3 requires the XY job's Build, TRX and artifact steps. Physical-router L4 NOT VERIFIED.

---

## 2026-11-26 03:10 CST (UTC+8) — Fix escaped OpenVPN lifecycle checker signatures

- Base SHA: `d9add97d1b68a5fcb468f27dc8accda1ab1e0e0b`; this is a checker/workflow dependency repair, committed as one main descendant.
- The server TUN/TAP topology-order contract was updated in exact SHA `d9add97d1b68a5fcb468f27dc8accda1ab1e0e0b`. Review of the same checker found six appended body() lookups with double-escaped newline markers; Python would search for literal backslash-n text instead of the actual C function signature when execution reaches those checks. Normalized them to one Python newline escape.
- The two-snapshot owned-resource OpenVPN rollback regression had already compiled and passed under GNU89 ASan/UBSan in exact VPN runs #45660655211 and #45661438577; the latter failed before reaching the appended function-body checks, on the stale TUN/TAP order markers now fixed in d9.
- Added the rollback policy header and test to the OpenVPN client credential workflow's push and pull_request path filters; its checker reads these files directly.
- Exact d9 VPN lifecycle #45661762483 and AI docs #45661762480 were in progress at last poll. Previous 1aa OpenVPN credential #45661438519 and AI docs #45661438442 PASS; VPN #45661438577 failed at the stale server topology/start TUN/TAP markers.
- XY-C1 Build/TRX/artifact on the detached-start production SHA remains to be verified; physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Align the remaining OpenVPN server TAP/TUN source-order assertions

- Base SHA: 1aa51a6ef623bdc50c4351e50bd4175f3410f670; result SHA: resolve from next main handoff. Scope: scripts/check_vpn_server_lifecycle.py, SESSION_HANDOFF.md, dev_log.md.
- Follow-up exact SHA 1aa51a6ef623bdc50c4351e50bd4175f3410f670: OpenVPN client credential staging #45661438519 PASS, AI handoff docs #45661438442 PASS. VPN lifecycle #45661438577 reached its last source-order assertion and failed because two tokens still expected the pre-ownership signatures openvpn_tunif_start(IFNAME_SERVER_TUN, 1) and openvpn_tapif_start(IFNAME_SERVER_TAP, 1, 1). The production helpers now correctly take &interface_created; updated those ordered tokens without relaxing the check.
- The preceding rollback checker now verifies exact detached-daemon postcondition ordering, invokes the shared rollback handler, and asserts that handler requires two full ABSENT inventories and per-attempt resource ownership. The dedicated startup rollback regression passed.
- dc6 XY-C1 run #45660655080 was still pending at last poll; verify its Build/TRX/artifact steps before claiming L3. Earlier d91 exact-SHA L3 remains PASS. L4 NOT VERIFIED.

---

## 2026-11-25 — Repair OpenVPN detached-startup contracts and restore the log archive

- Base SHA: dc6e17b4874cfb7a6d07d8b69fb2d195176efeb7; result SHA: resolve from the next main handoff. Scope: OpenVPN server/client source checkers, SESSION_HANDOFF.md, and dev_log.md.
- Exact-SHA OpenVPN client credential workflow #45660655024 failed only at a stale source-contract marker: the server TUN helper now takes the ownership out-parameter (&interface_created). The dedicated startup rollback policy regression compiled and passed.
- Exact-SHA VPN server lifecycle workflow #45660655211 failed only at a stale assertion expecting callback-link removal inline in each start function. Production code now centralizes rollback in openvpn_start_rollback_failed(), which requires two clean full process-inventory snapshots proving PROCESS_PRESENCE_ABSENT before removing resources created by the current attempt. The ownership/identity policy regression passed.
- Exact-SHA AI handoff docs #45660655384 failed because the preceding commit accidentally replaced dev_log.md with a single entry. Restored all 71 complete historical entries from the preceding main commit and prepended this record. No history was discarded.
- XY-C1 exact-SHA validation for dc6e17b4874cfb7a6d07d8b69fb2d195176efeb7 must be checked separately; no L3 claim is made here. Earlier d91 production source SHA d91bc841fddf1cc311cda431796de5a731cf0fd7 had Build XY-C1, TRX and artifact upload PASS. Physical-router L4 NOT VERIFIED.

---

## 2026-11-26 01:34 CST (UTC+8) — Erase transaction-scoped NVRAM value cache

- Base SHA: `d91bc841fddf1cc311cda431796de5a731cf0fd7`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `nvram_linux.c`, `nvram_linux.h`, new `nvram_value_cache_policy.h`/test, NVRAM source checker/workflow, XY-C1 filter, handoff/log.
- Review found `nvram_get_checked_unlocked()` used one process-global static ring cache for both legacy `nvram_get_checked()` and `nvram_transaction_get()`. The transaction API requires callers to copy values before the next read and holds the exclusive NVRAM lock, but sensitive transaction values remained resident after the lease ended.
- Introduced a transaction-specific cache separated from the legacy getter cache. Transaction begin wipes it before reuse; transaction end volatile-wipes the full array and resets the ring cursor before releasing the exclusive lock, including the wrong-owner child path. Legacy getter pointer storage is not invalidated.
- Added a GNU89/ASan/UBSan policy test for full-buffer erasure and cursor reset, source contracts for cache separation and erase-before-unlock, and documented the transaction value lifetime. Added shared NVRAM source/header/helper to XY-C1 path filtering so future production changes trigger a build.
- Exact base `d91bc841fddf1cc311cda431796de5a731cf0fd7`: OpenVPN credential #45657032237 PASS; VPN lifecycle #45657032067 PASS; AI docs #45657032165 PASS; Process inventory #45657032815 PASS; process lifecycle #45657032140 PASS. NVRAM audit #45657032113 and XY-C1 #45657032169 were still running at last poll. This new code requires its own exact-SHA NVRAM and XY-C1 validations. L4 NOT VERIFIED.

---

## 2026-11-26 01:12 CST (UTC+8) — Reject newline injection in OpenVPN client credentials

- Base SHA: `c2366fd5c60833e7e42f0aef457ea8e0773d11d3`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `openvpn_client_secret_file.h`, its regression, OpenVPN source checker, VPN lifecycle dependency checker, XY-C1 firmware path filter, handoff/log.
- The private auth-user-pass publisher used `fprintf("%s\\n%s\\n")` without rejecting CR/LF in either input. A newline in an NVRAM username or password could create extra records and violate the two-line credentials format. It now rejects CR/LF before `mkstemp()`, so an invalid input cannot create/replace any file.
- Regression tests assert both username-LF and password-CR rejection, `EINVAL`, unchanged previously published credentials, and zero leftover stage files. Source contract pins validation before staging.
- Added `openvpn_client_secret_file.h` to the XY-C1 production path filter and VPN lifecycle checker dependency list; a future header-only code change can no longer silently skip the firmware build.
- Exact predecessor evidence: OpenVPN callback link regression and source contract PASS #45655706781; VPN lifecycle source contract rerun PASS #45656077803; AI handoff docs #45656077743 PASS. XY-C1 on production SHA `c4c12ce5cf7b4e0098d71723ddde30e87c1e5ad6` was still running at last poll. This CR/LF change needs a new exact-SHA build/TRX/artifact; L4 NOT VERIFIED.

---

## 2026-11-26 01:02 CST (UTC+8) — Refresh OpenVPN callback link source contract

- Base SHA: `c4c12ce5cf7b4e0098d71723ddde30e87c1e5ad6`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `scripts/check_vpn_server_lifecycle.py`, `.github/workflows/vpn-server-stability.yml`, handoff/log.
- Exact SHA `c4c12ce5cf7b4e0098d71723ddde30e87c1e5ad6`: OpenVPN credential staging run #45655706781 PASS, including callback-link policy C regression and OpenVPN source/workflow contract. VPN server lifecycle run #45655706646 fails only because the final checker still requires the legacy raw `symlink()` call after interface setup. All earlier 13 steps passed, including all VPN client and PPP regressions.
- Checker now pins the intended safer order: exact callback link ensure before interface setup, interface failure abort before daemon launch, and cleanup restricted to a link created by this attempt; it rejects fallback to raw unchecked `symlink()`. The new helper header is also included in the server workflow push/PR path filters and XY-C1 dependency contract.
- At the same SHA, AI handoff docs #45655707053, HTTPD security #45655706750, IPv6 service type #45655706937 and Process inventory #45655706888 PASS. NVRAM audit #45655706763, process lifecycle #45655706696, XY-C1 #45655706852 still in progress at last poll. The XY-C1 production build/TRX/artifact on the exact production source SHA remains pending. L4 NOT VERIFIED.

---

## 2026-11-26 00:48 CST (UTC+8) — Fail closed when OpenVPN callback symlink setup fails

- Base SHA: `98f74b97af0d83f6b5d6542c8d355815b30274e8`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `vpn_openvpn.c`, new `openvpn_script_link.h` and regression, OpenVPN workflow/source checker, XY-C1 workflow path filter, handoff/log.
- Both OpenVPN server and client startup ignored `symlink("/sbin/rc", ...)` return values and continued starting the daemon. A missing/incorrect callback link would leave OpenVPN connected without the state/DNS callback lifecycle. New header code verifies the link is a symlink with literal target `/sbin/rc` using `lstat/readlink`, rechecks inode identity after reading, idempotently reuses a correct existing link, and fails closed for regular files, wrong links, or other errors.
- Server/client startup now ensures the callback link before interface setup; if interface setup fails, only the link created by this attempt is removed. Host regression covers create/reuse, mismatch, regular-file conflict, and ownership-aware cleanup. This regression and source contract are pending exact-head validation.
- At base SHA: OpenVPN credential wipe suite #45655016393 PASS; VPN lifecycle #45655016612 PASS; AI docs #45655016470 PASS; IPv6 service type #45655016600 PASS. NVRAM/process/HTTPD and XY-C1 run #45655016829 were still pending. The credential-wipe XY build requires confirmation; predecessor L3 PASS at production SHA `311d6080fd7cb5d6e4cb5906f5923b45c350e108`. L4 NOT VERIFIED.

---

## 2026-11-26 00:36 CST (UTC+8) — Zeroize OpenVPN client credential snapshots

- Base SHA: `21c84c9c7550bc06a8f987378cb08651d6f4db53`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `vpn_openvpn.c`, new `openvpn_client_credentials_policy.h`, new regression `tests/openvpn_client_credentials_policy.c`, OpenVPN source checker/workflow, XY-C1 workflow path filter, live handoff/log.
- Review found copied OpenVPN client username/password remained in the temporary `pending` snapshot after copying to the caller, and in the caller's stack config after config-file generation or an earlier runtime-state cleanup failure. A failed snapshot could also leave partially read secrets in the local pending struct.
- Added an explicit volatile-byte zeroization primitive and regression. The source clears the pending snapshot on both success and failure after ending the NVRAM transaction, and clears caller credential arrays on runtime-state cleanup failure or immediately after OpenVPN config generation (whether it succeeds or fails). Non-secret fields remain available for interface setup.
- Wired the regression/source contract into push and pull_request OpenVPN path filters; added the production helper header to the XY-C1 firmware path filter. Focused tests and exact-head Actions are PENDING.
- Prior proof: VPN lifecycle #45653816232 PASS; OpenVPN credential suite #45653449852 PASS; NVRAM audit #45653449837 PASS; Process inventory #45653449686 PASS; AI docs #45653449768 PASS. XY-C1 Build/TRX/artifact PASS on production SHA `311d6080fd7cb5d6e4cb5906f5923b45c350e108`; this new production-source commit requires its own full XY-C1 run. L4 NOT VERIFIED.

---

## 2026-11-26 00:28 CST (UTC+8) — Correct VPN PPP source-order assertion

- Base SHA: `d32243c4ab0eaf23cbdaf40e15ef73db4edcd9ff`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `scripts/check_vpn_client_nvram_state.py`, `SESSION_HANDOFF.md`, `dev_log.md`.
- Exact-head VPN server workflow #45653449911 confirmed the earlier fixture fix: `vpn_client_nvram_policy.c` and preceding VPN/PPP host regressions now pass. The remaining failure is a false source-contract expectation at checker line 96: `config.peer_line`, `config.user_line`, `config.pass_line` and `config.pppd_options` are intentionally consumed by `fprintf()` after `ppp_options_file_open()`; their values were already copied/validated in `vpn_client_read_start_config_snapshot()` before any file open. The checker now asserts that real sequencing instead of asserting writes before opening the file.
- Exact-head OpenVPN client secret #45653449852 PASS, AI handoff docs #45653449768 PASS, Process inventory #45653449686 PASS. NVRAM boundary #45653449837 was still in progress; XY-C1 #45653449737 was still in progress with TRX/artifact pending. This commit's exact-SHA reruns are PENDING.
- No firmware L3 or physical-router L4 result is claimed.

---

## 2026-11-26 00:16 CST (UTC+8) — Repair exact-head VPN regression contracts

- Base SHA: `311d6080fd7cb5d6e4cb5906f5923b45c350e108`; result SHA: THIS COMMIT (resolve on next recovery). Scope: `trunk/user/rc/src/tests/vpn_client_nvram_policy.c`, `scripts/check_openvpn_client_secret.py`, `SESSION_HANDOFF.md`, `dev_log.md`.
- Exact-head VPN server run #45652102199 failed because the regression asserted that a 13-byte value could be copied into an 8-byte buffer; the implementation correctly returned `ENAMETOOLONG`. The fixture now separately tests short-buffer rejection and successful full-buffer copying, and retains an explicit oversized-value failure assertion.
- Exact-head OpenVPN client secret run #45652102448 failed because its checker required the removed cache-backed `nvram_safe_get()` credential reads. The production code now validates and passes snapshot-copied `username`/`password` to the secret publisher; this checker now pins that behavior and rejects a regression to those getters.
- Verification before this fix: the OpenVPN secret policy binaries, config publication regression, lease snapshot policy and lease-lock policy passed; only the final stale source contract failed. VPN server suite's earlier C steps passed; the client NVRAM policy regression failed at the undersized fixture. This commit's focused tests and exact-head Actions are PENDING.
- Related exact-head evidence on base `311d6080fd7cb5d6e4cb5906f5923b45c350e108`: Process lifecycle regression #45652102139 PASS; NVRAM boundary #45652102079 PASS; AI docs #45652102145 PASS; Process inventory #45652101913 ended `startup_failure` before job steps; XY-C1 #45652102254 Build in progress, TRX/artifact pending. No L3/L4 claim.

---

## 2026-11-25 — Snapshot VPN client startup settings before side effects

- PPTP/L2TP startup reads enable/type/auth/MPPE/MTU/MRU/default-route/peer/user/password/custom PPP options in one checked NVRAM transaction. Backend or transaction-end errors, malformed values and oversized strings abort before runtime DNS clearing, PPP file creation or protocol selection; absent optional values retain defaults and MTU/MRU retain the 1000–1460 clamp.
- OpenVPN enable/type/selectors and credentials join its checked client-config transaction. Runtime DNS/state clearing follows full config validation, and secret publication uses copied credentials instead of unchecked cache-backed getters.
- Added policy regressions for read failure versus absence, stale-buffer clearing, bounded copy, defaults/clamps, malformed suffix and overflow; static contracts pin snapshot-before-side-effects. Exact VPN/Process/NVRAM/AI/XY-C1 Actions pending.
- DHCPv6 finite-expiry fix `cb8f014` has exact-SHA L3: XY-C1 #45647751940 Build, TRX and artifact steps all PASS on the same SHA; focused DHCPv6/Process/WAN/USB/NVRAM checks PASS. Physical-router L4 NOT VERIFIED.

---

## 2026-11-25 — Reject corrupt RStats history ring cursors before restore

- `load_history_raw()` accepted a persisted `history_t` when only its byte count and magic ID matched, then copied `dailyp` / `monthlyp` directly into live state. `bump_history()` indexes the daily/monthly arrays using those cursors, so a corrupt or stale record could cause an out-of-bounds access during the next statistics update.
- Added a capacity-aware cursor validator and discard/log invalid records before `memcpy`. Regression covers first/last valid positions, negative and capacity-equal cursors, and invalid capacities; the existing workflow locks validation-before-copy order.
- Base main SHA: `195bc627d2d809da1877bc7c119a5a5e73342799`. Exact-head RStats Actions pending. No firmware-build or hardware-verification claim.

---

## 2026-11-25 — Align optional-app source checker with Dropbear PID identity

- The first exact Process lifecycle run #45649732824 on `880c339` failed before the new shell fixture because `scripts/check_optional_app_start_status.py::check_dropbear()` still asserted the retired `pidof` output-validation expression and its mock `killall` behavior. Those assertions no longer described the hardened procfs implementation.
- Replaced the stale mock-specific checker with explicit source invariants for procfs identity, stat start time, exact executable identity, targeted TERM/KILL, absence of name-only discovery and presence/syntax of the new regression. The dedicated shell fixture remains the behavioral test. Exact rerun pending.

---

## 2026-11-25 — Correct Dropbear shell parameter expansion in identity checks

- Review of the first published Dropbear identity change found that the generated shell source contained literal backslashes before 13 `${...}` expansions. That would prevent procfs-root, stat-field and PID snapshot variables from expanding as intended.
- Removed the stray backslashes in the source. This is a required correction to the Dropbear scanner/stop implementation; the exact process-lifecycle workflow must pass on the corrected SHA before claiming verification.

---

## 2026-11-25 — Bind Dropbear lifecycle signals to executable identity

- The service controller used BusyBox `pidof dropbear` and `killall -q dropbear`. BusyBox `find_pid_by_name()` accepts a short process-`comm` match without validating the executable, so an unrelated process can spoof `dropbear` via `PR_SET_NAME` and be counted or killed by the root stop command.
- Replaced name-only discovery with a complete numeric procfs scan that validates the stat record, PID, state, start time, exact comm and `/proc/PID/exe` device/inode against the installed Dropbear binary. Incomplete/unknown procfs blocks start and stop; TERM/KILL signal only freshly revalidated PID/start-time/executable identities.
- Added a shell regression for comm spoofing, executable mismatch, PID-reuse mismatch, targeted TERM, bounded KILL, signal failure propagation, missing procfs, malformed stat and unreadable executable identity. Local fixture PASS; exact GitHub Actions pending.

---

## 2026-11-25 — Retain the complete maximum HTTPD closing delimiter across reads

- The parser accepts a 127-byte marker, optional closing `--`, up to 128 bytes of transport padding and a CRLF terminator. The former 257-byte carry window covered a regular delimiter but could truncate the leading marker byte when the maximum 259-byte closing delimiter was split immediately before its final LF; this could leave delimiter bytes in the uploaded file.
- Sized the carry window from explicit marker, padding and suffix maxima (259 bytes). Extended the existing carry regression to split the maximal closing delimiter before its last byte and assert the whole previous chunk fits the carry. The existing workflow compiles and checks this same fixture name; exact HTTPD rerun pending. Earlier fixture-correction run #45647312009 passed.

---

## 2026-11-25 — Restore AI handoff validation after roadmap size overflow

- Exact AI handoff run #45647751963 on `cb8f014893d99e8991c55cd792443e5c2c1f4a37` failed only because REFACTOR_ROADMAP.md reached 131682 bytes against its 131072-byte hard cap. DHCPv6 IA regression #45647751899, Process inventory #45647752064, WAN DHCP callback #45647751951 and USB storage #45647752298 all PASS on that exact source SHA; XY-C1 #45647751940 and NVRAM #45647752146 remained in progress at last poll.
- Removed two obsolete 2026-11-14 run-snapshot sections from the roadmap; their historical outcomes are retained in the development log/archive and the active module dashboard/priority queue remain unchanged. The current DHCPv6 finite-expiry fix summary stays at the top. Exact AI rerun pending.

---

## 2026-11-25 — Arm DHCPv6 IA finite expiry before address publication

- update_prefix6() previously armed a finite valid-lifetime timer only after all downstream IA_PD/IA_NA kernel address operations succeeded. A failure adding a later delegated-prefix interface returned early after earlier interfaces may already have received addresses, leaving partial state without a refreshed expiry timer while the Reply was retried.
- Move zero-lifetime removal/return and finite/infinite timer handling before fallible address publication. Finite lifetimes now arm/reset expiry first; infinite lifetime cancels any old timer; zero lifetime follows the cleanup-retry path without trying additions.
- Add a GNU89 policy fixture for zero, one-second, ordinary finite, maximum finite and infinite values. The Python source contract locks expiry timer setup before both delegated-prefix and IA_NA additions and confirms zero-lifetime removal returns before reconciliation. Exact-head CI pending.

---

## 2026-11-25 — Correct HTTPD multipart fixture escape sequences

- Review of the newly added boundary regression found that the C string literals had two backslashes before `r`, `n`, `t` and `0`. They exercised literal backslash text rather than actual CR/LF/tab/NUL byte semantics.
- Corrected the fixture literals to single C escape sequences. Production helper and parser logic are unchanged. The focused helper harness compiled and passed under GNU89 ASan/UBSan; the exact repository workflow is still pending.

---

## 2026-11-25 — Validate complete HTTPD multipart delimiters

- Storage-restore scanning previously accepted any `\r\n--boundary` prefix without checking the following delimiter-line syntax. A BZ2 payload containing the same prefix followed by an ordinary byte could be mistaken for end-of-part; the truncated upload could be atomically published and flagged ready, replacing the retained retry archive.
- The rolling lookbehind was 64 bytes although the fixed 128-byte boundary buffer permits a 127-byte marker. A marker split across reads could therefore be missed. Recognition now requires a regular/closing delimiter followed by CRLF or the existing LF compatibility ending, with bounded 128-byte transport padding; the carry is 257 bytes.
- Added GNU89 fixture cases for false prefixes, malformed closing suffixes, regular/closing/LF delimiters, transport padding, and a maximum-size marker split across reads. Focused local ASan/UBSan harness PASS; exact-head HTTPD and firmware Actions pending.

---

## 2026-11-25 — Correct VSFTPD MDTM fixture and source contract

- Exact VSFTPD run #45644905866 first exposed a fixture-adapter error: the test passed the full command length instead of the timestamp-token length. The fixture now extracts the first-space offset; a local GNU89 ASan/UBSan compile/run passed.
- Exact run #45645513224 compiled and ran the corrected fixture successfully, then failed only because the source checker searched for the removed `str_stat()` name inside an explanatory comment. Narrowed the checker to the actual raw-argument call expression and asserted the test's separator-based token extraction; exact-head rerun is pending.

---

## 2026-11-25 — Remove pre-ACL filesystem probing from FTP MDTM

- `handle_mdtm()` previously called `str_stat()` on the raw client argument before `vsf_access_check_file()` or the share ACL check. The result controlled whether a date-like token was treated as an MDTM write timestamp or as a filename, giving unauthorised path existence influence over command parsing.
- Replaced that filesystem-dependent ambiguity check with strict RFC-style time-token grammar validation before any filesystem access. Date-only legacy tokens, 14-digit timestamps and fractional seconds remain supported; malformed calendar dates/times and malformed fractions fall back to ordinary filename handling. Actual reads/writes still use descriptor-relative ACL-checked opens and fd-based mtime operations.
- Added GNU89 ASan/UBSan policy coverage for leap years, month/day boundaries, clock ranges, fractional syntax, delimiters and missing paths; exact-head VSFTPD workflow pending.

---

## 2026-11-25 — Reject ambiguous IPv4 notation in WAN gateway ARP configuration

- Confirmed that `inet_aton()` accepts abbreviated, octal, hexadecimal and single-integer IPv4 forms. For example, `0300.0.2.1` is interpreted as `192.0.2.1`, even though the value looks like a decimal dotted address; the WAN gateway monitor could therefore probe a different host than the configured text indicates.
- Changed the parser to `inet_pton(AF_INET, ...)`, requiring an unambiguous four-octet dotted-decimal address. Empty and explicit `0.0.0.0` retain the legacy no-gateway behavior, and parse failures leave the output untouched.
- Extended the GNU89 gateway-address regression with leading-zero, octal, abbreviated, hexadecimal, integer and trailing-whitespace forms. A focused host GNU89 ASan/UBSan harness passed; exact-head WAN ARP and XY-C1 Actions remain the verification gate.

---

## 2026-11-25 — Free IA_NA configuration prefix lists during DHCPv6 teardown

- Confirmed a config-lifetime leak in BusyBox DHCPv6: `configure_ia()` stores IA_NA addresses in `iac->prefix_list`, but `clear_iaconf()` only reached `dhcp6_clear_list()` through the IA_PD-only `clear_pd_pif()` path. IA_NA list nodes and nested values were therefore abandoned on configuration cleanup.
- Moved prefix-list ownership cleanup into a type-aware shared policy used by both IA_NA and IA_PD, retaining the existing IA_PD-specific interface cleanup and avoiding duplicate list release. Added four GNU89 ASan/UBSan cases (IA_NA, IA_PD, unrelated type and invalid callback inputs), plus source-order/adapter contracts.
- Both push/PR filters for the DHCPv6 IA regression and XY-C1 firmware validation now track the new helper and fixture. Parent `e985875` has fresh Actions in progress; exact results on this new source commit are pending.

---

## 2026-11-25 22:24 CST (UTC+8) — Correct vanished-interface route teardown contract

- - Exact SHA `578c8647eec5e0c39bb4910d7757a0de5752468e`: Firewall startup #45632938361 PASS, USB modem #45632938668 PASS, AI docs #45632938331 PASS, Process lifecycle inventory #45632938233 PASS, XY-C1 #45632938514 PASS host validation but Build XY-C1/TRX/artifact were SKIPPED because this SHA did not change product inputs. WAN route #45632938327 failed only at the stale assertion `return clear_status;`: `wan6_down()` now accumulates the earlier SIT-route cleanup and metadata-clear errors into `status` and correctly returns that aggregate. This correction narrows the check to the vanished-interface branch and asserts both aggregation and `return status;`.
- The subsequent exact source SHA `263050a5762527a9ebe20560f2d15cc87cf15378` adds a real SIT route-gateway normalization fix: empty gateway values are now passed as NULL to route_add/route_del instead of a non-NULL empty string, which the BusyBox route API rejects. Two GNU89 tests cover empty and non-empty gateway arguments; source checks cover both add and delete paths.
- On `263050a`, current exact-head suite is queued/in progress: XY-C1 #45633281371, WAN route #45633280992, WAN DHCP #45633281034, NVRAM #45633281147, IPv6 service #45633280943, Process inventory #45633280920/#45633281057, USB modem #45633281299, Firewall #45633281250, HTTPD security #45633281195. XY-C1 must reach Build XY-C1, TRX verification and artifact upload on this exact SHA. No L3 or physical L4 claim yet.

---

## 2026-11-25 22:10 CST (UTC+8) — Fail closed on malformed LAN IPv6 address before NVRAM publication

- Found a concrete state-corruption bug in `reset_lan6_vars()`: `ipv6_from_string()` returns `-1` on invalid input, but the old code skipped address formatting and still published the uninitialized-empty `addr6s` value to NVRAM. A subsequent `reload_lan_addr6()` interprets empty as a request to flush the live global IPv6 address on `br0`; malformed configuration could therefore erase the working address.
- The function now checks the parser result and `inet_ntop()` status explicitly, propagates `EINVAL`/the conversion errno (or `EIO` fallback), and returns before NVRAM write. Prefix-format failures are also fatal and leave the prior configuration untouched.
- Added five GNU89 policy regressions for invalid parse, stale errno, `inet_ntop` failure with/without errno and valid prefix length zero. Extended XY-C1 modem checker and WAN-route source contract to prove conversion guard and early return precede `nvram_set_temp("lan_addr6", ...)`.
- Parent `40a77df4fbb3e41251d135994035ee9876c4d2a8` added SIT endpoint-route cleanup ownership assertions/tests. Exact Actions on that parent included XY-C1 #45631397674, WAN route #45631397687, WAN DHCP #45631397694, Process inventory #456313973? and other userland workflows; most were queued/in progress at last poll. This source-bearing commit should run current XY-C1 Build/TRX/artifact on one exact SHA. Physical router L4 remains NOT VERIFIED.

---

## 2026-11-25 22:02 CST (UTC+8) — Align modem/XY-C1 checker with LAN6 recovery API

- Exact d5 source SHA `d5adff9d7413b391d523d4b2c0d571d75de8b98a`: LAN IPv6 regression compiled and printed PASS; WAN route #45629160990, NVRAM #45629160447, IPv6 service type #45629160900, AI docs #45629161062, Process inventory #45629160410 and DNSMasq #45629160328 PASS. WAN teardown #45629160931 failed only on a stale snapshot API assertion, corrected in `3ea1835`; exact `3ea1835` WAN teardown #45629626346 PASS and AI docs #45629626161 PASS.
- The same stale call remained in `scripts/check_modem_wan_start_status.py:699`. It blocked XY-C1 #45629161139 before Build/TRX/artifact and Process fail-closed #45629160483. Updated the checker to require the recovery-aware apply/snapshot policy, snapshot-before-mutation, both recovery logs, argv-based `ip addr replace`, and all seven recovery fixture cases.
- No source change in this checker-only commit; XY-C1 may run host checks but its production-input detector can skip Build. Current-source L3 remains NOT VERIFIED; physical L4 NOT VERIFIED. The next source-bearing commit must run Build + TRX + artifact on one identical SHA.

---

## 2026-11-25 21:57 CST (UTC+8) — Update WAN teardown LAN IPv6 reload contract

- Exact-head WAN teardown #45629160931 on `d5adff9d7413b391d523d4b2c0d571d75de8b98a` failed in the final Python source-contract block because it still expected `rc_lan_addr6_reload_from_snapshot()`; production now uses the recovery-aware `rc_lan_addr6_reload_from_snapshot_with_recovery()`. The GNU89 LAN IPv6 policy fixture had already compiled and printed PASS in this same job; the failure was a stale assertion, not a C regression.
- Updated the teardown contract to require the recovery-aware snapshot/apply APIs and argv-based BusyBox `ip addr replace` restoration. Added `net_lan6.c`, the policy header and fixture to the workflow's internal push/PR path assertions. No production source changed in this follow-up.
- On `d5adff9`: Process lifecycle inventory #45629160410, IPv6 service type #45629160900, AI docs #45629161062 and DNSMasq #45629160328 PASS; WAN teardown #45629160931 failed as described; WAN route #45629160990, XY-C1 #45629161139, NVRAM #45629160447 and Process fail-closed #45629160483 were still in progress at last poll. XY Build/TRX/artifact were pending. Current-source L3 is NOT VERIFIED; physical L4 is NOT VERIFIED.

---

## 2026-11-25 21:55 CST (UTC+8) — Recover LAN IPv6 address after failed reload

- Found a second destructive-update gap in `reload_lan_addr6()`: it flushes all global addresses on `br0`, then adds the configured NVRAM address. If the flush partially fails or the add fails, it previously returned an error without making a compensating attempt to preserve/recreate the configured address.
- Added a recovery-aware policy that retains the primary clear/add error and reports recovery status separately. Recovery uses BusyBox 1.24's verified `ip -6 addr replace <address> dev br0` operation; an already-present address is replaced idempotently, while a missing address is recreated. A failed NVRAM snapshot still blocks clear/add/recovery side effects.
- Added seven GNU89 regression cases for recovery after clear failure, recovery after add failure, independent recovery-error reporting, empty configuration, callback preflight, NVRAM read failure and normal success ordering. Updated WAN-route source contracts and XY-C1 source checks; added `net_lan6.c` to the XY-C1 product-input filter so this production change triggers Build/TRX/artifact validation.
- Main parent `7a9bdb35a879be1dfae3bbc8277b2a6a845249cf` follows `103e7355`'s fix for duplicate SIT endpoint host routes. On `103e7355`: WAN route #45627761309 PASS, AI docs #45627761272 PASS, Process inventory #45627761497 PASS, IPv6 service #45627761403 PASS and HTTPD security #45627761288 PASS; WAN DHCP #45627761300 failed only on an outdated `WAN6_SIT_CHECK(route_add(...))` assertion, corrected in `7a9bdb3`; WAN DHCP #45628195224 and AI docs #45628195241 PASS on `7a9bdb3`.
- On `103e7355`, NVRAM #45627761287 and XY-C1 #45627761382 were in progress at the last poll (Build active; TRX/artifact pending). Earlier checker-only XY-C1 #45627044645 host validation passed but Build/TRX/artifact were skipped because its change set lacked a production source diff. Historical L3 on `0bed846` does not validate SIT or current LAN reload code; no current-source L3 or physical L4 claim.

---

## 2026-11-25 21:44 CST (UTC+8) — Synchronize duplicate WAN6 command-count checks

- Two additional exact-head validations on `0ae462027968d0d31ddf9c349cd0b5f15ab0cdc7` failed on the same stale pre-SIT source-count assertion: XY-C1 #45626020329 at `scripts/check_modem_wan_start_status.py:744`, and WAN DHCP callback #45626020186 in its embedded source contract (workflow line 1472). All earlier C regression steps in those jobs passed; XY-C1 Build/TRX/artifact steps were skipped because host validation stopped at that contract.
- The source now has four argv-based `ip -6 addr add` sites after adding the rollback adapter. Updated both checks to expect four and explicitly require the saved address be passed as a single argument to `ip` on `br0`. The route workflow check had already been updated to four in `92578d78` and its YAML indentation corrected in `14bbc0a`.
- L3 record: XY-C1 run #45624855182 on exact source SHA `0bed846b6ac67f8f77f0104f06decddcaa62ce1a` completed successfully, including Build XY-C1, TRX verification and artifact upload. This does NOT validate later SIT source commit `0ae4620`; on `0ae4620`, XY-C1 host validation failed before build/TRX/artifact.
- Exact reruns of WAN route, WAN DHCP and XY-C1 are required for the current source; no physical-router L4 claim.

---

## 2026-11-25 21:43 CST (UTC+8) — Fix YAML block indentation in WAN route contract

- Review of the follow-up commit `92578d786df806f9da1f68d0b6ce835b0c397207` caught a formatting error in the newly added embedded Python assertion: it had been inserted at column zero instead of the ten-space indentation required by the YAML `run: |` block. This made the workflow file invalid; corrected the indentation here.
- The earlier exact WAN route failure on `0ae4620` remains attributed to the old hard-coded command count (3 vs the new 4); this commit now contains both the count update and properly nested argv-safety assertion. Require a fresh exact-head workflow validation before claiming PASS.
- XY-C1 run #45626456137 on `92578d7` is IN_PROGRESS with Build pending at last check; TRX and artifact upload are pending. No L3 claim.

---

## 2026-11-25 21:42 CST (UTC+8) — Align IPv6 source contract with SIT LAN restore callback

- Exact-head WAN route run #45626020132 on `0ae462027968d0d31ddf9c349cd0b5f15ab0cdc7` failed only in the final embedded source-contract step. All preceding 52 steps, including the new WAN IPv6 identity test compile and `check_wan_vlan_setup.py`, passed.
- The failure was a stale hard-coded count: the route workflow expected three argv-based `ip -6 addr add` calls in `net_wan6.c`, while the new safe LAN-address restoration callback correctly adds a fourth. Updated the expected count and added an explicit assertion that the restoration callback passes the saved address as one argv argument on `br0`. No production code changed in this follow-up.
- This correction needs fresh exact-head WAN route validation. On source SHA `0bed846b6ac67f8f77f0104f06decddcaa62ce1a`, all listed stability workflows and NVRAM audit #45624855071 PASS; XY-C1 #45624855182 was still building when last inspected, with TRX/artifact pending. L3 not yet claimed.

---

## 2026-11-25 21:39 CST (UTC+8) — Restore prior LAN IPv6 address when SIT activation fails

- Found a concrete rollback hole in `start_sit_tunnel()`: after the tunnel and routes were ready, the function flushed all global addresses from `br0` and attempted to add a derived 6to4/6rd LAN address. If the clear partially failed, the add failed, or publishing the new address to NVRAM failed, the common failure path removed the incomplete SIT tunnel but never restored the previously configured `lan_addr6`. The old address could therefore be lost after a failed WAN IPv6 transition.
- The code now performs a checked snapshot of `lan_addr6` before touching bridge addresses, marks the address transaction before the flush, and restores the previous configured value through argv-based `ip -6 addr add` after clearing partial state on any subsequent failure. Empty prior configuration is preserved as empty; rollback errors are logged separately and do not replace the original SIT startup failure.
- Added a callback-driven GNU89 policy and regression cases for restoration ordering, empty snapshot, clear/add failure aggregation, untouched state and invalid callbacks. Source-order checks are added to `scripts/check_wan_vlan_setup.py`; existing WAN route workflow already includes the touched source, policy, test and checker paths. Exact-head Actions pending.
- On parent `0bed846b6ac67f8f77f0104f06decddcaa62ce1a`: WAN route #45624855225, Wireless #45624855188, WAN DHCP #45624855254, Process inventory #45624855312, Process fail-closed #45624855157, WAN sysfs counters #45624855211, USB modem #45624855066, USB storage #45624855128, AI docs #45624855327 and IPv6 service type #45624855120 PASS. NVRAM #45624855071 and XY-C1 #45624855182 remained IN_PROGRESS at last poll. No same-SHA XY-C1 Build/TRX/artifact completion or physical L4 claim yet.

---

## 2026-11-25 21:31 CST (UTC+8) — Harden bridge M2U errno and snapshot validation

- Reviewed the M2U transaction after its harness corrections. Linux 4.4 exposes both bridge-port flags as boolean 0/1 values; the policy now fails closed with EPROTO if readback violates that kernel contract.
- The production setter adapter normalizes positive status-code errors and negative/-1 failures into -1 with a reliable errno, clearing stale errno before each write. The sysfs reader initializes its output on failure.
- Added GNU89 cases for rollback after the first setter itself fails and for impossible non-boolean snapshots. Existing second-write rollback, reverse-order restore and primary-error preservation cases remain.
- On parent `7192958c21ddce8798b8793c9b34df34717ed15d`, WAN route #45624479165 and AI docs #45624479128 were IN_PROGRESS at last poll. This follow-up requires fresh exact-head checks; no XY-C1 L3 or physical L4 claim is made.

---

## 2026-11-25 22:12 CST (UTC+8) — Correct M2U rollback-order checker symbol

- Exact-head WAN route #45624201337 on `fb86393c` passed the ASan/UBSan policy test binaries and failed only in the Python source checker: the declaration was named `m2u_restore_second`, but the ordering assertion referenced undefined `m2u_second_restore`.
- Corrected the identifier in the assertion. Wireless radio #45624201448 passed on the same `fb86393c` after the missing `<assert.h>` include. Exact-head WAN route and XY-C1 still need a green run on the next code-bearing SHA.

---

## 2026-11-25 22:08 CST (UTC+8) — Add assert declaration to bridge M2U regression

- Exact-head WAN route #45623857230 and wireless radio #45623856807 on `d9d23fe` both failed at compile time because new bridge M2U regression cases used `assert()` without including `<assert.h>`; `-Werror=implicit-function-declaration` stopped compilation. The existing bridge tests use manual expect helpers and did not previously need this header.
- Add the standard assertion header to the bridge membership test. No production logic changed. Both workflows should rerun on the corrected SHA; exact XY-C1 #45623856954 was still running at last poll.

---

## 2026-11-25 22:02 CST (UTC+8) — Make bridge M2U sysfs updates transactional

- Found a partial-update bug in `brport_set_m2u()`: it writes `multicast_fast_leave` then `multicast_to_unicast`; the second sysfs write can fail after the first has changed, leaving a partially applied M2U setting while returning an error.
- Added a checked sysfs reader for both bridge-port flags and a policy transaction that snapshots both values before any write. It tracks attempted writes, rolls them back in reverse order after a failure, preserves the primary errno, and separately reports the first rollback failure. Production logs an incomplete rollback through syslog.
- Added GNU89 regression coverage for snapshot-before-mutation, second-write failure, rollback failure/primary errno preservation, snapshot failure with no writes and idempotent target state. Existing WAN route workflow already compiles/runs the bridge membership policy fixture and invokes the VLAN/bridge source checker.

---

## 2026-11-25 21:51 CST (UTC+8) — Fix VLAN checker local-name collision

- Exact WAN route run #45622867970 on `134638e4` passed all sanitizer test binaries, including the corrected VLAN rollback test. The Python source checker then failed because an older section reused the name `create_vlan` for an integer offset, shadowing the function-body string expected by the new assertion.
- The appended assertions now use a distinct `vlan_create_body` variable from `body(net, ...)`. No product source or runtime test changed; exact-head route rerun required.

---

## 2026-11-25 21:48 CST (UTC+8) — Correct VLAN rollback mock capacity

- Exact WAN route run #45622569505 on `64bd4201` compiled the policy fixtures, but UBSan caught the new VLAN restore mock's pointer array as length 3 while the successful rollback path records 4 operations (down, MTU, MAC, final up). No production code was reached by this fixture failure.
- Corrected the mock array to four slots, recorder bound to four, and final restore-order assertion to index 3. Re-run the complete exact-head WAN route workflow; the source checker follows the sanitizer fixture.
- XY-C1 #45622569603 was still in progress on `64bd4201`; the correction commit triggers a fresh target build.

---

## 2026-11-25 21:43 CST (UTC+8) — Make reused VLAN rollback compatible with target 4.4 VLAN ioctl

- Kernel-source inspection found this repository's Linux 4.4 `vlan_ioctl_handler()` does not implement `GET_VLAN_EGRESS_PRIORITY_CMD` (it returns `-EOPNOTSUPP`). A snapshot design relying on that ioctl would make every existing VLAN with requested QoS fail closed.
- Reworked the transaction to snapshot only supported MTU and hardware address status readers plus the original IFF_UP bit. The egress setter is now deliberately the final configuration step. In this kernel, `vlan_dev_set_egress_priority()` changes an existing mapping and returns success, or for a new mapping allocates first and returns `-ENOBUFS` before publishing on failure; thus a failed setter leaves the previous mapping unchanged and, being last, there is no subsequent failure that needs a GET/restore operation.
- The policy rollback restores down state, requested MTU/MAC, then original admin-up state. Tests and semantic checker are adjusted to verify the target kernel's missing GET API and setter atomicity. Exact-head WAN route sanitizer workflow pending.

---

## 2026-11-25 — Fix VLAN rollback test compile error
- WAN route status #45621857629 failed at the VLAN policy test compile because it used strcmp() without declaring it. Added the missing <string.h> include; source logic unchanged.

---

## 2026-11-25 21:34 CST (UTC+8) — Restore reused VLAN attributes after failed setup

- Found a real rollback defect in `create_vlan_iface_internal()`: a pre-existing VLAN was taken down and reconfigured (egress priority, MTU, MAC), but the failure path only removed newly created VLANs. A reused VLAN could remain down or partially changed.
- Added checked snapshots for IFF_UP, requested MTU/MAC and skb-priority-0 egress QoS before mutation. On failure, rollback takes the reused VLAN down, restores captured attributes, and restores original up/down state. It attempts all rollback operations, logs first rollback error, and preserves primary setup errno. New VLAN deletion/ownership behavior is unchanged.
- Added GNU89 policy regression for restore order, continued attempts after errors and skipped untouched settings; extended WAN VLAN source checker. WAN route workflow already filters the source, shared netutils, VLAN policy/test and checker. Exact-head checks pending; no physical-router L4 claim.

---

## 2026-11-25 21:21 CST (UTC+8) — Fix DHCPv6 startup-cleanup checker assertion

- Exact-head WAN DHCP callback #45620059391 on `28a6bb0` passed the UBSan/ASan policy binaries and all preceding WAN/IPv6 checks, then failed only at the new `check_wan6_dns_policy.py` source assertion. The checker incorrectly searched for the helper's formal parameter names in the call site; production correctly calls it with `start_status, env_cleanup_status`.
- The assertion now inspects the helper body and pins the actual `start_status == 0 && cleanup_status != 0` decision. No production code or test behavior changed. Rerun required on current SHA.
- IA publication #45620275164 and AI docs #45620275432 PASS on parent `94c75e4`. Exact-head callback check remained unverified until this rerun.

---

## 2026-11-25 21:18 CST (UTC+8) — Correct DHCPv6 IA output contract

- Exact-head IA workflow #45619758139 on `6a1faf201e57eab3e9757cc845c15a779b8e8baa` passed the GNU89 ASan/UBSan regression, including failed-prefix-delete retention and retry. The Python source checker then failed only because it still required the old `DHCPv6 IA delegated-prefix retry policy: PASS` banner; the test now correctly emits `DHCPv6 IA delegated-prefix retry and removal policy: PASS`.
- Removed only that obsolete duplicate banner assertion. The checker still pins both new cleanup regressions and the new exact output banner. Exact-head rerun on the current commit is required; no production C changes in this checker-only correction.

---

## 2026-11-25 21:14 CST (UTC+8) — Stop DHCPv6 client after failed environment cleanup

- Base SHA: `6a1faf201e57eab3e9757cc845c15a779b8e8baa`.
- Found a resource/state leak in `start_dhcp6c()`: `eval("/sbin/dhcp6c", ...)` could launch the background client successfully, then `dhcp6c_callback_environment_clear()` could fail. The function converted this to startup failure and advanced the callback generation, but never stopped the already-running `dhcp6c`; callers could roll back IPv6 setup while the client remained alive.
- Added `dhcp6c_callback_start_requires_stop()` plus regression cases to distinguish failed launch from successful launch + cleanup failure. On that latter path, `start_dhcp6c()` now advances the generation then invokes the same bounded `kill_services("dhcp6c")` stop under the held session lock, logs stop failure, and only then unlocks. A failed launch still avoids unnecessary stop.
- Extended the existing DHCPv6 semantic source checker. Workflow `wan-dhcp-callback-stability.yml` already tracks the modified source, policy, test and checker. Exact-head CI required; no live-router test claimed.

---

## 2026-11-25 — Retain failed DHCPv6 address removals for retry
- remove_iaprefix() previously discarded IA_PD target records even when kernel deletion failed; IA_NA failures were ignored by cleanup callers. pref_cleanup() also unlinked the same entry twice. Stale IPv6 addresses could remain installed without retry state.
- Cleanup now retains failed targets, schedules bounded retry, keeps the IA/controller allocated until all deletes succeed, and resumes pending IA cleanup after each prefix timer. Already-absent deletes are idempotent; EEXIST is accepted only for ADD.
- Added host errno-classification/selective-removal regression and production source-contract checks. Exact-head Actions and XY-C1 Build/TRX verification pending.

---

## 2026-11-25 20:59 CST (UTC+8) — Clamp expired udhcpc select waits

- Base SHA: `aa1bdf567d3100f5e773bdd9b41235cc85346bbc`.
- Confirmed a signed/unsigned arithmetic defect in the BusyBox udhcpc main loop: after select() returns EINTR, `already_waited_sec` is increased. If it reaches or exceeds the signed `timeout`, `timeout - already_waited_sec` underwent unsigned subtraction and could produce a huge positive wait instead of processing the expired DHCP transition.
- Added `udhcpc_wait_remaining_seconds()`, which saturates expired/non-positive budgets at zero before subtraction, and routed `timeval.tv_sec` through it. Added GNU89 regression boundaries for elapsed less than/equal to/greater than timeout, UINT_MAX-like elapsed input, negative/zero timeouts and INT_MAX.
- Updated the existing source contract. The existing udhcpc workflow already filters the changed source, policy header, test and checker; the XY-C1 workflow already filters the source and policy header.
- Local isolated GNU89 ASan/UBSan policy smoke test passed. Full repository regression and target build are delegated to exact-head Actions; no complete-repository local test or firmware result is claimed.
- Pre-change current-main evidence: DHCPv6 IA #45618045032 PASS and AI handoff #45618045228 PASS on `aa1bdf5`; XY-C1 #45617826332 remained in progress on `a319ffe`. New exact-head udhcpc and XY-C1 workflows required.
- Resulting source/test/checker/docs commit: THIS COMMIT. Physical-router L4 remains unverified.

---

## 2026-11-25 — Resolve DHCPv6 checker definition lookup
- The latest IA workflow's GNU89 ASan/UBSan regression passed, but its static checker matched the client6_recvreply forward declaration. It now anchors on the full definition signature, consistent with the production callback checker.
- Exact-head AI docs, DHCPv6 script retry and WAN callback workflows passed after the compact roadmap and legacy declaration fixes. XY-C1 Build/TRX and NVRAM audit remain in progress.

---

## 2026-11-25 — Correct DHCPv6 regression contracts
- First exact-head iteration exposed checker-only failures after the GNU89 ASan/UBSan IA test passed: helper-definition lookup and an incorrect Reply-state anchor. The existing DHCPv6 script checker also depends on the original local declaration; retain it and declare IA status separately.
- AI handoff validation caught the 128 KiB roadmap hard limit. Replaced the oversized status paragraph with a compact entry. Re-run exact-head Actions after this correction.

---

## 2026-11-25 — Retry failed DHCPv6 IA kernel publication

- DHCPv6 IA_NA address-add and IA_PD downstream-prefix-add failures were logged and swallowed by update_ia(); the Reply was then considered handled and the configuration callback could run.
- IA_PD installation was gated on first creation and ignored add_ifprefix() status. If one downstream interface failed after another succeeded, later Replies skipped the missing interface permanently.
- update_ia() now returns the first kernel-configuration error. client6_recvreply() keeps the exchange alive and retries the same Reply after the existing bounded 5-second delay before running the callback. IA_PD reconciles every configured downstream interface on each Reply, skipping successful entries and propagating missing-target failures.
- Added a GNU89 ASan/UBSan partial-failure/selective-retry/idempotence regression, production-source checker and dedicated Actions workflow. Local host regression passed; exact-SHA Actions and XY-C1 Build/TRX/artifact verification are pending.
- Added dhcp6c_ia.c and its policy header to the XY-C1 firmware path filter, which previously did not rebuild the IA implementation when changed.

---

## 2026-11-25 — Point U2EC send assertions at bounded implementation

- Exact U2EC run #45610564863 on `e258dbf3` compiled and passed all GNU89 ASan/UBSan socketpair tests, including the new non-reading-peer timeout case. The production source checker then failed three legacy assertions because `send_policy` now correctly represents the compatibility wrapper, while `MSG_NOSIGNAL`, EINTR handling and the sent-byte return are in the new `send_timeout_policy` helper.
- Moved those three assertions to the bounded helper. Runtime code and tests are unchanged.
- Exact-head U2EC, storage, AI handoff and XY-C1 build/TRX/artifact checks required.

---
## 2026-11-25 — Bound U2EC client TCP sends as well as reads

- The main U2EC loop also used blocking `u2ec_send_all_no_sigpipe()` for client greetings and responses. A peer that stopped reading could fill the TCP send buffer and stall the same sole event loop, even after receive-side deadlines were added.
- Replaced the unbounded send loop with select-driven nonblocking sends under a 10-second absolute deadline. The compatibility wrapper retains the existing API and `MSG_NOSIGNAL` behavior; timeout and EPIPE remain ordinary errors handled by the existing disconnect path.
- Added successful socketpair delivery and a non-reading-peer send-buffer exhaustion regression; source-contract checks require all sends through the bounded wrapper. Exact U2EC ASan/UBSan/source-contract and XY-C1 validation pending.

---
## 2026-11-25 — Trigger MTD fill regressions for pull requests

- Exact USB-storage run #45609688415 on `eed1e197` passed the host-shell and BusyBox ash fill regressions and surrounding source contracts, but the workflow checker correctly found that `mtd_storage_fill_safe_paths.sh` was listed under `push.paths` and omitted from `pull_request.paths`.
- Added the regression script to the pull-request path filter beside the other MTD lifecycle tests. PRs changing the test or MTD fill contract will now trigger the same storage stability workflow.
- Exact-head storage and AI handoff workflows must be green before this slice is complete.

---
## 2026-11-25 — Fix storage checker filter scope and rotate log window

- Exact USB-storage run #45608745837 on `fa7d05e5` passed shell syntax, host-shell and BusyBox ash MTD fill regressions, then the source checker raised `NameError` because its early MTD contract referenced `push_filters`/`pull_filters` before the workflow-filter variables were defined. Moved this path assertion into the existing later push/pull filter inventory.
- The AI handoff validator also reported 121 root log entries. Archived the 101 oldest complete records byte-for-byte, chronological oldest-to-newest, and retained the newest 20 before this correction entry. The pre-rotation root blob SHA and archive shard are indexed.
- Exact-head storage and AI handoff reruns required. Product source was unchanged in this checker/governance correction.

---
## 2026-11-25 — Align U2EC framing source contract with bounded reads

- Exact U2EC run #45608574671 on `0c505e0b` compiled the GNU89 policy test with ASan/UBSan and passed both complete-field and partial-field timeout regressions. The final source checker failed only because three pre-existing assertions still required the old unbounded `u2ec_recv_all()` call sites in `usb_connection()`.
- Updated the production-source contract and read-order assertion to require `u2ec_recv_all_timeout()` for both the fixed header and IRP body. No runtime source changed in this follow-up.
- Exact-head U2EC, storage, and XY-C1 checks remain required.

---
## 2026-11-25 — Fix literal backslash count in chnroute source contract

- Exact USB-storage run #45608329431 on `14ec3f27` passed runtime MTD fill regressions but the checker-only chnroute rewrite assertion failed: the test shell source contains one backslash before each escaped quote, while the Python raw-string expectation contained two.
- Corrected the raw-string expectation to one literal backslash. This calibrates only the source contract; MTD runtime source and fixtures are unchanged.
- Exact-head storage rerun required.

---
## 2026-11-25 — Bound U2EC client TCP field reads

- `usb_connection()` runs synchronously in the sole U2EC event loop. Its blocking `u2ec_recv_all()` calls could wait indefinitely after a peer sent only part of the fixed TCP header or advertised IRP body, preventing other printer clients, FIFO events and timeout processing from running.
- Added a 10-second absolute deadline per incoming TCP field. Readiness waits use the remaining deadline and nonblocking recv, so partial progress does not reset the total budget; timeout returns `ETIMEDOUT` and the existing close path removes the stalled connection. Header and IRP-body reads use the bounded helper; discovery worker reads retain their separate one-second socket timeout.
- Added successful complete-field and stalled-after-partial-field socketpair regressions plus source-contract checks for the main event-loop call sites. Exact U2EC host ASan/UBSan/source-contract and XY-C1 Build/TRX/artifact verification pending; no device result claimed.

---
## 2026-11-25 — Match quoted chnroute fixture rewrite literally

- Exact storage run #45607584405 on `0cc1703c` passed shell syntax, host-shell and BusyBox ash MTD fill regressions, checked caller graph and preceding assertions. The final static checker failed because the sed substitution in the shell fixture escapes its inner double quotes, while the Python source contract expected the unescaped form.
- Updated the contract to match the literal escaped shell text. No production source or runtime test behavior changed.
- Exact-head storage workflow rerun required.

---

## 2026-11-25 — Align chnroute source contract with checked directory helper

- Exact USB-storage run #45607146427 on `72f9d8fa` passed shell syntax validation and both host-shell and BusyBox ash MTD fill regressions, including injected write/chmod/rename failures. The static checker then failed on a stale assertion that still expected raw `mkdir -p`; production code now correctly uses `func_fill_mkdir "$dir_chnroute" 755` to validate the path and propagate directory-creation errors.
- Updated the source contract to require the checked helper while retaining assertions for archive extraction failure cleanup. No production source or test behavior changed.
- Exact-head storage workflow rerun required.

---

## 2026-11-25 — Correct MTD fill function source-contract delimiter

- Exact USB-storage run #45606836538 on `da508394` passed the MTD shell syntax and host/BusyBox runtime regressions, then the static checker failed because its Python string searched for the literal characters `\\ncase` rather than the actual function boundary `\n}\n\ncase "$1" in`.
- Corrected the delimiter so the checker slices the complete `func_fill()` body through its closing brace. Production MTD behavior and test fixtures are unchanged.
- Exact-head USB storage rerun is required; the earlier checker failure is a test-contract error, not a firmware/runtime failure.

---

## 2026-11-25 20:40 CST — Include the string API used by SOAPAction tests

- Exact miniupnpd HTTP request framing run #45606537043 on `1f5e34e7` failed before executing tests: the new quoted-value assertion used `memcmp()` without including `<string.h>`, and the workflow treats implicit declarations as errors under GNU89 `-Werror`.
- Added the standard declaration include to the test translation unit. The parser change itself has not yet completed CI validation; exact-head build/test rerun required.

---

## 2026-11-25 20:35 CST — Extract MTD fill source without parsing heredoc bodies as C

- Exact USB-storage run #45605993018 on `1c62feb6` passed both host-shell and BusyBox ash MTD save/restore/load/erase/backup/fill regressions, including all eight injected fill failures and the normal success case. The final static checker failed when the C-style `body()` scanner tried to parse the shell `func_fill()` function: heredoc payloads contain shell braces and quoting that are not C source, so the scanner could not find a balanced closing brace.
- Replaced only the `func_fill()` extraction with its stable shell boundary: the function starts at `func_fill()` and ends at the top-level `case "$1" in` command dispatcher. The smaller helper functions continue using the shared brace parser. Runtime MTD source and tests are unchanged.
- Exact storage rerun required; `1c62feb6` is FAIL solely at the static checker after the runtime regression steps passed. No local or hardware result is claimed.

---

## 2026-11-25 20:30 CST — Reject malformed quoted MiniUPnP SOAPAction values

- `ParseHttpHeaders()` stripped surrounding quotes by testing `p[n-1]` after finding a value length. A one-byte value consisting only of a quote satisfied both ends of the check and reduced the signed SOAPAction length from 1 to -1; downstream `%.*s` logging then treated the negative precision as unbounded. The parser also retained trailing whitespace and scanned the line without an explicit header-bound check.
- Added a length-safe SOAPAction normalizer that trims optional edge whitespace, rejects unbalanced/interior quotes and embedded control bytes, and returns an offset/length pair without modifying outputs on failure. The production parser now bounds the value scan by `req_contentoff`; Content-Length scanning is bounded the same way. Added GNU89-compatible boundary tests for one-byte quotes, mismatched quotes, empty quoted value, whitespace and control characters.
- Exact MiniUPnP framing Actions required. No local build or hardware result is claimed.

---

## 2026-11-25 20:35 CST — Publish MTD default files atomically

- The latest MTD fill guard now detected failed heredoc writes and chmod errors, but wrote directly to live config paths before checking status. A partial file remained at its final name after ENOSPC/I/O failure; subsequent fills would skip it because the pathname already existed.
- Added same-directory temporary staging for generated scripts and configuration, verified the staged file's type/mode, then published with no-clobber rename. Failures remove the stage and propagate to reset/fill callers. DNSMasq base content and all optional overlays publish as one file; existing user config is not replaced.
- Extended host-shell and BusyBox ash regressions to inject content-write, chmod and rename failures and verify no partial target/stage remains. Exact USB-storage and XY-C1 Build/TRX/artifact workflows required; no local hardware result is claimed.

---

## 2026-11-25 20:25 CST — Teach the storage checker to parse shell parameter expansions

- Exact USB-storage run #45605321376 on `6dd7052c` passed the runtime restore/load/erase/backup tests and the pre-existing full-caller-graph contract, then the static checker failed while extracting `func_fill_check_file()`. Its C-style brace scanner interpreted the shell parameter expansion `${target%/*}` as the start of a C block comment and reached EOF without a closing delimiter.
- Extended the shared function-body extractor to skip nested `${...}` shell parameter expansions, and added a focused parser regression containing the slash-star pattern. This is a checker-only correction; MTD production/test behavior is unchanged.
- The exact `6dd7052c` storage run is FAIL due to checker parser incompatibility; exact-head rerun required. No local test or firmware result is claimed.

---

## 2026-11-25 20:20 CST — Fail MTD fill on default-file write or mode errors

- The 23 default-file heredoc writes and their `chmod` operations were unchecked. A full filesystem, read-only transition or write error could leave an empty/partial configuration file or a non-executable hook while `func_fill()` continued and reset started services.
- Added immediate status checks after every default-file `cat >`/`cat >>`, plus a common file-type/symlink/mode helper that propagates each chmod error. Added host/BusyBox ash injection for heredoc write and mode-setting failures; static checker pins all 23 heredoc status gates and representative mode operations.
- Exact USB storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 20:15 CST — Isolate clean fill from the corrupt archive fixture

- Exact USB-storage run #45604771628 on `13f9f431` passed restore/load/erase/backup, the four path-symlink denials, the injected mkdir failure, the reset_ss failure case, and the chnroute partial-extraction failure. The final positive-path assertion failed because the negative chnroute test deliberately left a plain-text invalid archive at the fixture path; the subsequent clean-fill case reused it, so real `tar jxf` correctly rejected it. The production source and mkdir wrapper were not at fault.
- The successful-fill fixture now removes the deliberately corrupt archive before running against a clean storage tree. Production source is unchanged.
- Exact storage run is FAIL due to test-state leakage between scenarios; new exact-head validation required. No local test or hardware result is claimed.

---

## 2026-11-25 20:10 CST — Point the chnroute failure fixture at its test archive

- Exact USB-storage run #45604235661 on `5dd0dcf0` passed restore/load/erase/backup and the preceding fill symlink and mkdir-failure cases, but the injected chnroute failure case unexpectedly succeeded because the test's sed pattern started at column zero while the production `chnroute_file` assignment is indented inside `func_fill()`. The test therefore left the production path `/etc_ro/chnroute.bz2` unchanged and skipped extraction.
- Replaced the fixture with an exact assignment-value substitution that preserves the indentation, and pinned the redirection in the source contract. Production code is unchanged.
- Exact storage and XY-C1 validation required; the 5dd0dcf storage run is FAIL due to fixture path injection, not a successful archive-failure test. No local test result is claimed.

---

## 2026-11-25 20:05 CST — Verify the safe default-fill success path

- The new fill regression already exercised four fail-closed cases; it lacked a clean-tree success case, so an overly strict path preflight or a shell portability issue could have made every fill fail while all negative assertions passed. The reset_ss fixture now publishes a valid start script on success, and the test verifies the core generated scripts and dnsmasq/inadyn/WLAN defaults under both host shell and BusyBox ash.
- Test-only expansion; the storage workflow and source contract are unchanged apart from requiring the positive-path assertion. Exact storage run required; no local test result is claimed.

---

## 2026-11-25 20:00 CST — Require successful reset_ss publication during fill

- Default fill previously invoked `reset_ss.sh -a` without checking its result or confirming that the expected `start_script.sh` was published. A write/rename failure could be hidden while fill returned success and reset proceeded to service startup.
- Fill now stops on a failing generator and independently verifies that the resulting start script is a regular non-symlink file. Added a host/BusyBox ash failure-injection case and static source-contract assertions.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:55 CST — Propagate required MTD fill directory-creation failures

- MTD default fill ignored several `mkdir -p` failures for cron, HTTPS, dnsmasq, inadyn, WLAN, OpenVPN and strongSwan state. The function could continue with an incomplete configuration tree and let reset proceed to application startup. Added a checked mkdir helper that revalidates the path, preserves existing-directory modes, logs failures, and aborts fill; migrated all required default directories to it.
- Extended the host/BusyBox ash fill regression with injected failure at the first required cron directory, asserting nonzero fill status and that reset-script generation never begins. Static source contract covers the directory call sites.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:50 CST — Propagate MTD fill failures to the command exit status

- The new path guard correctly returns failure from `func_fill()`, but the command dispatcher previously invoked it and then exited the script with the unchanged global `result=0`. Direct `mtd_storage.sh fill` callers therefore saw success even when a protected symlink or archive extraction failure aborted fill. The reset path already propagates the function result through its conditional.
- The `fill)` dispatcher now records a nonzero result when `func_fill` fails; the static contract pins this status propagation. The existing production-linked host/BusyBox fill regression checks that unsafe paths and partial chnroute extraction return nonzero.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:45 CST — Preserve shell parameter expansion in fill path guards

- Exact USB-storage run #45603292894 on `dfee708a` passed the earlier MTD restore/load/erase/backup regressions, then failed when the new fill-path test tried to create its sandbox: literal backslashes had been retained before `${TMPDIR}` and `${BUSYBOX_BIN}` in the generated shell fixture. The source helper's `${target}`/parent/component parameter expansions had the same quoting defect and would not walk path components correctly.
- Removed the unintended backslashes from the three production shell expansions and both test expansions; strengthened the static source contract to pin the parameter expansions themselves. Exact storage and XY-C1 validation required.
- The prior exact-head storage run is FAIL due to fixture/bootstrap expansion, not a passing result for the new fill guard. No local test or firmware result is claimed.

---

## 2026-11-25 19:40 CST — Make bundled chnroute extraction fail closed

- The MTD default-fill path used `mkdir -p ... && tar jxf ...` inside a conditional but discarded the compound command's failure. A corrupt bundled chnroute archive could leave a partial `/etc/storage/chinadns` directory; subsequent boots would see the directory and never retry extraction.
- Fill now checks directory creation, checks archive extraction, removes partial output on extraction failure (logs a critical cleanup failure if necessary), and returns failure so reset retains its save barrier and does not restart apps. Extended the host/BusyBox ash fill regression with injected partial extraction and cleanup assertions.
- Exact USB storage, static source contract and XY-C1 build verification required. No local BusyBox/firmware/hardware result is claimed.

---

