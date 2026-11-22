## 2026-11-22 — Current development checkpoint

- Live main product SHA: `922841600a62aa8c4c98922da9002c6e4cdfb328`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (immutable; read-only).
- libdisk storage-share reliability now has two source fixes: directory enumeration fails closed on `readdir()` errors, and folder-list metadata is written to a same-directory temporary file, checked/flushed/synced/closed, then atomically renamed. Failures remove the temp file and preserve the last good snapshot.
- Exact libdisk GNU89 ASan/UBSan + source-contract run #45015894653 PASS on `9228416`.
- XY-C1 run #45015894601 is IN_PROGRESS in Build XY-C1 on that same product SHA; TRX verification and artifact upload await the build. Process inventory #45015894636, HTTPD security #45015894612 and NVRAM transaction audit #45015894614 were IN_PROGRESS at last poll.
- XY-C1 path filter now includes libdisk Makefile/source/headers (correcting the prior handoff mismatch). The workflow remains main-push-only; the focused libdisk workflow covers push/PR host tests.
- L3 NOT VERIFIED until same-SHA Build, TRX and artifact upload all PASS. Hardware L4 NOT VERIFIED.


---

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; development on `main`. Parent SHA for the Inadyn slice: `31b13e1d0bf8f1dd055d5ebd7cea7eaa662b13bb`. Process checker repair `1f7add7650acf76f7eb0c3d37cf9e7530612ded9`: exact Process lifecycle #45010890087 PASS and AI docs #45010890010 PASS. The subsequent detect-WAN checker-only commit `a5c583ca01a807501388a246cb5e4c7b32071fab`: exact detect-WAN #45011122521 PASS and AI docs #45011122439 PASS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and untouched.
- ZeroTier uploaded-planet single-snapshot source SHA `2db8f2afc34cfa133e8f0940d3f99f90d99a43ba`: ZeroTier L2 #45010582528 PASS and AI docs #45010582559 PASS; same-SHA XY-C1 #45010582583 Build, TRX and artifact-upload steps all PASS (L3). Hardware router L4 NOT VERIFIED.
- LAN teardown checked gateway/conntrack identity source SHA `587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c`: WAN DHCP, NVRAM audit and multiple subsystem checks PASS; route correction #45010773255 PASS; Process checker fix `1f7add7` exact Process #45010890087 and docs #45010890010 PASS; same-SHA XY-C1 #45010423615 Build/TRX/artifact steps all PASS (L3). Hardware router L4 NOT VERIFIED.
- detect-WAN checked gateway source/checker sequence `00fb9c3` + `a5c583c`: exact detect-WAN #45011122521 PASS and AI docs #45011122439 PASS.
- Latest main before this slice was `31b13e1d0bf8f1dd055d5ebd7cea7eaa662b13bb` (`fix(detect-internet): validate and retry probe settings reads`). Its NVRAM audit #45011627863, AI docs #45011627812, detect_internet #45011627797, Process inventory #45011627779 and Process fail-closed #45011627851 are PASS; XY-C1 #45011627817 was still running at Build step 37, with TRX/artifact pending at last poll.
- THIS COMMIT replaces the mixed-read Inadyn writer with a single checked transaction for all 22 DDNS keys, copies rotating-cache data into bounded owned storage, and validates before stage/publish. Any read, close, truncation or parse error preserves the previous config. DDNS enable status is checked as well.
- Closed AliDDNS CA/HMAC PR #1 (`ba48e22609e0c81ad757b06756fe91d05f39cbf6`) is already incorporated/evolved on main; do not duplicate-port its old branch.

## Completed source slices

- USB/storage hotplug: physical-unplug marker is retained until eject cleanup and marker acknowledgement both succeed; an action-completion latch avoids replaying completed cleanup when only marker ack is retried (source commit `d414a788`). `safe_remove_stor_device()` now queues the storage restart via `notify_rc_status("on_hotplug_mass_storage")` after releasing the storage operation lock, so a short-lived rc applet does not rely on its own `alarm(5)` to wake PID 1 (source commit `89a6e740`). The exact-head storage workflow is being reconciled with the actual source ordering.

- VPN client state policy: PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`; a follow-up removed a retained legacy argument tail from `on_client_ifup()`. OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hooks. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- OpenVPN exported profiles activate `remote-cert-tls server` to match the generated certificate's `serverAuth` EKU. Dual-stack modes 4/5 use a validated WAN IPv4 endpoint only as final fallback after DDNS and both IPv6 sources fail; IPv6-only modes remain unchanged.
- WSD HTTP POST framing source `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e`: bounded CRLF/exact-body reads, strict non-zero decimal Content-Length, duplicate/overflow rejection and 413 mapping. WSD framing #44942356801, AI docs #44942356848, Process inventory #44942356815, Process lifecycle #44942356893 and NVRAM #44942356787 passed. Exact XY-C1 #44942356809 passed Build, TRX verification and artifact upload; L3 applies to that exact source SHA.
- WSD LLMNR parsing: bounded uncompressed DNS question parsing, label and wire/text bounds, rejection of ambiguous NUL/dot bytes, and forced hostname NUL termination. The regression fixture and source-contract locator were corrected; #44945105615 passed on source/checker SHA `63854309f6c8b877563062119d0e169b7a7fd95f`.
- WSD XML output: request-derived RelatesTo and all seven configured GetResponse metadata values are escaped before XML insertion. On source `edc9d59456fe40d5c991b3a07507515ddf2c5eaf`, HTTP framing #44945383956, XML reflection #44945383982, Process inventory #44945383886 and NVRAM #44945383856 passed. Exact XY-C1 #44945383870 also passed Build, TRX verification and artifact upload; L3 is verified for this exact metadata source SHA.
- p910nd bidirectional SIGPIPE fix: source `23248f7df1575885e991fe518629bb974fc05c35`; host socketpair regression, p910nd, Process inventory, Process lifecycle and AI checks passed. Exact XY-C1 #44941685839 passed Build/TRX/artifact. L3 applies to that SHA; physical printer-disconnect L4 is NOT VERIFIED.
- MiniUPnP checked configuration preflight: source `be4952e90111ddea8f91684f6c3fe5ecf25be785`; checked protocol, secure mode, HTTP port, cleanup timers and mapping ranges. MiniUPnP, NVRAM, Process inventory, Samba and AI checks passed; exact XY-C1 #44941839623 passed Build/TRX/artifact. L3 applies to that SHA. The earlier redirect-description source `4a5139f484943abe3a5ac9cf0f6d56bc70067a56` also passed #44940761694. Hardware L4 remains NOT VERIFIED.
- RC AP/router-mode and LAN/WAN checked-read gates were expanded in commits `74e0ecc4`, `dbce98c2`, `ae3b0eb0`, `3e3ba126` and `202266c5`. These have mixed exact-SHA results; do not infer current-head PASS from older source or checker-only commits.
- HTTPD security workflow offset correction is in `c9005f49bb7edf6ac9efad6bfa51b82f6b902cc4`. The original checker run #44945277626 failed; the later exact-head HTTPD run #44945662341 on source descendant `f56fe999...` completed/success, validating the correction. Do not confuse this with the cancelled intermediate run #44945615000.

- MAC-filter hardening complete: supported WebUI modes are enforced, indexed MAC/date/time values use checked NVRAM reads, address tokens are validated, and absent schedule keys preserve legacy defaults. Exact source commit `8dd8a5bcb515b752dc9c4fd07e11440e929d5c49` passed HTTPD, NVRAM, Firewall Startup, WAN DHCP, WAN route, FULLCONENAT, inventory and AI docs; exact XY-C1 #44955441811 passed Build/TRX/artifact (L3 only for that SHA).
- Static-route hardening complete through source commit `182cfee52473b9f072816c6e5f67ef727945143e`: fixed WebUI 64-row Add off-by-one, checked NVRAM enable/count/entry reads, fail-closed handling, strict metric and gateway validation, preserved the historical invalid-gateway fallback, and retained compatibility up to the generic 128-row storage limit. Maximum serialized snapshot is 7,552 bytes < 8,192-byte buffer. Source commits `1ace1932a59c91e5e313ec53fb1a8d2560d90717`, `06e243f66c34949cf77603479e6135c925422f2c`, `182cfee52473b9f072816c6e5f67ef727945143e`.
- Static-route Delete checker now matches the escaped quotes emitted in the JavaScript string (`37d0b7b96ee8dcd0b654c4e2e514d36fb4e6fdb7`); storage IPTV dispatch check ignores formatting-only whitespace (`f3a1cdda6e36c3014809f276c31c84a119016158`). WAN-route result-propagation contract was corrected in `923f8c64dc7598f92ce454a0fc2f71beb8f5a227`; exact run #44956642473 passed.

## Active task

- Active product slice: libdisk share-list persistence. `get_all_folder_in_mount_path()` distinguishes clean EOF from read failure and frees partial lists on errors. `initial_folder_list_in_mount_path()` no longer truncates the published file directly; it writes a temporary snapshot, checks every buffered write/flush, fsyncs and closes it, then atomically renames it over the destination.
- Regression coverage: readdir entry/EOF/EIO/invalid adapter; successful snapshot content; invalid data; /dev/full write failure. The source contract checks both failure-state transitions and the XY-C1 production-input path filter. Exact libdisk run #45015894653 PASS.
- The next known storage-share defect to address after current CI: `initial_one_var_file_in_mount_path()` ignores `fprintf()`/close failures, and `modify_if_exist_new_folder()` ignores append/close failures. Keep this follow-up separate from the checked folder-list transaction.
- No local full router checkout is available; same-SHA XY-C1 Build/TRX/artifact is required for firmware L3.

## Verification boundary

- Exact libdisk share-list run #45015894653 PASS on `922841600a62aa8c4c98922da9002c6e4cdfb328` (GNU89 sanitizer binary + production source/path contracts).
- Exact XY-C1 #45015894601 IN_PROGRESS in Build XY-C1 on the same SHA; TRX and artifact upload not yet complete.
- Process inventory #45015894636, HTTPD security #45015894612 and NVRAM audit #45015894614 IN_PROGRESS at last poll.
- Prior checker-format follow-up #45015600430 and its AI docs #45015600415 PASS on ancestor `75f461d`.
- L3 NOT VERIFIED until same-SHA Build/TRX/artifact all PASS. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect XY-C1 #45015894601 on product SHA `922841600a62aa8c4c98922da9002c6e4cdfb328`: Build, TRX verification and artifact upload must all pass on that same SHA.
2. Confirm Process inventory #45015894636, HTTPD security #45015894612 and NVRAM audit #45015894614 exact-SHA outcomes.
3. Continue the known next storage-share failure slice: propagate write/close failures in `initial_one_var_file_in_mount_path()` and `modify_if_exist_new_folder()`, with regression tests; do not combine it with unrelated changes.
4. Keep the backup immutable and hardware L4 marked NOT VERIFIED.

