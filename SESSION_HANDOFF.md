## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once in this session; do not repeat the initial audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. Only `backup/...` and `main` exist.
- Current main before this NVRAM secret-buffer hardening: `a606c54b09cda3fb70560e44571281ddd49c7f92`. Samba V2 journal production baseline is unchanged from `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`; a606's Samba durability and AI handoff checks both PASS. This commit adds volatile erasure for CLI save/restore/show value buffers and snapshots; exact-head NVRAM/XY-C1 validation is pending.
- PR #1 remains closed and unmerged; the CA trust-store feature is already integrated into main.

## Active task

- Security fix: NVRAM CLI profile buffers contain Wi-Fi, VPN and administrator credentials. `nvram_save()`'s `nvram_getall()` buffer, `nvram_restore()`'s decoded profile buffer and permanent/temporary snapshots, and `nvram_show()`'s value buffer were all freed without an explicit wipe. Add volatile full-region erasure before every free path, including malformed-profile and I/O failures.
- The CLI policy host regression now verifies full-region erasure and NULL/zero-length safety. The NVRAM semantic checker enforces that every `free()` in save/restore/show is immediately preceded by the matching wipe. Exact-head NVRAM workflow and XY-C1 image validation are required.
- Previous exact-head a606 checks: Samba V2 transaction/durability #46146532872 PASS; AI handoff #46146532867 PASS.

## Verification boundary

- Main a606c54b09cda3fb70560e44571281ddd49c7f92: Samba share #46146532872 PASS, including the C journal regression, account snapshot/failure-gate contracts and every V2 durability assertion; AI handoff #46146532867 PASS (root dev_log 113/119).
- New NVRAM CLI secret-buffer erasure source/test/checker change: THIS COMMIT; local tests NOT RUN because no local checkout exists. Exact-head NVRAM audit and XY-C1 Build/TRX/artifact pending.
- Production NVRAM kernel/userland transaction baseline and earlier NVRAM audit #46145970994 PASS remain historical; this new patch adds coverage for CLI heap copies not covered by the previous kernel scratch-buffer wipe checks.
- XY-C1 #46145600745 on production source SHA 670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a completed SUCCESS including Build, TRX and artifact upload. It predates this CLI binary change and does not validate THIS COMMIT.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch the live main SHA and inspect its NVRAM transaction-boundary check run. Require the existing GNU89 CLI policy binary to report `NVRAM CLI sensitive-buffer erasure: PASS` and the source checker to confirm every save/restore/show free is preceded by a full-buffer wipe.
2. Inspect exact-SHA XY-C1 Build, TRX and artifact. This modifies `trunk/user/nvram/src/nvram.c`, so the previous host-only/checker SHA is not sufficient for L3; record the result only on this production-input SHA.
3. If NVRAM checks pass, continue one independent security/stability slice outside the now-covered CLI profile-buffer lifecycle; do not repeat the NVRAM broad inventory.
4. Preserve the frozen backup, main-only atomic commits, and update this handoff only when the active task/first action materially changes. Hardware L4 remains NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
