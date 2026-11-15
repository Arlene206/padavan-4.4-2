## 2026-11-15 15/11/2026, 08:45 CST — Fail closed on UNKNOWN in Samba daemon launch/reload

- Parent main SHA: 5584a6d0f479b82cb2c872ca0effcd875e6f2b91.
- `run_samba()` used raw `pids()` flags, so an UNKNOWN result could be truthy and authorize name-wide HUP. It now captures complete smbd/nmbd and optional wsdd2 snapshots before account recreation, credential provisioning, config publication or cleanup. UNKNOWN aborts before those mutations; every snapshot is released.
- `samba_ensure_daemon()` acts only on captured complete state: RUNNING uses a callback-tested identity policy that preflights every PID/start-time/argv identity before the first signal and revalidates each identity immediately before targeted SIGHUP. ABSENT is rescanned before launch; UNKNOWN or an appeared process aborts.
- Added GNU89 callback-injected tests for ABSENT/RUNNING/UNKNOWN action mapping, preflight UNKNOWN with zero signals, mid-loop UNKNOWN, disappeared processes/ESRCH, EPERM and invalid inputs. Wired ASan/UBSan regression into Samba CI and updated source contracts.
- Exact-head Samba Action pending at commit creation. Local compiler not run because no checkout is exposed in this connector session. L3/L4 remain NOT VERIFIED.
- Result: THIS COMMIT.

---

