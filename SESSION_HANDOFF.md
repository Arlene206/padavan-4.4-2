# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for this work sequence: 4148ac7315d4835b9818c555956d99b195bf4947 (fix(rc): propagate file creation close failures). This handoff is committed in the new main tree; resolve live main before another write.
- All 13 exact-head Actions on 4148ac7315d4835b9818c555956d99b195bf4947 passed: DNSMasq 44608598453, process inventory/fail-closed 44608598436, HTTPD security 44608598480, XY-C1 firmware validation 44608598285, watchdog 44608598200, Crond 44608598209, AI handoff docs 44608598296, WAN route 44608598328, process lifecycle inventory 44608598245, WAN DHCP 44608598304, firewall startup 44608598224, USB hotplug 44608598233, NVRAM audit 44608598264.
- Backup remains at 72f4f157a084f8184b9597ca5bf334b8080e5807 and frozen. PR #1 remains closed and unmerged.
- No local MIPS toolchain or physical router is available. Host GNU89/ASan/UBSan policy tests are L2 only; XY-C1 Actions are L3; physical-device behavior is L4 NOT VERIFIED.

## Closed recent task — DHCP teardown monitor-pause status

- Production commit a7eae5a115dae74685e9130ff6d339b8202e5725 captures and aggregates monitor-pause errors during MAN/WAN DHCP teardown while completing cleanup.
- Checker correction dec2079fcbf9d45731dff1ce03b2765a23bd3a31 updated stale detect-internet and WAN route contracts. Do not revisit this closed slice.

## Closed recent task — DNSMasq resolver file and shared create_file status

- Commit b1835ef0936517df7a39ef06dc242168251dd4fe makes router-mode DNSMasq preparation fail closed if its configured resolver input cannot be created, preserving errno or falling back to EIO. AP mode is unchanged.
- Commit 4148ac7315d4835b9818c555956d99b195bf4947 fixes shared create_file() to report both open and close errors. GNU89/Werror and ASan/UBSan policy tests cover the outcomes. The exact-head DNSMasq regression and broad Actions set above passed.
- Do not repeat these source audits or fixes.

## Active security slice — private, atomic OpenVPN client export

- Parent main SHA: 4148ac7315d4835b9818c555956d99b195bf4947.
- ovpn_server_expcli_main() previously wrote embedded client private-key material to predictable /tmp/client.ovpn using fopen("w+"), ignored fclose() and chmod(0600) results, and returned success even if certificate/key generation or PEM file reads failed. A pre-existing symlink could be followed and default file permissions could expose credentials.
- The change uses openvpn_config_file_open/publish for owner-only staging, fsync/checked close, atomic rename and final mode 0600. Export key reads use O_NOFOLLOW/O_NONBLOCK/O_CLOEXEC, regular-file owner/link validation, restrictive mode for embedded secrets, PEM BEGIN/END checks and read/close status. Certificate and TLS-Crypt-v2 key generation are checked. The generated working directory is a unique mkdtemp() 0700 directory; cleanup is checked. The post-publication unix2dos/chmod path is removed because OpenVPN accepts LF.
- Existing openvpn-client-secret Actions run the expanded source contract checker and the atomic config-file mode/symlink/failure regression. VPN server lifecycle and XY-C1 workflows also run for vpn_openvpn.c changes. Inspect exact-head results after the new commit.

## Exact first action for the next GPT

1. Resolve live main and inspect exact-head openvpn-client-secret, VPN server lifecycle stability, AI handoff documentation contract and XY-C1 Firmware Validation runs. Do not call the OpenVPN export slice validated until these exact-SHA runs finish.
2. For any failed check/build, retrieve that run's logs and fix the real issue without weakening private staging, no-follow key reads, generated-key validation, mode 0600 or cleanup ordering.
3. Then continue the next independently evidenced P1 reliability/security issue across Padavan userland. Do not reopen the closed DHCP, DNSMasq or create_file fixes.

## Verification boundary

- L1: a concrete OpenVPN private-key exposure and unchecked-generation path was identified and addressed in ovpn_server_expcli_main().
- L2: exact-head OpenVPN export source contract and existing atomic config publication regression are required on the new commit; no new host/board result is claimed before Actions.
- L3: XY-C1 run 44608598285 passed on parent 4148ac7315d4835b9818c555956d99b195bf4947; the new source commit requires fresh exact-head build verification.
- L4: physical OpenVPN export permissions and imported client connection remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with the exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
