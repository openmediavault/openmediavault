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
import unittest

import mock
import openmediavault.mkrrdgraph


class MkRrdGraphTestCase(unittest.TestCase):
    @mock.patch("os.system")
    def test_call_rrdtool_graph(self, mock_system):
        args = []
        # yapf: disable
        args.append('--slope-mode')
        args.extend(['--lower-limit', '0'])
        args.extend(['--units-exponent', '0'])
        args.append('GPRINT:lmax:MAX:"%4.2lf Max"')
        args.append('GPRINT:lavg:LAST:"%4.2lf Last\\l"')
        # yapf: enable
        openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        mock_system.assert_called_with(
            "rrdtool graph --slope-mode "
            "--lower-limit 0 --units-exponent 0 "
            "GPRINT:lmax:MAX:\"%4.2lf Max\" "
            "GPRINT:lavg:LAST:\"%4.2lf Last\\l\" "
            ">/dev/null"
        )

    @mock.patch("shutil.copyfile")
    def test_copy_placeholder_image(self, mock_copyfile):
        openmediavault.setenv("OMV_RRDGRAPH_ERROR_IMAGE", "foo.png")
        openmediavault.mkrrdgraph.copy_placeholder_image("bar.png")
        mock_copyfile.assert_called_with("foo.png", "bar.png")

    @mock.patch("os.path.exists", return_value=True)
    @mock.patch("builtins.open", new_callable=mock.mock_open,
                read_data="\n".join([
                    "LoadPlugin interface",
                    "<Plugin interface>",
                    "  Interface 'ens6'",
                    "  Interface \"ens7\"",
                    "  IgnoreSelected false",
                    "</Plugin>",
                ]))
    def test_load_collectd_config(self, mock_open, mock_exists):
        openmediavault.setenv("OMV_COLLECTD_CONFIG_DIR", "/x/y/z")
        result = openmediavault.mkrrdgraph.load_collectd_config(
            "interface", "Interface"
        )
        mock_exists.assert_called_with("/x/y/z/interface.conf")
        mock_open.assert_called_with("/x/y/z/interface.conf", "r")
        self.assertListEqual(result, ["ens6", "ens7"])


if __name__ == "__main__":
    unittest.main()
