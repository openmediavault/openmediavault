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

import openmediavault.fs


class FsTestCase(unittest.TestCase):
    def test_make_mount_path_1(self):
        self.assertEqual(
            openmediavault.fs.make_mount_path(
                '9d60bdbb-6946-4b56-b677-a7fb77e7ff4f'
            ),
            '/srv/9d60bdbb-6946-4b56-b677-a7fb77e7ff4f/',
        )

    def test_make_mount_path_1(self):
        self.assertEqual(
            openmediavault.fs.make_mount_path(
                '/dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038'
            ),
            '/srv/_dev_disk_by-id_scsi-SATA_IBM-DHEA-36481_SG0SGF08038/',
        )


if __name__ == "__main__":
    unittest.main()
