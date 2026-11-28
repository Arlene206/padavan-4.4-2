## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and the one-time handoff recovery are complete; do not repeat the initial audit.
- Current source/firmware validation checkpoint: `9fc0a65fdff1a8c315119200efb1976e6e9d181d`. The only commits after it are the handoff-validator documentation corrections `307422cf470385e367e9968735ac08e6d902da6d` and `4e5117e18afbd51580f2b05ffa4f1fcf502c9eaf`; they do not alter production inputs.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; its CA trust-store feature is integrated.

## Active task

- The networkmap ASUS IBOX ProductID fixed-field bug is completed. `PKT_GET_INFO.ProductID[32]` now must contain a NUL inside its own field before hostname validation/copy, and its diagnostic uses a bounded precision. A NUL in the following field cannot satisfy the 32-byte boundary. This addresses an adjacent-field parsing defect; it is not claimed as an out-of-buffer read.
- The next governance-ledger candidate is the PARTIAL LAN DHCP client / AP-mode lease item: inspect full live leasefail/renew/NAK/deconfig transitions for a concrete source defect and add a state-transition regression only when evidence supports a behavior change. Do not treat a non-zero `leasefail` callback as a bug by itself: that event is advisory, the script runner already logs non-zero status, and retry/background/abort policy intentionally continues.

## Exact-SHA CI checkpoint

- Production source SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`:
  - Networkmap signal/packet-field regression #46149948622: PASS, including GNU89/Werror/ASan/UBSan binaries and source contracts.
  - NVRAM transaction-boundary audit #46149948646: PASS.
  - Process lifecycle inventory #46149948705: PASS.
  - XY-C1 firmware validation #46149948587: PASS — cross-build, TRX verification and artifact upload all succeeded on this exact production-source SHA.
- Current handoff/documentation contract SHA `4e5117e18afbd51580f2b05ffa4f1fcf502c9eaf`: AI handoff documentation #46150253122 PASS. The prior AI validator attempts failed on exact wording, then were corrected; this is now green.
- The two documentation commits after `9fc0a65` did not modify production source, so the exact XY-C1 L3 result remains valid for the production tree. Do not pretend the documentation-only descendant is a different firmware source tree.

## Verification boundary

- L1: isolated GNU89 host regression with `-Wall -Wextra -Werror -fsanitize=address,undefined`: PASS.
- L2: exact-head networkmap source contract and regression workflow on `9fc0a65`: PASS.
- L3: exact-head XY-C1 Build, TRX verification and artifact upload on `9fc0a65`: PASS.
- L4: physical router runtime/network behavior is NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, preserve the frozen backup, and inspect current Actions without repeating the completed startup audit.
2. Continue the LAN DHCP client/AP-mode lease investigation from `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`, its policy header/tests and `scripts/check_udhcpc_script_status.py`. Focus on real end-to-end state transitions, not the intentionally advisory `leasefail` return value.
3. On a concrete defect, implement the smallest source correction plus a regression, then use one complete Git tree/commit and a non-forced expected-SHA update. Check exact-SHA Actions and move to another independent task while any run is pending.
4. Distinguish host/CI L1-L3 evidence from physical-router L4; do not claim the latter.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Preserve security, architecture, compatibility and necessary test rules. Do not ask the user what to do next.
