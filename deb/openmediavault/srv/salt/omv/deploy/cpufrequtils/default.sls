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
# http://technowizah.com/2007/01/debian-how-to-cpu-frequency-management.html
# http://wiki.hetzner.de/index.php/Cool%27n%27Quiet
# http://wiki.ubuntuusers.de/powernowd
# https://wiki.debian.org/CpuFrequencyScaling
# https://kernel.org/doc/Documentation/cpu-freq/governors.txt

{% set config = salt['omv_conf.get']('conf.system.powermngmnt') %}

configure_default_cpufrequtils:
  file.managed:
    - name: "/etc/default/cpufrequtils"
    - source:
      - salt://{{ tpldir }}/files/cpufrequtils.j2
    - template: jinja
    - context:
        cpufreq: {{ config.cpufreq }}
    - user: root
    - group: root
    - mode: 644

configure_default_loadcpufreq:
  file.managed:
    - name: "/etc/default/loadcpufreq"
    - source:
      - salt://{{ tpldir }}/files/loadcpufreq.j2
    - template: jinja
    - context:
        cpufreq: {{ config.cpufreq }}
    - user: root
    - group: root
    - mode: 644

{% if config.cpufreq %}

start_cpufrequtils_service:
  service.running:
    - name: cpufrequtils
    - enable: True
    - watch:
      - file: configure_default_cpufrequtils

start_loadcpufreq_service:
  service.running:
    - name: loadcpufreq
    - enable: True
    - watch:
      - file: configure_default_loadcpufreq

{% else %}

# Note, when disabling the 'cpufrequtils' service the changes
# will not take effect before the system is rebooted.
{% for service in ['loadcpufreq', 'cpufrequtils'] %}

stop_{{ service }}_service:
  service.dead:
    - name: {{ service }}
    - enable: False

{% endfor %}

{% endif %}
