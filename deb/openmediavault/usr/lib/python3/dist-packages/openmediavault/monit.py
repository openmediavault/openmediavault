# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
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
__all__ = ['Monit']

import re
import subprocess

import openmediavault.subprocess


class Monit:
    def __init__(self, name):
        self._name = name

    @property
    def name(self):
        return self._name

    def _exec(self, action, quiet):
        try:
            openmediavault.subprocess.check_call(
                ['monit', action, self.name],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
        except subprocess.CalledProcessError as e:
            if not quiet:
                raise e

    def start(self, quiet=False):
        """
        Start the named service and enable monitoring for it.
        :param quiet: Do not throw an error on failure.
            Defaults to ``False``.
        :type: bool
        """
        self._exec('start', quiet)

    def stop(self, quiet=False):
        """
        Stop the named service and disable its monitoring.
        :param quiet: Do not throw an error on failure.
            Defaults to ``False``.
        :type: bool
        """
        self._exec('stop', quiet)

    def restart(self, quiet=False):
        """
        Restart the named service.
        :param quiet: Do not throw an error on failure.
            Defaults to ``False``.
        :type: bool
        """
        self._exec('restart', quiet)

    def monitor(self, quiet=False):
        """
        Enable monitoring of the named service.
        :param quiet: Do not throw an error on failure.
            Defaults to ``False``.
        :type: bool
        """
        self._exec('monitor', quiet)

    def unmonitor(self, quiet=False):
        """
        Disable monitoring of the named service.
        :param quiet: Do not throw an error on failure.
            Defaults to ``False``.
        :type: bool
        """
        self._exec('unmonitor', quiet)

    def status(self):
        """
        Get the status of a monitored process.
        """
        output = openmediavault.subprocess.check_output(
            ['monit', '-B', 'summary']
        )
        # Parse the command output.
        # Example:
        # Monit 5.20.0 uptime: 6m
        #  Service Name              Status                      Type
        #  omv4stretch               Running                     System
        #  rrdcached                 Running                     Process
        #  php5-fpm                  Execution failed | Does...  Process
        #  nginx                     Running                     Process
        #  omv-engined               Not monitored               Process
        #  collectd                  Execution failed | Does...  Process
        #  rootfs                    Accessible                  Filesystem
        for line in output.splitlines():
            m = re.match(
                r'^(\S+)\s+(.+)\s+Process$',
                line.decode().strip(),
                flags=re.IGNORECASE,
            )
            if not m:
                continue
            if m.group(1) != self.name:
                continue
            return m.group(2).strip().lower()
        return 'unknown'

    def is_running(self):
        """
        Check whether the specified service is running.
        :returns: ``True`` if the service is running, otherwise ``False``.
        :rtype: bool
        """
        return 'running' == self.status()
