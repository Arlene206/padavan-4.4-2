## Current repository state

# Cross-session execution mandate

# Cross-session execution mandate

Each source/test slice is one atomic main commit: construct one complete Git tree, create one commit, then update main by a non-forced expected-SHA compare-and-swap. Continue this repository as one cross-session engineering stream. A logical multi-file change lands as one complete Git tree and one commit on `main`, followed by exact-SHA push validation. Never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow does not block independent safe work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify `main` and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record material state transitions, and name the next concrete implementation task. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the files untouched and create no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not performed. Before context exhaustion, finish the current safe slice and preserve the exact verification boundary and first action.

- Parent main before this VSFTPD source slice: `f43b856869a0ad94bae5fd15a1e14daff1f8ec1f`. Re-fetch live main before every write.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- PR #1 remains closed/unmerged and deeply behind main. Valid AliDDNS CA-bootstrap/HMAC evolution is already integrated on main; do not port the stale PR wholesale.
- Only `main` is authorized for development and validation. Use one complete tree, one commit, and a non-forced expected-SHA ref update.

## Active task

### WAN DHCP client state-transition reliability — BusyBox udhcpc

Relevant files:
- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`
- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/udhcpc_script_policy.h`
- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/tests/udhcpc_script_policy.c`
- `scripts/check_udhcpc_script_status.py`
- `.github/workflows/udhcpc-script-status-stability.yml`

The main pending-deconfig retry scheduler now prepares a single tested plan (resume state, release intent, retry timeout, packet counter and elapsed-wait counter) before closing the listener, then publishes it after switching to `LISTEN_NONE`. C regression and source contracts cover nonpositive retry floors, resume-state/socket/timer setup, lease expiry, NAK server filtering, SIGUSR2 and `-R` cleanup.

The DHCP retry-plan/state-transition slice passed its exact-source gate on descendant `b2dab6b05fb2cf07abbdae01b1acff2cc134df31` (udhcpc #45480533291). Its product-source parent `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e` also passed XY-C1 Build, TRX verification and artifact upload (#45480041423), plus Process inventory #45480041644, Process lifecycle #45480041481, USB storage #45480041346, NVRAM #45480041721 and AI handoff #45480041430. DHCP source L3 is VERIFIED; hardware/router L4 is NOT VERIFIED.

The current source slice addresses VSFTPD firewall cleanup: a stop command can return an error even when the final process inventory proves `vsftpd` absent. Standalone stop and restart previously gated FTP firewall/UPnP reconciliation on the collapsed status==0 value, so a partial stop error could leave stale mappings. The locked stop now exports final tri-state presence independently; callers reconcile only on positively confirmed absence while preserving stop errors and fail-closed restart/eject behavior.

## Verification boundary

- Exact `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`: initial udhcpc #45480041494 had C GNU89/ASan/UBSan regression PASS and a stale Python assertion failure. After checker-only corrections, descendant `b2dab6b05fb2cf07abbdae01b1acff2cc134df31` udhcpc #45480533291 PASS (C regression and Python source contract).
- Exact `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`: XY-C1 #45480041423 PASS, including Build XY-C1, TRX verification and artifact upload. Process inventory #45480041644, Process lifecycle #45480041481, USB storage #45480041346, NVRAM #45480041721 and AI handoff #45480041430 PASS. L3 VERIFIED for this DHCP product-source SHA; hardware/router L4 NOT VERIFIED.
- Descendant `b2dab6b05fb2cf07abbdae01b1acff2cc134df31` AI handoff #45480533158 failed only the literal-string validator; docs-only descendant `f43b856869a0ad94bae5fd15a1e14daff1f8ec1f` AI handoff #45480803456 PASS.
- THIS VSFTPD stop/firewall source slice has no local compiler/MIPS result. Exact-head VSFTPD workflow and triggered dependent checks are pending; do not claim target verification until actual runs finish.

## Exact first action for the next GPT

1. Inspect exact-SHA `vsftpd-stability.yml` after this commit; require all GNU89 `-Wall -Wextra -Werror` binaries and Python FTP lifecycle/ACL source contracts to pass.
2. If a contract fails, fix only the matcher/integration contract that disagrees with the final-presence output argument and failed-stop cleanup branch. Keep tests proving UNKNOWN/RUNNING never authorize firewall cleanup.
3. Continue the VSFTPD upload-cleanup TOCTOU investigation only if a Linux 4.4-compatible atomic fix is demonstrable; otherwise preserve the residual-risk disclosure. Do not re-audit completed DHCP work.
