# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main SHA before this shell snapshot fix: 5c929dbe5b40cdc7414bfa9997cf3edce834714e.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; preserve it unchanged. Only main and the frozen backup branch exist.
- On ancestor 80bfd6670cd8ffa1bc06bdbdf36891710fee5922, USB storage run #45060433291 PASS and Process inventory #45060433269 PASS. XY-C1 #45060433345 was still building; DNSMasq #45060433306 failed only its stale checker assertion start.count("smartdns_restore_previous_instance") == 3; checker-only commit 5c929db updated that expected count to 4. DNSMasq run #45060566217 on 5c929db was in progress at last check.
- Exact 64d3b46 AI docs #45060286887 and Process #45060286908 PASS; HTTPD #45060286954 PASS. Storage #45060286951 passed on the same SHA and is superseded by successful storage run #45060433291 on 80bfd.
- The ejected-device swap parser semantic fix is in 64d3b46afa7c899a88defc7167155cd872b46c51; Storage #45060433291 on 80bfd includes and passed that production/test source.
- No local checkout or router hardware is available. Device L4 remains NOT VERIFIED.

## Active task

Close a subtle completeness hole in the shell /proc/swaps readers. Both automount_swap_status() and spindown_swap_status() capture the file using command substitution, which strips trailing LF bytes. They then appended a new LF with printf before awk, so a source snapshot missing its final header/record terminator could be normalized into a complete-looking inventory. Incomplete inventory must never authorize swapon or disk spindown.

The fix uses a private sentinel appended inside command substitution; after stripping exactly that sentinel, each helper requires the original snapshot to end in LF and feeds awk the retained bytes without appending a replacement terminator. The two shell regressions cover an unterminated header and an otherwise valid final swap row without LF, in addition to existing active/inactive, malformed, empty and unreadable cases.

## Verification boundary

- Parent main for this slice: 5c929dbe5b40cdc7414bfa9997cf3edce834714e; resolve the resulting exact SHA from live main after publication.
- Source change is limited to the two /proc/swaps shell readers. Missing original final LF yields UNKNOWN; cat/read failure and empty input remain UNKNOWN. Valid source content reaches awk with its original delimiters.
- Regression additions: storage_mountpoint_inventory.sh and spindown_mount_guard.sh each exercise a header missing LF and a full-looking final record missing LF. Existing workflow push/pull_request filters already cover both production scripts and both tests.
- Parent Storage #45060433291 on 80bfd PASS includes the physical-eject GNU89 semantic parser test. New final-LF cases require a fresh exact-head Storage run; local execution NOT RUN (no checkout).
- Parent AI docs #45060286887, Process #45060286908 and HTTPD #45060286954 PASS. Parent XY-C1 #45060433345 remained in progress; DNSMasq #45060433306 failed on the stale checker count, already corrected by 5c929db; rerun #45060566217 was in progress. This commit requires a fresh matching Storage/XY-C1 build; L3 remains NOT VERIFIED until detector, Build XY-C1, TRX and artifact all pass on the same production-source SHA. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main; match new Storage, AI handoff and XY-C1 runs to that exact SHA, and poll DNSMasq rerun #45060566217 for the corrected SmartDNS source contract. Confirm both missing-final-LF shell tests and the ejected-inventory GNU89 semantic parser regression pass. Then inspect same-SHA XY-C1 detector, Build, TRX and artifact upload. Diagnose failures from their first causal logs; continue independent work while checks are pending.

