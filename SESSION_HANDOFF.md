# 2026-11-19 continuation checkpoint — complete switch CLI argument bounds

## Current repository state

- Repository `Arlene206/padavan-4.4-2`, development branch `main`.
- Prior source slices: WAN checker return-order correction `bef7889d16ca08912d126c3cf97c2fb8c1d04f2c`; initial switch invalid-ACL/usage fix `4f2ae0b46f16b60230888d6422fc9c3c505b90ef`. Fetch live main and backup before changes.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. PR #1 remains closed/unmerged.

- Latest main at this checkpoint: `7edb5d58b4ba61e1c68deab9aa1601f90c5339d0`. LAN DHCP restoration follow-up `7edb5d58b4ba61e1c68deab9aa1601f90c5339d0` adds a one-way best-effort restore routine. This avoids a failed compensating write being “rolled back” by reapplying the uncommitted candidate lease. New tests cover restore continuing after a failed key and ioctl/close errors reported after a write.
- HTTPD run `44447312302` on parent `11813f0` failed after the DHCP policy binary printed PASS; the embedded HTTPD check still asserted the old single-key temporary NVRAM write. The new contract now checks seven-key transactional publication and complete rollback coverage. Re-run on current main.
- WAN callback run `44447008179` and AI docs run `44447008247` both PASS on `d1d8473`.
- Exact-head on `bef7889`: USB modem `44446007111` and AI docs `44446007055` passed; Process `44446007056` and Firewall `44446007049` were still in progress at last poll. No XY-C1 run was listed for that docs/checker-only SHA.
- WAN production changes retain first chronological failure across USB NDIS/RAS and callback-link/VLAN/APCLI-WISP preflight failure paths.

## Latest switch CLI robustness slice

- Main parent for this slice: 19b125963c38efd5c2dda0e24fb574b40a07bf83. The main and frozen backup refs were checked before preparing the atomic tree.
- Source review found four command helpers that relied on top-level dispatch but lacked local argument-count guards: dip_add (argc >= 5), dip_del (argc >= 4), sip_add (argc >= 6) and sip_del (argc >= 5). Truncated invocations could pass a missing argv entry into the parser.
- The add helpers also wrote switch registers before validating port-map input, leaving partial switch-table state for malformed input. This slice adds local guards, a pure eight-character binary port-map parser preserving the existing seven-port register mapping, malformed-input tests, and validates all input before the first MMIO write.
- Exact HTTPD 44448464312 and AI docs 44448464415 passed on parent 5fa04bc. No exact-head Switch CLI run yet covered the SIP/DIP argument paths.
- The latest concurrent source commit 19b1259 addresses wireless hardware NAT registration errors; its exact-SHA XY-C1 Build and NVRAM boundary Actions must be checked before any L3 claim.

- Exact Switch CLI run 44450415160 on f3d4e79 failed in the initial static contract because function_body('dip_add(...)') matched the earlier acl_dip_add symbol. The production functions already contain their local count guards; the checker now uses fully qualified void dip_add/dip_del/sip_add/sip_del signatures. The ASan/UBSan parser test step was skipped until this contract passes. AI docs 44450415070 and Process inventory 44450414996 passed; NVRAM boundary 44450415009 passed. XY-C1 run 44450415016 was still pending at last poll.
## Active next task

Fetch live main, inspect the Switch CLI and AI-doc exact-SHA Actions after the fully qualified helper signature correction, confirm XY-C1 Build/TRX/artifact outcome, then continue a P1 defect outside the completed WAN/LAN DHCP and switch CLI input-preflight slices.
## Active task

The initial switch fix rejects truncated ACL commands and returns `EXIT_FAILURE` for invalid usage. Further argument review found three more missing-arity paths:
- `set_mirror_to()` dereferenced `argv[3]` without requiring argc >= 4.
- `set_mirror_from()` dereferenced `argv[3]` and `argv[4]` without requiring argc >= 5.
- `acl_dip_trtcm()` only checked argc >= 6, but reads `argv[6]` through `argv[9]`; it now rejects argc < 10 before parsing any argument.
Each guard uses the common failing `usage(argv[0])`. Extend the existing Switch CLI workflow to assert all three guards precede the first argument read. No new workflow or path-filter expansion.

## Verification boundary

- L1: argument indexing and required parameter count verified from the production functions.
- L2: the initial ACL+invalid-usage Switch CLI run `44445815633` passed on the prior source SHA `4f2ae0b`; the expanded mirror/TrTCM contract needs its own exact-head rerun.
- WAN checker correction `bef7889`: USB modem `44446007111` PASS and AI docs `44446007055` PASS; Process and Firewall were pending at last poll.
- L3: XY-C1 Build/TRX/artifact not yet verified on the expanded switch SHA.
- L4: real switch hardware, PHY and MDIO behavior NOT VERIFIED. No local C test or MIPS build performed in this connector-only session.

## Exact first action for the next GPT

1. Fetch live main and immutable backup; inspect exact-head HTTPD, WAN DHCP, NVRAM transaction, XY-C1, Switch CLI and AI docs Actions. The current HTTPD exact-head check is specifically responsible for validating the new seven-key lease rollback contract.
2. Fix any specific failed check from its job log. The switch source contract must validate ACL dispatch, mirror-to, mirror-from and TrTCM arity before dereference.
3. Verify XY-C1 Build, TRX and artifact upload on one exact SHA before declaring L3.
4. Continue independent P1 stability/security work outside the completed switch argument paths and WAN first-error slice.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Create all blobs and one complete tree, then one commit with exact main parent; update main only with non-forced expected-SHA compare-and-swap. Continue across chats without waiting on unrelated pending runs. Backup is immutable.

## Before context exhaustion

Record exact source SHA, regression and CI evidence, and unverified build/hardware boundaries. Do not claim any test, build, TRX, artifact or hardware validation that did not pass.
