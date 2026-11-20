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

- Parent `main` for this atomic slice: `c5e41697bfeb593223a8c7c46b9e1e972fdc7be9`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and read-only.
- MiniUPnP allocation-safety source fix `2ffb8479a33ad55a4138b3384a31510f5b7c534e` passed exact-head MiniUPnP run `44789878471` on `8dbb0963cb6b816bf13cf13726f5de0ce3c867fb`, including the GNU89 ASan/UBSan allocation-cleanup test and normalized source contract.
- XY-C1 Firmware Validation `44790573472` was IN_PROGRESS on parent SHA `c5e41697bfeb593223a8c7c46b9e1e972fdc7be9` at this checkpoint; Build/TRX/artifact completion must be inspected on the new source SHA before claiming L3.
- Confirmed additional IPTC lifetime defect: the initial NAT deletion path did not clear its caller-side handle after `delete_rule_and_commit()` freed it. The filter, peer NAT and DSCP call sites already clear their local copies. This slice fixes the missing ownership marker and adds an exact four-callsite source contract.
- MIPS/uClibc target runtime and physical-router L4 remain NOT VERIFIED.

## Active task

Close the MiniUPnP IPTC handle double-free path and continue the netfilter transaction audit:

- `delete_rule_and_commit()` consumes and releases its `IPTC_HANDLE` on every result. The initial NAT deletion branch previously retained its stale local handle and could call `iptc_free()` again after deletion or commit failure.
- Clear the local handle immediately after the helper call, matching the already-correct filter, peer-NAT and DSCP paths.
- The focused Python source contract checks all four call sites and verifies that the helper closes the handle for both IPTABLES API variants. MiniUPnP push/PR path filters include the checker.
- After exact-head validation, continue with the partially committed NAT/MASQUERADE and cross-table rule installation paths. Do not repeat the completed DHCP leasefail/NAK/release or AP restore audit.

## Verification boundary

- MiniUPnP allocation-safety baseline `2ffb847`: exact-head host sanitizer/source-contract workflow PASS on `8dbb096` (`44789878471`).
- IPTC deletion ownership source fix and four-callsite contract are included in this atomic slice; exact-head MiniUPnP and AI documentation checks must pass.
- XY-C1 run `44790573472` on parent `c5e41697bfeb593223a8c7c46b9e1e972fdc7be9` was still running at the last check. This change touches `iptcrdr.c`, which is in the XY-C1 path filters; the new exact-head Build, TRX and artifact upload are required for L3.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, confirm the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, then inspect exact-head MiniUPnP, AI handoff and XY-C1 Actions for this commit. Require Build, TRX verification and artifact upload before claiming L3.
2. If those checks pass, continue the netfilter partial-install audit at `add_redirect_rule2()`: establish the cleanup/rollback behavior when DNAT succeeds but optional MASQUERADE fails, without deleting unrelated peer/filter rules.
3. Keep MIPS/uClibc runtime and physical-router L4 explicitly unverified until tested on target hardware.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
