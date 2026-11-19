# 2026-11-19 continuation checkpoint — strict switch ACL input boundary

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for the active switch ACL hardening slice: `bc4e2c0ce204af2b3827fcaf6cbed17343139092`.
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

## Completed slice — align HTTPD storage regression include path

- The exact HTTPD security run `44486117403` on 2a failed only in its duplicate storage host-regression compile because the corrected public include `disk_inventory_policy.h` was not found with only `-I trunk/user/rc/src`.
- Commit `bc4e2c0ce204af2b3827fcaf6cbed17343139092` added `-I trunk/user/libdisk/src` to that standalone host compile and pinned the include path in the workflow contract. Exact HTTPD security `44487439944` and AI docs `44487439972` both PASS on bc4.

## Completed slice — validate switch ACL inputs before MMIO writes

- Production target is `trunk/user/utils/src/switch/switch_gsw.c`; its Makefile explicitly builds `SRC = switch_gsw.c`. Do not change the unrelated legacy RC switch implementation for this CLI task.
- `acl_ethertype()`, `acl_l4_add()` and `acl_sp_add()` previously accepted partial numeric tokens through `strtoul(..., NULL, ...)`, silently ignored the eighth reserved port-map character, and returned success on missing arguments while allowing extra arguments.
- Require exactly six arguments; parse EtherType and L4 payload as strict 16-bit hexadecimal and source port as strict base-0 16-bit input; validate all eight port-map characters through the existing `parse_portmap_argument()` path before any MMIO write. Keep the original ACL register programming sequence intact.
- Added GNU89 `-Wall -Wextra -Werror` ASan/UBSan parser coverage for prefixes, valid bounds, signs, whitespace, partial input, overflow and unchanged output on failure; the exact-source Switch CLI run `44488416113` passed on production SHA `752c880e995333ded4d93b4aa82186a537e40b34`. Process inventory `44488416467`, AI docs `44488415970`, and NVRAM audit `44488416220` also PASS on 752.
- XY-C1 Firmware Validation `44488415852` for 752 is still in progress. Do not claim L3 until Build, TRX and artifact upload all succeed.

## Verification boundary

- Storage partition inventory: parser host L2 PASS on f873 (`44480440427`); production include correction on 2a verified by storage host workflow `44486117414`; HTTPD duplicate host compile correction on bc4 verified by HTTPD security `44487439944`. No target firmware Build/TRX/artifact for the corrected storage header has passed yet.
- ZeroTier static-route deletion: ZeroTier and AI docs PASS on d4 (`44483203855`, `44483203814`). NAT source-segment cleanup: ZeroTier `44484902778`, WAN route `44484902793`, Process inventory `44484902785` and AI docs `44484902809` all PASS on d82.
- Exact 2a AI docs `44486117156`, Process inventory `44486117196`, storage hotplug `44486117414`, and NVRAM audit `44486117346` PASS. HTTPD security and AI docs on bc4 (`44487439944`, `44487439972`) PASS.
- XY-C1 L3 is VERIFIED only on production SHA `255334d0dbd7e98a1a63dde528b0e3de59aef9c1` (`44476582849`). d82 XY-C1 `44484902954` failed before the 2a staged-header correction; no later production-source Build/TRX/artifact evidence is available yet. Physical router L4 remains NOT VERIFIED.
- Switch ACL ACL-input slice: Switch CLI regression PASS on exact source SHA 752 (`44488416113`); Process `44488416467`, AI docs `44488415970` and NVRAM audit `44488416220` PASS. XY-C1 run `44488415852` remains in progress; firmware L3 not yet verified.

## Active task — strict numeric boundaries for PHY, VLAN dump and raw register commands

- A second scan of production `switch_gsw.c` found the last six permissive conversions: PHY address (base-10), VLAN dump maximum ID (base-10), and raw register read/write/dump offsets and write values (base-16). These use `strtoul(..., NULL, base)` without end-pointer or range checks.
- Add a strict decimal parser preserving leading-zero decimal semantics; constrain PHY address to 0–31, VLAN max ID to signed-int range while retaining the existing 1–4095 clamp inside `vlan_dump()`, and raw register offsets/values to their int/uint32 representations. The register dump offset is additionally bounded so the final +0xfc read cannot overflow signed int.
- Require exact argument counts and exact command tokens for `reg r/w/d`; reject malformed input before ioctl/MMIO. Add parser unit coverage and source contracts to Switch CLI workflow.
- Exact source SHA `752c880e995333ded4d93b4aa82186a537e40b34`: Switch CLI `44488416113`, Process `44488416467`, AI docs `44488415970`, NVRAM audit `44488416220` PASS; XY-C1 `44488415852` still in progress at last poll.

## Exact first action for the next GPT

1. Check current `main` and exact XY-C1 `44488415852` Build/TRX/artifact job; inspect the result but continue the active strict numeric CLI slice while it remains pending.
2. Require Switch CLI parser ASan/UBSan and source contracts to pass on the resulting exact SHA. Require Build XY-C1, TRX verification and firmware artifact upload before claiming L3.
3. Then continue an independent P1 RC startup/lifecycle defect; do not repeat already completed ACL parsing or storage/ZeroTier fixes.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
