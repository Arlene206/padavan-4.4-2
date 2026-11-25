## 2026-11-26 04:45 CST (UTC+8) — Make the IAPP driver-signal pipe nonblocking and record-atomic

- `IAPP_USR2Handle()` previously wrote a driver signal context to a blocking pipe, while the pipe reader runs in the same IAPP event-loop thread. A full pipe could therefore block the callback and prevent the reader that would free capacity from running.
- `IAPP_SocketOpen()` now queries the pipe writer's runtime `_PC_PIPE_BUF`, rejects a limit smaller than `sizeof(RT_SIGNAL_STRUC)`, and sets only the write endpoint to `O_NONBLOCK` before registering SIGUSR2. The new policy helper validates the complete record against that limit, retries EINTR, rejects any short write as EIO, and passes EAGAIN through immediately rather than blocking or splitting a record. Pipe teardown clears the cached limit.
- Added GNU89 ASan/UBSan tests for invalid/zero/oversized records, EINTR retry, short-write rejection, EAGAIN propagation and a saturated real nonblocking pipe. Integrated the fixture and its header/source paths into both push and pull_request IAPP lifecycle coverage.
- Local helper regression: GNU89 compile with `-Wall -Wextra -Werror -fsanitize=address,undefined` and leak detection — PASS. Exact-head IAPP/Process/AI/XY-C1 Actions for THIS COMMIT pending at commit creation; no full MIPS or hardware result is claimed.
- Changed paths: `rtmpiapp.c`, `iapp_signal_pipe_policy.h`, `iapp_signal_pipe_policy.c`, `iappd_lifecycle.sh`, `iappd-lifecycle.yml`, canonical handoff/roadmap/log and indexed log archive.

---

## 2026-11-25 — Keep the TCP write failure label portable

- Source review of the bounded write helper caught a preprocessor portability defect before its exact-SHA CI completed: `goto write_fail` is part of the shared sender, but the label had been compiled only under `IAPP_OS_LINUX`. Non-Linux/VxWorks builds would therefore fail with an undefined label. Moved the label outside the platform conditional while keeping only the `fcntl()` restore conditional.
- Added a lifecycle source contract that keeps the shared failure label present independently of Linux-specific flag restoration.

---

## 2026-11-25 — Bound TCP frame write backpressure

- `IAPP_TCP_WriteFrame()` handled partial writes but used a blocking socket. If the peer stopped reading, a send could wait beyond the connect/read deadlines and stall the IAPP event loop.
- On Linux the sender now saves current flags, sets O_NONBLOCK, waits for writability with the same five-second absolute deadline, retries EINTR/EAGAIN/EWOULDBLOCK, and restores the original flags after completion or error. `MSG_NOSIGNAL` remains used when available. Non-Linux targets retain the legacy blocking send loop for compatibility.
- Lifecycle source contracts assert separate nonblocking connect/write setup and the write-side deadline/would-block retry path.

---

## 2026-11-25 — Bound outbound IAPP TCP connect latency

- All four outbound TCP paths called blocking `connect()` directly. A peer that drops packets or never completes a handshake could stall the daemon's event loop even though receive already has an absolute deadline.
- Added `IAPP_TCP_Connect()` for Linux: save file flags, set O_NONBLOCK, initiate connect, wait for writability using the existing five-second absolute-deadline helper, check SO_ERROR, then restore original socket flags before sending. Timeout/error restores flags and fails closed. Non-Linux/VxWorks retains legacy blocking connect for compatibility.
- Routed all four outbound TCP paths through the helper. The lifecycle source contract pins nonblocking mode, deadline wait, SO_ERROR validation and flag restoration.

---

## 2026-11-25 — Handle SIGPIPE for the raw-driver notification pipe

- Follow-up to TCP `MSG_NOSIGNAL`: `IAPP_USR2Handle()` also writes driver signal contexts into `PipeRawDrv`. If the pipe reader has closed, `write()` can deliver process-wide SIGPIPE before the existing short-write/error check runs, terminating the daemon.
- On Linux, the daemon now ignores SIGPIPE before registering the driver's SIGUSR2 handler or publishing its PID to the driver. Pipe writes therefore return EPIPE and use the existing error path; this does not alter VxWorks signal behavior. The IAPP lifecycle source contract pins the Linux guard and registration.

---

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
