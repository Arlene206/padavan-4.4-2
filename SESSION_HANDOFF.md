# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`; last verified remote inventory contained only `main`, no checkpoint branch.
- Base main SHA for THIS COMMIT: `947ee69cfbda917ec1728cdde39a00b6a7726eeb`; parent `8ec8198c4564e57cd3956ea675e343bc35098e72`. Resolve the resulting SHA from live `main` after publication.
- Bootstrap recovery completed in order: `AI_BOOTSTRAP_PROMPT.md`, `AI_DEVELOPMENT_PROTOCOL.md`, live main ref, this handoff, roadmap, newest root dev-log records, archive index, branch inventory and exact-SHA Actions.
- Root `dev_log.md`: 102 complete newest-first records before THIS COMMIT, 103 after. Current archive inventory: dev-log=6, handoff=22, roadmap=18. No rotation/index change is due.

## Closed task — NVRAM HDR2 profile-save atomic publication

- Commit `ce7efad34dbd81097d923f9568f0ae9946694b58`, parent `534e4e38ed5e729af845d7ad69bb84d8b96a5c47`.
- Exact-head HTTPD security [42992756073] and AI docs [42992755926] both SUCCESS on that SHA. All 39 HTTPD job steps passed.
- Logs verified `NVRAM profile policy: PASS`, `NVRAM profile atomic publication: PASS`, `NVRAM CLI assignment policy: PASS`, RC restore persistence and private HTTPD export caller path.
- Same-directory mode-0600 staging, complete short-write/EINTR handling, file fsync, checked close and atomic rename preserve last-good backup on pre-publication failure. Parent directory is not fsynced; directory-entry power-loss durability is not claimed.
- Production MIPS/uClibc object, full firmware/TRX and L4 remain NOT VERIFIED.

## Closed task — networkmap signal-context safety

- Production fix commit `a73ca2221712f16dd5aa4bab450eed18a6cd3f92`; handoff/source-contract correction commit `8ec8198c4564e57cd3956ea675e343bc35098e72`.
- On exact current head `8ec8198c4564e57cd3956ea675e343bc35098e72`, networkmap workflow [42996232222] and AI docs [42996232115] both completed SUCCESS. All job steps and full logs were inspected.
- The policy test printed `networkmap signal deferral policy: PASS`; checker printed `networkmap signal deferral and lifecycle source contracts: PASS`, including handler registration before daemonization and event-loop startup. Docs validator printed root log=102/119, batch=100, retain=20, archives dev-log=6, handoff=22, roadmap=18.
- Previous docs run [42995791241] failed on parent SHA `a73ca222...` because the handoff heading had been renamed. Exact required headings were restored and the exact-head rerun above passed.
- SIGUSR1/SIGTERM handlers now only record `sig_atomic_t` flags; refresh state, scan counters and cleanup execute in normal context. SIGUSR1 consumption preserves the caller's preexisting signal mask.
- The RT3352 MIPS/uClibc networkmap object, complete firmware/TRX and live LAN discovery/termination remain NOT VERIFIED.

## Active task — ASUS infosvr UDP packet initialization and bounds (P2 reliability/security)

### Evidence

- Daemon: `trunk/user/infosvr/src/infosvr.c`; metadata readers in `asus.c`; wire structures in `trunk/user/shared/src/include/ibox.h`.
- The old `read_packet(int fd, void *packet, int size)` used `memset(packet, 0, sizeof(*packet))`. GNU89 accepts `sizeof(void)` as a one-byte extension, so only byte zero of the 512-byte stack receive buffer was initialized. For shorter UDP packets, the rest of the stack buffer retained prior contents.
- The function returned `res == size` (0 or 1), but main checked only `< 0`; short/truncated datagrams were therefore still dispatched. `process_packet()` read the base IBOX header and, for other opcodes, the extended MAC prefix without receiving/validating the actual datagram length.
- The extended prefix consumed by code through `MacAddress[6]` ends at `offsetof(IBOX_COMM_PKT_HDR_EX, Password)` (14 bytes for the current wire struct). GETINFO / GETINFO_MANU require only the base `IBOX_COMM_PKT_HDR`.
- The non-basic opcode path also compared the request MAC with an uninitialized local array if NVRAM MAC was absent or `ether_atoe()` rejected its value.

### Fix and regression

- THIS COMMIT zeroes the entire caller-specified receive buffer; returns actual `recvfrom()` byte count/error; drops datagrams shorter than the base IBOX header; passes the received length into `process_packet()`; checks the extended prefix before accessing MAC fields; and checks local NVRAM MAC conversion before comparison.
- Added `infosvr_packet_policy.h`, a GNU89 regression, a source/ABI contract checker and a dedicated push/PR workflow. Test covers complete-buffer zeroing, short-packet zero tail, NULL/zero-size error, truncated/full base header and truncated/full extended MAC prefix.
- Local isolated `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` test passed: `infosvr packet initialization and length policy: PASS`.
- L1 source review: PASS for receive initialization, byte-count propagation and packet bounds.
## Verification boundary

- Local isolated GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` regression passed with leak detection: `infosvr packet initialization and length policy: PASS`.
- Exact-head infosvr workflow [42997353568] completed SUCCESS on base SHA `947ee69cfbda917ec1728cdde39a00b6a7726eeb`; full logs show both packet initialization/length test and UDP receive/source contract PASS.
- AI docs run [42997352476] FAILED on that base because this handoff draft omitted the validator-required `## Verification boundary` heading. This is a handoff-contract issue, not a source/test failure. THIS COMMIT restores the exact heading and adds a regression assertion/path filter for all required handoff headings; both workflows rerun on the resulting exact head.
- Infosvr production MIPS/uClibc object, complete firmware/TRX and L4 packet interoperability remain NOT VERIFIED. The old SIGTERM handler still performs cleanup/exit in signal context; track separately rather than mixing it into this bounded packet parsing fix.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 `fstatat()` then `unlinkat()` is not inode-conditional. Do not patch without a proved staged-publication design preserving STOR/APPE/REST/STOU, permissions and filesystem behavior.
2. NFS `async` durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require evidence from real clients.
3. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
4. Wireless bulk detach / bridge ownership across board variants — target behavior NOT VERIFIED.
5. Optware operation-lock / stop / unmount under BusyBox ash — target behavior NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior XY-C1 object evidence is not a full firmware build.
7. All complete MIPS/uClibc firmware builds, TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` SHA, parent and branch inventory. Confirm THIS COMMIT is still the live head before relying on this handoff.
2. Query Actions for that exact SHA. Inspect every step and complete logs for `infosvr packet receive stability regression` and `AI handoff documentation contract`. Require the GNU89 ASan/UBSan test, receive/parser/ABI source contract and docs archive validator to pass on the same SHA. Docs should report root log=103/119 and archive counts 6/22/18.
3. If anything fails, inspect the full failing log and correct the source/test/contract before starting another task.
4. If green, close this bounded infosvr packet parsing slice. Rotate to a different under-covered user-space module; do not immediately return to networkmap, NVRAM, HTTPD or SkipDBv2. Keep the FTP TOCTOU and NFS external-client evidence gates explicit.

## Documentation and verification rules

- `dev_log.md` is newest-first. Rotate oldest 100 only when a new entry would reach 120 records or the file passes 384 KiB; 512 KiB is the hard cap. Preserve archive shards byte-for-byte.
- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy files. Update in place for material task changes; do not create routine snapshot archives.
- Do not create follow-up docs-only commits solely to copy Actions results into these files; exact-head Actions is the verification source of truth.
- L1 = source review; L2 = host tests/Actions; L3 = target MIPS/uClibc objects and complete firmware; L4 = real router behavior. Never elevate evidence between levels.
