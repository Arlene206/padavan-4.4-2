# 2026-11-18 continuation checkpoint — fail-closed factory reset and exact-SHA validation

## Current repository state
- Parent main before THIS factory-reset source slice: `80dcdc629d72c11136e1b766fe730e0d33f21bd4`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- MiniUPnP lifecycle serialization is implemented in production commit `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`; public start/stop/WAN/update/restart wrappers hold the stable lock across internal `*_locked` operations.
- Exact `dc6d32df` Process integration run `44195398478`: PASS after lock-aware contracts were synchronized. The dedicated MiniUPnP rerun `fa36ad18` run `44195714916` and AI handoff `44195714933` were IN_PROGRESS at the last poll. The preceding dedicated run `44195395518` failed to create a job because an assertion escaped YAML heredoc indentation; parent `fa36ad18` corrects that indentation.
- XY-C1 production source baseline `88770d79` completed exact push run `44193165931` SUCCESS, including Build XY-C1, TRX verification and artifact upload. THIS factory-reset source slice modifies runtime C code and requires a new exact-head XY-C1 run before L3 can be claimed.
- No local MIPS toolchain or full checkout is available in this session. Physical-router L4 remains NOT VERIFIED.

## Active task
- The previous `erase_storage()` returned `void` and discarded `doSystem("/sbin/mtd_storage.sh erase")`; both reset-button paths ignored NVRAM reset status and unconditionally signalled PID 1. This could reboot after an incomplete MTD erase and leave mixed persistent reset state.
- THIS slice makes flash erase status observable, runs storage erase before committing the NVRAM reset marker, and asks PID 1 to exit only after both persistent reset operations succeed. Every abort attempts to restore configured rstats; the physical reset-button handler also restores the configured power LED on failure.
- A GNU89 policy regression covers storage/NVRAM failure gates. The GPIO workflow source checker verifies C call ordering, MTD script exit-status propagation, long-press path unification and failure recovery.

## Verification boundary
- Exact-head GPIO regression and XY-C1 Build/TRX/artifact for THIS source slice are pending until this commit's push runs finish.
- Exact `dc6d32df` Process integration run `44195398478`: PASS. Exact `fa36ad18` MiniUPnP run `44195714916` and AI handoff run `44195714933` were IN_PROGRESS at last poll.
- Exact `88770d79` XY-C1 run `44193165931`: SUCCESS (Build, TRX and artifact upload passed) for the source before THIS factory-reset change.
- Local host compile: NOT RUN. The new ASan/UBSan/GNU89 policy test is delegated to Actions; do not describe it as passed until exact-head CI completes. Physical-router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; inspect exact-`head_sha` push runs for GPIO button lifecycle, Process lifecycle, MiniUPnP, AI handoff and XY-C1 for THIS commit.
2. For GPIO, require both GNU89/ASan policy regressions and the Python source-order contract to pass. For XY-C1, require Build XY-C1, TRX verification and artifact upload; do not infer this from `88770d79`.
3. Repair the first causal failure in a separate atomic main commit; while a firmware build is pending, continue an independent P1 reliability/security issue.
4. Keep the backup read-only and hardware L4 marked NOT VERIFIED.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
