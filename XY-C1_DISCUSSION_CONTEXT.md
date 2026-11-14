---
kind: non-canonical-user-discussion-context
scope: XY-C1/C5 configuration discussion only
normal_repository_handoff: SKIP
read_when: user explicitly supplies this file/path and asks to resume this discussion
authority: user preferences + source observations at snapshot SHA; not a development instruction
---

# XY-C1/C5 参数讨论快照（非项目开发文档）

> **NORMAL HANDOFF: SKIP THIS FILE.** 不属于项目规范、开发路线图、handoff 或开发日志；正常接力开发不得自动读取、不得据此选择任务、不得改变开发优先级。仅当用户明确给出本文件路径并要求继续 XY-C1 参数讨论时才读取。项目开发仍以 `AI_DEVELOPMENT_PROTOCOL.md`、`SESSION_HANDOFF.md`、`REFACTOR_ROADMAP.md` 和实时 GitHub 状态为准。本文件不是代码修改授权。

## 0. 快照边界

- 核对日期：2026-11-14；源代码基线：`main=59105690eeef1e32fdeee1a4a9bf2f0665b18693`（本文件创建前的 parent）。
- 目标：小娱 XY-C1/C5，MT7621，用户报告 512MB RAM / 32MB Flash。硬件分区/可用固件上限尚未核实。
- 最近一次 XY-C1 实际编译：Actions Run `43541384942`，基线 SHA 同上，workflow_dispatch 成功；这是**未启用代理核心**的当前模板构建，不代表目标代理固件大小或 L4 硬件验证。
- 本文件记录偏好、已核实源码事实和待验证设计；后续必须重新读取实时模板/Makefile/Actions，不能把快照当作当前代码状态。

## 1. 用户场景与已表达偏好

- 自用家庭主路由；宽带 500M+。四口之家，其他成员主要手机/平板视频应用，重度网络使用者主要是用户本人。
- 不需要 QoS/SQM/CAKE/IMQ/IFB/复杂流量整形；不为理论上的公平调度牺牲转发性能。保留 SFE。
- C5 定位：稳定、低维护的家庭网关；SmartDNS 是核心需求；科学上网必须有可用 WebUI，明确希望 WebUI 支持 Hysteria2。
- N1 承担容器、计算、存储和额外服务；偏云端使用。Tailscale 默认放 N1，不要求 C5 实现；只有以后明确需要 C5 作 Subnet Router 才新增开发。
- 不希望为了极限精简而删除以后可能用到的 USB/SATA/存储能力。优先保留接口驱动、文件系统和 SMB/FTP 可选能力；不使用时相关服务应默认停止/关闭。编入固件不等于服务必须常驻。功耗收益须实测，不可仅凭移除 Kconfig 项推断。
- 当前倾向 CPU 默认不超频；之前同意过稳定优先，但后来再次询问空闲功耗，因此最终参数仍应以用户后续确认和实测为准。

## 2. 基线源码事实（仅对 snapshot SHA 负责）

### XY-C1 模板 `trunk/configs/templates/XY-C1.config`

| 项 | 基线值 | 说明 |
|---|---:|---|
| SFE / IPv6 / IPSet | y / y / y | 当前保留 |
| QoS / SQM / IMQ / IFB | n / n / n / n | 符合场景 |
| SmartDNS | y | 当前启用 |
| USB | y | 当前启用 |
| SMB / FTP | y / y | 当前编译包含能力 |
| SHADOWSOCKS | n | 关键构建门控 |
| XRAY / SINGBOX | y / n | **Xray 标记 y 不代表会被编译** |
| OC | y，值 `0x312` | 当前模板实际开启，和“默认不超频”目标相反 |

### 代理构建门控（关键）

- `trunk/user/Makefile` 中 Xray 和 Sing-box 目录加入位于 `ifeq ($(CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS),y)` 内。
- 因当前 `SHADOWSOCKS=n`，尽管 `XRAY=y`，Xray/Sing-box 都没有进入本次构建；`SINGBOX=n` 更不会构建。
- 本次 Actions 日志有 SmartDNS 的 BUILD/ROMFS 步骤，没有 Xray 或 Sing-box 的 BUILD/ROMFS 步骤；RC 编译宏也未见 `APP_SHADOWSOCKS`。
- 因而当前约 13MB TRX **没有代理核心**。不能用当前镜像大小推断加入核心后的容量。

### WebUI 与代理

- 旧界面：`trunk/user/www/n56u_ribbon_fixed/Shadowsocks.asp`，界面名称/结构为 ShadowSocksR Plus+；有 SS/SSR、VMess、VLESS、Trojan、SOCKS5 等旧式配置/导入逻辑。
- 当前没有 Sing-box JSON 配置生成/校验管理流程，也没有真正的 Hysteria2 表单。
- 旧 V2Ray 的 QUIC 传输选项不等于 Hysteria2。
- `www/Makefile` 在 `SHADOWSOCKS` 未启用时会移除旧 Shadowsocks 页面。简单打开总开关可能同时带入旧 Shadowsocks 子系统/依赖；未来应把“代理核心构建”“旧 SSR 兼容组件”“代理 WebUI”拆成独立开关/依赖，避免为 Sing-box 强制编译无用旧组件。

### 版本 / 工具链（核对日 2026-11-14）

| 组件 | 仓库基线 | 当日上游稳定版/约束 |
|---|---|---|
| SmartDNS | `1.2026.48.4`，构建成功 | 保留并审慎升级 |
| Xray | `1.8.24` | `26.11.9`；其 go.mod 要求 Go 1.27 |
| Sing-box | `1.10.0` | `1.14.4`；其 go.mod 要求 Go 1.25.5 |
| Actions Go | `1.23.x` | 两个新版核心都不能仅改 PKG_VERSION |

- Sing-box 1.14.4 发布资产中有 `linux-mipsle-softfloat-musl`；这是候选资产，不等于已经在本固件工具链/设备上验证 ABI、性能和功能。
- 倾向只维护一个代理核心，当前首选 Sing-box（因 Hysteria2 需求及协议覆盖），但必须先拿真实节点/订阅测试转换兼容性。若实际配置依赖 Sing-box 不支持或转换有差异的 Xray 特性，再重新评估；不要预先双核心常驻占 Flash。

### USB / SATA / Storage

- 基线内核配置 `trunk/configs/boards/XY-C1/kernel-4.4.x.config`：`CONFIG_MODULES=y`、`CONFIG_BLK_DEV_SD=y`、`CONFIG_SATA_AHCI=y`、`CONFIG_SATA_AHCI_PLATFORM=y`、`CONFIG_SATA_PMP=y`；USB Host 支持开启，`CONFIG_USB_STORAGE=m`。
- 模板还启用 USB、ANTFS、FAT、exFAT、EXT2/3/4；SMB/FTP 编译选项为 y。
- 目标策略：保留有用的驱动/文件系统/共享能力；确认 Samba/FTP 默认不启动或默认禁用。先区分内核能力、ROMFS 程序、运行中服务，禁止按“编进固件=常驻耗电”推断。
- `STORAGE_ENABLED` 会受 `CONFIG_BLK_DEV_SD` 等条件影响，删 SCSI/SD 可能连带改变 libdisk、存储工具和 WebUI，不应为了少量空间盲目关闭。

### CPU OC

- 当前模板是 `CONFIG_FIRMWARE_INCLUDE_OC=y`、`CONFIG_FIRMWARE_MT7621_OC="0x312"`。
- `trunk/configure` 在 OC=y 时写入 `CONFIG_MT7621_OC`；`trunk/linux-4.4.x/arch/mips/ralink/mt7621.c` 的 `ralink_clk_init()` 初始化期间修改 PLL 寄存器。
- 这不是按负载动态切换频率的 DVFS；CPU 空闲状态/idle 降功耗不等于 PLL 自动降回默认频率。不要把“空闲会省动态功耗”表述成“频率自动降低”。
- 目标基线建议 OC=n；若未来用户重新考虑超频，需实测温度、稳定性、吞吐和功耗后再决定。本文不推断 `0x312` 的精确 MHz。

## 3. 讨论形成的目标设计（偏好，不代表已实现）

1. **网络基础**：SmartDNS 常驻；SFE；IPv4/IPv6、DHCP、NAT 正常；不需要 QoS/SQM。
2. **代理**：优先一个 Sing-box 核心；支持真实所需协议，重点验证 VLESS/Reality、Trojan、Shadowsocks、Hysteria2；由新/重构 WebUI 生成 Sing-box JSON，先 check 再原子应用；启停、状态、日志、故障回滚完整。
3. **代理构建解耦**：新增独立代理核心/管理 UI 构建选项；不要为了编译 Sing-box 被迫打开整套旧 `SHADOWSOCKS` 依赖。Xray 默认不编译，除非真实节点证明需要。
4. **存储扩展**：保留 USB/SATA/SCSI、常用文件系统和 SMB/FTP 能力；服务默认关闭/按需启用。N1 才是日常 NAS/容器主力。
5. **远程组网**：Tailscale 先部署在 N1，必要时 N1 广播 LAN subnet；C5 暂不新增 Tailscale 插件/WebUI。
6. **CPU**：默认 OC=n；不以“编译超频只设上限”的假设配置。
7. **空间**：32MB 是整颗 Flash 标称值，不是全都能给 TRX。先查 XY-C1 实际 MTD 分区/升级限制，再定核心是否内置。

## 4. N1/云端下载核心：可行性与边界

- DHCP 不是访问 N1 的前提：只要 C5 LAN/bridge 和自身 LAN IP 已就绪，N1 使用同网段静态 IP，C5 可直接 ARP/IP 访问；不必等 DHCP 服务启动。
- 可选设计：固件内保留已验证核心作为 fallback；LAN ready 后可从 N1 静态 IP 下载新核心到 RAM（例如 `/tmp`），做 SHA256/签名校验、架构/ABI/版本检查，原子替换运行副本。
- 不建议把 N1、WAN、DNS 或云端下载设为路由启动的硬依赖。N1 离线/断电/升级失败时，C5 仍须提供普通路由、DHCP、SmartDNS；代理可降级关闭。
- RAM 方案还需验证 /tmp 是否可执行、内存峰值、下载超时/重试、校验信任链、启动顺序和失败回滚；512MB RAM 不代表核心运行开销已验证。

## 5. 当前固件实测基线（不能误读）

Actions Run `43541384942`（SHA `59105690...`）：
- SquashFS：`10333.52 Kbytes (10.09 Mbytes)`；未压缩文件系统：`30422.17 Kbytes (30.42 Mbytes)`。
- 日志中的最终 TRX 显示约 `13M`，uImage data size `12,626,097 bytes`；Artifact ZIP `12,579,693 bytes`。
- 此构建没有 Xray/Sing-box；因此该结果仅证明**当前不含代理核心的模板**能成功构建，不能证明加入 Sing-box 后仍满足 Flash/分区限制。
- 32MB Flash 可用空间、MTD 分区边界、固件最大允许尺寸仍待实际核对；不得简单用 32MB 减 TRX 大小推算余量。

## 6. 下一次继续讨论时的精确切入点

按顺序核实，先讨论/测量，不自动改代码：

1. 获取用户实际在用的节点/订阅协议清单（脱敏）；判定 Sing-box 单核心覆盖率。
2. 检查 Sing-box 1.14.4 MIPSLE-musl 发行二进制或本仓库交叉编译的 ABI、大小、内存、启动和 500M 网络吞吐；验证 Hysteria2。
3. 设计 Sing-box WebUI/配置模型/订阅导入/JSON check/原子应用；旧 WebUI 不得假装支持 Hysteria2。
4. 查板级 MTD 分区、TRX 上限和 Flash 实际余量，再决定核心内置还是“内置 fallback + N1 可选 RAM 更新”。
5. 复核 SmartDNS 与代理 DNS/透明代理的端口、分流和防循环关系。
6. 把模板里的 OC 改为默认 n 的方案与存储服务默认关闭策略分开评审；USB/SATA 存储能力默认保留。
7. 完成参数设计后，用户明确要求实施，再按项目开发协议操作。

## 7. 关键源码定位

- 模板：`trunk/configs/templates/XY-C1.config`
- 构建条件：`trunk/user/Makefile`
- Xray：`trunk/user/xray/Makefile`
- Sing-box：`trunk/user/sing-box/Makefile`
- SmartDNS：`trunk/user/smartdns/Makefile`
- WebUI：`trunk/user/www/n56u_ribbon_fixed/Shadowsocks.asp`、`trunk/user/www/Makefile`
- 内核选项：`trunk/configs/boards/XY-C1/kernel-4.4.x.config`
- OC 实现：`trunk/linux-4.4.x/arch/mips/ralink/mt7621.c`、`trunk/configure`

**状态：本文件只保存讨论上下文。未修改 XY-C1 模板、Makefile、WebUI、内核配置或任何产品代码；没有形成实施提交或硬件验证结论。**

## 8. 2026-11-14 continuation: C5 must be independently usable; proxy UI/rules redesign

### Newly confirmed user requirements

- C5 must have its **own independent proxy capability**, not merely be a transparent forwarder to N1. Reasons: N1 downtime/reconfiguration, disaster recovery, and having a way to fetch tools/subscriptions/firmware-related resources when N1 is unavailable. Local firmware upload through the LAN WebUI itself does not require WAN proxy; downloading firmware/resources from the Internet may.
- Normal operation may still use N1 as the preferred upstream: C5 performs LAN policy routing, domestic traffic exits WAN directly, selected overseas traffic goes through the N1 SOCKS5 endpoint, and N1 performs its own independent proxy/routing.
- When N1 is unavailable, C5 must be able to select and run its own node directly, without requiring N1, N1 DNS, or N1 storage. This is a standalone fallback/disaster-recovery path, not a cloud-only or N1-dependent design.
- N1's LAN address/subnet is **not hard-coded**. WebUI must allow the user to define one or more bypass/exempt LAN IPs/subnets (ideally both IPv4 and IPv6, with optional device/MAC selection if supported). These entries generate high-priority direct/bypass rules automatically. Distinguish source-device bypass (N1-originated traffic) and destination bypass (C5-to-N1 SOCKS endpoint must go directly over LAN); both must be enforced.
- User explicitly values the old WebUI's breadth (client, node management, advanced settings, rules, logs, help). Do not reduce the design to a raw JSON editor or a single core toggle. Preserve useful functions, redesign their boundaries/UX, and do not claim support for protocols not actually implemented.

### Current legacy WebUI/source findings

- trunk/user/www/n56u_ribbon_fixed/Shadowsocks.asp is a multi-tab ShadowSocksR Plus+ page with tabs for client, node management, advanced settings, rule management, logs, help. It is more than a simple node picker.
- Client section has main node, separate game UDP relay node, multi-thread selection, run modes (GFW list / bypass China IP / global / overseas return), proxy ports, LAN control (all or selected IPs), DNS proxy method, optional chinadns-ng, domestic DNS choices, etc.
- Node management supports legacy SS/SSR, VMess/VLESS, Trojan and SOCKS5 entries/import logic. It does not currently provide genuine Sing-box config generation or a real Hysteria2 node editor. Old V2Ray QUIC is not Hysteria2.
- Important distinction:
  1. Main proxy node can be a **remote SOCKS5 upstream** (currently used to hand C5's selected transparent traffic to N1).
  2. The page's separate socks5_enable section starts a **local SOCKS5 listener on C5** for LAN clients. It is an independent optional feature; don't conflate it with the main SOCKS5 upstream.
- Current SOCKS5-main adapter: trunk/user/shadowsocks/ss/gensocks.lua launches ipt2socks with -s server -p port -l 1080 -r -4 -R. Upstream ipt2socks option semantics: -r reuse-port, -4 IPv4-only (disables IPv6), -R use TCP REDIRECT rather than TPROXY; TCP and UDP are enabled by default unless -T/-U is passed. Thus the old bridge is not simply “TCP-only”; it explicitly disables IPv6, and real UDP depends on the N1 SOCKS5 server supporting UDP ASSOCIATE. Verify end-to-end TCP/UDP/IPv4/IPv6 on hardware. Source: https://github.com/zfl9/ipt2socks
- Legacy ss-rules builds ipset + iptables rules and has domestic/direct, GFW, overseas and global modes; it also has custom domain/IP white/blacklists, LAN lists, and WAN IP exception lists. This policy capability should be mapped into a clean, explicit rule model, not lost during a core change.
- Existing DNS/updates:
  - Page has manual “update now” buttons and separate automatic-update toggles for chnroute and GFWList.
  - shadowsocks.sh schedules optional daily updates at 07:00 for chnroute and 08:00 for GFWList when the toggles are enabled.
  - ss_chdns enables chinadns-ng and china_dns chooses its domestic DNS upstream. The old script loads /etc/storage/chinadns/chnroute.txt into the china ipset, and if /etc/storage/chinadns/chnlist_mini.txt exists, passes it to chinadns-ng for domain/CDN classification. No dedicated chnlist_mini updater was identified in the checked UI/source paths.
  - chinadns-ng in the legacy path listens on port 65353; its overseas DNS upstream is the local DNS proxy at 127.0.0.1:5353. The DNS chain must be diagrammed and tested to avoid loops with SmartDNS, dnsmasq and sing-box DNS.
  - update_gfwlist.sh downloads to /tmp and replaces the persistent copy, but currently removes the existing file before moving the new one. Redesign update path as fail-safe: stage in RAM, verify HTTP result/content/format/size/count/checksum as appropriate, then atomic replace; preserve last-known-good data on every failure.
- Keep chinadns-ng as an optional independently selected DNS policy module only if it proves useful beside SmartDNS. Do not automatically chain SmartDNS, chinadns-ng, dnsproxy/dns2tcp and sing-box DNS together. Choose one authoritative DNS flow per profile and document its upstreams/ports.

### Proposed C5 architecture (discussion direction, not implementation authorization)

1. One current, verified Sing-box candidate may provide C5's own direct node outbounds (SS, VMess, VLESS/Reality, Trojan, Hysteria2) and the N1 SOCKS5 upstream as another outbound. A single policy engine can route the appropriate traffic to either the N1 SOCKS5 outbound or a directly configured C5 node. Verify each node format/import against user's actual profiles. Sing-box has no ShadowsocksR outbound; SSR compatibility must be an explicit decision after checking real node inventory, not silently claimed or dropped.
2. Use explicit Linux TProxy inbound + C5-managed iptables/ipset rules as the first compatibility candidate for this Padavan 4.4.x environment. Do not assume Sing-box's newer nftables auto_redirect path is available/appropriate on the legacy Padavan kernel/userspace. Validate kernel TPROXY, policy routing, marks, IPv4/IPv6, TCP/UDP, SFE interaction and rollback on XY-C1 hardware.
3. SOCKS5 outbound is a first-class outbound in Sing-box and supports TCP and UDP by default; still verify that the actual N1 server has UDP ASSOCIATE enabled and carries the traffic classes the user needs. Source: https://sing-box.sagernet.org/configuration/outbound/socks/
4. C5 node selection should support at least profiles/modes: (a) preferred N1 SOCKS5 upstream, (b) C5 direct node, (c) direct/disabled for emergency diagnosis. Optional automatic failover can be considered with health checks, but must be visible, configurable, loop-safe and deterministic; don't silently switch routes without status.
5. Sing-box documentation confirms TProxy inbound is Linux-only; rule-sets can be local or remote, remote sets have update intervals/cache behavior, and local files can be reloaded on change. For this product, prefer UI-managed local last-known-good files and explicit reload/apply rather than making boot depend on a remote fetch. Sources:
   - https://sing-box.sagernet.org/configuration/inbound/tproxy/
   - https://sing-box.sagernet.org/configuration/rule-set/
   - https://sing-box.sagernet.org/configuration/route/rule/
6. The Sing-box 1.14.4 Hysteria2 outbound is a genuine Hysteria2 protocol implementation, not a legacy V2Ray QUIC transport. Upstream docs: https://sing-box.sagernet.org/configuration/outbound/hysteria2/
7. Keep proxy-core build, legacy SSR compatibility, rule-data/update managers, DNS modules (SmartDNS/chinadns-ng), and WebUI feature inclusion as separate build switches/dependencies. Enabling Sing-box must not drag in the entire old SHADOWSOCKS stack unless a specific compatibility function truly requires it.

### Rules/data lifecycle and WebUI requirements

- On boot: use a bundled baseline rule seed from firmware plus the last successfully installed persistent rule set, if present and valid. Do not require WAN, N1 or DNS downloads to boot; no mandatory rule download on every boot.
- After C5 WAN and DNS are ready, user can click “Update now” independently for China IP routes, overseas/GFW domains, adblock/custom lists and other enabled rule sources. Optional scheduled updates should be separately configurable and default off unless user requests otherwise.
- Downloads should go to RAM staging (/tmp), with strict validation. Only after successful validation should files be atomically committed to persistent storage (e.g. /etc/storage after partition/capacity validation). Failed or partial downloads must not replace the last-known-good rules. Record source URL, version/hash, item count, file size, last attempt, last successful update, and error.
- Applying IP rule sets should minimize transient policy gaps (e.g. build new ipset and swap rather than flush-first where possible); reload only the necessary rules/DNS service when safe.
- 512MB RAM is useful for staging and in-memory ipset/rule data, but is not proof that an arbitrarily large geo database or Sing-box runtime is harmless. Measure core RSS/peak memory, UDP state, DNS cache, ipset size and update peak separately. RAM staging is ephemeral; a persistent last-good copy is still required.
- WebUI design should retain capabilities but separate concepts:
  - Overview/status: selected mode/upstream, active core/version, service health, DNS health, TCP/UDP and IPv4/IPv6 status, rule update status, CPU/RAM, logs.
  - Client/policy: run mode, direct-vs-proxy policy, ports, LAN device selection, per-device forced proxy/direct, N1 SOCKS5 endpoint and explicit N1 bypass/exempt IP/subnets.
  - Node/subscription management: C5-owned direct nodes and remote SOCKS5 upstream as separate object types; clear support matrix, imports, validation and test connection.
  - Rules: China CIDR, overseas/GFW domain/IP rules, custom whitelist/blacklist, source/destination IP exceptions, manual refresh and optional schedule.
  - DNS: select one coherent SmartDNS/chinadns-ng/sing-box DNS design with explicit domestic/foreign upstreams and no loops.
  - Advanced: local C5 SOCKS5 listener as independent optional feature, timeouts/retry/health-check/resource settings, and troubleshooting.
  - Logs/help: actionable errors, last-good rollback state, exported diagnostics.
- No hard-coded N1 address. User-configurable exceptions must be preserved across firmware upgrades and subscription imports.
- Before implementation obtain redacted user node/subscription protocol inventory; verify C5's own direct Hysteria2/VLESS Reality/SS/VMess/Trojan; test SOCKS5 TCP/UDP to N1 and N1 outage fallback; measure flash/partition/core size/RAM/throughput; validate DNS and policy rules. Do not modify product code until user explicitly asks to implement and project protocol is followed.

**State update:** this section is user-approved discussion context only. No product code changed and no firmware/hardware validation was performed.


## 9. 2026-11-14 continuation: SmartDNS remains an independent service and WebUI

### User question and clarified decision

- User asked whether SmartDNS settings should be absorbed into the proxy/“scientific internet” page, or whether SmartDNS has its own independent WebUI.
- **Decision: SmartDNS keeps its own independent WebUI and service configuration. It must not be folded into the proxy page.** The proxy page may expose a small integration panel for selecting the DNS path/egress and displaying DNS health, but it must not duplicate SmartDNS's resolver configuration.

### Current repository evidence (live main at discussion time)

- Standalone UI exists at `trunk/user/www/n56u_ribbon_fixed/Advanced_smartdns.asp`. It has its own form with `sid_list=SmartdnsConf;`, invokes `smartdns_status()`, and loads through its own menu entry (`show_menu(5,16)`).
- The page has five independent panels: `wnd_sm_cfg`, `wnd_sm_exp`, `wnd_sm_sec`, `wnd_sm_dns`, and `wnd_sm_cou`. It has separate controls for service enable/status, listener name/port, TCP and IPv6 listeners, dnsmasq integration mode, cache/persistence, TTL and response limits, speed-check and dual-stack behavior, a configurable upstream-server list, groups and IP filters, and custom address/blacklist/whitelist/adblock configuration.
- Runtime configuration is independently generated by `trunk/user/smartdns/smartdns.sh` from SmartDNS-specific NVRAM values into persistent SmartDNS config under `/etc/storage`. The package has its own `trunk/user/smartdns/Makefile`; `trunk/user/Makefile` gates the package via `CONFIG_FIRMWARE_INCLUDE_SMARTDNS`, and `trunk/user/www/Makefile` independently removes the SmartDNS page when that option is disabled.
- The legacy proxy page `Shadowsocks.asp` also has DNS-related fields (`pdnsd_enable`, `ss_chdns`, `china_dns`, `ssp_dns_ip`). These are proxy/DNS integration and legacy resolver controls, not a reason to move all SmartDNS service settings into the proxy UI. They need a compatibility audit and a clean ownership boundary.
- This snapshot describes the existing repository implementation only; it does not claim the present SmartDNS WebUI is complete, modern, or bug-free.

### Proposed ownership boundary

1. **SmartDNS WebUI owns DNS service configuration**: service enable/status; LAN/local bind addresses and ports; UDP/TCP and IPv4/IPv6; upstream servers and transports (only those supported by the pinned binary/build); server groups and nameserver/domain rules; cache, TTL, prefetch and speed selection; DNS address/blacklist/whitelist policies; IPSet/nftset integration where actually supported; query/status/log diagnostics. Keep these settings usable even when the proxy plugin is disabled.
2. **Proxy WebUI owns proxy traffic policy and DNS integration only**: choose whether DNS requests use SmartDNS; choose the egress for proxy-bound DNS upstreams (N1 SOCKS5, C5's own proxy, or direct only when explicitly selected); show endpoint/health and integration status; protect proxy-node bootstrap resolution and bypass rules against DNS/proxy loops. Do not duplicate DNS upstream lists, cache/TTL tuning, or SmartDNS rule editors in the proxy page.
3. Treat SmartDNS as the LAN DNS resolution plane and Sing-box as the proxy transport/routing plane. Connect them through explicit local endpoints and named egress paths; neither service should become an unconditional boot dependency of the other.
4. Domain-to-resolver decisions (which DNS group answers a domain) and packet-egress decisions (whether the resulting connection uses WAN, N1, or a C5 node) are related but distinct. Reuse common rule data where practical, while generating service-specific outputs and avoiding conflicting duplicate rule lists.

### Proxy-assisted DNS and loop prevention

- Current upstream SmartDNS documentation supports named proxy endpoints via `proxy-server` and selecting one per upstream with `server ... -proxy <name>`. It documents SOCKS5 and HTTP proxies; SOCKS5 supports TCP and UDP, while HTTP proxying is TCP-only. See https://pymumu.github.io/smartdns/config/proxy/ and https://pymumu.github.io/smartdns/configuration/.
- The current repository's `smartdns.sh` generator does not yet expose or emit a `proxy-server` configuration, so this capability must not be described as already integrated in the Padavan WebUI. It is a candidate for a later, tested integration.
- Candidate flow: LAN clients -> dnsmasq/SmartDNS (one selected authoritative local DNS path) -> SmartDNS chooses domestic/direct or foreign/proxy upstream group -> selected upstream transport goes through the configured N1 SOCKS5 endpoint or a loopback SOCKS5 listener backed by C5's active Sing-box outbound. Validate actual TCP/UDP behavior on XY-C1 before relying on it.
- Prevent circular dependencies explicitly: resolving a proxy-node hostname or N1 endpoint must use a bootstrap/direct resolver and a direct/bypass route; proxy DNS must never require resolving itself through the same not-yet-connected proxy. Prefer numeric endpoints when appropriate, but retain explicit hostname bootstrap support.
- Do not automatically chain SmartDNS, chinadns-ng, dnsproxy/dns2tcp, dnsmasq forwarding, and Sing-box DNS together. Select one authoritative DNS path per operating profile and document each upstream/port/detour.

### UI direction

- Keep a dedicated SmartDNS menu/page separate from the proxy page, with ordinary settings first and advanced resolver/rule settings grouped below.
- The proxy page should offer a concise integration selector and status, with a link to the independent SmartDNS settings page.
- Existing SmartDNS page is a legacy implementation baseline, not a commitment to retain every current NVRAM field or its current layout. Redesign for clarity while preserving useful capabilities and backward-compatible settings where feasible.

**State:** discussion/design only. No product source, SmartDNS runtime, proxy code, WebUI, or firmware template was changed. No target build or XY-C1 hardware test was performed.
