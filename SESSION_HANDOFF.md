# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Current production source SHA: `10b57237ced5487a0d17e0bb09425ea96973a78e`; documentation-only handoff commit follows it.
- Policy/test foundation: `6c0ba561b93e9e4c6ee647718a251d7e4173d727` changes `dhcp_lease_nvram_publish()` to validate all entries, attempt every candidate write even after an individual error, and restore all previous values in reverse order while preserving the first write error.
- WAN source: `udhcpc_renew()` publishes changed DNS/domain/WINS/lease as a serialized runtime snapshot. It retries restoration in a fresh transaction on any failed write or transaction-end result and returns the original publication error rather than reporting renewal success.
- PPP source: `ipup_main()` uses the same shared policy for the five-field `ipaddr/netmask/ifname_t/gateway/dns` snapshot. It holds an NVRAM transaction during candidate writes, retries complete restoration under a fresh transaction and retains the existing interface rollback and route gate.
- WAN DHCP callback and WAN firewall source contracts were updated; the injected GNU89 test verifies validation-before-mutation, attempt-all writes and full tuple restoration.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Branch inventory last verified only `main` and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; AliDDNS HMAC/CA bootstrap work is already present and evolved on main. Do not repeat that port audit.

## Active task

1. Inspect exact-SHA Actions for production source `10b57237ced5487a0d17e0bb09425ea96973a78e`: WAN DHCP callback `44689024028`, WAN route `44689024130`, XY-C1 `44689024255`, AI handoff docs `44689024296`. Also poll NVRAM boundary `44689024044` and fix any failure from the failed step log.
2. Confirm the injected `dhcp_lease_nvram_publish()` regression compiles/runs under GNU89, Werror, ASan and UBSan. Verify the updated WAN renew and PPP source-contract assertions.
3. Claim L3 only if the same production-source SHA passes the input detector, MIPS build, TRX verification and artifact upload. Host-only CI does not establish L3; physical-router L4 remains NOT VERIFIED.
4. Once the DHCP renewal/PPP slice is green, continue with the address-change `udhcpc_bound()` path, which still publishes several lease values individually and needs the same fail-closed snapshot treatment.

## Verification boundary

- Previous main SHA `79d8f0a7cc59be4e96ee827ff09fcad0576b3720`: WAN route PASS (`44687337571`); AI docs FAIL (`44687337698`) because the handoff had uppercase `One atomic main commit`. Corrected here using the exact lowercase validator phrase.
- Production source SHA `10b57237ced5487a0d17e0bb09425ea96973a78e`: source Actions were in progress at the time of this handoff. No PASS/FAIL result is claimed for this SHA yet.
- No local GNU89 suite was run because no local checkout is available. No same-SHA MIPS/TRX/artifact result is yet confirmed; L3 is NOT VERIFIED. Physical L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main, then inspect exact-SHA results for the production source SHA above. Open failed job logs and repair the actual problem without weakening validation, attempt-all, rollback or route-gating assertions.
2. If the new source slice passes, investigate `udhcpc_bound()` for unchecked/partial lease tuple publication; add a focused injected policy regression and atomic source fix.
3. Do not wait on PENDING Actions when independent source work is possible. Record the exact commit/SHA and test status in the next handoff.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. A same-SHA detector, MIPS build, TRX verification and artifact upload are required for L3. Never infer L4 from CI.
