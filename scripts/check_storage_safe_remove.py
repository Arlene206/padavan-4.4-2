#!/usr/bin/env python3
"""Source integration checks for fail-closed storage eject before persistence."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def body(source, signature):
    start = source.find(signature)
    while start >= 0:
        signature_end = start + len(signature)
        brace = source.find("{", signature_end)
        semicolon = source.find(";", signature_end)
        if brace >= 0 and (semicolon < 0 or brace < semicolon):
            break
        start = source.find(signature, semicolon + 1) if semicolon >= 0 else -1
    require(start >= 0, "missing function definition: " + signature)
    brace = source.find("{", start + len(signature))
    require(brace >= 0, "missing function body: " + signature)
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 1
            elif char == "/" and nxt == "/":
                state = "line"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:index]
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state == "line" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


# A forward declaration must not bind the checker to the next function body.
_probe = "static int probe(void);\nstatic int probe(void) { return 1; }\n"
require("return 1;" in body(_probe, "probe(void)"),
        "function extractor must skip a forward declaration")


libdisk = read("trunk/user/libdisk/src/disk_initial.c")
disk_policy = read("trunk/user/libdisk/src/disk_inventory_policy.h")
disk_policy_test = read("trunk/user/libdisk/src/tests/disk_inventory_policy.c")
swap_policy = read("trunk/user/libdisk/src/storage_swap_policy.h")
swap_policy_test = read("trunk/user/rc/src/tests/storage_swap_policy.c")
disk_header = read("trunk/user/libdisk/src/disk_initial.h")
storage = read("trunk/user/rc/src/services_stor.c")
ejected_policy = read("trunk/user/rc/src/storage_ejected_inventory_policy.h")
ejected_policy_test = read("trunk/user/rc/src/tests/storage_ejected_inventory_policy.c")

# Disk inventory must not authorize safe removal from a partial /proc/swaps
# header or from malformed rows belonging to another device.
swapon_probe = body(libdisk, "static int\nis_device_swapon(")
swap_match = body(swap_policy, "storage_swap_line_matches_device(")
require("storage_swap_header_valid(line)" in swapon_probe and
        "storage_swap_line_matches_device(line, dev_path)" in swapon_probe and
        "STORAGE_SWAP_UNKNOWN" in swapon_probe,
        "disk inventory must preserve UNKNOWN swap state for failed procfs reads")
require("storage_swap_parse_swap_line" in swap_match and
        "storage_swap_line_is_complete" in swap_policy and
        '"Filename", "Type", "Size", "Used", "Priority"' in swap_policy,
        "swap inventory must validate the complete header and every five-column row")

mount_link_policy = read("trunk/user/rc/src/storage_nfs_export_policy.h")
mount_link_policy_test = read("trunk/user/rc/src/tests/storage_nfs_export_policy.c")
services = read("trunk/user/rc/src/services.c")
services_usb = read("trunk/user/rc/src/services_usb.c")
usb_name_policy = read("trunk/user/rc/src/storage_hotplug_name_policy.h")
usb_name_policy_test = read("trunk/user/rc/src/tests/storage_hotplug_name_policy.c")
hotplug_stor = read("trunk/user/rc/src/hotplug_stor.c")
notify_policy = read("trunk/user/rc/src/storage_hotplug_notify_policy.h")
notify_policy_test = read("trunk/user/rc/src/tests/storage_hotplug_notify_policy.c")
net = read("trunk/user/rc/src/net.c")
rc = read("trunk/user/rc/src/rc.c")
rc_header = read("trunk/user/rc/src/rc.h")
common_ex_source = read("trunk/user/rc/src/common_ex.c")
optware_policy = read("trunk/user/rc/src/storage_optware_start_policy.h")
optware_policy_test = read("trunk/user/rc/src/tests/storage_optware_start_policy.c")
policy = read("trunk/user/rc/src/storage_service_stop_policy.h")
ftp_policy = read("trunk/user/rc/src/ftp_daemon_policy.h")
ftp_test = read("trunk/user/rc/src/tests/ftp_daemon_policy.c")
daemon_wait_policy = read("trunk/user/rc/src/service_daemon_wait_policy.h")
workflow = read(".github/workflows/storage-hotplug-stability.yml")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")
mtd_storage = read("trunk/user/scripts/files/sbin/mtd_storage.sh")
httpd_header = read("trunk/user/httpd/src/httpd.h")
httpd_upload = read("trunk/user/httpd/src/upload.c")
httpd_web_ex = read("trunk/user/httpd/src/web_ex.c")
mtd_storage_lock_tests = tuple(
    read("trunk/user/scripts/tests/" + name)
    for name in (
        "mtd_storage_save_retry.sh",
        "mtd_storage_restore_transaction.sh",
        "mtd_storage_load_transaction.sh",
        "mtd_storage_erase_reset.sh",
        "mtd_storage_backup_failure.sh",
    )
)
persistence_policy = read("trunk/user/rc/src/storage_persistence_policy.h")
persistence_test = read("trunk/user/rc/src/tests/storage_persistence_policy.c")
mtd_storage_test = read("trunk/user/scripts/tests/mtd_storage_save_retry.sh")
mtd_storage_restore_test = read("trunk/user/scripts/tests/mtd_storage_restore_transaction.sh")
mtd_storage_load_test = read("trunk/user/scripts/tests/mtd_storage_load_transaction.sh")
mtd_storage_backup_test = read("trunk/user/scripts/tests/mtd_storage_backup_failure.sh")
mtd_storage_erase_reset_test = read("trunk/user/scripts/tests/mtd_storage_erase_reset.sh")

checked = body(libdisk, "read_disk_data_checked(disk_info_t **disk_info_out)")
require("if (!disk_info_out)" in checked, "checked inventory must validate its output")
require("fopen(PROC_PARTITIONS_FILE" in checked and "ferror(fp)" in checked and "fclose(fp)" in checked,
        "checked inventory must detect open/read/close failures")
require("if (!create_disk(" in checked and "if (!create_partition(" in checked,
        "allocation/partition decode failures must not publish partial inventory")
require("*disk_info_out = disk_info_list;" in checked and "return 0;" in checked,
        "checked inventory must distinguish valid empty inventory from failure")
require("disk_inventory_parse_header_line(line)" in checked and
        "disk_inventory_line_is_blank(line)" in checked and
        "disk_inventory_parse_partition_line(" in checked,
        "checked inventory must reject malformed headers, rows and truncation")
require("disk_inventory_parse_decimal" in disk_policy and "UINT_MAX" in disk_policy and "ULLONG_MAX" in disk_policy,
        "procfs numeric fields must be strict decimal values with overflow checks")
require("1844674407370955161ULL" in disk_policy and "/ 10ULL" not in disk_policy and
        "/ 10UL" not in disk_policy,
        "strict disk parsing must avoid division so MIPS libdisk cannot import __udivdi3")
require(" -8 0 10 sda" in disk_policy_test and "18446744073709551616" in disk_policy_test and "ULLONG_MAX" in disk_policy_test,
        "parser tests must reject signed numbers and integer overflow")
legacy = body(libdisk, "read_disk_data(void)")
require("read_disk_data_checked(&disk_info_list)" in legacy and "return disk_info_list;" in legacy,
        "legacy read_disk_data API must remain a compatibility wrapper")
require("read_disk_data_checked" in disk_header and "read_disk_data(void)" in disk_header,
        "both checked and legacy inventory APIs must be exported")

# Mount data is part of a disk's safety inventory, not an optional UI hint.
# An unreadable or malformed /proc/mounts snapshot must never collapse to
# "partition is not mounted", especially for a whole-device filesystem.
mountpoint_lookup = body(libdisk, "has_dev_mountpoint(const char *mount_path)")
require("storage_inventory_parse_mount_line_full(" in mountpoint_lookup and
        "ret >= 0 && ferror(fp)" in mountpoint_lookup and "close_status = fclose(fp)" in mountpoint_lookup and
        "if (ret < 0)" in mountpoint_lookup and "return -1;" in mountpoint_lookup and
        mountpoint_lookup.count("break;") == 1 and
        "sscanf(" not in mountpoint_lookup,
        "pool mountpoint lookup must decode procfs paths and fail closed on read/parse/close errors")
mount_path_adapter = body(libdisk, "get_mount_path(")
require("mount_status = has_dev_mountpoint(tmppath);" in mount_path_adapter and
        "if (mount_status != 1)" in mount_path_adapter and
        "*mount_path = NULL;" in mount_path_adapter and
        "errno = ENOENT;" in mount_path_adapter,
        "mount path resolver must distinguish missing mountpoints from unknown inventory")

mount_reader = body(libdisk, "get_partition_mount_data(partition_info_t *part_info)")
require("storage_inventory_parse_mount_line_full(" in mount_reader and
        "ferror(fp)" in mount_reader and "close_status = fclose(fp)" in mount_reader and
        "if (ret < 0)" in mount_reader and
        "errno = saved_errno ? saved_errno : EIO;" in mount_reader,
        "partition mount inspection must validate the entire procfs snapshot and close result")
require("if (!fp)" in mount_reader and "return -1;" in mount_reader,
        "unreadable mount inventory must be UNKNOWN, never an empty mount list")
disk_creator = body(libdisk, "create_disk(const char *dev_name")
require("errno = 0;" in disk_creator and "if (!create_partition(" in disk_creator and
        "errno != 0" in disk_creator and "errno = saved_errno;" in disk_creator,
        "whole-device mount inventory failures must propagate without rejecting a genuinely unmounted disk")
require("storage_inventory_parse_mount_line_full" in swap_policy and
        "filesystem_size" in swap_policy and "options_size" in swap_policy,
        "mount parser must expose checked filesystem/options fields to metadata readers")
require("three-digit octal escapes" in swap_policy and
        "decoded > 255U" in swap_policy and "decoded == 0" in swap_policy,
        "procfs mount/swap paths must decode octal escapes and reject invalid path bytes")
require(r"/media/USB\\040Disk" in swap_policy_test and
        r"/media/USB\\011Disk" in swap_policy_test and
        r"/media/USB\\000Disk" in swap_policy_test and
        'strcmp(mountpoint, "/media/USB Disk")' in swap_policy_test and
        r'strcmp(mountpoint, "/media/USB\tDisk")' in swap_policy_test,
        "procfs path decoding regression must cover spaces, tabs, buffer bounds and NUL escapes")
require("rw,relatime" in swap_policy_test and '"ro"' in swap_policy_test and
        "storage_inventory_parse_mount_line_full" in swap_policy_test,
        "storage mount parser regressions must cover metadata, read-only and malformed records")

count = body(storage, "count_stor_mountpoint(void)")
require("storage_stage_unmount_mounts(&mounts)" in count and
        "free_storage_unmount_mounts(mounts)" in count,
        "final mount count must use the complete parsed mount inventory")

# A physical unplug notification is delivered after /proc/partitions no
# longer lists the device. Stop dependent daemons before unmounting stale
# paths, then restore them only when other storage mounts remain.
ejected_mount_parser = body(ejected_policy, "storage_ejected_parse_mount_line(")
require("three-digit octal escapes" in ejected_policy and
        "decoded > 255U" in ejected_policy and "decoded == 0" in ejected_policy and
        "output_length + 1 >= token_size" in ejected_policy,
        "ejected-device mount and swap inventory must decode escaped paths with strict bounds")
require(r"/media/USB\\040Disk" in ejected_policy_test and
        r"/media/USB\\134Disk" in ejected_policy_test and
        r"/media/USB\\011Disk" in ejected_policy_test and
        r"/media/USB\\000Disk" in ejected_policy_test and
        'strcmp(mountpoint, "/media/USB Disk")' in ejected_policy_test and
        r'strcmp(mountpoint, "/media/USB\\Disk")' in ejected_policy_test and
        r'strcmp(mountpoint, "/media/USB\tDisk")' in ejected_policy_test,
        "ejected inventory tests must cover decoded space/backslash/tab and invalid escaped bytes")
require("storage_ejected_next_token" in ejected_mount_parser,
        "ejected mount parser must use the octal-decoding token reader")

ejected = body(
    storage,
    "umount_ejected_locked(int storage_apps_stopped, int restart_storage_apps)",
)
ejected_swap_stage = body(storage, "storage_stage_ejected_swaps(")
require(
    'char path[256];' in storage
    and 'strcpy(entry->path, swap_path);' in ejected_swap_stage
    and 'strncmp(swap_path, "/dev/", 5) == 0' in ejected_swap_stage
    and 'storage_ejected_swap_file_is_on_ejected_mount' in storage
    and 'storage_ejected_swap_path_is_on_mount(swap_path, mount->mountpoint)' in storage
    and "swapoff(swap_entry->path)" in storage,
    "physical eject must retain absolute swap-file paths and disable them when backed by an ejected mount",
)
require(
    'storage_ejected_swap_path_is_on_mount(\n\t\t"/media/usb/swapfile", "/media/usb")' in ejected_policy_test
    and '"/media/usb2/swapfile", "/media/usb"' in ejected_policy_test
    and '"/media/usb/../internal/swapfile", "/media/usb"' in ejected_policy_test,
    "swap-file path matching must enforce mount-component boundaries and reject traversal",
)
ejected_inventory = ejected.index("storage_stage_ejected_partitions(&partitions)")
ejected_scan = ejected.index("has_ejected_mounts = 1;")
ejected_stop = ejected.index("stop_stor_apps()")
ejected_hook = ejected.index('eval("/usr/bin/opt-umount.sh"')
require(
    ejected_inventory < ejected_scan < ejected_stop < ejected_hook,
    "physical eject must stage both inventories and stop storage daemons before Optware teardown/unmount",
)
ejected_rollback = ejected.index("storage_rollback_stor_apps_after_stop_failure(")
ejected_abort = ejected.index("return -1;", ejected_rollback)
require(
    ejected_rollback < ejected_abort < ejected_hook,
    "failed physical-eject service stop must roll back and leave mounts untouched",
)
require(
    "if (has_ejected_mounts && !storage_apps_stopped)" in ejected
    and "if (has_ejected_mounts && restart_storage_apps)" in ejected,
    "ejected cleanup must avoid double-stop/restart when called by the already-locked safe-remove path",
)
incomplete_cleanup_gate = ejected.index(
    "if (result != 0)",
    ejected.index("free_storage_ejected_partitions(partitions)"),
)
remaining = ejected.index("mount_count = count_stor_mountpoint()")
restart = ejected.index("start_status = start_stor_apps()", remaining)
require(
    incomplete_cleanup_gate < remaining < restart
    and "leaving storage applications stopped because physical eject cleanup was incomplete" in ejected[incomplete_cleanup_gate:remaining]
    and "usb_hotplug_ms" not in ejected[incomplete_cleanup_gate:remaining]
    and ejected.index("if (mount_count > 0)", remaining) < restart
    and "else if (mount_count == 0)" in ejected[remaining:]
    and "else {" in ejected[remaining:]
    and 'nvram_set_int_temp("usb_hotplug_ms", 1);' in ejected[remaining:],
    "physical eject must keep apps stopped and retry incomplete stale unmounts, then restart only for remaining mounts",
)
retry_scheduler = body(storage, "storage_schedule_ejected_cleanup_retry(void)")
require(
    'nvram_set_int_temp("usb_unplug_ms", 1);' in retry_scheduler
    and "alarm(5);" in retry_scheduler,
    "physical unplug cleanup must schedule a distinct unmount retry",
)
ejected_public = body(storage, "umount_ejected(void)")
require(
    "umount_ejected_locked(0, 1)" in ejected_public
    and ejected_public.count("storage_schedule_ejected_cleanup_retry();") == 2
    and ejected_public.index("storage_schedule_ejected_cleanup_retry();")
        < ejected_public.index("return -1;")
    and ejected_public.index("file_unlock(storage_lock);")
        < ejected_public.index("if (result != 0)")
    and ejected_public.index("if (result != 0)")
        < ejected_public.index("storage_schedule_ejected_cleanup_retry();",
                               ejected_public.index("if (result != 0)")),
    "physical unplug wrapper must retry lock and cleanup failures after releasing its lock",
)
deferred_storage = body(storage, "on_deferred_hotplug_dev(void)")
unplug_retry = deferred_storage.index('nvram_match("usb_unplug_ms", "1")')
unplug_clear = deferred_storage.index('nvram_set_int_temp("usb_unplug_ms", 0);')
unplug_call = deferred_storage.index("storage_unplug_status = umount_ejected();")
storage_start = deferred_storage.index("storage_start_result = try_start_stor_apps();")
require(
    unplug_retry < unplug_clear < unplug_call < storage_start
    and '!nvram_match("usb_unplug_ms", "1")' in deferred_storage
    and deferred_storage.index('!nvram_match("usb_unplug_ms", "1")') < storage_start,
    "deferred worker must complete physical unmount retries before starting storage applications",
)
require(
    "usb_hotplug_ms" not in ejected[incomplete_cleanup_gate:remaining]
    and 'nvram_set_int_temp("usb_hotplug_ms", 1);' in ejected[remaining:],
    "only application startup failures may use the storage-start retry flag",
)
targeted_eject = body(storage, "safe_remove_stor_device(")
targeted_eject_main = body(rc, "int\nmain(int argc, char **argv)")
require(
    "int result = -1;" in targeted_eject
    and "read_disk_data_checked(&disks_info)" in targeted_eject
    and "has_requested_disk = (dev_name == NULL);" in targeted_eject
    and "storage_ejected_disk_matches_request(dev_name, follow_disk->device)" in targeted_eject
    and "saved_errno = ENODEV;" in targeted_eject
    and "storage_ejected_disk_matches_request" in ejected_policy
    and 'storage_ejected_disk_matches_request("sda", "sdb")' in ejected_policy_test
    and "storage_ejected_target_remove_complete" in targeted_eject
    and "errno = saved_errno;" in targeted_eject
    and "return -1;" in targeted_eject
    and "int safe_remove_stor_device(" in rc_header
    and "ret = safe_remove_stor_device(ATA_VIRT_PORT_ID" in targeted_eject_main
    and "ret = safe_remove_stor_device(MMC_VIRT_PORT_ID" in targeted_eject_main
    and "storage_ejected_target_remove_complete(0, 0, 3, 0)" in ejected_policy_test
    and "storage_ejected_target_remove_complete(0, 0, 3, -1)" in ejected_policy_test,
    "ATA/MMC safe-remove applets must return checked inventory, cleanup and restart outcomes",
)
require(
    "storage_ejected_target_remove_complete" in ejected_policy
    and "remaining_mounts >= 0" in ejected_policy
    and "(remaining_mounts == 0 || start_status == 0)" in ejected_policy,
    "targeted eject completion must require a known remaining-mount inventory and successful app restart",
)
usb_remove = body(services_usb, "int\nsafe_remove_usb_device(")
usb_main = body(rc, "int\nmain(int argc, char **argv)")
require(
    "storage_hotplug_parse_decimal_token(dev_name, 1, INT_MAX" in usb_remove
    and "return safe_remove_stor_device(port, port, dev_name, 1);" in usb_remove
    and "return safe_remove_stor_device(1, BOARD_NUM_USB_PORTS, dev_name, 1);" in usb_remove
    and "if (!has_modem_port || !get_usb_modem_dev_wan(0, modem_devnum))" in usb_remove
    and "int safe_remove_usb_device(int port, const char *dev_name);" in rc_header,
    "USB eject must reject malformed or absent modem targets and propagate storage/modem removal outcomes",
)
usb_remove_contract = " ".join(usb_remove.split())
reconnect_call = usb_remove_contract.index("wan_reconnect_status = try_wan_reconnect(0, 0);")
reconnect_gate = usb_remove_contract.index("if (wan_reconnect_status != 0)", reconnect_call)
reconnect_failure_return = usb_remove_contract.index("return -1;", reconnect_gate)
reconnect_success_return = usb_remove_contract.index("return 0;", reconnect_failure_return)
require(
    "errno = 0; wan_reconnect_status = try_wan_reconnect(0, 0);" in usb_remove_contract
    and "if (wan_reconnect_status > 0) errno = wan_reconnect_status;" in usb_remove_contract
    and "else if (!errno) errno = EIO;" in usb_remove_contract
    and reconnect_call < reconnect_gate < reconnect_failure_return < reconnect_success_return
    and "USB modem was removed but WAN reconnect failed" in usb_remove_contract,
    "USB modem removal must propagate WAN stop/start/reconnect failures instead of reporting success",
)
require(
    "storage_hotplug_parse_decimal_token(" in usb_main
    and "0, BOARD_NUM_USB_PORTS," in usb_main
    and "ret = safe_remove_usb_device(port, devn);" in usb_main
    and "ret = (argc > 2) ? EINVAL : safe_remove_usb_device(1, devn);" in usb_main
    and "ret = (argc > 2) ? EINVAL : safe_remove_usb_device(2, devn);" in usb_main,
    "USB eject applets must validate complete port arguments and return safe-remove status",
)
require(
    "if (digit > maximum || value > (maximum - digit) / 10)" in usb_name_policy
    and "storage_hotplug_parse_decimal_token" in usb_name_policy_test
    and 'storage_hotplug_parse_decimal_token("1x", 0, 2, &value)' in usb_name_policy_test
    and 'storage_hotplug_parse_decimal_token("4294967296", 0, 4294967295U, &value)' in usb_name_policy_test,
    "USB eject numeric parser must reject suffixes and overflow without modifying the result",
)
require(
    "umount_ejected_locked(1, 0)" in targeted_eject
    and "if (storage_ejected_may_restart_apps(unmount_status, ejected_status, mount_count))" in targeted_eject
    and "static int\nstorage_ejected_may_restart_apps" in ejected_policy
    and "storage_ejected_may_restart_apps(0, 0, 3)" in ejected_policy_test
    and "storage_ejected_may_restart_apps(-1, 0, 3)" in ejected_policy_test
    and "storage_ejected_may_restart_apps(0, -1, 3)" in ejected_policy_test
    and "storage_ejected_may_restart_apps(0, 0, -1)" in ejected_policy_test,
    "targeted safe-remove must not restart apps after either unmount stage fails or inventory is unknown",
)

eject = body(storage, "safe_remove_all_stor_devices(int do_spindown)")
require("read_disk_data_checked(&disks_info)" in eject and
        eject.index("read_disk_data_checked(&disks_info)") < eject.index("stop_stor_apps()"),
        "obtain a complete inventory before stopping services")
require("path_status = disks_info ? umount_stor_path(disks_info, 0, NULL, do_spindown) : 0" in eject,
        "a valid empty inventory must still reach generic mount cleanup")
require("storage_operation_lock_acquire(file_lock)" in eject and "stop_stor_apps()" in eject,
        "eject must retain operation lock and storage-service stop preflight")
require("read_disk_data_checked(&disks_info)" in eject,
        "eject must refuse an unknown /proc/partitions snapshot")
require(eject.index("read_disk_data_checked(&disks_info)") < eject.index("umount_stor_path("),
        "eject must not unmount before the complete disk inventory")
require("umount_all_storage()" in eject and "count_stor_mountpoint()" in eject,
        "eject must verify fallback result and final mount inventory")
require("storage_safe_remove_complete(path_status, fallback_status, mount_count)" in eject,
        "eject must use the fail-closed postcondition")
require("return result;" in eject, "eject must return operation status")
rollback = body(storage, "storage_rollback_stor_apps_after_stop_failure(")
require("saved_errno = errno ? errno : EIO;" in rollback and
        "start_status = start_stor_apps();" in rollback and
        "storage_service_stop_status_merge(stop_status, start_status)" in rollback and
        "errno = saved_errno;" in rollback,
        "failed storage-stop preflight must restore already-stopped peers and preserve the original error")
targeted_eject = body(storage, "safe_remove_stor_device(")
targeted_eject_contract = " ".join(targeted_eject.split())
targeted_rollback = targeted_eject.index("storage_rollback_stor_apps_after_stop_failure(")
targeted_unlock = targeted_eject.index("file_unlock(storage_lock);", targeted_rollback)
require('storage_rollback_stor_apps_after_stop_failure( "targeted safe removal", stop_status)' in targeted_eject_contract and
        targeted_rollback < targeted_unlock,
        "targeted eject must roll back stopped storage services before releasing the operation lock")
eject_contract = " ".join(eject.split())
eject_rollback = eject.index("storage_rollback_stor_apps_after_stop_failure(")
eject_abort = eject.index("goto out;", eject_rollback)
require('storage_rollback_stor_apps_after_stop_failure( "eject-all", stop_status)' in eject_contract and
        eject_rollback < eject_abort,
        "eject-all must restore stopped storage services before aborting the operation")
require("storage_ejected_storage_start_retry_required( mount_count, start_status)" in targeted_eject_contract and
        "storage_ejected_may_restart_apps(unmount_status, ejected_status, mount_count)" in targeted_eject_contract and
        "storage_ejected_storage_start_retry_required( mount_count, 0)" not in targeted_eject_contract,
        "targeted eject must retry failed app starts after successful cleanup, but never restart from unknown/failed cleanup")
retry_schedule = targeted_eject.rsplit("if (retry_storage_start)", 1)[-1]
require('nvram_set_int_temp("usb_hotplug_ms", 1);' in retry_schedule and "alarm(5);" in retry_schedule,
        "targeted eject must re-arm the existing five-second deferred storage retry")
require(targeted_eject.rfind("file_unlock(storage_lock);") < targeted_eject.rfind("if (retry_storage_start)"),
        "deferred storage retry must be scheduled only after releasing the storage operation lock")

require("int safe_remove_all_stor_devices(int do_spindown);" in rc_header,
        "RC header must expose the checked eject result")
require('ret = safe_remove_all_stor_devices(1);' in rc,
        "ejall must propagate a failed eject status to its caller")
complete = body(policy, "storage_safe_remove_complete(")
require("path_status != 0" in complete and "fallback_status != 0" in complete and "remaining_mounts != 0" in complete,
        "safe eject must reject unmount failures and residual/unknown mounts")
require("storage_service_process_presence_changed" in policy and
        "before < 0 || after < 0" in policy,
        "unknown daemon state must force conservative firewall reconciliation")
require("storage_service_process_presence_changed" in storage,
        "FTP, Transmission and Aria firewall refresh must use tri-state process state")
presence = body(storage, "storage_daemon_presence(const char *daemon, int *scan_error)")
require("process_inventory_scan(daemon, &snapshot)" in presence and
        "process_snapshot_release(&snapshot);" in presence and
        "PROCESS_PRESENCE_UNKNOWN" in presence,
        "storage daemon query must preserve process-inventory UNKNOWN")
for daemon, signature in (
    ("vsftpd", "is_ftp_run(void)"),
    ("minidlna", "int is_dms_run(void)"),
    ("mt-daapd", "int is_itunes_run(void)"),
    ("transmission-daemon", "int is_torrent_run(void)"),
    ("aria2c", "int is_aria_run(void)"),
):
    state_fn = body(storage, signature)
    require('storage_daemon_presence("' + daemon + '", NULL)' in state_fn,
            "storage daemon query must use tri-state inventory: " + daemon)
stop_body = body(storage, "stop_dms(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_dms must verify daemon absence after stop")
stop_body = body(storage, "stop_itunes(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_itunes must verify daemon absence after stop")
for signature, daemon, suffix, label in (
        ("static int stop_torrent_locked(void)", "transmission-daemon", "torrent", "Transmission"),
        ("static int stop_aria_locked(void)", "aria2c", "aria", "Aria")):
    locked_stop = body(storage, signature)
    require("enum process_presence process_state;" in locked_stop and
            "process_state = is_{}_run();".format(suffix) in locked_stop,
            "{} stop must preserve tri-state preflight".format(label))
    require("process_state == PROCESS_PRESENCE_ABSENT" in locked_stop and
            "process_state != PROCESS_PRESENCE_RUNNING" in locked_stop and
            "refusing name-wide stop because daemon presence is unknown" in locked_stop,
            "{} stop must not signal on UNKNOWN presence".format(label))
    expected_svcs = 'char *svcs[] = { "' + daemon + '", NULL };'
    require(expected_svcs in locked_stop and
            "command_status = kill_services(svcs," in locked_stop and
            "storage_service_stop_complete(command_status, is_{}_run())".format(suffix) in locked_stop,
            "{} stop must use identity-validated process lifecycle transaction".format(label))
    legacy_script = "transmission.sh" if suffix == "torrent" else "aria.sh"
    require('eval("/usr/bin/{}"'.format(legacy_script) not in locked_stop,
            "{} stop must not call legacy pidof/killall shell path".format(label))

stop_body = body(storage, "stop_torrent(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_torrent must verify daemon absence after stop")
stop_body = body(storage, "stop_aria(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_aria must verify daemon absence after stop")
ftp_stop = body(storage, "stop_ftp_locked(void)")
require("storage_service_stop_complete(status, is_ftp_run())" in ftp_stop,
        "FTP stop must verify daemon absence before eject")
ftp_start = body(storage, "run_ftp(void)")
require("process_state != PROCESS_PRESENCE_ABSENT" in ftp_start and
        "PROCESS_PRESENCE_RUNNING" in ftp_start,
        "FTP start must refuse unknown daemon presence")
for signature, daemon, stop_callback in (
        ("int run_dms(int force_rescan)", "minidlna", "stop_dms_locked"),
        ("int run_itunes(void)", "mt-daapd", "stop_itunes_locked"),
        ("int run_torrent(void)", "transmission-daemon", "stop_torrent_locked"),
        ("int run_aria(void)", "aria2c", "stop_aria_locked"),
):
    run_body = body(storage, signature)
    require("process_state != PROCESS_PRESENCE_ABSENT" in run_body and
            "PROCESS_PRESENCE_RUNNING" in run_body,
            "storage daemon start must fail closed on unknown presence: " + signature)
    require("launch_errno = errno;" in run_body and
            "start_status = storage_log_daemon_start_result(" in run_body and
            "if (start_status != 0)" in run_body and
            "storage_rollback_failed_start(" in run_body and
            stop_callback in run_body,
            "fresh " + daemon + " start must roll back when launch/readiness postconditions fail")

rollback = body(storage, "static int\nstorage_rollback_failed_start(")
require("stop_status = stop_daemon();" in rollback and
        "process_inventory_scan(daemon, &snapshot)" in rollback and
        "storage_service_failed_start_may_leave_daemon(presence)" in rollback and
        "errno = start_errno > 0 ? start_errno : EIO;" in rollback,
        "failed fresh-start rollback must stop, re-inventory and preserve the original error")
require("PROCESS_PRESENCE_UNKNOWN" in daemon_wait_policy and
        "return -1;" in daemon_wait_policy and "EIO" in daemon_wait_policy,
        "generic detached-daemon wait must fail immediately on unknown process state")
require("PROCESS_PRESENCE_UNKNOWN" in ftp_test and "EACCES" in ftp_test,
        "FTP wait regression must cover process-inventory failures")

require("storage_service_process_presence_changed" in policy and
        "before < 0 || after < 0" in policy,
        "unknown daemon state must force conservative firewall reconciliation")
require("storage_service_process_presence_changed" in storage,
        "FTP, Transmission and Aria firewall refresh must use tri-state presence")
presence = body(storage, "storage_daemon_presence(const char *daemon, int *scan_error)")
require("process_inventory_scan(daemon, &snapshot)" in presence and
        "process_snapshot_release(&snapshot);" in presence and
        "PROCESS_PRESENCE_UNKNOWN" in presence,
        "storage daemon query must preserve process-inventory UNKNOWN")
for daemon, signature in (
    ("vsftpd", "is_ftp_run(void)"),
    ("minidlna", "int is_dms_run(void)"),
    ("mt-daapd", "int is_itunes_run(void)"),
    ("transmission-daemon", "int is_torrent_run(void)"),
    ("aria2c", "int is_aria_run(void)"),
):
    state_fn = body(storage, signature)
    require('storage_daemon_presence("' + daemon + '", NULL)' in state_fn,
            "storage daemon query must use tri-state inventory: " + daemon)
ftp_stop = body(storage, "stop_ftp_locked(void)")
require("storage_service_stop_complete(status, is_ftp_run())" in ftp_stop,
        "FTP stop must verify daemon absence before eject")
ftp_start = body(storage, "run_ftp(void)")
require("process_state != PROCESS_PRESENCE_ABSENT" in ftp_start and
        "PROCESS_PRESENCE_RUNNING" in ftp_start,
        "FTP start must refuse unknown daemon presence")
for signature in ("int run_dms(int force_rescan)", "int run_itunes(void)",
                  "int run_torrent(void)", "int run_aria(void)"):
    run_body = body(storage, signature)
    require("process_state != PROCESS_PRESENCE_ABSENT" in run_body and
            "PROCESS_PRESENCE_RUNNING" in run_body,
            "storage daemon start must fail closed on unknown presence: " + signature)

# Media/download helpers preserve launch/readiness failures and attempt a
# process-identity-checked rollback before returning a failed start.
for signature, daemon, stopper in (
    ("int run_dms(int force_rescan)", "minidlna", "stop_dms_locked"),
    ("int run_itunes(void)", "mt-daapd", "stop_itunes_locked"),
    ("int run_torrent(void)", "transmission-daemon", "stop_torrent_locked"),
    ("int run_aria(void)", "aria2c", "stop_aria_locked"),
):
    run_body = body(storage, signature)
    require("start_status = storage_log_daemon_start_result(" in run_body and
            "if (start_status != 0)" in run_body and
            "start_errno = launch_status != 0 ?" in run_body and
            "return storage_rollback_failed_start(" in run_body and
            'apps_name, "' + daemon + '", start_status, start_errno,' in run_body and
            stopper in run_body and
            "return errno ? errno : EIO;" in run_body,
            "storage daemon must return launch/readiness failures with rollback: " + signature)
    require(signature + ";" in rc_header,
            "status-bearing daemon prototype missing: " + signature)

rollback_body = body(storage, "storage_rollback_failed_start(")
require("stop_status = stop_daemon();" in rollback_body and
        "process_inventory_scan(daemon, &snapshot)" in rollback_body and
        "storage_service_failed_start_may_leave_daemon(presence)" in rollback_body and
        "return start_status;" in rollback_body and
        "errno = start_errno > 0 ? start_errno : EIO;" in rollback_body,
        "failed-start rollback must verify daemon identity and preserve the primary error")

daemon_result = body(storage, "storage_log_daemon_start_result(")
require("status = rc_first_service_start_status(status, ENOENT);" in daemon_result and
        "status = rc_first_service_start_status(status, process_status);" in daemon_result and
        "return status;" in daemon_result,
        "daemon readiness helper must return absent and unknown process failures")

storage_start = body(storage, "start_stor_apps(void)")
require(storage_start.count("rc_first_service_start_status(") == 8 and
        all(call in storage_start for call in (
            "service_status = run_ftp();",
            "service_status = run_samba();",
            "service_status = run_nfsd_locked();",
            "service_status = run_dms(0);",
            "service_status = run_itunes();",
            "service_status = run_torrent();",
            "service_status = run_aria();",
            "firewall_status = restart_firewall();",
            "start_status = rc_first_service_start_status(start_status, firewall_status);",
        )) and
        storage_start.index("firewall_status = restart_firewall();") <
        storage_start.index("return start_status;"),
        "all seven storage service statuses and firewall refresh result must be aggregated")

require("PROCESS_PRESENCE_UNKNOWN" in daemon_wait_policy and
        "return -1;" in daemon_wait_policy and "EIO" in daemon_wait_policy,
        "generic detached-daemon wait must fail immediately on unknown process state")
require("PROCESS_PRESENCE_UNKNOWN" in ftp_test and "EACCES" in ftp_test,
        "FTP wait regression must cover process-inventory failures")


# Printer daemon stop results must reach the firmware flash gate.
for declaration in ("int stop_u2ec(void);", "int stop_lpd(void);",
                    "int stop_p910nd(void);", "int stop_usb_printer_spoolers(void);"):
    require(declaration in rc_header, "RC header must expose checked printer stop API: " + declaration)
for signature in ("int\nstop_u2ec(void)", "int\nstop_lpd(void)", "int\nstop_p910nd(void)"):
    stop_fn = body(services_usb, signature)
    require("return kill_services(" in stop_fn,
            "printer stop must propagate kill_services result: " + signature)
printer_stop = body(services_usb, "int\nstop_usb_printer_spoolers(void)")
for daemon in ("u2ec", "lpd", "p910nd"):
    require('"' + daemon + '"' in printer_stop,
            "printer stop aggregate must include " + daemon)
require("record_usb_printer_stop_status" in printer_stop and "return stop_status;" in printer_stop,
        "printer group must aggregate every stop result")
for signature in ("restart_usb_printer_spoolers(void)", "try_start_usb_printer_spoolers(void)"):
    starter = body(services_usb, signature)
    require("int stop_status = stop_usb_printer_spoolers();" in starter and
            "if (stop_status != 0)" in starter and
            "return stop_status;" in starter and "return exec_printer_daemons(" in starter,
            "printer restart must propagate stop failure and abort launch: " + signature)
    require(starter.index("if (stop_status != 0)") <
            starter.index("return exec_printer_daemons("),
            "printer restart must not launch after a failed stop: " + signature)
stop_all = body(services, "int\nstop_services(int stopall)")
for daemon in ("p910nd", "lpd", "u2ec", "IPTV proxies",
               "scutclient", "mentohust", "vlmcsd", "iperf3",
               "AdGuardHome", "Shadowsocks", "Shadowsocks tunnel",
               "Adbyby", "DDNSTO", "SQM", "AliyunDrive WebDAV",
               "WireGuard", "SmartDNS", "ZeroTier", "AliDDNS", "ttyd", "FRP"):
    require('record_service_stop_status(&stop_status, "' + daemon + '"' in stop_all,
            "stop_services must aggregate stop status: " + daemon)

optional_identity_processes = {
    "scutclient": "bin_scutclient",
    "mentohust": "bin_mentohust",
    "vlmcsd": "vlmcsd",
    "iperf3": "iperf3",
}
for name, script in (
    ("scutclient", "/usr/bin/scutclient.sh"),
    ("mentohust", "/usr/bin/mentohust.sh"),
    ("ttyd", "/usr/bin/ttyd.sh"),
    ("ss", "/usr/bin/shadowsocks.sh"),
    ("ss_tunnel", "/usr/bin/ss-tunnel.sh"),
    ("vlmcsd", "/usr/bin/vlmcsd.sh"),
    ("iperf3", "/usr/bin/iperf3.sh"),
    ("adguardhome", "/usr/bin/adguardhome.sh"),
    ("ddnsto", "/usr/bin/ddnsto.sh"),
    ("sqm", "/usr/lib/sqm/run.sh"),
    ("aldriver", "/usr/bin/aliyundrive-webdav.sh"),
    ("wireguard", "/usr/bin/wireguard.sh"),
    ("adbyby", "/usr/bin/adbyby.sh"),
    ("smartdns", "/usr/bin/smartdns.sh"),
    ("aliddns", "/usr/bin/aliddns.sh"),
):
    stop_body = body(services, "int stop_" + name + "(void)")
    if name in optional_identity_processes:
        normalized = " ".join(stop_body.split())
        expected = ('return optional_daemon_stop_status("{0}", "{1}", '
                    'eval("{2}", "stop"));').format(
                        name, optional_identity_processes[name], script)
        require(expected in normalized,
                "identity-managed stop must preserve command status and verify daemon absence: " + name)
    else:
        require('return eval("' + script + '","stop");' in stop_body,
                "optional service stop must propagate its shell status: " + name)
    require("int stop_" + name + "(void);" in rc_header,
            "RC header must expose checked stop status: " + name)

optional_policy = read("trunk/user/rc/src/optional_daemon_policy.h")
optional_presence = body(services, "optional_daemon_presence(const char *process_name, int *scan_error)")
require("process_inventory_scan(process_name, &snapshot)" in optional_presence and
        "process_snapshot_release(&snapshot)" in optional_presence and
        "PROCESS_PRESENCE_UNKNOWN" in optional_presence,
        "optional daemon inventory must preserve UNKNOWN and release its snapshot")
optional_stop = body(services, "optional_daemon_stop_status(const char *service_name, const char *process_name,")
require("optional_daemon_presence(process_name, &scan_error)" in optional_stop and
        "optional_daemon_stop_complete(command_status, presence)" in optional_stop,
        "optional daemon stop must verify tri-state process absence after the command")
stop_policy = body(optional_policy, "optional_daemon_stop_complete(int command_status, enum process_presence state)")
require("if (command_status != 0)" in stop_policy and
        "return command_status;" in stop_policy and
        "state == PROCESS_PRESENCE_ABSENT ? 0 : -1" in stop_policy,
        "optional daemon stop policy must preserve command errors and require confirmed ABSENT")

zerotier_stop = body(services, "int stop_zerotier(void)")
require('return eval("/usr/bin/zerotier.sh","stop");' in zerotier_stop and
        "int stop_zerotier(void);" in rc_header,
        "ZeroTier stop must propagate launcher status")
zerotier_restart = " ".join(body(services, "int restart_zerotier(void)").split())
require("int stop_status = stop_zerotier();" in zerotier_restart and
        "if (stop_status != 0)" in zerotier_restart and
        "return stop_status;" in zerotier_restart and
        zerotier_restart.count("return start_zerotier();") == 1 and
        zerotier_restart.index("return stop_status;") < zerotier_restart.index("return start_zerotier();"),
        "ZeroTier restart must gate replacement start and propagate both stop and start statuses")
require('record_service_stop_status(&stop_status, "ZeroTier", stop_zerotier())' in stop_all,
        "stop_services must aggregate ZeroTier stop status")

frp_stop = body(services, "int stop_frp(void)")
require("return status;" in frp_stop and 'eval("/usr/bin/frp.sh","stop")' in frp_stop,
        "FRP stop must propagate the launcher status")
require("int stop_frp(void);" in rc_header,
        "RC header must expose checked FRP stop status")
for name in ("scutclient", "mentohust", "ttyd", "ss", "ss_tunnel",
             "vlmcsd", "iperf3", "sqm", "aldriver", "wireguard"):
    restart_body = body(services, "int restart_" + name + "(void)")
    require("int stop_status = stop_" + name + "();" in restart_body and "return stop_status;" in restart_body and "start_" + name + "();" in restart_body,
            "manual stop/start wrapper must not launch after failed stop: " + name)

# SmartDNS holds one shell-level lock across its entire restart transaction;
# the C wrapper must not split that transaction into separate stop/start calls.
smartdns_restart = body(services, "int restart_smartdns(void)")
require('return eval("/usr/bin/smartdns.sh","restart");' in smartdns_restart and
        "stop_smartdns()" not in smartdns_restart and
        "return start_smartdns();" not in smartdns_restart,
        "SmartDNS restart must delegate the full locked transaction to its shell wrapper")
require(stop_all.count('record_service_stop_status(&stop_status, "WireGuard"') == 1,
        "stop_services must only stop WireGuard once")
igmp_stop = body(net, "int\nstop_igmpproxy(char *wan_ifname)")
require("return 0;" in igmp_stop and "return kill_services(" in igmp_stop,
        "IPTV proxy stop must report success only for intentional no-op or verified stop")
igmp_start = body(net, "int\nstart_igmpproxy(char *wan_ifname)")
require("return stop_status;" in igmp_start and
        igmp_start.index("return stop_status;") < igmp_start.index("start_udpxy(wan_ifname)") and
        "rc_first_service_start_status(start_status, start_udpxy(wan_ifname))" in igmp_start and
        "rc_first_service_start_status(start_status, start_xupnpd(wan_ifname))" in igmp_start and
        "rc_first_service_start_status(start_status," in igmp_start and
        'eval("/bin/igmpproxy", igmpproxy_conf)' in igmp_start,
        "IPTV start must propagate stop and every enabled proxy launch status")
iptv_restart = body(net, "int\nrestart_iptv(int is_ap_mode)")
require("operation_status = config_bridge(is_ap_mode);" in iptv_restart and
        "operation_status = start_igmpproxy(wan_ifname);" in iptv_restart and
        "return rc_first_service_start_status(start_status, operation_status);" in iptv_restart and
        "stop_status = stop_igmpproxy(NULL);" in iptv_restart and
        "return rc_first_service_start_status(start_status, stop_status);" in iptv_restart and
        iptv_restart.index("return rc_first_service_start_status(start_status, stop_status);") <
        iptv_restart.index("start_udpxy(IFNAME_BR)") and
        "start_status = rc_first_service_start_status(start_status, start_udpxy(IFNAME_BR));" in iptv_restart and
        "start_status = rc_first_service_start_status(start_status, start_xupnpd(IFNAME_BR));" in iptv_restart and
        "return start_status;" in iptv_restart,
        "IPTV restart must preserve bridge errors, proxy stop gates and enabled launch outcomes")
require("int restart_iptv(int is_ap_mode);" in rc_header and
        "int start_igmpproxy(char *wan_ifname);" in rc_header,
        "IPTV start/restart declarations must return checked status")
iptv_dispatch = body(rc, 'else if (!strcmp(entry->d_name, RCN_RESTART_IPTV))')
require("rc_notify_merge_action_result(action_result, restart_iptv(is_ap_mode))" in iptv_dispatch,
        "PID 1 IPTV notification must return restart status to queue")
require("int stop_igmpproxy(char *wan_ifname);" in rc_header,
        "RC header must export checked IPTV stop status")

flash = body(rc, "flash_firmware(void)")
require("stop_usb_printer_spoolers() != 0" in flash and "stop_igmpproxy(NULL) != 0" in flash,
        "firmware write must be blocked by failed printer or IPTV proxy stop")
flash_gate = flash.index("if (safe_remove_all_stor_devices(0) != 0)")
require(flash_gate < flash.index("storage_save_time(") < flash.index("write_storage_to_mtd(") <
        flash.index('eval("/tmp/mtd_write"'),
        "firmware persistence and flash must follow successful storage eject")
shutdown = body(rc, "shutdown_router(int level)")
require("misc_stop_status = stop_misc();" in shutdown and
        "service_stop_status = stop_services(use_halt);" in shutdown and
        "printer_stop_status = stop_usb_printer_spoolers();" in shutdown and
        "wan_stop_status = stop_wan();" in shutdown and
        "lan_wan_stop_status = stop_services_lan_wan();" in shutdown,
        "shutdown must collect all checked service-stop results")
require("service shutdown incomplete; continuing system power-down" in shutdown and
        "LAN/WAN service shutdown incomplete; continuing power-down without storage persistence" in shutdown,
        "safe reboot/halt must proceed even when daemon stop fails")
shutdown_gate = shutdown.index("storage_remove_status = safe_remove_all_stor_devices(use_halt)")
persist_gate = shutdown.index("if (storage_remove_status == 0 && misc_stop_status == 0")
require("wan_stop_status == 0" in shutdown[persist_gate:],
        "shutdown must not persist storage when WAN stop failed")
require(shutdown_gate < persist_gate < shutdown.index("storage_save_time(") <
        shutdown.index("write_storage_to_mtd("),
        "shutdown must power down but gate storage persistence on all service-stop/eject results")
rwfs_unmount = body(common_ex_source, "int umount_rwfs_partition(void)")
require("int umount_rwfs_partition(void);" in rc_header,
        "RWFS unmount must expose a checked status")
require("opt-umount.sh" in rwfs_unmount and "return EIO;" in rwfs_unmount,
        "Optware stop-hook failure must propagate")
require("umount(mp_rwfs)" in rwfs_unmount and "return operation_status;" in rwfs_unmount,
        "RWFS filesystem unmount failure must propagate")
require("rwfs_ubi_attachment_presence(mtd_rwfs, &ubi_before)" in rwfs_unmount and
        "rwfs_ubi_attachment_presence(mtd_rwfs, &ubi_after)" in rwfs_unmount,
        "UBI attachment must be verified before and after detach")
require("storage_optware_ubi_detach_complete(ubi_before, ubi_after)" in rwfs_unmount,
        "UBI detach must require confirmed absence")
ubi_presence = body(common_ex_source, "rwfs_ubi_attachment_presence(int mtd_rwfs, int *presence)")
require('"/sys/class/ubi/ubi%d"' in ubi_presence and '"%s/mtd_num"' in ubi_presence and
        'fscanf(fp, "%d", &attached_mtd)' in ubi_presence,
        "UBI identity must be read from its kernel mtd_num attribute")
require("storage_optware_ubi_detach_complete" in optware_policy and
        "STORAGE_OPTWARE_STOP_UNKNOWN" in optware_policy_test,
        "UBI detach policy must have regression coverage")
require("rwfs_unmount_status = umount_rwfs_partition();" in shutdown and
        "rwfs_unmount_status == 0" in shutdown,
        "shutdown must gate MTD persistence on RWFS teardown")
rwfs_call = shutdown.index("rwfs_unmount_status = umount_rwfs_partition();")
require(rwfs_call < persist_gate < shutdown.index("storage_save_time(") <
        shutdown.index("write_storage_to_mtd("),
        "RWFS teardown must finish before MTD persistence")
require(shutdown.count("umount_rwfs_partition(") == 1,
        "shutdown must not unmount RWFS after MTD persistence")

push_at = workflow.index("  push:\n    paths:\n")
pr_at = workflow.index("  pull_request:\n    paths:\n")
jobs_at = workflow.index("\n\njobs:")
push_filters = workflow[push_at:pr_at]
pull_filters = workflow[pr_at:jobs_at]
for path in ("trunk/user/libdisk/src/disk_initial.c",
             "trunk/user/libdisk/src/disk_initial.h",
             "trunk/user/libdisk/src/disk_inventory_policy.h",
             "trunk/user/libdisk/src/storage_swap_policy.h",
             "trunk/user/libdisk/src/tests/disk_inventory_policy.c",
             "trunk/user/rc/src/tests/storage_swap_policy.c",
             "scripts/check_storage_safe_remove.py",
             "trunk/user/rc/src/optional_daemon_policy.h",
             "trunk/user/rc/src/service_daemon_wait_policy.h",
             "trunk/user/shared/src/process_inventory.h"):
    require('"' + path + '"' in push_filters, "storage push filter missing " + path)
    require('"' + path + '"' in pull_filters, "storage pull_request filter missing " + path)
for path in ("trunk/user/libdisk/src/disk_initial.c",
             "trunk/user/libdisk/src/disk_initial.h",
             "trunk/user/libdisk/src/disk_inventory_policy.h",
             "trunk/user/libdisk/src/storage_swap_policy.h",
             "trunk/user/rc/src/storage_ejected_inventory_policy.h",
             "trunk/user/rc/src/tests/storage_ejected_inventory_policy.c",
             "trunk/user/rc/src/storage_hotplug_name_policy.h",
             "trunk/user/rc/src/tests/storage_hotplug_name_policy.c",
             "trunk/user/rc/src/storage_service_stop_policy.h",
             "trunk/user/rc/src/storage_optware_start_policy.h",
             "trunk/user/rc/src/optional_daemon_policy.h",
             "trunk/user/rc/src/service_daemon_wait_policy.h"):
    require('"' + path + '"' in xy_workflow, "XY-C1 target build does not watch " + path)


# A failed flash must not advance the MD5 checkpoint and suppress identical retries.
mtd_save_marker = "\nfunc_save()\n"
mtd_save_start = mtd_storage.index(mtd_save_marker) + 1
mtd_save_end = mtd_storage.index("\nfunc_backup()", mtd_save_start)
mtd_save = mtd_storage[mtd_save_start:mtd_save_end]
require('md5sum "$tmp" > "$hash_tmp"' in mtd_save and
        'mv -f "$hash_tmp" "$hsh"' in mtd_save,
        "MTD save must stage its content hash")
require(mtd_save.index("if mtd_write write") <
        mtd_save.index('mv -f "$hash_tmp" "$hsh"'),
        "MTD hash checkpoint must be published only after flash success")
require("MTD write succeeded but storage hash checkpoint failed" in mtd_save and
        'rm -f "$tmp" "$tbz" "$hash_tmp"' in mtd_save,
        "MTD save must surface checkpoint failures and clean scratch files")
require("MTD_WRITE_FAIL=1" in mtd_storage_test and
        "expected three MTD attempts" in mtd_storage_test and
        "unchanged save unexpectedly flashed again" in mtd_storage_test,
        "MTD retry test must cover write failure, identical retry and unchanged skip")

# Boot-time MTD load must reject incomplete archives before publishing.
mtd_load_start = mtd_storage.index("func_load()")
mtd_load_end = mtd_storage.index("\nfunc_tarb()", mtd_load_start)
mtd_load = mtd_storage[mtd_load_start:mtd_load_end]
for token in ('if ! bzcat "$mtd_part_dev" > "$tmp" 2>/dev/null; then',
              'if ! tar xf "$tmp" -C "$load_storage" 2>/dev/null; then',
              'if ! md5sum "$tmp" > "$hash_tmp"; then',
              'mv "$dir_storage" "$backup_storage"',
              'mv "$load_storage" "$dir_storage"',
              'mv "$backup_storage" "$dir_storage"',
              'if ! mv -f "$hash_tmp" "$hsh"; then',
              'if ! rm -f "$slk"; then'):
    require(token in mtd_load, "MTD load misses " + token)
lock_cleanup_check = 'if ! rm -f "$slk"; then'
require(mtd_load.count(lock_cleanup_check) == 3,
        "MTD load must check zero-byte decompression failure, empty stream and successful-load cleanup")
require(mtd_load.index('if ! bzcat') < mtd_load.index('if ! tar xf') <
        mtd_load.index('if ! md5sum') < mtd_load.index('mv "$dir_storage" "$backup_storage"') <
        mtd_load.index('mv "$load_storage" "$dir_storage"') <
        mtd_load.index('if ! mv -f "$hash_tmp" "$hsh"') <
        mtd_load.rindex('if ! rm -f "$slk"; then'),
        "MTD load must validate and stage the full tree before publication")
require('tar xf "$tmp" -C "$dir_storage"' not in mtd_load and
        'rm -rf "$dir_storage"' not in mtd_load,
        "MTD load must never extract over or delete live config")

# If both restore publication and rollback fail, keep rstats stopped unless
# a valid active storage tree is still present.
restore_start = mtd_storage.index("func_restore()")
restore_end = mtd_storage.index("\nfunc_erase()", restore_start)
restore_body = mtd_storage[restore_start:restore_end]
require('if ! touch "$slk"; then' in restore_body and
        restore_body.index('if ! touch "$slk"; then') < restore_body.index('stat -c %s "$rtbz"') <
        restore_body.index("func_stop_apps"),
        "restore must establish the save barrier before validating or replacing its archive")
require('rtbz="/tmp/.httpd-private/storage-restore.tar.bz2"' in mtd_storage and
        'stat -c %s "$rtbz"' in restore_body and
        'tar xjf "$rtbz"' in restore_body and
        '"$tbz"' not in restore_body,
        "MTD restore must consume a dedicated upload archive separate from backup output")
require('#define STORAGE_RESTORE_FIFO_FILENAME\t"/tmp/.httpd-private/storage-restore.tar.bz2"' in httpd_header and
        'const char *upload_file = STORAGE_RESTORE_FIFO_FILENAME;' in httpd_upload and
        'sizeof(STORAGE_RESTORE_FIFO_FILENAME)' in httpd_upload and
        'const char *upload_file = STORAGE_RESTORE_FIFO_FILENAME;' in httpd_web_ex,
        "HTTPD restore upload and CGI must use the dedicated restore archive")
backup_handler_start = httpd_web_ex.index("do_storage_file(const char *url, FILE *stream)")
backup_handler_end = httpd_web_ex.index(chr(10) + "static void" + chr(10) + "do_syslog_file", backup_handler_start)
backup_handler = httpd_web_ex[backup_handler_start:backup_handler_end]
require('char *storage_file = STORAGE_FIFO_FILENAME;' in backup_handler and
        'STORAGE_RESTORE_FIFO_FILENAME' not in backup_handler,
        "HTTPD backup download must retain its independent backup archive path")
require("existing backup archive" in mtd_storage_restore_test and
        "grep -Fqx" in mtd_storage_restore_test and
        "storage.tar.bz2" in mtd_storage_restore_test,
        "restore regression must preserve an independent existing backup archive")
require("RESTORE_FAIL_TOUCH_PATH" in mtd_storage_restore_test and
        "restore lock creation failure unexpectedly succeeded" in mtd_storage_restore_test,
        "restore transaction regression must reject save-lock creation failure without mutation")
require('test -f "$sandbox/private/storage_locked"' in mtd_storage_restore_test and
        'if sh "$sandbox/mtd_storage.sh" save; then' in mtd_storage_restore_test and
        "MTD save ignored the restore save barrier" in mtd_storage_restore_test,
        "restore transaction regression must prove that its barrier blocks MTD save after staging failure")
start_apps_start = mtd_storage.index("func_start_apps()")
start_apps_end = mtd_storage.index("\nfunc_load()", start_apps_start)
start_apps = mtd_storage[start_apps_start:start_apps_end]
require('/sbin/rstats 9>&-' in start_apps and
        'RSTAT_DAEMON_TEST=1' in mtd_storage_restore_test and
        'flock -n 8' in mtd_storage_restore_test and
        'rstats daemon inherited the MTD operation lock' in mtd_storage_restore_test,
        "rstats must not retain the MTD operation lock after restore restarts the daemon")
restore_start_guard = 'if [ -d "$dir_storage" ] && [ ! -L "$dir_storage" ] && [ -f "$dir_storage/start_script.sh" ]; then'
require(restore_body.count(restore_start_guard) == 2 and
        restore_body.count('rstats remains stopped because no storage tree is available') == 2,
        "restore failure paths must only restart rstats when an active storage tree is available")
require('RESTORE_FAIL_MV_AT="2,3" run_restore' in mtd_storage_restore_test and
        "double restore rename failure unexpectedly succeeded" in mtd_storage_restore_test and
        'test ! -s "$RSTAT_LOG"' in mtd_storage_restore_test and
        'grep -Fqx \'old=preserve-me\' "$1/original.conf"' in mtd_storage_restore_test,
        "restore transaction regression must preserve rollback data and keep rstats stopped after double rename failure")

# func_erase() creates the one-shot marker to stop runtime repopulating flash.
# At next boot only zero-byte output proves the partition is blank and allows
# the lock to be released; nonempty failed decompression remains fail-closed.
bzcat_failure = mtd_load.split('if ! bzcat "$mtd_part_dev" > "$tmp" 2>/dev/null; then', 1)[1].split(chr(10) + "\tfi", 1)[0]
require('if [ "$fsz" = "0" ]; then' in bzcat_failure and
        'if ! rm -f "$hsh"; then' in bzcat_failure and
        'elif ! rm -f "$slk"; then' in bzcat_failure,
        "zero-byte failed decompression must remove the stale hash before releasing the one-shot lock")
empty_size_branch = mtd_load.split('if [ "$fsz" -le 0 ]; then', 1)[1].split(chr(10) + "\tfi", 1)[0]
require('if ! rm -f "$hsh"; then' in empty_size_branch and
        'elif ! rm -f "$slk"; then' in empty_size_branch,
        "successful empty decompression must remove the stale hash before releasing the one-shot lock")
# Destructive erase/reset must preserve a save barrier across uncertain outcomes.
erase_start = mtd_storage.index("func_erase()")
erase_end = mtd_storage.index("\nfunc_reset()", erase_start)
erase_body = mtd_storage[erase_start:erase_end]
for token in ('if ! touch "$slk"; then',
              'if ! mtd_write erase "$mtd_part_name"; then',
              'if ! rm -f "$hsh"; then',
              'if ! rm -rf "$dir_storage"; then',
              'if ! mkdir -p -m 755 "$dir_storage"; then'):
    require(token in erase_body, "MTD erase postcondition check misses " + token)
require(erase_body.index('if ! touch "$slk"; then') <
        erase_body.index('if ! func_stop_apps; then') <
        erase_body.index('if ! mtd_write erase "$mtd_part_name"; then') <
        erase_body.index('if ! rm -f "$hsh"; then') <
        erase_body.index('if ! rm -rf "$dir_storage"; then') <
        erase_body.index('if ! mkdir -p -m 755 "$dir_storage"; then'),
        "MTD erase must lock before flash and check hash/directory cleanup in order")
require("MTD erase failed; storage save lock retained" in erase_body,
        "uncertain or partial MTD erase must retain the save lock")

reset_start = mtd_storage.index("func_reset()")
reset_end = mtd_storage.index("\nfunc_fill()", reset_start)
reset_body = mtd_storage[reset_start:reset_end]
for token in ('if ! touch "$slk"; then',
              'if ! rm -f "$hsh"; then',
              'if ! rm -rf "$dir_storage"; then',
              'if ! mkdir -p -m 755 "$dir_storage"; then'):
    require(token in reset_body, "storage reset postcondition check misses " + token)
require(reset_body.index('if ! touch "$slk"; then') <
        reset_body.index('if ! rm -f "$hsh"; then') <
        reset_body.index('if ! rm -rf "$dir_storage"; then') <
        reset_body.index('if ! mkdir -p -m 755 "$dir_storage"; then'),
        "storage reset must retain the save barrier while replacing its directory")
reset_case_start = mtd_storage.index("\nreset)\n") + 1
reset_case_end = mtd_storage.index("\n\t;;", reset_case_start)
reset_dispatch = mtd_storage[reset_case_start:reset_case_end]
require("if func_reset && func_fill; then" in reset_dispatch and
        reset_dispatch.index('if ! rm -f "$slk"; then') <
        reset_dispatch.index("func_start_apps"),
        "reset dispatch must fill defaults before unlocking and starting applications")
require("Reset preparation failed; storage save lock retained" in reset_dispatch,
        "failed reset preparation must not continue into default fill or app startup")
for item in ("MTD_WRITE_STATUS", "EVENT_LOG", "erase-stop-failure", "FAIL_TOUCH_PATH", "FAIL_RM_PATH",
             "FAIL_MKDIR_PATH", "FILL_FAIL", "reset-lock-release-failure",
             "erase-directory-recreation-failure", "STOP_APPS_FAIL",
             "reset-stop-failure", 'grep -Fqx stop "$STOP_LOG"'):
    require(item in mtd_storage_erase_reset_test,
            "erase/reset fixture lacks injection case: " + item)
require('sh trunk/user/scripts/tests/mtd_storage_erase_reset.sh' in workflow and
        'BUSYBOX_BIN="$busybox_bin" sh trunk/user/scripts/tests/mtd_storage_erase_reset.sh' in workflow,
        "erase/reset fixture must run under host shell and firmware BusyBox ash")
require('"trunk/user/scripts/tests/mtd_storage_erase_reset.sh"' in push_filters and
        '"trunk/user/scripts/tests/mtd_storage_erase_reset.sh"' in pull_filters,
        "storage erase/reset test must be covered by push and pull_request path filters")

require("BZCAT_STATUS=2" in mtd_storage_load_test and
        "BZCAT_EMPTY=1" in mtd_storage_load_test and
        "empty successful decompression unexpectedly succeeded" in mtd_storage_load_test and
        "FAIL_RM_PATH" in mtd_storage_load_test and "REAL_RM" in mtd_storage_load_test and
        "zero-byte MTD hash-removal failure unexpectedly succeeded" in mtd_storage_load_test and
        "empty successful MTD hash-removal failure unexpectedly succeeded" in mtd_storage_load_test and
        "TAR_FAIL=1" in mtd_storage_load_test and
        "MV_FAIL_AT=2" in mtd_storage_load_test and
        "partial-extract.conf" in mtd_storage_load_test and
        'if [ -n "${BUSYBOX_BIN:-}" ]; then' in mtd_storage_load_test and
        '"$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" load' in mtd_storage_load_test and
        'BUSYBOX_BIN="$busybox_bin" sh trunk/user/scripts/tests/mtd_storage_load_transaction.sh' in workflow,
        "MTD load fixture must cover erased MTD, corruption, extraction, rollback and BusyBox ash")

tar_builder_start = mtd_storage.index("func_tarb()")
tar_builder_end = mtd_storage.index(chr(10) + "func_save()", tar_builder_start)
tar_builder = mtd_storage[tar_builder_start:tar_builder_end]
require('find . ! -name . -print0' in tar_builder and
        'find *' not in tar_builder and
        'sort -z | xargs -0 tar -cf "$tmp"' in tar_builder and
        '! -type d' not in tar_builder,
        "storage backup must include hidden files and empty directories in the complete tree")
require('storage/.hidden.conf' in mtd_storage_backup_test and
        'storage/empty-dir' in mtd_storage_backup_test and
        "'./.hidden.conf'" in mtd_storage_backup_test and
        "'./empty-dir/'" in mtd_storage_backup_test,
        "backup regression must verify hidden files and empty directories")
backup_start = mtd_storage.index("func_backup()")
backup_end = mtd_storage.index(chr(10) + "func_restore()", backup_start)
backup = mtd_storage[backup_start:backup_end]
for token in ('if ! bzip2 -9 "$tmp" 2>/dev/null; then',
              'stat -c %s "$tbz"',
              'if ! bzcat "$tbz" > "$tmp" 2>/dev/null ||',
              'tar tf "$tmp" >/dev/null 2>&1',
              'rm -f "$tmp" "$tbz"'):
    require(token in backup, "MTD backup validation misses " + token)
backup_dispatch = mtd_storage[mtd_storage.index("backup)"):mtd_storage.index("restore)")]
require("func_get_mtd" in backup_dispatch and
        backup_dispatch.index("func_get_mtd") < backup_dispatch.index("func_tarb") < backup_dispatch.index("func_backup"),
        "MTD backup must read the actual target partition size before compression")
require("BZIP2_MODE=fail" in mtd_storage_backup_test and
        "BZIP2_MODE=oversized" in mtd_storage_backup_test and
        "BZCAT_MODE=fail" in mtd_storage_backup_test and
        "BZCAT_MODE=invalid" in mtd_storage_backup_test and
        "MTD storage backup failure and integrity regression" in mtd_storage_backup_test,
        "MTD backup fixture must cover compression, size, stream, tar and successful archive")

writer = body(rc, "write_storage_to_mtd(void)")
require('return doSystem("/sbin/mtd_storage.sh %s", "save");' in writer and
        "int write_storage_to_mtd(void);" in rc_header,
        "RC MTD writer must propagate the storage script status")
time_save = body(rc, "storage_save_time(time_t delta)")
for operation in ("mkstemp(temp_path)", 'fdopen(fd, "w")',
                  "fchmod(fd, 0644)", "fflush(fp)", "fsync(fileno(fp))",
                  "fclose(fp)", 'rename(temp_path, "/etc/storage/system_time")'):
    require(operation in time_save, "timestamp save misses checked operation: " + operation)
require(time_save.index("fflush(fp)") < time_save.index("fsync(fileno(fp))") <
        time_save.index("if (fclose(fp) != 0)") <
        time_save.index('rename(temp_path, "/etc/storage/system_time")'),
        "timestamp temp file must be flushed, synced, closed and renamed in order")
require("(void)unlink(temp_path);" in time_save and
        "errno = saved_errno;" in time_save,
        "timestamp save must clean temporary files and preserve errno")

require("storage_persistence_mtd_may_run" in persistence_policy and
        "storage_persistence_complete" in persistence_policy and
        "storage_persistence_complete(-1, 0)" in persistence_test and
        "storage_persistence_complete(0, -1)" in persistence_test,
        "firmware persistence gate policy needs failure cases")
flash = body(rc, "flash_firmware(void)")
flash_time = flash.index("storage_time_status = storage_save_time(60)")
flash_mtd = flash.index("storage_mtd_status = storage_persistence_mtd_may_run(storage_time_status)")
flash_gate = flash.index("if (!storage_persistence_complete(storage_time_status, storage_mtd_status))")
flash_image = flash.index('eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME)')
require(flash_time < flash_mtd < flash_gate < flash_image and
        "watchdog_status = start_watchdog();" in flash[flash_gate:flash_image],
        "firmware image write must require successful storage persistence and recover watchdog on abort")
startup = body(rc, "init_router(void)")
require("storage_save_status = write_storage_to_mtd();" in startup and
        "if (storage_save_status != 0)" in startup and
        startup.index("if (storage_save_status != 0)") < startup.index("restart_crond()"),
        "startup must report snapshot failure and still restart crond")
shutdown_writer = body(rc, "shutdown_router(int level)")
shutdown_time = shutdown_writer.index("storage_time_status = storage_save_time(10)")
shutdown_mtd = shutdown_writer.index("storage_mtd_status = storage_persistence_mtd_may_run(storage_time_status)")
shutdown_gate = shutdown_writer.index("if (!storage_persistence_complete(storage_time_status, storage_mtd_status))")
require(shutdown_time < shutdown_mtd < shutdown_gate and
        "continuing power-down" in shutdown_writer[shutdown_gate:],
        "shutdown must report persistence errors without aborting power-down")
require("int storage_save_time(time_t delta);" in rc_header,
        "RC header must expose timestamp status")

for path in ("trunk/user/scripts/files/sbin/mtd_storage.sh",
             "trunk/user/scripts/tests/mtd_storage_save_retry.sh",
             "trunk/user/scripts/tests/mtd_storage_restore_transaction.sh",
             "trunk/user/scripts/tests/mtd_storage_backup_failure.sh",
             "trunk/user/scripts/tests/mtd_storage_load_transaction.sh",
             "trunk/user/rc/src/storage_persistence_policy.h",
             "trunk/user/rc/src/tests/storage_persistence_policy.c"):
    require('"' + path + '"' in push_filters and '"' + path + '"' in pull_filters,
            "storage-hotplug must trigger for persistence file " + path)
for path in ("trunk/user/scripts/files/sbin/mtd_storage.sh",
             "trunk/user/rc/src/storage_persistence_policy.h"):
    require('"' + path + '"' in xy_workflow,
            "XY-C1 must trigger for product persistence dependency " + path)

# All MTD storage commands share an OS-backed lock. A marker file alone
# cannot stop an already-running save that passed its marker check earlier.
lock_start = mtd_storage.index('lock_dir="/tmp/.httpd-private"')
lock_end = mtd_storage.index("func_get_mtd()", lock_start)
lock_setup = mtd_storage[lock_start:lock_end]
require('if ! exec 9>>"$lock_file"; then' in lock_setup and
        "flock -n 9" in lock_setup and
        lock_setup.index("chmod 700") < lock_setup.index('exec 9>>"$lock_file"') <
        lock_setup.index("flock -n 9"),
        "MTD commands must acquire a nonblocking lock under the protected private directory")
for index, fixture in enumerate(mtd_storage_lock_tests):
    require(r'lock_dir=\"$sandbox/private\"' in fixture or
            r'lock_dir=\"$base/private\"' in fixture,
            "MTD fixture must isolate the operation lock: index {}".format(index))
save_fixture = mtd_storage_lock_tests[0]
require("flock -n 8" in save_fixture and
        "concurrent MTD save bypassed the operation lock" in save_fixture and
        'run_mtd save' in save_fixture and
        'ln -s "$BUSYBOX_BIN" "$sandbox/bin/flock"' in save_fixture and
        '"$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" "$1"' in save_fixture and
        'BUSYBOX_BIN="$busybox_bin" sh trunk/user/scripts/tests/mtd_storage_save_retry.sh' in workflow,
        "MTD save regression must exercise real BusyBox ash/flock and exclude competing transactions")

presence_start = mtd_storage.index("func_rstats_presence()")
presence_end = mtd_storage.index("\nfunc_stop_apps()", presence_start)
presence_helper = mtd_storage[presence_start:presence_end]
stop_start = mtd_storage.index("func_stop_apps()")
stop_end = mtd_storage.index("\nfunc_start_apps()", stop_start)
stop_helper = mtd_storage[stop_start:stop_end]
require('local proc_root="/proc"' in presence_helper and
        '"$proc_root"/[0-9]*' in presence_helper and
        "return 2" in presence_helper and
        "stat_comm=${stat_line#" in presence_helper,
        "rstats procfs inventory must distinguish malformed/unreadable scans from absence")
require("func_rstats_presence" in stop_helper and
        "killall -q rstats" in stop_helper and
        "for attempts in 1 2 3 4 5" in stop_helper and
        "rstats did not stop within the bounded wait" in stop_helper,
        "storage app stop must verify bounded rstats absence rather than trust killall status")
restore_start = mtd_storage.index("func_restore()")
restore_end = mtd_storage.index("\nfunc_erase()", restore_start)
restore = mtd_storage[restore_start:restore_end]
require("mktemp -d" in restore and 'cp -a "$tmp_storage"/. "$stage_storage"/' in restore and
        'chmod 755 "$stage_storage"' in restore,
        "storage restore must prepare a complete same-filesystem staging tree")
require(restore.index("cp -a") < restore.index("func_stop_apps") <
        restore.index('mv "$dir_storage" "$rollback_storage"') <
        restore.index('mv "$stage_storage" "$dir_storage"'),
        "storage restore must stage before stopping apps and replacing live settings")
restore_stop = restore[restore.index("if ! func_stop_apps; then"):restore.index("\n\thad_storage=0")]
require('rm -rf "$stage_storage" "$tmp_storage"' in restore_stop and
        "return 1" in restore_stop,
        "storage restore must retain the live directory and abort when rstats stop is not confirmed")
reset_start = mtd_storage.index("\nreset)\n", mtd_storage.index('case "$1" in'))
reset_end = mtd_storage.index("\n\t;;", reset_start)
reset_case = mtd_storage[reset_start:reset_end]
require(reset_case.index('touch "$slk"') < reset_case.index("func_stop_apps") <
        reset_case.index("func_reset") < reset_case.index("func_fill") and
        "elif ! func_stop_apps; then" in reset_case,
        "storage reset must establish its save barrier and confirm rstats stop before deleting settings")
require('mv "$rollback_storage" "$dir_storage"' in restore and
        "restore install and rollback both failed" in restore and
        "Cannot prepare complete storage restore tree" in restore,
        "storage restore must preserve and recover the previous directory on install failures")
require("RESTORE_FAIL_CP=1" in mtd_storage_restore_test and
        "RESTORE_FAIL_MV_AT=2" in mtd_storage_restore_test and
        "rollback" in mtd_storage_restore_test,
        "storage restore regression must inject copy and install-rename failures")
require("malformed stat record" in mtd_storage_restore_test and
        "incomplete procfs inventory unexpectedly authorized storage restore" in mtd_storage_restore_test and
        "RSTAT_STOP_FAIL=1 run_restore" in mtd_storage_restore_test,
        "storage restore regression must cover UNKNOWN procfs state and failed daemon stop")


# create_mp_link keeps its compatibility boolean API but every failure has a
# deterministic errno; the status adapter is visible before all media callers.
mount_link = body(storage, "int create_mp_link(char *search_dir, char *link_path, int force_first_valid)")
require("errno = EINVAL;" in mount_link and "errno = 0;" in mount_link,
        "mount-link must reject invalid arguments and isolate stale errno")
require('fopen(PROC_MOUNTS_FILE, "r")' in mount_link and
        "storage_nfs_export_errno_or_eio(errno)" in mount_link,
        "mount inventory open failure must return its errno or EIO")
require("storage_nfs_export_parse_mount_line(" in mount_link and
        "storage_nfs_export_first_error(first_error, EINVAL)" in mount_link,
        "malformed mount rows must retain a deterministic parse error")
require("path_len >= (int)sizeof(target_path)" in mount_link and
        "ENAMETOOLONG" in mount_link,
        "overlong media database paths must produce ENAMETOOLONG")
require("check_if_dir_exist(target_path)" in mount_link and
        'mkdir_if_none(target_path, "777")' in mount_link and
        "ENOTDIR" in mount_link and "EEXIST" in mount_link,
        "directory discovery/preparation needs deterministic errors")
require(mount_link.count("symlink(target_path, link_path)") == 2,
        "both mount-link symlink paths must preserve creation errno")
require("ferror(procpt)" in mount_link and "fclose(procpt)" in mount_link and
        "storage_nfs_export_inventory_complete(read_error, close_error," in mount_link,
        "mount inventory read/parse/close failures must invalidate the link")
require("unlink(link_path)" in mount_link and
        "errno = storage_nfs_export_errno_or_eio(first_error);" in mount_link,
        "incomplete inventory must clean up the link and preserve its first error")
require("return 1;" in mount_link and "return 0;" in mount_link,
        "legacy create_mp_link API must remain boolean")

mount_status_adapter = body(storage, "create_mp_link_status(")
require("errno = 0;" in mount_status_adapter and
        "create_mp_link(search_dir, link_path, force_first_valid)" in mount_status_adapter and
        "errno = status;" in mount_status_adapter,
        "status adapter must isolate errno and return the normalized error")
adapter_declaration = storage.index("static int create_mp_link_status(")
for signature in ("int run_dms(int force_rescan)", "int run_itunes(void)",
                  "int run_torrent(void)", "int run_aria(void)"):
    require(adapter_declaration < storage.index(signature),
            "mount-link status adapter must be declared before " + signature)
    caller = body(storage, signature)
    require("create_mp_link_status(" in caller and "link_status" in caller and
            "return link_status;" in caller and "create_mp_link(" not in caller,
            "media caller must use the errno-normalizing adapter: " + signature)
for signature in ("int run_dms(int force_rescan)", "int run_itunes(void)"):
    caller = body(storage, signature)
    require(caller.index("create_mp_link_status(dest_dir, link_path, 0)") <
            caller.index("create_mp_link_status(dest_dir, link_path, 1)"),
            "media database fallback selection changed: " + signature)
for signature in ("int run_torrent(void)", "int run_aria(void)"):
    caller = body(storage, signature)
    require("create_mp_link_status(dest_dir, link_path, 0)" in caller and
            "create_mp_link_status(dest_dir, link_path, 1)" not in caller,
            "download daemon mount selection semantics changed: " + signature)

require("storage_nfs_export_first_error" in mount_link_policy and
        "storage_nfs_export_errno_or_eio" in mount_link_policy_test and
        "storage_nfs_export_first_error(ENOENT, EACCES)" in mount_link_policy_test and
        "storage_nfs_export_errno_or_eio(0) == EIO" in mount_link_policy_test,
        "mount-link errno policy requires GNU89 regression cases")
require("Compile and run storage mount-link errno policy regression" in workflow,
        "storage workflow must compile and run errno policy regression")
for path in ("trunk/user/rc/src/storage_nfs_export_policy.h",
             "trunk/user/rc/src/tests/storage_nfs_export_policy.c"):
    require(workflow.count('"' + path + '"') == 2,
            "storage push/PR filters must include " + path)


# SD/MMC hotplug handlers must not acknowledge work that PID 1 never queued.
notify_helper = body(hotplug_stor, "static int\nqueue_storage_hotplug_notification(")
require("notify_rc_status(event_name)" in notify_helper and
        "return status;" in notify_helper and
        "errno = saved_errno;" in notify_helper and
        "cannot queue %s notification" in notify_helper,
        "mass-storage notifications must use the checked PID-1 queue API and preserve the failure")
require("notify_rc(" not in hotplug_stor,
        "mass-storage handlers must not discard notification status through the legacy void API")
for signature in ("mdev_sd_main(int argc, char **argv)",
                  "mdev_mmc_main(int argc, char **argv)"):
    handler = body(hotplug_stor, signature)
    require("int notification_status = 0;" in handler and
            "storage_hotplug_completion_status(" in handler,
            "mdev return status must include PID-1 notification failures: " + signature)
    require(handler.count("queue_storage_hotplug_notification(device_name,") == 2 and
            "notify_rc(" not in handler,
            "both add/remove transitions must use checked notification in " + signature)
    removal = handler.split("if (!get_hotplug_action(action))", 1)[1].split(
        "goto out_unlock;", 1
    )[0]
    require("report_device_swapoff(device_name)" in removal and
            removal.index("report_device_swapoff(device_name)") <
            removal.index('queue_storage_hotplug_notification(device_name, "on_unplug_mass_storage")'),
            "whole-device removal must attempt its PID-1 event after swapoff processing")
    mount_success = handler.split("if (mount_status == 0)", 1)[1].split("else", 1)[0]
    require('queue_storage_hotplug_notification(device_name, "on_hotplug_mass_storage")' in mount_success,
            "successful automount must queue the add event before returning success")
require("storage_hotplug_completion_status(0, -EIO) == 1" in notify_policy_test and
        "storage_hotplug_completion_status(0, -EACCES) == 1" in notify_policy_test and
        "storage_hotplug_completion_status(0, 0) == 0" in notify_policy_test,
        "GNU89 regression must cover queued success and notification failures")
require("static inline int\nstorage_hotplug_completion_status(" in notify_policy,
        "mdev notification completion policy is missing")
for path in ("trunk/user/rc/src/storage_hotplug_notify_policy.h",
             "trunk/user/rc/src/tests/storage_hotplug_notify_policy.c"):
    require(workflow.count('"' + path + '"') == 2,
            "storage push/PR filters must include " + path)
    require(path in xy_workflow,
            "XY-C1 firmware validation must watch notification dependency " + path)
require("Compile and run storage hotplug notification failure regression" in workflow and
        "storage_hotplug_notify_policy.c" in workflow,
        "storage workflow must execute the notification regression")

print("Storage eject inventory, mount-link errno, failure propagation and persistence gates: PASS")
