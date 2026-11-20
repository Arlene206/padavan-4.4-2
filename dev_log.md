## 2026-11-19 — Harden privileged ZeroTier identity and network join writers

- Parent SHA: `df94216f2418bfb3a099aa55f847d7438a9e1f09`.
- Confirmed root-owned path traversal in `add_join()`: NVRAM-sourced `zerotier_id` was expanded unquoted into `touch $config_path/networks.d/$1.conf`. Identity keys were also written directly to persistent paths without enforcing private permissions or refusing symlink destinations; confidentiality depended on inherited umask.
- Added strict optional 16-hex-digit network-ID validation, quoted/contained join-file creation and symlink-leaf refusal. Startup now prepares config and networks.d directories as mode 0700, rejects symlink/non-directory paths, and stages identity outputs in a private same-filesystem directory before chmod-0600 and atomic per-file publication. The launcher sets umask 077 and aborts on preparation failures.
- Extended the shell lifecycle regression for permissive umask, directory/file modes, symlinked config paths, path traversal, malformed IDs, symlink leaves, generated/stored identity, failed public-key derivation and preservation of existing identity files. The Python source contract now asserts the production writer invariants.
- Local execution of these host regressions was unavailable in this environment. Exact-head ZeroTier lifecycle and XY-C1 Build/TRX/artifact Actions are the authoritative pending validation. No L3/L4 claim is made here.
- Exact-head CI: PENDING at commit creation. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Secure Unix password staging and propagate boot provisioning failures

- Parent SHA: `ae7874833c6cb870162222f415124d1db4e63dda`.
- Confirmed `change_passwd_unix()` used predictable `/tmp/tmpchpw` with `fopen("w")`; a pre-created symlink under world-writable `/tmp` could redirect a privileged credential write. The helper also ignored temporary-file, write/close, `chpasswd`, unlink and shadow-permission failures.
- Replaced password staging with exclusive `mkstemp()` mode-0600 creation, account/password delimiter validation, checked write/flush/fsync/close, command status, unlink and shadow chmod.
- `recreate_passwd_unix()` now stages passwd/group/shadow contents before per-file atomic rename, keeps root shadow locked until `chpasswd` succeeds, and returns the first provisioning error. `init_router()` includes this status in the boot DEGRADED result.
- Added a GNU89 ASan/UBSan host regression for unique exclusive files, exact permission modes and invalid templates. Process source contract covers production helper and PID 1 caller. Local sanitizer test passed.
- Previous wireless slice: Process `44649240357` and AI docs `44649240209` PASS on `ae787483`; production-source XY-C1 `44648971016` PASS on `9f565b1`, including Build, TRX and artifact upload (L3 for that exact SHA). L4 remains NOT VERIFIED.
- New exact-head Process/AI docs/XY-C1 Actions pending. Paired passwd/group rename rollback remains a focused fault-injection follow-up.

---

