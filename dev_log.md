## 2026-11-16 — Remove unnecessary shell execution from PPP control commands

- `net_ppp.c` had only two remaining process-shell calls: `system("/usr/sbin/l2tp-control \\\"start-session 0.0.0.0\\\"")` and `doSystem("/usr/bin/mwan.sh --rc-lifecycle-locked")`. Both commands and their arguments are static; no dynamic input is involved, but the shell added an unnecessary parsing layer and returned a different status representation.
- Replaced them with direct argv-based `eval()` calls while preserving the exact executable, single L2TP command argument, mwan lifecycle lock flag, surrounding nonzero checks and status propagation. The PPP module now has no `system()` or `doSystem()` calls.
- Added shared source contracts requiring both argv vectors, their status checks, and no remaining shell API in `net_ppp.c`. Focused local checks pass.
- Parent `71c95adc3e99b611ddfdb9f4b67014718ed4916a` exact-head reruns were IN_PROGRESS at last poll; the prior stale PPP ping checker issue was repaired there. Product-source XY-C1 L3 remains unverified.

---

## 2026-11-16 — Align PPP peer-probe source contracts with argv execution

- Parent `5327f7626fc307c8db5e2de670e5433cd0eef55b` exact-head WAN DHCP `43990883611` PASS; Firewall `43990883471`, WAN route `43990883432`, Process inventory `43990883425` and XY-C1 `43990883486` FAIL; NVRAM `43990883715` was still running.
- Firewall, Process and XY-C1 failure logs converged on `scripts/check_modem_wan_start_status.py:203`: the checker still located the shell-formatted ping after `wait_ppp_up()` had migrated to `eval("ping", "-c1", ppp_gate)`. WAN route's embedded checker had the same stale expression at Python line 300. Prior host regressions in those jobs passed before this source-contract failure.
- Updated the shared checker and WAN route workflow to require the argv ping command, reject the old `doSystem()` expression, and continue checking the success return occurs after the peer probe. No production C change was needed for this CI drift.
- Exact follow-up Actions and the XY-C1 build gate remain pending. The prior XY-C1 failure occurred in the stale host assertion before target compilation; it is not a firmware-build result.

---

