# 2026-11-19 continuation checkpoint — fail-closed ZeroTier static-route teardown

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for this ZeroTier cleanup slice: `f873c65531a1a20f9a3f4bdce14b4c4df026f86b`; the current slice is committed atomically with this handoff update.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- No local checkout was available: `git clone` could not resolve github.com in this environment. Exact GitHub Actions are the validation source.

## Completed recent slices

### SD/MMC whole-disk mount safety

- `41b6e0ca0cd29a4e2fd39036b92106d7670bde09` replaced permissive `/proc/partitions` scanning with strict full-line/header/decimal parsing. Unknown, malformed, truncated, overlong or unreadable inventory returns -1; the mdev caller refuses a whole-disk mount.
- `379fd897be61a52ee215d768fbb6c745febb087f` repaired two valid fixtures to include the required procfs header.
- `f873c65531a1a20f9a3f4bdce14b4c4df026f86b` repaired the workflow filter: the new regression source had been present in push but absent from pull_request.
- Exact storage Actions: `44480440427` PASS on f873, including GNU89 `-Wall -Wextra -Werror`, ASan/UBSan, the full storage lifecycle/caller graph and checked eject gates. AI handoff docs `44480440506` PASS. L2 host verification complete for this storage slice.

### XY-C1 production build boundary

- Exact XY-C1 run `44476582849` on production SHA `255334d0dbd7e98a1a63dde528b0e3de59aef9c1` passed Build XY-C1, TRX verification and firmware artifact upload (L3 on that SHA).
- The later ZeroTier fixture-only SHA `7cc78f44fc11aa3698ae1231ea3e1399360bcb38` job `44477057022` was host-only; Build/TRX/artifact were skipped because production inputs were unchanged. Do not call that run L3.

### ZeroTier startup, publication and stop ordering

- `3ffb25f6357535965f095eba2bfd45a813c96db8` added checked INPUT/FORWARD/NAT appends and firewall rollback.
- `67a45de88cb4776cf464164a9d3784e8555e0546` propagated static-route add failure and rolled back partial firewall/route publication.
- `255334d0dbd7e98a1a63dde528b0e3de59aef9c1` captures the virtual interface before stopping the daemon so cleanup still targets the original `zt0`.
- `7cc78f44fc11aa3698ae1231ea3e1399360bcb38` corrected the missing-NAT route-timeout test fixture. Exact ZeroTier `44477056786`, Process inventory `44477056791`, WAN route `44477056811`, AI docs `44477056873` and XY-C1 host job `44477057022` passed on 7cc.

## Completed slice — report incomplete ZeroTier static-route cleanup

- The previous `zero_route del` discarded every `ip route del` error with `|| true`; `stop_zero()` then deleted the ZeroTier identity/configuration without proving that configured routes were absent.
- The current source adds exact destination/gateway/interface tuple inspection, treats an already-absent route as idempotent success, checks `ip route del`, and re-reads the route to verify absence. Inspection errors, malformed enabled-route configuration, deletion errors or a still-present route produce failure; remaining routes are still attempted and aggregate failure is returned.
- `stop_zero()` now retains persistent identity/configuration and returns failure when route cleanup is uncertain. Failed startup route rollback also logs an incomplete cleanup.
- Regression coverage includes already-absent routes, deletion failure, an unreadable route inventory, a delete command that reports success while the route remains, saved-interface use after daemon exit, and partial route-add rollback. Static checker pins the exact tuple and stop gate.
- Exact ZeroTier `44483203855` and AI docs `44483203814` both PASS on d4 after the checker marker and mandatory section heading corrections. Process inventory `44482870006` and WAN route `44482869891` PASS on d45. The XY-C1 run `44482869903` on d45 remained in progress at the last check; inspect Build/TRX/artifact before claiming L3.
## Pending verification — retain ZeroTier NAT source segment through daemon stop

- `stop_zero()` already saves the virtual interface before stopping the daemon, but `del_rules()` re-queries the kernel connected route after daemon exit. The route can disappear together with the interface, leaving `ip_segment` empty and causing the source-NAT MASQUERADE rule to be deleted with the wrong source operand.
- Capture `ip_segment` while the daemon is running; `del_rules()` reuses that value and only discovers a segment when no pre-captured value exists. When NAT is enabled and the segment cannot be read before stop, abort without stopping the daemon or mutating firewall/configuration.
- Extend the lifecycle regression so the connected route is visible only while the daemon state is RUNNING, then disappears after TERM; assert cleanup still receives `10.147.17.0/24`. Static source contracts pin capture-before-kill, preserve-if-present, and the NAT-enabled fail-closed gate.
- Exact d82 Actions were in progress at the last check: ZeroTier `44484902778`, WAN route `44484902793`, XY-C1 `44484902954`, Process inventory `44484902785`, and AI docs `44484902809`. The NAT source-segment host regression has not yet been validated on its exact SHA. The prior route-deletion slice passed ZeroTier `44483203855` and AI docs `44483203814` on d4; no L4 router result is claimed.



## Active task — restore XY-C1 build compatibility for strict storage partition inventory

- Exact XY-C1 run `44482869903` on production SHA d45 failed in Build XY-C1 with `storage_partition_probe.h:6: fatal error: ../../libdisk/src/disk_inventory_policy.h: No such file or directory`. RC sources are compiled from staged `trunk/build/rc`, so that source-tree-relative path resolves outside `trunk/user`.
- RC already adds `-I$(STAGING_DIR)/include/libdisk` when STORAGE_ENABLED; `trunk/user/libdisk/src/Makefile` installs public headers into `$(DESTDIR)/include/libdisk`. Use `#include "disk_inventory_policy.h"` so the target compiler resolves the staged public header.
- Add `-I trunk/user/libdisk/src` to the standalone ASan/UBSan host regression compile. Extend the embedded workflow contract to verify the RC staged include flag, libdisk header-install rule, host compile include path, and header include name.
- The parser's L2 test passed on f873 (`44480440427`); d45 has no L3 because Build failed and TRX/artifact were skipped. Exact d82 ZeroTier, WAN route, XY-C1, Process inventory and AI docs runs were pending at the last check. The include-fix descendant must pass full Build/TRX/artifact before L3 can be claimed.

## Verification boundary

- Storage partition inventory: L2 PASS on exact SHA f873 (`44480440427`); AI docs `44480440506` PASS. d45 target integration failed because the staged RC build could not resolve the source-tree-relative policy include; this commit switches to the already-staged libdisk public-header path and extends host compile/checker coverage.
- XY-C1 L3: PASS on production SHA 255 (`44476582849`) only. d45 failed Build XY-C1 (no TRX/artifact); d82 had a fresh Build/TRX/artifact run pending at last check. No L3 on d45 or later SHA yet.
- ZeroTier route teardown passed ZeroTier/AI docs on d4 (`44483203855`, `44483203814`). The d82 NAT-segment fix was still awaiting exact-SHA validation. No L4 router result exists.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA d82 Actions: ZeroTier `44484902778`, WAN `44484902793`, XY-C1 `44484902954`, Process `44484902785`, and AI docs `44484902809`. Diagnose failures; require Build XY-C1, TRX and artifact upload for L3.
2. Fix the XY-C1 parser-header build include with the staged libdisk include path; verify the strict parser host regression remains GNU89 -Werror plus ASan/UBSan, then require the descendant XY-C1 Build/TRX/artifact run.
3. After this slice is closed, continue an independent network/security reliability issue across Padavan userland; do not repeat the closed SD/MMC parser or prior ZeroTier publication fixes.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
