# Development log archive index

- Root `dev_log.md` holds the newest 20 complete records, newest first.
- Archive parts are chronological oldest-to-newest. For this rolling-window rotation, keep the 18 previous newest records from the root first, then reverse the newly appended archived records.
- **Original source:** `dev_log.md` blob `6dc04f5aef231f9f4ea0e100abe87fce22c49fc0`, 984527 bytes, 663 records. Every historical record body is byte-identical to its source record.
- **Shard policy:** target 256–384 KiB; hard cap 512 KiB; split only between complete dated records.

| Archive file | Records | Date order | Bytes | Git blob SHA |
|---|---:|---|---:|---|
| `docs/dev-log/2026/11/part-001.md` | 294 | 2026-11-05 → 2026-11-06 | 357382 | `5d2cfa2439d7b1be33333cee6964c005c81dc5ad` |
| `docs/dev-log/2026/11/part-002.md` | 275 | 2026-11-06 → 2026-11-08 | 357977 | `164858e25153406e26e3f2b9bb6f3904557d5cda` |
| `docs/dev-log/2026/11/part-003.md` | 78 | 2026-11-08 → 2026-11-09 | 216933 | `f5e28c56ef9055a12f1b0ce7bc9ebd6fbc438233` |
| `docs/dev-log/2026/11/part-004.md` | 1 | 2026-11-09 → 2026-11-09 | 2653 | `f7016653093548013d8197b6925e7a69cc7b24db` |
| `docs/dev-log/2026/11/part-005.md` | 5 | 2026-11-09 → 2026-11-09 | 17212 | `b462b921d00e837bf5fd7b43d76f780a371bd9fb` |


- **Post-migration rotation:** the first evicted root-window record is preserved verbatim in `part-003.md`; its original root source blob was `733d3c678575611642b3f69fe4cfe5e19ece9b57`.

- **Post-migration rotation:** the second evicted root-window record is preserved verbatim in `part-003.md`; its pre-rotation root source blob was `93b0fe7c1d560797a339777b4fe264ba999a08cd`.

- **Post-migration rotation:** the third evicted root-window record is preserved verbatim in `part-003.md`; its pre-rotation root source blob was `187e6df1bb938adb753b9d8eda6e8b36ba7d62c7`.
- **Rolling-window continuation:** the 2026-11-09 05:38 CST record evicted from the root window is preserved verbatim in part-004, immediately after part-003's 05:30 CST final record.

- **Rolling-window continuation:** the 2026-11-09 05:57 CST root record evicted at this rotation is preserved verbatim in part-005, immediately after part-004's 05:38 CST record.

- **Rolling-window continuation:** the 2026-11-09 06:40 CST `xt_webstr` entry evicted at this rotation is appended after the 05:57 CST entry in part-005; original root source blob: `3df1cb86b5ea6f3f3324027346fba2da5ae2edfb`.

- **Rolling-window continuation:** the 2026-11-09 07:24 CST switch CLI stability entry evicted at this rotation is appended after the 06:40 CST entry in part-005; original root source blob: `55e0caeac9db3870dec54b0d40ef53e2b7d31a36`.

- **Rolling-window continuation:** the 2026-11-09 07:33 CST Clause 45 switch CLI argument-order entry evicted at this rotation is appended after the 07:24 CST switch CLI record in part-005.
- **Rolling-window continuation:** the 2026-11-09 07:41 CST Clause 22 MII CLI/MDIO timeout entry evicted at this rotation is appended after the 07:33 CST entry in part-005.
