# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken checks.
- PENDING Actions do not block independent work. Diagnose earliest causal failure logs.
- Finish a safe slice before context exhaustion and preserve exact next action. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live main, verify frozen backup SHA and check exact-SHA Actions at session end. Update this file only when source baseline, test evidence or first next action materially changes. Do not make no-op commits.

## Current source and test state

- Checkpoint parent main SHA: bb4a1910acd73c2ae9433d66971bdcd4313d4f23; latest product-source commit touching iptcrdr.c: 0b5eef8bd00fd6e732449bcf241949df84b4f40c; latest errno-runner path commit: c492d33b10baf96d8a971c4adc6773b3df5808f3.
- Frozen backup branch SHA: 72f4f157a084f8184b9597ca5bf334b8080e5807 (unchanged).
- Exact-head main Actions: MiniUPnP 44810270387:completed/failure; errno harness step completed/success; AI docs 44810270268:completed/failure; Process inventory not observed; NVRAM not observed.
- Product-source XY-C1 run for 0b5eef8bd00fd6e732449bcf241949df84b4f40c: 44809783977:in_progress/pending; steps: Build XY-C1=in_progress/pending; Verify TRX output=pending/pending; Run actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9=pending/pending. L3 is verified only if Build, TRX and artifact upload all passed on this source SHA.

- Source fix: delete_redirect_and_filter_rules_ex() now uses miniupnpd_record_first_failure_errno() for every primary and secondary errno assignment, so subsequent filter/mangle failures cannot overwrite an earlier causal error. The selector returns primary error before independent peer cleanup error.
- Policy regression covers first-error retention, primary precedence, secondary fallback and EIO fallback. Standalone and inline contracts reject any direct errno assignment bypassing the policy.
- scripts/test_miniupnpd_iptc_errno.py extracts the actual production delete_rule_and_commit() and insert_rule_and_commit() functions and builds both legacy pointer-to-handle and IPTABLES_143 by-value APIs. Injected iptc_strerror(), syslog() and iptc_free() deliberately clobber errno; cases cover delete ENOSPC/commit EROFS/no errno, insert init EACCES/operation EINVAL/commit EROFS. The generated C preamble includes syslog.h for LOG_ERR.

## Active task

1. Inspect this main SHA's exact MiniUPnP and AI Actions. Confirm the errno helper harness compiles/runs both API variants under GNU89/Werror/ASan/UBSan; confirm the source checker rejects unguarded first-error assignments.
2. Verify the XY-C1 steps on the latest product-source commit touching iptcrdr.c. Count L3 only if Build XY-C1, Verify TRX output and artifact upload each complete SUCCESS on that identical source SHA.
3. If all host checks pass, extend injected transaction-level tests for primary/secondary rollback sequencing; otherwise diagnose the earliest failure log and correct the source/test contract without weakening assertions.
4. Continue to a separate concrete Padavan userland reliability/security issue after the MiniUPnP slice is verified; avoid repeating already-completed audits.

## Verification boundary

- Previous confirmed L3 evidence for 30af417 and eccdab540 applies only to those exact product trees.
- Current product L3 is NOT VERIFIED unless the three actual XY-C1 steps listed above have passed for latestProductSha.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and the exact-head MiniUPnP Action. Read the errno harness step output for both handle API variants and confirm the first-error source contract. Then inspect XY-C1 Build/TRX/artifact step conclusions on latestProductSha. If green, add transaction-level rollback injection; if not, fix the first causal failure. Continue with a different userland module after this slice.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at 120 records, retaining newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
