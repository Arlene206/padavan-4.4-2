# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at their cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Compare live main with this record before every handoff. Record only material changes to the active task, validation boundary, blockers or exact first action.

## Current repository baseline
- Product-source baseline: `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`; live main before this checker repair: `474eec0dc8b19630c62b13713731c4a3976073ce`. The current changes are checker/workflow and continuity records only.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed/unmerged.
- Router-mode production fix on `d9bffce5` uses checked transactional NVRAM reads/writes, preserves AP-mode/no-op and absent-key defaults, validates full decimal values, and gates WAN stop plus NETFILTER NAT/firewall reload before side effects. Dedicated router-mode regression #44904910387 PASS on `0398903`; AI docs #44905019105 PASS on `474eec0`.
- XY-C1 #44904764739 on `d9bffce5` stopped before target build at the WAN caller source contract. Commit `474eec0` attempted whitespace-tolerant regex counting but used doubled backslashes in the raw expression, so the regex searched for literal backslashes. The same assertion then failed in four exact-head workflows on `474eec0`: USB modem #44905019162, Firewall Startup #44905019100, Process lifecycle #44905019098, and XY-C1 #44905019103. Policy tests and preceding WAN source contracts passed before the shared assertion failed. This commit uses the existing `compact(RC_SOURCE)` whitespace normalizer and retains exact count=2.
- USB storage #44904764585 previously passed all policy binaries, BusyBox/ash Optware tests, storage inventory and prior teardown checks, then failed its last Python gate on stale NFS assumptions: it expected the start launcher inside `run_nfsd_locked()`. This commit replaces that obsolete assertion with checked config/error propagation, enabled-only start/reload export gates, and restart config-read-before-inventory/stop plus checked-value reuse.
- NVRAM transaction audit #44904764756 PASS on source `d9bffce5`. Router-mode workflow #44904910387 PASS on `0398903`. HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5`. Exact reruns from this checker repair are pending.
- Previous XY-C1 #44903863658 fully passed Build XY-C1, TRX verification and artifact upload on product SHA `b284c28532f2913f4327b02ec59abaf97671be08` only. The later `d9bffce5` product update has not yet passed target build; the four XY stages skipped after source-contract failure. Do not claim current L3 from the older SHA.
- MiniUPnP SOAP input validation remains ranked P1 in the roadmap: reject internal port zero in AddPortMapping/AddAnyPortMapping with bounded non-zero parsing while retaining permitted external-port-zero semantics.

## Active tasks
1. Inspect exact-head USB storage, WAN source-check, Process, Firewall and XY-C1 runs from this contract repair. If any fails, read its first causal log line and correct the exact contract without dropping meaningful assertions.
2. Once the corrected XY-C1 host source gate passes, continue to a real MiniUPnP internal-port validation source/test slice. Its production-source commit should trigger XY-C1's production-input detector and consolidate the full MIPS build/TRX/artifact check for both current router-mode source and MiniUPnP fix.
3. Inspect Build XY-C1, TRX verification and artifact upload individually; claim L3 only if all three pass on one exact production-source SHA.

## Verification boundary
- Router-mode GNU89/Werror/ASan/UBSan policy and source contract PASS on `0398903`; AI docs PASS on `474eec0`.
- On `474eec0`, shared WAN source check fails in the overescaped checker regex; USB modem, Firewall, Process and XY jobs stop at that same assertion. Their prior policy/test stages passed; target firmware steps did not execute.
- USB storage host policy/lifecycle stages passed before the stale NFS source assertion on `d9bffce5`; the new checker assertions await exact-head rerun.
- NVRAM audit PASS on `d9bffce5`. The only fully proven later L3 cited here is old product SHA `b284c285`; no L3 for `d9bffce5`. Physical router L4 NOT VERIFIED.
- No local checkout/runtime or local RC compile/sanitizer run was available in this session.

Before context exhaustion, update this handoff when the validation boundary or next task changes; do not make no-op edits.

## Exact first action for the next GPT
Fetch live main and inspect the exact-head USB storage, shared WAN source-check (USB modem/Firewall/Process/XY) and AI docs runs. If those source contracts pass, proceed immediately to the MiniUPnP internal-port validation production fix while any unrelated jobs remain pending. The MiniUPnP source commit should trigger the complete XY-C1 build for the combined current product state.

Update this handoff only when the active engineering item, exact-head validation state, verification boundary or next action materially changes.