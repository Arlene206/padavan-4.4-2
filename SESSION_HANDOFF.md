# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify live main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. A checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait for pending CI when independent work is available and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Current source is the HW NAT/FTP/SFE tri-state fix in `1554a5f3fbb1bf13c3325ad765b7e806d74d1537`; current parent at this checkpoint: `ebf2d285454eba3480fe594808aa31367513ffe4`.
- The 1554 exact-head Process `43969810170`, Firewall Startup `43969809710` and XY-C1 `43969809836` shared one stale source-checker assertion that required `return -module_loaded;`, while the implementation correctly assigns `status = -module_loaded;` and returns `status`. The HTTPD run `43969809668` had a separate stale contract expecting `hwnat_loaded` passed without checking it was positively verified. This commit updates those test contracts.
- Exact-head ebf2 AI handoff `43970209185`, NVRAM `43970209177`, AdGuardHome `43970209909`, infosvr `43970209228`, AliyunDrive `43970209300` and AliDDNS `43970209283` passed. Process inventory `43970209366` failed at the same stale common checker assertion; this commit fixes it. Other 1554 exact runs passed USB modem identity, infosvr, WAN route, AliDDNS, AdGuardHome and AliyunDrive.
- d287 exact-head XY-C1 `43967039935` passed Build, TRX and artifact upload; its L3 result does not cover the later production source changes. Same-SHA XY-C1 is required for the 1554 source slice.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains recorded at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Never alter it. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Validate the production tri-state HW NAT/FTP/SFE changes after correcting two stale test expectations. HW NAT probe errors propagate through the errno-style status; disabled mode attempts strict unload; radio writes require loaded==1. FTP state UNKNOWN blocks parameter mutation. SFE always calls strict load/unload verification. The common RC checker asserts source callsites; the HTTPD workflow now asserts strict HW NAT preconditions.

## Verification boundary

- The source checker and HTTPD integration contracts have been corrected but their new exact-head runs are pending.
- Existing module-policy tests already exercise injected load probe errors, initial/post unload probe errors, busy modules and first-error aggregation.
- Same-SHA Process, Firewall Startup, HTTPD, XY-C1 and AI handoff must pass before this source slice is marked validated. Firmware L3 is verified for d287 only, not for the new HW NAT/FTP/SFE source. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-head Process, Firewall Startup, HTTPD, XY-C1 and AI handoff runs for the HW NAT/FTP/SFE source and checker correction. Fix any remaining failures from their exact logs.
2. Continue P1 RC/network result propagation. `flush_conntrack_table()` under `RCN_RESTART_NETFILTER` is the next independent candidate; trace the implementation and every caller before changing its API.
3. Keep complete-tree atomic commits with non-forced expected-SHA updates; keep the backup read-only and do not claim physical-router L4 without hardware evidence.
