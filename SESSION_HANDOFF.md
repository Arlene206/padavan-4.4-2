# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main parent for the Shadowsocks shell-expansion repair: `b33ed42356995a1ab6dcce42c29db7f9cdf5e67b`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact b33 Shadowsocks workflow #43781109010 FAIL: shell syntax and the existing process-lifecycle regression PASS; the new adblock test stopped at `mktemp` because literal backslashes before `${TMPDIR:-/tmp}` and `${ss_adblock_output}` prevented shell expansion. THIS COMMIT removes the escapes; no pass is claimed until rerun.
- Exact b33 Process lifecycle #43781108999, full inventory #43781109077, AI docs #43781108988, AdGuardHome #43781109013, infosvr #43781108991, AliyunDrive #43781109183 and AliDDNS #43781109014 PASS. NVRAM #43781109000 was still running at last poll.
- Exact 9cd XY-C1 #43779833547 PASS: Build XY-C1, TRX verification and artifact upload all succeeded on the same SHA. This commit adds the Shadowsocks script to the workflow's push path filter for future exact-source firmware validation.
- Prior fixes remain: AiDisk fail-closed Samba/FTP `891b46b51481d171aab6de88ac6634f4d6f56124`, unused-label cleanup `2d97d3b11b48464a21ace2113a04ab26769aff2a`, identity-bound Inadyn SIGUSR1 `f71ee12cbca49972cc927457338d395e64818381`.
- Frozen backup unchanged; no physical-router/L4 validation.

## Active task

### Repair mktemp templates and trigger XY-C1 for Shadowsocks script changes

The exact b33 Shadowsocks Action failed because literal backslashes before `${TMPDIR:-/tmp}` and `${ss_adblock_output}` prevented shell variable expansion in mktemp templates. Remove the escapes in both the production publisher and regression harness. Add the Shadowsocks script to the XY-C1 push path filter so future parser changes receive an exact-source firmware build.

## Verification boundary

- Exact b33 Shadowsocks #43781109010 FAIL only at `mktemp`: the new test's literal `${TMPDIR:-/tmp}` path could not be created. Shell syntax and the previous process-lifecycle test passed. THIS COMMIT removes the literal escapes; exact rerun pending.
- Exact b33 Process #43781108999, full inventory #43781109077, AI docs #43781108988, AdGuardHome #43781109013, infosvr #43781108991, AliyunDrive #43781109183 and AliDDNS #43781109014 PASS; NVRAM #43781109000 pending at last poll.
- Exact 9cd XY-C1 #43779833547 PASS: Build, TRX verification and artifact upload succeeded.
- THIS COMMIT adds the script to the XY-C1 push filter; exact-head Shadowsocks, AI-doc, Process and XY-C1 checks remain pending. No L4 claim.

## Exact first action for the next GPT

1. Inspect exact-head Shadowsocks workflow, Process regression, AI-doc and the newly triggered XY-C1 Build/TRX/artifact; diagnose any genuine failures.
2. Continue reviewing consequential legacy process-presence/name-wide signal consumers from inventory artifact #14186597970 without changing global Boolean API semantics.
3. Preserve the frozen backup and archived log shards/index; no L4 without physical hardware.
