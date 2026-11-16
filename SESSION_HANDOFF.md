# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main SHA for this source/test slice: `59feb2da538a282d9929d5a48bd97c4a8861c6ae`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact-parent Shadowsocks workflow #43781489845 PASS: shell syntax, process lifecycle regression, adblock publication regression and source/ROMFS checker all succeeded.
- Exact-parent AI handoff docs #43781489871, Process lifecycle #43781489848, Process inventory #43781489882, NVRAM #43781489851, infosvr #43781489801, AliDDNS #43781489923, AdGuardHome #43781489834 and AliyunDrive #43781489841 PASS.
- Exact-parent XY-C1 firmware validation #43781489913 is IN_PROGRESS at Build XY-C1; TRX verification and artifact upload have not run yet.
- This slice corrects the remaining C-style `/* ... */` preamble immediately before `ss_publish_adblock_config()` in a shell script. It is now a valid shell `#` comment; the regression extracts from that comment line so a non-shell preamble fails the test before exercising the parser.
- The parser still publishes supported dnsmasq/address input or simple `||domain^` rules with a same-directory temporary file and atomic rename; unsupported/empty input leaves the prior configuration untouched.
- No physical-router/L4 result is available.

## Active task

### Verify the shell-comment correction, then continue the process-consumer review

The production Shadowsocks script had a C block-comment token immediately before a shell function. Since the file is parsed as shell, that token was treated as a command rather than a comment. This commit replaces it with shell comments and strengthens the host regression to include the preamble in the extracted production helper. First verify this commit's exact-head Actions; then continue reviewing consequential legacy process-presence and name-wide signal consumers from inventory artifact #14186597970 without changing global Boolean API semantics.

## Verification boundary

- Exact `59feb2d` parent Shadowsocks #43781489845 PASS, including the adblock regression. Local POSIX-shell syntax and publication regression also PASS; a negative fixture using the former C-style preamble is rejected by the strengthened extraction guard.
- Exact-parent AI docs, Process lifecycle, Process inventory, NVRAM, infosvr, AliDDNS, AdGuardHome and AliyunDrive Actions PASS (run IDs above).
- Exact-parent XY-C1 #43781489913 remains IN_PROGRESS at Build XY-C1. No TRX/artifact or L3 claim is made yet.
- The current commit requires exact-head Shadowsocks, AI-doc, Process and XY-C1 Build/TRX/artifact checks. No physical-router/L4 validation is claimed.

## Exact first action for the next GPT

1. Fetch the live `main` SHA, then inspect exact-head Shadowsocks, AI handoff documentation, Process lifecycle/inventory and XY-C1 Actions for this shell-comment correction. Confirm the strengthened adblock test and same-SHA Build XY-C1 / TRX / artifact upload.
2. Once this slice is green, continue the whole-tree process-presence/name-wide signal consumer review driven by inventory artifact #14186597970; preserve the legacy Boolean API semantics and avoid repeating already-completed callsite audits.
3. Preserve `backup/pre-plugin-overhaul-2026-11-14` and archived `dev_log.md` shards/index byte-for-byte; no L4 claim without physical hardware.
