# Padavan 4.4 — Live session handoff

## Live source state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`; remote branch inventory before THIS COMMIT: `main` only, no remote checkpoint branch.
- Base main SHA for THIS COMMIT: `ce7efad34dbd81097d923f9568f0ae9946694b58` (parent `534e4e38ed5e729af845d7ad69bb84d8b96a5c47`). Resolve the resulting SHA from the live branch after publication.
- Bootstrap recovery completed in order: root `AI_BOOTSTRAP_PROMPT.md`, root `AI_DEVELOPMENT_PROTOCOL.md`, exact main ref, this handoff, roadmap, newest root dev-log entries, archive index, branches and exact-SHA Actions.
- Root `dev_log.md` had 100/119 records before THIS COMMIT and has 101 afterward. Current archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index edit is due.

## Closed immediately before this slice — atomic NVRAM profile save

- Commit `ce7efad34dbd81097d923f9568f0ae9946694b58`, parent `534e4e38ed5e729af845d7ad69bb84d8b96a5c47`.
- Exact-head HTTPD security run [42992756073](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42992756073) completed SUCCESS; AI handoff docs [42992755926](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42992755926) completed SUCCESS on the same SHA.
- All 39 HTTPD job steps passed. The complete log shows `NVRAM profile policy: PASS`, `NVRAM profile atomic publication: PASS`, `NVRAM CLI assignment policy: PASS`, RC NVRAM restore persistence, wireless atomic config, switch-status, NFS inventory and remaining HTTPD umbrella contracts all PASS. Atomic writer contract passed for private HTTPD export path and HDR2 output.
- AI docs step passed: root log=100/119, batch=100, retain=20, archive counts dev-log=6, handoff=22, roadmap=18.
- NVRAM atomic profile-save slice is CLOSED at L2 host/Actions. The helper fsyncs file content before rename but does not fsync the parent directory; directory-entry power-loss durability is not claimed. Production MIPS/uClibc object, full firmware/TRX and L4 remain NOT VERIFIED.

## Active task — networkmap signal-side cleanup / scan-state race (P2 reliability)

### Source and call-path evidence

- Daemon source: `trunk/user/networkmap/src/networkmap.c`; companion global `scan_now` declaration is in `function.c`. Build/package path: `trunk/user/networkmap/src/Makefile`, `trunk/user/networkmap/Makefile`, top-level `trunk/user/Makefile`. RC lifecycle: `services.c` start/stop/restart, `rc.h` declarations and `rc.c` restart dispatch.
- Original `sig_exit()` called `clear_resources()` and `exit(0)`. That cleanup closes the ARP socket, releases heap-allocated client nodes, writes NVRAM and removes the PID file. A SIGTERM interrupting allocator, stdio or NVRAM internals could re-enter unsafe process state.
- Original `sig_refresh()` directly mutated `networkmap_fullscan` and `scan_now` while the main loop could concurrently use those variables. `function.c` declares `extern unsigned int scan_now`, while the definition used `volatile unsigned int`; the patch removes unnecessary volatility once the handler no longer owns the value.
- THIS COMMIT makes both handlers record only `volatile sig_atomic_t` events using a macro that expands to direct assignments (no user helper function called in signal context). SIGUSR1 is temporarily blocked while the main loop snapshots and clears its flag, then the original signal mask is restored; a newly pending refresh is delivered for the next iteration. Scan variables and cleanup remain in ordinary process context.
- Shutdown observes `daemon_exit` in the existing ARP receive loop. `nmap_init()` sets `SO_RCVTIMEO` to 60ms for a bounded small subnet scan or 2 seconds otherwise; full-scan completion also sets the two-second timeout. The signal does not invoke cleanup directly.
- Added policy header/test, static source/caller/build contract and dedicated workflow. Push and PR filters watch all production dependencies, RC lifecycle wiring, tests, checker and workflow.

### Validation state

- Local GNU89 compile: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`; binary run with `ASAN_OPTIONS=detect_leaks=1`: PASS, output `networkmap signal deferral policy: PASS`.
- Test covers SIGUSR1 refresh-only mapping, repeated-request coalescing, SIGTERM exit flag, ignored other signals, single-consumption semantics, preservation of an already-blocked signal mask, pending SIGUSR1 delivery after unmasking, and NULL input rejection.
- L1 source review: PASS for handler purity and normal-context event/cleanup boundary.
- Exact-head networkmap workflow and AI docs: PENDING on THIS COMMIT. No Actions result may be inferred before inspection.
- L3 production MIPS/uClibc networkmap object, full firmware/TRX and L4 network discovery/termination behavior: NOT VERIFIED.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 `fstatat()` followed by `unlinkat()` is not inode-conditional. Do not patch absent an evidence-backed staged-publication approach preserving STOR/APPE/REST/STOU semantics, permissions and filesystem behavior.
2. NFS `async` durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; evidence from real clients required before policy changes.
3. WAN PPP reconnect, callback recovery, IPv6 rollback and nonzero runtime scope — target behavior NOT VERIFIED.
4. Wireless bulk detach / bridge ownership across board variants — target behavior NOT VERIFIED.
5. Optware operation-lock / stop / unmount under BusyBox ash — target behavior NOT VERIFIED.
6. FULLCONENAT/kernel/board coverage — prior XY-C1 object evidence is not a full firmware build.
7. All full-firmware MIPS/uClibc builds, firmware/TRX inspection and physical router flash/boot remain NOT VERIFIED.

## Exact first action for next GPT

1. Fetch live `main` SHA, parent and branch inventory. Confirm THIS COMMIT is still the live main head; never push using the stale base.
2. Query Actions for that exact SHA. Inspect every step and complete logs for `networkmap signal lifecycle stability regression` and `AI handoff documentation contract`. Require the GNU89 ASan/UBSan policy test, the signal/call-path/build/path-filter checker and docs validator to pass on the exact same SHA. Expected docs result: root log=101/119, archive counts dev-log=6, handoff=22, roadmap=18.
3. If any job fails, inspect the complete failing step/log and correct the implementation or contract; do not claim green or mark the task closed until both runs pass.
4. If green, close this bounded networkmap slice and choose the next under-covered userland service from the whole-repository ledger (avoid immediately returning to HTTPD, SkipDBv2, NVRAM or networkmap). Preserve the FTP TOCTOU and NFS external-client evidence gates.

## Documentation and verification rules

- `dev_log.md` is newest-first. Rotate oldest 100 complete records only when a new record would reach 120 or the file passes 384 KiB; 512 KiB is a hard cap. Preserve historical archive shards byte-for-byte.
- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy files; update in place on material state changes only. No new snapshot/archive is appropriate here.
- Do not create a follow-up docs-only commit solely to copy a completed Actions result into these files. Exact-head Actions remains the verification source of truth.
- Validation levels: L1 static/source review; L2 isolated host tests/Actions; L3 production target MIPS/uClibc objects and complete firmware; L4 real-device behavior. Never elevate evidence between levels.
