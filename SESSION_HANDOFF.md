# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Live main parent for this validator follow-up: `2c19a2172400fb3b775168c499f1148bb3c6b5a6`.
- Production implementation under validation: `10b57237ced5487a0d17e0bb09425ea96973a78e`. The shared `dhcp_lease_nvram_publish()` validates all values, attempts every candidate write despite an individual failure, preserves the first publication error, and restores the complete previous tuple in reverse order.
- WAN `udhcpc_renew()` publishes changed DNS/domain/WINS/lease duration through one serialized NVRAM transaction; publication errors are returned, and resolver refresh is withheld until the full snapshot succeeds.
- PPP ip-up publishes `ipaddr/netmask/ifname_t/gateway/dns` under a transaction, restores the tuple under a fresh transaction after failure, removes the interface address on failed publication, and gates WAN route setup on success.
- The injected GNU89/Werror/ASan/UBSan attempt-all/full-rollback regression passed in WAN DHCP callback run `44689620059` on `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`. NVRAM transaction boundary run `44689024044` passed on source `10b5723`. AI handoff documentation run `44689831389` passed on current parent.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only; branch inventory last verified only main and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; AliDDNS HMAC/CA-bootstrap work is already present and evolved on main. Do not repeat that port audit.

## Active task

1. Repair the remaining WAN/PPP source-checker mismatch: production `ppp_wan_ipup_publish_address()` uses `dhcp_lease_nvram_publish()` under an NVRAM transaction and `dhcp_lease_nvram_restore()` under a fresh transaction; the route checker incorrectly expected `rc_wan_nvram_set_all_values()`. The checker now asserts the actual shared publisher.
2. Repair HTTPD's embedded DHCP-renew contract: its old marker was merely a removed source comment (`dnsmasq domain-specific server routing too`). The actual behavior marks DNS/domain changes via `resolver_changed`, returns on snapshot publication error, and only then refreshes resolver state. The workflow contract now checks that ordering.
3. Inspect exact-SHA WAN route, HTTPD, WAN DHCP callback, AI handoff and XY-C1 runs for this correction. Correct causal failures without weakening transactional rollback or route gates.
4. After those gates pass, inspect `udhcpc_bound()` for partial address-change lease tuple writes and add an injected regression plus fail-closed atomic source fix where a real defect remains.

## Verification boundary

- PASS: WAN DHCP callback `44689620059` on `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`; AI handoff docs `44689831389` on `2c19a2172400fb3b775168c499f1148bb3c6b5a6`; NVRAM boundary `44689024044` on production source `10b57237ced5487a0d17e0bb09425ea96973a78e`; injected attempt-all/full-rollback test within WAN DHCP callback passed under GNU89/Werror/ASan/UBSan.
- FAIL, corrected in this source-contract follow-up: WAN route `44689432208` and XY-C1 `44689432315` on `5e7d8cf`, plus XY-C1 `44689620163` on `b5146272`; all failed on the stale PPP helper marker. HTTPD `44689024189` on `10b5723` failed on the removed comment-only renewal marker.
- XY-C1 has not yet passed the same-production-SHA detector, MIPS build, TRX verification and artifact upload. L3 is NOT VERIFIED. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head WAN route, HTTPD, WAN DHCP callback, AI docs and XY-C1 Actions for this correction. For each FAIL, open the failed job log and repair the real contract issue without weakening checked-write, attempt-all, full-tuple rollback, resolver-after-commit or WAN route-gating assertions. Once green, audit `udhcpc_bound()` for unchecked/partial address-change tuple publication and make the next production source slice independently testable and atomic. Claim L3 only when one production source SHA passes detector, MIPS build, TRX and artifact upload; keep L4 NOT VERIFIED without device evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. A same-production-SHA detector, MIPS build, TRX verification and artifact upload are required for L3. Never infer L4 from CI.
