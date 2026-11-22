# Development log archive index

- Root `dev_log.md` is a rolling window of 20–119 complete records, newest first; when the next entry would make 120, archive the oldest 100 complete records verbatim and retain the newest 20 in the root.
- Archive parts are chronological oldest-to-newest. For this rolling-window rotation, keep the 18 previous newest records from the root first, then reverse the newly appended archived records.
- **Original source:** `dev_log.md` blob `6dc04f5aef231f9f4ea0e100abe87fce22c49fc0`, 984527 bytes, 663 records. Every historical record body is byte-identical to its source record.
- **Shard policy:** target 256–384 KiB; hard cap 512 KiB; split only between complete dated records.

| Archive file | Records | Date order | Bytes | Git blob SHA |
|---|---:|---|---:|---|
| `docs/dev-log/2026/11/part-001.md` | 294 | 2026-11-05 → 2026-11-06 | 357382 | `5d2cfa2439d7b1be33333cee6964c005c81dc5ad` |
| `docs/dev-log/2026/11/part-002.md` | 275 | 2026-11-06 → 2026-11-08 | 357977 | `164858e25153406e26e3f2b9bb6f3904557d5cda` |
| `docs/dev-log/2026/11/part-003.md` | 78 | 2026-11-08 → 2026-11-09 | 216933 | `f5e28c56ef9055a12f1b0ce7bc9ebd6fbc438233` |
| `docs/dev-log/2026/11/part-004.md` | 1 | 2026-11-09 → 2026-11-09 | 2653 | `f7016653093548013d8197b6925e7a69cc7b24db` |
| `docs/dev-log/2026/11/part-005.md` | 14 | 2026-11-09 → 2026-11-09 | 46475 | `175b66ce369f061c4ee8b872c3fc01f73032d28b` |
| `docs/dev-log/2026/11/part-006.md` | 100 | 2026-11-09 → 2026-11-10 | 239599 | `453c9e3b28f0513581df22967f622690b79b0875` |
| `docs/dev-log/2026/11/part-007.md` | 100 | 2026-11-10 → 2026-11-11 | 260614 | `09670480113fb49aff423bdddf0a58f5cdb4f537` |
| `docs/dev-log/2026/11/part-008.md` | 100 | 2026-11-11 → 2026-11-13 | 251751 | `b9a16f053c5dbcd19bd3285e12da2d3876482dff` |
| `docs/dev-log/2026/11/part-009.md` | 100 | 2026-11-13 14:21:06 CST (UTC+8) → 2026-11-15 — Add shared PID/start-time file identity helper | 219669 | `3dfc7bffc0e4f251aa15e58c9d1586e29f26096c` |
| `docs/dev-log/2026/11/part-010.md` | 100 | 2026-11-14 → 2026-11-15 14:18 CST | 119850 | `d89ebd2529d3f8841d8145e212dff6d2d504c20d` |
| `docs/dev-log/2026/11/part-011.md` | 100 | 2026-11-15 → 2026-11-15 | 110042 | `41f363e173869e271c08b603f4f06b990aa4beec` |
| `docs/dev-log/2026/11/part-012.md` | 100 | 2026-11-15 — Check physical WAN interface bring-up before protocol startup → 2026-11-16 — Align storage IPTV source contract with aggregated restart results | 133040 | `07cadc2b9a77f5e7735faf769f954d9980ac4ebd` |
| `docs/dev-log/2026/11/part-013.md` | 100 | 2026-11-16 — Propagate loopback interface and route startup failures → 2026-11-17 — Repair extension-hook CI contracts after implementation | 123231 | `19c3d8ccc2d16c11936d3b498808a5ee886722e5` |
| `docs/dev-log/2026/11/part-014.md` | 100 | 2026-11-17 — Repair escaped firewall checker regex and HTTPD hook helper signature → 2026-11-17 — Apply all parser-supported DHCP MTU values, including explicit 1500 reset | 120777 | `82560e2f808f6c66850a84acd762769bf585d39a` |
| `docs/dev-log/2026/11/part-015.md` | 100 | 2026-11-17 — AliDDNS WAN IPv6 source correction → 2026-11-17 — Align HTTPD's duplicate storage check with tri-state hotplug topology | 111870 | `f3f0f8caa4ebe40e7500ad7f29a38b3cae2e7167` |
| `docs/dev-log/2026/11/part-016.md` | 100 | 2026-11-17 — Restore process workflow pull_request YAML nesting → 2026-11-17 — Stop MSD Lite during WAN teardown | 105054 | `21834b5077a752e54020339944937c5e0887c411` |
| `docs/dev-log/2026/11/part-017.md` | 101 | 2026-11-17 — Keep the common WAN checker out of redundant XY-C1 trigger fan-out → 2026-11-19 — Reject the remaining IANA-reserved IPv6 GUA ranges in AliDDNS | 121900 | `b05af15814dcbc355a8d13dcff0762ea874567f4` |
| `docs/dev-log/2026/11/part-018.md` | 101 | 2026-11-19 — Serialize Inadyn stop/start and configuration notifications → 2026-11-19 09:20 EST — Correct the committed AI handoff checkpoint contract | 123512 | `7c69f94c23df8035a0d8d3cdf357c54bc80c2be8` |
| `docs/dev-log/2026/11/part-019.md` | 100 | 2026-11-19 → 2026-11-20 | 114230 | `bf5f91f6e47631f2f1331a3be3dd46c3515d4a89` |
| `docs/dev-log/2026/11/part-020.md` | 100 | 2026-11-19 — Transactional LAN DHCP deconfig snapshot → 2026-11-20 — Escape untrusted UPnP mapping output before SOAP XML serialization | 129506 | `ca34b9403978210508f3102ec5cbd01579ecd198` |
| `docs/dev-log/2026/11/part-021.md` | 100 | 2026-11-20 → 2026-11-21 | 107650 | `f80bf2d087fa1c313cb671f2108e3279092db234` |
| `docs/dev-log/2026/11/part-022.md` | 100 | Correct watchdog PID-file and procfs fixture escaping → PPP and WAN rollback-state consistency | 113949 | `3bb594ab26cd04f116c9f63f9489ee8d6d8be9d6` |
| `docs/dev-log/2026/11/part-023.md` | 100 | 2026-11-21 → 2026-11-21 | 109696 | `d896a53f5bd56915dbabb5ec4987a2dc069d8388` |
| `docs/dev-log/2026/11/part-024.md` | 100 | 2026-11-21 — Snapshot VPN remote-LAN route ACL before route mutation → 2026-11-21 — Fail closed on WAN startup protocol reads | 129424 | `359eae06c5b550cf68dc48710a91ea546551ec8f` |
| `docs/dev-log/2026/11/part-025.md` | 101 | 2026-11-22 08:04 CST — Repair WAN startup checker to scope the matching error gate → 2026-11-22 — Keep physical storage-unplug cleanup durable until marker acknowledgement | 126105 | `daf3875420e99758082b0d43ab2e4b4b2cd57ab2` |


- **Post-migration rotation:** the first evicted root-window record is preserved verbatim in `part-003.md`; its original root source blob was `733d3c678575611642b3f69fe4cfe5e19ece9b57`.

- **Post-migration rotation:** the second evicted root-window record is preserved verbatim in `part-003.md`; its pre-rotation root source blob was `93b0fe7c1d560797a339777b4fe264ba999a08cd`.

- **Post-migration rotation:** the third evicted root-window record is preserved verbatim in `part-003.md`; its pre-rotation root source blob was `187e6df1bb938adb753b9d8eda6e8b36ba7d62c7`.
- **Rolling-window continuation:** the 2026-11-09 05:38 CST record evicted from the root window is preserved verbatim in part-004, immediately after part-003's 05:30 CST final record.

- **Rolling-window continuation:** the 2026-11-09 05:57 CST root record evicted at this rotation is preserved verbatim in part-005, immediately after part-004's 05:38 CST record.

- **Rolling-window continuation:** the 2026-11-09 06:40 CST `xt_webstr` entry evicted at this rotation is appended after the 05:57 CST entry in part-005; original root source blob: `3df1cb86b5ea6f3f3324027346fba2da5ae2edfb`.

- **Rolling-window continuation:** the 2026-11-09 07:24 CST switch CLI stability entry evicted at this rotation is appended after the 06:40 CST entry in part-005; original root source blob: `55e0caeac9db3870dec54b0d40ef53e2b7d31a36`.

- **Rolling-window continuation:** the 2026-11-09 07:33 CST Clause 45 switch CLI argument-order entry evicted at this rotation is appended after the 07:24 CST switch CLI record in part-005.
- **Rolling-window continuation:** the 2026-11-09 07:41 CST Clause 22 MII CLI/MDIO timeout entry evicted at this rotation is appended after the 07:33 CST entry in part-005.

- **Rolling-window continuation:** the 2026-11-09 08:14 CST NFS mount-inventory record evicted at this rotation is appended verbatim after the 07:33 and 07:41 CST records in part-005.

- **Rolling-window continuation:** the 2026-11-09 08:24 CST safe storage-removal inventory record evicted at this rotation is appended after the 08:14 CST NFS inventory record in part-005.

- **Rolling-window continuation:** the 2026-11-09 08:56 CST FULLCONENAT garbage-collection record evicted at this rotation is appended after the 08:24 CST safe-removal inventory record in part-005.

- **Rolling-window continuation:** the 2026-11-09 09:04 CST wireless radio-state record evicted at this rotation is appended after the 08:56 CST FULLCONENAT garbage-collection record in part-005.

- **Rolling-window continuation:** the 2026-11-09 09:13 CST guest AP scheduler/bridge-state record evicted at this rotation is preserved verbatim after the 09:04 CST record in part-005; original root source blob: `3bf6391ddb348e17f4c4955ba7658ec5444c8a10`.


- Rolling-window continuation: the ## 2026-11-09 09:18 CST — Gate AP and APCLI bridge publication on successful interface-up commands record evicted from the root 20-record window is preserved verbatim at the end of part-005; its original root record blob is d7c5fa747741c15ee95493156f9a96c3c3bb2e86.


- **Rolling-window continuation:** the `## 2026-11-09 09:23 CST — Gate WDS bridge attachment on successful interface-up commands` record evicted at this rotation is preserved verbatim at the end of part-005; original record blob `d7c5c8b915645b963e1776f6a718a813110493f2`.


- **Rolling-window continuation:** the 2026-11-09 09:30 CST — Fail safe when RT3352 iNIC interface startup fails record evicted from the root 20-record window is preserved verbatim after the 09:23 CST WDS bridge record in part-005; original root `dev_log.md` source blob: `c4db82201c8961746a30fc2e82eb99e9bd2668d4`.


- **Rolling-window continuation:** the 2026-11-09 09:49 CST WAN runtime-state preservation record evicted at this rotation is appended verbatim after the 09:30 CST iNIC startup record in part-005; original root `dev_log.md` source blob: `99185fd796cc02826c37a0e9b59c0d7a8172de08`.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved verbatim, oldest first, in part-006; source root blob before rotation: `ddd8c0a88ee46188b2d98c6c04edb792a6700f8d`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved verbatim, oldest first, in part-007; source root blob before rotation: `4446bd2eb1896fdc75377b6068192e8dc897813a`. The new HTTPD lifecycle record and 19 newest prior records remain in root.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved verbatim, oldest first, in part-008; source root blob before rotation: `bfff83b85369ff39e8a4dfa85bf5d702e6c8076f`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved verbatim, oldest first, in part-009; source root blob before rotation: `7fe9963b0eeaa302e787340e847738fc24e3b6de`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved verbatim, oldest first, in part-010; source root blob before rotation: `8142461da3a0bb0bf54a334a147c6de9570806d7`. Root retains the newest 20 complete records.
- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved verbatim, oldest first, in part-011; source root blob before rotation: `8c496aa2921a5cb0ba62f93d9f392b15ffdc9e84`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries are preserved oldest-first in part-012; pre-rotation root blob: `db55a3ce0deb7397a834ef419a85cdb05fda2080`. The newest 20 existing records remain in root plus this correction record.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` already contained 120 records at parent main `0b93418caf9e04a41fbae0a23ebdcf234ee47ae7` are preserved verbatim, oldest first, in part-013; pre-rotation root blob: `ede01b542db44e4a577a607468e65dee383c9c82`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted from root `dev_log.md` at parent main `c6edf06e2a9d148f24757926488166dea8f9205b` are preserved verbatim, oldest first, in `part-014.md`; pre-rotation root blob: `76139df80957fc7cf5c784ccd166357bcaccbae0`. The newest 21 records remain in root (the three new records plus the 18 previous newest records).
- **Rolling-window continuation:** 100 oldest complete records evicted from root `dev_log.md` at parent main `be0f5b56efa203fc802334e08b846381e9a3f04a` are preserved verbatim, oldest first, in `part-015.md`; pre-rotation root blob: `8171f3c551386e19bb0c5c5599bb4a014fa2b6c3`. Root retains the new firewall entry and the 19 newest previous records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` contained 120 records at parent main `6330828fca8a4d6f091e07e468732f40991e0909` are preserved verbatim, oldest first, in `part-016.md`; pre-rotation root blob: `046bd6f0898aa233527607e0b6f23b83a48ed91c`. Root retains the newest 20 pre-existing complete records before the next entry is added.

- **Rolling-window catch-up:** 101 oldest complete records evicted from root dev_log.md at parent main 1f0f765cc24d9db43823af5bfdaffe10bacdd2d1 are preserved verbatim, oldest first, in part-017.md; pre-rotation root blob: b1ae258460fce3239fdddde39848c2ab8f06549d. Root had reached 121 records (one beyond the 120-record threshold), so this catch-up batch removes one additional oldest record to restore exactly the newest 20. No record was rewritten or discarded.

- **Rolling-window catch-up:** 101 oldest complete records evicted from root `dev_log.md` at parent main `f4fbd093e934fda5304aa796cba30d59eb29b3a4` are preserved verbatim, oldest first, in `part-018.md`; pre-rotation root blob: `9525b05ab8d82a37f5108c603d06614d06adb11a`. Root retains the newest 20 pre-existing records plus the new documentation milestone.

- **Rolling-window catch-up:** 100 oldest complete records evicted from root `dev_log.md` at parent main `7f04d46a955ec4358388b4b3fe7f9d23fc59bb49` are preserved byte-for-byte, oldest first, in part-019; pre-rotation root blob: `eb7190fedf832884fdaf9b48045223a91dcdcb22`. Root retains the 20 newest previous records plus this rotation checkpoint (21 total).

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 records at parent main `9d8a9036cdd350a3a45971d8cd5d766d5ec93762` are preserved verbatim, oldest first, in `part-020.md`; pre-rotation root blob: `798571a86689f0e29ba8b0b172bf1dee6ae80c9e`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries at parent main `065837e4dcc383ebe2d5276b8165ffa834f9dd3f` are preserved verbatim, oldest first, in `part-021.md`; root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 records at parent main `6882aefa2c70b7bfc53da5e770610dc45dc802ba` are preserved verbatim, oldest first, in `part-022.md`; pre-rotation root blob: `7784a0c51d20d27de06d009aea7aa9b669479fbb`. Root retains the newest 20 complete records, including the new WAN checker correction.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 records at parent main `7ad1cb24f67b9c9dffafeb1a492b2a41343a8413` are preserved verbatim, oldest first, in part-023; pre-rotation root blob: `7fc5db49fc36bc93efc1ee25e707d828304115de`. Root retains the newest 20 complete records.

- **Rolling-window continuation:** 100 oldest complete records evicted when root `dev_log.md` reached 120 entries at parent main `09232ef0e8677c59bfa49a538fb908ce31259d0f` are preserved verbatim, oldest first, in `part-024.md`; pre-rotation root blob: `2360c058e5a886c4fff533a5cb9b63a6007e879a`. Root retains the 20 newest complete records.


- **Rolling-window catch-up:** 101 oldest complete records evicted from root `dev_log.md` when it contained 121 entries at parent main `a3b7e95b71776808fbb6e6bddf273db8cf0c98f4` are preserved verbatim, oldest first, in `part-025.md`; pre-rotation root blob: `418727893b781c626ffa7995bc378ac8b5cff66f`. Root retains the 20 newest original records plus this rotation checkpoint (21 total). No entry was rewritten or discarded.
