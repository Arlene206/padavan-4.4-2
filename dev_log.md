## 2026-11-23 UTC — Make ZeroTier Moon creation transactional

- Source review found `creat_moon()` ran `sed -i` inside command substitution rather than checking status, did not check `initmoon`, continued after endpoint-edit failure, ignored missing `genmoon` output, and did not check file/NVRAM publication results.
- Moon endpoint input is validated as IPv4 before interpolation. JSON and signature are produced in a private same-filesystem stage; endpoint insertion, generated 10-hex ID/signature, file modes and every publication operation are checked.
- Existing JSON/signature are hard-linked before replacement. Failed second-file publication or NVRAM set/commit triggers restoration of both previous files and the old `zerotiermoon_id`; incomplete rollback preserves the private recovery directory.
- Added shell regressions for invalid endpoint, initmoon/sed/genmoon failures, setter that writes then reports failure, second-file rename failure and successful generation. Exact-head ZeroTier/AI Actions pending; no local test/build claim.

---

