# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice lands as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main: `040b3da86f81007605c80a5687b7a516f41d19ce`. Recent source slices repair the checked four-output VPN pool caller in xl2tpd and make OpenVPN client pushed-DNS publication fail closed on unreadable `vpnc_pdns`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged and is not a development path. Do not create branches/PRs, force-push, amend history, touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Active task

- `ovpn_server_script_main()` still used `nvram_get_int("vpns_ov_mode")` for the missing-`dev` fallback. Backend/malformed reads became TAP (0), which could create the wrong lease interface for a callback.
- Corrective slice: treat OpenVPN's supplied `dev` as authoritative and avoid the NVRAM read when it exists. Only for legacy callbacks with no `dev`, use `nvram_get_checked()` and strict 0/1 parsing. If the fallback is unreadable, do not invent TAP; still dispatch valid connect/disconnect callbacks and hooks. Lease disconnect matching is address-based, so it may remove a lease without the missing interface name; lease additions continue to require a real interface.
- Existing VPN lifecycle source-contract workflow verifies the device fallback, disconnect behavior and hook execution ordering.

## Verification boundary

- On parent SHA `23520100e3cc74bb7f77a0d023afa361cded2abe`, VPN lifecycle #44937972537, AI docs #44937972518, WAN route #44937972672, WAN DHCP #44937972690, HTTPD security #44937972661, Process inventory #44937972749 and NVRAM audit #44937972629 passed. XY-C1 #44937972691 remained in progress at the last observation.
- The subsequent checked-`vpnc_pdns` commit is `040b3da86f81007605c80a5687b7a516f41d19ce`; its push Actions had not appeared in the latest Actions listing. Recheck exact-head Actions for it alongside this callback slice; do not infer PASS from an absent run.
- XY-C1 L3 is NOT VERIFIED until Build, TRX and artifact upload all pass on one exact SHA. L4 hardware flash/boot/runtime validation remains NOT VERIFIED. Do not claim local tests or cross-builds unless executed.

## Workflow and preservation rules

- Main only. For each logical slice create all blobs, one tree, one commit and a non-forced ref update with the exact expected SHA. No partial per-file commits, branches, PRs, force push, amend or history rewrite.
- Re-fetch main immediately before each write; if moved, rebuild from the new parent. Only exact-main push Actions count as verification. PENDING does not block independent work; fix FAIL from the first causal log error.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, NVRAM defaults, security and device compatibility. Do not weaken tests or workflow paths.
- `dev_log.md` retains 20–119 newest-first records. Before record 120, archive oldest 100 in one batch, leaving 20. Rotate early above 384 KiB and never exceed 512 KiB; preserve archive/index bytes. Update log on real milestones, roadmap only for material project-wide status changes; no routine handoff snapshots.

## Exact first action for the next GPT

1. Fetch live main. Inspect exact-head VPN lifecycle, OpenVPN client NVRAM, AI docs, NVRAM audit and XY-C1 runs for commits `040b3da86f81007605c80a5687b7a516f41d19ce` and the current server callback fix. Diagnose new failures from their logs.
2. Require same-SHA Build XY-C1, TRX verification and artifact upload for L3. Do not use the earlier d0 firmware pass as evidence for these newer source commits.
3. Review exported server profile parity against the checked server topology/options snapshots (device/protocol/port/TLS/auth/cipher/compression); do not reopen completed CCD ACL, credential staging, or client settings snapshots absent a same-SHA regression.
