## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and the one-time handoff recovery are complete; do not repeat the initial audit.
- Main before this source slice: `c70d33a49bbf981bcac61ab3e9f653c6fc35280c`. The networkmap IBOX ProductID boundary fix is complete and exact L1-L3 validated at production SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`; later commits are documentation only.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; its CA trust-store feature is integrated.

## Active task

- Fixed a reproducible BusyBox `udhcpc` discovery busy loop: `-T 0` is accepted by the `T+` option rule, and `-t 0` means unlimited retries. Without normalization, INIT_SELECTING receives a zero select timeout and immediately broadcasts DHCPDISCOVER again forever when no DHCP offer arrives.
- The source change clamps `discover_timeout` via the existing `udhcpc_retry_delay()` one-second floor immediately after `getopt32()`. Positive `-T` values and unlimited `-t 0` semantics are preserved; only a zero/nonpositive interval is throttled.
- Added a GNU89 host unit regression and a static state-machine contract asserting the timeout is normalized before INIT_SELECTING. Local isolated GNU89/Werror/ASan/UBSan test PASS. Exact-head udhcpc and XY-C1 Actions pending for the new commit.

## Exact-SHA CI checkpoint before this source slice

- Production source SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`: networkmap #46149948622 PASS; NVRAM boundary #46149948646 PASS; Process inventory #46149948705 PASS; XY-C1 #46149948587 PASS including build, TRX and artifact.
- Current handoff SHA `c70d33a49bbf981bcac61ab3e9f653c6fc35280c`: AI handoff documentation #46151071734 PASS.
- Physical router L4 remains NOT VERIFIED.

## Verification boundary

- L1: isolated GNU89/Werror/ASan/UBSan discovery retry floor regression PASS.
- L2: new exact-head udhcpc state-machine/source contract pending.
- L3: new exact-head XY-C1 Build/TRX/artifact pending; do not infer it from the previous firmware SHA.
- L4: physical router runtime and packet-rate behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Actions for this commit: udhcpc status/stability regression, AI handoff documentation and XY-C1 Build/TRX/artifact.
2. If any regression fails, inspect its exact failing step and make the smallest source/test correction in one complete-tree atomic commit. If firmware is pending, continue another independent reliability/security slice.
3. After validation, continue the LAN DHCP client/AP-mode lease investigation beyond this timer fix, especially actual renew/NAK/deconfig state transitions. Do not repeat the completed networkmap/NVRAM audits without new evidence.
4. Keep the frozen backup unchanged and distinguish CI L1-L3 from physical-router L4.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Preserve security, architecture, compatibility and necessary testing rules. Do not ask the user what to do next.
