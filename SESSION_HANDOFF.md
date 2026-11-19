# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main at checkpoint: `29fa24741613095593a03f4e602f1d77636a21e0` (`fix(wan): remove stale off-subnet gateway host route`).
- Baseline `28fa4dfa2855fd49ea857e3f6ea2f655180beed6` is fully verified for the preceding USB slice: USB `44582892745`, NVRAM `44582892741`, AI docs `44582892806`, HTTPD `44582892824`, Process inventory `44582892966`, and XY-C1 `44582892773` all PASS; XY-C1 passed production-input checks, Build XY-C1, TRX verification and artifact upload.
- On exact SHA `29fa24741613095593a03f4e602f1d77636a21e0`, WAN route run `44585450279` FAILED only at `Verify route result propagation and cleanup ordering` line 275. The GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` route policy binary compiled and ran successfully first. The Python contract compared a whitespace-sensitive helper-call string after `" ".join(split())`; source wraps the first argument onto a new line. The follow-up normalizes all whitespace for this call in both up/down contracts; no production C defect was shown by this run.
- Other exact-head `29fa...` checks completed PASS: WAN teardown `44585449784`, USB modem `44585449799`, WAN gateway ARP `44585450307`, detect_internet `44585449958`, VPN server `44585449754`, detect_link `44585449806`, ZeroTier `44585449938`, AI docs `44585449951`, Process lifecycle `44585449805`, HTTPD `44585450515`, miniupnpd `44585449714`, DNSMasq `44585449946`, WAN DHCP `44585449832`, Wireless radio `44585450126`, Process fail-closed `44585449736`, Firewall `44585450099`, watchdog `44585449709`, and NVRAM `44585450024`.
- XY-C1 firmware validation run `44585450325` was IN_PROGRESS at the last poll; do not claim L3 for `29fa...` until detector, Build, TRX and artifact steps are all confirmed.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never touch it. PR #1 remains closed and unmerged; do not revive or re-audit.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical WAN/router L4 remains NOT VERIFIED.
## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — repair the WAN route source-contract whitespace assertion

- Parent main SHA: `29fa24741613095593a03f4e602f1d77636a21e0`.
- The off-subnet gateway host `/32` lifecycle correction is already in production source: the same valid/non-PPP/off-subnet predicate guards setup and teardown; teardown attempts its host-route delete after default-route removal, normalizes ESRCH only, aggregates hard errors and proceeds to configured static routes.
- First exact-head WAN route run `44585450279` compiled and ran the GNU89/Werror/ASan/UBSan route policy regression successfully, then failed the Python source contract at script line 275. Root cause: the checker searched for `required(gateway_valid...` after joining lines with spaces, but the C call is split after `(`. This is a test-checker defect, not a route-policy binary failure. The same assertion exists for both WAN up and down; normalize all whitespace in both before matching.
- On `29fa...`, WAN teardown `44585449784`, WAN DHCP `44585449832`, USB modem `44585449799`, WAN gateway ARP `44585450307`, AI docs `44585449951`, HTTPD `44585450515`, NVRAM `44585450024`, Process lifecycle `44585449805`, Process fail-closed `44585449736`, and the other listed stability suites passed. XY-C1 `44585450325` remained pending at the checkpoint.
- After checker repair, inspect new exact-head WAN route and XY-C1 runs. L2 requires the source contract plus policy binary; L3 requires production input detector, Build XY-C1, TRX and artifact upload.
## Verification boundary

- L1: source review confirms the `/32` install/removal predicate symmetry and error aggregation.
- L2: policy binary is PASS on `29fa...`; exact WAN route source contract failed only because of whitespace-sensitive matching. Checker normalization is committed in this follow-up; new exact-head route run pending.
- L3: exact-head XY-C1 detector/build/TRX/artifact for the WAN production-source change pending (`44585450325` was still running at last poll).
- L4: live lease loss/renew, gateway reachability and hardware route-table validation NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head WAN route and XY-C1 Actions on the checker-normalization follow-up. Confirm the route source contract and sanitizer policy binary pass, then confirm detector, Build, TRX and artifact on the product SHA.
2. If the WAN route checker still fails, inspect the first assertion location; do not weaken route behavior or remove PPP/subnet exclusions.
3. Once L2/L3 are closed, select the next independent P1 stability/security defect outside this WAN/USB slice.
## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

