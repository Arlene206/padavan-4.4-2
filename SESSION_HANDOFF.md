# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Current main before this CI-trigger correction: `01f56e97c65d45fe998be26c69569208a92f7670`; MiniUPnP allocation-safety source fix is `2ffb8479a33ad55a4138b3384a31510f5b7c534e`, followed by checker normalization `8dbb0963cb6b816bf13cf13726f5de0ce3c867fb`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; unchanged).
- Exact-head MiniUPnP regression #44789878471 on `8dbb096`: PASS, including the GNU89 ASan/UBSan allocation cleanup test and the whitespace-normalized source contract. AI handoff documentation #44789878451: PASS.
- Process lifecycle #44789544079 and NVRAM transaction #44789543972 on `2ffb847`: PASS.
- XY-C1 Firmware Validation did not run on `2ffb847` or `8dbb096`: its push path filters omitted the newly changed `iptcrdr.c` production file and its allocation-policy header. This commit adds those paths (and keeps the workflow's self-trigger) so the next exact-head push runs the XY-C1 Build/TRX/artifact L3 gate.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make MiniUPnP legacy iptables rule construction fail safely under memory pressure:

- Guard all seven `calloc()`-backed match/target factories before their first dereference; restore allocator errno after diagnostic logging.
- Guard all five rule builders (DNAT, optional MASQUERADE, peer SNAT, peer DSCP and filter ACCEPT). An entry, match, target or final rule-buffer allocation failure must release all prior components, preserve errno and return before iptc append/commit.
- Add GNU89 ASan/UBSan regression coverage for partial-component cleanup and errno preservation, plus a Python source contract for every factory and builder.
- Extend MiniUPnP push/PR path filters and workflow steps so the production and new checks cannot silently skip verification.

After exact-head MiniUPnP and XY-C1 validation, continue reviewing remaining netfilter rule installation/deletion transactions. Do not repeat the completed leasefail/NAK/release or AP restore audit.

## Verification boundary

- MiniUPnP allocation fix `2ffb847`: exact-head MiniUPnP L2 initially failed only on checker whitespace; checker-only correction `8dbb096` passed the complete MiniUPnP Action #44789878471, including all policy binaries and source contracts.
- AI handoff #44789878451, Process lifecycle #44789544079, and NVRAM transaction #44789543972 passed.
- XY-C1 L3 did not trigger because its path list omitted the new netfilter source/header. The current commit adds these paths; check the newly triggered exact-head Build, TRX verification and artifact upload before claiming L3.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, verify the backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, and check the exact-head XY-C1 run triggered by the new `iptcrdr.c`/header paths (Build, TRX and artifact upload).
2. Check any remaining Actions on the new SHA and diagnose first causal failures from their logs.
3. Continue MiniUPnP netfilter rule installation/deletion and partial-commit auditing; choose a confirmed P1 fault, implement regression coverage and commit atomically. Physical-router L4 remains unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
