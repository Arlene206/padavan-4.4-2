#!/usr/bin/env python3
"""Failure-injection tests for customized firmware defaults publication."""

import errno
import importlib.util
import json
import os
import stat
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).resolve().parents[1] / "apply_customization.py"
SPEC = importlib.util.spec_from_file_location("apply_customization", SCRIPT)
apply_customization = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(apply_customization)


class AtomicCustomizationTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory(prefix="customization-test-")
        self.root = Path(self.temp_dir.name)
        self.defaults = self.root / "defaults.h"
        self.dictionary = self.root / "dict.cn"
        self.defaults.write_text('#define DEF_LAN_ADDR "192.168.2.1"\n', encoding="utf-8")
        self.dictionary.write_text("lan=192.168.2.1\n", encoding="utf-8")
        os.chmod(self.defaults, 0o640)
        os.chmod(self.dictionary, 0o600)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_success_publishes_all_files_and_preserves_modes(self):
        apply_customization.atomic_publish_text_files([
            (self.defaults, '#define DEF_LAN_ADDR "192.168.2.9"\n'),
            (self.dictionary, "lan=192.168.2.9\n"),
        ])
        self.assertIn("192.168.2.9", self.defaults.read_text(encoding="utf-8"))
        self.assertEqual("lan=192.168.2.9\n", self.dictionary.read_text(encoding="utf-8"))
        self.assertEqual(0o640, stat.S_IMODE(self.defaults.stat().st_mode))
        self.assertEqual(0o600, stat.S_IMODE(self.dictionary.stat().st_mode))
        self.assertEqual([], list(self.root.glob(".*.tmp")))

    def test_second_replace_failure_restores_first_file(self):
        real_replace = os.replace
        calls = 0

        def fail_second_replace(source, target):
            nonlocal calls
            calls += 1
            if calls == 2:
                raise OSError(errno.ENOSPC, "injected second rename failure")
            return real_replace(source, target)

        with mock.patch.object(apply_customization.os, "replace", side_effect=fail_second_replace):
            with self.assertRaises(OSError) as raised:
                apply_customization.atomic_publish_text_files([
                    (self.defaults, "new defaults\n"),
                    (self.dictionary, "new dictionary\n"),
                ])

        self.assertEqual(errno.ENOSPC, raised.exception.errno)
        self.assertEqual('#define DEF_LAN_ADDR "192.168.2.1"\n',
                         self.defaults.read_text(encoding="utf-8"))
        self.assertEqual("lan=192.168.2.1\n",
                         self.dictionary.read_text(encoding="utf-8"))
        self.assertEqual([], list(self.root.glob(".*.tmp")))

    def test_rollback_failure_keeps_original_snapshot(self):
        real_replace = os.replace
        calls = 0

        def fail_publication_and_rollback(source, target):
            nonlocal calls
            calls += 1
            if calls in (2, 3):
                raise OSError(errno.EIO, "injected replace failure")
            return real_replace(source, target)

        with mock.patch.object(apply_customization.os, "replace",
                               side_effect=fail_publication_and_rollback):
            with self.assertRaises(RuntimeError) as raised:
                apply_customization.atomic_publish_text_files([
                    (self.defaults, "new defaults\n"),
                    (self.dictionary, "new dictionary\n"),
                ])

        self.assertIn("rollback incomplete", str(raised.exception))
        self.assertIn("snapshot retained at", str(raised.exception))
        backups = list(self.root.glob(".defaults.h.*.tmp"))
        self.assertEqual(1, len(backups))
        self.assertEqual('#define DEF_LAN_ADDR "192.168.2.1"\n',
                         backups[0].read_text(encoding="utf-8"))

    def test_staging_failure_changes_neither_target(self):
        real_stage = apply_customization._stage_text_file
        calls = 0

        def fail_third_stage(target, contents, mode):
            nonlocal calls
            calls += 1
            if calls == 3:
                raise OSError(errno.ENOSPC, "injected staging failure")
            return real_stage(target, contents, mode)

        with mock.patch.object(apply_customization, "_stage_text_file",
                               side_effect=fail_third_stage):
            with self.assertRaises(OSError):
                apply_customization.atomic_publish_text_files([
                    (self.defaults, "new defaults\n"),
                    (self.dictionary, "new dictionary\n"),
                ])

        self.assertEqual('#define DEF_LAN_ADDR "192.168.2.1"\n',
                         self.defaults.read_text(encoding="utf-8"))
        self.assertEqual("lan=192.168.2.1\n",
                         self.dictionary.read_text(encoding="utf-8"))
        self.assertEqual([], list(self.root.glob(".*.tmp")))

    def test_symlink_target_is_rejected_without_following_it(self):
        link = self.root / "defaults-link.h"
        link.symlink_to(self.defaults)
        with self.assertRaises(OSError):
            apply_customization.atomic_publish_text_files([(link, "replacement\n")])
        self.assertEqual('#define DEF_LAN_ADDR "192.168.2.1"\n',
                         self.defaults.read_text(encoding="utf-8"))


    def test_main_updates_lan_ip_in_both_inputs(self):
        defaults = self.root / "integration_defaults.h"
        dictionary = self.root / "integration_dict.cn"
        defaults.write_text(
            '#define DEF_LAN_ADDR "192.168.2.1"\n'
            '#define DEF_LAN_DHCP_BEG "192.168.2.100"\n'
            '#define DEF_LAN_DHCP_END "192.168.2.244"\n',
            encoding="utf-8",
        )
        dictionary.write_text("lan=192.168.2.1\n", encoding="utf-8")

        environment = {"CUSTOMIZATION_JSON": json.dumps({"lanip": "192.168.2.9"})}
        arguments = ["apply_customization.py", str(defaults), str(dictionary)]
        with mock.patch.dict(os.environ, environment):
            with mock.patch.object(sys, "argv", arguments):
                apply_customization.main()

        rendered = defaults.read_text(encoding="utf-8")
        self.assertIn('#define DEF_LAN_ADDR "192.168.2.9"', rendered)
        self.assertIn('#define DEF_LAN_DHCP_BEG "192.168.2.100"', rendered)
        self.assertIn('#define DEF_LAN_DHCP_END "192.168.2.244"', rendered)
        self.assertEqual("lan=192.168.2.9\n", dictionary.read_text(encoding="utf-8"))

    def test_main_rejects_shared_header_and_dictionary_path(self):
        target = self.root / "same-input.h"
        original = (
            '#define DEF_LAN_ADDR "192.168.2.1"\n'
            '#define DEF_LAN_DHCP_BEG "192.168.2.100"\n'
            '#define DEF_LAN_DHCP_END "192.168.2.244"\n'
        )
        target.write_text(original, encoding="utf-8")
        environment = {"CUSTOMIZATION_JSON": json.dumps({"lanip": "192.168.2.9"})}
        arguments = ["apply_customization.py", str(target), str(target)]
        with mock.patch.dict(os.environ, environment):
            with mock.patch.object(sys, "argv", arguments):
                with self.assertRaises(SystemExit):
                    apply_customization.main()
        self.assertEqual(original, target.read_text(encoding="utf-8"))

    def test_duplicate_paths_are_rejected_before_publication(self):
        alias = self.root / "." / "defaults.h"
        with self.assertRaises(ValueError):
            apply_customization.atomic_publish_text_files([
                (self.defaults, "first\n"),
                (alias, "second\n"),
            ])
        self.assertEqual('#define DEF_LAN_ADDR "192.168.2.1"\n',
                         self.defaults.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
