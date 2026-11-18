# 2026-11-18 continuation checkpoint — Adbyby managed rule-pair recovery

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Current live main SHA before this checkpoint: `6836f3c9899466ac3cb025a6c4572922ca1d7fd2`. Adbyby product-source slice: `c957987ad9971cb3f029dfb6a054d6367d059fa0`; checker-only correction: `7e12500ebb45e32611cc0db7dc37e6f80f49e7f4` followed by `141bf15edf2d35f950e04c4d267ad8bd140795d4`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Active task
- Adbyby's managed `lazy.txt` / `video.txt` update previously used two atomic renames and rolled the first back if the second failed, but an abrupt shell termination between renames could leave a mixed rule generation.
- This slice adds a private same-filesystem transaction journal containing both old file states and backups before either live rename. Recovery validates the marker and private stage, restores both files idempotently, then removes the marker. Refresh runs recovery before inspecting cache digests; the EXIT trap attempts recovery on ordinary error/termination and preserves journal files if restoration cannot complete.
- Regression coverage includes second-rename failure, simulated interruption after publishing only lazy.txt, restoring an originally absent file, symlink/unsafe stage rejection, and incomplete-backup fail-closed/retry (marker and live pair are retained until the missing backup is repaired).
- The product implementation is in `c957987ad9971cb3f029dfb6a054d6367d059fa0`: `.rules-transaction` records the private `/tmp/adbyby-rules.*` stage path, and per-file `.state` / `.previous` records allow idempotent restore. The marker is validated for owner, mode, link count and strict stage-path form; the stage must be owner-controlled mode 700. The EXIT trap recovers on ordinary termination; a later refresh recovers after SIGKILL before digest/cache use.
- Checker-only commits `7e12500` and `141bf15` aligned the static contract with loop-generated `adbyby_rule_name.previous` / `.state` files. Test-fixture newline formatting was corrected in `f08f741`/`7615181`. The NVRAM error-propagation change in `0c102bc` is pinned by `6836f3c`; exact-head Adbyby host regression `44313402338` on `6836f3c9899466ac3cb025a6c4572922ca1d7fd2` PASSes.
- The NVRAM lifecycle correction in `0c102bc` makes non-stop actions fail closed on `nvram get` or transient `nvram set` errors. `stop` remains a safety cleanup: it always calls `adbyby_close`, logs both failures if teardown also fails, and returns the earlier NVRAM preparation status after teardown. Injected host cases cover write status 37, read status 39 and verify teardown still runs.
- The corrected source checker now pins the exact closing bracket in the stop branch and requires those three injected failure assertions. `6836f3c` added these checker/test contracts; its exact-host workflow is PASS.
- Managed-rule journal recovery handles process interruption while `/tmp` persists, not power-loss/reboot durability. No local shell run was performed in this connector-only session.

## Verification boundary
- Exact main SHA `6836f3c9899466ac3cb025a6c4572922ca1d7fd2`: Adbyby workflow `44313402338` PASS. It verifies the corrected stop-branch checker condition and injected NVRAM start/stop failure cases. The preceding source SHA `0c102bce73f5905713e496da2d1dfbbccc97a0fa` has Process fail-closed `44313181302`, Process inventory `44313181361`, and HTTPD security `44313181286` PASS. Its Adbyby `44313181284` failed only on a malformed checker string missing `]`; `6836f3c` fixed the checker literal and the Adbyby rerun passed.
- The NVRAM production source SHA is `0c102bce73f5905713e496da2d1dfbbccc97a0fa`; XY-C1 run `44313181329` is currently IN_PROGRESS at `Build XY-C1` with TRX verification and artifact upload PENDING. Do not mark current L3 PASS until all three steps succeed on this source SHA.
- Earlier product-source SHA `c957987ad9971cb3f029dfb6a054d6367d059fa0` XY-C1 run `44310230672` completed Build, TRX verification and artifact upload PASS. It predates the NVRAM production change and does not substitute for `0c102bc`.
- Earlier exact main SHA `7615181443ed02a919c702816c719f8048eb2499` Adbyby `44312347134`, Process inventory umbrella `44312347045`, Process fail-closed `44312347214`, and AI docs `44312347101` all PASS, including the managed journal negative-path/retry cases.
- Local checkout-based execution was NOT RUN. Target BusyBox/uClibc/MIPS runtime and physical-router L4 remain NOT VERIFIED.
- Prior unrelated source SHA `38a2e1b80c78d250c892377647a43a454623228b` XY-C1 `44307638792` succeeded, but that does not verify this Adbyby source change.
- Local checkout-based execution was NOT RUN. Target BusyBox/uClibc/MIPS runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Poll XY-C1 run `44313181329` for production source SHA `0c102bce73f5905713e496da2d1dfbbccc97a0fa`; inspect Build, TRX and artifact steps and fix any failure. Mark L3 only when all three succeed on that source SHA.
2. The new stop-path NVRAM source and checker host test are green (`6836f3c` Adbyby `44313402338`). Continue to the next risk-ranked stability issue in `REFACTOR_ROADMAP.md`; do not repeat Adbyby or Shadowsocks audits.
3. Preserve the frozen backup and use atomic complete-tree commits with expected-SHA non-forced updates.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the handoff untouched and do not create a no-op commit.

## Cross-session execution mandate
Use small source/test slices, one complete Git tree and one atomic main commit per logical task. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, change the frozen backup or claim validation not run on the same SHA.
