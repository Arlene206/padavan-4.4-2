# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main SHA before this source/test/checker slice: `9c75e1bf1e13da1dc673a120267827ee1e25c4a5`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; keep immutable. Only `main` and the backup branch exist.
- Exact `9c75e1bf1e13da1dc673a120267827ee1e25c4a5` checks: Process inventory #45059493600 PASS and HTTPD security #45059493548 PASS. USB Storage #45059493487 was still in progress, now containing the indentation/extraction fix from commit `9c75e1b`; at last step poll it had not reached the automount regression. XY-C1 #45059493535 was still in progress; L3 remains unverified until same-SHA Build, TRX and artifact all pass.
- The earlier storage failure #45059276804 on `d1e0edc` was caused by the embedded awk brace terminating the test's sed extraction early; exact root cause was fixed in `9c75e1b`. That ancestor failure must not be represented as the result of the current correction.
- AI handoff #45059167154 passed only on ancestor `eacdabf`; the new handoff in this commit needs its own exact-SHA AI docs run.
- No local repo checkout or router hardware is available. L4 hardware validation remains NOT VERIFIED.

## Active task

This slice hardens the global storage-unmount cleanup path. `trunk/user/libdisk/src/disk_initial.c::umount_all_storage()` stages the entire `/proc/swaps` snapshot before side effects, but `storage_inventory_parse_swap_line()` in `trunk/user/libdisk/src/storage_swap_policy.h` previously checked only that five tokens existed. Unlike the stricter per-device `storage_swap_parse_swap_line()`, it accepted a malformed type or non-decimal size, used and priority. A corrupt-but-five-column row could therefore be treated as a complete snapshot and reach the `swapoff()` loop.

The parser now validates Type as exactly `partition` or `file`, validates Size and Used as unsigned decimal fields and Priority as a signed decimal field, rejects extra columns, and continues using the octal-decoding token reader for paths. GNU89 test cases cover each invalid field, extra/truncated data and a valid path encoded with `\\040`. `scripts/check_storage_safe_remove.py` now pins these semantic checks.

The previous automount swap parser's CI extraction issue was fixed independently in `9c75e1bf1e13da1dc673a120267827ee1e25c4a5` by indenting the embedded awk program so the shell regression's sed range captures the full function. Its exact Storage run #45059493487 is still required to prove that fix.

First verification step: inspect exact-head Storage and AI-handoff Actions, then XY-C1 detector/Build/TRX/artifact on this new source SHA. Resolve any failed step using its first causal log; do not claim L3 from ancestor runs.

## Verification boundary

- Parent main before this atomic slice: `9c75e1bf1e13da1dc673a120267827ee1e25c4a5`; resulting SHA must be queried live.
- Current slice changes the global cleanup swap parser, its GNU89 regression, the storage source-contract checker and the storage roadmap/handoff/log. The workflow already watches all three code/test/checker paths for push and pull_request.
- Added unit cases reject bad Type, Size, Used and Priority, reject an extra field, retain truncated-row rejection and verify octal-escaped path decoding.
- Local tests NOT RUN (no checkout); exact-head Storage action is required for GNU89 compilation and parser regressions.
- On parent `9c75e1bf1e13da1dc673a120267827ee1e25c4a5`, Process inventory #45059493600 PASS and HTTPD security #45059493548 PASS. Storage #45059493487 and XY-C1 #45059493535 were in progress. These do not verify this new parser slice.
- L3 requires same-SHA firmware detector, Build XY-C1, TRX verification and artifact upload all PASS. L4 hardware remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`; inspect the exact-head USB storage regression and AI-handoff docs run, then the XY-C1 workflow's production detector, Build, TRX and artifact steps. Diagnose any failures from the first causal job log. After the semantic cleanup parser passes, continue storage removal recovery only if a distinct defect remains; do not revisit the already-completed module-lock, mount ownership or spindown inventory work.

