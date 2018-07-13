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

{% set config = salt['omv.get_config']('conf.system.apt.distribution') %}
{% set use_kernel_backports = salt['pillar.get']('default:OMV_APT_USE_KERNEL_BACKPORTS', True) -%}
{% set proxy_config = salt['omv.get_config']('conf.system.network.proxy') %}

configure_apt_sources_list_openmediavault:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault.list"
    - source:
      - salt://{{ slspath }}/files/etc-apt-sources_list_d-openmediavault_list.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_apt_proxy:
  file.managed:
    - name: "/etc/apt/apt.conf.d/99openmediavault-proxy"
    - source:
      - salt://{{ slspath }}/files/etc-apt-apt_conf_d-99openmediavault-proxy.j2
    - context:
        config: {{ proxy_config | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 640

{% if not use_kernel_backports | to_bool %}

remove_apt_pref_kernel_backports:
  file.absent:
    - name: "/etc/apt/preferences.d/openmediavault-kernel-backports.pref"

remove_apt_sources_list_kernel_backports:
  file.absent:
    - name: "/etc/apt/sources.list.d/openmediavault-kernel-backports.list"

{% else %}

configure_apt_pref_kernel_backports:
  file.managed:
    - name: "/etc/apt/preferences.d/openmediavault-kernel-backports.pref"
    - source:
      - salt://{{ slspath }}/files/etc-apt-preferences_d-openmediavault-kernel-backports_pref.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

configure_apt_sources_list_kernel_backports:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault-kernel-backports.list"
    - source:
      - salt://{{ slspath }}/files/etc-apt-sources_list_d-openmediavault-kernel-backports_list.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% endif %}

refresh_apt_database:
  module.run:
    - name: pkg.refresh_db
