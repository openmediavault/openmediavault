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
# https://help.ubuntu.com/community/IptablesHowTo
# http://www.linuxhomenetworking.com/wiki/index.php/Quick_HOWTO_:_Ch14_:_Linux_Firewalls_Using_iptables
# http://www.cyberciti.biz/tips/linux-iptables-how-to-specify-a-range-of-ip-addresses-or-ports.html
# http://manpages.debian.org/cgi-bin/man.cgi?query=iptables&apropos=0&sektion=0&manpath=Debian+7.8+wheezy&format=html&locale=en

{% set rules = salt['omv.get_config']('conf.system.network.iptables.rule') %}
{% set num_inet_rules = salt['omv.get_config_by_filter'](
  'conf.system.network.iptables.rule',
  '{"operator": "stringEquals", "arg0": "family", "arg1": "inet"}') | length %}
{% set num_inet6_rules = salt['omv.get_config_by_filter'](
  'conf.system.network.iptables.rule',
  '{"operator": "stringEquals", "arg0": "family", "arg1": "inet6"}') | length %}

configure_ifupdown_iptables_rules:
  file.managed:
    - name: "/etc/network/if-pre-up.d/openmediavault-iptables"
    - source:
      - salt://{{ slspath }}/files/ifupdown.j2
    - template: jinja
    - context:
        rules: {{ rules | json }}
        num_inet_rules: {{ num_inet_rules }}
        num_inet6_rules: {{ num_inet6_rules }}
    - user: root
    - group: root
    - mode: 750

apply_ifupdown_iptables_rules:
  cmd.run:
    - name: "/etc/network/if-pre-up.d/openmediavault-iptables"
    - onchanges:
      - file: configure_ifupdown_iptables_rules
