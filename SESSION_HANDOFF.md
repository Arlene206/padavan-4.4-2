# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root `dev_log.md` newest-first with 20–119 dated entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Current main before this workflow-contract repair: `79e1932c74dc289e710c82ba3f184c9ee2bd785e`. Current production-source baseline includes the SSH fix in this SHA. Earlier root source baseline `d9bffce5e1dd5e3683102a5ced9350e26c15d02e` contains the router-mode transaction fix; current SSH source is layered on top.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- SSH defect fixed on `79e1932`: public start and restart now use a checked `sshd_enable` parser; restart performs the checked read before process inventory or stop and reuses that mode after teardown. Mode backend errors and malformed values fail closed without disabling an existing listener.
- Exact-head SSH source tests on `79e1932` compiled and passed GNU89/`-Werror`/ASan/UBSan. The Process lifecycle #44905882972 failure is only a stale Python checker self-reference: it searched for the function's name inside its own function body. The corrected checker in this commit verifies API error and inconsistent result/pointer branches instead.
- PID 1 notification queue #44905882925 passed its bounded notification, marker trust-boundary, enqueue-status, optional-daemon and SSH GNU89/ASan/UBSan policy tests, then failed on an obsolete source contract expecting launcher calls inside public `start_sshd(void)`. The implementation now intentionally delegates from that checked wrapper to `start_sshd_with_mode(int sshd_mode)`; this commit updates the checker to validate the wrapper/delegation and the implementation separately.
- On exact SHA `79e1932`: AI handoff docs #44905882928 PASS; process inventory #44905882756 PASS; NVRAM transaction audit #44905882978 PASS. Other completed exact-head module checks include watchdog signal #44905882883, VPN server #44905882974, detect_link signal #44905882897, WAN gateway ARP #44905882994, detect_internet signal #44905882880, DNSMasq #44905882863, FRP #44905882965, Adbyby #44905882908, networkmap #44905882948, SkipDBv2 #44905882864, HTTPD security #44905882866, USB storage #44905882960, and related service workflows.
- XY-C1 #44905883001 is still in progress on exact production-source SHA `79e1932`, currently at Build XY-C1; TRX verification and artifact upload are pending. Do not claim L3 until Build, TRX and artifact all pass on this exact source SHA. Earlier XY-C1 #44905479795 was host-only with Build/TRX/artifact skipped and does not provide L3.
- MiniUPnP SOAP internal-port-zero validation is already implemented and tested in the existing source: both AddPortMapping handlers use bounded non-zero internal-port parsing while preserving the generic zero-capable external-port parser needed by AddAnyPortMapping. Do not restart that task as if it were missing. Router-mode transaction fix and NFS checked-enable fixes have prior exact host/workflow PASS results; no need to repeat them absent new evidence.
- Physical router L4 remains NOT VERIFIED.

## Active task
1. Inspect exact-head reruns from this workflow-contract repair. Confirm SSH Process lifecycle and PID 1 notification queue pass on the new SHA; if not, read the first causal log line and repair the contract/source without weakening assertions.
2. Inspect XY-C1 #44905883001 Build XY-C1, Verify TRX output and artifact upload for exact product SHA `79e1932`; current Build was in progress at the checkpoint. Claim L3 only if all three stages pass.
3. Continue with a genuinely open P1 reliability/security source defect across Padavan userland. Do not select the already-implemented MiniUPnP internal-port-zero parser or repeat completed NFS/HTTPD/router-mode audits. The older roadmap note about setter wrapper callsites may be stale; direct callsites for the legacy void WAN setters were not found in RC source.

## Verification boundary
- SSH GNU89/`-Werror`/ASan/UBSan policy test PASS on `79e1932`; Process source-checker false failure corrected here and exact rerun pending.
- PID 1 notification queue batch, marker trust-boundary, enqueue status, optional-daemon and SSH policy tests PASS on `79e1932`; its stale start-wrapper source contract corrected here and exact rerun pending.
- AI handoff docs and process inventory PASS on exact SHA `79e1932`. NVRAM transaction audit PASS on `79e1932`.
- XY-C1 Build is running on the exact production-source SHA `79e1932`; TRX/artifact pending. No L3 claim until the complete same-SHA chain passes.
- No local RC cross-compile or physical router/hardware test is claimed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT
Fetch live main and inspect the exact-head Process lifecycle, PID 1 notification queue, AI docs and XY-C1 run for the newly committed checker correction. Check XY Build/TRX/artifact as three separate steps. If those source contracts pass, continue the next confirmed P1 production-source defect rather than repeating completed work.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.