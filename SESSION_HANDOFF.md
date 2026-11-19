# 2026-11-19 continuation checkpoint — correct ZeroTier firewall append counting

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

## Completed prior slice — seed ZeroTier fixture interface state

- Commit `b2d3b94b4414d0dee4ba423791a9ad91ba175893` seeds `zt0=ztunit` because the test's `start_instance()` stub bypasses production interface discovery.
- Exact AI docs `44475876921`, process inventory `44475876966`, WAN route `44475876871` and XY-C1 `44475877010` PASS. The exact ZeroTier run `44475876933` advances past the original nounset error but fails the final firewall count assertion: `grep -c ' -A '` misses the three filter append lines where `-A` is the first token.

## Active task — count all ZeroTier filter and NAT append commands correctly

- The route-failure regression expected five firewall appends after all rules were installed. Its grep pattern required a space before `-A`, which matched the two NAT commands (`-t nat -A`) but missed the three filter commands beginning with `-A INPUT/FORWARD`.
- Replace it with a boundary-aware extended-regex pattern `(^| )-A `, which counts `-A` at the start of a command or after an option token. Pin the exact pattern in the Python source contract.
- This is a test-only correction; the firewall and static-route production behavior is unchanged. Exact-head ZeroTier and docs/process checks are pending; XY-C1 passed on the prior b2 source SHA.

## Verification boundary

- RStats L1/L2/L3 passed as recorded above; L4 not verified.
- Radio status/policy checks are green on the exact SHAs above; XY-C1 build confirmation was still outstanding on the radio parent SHAs at the last poll.
- ZeroTier firewall publication fix is on `3ffb25f6357535965f095eba2bfd45a813c96db8`; static-route add production fix is on `67a45de88cb4776cf464164a9d3784e8555e0546`. The fixture seed `b2d3b94b4414d0dee4ba423791a9ad91ba175893` passed exact XY-C1 `44475877010`, AI docs `44475876921`, process inventory `44475876966` and WAN route `44475876871`; ZeroTier `44475876933` then exposed a grep count bug, now corrected in this test-only follow-up. No MIPS result beyond the exact XY-C1 artifact, and no L4 router result is claimed.

## Exact first action for the next GPT

1. Inspect the exact ZeroTier run and AI docs/process inventory for this grep-count correction. Require the host regression to pass through its final summary and verify the production route-add rollback assertion executes.
2. The exact XY-C1 run `44475877010` passed on the unchanged production source from b2; do not inherit it as a check on unrelated future production changes.
3. Continue whole-device SD/MMC hotplug inventory hardening: `storage_partition_probe.h` ignores malformed/truncated `/proc/partitions` rows and stops scanning at the first partition match; implement strict parser and GNU89 CI coverage.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are separate validation levels.
