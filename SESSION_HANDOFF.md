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

- Parent `main` for the peer-SNAT handle-order correction: `721857934de255342b4cdb66e08155aacc65acc2`. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- NAT/filter update rollback `bfc1c166` / checker correction `ea745da6`: MiniUPnP #44792212170 PASS; AI #44792212195 PASS; XY-C1 #44791960769 on source SHA bfc1c16 PASS for Build, TRX and artifact upload.
- NAT/filter delete compensation `b7a803d`: MiniUPnP #44793663690 PASS; AI #44793663716 PASS; process inventory/regression #44793663655/#44793663596 PASS.
- Peer SNAT/DSCP compensation `721857934`: new exact-head MiniUPnP #44794249501 FAILED only the ownership source contract because `peer_delete_status = r2` was placed between the consuming helper call and `h = NULL`; correction is in this slice. XY-C1 #44794249117 and NVRAM #44794250260 were pending at last poll.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Finish exact ownership ordering for the peer SNAT deletion helper:

- `delete_rule_and_commit()` consumes and frees the by-value `IPTC_HANDLE`.
- Set the caller's `h = NULL` immediately after the peer SNAT helper call; only then copy the scalar result into `peer_delete_status`.
- Keep the regression contract strict across every delete helper call site. Do not weaken the test to accommodate an ownership gap.

After the MiniUPnP rerun passes, continue with the independently confirmed redirect-description/finite-lease metadata OOM defect: stage descriptor allocation before NAT updates and publish only after success.

## Verification boundary

- NAT/filter deletion compensation #44793663690 and peer workflow predecessor checks passed before the peer SNAT extension.
- Peer extension MiniUPnP #44794249501: FAILED at step 9, `Verify MiniUPnP IPTC delete-handle ownership`; all earlier test binaries and allocation source checks passed. Root cause and one-line source-order correction are recorded above.
- Exact-head MiniUPnP rerun required after this fix. XY-C1 #44794249117 and NVRAM #44794250260 were PENDING at last poll. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head MiniUPnP Actions for this peer handle-order correction; require the ownership checker and all preceding ASan/UBSan regressions to pass.
2. Check XY-C1 Build/TRX/artifact and NVRAM audit for their exact SHAs; report host-only/firmware levels accurately.
3. Start the redirect metadata allocation/OOM slice. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
