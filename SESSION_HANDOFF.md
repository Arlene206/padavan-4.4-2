# Cross-session execution mandate

Every session must inspect live repository state before ending or handing off. Confirm main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup, and the precise next action. Do not make no-op checkpoint commits.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Current source baseline before the restore-status correction: `b9850f9917a6eb77b7c6d021abe75c88b4ca444c`.
- PPP/L2TP command arguments use argv-based `eval()`; `safe_start_xl2tpd_locked()` now normalizes all nonzero command exit statuses to -1, returns 0 after a successful launch, and 1 only when a complete inventory proves xl2tpd is already running. WAN PPP launch accepts that already-running result.
- This follow-up fixes a remaining caller inconsistency: `restart_vpn_server()` must treat only negative xl2tpd restore status as failure. The completed server-start path can already have relaunched the shared daemon, so the subsequent restoration check can validly return 1.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact-base Actions on b9850f9917a6eb77b7c6d021abe75c88b4ca444c

PASS: VPN lifecycle `43993836496`, AI handoff `43993836752`, infosvr `43993836668`, WAN DHCP `43993836575`, standalone process inventory `43993836554`, AliyunDrive `43993836669`, HTTPD `43993836692`, WAN route `43993836463`, Process lifecycle `43993836708`, AdGuardHome `43993836497`, and AliDDNS `43993836617`.

PENDING when last checked: XY-C1 `43993836487` was on `Build XY-C1`; NVRAM audit `43993837253` was on raw userland ioctl/kernel NVRAM caller review. Both runs are pinned to the exact base SHA.

## Active task

Verify the restore-status gate correction on its exact new main SHA. Confirm that VPN lifecycle and the source-contract regression pass. Inspect XY-C1 Build, TRX verification and artifact upload together on one SHA, plus the NVRAM audit result.

## Verification boundary

- On base b985, the VPN lifecycle workflow including xl2tpd policy regression and source checker passed; the restore-callsite correction in this follow-up is not yet tested on its own SHA.
- Current-source MIPS/uClibc L3 (Build, TRX and artifact upload) is **NOT VERIFIED** until the new commit's XY-C1 workflow completes all levels.
- Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch `/actions/runs?head_sha=<live main SHA>`; verify VPN lifecycle and all affected exact-head checks. Fix any first causal failure from its job log.
2. Confirm same-SHA XY-C1 Build, TRX validation and artifact upload. If still pending, continue safe independent source work.
3. Resume the next highest-priority unverified stability/security module after this PPP/L2TP lifecycle slice is closed. Do not repeat completed argv/xl2tpd audits.
4. Keep the frozen backup unchanged and use a single complete tree/commit with a non-forced expected-SHA update.
