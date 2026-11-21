# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at their cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record; change the file only when task, exact-head result, validation boundary, blocker, or first action materially changes. Compare live main with this record before every handoff. Record only material changes to the active task, validation boundary, blockers or exact first action.

## Current repository state
- Product-source baseline: `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`; live main before this handoff correction: `2222f27be79a7f785a48887047403e29f360ef59`. Commits after the source baseline in this chain are checker/workflow and continuity records only.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed/unmerged.
- Router-mode production fix on `d9bffce5` uses checked transactional NVRAM reads/writes, preserves AP-mode/no-op and absent-key defaults, validates full decimal values, and gates WAN stop plus NETFILTER NAT/firewall reload before side effects. Dedicated router-mode regression #44904910387 PASS on `0398903`; AI docs #44905019105 PASS on `474eec0`.
- XY-C1 #44904764739 on `d9bffce5` stopped before target build at the WAN caller source contract. Commit `474eec0` attempted whitespace-tolerant regex counting but used doubled backslashes in the raw expression, so the regex searched for literal backslashes. The same assertion then failed in four exact-head workflows on `474eec0`: USB modem #44905019162, Firewall Startup #44905019100, Process lifecycle #44905019098, and XY-C1 #44905019103. Policy tests and preceding WAN source contracts passed before the shared assertion failed. This commit uses the existing `compact(RC_SOURCE)` whitespace normalizer and retains exact count=2.
- USB storage #44904764585 previously passed all policy binaries, BusyBox/ash Optware tests, storage inventory and prior teardown checks, then failed its last Python gate on stale NFS assumptions: it expected the start launcher inside `run_nfsd_locked()`. This commit replaces that obsolete assertion with checked config/error propagation, enabled-only start/reload export gates, and restart config-read-before-inventory/stop plus checked-value reuse.
- NVRAM transaction audit #44904764756 PASS on source `d9bffce5`. Router-mode workflow #44904910387 PASS on `0398903`. HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5`. Exact reruns from this checker repair are pending.
- XY-C1 run #44905479795 on `2222f27be79a7f785a48887047403e29f360ef59` concluded success, but Build XY-C1, Verify TRX output and artifact upload were all SKIPPED because this head contains no product-source changes. This is not L3 firmware validation for the `d9bffce5` product baseline. The older run #44903863658 passed all three firmware stages only on product SHA `b284c28532f2913f4327b02ec59abaf97671be08`.
- MiniUPnP SOAP internal-port-zero validation is already implemented: AddPortMapping and AddAnyPortMapping use `miniupnpd_soap_parse_nonzero_port()` for the internal port; regression tests cover zero rejection while retaining AddAnyPortMapping external-port-zero behavior. The change is in commit `1eb75818300dbefc79fbd3f706670df0e3011a6a`; do not reopen without new evidence.

## Active task
1. The checked NFS, modem/WAN source contract, firewall startup and process-inventory regressions now PASS on checker/docs SHA 2222f27be79a7f785a48887047403e29f360ef59. AI handoff documentation #44905479770 failed because this file used noncanonical headings; this correction supplies the required names.
2. Implement the confirmed SSH availability defect in trunk/user/rc/src/services.c: start_sshd() uses error-collapsing nvram_get_int(sshd_enable); restart_sshd() stops the existing daemon before that read, so a transient NVRAM failure can leave remote management stopped. Add checked mode reads, regression cases and update the existing process-lifecycle contract.
3. After the SSH source commit, inspect exact-SHA process-lifecycle and XY-C1 Build, TRX and artifact upload independently. The previous XY-C1 #44905479795 had all three firmware steps SKIPPED; it did not validate the product-source baseline.

## Verification boundary
- Router-mode GNU89/Werror/ASan/UBSan policy and source contract PASS on `0398903`; AI docs PASS on `474eec0`.
- On `474eec0`, shared WAN source check fails in the overescaped checker regex; USB modem, Firewall, Process and XY jobs stop at that same assertion. Their prior policy/test stages passed; target firmware steps did not execute.
- USB storage host policy/lifecycle stages passed before the stale NFS source assertion on `d9bffce5`; the new checker assertions await exact-head rerun.
- NVRAM audit PASS on `d9bffce5`. The only fully proven later L3 cited here is old product SHA `b284c285`; no L3 for `d9bffce5`. Physical router L4 NOT VERIFIED.
- No local checkout/runtime or local RC compile/sanitizer run was available in this session.

Before context exhaustion, update this handoff when the validation boundary or next task changes; do not make no-op edits. The checkpoint is mandatory; a document edit is conditional.

## Exact first action for the next GPT
Fetch live main and inspect the exact-head USB storage, shared WAN source-check (USB modem/Firewall/Process/XY) and AI docs runs. If those source contracts pass, proceed immediately to the MiniUPnP internal-port validation production fix while any unrelated jobs remain pending. The MiniUPnP source commit should trigger the complete XY-C1 build for the combined current product state.

Update this handoff only when the active engineering item, exact-head validation state, verification boundary or next action materially changes.