# Cross-session execution mandate

Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers and precise first action. If the checkpoint remains accurate, do not make a documentation-only commit.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

Before context exhaustion, finish the current safe slice, check exact-head Actions and preserve the precise first next action here.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; product development remains on `main` only.
- Current `main` source baseline: `8f0d792ec0ab7569ffd1ecf7216008b2396b2faf` (`fix(storage): preserve integrated startup retry events`). This commit is the storage retry fix; the current follow-up removes only four erroneous duplicate workflow assertions, with no product-source changes.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only, never modify or delete.
- PR #1 (`fix/aliddns-ca-bundle-bootstrap`) is closed and unmerged, 17 commits ahead of its base. Current main has independently changed the CA bootstrap implementation; the PR branch is archival, not a development base.

## Active task — USB storage integrated-service startup retry

- `start_stor_apps()` returns the first required failure while continuing independent FTP/Samba/NFS and optional media starts. Its duplicate declaration defects were fixed in `0111177585699cfacbf7878d1554eeb6bc861537`; exact-head XY-C1 run `43888486194` passed Build, TRX verification and artifact upload on that then-current source SHA. Subsequent source changes mean this result does not establish L3 for the current code.
- Remaining defect fixed here: `try_start_stor_apps()` formerly cleared `usb_hotplug_ms` even when the integrated daemon startup result was nonzero. The deferred callback then scheduled `alarm(5)` but had already discarded its retry event. The new path clears the flag only on status 0 and keeps failure pending.
- The Optware child result is consumed once before retrying integrated daemons. If the asynchronous child is still running, the callback returns before restarting FTP/Samba/NFS/media daemons; the Optware script is still launched only after releasing `storage.lock`.
- Regression coverage: `trunk/user/rc/src/tests/service_start_status.c`, `trunk/user/rc/src/service_start_status.h`, and `.github/workflows/storage-hotplug-stability.yml`.

## Verification boundary and current blockers

- Local host service-start policy regression (from the 8f0 source change): GNU89, `-Wall -Wextra -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer: PASS, including positive/negative startup failure retention.
- Exact-head Actions for `8f0d792ec0ab7569ffd1ecf7216008b2396b2faf`:
  - USB storage hotplug `43893228398`: FAIL at the whole storage caller graph step. The causal assertion is a duplicate pair of incorrectly double-escaped newline checks in the workflow (`"\\nint\\nstart_stor_apps(void)"`), not a product-source failure. The original correct newline-aware declaration checks pass and remain in place; this follow-up removes only the four duplicate escaped assertions.
  - WAN route operation status `43893228430`: PASS.
  - Process inventory/fail-closed `43893228310`: PASS; Process lifecycle inventory `43893228288`: PASS.
  - NVRAM transaction boundary `43893228512`: PASS.
  - AI handoff documentation `43893228266`: PASS; VSFTPD `43893228286`, USB modem identity `43893228115`, NFS `43893228172`, Samba `43893228324`, HTTPD security `43893228169`, AliDDNS `43893228296`, infosvr `43893228293`, and AliyunDrive `43893228138`: PASS.
  - XY-C1 Firmware Validation `43893228097`: IN_PROGRESS at the last query. Its Build/TRX/artifact outcome is not yet available; do not claim L3 for 8f0.
- The older XY-C1 pass on `0111177585699cfacbf7878d1554eeb6bc861537` is not evidence for later source commits. Physical-router L4 remains NOT VERIFIED.
- Prior source-checker failures on a706 were subsequently addressed: WAN route now passes on 8f0 (87cbea whitespace normalization); process-inventory/lifecycle workflow checks pass on 8f0 after the 7b8 checker adjustment.


## Exact first action for the next GPT

1. Check the exact-head Actions triggered by the workflow-only follow-up commit. Confirm the USB storage hotplug workflow passes now that the four duplicate, incorrectly escaped declarations assertions are removed; verify the original newline-aware C declaration checks still execute.
2. Re-check XY-C1 Firmware Validation for the source SHA `8f0d792ec0ab7569ffd1ecf7216008b2396b2faf` and its Build, TRX verification and artifact upload stages. If it is still pending, work another independent source issue rather than waiting. Require all same-SHA stages before claiming firmware L3.
3. Then continue the P1 cross-userland RC startup-result audit, tracing actual callers and preserving process identity, lock order, and first-error semantics.
4. Keep physical-router L4 NOT VERIFIED until actual device evidence exists.
