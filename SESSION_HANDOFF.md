# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main at this slice's parent: `7ad1cb24f67b9c9dffafeb1a492b2a41343a8413`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged. No feature branches/PRs, force-push or history rewriting; do not touch GPT Session Rotator or replay SHA `9d5a5cb`.
- The checked VPN server callback fallback and both OpenVPN client callback source contracts passed on exact parent #44938615669; AI docs #44938615721 also passed.

## Active task

- A second runtime DNS policy bug remained outside OpenVPN: `ipup_vpnc_main()` read `vpnc_pdns` using `nvram_get_int()`. Backend failures collapsed to disabled and could overwrite `vpnc_dns_t` with an empty value and refresh the resolver.
- This atomic slice adds a shared checked boolean parser in `vpn_client_nvram_policy.h`, tests legacy missing/empty defaults, 0/1, whitespace, malformed/range/backend failures and output preservation, and migrates both PPP VPN `ipup_vpnc_main()` and OpenVPN `on_client_ifup()` to the same helper.
- When the optional policy read fails, both callbacks preserve existing DNS/domain snapshots and avoid resolver refresh, but still attempt link-state publication, route/balancing work and configured up-hook execution; they propagate the policy error.

## Verification boundary

- On `23520100e3cc74bb7f77a0d023afa361cded2abe`, XY-C1 #44937972691 completed Build XY-C1, TRX verification and artifact upload successfully on that same SHA. This proves L3 for that exact source snapshot only.
- Subsequent source commits `040b3da86f81007605c80a5687b7a516f41d19ce` and `88b8124bfa1363b8f6f066e66b4e2c6db80e26f6` each had XY-C1 runs #44938233497 and #44938409776 at Build XY-C1 and still in progress at the last poll. They are not yet L3-verified.
- Latest parent `7ad1cb24f67b9c9dffafeb1a492b2a41343a8413`: VPN lifecycle #44938615669 PASS; AI handoff docs #44938615721 PASS. It was a checker/handoff-only corrective SHA after the production server callback change; the next source commit below triggers fresh target CI.
- No local cross-build or local test execution is claimed. L4 hardware flash/boot/runtime remains NOT VERIFIED.

## Workflow and preservation rules

- Main only. For every multi-file logical task create all blobs, one complete tree, one commit and one non-forced ref update with the exact expected SHA. No per-file partial commits, branches, PRs, force push, amend or history rewrite.
- Re-fetch main immediately before writes; if moved, rebuild from its new tree. Verify only exact-head main push Actions. PENDING does not block independent tasks; diagnose FAIL from its first causal error.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Do not weaken source-contract or test assertions.
- `dev_log.md` follows a newest-first 20–119 entry rolling window. Root is rotated at 120: oldest 100 records move verbatim, oldest-first, to the dated archive; newest 20 remain at root. Archive index tracks bytes, headings and Git blob SHA. Preserve every existing archive byte-for-byte.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA VPN lifecycle, AI docs, NVRAM audit and XY-C1 Actions for this shared boolean parser / dual-callback change. Diagnose any test/build failure from its first causal error; PENDING may be followed by independent work.
2. Confirm same-SHA XY-C1 Build, TRX verification and artifact upload for the latest source tree before reporting L3; L4 remains unverified without physical hardware validation.
3. Continue another independent whole-userland reliability/security bug after this callback slice, rather than reopening already passing lease locks, OpenVPN credentials or completed source-contract fixes.
