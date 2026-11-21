# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with this handoff: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, conclude the checkpoint without a file change or commit. Record relevant exact-head Action status changes when they alter validation gates.

## Current repository state
- Parent before NFS restart preflight correction: `ce227363a7cd0aee0beab112bca9773a79e4630f`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- NFS enablement fix initially had a misplaced preflight block: exact NFS Action #44903638098 on `ce227363` failed only at source checker after GNU89 policy tests passed. Root cause was enable read inserted in `storage_has_mounted_devices()`, leaving `restart_nfsd()` stopping before checking. This correction relocates it directly after lock acquisition and before inventory/stop/unload, and asserts the unrelated inventory function stays clean.
- MiniUPnP checked service-policy fix `0b0df9fd`: MiniUPnP lifecycle #44903303300 and AI docs #44903303195 PASS; Samba, Process, HTTPD, WAN route, DNSMasq, WAN DHCP and Inadyn exact-head gates PASS. Earlier XY-C1/NVRAM gates were pending at checkpoint.
- WAN route #44902460823 and AI docs #44902460812 PASS on `dffca184`; DNSMasq #44902327424 and AI docs #44902327438 PASS on `5a436698`.
- Older XY-C1 #44902070311 on product SHA `232e31d7` passed Build/TRX/artifact for that historical baseline only; not current source. Physical L4 remains NOT VERIFIED.
- Legacy PR #1 closed/unmerged; backup unchanged.

## Active task
1. Inspect exact-head NFS regression after this placement correction, plus current NVRAM audit and XY-C1 Build/TRX/artifact runs.
2. If NFS fails, read the exact causal log. `restart_nfsd()` must checked-read after lock acquisition and before mount count/stop/unload; `storage_has_mounted_devices()` must contain no NFS config state.
3. Continue independent P1 reliability/security work while Actions are PENDING; do not repeat completed audits.

## Verification boundary
- On `ce227363`, NFS policy compile/run passed but source-contract failed at restart-order assertions; this repair awaits exact-head rerun.
- AI docs #44903638076 and Samba #44903638071 PASS on `ce227363`. NVRAM audit #44903638191 and XY-C1 #44903638079 were IN_PROGRESS at last poll.
- No local RC compile, current-source MIPS firmware/TRX success or hardware L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and exact-SHA NFS regression result; verify source-contract PASS after relocation.
2. Inspect NVRAM audit and XY-C1 Build/TRX/artifact results for the same SHA.
3. Fix any exact-head FAIL at its cause and continue independent P1 userland work while PENDING.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first action materially changes; no no-op commit.
