# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current main parent for the HTTPD security-checker update: `2d97d3b11b48464a21ace2113a04ab26769aff2a`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact 2d97 Process lifecycle #43779569397 PASS, whole-tree inventory #43779569493 PASS, AI handoff docs #43779569387 PASS, infosvr #43779569427 PASS, AliyunDrive #43779569474 PASS, AdGuardHome #43779569325 PASS, AliDDNS #43779569367 PASS. XY-C1 #43779569458 and NVRAM #43779569419 remained in progress at last poll.
- Exact 2d97 HTTPD security #43779569517 FAIL only in its source-wiring assertion: `privileged_script_sinks["ddnsclient"]` still expected the old name-wide `killall -SIGUSR1`; the production handler now correctly calls `notify_rc(RCN_MANUAL_DDNS_UPDATE)`. All earlier compile, policy and HTTPD security checks in the same job passed.
- Manual DDNS identity-bound refresh commit `f71ee12cbca49972cc927457338d395e64818381`: exact Inadyn lifecycle #43779424646 PASS (GNU89/ASan/UBSan compile, SIGUSR1 success and UNKNOWN rejection, production integration checker).
- AiDisk orphaned-label cleanup commit `2d97d3b11b48464a21ace2113a04ab26769aff2a` removes the unreferenced success label and updates Process source-contract boundaries. Exact Process #43779569397 and AI docs #43779569387 PASS; XY-C1 still in progress.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 validation.

## Active task

### Repair the HTTPD security source-contract for identity-bound DDNS refresh

The manual DDNS change moved the privileged `ddnsclient` handler from name-wide `killall -SIGUSR1 inadyn` to `notify_rc(RCN_MANUAL_DDNS_UPDATE)`. The exact 2d97 HTTPD security workflow still asserted the former sink, so the job failed after all earlier security/policy checks passed. Update the allowlisted sink contract to the new RC event; the dedicated Inadyn checker already verifies the complete HTTPD→notify_rc→RC→identity-bound SIGUSR1 implementation.

## Verification boundary

- Exact 2d97 Process #43779569397, full-tree inventory #43779569493 and AI-doc #43779569387 PASS; XY-C1 #43779569458 / NVRAM #43779569419 were in progress at last poll.
- Exact 2d97 HTTPD security #43779569517 failed only because the source allowlist still expected the removed `killall -SIGUSR1 inadyn` string. The expected source sink is corrected in THIS COMMIT; exact-head HTTPD security must rerun and pass.
- Exact f71 Inadyn lifecycle #43779424646 PASS, including GNU89/ASan/UBSan and SIGUSR1/UNKNOWN tests. No L3/L4 claim until current exact-SHA XY-C1 Build/TRX/artifact pass.

## Exact first action for the next GPT

1. Inspect exact-head HTTPD security, Process, inventory, AI-doc and XY-C1 Actions for THIS COMMIT; confirm the ddnsclient sink assertion, identity-bound signal regressions and same-SHA Build/TRX/artifact.
2. Continue the process-lifecycle inventory review on consequential state-changing consumers; the next clear candidate is the dormant `wpacli_main()` DHCP force-renew branch, currently disabled under `#if 0`, so do not re-enable it without an explicit product compatibility requirement.
3. Preserve the frozen backup and archived log inventory; no L4 without physical hardware.
