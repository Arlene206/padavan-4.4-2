#!/usr/bin/env python3
"""Guard libdisk share-list output initialization and Samba error cleanup."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
LIBDISK = ROOT / "trunk/user/libdisk/src"
DISK_SOURCE = (LIBDISK / "disk_share.c").read_text()
DISK_IO_SOURCE = (LIBDISK / "disk_io_tools.c").read_text()
POLICY_TEST = (LIBDISK / "tests/disk_share_list_policy.c").read_text()
IO_REGRESSION = (LIBDISK / "tests/disk_io_tools_read_test.c").read_text()
POLICY = (LIBDISK / "disk_share_list_policy.h").read_text()
ACCOUNT_POLICY = (LIBDISK / "disk_share_account_nvram_policy.h").read_text()
DISK_INITIAL_HEADER = (LIBDISK / "disk_initial.h").read_text()
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

for signature in (
    "int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)",
    "int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)",
):
    body = function_body(DISK_SOURCE, signature)
    prepare = body.index("disk_share_list_prepare_outputs(mount_path, sh_num, folder_list)")
    use = body.index("strlen(mount_path)") if "strlen(mount_path)" in body else body.index("opendir(mount_path)")
    require(prepare < use,
            signature + " must reset outputs before using the mount path")


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
mkstemp = initial_writer.index("fd = mkstemp(temp_file)")
snapshot_write = initial_writer.index(
    "disk_share_list_write_snapshot(fp, sh_num, folder_list)", mkstemp
)
sync_file = initial_writer.index("fsync(fileno(fp))", snapshot_write)
close_file = initial_writer.index("if (fclose(fp) != 0)", sync_file)
publish_file = initial_writer.index("rename(temp_file, list_file)", close_file)
require(mkstemp < snapshot_write < sync_file < close_file < publish_file,
        "folder-list metadata must be flushed and closed before atomic publish")
require('fopen(list_file, "w")' not in initial_writer,
        "folder-list refresh must not truncate the last complete snapshot")
write_failure = initial_writer[initial_writer.index("failed:"):]
require("unlink(temp_file)" in write_failure and
        "errno = saved_errno;" in write_failure,
        "failed snapshot writes must remove the temporary file and preserve errno")


require("disk_share_write_account_defaults" in POLICY and
        "if (fflush(stream) != 0)" in POLICY,
        "account defaults writer must report buffered stream failures")
account_writer = function_body(
    DISK_SOURCE,
    "int initial_one_var_file_in_mount_path(const char *const account,"
)
account_mkstemp = account_writer.index("fd = mkstemp(temp_file)")
account_content = account_writer.index(
    "disk_share_write_account_defaults(fp, sh_num, folder_list",
    account_mkstemp
)
account_sync = account_writer.index("fsync(fileno(fp))", account_content)
account_close = account_writer.index("if (fclose(fp) != 0)", account_sync)
account_publish = account_writer.index("rename(temp_file, var_file)", account_close)
require(account_mkstemp < account_content < account_sync < account_close < account_publish,
        "account defaults must be fully written and closed before atomic publish")
account_failure = account_writer[account_writer.index("failed:"):]
require("unlink(temp_file)" in account_failure and
        "errno = saved_errno;" in account_failure,
        "failed account defaults writes must discard the temporary file")
require('fopen(var_file, "w")' not in account_writer,
        "account defaults must not truncate the last complete permission file")


modify_writer = function_body(
    DISK_SOURCE,
    "int modify_if_exist_new_folder(const char *const account,"
)
compact_modify = " ".join(modify_writer.split())
folder_lookup = compact_modify.index(
    "test_if_exist_folder_in_mount_path(mount_path, folder_list[i])"
)
lookup_error = compact_modify.index("if (result < 0)", folder_lookup)
lookup_missing = compact_modify.index("if (result == 0)", lookup_error)
modify_mkstemp = compact_modify.index("fd = mkstemp(temp_file)", lookup_missing)
modify_copy = compact_modify.index("fwrite(old_info, 1, old_len, fp)", modify_mkstemp)
modify_write = compact_modify.index(
    "disk_share_write_account_defaults(fp, missing_count", modify_copy
)
modify_sync = compact_modify.index("fsync(fileno(fp))", modify_write)
modify_close = compact_modify.index("if (fclose(fp) != 0)", modify_sync)
modify_rename = compact_modify.index("rename(temp_file, var_file)", modify_close)
require(folder_lookup < lookup_error < lookup_missing < modify_mkstemp <
        modify_copy < modify_write < modify_sync < modify_close < modify_rename,
        "new-folder permission updates must fail closed and atomically publish")
modify_failure = compact_modify[compact_modify.index("failed:"):]
require("unlink(temp_file)" in modify_failure and
        "errno = saved_errno;" in modify_failure,
        "failed new-folder updates must discard the temporary file")
require('fopen(var_file, "a+")' not in modify_writer,
        "new-folder updates must not append unchecked partial rows")


clear_body = function_body(DISK_SOURCE, "clear_share_var_files(const char *mount_path)")
clear_compact = " ".join(clear_body.split())
clear_read = clear_compact.index(
    "disk_share_list_next_entry(dir, readdir, &entry, &read_error)"
)
clear_eof = clear_compact.index("if (read_status == 0)", clear_read)
clear_error = clear_compact.index("if (read_status < 0)", clear_eof)
clear_unlink = clear_compact.index("if (unlink(path) != 0 && errno != ENOENT)", clear_error)
require(clear_read < clear_eof < clear_error < clear_unlink and
        "if (closedir(dir) != 0)" in clear_compact and
        "return -1;" in clear_compact,
        "managed metadata cleanup must fail closed on directory and unlink errors")
initial_all = function_body(
    DISK_SOURCE,
    "int initial_all_var_file_in_mount_path(const char *const mount_path)"
)
require("if (clear_share_var_files(mount_path) != 0)" in initial_all and
        "if (initial_one_var_file_in_mount_path(FTP_ANONYMOUS_USER," in initial_all and
        "if (initial_one_var_file_in_mount_path(account_list[i]," in initial_all and
        "if (initial_folder_list_in_mount_path(mount_path) != 0)" in initial_all,
        "full share metadata initialization must propagate every stage failure")
create_all = function_body(
    DISK_SOURCE, "int create_if_no_var_files(const char *const mount_path)"
)
require("path_len >= (int)sizeof(var_file)" in create_all and
        "result = modify_if_exist_new_folder" in create_all and
        "result = initial_one_var_file_in_mount_path" in create_all and
        "if (result != 0)" in create_all,
        "incremental account metadata initialization must check path and write results")


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
read_body = function_body(DISK_IO_SOURCE, "char *read_whole_file(const char *target)")
require("errno = EINVAL;" in read_body and
        "return read_whole_file_failure(fp, buffer, EFBIG);" in read_body and
        "if (fclose(fp) != 0)" in read_body and
        "saved_errno = errno ? errno : EIO;" in read_body,
        "read_whole_file must report invalid paths, oversized files, read and close failures")
for marker in (
    "test_reads_complete_text_file",
    "test_reads_empty_file",
    "test_rejects_invalid_and_missing_paths",
    "test_rejects_oversized_file_with_efbig",
    "test_reports_directory_read_error",
):
    require(marker in IO_REGRESSION and marker + "();" in IO_REGRESSION,
            "missing read_whole_file regression: " + marker)


get_permission = function_body(DISK_SOURCE, "int get_permission(const char *account,")
get_read = get_permission.index("var_info = read_whole_file(var_file);")
get_missing = get_permission.index("if (saved_errno == ENOENT)", get_read)
get_denied_error = get_permission.index("return -1;", get_missing)
require(get_read < get_missing < get_denied_error and
        "errno = saved_errno;" in get_permission[get_denied_error - 80:get_denied_error + 40],
        "permission query must preserve no-grant on ENOENT but report metadata I/O errors")
set_permission = function_body(DISK_SOURCE, "int set_permission(const char *account,")
set_read = set_permission.index("var_info = read_whole_file(var_file);")
set_enoent = set_permission.index("if (saved_errno != ENOENT)", set_read)
set_init = set_permission.index("initial_one_var_file_in_mount_path(account, mount_path)", set_enoent)
set_retry = set_permission.index("var_info = read_whole_file(var_file);", set_init)
require(set_read < set_enoent < set_init < set_retry and
        "sleep(1);" not in set_permission,
        "permission update may initialize defaults only for ENOENT and must check the result")
require("if (result != 0)" in set_permission[set_init:set_retry],
        "permission update must propagate metadata initialization failure")



account_list_body = function_body(
    DISK_SOURCE, "int get_account_list(int *acc_num, char ***account_list)"
)
account_list_compact = "".join(account_list_body.split())
require("disk_share_account_inventory_read(&ops,&transaction,MAX_ACCOUNT_NUM,acc_num,account_list)" in account_list_compact and
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

for marker in (
    "test_account_inventory_reads_one_checked_snapshot",
    "test_account_inventory_count_read_error_is_not_zero",
    "test_account_inventory_username_error_frees_partial_list",
    "test_account_inventory_transaction_end_error_discards_snapshot",
    "test_account_inventory_rejects_invalid_count_text_and_range",
    "test_account_inventory_rejects_sparse_and_reserved_names",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing checked account inventory regression: " + marker)

add_account_body = function_body(
    DISK_SOURCE, "int add_account(const char *const account, const char *const password)"
)
add_account_compact = "".join(add_account_body.split())
add_inventory = add_account_compact.index("read_disk_data_checked(&disk_list)")
add_inventory_error = add_account_compact.index(
    "first_error=errno?errno:EIO;", add_inventory
)
add_init_assignment = (
    "init_result=initial_one_var_file_in_mount_path("
    "account,follow_partition->mount_point);"
)
add_init = add_account_compact.index(add_init_assignment, add_inventory)
add_cleanup = add_account_compact.index("free_disk_data(disk_list)", add_init)
add_samba = add_account_compact.index('system("/sbin/run_samba")', add_cleanup)
add_error_return = add_account_compact.index("if(first_error!=0)", add_samba)
require(add_account_compact.count("read_disk_data_checked(&disk_list)") == 1 and
        "read_disk_data()" not in add_account_compact and
        add_inventory < add_inventory_error < add_init < add_cleanup < add_samba < add_error_return and
        "if(init_result!=0&&first_error==0)" in add_account_compact and
        "errno=first_error;" in add_account_compact and
        "return-1;" in add_account_compact[add_error_return:] and
        "return0;" not in add_account_compact[add_inventory:add_samba],
        "add_account must distinguish an empty inventory from a failed scan, keep processing accessible volumes, reload Samba even for empty/unknown inventory, and return the first inventory/metadata error")

del_account_body = function_body(
    DISK_SOURCE, "int del_account(const char *const account)"
)
del_account_compact = "".join(del_account_body.split())
saved_count = del_account_compact.index("account_count=acc_num;")
decrement_count = del_account_compact.index("--acc_num;")
free_inventory = del_account_compact.index(
    "free_2_dimension_list(&account_count,&account_list)", decrement_count
)
require(saved_count < decrement_count < free_inventory and
        "free_2_dimension_list(&acc_num,&account_list);" in
        del_account_compact[:decrement_count],
        "del_account must release the complete original account list after preserving its pre-decrement count")
del_inventory = del_account_compact.index("read_disk_data_checked(&disk_list)")
del_inventory_error = del_account_compact.index(
    "first_error=errno?errno:EIO;", del_inventory
)
del_path = del_account_compact.index(
    "disk_share_format_account_var_path(var_file,sizeof(var_file),",
    del_inventory
)
del_unlink = del_account_compact.index("unlink(var_file)", del_path)
require("read_disk_data()" not in del_account_compact and
        del_inventory < del_inventory_error < del_path < del_unlink and
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


mod_account_body = function_body(
    DISK_SOURCE,
    "int mod_account(const char *const account, const char *const new_account, const char *const new_password)"
)
mod_account_compact = "".join(mod_account_body.split())
mod_inventory = mod_account_compact.index("read_disk_data_checked(&disk_list)")
mod_loop = mod_account_compact.index("for(follow_disk=disk_list;", mod_inventory)
mod_old_path = mod_account_compact.index("disk_share_format_account_var_path(var_file,sizeof(var_file),", mod_loop)
mod_new_path = mod_account_compact.index("disk_share_format_account_var_path(new_var_file,sizeof(new_var_file),", mod_old_path)
mod_old_stat = mod_account_compact.index("lstat(var_file,&old_stat)", mod_new_path)
mod_target_stat = mod_account_compact.index("lstat(new_var_file,&new_stat)", mod_old_stat)
mod_rename = mod_account_compact.index("rename(var_file,new_var_file)", mod_target_stat)
mod_cleanup = mod_account_compact.index("free_disk_data(disk_list)", mod_rename)
mod_service = mod_account_compact.index('system("/sbin/run_ftpsamba")', mod_cleanup)
mod_error = mod_account_compact.index("if(first_error!=0)", mod_service)
require(mod_inventory < mod_loop < mod_old_path < mod_new_path < mod_old_stat <
        mod_target_stat < mod_rename < mod_cleanup < mod_service < mod_error,
        "mod_account must check inventory, bounded paths, source/target metadata and cleanup before reporting")
require("if(read_disk_data_checked(&disk_list)!=0)" in mod_account_compact and
        "first_error=errno?errno:EIO;" in mod_account_compact[mod_inventory:mod_loop],
        "mod_account must not treat incomplete storage inventory as empty")
require("S_ISREG(old_stat.st_mode)" in mod_account_body and
        "first_error=EEXIST;" in mod_account_compact[mod_target_stat:mod_rename],
        "mod_account must reject non-regular sources and pre-existing target metadata")
require("disk_share_account_rename_status(rename_result,rename_errno)" in mod_account_compact and
        "errno=first_error;" in mod_account_compact and
        "return-1;" in mod_account_compact[mod_error:],
        "mod_account must aggregate rename/inventory errors after cleanup")
require("snprintf(var_file" not in mod_account_body and
        "snprintf(new_var_file" not in mod_account_body,
        "mod_account must use bounded path formatter")
for marker in (
    "test_account_rename_status_accepts_success_and_missing_source",
    "test_account_rename_status_preserves_real_errors",
):
    require(marker in POLICY_TEST and marker + "();" in POLICY_TEST,
            "missing account rename result policy regression: " + marker)

add_folder_body = function_body(
    DISK_SOURCE,
    "int add_folder(const char *const mount_path, const char *const folder)"
)
add_compact = " ".join(add_folder_body.split())
add_lookup = add_compact.index(
    "test_if_exist_folder_in_mount_path(mount_path, folder)"
)
add_mkdir = add_compact.index("result = mkdir(full_path, 0777)", add_lookup)
add_publish = add_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", add_mkdir
)
add_permissions = add_compact.index(
    "create_if_no_var_files(mount_path)", add_publish
)
require(add_lookup < add_mkdir < add_publish < add_permissions and
        "errno = EEXIST;" in add_compact and
        "errno = ENAMETOOLONG;" in add_compact and
        "if (initial_folder_list_in_mount_path(mount_path) != 0)" in add_compact and
        "if (create_if_no_var_files(mount_path) != 0)" in add_compact,
        "add_folder must fail closed and propagate every metadata initialization stage")
require("get_account_list(" not in add_folder_body and
        "read_whole_file(" not in add_folder_body and
        'fopen(var_file, "a+")' not in add_folder_body,
        "add_folder must use the checked per-account metadata updater")


del_folder = function_body(
    DISK_SOURCE, "int del_folder(const char *const mount_path, const char *const folder)"
)
del_lookup = del_folder.index("test_if_exist_folder_in_mount_path(mount_path, folder)")
del_error = del_folder.index("if (result < 0)", del_lookup)
del_remove = del_folder.index("if (rmdir(full_path) != 0)", del_error)
del_snapshot = del_folder.index("initial_folder_list_in_mount_path(mount_path)", del_remove)
require(del_lookup < del_error < del_remove < del_snapshot,
        "folder deletion must validate the share-list and remove the directory before publishing the new snapshot")

for signature, expected_update in (
    ("int del_folder(const char *const mount_path, const char *const folder)",
     "disk_share_update_folder_permission_file(mount_path, account_list[i], target, NULL)"),
    ("int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)",
     "disk_share_update_folder_permission_file(mount_path, account_list[i], target, new_target)"),
):
    mutation = function_body(DISK_SOURCE, signature)
    account_call = "get_account_list(&acc_num, &account_list)"
    require(account_call in mutation,
            signature + " must load the complete account inventory")
    assigned_load = "result = " + account_call + ";"
    direct_guard = "if (" + account_call + " != 0)"
    guarded_load = (
        direct_guard in mutation or
        (assigned_load in mutation and
         mutation.index("if (result != 0)", mutation.index(assigned_load)) <
         mutation.index("for (i = 0; i < acc_num", mutation.index(assigned_load)))
    )
    require(guarded_load,
            signature + " must stop when account inventory loading fails")
    compact_mutation = "".join(mutation.split())
    compact_expected_update = "".join(expected_update.split())
    require(compact_expected_update in compact_mutation and
            "first_error==0" in compact_mutation and
            "if(first_error!=0)" in compact_mutation and
            "FTP_ANONYMOUS_USER" in mutation,
            signature + " must attempt all independent account files and preserve the first error")
    require("initial_folder_list_in_mount_path(mount_path) != 0" in mutation,
            signature + " must check the folder-list snapshot publication")

permission_update = function_body(
    DISK_SOURCE, "static int\ndisk_share_update_folder_permission_file("
)
update_compact = " ".join(permission_update.split())
update_path = update_compact.index(
    "disk_share_format_account_var_path(var_file, sizeof(var_file)"
)
update_read = update_compact.index("read_whole_file(var_file)", update_path)
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
atomic_replace = function_body(DISK_SOURCE, "disk_share_replace_file_atomic(const char *path,")
require("have_existing = 1;" in atomic_replace and
        "if (have_existing && fchmod(fd, existing_stat.st_mode & 0777) != 0)" in atomic_replace,
        "atomic replacement must not read an uninitialized stat result for a new metadata file")
require("mkstemp(temp_path)" in atomic_replace and
        "fdopen(fd, \"w\")" in atomic_replace and
        "fwrite(contents, 1, contents_len" in atomic_replace and
        "fflush(stream)" in atomic_replace and
        "fsync(fileno(stream))" in atomic_replace and
        "fclose(stream)" in atomic_replace and
        "rename(temp_path, path)" in atomic_replace and
        "unlink(temp_path)" in atomic_replace,
        "per-account permission replacement must check write, flush, sync, close, rename and cleanup")

set_permission = function_body(DISK_SOURCE, "int set_permission(const char *account,")
require("disk_share_replace_file_atomic(" in set_permission and
        "disk_share_format_account_var_path" in set_permission and
        'fopen(var_file, "w")' not in set_permission and
        'fopen(var_file, "a+")' not in set_permission,
        "set_permission must use checked path construction and atomic publication")

for signature in (
    "int del_folder(const char *const mount_path, const char *const folder)",
    "int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)",
):
    mutation = function_body(DISK_SOURCE, signature)
    require("disk_share_update_folder_permission_file(" in mutation and
            'fopen(var_file, "w")' not in mutation and
            'fopen(var_file, "a+")' not in mutation and
            "disk_share_format_account_var_path" in mutation,
            signature + " must use the checked atomic account updater and reject truncated paths")

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
