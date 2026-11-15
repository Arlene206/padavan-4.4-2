# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** Record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at the start of this source slice: `2199bdb463da98d26289f819baa407e9feec97b1`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Exact-main Process inventory/lifecycle run #43747014935 PASS; AI handoff docs #43747014985 PASS; NVRAM #43747014966 PASS.
- Product-source-identical XY-C1 run #43746522702 on `be7fd92337812a28ddc5b275c7898915be114987` completed successfully: Build XY-C1, TRX verification and artifact upload all PASS. The intervening `30e1848` and `2199bdb` commits changed checker/docs only, not firmware product source. L3 is VERIFIED on that source tree; no physical-router/L4 claim is made.
- IGMP proxy staged config publication and its GNU89 ASan/UBSan host regression are covered by the passing exact-main Process run. The previous MiniDLNA typed-state checker mismatch is fixed in `2199bdb`.
- Root `dev_log.md` has 89 entries before this slice; append in-place and do not rotate. Keep `docs/dev-log/INDEX.md` and archived shards unchanged.

## Active task

### Propagate IPTV DHCP/ZeroConf launcher failures through WAN startup

In `trunk/user/rc/src/net_wan.c`, `start_udhcpc_viptv()` and `start_zcip_viptv()` both return their `_eval()/eval()` status. `launch_viptv_wan()` previously discarded that status and returned 0 for both dynamic modes, even though `start_wan()` already checks `launch_viptv_wan()` and logs any nonzero result. This hid missing-binary, exec and launch failures as successful IPTV startup.

The source slice now assigns each dynamic launcher's result to `ret` and returns it. The existing `.github/workflows/wan-dhcp-callback-stability.yml` source-contract regression is extended to require both status assignments, returned status and caller-side error logging. This is a targeted startup-status propagation fix; it does not alter DHCP lease behavior or IPTV mode selection.

## Verification boundary

- The source commit is built on parent `2199bdb463da98d26289f819baa407e9feec97b1`; fetch its resulting main SHA and exact-head Actions before claiming verification.
- The WAN DHCP callback workflow must pass its host tests and new launcher propagation contract.
- Check any exact-head XY-C1 build through Build, TRX and artifact upload if the workflow is triggered by this source path.
- The preceding product-source baseline `be7fd92337812a28ddc5b275c7898915be114987` has exact run #43746522702 PASS including Build/TRX/artifact. That does not replace checking a newly triggered run for this source change.
- Frozen backup and archived dev-log/roadmap content remain untouched. No physical-router/L4 result is claimed.

## Exact first action for the next GPT

1. Fetch the new live main SHA and inspect exact-head Actions for the IPTV launcher status commit. Diagnose/fix any failing WAN DHCP callback source contract or host test; do not infer PASS from partial jobs.
2. If the XY-C1 workflow was triggered, verify Build, TRX and artifact upload for the exact new SHA. The older source-identical #43746522702 run is already PASS.
3. Continue the same startup-status audit at the next call boundary: `launch_wanx()` currently ignores the status from `start_udhcpc_wan()` / `start_zcip_wan()`, and callers in `start_wan()` need careful handling for PPPoE versus PPTP/L2TP (do not blindly abort PPPoE, which intentionally does not wait for a physical DHCP lease).
4. Do not redo IGMP proxy publication, the MiniDLNA typed logger checker, completed FTP cleanup/udpxy/xupnpd/IAPPD work or the completed XY-C1 build baseline.
