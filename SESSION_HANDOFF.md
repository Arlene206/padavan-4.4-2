## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical source/test slice, create all changed blobs, one complete Git tree based on the current main root tree, and one atomic main commit; then update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Do not spend another takeover repeating completed audits.

## Branch and checkpoint

- Current main before the wireless radio source slice: `cdf1517864852839b6651a5a82fa76add097bc10`, a documentation-only child of source baseline `717f983f07434cd78a4ad00ebdb944b8f6d30fa5`. Fetch the live main ref before every write; never assume a stored SHA remains current.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- `main` is the only development ref. PR #1 remains closed/unmerged and deeply behind main; do not port it wholesale.
- The source tree at 717 was verified against original main baseline `054ff22bf517a45bf29f2da21963a4206f8b3b83`: GitHub compare reports exactly four modified files, namely udhcpc source, policy header, policy test, and static checker. The root tree contains the expected repository directories and the full current repository content.

## Active task

### Wireless radio control — checked state reads and truthful notifications

Review found that `is_radio_on_wl/rt()` use the legacy `get_mlme_radio_wl/rt()` NVRAM getters on IWPRIV/RT3352-MII boards. A failed read collapses to zero. Control and notification paths then interpret the failed read as “radio already OFF”; an explicit OFF request may be reported as a successful no-op, and manual toggle may skip the operation.

This source slice adds a status-bearing `wireless_radio_power_status()` using checked MLME reads, negative status propagation from radio control, checked `wl_radio_x`/`rt_radio_x` enable reads for manual toggles, and `wireless_radio_notification_status_result()` so state-reader or operation errors cannot become successful no-ops. Legacy boolean readers remain available for display-only callers. GNU89/ASan/UBSan regressions and exact-head source-contract checks must pass; XY-C1 Build, TRX and artifact upload must also pass on the same SHA before claiming L3.

### Completed concurrent source slice — BusyBox udhcpc DHCPNAK identity handling in REBINDING

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
- Wireless radio state regression #45489316780: PASS on 717.
- NVRAM transaction boundary audit #45489317079: PASS on 717.
- WAN DHCP callback #45489318671 and VSFTPD configuration publication #45489317086: PASS on 717.
- AI handoff docs #45489793605 on documentation child cdf: FAIL at the exact handoff assertion requiring the phrase `one atomic main commit`; this update adds that wording. Earlier 717 AI run #45489318581 failed at the missing mandatory checkpoint section, also present in the current handoff. Verify the next exact-head AI run.
- XY-C1 firmware validation #45489316847: at last poll, steps 1–37 PASS, Build step 38 IN_PROGRESS, TRX step 39 and artifact upload step 40 PENDING. Do not claim L3 for 717 until all three product steps are PASS on that exact source SHA.
- Other current-head workflows may have progressed since this checkpoint; fetch the live Actions list and handle any FAIL before declaring the corresponding slice validated. A pending run is not a reason to stop independent work.

### Earlier exact source SHA 7e700e86e191bfbf11d563fce7f9cfbbcd539516

- VSFTPD quarantine regression #45486633987: PASS.
- AI handoff #45486633983: PASS.
- Process lifecycle inventory #45486634249: PASS.
- NVRAM transaction boundary audit #45486634206: PASS. It scanned 4,581 userland source files, found 23 raw ioctl references inside the locked adapter and zero raw userland bypasses; kernel-source scan found zero NVRAM API/writer references; board configuration inventory passed. This is a bounded scan, not proof of global atomicity against arbitrary external processes.
- XY-C1 firmware #45486634245: PASS through Build XY-C1, TRX verification and artifact upload on this exact SHA. L3 is VERIFIED for 7e only; do not project that firmware artifact to later product-source SHAs.
- No physical-router validation has occurred. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Wireless radio state regression for this checked-state slice. Fix the first compiler, GNU89/ASan/UBSan, or source-contract failure and rerun on the new SHA.
2. Inspect same-SHA XY-C1 Build, TRX and artifact-upload steps. The older 717 run #45489316847 had steps 1–37 PASS and Build step 38 IN_PROGRESS at last poll; only all three product steps PASS on one SHA permit L3.
3. Check the AI handoff documentation contract after the exact-phrase fix; then process any remaining FAIL from the exact-head Actions list.
4. Continue with the next P1 stability/security defect in `REFACTOR_ROADMAP.md`. LAN DHCP/AP-mode lease remains PARTIAL/AUDITING (`leasefail`, full live lease/NAK transitions, target MIPS/uClibc runtime and physical L4 remain open). Do not reopen the completed DHCPNAK policy or closed VSFTPD/process/NVRAM checks.
5. Before context exhaustion, perform the mandatory handoff checkpoint; edit root docs only when a material fact or first action has changed.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare the live repository, current exact-head Actions, active source baseline, validation boundary, blockers and the first next action against this file before ending. Record a newly resolved relevant PENDING/FAIL gate. If all facts remain accurate, explicitly conclude the checkpoint with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
