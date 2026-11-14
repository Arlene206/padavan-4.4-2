## 2026-11-14 08:43 CST (UTC+8) — Align live handoff with current documentation checkpoint

- Base main SHA: `7f8591436413161ad84124db301223b5c8448eaa`; active branch `main`. Refreshed `SESSION_HANDOFF.md` in place to distinguish the current documentation tip from the older recovery baseline and record the NVRAM push runs [43535751164] (still IN_PROGRESS on last check) and [43536039560] (QUEUED on exact log SHA `6a905a64f8200ef531d78f2755a96a149502b956`). No source or test files changed.
- Product-source comparison remains unchanged: 57 commits ahead / 0 behind from the historical L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`, with only `REFACTOR_ROADMAP.md`, `SESSION_HANDOFF.md`, and `dev_log.md` differing.
- This entry is part of the current documentation commit: THIS COMMIT. Root log advances 65→66 complete records; no rotation or INDEX update is due.
- L1 source review/PR triage only; L2 scanner host regressions and local docs checker NOT RUN (no checkout); L3 historical-only; L4 NOT VERIFIED.
- Exact next action: re-fetch live main and inspect exact-head push runs for this commit, recheck NVRAM runs [43535751164] and [43536039560], then complete exhaustive trunk/user process scanner/lifecycle caller enumeration before any tri-state API edit.

---

## 2026-11-14 08:42 CST (UTC+8) — Reconcile exact main Actions and complete legacy PR preservation triage

- Base main SHA: `0f3ddb47a02cb9b7cfeadeaaa0546d2018d8a488`; active branch `main`. Re-fetched live `refs/heads/main` and confirmed this exact base before the documentation update.
- Recovered the active P1 shared process-inventory fail-closed task. Re-read the scanner and lifecycle source at this base: `pids.c` blob `f5092806d6f1a76021643b0af54eb24705cbd8fe`, `shutils.h` `6ccb679cf049374839a0c5de6a543955614dc275`, `common_ex.c` `de5cc703ce1b9e2eb0897dc52ed7e1c5ec15eb97`, and `service_lifecycle_policy.h` `abe9f77b5354d8a33bce66c2575d1b1d09a2d4e2`. Confirmed NULL scan/allocation/`/proc` open failures and `readdir()` errors still collapse into end-of-scan/absence; lifecycle stop still sends TERM before authoritative inventory. Full cross-userland caller classification remains INCOMPLETE, so protocol correctly prohibits a partial API migration. No product source or tests changed.
- Reconciled all six exact-head `push` Actions on base `82ed2de22c73523fe39521d23a4a4f30768539e0`: AdGuardHome [43535751179], AliDDNS [43535751275], infosvr [43535751162], AI handoff docs [43535751090], and AliyunDrive [43535751171] SUCCESS; NVRAM [43535751164] was IN_PROGRESS at inspection. All six had the exact matching `head_sha`; NVRAM must be rechecked. No unrelated or PR run was counted.
- Completed the 13-path inventory for legacy PR #1 (CLOSED / UNMERGED; head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`, 17 ahead / 230 behind). CA trust-store bootstrap code/test are already on main, so those changes are superseded. The OpenSSL 3 EVP_MAC helper and dependent build/test/workflow/checker changes remain unported pending proof of firmware OpenSSL API/link compatibility. No PR branch was used.
- Refreshed `SESSION_HANDOFF.md` in place as `0f3ddb47a02cb9b7cfeadeaaa0546d2018d8a488` to record the exact baseline, run IDs/conclusions and PR triage. This log update is the next direct-main commit: THIS COMMIT. Root log advances 64→65 complete records; no rotation or INDEX update is due.
- Validation boundary: L1 targeted source review and PR patch triage only. L2 GNU89/`-Werror` scanner regressions and local docs checker NOT RUN (no local checkout); current-main exact-head CI for this documentation commit pending inspection. L3 remains limited to historical product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Exact next action: re-fetch live main; inspect all exact-head `push` runs for `0f3ddb47a02cb9b7cfeadeaaa0546d2018d8a488` and finish NVRAM run [43535751164] for its own SHA. Then obtain an exhaustive `trunk/user` source/caller inventory (not zero-result indexed search), classify every `pids`/procps and lifecycle/readiness/stop/signal/cleanup caller, and only then design RUNNING / ABSENT / UNKNOWN with injectable GNU89/`-Werror` regressions. Keep PR EVP_MAC work unported until firmware OpenSSL compatibility is proven.

---

## 2026-11-14 08:39 CST (UTC+8) — Reconcile live-main checkpoint and preserve resumable P1 work

- Base main SHA: `420924fe86a6541b4fa0211876fe6468e26a6a07`. This recovery reconciled the previously stale handoff/roadmap/log checkpoint; the intermediate documentation commits were `8eb2befba136c77492fdd5ef9dedccc9237ce857`, `5e3d8d60ba5d01522801f4a2b84cbc624b267629`, and `420924fe86a6541b4fa0211876fe6468e26a6a07`. Product source was not changed.
- Re-read the complete `AI_DEVELOPMENT_PROTOCOL.md`, bootstrap instructions, active handoff, roadmap priority ledger and newest root log records. Confirmed only remote branch is `main`; the unfinished P1 shared process-inventory tri-state/fail-closed task remains active. Legacy PR #1 compatibility audit remains incomplete.
- Re-fetched scanner/lifecycle source blob identifiers from the handoff; the exhaustive cross-userland caller inventory is still incomplete. No source edit or test was attempted because the protocol forbids a partial API migration before caller classification is complete and no full local checkout is available.
- Exact-head push Actions for the recovery commits are NOT VERIFIED. The available workflow-run connector exposes PR-triggered runs only; combined status is empty and the public Actions page could not be fetched. No CI success is claimed.
- L1: partial existing source review only. L2 GNU89/`-Werror` regressions and local docs checker NOT RUN (no local checkout). L3 remains limited to historical product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Root log advances 63→64/119; 154,433 characters before this entry; no rotation or INDEX update. `SESSION_HANDOFF.md` is refreshed in place as part of this recovery; result: THIS COMMIT.
- Exact next action: inspect push Actions for the exact live main SHA; complete the 13-path legacy PR #1 compatibility audit; then retrieve/classify all process-scanner and lifecycle/readiness/stop callers across `trunk/user` before designing and testing RUNNING / ABSENT / UNKNOWN.

---

## 2026-11-13 16:39 PST (UTC-8) — Recover live main and preserve the active P1 process-inventory task

- Re-fetched live `main`: `79ec0cffe57a327333b4de86bac9eaa35e663e2f` (parent `f0b5b78044f9584952bcfd4b00f07a5ec941b437`). The tip is documentation-only (`dev_log.md`); product source remains unchanged from the historical L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Remote branch inventory contains only `main`. Legacy PR #1 remains CLOSED / UNMERGED per the existing handoff; its 13-path compatibility audit remains incomplete. No PR/branch was used for development.
- Exact-head push Actions for `79ec0cffe57a327333b4de86bac9eaa35e663e2f` are NOT VERIFIED: the available workflow-run connector is PR-only, combined status is empty, and the public Actions page could not be fetched. No CI pass is inferred.
- Resumed recovery confirms the active P1 shared process-inventory fail-open defect is unchanged. Exhaustive caller/lifecycle inventory is still incomplete; no code or tests changed and no partial API migration was attempted.
- L1 remains partial source review; L2 GNU89/`-Werror` tests and local documentation checker NOT RUN (no checkout); L3 applies only to historical source SHA above; L4 NOT VERIFIED. Root log advances 62→63/119; no rotation or INDEX update.
- Exact next action: reconcile exact-main push Actions; finish the legacy PR #1 per-file compatibility audit; obtain exhaustive cross-userland scanner/lifecycle caller inventory before designing RUNNING / ABSENT / UNKNOWN and implementing fail-closed migration.

---

## 2026-11-14 08:34 CST (UTC+8) — Align whole-repository priority dashboard with active P1

- Base main SHA: `f0b5b78044f9584952bcfd4b00f07a5ec941b437`. Updated `REFACTOR_ROADMAP.md` in place: reconciled its stale source/CI checkpoint with live main, distinguished historical L3 source evidence from current-main validation, recorded current push Actions as NOT VERIFIED, updated the root log count/archive inventory, and promoted the shared process-inventory fail-open defect from P2 to P1 to match the active handoff and lifecycle risk. No module implementation status was otherwise changed.
- No production source or tests changed. The cross-userland scanner/lifecycle caller inventory remains INCOMPLETE; no API migration or host test is claimed.
- Exact-head Actions for the roadmap commit are PENDING / not yet inspectable; the available workflow-run wrapper remains PR-only. Local documentation checker NOT RUN because no checkout is available.
- Roadmap commit: `f0b5b78044f9584952bcfd4b00f07a5ec941b437`. Resulting log commit: THIS COMMIT. Root log advances 61→62/119; no rotation or INDEX update.
- Exact next action remains: inspect exact-main push Actions, complete the legacy PR #1 per-file compatibility audit, then obtain exhaustive caller inventory and implement the P1 RUNNING / ABSENT / UNKNOWN contract.

---

## 2026-11-14 08:32 CST (UTC+8) — Recover live main, audit shared process scanner and inventory legacy PR

- Re-fetched live main at `bb3c217e4c513f66c9edbd686a8cb4184b8775c9`; remote branch inventory contains only `main`. Legacy PR #1 is CLOSED / UNMERGED; compared current main with PR head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`: diverged, PR head 17 commits ahead / 230 behind, 13 changed paths. The diff includes CA trust-store bootstrap status propagation and an OpenSSL EVP_MAC HMAC helper/build integration. No PR changes were ported: each remaining hunk and target OpenSSL/build compatibility still require review; EVP_MAC is not assumed available on firmware.
- Re-read the current-main `pids.c`, `shutils.h`, `common_ex.c`, `services_ex.c` and lifecycle policy. Confirmed the existing P1 NULL/EOF ambiguity, partial process inventory on scan failure, and TERM-before-authoritative-inventory behavior. Six indexed source searches returned no results, but they are not exhaustive.
- Tried a shallow clone; it failed because this runtime cannot resolve `github.com`. The available GitHub workflow-run wrapper returns PR-triggered runs only; exact-head push run query for `bb3c217e4c513f66c9edbd686a8cb4184b8775c9` returned none and combined status was empty. Current-main push Actions therefore remain NOT VERIFIED; no CI pass is inferred.
- No product source or tests changed. The caller inventory prerequisite remains INCOMPLETE, so no unsafe partial tri-state API migration was made. L1 partial source review; L2 GNU89/`-Werror` regressions and local docs checker NOT RUN; L3 remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Updated `SESSION_HANDOFF.md` in place to the live baseline and recorded the PR preservation audit and exact next actions. Root log advances 60→61/119; no archive rotation or INDEX update. Whole-repository module priorities did not change, so `REFACTOR_ROADMAP.md` remains untouched.
- Handoff commit: `a4efbef15fcd04eef951c9434db70651aa51d302`. Resulting log commit: THIS COMMIT.
- Exact next action: verify exact-head push Actions for the current main tip; finish the per-file PR #1 compatibility audit and port only valid unmerged changes; then obtain exhaustive cross-userland process-scanner/lifecycle caller inventory before implementing RUNNING / ABSENT / UNKNOWN.
 
---

## 2026-11-14 08:29 CST (UTC+8) — Consolidate exact handoff and log checkpoint

- Base main SHA: `0e111486f9e0c5e355dcb93899cd4066cb2bb2bc`; active branch `main`. Updated `SESSION_HANDOFF.md` to this exact base and appended this record in the same commit. Result: THIS COMMIT.
- No product source or tests changed. Active P1 process-inventory failure remains open; complete cross-userland scanner/lifecycle caller enumeration is still incomplete, so no unsafe partial API migration was attempted.
- Exact-head Actions for this combined documentation commit: PENDING / not yet visible. Earlier base `c3698fa56a5d3580fad38aba588d8d5cd0e1952e` had four SUCCESS, NVRAM IN_PROGRESS at inspection, and no infosvr run. No CI pass is inferred for later commits.
- L1 partial source review only; L2 GNU89/`-Werror` regression and local docs checker NOT RUN; L3 remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED. Root log advances 59→60/119; no rotation or INDEX update. Roadmap/global priority unchanged.
- Exact next action: inspect push Actions for THIS COMMIT by exact `head_sha`; then retrieve source in bounded batches and finish the exhaustive process API/lifecycle caller inventory before designing RUNNING / ABSENT / UNKNOWN.

---

## 2026-11-14 08:27 CST (UTC+8) — Align handoff with the latest documentation checkpoint

- Base main SHA: `3662492bb06b2b9687bf2c80a2c2e45391b78033`; active branch `main`. Updated `SESSION_HANDOFF.md` in place to distinguish the base checkpoint from the current main tip; resulting handoff commit `1b79a029a5e9c55929c00e232d0b141ef0c7f3d4`.
- Exact-head Actions for `3662492bb06b2b9687bf2c80a2c2e45391b78033` were not yet visible at inspection; do not infer a pass. Base SHA `c3698fa56a5d3580fad38aba588d8d5cd0e1952e` had four successful workflows, NVRAM still in progress at inspection, and no infosvr run.
- No product source or tests changed. Active P1 process-inventory failure remains open; exhaustive caller inventory is the prerequisite and remains incomplete. L1 partial; L2 NOT RUN; L3 restricted to recorded source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Root log advances 58→59/119; no rotation or INDEX update. Roadmap status/priority unchanged.
- Exact next action: re-fetch live `main`, reconcile exact-head push runs for commits `8704d286eab3148948917be635cf2fe8cf0fc745`, `3662492bb06b2b9687bf2c80a2c2e45391b78033`, and this log commit; then resume exhaustive caller inventory.

---

## 2026-11-14 08:25 CST (UTC+8) — Refresh exact main checkpoint and preserve P1 process-inventory task

- Base main SHA: `c3698fa56a5d3580fad38aba588d8d5cd0e1952e`; active branch `main`. The preceding base commit changed only `dev_log.md`; no product-source delta from the recorded L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Reconciled exact-head push Actions for the base SHA: AI handoff docs [43533915410], AdGuardHome [43533915342], AliDDNS [43533915511], and AliyunDrive [43533915325] SUCCESS; NVRAM [43533915397] IN_PROGRESS; no infosvr run was present. Every listed run has exact `head_sha` and `event=push`.
- Re-fetched the shared process scanner and lifecycle sources. Confirmed `procps_scan()` treats all NULL `readdir()` results as EOF, while initialization, `/proc` open, and PID-list allocation failures also collapse to NULL/zero in wrappers. `service_lifecycle_stop()` sends TERM before an authoritative process inventory and Boolean `is_running` cannot represent UNKNOWN.
- The complete cross-userland caller inventory remains INCOMPLETE; GitHub indexed search is not exhaustive and no complete local checkout is available. Per the handoff/protocol, no partial tri-state API migration or source change was made. L1 partial source review; L2 host regressions/docs checker NOT RUN; L3 only the previously recorded source SHA; L4 NOT VERIFIED.
- Updated `SESSION_HANDOFF.md` in place with exact base SHA and CI boundary; resulting commit `8704d286eab3148948917be635cf2fe8cf0fc745`. Root log advances 57→58/119; no rotation or INDEX update. Roadmap/global module priorities did not materially change.
- Exact next action: reconcile Actions for `8704d286eab3148948917be635cf2fe8cf0fc745`; then obtain complete source content in bounded batches and classify every process scanner and lifecycle/readiness/stop caller before designing RUNNING / ABSENT / UNKNOWN and injectable GNU89/`-Werror` regressions.

---

## 2026-11-14 08:23 CST (UTC+8) — Reconcile current main and refresh process-inventory handoff

- Re-fetched live `main`: base SHA `f40aba8106e24993cbc64fd9a23dee4756620790`; refreshed `SESSION_HANDOFF.md` in place as `4bb6c27e91e7f2b68a00011b99127f889bfe5d0d`. This is a documentation-only commit; no product source changed.
- Remote refs: only `main`. Legacy PR #1 remains CLOSED / UNMERGED. No non-main ref or PR was used.
- Exact-head push Actions for base `f40aba8106e24993cbc64fd9a23dee4756620790` completed 6/6 SUCCESS with matching `head_sha`: NVRAM [43531298250], AdGuardHome [43531298340], infosvr [43531298263], AliDDNS [43531298235], AI handoff docs [43531298253], AliyunDrive [43531298258].
- Re-inspected `pids.c`, `shutils.h`, `common_ex.c`, `services_ex.c`, and lifecycle policy. Confirmed NULL/EOF ambiguity remains: scanner allocation and `/proc` open failures return NULL; `readdir()` errors are indistinguishable from EOF; PID-list allocation/growth failures can be mistaken for absence. RC/shared source directory inventories were retrieved. Six indexed code searches returned no results, but this is not exhaustive; full cross-userland caller enumeration remains INCOMPLETE. Do not change API or migrate callers until exhaustive classification is achieved.
- No product source or tests changed. L1 partial source review only; L2 GNU89/`-Werror` regressions and local docs checker NOT RUN because no local checkout. L3 remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254` / job `132107102854`); L4 NOT VERIFIED.
- Root log advances 56→57/119; current root size before entry was 142,625 characters, below the 384 KiB rotation trigger. No archive rotation or INDEX update; roadmap inventory/priority did not materially change.
- Exact next action: retrieve and inspect source contents in bounded batches across all `trunk/user` subtrees; enumerate every scanner API and lifecycle caller before designing/migrating RUNNING / ABSENT / UNKNOWN. Then add injectable GNU89/`-Werror` tests and verify exact-head push Actions.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-14 07:59 CST (UTC+8) — Reconcile current main and enumerate complete userland tree

- Re-fetched `refs/heads/main`: `eda03f83812c86e81a6f8d67130ac30f54ce4b18` (parent `6dba16674ed6ee0b0babcf65905a3722fac23876`). Current tip is documentation-only; no product-source delta from the recorded L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Remote branch inventory contains only `main`; legacy PR #1 is CLOSED / UNMERGED. No non-main ref or PR was used for development.
- Six current-main `push` Actions completed SUCCESS with exact matching `head_sha`: AliyunDrive [43530505638], AI docs [43530505416], AdGuardHome [43530505408], AliDDNS [43530505376], infosvr [43530505319], and NVRAM [43530505307]. Combined status endpoint returned no entries; it is not treated as a green check.
- Enumerated the `trunk/user` Git tree hierarchically at this exact commit: 10,203 entries, 764 directories, 4,409 C/H files, and `truncated=false`. Five code-search queries returned no indexed matches, but that is not evidence of no callers. A bulk 229-file RC/shared source retrieval exceeded the connector call budget before completion; exhaustive caller classification is still NOT COMPLETE.
- Partial source review confirms the fail-open concern in `shared/src/pids.c`: scan-context allocation and `opendir("/proc")` failures both return NULL; `procps_scan()` treats any NULL `readdir()` result as normal end-of-scan without distinguishing errno; PID-list wrappers and lifecycle consumers still require complete caller analysis before an API change. No source or test files were changed.
- Validation: L1 partial review/tree enumeration only. L2 process-inventory regressions and local docs checker NOT RUN (no local checkout). L3 evidence remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Root log advances 55→56/119; no size-triggered or count-triggered rotation; archive index unchanged.
- Exact next action: finish source-wide process API and lifecycle consumer inventory from the complete `trunk/user` tree, then design the RUNNING / ABSENT / UNKNOWN API and GNU89/`-Werror` regression. Do not partially migrate callers.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-13 15:53 PST (UTC-8) — Reconcile exact main checkpoint and process-inventory continuation

- Re-read the mandatory bootstrap and complete development protocol, then recovered the live handoff, roadmap, newest root-log records and archive index.
- Re-fetched `refs/heads/main`: `6dba16674ed6ee0b0babcf65905a3722fac23876` (parent `dbe61f648fdcaa34d7fb7dd00b76fe55db35e782`). Remote branch inventory contains only `main`; legacy PR #1 is CLOSED / UNMERGED and was inspected for state only. No non-main branch or PR was used.
- Exact-head push Actions completed 6/6 SUCCESS with matching `head_sha`: AI handoff docs [43529987739], AliDDNS [43529987764], AliyunDrive [43529987660], AdGuardHome [43529987814], infosvr [43529987788], NVRAM [43529987716]. Every job-step summary reports success.
- The active P1 shared process-inventory defect remains unfinished. The required complete caller inventory cannot be established with available GitHub code-search/tree results; the prior full/shallow checkout attempt is documented as blocked by GitHub DNS resolution. Do not change the shared API or migrate Boolean consumers until every caller and lifecycle path is enumerated. No production source, tests or workflow files were changed; no local test or docs checker was run.
- Product-source L3 evidence remains on `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (XY-C1 run 43489646254 / job 132107102854); current main product delta remains documentation-only. L4 router flash/boot remains NOT VERIFIED.
- Root log advances 54→55/119; size remains below the 384 KiB early-rotation threshold. Archive inventory remains dev-log=8 / handoff=22 / roadmap=18; no rotation or INDEX update.
- Exact next action: obtain a complete checkout or exhaustive hierarchical source enumeration; inventory every `pids()`, `pids_main()`, `find_pid_by_name()`, `procps_scan()` and lifecycle/readiness/stop caller, then implement and test RUNNING/ABSENT/UNKNOWN fail-closed behavior. Do not perform a partial migration.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-13 15:45 PST (UTC-8) — Reconcile live main after stale handoff checkpoint

- Recovered the repository in bootstrap order. Live `main` was `935a38ae44ece9243015e3a9c3dcae054e561097`; branch inventory contains only `main`. Legacy PR #1 remains CLOSED / UNMERGED. The handoff had still named `2b37f77d423c4a163e4ecb6789a45c690e9efd13`, so it was refreshed in place to the actual baseline and current Actions state.
- Six exact-head `push` workflows on `935a38ae44ece9243015e3a9c3dcae054e561097` completed SUCCESS: AliyunDrive [43528744171], AdGuardHome [43528744049], AI handoff docs [43528744087], infosvr [43528744070], AliDDNS [43528744042], and NVRAM [43528744152]. All reported matching `head_sha`; individual job-step logs were not inspected in this recovery.
- The active P1 shared process-inventory task remains OPEN / NOT FIXED. The complete caller-inventory prerequisite remains blocked: a fresh shallow clone failed because this runtime cannot resolve `github.com`; prior GitHub code search was non-exhaustive and recursive tree enumeration was truncated. No product source or tests changed; no host tests or local docs validator were run.
- Current-main product-source L3 evidence remains limited to `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; the previously documented later delta is documentation-only. L4 router flash/boot remains NOT VERIFIED.
- Root log advances 53→54/119; no archive rotation or INDEX update is due. Roadmap inventory and priority did not materially change, so `REFACTOR_ROADMAP.md` remains unchanged.
- Exact next action: obtain an exhaustive checkout/content enumeration; classify every scanner and lifecycle caller before tri-state implementation. Do not partially migrate the API.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-14 07:38 CST (UTC+8) — Reconcile live main and exact-head Actions; preserve P1 scanner blocker

- Recovered in the required bootstrap/protocol order. Re-fetched live `main` as `2b37f77d423c4a163e4ecb6789a45c690e9efd13`; branch inventory contains only `main`. Legacy PR #1 remains CLOSED / UNMERGED.
- Compared L3 product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` to live main: 33 commits ahead, zero behind; only `REFACTOR_ROADMAP.md`, `SESSION_HANDOFF.md`, and `dev_log.md` differ. No product-source delta.
- Exact-head `push` Actions all completed SUCCESS with matching `head_sha=2b37f77d423c4a163e4ecb6789a45c690e9efd13`: AI handoff docs [43526969760], AliDDNS [43526970045], AdGuardHome [43526969880], AliyunDrive [43526969918], NVRAM [43526969744]. Their jobs all completed successfully. No infosvr run was present; no result is inferred for it.
- Resumed the active P1 shared process-inventory task. Six code-search queries for `pids(`, `pids_main(`, `find_pid_by_name(`, `procps_scan(`, `service_lifecycle_stop`, and `service_lifecycle_is_running` returned zero indexed matches; this is not exhaustive. A fresh shallow clone again failed because this runtime cannot resolve `github.com`; recursive tree enumeration remains truncated. The complete caller-inventory prerequisite is therefore BLOCKED.
- No product source or tests changed. L1 partial review only; L2 GNU89/`-Werror` host regressions and local documentation validator NOT RUN. L3 remains limited to source SHA `691e4d6` (XY-C1 run [43489646254] / job [132107102854], artifact [14069572898]); L4 NOT VERIFIED.
- Updated the live handoff and this log only. Roadmap/module priority did not materially change; archive INDEX and historical shards remain untouched. Root log advances 52→53/119; no rotation is due. Resulting commit: THIS COMMIT.
- Exact next action: obtain an exhaustive source checkout/content enumeration and classify every process scanner/lifecycle caller before implementing the tri-state API and injectable GNU89/`-Werror` regressions. Do not make a partial API migration.

---

## 2026-11-14 07:24 CST (UTC+8) — Restore all CI-required live handoff sections

- Exact prior main SHA: `e945db3cca96332a0015a55d68382041358a26f8`. The exact-head documentation run `43526801426` and infosvr run `43526801450` on `0a43156b1ef399804e9c93d3d51e86c16a8552e2` both failed because `SESSION_HANDOFF.md` still lacked `## Active task`; the earlier missing `## Current repository state` had been fixed, exposing the next required heading. The infosvr GNU89/ASan/UBSan binaries themselves passed before its source-contract assertion.
- Inspected `scripts/check_ai_handoff_docs.py` and `scripts/check_infosvr_packets.py`: both require `## Current repository state`, `## Active task`, `## Verification boundary`, and `## Exact first action for the next GPT`. Restored the two missing sections in `SESSION_HANDOFF.md`, explicitly recording the P1 process-inventory task, exhaustive-inventory blocker, L1/L2/L3/L4 boundaries, and exact CI failures.
- Handoff correction committed directly to `main` as `0139b786c98f78d3fd656914e1f6f5c8eb65a7aa`. Exact-head `push` Actions for this SHA must be inspected; no pass is claimed yet. Local `scripts/check_ai_handoff_docs.py` was not run because a local checkout is unavailable.
- No product source or regression test changed. The active P1 process-inventory defect remains open: do not migrate the shared API until every scan/lifecycle caller is exhaustively enumerated. Recursive tree API is truncated and `git clone` failed because the runtime cannot resolve github.com. Existing L3 evidence remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Root `dev_log.md` advances 51→52/119. No rotation, INDEX update, or roadmap rewrite is due; global module inventory/priority is unchanged.

---

## 2026-11-14 07:22 CST (UTC+8) — Repair required handoff heading after exact-head CI failure

- Re-fetched live `main` at `435913a1244da38ccb722f89df0eb2a5d2648b10`; branch inventory contains only `main`. Legacy PR #1 remains CLOSED / UNMERGED. Recursive Git tree enumeration returned 64,005 entries with `truncated=true`, so it cannot satisfy the exhaustive caller-inventory prerequisite for the active P1 process-inventory API migration.
- Exact-head `push` runs on `435913a1244da38ccb722f89df0eb2a5d2648b10`: NVRAM `43525514810`, AdGuardHome `43525514772`, AliyunDrive `43525514717`, and AliDDNS `43525514693` SUCCESS; AI handoff docs `43525514688` FAILED because the live handoff omitted required heading `## Current repository state`. The preceding infosvr run `43525494041` on `c430f079d867d95dcb5c18193bc6ba5829948370` compiled and ran all three GNU89/ASan/UBSan regression binaries successfully, then failed its source checker on that same missing heading. No infosvr run exists for the inspected SHA.
- Restored the required heading in `SESSION_HANDOFF.md` directly on `main` as commit `07caeb90a6ab5707cb8c19e978e09eddd1aad857`. This is a documentation-contract correction only; it does not validate or change product source. Exact-head Actions for this commit are pending inspection.
- Active P1 shared process-inventory defect remains OPEN / NOT FIXED. No product source or regression test changed. Complete checkout and exhaustive caller inventory remain unavailable; do not perform an unsafe partial tri-state API migration. Existing L3 evidence remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 router flash/boot NOT VERIFIED.
- Root log advances 50→51/119; no archive rotation or INDEX update is due. The whole-repository module dashboard and task priority did not change, so `REFACTOR_ROADMAP.md` remains untouched. Local documentation validator was not run in this runtime.

---

## 2026-11-14 07:12 CST (UTC+8) — Refresh exact live process-inventory checkpoint

- Recovered the repository using the root bootstrap and full development protocol before considering source changes. Live `main` at start: `a41c997f7252838fbc2d82f58b3be9271e2894a8`; the previous live handoff still named `f79bbc11161109abf8a094f8d0c9fd30bb8c1a0d`. Compare confirmed two commits ahead, zero behind, with only `SESSION_HANDOFF.md` and `dev_log.md` changed. Product source remains at the previously recorded L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Branch inventory returned only `main`. No open PR was returned; the legacy PR #1 remains recorded in the live handoff as CLOSED / UNMERGED. No non-main ref was used.
- Updated `SESSION_HANDOFF.md` in place on `main` as `c430f079d867d95dcb5c18193bc6ba5829948370` to record the actual baseline and pending exact-head CI boundary. This log update is committed directly to `main`; resulting SHA: THIS COMMIT.
- Exact-head GitHub Actions for `a41c997f7252838fbc2d82f58b3be9271e2894a8` and `c430f079d867d95dcb5c18193bc6ba5829948370` remain NOT VERIFIED. The available workflow-run connector returns only PR-triggered runs (none for these SHAs); no push-run enumeration is exposed. Empty workflow/status responses are not a pass. Inspect Actions by exact `head_sha` and `event=push`.
- The unfinished P1 shared process-inventory defect remains the exact active task: explicit RUNNING / ABSENT / UNKNOWN semantics across `pids.c`, `shutils.h`, `common_ex.c`, `services_ex.c` and `service_lifecycle_policy.h`; UNKNOWN must block TERM/KILL, false absence, restart and destructive cleanup. Complete caller inventory is still blocked because this runtime has no full checkout or exhaustive repository-content enumeration.
- No product source or regression tests changed. L1 partial review only; L2 host tests and local `scripts/check_ai_handoff_docs.py` NOT RUN; current exact-head Actions NOT VERIFIED. Existing L3 evidence applies only to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254` / job `132107102854`); L4 router flash/boot NOT VERIFIED.
- Updated only the handoff and this log; the whole-repository module inventory and priority did not materially change, so `REFACTOR_ROADMAP.md` and `docs/dev-log/INDEX.md` were left untouched. Root log advances 49→50/119; the prior recorded size was 125,069 bytes, below the 384 KiB early-rotation threshold. No rotation or archive copy is due.
- Exact next action: inspect push Actions for the latest main SHA, obtain a complete checkout or demonstrably exhaustive content enumeration, then enumerate and classify every process scanner/lifecycle caller before implementing the tri-state API and injectable GNU89/`-Werror` regressions. Do not start another module or make an incomplete API migration.

---

## 2026-11-14 07:08 CST (UTC+8) — Reconcile live main SHA and preserve process-inventory continuation

- Base `main` SHA: `f79bbc11161109abf8a094f8d0c9fd30bb8c1a0d`. The live handoff was materially stale; it is now refreshed in place by `a351306f6d53c461ec1dfd7da372c58a330b6bd9` (handoff only). This log update is a direct fast-forward child on `main`; resulting SHA: THIS COMMIT.
- Re-fetched live repository metadata and branch inventory: default branch `main`, only remote branch `main`. Compare from `64cfa06d16fe966ec0f504373339973968b2fe27` to `f79bbc11161109abf8a094f8d0c9fd30bb8c1a0d` reports two commits ahead, zero behind, and only `SESSION_HANDOFF.md` / `dev_log.md` changes. Product source remains at the recorded L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Legacy PR #1 remains CLOSED / UNMERGED per the current checkpoint and prior live record; no non-main branch or PR was used. No valid unmerged PR work was identified for this round.
- Exact-head Actions for `f79bbc11161109abf8a094f8d0c9fd30bb8c1a0d` and the new documentation commits are NOT VERIFIED. The available workflow-run connector only returns PR-triggered runs and returned none; combined commit status returned no status objects. These empty responses do not establish a pass or failure. The next takeover must inspect Actions for exact `head_sha` and `event=push`, including the AI handoff documentation workflow.
- Active P1 shared process-inventory failure remains OPEN / NOT FIXED in `pids.c`, `shutils.h`, `common_ex.c`, `services_ex.c`, and `service_lifecycle_policy.h`. Scan initialization/OOM, `/proc` open, `readdir` error and PID-list allocation failure can be collapsed into absence/partial inventory; Boolean lifecycle consumers cannot represent UNKNOWN and stop may signal before inventory is authoritative.
- No complete checkout or exhaustive caller enumeration was obtained. GitHub code search and truncated tree results are not complete evidence. No product source or regression test was changed; L1 partial source review, L2 host regressions NOT RUN, local `scripts/check_ai_handoff_docs.py` NOT RUN. Existing L3 applies only to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254` / job `132107102854`); L4 router flash/boot NOT VERIFIED.
- Updated only `SESSION_HANDOFF.md` and this log; whole-repository inventory/priority did not change, so `REFACTOR_ROADMAP.md` and archive index remain untouched. Root log advances 48→49/119; 125,069 bytes before this entry, below the 384 KiB early-rotation trigger and 512 KiB hard limit. No rotation or archive copy is due.
- Exact next action: obtain an exhaustive checkout/content enumeration; classify every process scanner and lifecycle caller, then implement RUNNING / ABSENT / UNKNOWN propagation and injectable GNU89/`-Werror` regressions. Do not start another module or perform an incomplete API migration.

---

## 2026-11-14 07:04 CST (UTC+8) — Reconcile live process-inventory handoff and exact-head CI

- Base `main` SHA: `64cfa06d16fe966ec0f504373339973968b2fe27`; direct parent of the handoff commit `1d7e9683ef2daa44ee0978735900cc5f217f9611`. This log commit is a direct fast-forward child on `main`; resulting SHA: THIS COMMIT.
- Re-fetched live refs: only branch `main`; legacy PR #1 remains CLOSED / UNMERGED. Compared from prior recorded base `b101e16d6ebedce2dd25ce0f74e83c06157bacab`: two intervening commits changed only `SESSION_HANDOFF.md` and `dev_log.md`; no product source delta.
- Exact-head `push` Actions for base SHA `64cfa06d16fe966ec0f504373339973968b2fe27`: five visible runs completed SUCCESS, all exact `head_sha`: NVRAM `43523802575`, AliDDNS `43523802455`, AdGuardHome `43523802122`, AliyunDrive `43523802029`, AI handoff docs `43523802010`. Every reported job step succeeded. No infosvr run was present in the fetched run inventory; this is 5 visible successes, not 6/6.
- Re-fetched the five process-inventory/lifecycle source files; all recorded blob SHAs remain unchanged. The confirmed NULL/empty/partial inventory failure modes and Boolean lifecycle-stop hazard remain open. No complete checkout or exhaustive caller enumeration was obtained; GitHub code search and truncated recursive tree are not proof of completeness.
- No production source or regression test changed. L1 partial source review; L2 host tests NOT RUN; local documentation validator NOT RUN. Existing L3 remains limited to product source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Updated only `SESSION_HANDOFF.md` and this log; global module inventory/priority did not change, so roadmap and archive index remain untouched. Root log advances 47→48/119; no rotation or archive snapshot is due.
- Exact next action: obtain an exhaustive source checkout/enumeration and classify every process scanner and lifecycle caller before implementing RUNNING / ABSENT / UNKNOWN plus GNU89/`-Werror` regressions. Do not make an incomplete API migration.

---

## 2026-11-13 14:59 PST (UTC-8) — Reconcile latest main and preserve process-inventory tri-state blocker

- Re-fetched live `main`: `b101e16d6ebedce2dd25ce0f74e83c06157bacab` before the handoff update; the one-commit range from prior handoff SHA `977c248c79d08e64b57bf8ddaae5457a951dae92` changes only `SESSION_HANDOFF.md` and `dev_log.md`. The handoff refresh is now committed directly to main as `1d7e9683ef2daa44ee0978735900cc5f217f9611`.
- Exact-head push Actions for base SHA `b101e16d6ebedce2dd25ce0f74e83c06157bacab` completed 6/6 SUCCESS with matching `head_sha`: AdGuardHome `43523287578`, AliDDNS `43523287421`, infosvr `43523287345`, AliyunDrive `43523287340`, AI handoff docs `43523287331`, NVRAM `43523287310`. This does not validate the new handoff commit; its exact-head Actions must be checked separately.
- Re-read the bootstrap/protocol, live handoff, roadmap, newest root log records and archive index. Re-fetched the five process-scanner/lifecycle source files and confirmed the previously recorded blob SHAs remain unchanged. Six process API/lifecycle code-search queries returned zero indexed results, which is not an exhaustive caller inventory.
- No complete local checkout or demonstrably exhaustive source enumeration was obtained. No product source or regression tests were changed; L1 partial source review only, L2 host tests NOT RUN, local documentation validator NOT RUN. Existing L3 evidence remains restricted to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Updated only the live handoff and this log; roadmap/global priority unchanged. Root log advances 46→47/119; archive index and historical archives unchanged. No rotation or archive snapshot is due.
- Exact next action: obtain a complete checkout or exhaustive content enumeration, classify every process scanner and lifecycle consumer, then implement/test tri-state inventory and fail-closed signal policy. Do not perform an incomplete API migration.

---

## 2026-11-13 14:55 PST (UTC-8) — Reconcile live main and resolve exact-head process-inventory checkpoint

- Re-fetched `main`: `977c248c79d08e64b57bf8ddaae5457a951dae92` (parent `a15a9cb16bbafde1c13638d963d6113014511173`). Compare confirms the current commit is documentation-only (`SESSION_HANDOFF.md`, `dev_log.md`); product sources remain on L3-verified SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Confirmed only remote branch `main`; PR #1 remains CLOSED / UNMERGED. Exact-head push Actions for `977c248` completed 6/6 SUCCESS with matching `head_sha`, and all job steps succeeded. The previously pending NVRAM run `43522368539` for `6a59436` also resolved SUCCESS.
- Re-read active handoff, roadmap, newest root log records and archive index; re-fetched all five process-inventory/lifecycle source files and confirmed their blob SHAs remain unchanged. The tri-state inventory defect and incomplete caller inventory remain open.
- No production source/test edit was made: no complete checkout or exhaustive caller enumeration is available. L1 partial; L2 host tests NOT RUN; local documentation validator NOT RUN. Existing L3 remains limited to source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- Updated only the live handoff and this log. Roadmap and archive index unchanged; no rotation or archive copy is due. Resulting commit: THIS COMMIT. Exact-head Actions for this documentation commit must be checked on the next recovery.

---

## 2026-11-14 06:50 CST (UTC+8) — Reconcile process-inventory takeover checkpoint

- Base `main` SHA: `a15a9cb16bbafde1c13638d963d6113014511173` (the preceding checkpoint-refresh commit; product sources unchanged). This documentation reconciliation commit: THIS COMMIT.
- Re-fetched live refs and confirmed only `main` exists as a branch; legacy PR #1 remains CLOSED / UNMERGED and `toolchain` tag remains unchanged.
- Exact-head Actions for base SHA `6a59436e0dd8c6a0e75688d77bcab6959cba8db9`: five of six push workflows SUCCESS; NVRAM transaction boundary run `43522368539` remained IN_PROGRESS at inspection. The preceding handoff refresh `a15a9cb` had no Actions run visible at immediate inspection; results must be reconciled by exact `head_sha`, not inferred.
- Refreshed `SESSION_HANDOFF.md` to record the current live SHA, exact workflow states, known source blob SHAs, and the first action for the next GPT. No production source or tests changed.
- Shared process inventory remains P1 OPEN / NOT FIXED: scan-context allocation or `/proc` open failure, `readdir()` error and PID-list growth failure can be misreported as absence/partial inventory; lifecycle stop currently signals before authoritative inventory and has no UNKNOWN state. Complete caller inventory is still blocked by unavailable complete checkout and non-exhaustive GitHub search/tree retrieval.
- Validation: L1 partial source review only; L2 host regressions NOT RUN; local `scripts/check_ai_handoff_docs.py` NOT RUN because no local checkout is available. Exact-head Actions for THIS COMMIT: PENDING; reconcile after push. Existing L3 XY-C1 evidence remains limited to product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254` / job `132107102854`); L4 NOT VERIFIED.
- Next: obtain a complete checkout or exhaustive content enumeration; classify all process scanner and lifecycle callers before designing the RUNNING / ABSENT / UNKNOWN API and GNU89/`-Werror` regressions. Do not implement an incomplete caller migration.

---

## 2026-11-14 06:42 CST (UTC+8) — Correct live handoff checkpoint and record pending exact-head CI

- Base main SHA: 2804237172be23569cfcef68b049d78be44af0d6 (parent ebe8fc50584a51e0ce625f686f2d6e4bcd34529e). The preceding 06:39 docs commit updated roadmap/log but the intended handoff blob was byte-identical; this commit now explicitly advances the live handoff to the current SHA and pending CI state.
- Five exact-head push workflows for the base SHA were IN_PROGRESS at inspection: AI docs 43521188485, AliDDNS 43521188466, AliyunDrive 43521188537, AdGuardHome 43521188664, NVRAM 43521188486. No infosvr run was present. Final results are pending; no pass is claimed.
- No source or test changes. Process-inventory caller inventory remains incomplete; local clone again failed at github.com DNS resolution. L1 partial, L2 NOT RUN. Product-source L3 remains on 691e4d68a27580d0f053a8b4c9ae7a71bff03e6f; L4 NOT VERIFIED.
- Updated SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md to preserve the exact active task, main baseline and CI boundary. Root log advances 43→44/119; archive inventory unchanged at dev-log=8 / handoff=22 / roadmap=18.
- Resulting commit: THIS COMMIT. Inspect exact-head push Actions before claiming documentation validation. No local docs script, host regression, firmware build or hardware test is claimed.

---
## 2026-11-14 06:39 CST (UTC+8) — Reconcile exact main SHA and preserve process-inventory continuation

- Base main SHA: ebe8fc50584a51e0ce625f686f2d6e4bcd34529e (parent e2a227897feb3a558bde056884595bfd0a33ae3a). Branch inventory contains only main; PR #1 remains CLOSED / UNMERGED. Compare confirms the base commit changed only SESSION_HANDOFF.md and dev_log.md; product source remains the L3-verified 691e4d68a27580d0f053a8b4c9ae7a71bff03e6f tree.
- Exact-head push Actions for the base SHA completed 6/6 SUCCESS: AI docs 43518951163, infosvr 43518951037, AdGuardHome 43518951039, AliDDNS 43518951170, AliyunDrive 43518951061, NVRAM 43518951092. All runs have event=push and exact head_sha; every reported job step succeeded.
- Re-read pids.c, shutils.h, common_ex.c, services_ex.c and service_lifecycle_policy.h; source blob SHAs remain unchanged. Confirmed NULL/error collapse for scanner initialization, /proc open, readdir errors and PID-list allocation; lifecycle policy sends TERM before inventory can be trusted.
- Attempted a fresh shallow filtered clone; it failed before checkout because this runtime cannot resolve github.com. Code search remains non-exhaustive; no complete caller inventory or executable host regression is available. No product source or tests changed; L1 partial, L2 NOT RUN.
- Updated the live handoff and roadmap checkpoint to the exact current base SHA and exact Actions evidence, and appended this log entry. Root log advances 42→43/119; archive inventory remains dev-log=8 / handoff=22 / roadmap=18. No rotation or INDEX update.
- Existing product-source L3 XY-C1 MIPSel/TRX evidence remains on 691e4d68a27580d0f053a8b4c9ae7a71bff03e6f (run 43489646254 / job 132107102854); L4 router flash/boot remains NOT VERIFIED.
- Resulting commit: THIS COMMIT. Exact-head push Actions and documentation validator must be checked on the resulting commit SHA. No local docs script, host test, firmware build or hardware test is claimed.

---
## 2026-11-14 06:22 CST (UTC+8) — Reconcile current main and exact-head Actions; retain process-inventory blocker

- Base main SHA: `e2a227897feb3a558bde056884595bfd0a33ae3a` (parent `068e9896b5d0cff52a560bd4abe75287563f9155`). The branch inventory contains only `main`; legacy PR #1 remains CLOSED / UNMERGED. Compare confirms this recovery is docs-only; product source remains the L3-verified tree `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Exact-head `push` Actions for the base SHA completed 6/6 SUCCESS with exact `head_sha`: AI handoff docs [43515146895], infosvr [43515147060], AdGuardHome [43515147012], AliDDNS [43515146982], AliyunDrive [43515146872], and NVRAM [43515147016]. The combined commit-status endpoint returned no status objects.
- Re-read process scanner, lifecycle policy and known DNSMasq/service consumers. Confirmed NULL/error collapse for scan-context OOM, `/proc` open, `readdir` errors and PID-list allocation; stop policy signals before authoritative inventory and lacks UNKNOWN semantics.
- Whole-tree caller inventory remains incomplete. GitHub code search returned zero results for six exact API/caller queries, and a shallow clone attempt failed at DNS resolution before checkout. No production or regression changes were made; L1 partial, L2 NOT RUN.
- Updated only `SESSION_HANDOFF.md` and this log. Root log advances 41→42/119; archive inventory remains dev-log=8 / handoff=22 / roadmap=18. No rotation, INDEX, roadmap or snapshot archive change.
- Existing product-source L3 XY-C1 MIPSel/TRX evidence remains on `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254` / job `132107102854`); L4 router flash/boot remains NOT VERIFIED.
- Resulting commit: THIS COMMIT. Exact-head Actions for this documentation checkpoint must be verified on the next recovery; no local test, firmware build or hardware test is claimed.

---

## 2026-11-14 05:55 CST (UTC+8) — Reconcile live main and exact-head Actions; preserve process-inventory blocker

- Base main SHA: `068e9896b5d0cff52a560bd4abe75287563f9155` (parent `unknown`). Current branch inventory contains only `main`; legacy PR #1 remains CLOSED / UNMERGED. The current source tree remains unchanged from the L3-verified product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Exact-head push Actions for the base SHA completed 5/5 SUCCESS with exact `head_sha`: AdGuardHome [43513045226], AliDDNS [43513045288], AliyunDrive [43513044978], AI handoff docs [43513045093], and NVRAM [43513045180]. No infosvr run was returned for this docs-only change set.
- Reconfirmed the shared process-inventory fail-open path in `pids.c`: initial scan-context allocation / `opendir("/proc")` failure is indistinguishable from an empty list; `readdir()` NULL is accepted as EOF without checking `errno`; PID-list allocation failures also collapse to absence. Existing lifecycle consumers are Boolean and cannot safely represent UNKNOWN.
- GitHub code search returned no results for the relevant API and lifecycle symbols; recursive repository tree output was truncated before most userland paths. No complete checkout or equivalent whole-tree text search is available. Caller inventory remains incomplete, so no product source or regression was changed; L1 partial, L2 NOT RUN. This deliberately avoids an unsafe API change.
- Updated only the live handoff and this log. Root log advances 40→41/119; archive inventory remains dev-log=8 / handoff=22 / roadmap=18. No rotation, INDEX, roadmap, or snapshot archive change.
- Existing product-source L3 XY-C1 MIPSel/TRX evidence remains on `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254` / job `132107102854`); L4 router flash/boot remains NOT VERIFIED.
- Resulting commit: THIS COMMIT. Exact-head Actions for this documentation checkpoint must be checked during the next recovery; no local tests, firmware build, or hardware test are claimed.

---

## 2026-11-14 05:41 CST (UTC+8) — Reconcile exact-main Actions and preserve process-inventory blocker

- Base main SHA: `118c9655c7d4787f94568c54fc6a8d7838c4ae93`; live main before this checkpoint was `2be039ecc8bfb35203b3a41fda0f47cd4ee95b57`. The intervening commit changed only `SESSION_HANDOFF.md`; product source remains the L3-verified `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` tree.
- Exact-head push Actions for `2be039ecc8bfb35203b3a41fda0f47cd4ee95b57` were enumerated and each job/step inspected: infosvr `43511645485` / job `132183116113`, AliDDNS `43511645450` / `132183115493`, AdGuardHome `43511645423` / `132183115311`, NVRAM `43511645411` / `132183115314`, AI handoff docs `43511645365` / `132183116262`, AliyunDrive `43511645354` / `132183116280`; all six are event=push, exact head SHA, COMPLETED / SUCCESS with every step successful. Parent `35d891b064c23b8ddd2f380208ad29d5ad1a0fc6`'s six queued-at-checkpoint runs also resolved SUCCESS.
- Repository code search returned zero matches for six process API / lifecycle queries, but the index is not a reliable complete inventory. The previous filtered clone was blocked by GitHub DNS. No product source or test files were changed: caller inventory remains incomplete, L1 partial, L2 NOT RUN. Do not implement a shared API migration until every caller and lifecycle consumer is enumerated.
- Updated only the live handoff and this log to record current SHA, exact-head Actions and the blocker. Root log advances 39→40/119; archive inventory remains dev-log=8 / handoff=22 / roadmap=18. No rotation or INDEX update.
- Resulting main commit: THIS COMMIT. Exact-head AI docs and relevant push Actions for this documentation commit must be checked during the next recovery; no local test, firmware build or router validation is claimed.
- Next: obtain a complete checkout or reliable whole-tree search, enumerate every `pids()`, `pids_main()`, `find_pid_by_name()`, `procps_scan()` and service-lifecycle caller, then design tri-state inventory and fail-closed GNU89 regressions.

---

## 2026-11-14 05:30 CST (UTC+8) — Preserve queued exact-head CI and update the live handoff

- Baseline main `35d891b064c23b8ddd2f380208ad29d5ad1a0fc6` (parent `9eda1297321595f6e17909e0079ec642a1e174e8`) was re-fetched before the documentation checkpoint. The baseline was documentation-only relative to the L3-verified product-source tree `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Exact-head push Actions for baseline `35d891b064c23b8ddd2f380208ad29d5ad1a0fc6` were present for six workflows, all matching `event=push` and exact `head_sha`: AdGuardHome `43511558816`, AI docs `43511558811`, AliyunDrive `43511558737`, NVRAM `43511558690`, AliDDNS `43511558686`, infosvr `43511558685`. They were QUEUED at inspection; no success/failure is inferred. Next recovery must inspect their final results.
- Updated only `SESSION_HANDOFF.md` and `dev_log.md` so the live checkpoint carries the queued CI state, exact baseline SHA and first action. No roadmap change: global module priorities did not change.
- The shared process-inventory fail-open defect remains open. Scanner OOM/open/read errors and PID-list allocation failures are collapsed into absent/partial inventory; lifecycle policy can send TERM before checking and escalate without UNKNOWN. Full caller inventory is blocked because GitHub code search is incomplete and container DNS prevented checkout. No product source/tests changed; L1 partial, L2 NOT RUN.
- Root log advances 38→39/119; archive inventory unchanged (dev-log=8 / handoff=22 / roadmap=18). No rotation/INDEX update. This record's resulting commit SHA: THIS COMMIT.
- Next: verify this documentation commit's exact-head push workflows, then obtain reliable whole-tree search and complete process API caller inventory before implementing the tri-state fix.

---

## 2026-11-14 05:30 CST (UTC+8) — Reconcile exact main SHA and verify all current push workflows

- Recovered from the live repository after reading the mandatory bootstrap/protocol, live handoff, roadmap and newest 20 complete root dev-log records. Live main is `9eda1297321595f6e17909e0079ec642a1e174e8`, direct parent `300ea7175af6c906b3bac97517f3d270f4af55e2`.
- Compared `8672afe19c5a9f867f8cf89f8ff626329bcc5341...9eda1297321595f6e17909e0079ec642a1e174e8`: only `SESSION_HANDOFF.md` and `dev_log.md) changed. Compared product source `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f...9eda1297321595f6e17909e0079ec642a1e174e8`: six commits changed only the roadmap, handoff and log; no product source delta.
- Remote refs: only branch `main`; PR #1 CLOSED / UNMERGED with legacy head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`; prior handoff records its valid CA/HMAC work as ported. `toolchain` tag remains `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`.
- Enumerated all current exact-head Actions: 5/5 push runs completed SUCCESS, all `head_sha=9eda1297321595f6e17909e0079ec642a1e174e8`: AI docs `43511038469`, NVRAM `43511038548`, AdGuardHome `43511038550`, AliDDNS `43511038565`, AliyunDrive `43511038703`. Every job had no failed/pending steps; NVRAM inventory completed.
- Re-read `pids.c`, `shutils.h`, `common_ex.c`, `services_ex.c` and `service_lifecycle_policy.h`. Confirmed OOM / `opendir("/proc")` failure is collapsed to an empty list and `pids()` / `pids_main()` report 0; PID-list allocation/growth failure is also mapped to 0. `readdir()==NULL` is treated as EOF without checking `errno`, permitting a partial inventory on read error. Lifecycle policy currently sends TERM before inventory checks and cannot represent UNKNOWN; UNKNOWN must abort before signals, escalation, stop-success, restart or cleanup.
- Attempted a shallow filtered checkout; it failed because the environment could not resolve `github.com`. GitHub code-search API returned zero matches with `incomplete_results=true`. Full caller inventory is therefore still NOT COMPLETE; no source/test edits were made. L1 partial review; L2 NOT RUN.
- Product-source L3 remains the exact XY-C1 MIPSel build/TRX/artifact on `691e4d6` (run `43489646254` / job `132107102854`); later six commits are documentation-only. L4 NOT VERIFIED.
- Changed paths: `SESSION_HANDOFF.md` and `dev_log.md`. Root log 37→38/119; archive inventory unchanged; no rotation/INDEX/roadmap update. This record's resulting SHA: THIS COMMIT.
- Next: obtain a complete checkout/reliable whole-tree search, enumerate every process inventory and lifecycle-policy caller, then implement and test the checked tri-state API and fail-closed stop policy before migrating DNSMasq and other daemon consumers.

---

## 2026-11-14 05:30 CST (UTC+8) — Reconcile latest main and preserve process-inventory recovery boundary

- Re-fetched live main as `8672afe19c5a9f867f8cf89f8ff626329bcc5341`; its only parent-range changes from `947d9ce8aa51a8f000bbe3186e03774c226880c2` were documentation files. Refreshed the live handoff in commit `300ea7175af6c906b3bac97517f3d270f4af55e2`.
- Remote branches contain only `main`; PR #1 remains CLOSED / UNMERGED. No non-main ref or PR was used.
- Reconfirmed the shared process scanner failure path and the Boolean lifecycle callback / TERM-KILL escalation hazard. Also identified that `readdir()` EOF is not distinguished from a directory-read error in the current scanner. Whole-tree symbol search is unavailable because GitHub code search returned no indexed results; this must not be interpreted as an exhaustive caller inventory.
- No product source or tests changed. No local checkout was available; local documentation checker and host regressions NOT RUN. L1 partial source review only; L2 NOT RUN. Existing product-source L3 evidence remains the XY-C1 run `43489646254` on `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 NOT VERIFIED.
- For `8672afe19c5a9f867f8cf89f8ff626329bcc5341`, combined status had no entries and the available commit-run wrapper returned no runs (PR-triggered only). Exact-head push Actions are NOT VERIFIED; older-SHA green runs are not substituted.
- Root log was 36/119 before this entry; this makes 37/119, below rotation thresholds. Archive inventory and roadmap priorities did not change; no archive, INDEX, or roadmap update is due.
- Changed paths: `SESSION_HANDOFF.md`, `dev_log.md`. Handoff commit: `300ea7175af6c906b3bac97517f3d270f4af55e2`; resulting log commit: THIS COMMIT.
- Exact first action: obtain a full checkout / reliable whole-tree search, map every process-inventory caller, then implement and regression-test the checked tri-state API and fail-closed lifecycle policy.

---

## 2026-11-14 05:24 CST (UTC+8) — Reconcile live main and refine process-inventory fail-open continuation

- Re-fetched `refs/heads/main`: baseline exact SHA `947d9ce8aa51a8f000bbe3186e03774c226880c2`, parent `85b06794590a8d3fc7fe4218b2f967493d45d9e3`. The one-commit range is documentation-only; no product-source delta.
- Remote branch inventory contains only `main`; PR #1 remains CLOSED / UNMERGED; `toolchain` tag remains `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`.
- Exact-main push Actions for `947d9ce8aa51a8f000bbe3186e03774c226880c2` completed 6/6 SUCCESS with exact `head_sha`: AI docs [43501595814], infosvr [43501595647], AdGuardHome [43501595737], AliyunDrive [43501595819], AliDDNS [43501595686], NVRAM [43501595559].
- Re-read current `pids.c`, `shutils.h`, `common_ex.c`, `service_lifecycle_policy.h`, `services_ex.c`, DNSMasq policy/header and build manifests. Confirmed the scan-context/OOM/`/proc` open failure collapses to empty PID list / Boolean false. `common_ex.c` routes this through the generic service-stop callback; DNSMasq readiness/start/restart and Samba/miniupnpd-related checks also consume Boolean `pids()`.
- Refined the design constraint: generic `service_lifecycle_policy.h` currently accepts only a Boolean is-running callback and may escalate TERM to KILL. UNKNOWN must therefore abort before any signal escalation; simply changing `pids()` to return -1 is unsafe because legacy Boolean callers may interpret it as true/running or otherwise inconsistently.
- No production code/tests changed; no host test or local GNU89/Werror compile was run. GitHub code search produced no indexed matches, so a complete repository-wide caller inventory is still outstanding. This is a recovery/source-review milestone only.
- Product-source L3 evidence remains XY-C1 run [43489646254 / job 132107102854] on `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; later main commits are documentation-only. L4 remains NOT VERIFIED.
- Root log advances 35→36/119. Archive inventory unchanged (dev-log=8 / handoff=22 / roadmap=18); no rotation or INDEX update.

- Next: obtain a reliable full-repository checkout/search, inventory every process API caller, then design and test a checked RUNNING/ABSENT/UNKNOWN API plus fail-closed lifecycle policy before touching DNSMasq callers.
- Changed paths: `SESSION_HANDOFF.md`, `dev_log.md`. Resulting commit: THIS COMMIT.

---

## 2026-11-14 04:24 CST (UTC+8) — Reconcile live main checkpoint and preserve process-inventory continuation

- Re-fetched `refs/heads/main`: exact SHA `85b06794590a8d3fc7fe4218b2f967493d45d9e3`, parent `18561fddd214238dda124e0b8883030081ffca5d`. Compared the one-commit range; `85b0679` changes only SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. Product source remains the `691e4d6` tree; no source changes were made in this recovery.
- Remote branch inventory contains only `main`. Legacy PR #1 remains CLOSED / UNMERGED with head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`; it was inspected for state only and not used as a development target.
- Exact-head Actions for `85b06794590a8d3fc7fe4218b2f967493d45d9e3`, all `event=push`, completed 6/6 SUCCESS: AI docs [43492884427 / job 132118367981], AliyunDrive [43492884404], AliDDNS [43492884384], AdGuardHome [43492884381], infosvr [43492884353], NVRAM [43492884341]. AI docs validator passed the pre-entry 34/119 root log and archive inventory dev-log=8 / handoff=22 / roadmap=18.
- Continued source-level inspection of `trunk/user/shared/src/pids.c`: `alloc_procps_scan()` can fail at scan-context allocation or `opendir("/proc")`; the first `procps_scan(NULL, ...)` then returns NULL, but `find_pid_by_name()` currently returns its preallocated empty list and `pids()` reports 0. `trunk/user/rc/src/common_ex.c::service_lifecycle_is_running()` forwards this Boolean directly into `service_lifecycle_stop()`. The generic stop policy currently treats every nonzero callback result as “running” and can proceed to SIGKILL; it has no explicit unknown/error state. The active defect therefore spans both the shared scanner and its lifecycle-policy consumer.
- No code was changed or test run in this recovery. Do not patch `pids()` in isolation: first inventory every callsite and the full DNSMasq readiness/stop graph, then introduce a checked running/absent/unknown contract and make unknown abort stop without claiming absence or escalating signals. Add GNU89/Werror regression for scan OOM, `/proc` open/read errors, normal empty inventory, matched processes, and unknown during TERM/KILL waits.
- Product-source L3 remains the XY-C1 MIPSel build on `691e4d6` [run 43489646254 / job 132107102854], including TRX verification and artifact upload; current `85b0679` is docs-only relative to that product source. L4 physical router flash/boot remains NOT VERIFIED. FTP cleanup TOCTOU and NFS client/storage semantics remain OPEN/BLOCKED.
- Root dev_log advances 34→35/119. Archive inventory unchanged; no rotation, INDEX change, or handoff/roadmap snapshot is due.

---

## 2026-11-14 03:16 CST (UTC+8) — Confirm exact-source XY-C1 L3 and identify process-inventory fail-open

- After the previous documentation checkpoint, exact-main `18561fddd214238dda124e0b8883030081ffca5d` Actions completed 6/6 SUCCESS: AliDDNS [43490972337], AdGuardHome [43490972373], infosvr [43490972566], AliyunDrive [43490971776], NVRAM [43490971657] and AI docs [43490971630]. Every job completed with no failed or pending steps; the docs validator confirmed the previous state at 33/119 entries and archive counts dev-log=8 / handoff=22 / roadmap=18.
- Rechecked XY-C1 on exact product source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` [run 43489646254 / job 132107102854]. Build XY-C1, TRX verification and artifact upload all passed. The image is `TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261114.trx`, 12,625,949 bytes. Artifact `14069572898` is 12,579,578 bytes with SHA256 `c6581125da108ac1118dfcaf895bcfb2414e09554eb63b96be8bddb369c7b768`. The only subsequent main commit `18561f` is documentation-only, so current product source has L3 evidence. L4 physical router flash/boot remains NOT VERIFIED.
- Continued DNSMasq lifecycle trace into shared process inventory. `alloc_procps_scan()` returns NULL when its process-scan context allocation or `opendir("/proc")` fails. `procps_scan(NULL, ...)` then returns NULL; `find_pid_by_name()` does not distinguish that initial scan failure from a completed scan with no matches and returns its already-allocated empty PID list; `pids()` converts it to 0. This is a confirmed fail-open inventory contract.
- The consequence reaches service control: `common_ex.c::service_lifecycle_is_running()` directly returns `pids(service_name)`; DNSMasq `stop_dns_dhcpd_locked()` ultimately relies on `kill_services()` / this callback, and DNSMasq launch/readiness checks also use `pids("dnsmasq")`. Under OOM or procfs-open failure, a live daemon may be misreported absent, allowing stop to appear successful, static ARP cleanup, or a duplicate start. This defect is newly OPEN and not yet patched.
- Do not simply make `pids()` return -1 without auditing every caller: existing APIs assume Boolean semantics. Next work must introduce a checked tri-state inventory contract (running / absent / unknown), propagate scan/OOM errors, and add injectable host tests for failed scan initialization, empty inventory and matched process; then migrate daemon lifecycle callers fail-closed.
- Existing full LAN restart error propagation and HTTPD source-contract alignment remain verified on `ed010248` / `691e4d6`; no duplicate changes were made. Current main is `18561fddd214238dda124e0b8883030081ffca5d`, a docs-only child of the L3-verified product tree. Root dev_log advances 33→34/119; no archive rotation or INDEX update. FTP cleanup TOCTOU and NFS client/storage evidence remain OPEN/BLOCKED.

---

## 2026-11-14 03:15 CST (UTC+8) — Verify full-LAN DNSMasq failure propagation and align exact source contracts

- Recovery followed the repository bootstrap and development protocol before reading the live handoff, roadmap and root dev_log. Starting checkpoint from the prior handoff was `479f0d7a298f6980046a85c60a62271fc830893c`; while the audit was running, remote main advanced through two commits already created by the concurrent repository workflow: `ed010248a2d1bb073d7c6aa28c3bd9afcda3d07f` (production failure propagation) and `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (HTTPD source-contract alignment). This session did not duplicate those code/test commits.
- Inspected the `ed010248` diff: `full_restart_lan()` now returns the DNSMasq lifecycle result; lock acquisition, stop, start and release failures are preserved, and PID 1's `RCN_RESTART_LAN` branch merges the result with `rc_notify_merge_action_result()`. This closes a real notification reliability gap: a failed DNSMasq transition during full LAN restart can no longer be silently acknowledged as success.
- DNSMasq lifecycle workflow on the production source commit `ed010248` [run 43489264442 / job 132105772776] passed all steps: GNU89/Werror restart policy, DHCP range conflict policy, lifecycle lock, private atomic Inadyn configuration, daemon postconditions and full LAN / RC notification result contracts.
- NVRAM whole-tree audit on `ed010248` [run 43489264608 / job 132105772928] passed. The complete log reports `RAW_USERLAND_BYPASS_COUNT: 0`, NVRAM single-value ioctl snapshot contract PASS, device-node ownership/mode checks PASS, and caller inventory PASS.
- Investigated the sole failed `ed010248` workflow, HTTPD Authentication and Security [run 43489264377 / job 132105772925]. The first 16 security steps passed; failure was in the umbrella's DNSMasq source assertion, which still expected `dnsmasq_stop_status` after the function had correctly switched to `dnsmasq_status`. This was stale checker drift, not a production C failure. Commit `691e4d6` updates the HTTPD source contract and includes `rc.c` in both push and pull_request path filters. Exact `691e4d6` HTTPD regression [43489646217 / job 132107102491] completed SUCCESS with every step, including all DNSMasq, PPP credential, DHCP, storage, Samba and NVRAM checks.
- Exact `691e4d6` AI docs run [43489646340 / job 132107103804] passed the pre-update document state. Exact-main XY-C1 run [43489646254 / job 132107102854] was still in Build XY-C1 at the last poll; TRX verification and artifact upload remained pending. Do not claim current-main L3 yet. L4 router testing was not performed.
- Reviewed DNSMasq lock layering: public lifecycle wrappers acquire the common lock and call locked helpers; `full_restart_lan()` holds the lock and calls locked stop/start helpers without reacquiring it. `restart_dhcpd_locked()` prepares configuration before disrupting the running daemon, checks stop status and process disappearance before replacement launch, and confirms the newly launched daemon. The complete all-entrypoint caller graph, process identity and kill/wait/reap semantics remain unfinished; no additional confirmed production defect was claimed from this partial trace.
- The exact-main checkpoint is `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`. Root dev_log advances 32→33/119; the archive inventory remains dev-log=8, handoff=22, roadmap=18. No archive rotation, handoff/roadmap snapshot or INDEX churn is due. FTP cleanup TOCTOU remains OPEN/BLOCKED; NFS client/storage semantics still require real evidence.

---

## 2026-11-14 02:56 CST (UTC+8) — DNS Forwarder lifecycle serialization and failure reporting L2 verified

- Recovery started from live main 47c544fe98511bfaa5f5bb66d047272748e1b021 after checking bootstrap/development protocol and the current handoff, roadmap and root dev_log. The active handoff task was to continue DNS Forwarder/DNSMasq process supervision; the main branch was advanced only by ordinary fast-forward commits.
- Traced the DNS Forwarder service path. Before this patch, RC exposed separate void stop/start calls and performed restart as two independent calls, so another caller could interleave. The shell launcher lacked an interprocess lifecycle lock, start idempotence, verified stop completion and a failed-stop restart gate. The PID 1 notification branch discarded restart result.
- Commit 5be7a356957d47a1387346854966f3f19c8fde01 adds a persistent mode-0600 flock lifecycle lock with directory/file owner/mode, symlink and hardlink checks; reads enable/bind/port/server NVRAM inside the lock; uses the absolute installed binary path; makes start idempotent and checks daemon presence; performs bounded stop polling and refuses restart if the old daemon remains; closes lock FD 9 for the daemon child. RC service APIs return int and restart uses one script transaction; PID 1 merges restart status into notification batch accounting.
- Added a shell regression and extended semantic source / BusyBox configuration / workflow path contracts. The regression covers start/idempotent start, stop/idempotent stop, failed signal, daemon surviving stop timeout, successful restart, disabled restart, launcher status failures, successful launcher with no visible process, lock contention, FD non-inheritance and symlink/hardlink/unsafe-directory rejection.
- The first exact 5be7a35 DNS Forwarder workflow [run 43485626084 / job 132093172981] exposed a fixture defect: its fake pidof considered only non-empty files to represent a process, but the start-stop-daemon stub and seeded state wrote zero-byte files. The Python source contract passed, while the shell regression failed at initial start. The failure was investigated from the full log, not waived.
- Follow-up commit 479f0d7a298f6980046a85c60a62271fc830893c writes a non-empty fake PID marker on all fixture start/seed paths and statically asserts that contract. It also fixes production return propagation: failed start-stop-daemon calls now return their actual nonzero status instead of normalizing every failure to 1. The exact 479f0d7 DNS Forwarder rerun [run 43487234595 / job 132098791466] completed SUCCESS; full log contains CWD/source contract PASS and lifecycle locking/stop-gate/postcondition PASS.
- Exact 479f0d7 AI handoff docs [run 43487234617 / job 132098791118] completed SUCCESS; validator before this entry saw root dev_log at 31/119 and archives dev-log=8, handoff=22, roadmap=18. Current 479f0d7 NVRAM run [43487234555 / job 132098790943] was still in progress at checkpoint: its first eight compilation/policy/lock/device-node tests passed, and the final repository-wide kernel/userland raw-API audit remained active. Do not infer its result until polled.
- XY-C1 MIPSel build on exact implementation SHA 5be7a356957d47a1387346854966f3f19c8fde01 [run 43485625995 / job 132093173668] completed SUCCESS: Build XY-C1, TRX verification and upload all passed. Artifact 14067278386, 12,580,062 bytes, SHA256 0e0cf60b7be3b653d64b23a2554a5efcd3dab9255676673dcb71aa10d90c20cc. This precedes the production shell status-propagation edit in 479f0d7, so it is not exact-current-main L3 evidence. L4 was not performed.
- Current source checkpoints: implementation SHA 5be7a356957d47a1387346854966f3f19c8fde01, follow-up current main 479f0d7a298f6980046a85c60a62271fc830893c. Current main's L2 DNS Forwarder regression and docs validator passed; NVRAM audit was pending. Root dev_log advances 31→32/119; approximately 86 KiB, below the 120-entry / 384-KiB rotation thresholds. No archive or INDEX update was due.
- Next: continue full DNSMasq caller/lock graph and full LAN restart transaction audit, process identity/readiness, bounded stop/reap and fd/temp/config rollback checks. Do not repeat DNS Forwarder lifecycle locking, failed-stop gate or PID1 result propagation. FTP upload cleanup TOCTOU remains OPEN/BLOCKED; NFS AUTH_SYS/root-squash/durability needs real client/storage evidence; current-main L3/L4 remains unverified.

---

## 2026-11-14 02:37 CST (UTC+8) — Exact-head notification regression and storage governance gates verified

- Exact main at verification: `f01c05a455367e91d3302d2e302902a4eb2ff3dc`; code parent `ec93ae5d070f34b848652f1dbe4b1a441d6cc240`. Re-fetched `refs/heads/main` and enumerated Actions by exact `head_sha` and `event=push`: 28 runs, 28 COMPLETED / SUCCESS, zero failures, zero pending.
- New DNSMasq notification patch verification: run `43483330811`, job `132085197857`, SUCCESS. Full job steps and log inspected: DNSMasq restart policy, DHCP range/router address exclusion, cross-process lock, private Inadyn publication, service postconditions and RC result propagation all pass; assertions verify four `restart_dhcpd()` and two `restart_nmbd()` notification results are merged and no raw discarded calls remain.
- PID 1 notification verification: run `43483330787`, job `132085196895`, SUCCESS. The job compiled `tests/rc_notify_batch_policy.c` with `cc -std=gnu89 -Wall -Wextra -Werror`, ran it successfully, and passed marker trust-boundary / queue source contracts. Positive EBUSY/EIO, negative operation failure, success and existing-failure preservation cases passed.
- Documentation contract: run `43483330899`, job `132085198145`, SUCCESS. Validator reported root `dev_log.md` 30/119, batch=100, retain=20, archive counts dev-log=8, handoff=22, roadmap=18.
- The storage-hotplug regression on the exact main SHA completed SUCCESS: run `43483330558`, job `132085196419`. Full steps/log inspected: BusyBox 1.24.2 ash lifecycle, storage eject inventory, cross-process lock, deferred child result, daemon stop postcondition/group result and whole storage caller graph/lock ordering all PASS. NVRAM exact-head run `43483330633` / job `132085197401` also completed SUCCESS; raw userland bypass count=0 and kernel ioctl/caller contracts PASS.
- The parent storage commit `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` XY-C1 run `43481041936` / job `132077270140` completed: full MIPSel build and TRX verification passed. Artifact `14065946140`, 12,579,764 bytes, SHA256 `f55f38c3dbc64a73994d675c36f802c62c8f97c7b9947f259d56a87461408e06`. XY-C1 workflow path filters do not include the later `rc.c` / notification helper change, so it did not run on current main. Do not claim current-head L3 from the parent artifact; no L4 router test was done.
- No local checkout/compiler run was performed in this environment. The exact-head CI compiled and exercised the relevant host regressions. Full current-head firmware build remains deferred to the project's planned concentrated integration phase.
- Handoff and roadmap status are refreshed with exact-head L2 success, the parent-only XY-C1 L3 result, and the next DNS Forwarder/DNSMasq supervision audit. Root dev_log advances 30→31/119, approximately 83 KiB; no archive or INDEX update is due. No source files changed in this status refresh.
- Next: continue DNSMasq/DNS Forwarder service caller and child supervision audit; preserve explicit disabled-service positive-return semantics in infosvr/crond. FTP upload cleanup TOCTOU remains OPEN/BLOCKED and NFS client evidence remains outstanding.

---

## 2026-11-14 02:30 CST (UTC+8) — Preserve DNSMasq/NMBD restart failures in PID 1 notification markers

- Recovery checkpoint: current main before this batch was `ec93ae5d070f34b848652f1dbe4b1a441d6cc240`; this batch is a fast-forward child on main. The parent had 35 exact-head push workflows: 34 completed SUCCESS and only XY-C1 run `43481041936` / job `132077270140` remained in Build XY-C1. Storage-hotplug [43481041828 / 132077270277], vsftpd [43481042213 / 132077270992], Samba [43481042272 / 132077271289], AI docs [43481042175 / 132077271149], NVRAM [43481042215 / 132077271729], DNSMasq [43481042518] and PID1 notification [43481042145] all completed SUCCESS on the exact parent SHA.
- Caller audit found a notification reliability defect: `handle_notifications()` only retains an incomplete marker when `action_result < 0`, but `restart_dhcpd()` can return positive errno values on lifecycle-lock, config-preparation or daemon-start failure. Four notification branches (RADV, VLMCSD, WINS and DHCPD) discarded its return entirely. The NMBD and WINS branches also discarded the integer status from `restart_nmbd()`. These paths could clear the incomplete marker and acknowledge a failed service restart as successful.
- Added `rc_notify_merge_action_result()` to normalize any nonzero operation status (positive errno or negative failure) to the negative result expected by PID1's marker-retention logic, while preserving an earlier failure. All four DNSMasq callsites and both NMBD callsites now merge the operation result. The WINS path still executes DNSMasq restart and VPN reapply after an NMBD error rather than short-circuiting later work.
- Extended `tests/rc_notify_batch_policy.c` with success, EBUSY, EIO, negative error and prior-failure preservation cases. Added DNSMasq workflow contracts requiring all four `restart_dhcpd()` and two `restart_nmbd()` notification callers to use the helper, no raw ignored call, helper/test coverage and symmetric push/pull_request filters. PID1 notification workflow now also checks the helper's errno normalization contract.
- Source-edit preflight confirmed zero remaining bare `restart_dhcpd();` / `restart_nmbd();` calls in `rc.c` and exact normalized call counts (4 + 2). This is a source transformation/callsite preflight only; no local compiler or C test execution is claimed in this environment. Exact-head DNSMasq, PID1, AI docs and XY-C1 Actions for this new commit are pending.
- Parent `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` XY-C1 run `43481041936` / job `132077270140` was still running Build XY-C1 at the last check; no parent TRX/artifact result yet. Older source `2d5f44b` has a successful artifact, but it does not validate the storage-stop parent or this notification patch. L4 router testing was not performed.
- Changed paths: `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/rc_notify_batch_policy.h`, `trunk/user/rc/src/tests/rc_notify_batch_policy.c`, `.github/workflows/dnsmasq-lifecycle.yml`, `.github/workflows/rc-notification-queue-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev-log count advances 29→30/119. The log remains below the 384 KiB early-rotation threshold; no archive or INDEX edit is due. FTP upload final-name cleanup TOCTOU remains OPEN/BLOCKED; NFS AUTH_SYS/root-squash/client durability still needs real client/device evidence.
- Next: inspect every relevant Actions run on the resulting exact main SHA, especially DNSMasq, PID1 notification, AI docs and XY-C1. On green, continue DNS Forwarder/DNSMasq process supervision and full locked/public lifecycle caller auditing.

---

## 2026-11-14 02:10 CST (UTC+8) — Serialize standalone storage-daemon stop applets with hotplug

- Recovery checkpoint: main SHA `a64c9923a24550177c76f88ea81694725a0230bd`; this commit is a fast-forward child on main. The previously pending XY-C1 validation for functional source `2d5f44b1978c3272fa85633e273297c16bd34283` is now COMPLETED: run `43476679718` / job `132062194706` passed Build XY-C1, TRX verification and artifact upload. Artifact `14064545568`, 12,580,294 bytes, SHA256 `0fba859e6561464feaade2813bd2104d6e789c3829c4f5b30cb5875fa2d40e4d`. This is prior-source L3 only.
- The open stop-applet task was confirmed in the full RC/service call path: standalone `stop_ftp`, `stop_samba`, `stop_dms`, `stop_itunes`, `stop_torrent` and `stop_aria` applets could stop storage-backed daemons without taking the same process-shared lock used by mdev, safe removal and restarts. The combined `stop_ftpsamba` branch could also release neither a shared lock nor a single atomic stop transaction; four add-on applets discarded their stop return status.
- Split all six public stop functions into lock-taking wrappers and private `*_locked()` implementations. Public wrappers fail closed on lock acquisition error and release exactly once. FTP/Samba and four add-on restart wrappers, plus already-locked `stop_stor_apps()`, now call the private helpers, avoiding recursive use of process-associated fcntl locks that could release the caller's outer lock.
- Replaced the grouped FTP/Samba stop applet with `stop_ftpsamba()`: one shared lock covers both stop attempts; Samba is still attempted if FTP stop fails; a small policy helper preserves the first non-zero result. RC now propagates MiniDLNA/Firefly/Transmission/Aria stop results back through `main()`.
- Updated storage-hotplug, vsftpd and Samba workflow source contracts for wrapper lock ordering, failure gates, single unlock, private helper call sites, group status merge and RC return propagation. Existing push and pull_request path filters still include the changed production/contract files.
- Local isolated host test: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` compiled and ran `storage_service_stop_policy.c`; output: `storage daemon stop postcondition and group result policy: PASS`. A source/call-graph/workflow contract preflight also passed. No full checkout or full firmware compile was available.
- Exact-head Actions for the resulting main SHA are PENDING in this handoff: inspect storage-hotplug, vsftpd, Samba, AI docs and XY-C1 build/TRX on this exact SHA; don't reuse the successful 2d5f44b artifact as proof for the new C source.
- FTP upload failed-final-name inode-check/unlinkat TOCTOU remains OPEN/BLOCKED pending a proven Linux 4.4-safe conditional unlink design. NFS AUTH_SYS/root-squash/client durability still requires real client/device evidence. No L4 router test was run.
- Changed paths: `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/storage_service_stop_policy.h`, `trunk/user/rc/src/tests/storage_service_stop_policy.c`, `.github/workflows/storage-hotplug-stability.yml`, `.github/workflows/vsftpd-stability.yml`, `.github/workflows/samba-share-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this `dev_log.md` entry. Resulting main SHA: THIS COMMIT. Root log count advances 28→29/119; no archive or INDEX update is due.
- Next: re-fetch main and inspect every exact-head push workflow, particularly storage-hotplug, vsftpd, Samba and XY-C1. After exact SHA L2/L3 verification, rotate next to DNS Forwarder/DNSMasq lifecycle and child supervision; keep FTP cleanup TOCTOU blocked absent proven race-safe semantics.

---

## 2026-11-14 01:52 CST (UTC+8) — Fail closed on DNS server and resolver publication lock errors

- Source checkpoint: latest main 3ef3eedb18e15e1b29b155107c08d9b4337185bf (workflow-only PR filter correction); functional source changes at 2d5f44b1978c3272fa85633e273297c16bd34283, with fill_dnsmasq_servers() fix at c8b9a377cb530b3716b79279be5d33c4fecd205e.
- Audited shared dservers/resolv lock writers. fill_dnsmasq_servers() previously ignored file_lock("dservers") failure and could stage/publish DNS_SERVERS_FILE without serialization. It now preserves an error, logs and returns before staging/publication. update_resolvconf() previously ignored file_lock("resolv") failure and could generate/publish /etc/resolv.conf without the resolver writer lock; it now returns before opening staging file. lan_up_manual() also ignored the resolver lock result before route update and fopen("/etc/resolv.conf", "w+"); it now aborts before route or resolver side effects if lock acquisition fails. lan_publish_resolver_auto() and DHCP lease deconfig already had failure gates and remain unchanged.
- DNSMasq workflow source contract now checks all three lock acquisition→failure gate→first side-effect sequences, guards, return behavior, and that no unlock is attempted for a failed lock. net_wan.c and net_lan.c added symmetrically to push and pull_request paths.
- Test correction history: c8b9a DNSMasq run 43476277192 failed because the first file_unlock in fill_dnsmasq_servers is in the staging-open failure branch; the initial source assertion incorrectly expected the first unlock after publish. 95c25c1 corrected the checker to assert a post-publish unlock and three total unlocks; DNSMasq run 43476350414 / job 132061059376 PASS. 2d5f44b DNSMasq run 43476679854 failed because net_wan.c/net_lan.c had only been added to push filters; 3ef3eed corrected pull_request filters. Exact 3ef3eed DNSMasq run 43476935451 / job 132063063482 then completed all steps SUCCESS; AI docs run 43476935879 / job 132063065994 SUCCESS.
- Other exact 2d5f44b Actions passed: WAN DHCP 43476679931, WAN ARP socket 43476679928, WAN route 43476679902, detect_link 43476679844, VPN server 43476679812, HTTPD 43476679798, AI docs 43476679772, NVRAM boundary 43476679750, miniupnpd 43476679708, detect_internet 43476679703, WAN interface teardown 43476679687. XY-C1 run 43476679718 / job 132062194706 was still building XY-C1 at last check; TRX verify/artifact pending.
- Earlier storage work remains validated: 79f7963 storage-hotplug run 43474889693 PASS including BusyBox 1.24.2/ash, shared-lock contention 2,000 ms, eject inventory, Optware child status, ASan/UBSan daemon stop policy, and restart caller graph. 79f7963 XY-C1 build and TRX passed; artifact 14063536623, 12,625,941 bytes, SHA256 f37d9d0ab82d43e5c707a78c63999d03db64bfc4ca4e932451af7b05ac30209a. This does not verify later 2d5f44b WAN/LAN source edits.
- No local checkout/compiler/build or router test. Latest functional L3 build pending; L4 NOT VERIFIED. FTP final-name cleanup TOCTOU remains OPEN/BLOCKED.
- Changed production files: trunk/user/rc/src/services_ex.c, trunk/user/rc/src/net_wan.c, trunk/user/rc/src/net_lan.c. Workflow: .github/workflows/dnsmasq-lifecycle.yml. This raises root log 27→28/119; no archive or INDEX update is due.
- Next: re-check exact 2d5f44b XY-C1 run and TRX artifact; then continue standalone storage stop applet call-graph/lock analysis and broaden source reliability governance.

---
## 2026-11-14 01:43 CST (UTC+8) — Serialize all storage-backed public daemon restarts

- Recovery/source checkpoint: current main code SHA 79f79635daa46b748a0a1f987bf56c9e75ab855c, parent 2c2c134a22ae9d4e6a350233365b0c604083a4f9. The parent added the shared eject barrier to FTP/Samba; this commit extends it to MiniDLNA, Firefly, Transmission and Aria.
- Source review found the four add-on public restart wrappers lacked the process-shared storage fcntl lock around stop→/proc/mounts inventory→conditional start. More importantly, their old if (storage_has_mounted_devices(...)) treated an unknown negative result as true in C, potentially launching storage-dependent daemons even when mount inventory could not be read reliably.
- Added the same outer storage-operation lock to restart_dms(), restart_itunes(), restart_torrent() and restart_aria(). Acquisition failure refuses the transition; stop failure aborts restart; startup is permitted only for positive mount inventory; negative/unknown inventory leaves the daemon stopped; each path has one unlock. Transmission/Aria run-state comparison and associated firewall reconcile remain inside the lock.
- Did not add locks to shared internal run/stop helpers: start_stor_apps()/stop_stor_apps(), safe removal and deferred storage startup already call them under the global barrier. Nested same-process acquisition would be incorrect. Standalone stop applets still need a separate full caller/lock audit.
- Expanded .github/workflows/storage-hotplug-stability.yml source contracts to assert lock-before-stop, stop failure gate, inventory-before-positive-start, fail-closed unknown inventory and exactly one unlock for all four wrappers.
- Exact event=push Actions for SHA 79f7963: storage-hotplug [43474889693/job 132056043039] SUCCESS; Samba [43474890036/job 132056043239] SUCCESS; vsftpd [43474889690/job 132056042311] SUCCESS; NFS [43474889702/job 132056042637] SUCCESS; HTTPD security [43474889735/job 132056043153] SUCCESS; AI handoff docs [43474889938/job 132056042846] SUCCESS; NVRAM boundary [43474889827/job 132056042686] SUCCESS. XY-C1 firmware validation [43474889913/job 132056043295] remained IN_PROGRESS at last check (Build XY-C1 executing; TRX verify/artifact upload pending); must refresh before any L3 claim. Every completed job's steps/logs were checked.
- Storage-hotplug log records BusyBox 1.24.2/ash lifecycle regressions PASS, cross-process storage lock blocked 2,000 ms PASS, storage eject inventory and Optware lifecycle/child-result regressions PASS, storage stop postcondition ASan/UBSan PASS, and final full caller graph/lock ordering contracts PASS. NVRAM audit at same exact SHA: 4,179 userland C/C++ files, raw bypass 0; 44,044 kernel sources, external NVRAM API refs 0 and writer refs 0; 57 board configs; snapshot and boundary contracts PASS.
- No local checkout/host compiler test was performed. XY-C1 CI build is not yet concluded; no current-head TRX result is claimed. No router flash/boot/runtime test. FTP upload cleanup TOCTOU remains OPEN/BLOCKED; Linux 4.4 inode-conditional unlink semantics remain unproven.
- Changed paths: trunk/user/rc/src/services_stor.c and .github/workflows/storage-hotplug-stability.yml. Code commit: 79f79635daa46b748a0a1f987bf56c9e75ab855c. This handoff update increments root dev log from 26 to 27/119; archive/INDEX rotation is not due.
- Next: refresh current main and all exact-head Actions, especially XY-C1; then analyze standalone stop applets and every caller of the six storage-backed stop helpers without adding recursive global-lock acquisition. After that, independently review fill_dnsmasq_servers() for missing file_lock("dservers") failure handling.

---
## 2026-11-14 01:32 CST (UTC+8) — Serialize FTP and Samba restarts against storage removal

- Recovery checkpoint: exact main SHA 233d5991afac081e2cd6784d9c212462cd519b07. All six exact event=push Actions at this SHA were COMPLETED / SUCCESS with matching head_sha: infosvr 43470668893, AI handoff docs 43470668983, AdGuardHome 43470669030, AliDDNS 43470668996, AliyunDrive 43470668891 and NVRAM boundary 43470668895.
- Traced the existing process-shared storage barrier. mdev SD/MMC hotplug takes a per-device lock then the global storage-operation file lock; safe_remove_stor_device()/safe_remove_all_stor_devices(), ejected-device cleanup, deferred storage startup and public NFS operations also take that same global lock. The lock is backed by file_lock() and fcntl(F_SETLKW), and the existing cross-process regression verifies blocking between independent processes.
- Traced HTTPD aidisk direct run_ftp/run_samba/run_ftpsamba applets, storage variable EVM→RCN notifications, PID 1 notification dispatch and RC aliases. These paths reach restart_ftpd()/restart_smbd(), which previously stopped daemons, checked /proc/mounts and conditionally started them without the global storage lock. This allowed those service transitions to overlap a different process unmounting the same backing storage.
- Implemented one outer storage-operation lock in each public restart wrapper. It begins before stop/state observation, covers mount inventory and the full conditional FTP/Samba startup (plus Samba WINS decision), fails closed if acquisition or inventory fails, preserves stop/start error returns, and has one unlock exit. Deliberately did not put the lock inside run_ftp()/stop_ftp()/run_samba()/stop_samba(): start_stor_apps()/stop_stor_apps() reuse these helpers while already holding the global lock, so nested same-process acquisition would fail.
- Strengthened three semantic CI contracts: vsftpd stability, Samba share stability and storage-hotplug stability now assert the common lock acquisition precedes stop/inventory/start, unlock follows service/WINS decisions and is performed exactly once. Existing storage_operation_lock regression continues to test cross-process contention. No local execution is claimed; exact-head Actions for THIS COMMIT are pending.
- Updated SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log in the same coherent code/test commit. Root log count advances 25→26/119; no archival shards or docs/dev-log/INDEX.md changes are due.
- Remaining scope: direct standalone stop applets and restarts for other storage-backed daemons still need review. FTP failed-upload final-name TOCTOU remains blocked pending safe semantics. No full local checkout, current-head MIPS/uClibc firmware/TRX build or physical router test was performed.
- Changed paths: trunk/user/rc/src/services_stor.c; .github/workflows/vsftpd-stability.yml; .github/workflows/samba-share-stability.yml; .github/workflows/storage-hotplug-stability.yml; SESSION_HANDOFF.md; REFACTOR_ROADMAP.md; dev_log.md. Resulting main commit: THIS COMMIT; exact-head Actions: PENDING.
- Next: inspect every push workflow triggered for the resulting main SHA, especially vsftpd, Samba, storage-hotplug and AI handoff docs. Fix any failure before claiming L2, then audit the remaining storage-backed daemon restart/stop call graph.

---

## 2026-11-14 01:12 CST (UTC+8) — Snapshot Linux 4.4 NVRAM single-value ioctl reads under the driver lock

- Recovered the live handoff's active NVRAM kernel concurrency task on exact main 0735cd3eff71e9c77360ea55f1c96e6b5e5e855f. Source review confirmed user_nvram_get() called nvram_get(param), which returned t->value after releasing nvram_lock; the ioctl then performed strlen() and copy_to_user() without protecting the pool bytes. Concurrent set/unset/clear/rehash could change the bytes between measuring and copying. This is an unlocked read/write consistency race, not a proven UAF: ordinary clear resets the long-lived pool and does not free the backing nvram_values; module open/exit pinning was also traced.
- Production fix commit: d551d765acb3b2810786e64cca72529b7be1397c. Added nvram_value_span() for pool-bound validation and nvram_get_snapshot(): capture size under spinlock, allocate outside, re-resolve the key and revalidate under spinlock, copy the complete value while still locked, then call copy_to_user() only after unlocking. Growth between passes is retried at most three times; missing keys and invalid pool boundaries fail explicitly. user_nvram_get() no longer retains the unlocked nvram_get() pointer.
- Added nvram_snapshot_policy.h plus GNU89 host regression for capacity-ready, concurrent growth/retry, shrink and missing terminator/pool-boundary cases. The full source checker now checks lock/allocate/relock/copy ordering, absence of copy_to_user in the helper, bounded retry, all in-tree kernel writer lock coverage, init-before-register, exit unregister-before-free and open/release module pinning.
- The first exact NVRAM Actions attempt on d551 (43468924620) failed only because the new host test compile omitted -I trunk/linux-4.4.x/drivers/nvram; compiler could not find its sibling policy header. This harness error was fixed by CI-only commit a7a70397ec9f6458655a7f0520769c5270436258, without changing production source. Exact a7 NVRAM run 43469265744 / job 132036533462 and AI docs run 43469264009 both COMPLETED / SUCCESS.
- Exact NVRAM log on a7 confirms NVRAM single-value snapshot sizing policy: PASS, device node permission policy PASS, shared transaction lock inode/permissions/contention/symlink/hardlink/cross-process/crash-release PASS, kernel single-value ioctl snapshot contract PASS, and final source/inventory checker PASS. Inventory: 4,179 userland C/C++ files, adapter raw refs 25, external bypass 0; 44,044 kernel source files, external NVRAM API refs 0 and writer refs 0; 57 board configs.
- This closes only the kernel driver's single-value NVRAM_IOCTL_GET snapshot race at L1/L2. The legacy exported nvram_get() pointer-returning interface remains unchanged; the whole-tree scan found no other in-tree kernel caller, but out-of-tree callers are not ruled out. No full Linux kernel build, MIPS/uClibc firmware/TRX build or L4 router test was run. No archive/INDEX update is due; root log rises from 24 to 25 records.
- Updated live SESSION_HANDOFF.md and REFACTOR_ROADMAP.md to close the NVRAM ioctl slice and set the next task to verify storage-operation lock coverage around FTP/Samba restart versus safe removal/hotplug. These docs changes follow this production/CI checkpoint; the resulting docs-only commit will be THIS COMMIT.
- Next: inspect exact-head Actions for the docs-only commit, then trace restart_ftpd() / restart_smbd() and safe removal/hotplug lock order before deciding whether a concrete cross-process stability bug exists.

---

## 2026-11-14 00:38 CST (UTC+8) — Keep infosvr EBUSY failures visible to the RC queue

- Follow-up review of `aec7b2ca6d75712f6c18a9712d4478c2c3172070` caught a return-convention bug in the conservative post-stop recheck: `EBUSY` is a positive errno, but PID 1's notification queue classifies `action_result < 0` as failure. A stop-success / process-still-visible race would therefore have been logged but incorrectly acknowledged.
- Corrected the branch to preserve the negative `kill_services()` status; when the fresh process check alone blocks startup, it now sets `errno = EBUSY` and returns `-1`. The source checker now enforces the negative result and checks the ordering through the launch gate. This is a follow-up correction to the just-landed infosvr lifecycle batch, not a new scope.
- Existing GNU89 policy unit test remains applicable and passed previously with `-Wall -Wextra -Werror`, ASan/UBSan and leak detection. The updated source checker will run in the exact-head infosvr workflow. Actions for the parent SHA `aec7b2ca...` are not final evidence for this corrected code.
- Changed paths: `trunk/user/rc/src/services.c`, `scripts/check_infosvr_lifecycle.py`, and this dev-log record. New main SHA: THIS COMMIT; exact-head Actions pending.
- Next: inspect this corrected SHA's complete push Actions and logs; if all green, refresh the handoff/roadmap with the verified NVRAM closure and the exact infosvr source/CI checkpoint.

---

## 2026-11-14 00:32 CST (UTC+8) — Gate infosvr restart on a confirmed daemon stop

- Exact-main recovery: live main `199d0f23f09931a84acb965a322a42bc32496830`. This is a docs-only continuation of `231fa506b898cf629391a6b6846ee36f3a5f78c4`; the exact 231fa push Actions were all green, including HTTPD, NVRAM boundary, AI docs, infosvr, four add-on workflows and XY-C1 firmware/TRX artifact. The NVRAM root-only fix is therefore closed at the audited shared-userland boundary, not a claim of kernel/external-writer global atomicity.
- Also inspected 199d exact-head push runs. Its AI docs job passed (root dev-log 22/119, dev-log archives 8, handoff 22, roadmap 18); NVRAM whole-tree scan passed: 4,177 userland C/C++ sources, 0 raw device/ioctl bypasses outside the locked adapter, 44,042 kernel sources, 0 external kernel NVRAM API/writer refs and 57 board configs. The only flagged kernel follow-up is the already evidence-gated `user_nvram_get()` tuple-pointer snapshot lifetime; no speculative kernel change is made. The exact 199d NVRAM and AI run IDs are 43463532490 and 43463532413.
- Repository-wide service-lifecycle rotation selected RC infosvr after source review confirmed a concrete duplicate-start risk: `stop_infosvr()` discarded the bounded `kill_services()` result; `restart_infosvr()` always launched a replacement regardless of whether the old daemon survived; PID 1's `RCN_RESTART_ADSC` branch discarded the restart result even though the notification queue already retains completion markers for negative `action_result`.
- The code change returns the infosvr stop status, requires both stop success and a fresh no-process check before restart, returns `EBUSY` if a matching daemon remains, and propagates the restart result through the ADSC notification dispatcher. Added a GNU89 policy regression for failed stop, lingering process and successful stop, plus a source/call-path checker. The existing infosvr Actions workflow now watches the RC production/helper/test/checker files on both push and pull_request and runs the new regression under `-Wall -Wextra -Werror`, ASan/UBSan and leak detection.
- Exact helper/test host run: PASS (GNU89, Werror, ASan/UBSan, leak detection). The new end-to-end source checker and GitHub Actions for this commit are pending; do not mark the integrated fix closed until the exact-head infosvr and AI-docs jobs are inspected. No full local checkout, MIPS build, TRX artifact or router test was run for this infosvr change.
- Root dev-log will retain 23 complete newest-first records, below rotation threshold; no archive or index update is due.
- Changed paths: `trunk/user/rc/src/services.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/rc.c`, new `trunk/user/rc/src/infosvr_daemon_policy.h`, new `trunk/user/rc/src/tests/infosvr_daemon_policy.c`, new `scripts/check_infosvr_lifecycle.py`, `.github/workflows/infosvr-packet-stability.yml` and this dev-log entry. Resulting main SHA: THIS COMMIT.
- Next: inspect exact-head infosvr and AI handoff Actions plus full job logs; repair any true source/contract/build failure before selecting another repository-wide stability target.

---

## 2026-11-14 00:23 CST (UTC+8) — Pin exact-main NVRAM CI continuation checkpoint

- Live main at recovery: `231fa506b898cf629391a6b6846ee36f3a5f78c4` (`test(ci): fix HTTPD NVRAM contract indentation`). This round updated the live handoff in place at `8036bb29a626e2140abf009e665acad4e0d5d16f` to distinguish the exact main SHA and the remaining verification blocker; no production source, test or workflow code changed.
- Re-read the active handoff, roadmap, newest root dev-log records and archive index. The recorded NVRAM device-node permission / HTTPD YAML validation task remains open. Do not select a new module before exact-head push checks are reconciled.
- Remote branch inventory returned only `main`; the next branch page was empty. PR #1 is CLOSED / UNMERGED (head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`). Compare to current main reports 17 PR-ahead commits, 160 main-ahead commits and 13 changed files. Its closing comment / prior handoff state that valid CA/HMAC work was ported to main. No PR or ref mutation was performed. Tag enumeration is unavailable through the connected actions.
- Current-SHA CI is **NOT VERIFIED**, not PASS: combined statuses for `231fa506...` were empty, and the available commit-workflow-runs action only returns PR-triggered runs; it returned no run for this SHA. GitHub Actions push-run list / API could not be accessed through the available interface. Older-parent results cannot satisfy exact-head validation.
- No host tests, Python docs checker, MIPS build, firmware/TRX generation or hardware test were run in this round. No local checkout was available. No L3/L4 claim is made.
- Changed paths in the two documentation commits: `SESSION_HANDOFF.md` and `dev_log.md`. This entry is part of the dev-log commit: THIS COMMIT. Exact-head AI docs Actions for that resulting SHA: NOT VERIFIED.
- Next: retrieve all push Actions for exact main SHA `231fa506b898cf629391a6b6846ee36f3a5f78c4`; match `event=push` and exact `head_sha`; inspect HTTPD/NVRAM/docs and relevant sibling workflow jobs and full logs. Repair any causal failure on main before closing the NVRAM task and resuming repository-wide P2 stability work.

---

## 2026-11-14 00:08 CST (UTC+8) — Repair HTTPD umbrella workflow block indentation

- Base main SHA: ead18adf3dbdf8dd2cf156f889501325bc3b1534. The source-contract update had correct assertions but the newly inserted Python lines after node_policy were missing the YAML run-block indentation. GitHub therefore emitted a zero-job failed workflow run [43459535203] named after the workflow path; it never reached a test step. This was a CI syntax/indentation defect, not a runtime regression.
- Re-indented every inserted Python source-contract line under the existing run: block. The contract still verifies NVRAM_DEVICE_NODE_MODE 0600, init_nvram_node metadata verification and lstat/chmod calls, init_nodes error propagation, and the PID 1 router_ready gate. New node-policy header and test remain present in push and pull_request path filters.
- The same SHA's NVRAM audit workflow [43459538956] was still scanning; previous parent SHA 728098a793bdbb072f7c0f4b708d8922a47241c5 passed both NVRAM lock/device-node host regressions and full inventory (4177 userland sources, zero raw bypasses, 44042 kernel sources, zero kernel API refs, 57 board configs).
- No production runtime files changed in this repair. The previous log rotation to part-008 is complete; root dev_log.md currently retains 20 complete newest-first records and will contain 21 after this entry, within policy. No new archive rotation is due.
- Exact-head HTTPD, NVRAM boundary, AI docs and sibling Actions for this workflow indentation repair: THIS COMMIT / pending. No local full build, target MIPS compilation, TRX image or hardware test is claimed.
- Changed paths: .github/workflows/httpd-security.yml, SESSION_HANDOFF.md and dev_log.md. Resulting main commit: THIS COMMIT.
- Next: inspect full exact-head Actions. Verify HTTPD run gets a real job and passes the updated source contracts; verify NVRAM scan, AI archive integrity and all affected sibling checks. Then close the NVRAM permission fix and return to wider stability coverage.

---

## 2026-11-14 00:05 CST (UTC+8) — Align HTTPD NVRAM recovery contract with PID 1 access control

- Base main SHA: 728098a793bdbb072f7c0f4b708d8922a47241c5. The new NVRAM security commit's exact-head AI docs [43458726546], NVRAM scan workflow [43458726491 pending at this entry], DNS Forwarder [43458726367], mdev config [43458726816], PID 1 notification queue [43458726842], USB storage [43458726601], AliDDNS [43458726360], AdGuardHome [43458726307], infosvr [43458726538] and AliyunDrive [43458726264] were checked by exact head SHA. All listed completed sibling runs are SUCCESS; NVRAM audit remains pending while its whole-tree scan runs.
- HTTPD umbrella [43458726361] failed only at the stale source-contract assertion router_ready = (init_router() == 0);. The underlying NVRAM restore policy, restore rollback, profile transaction and policy C regression all passed. PID 1 now intentionally gates that call with nvram_node_result == 0, so the previous exact-string assertion is obsolete, not a production failure.
- Updated .github/workflows/httpd-security.yml to assert the new root:root 0600 device-node policy, the audited init_nvram_node() metadata validation, init_nodes() error propagation and fail-closed init_main_loop() gate. Added the new policy header and test to both push and pull-request path filters so changes cannot silently bypass this umbrella.
- This workflow contract update does not weaken any test; it replaces the outdated unconditional-start assertion with checks for the stronger secure-startup contract. Existing production code was not changed in this commit.
- Root log reached 120 complete records with this entry, so performed the mandatory batch rotation: the oldest 100 complete entries are preserved verbatim oldest-first in docs/dev-log/2026/11/part-008.md; the newest 20 records remain in dev_log.md. Updated docs/dev-log/INDEX.md in the same commit. The archive was checked for 100 complete headings, chronological order and a sub-512-KiB byte size.
- Exact-head HTTPD + NVRAM + AI docs runs for this workflow-contract commit: THIS COMMIT / pending at authoring time. No target MIPS build, firmware/TRX image or router hardware test is claimed.
- Changed paths: .github/workflows/httpd-security.yml, SESSION_HANDOFF.md, dev_log.md, docs/dev-log/INDEX.md, and new archive docs/dev-log/2026/11/part-008.md. Resulting main commit: THIS COMMIT.
- Next: inspect exact-head NVRAM boundary and HTTPD workflows. Confirm the 0600 device node policy and all tests; then resume the repository-wide stability rotation through DNS/resolver, crond and storage/service supervision.

---

## 2026-11-13 23:55 CST (UTC+8) — Restrict NVRAM character device to root

- Base main SHA: 16caf1d0174ec8cbd4d0ce9ee37570e782255677. Its NVRAM boundary audit exact push run 43456843141 scanned 4175 userland and 44042 kernel C/C++ source/header files. It found zero in-tree kernel API references outside the common driver, but found PID 1 creating /dev/nvram as S_IFCHR|0666 in init.c:376. init_main_loop sets umask(0000), so the node was definitely world-accessible; device ioctl handlers expose SET/CLEAR/COMMIT without a kernel privilege check. The old audit failed on this confirmed finding; the independent shared-lock test compiled and ran PASS.
- Added nvram_device_node_policy.h and tests/nvram_device_node_policy.c. The policy requires a character device with the exact 228:0 device number, root:root ownership, mode 0600 and one link. Host cases reject world/group writable permissions, non-root uid/gid, wrong device number, regular-file/symlink substitution and unexpected hard links.
- Updated trunk/user/rc/src/init.c: init_nvram_node() creates or repairs an existing /dev/nvram node, verifies type and major/minor before changing ownership/mode, rechecks the final inode metadata, and fails on unsafe replacements or permission errors. init_nodes() propagates this failure; PID 1 skips init_router() and stays in recovery mode if the NVRAM device cannot be secured. Other hwnat/crypto/tun node policies are unchanged.
- Updated scripts/check_nvram_transaction_boundary.py to allow the device path only through this audited PID 1 setup and to verify the exact 0600, node-identity and fail-closed startup contract. Updated the NVRAM workflow to compile/run both policy and lock regressions before whole-tree inventory.
- Exact-head validation: THIS COMMIT / pending at authoring time. No local full checkout, compile or host test is claimed. The next exact-head workflow must prove the new policy test, prior lock test, 4175-file raw path scan, kernel API and board geometry inventory all PASS. AI handoff docs and sibling workflows must also match the resulting main SHA.
- Potential kernel user_nvram_get() post-unlock tuple-pointer copy remains a separate candidate. No external in-tree kernel NVRAM API callers were found by the full kernel scan; no speculative kernel patch is included.
- Root dev-log record count: 118 to 119. The previous audit entry timestamp is corrected to 23:51 CST; no archive rotation or INDEX change is due. The next new record (record 120) requires a single batch rotation of the oldest 100 complete records, retaining the newest 20.
- Changed paths: trunk/user/rc/src/init.c, trunk/user/rc/src/nvram_device_node_policy.h, trunk/user/rc/src/tests/nvram_device_node_policy.c, scripts/check_nvram_transaction_boundary.py, .github/workflows/nvram-transaction-boundary.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. Commit message: fix(nvram): restrict character device to root. Resulting SHA: THIS COMMIT.
- Next: inspect exact-head Actions, ensure /dev/nvram is root-only before init_router, then resume repository-wide stability rotation through crond, PID 1 service notifications, storage and DNS/resolver.

---

## 2026-11-13 23:51 CST (UTC+8) — Add whole-tree NVRAM transaction boundary audit gate

- Recovery baseline: live main SHA b14f4e5c707acc2e31fbb2fd73674fd6de3837ff; only remote branch is main, no open PR. f078956275ed6362a71c66f4751a3e1f9eb124b0 to b14f4e5c707acc2e31fbb2fd73674fd6de3837ff changes only SESSION_HANDOFF.md. Exact-head push Actions at b14f4e5 all SUCCESS: AliDDNS 43454623297, AdGuardHome 43454623265, infosvr 43454623330, AliyunDrive 43454623162, AI docs 43454623438.
- Inspected legacy PR #1 (closed/unmerged, head ba48e22609e0c81ad757b06756fe91d05f39cbf6). Its valid HMAC/CA implementation and regression files are already present on main; helper/source/test/CA bootstrap/copyscripts blobs match PR head where applicable, while main aliddns.sh is newer. No cherry-pick or ref mutation is required.
- Continuing the recorded NVRAM boundary task: added scripts/check_nvram_transaction_boundary.py to scan every userland C/C++ source/header for raw /dev/nvram and NVRAM_IOCTL_* references outside the locked shared adapter; verify shared wrapper/transaction lock source contracts; inventory kernel nvram_get/getall/set/unset/commit/clear references outside the common driver; and report board NVRAM geometry selectors plus common driver wiring.
- Added .github/workflows/nvram-transaction-boundary.yml, main push only. It compiles and runs the existing NVRAM lock regression with GNU89, -Wall -Wextra -Werror, ASan/UBSan and leak detection, then runs the whole-tree inventory script. No production kernel/userland runtime code changed in this audit-gate commit.
- Exact-head CI status: THIS COMMIT / pending at authoring time. Do not claim the repository-wide inventory is complete until the workflow full log output is inspected. Source review identified a separate potential kernel user_nvram_get() snapshot-lifetime window (nvram_get() returns an internal tuple pointer before copy_to_user()); it is explicitly a follow-up candidate, not yet a confirmed or fixed defect.
- Local full checkout remains unavailable in this environment; no local compile, sanitizer execution or Python scan is claimed. Prior exact eebac9 HTTPD NVRAM lock integration passed 41/41 steps, and XY-C1 L3 build/TRX evidence remains limited to its recorded ancestor source SHA; no current-main L3 or L4 result is claimed.
- Root dev-log count: 117 to 118 complete records; current file remains below 384 KiB and 512 KiB; no archive rotation or INDEX mutation is due. Updated live handoff/roadmap for the exact next action.
- Changed paths: scripts/check_nvram_transaction_boundary.py, .github/workflows/nvram-transaction-boundary.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. Resulting main commit: THIS COMMIT.
- Next: inspect the new exact-head audit workflow and determine whether any raw userland writer exists, enumerate real kernel API writers, and complete the board MTD geometry matrix. Fix only a confirmed bypass or lifetime race with a reproducible regression; otherwise rotate to the next P2 service-supervision target.

---

## 2026-11-13 23:28 CST (UTC+8) — Serialize shared-userland NVRAM restore and repair exact-main regressions

- Recovery began from live main `dcf13295f9e95b992aa75d46cf022c326c7ef613`; the inherited active task was to establish whether NVRAM profile restore snapshots, clear, tuple apply and rollback could interleave with concurrent writers. Main advanced during recovery to `1e260d7de91c13cebf3677f87eaa786c62560063`, whose tree accidentally contained only the `.github` and `trunk` roots and dropped roughly 64,000 repository paths, including governance records, scripts, toolchain and most Actions workflows. Its exact-head HTTPD run `43449907836` failed because `autostart.sh` was absent. A forward-only repair commit `d1099f49b08ed337a685e0cc33229142d8499a75` restored the complete `dcf132` parent tree and retained the six intended NVRAM/shared/workflow blobs; no history rewrite, force push, new ref or PR was used.
- The NVRAM transaction implementation now adds `nvram_transaction_lock.h`: a stable owner-only `/tmp/.nvram.api.lock` inode, root/sticky-`/tmp` validation, `O_NOFOLLOW|O_CLOEXEC`, regular-file/owner/mode/link-count checks and bounded `flock` contention. The shared NVRAM wrapper serializes get/getall/set/set-temp/unset/commit/clear; the explicit transaction API retains the exclusive lock while profile restore snapshots persistent and volatile tuples, clears, applies and performs compensating rollback. Process ownership is checked at transaction use/end. This is shared-userland serialization only; raw `/dev/nvram` ioctl bypasses and every board/caller have not yet been exhaustively inventoried, so do not claim global NVRAM atomicity.
- The exact lock helper/test blobs (`988de48449223148d4e9c1e8b923f55b4c97dc8c`, `0b6a1824fb451e818317607ddfa8945e3794febd`) were compiled locally with GNU89, `-Wall -Wextra -Werror`, ASan/UBSan and leak detection; the inode, permissions, symlink/hardlink rejection, bounded contention, cross-process exclusion and crash-release regression passed. No complete local checkout was available.
- Exact main XY-C1 build first exposed a real production compile error: `nvram_linux.c` used `struct nvram_transaction` without including `nvram_linux.h`. Commit `afe5e2a906be66dc0403cbf4ee973f54e4768e8c` adds the missing include. The first HTTPD attempts then exposed stale source-contract assertions which still expected the pre-transaction direct `nvram_getall`/setter and raw-ioctl bodies. Commits `a703bf0802d5401e50243bb591959ed4b6b3cdae` and `eebac9fd0e4b27f9cbf0c23b5fd3353b3a9e8b0a` update those checks to validate the transaction-begin/snapshot/apply/end order and the `*_unlocked` ioctl helpers. Exact `eebac9` HTTPD run `43452607752` / job `131978400648` passed all 41 steps, including the NVRAM profile, rollback, RC and shared-userland transaction regressions. Exact XY-C1 run `43452421415`, head_sha `a703bf0802d5401e50243bb591959ed4b6b3cdae`, completed SUCCESS: Build XY-C1, Verify TRX output and artifact upload all passed. Artifact `14051533481` (`XY-C1-firmware-a703bf0802d5401e50243bb591959ed4b6b3cdae`) is 12,579,096 bytes, SHA-256 `ab9dd2592f9e9571c9f5d4b4977b2aceb9db786a4d087915ba06475b7a958466`, expires 2026-11-20. Compare `a703bf0` to the then-current main `536d558` shows only `.github/workflows/httpd-security.yml` and documentation changes; no production source changed after the successful code SHA.
- The DDNSTO lifecycle run on `d1099f4` initially exposed an asynchronous test-observation race: the mock wrote its START marker just before publishing state=running. Commit `85aaeaea2f083e3fddb73e4ab1a6311e6bde746c` makes the bounded test wait for both conditions. The rerun and the exact `85aae` DDNSTO run `43451687593` both passed; this changed the regression observation, not production lifecycle behavior.
- Verification level: NVRAM shared-userland lock has GNU89/Werror/ASan/UBSan host evidence (L2), exact HTTPD 41-step Actions success, and an XY-C1 MIPS/uClibc firmware cross-build + TRX output + artifact upload success at production-code SHA `a703bf0` (L3 for XY-C1 only). No L4 router flash/boot or real concurrent NVRAM writer test was run. Prior intermediate failures are retained above rather than attributed to the final passing head.

## 2026-11-13 22:32 CST (UTC+8) — Restore prior NVRAM state after profile-apply failure

- Recovery baseline: previous handoff production checkpoint `d3e8edc792a1001d475e4e1fcbe6e06496399efd`; live main at the start of this unfinished batch was `30b9e7134382eac747bd8f2d6dbb557d9107231c`. Only `main` is authorized; no branch, tag, PR or GPT Session Rotator checkpoint was created or changed.
- Audited the in-progress NVRAM profile restore implementation. The previous path validated the incoming profile, called `nvram_clear()`, then applied tuples; a clear or later `nvram_set()` failure could leave the live persistent configuration empty or partially replaced.
- `f1f5cef69d9daa64ea81d7b8373fa33aef264e82` implements the restore transaction helper and integration: snapshot/validate both persistent and volatile tuples before mutation; on initial clear or profile-apply failure, attempt a second clear and replay both previous snapshots; continue rollback after an individual setter error, preserve the primary failure errno and report incomplete rollback. The helper intentionally describes best-effort compensation, not flash-atomic recovery.
- The same batch adds the volatile-only `nvram_getall(..., 2)` mode, documents 0=persistent / 1=all / 2=volatile-only, rejects unknown ioctl modes, makes the kernel tuple exporter return `-EOVERFLOW` rather than silently truncate a full snapshot, and propagates shared-wrapper invalid-argument, ioctl and close failures. The CLI show caller now checks getall failure.
- Source/test/workflow sequence: `f1f5cef69d9daa64ea81d7b8373fa33aef264e82` production rollback; `5c37855d174291031a40b7e0bd04b89d96d47b0e` HTTPD workflow contracts and path coverage; `30b9e7134382eac747bd8f2d6dbb557d9107231c` codec parse/decode regression; `f1371be9e12a03937370fff7cdf07cb03a3d823e` fixes the new test's double-escaped NUL character literal and diagnostic newlines.
- Intermediate exact-head failures are recorded: f1f5 and 5c378 HTTPD runs failed because the new test included the profile policy header while parser/decoder static helpers were not yet called (`-Werror=unused-function`); 30b9 added their regression use but its first HTTPD run exposed a double-backslash multi-character constant (`'\\\\0'`). The test fix was made on main, not bypassed.
- Exact f137 Actions: HTTPD Authentication and Security [43443643075 / job 131947352282] SUCCESS; AI handoff docs [43443642982 / job 131947351586] SUCCESS; both `head_sha` exactly equals f137. HTTPD job passed all 40 listed steps. The GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` restore/codec binary and source contracts passed; logs show `NVRAM restore transaction and profile codec regression: PASS` and `NVRAM restore rollback, complete snapshot, temporary tuple and overflow contracts: PASS`. AI docs validator passed with root log 115/119, batch=100, retain=20, archives dev-log=7 / handoff=22 / roadmap=18.
- The restore snapshot/clear/replay sequence consists of multiple independent ioctls. This batch does not establish serialization against concurrent NVRAM writers and does not claim cross-call atomicity. The next handoff must inventory all direct callers and boards, trace existing writer locks and add a deterministic interleaving regression before changing the transaction lock model.
- No complete local checkout or local tests were available. Exact f137 has no current-head MIPS/uClibc full firmware/TRX result or router test; the earlier XY-C1 runs were on ancestor SHAs. No live router profile restore / flash-failure / concurrent-writer test is claimed.
- Changed production/test wiring includes `trunk/user/nvram/src/nvram.c`, new `nvram_restore_transaction.h` and host regression, `trunk/user/shared/src/nvram_linux.c/.h`, kernel NVRAM tuple exporter/ioctl, and `.github/workflows/httpd-security.yml`.
- Root `dev_log.md` was 115/119 records and 286652 bytes before this entry; now 116/119 and still below the 384 KiB early-roll threshold. No archive or `docs/dev-log/INDEX.md` change is due.
- Production/test checkpoint: `f1371be9e12a03937370fff7cdf07cb03a3d823e`; this record and the refreshed handoff/roadmap are committed in the following docs-only checkpoint.

---

## 2026-11-13 22:16 CST (UTC+8) — Bind FRP process identity to the kernel executable

- Recovery baseline: be42800f99d4f6d1d187f9aea235f9e71eb8ae4e; only development branch is main. The previous DHCPv6 callback child lifecycle task was re-audited before selecting this next handoff item.
- DHCPv6 review traced net_wan6.c session lock/generation/interface checks, generation publication and error propagation; BusyBox dhcp6c_script.c uses synchronous spawn_and_wait(), preserving exec errno and script exit status through wait/reap; dhcp6c.c logs failures. Stop advances the generation before terminating the client. No new independently confirmed lifecycle defect was found, so no gratuitous DHCPv6 source patch was made.
- FRP process identity review found frp_process_matches() trusted the cmdline argv tuple alone. argv[0] is caller-controlled; an unrelated local program could mimic frpc -c /tmp/frp/myfrpc.ini and be mistaken for the owned daemon during root TERM/KILL cleanup.
- Commit ea9b86690ec5276e04253a7b57dd7bf022b69a4f binds frpc / frps to fixed /usr/bin/frpc /usr/bin/frps executable paths via the kernel-reported /proc/PID/exe, while retaining the complete argv tuple check and pre-signal revalidation. Added watchdog and stop regressions for an impostor whose argv is exact but executable points to /bin/sleep; CI source checker covers the executable map and procfs path.
- Intermediate exact-head failures were inspected and repaired: FRP run 43440341757 on ea9 failed because the test fixture re-created existing exe symlinks and lacked a directory for fake PID 204; 81a2d0471891a4480ac68c02387956f6511051d6 fixed those fixtures, after which shell regressions passed but the Python source checker had an indentation error; d3e8edc792a1001d475e4e1fcbe6e06496399efd corrected the checker tuple.
- Final exact-head Actions on d3e: FRP lifecycle 43440754101 / job 131937424207 SUCCESS; AI docs 43440754169 / job 131937424241 SUCCESS. The FRP job logs show PASS for daemon detection/readiness, managed cron, role-aware boot, private config and all paired-config rollback cases, bounded TERM/KILL stop, launcher lock/FD 9 and the Python source/workflow checker.
- The paired client/server config transaction rollback and bounded asynchronous readiness pre-existed this batch; the current exact-head suite was re-run and passed them. This batch changes process identity, not the paired config transaction.
- Linux 4.4 lacks pidfd-like process handles; the narrow PID-reuse window between final /proc/PID/exe revalidation and kill(2) remains. Do not claim that procfs checking eliminates PID reuse completely.
- No full local checkout / local tests, current-head MIPS/uClibc firmware build, TRX artifact or L4 router test was available. The older e9 XY-C1 build is ancestor evidence only, not current-main L3.
- Changed files: trunk/user/frp/frp.sh, trunk/user/frp/tests/frp_lifecycle.sh, trunk/user/frp/tests/frp_stop.sh, scripts/check_frp_lifecycle.py. Source commit chain: ea9b866..., 81a2d04..., d3e8edc....
- Root dev_log.md record count: 114→115/119; file size remains under 384 KiB, so no archive rotation or docs/dev-log/INDEX.md change is due.
- Resulting code checkpoint: d3e8edc792a1001d475e4e1fcbe6e06496399efd; handoff, roadmap and this record are refreshed in the following documentation checkpoint.

## 2026-11-13 21:03 CST (UTC+8) — Complete secondary WAN bridge batch rollback

- Base/source SHA: e9c38f80e74c5d1e80ce6edbe74310e3645be2fe; sole development branch main. Source commits this round: de29e51b643d60e829b89027496a0244ad0176fe implements the multi-bridge ownership ledger and reverse rollback; 17956f0f7938e6430dd2a1a2515eb97f58d0ac42, d304121e7e3f8d1ca854a4da28ad58603a47e7d7, and 0905bf5af4863ccba54926c8f562744814c48ca2 align source/workflow contracts and path-section checks; e9c38f80e74c5d1e80ce6edbe74310e3645be2fe fixes the staged shared-header include after the first target build exposed the wrong relative include.
- config_soft_bridges_other() now tracks up to four outer IFNAME_MAC2.<vid> VLAN / brwan2..5 bridge entries, including which VLANs/bridges were created and which WAN/bwan port memberships were added. On failure it reverses the current batch in reverse construction order, only removes per-call-created VLANs, and removes a new bridge only when a complete Linux 4.4 BRCTL_GET_PORT_LIST snapshot proves it empty. Existing objects are restored to their captured UP/DOWN state when applicable; uncertain/non-empty bridges are preserved and kept down. No br_del_all_ifs() is used.
- Added shared br_bridge_is_empty() query using the full 1024-entry Linux 4.4 bridge port list, plus policy helpers and GNU89 regression cases for empty/non-empty/unknown inventory, created-vs-preexisting bridge deletion, and owned/ambiguous membership. Workflow path filters now include net_wan.c; source checker checks reverse cleanup order and full inventory gating.
- First de29 XY-C1 build did NOT pass: it stopped at net_wan.c because ../../shared/src/bridge_membership_policy.h is not present at that relative path in the RC build tree. This was corrected in e9 to use the existing staged include path #include "bridge_membership_policy.h". Exact e9 XY-C1 run [43429016318 / job 131897515445] then completed SUCCESS: MIPS/uClibc build, TRX output verification and artifact upload all passed. Output trunk/images/TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261113.trx; artifact ID 14039361485, upload size 12,576,910 bytes, package SHA256 581c06a9e8fe011322516cdd3a655f9c658f383b2e1303c3643d6e05b419e839.
- Exact e9 SHA has 11/11 push Actions SUCCESS (all head_sha=e9c38f80...): WAN route [43429016219], AI handoff [43429016261], WAN teardown [43429016457], HTTPD security [43429016301], WAN DHCP callback [43429016308], WAN ARP [43429016193], detect_link [43429016366], detect_internet [43429016453], miniupnpd [43429016528], VPN lifecycle [43429016754], and XY-C1 [43429016318]. WAN route job [131897514715] compiled/ran GNU89 -Wall -Wextra -Werror ASan/UBSan policy tests and passed VLAN/bridge + route-result/cleanup source contracts.
- Build logs still contain non-fatal warnings: parted/libtool attribute/moved-library warnings, depmod missing modules.builtin.modinfo, and objcopy/strip "no sections" for empty placeholder ddnsto and zerotier-one; workflow still verified the TRX and uploaded the artifact. Do not describe the build as warning-free.
- No complete local checkout or local tests were available; all host tests/contracts and the target build evidence are from exact-head GitHub Actions. Physical router flash/boot and L4 multi-bridge runtime remain NOT VERIFIED. Rollback is compensating rather than fully atomic; IPv6 sysctl side effects and pre-existing M2U/MAC/MTU/STP/priority attributes are not comprehensively restored.
- Active next task: audit DHCPv6 callback child lifecycle in trunk/user/rc/src/net_wan6.c (PID/generation association, wait/reap, stop/restart races, stale child cleanup), then return to the whole-repository stability/security priority queue.
- Root log record count: 113→114/120; no archive rotation or docs/dev-log/INDEX.md update is due. Documentation result: THIS COMMIT.

## 2026-11-13 20:22 CST (UTC+8) — Add ownership-aware LAN bridge rollback and validate XY-C1

- Recovery baseline: `98079bdb24d4191318ee95ca966835a8a723be95`; authorized branch remains `main`. Branch inventory contains only `main`; no checkpoint branch/tag/PR or GPT Session Rotator changes were made (pre-existing `refs/pull/1/head` and `refs/tags/toolchain` were left untouched).
- Following the existing WAN rollback chain (`ea38b3ef`, `36c7a798`, `fcac7ee1`, `44b0353c`), commit `8195276a5d20fbca8a51b970f12bf80e900424c1` implements a tracked, compensating transition in `config_soft_bridge_lan()`: snapshots VLAN existence/UP state, tracks changes to physical/VLAN bridge membership, delays old VLAN removal until the replacement port is configured, undoes only invocation-owned membership, removes only a newly created VLAN known to be detached, restores a pre-existing VLAN's UP/DOWN state when safe, and preserves the primary errno/status. The policy gates and test cases were added to `bridge_membership_policy.h` / its host test.
- First WAN route run after `8195276a` [43422397171 / job 131875325075] and the next after `ddf9d691` [43422704383 / job 131875358085] both compiled and ran the bridge policy sanitizer binary successfully (`Bridge membership and teardown policy regression: PASS`) but failed the static source contract because it required bare membership markers rather than the actual guarded rollback conditions. `ddf9...` also removed an unused local. The final checker correction in `d9ae3f12cf24e37402cfaa6293f79a1856169061` matches `if (physical_detached && vlan_state_safe)` and `if (vlan_detached && physical_state_safe)`; no production source changed in d9.
- Exact d9 WAN route [43423405123 / job 131878717264] completed SUCCESS: route policy compile/run with GNU89, `-Wall -Wextra -Werror`, ASan/UBSan; bridge membership policy binary PASS; WAN VLAN/bridge source contract PASS; route result propagation / cleanup ordering source contract PASS. Exact d9 AI handoff contract [43423404888 / job 131878716417] also SUCCESS.
- On exact production-source SHA `ddf9d691781e648bb19d98891f3fc9b64086f5e0`, XY-C1 [43422704135 / job 131876356971] completed SUCCESS. All staged-header checks, MIPS/uClibc build, TRX verification and artifact upload steps passed. Output: `trunk/images/TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261113.trx` (~13 MiB); artifact [14035616020], 12,576,776 bytes. d9 only changes the Python checker, leaving the production source identical to ddf9.
- The ddf9 11-run set had 10 SUCCESS and one WAN route checker failure; all other exact-head runs succeeded: VPN [43422704132], WAN teardown [43422704188], AI docs [43422704130], miniupnpd [43422704284], HTTPD [43422704298], WAN DHCP callback [43422704133], detect_link [43422703988], detect_internet [43422704066], WAN ARP socket [43422704025], and XY-C1 [43422704135]. The route checker failure was corrected and its d9 rerun is green. XY-C1 logs still contain ancillary non-fatal messages (groff HTML DESC missing, several curl 404s, and objcopy/strip "no sections" for empty zerotier-one/ddnsto files); the workflow still built, verified and uploaded the TRX successfully.
- Still open: transactional batch rollback in `config_soft_bridges_other()`; DHCPv6 callback child supervision in `net_wan6.c`; then continue whole-repository governance. LAN rollback intentionally does not claim global atomicity: `control_if_ipv6()` side effects and possible partial `brport_set_m2u()` sysfs writes are not restored yet; existing VLAN MAC/priority and pre-existing WAN bridge MAC/MTU/STP values are not fully snapshotted.
- Verification boundary: no complete local checkout or local host test execution was available. Host tests/contracts are evidenced by exact-head Actions. MIPS/uClibc XY-C1 TRX build/verification/upload passed on ddf9 production source; latest d9 changes only its checker. L4 physical-router flash/boot, LAN/WAN and multi-bridge topology remain NOT VERIFIED.
- Root log count: 112→113/120; no archive rotation or INDEX update is due.
- Resulting production-source checkpoint: `ddf9d691781e648bb19d98891f3fc9b64086f5e0`; final checker checkpoint: `d9ae3f12cf24e37402cfaa6293f79a1856169061`. Documentation checkpoint `6c47c43405a71ccf7f1d6ee9fd128ff1ca77293c` refreshed handoff, roadmap and this log without changing production source; this follow-up only clarifies the source-versus-doc SHA boundary.

---

## 2026-11-13 19:48 CST (UTC+8) — Implement ownership-aware WAN soft-bridge rollback phase 1

- Recovery / code baseline: `98079bdb24d4191318ee95ca966835a8a723be95`; all 15 exact-head Actions passed there, including WAN and XY-C1 TRX build.
- This round's source commits, all on the existing `main`: `ea38b3efc0de62580feed33bf589e1f989fb69a1` adds tracked VLAN creation and bridge-port membership plus `config_soft_bridge_wan()` compensation; `36c7a7984d2c2ca57f1f5169860faddc723d55f5` adds sanitizer tests and source contracts; `fcac7ee169e99393cd3864805dedb76e70ab133f` aligns the WAN/Wireless source contracts and initializes rollback names; `44b0353cd3fc7bfd582e055cc768373819c6c0d2` routes final bridge activation failure through rollback and fixes the Linux 4.4 race-policy comment placement.
- Ownership is conservative: `create_vlan_iface_tracked()` reports only a fully created VLAN from this invocation; `br_add_del_if_tracked()` reports only an actual successful membership transition, not pre-existing/idempotent or race-completed membership. Rollback detaches owned ports in reverse order, removes only safe owned VLAN/bridge objects, restores observed UP/DOWN state for pre-existing interfaces, preserves primary error/errno, and logs cleanup failures. Ambiguous concurrent ownership intentionally causes preservation rather than destructive cleanup.
- Added regression cases for ownership decisions; WAN route workflow compiles/runs the policy binary under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. Source contracts assert tracking publication, rollback ordering, uncertainty guards and final activation cleanup. The same policy test is exercised by Wireless radio regression.
- Intermediate check run for `36c7...` first failed two static source contracts (old helper names); the test binary itself printed PASS. Contract references were updated in `fcac...`, and exact-head WAN route/Wireless Actions passed. Final activation cleanup was then found in review and fixed in `44b...`.
- Exact latest code SHA `44b0353cd3fc7bfd582e055cc768373819c6c0d2`: 11/12 push Actions SUCCESS with matching `head_sha` (WAN route [43419600585], Wireless [43419600810], WAN teardown [43419600755], WAN DHCP callback [43419600832], HTTPD [43419600791], AI handoff [43419600775], VPN [43419600653], ARP probe [43419600787], detect-link [43419600637], detect-internet [43419600678], miniupnpd [43419600860]). XY-C1 run [43419600758] / job [131865973911] was still in `Build XY-C1`; its TRX verification/upload remains unverified at this log checkpoint.
- Still open: LAN bridge handoff compensation in `config_soft_bridge_lan()`; cross-bridge/secondary VLAN batch rollback in `config_soft_bridges_other()`; DHCPv6 callback process supervision in `net_wan6.c`. Return to the wider whole-repo stability/security queue after these bounded networking items.
- Verification boundary: no full local checkout or local test run was available. Host ASan/UBSan and source contracts were exercised by GitHub Actions. No current-head L3 pass is claimed while XY-C1 is pending; L4 physical router behavior is NOT VERIFIED. The helper's phase-1 rollback does not yet restore all pre-existing bridge attributes (MAC/MTU/STP) or IPv6 side effects.
- Root log count: 111→112/120 threshold; no archive rotation, INDEX change or unrelated docs-only commit is due.
- Resulting code checkpoint: `44b0353cd3fc7bfd582e055cc768373819c6c0d2`; handoff/roadmap are refreshed in THIS documentation checkpoint.

---

## 2026-11-13 18:55 CST (UTC+8) — Repair WAN VLAN compile and regression contracts

- Base main SHA: `38b05ec60d650a8d88074c1c3166678bd7436ba4`; production source baseline commit `28f6b172796659242e0c4d6f093b85f8d6e35dce`.
- The exact-head XY-C1 run [43412016388] / job [131840501374] reached `net_wan.c` and failed on duplicate declarations of `ret` in `config_vinet_wan()` and `launch_viptv_wan()`. Removed the accidental standalone declarations; each function retains its existing grouped declaration.
- WAN route run [43412016100] failed because the IPTV publication checker compared against the first NVRAM publication, which is the valid empty-name branch for a shared VLAN. Scoped the assertion to the distinct-VLAN setup path and require VLAN, optional bridge and `rp_filter` status checks before publishing its interface name.
- WAN teardown run [43412016822] failed because the checker expected `config_soft_bridge_lan();` although production correctly gates `if (config_soft_bridge_lan() != 0)`. Updated the contract to verify that failure return precedes `start_wan()`.
- WAN DHCP callback run [43412016122] passed its preceding policy binaries but failed a source assertion: the workflow contained a double-escaped newline, searching for literal `\\n` instead of the C source's line break. Replaced it with an explicit `chr(10)` signature check for all five private helpers.
- Updated the live handoff and roadmap to prioritize exact-head WAN regression/build validation, then partial bridge rollback and DHCPv6 callback lifecycle. Promoted the preceding VLAN/bridge milestone into the proper newest-first root-log format; no archive rotation is due.
- Local full checkout and local test execution are unavailable in this session. New exact-main WAN host Actions, AI-doc validation and XY-C1 build are the required evidence; do not claim them before inspecting their logs.
- Resulting commit: THIS COMMIT.
- Next: inspect all relevant push Actions against the resulting main SHA; repair any new failure on main before starting bridge rollback work.

---

## 2026-11-13 18:45 CST (UTC+8) — WAN/LAN VLAN 与软件桥错误传播

- Code checkpoint: `12d100d1d0d0972e708644064f8541ec25418c79` — fix(wan): propagate VLAN and bridge setup failures.
- 将 `create_vlan_iface`、`remove_vlan_iface` 及 bridge sysfs 参数写入改为返回状态；新建 VLAN 后续配置失败时仅回滚本次新建接口，并保留原始错误状态。
- 将 LAN/WAN/IPTV 软桥及附加 LAN bridge 的 VLAN 创建、bridge 参数、端口加入、隔离和接口启用错误向调用者传播；主 WAN 配置失败不再继续报告成功，IPTV 接口名在配置成功后才发布。
- 加入 `scripts/check_wan_vlan_setup.py` 并挂入 WAN host workflow，覆盖声明、状态传递和关键调用顺序。
- CI: WAN route operation status regression: completed/failure (43411514756). 完整固件编译和实机验证未执行。
- 接续方向：审查 bridge 创建后部分失败的清理策略；随后治理 DHCPv6 callback 生命周期。

---

## 2026-11-13 17:27 CST (UTC+8) — Trace remaining FRP signal, readiness and paired-config rollback gaps

- Base main SHA: `252f328f48b8e4ee6bf8b9bfdc18ab8bd574311f`. Its five triggered push workflows all completed SUCCESS with matching checkout SHA and full logs inspected: AI docs [43400944647 / 131803346333], AliyunDrive [43400944593 / 131803346122], AliDDNS [43400944683 / 131803346094], AdGuardHome [43400944684 / 131803346971], infosvr [43400945908 / 131803349718].
- A deeper source pass found that launcher flock serialization is not a complete process-ownership proof: `frp_close()` still uses `pidof` and `killall` by comm name, and `frp_script.sh` has two additional broad `killall frpc frps` call sites. The script backgrounds daemons and `frp_start()` does not fail when an enabled process never becomes visible; a bounded readiness contract is still needed.
- The config script stages both enabled configs before publishing, but performs separate client/server renames. A failure on the second rename can leave the first file replaced; existing tests inject a per-file rename failure but do not cover second-rename rollback of the pair. Add all-or-rollback coverage before considering the config transaction closed.
- Updated the handoff and roadmap to make these three issues the next bounded FRP task, followed by WAN bridge/VLAN, Wi-Fi bridge detach, DHCPv6 and service supervision. Corrected the prior 17:10 dev-log record's AI docs job reference: run [43399117134] job is [131797242686], not the final-SHA job ID.
- Root dev-log count: 108→109, below 120 records and 384 KiB target; archive inventory remains 7/22/18, no rotation/index update. No production source changed in this checkpoint. The exact-head AI docs workflow for this new documentation tree must pass before this checkpoint is considered fully validated.
- No local host test, MIPS/uClibc build, firmware/TRX build or router test is claimed.

---

## 2026-11-13 17:20 CST (UTC+8) — Refresh FRP handoff and roadmap after serialized lifecycle verification

- Recovery followed `AI_BOOTSTRAP_PROMPT.md` and `AI_DEVELOPMENT_PROTOCOL.md` in prescribed order. Re-read current handoff, roadmap, newest 20 root log entries and archive index; recovery HEAD was `b45a624f70e18e45415256092b3c6e6aa978f0f5`, current FRP source/test checkpoint is `7787210e670b74184d0326f1e8e68efa9063c65c`.
- FRP exact-head push [43399797821] / job [131799522268] and AI handoff [43399798215] / job [131799523616] both SUCCESS on the exact code checkpoint; all job steps/full logs inspected. FRP lifecycle lock, async restart, cron, boot, private config, bounded stop and source/workflow tests pass.
- Updated the active handoff to mark only the bounded serialization slice L2 closed and set the next task to exact PID/argv identity, bounded startup readiness and process-specific stop outcomes. Updated FRP roadmap status and promoted remaining whole-tree WAN bridge/VLAN, Wi-Fi detach, DHCPv6 and service-supervision work after that bounded slice.
- Root dev-log adds four records in total (this checkpoint and the three code/test commits): 104→108, under the 120-record rotation and 384 KiB early-rotation thresholds. Archive inventories remain dev-log=7, handoff=22, roadmap=18; no archive/index mutation.
- No source code is changed in this documentation checkpoint. The exact-head AI documentation workflow for the resulting tree must validate the updated handoff, roadmap and root log. No local test, full firmware/TRX build or hardware test is claimed.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-13 17:15 CST (UTC+8) — Assert post-action lock release and reject inherited FRP FD 9

- Base main SHA: `1802e0a050b65cf78f25bc979d490e71cf6df359`.
- The exact-head 1802 FRP regression passed shell syntax and every host fixture, but the semantic checker selected the earlier FD-9 close in the contention-timeout branch when ordering the post-action unlock. This was a checker-ordering defect, not a production lock failure.
- Changed the source contract to compare against the final `exec 9>&-` after the locked action; extended the host regression to pre-open FD 9 and require the launcher to reject it without running the action.
- On exact main `7787210e670b74184d0326f1e8e68efa9063c65c`, FRP [43399797821] / job [131799522268] and AI docs [43399798215] / job [131799523616] both SUCCESS. Every checkout SHA matched, all job steps and full logs were inspected. FRP output confirms stable inode, unsafe path, bounded contention, status propagation, inherited-FD and semantic contract PASS; AI docs reports 104/119 root entries and archives 7/22/18.
- Changed paths: `scripts/check_frp_lifecycle.py` and `trunk/user/frp/tests/frp_lock.sh`; no production behavior change. No firmware/TRX or hardware validation is claimed.

---

## 2026-11-13 17:10 CST (UTC+8) — Repair FRP lifecycle-lock fixture output and RC restart contracts

- Base main SHA: `dccfed4cb318c6babc11da1af7675f5173251658`.
- The dcc exact-head FRP workflow [43398856615] failed in the new lock regression because over-escaped printf newlines produced a literal backslash-n in the fixture's lock path/output; all earlier FRP shell regressions passed. This was a test-fixture defect.
- Corrected shell printf strings to emit real newlines and aligned the duplicate C RC checker with the single serialized `frp.sh restart` dispatcher. Production process-control behavior was not changed by this correction commit.
- On exact main `1802e0a050b65cf78f25bc979d490e71cf6df359`, AI docs [43399117134] / job [131797242686] SUCCESS and FRP [43399116749] reached the final source checker after all host tests passed; the checker then failed on its first-close ordering assertion, corrected by the following commit. Both exact checkouts and all FRP job steps were inspected.
- Changed paths: `trunk/user/frp/frp.sh`, `trunk/user/frp/tests/frp_lock.sh`, and `scripts/check_frp_lifecycle.py`. No full firmware or router test is claimed.

---

## 2026-11-13 17:08 CST (UTC+8) — Serialize FRP lifecycle operations across RC, cron and watchdog

- Base main SHA: `b45a624f70e18e45415256092b3c6e6aa978f0f5`. Auditing the complete UI/HTTPD/PID1/RC/boot/cron path found that watchdog check, start, stop and RC/UI restart were separate uncoordinated launcher processes; the RC restart released state between stop and start and crontab transactions could overlap.
- Added a stable private `/tmp/.frp.lifecycle.lock` inode with owner/mode/link/type/path checks, bounded flock, under-lock NVRAM refresh and a common start/stop/restart/watchdog dispatcher. PID1 RC restart now calls one `restart` action; the custom config script closes FD 9 before launching long-lived daemons. Added a dedicated lock regression and workflow/source-contract coverage.
- Exact dcc source SHA `dccfed4cb318c6babc11da1af7675f5173251658` triggered 18 push workflows: 17 completed SUCCESS; FRP [43398856615] failed in the new lock fixture because of escaped newline/path output. Later exact-head commits corrected the fixture and checker; this failure is not represented as a passing test.
- Changed paths: `trunk/user/frp/frp.sh`, `trunk/user/rc/src/services.c`, `trunk/user/frp/tests/frp_lock.sh`, `scripts/check_frp_lifecycle.py`, and `.github/workflows/frp-lifecycle.yml`. L2 is claimed only for the final exact SHA 7787210e670b74184d0326f1e8e68efa9063c65c; no full firmware/TRX or router validation.

---

## 2026-11-13 15:38 CST (UTC+8) — Normalize FRP HTTPD event-map source contract

- Base main SHA: `bec830e987c210f2646cb70525159b314e8c6350`.
- Exact-head FRP push [43386702735] / job [131756753371] checked out this SHA. Launcher syntax, asynchronous watchdog regression, managed-crontab regression, boot-script syntax, boot-role regression and all shell test steps passed; the final semantic checker failed on one HTTPD event-map assertion.
- Root cause: `variables.c` formats `{EVM_RESTART_FRP, EVT_RESTART_FRP, RCN_RESTART_FRP, 0}` with tab-separated whitespace; the new source checker required literal single spaces. This was a brittle checker assertion, not a production or regression-test failure.
- On the same exact SHA, AI docs [43386702613] / job [131756753049] passed (103/119 root records; archives 7/22/18), and all other triggered workflows passed: AliDDNS [43386702607], infosvr [43386702665], AliyunDrive [43386702712] and AdGuardHome [43386702629]. Every run's checkout SHA and every job step/full log were inspected.
- THIS COMMIT normalizes whitespace in both the actual HTTPD source and expected markers before checking the FRP persistence/event mapping. It does not change production code or regression behavior. The previous handoff's adjacent verification bullets are also separated correctly.
- Exact-head Actions after THIS COMMIT are pending until inspected. No local full checkout, local test run, full firmware build or router test is claimed.
- Root log count: 103→104 records; file remains below the 384 KiB target and 512 KiB hard limit; archive inventory remains 7/22/18; no rotation/index change.
- Changed paths: `scripts/check_frp_lifecycle.py`, `SESSION_HANDOFF.md`, and this log. Resulting main commit: THIS COMMIT.
- Next: verify all exact-main push Actions triggered by THIS COMMIT, especially FRP source/RC/UI caller-chain and AI handoff documentation. Then continue the FRP private-config/credential-permission and cross-process lifecycle serialization audit.

---

## 2026-11-13 15:22 CST (UTC+8) — Avoid idle FRP watchdog when both roles are disabled

- Base main SHA: `c5ee1e4d8afc19a607b93eb40d00e329a63aebb7`; remote branch inventory contains only `main`; legacy PR #1 remains CLOSED/unmerged.
- Recovered the pending FRP caller-path audit. On the exact base SHA, FRP [43384819463] / job [131750709887], HTTPD security [43384819483] / job [131750710183], AliDDNS [43384819533] / job [131750710147], and AI handoff docs [43384819555] / job [131750710169] all completed SUCCESS on `push` events. Every job checked out the exact SHA and every step/full log was inspected. FRP shell syntax, delayed-PID single dispatch, managed crontab preservation/failure cleanup, client-only/server-only/both-off boot behavior and source contracts passed; AI docs reports 102/119 records and archive inventory 7/22/18.
- Traced the complete UI/HTTPD/PID1/RC/ROMFS chain: `Advanced_frp.asp` exposes both enable flags and `scripts.frp_script.sh`; `variables_FrpConf` maps them to `EVM_RESTART_FRP`; PID 1 dispatches `RCN_RESTART_FRP`; `services.c::restart_frp()` always invokes stop then start; `copyscripts.sh` seeds the persistent root-run script from the ROMFS template; the watchdog runs `/usr/bin/frp.sh C` via the root crontab.
- Found a concrete lifecycle leak: when both NVRAM role flags are `0`, an RC/UI restart still invokes `frp_start()`. The previous function nevertheless ran the editable custom script and re-added the per-minute watchdog, leaving an idle cron job with no enabled FRP daemon. THIS COMMIT routes the both-disabled case through `frp_close()` before running user code, so it removes only the exact managed watchdog and preserves operator jobs.
- Extended the no-network crontab regression to verify both-disabled start does not invoke the custom script or leave the watchdog, preserves an unrelated operator job, server-only start still executes once and publishes one watchdog, and disabling the final role removes only the managed line. Expanded the semantic checker to cover UI → HTTPD persistence → PID 1 → RC restart ordering → script seeding/launch, and added symmetric workflow path filters for each caller/template source.
- Exact-head Actions for THIS COMMIT are pending inspection. No local full checkout or local shell/Python run was available; no local test execution is claimed. Current main L3 MIPS/uClibc firmware/TRX and L4 router validation remain NOT VERIFIED.
- Root log count: 102→103 records; 256285-byte baseline is below the 384 KiB size trigger and 512 KiB hard limit. Archive inventory remains 7/22/18; no rotation/index change.
- Changed paths: `trunk/user/frp/frp.sh`, `trunk/user/frp/tests/frp_crontab.sh`, `scripts/check_frp_lifecycle.py`, `.github/workflows/frp-lifecycle.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and this log.
- Resulting main commit: THIS COMMIT.
- Next: inspect every exact-main `push` Action triggered by THIS COMMIT (FRP and AI docs at minimum); verify each run's `head_sha`, checkout, all steps and complete logs. Then continue FRP P2 audit on private config/token storage and lifecycle serialization/stop postconditions, without claiming target behavior from host results.

---

## 2026-11-13 14:24 CST (UTC+8) — Close Adbyby configured hosts-source download hardening

- Recovered the live main and handoff after the configured /etc/storage/adbyby_host.sh task had already landed. Current validated source/test SHA is 1994f29e9e8180131011fb811bd9f6402e52523f; production implementation is 76f514cb5111c6787c4a6eda325619062845bace, and the tip commit corrects only the regression fixture's custom dnsmasq path.
- Inspected the failed first exact-source Adbyby run [43377852791] / job [131728447338]: shell syntax and prior tests passed, while the no-cache cleanup assertion used hard-coded /tmp/adbyby/hosts although the helper correctly received the temporary $PROG_PATH/hosts. The fixture correction in 1994f29e9e8180131011fb811bd9f6402e52523f fixes the assertion without changing production behavior.
- On exact main SHA 1994f29e9e8180131011fb811bd9f6402e52523f, all five triggered workflows completed SUCCESS: Adbyby [43378151993] / job [131729407848], AI handoff docs [43378151976] / job [131729407933], AliyunDrive [43378151941] / job [131729407811], AdGuardHome [43378151961] / job [131729407913], and AliDDNS [43378151950] / job [131729407894]. Every job fetched/checked out this exact SHA, every job step succeeded, and the full logs were inspected. Adbyby shell syntax, all seven regression scripts, configured host-source HTTPS/format/aggregate/cache/atomic-publication test, and producer-chain/source/workflow checker all PASS. AI docs reports 101/119 root records and archive inventory 7/22/18 at this source/test checkpoint.
- L2 configured hosts-source download is CLOSED. The implementation removes the insecure curl -k, URL word-splitting and shared /tmp/host.txt path; uses quoted bounded firmware-CA HTTPS for every configured source; validates hosts IPv4/IPv6 mappings, DNS labels, record size and aggregate size; stages privately on the target filesystem; atomically publishes the deduplicated complete file; and preserves validated last-good data or fails closed.
- Changed documents in this follow-up: SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. The next bounded task is the FRP full RC/UI/cron/shared-script audit. FTP failed-upload cleanup remains OPEN/BLOCKED on Linux 4.4 final-name TOCTOU; NFS requires real-client evidence; current-main L3 MIPS/uClibc firmware/TRX and L4 router remain NOT VERIFIED.
- The execution environment still has no usable full local repository checkout (GitHub DNS resolution failed); no local shell/Python test is claimed. Network requests were mocked in regression tests; no real external download, full firmware/TRX build or router test was run.
- Root dev-log count: 101 before this record and 102 afterward; still below 120-entry / 384 KiB early-rotation / 512 KiB hard limits. Archive/index inventory remains dev-log=7, handoff=22, roadmap=18; no rotation or index edit is due.
- Exact-head AI docs validation for this handoff/roadmap/log update is the next required action; require the resulting documentation SHA and 102/119 root records.
- Resulting main commit: THIS COMMIT.

---


## 2026-11-13 14:21:06 CST (UTC+8) — Correct Adbyby host-list test fixture path

- Follow-up to implementation commit `76f514cb5111c6787c4a6eda325619062845bace`. The exact-head Adbyby workflow run [43377852791] / job [131728447338] executed shell syntax and all prior regressions successfully, then the new test failed at the no-cache cleanup assertion.
- Root cause was a test-fixture path mismatch, not the production cleanup: `hosts_ads` correctly passed its temporary `$PROG_PATH/hosts` to the existing exact-line cleanup helper, while the fixture hard-coded `addn-hosts=/tmp/adbyby/hosts`. The helper therefore had no matching line to remove. The regression now writes and asserts the actual temporary path, preserving the same production code and exercising the real cleanup contract.
- Corrected the checker path tuple's missing comma before the initial commit was published; the committed checker already contains the separated path entries. The first exact-head AI handoff validator passed at run [43377852866] / job [131728447504], root log=100/119 and archive counts 7/22/18.
- This is a test correction only. No production behavior is changed in this follow-up. Other exact-head Actions from the first commit were still running when inspected; they must be checked again against their own exact SHA before closing the slice.
- Local checkout remains unavailable due to this execution environment's DNS failure resolving github.com. No local shell/Python result is claimed; no firmware build or router test was run.
- Root log now has 101/119 records; no rotation is due. The Adbyby host-list regression must be rerun on this commit SHA. If it passes, continue to check every exact-head workflow and then update the live handoff/roadmap to the next FRP RC/UI/cron/shared-script audit.
- Resulting main commit: THIS COMMIT.

---

