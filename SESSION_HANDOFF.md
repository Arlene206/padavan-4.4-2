## Current repository state

# Cross-session execution mandate

# Cross-session execution mandate

Each source/test slice is one atomic main commit: construct one complete Git tree, create one commit, then update main by a non-forced expected-SHA compare-and-swap. Continue this repository as one cross-session engineering stream. A logical multi-file change lands as one complete Git tree and one commit on `main`, followed by exact-SHA push validation. Never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow does not block independent safe work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify `main` and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record material state transitions, and name the next concrete implementation task. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the files untouched and create no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not performed. Before context exhaustion, finish the current safe slice and preserve the exact verification boundary and first action.

- Parent main before this VSFTPD source slice: `f43b856869a0ad94bae5fd15a1e14daff1f8ec1f`. Re-fetch live main before every write.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- PR #1 remains closed/unmerged and deeply behind main. Valid AliDDNS CA-bootstrap/HMAC evolution is already integrated on main; do not port the stale PR wholesale.
- Only `main` is authorized for development and validation. Use one complete tree, one commit, and a non-forced expected-SHA ref update.

## Active task

### VSFTPD stop/firewall reconciliation — RC service-control path

Relevant files:
- trunk/user/rc/src/services_stor.c
- trunk/user/rc/src/ftp_daemon_policy.h
- trunk/user/rc/src/tests/ftp_daemon_policy.c
- .github/workflows/vsftpd-stability.yml
- .github/workflows/storage-hotplug-stability.yml

A stop command can report an error while the final procfs scan proves vsftpd absent. Standalone stop, restart and FTP/Samba group-stop callers decide FTP firewall/UPnP reconciliation from the independent final tri-state process observation, not the collapsed stop status. Only PROCESS_PRESENCE_ABSENT permits cleanup; RUNNING/UNKNOWN/invalid remain fail-closed. The original stop error is preserved and restart/eject does not proceed after a failed stop.

Exact source commit b3f165ef4e08b585cc5a77ccdfb791165ba60496 changed service control, policy regression and VSFTPD source contract. Exact descendant 155a8be964e1ceb1ce82583df241be2248d1ec9d passed VSFTPD #45483093422 (GNU89 regression and embedded Python checker) and AI handoff #45483093417. The source commit's storage-wide checker was stale; THIS commit aligns that caller graph. No local cross compiler/MIPS build is claimed.

### Shared process inventory — exact 15-byte comm boundary

Review found an off-by-one in `proc_parse_stat()`: a 15-byte `comm` exactly fits the 16-byte kernel task-name buffer including its NUL terminator, but the scanner marked lengths >= 15 as truncated. When cmdline is empty or does not match the requested name, a valid maximum-length process name could be missed. This source slice changes the boundary to > 15 and adds a GNU89 regression using an exact 15-byte name with an empty cmdline. The Process workflow now pins both the comparison and test case. No local compiler run or MIPS build is claimed.

### Parallel FTP verification — final-presence firewall reconciliation

The production FTP stop/firewall change remains in `b3f165e`; VSFTPD checks passed on `155a8be`. Storage-hotplug run `d91dd33` passed its compiled C tests and full caller-graph checker but exposed duplicate stale FTP contracts in `scripts/check_storage_safe_remove.py`. The checker-only correction is now on current parent `f5b01d9`; exact storage-hotplug run `45484032665` and XY-C1 run `45484032783` were in progress at last observation. AI handoff `45484032630` passed. Do not transfer status between SHAs.
 
## Verification boundary

- Exact `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`: initial udhcpc #45480041494 had C GNU89/ASan/UBSan regression PASS and a stale Python assertion failure. After checker-only corrections, descendant `b2dab6b05fb2cf07abbdae01b1acff2cc134df31` udhcpc #45480533291 PASS (C regression and Python source contract).
- Exact `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`: XY-C1 #45480041423 PASS, including Build XY-C1, TRX verification and artifact upload. Process inventory #45480041644, Process lifecycle #45480041481, USB storage #45480041346, NVRAM #45480041721 and AI handoff #45480041430 PASS. L3 VERIFIED for this DHCP product-source SHA; hardware/router L4 NOT VERIFIED.
- Descendant `b2dab6b05fb2cf07abbdae01b1acff2cc134df31` AI handoff #45480533158 failed only the literal-string validator; docs-only descendant `f43b856869a0ad94bae5fd15a1e14daff1f8ec1f` AI handoff #45480803456 PASS.
- Exact SHA `b3f165ef4e08b585cc5a77ccdfb791165ba60496`: VSFTPD #45482945754 failed only embedded source assertions; its GNU89 C policy binary passed. Exact descendant `155a8be964e1ceb1ce82583df241be2248d1ec9d` VSFTPD #45483093422 PASS (GNU89 regression + Python source contracts) and AI handoff #45483093417 PASS.
- Exact SHA `d91dd33195eb32b29e2250c9cec4cd1604e50b77` storage-hotplug #45483595415: all compiled C regressions and whole-caller-graph check PASS; the final `scripts/check_storage_safe_remove.py` step failed on two stale `stop_ftp_locked(void)` lookups. THIS commit aligns both duplicate FTP contracts with the final-presence output. Exact-head rerun pending.
- Other b3 results: libdisk, USB modem identity, MiniDLNA, Samba, IPv6, Firefly, storage-service enable, NFSD, AI docs, Process inventory, firewall startup and HTTPD security PASS; XY-C1 #45482945636, NVRAM #45482945421 and Process lifecycle #45482945410 were pending at last observation.

## Exact first action for the next GPT
1. Inspect exact-head Process lifecycle/inventory and XY-C1 Actions for the new 15-byte `comm` boundary fix. Require GNU89 ASan/UBSan regression and source contract PASS; L3 requires Build, TRX and artifact upload PASS on the same product SHA.
2. Inspect f5b01d9 storage-hotplug #45484032665 and XY-C1 #45484032783 to close the FTP stop/firewall validation boundary; AI docs #45484032630 is PASS. Also check b3f165e XY-C1 #45482945636 only by its own SHA.
3. Continue an independently evidenced stability/security issue across Padavan userland; do not repeat closed DHCP work. Hardware/router L4 remains unverified.
