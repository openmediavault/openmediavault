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

# Documentation/Howto:
# https://help.ubuntu.com/community/IptablesHowTo
# http://www.linuxhomenetworking.com/wiki/index.php/Quick_HOWTO_:_Ch14_:_Linux_Firewalls_Using_iptables
# http://www.cyberciti.biz/tips/linux-iptables-how-to-specify-a-range-of-ip-addresses-or-ports.html
# http://manpages.debian.org/cgi-bin/man.cgi?query=iptables&apropos=0&sektion=0&manpath=Debian+7.8+wheezy&format=html&locale=en

{% set rules_config = salt['omv_conf.get']('conf.system.network.iptables.rule') %}
{% set num_inet_rules = salt['omv_conf.get_by_filter'](
  'conf.system.network.iptables.rule',
  {'operator': 'stringEquals', 'arg0': 'family', 'arg1': 'inet'}) | length %}
{% set num_inet6_rules = salt['omv_conf.get_by_filter'](
  'conf.system.network.iptables.rule',
  {'operator': 'stringEquals', 'arg0': 'family', 'arg1': 'inet6'}) | length %}

configure_firewall_script:
  file.managed:
    - name: "/etc/iptables/openmediavault-firewall.sh"
    - source:
      - salt://{{ tpldir }}/files/etc_iptables_openmediavault-firewall.j2
    - template: jinja
    - context:
        rules: {{ rules_config | json }}
        num_inet_rules: {{ num_inet_rules }}
        num_inet6_rules: {{ num_inet6_rules }}
    - user: root
    - group: root
    - mode: 750
    - makedirs: True

configure_firewall_unit_file:
  file.managed:
    - name: "/etc/systemd/system/openmediavault-firewall.service"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Unit]
        Description=openmediavault iptables firewall service
        After=network.target

        [Service]
        Type=oneshot
        RemainAfterExit=yes
        ExecStart=/etc/iptables/openmediavault-firewall.sh start
        ExecStop=/etc/iptables/openmediavault-firewall.sh stop

        [Install]
        WantedBy=multi-user.target
    - user: root
    - group: root
    - mode: 644

iptables_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

enable_firewall_service:
  service.enabled:
    - name: openmediavault-firewall.service
    - enable: True

restart_firewall_service:
  module.run:
    - service.restart:
      - name: openmediavault-firewall.service
