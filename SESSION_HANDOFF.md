# 2026-11-19 continuation checkpoint — USB RAS PPP cleanup and WAN rollback

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Current production-source main SHA: `3ae9fd9a7318ef3945c72d3ab313b9c0126927bc` (RAS PPP failure cleanup/rollback; parent `eb379476c57a11da1f8c05af92bd60a53b7902cf`).
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

## Completed slice — validate switch ACL inputs before MMIO writes

- ACL EtherType, L4 payload and source-port commands now require exactly six arguments, strictly parse bounded numeric values and validate all eight port-map characters before the first MMIO write.
- Exact Switch CLI `44488416113`, Process inventory `44488416467`, AI docs `44488415970` and NVRAM audit `44488416220` PASS on production SHA `752c880e995333ded4d93b4aa82186a537e40b34`. XY-C1 `44488415852` completed successfully on 752; inspect its individual build, TRX and artifact steps before using it as L3 evidence.

## Completed slice — USB RAS PPP launch cleanup and physical-WAN rollback

- `launch_wan_modem_ras()` previously returned the direct `eval("/usr/sbin/pppd", "call", call_file)` result. A failed launch therefore bypassed the common stale-peer cleanup path and could leave a `/tmp/ppp/peers/modem.wanN` file containing modem credentials.
- The launcher now captures the pppd status, keeps the peer file only on success, and removes the credential-bearing file after a failed launch. It preserves both the original pppd status and errno across best-effort unlink. A small policy helper and GNU89 regression cases cover success, positive failure and negative failure.
- `start_wan()` now restores the physical WAN binding/firewall after a RAS PPP startup failure using the existing fail-closed `restore_wan_after_firewall_failure()` path. The pppd error is aggregated before the recovery result; failed recovery keeps the physical interface down and is logged.
- The exact-source checker verifies cleanup ordering, errno/status preservation and rollback/error aggregation.
- On production SHA `3ae9fd9a7318ef3945c72d3ab313b9c0126927bc`, USB modem workflow `44490980473` and WAN route workflow `44490980282` passed all steps. Process fail-closed `44490980113`, Process inventory `44490980140` and NVRAM audit `44490980245` also passed. XY-C1 `44490980325` was still building at the last poll; TRX verification and artifact upload were pending, so no L3 claim is made.

## Verification boundary

- ACL input parsing: Switch CLI, Process, AI docs and NVRAM audit PASS on exact production SHA 752 (`44488416113`, `44488416467`, `44488415970`, `44488416220`). XY-C1 `44488415852` is completed PASS on 752; confirm its Build/TRX/artifact step results before claiming L3.
- Follow-up PHY/VLAN/raw-register numeric parsing: production changes are on exact SHA 3d. GNU89/Werror ASan/UBSan parser compilation and runtime tests PASS; prior switch contracts and ACL contract PASS. Switch CLI `44490029309` failed only its new source contract because it did not normalize the C line-wrapped decimal parser call. This commit corrects the checker without weakening assertions.
- On 3d, Process inventory `44490029325` and AI docs `44490029437` PASS; XY-C1 `44490029298` and NVRAM audit `44490029411` were still pending at the last poll.
- Storage partition inventory: host L2 PASS on f873 and staged include/duplicate HTTPD host checks PASS on 2a/bc4; corrected production header still requires a fresh exact-source Build/TRX/artifact run. ZeroTier lifecycle verifications are recorded above. Physical switch/router L4 NOT VERIFIED.

## Active task — verify XY-C1 and inspect AP DHCP retry-state failure handling

- The PHY/VLAN/raw-register parser source-contract failure on SHA 3d was a whitespace-format assertion, not a production-code failure. The whitespace-normalized contract correction is on `eb379476c57a11da1f8c05af92bd60a53b7902cf`; Switch CLI run `44490557134` and AI handoff docs run `44490556946` both PASS on that exact SHA.
- The current production-source SHA is `3ae9fd9a7318ef3945c72d3ab313b9c0126927bc`. Its USB modem and WAN route host regressions, Process inventory/fail-closed checks and NVRAM transaction audit all PASS. XY-C1 run `44490980325` is still in progress at Build; TRX and artifact steps have not yet completed.
- After checking that exact XY-C1 run, continue with the AP DHCP `lan_hosts_retry` recovery path: establish whether retry-state publication failures can strand a committed lease without a later repair attempt. This is an investigation target, not a confirmed defect; do not change the intentionally non-fatal post-commit lease semantics without proving a failure mode.

## Exact first action for the next GPT

1. Inspect XY-C1 run `44490980325` on exact source SHA `3ae9fd9a7318ef3945c72d3ab313b9c0126927bc`, including Build, TRX and artifact-upload step results. Do not wait if it remains pending.
2. Continue the AP DHCP retry-state task by tracing the return contract of `nvram_set_int_temp()`, the retry marker and PID-1 queue failure path. Add a regression and source fix only if the current code can demonstrably lose the retry.
3. Do not repeat the completed RAS peer cleanup/rollback or switch numeric parser work.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
