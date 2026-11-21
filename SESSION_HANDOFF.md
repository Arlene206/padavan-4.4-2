# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development and validation branch: `main`.
- Main at takeover: `5365b10a5136e479e9db91079eb525634c3fcfa8`; this commit added checked OpenVPN server export snapshots but exposed a MIPS build blocker in an older xl2tpd caller of the changed VPN address-pool API.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable and must not be modified or deleted.
- PR #1 is closed/unmerged and is not a development path. Do not create branches or PRs. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Active task

- XY-C1 exact-head Build run #44936762599 on `5365b10a5136e479e9db91079eb525634c3fcfa8` failed compiling `trunk/user/rc/src/net_ppp.c`: `safe_start_xl2tpd_locked()` still called `get_vpns_pool(nvram_get_int("vpns_vuse"), ...)` with the obsolete five-argument signature. The current checked API takes four output pointers and reads `vpns_vuse` inside its own exclusive NVRAM transaction.
- Corrective slice: use four output pointers, preserve the validated result for all generated L2TP address fields, and complete the pool read before `fopen("/etc/xl2tpd.conf", "w")`. On read/parse/end-transaction error, preserve errno and return before truncating or publishing the live config. The VPN lifecycle source contract verifies the API, ordering and fail-closed path.
- This fix is the current main-tip source slice; fetch live main rather than relying on a hard-coded self-SHA. The prior failing run above is the confirmed cause; exact-head Actions for the corrective commit must be inspected before treating it as verified.

## Verification boundary

- On the failed parent SHA, VPN server lifecycle #44936762607, OpenVPN client credential staging #44936762612, Process inventory #44936762633, HTTPD security #44936762655, NVRAM audit #44936762657 and AI handoff docs #44936762626 all passed on exact push SHA `5365b10a5136e479e9db91079eb525634c3fcfa8`. XY-C1 #44936762599 failed at the cross-compile error above; TRX and artifact upload were skipped.
- The corrective production source must be validated by exact-head main push Actions. L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all pass on one SHA. L4 hardware flash/boot/runtime validation remains NOT VERIFIED.
- Do not report local cross-compilation or tests unless actually executed. Host regression and source-contract success do not substitute for MIPS build or hardware validation.

## Workflow and preservation rules

- Main only; no feature/fix/checkpoint branches and no PR-based development. For each multi-file logical slice create all blobs, one tree, one commit and a non-forced main ref update with the exact expected SHA. No per-file partial commits, force push, amend, or history rewrite.
- Re-fetch main immediately before each write. If it moved, rebuild the patch from the new head. Verify only Actions whose push event and head SHA match the published main commit.
- PENDING checks do not block independent engineering. Fix relevant FAILs from the first causal log error before calling a slice verified.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Do not weaken regression coverage or workflow path filters.
- `dev_log.md` retains 20–119 newest-first entries. Batch archive oldest 100 before reaching 120; rotate early above 384 KiB and never exceed 512 KiB. Preserve all archives byte-for-byte. Update the log for real milestones; update roadmap only when repository-wide status/priority changes; do not add routine handoff/roadmap snapshots.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head push Actions for the current xl2tpd pool-snapshot fix. Diagnose any failing VPN lifecycle or XY-C1 run from its first causal error; require Build + TRX + artifact on the same SHA for L3.
2. Continue the prior OpenVPN task: inspect `ovpn_server_script_main()`, `on_client_ifup()` and `on_client_ifdown()` in `trunk/user/rc/src/vpn_openvpn.c`, especially runtime DNS/WINS/DOMAIN and `vpnc_pdns` reads. Preserve mandatory DNS restoration, route teardown and hook cleanup when optional NVRAM policy reads fail; add meaningful regression coverage.
3. Compare exported server profile device/protocol/port/TLS/auth/cipher/compression output to the checked server network/options snapshots without reopening completed CCD ACL, credentials, or client option snapshots absent a same-SHA regression.
