## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical source/test slice, create all changed blobs, one complete Git tree based on the current main root tree, and one commit; then update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Do not spend another takeover repeating completed audits.

## Branch and checkpoint

- Current source baseline before this documentation checkpoint: `717f983f07434cd78a4ad00ebdb944b8f6d30fa5`.
- This handoff/log/roadmap refresh is a single documentation-only child of that baseline. The live main SHA is authoritative; fetch it on takeover instead of assuming the SHA written here equals current main.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- `main` is the only development ref. PR #1 remains closed/unmerged and deeply behind main; do not port it wholesale.
- The source tree at 717 was verified against original main baseline `054ff22bf517a45bf29f2da21963a4206f8b3b83`: GitHub compare reports exactly four modified files, namely udhcpc source, policy header, policy test, and static checker. The root tree contains the expected repository directories and the full current repository content.

## Active task

### BusyBox udhcpc — DHCPNAK identity handling in REBINDING

Completed source slice:
- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`: the DHCPNAK branch now tells the identity policy whether the client is in `REBINDING`.
- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcpc_script_policy.h`: outside REBINDING, NAKs remain pinned to the selected server; during REBINDING, the client accepts an identified NAK from any server because the DHCPREQUEST is broadcast. Missing server-ID remains rejected while rebinding. The no-known-server compatibility behavior before server selection is preserved.
- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcpc_script_policy.c`: added coverage for alternate-server acceptance during REBINDING, missing IDs, and unrelated server IDs outside REBINDING.
- `scripts/check_udhcpc_script_status.py`: source contract pins the state-aware filter call and tested policy.
- Exact-head udhcpc workflow #45489316871 on source SHA `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` is PASS. The test job compiled with GNU89, `-Wall -Wextra -Werror`, ASan/UBSan, ran the policy binary, and passed the source-contract checker.

Why the fix matters: a DHCPREQUEST in REBINDING is broadcast so any DHCP server can answer. A strict comparison against the old selected server can discard a legitimate NAK from a replacement server, leaving an invalid old address configured until lease expiry. The state-aware policy preserves strict server matching in non-rebinding states.

### Git object recovery — disclose and preserve

The initial source commit `ff9cb0ec0f9143cd398d8de1d9b4eb13dca87be5` was mistakenly built with an empty tree base. Its root tree omitted unrelated paths. The mistake was detected by checking GitHub's commit statistics before claiming the commit was validated. A non-forced child commit `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` immediately restored the full repository tree from the actual parent tree and retained the four intended DHCP file changes. A REST compare from original main `054ff22bf517a45bf29f2da21963a4206f8b3b83` to 717 confirms exactly those four files changed and no others. The current tree is complete; the udhcpc workflow passed on it. Do not rewrite history or attempt to erase the intermediate commit; continue from live main. For every future Git Data API commit, fetch the current commit's `commit.tree.sha` and pass it as `base_tree_sha`; never use an empty base tree.

## Verification boundary

### Current exact source SHA 717

- BusyBox udhcpc event-script status stability #45489316871: PASS.
- Process lifecycle inventory #45489318666: PASS.
- Router runtime NVRAM reset #45489318744: PASS.
- MiniDLNA config safety #45489318793: PASS.
- DNSMasq lifecycle #45489318703: PASS.
- EAPoL configuration/credential escaping #45489318738: PASS.
- AI handoff docs #45489318581: FAIL only at its explicit checkpoint assertion: current `SESSION_HANDOFF.md` did not contain the mandatory end-of-session handoff checkpoint rule. This documentation-only checkpoint adds the required section and phrase; verify the new exact-head AI workflow after this commit.
- XY-C1 firmware validation #45489316847: at last poll, 1–36 passed; production-input detector step 37 was IN_PROGRESS; Build step 38, TRX step 39 and artifact upload step 40 were PENDING. Do not claim L3 for 717 until all three product steps are PASS on this exact source SHA.
- WAN DHCP callback #45489318671: QUEUED at last poll.
- VSFTPD configuration publication #45489317086: IN_PROGRESS at last poll.
- Other current-head workflows may have progressed since this checkpoint; fetch the live Actions list and handle any FAIL before declaring the corresponding slice validated. A pending run is not a reason to stop independent work.

### Earlier exact source SHA 7e700e86e191bfbf11d563fce7f9cfbbcd539516

- VSFTPD quarantine regression #45486633987: PASS.
- AI handoff #45486633983: PASS.
- Process lifecycle inventory #45486634249: PASS.
- NVRAM transaction boundary audit #45486634206: PASS. It scanned 4,581 userland source files, found 23 raw ioctl references inside the locked adapter and zero raw userland bypasses; kernel-source scan found zero NVRAM API/writer references; board configuration inventory passed. This is a bounded scan, not proof of global atomicity against arbitrary external processes.
- XY-C1 firmware #45486634245: PASS through Build XY-C1, TRX verification and artifact upload on this exact SHA. L3 is VERIFIED for 7e only; do not project that firmware artifact to later product-source SHAs.
- No physical-router validation has occurred. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch the live main ref and exact-head Actions. Inspect the new AI handoff run triggered by this documentation checkpoint; the previous #45489318581 failure should be closed only after its required checkpoint assertion passes.
2. Recheck XY-C1 #45489316847 for source SHA 717. If Build, TRX and artifact upload all pass, record L3 for 717; if a step fails, inspect its log and fix the first cause in one new atomic main slice. Also revisit WAN DHCP callback #45489318671 and VSFTPD #45489317086, processing any completed FAIL rather than waiting on queued runs.
3. Continue repository-wide stability work after those checks. LAN DHCP/AP-mode lease remains PARTIAL/AUDITING: the state-aware DHCPNAK fix is now covered, but `leasefail`, full live lease/NAK transitions, target MIPS/uClibc runtime and physical L4 remain open. Do not reopen the completed NAK policy regression or repeat the 7e NVRAM/XY audits.
4. Select the next concrete high-risk source defect from the module dashboard in `REFACTOR_ROADMAP.md`; prioritize severe service/network failure and security issues, then implement source + regression, run relevant tests, atomically commit and inspect that exact SHA's Actions. PENDING means move to another independent task.
5. Before context exhaustion, perform the mandatory handoff checkpoint and update the root handoff in place only if material facts or the next action changed.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare the live repository, current exact-head Actions, active source baseline, validation boundary, blockers and the first next action against this file before ending. Record a newly resolved relevant PENDING/FAIL gate. If all facts remain accurate, explicitly conclude the checkpoint with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
