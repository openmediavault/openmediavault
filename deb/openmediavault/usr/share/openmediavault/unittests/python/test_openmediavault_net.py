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

import openmediavault.net


class NetTestCase(unittest.TestCase):
    def test_is_ethernet(self):
        self.assertTrue(openmediavault.net.is_ethernet('enp2s0'))
        self.assertFalse(openmediavault.net.is_ethernet('br1'))

    def test_is_wifi(self):
        self.assertTrue(openmediavault.net.is_wifi('wlan0'))
        self.assertTrue(openmediavault.net.is_wifi('wlp3s0'))
        self.assertFalse(openmediavault.net.is_wifi('venet2'))

    def test_is_bond(self):
        self.assertTrue(openmediavault.net.is_bond('bond0'))
        self.assertFalse(openmediavault.net.is_bond('wlan1'))

    def test_is_bridge(self):
        self.assertTrue(openmediavault.net.is_bridge('br2'))
        self.assertFalse(openmediavault.net.is_bridge('thunderbolt0'))

    def test_is_vlan(self):
        self.assertTrue(openmediavault.net.is_vlan('eth0.1'))
        self.assertFalse(openmediavault.net.is_vlan('ens1'))


if __name__ == "__main__":
    unittest.main()
