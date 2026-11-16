# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main parent for the Shadowsocks adblock parser/publication fix: `9cd5348bba7bee2a2f163d46cc553e32ffb44daa`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact 9cd HTTPD security #43779833479 PASS; AI docs #43779833509 PASS; Process lifecycle #43779833453 PASS; NVRAM #43779833462 PASS; AdGuardHome #43779833447 PASS; infosvr #43779833483 PASS; AliDDNS #43779833617 PASS; AliyunDrive #43779833471 PASS.
- Exact 9cd XY-C1 #43779833547 remains in progress. Last poll: Build XY-C1 running; TRX verification and artifact upload pending. This SHA includes the AiDisk unused-label cleanup and identity-bound Inadyn SIGUSR1 change.
- Manual DDNS identity-bound refresh commit `f71ee12cbca49972cc927457338d395e64818381`: exact Inadyn lifecycle #43779424646 PASS (GNU89/ASan/UBSan, PID-bound SIGUSR1 success + UNKNOWN rejection).
- AiDisk fail-closed Samba/FTP change `891b46b51481d171aab6de88ac6634f4d6f56124`; unused success label cleaned in `2d97d3b11b48464a21ace2113a04ab26769aff2a`. Exact 9cd Process/HTTPD/AI-doc checks pass.
- Current independent source defect: `trunk/user/shadowsocks/scripts/shadowsocks.sh:start_AD()` contains invalid `check = ...` shell syntax, so intended dnsmasq/Adblock format detection is not performed. THIS COMMIT adds a tested parser and atomic last-good publication.
- Frozen backup unchanged; no physical-router/L4 validation.

## Active task

### Fix Shadowsocks adblock format detection and preserve last-good configuration

The `start_AD()` function contains `check = `grep -wq ...``, which attempts to execute a command instead of assigning a variable; quiet grep also cannot provide the intended content test. Correctly distinguish dnsmasq `address=` files from simple `||domain^` rules, convert only supported domain rules, reject empty/unsupported input, and publish through a same-directory temporary file and atomic rename. A focused shell regression and Shadowsocks CI workflow test this behavior.

## Verification boundary

- Exact 9cd HTTPD security, Process lifecycle, NVRAM, AI docs, AdGuardHome, infosvr, AliDDNS and AliyunDrive Actions all PASS.
- Exact 9cd XY-C1 #43779833547 remained at Build XY-C1 at last poll; TRX verification and artifact upload pending.
- THIS COMMIT adds a host shell regression for dnsmasq passthrough, Adblock domain conversion, unsupported/empty input rejection and last-good preservation. The Shadowsocks lifecycle workflow must run on this exact source SHA.
- No L3/L4 claim until exact-source XY-C1 Build, TRX validation and artifact upload all pass; no hardware testing is available.

## Exact first action for the next GPT

1. Inspect exact-head Shadowsocks lifecycle and XY-C1 Actions. Confirm both new adblock regressions and same-SHA Build XY-C1 / TRX / artifact upload.
2. Review remaining high-impact legacy process-presence and name-wide signal consumers from artifact #14186597970, without changing global Boolean API semantics.
3. Preserve the frozen backup and archived log inventory; no L4 without physical hardware.
