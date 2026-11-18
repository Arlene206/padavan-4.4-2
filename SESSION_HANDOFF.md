# 2026-11-18 continuation checkpoint — propagate manual WAN disconnect failures

## Current repository state
- Repository: Arlene206/padavan-4.4-2; parent main before THIS slice: 94b5ddbf49fd71a06ce83d907c9e6b3b23caadd0.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Exact parent 94b5 PID 1 notification queue regression 44209458419 PASS, including positive command-wait-status normalization. AI handoff 44209458097 PASS; AliDDNS 44209458249, VPN server 44209458344, Samba 44209458318, FRP 44209458244, infosvr 44209458058, rstats 44209458146, USB modem 44209458325, DNS Forwarder 44209458272, DNSMasq 44209458260, Firewall 44209458149 and iNIC 44209458219 PASS.
- Exact parent 94b5 WAN route 44209458297, WAN gateway ARP 44209458216, XY-C1 44209458268, NVRAM 44209458222 and several unrelated lifecycle workflows were IN_PROGRESS/QUEUED at last poll. Require same-SHA Build, Verify TRX output and artifact upload before recording L3.
- PID 1 manual WAN disconnect used void manual_wan_disconnect(), discarding every stop_wan()/stop_wan_ppp() error; rc.c called it without updating action_result. A failed manual disconnect was therefore acknowledged as complete, unlike the neighboring manual reconnect path.
- THIS slice converts manual_wan_disconnect() to int, returns the selected stop result, logs nonzero status, updates rc.h, and merges the status in PID 1 via rc_notify_merge_action_result(). This keeps the notification incomplete marker when manual teardown fails. AP mode remains a no-op returning zero.
- scripts/check_wan_detect_wan_status.py asserts the public prototype, modem/PPP/ordinary stop branches, error diagnostic, AP-mode no-op and PID 1 action-result aggregation. Existing WAN route workflow watches all changed C/checker paths and runs the GNU89/ASan/UBSan policy suite.
- No local MIPS firmware build or physical router L4 claim.

## Active task
- Propagate the actual manual WAN disconnect result end-to-end from stop_wan()/stop_wan_ppp() to PID 1 notification completion; prevent failure from being acknowledged as success.
- Exact-head WAN route regression and XY-C1 Build/TRX/artifact checks are pending for THIS commit.

## Verification boundary
- Exact parent 94b5 PID 1 notification queue, AI handoff, AliDDNS, VPN server, Samba, FRP, infosvr, rstats, USB modem, DNS Forwarder, DNSMasq, Firewall and iNIC checks PASS.
- Parent 94b5 WAN route 44209458297, XY-C1 44209458268, NVRAM 44209458222 and WAN gateway 44209458216 were not yet complete at last poll.
- THIS commit's exact-head WAN route source contract and policy sanitizer checks pending.
- Hardware L4 NOT VERIFIED; no same-SHA L3 claim.

## Exact first action for the next GPT
1. Inspect THIS commit's exact-head WAN route run. Require route status C GNU89/ASan/UBSan policy tests and the new manual WAN disconnect return/caller/source-contract assertions to PASS.
2. Inspect exact-head XY-C1 Build, Verify TRX output and artifact-upload steps on this same source SHA. Record L3 only if all three pass.
3. Check the current NVRAM and other pending security/lifecycle Actions. Continue independent P1 work while they run; do not wait idle.
4. Frozen backup remains untouched. Do not repeat bootstrap recovery.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end, inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and exact first action. If these facts remain accurate, no documentation edit or no-op commit is needed.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create dev branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
