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

# Documentation/Howto:
# https://docs.saltstack.com/en/latest/ref/states/all/salt.states.network.html
# https://github.com/saltstack/salt/blob/develop/salt/states/network.py
# https://github.com/saltstack/salt/blob/develop/salt/modules/debian_ip.py
# https://github.com/saltstack/salt/blob/develop/salt/templates/debian_ip/debian_eth.jinja
# http://www.kernel.org/doc/Documentation/networking/bonding.txt
# http://www.kernel.org/doc/Documentation/networking/ip-sysctl.txt
# http://www.cyberciti.biz/tips/configuring-static-routes-in-debian-or-red-hat-linux-systems.html
# http://www.itsyourip.com/networking/add-persistent-static-routes-in-debian-linux
# http://wiki.debian.org/Bonding
# http://www.thomas-krenn.com/de/wiki/NIC_Bonding_unter_Debian
# http://www.linuxfoundation.org/collaborate/workgroups/networking/bonding
# http://www.howtoforge.com/nic-bonding-on-debian-lenny
# http://wiki.linuxmce.org/index.php/MTU
# http://www.simpledns.com/private-ipv6.aspx

{% set dns = salt['omv.get_config']('conf.system.network.dns') %}
{% set interfaces = salt['omv.get_config_by_filter'](
  'conf.system.network.interface',
  '{"operator": "stringEquals", "arg0": "type", "arg1": "bond"}') %}

configure_interfaces_bond:
  file.append:
    - name: "/etc/network/interfaces"
    - sources:
      - salt://{{ slspath }}/files/bond.j2
    - template: jinja
    - context:
        dns: {{ dns | json }}
        interfaces: {{ interfaces | json }}
