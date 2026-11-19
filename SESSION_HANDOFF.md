# 2026-11-19 continuation checkpoint — repair HTTPD storage host regression include

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for the active HTTPD workflow correction: `2a65185fa4cb2910f80429b6b5450677a9fde25d`.
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
## Completed host regression — retain ZeroTier NAT source segment through daemon stop

- `stop_zero()` captures the connected network segment while the daemon is still running; `del_rules()` preserves and uses that saved segment after the interface/route disappears.
- NAT-enabled stop fails closed before signaling the daemon when the source segment cannot be captured. Regression fixtures cover route visibility only while RUNNING, saved `10.147.17.0/24` use after shutdown, and missing-route refusal without daemon/firewall/config mutation.
- Exact d82 ZeroTier `44484902778`, WAN route `44484902793`, Process inventory `44484902785`, and AI docs `44484902809` all PASS. XY-C1 `44484902954` failed on d82 before the later storage-header fix; Build failed at the old source-tree-relative include, so no L3 is claimed for d82.

## Active task — align HTTPD storage regression include path

- Main `2a65185fa4cb2910f80429b6b5450677a9fde25d` fixes the production RC include by changing `storage_partition_probe.h` to `#include "disk_inventory_policy.h"`; the staged RC build already adds `-I$(STAGING_DIR)/include/libdisk`. Its exact storage host workflow `44486117414` passes on 2a.
- The same main SHA's HTTPD security run `44486117403` fails only in its duplicate storage host-regression compile: that workflow still supplies `-I trunk/user/rc/src` but omits `-I trunk/user/libdisk/src`, so the corrected public include cannot resolve. Its other preceding checks passed.
- This atomic correction adds the missing include path to the HTTPD compile command and embeds a contract for the include name and compile path. It does not alter production code.
- Exact 2a AI docs `44486117156` and Process inventory `44486117196` PASS; NVRAM transaction audit `44486117346` was still in progress at last inspection. The exact XY-C1 d82 run `44484902954` predates the production include correction and failed on the obsolete path; the corrected storage production source still needs its own Build/TRX/artifact run before L3 can be claimed.

## Verification boundary

- Storage partition inventory: policy host L2 PASS on f873 (`44480440427`); production header include correction committed on 2a and storage workflow `44486117414` PASS. HTTPD's separate host-test include path was missing on 2a (`44486117403`); this workflow-only commit fixes that regression runner. Current corrected production source still requires exact-source XY-C1 Build/TRX/artifact before L3.
- ZeroTier static-route deletion: ZeroTier and AI docs PASS on d4 (`44483203855`, `44483203814`). NAT source-segment cleanup: ZeroTier `44484902778`, WAN `44484902793`, Process `44484902785`, AI docs `44484902809` all PASS on d82.
- Exact 2a AI docs `44486117156`, Process inventory `44486117196`, and storage hotplug `44486117414` PASS. HTTPD security `44486117403` failed the storage policy host compile as described above; NVRAM audit `44486117346` was pending at the last check.
- XY-C1 L3 is VERIFIED only on production SHA `255334d0dbd7e98a1a63dde528b0e3de59aef9c1` (`44476582849`). The d82 run `44484902954` failed Build before the 2a public-header include correction; no L3 claim for 2a or this descendant. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Confirm live `main` equals the new commit and inspect exact-head HTTPD security, AI handoff docs, storage hotplug, Process inventory and XY-C1 runs. Inspect failure logs and require each run's `head_sha` to match.
2. Require HTTPD's storage policy test to compile GNU89 `-Wall -Wextra -Werror` with both RC and libdisk include paths and to pass its source contract. The storage hotplug regression must remain PASS.
3. The corrected storage production header must still be validated by a fresh XY-C1 Build/TRX/artifact run; then continue independent userland stability/security fixes, including the switch ACL argument defect already identified for the next slice.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
