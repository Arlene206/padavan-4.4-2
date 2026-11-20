# 2026-11-20 continuation checkpoint

This checkpoint supersedes older summaries below. Continue from live main; do not repeat closed audits.

## Git state
- Main SHA before this checkpoint: c10f4fd6be13a4dc915f397ccc3292d92509e689.
- NTP readiness source commit: c10f4fd6be13a4dc915f397ccc3292d92509e689.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: 72f4f157a084f8184b9597ca5bf334b8080e5807 (read-only).
- Main-only atomic commits, expected-SHA updates with `force=false`; no force-push.

## Current source work — NTP readiness persistence
- `ntpc_updated_main()` now treats a non-empty `offset` as proof of an actual clock step before incrementing the temporary `ntpc_counter`; it checks and returns the persistent `ntp_ready` write result.
- A failed `ntp_ready` write is retried by the watchdog's fixed timer whenever a recorded NTP step exists. The retry is independent of WAN reachability and is executed before time-dependent service checks; error/recovery logging is transition-throttled.
- Added GNU89-compatible policy tests for absent/empty offsets and readiness retry gating; extended the source contract to protect ordering and ensure readiness recovery is processed before schedule checks.
- Exact-main watchdog Action: PASS (run 44731160914).
- Exact-main XY-C1 Action: PENDING (run 44731160909). Build/TRX/artifact step evidence: [{"name":"Build XY-C1","conclusion":null},{"name":"Verify TRX output","conclusion":null},{"name":"Run actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9","conclusion":null}].
- L3 is not claimed unless one exact SHA completes Build XY-C1, TRX verification and artifact upload successfully.

## First actions for next continuation
1. Inspect exact-main watchdog Action 44731160914 and repair any source-contract or regression failure.
2. Complete XY-C1 run 44731160909 for exact main c10f4fd6be13a4dc915f397ccc3292d92509e689; all Build/TRX/artifact steps must PASS on that SHA.
3. If XY-C1 is pending, continue an independent service/lifecycle defect; do not wait.
4. Keep physical-router L4 unverified unless tested on hardware.

## Verification boundary
- Prior detect_link `link_wan` mirror fix: detect-link and Process checks PASS; exact-SHA XY-C1 Build/TRX/artifact PASS on `dc43d3d8`.
- Prior detect_internet `link_internet` mirror fix: exact detector and AI-docs checks PASS on `327b83d6`; same-source XY-C1 was still pending at last known poll on `7210c3a3`.
- NTP readiness source slice: exact-main watchdog=PASS; XY-C1=PENDING. No L3 claim unless the three product steps are all PASS on the same SHA.
- Physical-router L4 remains NOT VERIFIED.

## Evidence discipline
Keep L1 source review, L2 host regression, exact-SHA Actions, L3 MIPS Build/TRX/artifact and physical-router L4 distinct.


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
