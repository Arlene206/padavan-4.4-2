#!/usr/bin/env python3
"""Guard the crash-consistency ordering of the Samba passdb/config journal."""

from pathlib import Path

HEADER = Path("trunk/user/rc/src/samba_config_transaction.h").read_text()
SYNC_HEADER = Path("trunk/user/rc/src/config_file_sync.h").read_text()
assert '#include "config_file_sync.h"' in HEADER
assert '#include "config_file_sync.h"' in HEADER
assert "#define CONFIG_FILE_SYNC_MAX_EINTR_RETRIES 8U" in SYNC_HEADER
sync_helper = SYNC_HEADER[SYNC_HEADER.index("config_file_sync_fd(int fd)"):]
assert "for (;;)" in sync_helper
assert "status = CONFIG_FILE_SYNC_FSYNC(fd);" in sync_helper
assert "status == 0 || errno != EINTR" in sync_helper
assert "interruptions >= CONFIG_FILE_SYNC_MAX_EINTR_RETRIES" in sync_helper
assert "++interruptions;" in sync_helper



def body(signature: str) -> str:
    start = HEADER.index(signature)
    opening = HEADER.index("{", start)
    depth = 0
    for offset in range(opening, len(HEADER)):
        char = HEADER[offset]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return HEADER[opening + 1 : offset]
    raise AssertionError(f"unterminated function: {signature}")


def compact(source: str) -> str:
    return "".join(source.split())


sync_directory = compact(body("samba_config_transaction_sync_directory("))
assert "open(directory,flags)" in sync_directory
assert "O_DIRECTORY|O_NOFOLLOW" in sync_directory
assert sync_directory.index("config_file_sync_fd(fd)") < sync_directory.index("close(fd)")
sync_parent = compact(body("samba_config_transaction_sync_parent("))
assert "strrchr(parent,'/')" in sync_parent
assert "samba_config_transaction_sync_directory(parent)" in sync_parent

prepare = compact(body("samba_config_transaction_prepare_parent_directory("))
assert prepare.index("fchmod(fd,0700)") < prepare.index("config_file_sync_fd(fd)")
assert "created&&samba_config_transaction_sync_parent(directory)" in prepare

write_state = compact(body("samba_config_transaction_write_state("))
assert write_state.index("config_file_sync_fd(fd)") < write_state.index("rename(next_path,state_path)")
assert write_state.index("rename(next_path,state_path)") < write_state.index(
    "samba_config_transaction_sync_directory(directory)"
)

begin = compact(body("samba_config_transaction_begin("))
assert begin.index("mkdir(directory,0700)") < begin.index(
    "samba_config_transaction_sync_parent(directory)"
)
assert begin.index("samba_config_transaction_sync_parent(directory)") < begin.index(
    "samba_config_transaction_snapshot(passdb_path"
)
assert begin.index("samba_config_transaction_snapshot(config_path") < begin.index(
    'samba_config_transaction_write_state(directory,passdb_had_original,config_had_original,"PREPARED")'
)

restore = compact(body("samba_config_transaction_restore_file("))
assert restore.index("unlink(target_path)") < restore.index(
    "samba_config_transaction_sync_parent(target_path)"
)
assert restore.index("unlink(target_path)") < restore.index(
    "samba_config_transaction_sync_parent(target_path)"
)
assert restore.index("rename(restore_path,target_path)") < restore.rindex(
    "samba_config_transaction_sync_parent(target_path)"
)

commit = compact(body("samba_config_transaction_commit("))
assert commit.index("samba_config_transaction_sync_parent(passdb_path)") < commit.index(
    "samba_config_transaction_sync_parent(config_path)"
)
assert commit.index("samba_config_transaction_sync_parent(config_path)") < commit.index(
    '"COMMITTED"'
)

cleanup = compact(body("samba_config_transaction_cleanup("))
marker_unlink = cleanup.index("unlink(path)")
marker_sync = cleanup.index("samba_config_transaction_sync_directory(directory)")
snapshot_loop = cleanup.index("for(i=0;snapshots[i]!=NULL;++i)")
rmdir = cleanup.index("rmdir(directory)")
assert marker_unlink < marker_sync < snapshot_loop < rmdir
assert cleanup.rindex("samba_config_transaction_sync_directory(directory)") < rmdir
assert rmdir < cleanup.index("samba_config_transaction_sync_parent(directory)")

passdb_sync = compact(body("samba_config_transaction_sync_file("))
assert "flags=O_RDWR" in passdb_sync
assert "flags|=O_NOFOLLOW" in passdb_sync
assert "open(path,flags)" in passdb_sync
assert "fstat(fd,&st)" in passdb_sync
assert "S_ISREG(st.st_mode)" in passdb_sync
assert "st.st_uid!=geteuid()" in passdb_sync
assert "(st.st_mode&07777)!=0600" in passdb_sync
assert passdb_sync.index("config_file_sync_fd(fd)") < passdb_sync.index("close(fd)")

print("Samba paired-transaction directory durability contract: PASS")

from pathlib import Path

SERVICES = Path("trunk/user/rc/src/services_stor.c").read_text()


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1 : offset]
    raise AssertionError(f"unterminated function: {signature}")


run_samba = compact(function_body(SERVICES, "int run_samba(void)"))
state_read = run_samba.index("samba_config_transaction_read_state_with_accounts(")
abort_call = run_samba.index("samba_config_transaction_abort_with_accounts(")
assert state_read < abort_call
assert "state_result==1&&is_committed" in run_samba
assert "elseif(state_result==1)" in run_samba
assert "prior transaction outcome is unavailable" in SERVICES
assert "publication remains committed; recovery journal cleanup completed" in SERVICES
assert "publication rolled back" in SERVICES
assert "paired publication rollback failed" not in SERVICES
transaction_begin = run_samba.index("samba_config_transaction_begin_with_accounts(")
passwd_publish = run_samba.index("recreate_passwd_unix_snapshot(")
credential_failure = run_samba.index("if(credential_failed)")
passdb_sync_call = run_samba.index("samba_config_transaction_sync_file(staged_passdb)")
config_publish = run_samba.index("write_smb_conf_unlocked(router_username,account_list_snapshot,sh_num)!=0")
commit_call = run_samba.index("samba_config_transaction_commit_with_accounts(")
assert transaction_begin < passwd_publish < credential_failure < passdb_sync_call < config_publish < commit_call
assert "transaction_active=1" in run_samba
assert "samba_config_transaction_recover_with_accounts(" in run_samba
assert "stagedpassdbcouldnotbedurablysynchronized" in run_samba


print("Samba committed-versus-rolled-back recovery reporting: PASS")


# V2 journal covers all four mutually dependent account/configuration files.
v2_write = compact(body("samba_config_transaction_write_state_with_accounts("))
v2_read = compact(body("samba_config_transaction_read_state_with_accounts("))
v2_recover = compact(body("samba_config_transaction_recover_with_accounts("))
v2_begin = compact(body("samba_config_transaction_begin_with_accounts("))
v2_commit = compact(body("samba_config_transaction_commit_with_accounts("))
assert '"PADAVAN_SAMBA_CONFIG_TXN_V2' in v2_write
assert "PASSWD=%d" in v2_write and "GROUP=%d" in v2_write
assert '"PADAVAN_SAMBA_CONFIG_TXN_V1' in v2_read
assert '"PADAVAN_SAMBA_CONFIG_TXN_V2' in v2_read
assert '*tracks_accounts = 0' in v2_read and '*tracks_accounts = 1' in v2_read
assert v2_begin.index('samba_config_transaction_snapshot(passdb_path') < v2_begin.index('samba_config_transaction_snapshot(config_path')
assert v2_begin.index('samba_config_transaction_snapshot(config_path') < v2_begin.index('samba_config_transaction_snapshot(passwd_path')
assert v2_begin.index('samba_config_transaction_snapshot(passwd_path') < v2_begin.index('samba_config_transaction_snapshot(group_path')
assert v2_begin.index('samba_config_transaction_snapshot(group_path') < v2_begin.index('"PREPARED"')
assert v2_recover.index('config_path, config_snapshot') < v2_recover.index('passdb_path, passdb_snapshot')
assert v2_recover.index('passdb_path, passdb_snapshot') < v2_recover.index('group_path, group_snapshot')
assert v2_recover.index('group_path, group_snapshot') < v2_recover.index('passwd_path, passwd_snapshot')
assert 'samba_config_transaction_recover(directory, passdb_path, config_path)' in v2_recover
assert 'samba_config_transaction_sync_regular_file(config_path)' in v2_commit
assert 'samba_config_transaction_sync_regular_file(passwd_path)' in v2_commit
assert 'samba_config_transaction_sync_regular_file(group_path)' in v2_commit
assert v2_commit.index('samba_config_transaction_sync_regular_file(group_path)') < v2_commit.index('"COMMITTED"')
assert 'samba_config_transaction_sync_parent(passwd_path)' in v2_commit
assert 'samba_config_transaction_sync_parent(group_path)' in v2_commit
assert 'samba_config_transaction_cleanup(directory)' in v2_commit
