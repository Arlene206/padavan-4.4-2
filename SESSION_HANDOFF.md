# Cross-session execution mandate

Recover live repository state once, then execute the first action below. Do not repeat completed audits or remain at planning. A PENDING check does not block independent work.

For each logical source/test/doc slice, create every blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate. Do not ask the user what to do next.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. the checkpoint is mandatory; a document edit is conditional. Record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Current main: d45672a7f0c74b99eaf93ddf6140fbac9f4fcbe7.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; do not modify it.
- docs/dev-log/INDEX.md, docs/handoffs archive, docs/roadmap-history archive and their indexed shards are immutable. Append only to root dev_log.md when recording useful work.
- No physical-router L4 evidence is claimed.

## Active task

### P2 — Verify failed-FTP-upload cleanup hardening on XY-C1

- The source fix is in 8e7ac1a9680d03e37c01d4701e80b2d0e19faf97. ftp_acl_path_resolve_write_entry() no longer grants cleanup eligibility based on a pathname preflight. The descriptor-relative ftp_acl_path_open_upload_entry() uses O_CREAT|O_EXCL to atomically create a new target; for EEXIST it reopens without O_CREAT. Only the successful creator is cleanup-eligible. Unique STOU retains exclusive-create behavior.
- The host regression covers existing STOR destination preservation, rollback of a newly-created partial file and duplicate exclusive upload rejection. Test/checker integration corrections landed through 079f35644072056f333cf04adffe63f8476d38bd.
- Exact 079f VSFTPD workflow #43744355353 PASS. Process #43744355434, AI docs #43744355446, AliyunDrive #43744355411, AdGuardHome #43744355352 and AliDDNS #43744355620 also PASS.
- d45672a7f0c74b99eaf93ddf6140fbac9f4fcbe7 added asus_ext.c, postlogin.c, ftp_acl_path_policy.h and tests/ftp_acl_path_policy.c to the XY-C1 workflow push filter. This ensures changes to those previously untracked production/helper sources trigger firmware validation.
- Exact d456 XY-C1 workflow #43744574504 is IN_PROGRESS at Build XY-C1. Verify TRX output and artifact upload before making an L3 claim. Do not infer current L3 from older firmware runs.

### Other completed fixes in this work period

- Firefly configuration publication is atomic/fail-closed at d3531270e9d3cf9523f2c0b8f0dc577b687acafb; exact Process #43742499910 PASS with the GNU89 ASan/UBSan host test and static contract.
- MiniDLNA configuration publication is atomic/fail-closed at c20bbfcfc8971a17dcea974f0e31416dafd8c1f4. A stale void prototype in rc.h was fixed by 03d251398ffe1dce36abb628f64502a0cfd4e0fb; Process #43743350500 PASS and inventory/docs checks passed.
- create_mp_link() in services_stor.c validates the entire /proc/mounts inventory, checks parse/read/close outcomes, and removes a provisional link on incomplete input. Fix at e3b14c7a0f3f5a3bb247489e4a8626044bc2739b; exact Process #43743580651 PASS and NFS lifecycle #43743580541 PASS.

## Verification boundary

Exact d456 Actions as last polled:
- Process inventory/lifecycle #43744574506 — PASS.
- AI handoff documentation #43744574423 — PASS.
- NVRAM transaction boundary #43744574473 — PASS.
- AdGuardHome security #43744574467 — PASS.
- AliDDNS transport #43744574478 — PASS.
- AliyunDrive WebDAV lifecycle #43744574503 — PASS.
- XY-C1 firmware #43744574504 — IN_PROGRESS at Build XY-C1; TRX and artifact steps pending.

The exact VSFTPD source/test workflow last ran on 079f (its source and workflow were unchanged by d456) and passed as #43744355353. d456 itself did not modify VSFTPD source; it corrected the XY-C1 path filter. No current-source XY-C1 Build/TRX/artifact success is recorded yet.

The failed-STOR data-loss bug is fixed at host-test/source-contract level. A separate residual race remains in ftp_acl_path_unlink_opened_entry(): fstatat() identity comparison and unlinkat() are separate operations, so a same-directory writer may replace the leaf between them. Linux 4.4 has no unlink-by-file-descriptor primitive. Do not state that the entire cleanup TOCTOU is solved, and do not change it until a strategy is proven not to lose or overwrite a concurrent writer's file while preserving STOR/APPE/REST/STOU semantics.

## Exact first action for the next GPT

1. Poll XY-C1 #43744574504 for exact d456. Inspect Build XY-C1, Verify TRX output and artifact upload; investigate any real failure and make no L3 claim until all three steps pass.
2. Continue an independent, repository-wide user-space stability/security slice; do not re-audit the completed MiniDLNA, Firefly, /proc/mounts or existing-STOR-destination fixes without new evidence.
3. Keep the residual fstatat()/unlinkat() race explicitly blocked unless a Linux 4.4-compatible, no-data-loss solution is proven by tests.
4. Preserve the frozen backup and archive shards. Use one atomic tree/commit and non-forced expected-SHA update for each logical task.

## Invariants

- Preserve disabled-feature no-op semantics and current compatibility constraints unless an evidence-backed defect requires otherwise.
- Exact-head Actions evidence only; PENDING permits independent work.
- No firmware L3 without exact-head Build, TRX verification and artifact upload. No physical-router L4 without hardware evidence.
- Do not mutate backup/pre-plugin-overhaul-2026-11-14, docs/dev-log archive shards, docs/handoffs archives, docs/roadmap-history archives or docs/dev-log/INDEX.md.
