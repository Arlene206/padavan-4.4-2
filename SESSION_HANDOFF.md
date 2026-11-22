## 2026-11-22 — Current development checkpoint

- Repository: Arlene206/padavan-4.4-2; active branch main. Main parent before THIS COMMIT: fa49d01bc7884e68e5adcd3832d2b0480bfaa974.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807 and must remain immutable.
- This slice centralizes folder permission file updates using the bounded account path helper, checked read, line-preserving rewrite and same-directory atomic replacement. del_folder() and mod_folder() now attempt all independent account files and anonymous FTP after a per-file error, then return the first error. get_permission() now fails closed on pathname truncation.
- Parent exact-head libdisk run #45019159131 PASS on fa49d01bc7884e68e5adcd3832d2b0480bfaa974. Runs #45018787328, #45018946513 and #45019075810 were checker failures, not C sanitizer failures: obsolete rmdir assertion, account-inventory assertion, then undefined SHARE_TEST. The latest passing run confirms the previous checker corrections.
- Exact-head Actions for THIS COMMIT must be checked live. XY-C1 L3 requires Build XY-C1, TRX verification and artifact upload all PASS on the same product-source SHA. Hardware L4 NOT VERIFIED.

## Current libdisk state

- Directory enumeration distinguishes clean EOF from readdir errors and has GNU89 ASan/UBSan coverage.
- Folder-list snapshots, initial account permission files, new-folder permission defaults and set_permission() changes use checked atomic publication.
- Metadata reads are bounded and errno-aware; account paths are checked for truncation; add_folder() fails closed on every setup stage.
- Folder deletion and rename use the shared atomic account updater. Per-file failures no longer stop processing later accounts or anonymous FTP; the first error is returned after independent cleanup attempts.

## Exact first action

1. Fetch live main and inspect exact-head libdisk share-list, Process inventory, NVRAM and XY-C1 push Actions for THIS COMMIT. Fix a real failed contract/test before declaring host validation.
2. Continue the storage-account boundary: inspect add_account() and del_account(), their NVRAM mutations, and every per-volume metadata initialization/removal result; add regressions and preserve consistent state on failure.
3. Keep backup/pre-plugin-overhaul-2026-11-14 immutable. Claim XY-C1 L3 only when Build, TRX and artifact upload all pass on one exact product SHA; keep hardware L4 NOT VERIFIED.
