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
import os
import re
import subprocess

# Import Salt libs
import salt.utils.network
from salt.utils.decorators.jinja import jinja_filter

def _get_config(args):
	"""
	Use the CLI tool to query the database. As soon as Salt is Python 3
	compliant, then use the OMV Python API.
	"""
	#db = openmediavault.config.Database()
	#return db.get(id, uuid)
	output = subprocess.check_output(args, env=dict(os.environ, LANG='C'))
	return json.loads(output)

def get_config(id, uuid=None):
	args = ['omv-confdbadm', 'read']
	if uuid is not None:
		args.append('--uuid')
		args.append(uuid)
	args.append(id)
	return _get_config(args)

def get_config_by_filter(id, filter):
	args = ['omv-confdbadm', 'read', '--filter']
	args.append(filter)
	args.append(id)
	return _get_config(args)

def get_sharedfolder_path(uuid):
	sfObj = get_config('conf.system.sharedfolder', uuid)
	mpObj = get_config('conf.system.filesystem.mountpoint', sfObj['mntentref'])
	return os.path.join(mpObj['dir'], sfObj['reldirpath'])

def get_sharedfolder_name(uuid):
	sfObj = get_config('conf.system.sharedfolder', uuid)
	return sfObj['name']

def get_sharedfolder_mount_dir(uuid):
	sfObj = get_config('conf.system.sharedfolder', uuid)
	mpObj = get_config('conf.system.filesystem.mountpoint', sfObj['mntentref'])
	return mpObj['dir']

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
