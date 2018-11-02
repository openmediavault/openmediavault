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

import json
import openmediavault.config
import os
import re
import subprocess

# Import Salt libs
import salt.utils.network
from salt.utils.decorators.jinja import jinja_filter


def get_config(id, identifier=None):
    db = openmediavault.config.Database()
    objs = db.get(id, identifier)
    if isinstance(objs, list):
        return [obj.get_dict() for obj in objs]
    return objs.get_dict()


def get_config_by_filter(id, filter):
    db = openmediavault.config.Database()
    objs = db.get_by_filter(id, openmediavault.config.DatabaseFilter(filter))
    if isinstance(objs, list):
        return [obj.get_dict() for obj in objs]
    return objs.get_dict()


def get_sharedfolder_path(uuid):
    sf_obj = get_config('conf.system.sharedfolder', uuid)
    mp_obj = get_config('conf.system.filesystem.mountpoint',
                        sf_obj['mntentref'])
    return os.path.join(mp_obj['dir'], sf_obj['reldirpath'])


def get_sharedfolder_name(uuid):
    sf_obj = get_config('conf.system.sharedfolder', uuid)
    return sf_obj['name']


def get_sharedfolder_mount_dir(uuid):
    sf_obj = get_config('conf.system.sharedfolder', uuid)
    mp_obj = get_config('conf.system.filesystem.mountpoint',
                        sf_obj['mntentref'])
    return mp_obj['dir']


def is_ipv6_enabled():
    """
    Check whether IPv6 is enabled.
    :return: Return True if IPv6 is enabled, otherwise False.
    :rtype: bool
    """
    if not os.path.exists('/proc/net/if_inet6'):
        return False
    with open('/proc/net/if_inet6') as f:
        lines = f.readlines()
    # Filter unwanted interfaces.
    lines = [l for l in lines if not re.match(r'^\s+lo$', l)]
    return len(lines) > 0


@jinja_filter('network_prefix_len')
def get_net_size(mask):
    """
    Turns an IPv4 netmask into it's corresponding prefix length
    (255.255.255.0 -> 24 as in 192.168.1.10/24).
    """
    return salt.utils.network.get_net_size(mask)
