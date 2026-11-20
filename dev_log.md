## 2026-11-19 — Secure Unix password staging and propagate boot provisioning failures

- Parent SHA: `ae7874833c6cb870162222f415124d1db4e63dda`.
- Confirmed `change_passwd_unix()` used predictable `/tmp/tmpchpw` with `fopen("w")`; a pre-created symlink under world-writable `/tmp` could redirect a privileged credential write. The helper also ignored temporary-file, write/close, `chpasswd`, unlink and shadow-permission failures.
- Replaced password staging with exclusive `mkstemp()` mode-0600 creation, account/password delimiter validation, checked write/flush/fsync/close, command status, unlink and shadow chmod.
- `recreate_passwd_unix()` now stages passwd/group/shadow contents before per-file atomic rename, keeps root shadow locked until `chpasswd` succeeds, and returns the first provisioning error. `init_router()` includes this status in the boot DEGRADED result.
- Added a GNU89 ASan/UBSan host regression for unique exclusive files, exact permission modes and invalid templates. Process source contract covers production helper and PID 1 caller. Local sanitizer test passed.
- Previous wireless slice: Process `44649240357` and AI docs `44649240209` PASS on `ae787483`; production-source XY-C1 `44648971016` PASS on `9f565b1`, including Build, TRX and artifact upload (L3 for that exact SHA). L4 remains NOT VERIFIED.
- New exact-head Process/AI docs/XY-C1 Actions pending. Paired passwd/group rename rollback remains a focused fault-injection follow-up.

---

