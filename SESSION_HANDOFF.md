# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA for this checkpoint: `e49224f71f90339247917b66d8a9add54228e8a1`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- The checked NVRAM CLI and AliyunDrive fail-closed startup-read slice is now closed at L1/L2 on exact main SHA `11576ae4e50f8bb063682bbb94929a7bae394a47`.
- New WAN route slice: `full_restart_wan()` explicitly permits an empty `wan0_ifname` but previously passed it as a required flush target after `stop_wan()`. The route-flush policy returns `EINVAL` for an empty device, so a restart with no manual interface would stop WAN and then abort. The production fix and three policy regressions are committed at `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`; `45a78362` corrected one selector source assertion. The route-selector C regression itself printed PASS in the exact `b56e0b5` WAN, HTTPD and XY test runs. Four Python/source contracts were stale because they still required the removed fixed two-entry array/`route_ifnames[0]` assignment; the present commit updates those contracts to verify optional manual-interface selection, mandatory bridge retention, ordering before WAN stop, and actual dynamic count.

## Active task

继续稳定性修复，避免重复审计已完成工作。已提交的重点源码修复：NFS exports ACL 12-key NVRAM transaction snapshot；WAN physical/ IPTV DHCP overlap 的 LAN address/netmask transaction snapshots；LAN startup/reset subnet snapshot；AP DHCP hostname address/domain snapshot；L2TP server selector checked-read fail-closed；WAN VLAN filter/VID checked-read fail-closed。关联测试与 source-contract workflows 已同步更新。当前重点转向精确 SHA Actions 验证和修复仍失败的 checker/独立稳定性问题。

## Verification boundary

- 当前 main: `372451d468fd772858635590f1c1c10bccf95dc0` — `test(wan): update process L2TP fail-closed contract`; parent `6418e87add0ffce79fd918077bb1e7dfdded2fc0`. Backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (保持冻结，未更新)。
- 本次已提交逻辑切片：`c139dd5` NFS ACL atomic snapshot；`1b193a6` IPTV overlap LAN tuple；`3d1ded0` physical WAN overlap LAN tuple；`1da00c0` LAN startup/reset snapshot；`bf51be3` AP hostname tuple；`0c3d6dc` L2TP checked selector fail-closed；`b5fbceb` WAN VLAN checked VID fail-closed。Checker follow-up commits: `4f9ed0e`, `f5b93cf`, `f902d00`, `1fe718a`, `b48c884`, `fbee4e5`, `6c360d4`, `19af0ec`, `55e39b6`, `9056ed1`, `6418e87`, `372451d`.
- 当前 HEAD 的 exact-SHA Actions:
- #45051059927 Process inventory and lifecycle fail-closed regression: completed/success (372451d468fd)
- #45051059851 AI handoff documentation contract: completed/failure (372451d468fd)
- XY-C1: 当前 SHA 未找到匹配运行；L3 NOT VERIFIED。
- 较早 SHA 的 CI FAIL 曾定位为 checker 不同步（IPTV/physical snapshot 旧 getter 断言、AP identity 旧 getter 断言、L2TP 未更新的流程断言、缺少 `re` import 和 route regression 文件加载）。已经有对应 checker commits；不得把旧 SHA 的 PASS 冒充当前 SHA PASS。
- 未在本地运行编译器/测试，也未做路由器硬件验证。L3/L4 仍需根据 exact current SHA 与实机结果确认，不得推断。
## Exact first action for the next GPT

1. 先查看当前 SHA `372451d468fd772858635590f1c1c10bccf95dc0` 上所有 Actions 的最终结果；对 FAIL 取对应 job logs，修复 checker 或源码后按新 SHA 重跑。PENDING 时继续独立任务，不等候。
2. 特别核对 XY-C1 Build、TRX validation、artifact upload 是否在同一 SHA 全部 PASS；若没有，不得标记 firmware L3 verified。
3. 继续跨模块 stability/security 扫描，跳过本文件已列出的 NFS/WAN DHCP overlap/LAN snapshot/L2TP/VLAN 修复；每个逻辑切片源码、回归测试、原子提交、精确 SHA Actions。

1. Inspect NFS daemon workflow on the exact new main SHA: GNU89 `-Wall -Wextra -Werror` policy test, ASan/UBSan regressions and Python source contracts.
2. Inspect XY-C1 on that same SHA. Require Build, TRX validation and artifact upload all to succeed before claiming firmware verification.
3. If either workflow is pending, continue an independent stability/security fix; if one fails, diagnose the failing step without repeating completed audits.
