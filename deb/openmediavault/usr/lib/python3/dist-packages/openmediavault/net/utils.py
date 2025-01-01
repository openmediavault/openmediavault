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
import re


def is_ethernet(name):
    """
    Check if the given interface device name is an ethernet interface.
    :param name: The interface device name to check.
    :type name: str
    :return: Return ``True`` if the interface device name is an
        ethernet interface, otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(name, str):
        return False
    # Examples:
    # - eth0
    # - enp2s0
    # - enp2s0f0, enp5165p0s0
    # - enx00259025963a
    # - ens1
    # - eno16777736
    # - encf5f0, enx026d3c00000a
    # - end0
    #
    # Predictable network interface device name types:
    # - BCMA bus core number
    #   b<number>
    # - CCW bus group name, without leading zeros [s390]
    #   c<bus_id>
    # - on-board device index number
    #   o<index>[n<phys_port_name>|d<dev_port>]
    # - hot-plug slot index number
    #   s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
    # - MAC address
    #   x<MAC>
    # - PCI geographical location
    #   [P<domain>]p<bus>s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
    # - USB port number chain
    #   [P<domain>]p<bus>s<slot>[f<function>][u<port>][..][c<config>][i<interface>]
    # - Device tree
    #   d<number>
    #
    # Understanding systemd’s predictable network device names:
    # https://github.com/systemd/systemd/blob/master/src/udev/udev-builtin-net_id.c
    return bool(
        re.match(
            r'^eth[0-9]+|'
            r'en(b\d+|c\d+|d\d+|o\d+(n\S+|d\d+)?|s\d+(f\d+)?(n\S+|d\d+)?|'
            r'x[\da-f]{12}|(P\d+)?p\d+s\d+(f\d+)?(n\S+|d\d+)?|'
            r'(P\d+)?p\d+s\d+(f\d+)?(u\d+)*(c\d+)?(i\d+)?)$',
            name
        )
    )


def is_wifi(name):
    """
    Check if the given interface device name is a wifi interface.
    :param name: The interface device name to check.
    :type name: str
    :return: Return ``True`` if the interface device name is a
        wifi interface, otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(name, str):
        return False
    # Examples:
    # - wlan1
    # - wlp3s0
    #
    # Predictable network interface device name types:
    # - BCMA bus core number
    #   b<number>
    # - CCW bus group name, without leading zeros [s390]
    #   c<bus_id>
    # - on-board device index number
    #   o<index>[n<phys_port_name>|d<dev_port>]
    # - hot-plug slot index number
    #   s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
    # - MAC address
    #   x<MAC>
    # - PCI geographical location
    #   [P<domain>]p<bus>s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
    # - USB port number chain
    #   [P<domain>]p<bus>s<slot>[f<function>][u<port>][..][c<config>][i<interface>]
    #
    # Understanding systemd’s predictable network device names:
    # https://github.com/systemd/systemd/blob/master/src/udev/udev-builtin-net_id.c
    return bool(
        re.match(
            r'^wlan[0-9]+|wl(b\d+|c\d+|o\d+(n\S+|d\d+)?|'
            r's\d+(f\d+)?(n\S+|d\d+)?|'
            r'x[\da-f]{12}|(P\d+)?p\d+s\d+(f\d+)?(n\S+|d\d+)?|'
            r'(P\d+)?p\d+s\d+(f\d+)?(u\d+)*(c\d+)?(i\d+)?)$',
            name
        )
    )


def is_bond(name):
    """
    Check if the given interface device name is a bond interface.
    :param name: The interface device name to check.
    :type name: str
    :return: Return ``True`` if the interface device name is a
        bond interface, otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(name, str):
        return False
    return bool(re.match(r'^bond[0-9]+$', name))


def is_bridge(name):
    """
    Check if the given interface device name is a bridge interface.
    :param name: The interface device name to check.
    :type name: str
    :return: Return ``True`` if the interface device name is a
        bridge interface, otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(name, str):
        return False
    return bool(re.match(r'^br[0-9]+$', name))


def is_vlan(name):
    """
    Check if the given interface device name is a vlan interface.
    :param name: The interface device name to check.
    :type name: str
    :return: Return ``True`` if the interface device name is a
        vlan interface, otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(name, str):
        return False
    return bool(re.match(r'^(\S+\d+)\.(\d+)$', name))
