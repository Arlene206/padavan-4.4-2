# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent main: `418373c53d35051fc316c67f119d372126a08f89`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Active engineering slice

- The prior source commit `418373c53d35051fc316c67f119d372126a08f89` migrated PPP VPN and OpenVPN callbacks to the shared checked `vpn_client_nvram_read_bool_status()` reader. Its GNU89 policy test passed, but the OpenVPN callback retained a stale argument tail from the former parser invocation; this commit removes that invalid tail and updates the VPN source contract to assert the new shared helper exactly once.
- The same parent AI handoff check found `docs/dev-log/2026/11/part-023.md` indexed as 109434 bytes while the immutable archive is 109696 bytes. This commit corrects only the index byte count; the archive blob and bytes remain untouched.
- Parent 418 Actions: OpenVPN credential staging #44939243151 PASS; DNSMasq lifecycle #44939243079 PASS. VPN lifecycle #44939242958 failed at the stale OpenVPN helper source assertion (after all compiled policy tests passed). AI handoff docs #44939242988 failed only at the part-023 byte count. XY-C1 #44939242892 was still preparing the build before Build XY-C1 at last poll; NVRAM #44939243070, Firewall #44939242926, HTTPD #44939242904, WAN route #44939242908 and Process inventory/lifecycle #44939243154/#44939242924 were pending.
- No exact parent 418 target Build/TRX/artifact PASS is established. L4 hardware flash/boot/runtime remains NOT VERIFIED.

## Exact first actions for the next GPT

1. Inspect exact-head VPN lifecycle, AI docs, NVRAM and XY-C1 Actions for this repair commit. The VPN run must pass the shared boolean helper contract; AI docs must accept part-023's indexed 109696-byte size. XY-C1 must pass Build, TRX verification and artifact upload on the same exact SHA before L3 is claimed.
2. If checks pass, continue the next OpenVPN export-profile security/functionality issue: enable `remote-cert-tls server` (the generated server certificate carries `serverAuth` EKU) and add validated IPv4 endpoint fallback only for dual-stack modes 4/5 after DDNS and IPv6 lookup fail. Keep IPv6-only modes unchanged.
3. After that independent source slice, switch to the highest-risk unfinished reliability/security defect in another Padavan userland module.

## Workflow and preservation rules

- Develop only on `main`. For each logical slice create all blobs, one complete Git tree, one commit and one non-forced expected-SHA ref update. Never force-push, amend history, create branches or PRs.
- Re-fetch main at the write boundary; if it moves, rebuild from its new tree. Inspect only exact-head push Actions. PENDING does not block independent work; diagnose FAIL from its first causal error.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Do not weaken tests/source contracts or claim tests/builds not run.
- Root `dev_log.md` remains a newest-first rolling window of 20–119 records; archive the 100 oldest complete entries when the next entry would make 120. Preserve immutable archive files and index integrity.

