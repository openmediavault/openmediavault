# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

{% set dirpath = '/srv/salt' | path_join(tpldir) %}
{% set k3s_version = salt['pillar.get']('default:OMV_K8S_K3S_VERSION', 'v1.33.5+k3s1') %}
{% set k8s_config = salt['omv_conf.get']('conf.service.k8s') %}

include:
{% for file in salt['file.readdir'](dirpath) | sort %}
{% if file | regex_match('^(\d+.+).sls$', ignorecase=True) %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endfor %}

{% if k8s_config.enable | to_bool %}

install_k3s:
  cmd.run:
    - name: set -o pipefail; wget -O - https://get.k3s.io | INSTALL_K3S_SKIP_ENABLE=true INSTALL_K3S_VERSION='{{ k3s_version }}' {% if k8s_config.datastore == "etcd" %}INSTALL_K3S_EXEC="--cluster-init"{% endif %} sh -
    - shell: /usr/bin/bash
    - onlyif: "! which k3s || test -e /var/lib/openmediavault/upgrade_k3s"
    - failhard: True

remove_k3s_upgrade_flag:
  file.absent:
    - name: "/var/lib/openmediavault/upgrade_k3s"

# remove_k3s_helm_upgrade_flag:
#   file.absent:
#     - name: "/var/lib/openmediavault/upgrade_helm"

fix_k3s_systemd_unit_file_mode_bits:
  file.managed:
    - name: /etc/systemd/system/k3s.service
    - replace: False
    - create: False
    - mode: 644

start_k3s_service:
  service.running:
    - name: k3s
    - enable: True

{% else %}

stop_k3s_service:
  service.dead:
    - name: k3s
    - enable: False

{% endif %}
