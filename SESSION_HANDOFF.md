# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Current source SHA: `b7aa7296aab8274461a68182402357352a9d905e`.
- LAN IPv6 cleanup/reload failure propagation is implemented; regression tests pass. Three dynamic WAN IPv6 address additions in `net_wan6.c` were moved from shell-formatted `doSystem()` to argv-based `eval()`, preserving the existing WAN first-error/fail-fast macros.
- Exact `b7aa7296aab8274461a68182402357352a9d905e` Process `43983424164`, Process inventory `43983424187`, Firewall Startup `43983424297`, WAN route `43983424396`, AI handoff `43983424148`, HTTPD security `43983424199`, AdGuardHome `43983424381`, AliDDNS `43983424438`, infosvr `43983424401`, USB modem identity `43983424177`, and AliyunDrive `43983424352` PASS.
- Exact `b7aa7296aab8274461a68182402357352a9d905e` WAN DHCP callback `43983424171` failed only in its inline source checker: it still expected the old shell-formatted SIT/native address-add commands inside `WAN6_*_CHECK(doSystem(...))`. The production change deliberately uses `WAN6_*_CHECK(eval(...))`; update only those checker assertions. The policy/source code itself was not reported as failing.
- Exact XY-C1 `43983424207` passed all host/source regression steps and was building the XY-C1 firmware; TRX verification and artifact upload remained pending at the last poll. Current-source L3 remains **NOT VERIFIED** until Build/TRX/artifact all pass on one exact SHA.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Verify the WAN IPv6 argv-safety change and its integration checker. The workflow-only correction in this commit updates the inline SIT/native operation and status-gating assertions to expect the argv-based calls, and adds guards against restoring shell-formatted IPv6 address additions. Then continue auditing remaining dynamic `doSystem()` arguments in `net_wan6.c`/IPv6 helpers; distinguish validated interface names and generated literals from raw NVRAM input.

## Verification boundary

- Exact `b7aa7296aab8274461a68182402357352a9d905e`: Process, Firewall Startup, WAN route, AI handoff, HTTPD security and related source suites passed.
- Exact WAN DHCP callback `43983424171` failed at a stale inline checker expecting removed `doSystem("ip -6 addr add %s dev %s", ...)` calls. This commit corrects that test expectation; no production C source changes in this checker-only slice.
- Exact XY-C1 `43983424207`: all host checks passed; Build was still in progress, TRX and artifact upload pending. Product-source L3 **NOT VERIFIED**; physical-router L4 **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch exact-head WAN DHCP callback, WAN route, Process, Firewall, AI handoff and XY-C1 Actions for the checker correction; inspect any failing step logs and resolve the earliest causal failure.
2. Confirm the exact-head XY-C1 Build, TRX verification and artifact upload. Do not claim product-source L3 before all three pass on one SHA.
3. Continue the IPv6 command audit. Review native-interface provenance and the remaining route/tunnel `doSystem()` arguments; convert dynamic inputs to argv only after validating types and preserving cleanup/status behavior. Keep the backup branch frozen and use non-forced expected-SHA updates.
