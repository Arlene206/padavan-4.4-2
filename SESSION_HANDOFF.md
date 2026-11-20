# 2026-11-20 continuation checkpoint

This live checkpoint supersedes older summaries. Continue from current main and do not repeat completed startup recovery, WAN snapshot, FTP FIFO/APPE, or RADIUS zeroization work.

## Git state

- Current parent main SHA: `573d215607c8e8a4b7e22048c2bbdd633a3a2d5f`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact FIFO-rejection SHA `5c7fa81094c2e31ed58d0e8381201ca769db7a2c`: VSFTPD, Process inventory, AI docs and NVRAM Actions PASS; same-SHA XY-C1 Build/TRX/artifact PASS.
- APPE product-source SHA `8ded66573ba530efd6c66763981e136dc7a1097a`: initial VSFTPD source-contract run failed on multiline whitespace; correction SHA `573d215607c8e8a4b7e22048c2bbdd633a3a2d5f` passes VSFTPD `44734430382` and AI docs `44734430469`. APPE exact-source XY-C1 run `44734275996` was still IN_PROGRESS at last poll.
- RADIUS secret-zeroization source SHA `821c133887352c069067688f442cd5300bd619b2`: Process inventory, Process lifecycle and AI docs PASS; NVRAM audit was cancelled; XY-C1 run `44734074955` was IN_PROGRESS at last poll.
- The latest main head was checked identical to `main` before beginning this MiniUPnP slice. Do not touch the frozen backup.

## Active MiniUPnP work

- Confirmed reliability defect in `trunk/user/miniupnpd/miniupnpd-2.x/daemonize.c::writepidfile()`: direct open did not truncate a longer previous PID, `write()` short/error results were not propagated, and the old path could expose a partially replaced PID after interruption or storage failure.
- New helper `miniupnpd_pidfile_policy.h` stages the PID beside its destination with `mkstemp()`, applies the prior writer's intended 0644 mode, completes short writes and bounded EINTR retries, syncs and checks close, then atomically renames. A failed publish preserves the previous complete PID file; replacing a symlink destination does not follow its target.
- GNU89 host regression exercises shorter replacement, interrupted/one-byte writes, bounded signal storms, ENOSPC and zero-write failure, temp cleanup, and symlink-target preservation. Standalone `-Wall -Wextra -Werror -fsanitize=address,undefined` execution PASS locally.
- The same logical slice updates MiniUPnP's Makefile dependency, both push/PR path filters, workflow host test and source contract. Exact-head Actions and same-source XY-C1 are pending until this commit is pushed.

## Exact first actions

1. Inspect the exact new MiniUPnP stability Action and AI handoff check; repair only causal failures without weakening assertions.
2. Verify XY-C1 on the new product-source SHA through detector, MIPS build, TRX verification and artifact upload. FIFO SHA 5c is L3-verified; APPE 8ded and RADIUS 821 were still pending at last poll.
3. Continue a different userland stability/security candidate while relevant Actions are pending. Do not poll repeatedly or re-audit closed FTP/RADIUS work.
4. Physical router L4 remains NOT VERIFIED.

## Verification boundary

- Host test PASS on the standalone new PID policy/helper with GNU89, warnings-as-errors, ASan and UBSan.
- New MiniUPnP exact-head workflow and XY-C1 result: PENDING until their runs complete on the resulting commit SHA.
- Keep host L2, exact-head Actions, same-source firmware/TRX L3 and device L4 separate.


---

# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Documentation checkpoint base main SHA: `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49` (this handoff/log-rotation commit is its child). The production source baseline remains `ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1`; intervening changes are handoff/workflow contract maintenance.
- WAN DHCP (11-field), virtual-IPTV DHCP (7-field), ZeroConf WAN (4-field), and ZeroConf IPTV (3-field) bound callbacks use a shared serialized NVRAM snapshot publisher with previous-value rollback and network/interface compensation.
- New source commit ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1 adds a length predicate for prior IP/netmask/gateway text before copying snapshot values into fixed INET_ADDRSTRLEN rollback buffers. This prevents a corrupt overlong value such as 255.255.255.255x from being truncated to a different valid address. The same guard is used in all four DHCP/ZeroConf callbacks.
- The commit adds focused tests for empty/exact-fit/overlong-prefix/NULL/zero-capacity cases and an 11-field fake NVRAM write failure: all eleven writes must be attempted and all old fields restored in reverse order after an after-write ENOSPC.
- Exact-SHA results for production source SHA `ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1`: WAN route `44693084041`, HTTPD `44693084106`, AI docs `44693084139`, NVRAM boundary `44693084087`, and other completed host checks passed. WAN callback `44693084363` failed at an obsolete workflow assertion requiring a direct `wan_dhcp_interface_clear(&interface_context)` call after production code had centralized rollback in `dhcp_wan_interface_restore_previous()`; workflow-only fix `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49` checks the shared helper's set/clear branches instead and queued rerun `44693740862`. XY-C1 run `44693084076` was still in progress at last poll; no L3 claim.
- Previous source-equivalent checks: WAN route 44692804478, HTTPD 44692804337, AI docs 44692804357 passed on 5e2c84586842bffc46238d491b042dc1b9ee4741; the WAN callback on that SHA failed on a ZeroConf source-body extraction assertion, corrected by the prototype/call-site-aware parser in ee8ef67.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only. Legacy PR #1 remains CLOSED / NOT MERGED; AliDDNS hardening is already on main.

## Active task

1. Verify exact-head WAN callback rerun `44693740862` for source/workflow base `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49`. If it fails, use the causal log line; the prior exact failure was the stale direct-clear assertion, now replaced with checks on the shared restore policy.
2. Confirm `dhcp_wan_interface_restore_previous()` valid-address and first-lease-clear unit tests execute under GNU89/`-Werror`/ASan/UBSan, together with the four callback prior-value bounds and 11-field transaction rollback.
3. Poll XY-C1 run `44693084076` for production source SHA `ee8ef67`. L3 is not verified unless the same production SHA completes MIPS build, TRX verification and artifact upload. The intervening docs/workflow-only commits do not change the production C SHA.
4. Next implementation slice: transactional IPTV MAN route metadata deconfig plus checked IPTV DHCP/ZeroConf-IPTV empty-address publication; propagate NVRAM failures to retry callers and test injected write/rollback failures.

## Verification boundary

- PASS on 5e2c84586842bffc46238d491b042dc1b9ee4741: WAN route 44692804478, HTTPD 44692804337, AI docs 44692804357. WAN callback 44692804497 failed at a source-body assertion after the ZeroConf rollback helper was centralized; corrected parser selection and assertions are included in ee8ef67.
- EXACT-SHA `ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1`: WAN route `44693084041`, HTTPD `44693084106`, AI docs `44693084139`, NVRAM boundary `44693084087` and other completed host checks passed; WAN callback `44693084363` failed on the superseded direct-clear assertion; XY-C1 `44693084076` remained in progress at last poll.
- Workflow-only follow-up `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49` is queued as WAN callback rerun `44693740862`; it pins the centralized previous-address helper's set/clear semantics and removes the obsolete caller-level clear assertion.
- Exact-head AI docs run `44693247011` on docs baseline `284735ec1a79a9ad30f92e5ab1f3c74b4f691c55` failed because root `dev_log.md` held 120 records. This checkpoint archives the oldest 100 verbatim into part-019 and retains the newest 20 plus the new checkpoint record.
- No same-source SHA has yet completed MIPS build + TRX verification + artifact upload after the WAN/IPTV/ZeroConf snapshot changes. L3 remains NOT VERIFIED. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and exact-SHA statuses for the child of `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49`; confirm AI docs passes after the part-019 archive and inspect WAN callback rerun `44693740862`. Then implement the next deconfig transaction slice. Keep the source SHA `ee8ef67` MIPS/TRX/artifact evidence separate; do not claim L3 without all three on the same production SHA or L4 without router evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. A same-production-SHA detector, MIPS build, TRX verification and artifact upload are required for L3. Never infer L4 from CI.


## Session handoff — 2026-11-19

### Repository state
- `main` before this handoff update: `02285d690401d1a27d90354f9243b82b4ca8d2ee`.
- Backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (checked; not modified).
- Exact-SHA Actions for the pre-handoff `main` head:
- WAN DHCP callback state stability regression: completed/success (run 44703168138)
- AI handoff documentation contract: completed/success (run 44703168044)

### Completed implementation in this work period
- `7485455e53de44887907218c9ee22990974d11bf` — updated LAN DHCP/MANv deconfig source contracts, including five-key atomic metadata publication and route cleanup boundaries.
- `6b40725ac83447fec99ec79e0b6dfc79eefce83f` — follow-on source-contract correction for the multiline MANv default-route call.
- `e34572aed4510b50a3e1d171b5b44940c95e68ec` — WAN teardown statistics-clear status handling.
- `ea976ca3a553f238d202d84be0b89022efed062b` — LAN DHCP deconfig now snapshots `lan_gateway_t`, `lan_ipaddr`, `lan_netmask`, and `lan_domain` in one exclusive NVRAM transaction. It copies each rotating-cache value before the next read; a read, truncation, lock, or unlock error invalidates the tuple and returns before route/interface/hosts changes. The interface and hosts restoration consume the same snapshot.

### Regression and CI coverage
- Updated `scripts/check_lan_dhcp_deconfig.py` to assert the four-key snapshot mapping, lock/read/unlock/invalidate policy, cleanup order, and absence of unlocked reads in the LAN deconfig path.
- Extended `.github/workflows/wan-dhcp-callback-stability.yml` to compile/run `tests/wan_nvram_status_policy.c` with `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` and `ASAN_OPTIONS=detect_leaks=1` before the LAN deconfig source checker.
- The common NVRAM snapshot regression covers cache-slot overwrite, missing fields, read failures, oversized values, begin/unlock failures, and invalid arguments. Do not claim a PASS unless the matching exact-SHA Actions run is completed successfully.

### First task for the next session
1. Fetch current `main` and inspect only commits after this handoff. Poll exact-SHA Actions for the implementation commit above and the new handoff commit; resolve any failing LAN callback or XY-C1 build check. If a run is still pending, continue an independent stability/security fix rather than waiting.
2. Continue whole-userland implementation from a concrete defect; do not repeat startup recovery or re-audit the completed MANv, static WAN/MAN, or LAN snapshot work.
3. Keep changes atomic and update this file after the next safe slice.

### Recent main history observed at handoff
- 02285d690401 test(wan): align static IPTV callback source contract
- ea98cd8cdf5d test(lan): skip prototypes in C source checker
- 3ca051d4590e fix(wan): fail closed on static IPTV snapshot errors
- ea976ca3a553 fix(lan): restore DHCP state from one NVRAM snapshot
- e34572aed451 fix(wan): check statistics clears during teardown
- 6b40725ac834 test(wan): scope IPTV deconfig assertions to route helper
- 7485455e53de test(dhcp): update MANv deconfig contract for atomic snapshots
- dc061deb6441 ci(wan): fix route snapshot contract indentation
- cd62df466da9 fix(wan): publish IPTV deconfig metadata atomically
- 86e30d782ba8 test(wan): align route and signal contracts with NVRAM snapshot
- 73abb6125238 test(wan): exercise transactional NVRAM tuple snapshots
- 0d929bab7711 test(wan): normalize MAN snapshot declaration assertions
