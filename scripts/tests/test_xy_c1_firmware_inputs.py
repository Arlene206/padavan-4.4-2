#!/usr/bin/env python3
"""Unit tests for fail-safe XY-C1 firmware-input detection."""
from __future__ import annotations

import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from check_xy_c1_firmware_inputs import decide_firmware_build, is_non_firmware_input  # noqa: E402

BEFORE = "1" * 40
AFTER = "2" * 40


def result(args, code=0, stdout="", stderr=""):
    return subprocess.CompletedProcess(args, code, stdout, stderr)


class FirmwareInputPathTests(unittest.TestCase):
    def test_each_commit_keeps_its_xy_c1_run_alive(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        self.assertIn("group: xy-c1-firmware-validation-${{ github.ref }}-${{ github.sha }}", workflow)
        self.assertIn("cancel-in-progress: false", workflow)

    def test_dns_forwarder_source_triggers_firmware_validation(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        source_path = '      - "trunk/user/dns-forwarder/**"'
        self.assertEqual(workflow.count(source_path), 1)
        self.assertFalse(is_non_firmware_input("trunk/user/dns-forwarder/dns-forwarder.sh"))
        self.assertIn('      - "trunk/user/shared/scripts/process_lifecycle.sh"', workflow)

    def test_networkmap_production_sources_trigger_xy_c1_firmware_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        source_path = '      - "trunk/user/networkmap/**"'
        self.assertEqual(workflow.count(source_path), 1)
        self.assertFalse(is_non_firmware_input("trunk/user/networkmap/src/networkmap.c"))
        self.assertFalse(is_non_firmware_input("trunk/user/networkmap/src/networkmap_arp_policy.h"))
        self.assertTrue(is_non_firmware_input(
            "trunk/user/networkmap/src/tests/networkmap_arp_policy.c"
        ))

    def test_ralink_iapp_c_sources_trigger_xy_c1_firmware_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        source_path = '      - "trunk/user/ralinkiappd/src/**"'
        self.assertEqual(workflow.count(source_path), 2)
        for path in (
            "trunk/user/ralinkiappd/src/rtmpiapp.c",
            "trunk/user/ralinkiappd/src/mt_iapp_util.c",
            "trunk/user/ralinkiappd/src/Makefile",
        ):
            with self.subTest(path=path):
                self.assertFalse(is_non_firmware_input(path))

    def test_regression_test_file_triggers_its_own_workflow(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        test_path = '      - "scripts/tests/test_xy_c1_firmware_inputs.py"'
        self.assertEqual(workflow.count(test_path), 1)
        self.assertTrue(is_non_firmware_input("scripts/tests/test_xy_c1_firmware_inputs.py"))

    def test_documentation_workflow_checker_and_test_paths_skip_build(self):
        paths = [
            "scripts/check_xy_c1_firmware_inputs.py",
            "scripts/tests/test_xy_c1_firmware_inputs.py",
            "docs/dev-log/INDEX.md",
            "SESSION_HANDOFF.md",
            "REFACTOR_ROADMAP.md",
            "dev_log.md",
            "trunk/user/rc/src/tests/example.c",
        ]
        for path in paths:
            with self.subTest(path=path):
                self.assertTrue(is_non_firmware_input(path))

    def test_xy_c1_workflow_and_busybox_init_sources_trigger_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        workflow_path = ".github/workflows/xy-c1-firmware-validation.yml"
        self.assertEqual(workflow.count('      - "' + workflow_path + '"'), 1)
        self.assertIn('      - "trunk/user/busybox/busybox-1.24.x/init/**"', workflow)
        self.assertFalse(is_non_firmware_input(workflow_path))
        self.assertFalse(is_non_firmware_input(
            "trunk/user/busybox/busybox-1.24.x/init/init.c"
        ))
        self.assertFalse(is_non_firmware_input(
            "trunk/user/busybox/busybox-1.24.x/init/init_reap_queue.h"
        ))
        self.assertTrue(is_non_firmware_input(
            "trunk/user/busybox/busybox-1.24.x/init/tests/init_reap_queue.c"
        ))

    def test_dhcp_domain_codec_production_source_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/domain_codec.c"
        self.assertEqual(workflow.count('      - "' + path + '"'), 1)
        self.assertFalse(is_non_firmware_input(path))

    def test_production_sources_and_configuration_require_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        self.assertEqual(
            workflow.count('      - "trunk/user/openvpn/openvpn-cert.sh"'),
            1,
        )
        paths = [
            "trunk/user/rc/src/init.c",
            "trunk/user/openvpn/openvpn-cert.sh",
            "trunk/user/rc/src/detect_wan_arp_result_policy.h",
            "trunk/user/httpd/src/httpd.c",
            "trunk/user/shared/scripts/process_lifecycle.sh",
            "trunk/linux-4.4.x/net/ipv4/tcp.c",
            "trunk/configs/boards/XY-C1/kernel-4.4.x.config",
            "trunk/user/busybox/busybox-1.24.x/util-linux/Config.src",
            "trunk/user/busybox/busybox-1.24.x/init/init.c",
            "trunk/user/busybox/busybox-1.24.x/init/init_reap_queue.h",
            "trunk/user/rc/Makefile",
            "trunk/user/802.1x/src/config.c",
        ]
        for path in paths:
            with self.subTest(path=path):
                self.assertFalse(is_non_firmware_input(path))

    def test_spindown_script_triggers_xy_c1_firmware_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/scripts/files/sbin/spindown.sh"
        self.assertEqual(workflow.count('      - "' + path + '"'), 1)
        self.assertFalse(is_non_firmware_input(path))

    def test_miniupnpd_pidfile_sources_trigger_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        paths = (
            "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd.c",
            "trunk/user/miniupnpd/miniupnpd-2.x/upnpsoap.c",
            "trunk/user/miniupnpd/miniupnpd-2.x/upnpsoap_input_policy.h",
            "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptpinhole.c",
            "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_pinhole_policy.h",
            "trunk/user/miniupnpd/miniupnpd-2.x/upnpsoap_xml_policy.h",
            "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c",
            "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_lease_file_policy.h",
            "trunk/user/miniupnpd/miniupnpd-2.x/daemonize.c",
            "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_pidfile_policy.h",
            "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux",
        )
        for path in paths:
            with self.subTest(path=path):
                self.assertIn('      - "' + path + '"', workflow)
                self.assertFalse(is_non_firmware_input(path))

    def test_lldt_startup_wrapper_triggers_xy_c1_firmware_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/lldt/src/S75lld2"
        self.assertEqual(workflow.count('      - "' + path + '"'), 1)
        self.assertFalse(is_non_firmware_input(path))

    def test_firewall_nvram_match_policy_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/rc/src/firewall_nvram_match_policy.h"
        self.assertIn('      - "' + path + '"', workflow)
        self.assertFalse(is_non_firmware_input(path))

    def test_dnsmasq_option_policy_header_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/rc/src/dnsmasq_nvram_option_policy.h"
        self.assertEqual(workflow.count('      - "' + path + '"'), 2)
        self.assertFalse(is_non_firmware_input(path))

    def test_dnsmasq_static_host_nvram_policy_header_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/rc/src/dnsmasq_static_host_nvram_policy.h"
        self.assertEqual(workflow.count('      - "' + path + '"'), 2)
        self.assertFalse(is_non_firmware_input(path))

    def test_watchdog_checked_nvram_policy_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        for path in (
            "trunk/user/rc/src/services.c",
            "trunk/user/rc/src/watchdog_cpu_restart_policy.h",
            "trunk/user/rc/src/wan_nvram_status_policy.h",
        ):
            with self.subTest(path=path):
                self.assertIn('      - "' + path + '"', workflow)
                self.assertFalse(is_non_firmware_input(path))

    def test_samba_config_mode_policy_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        paths = (
            "trunk/user/rc/src/services_stor.c",
            "trunk/user/rc/src/samba_config_mode_policy.h",
            "trunk/user/rc/src/wan_nvram_status_policy.h",
        )
        for path in paths:
            with self.subTest(path=path):
                self.assertIn('      - "' + path + '"', workflow)
                self.assertFalse(is_non_firmware_input(path))

    def test_libdisk_io_tools_reader_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        for path in (
            "trunk/user/libdisk/src/disk_io_tools.c",
            "trunk/user/libdisk/src/disk_io_tools.h",
        ):
            with self.subTest(path=path):
                self.assertEqual(workflow.count('      - "' + path + '"'), 1)
                self.assertFalse(is_non_firmware_input(path))

    def test_qmi_private_credentials_sources_require_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        self.assertIn('      - "trunk/user/uqmi/**"', workflow)
        self.assertIn('      - "trunk/user/rc/src/modem_qmi_credentials_file.h"', workflow)
        paths = (
            "trunk/user/uqmi/main.c",
            "trunk/user/uqmi/commands.c",
            "trunk/user/uqmi/uqmi-private-credentials.c",
            "trunk/user/uqmi/uqmi-private-credentials.h",
            "trunk/user/rc/src/usb_modem.c",
            "trunk/user/rc/src/modem_qmi_credentials_file.h",
        )
        for path in paths:
            with self.subTest(path=path):
                self.assertFalse(is_non_firmware_input(path))

    def test_scutclient_credentials_overlay_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        for path in (
            "trunk/user/scutclient/Makefile",
            "trunk/user/scutclient/patches/scutclient_credentials.c",
            "trunk/user/scutclient/patches/scutclient_credentials.h",
            "trunk/user/scutclient/patches/0001-private-credentials-file.patch",
        ):
            with self.subTest(path=path):
                self.assertIn('      - "' + ("trunk/user/scutclient/patches/**" if "/patches/" in path else path) + '"', workflow)
                self.assertFalse(is_non_firmware_input(path))
    def test_scut_watchdog_flock_dependency_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        for path in (
            "trunk/user/scutclient/scripts/scutclient_watchcat.sh",
            "trunk/user/busybox/busybox-1.24.x/util-linux/Config.src",
        ):
            with self.subTest(path=path):
                self.assertIn('      - "' + path + '"', workflow)
                self.assertFalse(is_non_firmware_input(path))

    def test_watchdog_policy_headers_trigger_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        for path in (
            "trunk/user/rc/src/watchdog_ntp_policy.h",
            "trunk/user/rc/src/watchdog_signal_policy.h",
            "trunk/user/rc/src/detached_daemon_start_policy.h",
        ):
            with self.subTest(path=path):
                self.assertIn('      - "' + path + '"', workflow)
                self.assertFalse(is_non_firmware_input(path))

        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        for path in (
            "trunk/user/rc/src/watchdog_ntp_policy.h",
            "trunk/user/rc/src/watchdog_signal_policy.h",
        ):
            with self.subTest(path=path):
                self.assertIn('      - "' + path + '"', workflow)

    def test_non_push_events_fail_safe_to_build(self):
        for event in ("workflow_dispatch", "pull_request", ""):
            changed, paths, reason = decide_firmware_build(
                event, BEFORE, AFTER, lambda args: self.fail("git must not run")
            )
            self.assertTrue(changed)
            self.assertEqual(paths, [])
            self.assertIn("full build", reason)

    def test_invalid_or_initial_before_sha_fails_safe(self):
        for before in ("", "not-a-sha", "0" * 40):
            with self.subTest(before=before):
                calls = []
                changed, paths, reason = decide_firmware_build(
                    "push", before, AFTER,
                    lambda args: calls.append(args) or self.fail("git must not run"),
                )
                self.assertTrue(changed)
                self.assertEqual(paths, [])
                self.assertIn("fail-safe", reason)
                self.assertEqual(calls, [])

    def test_full_git_diff_detects_product_change_without_event_commit_array(self):
        calls = []
        def fake_git(args):
            calls.append(args)
            if args[:2] == ["rev-parse", "HEAD"]:
                return result(args, stdout=AFTER + "\n")
            if args[:3] == ["fetch", "--no-tags", "--depth=1"]:
                return result(args)
            if args[:2] == ["diff", "--name-only"]:
                return result(args, stdout=(
                    ".github/workflows/xy-c1-firmware-validation.yml\0"
                    "trunk/user/rc/src/init.c\0"
                ))
            self.fail("unexpected git args: " + repr(args))

        changed, paths, reason = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertTrue(changed)
        self.assertEqual(paths, [
            ".github/workflows/xy-c1-firmware-validation.yml",
            "trunk/user/rc/src/init.c",
        ])
        self.assertIn("production inputs changed", reason)
        self.assertEqual(calls[0], ["rev-parse", "HEAD"])
        self.assertEqual(calls[1], ["fetch", "--no-tags", "--depth=1", "origin", BEFORE])
        self.assertEqual(calls[2], ["diff", "--name-only", "--no-renames", "-z", BEFORE, AFTER])

    def test_xy_c1_workflow_only_push_triggers_build(self):
        def fake_git(args):
            if args[:2] == ["rev-parse", "HEAD"]:
                return result(args, stdout=AFTER + "\n")
            if args[0] == "fetch":
                return result(args)
            if args[0] == "diff":
                return result(args, stdout=(
                    ".github/workflows/xy-c1-firmware-validation.yml\0"
                ))
            self.fail("unexpected git args: " + repr(args))

        changed, paths, reason = decide_firmware_build(
            "push", BEFORE, AFTER, fake_git
        )
        self.assertTrue(changed)
        self.assertEqual(paths, [".github/workflows/xy-c1-firmware-validation.yml"])
        self.assertIn("production inputs changed", reason)

    def test_web_ui_asset_push_triggers_firmware_build(self):
        def fake_git(args):
            if args[:2] == ["rev-parse", "HEAD"]:
                return result(args, stdout=AFTER + "\n")
            if args[0] == "fetch":
                return result(args)
            if args[0] == "diff":
                return result(args, stdout=(
                    "trunk/user/www/n56u_ribbon_fixed/Advanced_WAN_Content.asp\0"
                    "trunk/user/www/n56u_ribbon_fixed/disk_folder_tree.js\0"
                ))
            self.fail("unexpected git args: " + repr(args))

        changed, paths, reason = decide_firmware_build(
            "push", BEFORE, AFTER, fake_git
        )
        self.assertTrue(changed)
        self.assertEqual(paths, [
            "trunk/user/www/n56u_ribbon_fixed/Advanced_WAN_Content.asp",
            "trunk/user/www/n56u_ribbon_fixed/disk_folder_tree.js",
        ])
        self.assertIn("production inputs changed", reason)

    def test_documentation_only_push_skips_build(self):
        def fake_git(args):
            if args[:2] == ["rev-parse", "HEAD"]:
                return result(args, stdout=AFTER + "\n")
            if args[0] == "fetch":
                return result(args)
            if args[0] == "diff":
                return result(args, stdout=(
                    "SESSION_HANDOFF.md\0"
                    "scripts/check_xy_c1_firmware_inputs.py\0"
                ))
            self.fail("unexpected git args: " + repr(args))

        changed, paths, reason = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertFalse(changed)
        self.assertEqual(len(paths), 2)
        self.assertIn("no firmware production input changed", reason)

    def test_empty_successful_diff_skips_build(self):
        def fake_git(args):
            if args[:2] == ["rev-parse", "HEAD"]:
                return result(args, stdout=AFTER + "\n")
            if args[0] in ("fetch", "diff"):
                return result(args)
            self.fail("unexpected git args: " + repr(args))
        changed, paths, _ = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertFalse(changed)
        self.assertEqual(paths, [])

    def test_fetch_failure_fails_safe(self):
        def fake_git(args):
            if args[0] == "rev-parse":
                return result(args, stdout=AFTER + "\n")
            return result(args, code=1, stderr="fetch failed")
        changed, paths, reason = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertTrue(changed)
        self.assertEqual(paths, [])
        self.assertIn("fail-safe", reason)

    def test_diff_failure_fails_safe(self):
        def fake_git(args):
            if args[0] == "rev-parse":
                return result(args, stdout=AFTER + "\n")
            if args[0] == "fetch":
                return result(args)
            return result(args, code=1, stderr="diff failed")
        changed, paths, reason = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertTrue(changed)
        self.assertEqual(paths, [])
        self.assertIn("fail-safe", reason)

    def test_checkout_sha_mismatch_fails_safe(self):
        def fake_git(args):
            return result(args, stdout=BEFORE + "\n")
        changed, paths, reason = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertTrue(changed)
        self.assertEqual(paths, [])
        self.assertIn("fail-safe", reason)

    def test_git_timeout_exception_fails_safe(self):
        def fake_git(args):
            if args[0] == "rev-parse":
                return result(args, stdout=AFTER + "\n")
            raise subprocess.TimeoutExpired(["git", *args], 60)
        changed, paths, reason = decide_firmware_build("push", BEFORE, AFTER, fake_git)
        self.assertTrue(changed)
        self.assertEqual(paths, [])
        self.assertIn("fail-safe", reason)

    def test_openvpn_start_rollback_policy_header_triggers_xy_c1_build(self):
        workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
        path = "trunk/user/rc/src/openvpn_start_rollback_policy.h"
        self.assertEqual(workflow.count('      - "' + path + '"'), 1)
        self.assertFalse(is_non_firmware_input(path))


if __name__ == "__main__":
    unittest.main(verbosity=2)
