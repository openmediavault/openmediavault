# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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
from .block import BlockDevice


class StorageDevice(BlockDevice):
    @property
    def model(self):
        """
        Get the device model.
        :return: The device model, otherwise an empty string.
        :rtype: str
        """
        file = '/sys/block/{}/device/model'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                return f.readline().strip()
        except (IOError, FileNotFoundError):
            pass
        return ''

    def is_rotational(self):
        """
        Check if the device is of rotational or non-rotational type.
        See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
        :return: Return True if device is rotational, otherwise False.
        :rtype: bool
        """
        if self.has_udev_property('ID_SSD'):
            # If ID_SSD is not 1 then it is rotational.
            return self.get_udev_property('ID_SSD') != '1'
        if self.has_udev_property('ID_ATA_ROTATION_RATE_RPM'):
            # If ID_ATA_ROTATION_RATE_RPM is non-zero then it is rotational.
            return self.get_udev_property('ID_ATA_ROTATION_RATE_RPM') != '0'
        if self.has_udev_property('ID_ATA_FEATURE_SET_AAM'):
            # If ID_ATA_FEATURE_SET_AAM is non-zero then it is rotational.
            return self.get_udev_property('ID_ATA_FEATURE_SET_AAM') != '0'
        # Use kernel attribute.
        file = '/sys/block/{}/queue/rotational'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                # If file content is non-zero then it is rotational.
                return f.readline().strip() != '0'
        except (IOError, FileNotFoundError):
            pass
        # Use heuristic.
        return 'SSD' in self.model
