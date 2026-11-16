# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main parent for downloaded dnsmasq directive validation: `a22914f4de9a47b7a7e6c28711b9927c93ae8120`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact a229 Shadowsocks lifecycle #43782262134, Process #43782262196, full inventory #43782262138, AI docs #43782262148, infosvr #43782262209, AdGuardHome #43782262073 and AliyunDrive #43782262128 PASS.
- Exact a229 AliDDNS #43782262168, NVRAM #43782262086 and XY-C1 #43782262108 were still running at last poll; XY-C1 Build was in progress, with TRX/artifact pending.
- Exact 59feb Shadowsocks #43781489845, Process #43781489882, inventory #43781489848 and AI docs #43781489871 PASS; exact 9cd XY-C1 #43779833547 PASS (Build/TRX/artifact).
- THIS COMMIT whitelists downloaded dnsmasq-style input to comments, blank lines and `address=` rules, rejecting additional `dhcp-script=` / `conf-file=` directives without replacing last-good config.
- Frozen backup unchanged; no physical-router/L4 validation.

## Active task

### Reject non-address dnsmasq directives in downloaded adblock lists

The configured remote adblock URL is untrusted input. The publisher previously copied every line of a dnsmasq-style file if it contained at least one `address=` line. Restrict it to comments, blank lines and `address=` rules; reject any other directive, including `dhcp-script=` and `conf-file=`, and preserve prior output on rejection. Extend the regression and source contract.

## Verification boundary

- Exact a229 Shadowsocks #43782262134, Process #43782262196, inventory #43782262138, AI docs #43782262148, infosvr #43782262209, AdGuardHome #43782262073 and AliyunDrive #43782262128 PASS.
- Exact a229 AliDDNS #43782262168, NVRAM #43782262086 and XY-C1 #43782262108 were running at last poll; XY-C1 Build had not completed and TRX/artifact remained pending.
- THIS COMMIT adds a mixed dnsmasq injection regression for `dhcp-script=` / `conf-file=`, requires last-good preservation, and extends the source contract. Exact-head Actions pending.
- No L3/L4 claim unless current exact-SHA Build XY-C1, TRX and artifact all pass.

## Exact first action for the next GPT

1. Inspect exact-head Shadowsocks parser/source, Process/AI-doc and XY-C1 Build/TRX/artifact Actions for THIS COMMIT; fix any regression failures.
2. Continue reviewing consequential process-presence/name-wide signal consumers from inventory artifact #14186597970 without changing legacy global Boolean API semantics.
3. Preserve frozen backup and archived log inventory; no physical-router/L4 claims.
