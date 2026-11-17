# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. This xl2tpd result-normalization slice is one atomic commit parented to `f87343b7e7a2da599c16baf1192ec1561dc44a9b`.
- Exact parent `f87343b`: HTTPD `43992613516`, AliyunDrive `43992613319`, AliDDNS `43992613480`, Process lifecycle `43992613370`, VPN lifecycle `43992613425`, AI handoff `43992613327`, USB WDM/NDIS `43992613561`, Firewall `43992613416`, AdGuardHome `43992613329`, infosvr `43992613374`, and NVRAM `43992613294` all completed SUCCESS. XY-C1 `43992613368` passed every host regression through step 22 and was still in `Build XY-C1`; TRX verification and artifact upload were pending.
- A concrete L2TP result-collision defect is fixed here. `safe_start_xl2tpd_locked()` used to return raw positive `eval()` child exit statuses, while `1` is reserved to mean "already running"; callers that treat nonnegative as success could therefore accept a failed daemon launch. The result is now normalized to `-1` on every nonzero launch status while preserving `errno`. The WAN PPP launch path maps the documented already-running sentinel to success.
- The frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Run the xl2tpd policy regression and VPN lifecycle source checker on the exact commit SHA. If they pass, inspect exact-head XY-C1 Build/TRX/artifact completion and continue adjacent PPP lifecycle auditing.

## Verification boundary

- The new policy test covers daemon absent/running/unknown, child command exit statuses 1 and 127, and WAN-launch acceptance of the already-running sentinel. VPN workflow compiles it under GNU89 with `-Wall -Wextra -Werror`.
- Parent `f87343b` had all 11 non-XY workflows PASS. The XY-C1 cross-build was still running at Build step 23; TRX and artifact upload were not yet started. No L3 claim is made until all three pass on a single SHA.
- Product-source MIPS/uClibc L3 remains **NOT VERIFIED**; physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch exact-head VPN and XY-C1 Actions for this commit. Resolve source/test failures from exact logs.
2. Confirm same-SHA XY-C1 Build, TRX validation, and artifact upload; while pending, continue an independent PPP/L2TP lifecycle slice rather than waiting.
3. Rotate to the next unverified P1 stability/security issue after the PPP lifecycle review. Keep the backup frozen and use non-forced expected-SHA main updates.
