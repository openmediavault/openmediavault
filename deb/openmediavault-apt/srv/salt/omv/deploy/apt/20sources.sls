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
# https://repolib.readthedocs.io/en/latest/deb822-format.html#deb822-style-format

{% set sources_prefix = salt['pillar.get']('default:OMV_APT_CUSTOM_SOURCES_LIST_PREFIX', 'openmediavault-custom-') %}
{% set sources = salt['omv_conf.get']('conf.system.apt.source') %}

cleanup_apt_sources_list_deb822:
  module.run:
    - file.find:
      - path: "/etc/apt/sources.list.d/"
      - iname: "{{ sources_prefix }}*.sources"
      - delete: "f"

{% for item in sources %}

configure_apt_sources_list_deb822_{{ item.uuid }}:
  file.managed:
    - name: "/etc/apt/sources.list.d/{{ sources_prefix }}{{ item.uuid }}.sources"
    - source:
      - salt://{{ tpldir }}/files/etc-apt-sources_list_d-sources.j2
    - context:
        config: {{ item | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% endfor %}
