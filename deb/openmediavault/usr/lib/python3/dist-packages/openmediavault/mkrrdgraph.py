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
__all__ = ['IPlugin', 'load_collectd_config']

import abc
import os
import re
import shutil

import openmediavault


class IPlugin(metaclass=abc.ABCMeta):  # lgtm[py/syntax-error]
    @abc.abstractmethod
    def create_graph(self, config):
        """
        Build the RRD graph.
        """


def call_rrdtool_graph(args):
    """
    Call the rrdtool command line executable with the given arguments.
    """
    # The command below does not work because the RRD tool synatx is escaped
    # and the graph legend is not rendered as expected.
    # return openmediavault.subprocess.check_output(['rrdtool', 'graph', *args])
    return os.system(' '.join(['rrdtool', 'graph', *args, '>/dev/null']))


def copy_placeholder_image(filename):
    """
    Helper function to copy the error graph image.
    :param filename: The destination filename.
    """
    src = openmediavault.getenv(
        'OMV_RRDGRAPH_ERROR_IMAGE',
        '/usr/share/openmediavault/icons/rrd_graph_error_64.png',
    )
    shutil.copyfile(src, filename)


def load_collectd_config(plugin_name, option):
    """
    A simple configuration loader.
    """
    result = []
    section_found = False
    filename = os.path.join(
        openmediavault.getenv(
            'OMV_COLLECTD_CONFIG_DIR', '/etc/collectd/collectd.conf.d'
        ),
        '{}.conf'.format(plugin_name),
    )
    if os.path.exists(filename):
        with open(filename, 'r') as fd:
            for line in fd:
                line = line.strip()
                m = re.match(
                    r'^<Plugin\s+{}\s*>$'.format(plugin_name),
                    line,
                    flags=re.IGNORECASE,
                )
                if m:
                    section_found = True
                    continue
                if section_found:
                    m = re.match(
                        r'^\s*{}\s*(.+)$'.format(option),
                        line,
                        flags=re.IGNORECASE,
                    )
                    if m:
                        result.append(m.group(1).strip('"\''))
                        continue
                    if re.match(r'^</Plugin>$', line, flags=re.IGNORECASE):
                        break
    return result
