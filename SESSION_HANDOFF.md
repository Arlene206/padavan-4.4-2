# Cross-session execution mandate

## 2026-11-23 continuation handoff (parent: 53c6f6623e5531deeb889762e81bf1943c1abf50; always fetch live main)

### Completed production slices and exact verification
- 9ebc57999dad5da47e20414174cd7572ee64bbd7: shared process inventory revalidates /proc/PID/stat after reading cmdline, rejecting PID reuse rather than combining two processes into a false RUNNING identity. Exact Process inventory #45227637642 PASS; Process fail-closed #45227637610 PASS; AI docs #45227637592 PASS. XY-C1 #45227637594 was IN_PROGRESS at the last poll; no L3 claim.
- 7e81b11d63b1bf20d8a1eb103cc7598b9b898b1a: repair the Shadowsocks reserver path, which previously called an undefined TCP startup helper, applied firewall generation to the wrong node, and hid required startup failures. Exact Shadowsocks lifecycle #45228682477 PASS; Process inventory #45228682072 PASS; AI docs #45228682265 PASS. XY-C1 #45228682113 was IN_PROGRESS at the last poll; no L3 claim.
- 53c6f6623e5531deeb889762e81bf1943c1abf50: repair the DNSMasq split-DNS source contract after a false-negative helper-invocation assertion. Exact DNSMasq lifecycle #45229862631 PASS; AI handoff documentation #45229862614 PASS.
- Current source slice (this commit's parent is 53c6f6623e5531deeb889762e81bf1943c1abf50): bind AliyunDrive WebDAV process detection to the executable's device/inode, not merely BusyBox pidof's spoofable comm/argv[0] name match. Exact-head AliyunDrive lifecycle and AI documentation Actions are pending for the new commit. Do not claim the regression or source contract passed until the exact SHA Actions confirm it.

### Current repository state
- Repository: Arlene206/padavan-4.4-2; work on main only.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify or force-push it.
- PR #1 is closed/unmerged. AliDDNS CA trust-store/HMAC work is represented or superseded on main.
- No local checkout or MIPS/uClibc toolchain is available in this session. Do not claim local compilation, firmware/TRX/artifact validation or physical hardware results.
- XY-C1 template sets CONFIG_FIRMWARE_INCLUDE_ALDRIVER=n; the AliyunDrive source change does not imply or require XY-C1 validation. Its dedicated lifecycle workflow is the relevant exact-head test.

### Active task
#### AliyunDrive WebDAV process identity
- BusyBox 1.24 pidof matches comm and/or basename of argv[0]; that is not proof that a candidate executes the configured daemon.
- The launcher now uses stat -L -c '%d:%i' on $ALD_BIN and each /proc/$pid/exe. Missing, malformed, unreadable or mismatched identity maps to UNKNOWN (2), preventing duplicate starts and all stop signals. Existing TERM/KILL sequencing and no-cached-PID signaling remain unchanged.
- The lifecycle fixture covers a same-name impostor, inaccessible and malformed executable metadata, no signalling on mismatch, and no duplicate start. The source checker verifies the exact inode comparison.
- Relevant path filters already exist in .github/workflows/aliyundrive-webdav-lifecycle.yml; no workflow edit is needed. Exact-head test results are pending at this handoff.

### Verification boundary
- AliyunDrive shell syntax, MIPSel binary QEMU capability probe, lifecycle regression, dev_init private-lock checks and source/RC integration checker run in the dedicated workflow. Inspect every step on the exact commit SHA; a workflow PASS is required before calling L2 verified.
- 9ebc579 Process inventory/fail-closed and AI docs exact runs passed. XY-C1 #45227637594 was last observed IN_PROGRESS; Build, TRX and artifact steps must all pass on that same product-source SHA before L3.
- 7e81b11 Shadowsocks lifecycle, Process inventory and AI docs exact runs passed. XY-C1 #45228682113 was last observed IN_PROGRESS; no L3 claim until Build, TRX and artifact upload all pass on that same source SHA.
- 53c6f66 DNSMasq lifecycle and AI handoff documentation exact runs #45229862631 and #45229862614 passed.
- AliyunDrive is disabled in XY-C1's board template, so L3 firmware coverage for this AliyunDrive-only change is not implied.
- Physical router / production AliyunDrive account behaviour remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect the exact-push AliyunDrive lifecycle and AI handoff Actions for the current commit. Confirm sh -n, the QEMU CLI capability probe, mismatch/unreadable/malformed executable regressions, and inode source contract all pass. Diagnose and fix any FAIL; do independent work while checks are PENDING.
2. Recheck the outstanding XY-C1 runs #45227637594 (9ebc579) and #45228682113 (7e81b11) individually. L3 is valid only if Build, TRX and artifact upload all pass on one identical product SHA.
3. Continue the next concrete reliability/security defect in the live whole-repository roadmap; do not re-audit completed process inventory, backup-server start, DNSMasq split-DNS contract, APCLI, WAN ARP or printer work without new evidence.
4. Use Git data APIs to create all blobs, one complete tree and one commit for each logical slice; update main with non-forced expected-SHA compare-and-swap. Preserve the backup branch.

## Cross-session operating constraints
Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on main; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update main using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state before ending. Record only material changes: a relevant exact-head Action changing from PENDING to PASS/FAIL, a new commit, an unresolved failure, or a changed first action. If all checkpoint facts and the first action are already accurate, explicitly conclude with no file change and no commit. Do not manufacture documentation churn or a no-op commit.

## Development loop
- Source modification → targeted tests → one complete tree/one atomic commit → exact-SHA Actions.
- PASS advances to the next issue; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record the exact latest SHA, test results, CI state, unresolved issues and one concrete first task. Never claim unverified work.