# 2026-11-19 continuation checkpoint — fail closed on ZeroTier firewall publication errors

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for the ZeroTier firewall publication fix: `a4f6176bd774ab0ea44ab9515a367e1b8aac41d6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Completed prior slices

### RStats publication permissions

- Product fix `5e11aa501a9ae2ad16e2946f0004ebff3535574e` makes both new RStats atomic outputs use `0666 & ~umask`, preserving existing file modes.
- Test-only follow-up `ef26e97d5791fcb00c7f3e1af0693ad1bd440d4b` corrected the initially double-escaped NUL.
- RStats host `44470244091` PASS on ef26; XY-C1 `44470078199` passed Build/TRX/artifact on unchanged production SHA 5e11; NVRAM audit `44470244067` and process inventory `44470244196` PASS on ef26. No L4 device claim.

### PID-1 radio notification and radio CLI status

- `1d1a839cab45ea4db21d2fec1ba74028baa44338` fixes all six PID-1 radio events that previously discarded `control_radio_*` results; failure now retains the incomplete notification marker. Exact wireless `44472557258`, process inventory `44472557323`, Process lifecycle regression `44472557172`, AI docs `44472557416`, PID-1 queue `44472557326` and NVRAM `44472557377` passed; parent XY-C1 `44472557312` remained in progress at last check.
- `a4f6176bd774ab0ea44ab9515a367e1b8aac41d6` fixes ten `radio2_*`/`radio5_*` applet paths that returned false success after failed radio transitions, preserving legacy changed/no-change APIs and negative watchdog settling errors. Exact wireless `44473205481`, process inventory `44473205503`, AI docs `44473205559`, PID-1 queue `44473205689` and ZeroTier baseline `44473205688` passed. Process lifecycle regression `44473205836`, XY-C1 `44473205687` and NVRAM `44473205667` were in progress at last check.

## Active task — ZeroTier firewall publication status

- `rules()` previously ignored every `iptables -A` status for its three INPUT/FORWARD and two NAT rules, then returned success. This could report a healthy ZeroTier service while its required firewall protections were only partially installed.
- The checked publisher now wraps all five appends; on failure it logs, attempts to remove the partial rule set, and returns nonzero. The existing start path then reports failure and attempts bounded daemon-stop cleanup.
- Regression injects failure on the second filter append and second NAT append, verifies later rules are not attempted, and requires cleanup. The source-contract checker requires all five append paths to use the checked publisher.
- Parent baseline ZeroTier regression `44473205688` passed before this change. The production script is covered by XY-C1 path filter `trunk/user/zerotier/**`; exact-head ZeroTier, XY-C1 Build/TRX/artifact, process inventory and AI docs checks are pending for this commit.

## Verification boundary

- RStats L1/L2/L3 passed as recorded above; L4 not verified.
- PID-1 radio and radio CLI source/policy Actions are green on their respective exact SHAs; the latest parent a4f XY-C1, NVRAM and Process lifecycle results still need exact status closure.
- ZeroTier firewall status change is committed with injected regressions; exact-head Actions are pending. No local checkout, MIPS build result, or physical-router behavior is claimed yet.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head ZeroTier lifecycle, process inventory, AI docs, XY-C1 and NVRAM workflow runs for this commit.
2. Require the injected ZeroTier shell regression/source contracts and XY-C1 Build, TRX verification and artifact upload to PASS; fix any real failure without weakening rollback coverage.
3. Also close parent a4f XY-C1 `44473205687`, Process lifecycle `44473205836` and NVRAM `44473205667` by their own SHA; never inherit those results to a child commit.
4. Then continue whole-userland stability/security review. A separate unclosed ZeroTier follow-up is static-route add status propagation in `zero_route()`; it was not changed in this atomic firewall-rule slice.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, current product-source SHA, relevant exact-head Actions, verification boundary, and next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions reported by exact SHA; host tests, firmware builds and physical-device checks are separate validation levels.
