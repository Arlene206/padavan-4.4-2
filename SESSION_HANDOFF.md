## Current repository state

- Parent main SHA: 8e3c3ba6702c2fe7a1a28ca3016a22611aad0b7a. Re-fetch live main before every write because concurrent sessions may advance it.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC work is integrated on main with later evolution. Do not port the stale PR branch wholesale.
- Main-only work: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### DHCP client ARP preflight failure handling

- The DHCPACK address-conflict check was unsafe: arpping() returned 1 (“no conflict”) after setsockopt, sendto, poll or read failures. dhcpc.c checked only !arpping(), so I/O failures could be treated as proof that an offered address was free.
- This change makes probe errors return -1, keeps 0 for a confirmed conflict and 1 for a completed no-conflict probe. DHCPACK processing fails closed on a negative result, does not run bound/renew or commit the offered lease, preserves the current lease deadline, accounts rounded-up probe runtime and retries DHCP.
- Added policy unit coverage and source-check assertions; the workflow also watches arpping.c.

## Verification boundary

- Parent 8e3c3ba6702c2fe7a1a28ca3016a22611aad0b7a includes the libdisk C89 declaration fix for disk_share_account_metadata_status_at() and disk_share_remove_folder_path(). Exact parent XY-C1 #45463559354, libdisk #45463559390, AI handoff #45463559414, HTTPD #45463559367, NVRAM #45463559339 and Process #45463559518 were IN_PROGRESS at last poll.
- Earlier product SHA 572ec294eeae01befe36684ed7ab3102b9430e21 failed XY-C1 MIPS compilation because the two libdisk helpers lacked declarations; the follow-up prototypes are in 8e3c3ba6702c2fe7a1a28ca3016a22611aad0b7a and require that exact build's result.
- This DHCP slice requires exact-head udhcpc host sanitizer/source validation and XY-C1 cross-build. No local MIPS build is claimed. L3 remains NOT VERIFIED until Build, TRX and artifact upload PASS on the same production SHA; physical L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head udhcpc GNU89 ASan/UBSan policy test and Python source contract. Confirm negative ARP probe errors cannot fall through to a DHCPACK lease commit.
2. Check exact-head XY-C1 Build, TRX verification and artifact upload for the libdisk C89 prototype fix; inspect the precise compiler output if it still fails.
3. Continue P1 removable-storage mount/symlink/stale-inode audits, then other userland stability/security fixes. PENDING checks do not block independent work.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, tests, Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test, commit and verify. Keep one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware/hardware validation. Complete this checkpoint Before context exhaustion.
