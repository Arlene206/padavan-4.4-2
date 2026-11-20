# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Complete one logical task per atomic commit: each task ends in one atomic main commit. For multi-file tasks create all blobs, one complete Git tree, one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from logs; while any check is PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Main at this checkpoint: `bf53b1c08fa068fed5e06ef11098e523be032dcf`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival and untouched.
- Latest production commit: `bffe074d853e9083d299d3ce07dd96d827a6d681`, parent `b6150ea46a084510a3c56f91646ea9ada905e72a`. It fixes ZeroTier identity-file partial publication with rollback.
- Latest account database production source is `740c82cee671e5bcc133289d9126f75afd654ec3`; descendants `775ef93`, `590bc8a`, `1e09d06`, `b6150ea` strengthen rollback regression/source contracts without changing production C.

## Active task

- Verify exact-head XY-C1 for ZeroTier production source `bffe074`; do not report L3 until Build XY-C1, TRX verification and artifact upload all pass.
- Restore the mandatory handoff checkpoint contract for the AI documentation checker. Runs `44656214060` on `b6150ea` and `44656543369` on `bffe074` both failed only because the handoff omitted the required end-of-session checkpoint heading and exact sentence. This document update adds the requirement.
- After verification, continue P1 service-result propagation in `trunk/user/rc/src/services_ex.c`. Trace a specific nonzero return discarded by a direct caller into the resulting service/boot state; add a focused executable regression or source contract before changing code. Do not revisit completed wireless bridge, USB modem reconnect, account staging/rollback, or unused-wrapper audits without a new failing case.

## Verification boundary

Exact-head checks observed:
- On parent `b6150ea46a084510a3c56f91646ea9ada905e72a`: Process lifecycle `44656213985` PASS; Process inventory `44656214119` PASS; NVRAM transaction boundary `44656214084` PASS.
- On production source `bffe074d853e9083d299d3ce07dd96d827a6d681`: ZeroTier lifecycle `44656543332` PASS; WAN route operation status `44656543283` PASS.
- AI handoff docs `44656214060` (`b6150ea`) and `44656543369` (`bffe074`) FAIL for the missing handoff checkpoint wording. This commit adds the exact required contract; rerun the validator.
- XY-C1 `44656543236` on `bffe074`: input detector PASS; Build XY-C1, TRX verification and artifact upload were IN_PROGRESS at last poll. No L3 claim for this ZeroTier source until all three succeed.
- Production-source L3 for account security/transaction SHA `740c82cee671e5bcc133289d9126f75afd654ec3`: XY-C1 run `44654005725` passed input detection, Build XY-C1, TRX verification and artifact upload.
- XY-C1 run `44656213993` on `b6150ea` skipped Build/TRX/artifact because that descendant changed only rollback tests. Do not count it as a production build.
- Physical-router L4 runtime/boot validation remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check the AI handoff documentation workflow on this commit. The previous exact failure was `current handoff does not require an end-of-session accuracy checkpoint`; preserve the required heading and exact conditional-edit sentence below.
2. Poll XY-C1 `44656543236` on `bffe074`. Claim L3 only after Build XY-C1, TRX verification and artifact upload all pass.
3. Continue the concrete service-result investigation in `services_ex.c`: establish the helper's error contract and direct callers, then write a regression for the dropped status before making a bounded source change.
4. Keep backup untouched and maintain distinct L1/L2/L3/L4 evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
