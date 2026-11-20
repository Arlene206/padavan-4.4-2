# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; authorized branch main. Live main at recovery: 0d9719e7960f83d26a2df931902f61bee5b94bfe. Frozen archival branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and immutable.
- Legacy PR #1 is closed/unmerged; all relevant AliDDNS CA bootstrap, HMAC stdin helper, tests and workflow payload is already on main.
- MiniUPnP source fixes include OOM-safe rule snapshots, lease-key matching and persistence error reporting, NAT/filter rollback, IPTC ownership, metadata staging and shared fatal-event-loop teardown.
- Commit 7065465d000b0bc17763b51c61533cebba243f62 removed a build-blocking duplicate static int specifier from upnpredirect.c::lease_file_remove(). Its initial declaration-contract assertion over-escaped the newline; exact-head MiniUPnP run #44799072749 FAILED at that checker assertion. THIS COMMIT replaces the regex with a byte-exact declaration-prefix check.
- Exact-head XY-C1 run #44799072655 on 7065465d000b0bc17763b51c61533cebba243f62 was in progress at last check and is expected to provide Build/TRX/artifact evidence for the production source fix, including the cumulative infosvr production source. No L3 conclusion until the run finishes successfully.
- The XY-C1 workflow path filters now include the infosvr production sources and host tests (added in b90cdf4f6cd8ce0725bdee19dc7b605c0163a775). The exact-head infosvr regression on 3358e551a0ca512324a00196a723bec9e33127d2 FAILED because its new test incorrectly asserted that the extended response MacAddress field is zero; the field overlaps the first six bytes of PKT_GET_INFO.PrinterInfo in the legacy payload layout. Correct the test against the actual protocol layout.
- Main-only non-forced CAS remains mandatory. Hardware L4 remains NOT VERIFIED.

## Active task

1. Repair the infosvr regression at trunk/user/infosvr/src/tests/infosvr_dispatch_test.c: seed/reject duplicate unsupported opcode requests as intended, then assert the response layout correctly (the extended header MacAddress overlays the beginning of PrinterInfo because the payload starts at sizeof(IBOX_COMM_PKT_RES)). Do not change production protocol bytes unless a separate compatibility review proves a real defect. Rerun the exact infosvr Action.
2. Inspect the result of XY-C1 #44799072655 on 7065465d000b0bc17763b51c61533cebba243f62; require the complete Build, TRX verification and artifact upload to pass. The old #44798143782 PASS on SHA 545d8ca3ddbe85e95271ed5e21a3990e81f2bc76 is only a baseline, not a substitute.
3. Continue the MiniUPnP lease/firewall transaction audit in upnp_redirect() and _upnp_delete_redir(). Read callers and existing rollback helpers before changing ordering; add injectable GNU89 tests for all error edges.
4. Continue cross-userland stability/security work, avoiding already completed NAT/filter, OOM snapshot, lease-key and IPTC ownership work.

## Verification boundary

- MiniUPnP correction commit 7065465d000b0bc17763b51c61533cebba243f62: production declaration corrected; initial exact-head MiniUPnP #44799072749 FAILED only at the malformed Python regex and has been diagnosed. THIS COMMIT corrects the checker implementation.
- XY-C1 #44799072655 on SHA 7065465d000b0bc17763b51c61533cebba243f62 was IN_PROGRESS at last observation. No current-source L3 claim until Build, TRX verification and artifact upload all succeed.
- Infosvr #44799216221 on 3358e551a0ca512324a00196a723bec9e33127d2 FAILED only at the new test's incorrect zero-MAC assertion; logs show preceding packet-policy ASan/UBSan test passed. A test-only correction is required and a new exact-head run must pass.
- MIPS/uClibc runtime outside the full image and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect the exact-head MiniUPnP rerun for THIS COMMIT; confirm the declaration assertion and all existing lease/persistence contracts pass.
2. Fix the infosvr dispatch regression using the actual IBOX_COMM_PKT_RES_EX / IBOX_COMM_PKT_RES / PKT_GET_INFO layout, then rerun its host suite and exact-head Actions.
3. Inspect XY-C1 #44799072655 for final Build/TRX/artifact results. If failed, diagnose its first causal error and repair on main.
4. Review upnp_redirect() and _upnp_delete_redir() for consistency between live firewall state and persisted lease state on partial failure. Preserve GNU89/MIPS/uClibc compatibility and commit one logical fix at a time.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
