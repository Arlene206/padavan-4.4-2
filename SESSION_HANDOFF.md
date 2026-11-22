# Cross-session execution mandate

Continue repository development across GPT conversations. Recover once, then implement. Do not repeat completed audits or ask the user what to do next.

- One logical slice means source change, regression coverage, one atomic main commit, and exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree and one commit. Update `main` with non-forced compare-and-swap against the expected parent SHA. Never force-push or expose a multi-file half-state.
- Before context exhaustion, inspect live main and backup refs, exact-head Actions and tests, then keep this handoff truthful about the first next task.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending, including `main` SHA, backup SHA, relevant exact-head Actions, test facts and the first next task.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; current source commit: THIS COMMIT (parent b7185685f1868b72ad48e87fadf434bdf7f7f0ee).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable.
- libdisk now uses checked directory enumeration and folder snapshots, bounded metadata reads and account paths, atomic permission-file publication, fail-closed folder/account initialization, and first-error propagation for folder delete/rename and account add/delete operations.
- add_account() propagates every mounted-volume permission initializer failure after attempting all volumes and reloading Samba; the existing NVRAM ordering is retained.
- del_account() frees the complete original account list, uses bounded metadata paths, ignores only missing-file unlink errors, continues through all mounted volumes, and reports the first cleanup failure after ftpsamba reload.
- mod_account() now formats both old/new per-volume metadata paths with the bounded formatter; ignores only missing old files, aggregates other rename/path errors across volumes, frees disk inventory and reloads ftpsamba before returning the first failure. The existing NVRAM commit ordering is unchanged.
- mod_account() also uses the checked storage inventory API, rejects non-regular source metadata and checks for existing target files before rename rather than overwriting stale per-volume permissions.
- get_account_list() now uses one exclusive checked NVRAM transaction for acc_num and all usernames; it validates a bounded decimal count, copies shared-cache strings, rejects sparse/duplicate/reserved/invalid names, and frees partial state on backend or transaction-release failures. Missing count remains an empty list.

## Active task

- launch_viptv_wan() publishes the IPTV VLAN recovery identity before starting DHCP/ZeroConf, but previously returned a failed dynamic client startup without stopping a possibly detached child or rolling back the owned VLAN. It now stops the udhcpc PID-file client or zcip process before VLAN rollback; only a confirmed stop allows rollback. An uncertain stop deliberately retains the VLAN and recovery identity.
- Regression covers successful start, start-failure cleanup order, stop failure preserving the recovery handle, rollback failure preserving the primary error and invalid callbacks. scripts/check_wan_vlan_setup.py checks production adapter wiring and test registration.
- The checked libdisk get_account_list() transaction snapshot is implemented; ten fake-backend cases run from test main(). Account mutation setter/commit rollback remains separate and is not changed here.
- Next: verify exact-head WAN/XY-C1/Process/AI Actions for this SHA, then select another bounded WAN/service lifecycle failure outside IPTV and libdisk.

## Verification boundary

- Parent b7185685f1868b72ad48e87fadf434bdf7f7f0ee: AI handoff #45021657238, WAN DHCP callback #45021657373 and WAN route #45021657378 PASS on this exact parent. These do not verify the new source SHA.
- Existing WAN route workflow watches net_wan.c, the IPTV policy/test and scripts/check_wan_vlan_setup.py and runs the sanitizer test plus source checker. XY-C1 watches net_wan.c.
- XY-C1 L3 remains NOT VERIFIED until Build, TRX verification and artifact upload all PASS on one identical product/source SHA. Hardware L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Fetch exact-head push Actions for latest main. Resolve failures from that SHA's logs; do not transfer parent PASS results.
2. Continue with a different bounded WAN/service lifecycle reliability issue outside libdisk and IPTV; add focused regression and atomic source fix.
3. Keep frozen backup immutable. XY-C1 L3 only after same-SHA Build, TRX and artifact upload all PASS; hardware L4 remains NOT VERIFIED.
## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched.

