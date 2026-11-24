#!/usr/bin/env python3
"""Guard libdisk share-list output initialization and Samba error cleanup."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
LIBDISK = ROOT / "trunk/user/libdisk/src"
DISK_SOURCE = (LIBDISK / "disk_share.c").read_text()
DISK_IO_SOURCE = (LIBDISK / "disk_io_tools.c").read_text()
DISK_IO_HEADER = (LIBDISK / "disk_io_tools.h").read_text()
POLICY_TEST = (LIBDISK / "tests/disk_share_list_policy.c").read_text()
IO_REGRESSION = (LIBDISK / "tests/disk_io_tools_read_test.c").read_text()
POLICY = (LIBDISK / "disk_share_list_policy.h").read_text()
ACCOUNT_POLICY = (LIBDISK / "disk_share_account_nvram_policy.h").read_text()
DISK_INITIAL_HEADER = (LIBDISK / "disk_initial.h").read_text()
ACCOUNT_NAME_POLICY = (ROOT / "trunk/user/shared/src/include/router_account_policy.h").read_text()
REGRESSION = (LIBDISK / "tests/disk_share_list_policy.c").read_text()
SAMBA_SOURCE = (ROOT / "trunk/user/rc/src/services_stor.c").read_text()


def function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and next_char == "/":
                state = "line_comment"
                index += 2
                continue
            if char in ("'", '"'):
                state = "quoted"
                quote = char
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        elif state == "quoted":
            if char == "\\":
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        elif state == "line_comment" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("libdisk share-list contract: FAIL: " + message)


require('#include "disk_share_list_policy.h"' in DISK_SOURCE,
        "disk_share.c must include the output-preparation policy")
require("disk_share_list_prepare_outputs" in POLICY,
        "shared output-preparation policy is missing")
require("disk_share_folder_entry_status" in POLICY,
        "real-directory-only storage share entry policy is missing")

folder_list_reader = function_body(
    DISK_SOURCE,
    "int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)"
)
folder_list_reader_compact = "".join(folder_list_reader.split())
cache_read = folder_list_reader_compact.index(
    "list_info=read_regular_file_nofollow(list_file)"
)
cache_missing = folder_list_reader_compact.index(
    "if(!list_info&&errno==ENOENT)", cache_read
)
cache_refresh = folder_list_reader_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", cache_missing
)
cache_retry = folder_list_reader_compact.index(
    "list_info=read_regular_file_nofollow(list_file)", cache_refresh
)
cache_cleanup = folder_list_reader_compact.index("free(list_file);", cache_retry)
cache_null = folder_list_reader_compact.index("if(!list_info)", cache_cleanup)
require(cache_read < cache_missing < cache_refresh < cache_retry <
        cache_cleanup < cache_null and
        "test_if_file(list_file)" not in folder_list_reader_compact and
        "read_whole_file(list_file)" not in folder_list_reader_compact,
        "removable folder-list cache must use a nonblocking no-follow regular-file read and regenerate only on ENOENT")

require("disk_io_open_parent_nofollow(mount_path,&mount_leaf)" in folder_list_reader_compact and
        "openat(mount_parent_fd,mount_leaf,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC)" in folder_list_reader_compact and
        "fstatat(mount_directory_fd,tmp_folder_list[i],&entry_status,AT_SYMLINK_NOFOLLOW)" in folder_list_reader_compact and
        "lstat(folder_path,&entry_status)" not in folder_list_reader_compact and
        "disk_share_folder_entry_status" in folder_list_reader_compact and
        "valid_count" in folder_list_reader_compact,
        "cached folder names must be revalidated against real non-symlink directories before sharing")

for signature in (
    "int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)",
    "int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)",
):
    body = function_body(DISK_SOURCE, signature)
    prepare = body.index("disk_share_list_prepare_outputs(mount_path, sh_num, folder_list)")
    use = body.index("disk_io_open_parent_nofollow(mount_path, &mount_leaf)")
    require(prepare < use,
            signature + " must reset outputs before using the mount path")


get_permission_body = function_body(
    DISK_SOURCE, "int get_permission("
)
set_permission_body = function_body(
    DISK_SOURCE, "int set_permission("
)

permission_lookup = function_body(
    POLICY,
    "disk_share_permission_find_unique_record(const char *source, const char *target,"
)
require("errno = EEXIST;" in permission_lookup and
        "source[value_offset + 3] != '\\n'" in permission_lookup and
        "source[value_offset] < '0'" in permission_lookup,
        "permission lookup must reject duplicate case-insensitive keys and malformed rights")
require("disk_share_permission_find_unique_record" in get_permission_body and
        "upper_strstr(var_info, target)" not in get_permission_body and
        "disk_share_permission_find_unique_record" in set_permission_body and
        "upper_strstr(var_info, target)" not in set_permission_body,
        "permission reads and writes must refuse ambiguous duplicate ACL records")
for marker in (
    "test_permission_lookup_finds_one_case_insensitive_record",
    "test_permission_lookup_reports_missing_record",
    "test_permission_lookup_rejects_duplicate_case_insensitive_records",
    "test_permission_lookup_rejects_malformed_rights",
):
    require(marker + "();" in POLICY_TEST,
            marker + " needs a policy regression test")

permission_rewrite = function_body(
    POLICY,
    "disk_share_permission_rewrite(const char *source, const char *target,"
)
require("if (match)" in permission_rewrite and "errno = EEXIST;" in permission_rewrite,
        "permission rewrite must reject duplicate case-insensitive ACL keys instead of leaving stale duplicate records")

require("disk_share_folder_name_is_visible" in POLICY,
        "folder visibility policy must reject hidden and system directories")
cached_folders = function_body(
    DISK_SOURCE,
    "int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)"
)
fresh_folders = function_body(
    DISK_SOURCE,
    "int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)"
)
require("disk_share_folder_name_is_visible" in cached_folders and
        "test_if_System_folder" in cached_folders and
        "disk_share_folder_name_is_visible" in fresh_folders and
        "test_if_System_folder" in fresh_folders,
        "cached share names must not bypass hidden/system-directory exclusions")
require("test_folder_scans_use_pinned_directory_descriptors();" in POLICY_TEST and
        "test_folder_name_visibility_policy();" in POLICY_TEST,
        "hidden/system directory cache bypass needs a policy regression test")

require("disk_share_list_next_entry" in POLICY,
        "shared readdir status policy is missing")
all_folders = function_body(DISK_SOURCE,
                            "int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)")
compact_all_folders = " ".join(all_folders.split())
read_entry_match = re.search(
    r"disk_share_list_next_entry\(\s*pool_to_open,\s*readdir,\s*&dp,\s*&read_error\s*\)",
    compact_all_folders,
)
require(read_entry_match is not None,
        "directory loop must use the checked readdir adapter")
read_entry = read_entry_match.start()
eof_gate = compact_all_folders.index("if (read_status == 0)", read_entry)
error_gate = compact_all_folders.index("if (read_status < 0)", eof_gate)
errno_restore = compact_all_folders.index(
    "errno = read_error != 0 ? read_error : EIO;", error_gate
)
require(read_entry < eof_gate < error_gate < errno_restore and
        "disk_io_open_parent_nofollow(mount_path, &mount_leaf)" in compact_all_folders and
        "openat(mount_parent_fd, mount_leaf, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC)" in compact_all_folders and
        "fdopendir(mount_fd)" in compact_all_folders and
        "fstatat(directory_fd, dp->d_name, &entry_status, AT_SYMLINK_NOFOLLOW)" in compact_all_folders and
        "lstat(testdir, &entry_status)" not in compact_all_folders and
        "disk_share_folder_entry_status" in compact_all_folders and
        "test_if_dir(testdir)" not in compact_all_folders,
        "directory enumeration must reject symlinked/non-directory entries without following them")

require(read_entry < eof_gate < error_gate < errno_restore,
        "directory iteration must distinguish clean EOF from readdir errors")
failure_cleanup = compact_all_folders[compact_all_folders.index("failed:"):]
require("saved_errno = errno ? errno : EIO;" in failure_cleanup and
        "errno = saved_errno;" in failure_cleanup,
        "directory read/close failures must preserve errno during cleanup")


require("disk_share_list_write_snapshot" in POLICY and
        "if (fflush(stream) != 0)" in POLICY,
        "snapshot writer must report buffered stream failures")
initial_writer = function_body(
    DISK_SOURCE, "int initial_folder_list_in_mount_path(const char *const mount_path)"
)
initial_writer_compact = "".join(initial_writer.split())
require("disk_share_open_mount_directory(mount_path)" in initial_writer_compact and
        "initial_folder_list_in_directory_fd(directory_fd)" in initial_writer_compact and
        "close(directory_fd)" in initial_writer_compact and
        "disk_share_write_stream_atomic(" not in initial_writer_compact,
        "folder-list snapshots must open the mount once and keep that directory descriptor through atomic publication")


metadata_begin_body = function_body(
    DISK_SOURCE, "static int\ndisk_share_account_metadata_write("
)
metadata_begin_compact = "".join(metadata_begin_body.split())
require("get_account_list(&acc_num,&account_list)" in metadata_begin_compact and
        "disk_share_account_nvram_begin(&transaction)" in metadata_begin_compact and
        "disk_share_account_inventory_matches_locked(" in metadata_begin_compact and
        "writer(account,mount_path)" in metadata_begin_compact and
        "disk_share_account_nvram_end(&transaction)" in metadata_begin_compact,
        "public account metadata writers must validate a live account snapshot and own the writer lease around file mutation")
for public_writer in (
    "int initial_one_var_file_in_mount_path(const char *const account,",
    "int modify_if_exist_new_folder(const char *const account,",
):
    public_body = function_body(DISK_SOURCE, public_writer)
    require("disk_share_account_metadata_write(" in public_body and
            "disk_share_account_nvram_begin(" not in public_body,
            "public metadata APIs must delegate to the single transaction-owning wrapper")
for locked_writer in (
    "static int initial_one_var_file_in_mount_path_locked(",
    "static int modify_if_exist_new_folder_locked(",
):
    locked_body = function_body(DISK_SOURCE, locked_writer)
    require("disk_share_account_nvram_begin(" not in locked_body and
            "disk_share_account_nvram_end(" not in locked_body,
            "lock-owned metadata workers must not acquire a nested NVRAM transaction")


pinned_scanner = function_body(
    DISK_SOURCE, "get_all_folder_in_directory_fd(int directory_fd,"
)
pinned_scanner_compact = "".join(pinned_scanner.split())
require('openat(directory_fd,".",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC)' in
        pinned_scanner_compact and
        "fdopendir(scan_fd)" in pinned_scanner_compact and
        "fstatat(directory_fd,entry->d_name,&entry_status,AT_SYMLINK_NOFOLLOW)" in
        pinned_scanner_compact and
        "disk_share_list_next_entry" in pinned_scanner_compact and
        "opendir(mount_path)" not in pinned_scanner_compact and "lstat(" not in pinned_scanner_compact,
        "internal storage scanner must enumerate and validate entries through one pinned directory descriptor")

pinned_cache = function_body(
    DISK_SOURCE, "get_folder_list_in_directory_fd(int directory_fd,"
)
pinned_cache_compact = "".join(pinned_cache.split())
cache_read_at = pinned_cache_compact.index(
    'read_regular_file_nofollow_at(directory_fd,".__folder_list.txt")'
)
cache_regen = pinned_cache_compact.index(
    "initial_folder_list_in_directory_fd(directory_fd)", cache_read_at
)
cache_fstatat = pinned_cache_compact.index(
    "fstatat(directory_fd,result_list[i],&entry_status,AT_SYMLINK_NOFOLLOW)"
)
require(cache_read_at < cache_regen < cache_fstatat and
        "disk_share_folder_entry_status" in pinned_cache_compact and
        "lstat(" not in pinned_cache_compact,
        "pinned cache parser must read, regenerate and revalidate entries within the original volume")

for signature in (
    "int initial_folder_list_in_mount_path(const char *const mount_path)",
):
    body = function_body(DISK_SOURCE, signature)
    compact = "".join(body.split())
    require("disk_share_open_mount_directory(mount_path)" in compact and
            "initial_folder_list_in_directory_fd(directory_fd)" in compact and
            "close(directory_fd)" in compact,
            "folder snapshot API must keep its mount directory open through generation and publication")
initial_snapshot = function_body(
    DISK_SOURCE, "initial_folder_list_in_directory_fd(int directory_fd)"
)
initial_snapshot_compact = "".join(initial_snapshot.split())
require("get_all_folder_in_directory_fd(directory_fd,&sh_num,&folder_list)" in
        initial_snapshot_compact and
        'disk_share_write_stream_atomic_at(directory_fd,".__folder_list.txt",disk_share_write_snapshot_stream,&writer_context,0)' in
        initial_snapshot_compact,
        "folder cache snapshot must enumerate and publish on one pinned directory descriptor")

require("disk_share_write_account_defaults" in POLICY and
        "if (fflush(stream) != 0)" in POLICY,
        "account defaults writer must report buffered stream failures")
account_writer = function_body(
    DISK_SOURCE,
    "static int initial_one_var_file_in_mount_path_locked(const char *const account,"
)
account_writer_compact = "".join(account_writer.split())
require("disk_share_open_mount_directory(mount_path)" in account_writer_compact and
        "get_all_folder_in_directory_fd(directory_fd,&sh_num,&folder_list)" in account_writer_compact and
        "disk_share_write_stream_atomic_at(directory_fd,var_leaf,disk_share_write_defaults_stream,&writer_context,0)" in account_writer_compact and
        "mkstemp(" not in account_writer_compact and "rename(" not in account_writer_compact,
        "new account ACL defaults must enumerate and publish through the same pinned mount descriptor")


atomic_replace = function_body(
    DISK_SOURCE, "disk_share_replace_file_atomic(const char *path,"
)
atomic_replace_compact = "".join(atomic_replace.split())
atomic_replace_at = function_body(
    POLICY, "disk_share_replace_file_atomic_at(int directory_fd,"
)
atomic_replace_at_compact = "".join(atomic_replace_at.split())
atomic_stream = function_body(
    POLICY, "disk_share_write_stream_atomic_at(int directory_fd,"
)
atomic_stream_compact = "".join(atomic_stream.split())

read_at_body = function_body(
    DISK_IO_SOURCE, "read_regular_file_nofollow_at(int parent_fd, const char *leaf)"
)
read_at_compact = "".join(read_at_body.split())
require("openat(parent_fd,leaf,O_RDONLY|O_NONBLOCK|O_NOFOLLOW|O_CLOEXEC)" in read_at_compact and
        "fstat(fd,&file_status)" in read_at_compact and
        "!S_ISREG(file_status.st_mode)" in read_at_compact and
        "test_read_regular_file_nofollow_at_stays_with_pinned_directory();" in IO_REGRESSION and
        "externchar*read_regular_file_nofollow_at(int,constchar*);" in
        "".join(DISK_IO_HEADER.split()),
        "descriptor-relative metadata reader must reject links/non-regular files and pass its mount-swap regression")

parent_opener = function_body(
    DISK_IO_SOURCE, "disk_io_open_parent_nofollow(const char *target,"
)
parent_opener_compact = "".join(parent_opener.split())
require("externintdisk_io_open_parent_nofollow(constchar*,char**);" in
        "".join(DISK_IO_HEADER.split()) and
        "intdisk_io_open_parent_nofollow(" in "".join(DISK_IO_SOURCE.split()) and
        "openat(parent_fd,component" in parent_opener_compact and
        "O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC" in parent_opener_compact,
        "atomic metadata writers must obtain a no-symlink parent directory descriptor")
require("disk_io_open_parent_nofollow(path,&leaf)" in atomic_replace_compact and
        "disk_share_replace_file_atomic_at(directory_fd,leaf,contents,contents_len)" in atomic_replace_compact and
        "mkstemp(" not in atomic_replace_compact and
        "rename(" not in atomic_replace_compact,
        "metadata publication must remain relative to the pinned parent directory")
require("disk_share_write_stream_atomic_at(directory_fd,leaf,disk_share_write_atomic_buffer,&context,1)" in
        atomic_replace_at_compact and
        "fstatat(directory_fd,leaf,&existing_stat,AT_SYMLINK_NOFOLLOW)" in atomic_stream_compact and
        "openat(directory_fd,temp_leaf,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600)" in atomic_stream_compact and
        "renameat(directory_fd,temp_leaf,directory_fd,leaf)" in atomic_stream_compact and
        "unlinkat(directory_fd,temp_leaf,0)" in atomic_stream_compact and
        "errno=0;if(writer(stream,context)!=0)" in atomic_stream_compact and
        "errno=0;if(writer(stream,context)!=0)" in atomic_stream_compact and
        "fflush(stream)" in atomic_stream_compact and
        "fsync(fileno(stream))" in atomic_stream_compact and
        "fclose(stream)" in atomic_stream_compact and
        "test_atomic_metadata_replace_stays_in_pinned_directory();" in POLICY_TEST and
        "errno = ENOENT;" in POLICY_TEST and
        "fixture->fail_after_write == 2" in POLICY_TEST,
        "atomic ACL metadata replacement must use the checked stream publisher through one pinned dirfd")


modify_writer = function_body(
    DISK_SOURCE,
    "static int modify_if_exist_new_folder_locked(const char *const account,"
)
compact_modify = "".join(modify_writer.split())
modify_open = compact_modify.index("disk_share_open_mount_directory(mount_path)")
modify_scan = compact_modify.index("get_all_folder_in_directory_fd(directory_fd,&sh_num,&folder_list)", modify_open)
modify_cache = compact_modify.index("get_folder_list_in_directory_fd(directory_fd,&cached_count,&cached_folder_list)", modify_scan)
modify_compare = compact_modify.index("upper_strcmp(folder_list[i],cached_folder_list[j])", modify_cache)
modify_read = compact_modify.index("read_regular_file_nofollow_at(directory_fd,var_leaf)", modify_compare)
modify_publish = compact_modify.index("disk_share_write_stream_atomic_at(directory_fd,var_leaf,disk_share_write_update_stream,&writer_context,0)", modify_read)
require(modify_open < modify_scan < modify_cache < modify_compare < modify_read < modify_publish and
        "test_if_exist_folder_in_mount_path" not in compact_modify and
        "read_regular_file_nofollow(" not in compact_modify and
        "disk_share_write_stream_atomic(" not in compact_modify,
        "new-folder ACL update must enumerate, read and publish on one pinned removable-media volume")


stale_pruner = function_body(
    DISK_SOURCE, "remove_stale_share_var_files("
)
stale_pruner_compact = "".join(stale_pruner.split())
prune_read = stale_pruner_compact.index(
    "disk_share_list_next_entry(dir,readdir,&entry,&read_error)"
)
prune_eof = stale_pruner_compact.index("if(read_status==0)", prune_read)
prune_error = stale_pruner_compact.index("if(read_status<0)", prune_eof)
prune_skip_anonymous = stale_pruner_compact.index(
    "strcmp(account,FTP_ANONYMOUS_USER)==0", prune_error
)
prune_active_match = stale_pruner_compact.index(
    "strcmp(account_list[i],account)==0", prune_skip_anonymous
)
prune_directory_fd = stale_pruner_compact.index("directory_fd=disk_share_open_mount_directory(mount_path)")
prune_unlink = stale_pruner_compact.index(
    "disk_share_unlink_stale_metadata_entry(directory_fd,entry->d_name)",
    prune_active_match
)
prune_open_stream = stale_pruner_compact.index(
    'openat(directory_fd,".",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC)',
    prune_directory_fd
)
prune_fdopendir = stale_pruner_compact.index("fdopendir(scan_fd)", prune_open_stream)
require(prune_directory_fd < prune_open_stream < prune_fdopendir < prune_read <
        prune_eof < prune_error < prune_skip_anonymous < prune_active_match <
        prune_unlink and
        "opendir(mount_path)" not in stale_pruner_compact and
        "directory_fd=dirfd(dir)" not in stale_pruner_compact and
        "unlink(path)" not in stale_pruner_compact and
        "if(closedir(dir)!=0)" in stale_pruner_compact and
        "strcmp(entry->d_name+name_len-8,\"_var.txt\")" in stale_pruner_compact and
        "test_stale_metadata_unlink_uses_pinned_directory_fd();" in POLICY_TEST,
        "stale metadata pruning must scan a no-follow pinned mount descriptor and unlink entries relative to that same directory")
initial_all = function_body(
    DISK_SOURCE,
    "int initial_all_var_file_in_mount_path(const char *const mount_path)"
)
require("if (initial_one_var_file_in_mount_path_locked(FTP_ANONYMOUS_USER," in initial_all and
        "if (initial_one_var_file_in_mount_path_locked(account_list[i]," in initial_all and
        "if (initial_folder_list_in_mount_path(mount_path) != 0)" in initial_all and
        "if (remove_stale_share_var_files(mount_path, acc_num, account_list) != 0)" in initial_all,
        "full share metadata initialization must rebuild active ACLs before pruning stale files and propagate every stage failure")
initial_all = function_body(
    DISK_SOURCE, "int initial_all_var_file_in_mount_path(const char *const mount_path)"
)
initial_all_compact = "".join(initial_all.split())
initial_all_inventory = initial_all_compact.index("get_account_list(&acc_num,&account_list)")
initial_all_preflight = initial_all_compact.index("disk_share_format_account_var_path(var_file,sizeof(var_file),mount_path,FTP_ANONYMOUS_USER)", initial_all_inventory)
initial_all_begin = initial_all_compact.index("disk_share_account_nvram_begin(&transaction)", initial_all_preflight)
initial_all_validate = initial_all_compact.index(
    "disk_share_account_inventory_matches_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list)",
    initial_all_begin
)
initial_all_anon = initial_all_compact.index("initial_one_var_file_in_mount_path_locked(FTP_ANONYMOUS_USER,mount_path)", initial_all_validate)
initial_all_accounts = initial_all_compact.index("initial_one_var_file_in_mount_path_locked(account_list[i],mount_path)", initial_all_anon)
initial_all_snapshot = initial_all_compact.index("initial_folder_list_in_mount_path(mount_path)", initial_all_accounts)
initial_all_prune = initial_all_compact.index("remove_stale_share_var_files(mount_path,acc_num,account_list)", initial_all_snapshot)
initial_all_end = initial_all_compact.index("disk_share_account_nvram_end(&transaction)", initial_all_prune)
require(initial_all_inventory < initial_all_preflight < initial_all_begin <
        initial_all_validate < initial_all_anon < initial_all_accounts <
        initial_all_snapshot < initial_all_prune < initial_all_end and
        "clear_share_var_files(" not in initial_all_compact and
        "get_account_list(" not in initial_all_compact[initial_all_begin:] and
        "first_error=errno?errno:EIO;" in initial_all_compact,
        "full metadata rebuild must preflight one account snapshot, rebuild each active file first, and prune only stale files after the folder-list snapshot succeeds")

stale_pruner = function_body(
    DISK_SOURCE, "remove_stale_share_var_files("
)
stale_pruner_compact = "".join(stale_pruner.split())
require("strcmp(account,FTP_ANONYMOUS_USER)==0" in stale_pruner_compact and
        "strcmp(account_list[i],account)==0" in stale_pruner_compact and
        "disk_share_unlink_stale_metadata_entry(directory_fd,entry->d_name)" in stale_pruner_compact and
        'strcmp(entry->d_name,".__folder_list.txt")' not in stale_pruner_compact,
        "metadata cleanup must preserve anonymous and active accounts, and remove only orphaned account files after rebuild")

create_all = function_body(
    DISK_SOURCE, "create_if_no_var_files(const char *const mount_path)"
)
create_all_compact = "".join(create_all.split())
require("get_account_list(&acc_num,&account_list)" in create_all_compact and
        "disk_share_account_nvram_begin(&transaction)" in create_all_compact and
        "disk_share_account_inventory_matches_locked(" in create_all_compact and
        "create_if_no_var_files_with_accounts(mount_path,acc_num,account_list)" in create_all_compact and
        "disk_share_account_nvram_end(&transaction)" in create_all_compact,
        "public incremental metadata initialization must serialize its owned account snapshot through permission-file publication" and
        "\nint\nint\ncreate_if_no_var_files(" not in DISK_SOURCE)
create_with_accounts = function_body(
    DISK_SOURCE, "create_if_no_var_files_with_accounts("
)
create_with_accounts_compact = "".join(create_with_accounts.split())
worker_guard = (
    "if(!mount_path||!*mount_path||acc_num<0||acc_num>MAX_ACCOUNT_NUM||"
    "(acc_num>0&&!account_list))"
)
worker_first_loop = create_with_accounts_compact.index("for(i=0;i<acc_num;++i){")
worker_first_write = create_with_accounts_compact.index(
    "initial_one_var_file_in_mount_path_locked(FTP_ANONYMOUS_USER,mount_path)"
)
require("get_account_list(" not in create_with_accounts and
        "intacc_num=0;" not in create_with_accounts_compact and
        "char**account_list=NULL;" not in create_with_accounts_compact and
        worker_guard in create_with_accounts_compact and
        "if(!account_list[i]||!*account_list[i])" in create_with_accounts_compact and
        worker_first_loop < worker_first_write and
        "path_len>=(int)sizeof(var_file)" in create_with_accounts_compact and
        "result=modify_if_exist_new_folder_locked" in create_with_accounts_compact and
        "result=initial_one_var_file_in_mount_path_locked" in create_with_accounts_compact and
        "if(result!=0)" in create_with_accounts_compact,
        "incremental folder metadata worker must validate its bounded inventory and preflight every account path before the first permission write")




require("#include <errno.h>" in DISK_IO_SOURCE,
        "disk_io_tools.c must include errno definitions")

sanity_body = function_body(DISK_IO_SOURCE, "void sanity_name(char *name)")
require("(unsigned char)name[i] == 0x84" in sanity_body,
        "sanity_name must compare non-ASCII bytes without signed-char truncation")
require("test_sanity_name_replaces_high_bit_reserved_byte();" in IO_REGRESSION,
        "high-bit reserved filename byte needs a regression test")


require("disk_share_format_account_var_path" in POLICY and
        "errno = ENAMETOOLONG;" in POLICY,
        "account permission metadata paths must reject truncation")
get_permission_body = function_body(DISK_SOURCE, "int get_permission(")
set_permission_body = function_body(DISK_SOURCE, "int set_permission(")
set_permission_compact = "".join(set_permission_body.split())
require("lstat(folder_path,&folder_entry_stat)" in set_permission_compact and
        "disk_share_folder_entry_status" in set_permission_compact and
        "test_if_dir(folder_path)" not in set_permission_compact,
        "permission updates must validate the live share directory without following removable-media symlinks")
require("disk_share_format_account_var_path(var_file, sizeof(var_file)" in
        " ".join(get_permission_body.split()) and
        "disk_share_format_account_var_path(var_file, sizeof(var_file)" in
        " ".join(set_permission_body.split()),
        "permission read/write must use the bounded account pathname helper")
require("test_account_var_path_rejects_truncation();" in POLICY_TEST,
        "account pathname overflow needs a regression")

read_failure = function_body(
    DISK_IO_SOURCE, "read_whole_file_failure(FILE *fp, char *buffer, int saved_errno)"
)
require("if (fclose(fp) != 0 && saved_errno == 0)" in read_failure and
        "errno = saved_errno ? saved_errno : EIO;" in read_failure,
        "read_whole_file failure cleanup must check close and preserve the primary errno")
stream_body = function_body(
    DISK_IO_SOURCE, "static char *\nread_whole_file_stream(FILE *fp)"
)
stream_compact = "".join(stream_body.split())
require("READ_WHOLE_FILE_MAX(1024U*1024U)" in stream_compact and
        "read_whole_file_failure(fp,buffer,EFBIG)" in stream_compact and
        "fclose(fp)!=0" in stream_compact and
        "saved_errno=errno?errno:EIO;" in stream_compact,
        "shared stream reader must retain its 1 MiB bound and checked read/close handling")
read_body = function_body(DISK_IO_SOURCE, "char *read_whole_file(const char *target)")
require("errno = EINVAL;" in read_body and
        'fopen(target, "r")' in read_body and
        "read_whole_file_stream(fp)" in read_body,
        "legacy file reader must preserve its path validation and bounded stream behavior")
parent_open = function_body(
    DISK_IO_SOURCE, "disk_io_open_parent_nofollow(const char *target, char **leaf_out)"
)
parent_open_compact = "".join(parent_open.split())
require('open("/",O_RDONLY|O_DIRECTORY|O_CLOEXEC)' in parent_open_compact and
        "openat(parent_fd,component" in parent_open_compact and
        "O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC" in parent_open_compact and
        'strcmp(component,"..")' in parent_open_compact,
        "removable-media metadata paths must pin each absolute parent component without following symlinks")
safe_read = function_body(
    DISK_IO_SOURCE, "char *read_regular_file_nofollow(const char *target)"
)
safe_compact = "".join(safe_read.split())
require("disk_io_open_parent_nofollow(target,&leaf)" in safe_compact and
        "openat(parent_fd,leaf,O_RDONLY|O_NONBLOCK|O_NOFOLLOW|O_CLOEXEC)" in safe_compact and
        "fstat(fd,&file_status)" in safe_compact and
        "!S_ISREG(file_status.st_mode)" in safe_compact and
        'fdopen(fd,"r")' in safe_compact and
        "read_whole_file_stream(fp)" in safe_compact,
        "untrusted cache reads must pin parent directories and reject symlinks/non-regular files without blocking")
status_read = function_body(
    DISK_IO_SOURCE, "int test_if_regular_file_nofollow(const char *target)"
)
status_compact = "".join(status_read.split())
require("disk_io_open_parent_nofollow(target,&leaf)" in status_compact and
        "fstatat(parent_fd,leaf,&file_status,AT_SYMLINK_NOFOLLOW)" in status_compact and
        "S_ISLNK(file_status.st_mode)" in status_compact and
        "S_ISREG(file_status.st_mode)" in status_compact,
        "removable account metadata existence checks must pin ancestors and reject final symlinks/special files")
for marker in (
    "test_reads_complete_text_file",
    "test_reads_empty_file",
    "test_rejects_invalid_and_missing_paths",
    "test_rejects_oversized_file_with_efbig",
    "test_reports_directory_read_error",
    "test_read_regular_file_nofollow_accepts_regular_file",
    "test_read_regular_file_nofollow_rejects_symlink",
    "test_read_regular_file_nofollow_rejects_fifo_without_blocking",
    "test_read_regular_file_nofollow_rejects_directory",
    "test_regular_file_status_nofollow_distinguishes_missing_file",
    "test_regular_file_status_nofollow_rejects_symlink_and_fifo",
    "test_metadata_readers_reject_symlinked_parent_components",
):
    require(marker in IO_REGRESSION and marker + "();" in IO_REGRESSION,
            "missing read_whole_file regression: " + marker)


get_permission = function_body(DISK_SOURCE, "int get_permission(const char *account,")
get_read = get_permission.index("var_info = read_regular_file_nofollow(var_file);")
get_missing = get_permission.index("if (saved_errno == ENOENT)", get_read)
get_denied_error = get_permission.index("return -1;", get_missing)
require(get_read < get_missing < get_denied_error and
        "errno = saved_errno;" in get_permission[get_denied_error - 80:get_denied_error + 40],
        "permission query must preserve no-grant on ENOENT but report metadata I/O errors")
set_permission = function_body(DISK_SOURCE, "int set_permission(const char *account,")
set_permission_compact = "".join(set_permission.split())
set_accounts = set_permission_compact.index("get_account_list(&acc_num,&account_list)")
set_account_lookup = set_permission_compact.index("strcmp(account,FTP_ANONYMOUS_USER)==0", set_accounts)
set_begin = set_permission_compact.index("disk_share_account_nvram_begin(&transaction)", set_account_lookup)
set_validate = set_permission_compact.index(
    "disk_share_account_inventory_matches_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list)",
    set_begin
)
set_live_dir = set_permission_compact.index("lstat(folder_path,&folder_entry_stat)", set_validate)
set_snapshot_lookup = set_permission_compact.index(
    "test_if_exist_folder_in_mount_path(mount_path,folder)", set_live_dir
)
set_snapshot_refresh = set_permission_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", set_snapshot_lookup
)
set_read = set_permission_compact.index("var_info=read_regular_file_nofollow(var_file)", set_snapshot_refresh)
set_write = set_permission_compact.index("disk_share_replace_file_atomic(var_file", set_read)
set_release = set_permission_compact.index("disk_share_account_nvram_end(&transaction)", set_write)
set_reload = set_permission_compact.index('system("/sbin/run_samba")', set_release)
require(set_accounts < set_account_lookup < set_begin < set_validate < set_live_dir <
        set_snapshot_lookup < set_snapshot_refresh < set_read <
        set_write < set_release < set_reload and
        "if(!account_found)" in set_permission_compact and
        "saved_errno=ENOENT;" in set_permission_compact and
        "errno=ENOENT;" in set_permission_compact and
        "get_account_list(" not in set_permission_compact[set_begin:],
        "permission updates must reject deleted accounts and serialize metadata read/modify/write under the shared account writer lease, releasing it before Samba reload")

set_read = set_permission.index("var_info = read_regular_file_nofollow(var_file);")
set_enoent = set_permission.index("if (saved_errno != ENOENT)", set_read)
set_init = set_permission.index("initial_one_var_file_in_mount_path_locked(account, mount_path)", set_enoent)
set_retry = set_permission.index("var_info = read_regular_file_nofollow(var_file);", set_init)
require(set_read < set_enoent < set_init < set_retry and
        "sleep(1);" not in set_permission,
        "permission update may initialize defaults only for ENOENT and must check the result")
require("if (result != 0)" in set_permission[set_init:set_retry],
        "permission update must propagate metadata initialization failure")




reserved_account_body = function_body(
    DISK_SOURCE, "static int\nis_reserved_account_name(const char *name)"
)
reserved_account_compact = "".join(reserved_account_body.split())
require("nvram_get_checked(\"http_username\",&management_name)" in reserved_account_compact and
        "router_account_name_is_reserved_after_read(" in reserved_account_compact and
        "nvram_safe_get(" not in reserved_account_compact and
        "if(read_status<0&&errno==0)errno=EIO;" in reserved_account_compact,
        "reserved storage-account checks must reject transient or inconsistent management-name reads rather than treating errors as an absent username")
require("router_account_name_is_reserved_after_read" in ACCOUNT_NAME_POLICY and
        "read_status!=1||!management_name" in "".join(ACCOUNT_NAME_POLICY.split()),
        "reserved-name policy must fail closed for backend errors and inconsistent present-value results")

account_list_body = function_body(
    DISK_SOURCE, "int get_account_list(int *acc_num, char ***account_list)"
)
account_list_compact = "".join(account_list_body.split())
require("disk_share_account_inventory_read(&disk_share_account_nvram_operations,&transaction,MAX_ACCOUNT_NUM,acc_num,account_list)" in account_list_compact and
        "nvram_get_int(" not in account_list_body and
        "nvram_safe_get(" not in account_list_body,
        "get_account_list must use the checked account inventory policy rather than error-collapsing NVRAM getters")

for helper, expected in (
    ("disk_share_account_nvram_begin", "nvram_transaction_begin("),
    ("disk_share_account_nvram_get", "nvram_transaction_get("),
    ("disk_share_account_nvram_end", "nvram_transaction_end("),
):
    helper_body = function_body(DISK_SOURCE, "static int\n" + helper + "(")
    require(expected in helper_body,
            helper + " must bind inventory reads to the checked NVRAM transaction API")

require("router_account_nvram_read_count" in ACCOUNT_POLICY and
        "router_account_nvram_read_copy" in ACCOUNT_POLICY and
        "ops->end(context)" in ACCOUNT_POLICY and
        "seen_empty" in ACCOUNT_POLICY and
        "ops->reserved_name(value, http_username)" in ACCOUNT_POLICY and
        "disk_share_account_inventory_free(count, items)" in ACCOUNT_POLICY,
        "account inventory policy must check/parse count, copy cache-backed identity values, release its transaction, reject sparse/reserved records and free partial snapshots")

account_inventory_tests = (
    "test_account_inventory_reads_one_checked_snapshot",
    "test_account_inventory_missing_count_is_empty",
    "test_account_inventory_count_read_error_is_not_zero",
    "test_account_inventory_username_error_frees_partial_list",
    "test_account_inventory_transaction_end_error_discards_snapshot",
    "test_account_inventory_begin_error_does_not_end_transaction",
    "test_account_inventory_rejects_invalid_count_text_and_range",
    "test_account_inventory_rejects_sparse_and_reserved_names",
    "test_account_inventory_rejects_invalid_get_status",
    "test_account_inventory_rejects_duplicate_names",
)
for marker in account_inventory_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing checked account inventory regression: " + marker)

policy_test_main = function_body(POLICY_TEST, "main(void)")
policy_test_return = policy_test_main.index("return 0;")
for marker in account_inventory_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "account inventory regression is defined but not executed: " + marker)

add_account_body = function_body(
    DISK_SOURCE, "int add_account(const char *const account, const char *const password)"
)
add_account_compact = "".join(add_account_body.split())
add_inventory = add_account_compact.index("get_account_list(&acc_num,&account_list)")
add_disk_preflight = add_account_compact.index("read_disk_data_checked(&disk_list)")
add_begin = add_account_compact.index("disk_share_account_nvram_begin(&transaction)")
add_validate = add_account_compact.index("disk_share_account_inventory_add_validate_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list,account)")
add_init = add_account_compact.index("initial_one_var_file_in_mount_path_locked(", add_validate)
add_publish = add_account_compact.index("disk_share_account_inventory_add_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list,account,password)")
add_rollback = add_account_compact.index(
    "disk_share_account_created_files_rollback(files,unlink,&rollback_error)",
    add_publish
)
add_end = add_account_compact.index("disk_share_account_nvram_end(&transaction)", add_rollback)
add_service_reload = add_account_compact.index('system("/sbin/run_samba")', add_end)
require(add_inventory < add_disk_preflight < add_begin < add_validate < add_init <
        add_publish < add_rollback < add_end < add_service_reload and
        "nvram_set(" not in add_account_body and
        "nvram_set_int(" not in add_account_body and
        "nvram_commit_safe(" not in add_account_body,
        "add_account must revalidate a checked inventory under one writer lease, create all volume metadata before transactional publication, compensate failure, and release the lease before service reload")
del_account_body = function_body(
    DISK_SOURCE, "int del_account(const char *const account)"
)
del_account_compact = "".join(del_account_body.split())
saved_count = del_account_compact.index("account_count=acc_num;")
target_scan = del_account_compact.index("for(i=0;i<account_count;++i)")
delete_begin = del_account_compact.index("disk_share_account_nvram_begin(&transaction)")
delete_transaction = del_account_compact.index(
    "disk_share_account_inventory_remove_locked(&disk_share_account_nvram_operations,&transaction,account_count,target,MAX_ACCOUNT_NUM,account_list)",
    delete_begin
)
delete_failure_free = del_account_compact.index(
    "free_2_dimension_list(&account_count,&account_list);", delete_transaction
)
delete_failure_return = del_account_compact.index("return-1;", delete_failure_free)
delete_success_free = del_account_compact.index(
    "free_2_dimension_list(&account_count,&account_list);", delete_failure_return
)
require(saved_count < target_scan < delete_transaction < delete_failure_free <
        delete_failure_return < delete_success_free and
        "free_2_dimension_list(&acc_num,&account_list);" in
        del_account_compact[:saved_count] and
        "--acc_num;" not in del_account_compact and
        "account_count,target,MAX_ACCOUNT_NUM,account_list" in
        del_account_compact[delete_transaction:delete_failure_free],
        "del_account must retain and pass the complete original account inventory to the checked transactional remover, then release it on both failure and success")

delete_inventory = del_account_compact.index("read_disk_data_checked(&disk_list)")
delete_transaction = del_account_compact.index(
    "disk_share_account_inventory_remove_locked(&disk_share_account_nvram_operations,&transaction,account_count,target,MAX_ACCOUNT_NUM,account_list)"
)
delete_unlink = del_account_compact.index("unlink(var_file)", delete_transaction)
delete_end = del_account_compact.index("disk_share_account_nvram_end(&transaction)", delete_unlink)
require(delete_inventory < delete_begin < delete_transaction < delete_unlink < delete_end and
        "nvram_set(" not in del_account_body and
        "nvram_set_int(" not in del_account_body and
        "nvram_safe_get(" not in del_account_body and
        "nvram_commit_safe(" not in del_account_body,
        "del_account must resolve a complete storage inventory first and retain one checked writer lease through NVRAM compaction and per-volume metadata cleanup")


commit_policy_tests = (
    "test_account_commit_policy_defaults_to_commit_for_missing_key",
    "test_account_commit_policy_preserves_manual_commit_setting",
    "test_account_commit_policy_rejects_read_errors",
    "test_account_commit_policy_rejects_inconsistent_results",
)
for marker in commit_policy_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing checked NVRAM commit policy regression: " + marker)
for marker in commit_policy_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "NVRAM commit policy regression is defined but not executed: " + marker)


account_snapshot_tests = (
    "test_account_inventory_matches_locked_accepts_current_snapshot",
    "test_account_inventory_matches_locked_rejects_stale_snapshot",
    "test_account_inventory_matches_locked_rejects_invalid_or_failed_reads",
    "test_account_inventory_matches_locked_accepts_empty_inventory",
)
for marker in account_snapshot_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing locked account-snapshot regression: " + marker)
for marker in account_snapshot_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "locked account-snapshot regression is defined but not executed: " + marker)

snapshot_match_body = function_body(
    ACCOUNT_POLICY, "disk_share_account_inventory_matches_locked("
)
snapshot_match_compact = "".join(snapshot_match_body.split())
require("current_count!=expected_count" in snapshot_match_compact and
        "strcmp(current_user,expected_users[i])!=0" in snapshot_match_compact and
        "if(!expected_users[i])" in snapshot_match_compact,
        "folder metadata writers must be able to revalidate an owned account inventory under the shared writer lease")

account_add_tests = (
    "test_account_inventory_add_appends_under_locked_snapshot",
    "test_account_inventory_add_handles_empty_inventory",
    "test_account_inventory_add_honors_manual_commit",
    "test_account_inventory_add_rejects_duplicate_and_stale_inventory",
    "test_account_inventory_add_rejects_full_and_reserved_inventory",
    "test_account_inventory_add_rejects_tail_snapshot_errors",
    "test_account_inventory_add_rolls_back_each_failed_setter",
    "test_account_inventory_add_rolls_back_failed_commit",
    "test_account_inventory_add_reports_rollback_failure",
)
for marker in account_add_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing transactional account creation regression: " + marker)
for marker in account_add_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "transactional account creation regression is defined but not executed: " + marker)

add_validate_body = function_body(
    ACCOUNT_POLICY, "disk_share_account_inventory_add_validate_locked("
)
add_validate_compact = "".join(add_validate_body.split())
require("current_count!=expected_count" in add_validate_compact and
        "ops->reserved_name(account,http_username)" in add_validate_compact and
        "strcmp(current_user,expected_users[i])!=0" in add_validate_compact,
        "account creation must reject stale snapshots and current management/reserved identities")

add_policy_body = function_body(
    ACCOUNT_POLICY, "disk_share_account_inventory_add_locked("
)
add_policy_compact = "".join(add_policy_body.split())
for token in (
    "ops->set(context,\"acc_num\",count_value)",
    "ops->set(context,key,account)",
    "ops->set(context,key,password)",
    "rollback:",
    "if(rollback_errno!=0)saved_errno=EIO;",
    "disk_share_account_commit_required(",
):
    require(token in add_policy_compact,
            "transactional account creation is missing rollback/commit protection: " + token)

account_update_tests = (
    "test_account_inventory_update_renames_and_changes_password_atomically",
    "test_account_inventory_update_supports_password_only_and_manual_commit",
    "test_account_inventory_update_rejects_duplicate_reserved_and_stale",
    "test_account_inventory_update_rolls_back_ambiguous_setters",
    "test_account_inventory_update_rolls_back_failed_commit_and_absent_password",
    "test_account_inventory_update_rejects_snapshot_and_manual_read_errors",
)
for marker in account_update_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing transactional account-update regression: " + marker)
for marker in account_update_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "transactional account-update regression is defined but not executed: " + marker)

update_validate_body = function_body(
    ACCOUNT_POLICY, "disk_share_account_inventory_update_validate_locked("
)
update_validate_compact = "".join(update_validate_body.split())
require("current_count!=expected_count" in update_validate_compact and
        "!expected_users[target]" in update_validate_compact and
        "strcmp(current_user,expected_users[i])!=0" in update_validate_compact and
        "strcmp(expected_users[i],new_account)==0" in update_validate_compact,
        "account updates must reject stale inventories and duplicate destinations")
update_policy_body = function_body(
    ACCOUNT_POLICY, "disk_share_account_inventory_update_locked("
)
update_policy_compact = "".join(update_policy_body.split())
for token in (
    "ops->set(context,username_key,new_account)",
    "ops->set(context,password_key,new_password)",
    "rollback:",
    "password_present?old_password:NULL",
    "disk_share_account_commit_required(",
):
    require(token in update_policy_compact,
            "transactional account updates are missing compensation/commit policy: " + token)

account_delete_tests = (
    "test_account_inventory_remove_locked_keeps_caller_transaction_open",
    "test_account_inventory_remove_compacts_under_one_transaction",
    "test_account_inventory_remove_defaults_last_account_modes",
    "test_account_inventory_remove_rolls_back_last_account_mode_setter",
    "test_account_inventory_remove_honors_manual_commit",
    "test_account_inventory_remove_rejects_stale_inventory",
    "test_account_inventory_remove_rolls_back_failed_set",
    "test_account_inventory_remove_rolls_back_failed_commit",
    "test_account_inventory_remove_reports_rollback_failure",
    "test_account_inventory_remove_rejects_password_read_error",
)
for marker in account_delete_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing atomic account deletion regression: " + marker)
for marker in account_delete_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "atomic account deletion regression is defined but not executed: " + marker)
delete_policy_body = function_body(
    ACCOUNT_POLICY, "static inline int\ndisk_share_account_inventory_remove_locked"
)
delete_policy_compact = "".join(delete_policy_body.split())
require("ops->begin(" not in delete_policy_body and "ops->end(" not in delete_policy_body,
        "locked account deletion policy must leave transaction ownership with its caller")
remove_wrapper_body = function_body(
    ACCOUNT_POLICY, "disk_share_account_inventory_remove("
)
require("ops->begin(context)" in remove_wrapper_body and
        "disk_share_account_inventory_remove_locked(" in remove_wrapper_body and
        "ops->end(context)" in remove_wrapper_body,
        "standalone account deletion adapter must preserve begin/operate/end semantics")

for token in (
    "ops->set(context,\"acc_num\",count_value)",
    "ops->commit(context)",
    "rollback:",
    "if(rollback_errno!=0)saved_errno=EIO;",
    "current_count!=expected_count",
):
    require(token in delete_policy_compact,
            "atomic account deletion policy is missing rollback/snapshot behavior: " + token)

account_mode_tests = (
    "test_account_mode_policy_defaults_missing_values",
    "test_account_mode_policy_preserves_supported_modes",
    "test_account_mode_policy_rejects_backend_and_malformed_values",
)
for marker in account_mode_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing checked account mode regression: " + marker)
for marker in account_mode_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "account mode regression is defined but not executed: " + marker)
require("disk_share_account_mode_needs_default" in ACCOUNT_POLICY and
        "parsed<INT_MIN||parsed>INT_MAX" in "".join(ACCOUNT_POLICY.split()),
        "last-account mode policy must reject invalid backend values and integer overflow")

delete_policy_body = function_body(
    ACCOUNT_POLICY, "static inline int\ndisk_share_account_inventory_remove"
)
delete_policy_compact = "".join(delete_policy_body.split())
require("disk_share_account_mode_needs_default(status,value,&mode_default[j])" in delete_policy_compact and
        "mode_attempted[j]=1;" in delete_policy_compact and
        "mode_present[j]?old_mode[j]:NULL" in delete_policy_compact,
        "last-account mode updates must snapshot and restore exact prior values inside the account transaction")

require("disk_share_account_commit_required" in ACCOUNT_POLICY,
        "account persistence policy must preserve the legacy manual-commit rule")

commit_helper = function_body(DISK_SOURCE, "static int nvram_commit_safe(void)")
commit_helper_compact = "".join(commit_helper.split())
manual_read = 'nvram_get_checked("nvram_manual",&manual_value)'
manual_policy = "disk_share_account_commit_required(read_status,manual_value,&commit_required)"
require(manual_read in commit_helper_compact and manual_policy in commit_helper_compact and
        "nvram_get_int(" not in commit_helper and "nvram_safe_get(" not in commit_helper and
        commit_helper_compact.index(manual_read) < commit_helper_compact.index(manual_policy) <
        commit_helper_compact.index("nvram_commit()"),
        "account persistence must check the manual-commit read and commit result rather than collapsing NVRAM errors")


del_inventory = del_account_compact.index("read_disk_data_checked(&disk_list)")
del_inventory_error = del_account_compact.index(
    "saved_errno=errno?errno:EIO;", del_inventory
)
del_inventory_errno_restore = del_account_compact.index(
    "errno=saved_errno;", del_inventory_error
)
del_inventory_return = del_account_compact.index(
    "return-1;", del_inventory_errno_restore
)
del_path = del_account_compact.index(
    "disk_share_format_account_var_path(var_file,sizeof(var_file),",
    del_inventory_return
)
del_unlink = del_account_compact.index("unlink(var_file)", del_path)
require("read_disk_data()" not in del_account_compact and
        del_inventory < del_inventory_error < del_inventory_errno_restore <
        del_inventory_return < del_path < del_unlink and
        "if(disk_share_format_account_var_path(" in del_account_compact and
        "errno!=ENOENT" in del_account_compact[del_unlink:] and
        "first_error==0" in del_account_compact[del_unlink:],
        "del_account must distinguish an empty inventory from a failed scan, reject truncated paths, ignore only ENOENT, and preserve other unlink failures")
del_free_disk = del_account_compact.index("free_disk_data(disk_list)", del_unlink)
del_service = del_account_compact.index('system("/sbin/run_ftpsamba")', del_free_disk)
del_error = del_account_compact.index("if(first_error!=0)", del_service)
require(del_unlink < del_free_disk < del_service < del_error and
        "errno=first_error;" in del_account_compact and
        "return-1;" in del_account_compact[del_error:] and
        "return0;" not in del_account_compact[del_inventory:del_service],
        "del_account must attempt all visible volumes, reload FTP/Samba even for empty/unknown inventory, and return the first inventory/cleanup error")

require("0 = complete inventory (list may be NULL); -1 = incomplete/unknown." in DISK_INITIAL_HEADER and
        "read_disk_data_checked(disk_info_t **disk_info_out)" in DISK_INITIAL_HEADER,
        "disk inventory API must preserve empty-versus-incomplete status semantics")

password_shift_tests = (
    "test_account_password_shift_snapshot_copies_values_in_one_transaction",
    "test_account_password_shift_snapshot_preserves_missing_as_empty",
    "test_account_password_shift_snapshot_read_error_clears_all_values",
    "test_account_password_shift_snapshot_end_error_discards_values",
    "test_account_password_shift_snapshot_rejects_oversized_value",
)
for marker in password_shift_tests:
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing password-shift snapshot regression: " + marker)
for marker in password_shift_tests:
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "password-shift snapshot regression is defined but not executed: " + marker)



mod_account_body = function_body(
    DISK_SOURCE,
    "int mod_account(const char *const account, const char *const new_account, const char *const new_password)"
)
mod_account_compact = "".join(mod_account_body.split())
mod_inventory = mod_account_compact.index("read_disk_data_checked(&disk_list)")


require("disk_share_command_status" in POLICY,
        "account-management service reloads must normalize system() wait status")
for signature, command in (
    ("int add_account(const char *const account, const char *const password)", 'system("/sbin/run_samba")'),
    ("int del_account(const char *const account)", 'system("/sbin/run_ftpsamba")'),
    ("int mod_account(const char *const account, const char *const new_account, const char *const new_password)", 'system("/sbin/run_ftpsamba")'),
):
    body = function_body(DISK_SOURCE, signature)
    compact = "".join(body.split())
    command_offset = compact.index(command)
    status_offset = compact.index("disk_share_command_status(service_result,errno)", command_offset)
    error_offset = compact.index("first_error=service_error;", status_offset)
    require(command_offset < status_offset < error_offset and
            "service_error!=0&&first_error==0" in compact[status_offset:error_offset + 40],
            "account-management functions must propagate failed service reloads: " + signature)
for marker in (
    "test_account_service_command_status_accepts_exit_zero",
    "test_account_service_command_status_rejects_nonzero_exit_and_signal",
    "test_account_service_command_status_preserves_invocation_errno",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing service reload result regression: " + marker)
for marker in (
    "test_account_service_command_status_accepts_exit_zero",
    "test_account_service_command_status_rejects_nonzero_exit_and_signal",
    "test_account_service_command_status_preserves_invocation_errno",
):
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "service reload regression is defined but not executed: " + marker)


require("sizeof(((structdisk_share_account_created_file_entry*)0)->mount_path)" in "".join(add_account_body.split()),
        "add_account must validate mount-path length against the complete fixed-size destination buffer")

require("disk_share_account_created_files_rollback" in POLICY and
        "unlink_path(entry->path)" in "".join(POLICY.split()),
        "new account metadata must support compensating removal after failed NVRAM publication")
for marker in (
    "test_account_created_file_rollback_removes_all_created_files",
    "test_account_created_file_rollback_reports_failures_and_continues",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing new-account metadata rollback regression: " + marker)
for marker in (
    "test_account_created_file_rollback_removes_all_created_files",
    "test_account_created_file_rollback_reports_failures_and_continues",
):
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "new-account metadata rollback regression is defined but not executed: " + marker)

require("disk_share_account_rename_entries_apply" in POLICY and
        "rename_path(undo->destination,undo->source)" in "".join(POLICY.split()),
        "multi-volume account metadata rename policy must compensate prior moves in reverse order")
for marker in (
    "test_account_rename_batch_rolls_back_prior_moves",
    "test_account_rename_batch_reports_incomplete_rollback",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing cross-volume rename rollback regression: " + marker)
for marker in (
    "test_account_rename_batch_rolls_back_prior_moves",
    "test_account_rename_batch_reports_incomplete_rollback",
):
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "cross-volume rename rollback regression is defined but not executed: " + marker)

mod_loop = mod_account_compact.index("for(follow_disk=disk_list;", mod_inventory)
mod_old_path = mod_account_compact.index("disk_share_format_account_var_path(var_file,sizeof(var_file),", mod_loop)
mod_new_path = mod_account_compact.index("disk_share_format_account_var_path(new_var_file,sizeof(new_var_file),", mod_old_path)
mod_old_stat = mod_account_compact.index("lstat(var_file,&old_stat)", mod_new_path)
mod_target_stat = mod_account_compact.index("lstat(new_var_file,&new_stat)", mod_old_stat)
mod_plan = mod_account_compact.index("calloc(1,sizeof(*move))", mod_target_stat)
mod_begin = mod_account_compact.index("disk_share_account_nvram_begin(&transaction)", mod_plan)
mod_validate = mod_account_compact.index("disk_share_account_inventory_update_validate_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list,account_order,account,rename_required?new_account:NULL)", mod_begin)
mod_rename = mod_account_compact.index("disk_share_account_rename_entries_apply(moves,rename,&rollback_error)", mod_validate)
mod_publish = mod_account_compact.index("disk_share_account_inventory_update_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list,account_order,account,rename_required?new_account:NULL,password_update?new_password:NULL)", mod_rename)
mod_fs_rollback = mod_account_compact.index("disk_share_account_rename_entries_rollback(moves,rename,&rollback_error)", mod_publish)
mod_cleanup = mod_account_compact.index("free_disk_data(disk_list)", mod_fs_rollback)
mod_end = mod_account_compact.index("disk_share_account_nvram_end(&transaction)", mod_cleanup)
mod_service = mod_account_compact.index('system("/sbin/run_ftpsamba")', mod_end)
mod_error = mod_account_compact.index("if(first_error!=0)", mod_service)
require(mod_inventory < mod_loop < mod_old_path < mod_new_path < mod_old_stat <
        mod_target_stat < mod_plan < mod_begin < mod_validate < mod_rename <
        mod_publish < mod_fs_rollback < mod_cleanup < mod_end < mod_service < mod_error and
        "rename(var_file,new_var_file)" not in mod_account_compact and
        "nvram_set(" not in mod_account_body and
        "nvram_set_int(" not in mod_account_body and
        "nvram_commit_safe(" not in mod_account_body and
        "nvram_get_checked(" not in mod_account_body,
        "mod_account must preflight every volume, then revalidate and serialize the metadata rename/NVRAM update under one writer lease")
require("saved_errno==ENOENT" in mod_account_compact[mod_old_stat:mod_target_stat] and
        "S_ISREG(old_stat.st_mode)" in mod_account_body and
        "first_error=EEXIST;" in mod_account_compact[mod_target_stat:mod_plan],
        "mod_account must treat only missing source metadata as idempotent and reject non-regular or colliding paths")
require("rollback_error!=0" in mod_account_compact and
        "errno=first_error;" in mod_account_compact and
        "return-1;" in mod_account_compact[mod_error:],
        "mod_account must report incomplete rename compensation and preserve the first error")
require("snprintf(var_file" not in mod_account_body and
        "snprintf(new_var_file" not in mod_account_body,
        "mod_account must use bounded account metadata path formatting")
for marker in (
    "test_account_rename_batch_rolls_back_after_successful_publish",
    "test_account_rename_batch_reports_failed_compensation",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing compensating account rename regression: " + marker)
for marker in (
    "test_account_rename_batch_rolls_back_after_successful_publish",
    "test_account_rename_batch_reports_failed_compensation",
):
    require(policy_test_main.index(marker + "();") < policy_test_return,
            "compensating account rename regression is defined but not executed: " + marker)

add_folder_body = function_body(
    DISK_SOURCE,
    "int add_folder(const char *const mount_path, const char *const folder)"
)
add_compact = "".join(add_folder_body.split())
add_inventory = add_compact.index("get_account_list(&acc_num,&account_list)")
add_begin = add_compact.index("disk_share_account_nvram_begin(&transaction)", add_inventory)
add_validate = add_compact.index(
    "disk_share_account_inventory_matches_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list)",
    add_begin
)
add_lookup = add_compact.index(
    "test_if_exist_folder_in_mount_path(mount_path,folder)", add_validate
)
add_mkdir = add_compact.index("result=mkdir(full_path,0777)", add_lookup)
add_publish = add_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", add_mkdir
)
add_permissions = add_compact.index(
    "create_if_no_var_files_with_accounts(mount_path,acc_num,account_list)",
    add_publish
)
add_success_end = add_compact.index(
    "disk_share_account_nvram_end(&transaction)", add_permissions
)
add_rollback = add_compact.index("rmdir(full_path)", add_success_end)
add_remove_guard = add_compact.index(
    "disk_share_folder_rollback_can_remove_permissions(remove_result,remove_errno)",
    add_rollback
)
add_permission_rollback = add_compact.index(
    "disk_share_update_folder_permission_file(mount_path,account_list[i],target,NULL)",
    add_rollback
)
add_anonymous_rollback = add_compact.index(
    "disk_share_update_folder_permission_file(mount_path,FTP_ANONYMOUS_USER,target,NULL)",
    add_permission_rollback
)
add_snapshot_restore = add_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", add_anonymous_rollback
)
add_failed_end = add_compact.index(
    "disk_share_account_nvram_end(&transaction)", add_snapshot_restore
)
require(add_inventory < add_begin < add_validate < add_lookup < add_mkdir <
        add_publish < add_permissions < add_success_end < add_rollback < add_remove_guard <
        add_permission_rollback < add_anonymous_rollback < add_snapshot_restore <
        add_failed_end and
        "errno=EEXIST;" in add_compact and
        "errno=ENAMETOOLONG;" in add_compact and
        "create_if_no_var_files(mount_path)" not in add_folder_body and
        "disk_share_folder_rollback_can_remove_permissions" in POLICY and
        "test_folder_rollback_preserves_permissions_when_directory_survives" in POLICY_TEST and
        "test_folder_rollback_preserves_permissions_when_directory_survives();" in POLICY_TEST,
        "add_folder must serialize with account mutations, preserve ACL rows when rollback cannot remove a surviving directory, and undo partial permission rows only after directory removal")


require("disk_share_reserve_staging_directory" in POLICY and
        "EEXIST" in POLICY and
        "disk_share_mkdir_staging_directory" in DISK_SOURCE and
        "disk_share_create_directory_entry_nofollow(directory_fd,leaf,0700)" in DISK_SOURCE and
        "mkdir(path, 0700)" not in DISK_SOURCE and
        "disk_share_remove_folder_path(staging_dir)" in DISK_SOURCE,
        "folder deletion must reserve, create and remove its staging parent through pinned directory descriptors")

del_folder = function_body(
    DISK_SOURCE, "int del_folder(const char *const mount_path, const char *const folder)"
)
del_folder_compact = "".join(del_folder.split())
del_inventory = del_folder_compact.index("get_account_list(&acc_num,&account_list)")
del_begin = del_folder_compact.index("disk_share_account_nvram_begin(&transaction)", del_inventory)
del_validate = del_folder_compact.index(
    "disk_share_account_inventory_matches_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list)",
    del_begin
)
del_plan = del_folder_compact.index("disk_share_permission_batch_prepare_rewrite(", del_validate)
del_stage = del_folder_compact.index("disk_share_create_folder_staging_paths(", del_plan)
del_batch = del_folder_compact.index(
    "disk_share_folder_delete_batch(full_path,staging_path,mount_path,permission_batch,permission_batch_count,disk_share_replace_file_atomic,disk_share_rename_folder_path,disk_share_remove_folder_path,disk_share_refresh_folder_snapshot,&staging_applied,&rollback_error)",
    del_stage
)
del_release = del_folder_compact.index(
    "disk_share_account_nvram_end(&transaction)", del_batch
)
require(del_inventory < del_begin < del_validate < del_plan < del_stage <
        del_batch < del_release and
        "disk_share_permission_batch_prepare_rewrite" in DISK_SOURCE and
        "disk_share_reserve_staging_directory" in DISK_SOURCE and
        "disk_share_folder_delete_batch" in POLICY and
        "disk_share_remove_folder_path" in DISK_SOURCE and
        "disk_share_remove_folder_path(staging_dir)" in DISK_SOURCE and
        "mkdir(path, 0700)" not in DISK_SOURCE and
        "lstat(staging_path" not in DISK_SOURCE and
        "disk_share_folder_name_is_visible" in "".join(DISK_SOURCE.split()),
        "folder deletion must preflight metadata then use the tested staged-directory/ACL/snapshot compensation state machine")

for marker in (
    "test_folder_delete_batch_commits_directory_snapshot_and_permissions",
    "test_folder_delete_batch_restores_directory_after_snapshot_failure",
    "test_folder_delete_batch_restores_files_and_directory_after_write_failure",
    "test_folder_delete_batch_restores_after_staged_directory_remove_failure",
    "test_folder_delete_batch_preserves_staging_on_directory_compensation_failure",
    "test_folder_entry_status_rejects_symlinks_and_non_directories",
    "test_stale_metadata_unlink_uses_pinned_directory_fd",
    "test_folder_rename_batch_commits_directory_snapshot_and_permissions",
    "test_folder_rename_batch_restores_directory_after_snapshot_failure",
    "test_folder_rename_batch_restores_files_and_directory_after_write_failure",
    "test_folder_rename_batch_reports_directory_compensation_failure",
    "test_staging_directory_reserves_with_atomic_collision_retry",
    "test_staging_directory_preserves_create_errors",
    "test_staging_directory_rejects_truncated_paths_before_create",
    "test_permission_batch_applies_and_restores_all_files",
    "test_permission_batch_rolls_back_prior_file_after_write_failure",
    "test_permission_batch_reports_and_retries_failed_compensation",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing permission batch rollback regression: " + marker)

folder_delete_policy = function_body(POLICY, "disk_share_folder_delete_batch(")
folder_delete_policy_compact = "".join(folder_delete_policy.split())
delete_move = folder_delete_policy_compact.index("move_path(folder_path,staging_path)")
delete_refresh = folder_delete_policy_compact.index("refresh_snapshot(mount_path)", delete_move)
delete_apply = folder_delete_policy_compact.index(
    "disk_share_permission_batch_apply(entries,count,write_file,&compensation_error)",
    delete_refresh
)
delete_remove = folder_delete_policy_compact.index("remove_path(staging_path)", delete_apply)
delete_rollback = folder_delete_policy_compact.index(
    "disk_share_permission_batch_rollback(entries,count,write_file,&compensation_error)",
    delete_remove
)
delete_move_back = folder_delete_policy_compact.index(
    "move_path(staging_path,folder_path)", delete_rollback
)
delete_refresh_back = folder_delete_policy_compact.index(
    "refresh_snapshot(mount_path)", delete_move_back
)
require(delete_move < delete_refresh < delete_apply < delete_remove <
        delete_rollback < delete_move_back < delete_refresh_back,
        "delete policy must compensate ACLs, directory placement and folder snapshot after any post-stage failure")

mod_folder_body = function_body(
    DISK_SOURCE,
    "int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)"
)
mod_folder_compact = "".join(mod_folder_body.split())
mod_accounts = mod_folder_compact.index("get_account_list(&acc_num,&account_list)")
mod_begin = mod_folder_compact.index("disk_share_account_nvram_begin(&transaction)", mod_accounts)
mod_validate = mod_folder_compact.index(
    "disk_share_account_inventory_matches_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list)",
    mod_begin
)
mod_live_check = mod_folder_compact.index("lstat(full_path,&target_stat)", mod_validate)
mod_no_follow_check = mod_folder_compact.index(
    "disk_share_folder_entry_status(result,errno", mod_live_check
)
mod_snapshot_check = mod_folder_compact.index(
    "test_if_exist_folder_in_mount_path(mount_path,folder)", mod_no_follow_check
)
mod_refresh = mod_folder_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", mod_snapshot_check
)
mod_plan = mod_folder_compact.index(
    "disk_share_permission_batch_prepare_rewrite(mount_path,acc_num,account_list,target,new_target,&permission_batch,&permission_batch_count)",
    mod_refresh
)
mod_apply = mod_folder_compact.index(
    "disk_share_folder_rename_batch(full_path,new_full_path,mount_path,permission_batch,permission_batch_count,disk_share_replace_file_atomic,disk_share_rename_folder_path,disk_share_refresh_folder_snapshot,&rollback_error)",
    mod_plan
)
mod_release = mod_folder_compact.index("disk_share_account_nvram_end(&transaction)", mod_apply)
require(mod_accounts < mod_begin < mod_validate < mod_live_check <
        mod_no_follow_check < mod_snapshot_check < mod_refresh < mod_plan <
        mod_apply < mod_release and
        "disk_share_update_folder_permission_file(" not in mod_folder_compact,
        "folder rename must preflight every ACL image and delegate filesystem/ACL rollback to the tested transaction policy")

folder_rename_policy = function_body(POLICY, "disk_share_folder_rename_batch(")
folder_rename_policy_compact = "".join(folder_rename_policy.split())
policy_move = folder_rename_policy_compact.index("move_path(old_path,new_path)")
policy_refresh = folder_rename_policy_compact.index("refresh_snapshot(mount_path)", policy_move)
policy_apply = folder_rename_policy_compact.index(
    "disk_share_permission_batch_apply(entries,count,write_file,&compensation_error)",
    policy_refresh
)
policy_rollback = folder_rename_policy_compact.index(
    "disk_share_permission_batch_rollback(entries,count,write_file,&compensation_error)",
    policy_apply
)
policy_move_back = folder_rename_policy_compact.index("move_path(new_path,old_path)", policy_rollback)
policy_refresh_back = folder_rename_policy_compact.index("refresh_snapshot(mount_path)", policy_move_back)
require(policy_move < policy_refresh < policy_apply < policy_rollback <
        policy_move_back < policy_refresh_back,
        "rename policy must compensate permission contents, directory name and folder snapshot after a failed step")

rename_folder = function_body(
    DISK_SOURCE, "disk_share_rename_folder_path("
)
rename_folder_compact = "".join(rename_folder.split())
rename_policy = function_body(
    POLICY, "disk_share_rename_directory_entry_nofollow("
)
rename_policy_compact = "".join(rename_policy.split())
require("disk_io_open_parent_nofollow(old_path,&old_leaf)" in rename_folder_compact and
        "disk_io_open_parent_nofollow(new_path,&new_leaf)" in rename_folder_compact and
        "disk_share_rename_directory_entry_nofollow(old_directory_fd,old_leaf,new_directory_fd,new_leaf)" in rename_folder_compact and
        "rename(old_path,new_path)" not in rename_folder_compact and
        "fstatat(old_directory_fd,old_leaf,&source_status,AT_SYMLINK_NOFOLLOW)" in rename_policy_compact and
        "!S_ISDIR(source_status.st_mode)" in rename_policy_compact and
        "fstatat(new_directory_fd,new_leaf,&target_status,AT_SYMLINK_NOFOLLOW)" in rename_policy_compact and
        "returnrenameat(old_directory_fd,old_leaf,new_directory_fd,new_leaf)" in rename_policy_compact and
        "test_folder_rename_uses_pinned_parent_descriptors();" in POLICY_TEST,
        "folder moves must pin both parent directories and reject symlink/non-directory sources or occupied targets")

mkdir_staging = function_body(
    DISK_SOURCE, "disk_share_mkdir_staging_directory("
)
mkdir_staging_compact = "".join(mkdir_staging.split())
remove_staging = function_body(
    DISK_SOURCE, "disk_share_remove_folder_path("
)
remove_staging_compact = "".join(remove_staging.split())
require("disk_io_open_parent_nofollow(path,&leaf)" in mkdir_staging_compact and
        "disk_share_create_directory_entry_nofollow(directory_fd,leaf,0700)" in mkdir_staging_compact and
        "disk_io_open_parent_nofollow(path,&leaf)" in remove_staging_compact and
        "disk_share_remove_directory_entry_nofollow(directory_fd,leaf)" in remove_staging_compact and
        "mkdirat(directory_fd,leaf,mode)" in rename_policy_compact and
        "unlinkat(directory_fd,leaf,AT_REMOVEDIR)" in rename_policy_compact and
        "test_staging_directory_operations_use_pinned_parent_fd();" in POLICY_TEST,
        "staging directory creation and removal must remain under the pinned mount parent and reject final symlinks")




permission_update = function_body(
    DISK_SOURCE,
    "static int\ndisk_share_update_folder_permission_file(const char *mount_path,\n                                         const char *account,\n                                         const char *target,\n                                         const char *replacement)\n{"
)
update_compact = " ".join(permission_update.split())
update_path = update_compact.index(
    "disk_share_format_account_var_path(var_file, sizeof(var_file)"
)
update_read = update_compact.index("read_regular_file_nofollow(var_file)", update_path)
update_rewrite = update_compact.index(
    "disk_share_permission_rewrite(var_info, target, replacement", update_read
)
update_replace = update_compact.index(
    "disk_share_replace_file_atomic(var_file, updated_info", update_rewrite
)
require(update_path < update_read < update_rewrite < update_replace and
        "if (saved_errno == ENOENT)" in permission_update and
        "errno = saved_errno;" in permission_update,
        "shared account updater must bound paths, skip only missing files/records and atomically publish complete permission data")

mod_folder_body = function_body(
    DISK_SOURCE,
    "int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)"
)
mod_folder_compact = "".join(mod_folder_body.split())
mod_source_exists = mod_folder_compact.index("lstat(full_path,&target_stat)")
mod_no_follow = mod_folder_compact.index("disk_share_folder_entry_status(result,errno", mod_source_exists)
mod_snapshot_lookup = mod_folder_compact.index(
    "test_if_exist_folder_in_mount_path(mount_path,folder)", mod_source_exists
)
mod_stale_refresh = mod_folder_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", mod_snapshot_lookup
)
mod_rename_batch = mod_folder_compact.index(
    "disk_share_folder_rename_batch(full_path,new_full_path,mount_path",
    mod_stale_refresh
)
require(mod_source_exists < mod_no_follow < mod_snapshot_lookup < mod_stale_refresh < mod_rename_batch and
        "add_folder(mount_path,folder)" not in mod_folder_compact[mod_snapshot_lookup:mod_rename_batch] and
        "rename(full_path,new_full_path)" not in mod_folder_compact,
        "mod_folder must refresh stale folder metadata, then delegate directory/ACL publication and rollback to the tested rename batch")

get_permission = function_body(DISK_SOURCE, "int get_permission(")
get_permission_compact = " ".join(get_permission.split())
get_path_check = get_permission_compact.index(
    "disk_share_format_account_var_path(var_file, sizeof(var_file)"
)
require("return -1;" in get_permission_compact[get_path_check:],
        "permission lookup must fail closed when the metadata pathname is too long")

set_permission = function_body(DISK_SOURCE, "int set_permission(const char *account,")
require("dms_right = flag;" in set_permission and
        "record_rights[2] = (char)('0' + dms_right);" in set_permission,
        "new DMS permission records must preserve the requested DMS right")
atomic_replace_at = function_body(
    POLICY, "disk_share_replace_file_atomic_at(int directory_fd,"
)
atomic_replace_at_compact = "".join(atomic_replace_at.split())
require("have_existing=1;" in atomic_stream_compact and
        "if(preserve_existing_mode&&have_existing&&fchmod(fd,existing_stat.st_mode&0777)!=0)" in atomic_stream_compact,
        "atomic replacement must only preserve an initialized existing-file mode when explicitly requested")

set_permission = function_body(DISK_SOURCE, "int set_permission(const char *account,")
require("disk_share_replace_file_atomic(" in set_permission and
        "disk_share_format_account_var_path" in set_permission and
        'fopen(var_file, "w")' not in set_permission and
        'fopen(var_file, "a+")' not in set_permission,
        "set_permission must use checked path construction and atomic publication")

del_folder_body = function_body(
    DISK_SOURCE, "int del_folder(const char *const mount_path, const char *const folder)"
)
require("disk_share_permission_batch_prepare_rewrite(" in del_folder_body and
        "disk_share_folder_delete_batch(" in del_folder_body and
        "disk_share_create_folder_staging_paths(" in del_folder_body and
        "disk_share_remove_folder_path" in DISK_SOURCE and
        "disk_share_refresh_folder_snapshot" in DISK_SOURCE and
        "disk_share_remove_folder_path(staging_dir)" in del_folder_body and
        "!staging_applied" in del_folder_body,
        "folder deletion must prepare ACL images then delegate staged directory, permission publication and snapshot compensation to the tested transaction helper")

permission_batch_prepare = function_body(
    DISK_SOURCE, "disk_share_permission_batch_prepare_rewrite("
)
permission_batch_prepare_compact = "".join(permission_batch_prepare.split())
require("read_whole_file(" not in DISK_SOURCE and
        DISK_SOURCE.count("read_regular_file_nofollow(") >= 6 and
        "test_if_file(var_file)" not in DISK_SOURCE,
        "all account ACL files on removable volumes must use no-follow regular-file inspection/read paths")
require("disk_share_format_account_var_path(" in permission_batch_prepare_compact and
        "read_regular_file_nofollow(" in permission_batch_prepare_compact and
        "disk_share_permission_rewrite(" in permission_batch_prepare_compact and
        "replacement" in permission_batch_prepare_compact and
        "saved_errno==ENOENT" in permission_batch_prepare_compact,
        "folder batch preparation must snapshot original contents and construct checked replacement images before publishing writes")

mod_folder_body = function_body(
    DISK_SOURCE,
    "int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)"
)
require("disk_share_folder_rename_batch(" in mod_folder_body and
        "disk_share_rename_folder_path" in DISK_SOURCE and
        "disk_share_refresh_folder_snapshot" in DISK_SOURCE and
        'fopen(var_file, "w")' not in mod_folder_body and
        'fopen(var_file, "a+")' not in mod_folder_body,
        "folder rename must use the tested filesystem/permission transaction policy")

for marker in (
    "test_permission_rewrite_removes_complete_record",
    "test_permission_rewrite_renames_key_and_preserves_rights",
    "test_permission_rewrite_reports_missing_record_and_clears_outputs",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing permission record rewrite regression: " + marker)


samba = function_body(SAMBA_SOURCE, "write_smb_conf_unlocked(")
require("char **folder_list = NULL;" in samba,
        "Samba caller must initialize folder_list before the fallible API call")
call = samba.index("get_folder_list_in_mount_path(")
failure = samba.index("free_2_dimension_list(&sh_num, &folder_list);", call)
require(call < failure and "result = -1;" in samba[failure:],
        "Samba failure path must release the initialized list output")

for marker in (
    "test_invalid_mount_paths_clear_outputs",
    "test_missing_output_parameters_are_safe",
    "test_valid_mount_path_starts_with_empty_outputs",
    "test_readdir_entry_and_clean_eof",
    "test_readdir_error_is_not_clean_eof",
    "test_readdir_rejects_missing_adapters",
    "test_snapshot_write_flushes_complete_content",
    "test_snapshot_write_reports_io_failure",
    "test_snapshot_write_rejects_invalid_data",
    "test_account_defaults_write_complete_content",
    "test_account_defaults_write_reports_io_failure",
    "test_account_defaults_write_rejects_invalid_data",
):
    require(marker in REGRESSION and marker + "();" in REGRESSION,
            "missing output initialization regression: " + marker)



# Changes to share-list code and its installed public headers must schedule the XY-C1 build.
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
from check_xy_c1_firmware_inputs import is_non_firmware_input
for path in (
    "trunk/user/libdisk/src/Makefile",
    "trunk/user/libdisk/src/disk_share.c",
    "trunk/user/libdisk/src/disk_share.h",
    "trunk/user/libdisk/src/disk_share_list_policy.h",
    "trunk/user/libdisk/src/disk_io_tools.c",
):
    require(XY_WORKFLOW.count('      - "' + path + '"') == 1 and
            not is_non_firmware_input(path),
            "XY-C1 firmware path filter must watch production libdisk input: " + path)

print("libdisk share-list enumeration and Samba cleanup regression: PASS")
