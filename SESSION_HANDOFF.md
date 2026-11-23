# Cross-session execution mandate

Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic tree and one commit, followed by a non-forced compare-and-swap ref update. Do not ask the user what to do next.

Before context exhaustion, finish the current safe slice, inspect exact-head Actions, and leave the next task, commit SHA, test evidence, and unresolved CI states here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Record only material changes: a relevant exact-head Action changing from PENDING to PASS/FAIL, a new commit, an unresolved failure, or a changed first action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not manufacture documentation churn or a no-op commit.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; writable branch is `main` only.
- Main immediately before this handoff correction: `e4d34fd7f13baa203bbbae90130b0782a9b22e5c`; fetch live main for the resulting commit.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserve it unchanged.
- PR #1 is closed and unmerged. Its AliDDNS CA trust-store and HMAC helper/security work is already represented or superseded on main; do not reopen the divergent PR.
- No local checkout or MIPS/uClibc toolchain is available in this session. No local build, firmware/TRX, artifact, or physical-hardware result is claimed.

## Active task

### USB printer retry idempotency and checker alignment

- `trunk/user/rc/src/services_usb.c` routes U2EC/LPD startup through a shared launcher. It scans process presence before PID cleanup/fork, treats RUNNING as idempotent success, fails closed on UNKNOWN, and removes stale PID files only for ABSENT. U2EC/LPD pass daemon-specific PID-file paths and do not unlink their PID files in the wrappers.
- USB modem run #45220033428 on `e6dc19c7c8ddf9c185d48817c28dc952412028b9` initially failed because `scripts/check_usb_printer_nvram.py` still expected caller-side unlink. Commit `f9475bf081db66e795df8c25332253ef7c6c80a2` aligns that checker with the shared launcher and fixes the LPRng PID path literal. Exact USB modem run #45220509149 on f947 PASS, including the formerly failing NVRAM source-contract step.
- Process fail-closed #45220191260, NVRAM audit #45220191315 and USB storage #45220191366 PASS on `ecff475abb782b32e0a8f4f02026acc709ea6844`.

### APCLI/WISP cache lifetime

- APCLI startup uses checked SSID/WISP reads and snapshots SSID presence before the next NVRAM read, avoiding retention of cache-owned text pointers. Exact source SHA `ab4b706f220bcdc84310063936bd074802876a7f` passed Wireless, USB modem, Firewall, Process, NVRAM, HTTPD, watchdog, iNIC, Process inventory and AI docs checks.

### WAN gateway ARP egress identity

- `detect_wan` snapshots the egress interface through checked NVRAM/bridge text, rejects empty or overlong names before socket creation, then copies the exact name into both `ifreq.ifr_name` and Linux 4.4 `SOCK_PACKET`'s 14-byte `sockaddr.sa_data`. It requires device binding and a finite receive timeout. Host policy and normalized source-contract checks PASS on `40ab459a16dfb914d571c8f559723e907a1b59e9` (#45218700965); AI docs #45218701533 PASS.
- The XY-C1 workflow path now includes `detect_wan_arp_socket_policy.h` beside `detect_wan.c`, closing a header-only trigger gap. The workflow remains push-only on main.

## Verification boundary

- USB printer checker: PASS on f947 / #45220509149.
- Process fail-closed, NVRAM audit and USB storage: PASS on ecff / #45220191260, #45220191315, #45220191366.
- XY-C1 #45220033426 on e6dc and #45220191233 on ecff remained IN_PROGRESS at last observation, with Build active and TRX/artifact pending. These do not yet establish L3.
- XY-C1 #45221089272 on workflow/documentation commit e4d34 is IN_PROGRESS; Build/TRX/artifact step outcomes must be inspected. The AI handoff contract #45221089290 FAIL on e4d34 because this file did not include the mandatory end-of-session accuracy checkpoint; THIS correction adds its required terms and structure. Exact rerun is pending.
- Physical router / USB printer runtime (L4): NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect the exact-head AI handoff documentation run for THIS correction. Confirm it passes all handoff, roadmap, and log archive contracts; if it fails, fix the first causal assertion without dropping any required governance rule.
2. Poll XY-C1 runs #45220033426 (e6dc), #45220191233 (ecff), and #45221089272 (e4d34). Claim L3 only for a single exact product-source SHA where Build XY-C1, TRX verification, and artifact upload all succeed. Distinguish host-only documentation checks from target builds.
3. Continue the repository-wide stability/security queue from the current roadmap; do not repeat APCLI, WAN ARP, or printer investigations absent new evidence.
4. Keep one complete tree per logical task and update main using a non-forced expected-SHA compare-and-swap. Preserve the frozen backup.

## Cross-session operating constraints

- Recover once, then continue implementation rather than repeated audits or planning.
- Use source change → relevant test → one atomic main commit → exact-SHA Actions. PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc, router configuration, security and backward-compatibility constraints.
- Before context exhaustion, leave accurate commit/test/CI facts and the first action above. Do not claim work that was not verified.

