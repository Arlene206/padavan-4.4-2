## 2026-11-24 — Normalize multiline XUPnP stage assertions in lifecycle contract

- Process lifecycle run #45402388244 on `25db7b79cae620b6c2e36b1454e5cf1c92e5cfa5` passed the earlier process regressions, including the GNU89 ASan/UBSan XUPnP storage/file-safety test, but still failed at the updated checker’s first new assertion. The call is split across source lines in `net.c`, while the checker searched raw source for a single-line string.
- Updated the source contract to compare the whitespace-compacted XUPnP function body for both staged tempfile calls. Product source and test binary unchanged. Exact-head Process regression is required again.

---

## 2026-11-24 — Align lifecycle integration checks with staged XUPnP pair

- Exact Process lifecycle run #45401700271 on `eb0fbb1f3d858876e754efc2815c3880b74f764a` passed the XUPnPD storage-directory and file-safety C regression plus multiple other daemon regressions, then failed only because `check_process_lifecycle_integration.py` still required two calls using `tmp2/tmp1` and sequential `rename(tmp1,tmp2)`. The product code now stages `common_path/common_stage` and `runtime_path/runtime_stage`, then calls `dnsmasq_file_publish_pair()`.
- Updated the lifecycle checker to require the two distinct staged paths, both source inputs and the rollback-capable publisher, and to reject the old sequential-rename contract. This is a validator-only fix.
- The separate DNSMasq lifecycle checker was also fixed in `2fd4c74` after #45401700240 exposed a raw-versus-compacted offset comparison bug. Both exact-head workflows must rerun; no new product source change in these checker corrections.

---

## 2026-11-24 — Fix XUPnP source-checker offset comparison

- Exact DNSMasq lifecycle run #45401700240 on source SHA `eb0fbb1f3d858876e754efc2815c3880b74f764a` passed the GNU89 ASan/UBSan DNSMasq file regression and failed the XUPnP source-order checker. The checker compared character offsets from un-compacted `body` text to offsets from whitespace-compacted `compact` text; it therefore rejected correctly ordered fsync/NVRAM operations.
- Corrected all four relative-order comparisons to use the same compact source string. No production C or test code changed. Exact-head DNSMasq and documentation checks must rerun on this correction SHA.

---

## 2026-11-24 — Publish XUPnP Lua configuration as a rollback-capable pair

- `start_xupnpd()` previously renamed staged `common.lua` live before preparing `xupnpd.lua`. A later source/NVRAM/write/fsync failure could leave a mixed config generation while startup returned an error.