# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
import subprocess
import unittest

import mock
import openmediavault.procutils


class SubprocessTestCase(unittest.TestCase):
    def _validate_call_args(self, call_args):
        kwargs = call_args[1]
        self.assertIn("env", kwargs)
        env = kwargs["env"]
        self.assertIn("LANG", env)
        self.assertEqual(env["LANG"], "C.UTF-8")

    def test_popen(self):
        with mock.patch("subprocess.Popen.__init__") as mock_call:
            p = openmediavault.procutils.Popen(
                ["cat", "/proc/uptime"], stdout=subprocess.PIPE, shell=False
            )
            self._validate_call_args(mock_call.call_args_list[0])

    def test_call(self):
        with mock.patch("subprocess.call") as mock_call:
            openmediavault.procutils.call(["whoami"])
            self._validate_call_args(mock_call.call_args_list[0])

    def test_check_call(self):
        with mock.patch("subprocess.check_call") as mock_check_call:
            openmediavault.procutils.check_call(["ls"])
            self._validate_call_args(mock_check_call.call_args_list[0])

    def test_check_output(self):
        with mock.patch("subprocess.check_output") as mock_check_output:
            openmediavault.procutils.check_output(["uname"])
            self._validate_call_args(mock_check_output.call_args_list[0])


if __name__ == "__main__":
    unittest.main()
