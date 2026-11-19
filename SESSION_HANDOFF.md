# 2026-11-19 continuation checkpoint — complete switch CLI argument bounds

## Current repository state

- Repository `Arlene206/padavan-4.4-2`, development branch `main`.
- Prior source slices: WAN checker return-order correction `bef7889d16ca08912d126c3cf97c2fb8c1d04f2c`; initial switch invalid-ACL/usage fix `4f2ae0b46f16b60230888d6422fc9c3c505b90ef`. Fetch live main and backup before changes.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. PR #1 remains closed/unmerged.
- Exact-head on `bef7889`: USB modem `44446007111` and AI docs `44446007055` passed; Process `44446007056` and Firewall `44446007049` were still in progress at last poll. No XY-C1 run was listed for that docs/checker-only SHA.
- WAN production changes retain first chronological failure across USB NDIS/RAS and callback-link/VLAN/APCLI-WISP preflight failure paths.

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

1. Fetch live main and immutable backup; inspect exact-head Switch CLI, XY-C1, WAN USB/Firewall/Process and AI docs Actions.
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
