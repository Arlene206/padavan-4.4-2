# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development continues on `main` only.
- Parent `main` for the current source slice: `5bec4f752ac0961c823a2750da03d34a3da0789a`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed and unmerged.
- Startup/protocol recovery was already completed once. Do not repeat it or re-audit completed work.

## Latest exact-SHA validation

- On parent `5bec4f752ac0961c823a2750da03d34a3da0789a`, WAN DHCP callback run #45783956641 PASS. Its new IPTV proxy NVRAM/IGMP config regressions and source contract all executed and passed; the existing WAN/IPv6 policy regressions also passed.
- Firewall Startup #45783956578, Process inventory #45783956580, Process fail-closed #45783956551, WAN route status #45783956522 and NVRAM boundary #45783956493 PASS on that exact SHA.
- AI handoff #45783956616 FAILED only because this checkpoint lacked the mandatory `Mandatory handoff checkpoint` section and its exact policy sentence. This commit adds that missing required checkpoint language; exact-head AI rerun is pending.
- XY-C1 run #45783956548 completed SUCCESS as a host-only validation. Build XY-C1, TRX verification and artifact upload were all SKIPPED because the parent contained only tests/workflow/docs changes. It is not new firmware L3 evidence; retain product-source L3 baseline `dd2885fc9941d3a57fd9c81cf01ed86488b0183d`, whose same-SHA Build/TRX/artifact all passed (artifact #15098382009). No physical router L4 is claimed.

## Active task

- Confirmed an internal WAN IPv6 SIT endpoint-route identity invariant defect: `wan6_sit_route4_record_format()` could serialize interface names of 16+ bytes, while `wan6_sit_route4_record_parse()` uses a 16-byte buffer and rejects those records. A record could therefore be persisted successfully but never parsed later for exact owned-route cleanup.
- The format helper now rejects names with length `>= WAN6_SIT_ROUTE4_IFNAME_SIZE` using `ENAMETOOLONG`. Regression coverage verifies a 15-character name round-trips and a 16-character name is rejected. This matches Linux's 15-byte interface-name payload bound and does not alter valid device behavior.
- The existing WAN DHCP callback workflow compiles `tests/wan6_identity_policy.c` with GNU89, `-Werror`, ASan and UBSan. Exact-head WAN/AI/XY validation after this source change is pending.
- No local repository checkout is available here; do not claim the original C suite or Python governance checker was run locally. This is a focused source/test change; build and CI status must be established on its exact commit SHA.

## Verification boundary

- On parent `5bec4f752ac0961c823a2750da03d34a3da0789a`, the WAN DHCP callback policy suite passed, including both newly wired IPTV regression binaries and their Python source contract. The same parent had the listed Firewall, Process, WAN-route and NVRAM checks PASS.
- On parent `5bec4f7`, XY-C1 workflow was host-only and skipped Build/TRX/artifact; this does not create a new product L3 result. Product-source baseline remains `dd2885fc9941d3a57fd9c81cf01ed86488b0183d`.
- The prior exact AI handoff run failed twice on required handoff contract fields. This revision now includes all required markers from `scripts/check_ai_handoff_docs.py`; verify the exact current SHA AI job rather than assuming it passes.
- The current SIT route identity source/test change has not yet received exact-head WAN/XY validation. No local checkout or original local C suite is claimed; physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head WAN DHCP callback, AI handoff and XY-C1 Actions for this commit. Confirm the SIT interface-name boundary regression executes/passes, and confirm the governance checkpoint contract is green.
2. For XY-C1, inspect the actual `Build XY-C1`, `Verify TRX output` and artifact-upload step conclusions. A skipped step is not a pass; only same-SHA Build + TRX + artifact PASS establishes product L3.
3. Continue the remaining P1 WAN/IPv6 route-edge backlog in `trunk/user/rc/src/net_wan6.c`; trace install/withdrawal, persisted ownership and first-error cleanup. Change production behavior only for a confirmed defect and add a focused regression.

## Mandatory handoff checkpoint

**Mandatory handoff checkpoint:** Every session MUST compare the live repository and relevant exact-head Actions with this file before ending. Verify the engineering/source baseline, active task and progress, validation boundary, blockers, and one exact first action for the next GPT. Do not repeat completed recovery or audits. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts are accurate, leave this file untouched and do not create a no-op commit. Update this handoff only when a material fact or next action changes, batching the correction with the engineering milestone whenever possible.

# Cross-session execution mandate

Continue as the same repository maintainer across conversations. Do not wait for conversational permission or ask what to do next when the live handoff defines the next action.

Every independently reviewable source/test milestone must be published as one atomic main commit: create all changed blobs, one complete Git tree and one commit, then update `main` with `force:false` and the expected parent SHA. Never publish partial-file commits or force-push.

Before context exhaustion, finish and commit a safe bounded slice, inspect exact-SHA Actions, and leave this handoff with the precise next task, verification state and blockers.

## Execution invariants

- Develop only on `main`; create complete blobs and one complete Git tree/commit per logical task. Update `main` non-forced with the expected parent SHA; never force-push.
- Preserve the frozen backup and do not reopen PR #1.
- Exact-SHA Actions are authoritative. PENDING is not a reason to wait idly; proceed with independent safe work.
- Firmware L3 requires same-product-source-SHA Build XY-C1, TRX verification and artifact upload all PASS. Physical router L4 requires actual target-device testing.
- Update the dev log for a completed engineering milestone. Update the roadmap only for a material module/status/priority change; never create routine handoff/roadmap archives.
