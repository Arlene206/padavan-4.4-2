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
    DISK_SOURCE, "create_if_no_var_files(const char *const mount_path)"
)
require("get_account_list(&acc_num, &account_list)" in create_all and
        "disk_share_account_nvram_begin(&transaction)" in create_all and
        "disk_share_account_inventory_matches_locked(" in create_all and
        "create_if_no_var_files_with_accounts(mount_path, acc_num, account_list)" in create_all and
        "disk_share_account_nvram_end(&transaction)" in create_all,
        "public incremental metadata initialization must serialize its owned account snapshot through permission-file publication")
create_with_accounts = function_body(
    DISK_SOURCE, "create_if_no_var_files_with_accounts("
)
create_with_accounts_compact = "".join(create_with_accounts.split())
require("get_account_list(" not in create_with_accounts and
        "path_len>=(int)sizeof(var_file)" in create_with_accounts_compact and
        "result=modify_if_exist_new_folder" in create_with_accounts_compact and
        "result=initial_one_var_file_in_mount_path" in create_with_accounts_compact and
        "if(result!=0)" in create_with_accounts_compact,
        "incremental folder metadata worker must reuse a checked account snapshot and propagate every bounded path/write failure")




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
add_init = add_account_compact.index("initial_one_var_file_in_mount_path(", add_validate)
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

create_with_accounts = function_body(
    DISK_SOURCE,
    "static int\\ncreate_if_no_var_files_with_accounts("
)
create_with_accounts_compact = "".join(create_with_accounts.split())
require("get_account_list(" not in create_with_accounts and
        "initial_one_var_file_in_mount_path(" in create_with_accounts and
        "modify_if_exist_new_folder(" in create_with_accounts,
        "folder metadata initialization under a writer lease must reuse the caller's account snapshot without nested NVRAM reads")

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
        add_publish < add_permissions < add_success_end < add_rollback <
        add_permission_rollback < add_anonymous_rollback < add_snapshot_restore <
        add_failed_end and
        "errno=EEXIST;" in add_compact and
        "errno=ENAMETOOLONG;" in add_compact and
        "create_if_no_var_files(mount_path)" not in add_folder_body,
        "add_folder must serialize with account mutations and undo partial permission rows, directory creation and folder snapshot on failure")


del_folder = function_body(
    DISK_SOURCE, "int del_folder(const char *const mount_path, const char *const folder)"
)
del_inventory = del_folder.index("get_account_list(&acc_num, &account_list)")
del_begin = del_folder.index("disk_share_account_nvram_begin(&transaction)", del_inventory)
del_validate = del_folder.index(
    "disk_share_account_inventory_matches_locked(&disk_share_account_nvram_operations,&transaction,acc_num,MAX_ACCOUNT_NUM,account_list)",
    del_begin
)
del_lookup = del_folder.index("test_if_exist_folder_in_mount_path(mount_path, folder)", del_validate)
del_error = del_folder.index("if (result < 0)", del_lookup)
del_remove = del_folder.index("if (rmdir(full_path) != 0)", del_error)
del_snapshot = del_folder.index("initial_folder_list_in_mount_path(mount_path)", del_remove)
del_permissions = del_folder.index(
    "disk_share_update_folder_permission_file(mount_path, account_list[i], target, NULL)",
    del_snapshot
)
del_end = del_folder.index("disk_share_account_nvram_end(&transaction)", del_permissions)
require(del_inventory < del_begin < del_validate < del_lookup < del_error <
        del_remove < del_snapshot < del_permissions < del_end and
        "get_account_list(" not in del_folder[del_begin:],
        "folder deletion must revalidate its account snapshot under one writer lease and hold it through directory and per-account permission cleanup")

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
mod_live_check = mod_folder_compact.index("test_if_dir(full_path)", mod_validate)
mod_refresh = mod_folder_compact.index("initial_folder_list_in_mount_path(mount_path)", mod_live_check)
mod_rename = mod_folder_compact.index("rename(full_path,new_full_path)", mod_refresh)
mod_permissions = mod_folder_compact.index(
    "disk_share_update_folder_permission_file(mount_path,account_list[i],target,new_target)",
    mod_rename
)
mod_release = mod_folder_compact.index("disk_share_account_nvram_end(&transaction)", mod_permissions)
require(mod_accounts < mod_begin < mod_validate < mod_live_check < mod_refresh <
        mod_rename < mod_permissions < mod_release and
        "get_account_list(" not in mod_folder_compact[mod_begin:] and
        "add_folder(mount_path,folder)" not in mod_folder_compact,
        "folder rename must revalidate the account snapshot and all live folder state under one writer lease through permission-file publication")

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

mod_folder_body = function_body(
    DISK_SOURCE,
    "int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)"
)
mod_folder_compact = "".join(mod_folder_body.split())
mod_source_exists = mod_folder_compact.index("if(test_if_dir(full_path)!=1)")
mod_snapshot_lookup = mod_folder_compact.index(
    "test_if_exist_folder_in_mount_path(mount_path,folder)", mod_source_exists
)
mod_stale_refresh = mod_folder_compact.index(
    "initial_folder_list_in_mount_path(mount_path)", mod_snapshot_lookup
)
mod_rename_folder = mod_folder_compact.index("rename(full_path,new_full_path)", mod_stale_refresh)
require(mod_source_exists < mod_snapshot_lookup < mod_stale_refresh < mod_rename_folder and
        "add_folder(mount_path,folder)" not in mod_folder_compact[mod_snapshot_lookup:mod_rename_folder],
        "mod_folder must refresh a stale folder-list snapshot from the existing directory instead of trying to mkdir an already-existing source")

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
