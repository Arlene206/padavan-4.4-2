## 2026-11-18 — Withdraw SSH listener when public firewall refresh fails

- Parent main SHA: `7ffa87073cb7a18fadd5c7dc0fdbdd794b272f20`; production paths `trunk/user/rc/src/services.c` and `sshd_daemon_policy.h`; regression: GNU89 SSH policy test plus `scripts/check_sshd_firewall_status.py`.
- `restart_sshd()` could start a listener after a process-state transition, call `restart_firewall()` for enabled public SSH, then return the refresh error without stopping the newly active listener. That could leave SSH reachable under an old or incomplete ruleset.
- Add `sshd_firewall_failure_requires_stop()`: nonzero refresh requires shutdown unless state is confirmed ABSENT. On failure invoke `stop_sshd()`, rescan the daemon, and treat only PROCESS_PRESENCE_ABSENT as confirmed withdrawn. RUNNING/UNKNOWN is logged as unresolved; original refresh errno is preserved.
- Add GNU89 `-Wall -Wextra -Werror` ASan/UBSan coverage for zero/error and ABSENT/RUNNING/UNKNOWN/invalid states; add a source-order contract; wire into Process CI and PR path filters.
- Prior 319fd181 passed exact-head XY-C1 Build/TRX/artifact. PPPoE-MWAN follow-ups through `7ffa8707` require exact-head validation. THIS SSH source commit needs fresh exact-head Process/XY-C1 checks; router L4 remains NOT VERIFIED; backup frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

