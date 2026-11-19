# 2026-11-19 continuation checkpoint — SmartDNS lifecycle integration and Inadyn recovery

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Main parent before THIS downstream-contract repair: `03dc5a8f3a38561b8a33eb1a530d7a0da6dfc7ae`. The active source line includes Inadyn cache-directory permissions hardening and its fixture correction; latest SmartDNS/Inadyn source tasks are listed below.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- No local checkout is available in this connector-only environment. Exact GitHub Actions are the host/target validation source.

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

## Completed slice — recover AP DHCP host publication on later renewals

- `aeec47de88127b46a722ee426549349f3e54de73` changed AP-mode same-value DHCP RENEW handling so every later renewal retries `create_hosts_lan()`; this is the durable fallback when both the volatile retry marker and immediate PID 1 notification queue fail. Removed the obsolete `lan_hosts_retry` state and retained the immediate notification retry.
- `f9d54df535c24e09ee9d7781003c180d38f896cf` removed a stale invocation of the deleted retry-status helper from the policy test. Exact WAN DHCP callback regression `44496707114` passed on that follow-up SHA.
- The committed DHCP lease remains successful when ancillary hostname publication fails; the change repairs eventual host-file consistency without rolling back the lease.

## Completed slice — bound NVRAM WAN interface-name copies

- `a6cf1fd26584614fe9212cd7be55f4a2f05cb3c0` added `wan_ifname_copy()` to reject empty, overlong and unsafe interface tokens before copying into fixed-size buffers. `get_wan_ifname()` now returns status; miniupnpd, Inadyn and firewall consumers check it, and Inadyn's direct management-interface `strcpy()` was removed.
- `9f2367b578d376593d4a32d2849cba97708961a7` normalized a line-wrapped source assertion without weakening the contract. Exact WAN callback regression `44499846484` passed. XY-C1 run `44499578824` on `a6cf1fd` completed Build XY-C1, TRX verification and artifact upload successfully (L3 for that SHA).

## Completed slice — validate PPPoE interface names before config parsing and relay launch

- `1ed9992e8af14913fdf2f030de70206b8140197a` validates the NVRAM-managed interface with `wan_ifname_copy()` before PPP session generation changes or options-file publication. The generated `nic-*` option now uses only the bounded local copy, preventing an interface value containing a newline or parser metacharacter from creating additional privileged `pppd` options.
- `213546dfb420c350c3754eb9cb9b2b6d706dd350` applies the same bounded validation before passing the management interface as an argv value to `pppoe-relay`; the focused WAN workflow's push and pull-request path filters now include `net.c`.
- The first exact `213546` WAN run `44501588287` exposed a checker-only delimiter bug after its ASan/UBSan interface-copy test had passed. `72fc0604832ff6fa3f3cfc7a152a0895b5e74896` corrected the checker function-boundary literal; exact WAN regression `44501725249` now passes on current main.

## Verification boundary

- AP DHCP renewal recovery: exact WAN callback regression `44496707114` PASS on `f9d54df535c24e09ee9d7781003c180d38f896cf`.
- WAN interface-name bounds: exact WAN callback regression `44499846484` PASS on `9f2367b578d376593d4a32d2849cba97708961a7`. The same source SHA's XY-C1 workflow `44499578824` completed Build, TRX and artifact-upload steps successfully.
- PPPoE config and relay validation: exact WAN callback `44501725249` PASS on current main `72fc0604832ff6fa3f3cfc7a152a0895b5e74896`. The earlier `44501588287` failure on `213546` was only the static checker's escaped-newline boundary; the underlying ASan/UBSan bounds test passed and the corrected exact-SHA rerun is green.
- On `213546dfb420c350c3754eb9cb9b2b6d706dd350`, NVRAM audit `44501588395`, AI docs `44501588399`, HTTPD auth/security `44501588392`, Process inventory `44501588327`, Process fail-closed `44501588361`, Firewall startup `44501588307`, WAN route `44501588455` and USB storage `44501588302` all PASS.
- XY-C1 run `44501588862` on production SHA `213546dfb420c350c3754eb9cb9b2b6d706dd350` remains IN_PROGRESS at Build; TRX and artifact upload are pending. The older `44500974346` run on `1ed9992` is also in progress. Do not claim L3 for either until Build, TRX and upload complete. Physical-router L4 remains NOT VERIFIED.

## Retained historical verification details

- ACL input parsing: Switch CLI, Process, AI docs and NVRAM audit PASS on exact production SHA 752 (`44488416113`, `44488416467`, `44488415970`, `44488416220`). XY-C1 `44488415852` is completed PASS on 752; confirm its Build/TRX/artifact step results before claiming L3.
- Follow-up PHY/VLAN/raw-register numeric parsing: production changes are on exact SHA 3d. GNU89/Werror ASan/UBSan parser compilation and runtime tests PASS; prior switch contracts and ACL contract PASS. Switch CLI `44490029309` failed only its new source contract because it did not normalize the C line-wrapped decimal parser call. This commit corrects the checker without weakening assertions.
- On 3d, Process inventory `44490029325` and AI docs `44490029437` PASS; XY-C1 `44490029298` and NVRAM audit `44490029411` were still pending at the last poll.
- Storage partition inventory: host L2 PASS on f873 and staged include/duplicate HTTPD host checks PASS on 2a/bc4; corrected production header still requires a fresh exact-source Build/TRX/artifact run. ZeroTier lifecycle verifications are recorded above. Physical switch/router L4 NOT VERIFIED.

## Active task

- Resume on `main` only. Parent for THIS handoff repair: `03dc5a8f3a38561b8a33eb1a530d7a0da6dfc7ae`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and must not be modified. PR #1 remains closed/unmerged.
- The former `get_ifstats_counter()` handoff task is already implemented at `2a490739eb0ef8d9da0a1772bb779fe686f837cd`: strict interface-component/sysfs path validation, snprintf bounds, strict uint64 decimal parsing, GNU89/ASan/UBSan cases and Process/XY-C1 trigger coverage. Do not repeat it.
- SmartDNS source work now includes atomic generated-config publication, failed-prepare rollback, restoration of the previous daemon on failed replacement start, verified DNSMasq reload before teardown, a single shell lock across start/stop/restart, and fail-closed custom-config append errors. Dedicated shell tests cover lock ownership, config publication and append failure rollback.
- Inadyn work includes restoration after prelaunch failures and cache-directory permissions hardening. Latest exact-head Inadyn and Process Actions are being monitored.
- CI evidence:
  - DNSMasq run `44509496752` PASS on `03dc5a8f3a38561b8a33eb1a530d7a0da6dfc7ae`.
  - Inadyn run `44509496651`, Process inventory run `44509496666`, and WAN route run `44509496774` PASS on the same `03dc5a8` SHA.
  - XY-C1 run `44509496895` on `03dc5a8` was IN_PROGRESS at the last check. XY-C1 run `44508947825` on `b20805b` completed successfully but Build XY-C1, TRX and artifact upload were SKIPPED (host-only); do not claim L3. The latest exact production-source Build/TRX/artifact sequence remains pending.
  - PID 1 notification queue run `44507439447` on `01f20c6` failed only at its generic SmartDNS C restart assertion (line 122); all preceding GNU89 policy tests passed. USB storage run `44508947435` on `b20805b` passed inventory and caller-graph checks but failed the generic SmartDNS stop/start assertion. THIS COMMIT updates both consumers to require SmartDNS's exact shell-locked restart delegation while preserving generic service failure gates for every other daemon.
  - Physical-router L4 remains NOT VERIFIED. Do not infer L3 for latest main from an earlier host-only or older firmware run.
- No production source behavior changes in THIS commit; it aligns static integration contracts and refreshes this live handoff. After exact-head PID 1, USB storage and docs Actions pass, select an independent reliability/security task outside the just-completed SmartDNS/Inadyn slices.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head PID 1 notification queue, USB storage hotplug, and AI handoff documentation Actions for THIS COMMIT. Diagnose any new causal failure; do not repeat passing source/test suites or treat failed ancestors as current failures.
2. Inspect the exact XY-C1 run on the current production source SHA. Claim L3 only if Build XY-C1, TRX verification and artifact upload all pass on the same SHA.
3. Continue an independent P1/P2 stability or security slice in a different integrated userland subsystem. Keep the backup immutable; create one complete tree/commit and update `main` with `force=false` and the exact expected parent.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
