## 2026-11-17 19:01:50 CST — Distinguish absent guest VIFs from unknown state; tie rollback to AP start attempt

- Parent main SHA: 9bb789d0f6597e40bcebcb1a26f42519bc6619e8; result: THIS COMMIT.
- The guest state helper in parent 801d1bbf collapsed failed flags reads into DOWN. Parent 9bb789d0 also moved LAN isolation reads to the helper; exact-head Process/Wireless runs 44069490305 and 44069490310 failed only because the checker still expected raw flags in rollback callbacks.
- This commit carries errno through link-state policy: ENODEV (optional VIF not created yet) is known DOWN; all other socket/ioctl errors return -EIO. Guest enable/disable, rollback, bridge-isolation and boot snapshots share the checked reader.
- init_bridge records pre-state query status and skips AP startup on that radio if the state is unknown. Rollback eligibility is the pre-state plus an explicit AP-start-attempt flag (set only for an enabled radio immediately before start_wifi_ap_*), not a post-state query.
- Expanded GNU89 tests for generic error, absent VIF, present DOWN/UP and invalid output; updated source-contract checks for the shared helper, ownership flags and callback usage.
- Parent 5c530087 Process lifecycle 44069081348 passed; XY-C1 run 44068629924 was still running at last poll. Parent 9bb789d0 exact-head Process/Wireless failed at the stale checker assertion; this commit's exact-head checks pending.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; physical-router L4 NOT VERIFIED.

---
