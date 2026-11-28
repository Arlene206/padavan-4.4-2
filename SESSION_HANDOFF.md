## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- Latest main SHA: `25f47f5655f53d1c0636328d3840571f86ca690e`; parent `125a21650dc4d5276cc4d3340c18581b106191dd`. DHCP raw-receiver and server-option source fixes are committed; no uncommitted working tree exists.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store bootstrap is integrated.

## Active task

- BusyBox DHCP raw packet and server option hardening is committed. DHCPv4/v6 raw receivers reject short/inconsistent IP/UDP lengths before header subtraction/copy and clear both input/output packet buffers. DHCP server lookups now preserve malformed-present vs absent status for SERVER_ID/REQUESTED_IP and require exactly one byte for DHCP_MESSAGE_TYPE; malformed option encodings are ignored before message-state mutations. See commits `4b84d069`, `fb1f82f`, `e8aa2a20`, `125a2165`, `25f47f56`.
- Exact DHCP client/IPv6 and server policy/source contracts pass. Current exact-head XY-C1, NVRAM and storage statuses are still being resolved; see checkpoint below. Do not repeat completed DHCP source changes.

### Completed WAN source slice

- `e0efa3a421051d24a21b8519983892ffdef75456` added bounded detached-daemon startup postcondition and EINTR-preserving waits.
- `9fc2f1015f1f0f8933b1c02bb007711dc95e538b` tightened the probe to `process_pidfile_read_identity("/var/run/detect_wan.pid", "detect_wan", ...)`, requiring PID/start-time identity of the published live child. Missing/stale identity is retried as ABSENT; unreadable/malformed/indeterminate state fails closed as UNKNOWN.
- Existing GNU89/ASan detached-daemon regression covers delayed appearance, absent-child timeout, UNKNOWN, launcher failure and interrupted waits.

## Exact-SHA Actions checkpoint

- DHCPv4/v6 raw receive regression/source contract #46155870736 PASS and DHCPv6 Reply retry #46155870708 PASS on exact product SHA `fb1f82f116d4f10f563d155a9616a61c132183be`. AI docs #46155870682, Process inventory #46155870699, WAN DHCP callback #46155870761, USB storage #46155870664, Process lifecycle #46155870697 and NVRAM audit #46155870680 also PASS on that SHA.
- DHCP server malformed-option regression #46156530716 PASS and Process inventory #46156530717 PASS on exact SHA `25f47f5655f53d1c0636328d3840571f86ca690e`. The initial run #46156337300 on parent `e8aa2a2` passed the GNU89 C binary but failed only on split-line checker matching; checker-only commit `125a216` normalized whitespace and the current exact run passed.
- NVRAM transaction audit #46156530675, USB storage regression #46156530714 and XY-C1 firmware validation #46156530715 were IN_PROGRESS at handoff. Require Build XY-C1, TRX verification and artifact upload on a production-source SHA before L3; do not wait idly.
- WAN route #46155166807 PASS on `c95799c`; WAN ARP #46154974007 PASS on `9fc2f10` containing the PID-identity gate. AI docs #46155870682 PASS on `fb1f82f`; a new exact docs run will be triggered by this handoff commit.
- Frozen backup branch remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify.

## Verification boundary

- L1: DHCPv4/v6 raw-length policy and server option policy compile/run with GNU89, Werror, ASan and UBSan; source integration contracts PASS (#46155870736 on `fb1f82f`; #46156530716 on `25f47f`).
- L2: Process inventory #46156530717 PASS on `25f47f`; WAN route #46155166807 and WAN ARP #46154974007 pass on source containing the PID-identity gate. Current NVRAM #46156530675 and storage #46156530714 were still pending.
- L3: XY-C1 #46156530715 pending at last poll. Do not claim firmware verification until Build XY-C1, Verify TRX output and artifact upload all succeed on a source SHA containing current product changes.
- L4: physical router DHCP lease transitions, WAN gateway-loss and hardware runtime NOT VERIFIED.

## Exact first action for the next GPT

1. Query exact-head #46156530715 (XY-C1), #46156530675 (NVRAM) and #46156530714 (USB storage); inspect the XY job steps and confirm Build, TRX and artifact upload.
2. If any remain pending, work on the next independent P1 userland stability/security defect; do not repeat the DHCP option/raw-length checks already passing.
3. Keep the backup branch immutable; make complete-tree single-commit writes with non-forced expected-SHA updates.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
