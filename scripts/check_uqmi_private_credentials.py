#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UQMI = ROOT / "trunk/user/uqmi"
RC = ROOT / "trunk/user/rc/src"


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("incomplete function: " + signature)


def main() -> None:
    modem = (RC / "usb_modem.c").read_text()
    modem_header = (RC / "modem_qmi_credentials_file.h").read_text()
    modem_test = (RC / "tests/modem_qmi_credentials_file.c").read_text()
    start_policy = (RC / "modem_wan_start_policy.h").read_text()
    start_policy_test = (RC / "tests/modem_wan_start_policy.c").read_text()
    parser = (UQMI / "uqmi-private-credentials.c").read_text()
    parser_header = (UQMI / "uqmi-private-credentials.h").read_text()
    parser_test = (UQMI / "tests/private_credentials.c").read_text()
    uqmi_main = (UQMI / "main.c").read_text()
    uqmi_header = (UQMI / "commands.h").read_text()
    uqmi_commands = (UQMI / "commands.c").read_text()
    uqmi_wds = (UQMI / "commands-wds.c").read_text()
    makefile = (UQMI / "Makefile").read_text()
    workflow = Path(".github/workflows/usb-modem-identity-stability.yml").read_text()

    qmi_stop = body(modem, "qmi_control_network(const char *control_node, int is_start)")
    pdh_cleanup = qmi_stop.index("cleanup_status = modem_qmi_handle_file_unlink_status(unlink_status, errno);")
    release_gate = qmi_stop.index("if (!modem_qmi_client_id_release_allowed(cleanup_status))", pdh_cleanup)
    release_return = qmi_stop.index("return 1;", release_gate)
    cid_read = qmi_stop.index("qmi_client_id = get_qmi_handle(QMI_CLIENT_ID);", release_gate)
    release_command = qmi_stop.index('"--release-client-id"', release_gate)
    assert pdh_cleanup < release_gate < release_return < cid_read < release_command
    assert "preserving WDS client ID for recovery" in qmi_stop[release_gate:release_return]
    assert "modem_qmi_client_id_release_allowed(int packet_data_handle_cleanup_status)" in start_policy
    assert "return packet_data_handle_cleanup_status == 0;" in start_policy
    assert "test_qmi_client_id_release_waits_for_pdh_cleanup" in start_policy_test

    qmi_start = body(modem, "qmi_control_network_start(")
    assert "rc_wan_nvram_copy_snapshot(" in qmi_start
    for key in ("modem_pin", "modem_user", "modem_pass", "modem_apn", "modem_nets"):
        assert f'"{key}"' in qmi_start
    assert "rc_wan_nvram_parse_int_value(" in qmi_start
    assert "nvram_safe_get(" not in qmi_start
    assert "nvram_get_int(" not in qmi_start
    assert "modem_qmi_credentials_file_create(pin_code, NULL, NULL" in qmi_start
    assert "modem_qmi_credentials_file_create(NULL, username, password" in qmi_start
    assert qmi_start.count('"--private-credentials-file"') == 2
    for secret_option in ('"--verify-pin1"', '"--username"', '"--password"'):
        assert secret_option not in qmi_start
    for value in ("pin_code", "username", "password"):
        assert f"modem_qmi_credentials_wipe({value}," in qmi_start
    assert "credentials_file.path" in qmi_start

    assert '#include "modem_qmi_credentials_file.h"' in modem
    assert "mkdtemp(" in modem_header
    assert "O_NOFOLLOW" in modem_header and "O_DIRECTORY" in modem_header
    assert "(directory_stat.st_mode & 07777) != 0700" in modem_header
    assert "(file_stat.st_mode & 07777) != 0600" in modem_header
    assert "O_CREAT | O_EXCL | O_NOFOLLOW" in modem_header
    assert "fstatat(" in modem_header and "unlinkat(" in modem_header
    assert "modem_qmi_credentials_file_cleanup(" in modem_header
    assert "file->file_identity_valid = 1;" in modem_header
    cleanup = body(modem_header, "modem_qmi_credentials_file_cleanup(")
    assert "file_stat.st_dev != file->file_device" in cleanup
    assert "file_stat.st_ino != file->file_inode" in cleanup
    assert "test_private_qmi_credentials_round_trip_and_cleanup" in modem_test
    assert "test_injection_and_incomplete_pairs_are_rejected" in modem_test
    assert "test_cleanup_refuses_replacement_inode" in modem_test

    assert '"private-credentials-file"' in uqmi_main
    assert '"d:k:C:"' in uqmi_main
    assert "uqmi_private_credentials_load(" in uqmi_main
    assert "uqmi_add_command_before_requests(private_credentials.pin" in uqmi_main
    assert "uqmi_add_command(private_credentials.username" in uqmi_main
    assert "uqmi_add_command(private_credentials.password" in uqmi_main
    assert "uqmi_private_credentials_clear(&private_credentials)" in uqmi_main

    queue_header = (UQMI / "uqmi-command-queue-policy.h").read_text()
    queue_test = (UQMI / "tests/command_queue.c").read_text()
    append = body(uqmi_commands, "uqmi_add_command(")
    insert = body(uqmi_commands, "uqmi_add_command_before_requests(")
    queue_insert = body(queue_header, "uqmi_cmd_queue_insert(")
    assert "uqmi_cmd_queue_insert(&cmds, &n_cmds, n_cmds" in append
    assert "uqmi_cmd_queue_insert(&cmds, &n_cmds, insert_at" in insert
    assert "int uqmi_add_command(char *arg, int longidx);" in uqmi_header
    assert "if (uqmi_add_command(optarg, cmd_opt) < 0)" in uqmi_main
    assert "uqmi_add_command(private_credentials.username" in uqmi_main
    assert "uqmi_add_command(private_credentials.password" in uqmi_main
    assert "if (!resized)" in queue_insert
    assert "*commands = resized;" in queue_insert
    assert "(*count)++;" in queue_insert
    assert "test_allocation_failure_preserves_existing_queue" in queue_test
    assert "test_invalid_index_and_count_overflow_are_rejected" in queue_test
    insert = body(uqmi_commands, "uqmi_add_command_before_requests(")
    assert "cmds[insert_at].handler->type == CMD_TYPE_OPTION" in insert
    assert "memmove(&resized[index + 1]" in queue_insert
    assert "return -1;" in insert
    assert "reallocate" in queue_insert
    assert "cmds[idx].arg = optarg;" not in uqmi_commands
    assert "uqmi_wipe_memory(uqmi_command_buffer" in uqmi_commands
    assert "uqmi_wds_clear_sensitive_options();" in uqmi_commands
    assert "uqmi_wds_clear_sensitive_options" in uqmi_header
    clear_wds = body(uqmi_wds, "uqmi_wds_clear_sensitive_options(")
    assert "qmi_set_ptr(&wds_sn_req, username, NULL)" in clear_wds
    assert "qmi_set_ptr(&wds_sn_req, password, NULL)" in clear_wds
    assert "qmi_set_ptr(&wds_sn_req, apn, NULL)" in clear_wds
    assert "uqmi-private-credentials.c" in makefile

    assert "openat(dirfd, basename" in parser
    assert "O_RDONLY | O_NOFOLLOW" in parser
    assert "fstatat(dirfd, basename" in parser
    assert parser.index("unlinkat(dirfd, basename, 0)") < parser.index("count = read(")
    assert "directory_stat.st_uid != geteuid()" in parser
    assert "opened_stat.st_uid != geteuid()" in parser
    assert "opened_stat.st_nlink != 1" in parser
    assert "AT_SYMLINK_NOFOLLOW" in parser
    assert "uqmi_private_credentials_wipe(buffer, sizeof(buffer))" in parser
    assert "UQMI_PRIVATE_CREDENTIALS_MAX" in parser_header
    assert "test_symlinks_are_not_followed" in parser_test
    assert "test_file_mode_and_link_count_are_checked" in parser_test
    assert "test_malformed_files_are_consumed_but_not_accepted" in parser_test

    for path in (
        "trunk/user/uqmi/main.c",
        "trunk/user/uqmi/commands.c",
        "trunk/user/uqmi/uqmi-command-queue-policy.h",
        "trunk/user/uqmi/tests/command_queue.c",
        "trunk/user/uqmi/commands.h",
        "trunk/user/uqmi/commands-wds.c",
        "trunk/user/uqmi/uqmi-private-credentials.c",
        "trunk/user/uqmi/uqmi-private-credentials.h",
        "trunk/user/uqmi/tests/private_credentials.c",
        "trunk/user/uqmi/Makefile",
        "trunk/user/rc/src/modem_qmi_credentials_file.h",
        "trunk/user/rc/src/tests/modem_qmi_credentials_file.c",
        "trunk/user/rc/src/modem_wan_start_policy.h",
        "trunk/user/rc/src/tests/modem_wan_start_policy.c",
        "trunk/user/rc/src/usb_modem.c",
        "scripts/check_uqmi_private_credentials.py",
    ):
        assert workflow.count(path) >= 2 if path not in (
            "trunk/user/uqmi/uqmi-private-credentials.c",
            "trunk/user/uqmi/uqmi-private-credentials.h",
            "trunk/user/uqmi/Makefile",
            "trunk/user/rc/src/modem_qmi_credentials_file.h",
            "trunk/user/rc/src/tests/modem_qmi_credentials_file.c",
            "scripts/check_uqmi_private_credentials.py",
        ) else workflow.count(path) >= 1, f"missing workflow path: {path}"

    print("QMI credential isolation and WDS teardown ownership contracts: PASS")


if __name__ == "__main__":
    main()
