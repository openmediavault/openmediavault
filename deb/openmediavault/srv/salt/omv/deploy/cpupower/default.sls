# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

# Documentation/Howto:
# https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=894906

{% set config = salt['omv_conf.get']('conf.system.powermngmnt') %}

install_cpupower_script:
  file.managed:
    - name: "/usr/libexec/cpupower"
    - source:
      - salt://{{ tpldir }}/files/usr-libexec-cpupower.j2
    - user: root
    - group: root
    - mode: 755

install_cpupower_systemd_service:
  file.managed:
    - name: "/usr/lib/systemd/system/cpupower.service"
    - source: salt://{{ tpldir }}/files/usr-lib-systemd-system-cpupower_service.j2
    - user: root
    - group: root
    - mode: 644

cpupower_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:
    - onchanges:
      - file: install_cpupower_systemd_service

{% if config.cpufreqgovernor != "" and grains['virtual'] == "physical" %}

configure_cpupower_conf:
  file.managed:
    - name: "/etc/cpupower-service.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-cpupower-service_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

divert_cpupower_conf:
  omv_dpkg.divert_add:
    - name: "/etc/cpupower-service.conf"

start_cpupower_service:
  service.running:
    - name: cpupower
    - enable: True
    - watch:
      - file: configure_cpupower_conf

{% else %}

stop_cpupower_service:
  service.dead:
    - name: cpupower
    - enable: False

{% endif %}
