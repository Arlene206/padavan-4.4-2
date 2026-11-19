# 2026-11-19 continuation checkpoint — capture ZeroTier interface before daemon stop

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for the ZeroTier stop-order correction: `f65813735c6f817338006777e81645b2015bc215`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- Concurrent main commit `83b726a87e5948d38afee7885e050d82d2d03452` added transient mdev hotplug setup retries; exact mdev regression `44475349712`, Process inventory `44475349769), AI docs `44475349699` PASS.

## Completed slices

### RStats publication permissions

- `5e11aa501a9ae2ad16e2946f0004ebff3535574e`: atomic RStats outputs honor `0666 & ~umask`, preserving existing modes.
- Test-only follow-up `ef26e97d5791fcb00c7f3e1af0693ad1bd440d4b` corrected the double-escaped NUL fixture.
- RStats host `44470244091` PASS; XY-C1 `44470078199` passed Build/TRX/artifact on unchanged production SHA 5e11; NVRAM `44470244067` and process inventory `44470244196` PASS. No L4 claim.

### Radio status propagation

- PID-1 notification fix `1d1a839cab45ea4db21d2fec1ba74028baa44338`: wireless `44472557258`, process inventory `44472557323`, Process lifecycle `44472557172`, AI docs `44472557416`, PID-1 queue `44472557326), NVRAM `44472557377` PASS.
- Radio CLI fix `a4f6176bd774ab0ea44ab9515a367e1b8aac41d6`: wireless `44473205481`, process inventory `44473205503), Process lifecycle `44473205836`, AI docs `44473205559), PID-1 queue `44473205689), NVRAM `44473205667` PASS.

### ZeroTier startup status

- `3ffb25f6357535965f095eba2bfd45a813c96db8`: check all five INPUT/FORWARD/NAT appends and roll back partial firewall publication. ZeroTier `44473915670`, process inventory `44473915585), AI docs `44473915620` PASS; XY-C1 `44473915674` last known in progress.
- `67a45de88cb4776cf464164a9d3784e8555e0546`: validate static-route config, quote operands, propagate `ip route add` failure and roll back firewall/routes. WAN route `44474852328`, Process inventory `44474852420), AI docs `44474852357` PASS; ZeroTier `44474852372` failed in the strict-nounset stop fixture.
- `b2d3b94b4414d0dee4ba423791a9ad91ba175893` and `f65813735c6f817338006777e81645b2015bc215` are test-only corrections (seeded fixture interface and corrected grep count). XY-C1 `44475877010` passed on b2d; exact ZeroTier rerun for current tests was still pending when this handoff was written.

## Completed prior slice — retain interface identity while stopping ZeroTier

- Commit `255334d0dbd7e98a1a63dde528b0e3de59aef9c1` captures `zt0` before `kill_z()`; `del_rules()` also tolerates an unset variable in a fresh shell. The fixture makes `ztunit` disappear as soon as the daemon exits and requires firewall/static-route cleanup to use the saved interface.
- Exact ZeroTier `44476582735` failed later in the independent “Missing NAT route” regression because that case inherited `TEST_NAT_ENABLE=0` from the prior fixture; production correctly skipped NAT route readiness. WAN route `44476582757`, AI docs `44476582739` and Process inventory `44476582787` PASS. XY-C1 `44476582849` was IN_PROGRESS.

## Active task — explicitly enable NAT in the missing-kernel-route regression

- Set `TEST_NAT_ENABLE=1` inside the “Missing NAT route” test before invoking `rules()`, while retaining `TEST_ROUTE_MODE=none`.
- Pin this fixture precondition in `check_zerotier_lifecycle.py`. This is test-only; production route/firewall code is unchanged.
- Exact-head ZeroTier, AI docs, Process lifecycle, NVRAM and XY-C1 validation pending after this correction.

## Verification boundary

- RStats and radio host/policy checks passed on their exact SHAs; RStats L3 proven on SHA 5e11 only.
- ZeroTier firewall publication L2 passed on 3ff. Static-route add fix is on 67a; stop-interface-preservation correction is on 255. Exact 255 ZeroTier `44476582735` fails only because the missing-NAT fixture omitted its required `TEST_NAT_ENABLE=1`; WAN route `44476582757`, AI docs `44476582739` and Process inventory `44476582787` PASS. XY-C1 `44476582849` remained IN_PROGRESS; no L4 router result.

## Exact first action for the next GPT

1. Verify this missing-NAT fixture correction by exact-head ZeroTier and source-contract Actions; require the 30-poll missing-route timeout and final PASS summary.
2. Inspect XY-C1 `44476582849` for production SHA 255 and require Build/TRX/artifact before L3.
3. Continue SD/MMC partition inventory hardening: strict complete-line `/proc/partitions` parsing is needed because malformed/truncated records can currently be ignored and the scan stops after the first match.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
