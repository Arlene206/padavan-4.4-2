# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; continue on `main` only. This checker-correction slice is based on parent `dd2885fc9941d3a57fd9c81cf01ed86488b0183d`; resolve current live main on recovery.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. PR #1 remains closed/unmerged.
- Startup/protocol recovery was completed once. Do not repeat bootstrap or completed audits.

## Active task

- Correct the stale FTP portion of `scripts/check_storage_safe_remove.py` exposed by exact-SHA USB storage hotplug run #45764010116.
- Production `services_stor.c` no longer applies the older raw FTP-presence expression in both `stop_stor_apps()` and `start_stor_apps()`. Storage stop records `ftp_was_present = is_ftp_run() != PROCESS_PRESENCE_ABSENT` and calls `ftp_storage_stop_needs_firewall_refresh(ftp_was_present, ftp_final_presence)`. Storage start snapshots tri-state before/after the launch and calls `ftp_firewall_reconcile_needed()`. The checker still required the old expression in both paths and failed after all earlier tests passed.
- This slice updates the checker to validate the current helper call/semantics and existing `ftp_daemon_policy.c` cases for prior-present vs prior-absent states; it does not change production FTP behavior. The storage workflow already watches this checker in both push and pull_request path filters.
- No local checkout is available. Do not claim local execution.

## Verification boundary

- PPP callback state-file runtime coverage exact SHA `7f94feaabb326b9cdc2fa0294d045d6a7de07560`: WAN DHCP callback #45779441563 PASS; its ASan/UBSan GNU89/Werror state-file fixture executed and printed `PPP callback stable lock and atomic generation-state publication: PASS`. AI handoff #45779441622 PASS. Production source baseline `a18496e1b8e5ca06a705f7268d19fdab278873dc` has XY-C1 Build/TRX/artifact PASS; no hardware L4 claim.
- Latest parent main `dd2885fc9941d3a57fd9c81cf01ed86488b0183d`: Process inventory #45779633395, Process fail-closed #45779633230, AI handoff #45779633232, DDNSTO #45779633286 and DNS Forwarder #45779633250 PASS. XY-C1 #45779633237 was IN_PROGRESS at last poll; finish that exact run and inspect Build/TRX/artifact.
- USB hotplug failure source SHA `87cfc6c7983710d5a85af2a8cd5fb68948f22c5d`: run #45764010116 failed only in checker at the obsolete FTP condition (the failing assertion text was “unknown FTP process presence must conservatively request firewall reconciliation”); compiled regression steps and earlier source contracts passed. VSFTPD #45764010135 PASS. XY-C1 #45764010242 Build XY-C1, TRX verification and artifact upload all PASS. This is a checker correction, not evidence of a production-source fault.
- This correction's exact-SHA USB storage hotplug and AI workflows are pending after publication. XY-C1 may skip because only checker/docs files changed; do not infer a new L3 build from a skipped job. No physical-router L4 claim.

## Exact first action for the next GPT

1. Fetch live `main`, resolve this checker correction's exact SHA and inspect its USB storage hotplug stability and AI handoff Actions. Confirm the stale assertion is gone and the existing FTP daemon policy regression executes; diagnose from the first causal job log if anything fails.
2. Finish the parent `dd2885fc9941d3a57fd9c81cf01ed86488b0183d` XY-C1 run #45779633237, reporting Build, TRX and artifact individually. Process/AI/DNS-forwarder/DDNSTO checks on that parent are already PASS; do not repeat them.
3. When the checker rerun passes, continue an unresolved P1 source task from `REFACTOR_ROADMAP.md` (IPTV proxy altnet is still recorded as a concrete remaining validation target); do not re-audit completed PPP, VPN, IAPP, SmartDNS, MiniUPnP or firewall fixes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live `main` and relevant exact-head Actions before ending. Record material PASS/FAIL/PENDING changes and confirm the first next action remains actionable. If all checkpoint facts and the first action remain accurate, explicitly conclude with no file change and no commit.

# Cross-session execution mandate

Continue one autonomous engineering stream across GPT conversations. Every logical source/test slice must create all blobs, one complete Git tree and one atomic main commit, then publish using a non-forced compare-and-swap against the expected parent SHA. Before context exhaustion, finish the current safe slice and leave the next GPT an accurate exact-SHA validation state plus a concrete first task. Do not ask the user what to do next.

## Execution invariants

- Main only; one logical slice = all blobs, one complete tree, one atomic commit, then non-forced expected-parent SHA update. On compare-and-swap conflict, refresh and rebuild; never force-push.
- Never modify the frozen backup or reopen PR #1.
- Host Actions are not MIPS/uClibc runtime proof. XY-C1 L3 requires same-SHA Build, TRX verification and artifact upload PASS. Physical-router L4 remains unverified absent explicit testing.
- Update handoff only when engineering baseline, verification boundary or first concrete action materially changes; no poll-only or SHA-only commit.
