# 2026-11-19 continuation checkpoint — propagate ZeroTier static-route add failures

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

## Active task — ZeroTier static-route add status

- `zero_route()` previously ignored `ip route add` failures and `rules()` ignored the helper result. NAT-enabled ZeroTier could therefore report ready while configured LAN routes were absent.
- Route count and enable flags are now validated, route operands are quoted, and add failure returns nonzero. The caller rolls back the already-published firewall rules and attempts route cleanup before failing startup.
- The shell regression now exercises production `zero_route()` with mocked NVRAM/ip commands, injects a failed static-route add after all five firewall rules, and verifies rollback. Exact-head ZeroTier, process-inventory, AI-docs and XY-C1 checks are pending for this follow-up.

## Verification boundary

- RStats L1/L2/L3 passed as recorded above; L4 not verified.
- Radio status/policy checks are green on the exact SHAs above; XY-C1 build confirmation was still outstanding on the radio parent SHAs at the last poll.
- ZeroTier firewall fix is L2 PASS on 3ff. Static-route add propagation is committed with an injected host regression; exact-head Actions remain pending. No local checkout, MIPS firmware result or L4 router test is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head ZeroTier lifecycle, process inventory, AI docs, XY-C1 Build/TRX/artifact and NVRAM runs for the static-route fix.
2. Require the production-shell route-add failure injection, firewall rollback checks and source contracts to PASS; diagnose any failure without weakening tests.
3. Close parent XY-C1 `44473915674` (firewall commit) and `44473205687` (radio CLI commit) by exact SHA; do not inherit those results to descendants.
4. Continue whole-userland stability/security review after CI status is settled. If the route fix passes, review whether ZeroTier stop reports incomplete route cleanup; that was intentionally not changed here.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are separate validation levels.
