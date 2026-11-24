## Current repository state

# Cross-session execution mandate

Continue this repository as one cross-session engineering stream. A logical multi-file change lands as one complete Git tree and one commit on `main`, followed by exact-SHA push validation. Never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow does not block independent safe work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify `main` and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record material state transitions, and name the next concrete implementation task. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the files untouched and create no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not performed. Before context exhaustion, finish the current safe slice and preserve the exact verification boundary and first action.

- Live main before this checker correction: `c522dc20b504eac7a7c701d341034752ac3f0ec1`. Re-fetch the exact tip before every write; the handoff records the engineering baseline and need not chase documentation-only commits.
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

The active failure is a compact-source token mismatch in `scripts/check_udhcpc_script_status.py`: after whitespace removal, `else if(state==DECLINE_PENDING)` is `elseif(state==DECLINE_PENDING)`. THIS correction changes only that expected token; DECLINE_PENDING and the subsequent non-discovery listener/timeout checks remain intact. Once the exact-head suite passes, continue the broader DHCP pending-release integration boundary, then rotate to other high-risk userland modules.

## Verification boundary

- Exact `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e` udhcpc #45480041494: GNU89 ASan/UBSan C regression PASS; Python checker failed at line 238 because the scheduler assertion had not moved with the tested retry-plan helper. Subsequent correction at `c522dc20` moved that checker contract to the plan initializer.
- Exact `c522dc20b504eac7a7c701d341034752ac3f0ec1` udhcpc #45480285457: GNU89 ASan/UBSan C regression PASS; Python checker failed at line 276 on the whitespace-compacted `else if` branch token. THIS correction fixes that token only. AI handoff #45480285572 was IN_PROGRESS at last observation.
- On `fd59df3`, AI handoff #45480041430, Process inventory #45480041644, Process lifecycle #45480041481 and USB storage #45480041346 PASS; NVRAM #45480041721 and XY-C1 #45480041423 were IN_PROGRESS at last observation. Inspect remaining results by exact SHA; do not attribute them to this checker-only correction.
- Prior L3 VERIFIED source/test tree: `36a10c0e5844daf31b3cc483c20d9e25a798ef6b`, XY-C1 #45476128451 Build, TRX verification and artifact upload all PASS. Product-source changes on `fd59df3` are newer and still require same-SHA target-build evidence. Hardware/router L4 remains NOT VERIFIED.
- No local repository checkout, host compile or MIPS build was performed in this session.

## Exact first action for the next GPT

1. Inspect exact-head udhcpc Actions for THIS branch-token checker correction. Require GNU89 `-Wall -Wextra -Werror` ASan/UBSan and all Python contracts to PASS; triage the first failure without loosening semantic assertions.
2. Inspect the exact-head AI handoff result and close out the pending `fd59df3` XY-C1, NVRAM, Process and storage runs; only same-SHA Build/TRX/artifact establishes L3.
3. Once this test gate passes, continue the DHCP pending-release/exit integration case, then rotate to the next highest-risk unaudited userland stability issue from `REFACTOR_ROADMAP.md`. Keep the frozen backup read-only and update only `main` by expected-SHA non-force CAS.
