# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with this handoff: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, conclude without a file change/commit. Record relevant exact-head Action state changes when they alter validation gates.

## Current repository state
- Parent before HTTPD contract correction: `b284c28532f2913f4327b02ec59abaf97671be08`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- NFS service-enable fix commits `ce227363` + placement correction `b284c285`. Exact NFS lifecycle #44903863601 PASS on `b284c285`; its prior failure #44903638098 on `ce227363` exposed the mislocated preflight, now corrected and covered by assertions. Restart now checks `nfsd_enable` after storage lock acquisition and before mount inventory/stop/unload; checked value is reused during restart.
- HTTPD umbrella #44903863573 failed on `b284c285` only because its duplicated NFS source contract still expected the old wrapper shape. Checked config is now read in `run_nfsd_locked()` / `reload_nfsd_locked()`, while export generation and launcher execution live in their new `*_enabled_locked()` functions. This commit updates HTTPD assertions without dropping the checked-read or launch-order checks.
- On `b284c285`, NFS #44903863601 PASS, AI docs #44903863682 and Samba #44903863669 PASS. HTTPD rerun pending after this contract update. XY-C1 #44903863658 was still in Build, TRX/artifact pending; NVRAM audit #44903863567 in progress.
- MiniUPnP checked transactional config gate commit `0b0df9fd`: MiniUPnP lifecycle and AI docs passed on exact SHA, as did Samba, Process, HTTPD, WAN route, DNSMasq, WAN DHCP and Inadyn.
- WAN route #44902460823 / docs #44902460812 PASS on `dffca184`; DNSMasq #44902327424 / docs #44902327438 PASS on `5a436698`.
- Historical XY-C1 #44902070311 passed Build/TRX/artifact on source `232e31d7` only, not later product commits. Physical router L4 NOT VERIFIED.
- Legacy PR #1 closed/unmerged; frozen backup unchanged.

## Active task
1. Inspect exact-head HTTPD umbrella after its NFS contract update.
2. Check XY-C1 Build, TRX verification, artifact upload and NVRAM audit for the current product-source SHA. Do not claim L3 until all three firmware stages pass on the same product SHA.
3. If any Action fails, inspect first causal log and repair without weakening assertions. Continue independent P1 work while actions are PENDING.

## Verification boundary
- NFS lifecycle exact head PASS on `b284c285`; source policy binary passed on the preceding failed source-contract run too.
- HTTPD umbrella failure was a stale source checker (old function placement assumptions); its follow-up is pending.
- XY-C1 Build was in progress on `b284c285`; TRX/artifact were pending. NVRAM audit in progress.
- No local RC compile, current-source firmware/TRX success or hardware L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and inspect the exact HTTPD umbrella rerun.
2. Inspect XY-C1 Build/TRX/artifact and NVRAM audit for current production source.
3. Continue a distinct P1 userland issue while PENDING work proceeds independently.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first action materially changes; no no-op commit.
