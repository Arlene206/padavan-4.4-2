# 2026-11-19 continuation checkpoint — repair ZeroTier route-test fixture

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for the static-route status fix: `3ffb25f6357535965f095eba2bfd45a813c96db8`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Completed prior slices

### RStats publication permissions

- Product fix `5e11aa501a9ae2ad16e2946f0004ebff3535574e` makes both new RStats atomic outputs use `0666 & ~umask`, preserving existing file modes.
- Test-only follow-up `ef26e97d5791fcb00c7f3e1af0693ad1bd440d4b` corrected the initially double-escaped NUL.
- RStats host `44470244091` PASS on ef26; XY-C1 `44470078199` passed Build/TRX/artifact on unchanged production SHA 5e11; NVRAM audit `44470244067` and process inventory `44470244196` PASS on ef26. No L4 device claim.

### Radio status propagation

- PID-1 radio notification fix `1d1a839cab45ea4db21d2fec1ba74028baa44338`: wireless `44472557258`, process inventory `44472557323`, Process lifecycle `44472557172`, AI docs `44472557416`, PID-1 queue `44472557326), and NVRAM `44472557377` passed; parent XY-C1 `44472557312` was still in progress at last check.
- Radio CLI status fix `a4f6176bd774ab0ea44ab9515a367e1b8aac41d6`: wireless `44473205481`, process inventory `44473205503`, Process lifecycle `44473205836`, AI docs `44473205559`, PID-1 queue `44473205689), NVRAM `44473205667` all PASS. Parent XY-C1 `44473205687` remained in progress at last check.

### ZeroTier firewall rule publication

- `3ffb25f6357535965f095eba2bfd45a813c96db8` wraps all five ZeroTier INPUT/FORWARD/NAT rule appends with a checked publisher. A rejected append now logs, rolls back the partial rule set and returns nonzero; regression injects second-filter and second-NAT failures and proves no later append occurs.
- Exact ZeroTier `44473915670`, process inventory `44473915585), AI docs `44473915620` PASS on 3ff; XY-C1 `44473915674` remained in progress. This SHA is the parent for the current static-route fix.

## Completed prior slice — ZeroTier static-route add status

- Commit `67a45de88cb4776cf464164a9d3784e8555e0546` validates static-route count/enable values, quotes route operands, returns nonzero on `ip route add` failure, and rolls back the already-published firewall and route state.
- Exact WAN-route `44474852328`, process inventory `44474852420` and AI docs `44474852357` PASS. The exact-head ZeroTier run `44474852372` failed in the shell fixture before route assertions because the test's stubbed `start_instance()` never seeded `zt0`, while strict `set -u` made stop cleanup reference it. XY-C1 `44474852399` was IN_PROGRESS at last poll.

## Active task — repair ZeroTier route-test fixture

- The lifecycle test sources production functions under `set -u`, but replaces `start_instance()` with a no-op stub. That stub bypasses the normal interface-discovery path that assigns `zt0`; a later stop-cleanup case with configured static routes therefore aborts with `zt0: parameter not set`.
- Seed `zt0=ztunit` in the fixture to model the interface state established by successful startup, and assert that contract in `check_zerotier_lifecycle.py`. Production ZeroTier logic is unchanged in this test-only repair.
- Exact-head ZeroTier, AI docs, process inventory and XY-C1 checks are pending after this commit. No local or physical-router test is claimed.

## Verification boundary

- RStats L1/L2/L3 passed as recorded above; L4 not verified.
- Radio status/policy checks are green on the exact SHAs above; XY-C1 build confirmation was still outstanding on the radio parent SHAs at the last poll.
- ZeroTier firewall publication fix is committed on `3ffb25f6357535965f095eba2bfd45a813c96db8`; static-route add production fix is on `67a45de88cb4776cf464164a9d3784e8555e0546`. The exact 67a ZeroTier shell run `44474852372` failed from the unseeded fixture variable described above; this follow-up only repairs the fixture, with exact rerun pending. Parent XY-C1 `44474852399` was IN_PROGRESS at last poll. No MIPS firmware or L4 router result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head ZeroTier, process inventory, AI docs and XY-C1 runs for this fixture-only repair. Confirm the route-add failure injection and all existing lifecycle cases reach the test's final PASS line.
2. Require XY-C1 Build, TRX verification and artifact upload before claiming L3 for the ZeroTier production-source SHA `67a45de88cb4776cf464164a9d3784e8555e0546`.
3. Then continue the independent whole-device SD/MMC hotplug inventory hardening: `storage_partition_probe.h` ignores malformed/truncated `/proc/partitions` rows and stops scanning at the first partition match; add strict parser and GNU89 CI coverage.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are separate validation levels.
