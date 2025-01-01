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
import syslog
import unittest

import mock
import openmediavault.log


class LogTestCase(unittest.TestCase):
    @mock.patch('syslog.syslog')
    @mock.patch('sys.stdout.write')
    def test_info(self, mock_write, mock_syslog):
        openmediavault.log.info('This is a test')
        mock_write.assert_called_with('INFO: This is a test\n')
        mock_syslog.assert_called_with(syslog.LOG_INFO, 'This is a test')

    @mock.patch('syslog.syslog')
    @mock.patch('sys.stdout.write')
    def test_warning(self, mock_write, mock_syslog):
        openmediavault.log.warning('%s - %d', 'foo', 1)
        mock_write.assert_called_with('WARNING: foo - 1\n')
        mock_syslog.assert_called_with(syslog.LOG_WARNING, 'foo - 1')

    @mock.patch('syslog.syslog')
    @mock.patch('sys.stderr.write')
    def test_error(self, mock_write, mock_syslog):
        openmediavault.log.error('%s bar', 'foo')
        mock_write.assert_called_with('ERROR: foo bar\n')
        mock_syslog.assert_called_with(syslog.LOG_ERR, 'foo bar')

    @mock.patch('syslog.syslog')
    @mock.patch('sys.stderr.write')
    def test_debug(self, mock_write, mock_syslog):
        openmediavault.log.debug('xyz', verbose=False)
        mock_write.assert_not_called()
        mock_syslog.assert_called_with(syslog.LOG_DEBUG, 'xyz')


if __name__ == "__main__":
    unittest.main()
