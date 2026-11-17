## 2026-11-18 — Verify SSH daemon exit with identity-bound lifecycle stop

- Parent main SHA: `07b8266ea07de74d6b898a184018cf949b4784a2`; production: `trunk/user/rc/src/services.c`, shared decision policy `sshd_daemon_policy.h`; regression: extended `tests/sshd_daemon_policy.c` and `scripts/check_sshd_firewall_status.py`.
- Prior `stop_sshd()` returned `eval("/usr/bin/sshd.sh","stop")`. OpenSSH/Dropbear `func_stop()` only ran `killall -q sshd/dropbear`, so a zero status meant a signal was sent, not that the daemon was gone. This could allow restart callers to trust a stop result before the old listener had exited.
- Add `sshd_process_name_for_binaries()` and reuse it for inventory + stop targeting. `stop_sshd()` now calls `kill_services(service_names, 3, 1)`, using the existing PID/start-time/cmdline identity lifecycle engine. Only confirmed absence returns STOPPED; UNKNOWN/CHANGED/FAILED/TIMEOUT blocks restart, escalation is bounded.
- Unit tests cover Dropbear/OpenSSH/no-binary selection, and the Process source checker ensures both inventory and stop use the same helper and no shell `killall` wrapper remains.
- Exact parent 6a5f Process/Firewall/USB/AI passed; XY-C1 `44188951477` was still in Build at last observation, and requires separate confirmation. The 07b826e AliyunDrive special-domain startup failure `44189266857` remains a separate task. Router hardware L4 remains NOT VERIFIED; backup remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

