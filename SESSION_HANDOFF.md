# Cross-session execution mandate

Continue repository development across GPT conversations. Recover once, then implement. Do not repeat completed audits or ask the user what to do next.

- One logical slice means source change, regression coverage, one atomic main commit, and exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree and one commit. Update `main` with non-forced compare-and-swap against the expected parent SHA. Never force-push or expose a multi-file half-state.
- Before context exhaustion, inspect live main and backup refs, exact-head Actions and tests, then keep this handoff truthful about the first next task.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending, including `main` SHA, backup SHA, relevant exact-head Actions, test facts and the first next task.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; current source commit: THIS COMMIT (parent fe22b9fae01db2bb9fe57f2fffe67cee067ea364).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable.
- libdisk now uses checked directory enumeration and folder snapshots, bounded metadata reads and account paths, atomic permission-file publication, fail-closed folder/account initialization, and first-error propagation for folder delete/rename and account add/delete operations.
- add_account() propagates every mounted-volume permission initializer failure after attempting all volumes and reloading Samba; the existing NVRAM ordering is retained.
- del_account() frees the complete original account list, uses bounded metadata paths, ignores only missing-file unlink errors, continues through all mounted volumes, and reports the first cleanup failure after ftpsamba reload.
- mod_account() now formats both old/new per-volume metadata paths with the bounded formatter; ignores only missing old files, aggregates other rename/path errors across volumes, frees disk inventory and reloads ftpsamba before returning the first failure. The existing NVRAM commit ordering is unchanged.
- mod_account() also uses the checked storage inventory API, rejects non-regular source metadata and checks for existing target files before rename rather than overwriting stale per-volume permissions.
- get_account_list() now uses one exclusive checked NVRAM transaction for acc_num and all usernames; it validates a bounded decimal count, copies shared-cache strings, rejects sparse/duplicate/reserved/invalid names, and frees partial state on backend or transaction-release failures. Missing count remains an empty list.

## Active task

- Dynamic IPTV launch failure compensation is implemented in launch_viptv_wan(): stop the udhcpc PID-file client or zcip service first; only a confirmed stop permits ownership-aware VLAN rollback. If stop is uncertain, retain the VLAN recovery identity. Five sanitizer cases cover success, cleanup order, uncertain stop, rollback failure and invalid callbacks.
- Exact-head WAN route #45022142699 passed the WAN route suite, including the IPTV policy sanitizer tests and scripts/check_wan_vlan_setup.py. WAN DHCP callback #45022142584 passed all C regressions (including the IPTV test) but failed only because its inline Python contract still required direct start_zcip_viptv/start_udhcpc_viptv calls inside launch_viptv_wan(). The new launcher delegates through wan_viptv_dynamic_start_callback(); that assertion is corrected in this follow-up.
- Checked libdisk get_account_list() transaction snapshot is implemented; ten fake-backend cases run from test main(). Account mutation setter/commit rollback remains separate and unchanged.
- After this checker correction, inspect exact-head WAN DHCP, WAN route, AI, Process, NVRAM and XY-C1 Actions. Then continue with another bounded WAN/service lifecycle defect outside IPTV and libdisk.

- Next: audit remaining DHCPv6 lease/service startup inputs for error-collapsing NVRAM getters that affect routes, DNS, DUID or client lifetime.


- `start_dhcp6c()` now snapshots `wan_hwaddr` before DUID/file/session side effects; missing/invalid MAC keeps legacy skip behavior while backend/close errors abort startup. Three fake-backend regressions cover missing key, read failure and close failure.
- `reset_wan6_vars_status()` uses a mode-selected checked snapshot for static WAN identity and 6rd settings; three fake-backend tests cover coherent tuple, per-field failure and dynamic-mode selection.
## Verification boundary

- Exact source SHA fe22b9fae01db2bb9fe57f2fffe67cee067ea364: WAN route #45022142699 PASS; AI handoff #45022142702 PASS; WAN DHCP callback #45022142584 FAILED only at stale inline source assertion after all preceding C sanitizer regressions passed, including the new IPTV test. The assertion expected direct launch calls inside launch_viptv_wan(); the source now delegates through its tested callback. WAN teardown #45022142730 PASS; HTTPD #45022142700 PASS; IPv6 service-type #45022142701 PASS; NVRAM boundary #45022142711 and XY-C1 #45022142628 remained in progress at last check; Process inventory #45022142755 was still in progress after inventory upload.
- The checker-only follow-up must be verified on its own exact SHA. XY-C1 L3 remains NOT VERIFIED until same-SHA Build, TRX verification and artifact upload all PASS. Hardware L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Fetch exact-head WAN DHCP and all other push Actions after the inline contract correction; resolve any remaining causal failure.
2. Continue with a different bounded WAN/service lifecycle reliability defect outside IPTV/libdisk; add a focused regression and atomic fix.
3. Preserve the frozen backup. XY-C1 L3 only after same-SHA Build, TRX and artifact upload all PASS; hardware L4 remains NOT VERIFIED.
## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched.

